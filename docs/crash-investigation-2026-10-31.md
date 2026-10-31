# Pipeline attachment dismantle crash, 2026-10-31

The recent production crashes come from a player dismantle RPC on the Game
Thread. They are different from the historical asynchronous factory tick crash.

## Observed failure

The journal contains the same signature at 2026-10-30 23:18:51, 23:20:14 and
23:36:51 UTC (2026-10-31 00:18:51, 00:20:14 and 00:36:51 CET):

```text
Assertion failed: O != __null
FGDismantleInterface.gen.cpp:48
IFGDismantleInterface::Execute_CanDismantle(UObject const*)
AFGBuildablePipelineAttachment::Dismantle_Implementation()
UFGBuildGunStateDismantle::Internal_DismantleActor(...)
UFGBuildGunStateDismantle::Server_DismantleActors_Implementation(...)
```

The last crash occurred after `LD_PRELOAD` had been removed and the process
was verified not to map `libsatis_task_bridge.so`. Disabling async alone therefore
does not prevent this crash. The assertion handler's subsequent SIGSEGV writes
to address `0x3`; that address is not evidence of an independent worker race.

Private raw journal excerpts are retained outside the repository because they
contain player authentication material. No recent core dump was found in the
two inspected Saved/Crashes locations or systemd's core directory.

## Binary evidence

The production and local FactoryGame library both have SHA-256
`637c2c70e65f6386697a6a5f5746f64b187d2fcfeee56df1aeef3f172268c0c1`,
GNU build ID `7dd11ed7c1cc5e43` (Steam build `25782485`).

REA objdump evidence
`ev_abe974190f823a2f3042140eb94ac7dc3e8c5080f087b6c6814842b3032c64c5`
shows this flow in the official binary:

- `0x17166f0`: attachment dismantle entry.
- `0x1716880`–`0x17168e2`: get the two connected components' owners and cast
  them to pipelines. Failed casts set `r14` or `r15` to null.
- `0x171690f` / `0x171691b`: call `Execute_CanDismantle` without checking
  those cast results.
- `0x17169b6` / `0x17169dd`: even the failed-CanDismantle logging branch
  dereferences both owners. Merely returning false for null is insufficient.
- `0x1716a97`: the existing base factory dismantle path, which skips optional
  pipe merging.

The runtime stack establishes the null argument and its caller. Static analysis
explains how a missing or non-pipeline owner can produce it; which particular
actor/connection caused the cast failure has not been recorded.

## Repair

`satis_pipeline_guard` is a separate synchronous library. It does not intercept
factory ticking or enable the async bridge. It interposes only the pipeline
attachment dismantle implementation. It uses the same component enumeration
as the original, including its inline-array ABI, names, peer validity flag and
class-ancestry test.

When both named peers exist but either owner is missing or is not a pipeline,
the guard calls the original base factory dismantle method. The attachment can
be removed without entering the invalid optional merge. Valid pipeline pairs
retain the original behavior. Missing peers retain the original path, whose
existing checks already skip merging. In the guarded case neighboring pipes
are not automatically merged.

Build ID, function offset, instruction prefixes and required symbols are
validated before accessing engine layouts. Unsupported builds retain original
behavior and report why the guard is unavailable.

The guard was installed in the existing production process without another
server restart. A native replacement retained in the process handles the live
installation; a dedicated systemd drop-in preloads only this guard on the next
start. `SATIS_ASYNC_TICK=0` remains configured, and the running process has no
async bridge mapped. Configuration backups remain on the server.

## Verification and limits

The existing suite plus the guard regression test pass. The new native harness
runs with AddressSanitizer and UndefinedBehaviorSanitizer and covers null and
non-pipeline owners on either side, valid and derived pipeline classes,
missing/destroyed peers, duplicate names, heap ownership, unsupported builds,
thread checks and live trampoline publication.

Live binding validation succeeded; process memory independently confirmed the
loaded guard, live replacement state, and absence of the async bridge after
the installer detached. No player dismantle action had yet exercised the guard
when these checks were recorded. A repeat of the triggering player action is
still needed for gameplay confirmation.

## Subsequent async restart

At the administrator's request the service restarted at 2026-10-30 23:58:23 UTC
(2026-10-31 00:58:23 CET) with both the pipeline guard and async bridge preloaded.
The first startup observation at 23:59:09 UTC reported 531 dispatched and completed
async ticks, zero in-flight ticks or synchronous fallbacks, zero fatal log matches,
and a healthy API on native port 7777 at approximately 29.81 TPS. No players were
connected in that sample. This supersedes the earlier temporary synchronous
operating mode without changing the historical finding that the null-owner crash
also occurred without async.

A durable two-hour supervisor is running until 2026-10-31 01:59:09 UTC
(02:59:09 CET). Existing systemd auto-restart plus the bounded supervisor handle
process failure or sustained API unavailability. The full observation is not
complete. See [anonymous startup evidence](../evidence/runtime/async-alpha5-startup-2026-10-31.json).

## Separate API availability issue

After the unmodified restart the API selected TCP 7778, leaving the expected
TCP 7777 unavailable. A temporary socket-activated TCP proxy restores 7777 to
the existing 7778 listener without restarting the game. UDP 7777 and reliable
TCP 8888 remain owned by the server. HealthCheck through TCP 7777 returned
healthy and QueryServerState reported approximately 29.7 TPS.

The compatibility socket/service are runtime units under `/run/systemd/system`
and are part of the game service's stop/restart lifecycle. Their removal belongs
with a planned restart that restores the native API listener on 7777.

## Build and operation

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build --target satis_pipeline_guard
python3 -m pytest -q tests/
```

For a compatible server, preload `build/libsatis_pipeline_guard.so` directly,
without `run_async_server.sh` or `libsatis_task_bridge.so`. The live installer
requires the existing Frida Python package and attach privileges:

```bash
python3 scripts/install_pipeline_guard_live.py --pid SERVER_PID --library /absolute/path/libsatis_pipeline_guard.so --check-only
python3 scripts/install_pipeline_guard_live.py --pid SERVER_PID --library /absolute/path/libsatis_pipeline_guard.so
python3 scripts/install_pipeline_guard_live.py --pid SERVER_PID --library /absolute/path/libsatis_pipeline_guard.so --status-only
```

Status reads the native counters through `/proc`, without another Frida
attachment. The library must remain unstripped and match the mapped inode.
Never overwrite a mapped library in place. A retained live replacement lasts
until process exit; future starts use the separately configured preload.
