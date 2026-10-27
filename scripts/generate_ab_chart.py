#!/usr/bin/env python3
"""Generate matched A/B benchmark chart from evidence JSON without external dependencies."""
import json
from pathlib import Path
import subprocess

ROOT = Path(__file__).resolve().parents[1]
DATA_PATH = ROOT / "evidence/runtime/matched-ab-benchmark-production.json"
ASSETS_DIR = ROOT / "docs/assets"

data = json.loads(DATA_PATH.read_text())

async_on = data["async_enabled"]
sync_vanilla = data["vanilla_synchronous"]
async_restored = data["async_restored"]

modes = [
    ("Async Enabled (Run 1)", async_on),
    ("Vanilla Synchronous", sync_vanilla),
    ("Async Restored (Run 2)", async_restored)
]

width = 800
height = 420
margin_top = 70
margin_bottom = 70
margin_left = 80
margin_right = 40
chart_w = width - margin_left - margin_right
chart_h = height - margin_top - margin_bottom

max_y = 120.0

def y_coord(val):
    return margin_top + chart_h - (val / max_y) * chart_h

svg_lines = [
    f'<svg xmlns="http://www.w3.org/2000/svg" viewBox="0 0 {width} {height}" width="{width}" height="{height}" style="background:#ffffff; font-family:sans-serif;">',
    '  <!-- Title and Subtitle -->',
    '  <text x="80" y="32" font-size="18" font-weight="bold" fill="#1e293b">Matched A/B Benchmark: Satisfactory Dedicated Server</text>',
    '  <text x="80" y="52" font-size="12" fill="#64748b">AMD Ryzen 7 5825U · 708-hour production save (unpaused) · 30s per window · 30 TPS target</text>',
    '  <!-- Grid lines -->',
]

for y_val in [0, 20, 40, 60, 80, 100, 120]:
    y_pos = y_coord(y_val)
    svg_lines.append(f'  <line x1="{margin_left}" y1="{y_pos:.1f}" x2="{width - margin_right}" y2="{y_pos:.1f}" stroke="#e2e8f0" stroke-width="1" />')
    svg_lines.append(f'  <text x="{margin_left - 10}" y="{y_pos + 4:.1f}" font-size="11" fill="#64748b" text-anchor="end">{y_val}%</text>')

group_width = chart_w / len(modes)
bar_width = 45
gap = 12

colors = {
    "gt": "#dc2626",      # Red for Game Thread
    "worker": "#2563eb",  # Blue for Workers
    "total": "#059669"    # Green for Total Process
}

for i, (name, metrics) in enumerate(modes):
    group_center = margin_left + (i + 0.5) * group_width
    
    gt_val = metrics["game_thread_cpu_pct"]["mean"]
    worker_val = metrics["worker_cpu_pct"]["mean"]
    total_val = metrics["total_process_cpu_pct"]["mean"]
    
    bars = [
        ("Game Thread", gt_val, colors["gt"], group_center - bar_width - gap/2),
        ("Workers", worker_val, colors["worker"], group_center),
        ("Total", total_val, colors["total"], group_center + bar_width + gap/2)
    ]
    
    for bar_label, val, col, x_pos in bars:
        b_x = x_pos - bar_width / 2
        b_y = y_coord(val)
        b_h = y_coord(0) - b_y
        svg_lines.append(f'  <rect x="{b_x:.1f}" y="{b_y:.1f}" width="{bar_width}" height="{b_h:.1f}" rx="3" fill="{col}" opacity="0.9" />')
        svg_lines.append(f'  <text x="{x_pos:.1f}" y="{b_y - 6:.1f}" font-size="11" font-weight="bold" fill="{col}" text-anchor="middle">{val:.1f}%</text>')
    
    svg_lines.append(f'  <text x="{group_center:.1f}" y="{height - margin_bottom + 22}" font-size="12" font-weight="bold" fill="#334155" text-anchor="middle">{name}</text>')
    tps_text = f"TPS: {metrics['tps']['mean']:.2f}"
    svg_lines.append(f'  <text x="{group_center:.1f}" y="{height - margin_bottom + 38}" font-size="11" fill="#64748b" text-anchor="middle">{tps_text}</text>')

legend_y = height - 15
svg_lines.extend([
    '  <!-- Legend -->',
    f'  <rect x="120" y="{legend_y - 10}" width="14" height="14" rx="2" fill="{colors["gt"]}" />',
    f'  <text x="140" y="{legend_y + 1}" font-size="11" fill="#334155">Game Thread CPU (Main Thread)</text>',
    f'  <rect x="360" y="{legend_y - 10}" width="14" height="14" rx="2" fill="{colors["worker"]}" />',
    f'  <text x="380" y="{legend_y + 1}" font-size="11" fill="#334155">Worker Threads CPU (TaskGraph)</text>',
    f'  <rect x="600" y="{legend_y - 10}" width="14" height="14" rx="2" fill="{colors["total"]}" />',
    f'  <text x="620" y="{legend_y + 1}" font-size="11" fill="#334155">Total Server Process</text>',
    '</svg>'
])

svg_content = chr(10).join(svg_lines) + chr(10)
svg_path = ASSETS_DIR / "matched_ab_comparison.svg"
png_path = ASSETS_DIR / "matched_ab_comparison.png"

svg_path.write_text(svg_content)
print(f"Generated {svg_path}")

try:
    subprocess.run(["convert", "-density", "150", str(svg_path), str(png_path)], check=True)
    print(f"Generated {png_path}")
except Exception as e:
    print(f"Convert error: {e}")
