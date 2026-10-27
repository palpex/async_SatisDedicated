#!/usr/bin/env python3
import json
import argparse
import platform
import subprocess
from pathlib import Path

parser = argparse.ArgumentParser(description='Synthetic std::thread compute sweep; does not measure live server TPS')
parser.add_argument('--output', default='build/scaling-sweep.json')
args = parser.parse_args()

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

output = Path(args.output)
output.parent.mkdir(parents=True, exist_ok=True)
with output.open("w") as f:
    json.dump({"hardware": platform.uname()._asdict(), "scope": "synthetic integer compute; not live server TPS", "sweep": results}, f, indent=2)

for r in results:
    print(f"Tasks {r['tasks']:3d}: Serial {r['sync_mean_ms']:6.2f} ms -> Threaded {r['async_mean_ms']:6.2f} ms | Compute speedup: {r['speedup']:4.2f}x")

