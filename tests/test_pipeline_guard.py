"""Standalone pipeline dismantle guard regression coverage using synthetic ABI fixtures."""
import pathlib
import subprocess

ROOT = pathlib.Path(__file__).resolve().parents[1]


def test_pipeline_dismantle_guard_contract(tmp_path):
    executable = tmp_path / "pipeline-guard-contract"
    subprocess.run(
        ["c++", "-std=c++17", "-O1", "-g", "-Wall", "-Wextra", "-Werror",
         "-fno-exceptions", "-fno-rtti", "-fsanitize=address,undefined",
         "-I", str(ROOT / "include"),
         str(ROOT / "tests/pipeline_guard_contract.cpp"),
         "-ldl", "-pthread", "-o", str(executable)],
        check=True, timeout=60,
    )
    subprocess.run([str(executable)], check=True, timeout=10)
