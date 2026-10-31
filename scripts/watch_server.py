#!/usr/bin/env python3
"""Bounded host-local server supervision with anonymous telemetry and recovery.

Run as root on the server host. Unlike observe_server.py this explicitly may
start/restart the selected service and repair its expected TCP API listener.
It never changes gameplay state or enables/disables asynchronous simulation.
"""
import argparse
import datetime as dt
import json
import os
from pathlib import Path
import signal
import ssl
import subprocess
import time
import urllib.request

from observe_server import api_state, bridge_counters, journal_counts
from install_pipeline_guard_live import read_status as pipeline_guard_status


def utc_now():
    return dt.datetime.now(dt.timezone.utc).isoformat()


def service_state(service):
    output = subprocess.check_output(
        ["systemctl", "show", service, "-p", "ActiveState", "-p", "SubState",
         "-p", "MainPID", "-p", "NRestarts", "-p", "ControlGroup", "-p", "InvocationID"],
        text=True, timeout=10)
    return dict(row.split("=", 1) for row in output.splitlines() if "=" in row)


def engine_pid(state):
    group = state.get("ControlGroup", "")
    if not group or not group.startswith("/") or ".." in group.split("/"):
        return None
    try:
        pids = (Path("/sys/fs/cgroup") / group.lstrip("/") / "cgroup.procs").read_text().split()
    except FileNotFoundError:
        return None
    for pid in pids:
        try:
            if (Path("/proc") / pid / "exe").resolve(strict=True).name == "FactoryServer-Linux-Shipping":
                return int(pid)
        except (FileNotFoundError, ProcessLookupError, PermissionError):
            pass
    return None


def health(port):
    req = urllib.request.Request(
        f"https://127.0.0.1:{port}/api/v1",
        data=b'{"function":"HealthCheck","data":{"clientCustomData":""}}',
        headers={"Content-Type": "application/json"})
    try:
        with urllib.request.urlopen(req, context=ssl._create_unverified_context(), timeout=3) as response:
            return json.load(response).get("data", {}).get("health") == "healthy"
    except (OSError, ValueError):
        return False


def repair_api_proxy(port):
    """Restore expected TCP 7777 only when a healthy alternate API was found."""
    if port == 7777:
        return False
    proxy = next((p for p in ("/usr/lib/systemd/systemd-socket-proxyd",
                              "/lib/systemd/systemd-socket-proxyd") if Path(p).is_file()), None)
    if not proxy:
        raise RuntimeError("systemd_socket_proxy_unavailable")
    subprocess.run(["systemctl", "stop", "satisfactory-api-compat.socket",
                    "satisfactory-api-compat.service"], check=False, capture_output=True, timeout=15)
    socket = """[Unit]
Description=Temporary Satisfactory API compatibility listener
PartOf=satisfactory.service
[Socket]
ListenStream=7777
BindIPv6Only=both
NoDelay=true
"""
    service = f"""[Unit]
Description=Temporary Satisfactory API proxy to active port {port}
PartOf=satisfactory.service
[Service]
ExecStart={proxy} 127.0.0.1:{port}
User=nobody
NoNewPrivileges=true
PrivateTmp=true
"""
    Path("/run/systemd/system/satisfactory-api-compat.socket").write_text(socket)
    Path("/run/systemd/system/satisfactory-api-compat.service").write_text(service)
    subprocess.run(["systemctl", "daemon-reload"], check=True, timeout=15)
    subprocess.run(["systemctl", "start", "satisfactory-api-compat.socket"], check=True, timeout=15)
    return True


def restart_reason(state, healthy, process_age, unhealthy_for, since_action, was_healthy):
    """Respect systemd's existing auto-restart and long world startup grace."""
    if since_action < 60:
        return None
    if state.get("ActiveState") in ("failed", "inactive"):
        return "service_not_running"
    if state.get("ActiveState") != "active" or healthy:
        return None
    if process_age >= 300 and unhealthy_for >= (120 if was_healthy else 300):
        return "api_unresponsive_after_grace"
    return None


def write_json(path, value):
    temporary = path.with_suffix(path.suffix + ".tmp")
    temporary.write_text(json.dumps(value, indent=2) + "\n")
    temporary.replace(path)


def preserve_diagnostics(service, directory, index):
    output = subprocess.check_output(
        ["journalctl", "-u", service, "-n", "250", "--no-pager", "-o", "short-iso-precise"],
        text=True, errors="replace", timeout=15)
    path = directory / f"private-diagnostic-{index:03}.log"
    path.write_text(output)
    path.chmod(0o600)


def watch(args):
    if os.geteuid() != 0:
        raise SystemExit("Run this supervisor as root on the server host")
    os.umask(0o077)
    args.output.mkdir(parents=True, exist_ok=True, mode=0o700)
    args.output.chmod(0o700)
    summary_path = args.output / "summary.json"
    if summary_path.exists():
        report = json.loads(summary_path.read_text())
        if report.get("end_at_epoch") != args.end_at:
            raise RuntimeError("output_directory_belongs_to_another_observation")
    else:
        report = {"started_at": utc_now(), "started_at_epoch": time.time(),
                  "end_at_epoch": args.end_at, "complete": False, "samples": 0,
                  "recoveries": [], "process_changes": [], "api_proxy_repairs": 0,
                  "maximum_connected_players": 0, "tick_rate_min": None, "tick_rate_max": None}
    last_pid = report.get("last_pid")
    pid_since = time.monotonic()
    unhealthy_since = None
    last_action = -float("inf")
    was_healthy = False
    stopped = False

    def stop(_signal, _frame):
        nonlocal stopped
        stopped = True

    signal.signal(signal.SIGTERM, stop)
    signal.signal(signal.SIGINT, stop)
    log_path = args.output / "samples.jsonl"
    with log_path.open("a", buffering=1) as stream:
        while not stopped and time.time() < args.end_at:
            now = time.monotonic()
            sample = {"at": utc_now()}
            try:
                state = service_state(args.service)
                pid = engine_pid(state)
                sample.update(pid=pid, service={k: state.get(k) for k in
                    ("ActiveState", "SubState", "NRestarts", "InvocationID")})
                if pid != last_pid:
                    if last_pid:
                        report["process_changes"].append({"at": utc_now(), "previous_pid": last_pid, "pid": pid})
                        preserve_diagnostics(args.service, args.output, len(report["process_changes"]))
                    last_pid, pid_since = pid, now
                    unhealthy_since, was_healthy = None, False
                sample["api_healthy"] = health(7777)
                sample["api_port"] = 7777 if sample["api_healthy"] else None
                if not sample["api_healthy"] and pid:
                    for port in (7778, 7779):
                        if health(port):
                            sample["api_port"] = port
                            sample["api_healthy"] = True
                            if repair_api_proxy(port):
                                report["api_proxy_repairs"] += 1
                                sample["expected_api_restored"] = health(7777)
                            break
                if sample["api_healthy"]:
                    unhealthy_since, was_healthy = None, True
                    try:
                        sample["api"] = api_state(f"https://127.0.0.1:{sample['api_port']}/api/v1")
                        players = sample["api"].get("numConnectedPlayers")
                        if players is not None:
                            report["maximum_connected_players"] = max(report["maximum_connected_players"], players)
                        tps = sample["api"].get("averageTickRate")
                        if tps is not None:
                            report["tick_rate_min"] = min(report["tick_rate_min"], tps) if report["tick_rate_min"] is not None else tps
                            report["tick_rate_max"] = max(report["tick_rate_max"], tps) if report["tick_rate_max"] is not None else tps
                    except Exception as exc:
                        sample["state_error"] = type(exc).__name__
                elif unhealthy_since is None:
                    unhealthy_since = now
                if pid:
                    try:
                        sample["bridge"] = bridge_counters(pid)
                        mappings = Path(f"/proc/{pid}/maps").read_text()
                        sample["pipeline_guard_loaded"] = "libsatis_pipeline_guard.so" in mappings
                        if sample["pipeline_guard_loaded"]:
                            guard = next(row.split(maxsplit=5)[5] for row in mappings.splitlines()
                                         if "libsatis_pipeline_guard.so" in row and row.split()[2] == "00000000")
                            sample["pipeline_guard"] = pipeline_guard_status(pid, Path(guard))
                    except Exception as exc:
                        # Telemetry failure must not suppress recovery decisions.
                        sample["bridge_error"] = type(exc).__name__
                reason = restart_reason(state, sample["api_healthy"], now - pid_since,
                    now - unhealthy_since if unhealthy_since is not None else 0,
                    now - last_action, was_healthy)
                if reason:
                    preserve_diagnostics(args.service, args.output, 1000 + len(report["recoveries"]))
                    subprocess.run(["systemctl", "reset-failed", args.service], check=False, capture_output=True, timeout=10)
                    action = "start" if reason == "service_not_running" else "restart"
                    subprocess.run(["systemctl", action, "--no-block", args.service], check=True, timeout=10)
                    last_action = now
                    sample["recovery"] = action
                    report["recoveries"].append({"at": utc_now(), "reason": reason, "action": action})
            except Exception as exc:
                sample["observation_error"] = type(exc).__name__
            report["samples"] += 1
            report["last_pid"] = last_pid
            report["last_sample"] = sample
            report["updated_at"] = utc_now()
            stream.write(json.dumps(sample) + "\n")
            write_json(summary_path, report)
            print(json.dumps(sample), flush=True)
            remaining = min(args.interval, max(0, args.end_at - time.time()))
            while remaining > 0 and not stopped:
                delay = min(1, remaining)
                time.sleep(delay)
                remaining -= delay
    report["complete"] = not stopped and time.time() >= args.end_at
    report["ended_at"] = utc_now()
    report["observed_seconds"] = round(time.time() - report["started_at_epoch"], 1)
    try:
        since = dt.datetime.fromtimestamp(report["started_at_epoch"], dt.timezone.utc).strftime("%Y-%m-%d %H:%M:%S UTC")
        report["journal_counts"] = journal_counts(args.service, since)
    except Exception as exc:
        report["journal_error"] = type(exc).__name__
    write_json(summary_path, report)
    print(json.dumps({"complete": report["complete"], "samples": report["samples"],
                      "observed_seconds": report["observed_seconds"], "summary": str(summary_path)}), flush=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--service", default="satisfactory.service")
    parser.add_argument("--end-at", type=float, required=True, help="Absolute UNIX deadline; preserved across supervisor restarts")
    parser.add_argument("--interval", type=float, default=15)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    if args.service != "satisfactory.service" or args.interval < 5:
        parser.error("This host helper supports satisfactory.service and intervals of at least five seconds")
    watch(args)


if __name__ == "__main__":
    main()
