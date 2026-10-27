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

## Parallel-group tuning boundary

The FactoryGame build already enables its internal parallel paths (`UseParallelBuildablesTick`, grouped conveyors, conveyor load balancing, and unbalanced scheduling). `ParallelGroupSize` is a live engine variable used to size internal arrays and partition work. A controlled production test that changed it from 100 to 32 while the process was running produced `Array index out of bounds: 52 into an array of size 32` in `AFGBuildableSubsystem::TickFactoryActors`, followed by signal 11. The value was restored to 100 by the service restart and the post-restart process remained clean.

This variable must not be mutated through `/proc` or another live memory write. Any future tuning must be applied before process startup through a supported configuration or launch-time console-variable mechanism, then verified after initialization. Until that path is identified and tested, the validated value remains 100.

## Guarded alpha: findings and remaining boundaries

| Case | Evidence | Bridge behavior |
| --- | --- | --- |
| Conveyor migration spawns an actor | Two recorded crash stacks reached FinishSpawning through MigrateConveyorGroupToChainActor; original TickFactoryActors reads migration count at +0x400 | Keep a tick with pending migration on the submitting Game Thread |
| Extensible factory handlers run before the first ParallelFor | Stored REA pseudocode and disassembly show an indirect handler call at +0x940/+0x948 | Keep ticks with handlers inline; handler thread contracts are unknown |
| No completion event or binding failure | Hook control-flow review showed dispatch previously continued even when its parent could not be gated | Execute inline; never dispatch untracked work |
| Missing AddNested | Engine DontCompleteUntil uses AddNested for executing tasks | Return an error; never call AddPrerequisites on an executing task as a substitute |
| Factory layout mismatch | Core ABI identity alone says nothing about FactoryGame member offsets | Check FactoryGame GNU build-ID 7dd11ed7c1cc5e43 and ExecuteTick offset before reading layout |
| GC/package save already active at entry | Exported GIsGarbageCollecting and GIsSavingPackage flags | Execute inline; flags do not lock out a future GC/save |
| Incremental reachability pending | TickFactoryActors calls UE::GC::MarkAsReachable while `UE::GC::GIsIncrementalReachabilityPending` is set | Execute inline; do not run the GC reachability path on a worker |
| Worker thread identity | Thread tags and IsInGameThread checks are engine contracts | Preserve checks; no task-tag spoofing; pass CurrentThread=255 for the worker call |

The previous claim that task-tag value 2 meant ParallelGameThread was incorrect: this build compares IsInParallelGameThread against 0x40000002. Tag 2 claims the actual Game Thread. Assigning that tag or overriding IsInGameThread does not make UObject access thread-safe; both mechanisms have been removed.

The migration count and incremental reachability flag are sampled at tick entry. It cannot prove no migration will become pending later in the tick; a broader split between worker computation and Game Thread world mutations remains necessary. This alpha has not established global protection against actor destruction, world transitions, incremental GC, or simultaneous access by all other subsystems. Completion gating protects the declared parent task, not every unrelated engine operation. Internal ParallelFor joins remain in the original factory implementation.

Tests/tick_contract.cpp executes the actual hook against synthetic ABI functions under address/undefined-behavior sanitizers. It verifies fallback, no early signal, worker lane, completion and cleanup control flow. It cannot prove Unreal gameplay safety. Historical REA probes remain stored separately from the new anonymous observational JSON.

