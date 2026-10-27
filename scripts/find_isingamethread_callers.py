import subprocess
import re

so = "server/FactoryGame/Binaries/Linux/libFactoryServer-FactoryGame-Linux-Shipping.so"
cmd = ["objdump", "-d", "-C", so]
proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, text=True)

current_func = None
callers = {}

for line in proc.stdout:
    m = re.match(r"^[0-9a-fA-F]+ <(.*)>:", line)
    if m:
        current_func = m.group(1)
        continue
    if "IsInGameThread" in line:
        callers[current_func] = callers.get(current_func, 0) + 1

proc.wait()

print(f"Total functions calling IsInGameThread: {len(callers)}")
for f, count in sorted(callers.items(), key=lambda x: x[1], reverse=True)[:30]:
    print(f"  {count} calls: {f}")
