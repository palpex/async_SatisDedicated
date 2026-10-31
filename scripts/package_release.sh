#!/usr/bin/env bash
# Assemble project-owned files only. Official server binaries and saves excluded.
set -euo pipefail
project_root=$(cd -- "$(dirname -- "$0")/.." && pwd)
cd "$project_root"
version=v0.1.0-alpha.5
if [[ $# -gt 0 ]]; then version=$1; fi
[[ $version =~ ^v[0-9]+[.][0-9]+[.][0-9]+(-[a-zA-Z0-9.]+)?$ ]] || exit 2
for library in build/libsatis_task_bridge.so build/libsatis_pipeline_guard.so; do
    [[ -f "$library" ]] || { echo "Build both libraries first: missing $library" >&2; exit 2; }
done
stage=$(mktemp -d)
trap 'rm -rf -- "$stage"' EXIT
install -m 0755 build/libsatis_task_bridge.so build/libsatis_pipeline_guard.so run_async_server.sh "$stage/"
install -m 0644 README.md "$stage/"
mkdir -p "$stage/docs/releases" "$stage/include" "$stage/scripts" "$stage/docs/assets" "$stage/evidence/runtime"
install -m 0644 docs/installation.md docs/task-bridge.md docs/threading_architecture_analysis.md docs/crash-investigation-2026-10-27.md docs/crash-investigation-2026-10-31.md "$stage/docs/"
install -m 0644 docs/performance_benchmarks.md docs/investigation.md docs/next-step.md "$stage/docs/"
install -m 0644 docs/assets/*.svg docs/assets/*.png "$stage/docs/assets/"
install -m 0644 evidence/runtime/guarded-observation.json evidence/runtime/guarded-followup-observation.json evidence/runtime/release-candidate-observation.json evidence/runtime/guarded-incremental-observation.json evidence/runtime/crash-investigation-2026-10-27.json evidence/runtime/scaling-sweep.json evidence/runtime/pipeline-dismantle-guard-2026-10-31.json evidence/runtime/async-alpha5-startup-2026-10-31.json "$stage/evidence/runtime/"
install -m 0644 "docs/releases/$version.md" "$stage/docs/releases/"
install -m 0644 include/satis_task_bridge.h "$stage/include/"
install -m 0755 scripts/monitor_server.py scripts/observe_server.py scripts/install_pipeline_guard_live.py scripts/watch_server.py "$stage/scripts/"
git rev-parse HEAD > "$stage/SOURCE_COMMIT"
(cd "$stage" && sha256sum libsatis_task_bridge.so libsatis_pipeline_guard.so > SHA256SUMS)
cat "$stage/SHA256SUMS"
mkdir -p dist
asset="async-satis-$version-linux-x86_64.tar.gz"
tar -czf "dist/$asset" -C "$stage" .
(cd dist && sha256sum "$asset" > "$asset.sha256")
echo "Release assets: dist/$asset and dist/$asset.sha256"
