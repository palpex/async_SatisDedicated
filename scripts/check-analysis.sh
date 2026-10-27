#!/usr/bin/env bash
set -euo pipefail
project_root=$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")/.." && pwd)
cd -- "$project_root"
python3 - <<'PY'
import json, os, pathlib
state = json.loads(pathlib.Path('evidence/analysis-state.json').read_text())
for key in ('rea_pid', 'ghidra_pid'):
    pid = state[key]
    path = pathlib.Path('/proc') / str(pid) / 'cmdline'
    try:
        command = path.read_bytes().replace(b'\0', b' ').decode(errors='replace')
    except FileNotFoundError:
        print(f'{key}: PID {pid} absent')
        continue
    expected = 'rea function' if key == 'rea_pid' else 'ghidra.app.util.headless.AnalyzeHeadless'
    if expected not in command:
        print(f'{key}: PID {pid} no longer matches original command')
    else:
        print(f'{key}: PID {pid} live and command matches')
output = pathlib.Path(state['output'])
print(f'Output: {output.stat().st_size if output.exists() else 0} bytes')
log = pathlib.Path(state['runtime_directory']) / 'ghidra.log'
if log.exists():
    print('\n'.join(log.read_text(errors='replace').splitlines()[-8:]))
PY
