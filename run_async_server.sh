#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SCRIPT_DIR}/build"
BRIDGE_LIB="${BUILD_DIR}/libsatis_task_bridge.so"
SERVER_SCRIPT="${SCRIPT_DIR}/server/FactoryServer.sh"

echo "================================================================="
echo "   Satisfactory Dedicated Server - Asynchronous Engine Launcher"
echo "================================================================="

if [[ ! -f "${BRIDGE_LIB}" ]]; then
    echo ">> Building satis_task_bridge library..."
    cmake -S "${SCRIPT_DIR}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=RelWithDebInfo
    cmake --build "${BUILD_DIR}" -j$(nproc)
fi

if [[ ! -f "${SERVER_SCRIPT}" ]]; then
    echo "ERROR: ${SERVER_SCRIPT} not found!"
    echo "Please ensure the official dedicated server is installed in ${SCRIPT_DIR}/server"
    exit 1
fi

export LD_PRELOAD="${BRIDGE_LIB}${LD_PRELOAD:+:${LD_PRELOAD}}"
export SATIS_ASYNC_TICK=1

echo ">> Configuration:"
echo "   - LD_PRELOAD: ${BRIDGE_LIB}"
echo "   - SATIS_ASYNC_TICK: ${SATIS_ASYNC_TICK}"
echo "   - Server Target: ${SERVER_SCRIPT}"

# Satisfactory / Unreal Engine refuses to run with root privileges
if [[ $(id -u) -eq 0 ]]; then
    RUN_USER="${SATIS_USER:-nobody}"
    echo ">> Notice: Unreal Engine refuses to run as root. Switching to user '${RUN_USER}'."
    echo "================================================================="
    exec su -s /bin/bash "${RUN_USER}" -c "export LD_PRELOAD='${BRIDGE_LIB}${LD_PRELOAD:+:${LD_PRELOAD}}'; export SATIS_ASYNC_TICK=1; '${0}' \"$@\"" -- "${SERVER_SCRIPT}" "$@"
else
    echo ">> Launching Satisfactory Dedicated Server with Asynchronous Task Bridge..."
    echo "================================================================="
    exec "${SERVER_SCRIPT}" "$@"
fi

