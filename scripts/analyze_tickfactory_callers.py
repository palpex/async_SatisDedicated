import subprocess
import re

so = "server/FactoryGame/Binaries/Linux/libFactoryServer-FactoryGame-Linux-Shipping.so"
cmd = ["objdump", "-d", "-C", so]
proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, text=True)

current_func = None
callers = []

# Address of AFGBuildableSubsystem::TickFactory is 0x187faf0
target_addr = "187faf0"

for line in proc.stdout:
    m = re.match(r"^[0-9a-fA-F]+ <(.*)>:", line)
    if m:
        current_func = m.group(1)
        continue
    if "call" in line and target_addr in line:
        callers.append((current_func, line.strip()))

proc.wait()

print(f"Callers of AFGBuildableSubsystem::TickFactory: {len(callers)}")
for func, l in callers:
    print(f"  In: {func}\n    {l}")
