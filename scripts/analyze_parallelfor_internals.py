import subprocess
import re

so = "server/FactoryGame/Binaries/Linux/libFactoryServer-FactoryGame-Linux-Shipping.so"
cmd = ["objdump", "-d", "-C", "--start-address=0x184eea0", "--stop-address=0x184f4a0", so]
res = subprocess.run(cmd, capture_output=True, text=True)

with open("evidence/parallelfor_entry_disasm.txt", "w") as f:
    f.write(res.stdout)

print(f"Disassembled ParallelForInternal entry ({len(res.stdout.splitlines())} lines)")
