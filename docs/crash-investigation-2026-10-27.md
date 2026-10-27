# Crash investigation: Game Thread assertion

**Investigation date:** 2026-10-27  
**Host:** production Satisfactory server on the Ryzen 7 5825U host  
**Scope:** server restarts observed before and after the guarded bridge deployment

## Finding

The confirmed crash was caused by the earlier asynchronous bridge dispatching the complete `FFactoryTickFunction::ExecuteTick` body to an Unreal TaskGraph worker. During a conveyor-group migration, the FactoryGame code called:

`AFGBuildableSubsystem::MigrateConveyorGroupToChainActor` → `AActor::FinishSpawning` → `FEditorScriptExecutionGuard`

`FEditorScriptExecutionGuard` asserted `IsInGameThread()`, followed by signal 11. The crash stack also showed the TaskGraph worker loop and the bridge frame immediately above `ExecuteTick`. This is an engine thread-contract violation, rather than an ordinary out-of-memory or systemd restart.

Two historical crash windows showed the same signature (12:26 and 12:28 UTC): the assertion was in `Runtime/CoreUObject/Private/UObject/ScriptCore.cpp:404`, and both stacks reached `MigrateConveyorGroupToChainActor` from a worker task.

## Current process state

The service restarted at 13:45:53 UTC with:

- `LD_PRELOAD=/home/satisfactory/SatisfactoryDedicatedServer/libsatis_task_bridge.so`
- `SATIS_ASYNC_TICK=1`
- bridge SHA-256: `3f21cd34d4b68097ba72bca0c5a355d734b481fa0ebe0be1a5f58577bfc42665`
- systemd `NRestarts=0` at the time of verification
- no assertion, fatal-signal, or critical-error journal entries after that restart

The corrected bridge checks the verified FactoryGame layout and samples pending conveyor migration, custom handlers, garbage collection, package saving, and completion-gate setup at tick entry. It runs those ticks inline on the Game Thread. Safe ticks remain asynchronous, but the worker call preserves the actual worker lane and does not spoof `IsInGameThread()` or Unreal task tags.

## Limits

The player interaction test (building, connecting, and dismantling belts) was not available during this investigation. The current result proves that the previously observed crash signature is fixed for the guarded entry conditions and that the production process is currently stable. It does not prove that every future actor mutation is safe while asynchronous work is active. A live player session remains the required next validation step.

## Evidence

The raw production journal is intentionally not committed because it contains private server and save identifiers. The repository keeps only this anonymized finding and the existing static-analysis/runtime evidence.
