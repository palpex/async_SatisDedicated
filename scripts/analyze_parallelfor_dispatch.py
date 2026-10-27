import subprocess

so = "server/FactoryGame/Binaries/Linux/libFactoryServer-FactoryGame-Linux-Shipping.so"

# Disassemble from 187fcab to 1880200
cmd = ["objdump", "-d", "-C", "--start-address=0x187fcab", "--stop-address=0x1880220", so]
res = subprocess.run(cmd, capture_output=True, text=True)

with open("evidence/tickfactoryactors_phase1.txt", "w") as f:
    f.write(res.stdout)

print(f"Written {len(res.stdout.splitlines())} lines to evidence/tickfactoryactors_phase1.txt")
