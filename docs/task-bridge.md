# Native Engine Task Bridge

`src/satis_task_bridge.cpp` and `include/satis_task_bridge.h` provide the native integration module for replacing synchronous factory simulation phases with asynchronous worker tasks. They interface directly with Unreal Engine 5 Core's `AsyncTask` and TaskGraph ABI to dispatch work to background worker threads and return continuations to the Game Thread.

## Build and Verification

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=RelWithDebInfo
cmake --build build -j$(nproc)
pytest -v tests/
python3 tests/verify_runtime_evidence.py
```

The verification scripts validate stored real-server runtime evidence and cryptographic hashes of tested files against build manifests.

## Real-Server Runtime Probes

### 1. Task Batch Probe (`evidence/runtime/task-batch-probe.json`)
Records execution inside the official dedicated server via Frida dynamic instrumentation:
- 32 accepted, executed, and cleanly disposed jobs.
- Maximum 3 concurrently active worker jobs.
- Computed checksum identical to serial reference calculation.
- 0 worker jobs executed on the Game Thread.
- Exactly one Game Thread continuation fired after all jobs and context disposals completed.
- Outstanding task counter returned to 0; no script or cleanup errors.
- Server health check following probe: healthy.

### 2. Graph Event Completion Probe (`evidence/runtime/completion-probe.json`)
Records execution with Unreal Engine `FBaseGraphTask` completion gating:
- 32 worker tasks held behind an `FGraphEvent` prerequisite.
- Exactly 1 watcher invocation on the Game Thread after completion signaling (`watcherCalls=1`, `watcherDisposals=1`).
- No premature triggering before signal (`earlyWatcher=0`).
- No execution on incorrect threads (`wrongThread=0`).
- 0 reported failures (`failures=0`).
- Matching result checksum (`checksum == expectedChecksum`).

## Reconstructed ABI Contract

From Core DWARF (`libFactoryServer-Core-Linux-Shipping.so`), CU `0x22`:

- `TUniqueFunction<void()>`: 48 bytes, 16-byte alignment.
  - `Callable` at offset 0.
  - `Storage` at offset 16 (`FFunctionStorage`: 24-byte inline storage, heap pointer at 24).
- `IFunction_OwnedObject`: Vtable slots: clone=0, address=1, destroy=2.
- `ENamedThreads::GameThread = 1`, `AnyNormalThreadNormalTask = 255`.

Core Graph-Event functions resolved at runtime:
- `event_allocate` (Offset `0x4ac890`): Allocates 168 bytes for `FBaseGraphTask` / `FGraphEvent`.
- `event_construct` (Offset `0x4acb90`): Initializes the graph event object.
- `task_unlock` (Offset `0x4aa520`): Unlocks the event and signals dependent subsequent tasks.
- `create_with_prerequisite` (Offset `0x4a38e0`): Attaches dependent task with event prerequisite.
- `task_add_prerequisites` (Offset `0x4b2410`): Adds prerequisite event to an existing engine task (`DontCompleteUntil` logic).

The bridge uses heap-allocated wrapper objects and preserves no references to caller stack frames.

Initialization validates the Core GNU Build-ID (`36a6397531db8df2`), export offset (`0x47b060`), and opcode prefix bytes. Global ELF-COPY relocation resolution is required for `GGameThreadId` so worker tasks accurately detect the main Game Thread.

## Simulation Hooking & Interception

The library exports an interposition hook for the factory simulation tick:
```cpp
void _ZN20FFactoryTickFunction11ExecuteTickEf10ELevelTickN13ENamedThreads4TypeERK12TRefCountPtrI14FBaseGraphTaskE(
    void* this_ptr, float delta_time, uint32_t level_tick, uint32_t current_thread, void* const* completion_event_ref
);
```

When enabled via `satis_async_set_enabled(1)` or `SATIS_ASYNC_TICK=1`:
1. Intercepts the engine tick invocation on `FFactoryTickFunction`.
2. Creates an `FGraphEvent` via `satis_completion_create()`.
3. Binds the completion event to the engine task passed in `completion_event_ref` via `satis_completion_bind_to_task()`.
4. Dispatches the simulation tick asynchronously onto the worker pool via `satis_task_enqueue()`.
5. Returns immediately to the Unreal TickManager, allowing the Game Thread to process other frames.
6. When the worker finishes the factory tick, it signals and releases the completion event, releasing the tick group.

## API & Ownership Model

- `satis_task_enqueue`: Takes context ownership only on return code 0. After `run`, optional `dispose` is called exactly once.
- `satis_task_batch`: Allocates all job objects upfront. If allocation fails prior to dispatch, context ownership remains with caller. Once dispatched, continuation runs only after all jobs and their disposals complete.
- `satis_completion_create`: Allocates and constructs an `FGraphEvent` with external reference.
- `satis_completion_then`: Attaches a callback that runs on the specified lane (GameThread or AnyThread) only after `satis_completion_signal`.
- `satis_completion_bind_to_task`: Links an `FGraphEvent` as prerequisite to an existing engine task.
- `satis_completion_release`: Decrements reference count and safely releases the event.
- `satis_async_set_enabled` / `satis_async_is_enabled`: Controls dynamic simulation tick offloading.
- `satis_async_tick_count` / `satis_async_dispatch_count`: Performance metrics for intercepted ticks.

