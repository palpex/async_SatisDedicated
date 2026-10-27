#pragma once
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

// Experimental bridge to Core's AsyncTask ABI in Steam build 25782485.
// Callbacks must be noexcept C functions. Ownership transfers only on success.
// They may access only data for which the caller has established thread safety.
typedef void (*satis_task_callback)(void*);
typedef void (*satis_task_dispose)(void*);
enum satis_task_lane { SATIS_GAME_THREAD = 0, SATIS_WORKER_THREAD = 1 };
int satis_task_bridge_init(void);
const char* satis_task_bridge_error(void);
int satis_task_enqueue(satis_task_callback run, void* context,
                       satis_task_dispose dispose, enum satis_task_lane lane);
struct satis_task_item {
    satis_task_callback run;
    void* context;
    satis_task_dispose dispose;
};
// Fan out worker tasks and enqueue a continuation only after all jobs and
// disposals finish. No waits or caller stack captures. On failure, ownership
// of all supplied contexts stays with the caller.
int satis_task_batch(const struct satis_task_item* items, uint32_t count,
                     satis_task_callback continuation, void* continuation_context,
                     satis_task_dispose continuation_dispose,
                     enum satis_task_lane continuation_lane);
uint64_t satis_task_outstanding(void);
// Nonzero after every accepted callback and dispose callback completed.
// The module must stay loaded until server exit. A zero counter is a state
// observation, not permission to unload: destructor return instructions can
// still reside in this module.
int satis_task_quiescent(void);

// Bounded pure-data probe. Never invokes or changes gameplay state.
int satis_task_probe_start(uint32_t tasks, uint32_t iterations);
struct satis_task_probe_result {
    uint64_t submitted, completed, destroyed, max_concurrency;
    uint64_t checksum, expected_checksum, wrong_thread, failures;
    uint64_t continuation_calls, continuation_disposals, continuation_wrong_thread;
};
int satis_task_probe_read(struct satis_task_probe_result* result);
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
extern "C" {
#endif
// One externally signaled Unreal graph event, with an external owning ref.
// All operations require successful bridge initialization. Callers serialize
// access to this handle and must not release it concurrently with other calls.
struct satis_completion;
struct satis_completion* satis_completion_create(void);
int satis_completion_then(struct satis_completion* completion,
                           satis_task_callback run, void* context,
                           satis_task_dispose dispose, enum satis_task_lane lane);
int satis_completion_signal(struct satis_completion* completion);
void satis_completion_release(struct satis_completion* completion);
int satis_completion_probe_start(uint32_t tasks, uint32_t iterations);
struct satis_completion_probe_result {
    uint64_t submitted, completed, destroyed, watcher_calls, watcher_disposals;
    uint64_t early_watcher, wrong_thread, checksum, expected_checksum;
    uint64_t signal_calls, failures, max_concurrency;
};
int satis_completion_probe_read(struct satis_completion_probe_result* result);
#ifdef __cplusplus
}
#endif
