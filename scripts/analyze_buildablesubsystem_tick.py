import subprocess
import re

so = "server/FactoryGame/Binaries/Linux/libFactoryServer-FactoryGame-Linux-Shipping.so"
cmd = ["objdump", "-d", "-C", "--start-address=0x1881940", "--stop-address=0x1882d40", so]
res = subprocess.run(cmd, capture_output=True, text=True)

calls = []
for line in res.stdout.splitlines():
    if "call" in line:
        calls.append(line.strip())

print(f"Total calls in AFGBuildableSubsystem::Tick: {len(calls)}")
for c in calls[:50]:
    print("  ", c)
if len(calls) > 50:
    print(f"  ... and {len(calls)-50} more calls")
