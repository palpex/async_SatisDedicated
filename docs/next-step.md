# Integration Milestones & Roadmap

This document outlines the roadmap for migrating the Satisfactory Dedicated Server simulation to the native task bridge.

## 1. Verified Milestones (Completed)

- [x] Official binary extraction and DWARF symbol analysis.
- [x] Identification of `FFactoryTickFunction::ExecuteTick`, `AFGBuildableSubsystem::TickFactory`, and `TickFactoryActors` disassembly.
- [x] Reconstructed Core TaskGraph ABI (`AsyncTask`, `FBaseGraphTask`, `FFunctionGraphTask`).
- [x] Implementation of `satis_task_bridge` with batch dispatch and completion graph events.
- [x] Live Frida probe verification inside the official running server:
  - 32 parallel worker tasks verified.
  - Zero worker tasks executed on the Game Thread.
  - Exactly one continuation executed on the Game Thread.
  - Graph event gating verified without premature watcher execution.
- [x] Automated test suite (`pytest`) and runtime evidence verification.
- [x] Public GitHub repository with documentation and co-author credit.

## 2. Next Milestones (Simulation Rewriting)

1. **Completion Graph Event Hooking:**
   - Intercept `FFactoryTickFunction::ExecuteTick` to capture `MyCompletionGraphEvent` passed in register `rcx`.
   - Prevent the engine tick manager from stalling while simulation batches run in background.

2. **Snapshot-Based Factory Actor Processing:**
   - Isolate read-only state for building components and conveyor chains at the beginning of each tick substep.
   - Dispatch decoupled simulation stages through `satis_task_batch`.

3. **Subsystem Decoupling:**
   - Pipeline `AFGPipeSubsystem::SimulatePipeNetworks` alongside `AFGCircuitSubsystem::Tick`.
   - Implement thread-safe synchronization boundaries before inventory and actor state mutations.

4. **Stress & Scaling Benchmarks:**
   - Benchmark with large end-game production saves (e.g. 50k+ buildables).
   - Measure frame tick times, worker thread utilization, and server TPS under heavy load.

