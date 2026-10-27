import subprocess

so = "server/FactoryGame/Binaries/Linux/libFactoryServer-FactoryGame-Linux-Shipping.so"
cmd = ["objdump", "-d", "-C", "--start-address=0x187f690", "--stop-address=0x187f900", so]
res = subprocess.run(cmd, capture_output=True, text=True)
print(res.stdout)
