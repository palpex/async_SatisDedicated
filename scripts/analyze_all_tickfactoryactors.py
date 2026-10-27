import subprocess
import re

so = "server/FactoryGame/Binaries/Linux/libFactoryServer-FactoryGame-Linux-Shipping.so"
cmd = ["objdump", "-d", "-C", "--start-address=0x187fc40", "--stop-address=0x1881000", so]
res = subprocess.run(cmd, capture_output=True, text=True)

with open("evidence/full_tickfactoryactors.txt", "w") as f:
    f.write(res.stdout)

print(f"Total lines: {len(res.stdout.splitlines())}")

# Search for ParallelFor calls and surrounding instructions
lines = res.stdout.splitlines()
for idx, l in enumerate(lines):
    if "ParallelForInternal" in l:
        print(f"--- ParallelFor call at line {idx}: ---")
        for context in lines[max(0, idx-15):min(len(lines), idx+10)]:
            print("  ", context)
