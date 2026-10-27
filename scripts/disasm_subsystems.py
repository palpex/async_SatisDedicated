import subprocess

so = "server/FactoryGame/Binaries/Linux/libFactoryServer-FactoryGame-Linux-Shipping.so"

funcs = [
    ("AFGBuildableSubsystem::Tick", 0x1881940, 0x1881cb0),
    ("AFGPipeSubsystem::Tick", 0x1b24ec0, 0x1b25200),
    ("AFGCircuitSubsystem::Tick", 0x192a120, 0x192a500),
    ("AFGConveyorChainSubsystem::Tick", 0x1995ce0, 0x1996000),
]

for name, start, stop in funcs:
    print(f"================ {name} ({hex(start)} - {hex(stop)}) ================")
    cmd = ["objdump", "-d", "-C", f"--start-address={hex(start)}", f"--stop-address={hex(stop)}", so]
    res = subprocess.run(cmd, capture_output=True, text=True)
    lines = res.stdout.splitlines()
    for l in lines[:40]:
        print(l)
    if len(lines) > 40:
        print(f"... ({len(lines)-40} more lines)")
