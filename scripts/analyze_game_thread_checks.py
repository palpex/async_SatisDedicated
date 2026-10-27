import subprocess
import re

so = "server/FactoryGame/Binaries/Linux/libFactoryServer-FactoryGame-Linux-Shipping.so"
cmd = ["nm", "-D", "-C", so]
proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, text=True)

thread_funcs = []
for line in proc.stdout:
    if any(k in line for k in ["IsInGameThread", "IsInParallelGameThread", "check(IsInGameThread", "TaskGraph", "ExecuteTick"]):
        thread_funcs.append(line.strip())
proc.wait()

print(f"Thread check functions: {len(thread_funcs)}")
for f in thread_funcs:
    print("  ", f)
