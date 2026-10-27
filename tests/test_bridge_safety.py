import ctypes
import pathlib
import pytest

ROOT = pathlib.Path(__file__).resolve().parents[1]
LIB_PATH = ROOT / "build/libsatis_task_bridge.so"

@pytest.fixture(scope="module")
def bridge():
    assert LIB_PATH.exists(), f"Bridge library not built: {LIB_PATH}"
    lib = ctypes.CDLL(str(LIB_PATH))
    lib.satis_task_bridge_init.restype = ctypes.c_int
    lib.satis_task_bridge_error.restype = ctypes.c_char_p
    lib.satis_task_enqueue.restype = ctypes.c_int
    lib.satis_task_outstanding.restype = ctypes.c_uint64
    lib.satis_task_quiescent.restype = ctypes.c_int
    lib.satis_completion_create.restype = ctypes.c_void_p
    lib.satis_completion_then.restype = ctypes.c_int
    lib.satis_completion_signal.restype = ctypes.c_int
    lib.satis_completion_release.restype = None
    lib.satis_completion_bind_to_task.restype = ctypes.c_int
    lib.satis_async_set_enabled.restype = None
    lib.satis_async_set_enabled.argtypes = [ctypes.c_int]
    lib.satis_async_is_enabled.restype = ctypes.c_int
    lib.satis_async_tick_count.restype = ctypes.c_uint64
    lib.satis_async_dispatch_count.restype = ctypes.c_uint64
    lib.satis_async_sync_fallback_count.restype = ctypes.c_uint64
    lib.satis_async_pool_hit_count.restype = ctypes.c_uint64
    return lib

def test_initialization_outside_server(bridge):
    ret = bridge.satis_task_bridge_init()
    err = bridge.satis_task_bridge_error()
    assert ret == -1
    assert err == b"The official Core module is not loaded"

def test_enqueue_safeguards(bridge):
    assert bridge.satis_task_enqueue(None, None, None, 0) == -1
    assert bridge.satis_task_enqueue(None, None, None, 1) == -1

def test_state_counters(bridge):
    assert bridge.satis_task_outstanding() == 0
    assert bridge.satis_task_quiescent() == 1

def test_completion_handles_safe_nulls(bridge):
    assert bridge.satis_completion_create() is None
    assert bridge.satis_completion_signal(None) == -1
    assert bridge.satis_completion_then(None, None, None, None, 0) == -1
    assert bridge.satis_completion_bind_to_task(None, None) == -1
    # release with nullptr must not crash
    bridge.satis_completion_release(None)

def test_async_interceptor_controls(bridge):
    bridge.satis_async_set_enabled(1)
    assert bridge.satis_async_is_enabled() == 1
    bridge.satis_async_set_enabled(0)
    assert bridge.satis_async_is_enabled() == 0
    assert bridge.satis_async_tick_count() == 0
    assert bridge.satis_async_dispatch_count() == 0
    assert bridge.satis_async_sync_fallback_count() == 0
    assert bridge.satis_async_pool_hit_count() == 0

