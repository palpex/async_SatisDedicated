import subprocess

so = "server/FactoryGame/Binaries/Linux/libFactoryServer-FactoryGame-Linux-Shipping.so"
cmd = ["objdump", "-d", "-C", "--start-address=0x187fcb0", "--stop-address=0x1880000", so]
res = subprocess.run(cmd, capture_output=True, text=True)

with open("evidence/cvar_parallel_branch_logic.txt", "w") as f:
    f.write(res.stdout)

print(f"Dumped {len(res.stdout.splitlines())} lines of CVar logic")
