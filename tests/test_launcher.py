"""Real launcher argument/environment tests using a fake server/account switch."""
import json
import os
from pathlib import Path
import subprocess
import pytest

ROOT = Path(__file__).resolve().parents[1]

@pytest.fixture
def launch(tmp_path):
    server = tmp_path / "server with spaces" / "FactoryServer.sh"
    server.parent.mkdir()
    server.write_text('#!/usr/bin/env python3\nimport json,os,sys\nprint(json.dumps({"args":sys.argv[1:],"async":os.environ["SATIS_ASYNC_TICK"]}))\n')
    server.chmod(0o755)
    env = dict(os.environ, SATIS_SERVER_DIR=str(server.parent), SATIS_BRIDGE_LIB=str(ROOT / "build/libsatis_task_bridge.so"), SATIS_ASYNC_TICK="0")
    env.pop("LD_PRELOAD", None)
    env.pop("SATIS_USER", None)
    binaries = tmp_path / "bin"
    binaries.mkdir()
    identity = binaries / "id"
    identity.write_text('#!/bin/bash\nif [[ $1 == -u ]]; then if [[ $# == 1 || $2 == root ]]; then echo 0; else echo 1000; fi; else exit 0; fi\n')
    identity.chmod(0o755)
    runuser = binaries / "runuser"
    runuser.write_text('#!/bin/bash\nshift 3\nexec "$@"\n')
    runuser.chmod(0o755)
    env["PATH"] = str(binaries) + os.pathsep + env["PATH"]
    def run(*args, **overrides):
        return subprocess.run(["bash", str(ROOT / "run_async_server.sh"), *args], env=dict(env, **overrides), text=True, capture_output=True, timeout=10)
    return run

def test_arguments_and_disabled_mode(launch):
    result = launch("-log", "value with spaces", "literal;$(not-executed)", SATIS_USER="service")
    assert result.returncode == 0, result.stderr
    data = json.loads(result.stdout.splitlines()[-1])
    assert data == {"args": ["-log", "value with spaces", "literal;$(not-executed)"], "async": "0"}

def test_root_and_invalid_input(launch):
    for overrides, message in [({}, "do not run"), ({"SATIS_USER": "root"}, "unprivileged"), ({"SATIS_ASYNC_TICK": "bad"}, "must be"), ({"SATIS_BRIDGE_LIB": "/missing/library.so"}, "does not exist")]:
        result = launch(**overrides)
        assert result.returncode != 0
        assert message in result.stdout + result.stderr
