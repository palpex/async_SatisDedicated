#!/usr/bin/env python3
"""Run a controlled matched A/B benchmark on the Satisfactory Dedicated Server.

Toggles libsatis_task_bridge.so between asynchronous and synchronous modes
across matched observation windows to measure Game Thread CPU load, worker compute,
and server tick rate stability on the same world state without process restart.
"""
import argparse
import datetime as dt
import json
import math
import os
from pathlib import Path
import ssl
import struct
import subprocess
import time
import urllib.request

def api_call(url, function, data=None, token=None):
    headers = {"Content-Type": "application/json"}
    if token:
        headers["Authorization"] = "Bearer " + token
    req = urllib.request.Request(url, json.dumps({"function": function, "data": data or {}}).encode(), headers)
    with urllib.request.urlopen(req, context=ssl._create_unverified_context(), timeout=4) as response:
        result = json.load(response)
    if "errorCode" in result:
        raise RuntimeError(result["errorCode"])
    return result.get("data", {})

def get_server_tps(url, token):
    state = api_call(url, "QueryServerState", token=token)["serverGameState"]
    return state.get("averageTickRate")

def server_pid():
    for entry in Path("/proc").iterdir():
        if entry.name.isdigit():
            try:
                if (entry / "exe").resolve(strict=True).name == "FactoryServer-Linux-Shipping":
                    return int(entry.name)
            except (FileNotFoundError, PermissionError, ProcessLookupError):
                pass
    raise RuntimeError("server_process_not_found")

def bridge_info(pid):
    maps = Path(f"/proc/{pid}/maps").read_text().splitlines()
    mapping = next((line.split(maxsplit=5) for line in maps if "libsatis_task_bridge.so" in line and line.split()[2] == "00000000"), None)
    if not mapping:
        return None, None
    base = int(mapping[0].split("-")[0], 16)
    return base, mapping[5]

def read_bridge_stats(pid, base):
    with open(f"/proc/{pid}/mem", "rb", buffering=0) as mem:
        mem.seek(base + 0x7270)
        completed = struct.unpack("<Q", mem.read(8))[0]
        fallbacks = struct.unpack("<Q", mem.read(8))[0]
        inflight = struct.unpack("<Q", mem.read(8))[0]
        dispatches = struct.unpack("<Q", mem.read(8))[0]
        intercepted = struct.unpack("<Q", mem.read(8))[0]
        enabled = struct.unpack("<B", mem.read(1))[0]
        return {
            "enabled": enabled,
            "intercepted": intercepted,
            "dispatches": dispatches,
            "completed": completed,
            "inflight": inflight,
            "fallbacks": fallbacks
        }

def set_bridge_enabled(pid, base, enabled_val):
    with open(f"/proc/{pid}/mem", "r+b", buffering=0) as mem:
        mem.seek(base + 0x7298)
        mem.write(struct.pack("<B", 1 if enabled_val else 0))

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

def sample_window(pid, base, url, token, duration_sec, interval_sec=1.0):
    start_time = time.monotonic()
    deadline = start_time + duration_sec
    b_start = read_bridge_stats(pid, base)
    
    gt_cpu_samples = []
    worker_cpu_samples = []
    total_cpu_samples = []
    tps_samples = []
    
    prev_ticks = thread_ticks(pid)
    prev_time = time.monotonic()
    
    time.sleep(interval_sec)
    
    while True:
        now = time.monotonic()
        curr_ticks = thread_ticks(pid)
        dt_val = now - prev_time
        factor = 100.0 / os.sysconf("SC_CLK_TCK") / max(0.001, dt_val)
        
        usages = {tid: max(0, tick - prev_ticks.get(tid, tick)) * factor for tid, tick in curr_ticks.items()}
        gt_cpu = usages.get(pid, 0.0)
        worker_cpu = sum(val for tid, val in usages.items() if tid != pid)
        proc_cpu = sum(usages.values())
        
        gt_cpu_samples.append(gt_cpu)
        worker_cpu_samples.append(worker_cpu)
        total_cpu_samples.append(proc_cpu)
        
        try:
            tps = get_server_tps(url, token)
            if tps is not None:
                tps_samples.append(tps)
        except Exception:
            pass
            
        prev_ticks = curr_ticks
        prev_time = now
        
        if now >= deadline:
            break
        time.sleep(min(interval_sec, max(0.1, deadline - now)))
        
    b_end = read_bridge_stats(pid, base)
    actual_duration = time.monotonic() - start_time
    
    def stats(arr):
        if not arr:
            return {"mean": 0.0, "std_dev": 0.0, "min": 0.0, "max": 0.0, "count": 0}
        m = sum(arr) / len(arr)
        var = sum((x - m) ** 2 for x in arr) / len(arr)
        return {
            "mean": round(m, 2),
            "std_dev": round(math.sqrt(var), 3),
            "min": round(min(arr), 2),
            "max": round(max(arr), 2),
            "count": len(arr)
        }

    return {
        "actual_duration_s": round(actual_duration, 2),
        "bridge_enabled": bool(b_start["enabled"]),
        "ticks_in_window": b_end["intercepted"] - b_start["intercepted"],
        "dispatches_in_window": b_end["dispatches"] - b_start["dispatches"],
        "fallbacks_in_window": b_end["fallbacks"] - b_start["fallbacks"],
        "game_thread_cpu_pct": stats(gt_cpu_samples),
        "worker_cpu_pct": stats(worker_cpu_samples),
        "total_process_cpu_pct": stats(total_cpu_samples),
        "tps": stats(tps_samples)
    }

def main():
    parser = argparse.ArgumentParser(description="Run matched A/B benchmark on Satisfactory server.")
    parser.add_argument("--url", default="https://127.0.0.1:7777/api/v1", help="Server API URL")
    parser.add_argument("--window", type=float, default=30.0, help="Window duration in seconds per phase")
    parser.add_argument("--interval", type=float, default=1.0, help="Sampling interval in seconds")
    parser.add_argument("--output", default="evidence/runtime/matched-ab-benchmark-production.json", help="Output JSON path")
    args = parser.parse_args()

    login = api_call(args.url, "PasswordlessLogin", {"minimumPrivilegeLevel": "Client"})
    token = login["authenticationToken"]
    pid = server_pid()
    base, lib_path = bridge_info(pid)
    if not base:
        raise RuntimeError("libsatis_task_bridge.so is not loaded in server process")
        
    print(f"Target PID: {pid}, Bridge Base: {hex(base)}")
    print(f"Starting matched A/B measurement ({args.window}s per phase)...")
    
    # 1. Async ON
    print("Phase 1: Async Bridge ENABLED...")
    set_bridge_enabled(pid, base, True)
    time.sleep(2)
    async_on = sample_window(pid, base, args.url, token, args.window, args.interval)
    
    # 2. Async OFF (Vanilla synchronous)
    print("Phase 2: Async Bridge DISABLED (Vanilla Synchronous)...")
    set_bridge_enabled(pid, base, False)
    time.sleep(2)
    async_off = sample_window(pid, base, args.url, token, args.window, args.interval)
    
    # 3. Async Restored
    print("Phase 3: Async Bridge RESTORED...")
    set_bridge_enabled(pid, base, True)
    time.sleep(2)
    async_restored = sample_window(pid, base, args.url, token, args.window, args.interval)
    
    report = {
        "timestamp": dt.datetime.now(dt.timezone.utc).isoformat(),
        "save_type": "production save (unpaused)",
        "host": "AMD Ryzen 7 5825U (8C/16T, Ubuntu 24.04)",
        "async_enabled": async_on,
        "vanilla_synchronous": async_off,
        "async_restored": async_restored,
        "analysis": {
            "game_thread_cpu_reduction_pct": round(
                (async_off["game_thread_cpu_pct"]["mean"] - async_on["game_thread_cpu_pct"]["mean"])
                / max(0.01, async_off["game_thread_cpu_pct"]["mean"]) * 100.0,
                2
            ),
            "worker_cpu_increase_pct": round(
                (async_on["worker_cpu_pct"]["mean"] - async_off["worker_cpu_pct"]["mean"])
                / max(0.01, async_off["worker_cpu_pct"]["mean"]) * 100.0,
                2
            )
        }
    }
    
    out_path = Path(args.output)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    out_path.write_text(json.dumps(report, indent=2) + chr(10))
    print(f"Benchmark written to {out_path}")
    print(json.dumps(report["analysis"], indent=2))

if __name__ == "__main__":
    main()
