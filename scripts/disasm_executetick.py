import subprocess

so = "server/FactoryGame/Binaries/Linux/libFactoryServer-FactoryGame-Linux-Shipping.so"
cmd = ["objdump", "-d", "-C", "--start-address=0x180cba0", "--stop-address=0x180cd00", so]
res = subprocess.run(cmd, capture_output=True, text=True)
print(res.stdout)
