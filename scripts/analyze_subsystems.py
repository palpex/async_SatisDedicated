import subprocess
import re

so_path = "server/FactoryGame/Binaries/Linux/libFactoryServer-FactoryGame-Linux-Shipping.so"

cmd = ["nm", "-D", "-C", so_path]
proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)

patterns = [
    r"AFGBuildableSubsystem::Tick",
    r"AFGPipeSubsystem::Tick",
    r"AFGCircuitSubsystem::Tick",
    r"AFGConveyorChainSubsystem::Tick",
    r"AFGConveyorItemSubsystem::Tick",
    r"AFGRailroadSubsystem::Tick",
    r"AFGVehicleSubsystem::Tick",
    r"ParallelFor",
    r"CVar"
]

results = {p: [] for p in patterns}

for line in proc.stdout:
    for p in patterns:
        if re.search(p, line):
            results[p].append(line.strip())

proc.wait()

for p, lines in results.items():
    print(f"=== Pattern: {p} (count: {len(lines)}) ===")
    for l in lines[:15]:
        print("  ", l)
    if len(lines) > 15:
        print(f"   ... and {len(lines)-15} more")
