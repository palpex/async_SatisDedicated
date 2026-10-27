#!/usr/bin/env python3
import json
import subprocess

tasks = [16, 32, 64, 128]
results = []

for t in tasks:
    out = subprocess.check_output(["./build/satis_benchmark", str(t), "150000", "50"]).decode()
    data = json.loads(out)
    results.append({
        "tasks": t,
        "sync_mean_ms": data["synchronous_baseline"]["mean_frame_time_ms"],
        "sync_tps": data["synchronous_baseline"]["effective_tps"],
        "async_mean_ms": data["asynchronous_bridge"]["mean_frame_time_ms"],
        "async_tps": data["asynchronous_bridge"]["effective_tps"],
        "speedup": data["summary"]["speedup_factor"],
        "stall_reduction_pct": data["gamethread_impact"]["gamethread_stall_reduction_pct"],
        "gamethread_stall_mean_ms": data["gamethread_impact"]["gamethread_stall_mean_ms"]
    })

with open("evidence/runtime/scaling-sweep.json", "w") as f:
    json.dump({"hardware": "Intel Core i5-4570 @ 3.20GHz (4 Cores / 3 Workers)", "sweep": results}, f, indent=2)

for r in results:
    print(f"Tasks {r['tasks']:3d}: Sync {r['sync_mean_ms']:6.2f} ms ({r['sync_tps']:5.1f} TPS) -> Async {r['async_mean_ms']:6.2f} ms ({r['async_tps']:5.1f} TPS) | Speedup: {r['speedup']:4.2f}x | Stall Reduction: {r['stall_reduction_pct']:5.2f}%")

