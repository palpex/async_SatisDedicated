import hashlib
import json
import pathlib
import pytest

ROOT = pathlib.Path(__file__).resolve().parents[1]

def test_task_batch_probe():
    probe_path = ROOT / "evidence/runtime/task-batch-probe.json"
    assert probe_path.exists()
    data = json.loads(probe_path.read_text())
    assert data["operation"] == "instrument_with_frida"
    res = data["normalized_result"]
    assert res["cleanup_error"] is None
    assert not res["messages_truncated"]
    messages = [m["payload"] for m in res["messages"] if m.get("type") == "send"]
    probe_msg = [m for m in messages if m.get("kind") == "bridge-probe"][-1]
    assert int(probe_msg["submitted"]) == 32
    assert int(probe_msg["completed"]) == 32
    assert int(probe_msg["destroyed"]) == 32
    assert int(probe_msg["wrongThread"]) == 0
    assert int(probe_msg["failures"]) == 0
    assert int(probe_msg["maxConcurrency"]) > 1
    assert int(probe_msg["continuationCalls"]) == 1
    assert probe_msg["checksum"] == probe_msg["expectedChecksum"]

def test_completion_probe():
    probe_path = ROOT / "evidence/runtime/completion-probe.json"
    assert probe_path.exists()
    data = json.loads(probe_path.read_text())
    assert data["operation"] == "instrument_with_frida"
    res = data["normalized_result"]
    assert res["cleanup_error"] is None
    assert not res["messages_truncated"]
    messages = [m["payload"] for m in res["messages"] if m.get("type") == "send"]
    probe_msg = [m for m in messages if m.get("kind") == "completion-probe"][-1]
    assert int(probe_msg["submitted"]) == 32
    assert int(probe_msg["completed"]) == 32
    assert int(probe_msg["destroyed"]) == 32
    assert int(probe_msg["earlyWatcher"]) == 0
    assert int(probe_msg["wrongThread"]) == 0
    assert int(probe_msg["failures"]) == 0
    assert int(probe_msg["watcherCalls"]) == 1
    assert int(probe_msg["watcherDisposals"]) == 1
    assert int(probe_msg["signalCalls"]) == 1
    assert int(probe_msg["maxConcurrency"]) > 1
    assert probe_msg["checksum"] == probe_msg["expectedChecksum"]

def test_completion_build_manifest():
    manifest_path = ROOT / "evidence/runtime/completion-build.json"
    assert manifest_path.exists()
    manifest = json.loads(manifest_path.read_text())
    for rel_path, expected_hash in manifest["sha256"].items():
        full_path = ROOT / rel_path
        assert full_path.exists(), f"File {rel_path} does not exist"
        actual_hash = hashlib.sha256(full_path.read_bytes()).hexdigest()
        assert actual_hash == expected_hash, f"Hash mismatch for {rel_path}"

def test_factorygame_binary_digest():
    so_path = ROOT / "server/FactoryGame/Binaries/Linux/libFactoryServer-FactoryGame-Linux-Shipping.so"
    assert so_path.exists()
    expected_hash = "637c2c70e65f6386697a6a5f5746f64b187d2fcfeee56df1aeef3f172268c0c1"
    actual_hash = hashlib.sha256(so_path.read_bytes()).hexdigest()
    assert actual_hash == expected_hash

