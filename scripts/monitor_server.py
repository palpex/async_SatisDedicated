#!/usr/bin/env python3
"""Monitor Satisfactory Dedicated Server health, state, and tick performance."""
import argparse
import json
import ssl
import time
import urllib.request
import urllib.error

def query_api(function, data=None, port=7777, timeout=5.0, token=None):
    if data is None:
        data = {}
    headers = {"Content-Type": "application/json"}
    if token:
        headers["Authorization"] = "Bearer " + token
    req = urllib.request.Request(
        f"https://127.0.0.1:{port}/api/v1",
        data=json.dumps({"function": function, "data": data}).encode(),
        headers=headers,
        method="POST"
    )
    try:
        with urllib.request.urlopen(req, context=ssl._create_unverified_context(), timeout=timeout) as resp:
            body = resp.read()
            result = json.loads(body) if body else {}
            if "errorCode" in result:
                return {"error": result["errorCode"]}
            return result
    except urllib.error.URLError as e:
        return {"error": str(e)}

def main():
    parser = argparse.ArgumentParser(description="Monitor Satisfactory Dedicated Server")
    parser.add_argument("--port", type=int, default=7777, help="Server HTTPS API port")
    parser.add_argument("--interval", type=float, default=2.0, help="Poll interval in seconds (0 = single shot)")
    args = parser.parse_args()
    if not 1 <= args.port <= 65535 or args.interval < 0:
        parser.error("port must be valid and interval nonnegative")

    print(f">> Polling Satisfactory Dedicated Server API on https://127.0.0.1:{args.port}/api/v1...")

    while True:
        health = query_api("HealthCheck", {"clientCustomData": ""}, port=args.port)
        login = query_api("PasswordlessLogin", {"minimumPrivilegeLevel": "Client"}, port=args.port)
        token = login.get("data", {}).get("authenticationToken")
        state = query_api("QueryServerState", port=args.port, token=token) if token else {"error": login.get("error", "client_login_failed")}

        if "error" in health and "error" in state:
            print(f"[{time.strftime('%X')}] Server offline or unreachable: {health.get('error')}")
        else:
            h_status = health.get("data", {}).get("health", "unknown")
            s_data = state.get("data", {}).get("serverGameState", {})
            num_players = s_data.get("numConnectedPlayers", 0)
            is_paused = s_data.get("isGamePaused", False)
            tick_rate = s_data.get("averageTickRate", 0.0)

            if "error" in state:
                print(f"[{time.strftime('%X')}] State unavailable: {state['error']}")
            else:
                print(f"[{time.strftime('%X')}] Status: {str(h_status).upper()} | Players: {num_players} | Paused: {is_paused} | Avg TickRate: {tick_rate:.1f} TPS")

        if args.interval <= 0:
            break
        time.sleep(args.interval)

if __name__ == "__main__":
    main()

