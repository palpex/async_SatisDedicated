# Threading Architecture & Execution Contract Analysis

This report documents the verified reverse-engineering findings of the Satisfactory Dedicated Server execution pipeline.

## Observed Tick Calling Contract

Assembly disassembly at `evidence/tick-completion-assembly.txt` demonstrates:
- `FFactoryTickFunction::ExecuteTick` begins at offset `0x180cba0` in `libFactoryServer-FactoryGame-Linux-Shipping.so`.
- Checks target pointer at TickFunction offset `0x28` (`AFGBuildableSubsystem*`) and object validity flags.
- Multiplies DeltaTime with target factor at offset `+0x70` and synchronously calls `AFGBuildableSubsystem::TickFactory` at `0x180cced`.
- Measures wall-clock monotonic time before and after the call to update global `GTickFactoryTime`.

Under the SysV AMD64 ABI:
- `edx` holds `CurrentThread`.
- `rcx` holds the reference to `MyCompletionGraphEvent` (`const TRefCountPtr<FBaseGraphTask>&`).
- The original function body overwrites these registers immediately without utilizing `MyCompletionGraphEvent` to defer completion. Consequently, the Unreal Engine Tick Manager waits synchronously for `ExecuteTick` to return.

`RegisterFactoryTickFunction` at offset `0x187f690` initializes the embedded tick function at subsystem offset `+0x6d8`, sets the target at `+0x700`, and registers it with the actor level.

## Static Analysis with REA & Ghidra

The shared library is imported with image base `0x100000`. Verified symbol-based Evidence records:
- `TickFactoryActors`: `evidence/rea-corrected-tick-factory-pseudocode.json` (Evidence ID `ev_a43d8a6cbcfd31cab9e0c8c1a1d9716cf6d8ce8790a64cb0fc863fd81ecc4b43`).
- `TickFactory`: `evidence/rea-corrected-tick-substeps.json` (Evidence ID `ev_6eb823912cf018cd8b958b2c07ac6243db293d53d5d266190516cd038ac7c9e7`).
- `ExecuteTick`: `evidence/rea-corrected-execute-tick.json` (Evidence ID `ev_af97a30125ad8e6b6a1dc92b902ddf2225d9c2f1a4cc5a0465e92b168cc89f2c`).
- `SimulatePipeNetworks`: `evidence/rea-corrected-pipe-networks.json` (Evidence ID `ev_c3d31ccc30cb682f27290560d7c0c3a032663475c1a4775d348225c5504ead3e`).

## Subsystem Parallelism & Fork-Join Bottlenecks

`TickFactoryActors` contains three `ParallelForInternal` invocation points:
1. Building pointer arrays.
2. Conveyor chain actor groups (`AFGConveyorChainActor::Factory_Tick`).
3. Conveyor groups (`FConveyorTickGroup`).

Although these utilize background threads, each `ParallelFor` acts as a synchronous fork-join barrier. The Game Thread halts until every item in the batch completes.

In `AFGPipeSubsystem::SimulatePipeNetworks` (offset `0x1b259b0`), active fluid networks are batched and dispatched via `ParallelFor` when `CVarUseParallelSimulation` is enabled.

## Runtime Profiling Observations

`evidence/runtime/baseline-tick-profile.json` records Frida instrumentation across the main subsystem tick entrypoints on a fresh server instance:
- All six entrypoints (`factory`, `factoryActors`, `pipes`, `circuits`, `railroad`, `vehicles`) ran on thread `172161` (GameThread).
- Factory ticks constituted over 60% of total frame simulation time even in a world without player constructions.

## Target Asynchronous Architecture

To transition the server to an asynchronous execution model:
1. Defer `FFactoryTickFunction::ExecuteTick` completion by chaining an `FGraphEvent` to `MyCompletionGraphEvent`.
2. Offload building and conveyor ticks to non-blocking TaskGraph worker batches.
3. Guard against GC, actor destruction, and world transitions during pending asynchronous worker phases.

