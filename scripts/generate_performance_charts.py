#!/usr/bin/env python3
"""Regenerate README figures from committed JSON (requires matplotlib)."""
import datetime as dt
import json
import io
from pathlib import Path
import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

ROOT = Path(__file__).resolve().parents[1]
ASSETS = ROOT / "docs/assets"
plt.rcParams.update({"font.family": "DejaVu Sans", "font.size": 11,
                     "svg.fonttype": "path", "svg.hashsalt": "satis-v0.1", "axes.spines.top": False,
                     "axes.spines.right": False})
report = json.loads((ROOT / "evidence/runtime/guarded-observation.json").read_text())
samples = report["samples"]
start = dt.datetime.fromisoformat(samples[0]["at"])
x = [(dt.datetime.fromisoformat(s["at"]) - start).total_seconds() for s in samples]

def finish(fig, name):
    fig.savefig(ASSETS / name.replace(".svg", ".png"), dpi=160)
    output = io.StringIO()
    fig.savefig(output, format="svg", metadata={"Date": None})
    # Matplotlib indents path data with trailing spaces; normalize for git checks.
    (ASSETS / name).write_text(chr(10).join(line.rstrip() for line in output.getvalue().splitlines()) + chr(10))
    plt.close(fig)

fig, ax = plt.subplots(figsize=(10, 4.8), layout="constrained")
ax.plot(x, [s["api"]["averageTickRate"] for s in samples], "o-", color="#146b91", label="API average TPS")
ax.axhline(30, color="#9a6700", linestyle="--", linewidth=1, label="30 TPS reference")
ax.set(title="Observed server tick rate · guarded alpha", xlabel="Seconds since observation started", ylabel="Averaged TPS", ylim=(0, 32))
ax.grid(axis="y", alpha=.2)
ax.legend(loc="lower left")
fig.suptitle("120-second window · no connected players · not an A/B speedup test", fontsize=10)
finish(fig, "gamethread_cpu_reduction.svg")

fig, ax = plt.subplots(figsize=(10, 4.8), layout="constrained")
cpu = [(t, s) for t, s in zip(x, samples) if "game_thread_cpu_pct" in s]
ax.plot([t for t, _ in cpu], [s["process_cpu_pct"] for _, s in cpu], "o-", color="#146b91", label="Whole server process")
ax.plot([t for t, _ in cpu], [s["game_thread_cpu_pct"] for _, s in cpu], "o-", color="#a84910", label="Main thread")
ax.set(title="Observed CPU use over matching intervals", xlabel="Seconds since observation started", ylabel="CPU use (% of one logical CPU)", ylim=(0, 130))
ax.legend(loc="center right")
ax.grid(axis="y", alpha=.2)
fig.suptitle("Ryzen 7 5825U · 8 cores / 16 hardware threads · idle-world observation", fontsize=10)
finish(fig, "multithread_cpu_distribution.svg")

sweep = json.loads((ROOT / "evidence/runtime/scaling-sweep.json").read_text())["sweep"]
fig, ax = plt.subplots(figsize=(10, 5), layout="constrained")
positions = list(range(len(sweep)))
for offset, key, color, label in [(-.19,"sync_mean_ms","#a84910","Serial integer workload"),(.19,"async_mean_ms","#146b91","Threaded integer workload")]:
    bars = ax.bar([p+offset for p in positions], [s[key] for s in sweep], width=.36, color=color, label=label)
    ax.bar_label(bars, fmt="%.2f", padding=4, fontsize=10)
ax.set_xticks(positions, [str(s["tasks"]) for s in sweep])
ax.set(title="Synthetic compute scaling · lower duration is better", xlabel="Independent compute tasks", ylabel="Mean compute batch duration (ms)", ylim=(0, 61))
ax.legend(loc="upper left")
ax.grid(axis="y", alpha=.15)
ax.set_axisbelow(True)
fig.suptitle("Intel i5-4570 · standalone std::thread benchmark · not live Satisfactory TPS", fontsize=10)
finish(fig, "scaling_throughput_tps.svg")
