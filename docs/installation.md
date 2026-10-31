# Installation and rollback

This project is an experimental native bridge for one validated Linux Satisfactory Dedicated Server build. The launcher preloads `libsatis_pipeline_guard.so` and `libsatis_task_bridge.so`, intercepts the factory tick, and keeps eligible factory work asynchronous. A tick with pending conveyor-chain actor migration is routed to the Game Thread because Unreal actor creation and component registration require that thread.

The bridge does not modify the official server binaries. Keep a complete save and configuration backup before enabling it.

## Compatibility

The checked artifact is Steam App `1690800`, Linux depot `1690802`, public build `25782485`. The bridge validates the Core GNU Build-ID and instruction prefixes at startup. A different server build must be analyzed and validated separately; incompatible builds are rejected.

Runtime evidence covers TaskGraph dispatch, graph-event completion, and the conveyor-migration safety fallback. Cross-tick object lifetime, garbage collection during pending work, world transitions, and every UObject access outside the factory tick remain engine-dependent. Treat this release as alpha quality.

## Build and test

Install GCC or Clang with C++17 support, CMake, Python 3.10+, pytest, and binutils. Download the official server separately with SteamCMD (`app_update 1690800 validate`); check its build before enabling this build-specific bridge. Steam's current public build may differ. The prebuilt asset requires Linux x86-64, glibc 2.34+ and libstdc++ with CXXABI_1.3.9.

```bash
git clone https://github.com/palpex/async_SatisDedicated.git
cd async_SatisDedicated
git checkout v0.1.0-alpha.5
```

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build -j"$(nproc)"
pytest -q tests/
python3 tests/verify_runtime_evidence.py
```

The expected libraries are `build/libsatis_task_bridge.so` and `build/libsatis_pipeline_guard.so`. The standalone pipeline guard prevents the confirmed null-owner crash in the optional pipe merge during attachment dismantling; see [the investigation](crash-investigation-2026-10-31.md). Binary digest tests require the supported official server under `server/`; without it those two tests are explicitly skipped. Stored probe verification reads historical evidence; it does not run fresh gameplay tests.

## Download the binary release

```bash
curl -fLO https://github.com/palpex/async_SatisDedicated/releases/download/v0.1.0-alpha.5/async-satis-v0.1.0-alpha.5-linux-x86_64.tar.gz
curl -fLO https://github.com/palpex/async_SatisDedicated/releases/download/v0.1.0-alpha.5/async-satis-v0.1.0-alpha.5-linux-x86_64.tar.gz.sha256
sha256sum -c async-satis-v0.1.0-alpha.5-linux-x86_64.tar.gz.sha256
mkdir async-satis-release
tar -xzf async-satis-v0.1.0-alpha.5-linux-x86_64.tar.gz -C async-satis-release
cd async-satis-release
SATIS_BRIDGE_LIB="$PWD/libsatis_task_bridge.so" SATIS_SERVER_DIR=/srv/satisfactory/SatisfactoryDedicatedServer ./run_async_server.sh -unattended -log
```

The archive contains project-owned binaries/tools/docs only. Obtain the game server from Steam. Library paths containing whitespace or colons are unsupported by LD_PRELOAD; the launcher rejects them. Server paths and arguments may contain spaces.

## Startup CVar experiments

The launcher forwards startup Unreal console variables for matched A/B experiments. Example:

    SATIS_FACTORY_CVARS="t.ParallelTickGroupSize=100,t.UseParallelTick=1" \
    SATIS_SERVER_DIR=/srv/satisfactory/SatisfactoryDedicatedServer \
    SATIS_USER=satisfactory \
    ./run_async_server.sh -unattended -log

Use only names supported by the target build. Never change the parallel group size in a running process; back up the save, restart for each value, verify the setting after startup, and compare the same workload. The isolated startup probes accepted the argument but did not expose a log line confirming the effective value, so runtime telemetry is required before drawing a performance conclusion.

## Run locally

Run as the unprivileged account that owns the server files. The server path defaults to `./server/FactoryServer.sh`; set `SATIS_SERVER_DIR` for another installation.

```bash
SATIS_SERVER_DIR=/srv/satisfactory/SatisfactoryDedicatedServer \
SATIS_ASYNC_TICK=1 \
SATIS_USER=satisfactory \
./run_async_server.sh -unattended -log
```

`SATIS_ASYNC_TICK=0` keeps both libraries preloaded while disabling asynchronous tick dispatch for a controlled baseline. `SATIS_BRIDGE_LIB`, `SATIS_PIPELINE_GUARD_LIB`, `SATIS_BUILD_DIR`, and `SATIS_SERVER_SCRIPT` override the two library paths, build directory, and server script independently. The guard defaults to the bridge library’s directory, so extracted releases load both bundled libraries automatically. Arguments after the launcher are passed unchanged to `FactoryServer.sh`.

The launcher refuses to silently run as root or switch to `nobody`. When invoked as root, supply an existing `SATIS_USER` service account.

## Install with systemd

Stop the service, copy the verified library atomically, and retain the previous file for rollback:

```bash
sudo systemctl stop satisfactory.service
backup_dir=/home/satisfactory/backups/bridge-before-v0.1.0-alpha.5
sudo install -d -m 0700 "$backup_dir"
# Use a new backup directory for each installation; preserve previous backups.
sudo tar -czf "$backup_dir/saved.tar.gz" -C /home/satisfactory/.config/Epic/FactoryGame Saved
sudo tar -tzf "$backup_dir/saved.tar.gz" >/dev/null
sudo cp -a /etc/systemd/system/satisfactory.service "$backup_dir/"
if [ -d /etc/systemd/system/satisfactory.service.d ]; then sudo cp -a /etc/systemd/system/satisfactory.service.d "$backup_dir/"; fi
for library in libsatis_task_bridge.so libsatis_pipeline_guard.so; do
    if [ -f "/home/satisfactory/SatisfactoryDedicatedServer/$library" ]; then sudo cp -p "/home/satisfactory/SatisfactoryDedicatedServer/$library" "$backup_dir/"; fi
done
sudo install -d -o satisfactory -g satisfactory /home/satisfactory/SatisfactoryDedicatedServer
for library in libsatis_task_bridge.so libsatis_pipeline_guard.so; do
    sudo install -o satisfactory -g satisfactory -m 0755 "build/$library" "/home/satisfactory/SatisfactoryDedicatedServer/$library.new"
    sudo mv "/home/satisfactory/SatisfactoryDedicatedServer/$library.new" "/home/satisfactory/SatisfactoryDedicatedServer/$library"
done
```

For the prebuilt archive, replace `build/$library` with `./$library` in the install loop. Verify the extracted libraries with `sha256sum -c SHA256SUMS`. Copy the backup off-host and compare SHA-256 hashes. Do not overwrite a library while any process still maps it; stop the service and install a new inode before renaming it. Existing LD_PRELOAD modules must be preserved explicitly in the drop-in's colon-separated value.

Create `/etc/systemd/system/satisfactory.service.d/async-task-bridge.conf`:

```ini
[Service]
Environment=LD_PRELOAD=/home/satisfactory/SatisfactoryDedicatedServer/libsatis_pipeline_guard.so:/home/satisfactory/SatisfactoryDedicatedServer/libsatis_task_bridge.so
Environment=SATIS_ASYNC_TICK=1
```

Then reload and start the service:

```bash
sudo systemctl daemon-reload
sudo systemctl start satisfactory.service
sudo systemctl status satisfactory.service --no-pager
```

Keep existing `ExecStart` settings and other drop-ins. Consolidate earlier bridge/guard `LD_PRELOAD` assignments into this single combined value: a later drop-in can otherwise replace it. Remove a prior `UnsetEnvironment=LD_PRELOAD` directive when enabling these libraries. Confirm the active process has the expected environment before allowing a player to connect:

```bash
pid=$(pgrep -f 'FactoryServer-Linux-Shipping FactoryGame' | head -n 1)
sudo strings /proc/"$pid"/environ | grep -E '^(LD_PRELOAD|SATIS_ASYNC_TICK)='
```

## Observe and roll back

```bash
sudo journalctl -fu satisfactory.service
sudo journalctl -u satisfactory.service --since '10 minutes ago' | grep -E 'Assertion failed|Critical error|Unhandled Exception|Signal [0-9]+ caught'
sudo python3 scripts/observe_server.py --duration 120 --interval 10 --output observation.json
python3 scripts/monitor_server.py --port 7777 --interval 0
```

The bridge exports `satis_async_tick_count`, `satis_async_dispatch_count`, and `satis_async_sync_fallback_count` for runtime instrumentation. The fallback counter should increase when conveyor migration is encountered; those ticks are intentionally synchronous.

To disable asynchronous dispatch, set `SATIS_ASYNC_TICK=0` in the drop-in, reload systemd, and restart. To run synchronously without the async bridge while retaining the pipeline crash fix, set `LD_PRELOAD` to only `/home/satisfactory/SatisfactoryDedicatedServer/libsatis_pipeline_guard.so`. To remove both libraries, stop the service, remove their `LD_PRELOAD` assignments, reload systemd, and start the official server. Restore the pre-install library and save backup if the server shows a crash or save-integrity problem.

Keep the latest saves before restoring any older archive. A save rollback deliberately discards later progress; inspect the backup contents, stop the service, move the current Saved directory aside, and extract the selected archive as the service account. Never publish saves, tokens, private session names or raw logs containing them.

## Troubleshooting

- `Unsupported Core GNU build-id` or an instruction validation error means that the installed server build does not match the analyzed ABI.
- `IsInGameThread()` assertions, actor/component registration errors, or a crash during player building require disabling the bridge and preserving the first stack trace.
- Inventory-class and NetGUID warnings should be recorded with the surrounding player action; they are not by themselves proof that the bridge caused the issue.

## Pipeline guard and live installation

The pipeline guard contains no asynchronous factory scheduling. When two named
pipe peers exist but an owner is missing or is not a pipeline, it uses the
existing base dismantle path and skips optional automatic pipe merging. Valid
pipeline pairs keep the original behavior. This fixes the observed assertion,
but does not establish that every earlier save or multiplayer action is safe.

For an already running compatible server, the included
`scripts/install_pipeline_guard_live.py` can install this guard without a game
restart when Frida and attach privileges already exist. This is separate from
restarting with asynchronous factory ticks. See the [crash report](crash-investigation-2026-10-31.md)
for exact commands, verification, lifetime, and rollback limits.

## Bounded two-hour observation and recovery

`scripts/watch_server.py` is an optional host-local supervisor that explicitly
starts a failed service or restarts an API-unresponsive process after a grace
period. It preserves the configured simulation mode and never toggles async.
The existing systemd auto-restart remains the first recovery mechanism.

Run it on the server as root with an absolute deadline:

```bash
end_at=$(python3 -c 'import time; print(time.time() + 7200)')
sudo python3 scripts/watch_server.py --end-at "$end_at" --interval 15 --output /var/lib/satisfactory-observations/async-two-hours
```

The helper writes `summary.json` and `samples.jsonl` in a private output
directory. A running observation has `complete=false`; only a completed deadline
sets it true. The summary records process changes, recoveries, API health,
connected-player counts and tick-rate bounds. Raw recovery journal excerpts are
private and must not be published. When running unattended, use a transient
systemd service so observation survives the terminal closing.
