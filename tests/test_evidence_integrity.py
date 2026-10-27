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

def test_build_manifests_bound_to_probes():
    manifests = [
        ("evidence/runtime/task-batch-build.json", "evidence/runtime/task-batch-probe.json"),
        ("evidence/runtime/completion-build.json", "evidence/runtime/completion-probe.json")
    ]
    for m_rel, p_rel in manifests:
        m_path = ROOT / m_rel
        p_path = ROOT / p_rel
        assert m_path.exists()
        assert p_path.exists()
        manifest = json.loads(m_path.read_text())
        probe = json.loads(p_path.read_text())
        assert manifest["runtime_evidence_id"] == probe["evidence_id"]
        assert manifest["server_pid"] == probe["normalized_result"]["pid"]
        assert manifest["core_gnu_build_id"] == "36a6397531db8df2"

def test_factorygame_binary_digest():
    so_path = ROOT / "server/FactoryGame/Binaries/Linux/libFactoryServer-FactoryGame-Linux-Shipping.so"
    if not so_path.exists():
        pytest.skip("Install the supported official server for binary digest verification")
    expected_hash = "637c2c70e65f6386697a6a5f5746f64b187d2fcfeee56df1aeef3f172268c0c1"
    actual_hash = hashlib.sha256(so_path.read_bytes()).hexdigest()
    assert actual_hash == expected_hash

def test_core_binary_digest():
    core_path = ROOT / "server/Engine/Binaries/Linux/libFactoryServer-Core-Linux-Shipping.so"
    if not core_path.exists():
        pytest.skip("Install the supported official server for binary digest verification")
    expected_hash = "954e4f4055f1c7775732a8be797ddccc44391453bb6d807103dd1edf37fb34c7"
    actual_hash = hashlib.sha256(core_path.read_bytes()).hexdigest()
    assert actual_hash == expected_hash


def test_matched_ab_benchmark_evidence():
    bench_path = ROOT / "evidence/runtime/matched-ab-benchmark-production.json"
    assert bench_path.exists(), "matched A/B benchmark evidence must exist"
    raw_text = bench_path.read_text()
    assert "schwanz" not in raw_text.lower(), "private save identifier must not be present"
    data = json.loads(raw_text)
    assert data["async_enabled"]["bridge_enabled"] is True
    assert data["vanilla_synchronous"]["bridge_enabled"] is False
    assert data["async_restored"]["bridge_enabled"] is True
    assert data["async_enabled"]["ticks_in_window"] > 500
    assert data["vanilla_synchronous"]["ticks_in_window"] > 500
    assert data["async_restored"]["ticks_in_window"] > 500
    assert data["analysis"]["game_thread_cpu_reduction_pct"] > 10.0
    assert data["analysis"]["worker_cpu_increase_pct"] > 0.0
