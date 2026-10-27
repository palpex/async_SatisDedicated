#!/usr/bin/env python3
"""Call only the isolated loopback test server; TLS trust is local-test-only."""
import argparse
import json
import ssl
import urllib.request

parser = argparse.ArgumentParser()
parser.add_argument('function')
parser.add_argument('--data', default='{}')
parser.add_argument('--port', type=int, default=17877)
parser.add_argument('--timeout', type=float, default=60)
args = parser.parse_args()
if not 1024 <= args.port <= 65535:
    parser.error('Invalid local test port')
request = urllib.request.Request(
    f'https://127.0.0.1:{args.port}/api/v1',
    data=json.dumps({'function': args.function, 'data': json.loads(args.data)}).encode(),
    headers={'Content-Type': 'application/json'}, method='POST')
with urllib.request.urlopen(request, context=ssl._create_unverified_context(), timeout=args.timeout) as response:
    body=response.read()
    if body:
        result=json.loads(body)
        print(json.dumps(result, indent=2))
        if 'errorCode' in result:
            raise SystemExit(1)
    else:
        print(json.dumps({'http_status': response.status}))
