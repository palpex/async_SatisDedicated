#!/usr/bin/env python3
"""Install the standalone pipeline guard in one existing server, without restart.

Requires an existing local Frida installation and permission to attach. The
installed native replacement deliberately survives detach until process exit;
startup persistence is configured separately through systemd LD_PRELOAD.
"""
import argparse
import json
import os
import pathlib
import struct
import subprocess
import threading


def read_status(pid, library):
    """Read native atomics without attaching another instrumentation session."""
    proc = pathlib.Path('/proc') / str(pid)
    mappings = (proc / 'maps').read_text().splitlines()
    library_stat = library.stat()
    image = next(row.split(maxsplit=5) for row in mappings
                 if len(row.split(maxsplit=5)) == 6
                 and row.split(maxsplit=5)[5] == str(library)
                 and row.split()[2] == '00000000')
    if int(image[4]) != library_stat.st_ino:
        raise RuntimeError('Mapped library and on-disk inode differ; do not read guessed offsets')
    mapped_device = tuple(int(x, 16) for x in image[3].split(':'))
    if mapped_device != (os.major(library_stat.st_dev), os.minor(library_stat.st_dev)):
        raise RuntimeError('Mapped library and on-disk device differ')
    base = int(image[0].split('-')[0], 16)
    symbols = {parts[2]: int(parts[0], 16)
               for row in subprocess.check_output(['nm', '--defined-only', str(library)], text=True).splitlines()
               if len(parts := row.split()) == 3}
    values = {}
    with (proc / 'mem').open('rb') as mem:
        for key, symbol, fmt in (
                ('checks', '_ZN12_GLOBAL__N_16checksE', 'Q'),
                ('fallbacks', '_ZN12_GLOBAL__N_19fallbacksE', 'Q'),
                ('liveMode', '_ZN12_GLOBAL__N_19live_modeE', '?'),
                ('liveOriginal', '_ZN12_GLOBAL__N_113live_originalE', 'Q')):
            mem.seek(base + symbols[symbol])
            values[key] = struct.unpack('<' + fmt, mem.read(struct.calcsize(fmt)))[0]
    values['pid'] = pid
    values['liveInstalled'] = values.pop('liveOriginal') != 0 and values['liveMode']
    values['asyncBridgeLoaded'] = any('libsatis_task_bridge.so' in row for row in mappings)
    return values


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--pid", type=int, required=True)
    parser.add_argument("--library", type=pathlib.Path, required=True)
    parser.add_argument("--check-only", action="store_true", help="Validate bindings without intercepting gameplay")
    parser.add_argument("--status-only", action="store_true", help="Read an already-loaded guard without installing")
    args = parser.parse_args()
    library = args.library.resolve(strict=True)
    if args.pid <= 1 or (args.check_only and args.status_only):
        parser.error("Select a server PID and one operation")
    if args.status_only:
        print(json.dumps(read_status(args.pid, library)), flush=True)
        return
    import frida
    session = frida.attach(args.pid)
    script = None
    installed = False
    result = []
    done = threading.Event()
    try:
        script = session.create_script("""
        const library = LIBRARY;
        const statusOnly = STATUS_ONLY;
        const checkOnly = CHECK_ONLY;
        const name = library.substring(library.lastIndexOf('/') + 1);
        const guard = statusOnly ? Process.getModuleByName(name) : Module.load(library);
        const fn = (name, result, args = []) => new NativeFunction(guard.getExportByName(name), result, args);
        const init = fn('satis_pipeline_guard_init', 'int');
        const status = fn('satis_pipeline_guard_status', 'pointer');
        if (init() !== 0) throw new Error(status().readUtf8String());
        const isInstalled = fn('satis_pipeline_guard_live_installed', 'int');
        const cancel = fn('satis_pipeline_guard_cancel_live', 'void');
        let target = null;
        let installedHere = false;
        rpc.exports = {rollback() {
            if (installedHere) {
                Interceptor.revert(target);
                Interceptor.flush();
                cancel();
                installedHere = false;
            }
        }};
        let didInstall = false;
        if (!statusOnly && !checkOnly && !isInstalled()) {
            const factory = Process.getModuleByName('libFactoryServer-FactoryGame-Linux-Shipping.so');
            const symbol = '_ZN30AFGBuildablePipelineAttachment24Dismantle_ImplementationEv';
            const prepare = fn('satis_pipeline_guard_prepare_live', 'void');
            const setOriginal = fn('satis_pipeline_guard_set_original', 'void', ['pointer']);
            target = factory.getExportByName(symbol);
            prepare();
            try {
                const trampoline = Interceptor.replaceFast(target, guard.getExportByName(symbol));
                installedHere = true;
                setOriginal(trampoline);
                Interceptor.flush();
            } catch (error) {
                if (installedHere) {
                    Interceptor.revert(target);
                    Interceptor.flush();
                    installedHere = false;
                }
                cancel();
                throw error;
            }
            didInstall = true;
        }
        send({pid: Process.id, status: status().readUtf8String(), didInstall,
            liveInstalled: Boolean(isInstalled()),
            asyncBridgeLoaded: Process.findModuleByName('libsatis_task_bridge.so') !== null,
            checks: fn('satis_pipeline_guard_check_count', 'uint64')().toString(),
            fallbacks: fn('satis_pipeline_guard_fallback_count', 'uint64')().toString()});
        """.replace("LIBRARY", json.dumps(str(library)))
            .replace("STATUS_ONLY", json.dumps(args.status_only))
            .replace("CHECK_ONLY", json.dumps(args.check_only)))

        def on_message(message, _data):
            result.append(message)
            done.set()

        script.on("message", on_message)
        script.load()
        if not done.wait(10):
            raise RuntimeError("No installation status returned; inspect target before retrying")
        for message in result:
            if message.get("type") == "error":
                raise RuntimeError(message.get("stack", message.get("description")))
            payload = message.get("payload", {})
            if payload.get("didInstall"):
                # Required lifetime: detaching must not silently remove the fix.
                script.eternalize()
                installed = True
            print(json.dumps(payload), flush=True)
    finally:
        if script is not None and not installed:
            try:
                script.exports_sync.rollback()
            except (frida.InvalidOperationError, frida.RPCException):
                pass
            script.unload()
        session.detach()


if __name__ == "__main__":
    main()
