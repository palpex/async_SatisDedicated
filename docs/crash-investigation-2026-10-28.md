# Crash Investigation: Dismantle Race Condition in ParallelFor

**Investigation date:** 2026-10-28  
**Host:** Production Satisfactory Dedicated Server (Ryzen 7 5825U)  
**Trigger:** Live player interaction and building dismantling  

---

## 1. Executive Summary

During active player testing on the production server (00:32–00:45 UTC / 01:32–01:45 CET), the server crashed twice in rapid succession:
- **Crash 1:** 00:39:43 UTC (Signal 11 / SIGSEGV at `0x189dc93`)
- **Crash 2:** 00:44:35 UTC (Signal 11 / SIGSEGV at `0x189dc93`)

Both crashes occurred immediately while connected players were dismantling factory structures using the in-game dismantle tool (`BP_MaterialEffect_Dismantle_C`).

The root cause is a classic concurrent read-write race condition: asynchronous offloading of `FFactoryTickFunction::ExecuteTick` to TaskGraph worker threads allowed background workers to iterate through `AFGBuildableSubsystem::mBuildables` while the Game Thread simultaneously modified the array and destroyed actor instances during player dismantle RPCs.

---

## 2. Crash Signature & Disassembly

### Backtrace from Server Journal (00:39:43 UTC and 00:44:35 UTC)
```text
Unhandled Exception: SIGSEGV: invalid attempt to read memory at address 0x0000000000000000
libFactoryServer-FactoryGame-Linux-Shipping.so!UE::Core::Private::Function::TFunctionRefCaller<AFGBuildableSubsystem::TickFactoryActors(float)::$_0, void, int>::Call(void*, int&) [FGBuildableSubsystem.cpp:644]
libFactoryServer-FactoryGame-Linux-Shipping.so!void ParallelForImpl::ParallelForInternal<...>::FParallelExecutor::operator()(bool) const
libFactoryServer-Core-Linux-Shipping.so!LowLevelTasks::FScheduler::ExecuteTask(...)
libFactoryServer-Core-Linux-Shipping.so!LowLevelTasks::FScheduler::WorkerLoop(...)
```

### Disassembly at Crash Point (`0x189dc80 - 0x189dc9c`)
```asm
 189dc80:  mov    0x3c0(%r14),%rax   # %rax = mBuildables array pointer
 189dc87:  mov    (%rax,%rbp,8),%rdi # %rdi = mBuildables[index] (AFGBuildable*)
 189dc8b:  mov    0x10(%rbx),%rax    # %rax = &DeltaTime
 189dc8f:  movss  (%rax),%xmm0       # %xmm0 = DeltaTime
 189dc93:  mov    (%rdi),%rax        # CRASH POINT: dereferences vtable pointer at %rdi
 189dc96:  call   *0x930(%rax)       # virtual call: AFGBuildable::Factory_Tick(dt)
 189dc9c:  inc    %rbp
```

When a player dismantles a machine:
1. The client sends a dismantle RPC to the server.
2. The server processes the dismantle RPC on the **Game Thread**, invoking `AFGBuildableSubsystem::RemoveBuildable(AFGBuildable*)`.
3. `RemoveBuildable` modifies the `mBuildables` array and marks the actor for destruction.
4. Concurrently, a background TaskGraph worker thread executing the offloaded `TickFactoryActors` is iterating through `mBuildables` in a `ParallelFor` batch.
5. The worker reads a null or invalidated pointer at index `%rbp`, resulting in `%rdi = 0x0`.
6. `mov (%rdi), %rax` attempts to read from address `0x0`, triggering an instant segmentation fault (`SIGSEGV`).

---

## 3. Why Idle Testing Passed While Multiplayer Failed

- **Idle World (0 Players):** When no players are connected, no build or dismantle actions take place. The `mBuildables` array remains static in size and memory location. Workers read valid pointers without mutation, producing stable execution and clean metrics.
- **Active Multiplayer (>0 Players):** Players continuously send construct, dismantle, and interaction packets. Because Unreal Engine's actor containers (`TArray<AFGBuildable*>`) lack internal read-write synchronization, concurrent modification by the Game Thread during an asynchronous worker tick guarantees data races and pointer invalidation.

---

## 4. Remediation Options

1. **Immediate Playability (Synchronous Mode):**
   Toggle `satis_async_set_enabled(0)` or set `SATIS_ASYNC_TICK=0` in the systemd configuration. This routes all ticks through the vanilla Game Thread execution path, completely eliminating races and crashes so players can play normally.

2. **Player-Aware Dynamic Fallback:**
   Configure the bridge to automatically run synchronously whenever `numConnectedPlayers > 0` or when construction/dismantle events are in progress.

3. **Fine-Grained Worker Splitting:**
   Instead of offloading the entire actor simulation loop (which touches unprotected actor containers), restrict parallel execution to pure numeric subsystems that do not modify or read UObject actor arrays concurrently with Game Thread mutations.
