#!/usr/bin/env python3
"""Validate recorded historical ABI probes and current standalone checks.

This does not run fresh gameplay probes or establish current-server stability.
"""
import argparse
import ctypes
import hashlib
import json
import pathlib
import sys

root = pathlib.Path(__file__).resolve().parents[1]

def verify_standalone_safety():
    """Verify bridge library loads safely outside Core and fails gracefully."""
    lib_path = root / 'build/libsatis_task_bridge.so'
    assert lib_path.exists(), f"Bridge library missing: {lib_path}"
    lib = ctypes.CDLL(str(lib_path))
    lib.satis_task_bridge_init.restype = ctypes.c_int
    lib.satis_task_bridge_error.restype = ctypes.c_char_p
    lib.satis_task_enqueue.restype = ctypes.c_int
    lib.satis_task_outstanding.restype = ctypes.c_uint64
    lib.satis_task_quiescent.restype = ctypes.c_int
    lib.satis_completion_create.restype = ctypes.c_void_p
    lib.satis_completion_then.restype = ctypes.c_int
    lib.satis_completion_signal.restype = ctypes.c_int
    lib.satis_completion_bind_to_task.restype = ctypes.c_int
    lib.satis_async_set_enabled.restype = None
    lib.satis_async_set_enabled.argtypes = [ctypes.c_int]
    lib.satis_async_is_enabled.restype = ctypes.c_int
    lib.satis_async_tick_count.restype = ctypes.c_uint64
    lib.satis_async_dispatch_count.restype = ctypes.c_uint64

    ret = lib.satis_task_bridge_init()
    err = lib.satis_task_bridge_error()
    assert ret == -1, f"Expected init -1 outside Core, got {ret}"
    assert err == b"The official Core module is not loaded", f"Unexpected error: {err}"
    assert lib.satis_task_enqueue(None, None, None, 0) == -1
    assert lib.satis_task_outstanding() == 0
    assert lib.satis_task_quiescent() == 1
    assert lib.satis_completion_create() is None
    assert lib.satis_completion_signal(None) == -1
    assert lib.satis_completion_bind_to_task(None, None) == -1

    lib.satis_async_set_enabled(1)
    assert lib.satis_async_is_enabled() == 1
    lib.satis_async_set_enabled(0)
    assert lib.satis_async_is_enabled() == 0
    print("✓ Standalone safety checks: graceful rejection when outside Core.")

def verify_probe_file(probe_path: pathlib.Path):
    evidence = json.loads(probe_path.read_text())
    assert evidence['operation'] == 'instrument_with_frida'
    observation = evidence['normalized_result']
    assert observation['cleanup_error'] is None, observation['cleanup_error']
    assert not observation['messages_truncated'], "Messages truncated in Frida probe"
    assert not [m for m in observation['messages'] if m.get('type') == 'error']
    messages = [m['payload'] for m in observation['messages'] if m.get('type') == 'send']

    name = probe_path.name
    if 'completion' in name:
        init_msg = next(m for m in messages if m.get('kind') == 'completion-init')
        assert init_msg['result'] == 0, f"Init failed: {init_msg}"
        start_msg = next(m for m in messages if m.get('kind') == 'completion-start')
        assert start_msg['result'] == 0, f"Start failed: {start_msg}"
        result = [m for m in messages if m.get('kind') == 'completion-probe'][-1]
        for field in ('submitted', 'completed', 'destroyed'):
            assert int(result[field]) == 32, (field, result)
        for field in ('earlyWatcher', 'wrongThread', 'failures'):
            assert int(result[field]) == 0, (field, result)
        assert result['checksum'] == result['expectedChecksum'], "Checksum mismatch"
        assert int(result['maxConcurrency']) > 1, "Concurrency not observed"
        assert int(result['watcherCalls']) == 1, "Watcher calls != 1"
        assert int(result['watcherDisposals']) == 1, "Watcher disposals != 1"
        assert int(result['signalCalls']) == 1, "Signal calls != 1"

        manifest_path = root / 'evidence/runtime/completion-build.json'
        if manifest_path.exists():
            manifest = json.loads(manifest_path.read_text())
            assert manifest['runtime_evidence_id'] == evidence['evidence_id']
            assert manifest['server_pid'] == observation['pid']
        print(f"✓ Historical completion evidence: 32 jobs, {result['maxConcurrency']} concurrent; stored gating result valid.")

    elif 'task-batch' in name or 'task-bridge' in name:
        init_msg = next(m for m in messages if m.get('kind') == 'bridge-init')
        assert init_msg['result'] == 0, f"Init failed: {init_msg}"
        start_msg = next(m for m in messages if m.get('kind') == 'bridge-probe-start')
        assert start_msg['result'] == 0, f"Start failed: {start_msg}"
        result = [m for m in messages if m.get('kind') == 'bridge-probe'][-1]
        for field in ('submitted', 'completed', 'destroyed'):
            assert int(result[field]) == 32, (field, result)
        for field in ('outstanding', 'wrongThread', 'failures', 'continuationWrongThread'):
            assert int(result[field]) == 0, (field, result)
        assert result['checksum'] == result['expectedChecksum'], "Checksum mismatch"
        assert int(result['maxConcurrency']) > 1, "Concurrency not observed"
        assert int(result['continuationCalls']) == 1, "Continuation calls != 1"
        assert int(result['continuationDisposals']) == 1, "Continuation disposals != 1"

        manifest_path = root / 'evidence/runtime/task-batch-build.json'
        if manifest_path.exists():
            manifest = json.loads(manifest_path.read_text())
            assert manifest['runtime_evidence_id'] == evidence['evidence_id']
            assert manifest['server_pid'] == observation['pid']
        print(f"✓ Historical task-batch evidence: 32 jobs, {result['maxConcurrency']} concurrent; stored continuation/disposal result valid.")

def main():
    parser = argparse.ArgumentParser(description="Verify runtime evidence and bridge safety")
    parser.add_argument("probe", nargs="?", default=None, help="Optional specific probe JSON path to verify")
    args = parser.parse_args()

    verify_standalone_safety()

    if args.probe:
        p = pathlib.Path(args.probe)
        if not p.is_absolute():
            p = root / p
        verify_probe_file(p)
    else:
        probes = [
            root / 'evidence/runtime/completion-probe.json',
            root / 'evidence/runtime/task-batch-probe.json'
        ]
        for p in probes:
            if p.exists():
                verify_probe_file(p)

    print("\nAll evidence & safety checks PASSED.")

if __name__ == '__main__':
    main()

