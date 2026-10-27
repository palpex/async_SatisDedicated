import subprocess
import re

so = "server/FactoryGame/Binaries/Linux/libFactoryServer-FactoryGame-Linux-Shipping.so"

# Let's inspect references to AFGBuildableSubsystem, AFGPipeSubsystem, AFGCircuitSubsystem
cmd = ["nm", "-D", "-C", so]
proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, text=True)

buildable_calls = []
pipe_calls = []
circuit_calls = []

for line in proc.stdout:
    if "AFGBuildable::" in line:
        if "Pipe" in line or "Fluid" in line:
            pipe_calls.append(line.strip())
        if "Circuit" in line or "Power" in line:
            circuit_calls.append(line.strip())

proc.wait()

print("Buildable <-> Pipe / Fluid interactions:")
for l in pipe_calls[:15]:
    print("  ", l)
print("\nBuildable <-> Circuit / Power interactions:")
for l in circuit_calls[:15]:
    print("  ", l)
