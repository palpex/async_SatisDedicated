"""Bridge-owned scheduling/lifetime regression tests, independent of saved probes."""
import pathlib
import subprocess

ROOT = pathlib.Path(__file__).resolve().parents[1]


def test_tick_control_flow(tmp_path):
    executable = tmp_path / "tick-contract"
    subprocess.run(["c++", "-std=c++17", "-O1", "-g", "-Wall", "-Wextra", "-Werror",
                    "-fno-exceptions", "-fno-rtti", "-fsanitize=address,undefined",
                    "-I", str(ROOT / "include"), str(ROOT / "tests/tick_contract.cpp"),
                    "-ldl", "-pthread", "-o", str(executable)], check=True)
    subprocess.run([str(executable)], check=True, timeout=10)
