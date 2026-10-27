# Reverse Engineering Investigation & Ground Truth Report

This report outlines the verified binary analysis and architectural findings for the Satisfactory Dedicated Server asynchronous multithreading initiative.

## Target Ground Truth & Environment

The official Linux Dedicated Server was installed via SteamCMD with validation:
- **App ID:** `1690800`
- **Linux Depot:** `1690802`
- **Manifest:** `2874226577494387589`
- **Build ID:** `25782485`
- **Binary Digest:** `libFactoryServer-FactoryGame-Linux-Shipping.so` (SHA-256: `637c2c70e65f6386697a6a5f5746f64b187d2fcfeee56df1aeef3f172268c0c1`)
- **Export Inventory:** `evidence/rea-factorygame-exports.bundle.json` (50,511 exported symbols).
- **Tooling:** REA 6.8.0, Ghidra 12.1.4, OpenJDK 21, Clang/GCC C++17.

## Static Analysis Summary

| Function | Relative ELF Offset | Evidence File |
| --- | --- | --- |
| `AFGBuildableSubsystem::TickFactory` | `0x187faf0` | `tick-factory-assembly.txt` |
| Substep invocation of `TickFactoryActors` | `0x187fc1b` | `tick-factory-assembly.txt` |
| `AFGBuildableSubsystem::TickFactoryActors` | `0x187fc40` | `tick-factory-assembly.txt` |
| First `ParallelForInternal` invocation | `0x187fdf3` | `tick-factory-assembly.txt` |
| Second `ParallelForInternal` invocation | `0x18801da` | `tick-factory-assembly.txt` |
| Third `ParallelForInternal` invocation | `0x1880f2d` | `tick-factory-assembly.txt` |
| `ParallelForInternal` template instantiation | `0x184eea0` | `parallel-for-assembly.txt` |

Addresses are relative to ELF image base.

## Callback Implementations

Four local `TFunctionRefCaller` bodies were identified in `evidence/factory-tick-callbacks-assembly.txt`:
- `$_0` (`0x189dbe0`): Reads pointer list at `this+0x3c0` (count at `+0x3c8`) and calls vtable slot `+0x930` with tick delta.
- `$_1` (`0x189dcc0`): Reads groups at `this+0x490`/`+0x498` and calls `AFGConveyorChainActor::Factory_Tick(float)`.
- `$_2` (`0x189dd80`): Reads groups at `this+0x468`/`+0x470`, ticking `ChainActor` directly or accumulating tick deltas for conveyor objects.
- `$_3` (`0x189dec0`): Processes group list at `this+0x3e8`/`+0x3f0`.

DWARF debug info at Compilation Unit `0x314b779` confirms the `FConveyorTickGroup` layout (32 bytes: `ForceIntoSharedByBuildable` at 0, `Conveyors` array at 8, `ChainActor` at 24).

## Server Baseline Profile

A dedicated server instance was launched locally on loopback. A baseline save (`AsyncRewriteBaseline.sav`) was created, unpaused, and saved via HTTP API. Initial execution metrics show that all major subsystem ticks ran exclusively on thread `172161` (the main Game Thread).

## Native Task Bridge Verification

The native bridge (`src/satis_task_bridge.cpp`) was compiled and injected into the running dedicated server process using Frida. Across 32 worker tasks:
- Maximum concurrency of 3 was observed.
- All tasks ran on worker threads; zero ran on the Game Thread.
- One Game Thread continuation was dispatched upon completion.
- Graph completion event holding and release were successfully verified.

