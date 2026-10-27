import subprocess

so = "server/FactoryGame/Binaries/Linux/libFactoryServer-FactoryGame-Linux-Shipping.so"
cmd = ["objdump", "-d", "-C", "--start-address=0x184f350", "--stop-address=0x184f470", so]
res = subprocess.run(cmd, capture_output=True, text=True)
print(res.stdout)
