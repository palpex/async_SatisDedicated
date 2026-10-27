#!/usr/bin/env python3
"""Monitor Satisfactory Dedicated Server health, state, and tick performance."""
import argparse
import json
import ssl
import sys
import time
import urllib.request
import urllib.error

def query_api(function, data=None, port=17877, timeout=5.0):
    if data is None:
        data = {}
    req = urllib.request.Request(
        f"https://127.0.0.1:{port}/api/v1",
        data=json.dumps({"function": function, "data": data}).encode(),
        headers={"Content-Type": "application/json"},
        method="POST"
    )
    try:
        with urllib.request.urlopen(req, context=ssl._create_unverified_context(), timeout=timeout) as resp:
            body = resp.read()
            return json.loads(body) if body else {}
    except urllib.error.URLError as e:
        return {"error": str(e)}

def main():
    parser = argparse.ArgumentParser(description="Monitor Satisfactory Dedicated Server")
    parser.add_argument("--port", type=int, default=17877, help="Server HTTPS API port")
    parser.add_argument("--interval", type=float, default=2.0, help="Poll interval in seconds (0 = single shot)")
    args = parser.parse_args()

    print(f">> Polling Satisfactory Dedicated Server API on https://127.0.0.1:{args.port}/api/v1...")

    while True:
        health = query_api("HealthCheck", port=args.port)
        state = query_api("GetServerState", port=args.port)

        if "error" in health and "error" in state:
            print(f"[{time.strftime('%X')}] Server offline or unreachable: {health.get('error')}")
        else:
            h_status = health.get("data", {}).get("health", "unknown")
            s_data = state.get("data", {}).get("serverGameState", {})
            active_session = s_data.get("activeSessionName", "None")
            num_players = s_data.get("numConnectedPlayers", 0)
            is_paused = s_data.get("isGamePaused", False)
            tick_rate = s_data.get("averageTickRate", 0.0)

            print(f"[{time.strftime('%X')}] Status: {h_status.upper()} | Session: {active_session} | Players: {num_players} | Paused: {is_paused} | Avg TickRate: {tick_rate:.1f} TPS")

        if args.interval <= 0:
            break
        time.sleep(args.interval)

if __name__ == "__main__":
    main()

