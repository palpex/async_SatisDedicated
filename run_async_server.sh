#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="${SATIS_BUILD_DIR:-${SCRIPT_DIR}/build}"
BRIDGE_LIB="${SATIS_BRIDGE_LIB:-${BUILD_DIR}/libsatis_task_bridge.so}"
SERVER_DIR="${SATIS_SERVER_DIR:-${SCRIPT_DIR}/server}"
SERVER_SCRIPT="${SATIS_SERVER_SCRIPT:-${SERVER_DIR}/FactoryServer.sh}"
ASYNC_TICK="${SATIS_ASYNC_TICK:-1}"
FACTORY_CVARS="${SATIS_FACTORY_CVARS:-}"
CVAR_ARGS=()

echo "================================================================="
echo "   Satisfactory Dedicated Server - Task Bridge Launcher"
echo "================================================================="

if [[ ! -f "${BRIDGE_LIB}" ]]; then
    if [[ -n ${SATIS_BRIDGE_LIB:-} ]]; then
        echo "ERROR: SATIS_BRIDGE_LIB does not exist: ${BRIDGE_LIB}" >&2
        exit 1
    fi
    echo ">> Building satis_task_bridge library..."
    cmake -S "${SCRIPT_DIR}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=RelWithDebInfo
    cmake --build "${BUILD_DIR}" -j$(nproc)
fi
if [[ ! -r "${BRIDGE_LIB}" ]]; then
    echo "ERROR: library is not readable: ${BRIDGE_LIB}" >&2
    exit 1
fi
# The ELF loader splits LD_PRELOAD at spaces and colons without shell quoting.
if [[ ${BRIDGE_LIB} =~ [[:space:]:] ]]; then
    echo "ERROR: the library path must not contain spaces or colons (LD_PRELOAD limitation)." >&2
    exit 1
fi
BRIDGE_LIB=$(realpath -- "${BRIDGE_LIB}")

if [[ ! -f "${SERVER_SCRIPT}" ]]; then
    echo "ERROR: ${SERVER_SCRIPT} not found!"
    echo "Set SATIS_SERVER_DIR to the official dedicated server directory."
    exit 1
fi

if [[ ! -x "${SERVER_SCRIPT}" ]]; then
    echo "ERROR: ${SERVER_SCRIPT} is not executable."
    exit 1
fi
if [[ -n "${FACTORY_CVARS}" ]]; then
    if [[ ! "${FACTORY_CVARS}" =~ ^[A-Za-z0-9_.-]+=([A-Za-z0-9_.-]+)(,[A-Za-z0-9_.-]+=([A-Za-z0-9_.-]+))*$ ]]; then
        echo "ERROR: SATIS_FACTORY_CVARS must be comma-separated name=value pairs." >&2
        exit 1
    fi
    IFS=, read -r -a _cvar_pairs <<< "${FACTORY_CVARS}"
    for _pair in "${_cvar_pairs[@]}"; do CVAR_ARGS+=("-${_pair}"); done
fi
if [[ "${ASYNC_TICK}" != 0 && "${ASYNC_TICK}" != 1 && "${ASYNC_TICK}" != true && "${ASYNC_TICK}" != false ]]; then
    echo "ERROR: SATIS_ASYNC_TICK must be 0, 1, true, or false."
    exit 1
fi
PRELOAD_VALUE="${BRIDGE_LIB}${LD_PRELOAD:+:${LD_PRELOAD}}"

echo ">> Configuration:"
echo "   - LD_PRELOAD: ${PRELOAD_VALUE}"
echo "   - SATIS_ASYNC_TICK: ${ASYNC_TICK}"
if [[ -n "${FACTORY_CVARS}" ]]; then echo "   - SATIS_FACTORY_CVARS: ${FACTORY_CVARS}"; fi
echo "   - Server Target: ${SERVER_SCRIPT}"

# Satisfactory / Unreal Engine refuses to run with root privileges.
if [[ $(id -u) -eq 0 ]]; then
    RUN_USER="${SATIS_USER:-}"
    if [[ -z "${RUN_USER}" ]]; then
        echo "ERROR: do not run the server as root; set SATIS_USER to its service account."
        exit 1
    fi
    if ! id "${RUN_USER}" >/dev/null 2>&1; then
        echo "ERROR: SATIS_USER '${RUN_USER}' does not exist."
        exit 1
    fi
    if [[ $(id -u "${RUN_USER}") -eq 0 ]]; then
        echo "ERROR: SATIS_USER must be an unprivileged account." >&2
        exit 1
    fi
    echo ">> Notice: Unreal Engine refuses to run as root. Switching to user '${RUN_USER}'."
    echo "================================================================="
    exec runuser -u "${RUN_USER}" -- env LD_PRELOAD="${PRELOAD_VALUE}" SATIS_ASYNC_TICK="${ASYNC_TICK}" "${SERVER_SCRIPT}" "${CVAR_ARGS[@]}" "$@"
else
    echo ">> Launching Satisfactory Dedicated Server with Task Bridge..."
    echo "================================================================="
    exec env LD_PRELOAD="${PRELOAD_VALUE}" SATIS_ASYNC_TICK="${ASYNC_TICK}" "${SERVER_SCRIPT}" "${CVAR_ARGS[@]}" "$@"
fi

