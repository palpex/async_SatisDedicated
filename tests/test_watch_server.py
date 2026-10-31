"""Recovery policy must preserve healthy servers and allow lengthy world loads."""
import importlib.util
from pathlib import Path
import sys

SCRIPTS = Path(__file__).resolve().parents[1] / "scripts"
sys.path.insert(0, str(SCRIPTS))
spec = importlib.util.spec_from_file_location("watch_server", SCRIPTS / "watch_server.py")
watch = importlib.util.module_from_spec(spec)
spec.loader.exec_module(watch)


def test_healthy_server_is_never_restarted():
    assert watch.restart_reason({"ActiveState": "active"}, True, 900, 900, 900, True) is None


def test_existing_auto_restart_is_respected():
    assert watch.restart_reason({"ActiveState": "activating", "SubState": "auto-restart"}, False, 900, 900, 900, True) is None


def test_failed_service_is_recovered_with_rate_limit():
    assert watch.restart_reason({"ActiveState": "failed"}, False, 0, 0, 100, False) == "service_not_running"
    assert watch.restart_reason({"ActiveState": "failed"}, False, 0, 0, 20, False) is None


def test_startup_and_transient_api_failures_have_grace_periods():
    active = {"ActiveState": "active"}
    assert watch.restart_reason(active, False, 299, 299, 900, False) is None
    assert watch.restart_reason(active, False, 301, 301, 900, False) == "api_unresponsive_after_grace"
    assert watch.restart_reason(active, False, 900, 119, 900, True) is None
    assert watch.restart_reason(active, False, 900, 121, 900, True) == "api_unresponsive_after_grace"
