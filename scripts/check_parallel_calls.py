import subprocess
import re

so = "server/FactoryGame/Binaries/Linux/libFactoryServer-FactoryGame-Linux-Shipping.so"
cmd = ["objdump", "-d", "-C", so]
proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, text=True)

current_func = None
parallel_usages = {}

for line in proc.stdout:
    m = re.match(r"^[0-9a-fA-F]+ <(.*)>:", line)
    if m:
        current_func = m.group(1)
        continue
    if "ParallelForInternal" in line:
        parallel_usages[current_func] = parallel_usages.get(current_func, 0) + 1

proc.wait()

print(f"Total functions calling ParallelForInternal: {len(parallel_usages)}")
for f, count in sorted(parallel_usages.items(), key=lambda x: x[1], reverse=True)[:30]:
    print(f"{count:2d} calls: {f}")
