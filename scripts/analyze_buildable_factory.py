import subprocess
import re

so = "server/FactoryGame/Binaries/Linux/libFactoryServer-FactoryGame-Linux-Shipping.so"

cmd = ["nm", "-D", "-C", so]
proc = subprocess.Popen(cmd, stdout=subprocess.PIPE, text=True)

factory_methods = []
for line in proc.stdout:
    if "AFGBuildableFactory::" in line:
        factory_methods.append(line.strip())
proc.wait()

print(f"AFGBuildableFactory methods: {len(factory_methods)}")
for l in factory_methods[:30]:
    print("  ", l)
