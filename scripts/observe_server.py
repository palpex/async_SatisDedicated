#!/usr/bin/env python3
"""Read bounded, anonymous telemetry on the server host (sudo for /proc/PID/mem).

No instrumentation is injected and no gameplay/server setting is changed.
"""
import argparse
import datetime as dt
import hashlib
import json
import os
from pathlib import Path
import re
import ssl
import subprocess
import time
import urllib.request

COUNTERS = {"ticks": "_ZN12_GLOBAL__N_117intercepted_ticksE",
            "inflight": "_ZN12_GLOBAL__N_120inflight_async_ticksE",
            "dispatched": "_ZN12_GLOBAL__N_116async_dispatchesE",
            "completed": "_ZN12_GLOBAL__N_121completed_async_ticksE",
            "fallbacks": "_ZN12_GLOBAL__N_114sync_fallbacksE",
            "pool_hits": "_ZN12_GLOBAL__N_19pool_hitsE",
            "active_mutations": "_ZN12_GLOBAL__N_126active_buildable_mutationsE",
            "mutation_waits": "_ZN12_GLOBAL__N_114mutation_waitsE",
            "enabled": "_ZN12_GLOBAL__N_118async_tick_enabledE"}

def call(url, function, data=None, token=None):
    headers = {"Content-Type": "application/json"}
    if token:
        headers["Authorization"] = "Bearer " + token
    req = urllib.request.Request(url, json.dumps({"function": function, "data": data or {}}).encode(), headers)
    with urllib.request.urlopen(req, context=ssl._create_unverified_context(), timeout=4) as response:
        result = json.load(response)
    if "errorCode" in result:
        raise RuntimeError(result["errorCode"])
    return result.get("data", {})

def api_state(url):
    login = call(url, "PasswordlessLogin", {"minimumPrivilegeLevel": "Client"})
    state = call(url, "QueryServerState", token=login["authenticationToken"])["serverGameState"]
    # Never report names, save paths, authentication tokens or player identifiers.
    return {key: state.get(key) for key in ("numConnectedPlayers", "averageTickRate", "isGamePaused", "isGameRunning")}

def server_pid():
    for entry in Path("/proc").iterdir():
        if entry.name.isdigit():
            try:
                if (entry / "exe").resolve(strict=True).name == "FactoryServer-Linux-Shipping":
                    return int(entry.name)
            except (FileNotFoundError, PermissionError, ProcessLookupError):
                pass
    raise RuntimeError("server_process_not_found")

def bridge_counters(pid):
    maps = Path(f"/proc/{pid}/maps").read_text().splitlines()
    mapping = next((line.split(maxsplit=5) for line in maps if "libsatis_task_bridge.so" in line and line.split()[2] == "00000000"), None)
    if not mapping:
        return {"loaded": False}
    base = int(mapping[0].split("-")[0], 16)
    symbols = subprocess.check_output(["nm", "--defined-only", mapping[5]], text=True)
    offsets = {parts[2]: int(parts[0], 16) for line in symbols.splitlines() if len(parts := line.split()) == 3}
    result = {"loaded": True, "library_sha256": hashlib.sha256(Path(mapping[5]).read_bytes()).hexdigest()}
    with open(f"/proc/{pid}/mem", "rb", buffering=0) as memory:
        for key, symbol in COUNTERS.items():
            if symbol in offsets:
                memory.seek(base + offsets[symbol])
                width = 1 if key == "enabled" else (4 if key == "active_mutations" else 8)
                result[key] = int.from_bytes(memory.read(width), "little")
            else:
                result[key] = None
    return result

def thread_ticks(pid):
    result = {}
    for entry in Path(f"/proc/{pid}/task").iterdir():
        try:
            line = (entry / "stat").read_text()
            fields = line[line.rfind(")") + 2:].split()
            result[int(entry.name)] = int(fields[11]) + int(fields[12])
        except (FileNotFoundError, ProcessLookupError):
            pass
    return result

def journal_counts(service, since):
    logs = subprocess.check_output(["journalctl", "-u", service, "--since", since, "--no-pager", "-o", "cat"], text=True, errors="replace")
    patterns = {"assertions": r"Assertion failed:", "fatal_signals": r"Signal [0-9]+ caught",
                "critical_errors": r"=== Critical error:", "successful_joins": r"Join succeeded:",
                "inventory_warnings": r"Add failed cause InventoryItemClass was null",
                "netguid_warnings": r"NOT Supported[.]|Unable to resolve default guid",
                "component_warnings": r"Already registered[.] Aborting",
                "saves": r"World Serialization [(]save[)]:"}
    return {key: len(re.findall(pattern, logs)) for key, pattern in patterns.items()}

def observe(args):
    started = dt.datetime.now(dt.timezone.utc)
    samples, previous = [], None
    deadline = time.monotonic() + args.duration
    while True:
        sample = {"at": dt.datetime.now(dt.timezone.utc).isoformat()}
        try:
            pid = server_pid()
            sample.update(pid=pid, bridge=bridge_counters(pid))
            ticks, now = thread_ticks(pid), time.monotonic()
            if previous and previous[0] == pid:
                factor = 100 / os.sysconf("SC_CLK_TCK") / (now - previous[1])
                usages = {tid: max(0, tick - previous[2].get(tid, tick)) * factor for tid, tick in ticks.items()}
                sample["game_thread_cpu_pct"] = round(usages.get(pid, 0), 2)
                sample["process_cpu_pct"] = round(sum(usages.values()), 2)
                sample["active_other_threads"] = sum(value > 0 for tid, value in usages.items() if tid != pid)
            previous = (pid, now, ticks)
            try:
                sample["api"] = api_state(args.url)
            except Exception as exc:
                sample["api_error"] = type(exc).__name__
        except Exception as exc:
            sample["observation_error"] = type(exc).__name__
        samples.append(sample)
        print(json.dumps(sample), flush=True)
        if time.monotonic() >= deadline:
            break
        time.sleep(min(args.interval, max(0, deadline - time.monotonic())))
    report = {"started_at": started.isoformat(), "ended_at": dt.datetime.now(dt.timezone.utc).isoformat(),
              "scope": "bounded observation; no baseline speedup inferred", "samples": samples,
              "journal_counts": journal_counts(args.service, started.strftime("%Y-%m-%d %H:%M:%S UTC"))}
    if args.output:
        Path(args.output).write_text(json.dumps(report, indent=2) + chr(10))
    print(json.dumps({"journal_counts": report["journal_counts"], "sample_count": len(samples)}), flush=True)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--url", default="https://127.0.0.1:7777/api/v1")
    parser.add_argument("--service", default="satisfactory.service")
    parser.add_argument("--duration", type=float, default=60)
    parser.add_argument("--interval", type=float, default=5)
    parser.add_argument("--output")
    args = parser.parse_args()
    if args.duration < 0 or args.interval <= 0:
        parser.error("duration must be nonnegative and interval must be positive")
    observe(args)
