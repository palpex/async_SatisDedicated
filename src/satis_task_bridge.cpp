#include "satis_task_bridge.h"
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <mutex>
#include <link.h>
#include <new>
#include <sys/syscall.h>
#include <unistd.h>

struct satis_completion {
    void* event;
    std::atomic<bool> signaled{false};
};

namespace {
// DWARF Core CU 0x22: TUniqueFunction<void()> size 48; Callable at 0,
// Storage at 16; FFunctionStorage has inline bytes[24], HeapAllocation at 24.
// AsyncTask moves all storage bits, then clears Callable/HeapAllocation.
struct alignas(16) UniqueFunction {
    void (*call)(void*);
    std::uint64_t padding{};
    alignas(16) std::byte inline_storage[24]{};
    void* heap{};
};
static_assert(sizeof(UniqueFunction) == 48);
static_assert(offsetof(UniqueFunction, inline_storage) == 16);
static_assert(offsetof(UniqueFunction, heap) == 40);

// Core IFunction_OwnedObject vtable: clone, address, destroy, destructors.
// Heap objects stay at a stable address when the storage itself moves.
class OwnedCallback {
public:
    virtual void* clone(void*) { return nullptr; } // Unique, never copied.
    virtual void* address() { return this; }
    virtual void destroy() { delete this; }
    virtual ~OwnedCallback() {
        if (owned_ && dispose_) dispose_(context_);
        if (counted_) outstanding.fetch_sub(1, std::memory_order_release);
    }
    OwnedCallback(satis_task_callback run, void* context, satis_task_dispose dispose)
        : run_(run), context_(context), dispose_(dispose) {}
    void invoke() { run_(context_); }
    void publish() { counted_=true; outstanding.fetch_add(1,std::memory_order_relaxed); }
    void abandon() { owned_=false; delete this; }
    static std::atomic<std::uint64_t> outstanding;
private:
    satis_task_callback run_;
    void* context_;
    satis_task_dispose dispose_;
    bool owned_{true}, counted_{false};
};
std::atomic<std::uint64_t> OwnedCallback::outstanding{0};
using AsyncTaskFunction = void (*)(std::uint32_t, UniqueFunction*);
AsyncTaskFunction async_task{};
using EventAllocate = void* (*)(std::size_t);
using EventConstruct = void (*)(void*);
using TaskUnlock = bool (*)(void*, bool*);
using TaskAddPrerequisites = bool (*)(void*, void*);
// SysV nontrivial return uses RDI sret. Empty TStatId is omitted in this build.
using CreateWithPrerequisite = void (*)(void**,UniqueFunction*,void* const*,std::uint32_t);
EventAllocate event_allocate{};
EventConstruct event_construct{};
TaskUnlock task_unlock{};
TaskAddPrerequisites task_add_prerequisites{};
using TaskAddNested = void (*)(void*, void*);
TaskAddNested task_add_nested{};
void* engine_handle{};
CreateWithPrerequisite create_with_prerequisite{};

using ExecuteTickSignature = void (*)(void*, float, std::uint32_t, std::uint32_t, void* const*);
std::atomic<ExecuteTickSignature> original_execute_tick{nullptr};
std::atomic<bool> async_tick_enabled{false};
std::atomic<std::uint64_t> intercepted_ticks{0};
std::atomic<std::uint64_t> async_dispatches{0};
std::atomic<std::uint64_t> sync_fallbacks{0};
std::atomic<std::uint64_t> completed_async_ticks{0};
bool factory_layout_supported{};
// Published on the Game Thread during initialization; sampled only at tick entry.
const bool* gc_active{};
const bool* saving_package{};

void task_add_ref(void* task) {
    __atomic_fetch_add(reinterpret_cast<std::uint32_t*>(static_cast<char*>(task)+8),1u,__ATOMIC_RELAXED);
}
void task_release(void* task) {
    if(!task)return;
    if(__atomic_sub_fetch(reinterpret_cast<std::uint32_t*>(static_cast<char*>(task)+8),1u,__ATOMIC_ACQ_REL)==0) {
        auto** vtable=*static_cast<void***>(task);
        reinterpret_cast<void(*)(void*)>(vtable[1])(task);
    }
}
std::atomic<bool> initialized{false};
std::mutex init_mutex;
std::atomic<const char*> failure{"Bridge not initialized"};
std::uint32_t* game_thread_id{};
void* core_handle{}; // Retained for the lifetime of this bridge.
struct BuildCheck { std::uintptr_t base; bool matched{}; const unsigned char* expected_id{}; };
int check_core_build(dl_phdr_info* info, std::size_t, void* opaque) {
    auto& check=*static_cast<BuildCheck*>(opaque);
    if(info->dlpi_addr!=check.base) return 0;
    const unsigned char core_id[]={0x36,0xa6,0x39,0x75,0x31,0xdb,0x8d,0xf2};
    const auto* expected=check.expected_id ? check.expected_id : core_id;
    for(std::size_t i=0;i<info->dlpi_phnum;++i) {
        const auto& ph=info->dlpi_phdr[i];
        if(ph.p_type!=PT_NOTE) continue;
        auto cursor=static_cast<std::uintptr_t>(ph.p_vaddr)+check.base;
        const auto end=cursor+ph.p_memsz;
        while(cursor+sizeof(ElfW(Nhdr))<=end) {
            const auto* note=reinterpret_cast<const ElfW(Nhdr)*>(cursor);
            const auto name=cursor+sizeof(*note);
            const auto desc=name+((note->n_namesz+3u)&~std::uint64_t(3));
            const auto next=desc+((note->n_descsz+3u)&~std::uint64_t(3));
            if(next>end || next<=cursor) return 1;
            if(note->n_type==NT_GNU_BUILD_ID && note->n_namesz==4 &&
               !std::memcmp(reinterpret_cast<void*>(name),"GNU",4) &&
               note->n_descsz==8 &&
               !std::memcmp(reinterpret_cast<void*>(desc),expected,8)) check.matched=true;
            cursor=next;
        }
    }
    return 1;
}
struct EngineFinder { std::uintptr_t base{}; bool matched{}; };
int check_engine_build(dl_phdr_info* info, std::size_t, void* opaque) {
    auto& check = *static_cast<EngineFinder*>(opaque);
    if (check.base != 0) {
        if (info->dlpi_addr != check.base) return 0;
    } else {
        if (!info->dlpi_name || !std::strstr(info->dlpi_name, "libFactoryServer-Engine-Linux-Shipping.so")) {
            return 0;
        }
        check.base = info->dlpi_addr;
    }
    const unsigned char expected[] = {0xb5, 0x0b, 0xbe, 0x68, 0x22, 0x05, 0xdc, 0x29};
    for (std::size_t i = 0; i < info->dlpi_phnum; ++i) {
        const auto& ph = info->dlpi_phdr[i];
        if (ph.p_type != PT_NOTE) continue;
        auto cursor = static_cast<std::uintptr_t>(ph.p_vaddr) + check.base;
        const auto end = cursor + ph.p_memsz;
        while (cursor + sizeof(ElfW(Nhdr)) <= end) {
            const auto* note = reinterpret_cast<const ElfW(Nhdr)*>(cursor);
            const auto name = cursor + sizeof(*note);
            const auto desc = name + ((note->n_namesz + 3u) & ~std::uint64_t(3));
            const auto next = desc + ((note->n_descsz + 3u) & ~std::uint64_t(3));
            if (next > end || next <= cursor) return 1;
            if (note->n_type == NT_GNU_BUILD_ID && note->n_namesz == 4 &&
                !std::memcmp(reinterpret_cast<void*>(name), "GNU", 4) &&
                note->n_descsz == sizeof(expected) &&
                !std::memcmp(reinterpret_cast<void*>(desc), expected, sizeof(expected))) {
                check.matched = true;
                return 1;
            }
            cursor = next;
        }
    }
    return 1;
}
void call_owned(void* object) { static_cast<OwnedCallback*>(object)->invoke(); }
void submit_owned(OwnedCallback* object,satis_task_lane lane) {
    object->publish();
    UniqueFunction function{};
    function.call=call_owned;
    function.heap=object;
    async_task(lane==SATIS_GAME_THREAD ? 1u : 0xffu,&function);
}
struct Batch;
struct BatchJob { Batch* batch; satis_task_item item; OwnedCallback* callback; };
struct Batch {
    std::atomic<std::uint32_t> remaining;
    BatchJob* jobs;
    OwnedCallback* continuation;
    satis_task_lane lane;
};
void batch_run(void* opaque) {
    const auto* job=static_cast<BatchJob*>(opaque);
    job->item.run(job->item.context);
}
void batch_dispose(void* opaque) {
    auto* job=static_cast<BatchJob*>(opaque);
    Batch* batch=job->batch;
    if(job->item.dispose)job->item.dispose(job->item.context);
    if(batch->remaining.fetch_sub(1,std::memory_order_acq_rel)==1) {
        auto* continuation=batch->continuation;
        const auto lane=batch->lane;
        delete[] batch->jobs;
        delete batch;
        submit_owned(continuation,lane);
    }
}

std::uint32_t tid() { return static_cast<std::uint32_t>(syscall(SYS_gettid)); }
std::uint64_t work(std::uint32_t index, std::uint32_t iterations) {
    std::uint64_t x = 0x9e3779b97f4a7c15ULL ^ index;
    for (std::uint32_t i=0; i<iterations; ++i) {
        x ^= x >> 12; x ^= x << 25; x ^= x >> 27;
        x *= 0x2545f4914f6cdd1dULL;
    }
    return x;
}
struct Probe {
    std::atomic<std::uint64_t> submitted{0}, completed{0}, destroyed{0};
    std::atomic<std::uint64_t> active{0}, maximum{0}, checksum{0};
    std::atomic<std::uint64_t> wrong_thread{0}, failures{0};
    std::atomic<std::uint64_t> continuation_calls{0}, continuation_disposals{0}, continuation_wrong_thread{0};
    std::atomic<std::uint64_t> expected{0};
    std::uint32_t iterations{}, tasks{};
    std::atomic<bool> started{false};
} probe;
struct ProbeJob { std::uint32_t index; };
void probe_run(void* opaque) {
    const auto job=static_cast<ProbeJob*>(opaque);
    if (tid()==*game_thread_id) probe.wrong_thread.fetch_add(1);
    const auto active=probe.active.fetch_add(1)+1;
    auto maximum=probe.maximum.load();
    while(maximum<active && !probe.maximum.compare_exchange_weak(maximum,active)) {}
    const auto value=work(job->index,probe.iterations);
    probe.checksum.fetch_xor(value);
    probe.active.fetch_sub(1);
    probe.completed.fetch_add(1,std::memory_order_release);
}
void probe_commit(void*) {
    if(tid()!=*game_thread_id)probe.continuation_wrong_thread.fetch_add(1);
    if(probe.completed.load(std::memory_order_acquire)!=probe.tasks ||
       probe.destroyed.load(std::memory_order_acquire)!=probe.tasks ||
       probe.checksum.load()!=probe.expected.load())probe.failures.fetch_add(1);
    probe.continuation_calls.fetch_add(1);
}
void probe_commit_dispose(void*) {probe.continuation_disposals.fetch_add(1);}
void probe_dispose(void* opaque) {
    delete static_cast<ProbeJob*>(opaque);
    probe.destroyed.fetch_add(1,std::memory_order_release);
}
}

extern "C" int satis_task_bridge_init() {
    std::lock_guard<std::mutex> lock(init_mutex);
    if(initialized.load(std::memory_order_acquire)) return 0;
    core_handle=dlopen("libFactoryServer-Core-Linux-Shipping.so",RTLD_NOW|RTLD_NOLOAD);
    if(!core_handle) {failure="The official Core module is not loaded";return -1;}
    void* symbol=dlsym(core_handle,"_Z9AsyncTaskN13ENamedThreads4TypeE15TUniqueFunctionIFvvEE");
    Dl_info info{};
    if(!symbol || !dladdr(symbol,&info) ||
       reinterpret_cast<std::uintptr_t>(symbol)-reinterpret_cast<std::uintptr_t>(info.dli_fbase)!=0x47b060) {
        failure="Unsupported Core AsyncTask symbol/offset";return -2;
    }
    BuildCheck build{reinterpret_cast<std::uintptr_t>(info.dli_fbase)};
    dl_iterate_phdr(check_core_build,&build);
    if(!build.matched) {failure="Unsupported Core GNU build-id";return -5;}
    const unsigned char prefix[]={0x55,0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x53,0x50,0x49,0x89,0xf7,0x89,0xfd};
    if(std::memcmp(symbol,prefix,sizeof(prefix))) {
        failure="Unsupported Core AsyncTask instructions";return -3;
    }
    const auto base=reinterpret_cast<std::uintptr_t>(info.dli_fbase);
    struct LocalEntry {std::uintptr_t offset; const unsigned char prefix[12]; std::size_t length;};
    const LocalEntry entries[]={
        {0x4ac890,{0x55,0x41,0x57,0x41,0x56,0x41,0x54,0x53},8},
        {0x4acb90,{0x41,0x57,0x41,0x56,0x53,0x48,0x89,0xfb},8},
        {0x4aa520,{0x41,0x57,0x41,0x56,0x41,0x54,0x53,0x50},8},
        {0x4a38e0,{0x55,0x41,0x57,0x41,0x56,0x41,0x54,0x53},8},
        {0x4b2410,{0x55,0x41,0x57,0x41,0x56,0x41,0x55,0x41,0x54,0x53,0x50},11}
    };
    for(const auto& entry:entries) {
        if(std::memcmp(reinterpret_cast<void*>(base+entry.offset),entry.prefix,entry.length)) {
            failure="Unsupported Core completion entry instructions";return -6;
        }
    }
    event_allocate=reinterpret_cast<EventAllocate>(base+0x4ac890);
    event_construct=reinterpret_cast<EventConstruct>(base+0x4acb90);
    task_unlock=reinterpret_cast<TaskUnlock>(base+0x4aa520);
    create_with_prerequisite=reinterpret_cast<CreateWithPrerequisite>(base+0x4a38e0);
    task_add_prerequisites=reinterpret_cast<TaskAddPrerequisites>(base+0x4b2410);
    game_thread_id=static_cast<std::uint32_t*>(dlsym(RTLD_DEFAULT,"GGameThreadId"));
    if(!game_thread_id || !*game_thread_id) {failure="Game thread is not initialized";return -4;}
    async_task=reinterpret_cast<AsyncTaskFunction>(symbol);

    engine_handle = dlopen("libFactoryServer-Engine-Linux-Shipping.so", RTLD_NOW | RTLD_NOLOAD);
    EngineFinder eng_finder{};
    if (engine_handle) {
        void* sym = dlsym(engine_handle, "AccumulateWithAdditiveScale");
        Dl_info eng_info{};
        if (sym && dladdr(sym, &eng_info)) {
            eng_finder.base = reinterpret_cast<std::uintptr_t>(eng_info.dli_fbase);
        }
    }
    dl_iterate_phdr(check_engine_build, &eng_finder);
    if (eng_finder.matched) {
        const unsigned char nested_prefix[] = {0x55, 0x41, 0x57, 0x41, 0x56, 0x41, 0x54, 0x53};
        if (!std::memcmp(reinterpret_cast<void*>(eng_finder.base + 0x16b4480), nested_prefix, sizeof(nested_prefix))) {
            task_add_nested = reinterpret_cast<TaskAddNested>(eng_finder.base + 0x16b4480);
        }
    }

    gc_active=static_cast<const bool*>(dlsym(RTLD_DEFAULT,"GIsGarbageCollecting"));
    saving_package=static_cast<const bool*>(dlsym(RTLD_DEFAULT,"GIsSavingPackage"));
    // Core compatibility alone cannot validate the actor member offsets below.
    void* factory=dlopen("libFactoryServer-FactoryGame-Linux-Shipping.so",RTLD_NOW|RTLD_NOLOAD);
    if(factory) {
        void* tick=dlsym(factory,"_ZN20FFactoryTickFunction11ExecuteTickEf10ELevelTickN13ENamedThreads4TypeERK12TRefCountPtrI14FBaseGraphTaskE");
        Dl_info factory_info{};
        if(tick && dladdr(tick,&factory_info)) {
            const unsigned char factory_id[]={0x7d,0xd1,0x1e,0xd7,0xc1,0xcc,0x5e,0x43};
            BuildCheck check{reinterpret_cast<std::uintptr_t>(factory_info.dli_fbase),false,factory_id};
            dl_iterate_phdr(check_core_build,&check);
            factory_layout_supported=check.matched &&
                reinterpret_cast<std::uintptr_t>(tick)-check.base==0x180cba0;
        }
    }

    const char* env_async = std::getenv("SATIS_ASYNC_TICK");
    if(env_async && (!std::strcmp(env_async, "1") || !std::strcmp(env_async, "true"))) {
        async_tick_enabled.store(true, std::memory_order_release);
    }

    failure="";
    initialized.store(true,std::memory_order_release);
    return 0;
}
extern "C" const char* satis_task_bridge_error() {return failure.load(std::memory_order_acquire);}
extern "C" int satis_task_enqueue(satis_task_callback run,void* context,
                                   satis_task_dispose dispose,satis_task_lane lane) {
    if(!initialized.load(std::memory_order_acquire)) return -1;
    if(!run || (lane!=SATIS_GAME_THREAD && lane!=SATIS_WORKER_THREAD)) return -2;
    auto* object=new(std::nothrow) OwnedCallback(run,context,dispose);
    if(!object) return -3;
    submit_owned(object,lane);
    // The callee owns object after move; stack storage is no longer referenced.
    return 0;
}
extern "C" int satis_task_batch(const satis_task_item* items,std::uint32_t count,
    satis_task_callback continuation,void* context,satis_task_dispose dispose,satis_task_lane lane) {
    if(!initialized.load(std::memory_order_acquire))return -1;
    if(!items || !continuation || count==0 || count>65536 ||
       (lane!=SATIS_GAME_THREAD && lane!=SATIS_WORKER_THREAD))return -2;
    for(std::uint32_t i=0;i<count;++i)if(!items[i].run)return -2;
    auto* batch=new(std::nothrow) Batch{count,nullptr,nullptr,lane};
    if(!batch)return -3;
    batch->jobs=new(std::nothrow) BatchJob[count];
    batch->continuation=new(std::nothrow) OwnedCallback(continuation,context,dispose);
    if(!batch->jobs || !batch->continuation) {
        delete[] batch->jobs;
        if(batch->continuation)batch->continuation->abandon();
        delete batch;return -3;
    }
    std::uint32_t prepared=0;
    for(;prepared<count;++prepared) {
        batch->jobs[prepared]={batch,items[prepared],nullptr};
        auto* callback=new(std::nothrow) OwnedCallback(batch_run,&batch->jobs[prepared],batch_dispose);
        if(!callback)break;
        batch->jobs[prepared].callback=callback;
    }
    if(prepared!=count) {
        for(std::uint32_t i=0;i<prepared;++i)batch->jobs[i].callback->abandon();
        batch->continuation->abandon();delete[] batch->jobs;delete batch;return -3;
    }
    // The last job can finish synchronously or on another core. Capture each
    // pointer before dispatch and never access batch after the last dispatch.
    for(std::uint32_t i=0;i<count;++i) {
        auto* callback=batch->jobs[i].callback;
        submit_owned(callback,SATIS_WORKER_THREAD);
    }
    return 0;
}
extern "C" std::uint64_t satis_task_outstanding() {return OwnedCallback::outstanding.load(std::memory_order_acquire);}
extern "C" int satis_task_quiescent() {return satis_task_outstanding()==0;}
extern "C" int satis_task_probe_start(std::uint32_t tasks,std::uint32_t iterations) {
    if(!initialized.load(std::memory_order_acquire)) return -1;
    if(tasks<2 || tasks>256 || iterations<1000 || iterations>2000000) return -2;
    bool expected=false;
    if(!probe.started.compare_exchange_strong(expected,true)) return -3;
    probe.iterations=iterations;
    probe.tasks=tasks;
    satis_task_item items[256];
    for(std::uint32_t i=0;i<tasks;++i) {
        probe.expected.fetch_xor(work(i,iterations));
        auto* job=new(std::nothrow) ProbeJob{i};
        if(!job) {
            for(std::uint32_t j=0;j<i;++j)delete static_cast<ProbeJob*>(items[j].context);
            probe.failures.fetch_add(1);return -4;
        }
        items[i]={probe_run,job,probe_dispose};
    }
    const int result=satis_task_batch(items,tasks,probe_commit,nullptr,probe_commit_dispose,SATIS_GAME_THREAD);
    if(result) {
        for(std::uint32_t i=0;i<tasks;++i)delete static_cast<ProbeJob*>(items[i].context);
        probe.failures.fetch_add(1);return result;
    }
    probe.submitted.store(tasks);
    return 0;
}
extern "C" int satis_task_probe_read(satis_task_probe_result* r) {
    if(!r) return -1;
    r->submitted=probe.submitted.load();r->completed=probe.completed.load(std::memory_order_acquire);
    r->destroyed=probe.destroyed.load(std::memory_order_acquire);r->max_concurrency=probe.maximum.load();
    r->checksum=probe.checksum.load();r->expected_checksum=probe.expected.load();
    r->wrong_thread=probe.wrong_thread.load();r->failures=probe.failures.load();
    r->continuation_calls=probe.continuation_calls.load();
    r->continuation_disposals=probe.continuation_disposals.load();
    r->continuation_wrong_thread=probe.continuation_wrong_thread.load();
    return 0;
}

extern "C" satis_completion* satis_completion_create() {
    if(!initialized.load(std::memory_order_acquire))return nullptr;
    auto* result=new(std::nothrow) satis_completion{};
    if(!result)return nullptr;
    result->event=event_allocate(168);
    if(!result->event){delete result;return nullptr;}
    event_construct(result->event);
    // Constructor owns one internal ref; add the external owning handle ref.
    // Same sequence as original AnyTaskCompleted at 0x484ff9..0x48501e.
    task_add_ref(result->event);
    return result;
}
extern "C" int satis_completion_then(satis_completion* completion,satis_task_callback run,
    void* context,satis_task_dispose dispose,satis_task_lane lane) {
    if(!initialized.load(std::memory_order_acquire))return -1;
    if(!completion || !run || (lane!=SATIS_GAME_THREAD && lane!=SATIS_WORKER_THREAD))return -2;
    auto* object=new(std::nothrow) OwnedCallback(run,context,dispose);
    if(!object)return -3;
    object->publish();
    UniqueFunction function{};
    function.call=call_owned;function.heap=object;
    void* returned{};
    create_with_prerequisite(&returned,&function,&completion->event,lane==SATIS_GAME_THREAD?1u:255u);
    // The returned owning task ref is not needed by this fire-and-observe API.
    // Core retains its internal ref and the prerequisite retains its edge.
    task_release(returned);
    return 0;
}
extern "C" int satis_completion_signal(satis_completion* completion) {
    if(!initialized.load(std::memory_order_acquire))return -1;
    if(!completion)return -2;
    bool expected=false;
    if(!completion->signaled.compare_exchange_strong(expected,true))return -3;
    bool wake=true;
    task_unlock(completion->event,&wake);
    return 0;
}
extern "C" void satis_completion_release(satis_completion* completion) {
    if(!completion)return;
    task_release(completion->event);
    delete completion;
}

extern "C" int satis_completion_bind_to_task(satis_completion* completion, void* engine_task) {
    if(!initialized.load(std::memory_order_acquire)) return -1;
    if(!completion || !completion->event || !engine_task) return -2;

    const auto* task_bytes = static_cast<const unsigned char*>(engine_task);
    // Task[0x1c] is the task state / type byte in UE::Tasks::Private::FTaskBase.
    // 0x2 indicates a named thread task (e.g. GameThread), which accepts prerequisites.
    // Any other value indicates an executing task function where AddPrerequisites asserts,
    // and AddNested must be used instead.
    if(task_bytes[0x1c] == 2) {
        bool ok = task_add_prerequisites(engine_task, completion->event);
        return ok ? 0 : 1;
    }

    if(task_add_nested) {
        task_add_nested(engine_task, completion->event);
        return 0;
    }

    // AddPrerequisites on an executing task asserts. Never substitute it for
    // a missing/unsupported AddNested implementation.
    return -3;
}

extern "C" void satis_async_set_enabled(int enabled) {
    async_tick_enabled.store(enabled != 0, std::memory_order_release);
}
extern "C" int satis_async_is_enabled() {
    return async_tick_enabled.load(std::memory_order_acquire) ? 1 : 0;
}
extern "C" std::uint64_t satis_async_tick_count() {
    return intercepted_ticks.load(std::memory_order_acquire);
}
extern "C" std::uint64_t satis_async_dispatch_count() {
    return async_dispatches.load(std::memory_order_acquire);
}
extern "C" std::uint64_t satis_async_sync_fallback_count() {
    return sync_fallbacks.load(std::memory_order_acquire);
}
extern "C" std::uint64_t satis_async_completed_count() {
    return completed_async_ticks.load(std::memory_order_acquire);
}

namespace {
struct AsyncTickContext {
    ExecuteTickSignature orig;
    void* this_ptr;
    float delta_time;
    std::uint32_t level_tick;
    std::uint32_t current_thread;
    void* completion_event;
    satis_completion* completion_handle;
};

void async_tick_runner(void* opaque) {
    auto* ctx = static_cast<AsyncTickContext*>(opaque);

    void* completion_ptr = ctx->completion_event;
    // This call now runs on an AnyThread worker, not the submitting named thread.
    ctx->orig(ctx->this_ptr, ctx->delta_time, ctx->level_tick, 255u, &completion_ptr);
    completed_async_ticks.fetch_add(1, std::memory_order_release);

    if(ctx->completion_handle) {
        satis_completion_signal(ctx->completion_handle);
        satis_completion_release(ctx->completion_handle);
    }
    delete ctx;
}
}

extern "C" void _ZN20FFactoryTickFunction11ExecuteTickEf10ELevelTickN13ENamedThreads4TypeERK12TRefCountPtrI14FBaseGraphTaskE(
    void* this_ptr, float delta_time, std::uint32_t level_tick, std::uint32_t current_thread, void* const* completion_event_ref
) {
    intercepted_ticks.fetch_add(1, std::memory_order_relaxed);

    ExecuteTickSignature orig = original_execute_tick.load(std::memory_order_acquire);
    if(!orig) {
        orig = reinterpret_cast<ExecuteTickSignature>(
            dlsym(RTLD_NEXT, "_ZN20FFactoryTickFunction11ExecuteTickEf10ELevelTickN13ENamedThreads4TypeERK12TRefCountPtrI14FBaseGraphTaskE")
        );
        if(!orig) {
            void* fg = dlopen("libFactoryServer-FactoryGame-Linux-Shipping.so", RTLD_NOW | RTLD_NOLOAD);
            if(fg) {
                orig = reinterpret_cast<ExecuteTickSignature>(
                    dlsym(fg, "_ZN20FFactoryTickFunction11ExecuteTickEf10ELevelTickN13ENamedThreads4TypeERK12TRefCountPtrI14FBaseGraphTaskE")
                );
            }
        }
        original_execute_tick.store(orig, std::memory_order_release);
    }

    if(!initialized.load(std::memory_order_acquire)) {
        satis_task_bridge_init();
    }

    if(!async_tick_enabled.load(std::memory_order_acquire) ||
       !initialized.load(std::memory_order_acquire) || !orig) {
        if(orig) {
            orig(this_ptr, delta_time, level_tick, current_thread, completion_event_ref);
        }
        return;
    }

    auto run_synchronously = [&] {
        sync_fallbacks.fetch_add(1, std::memory_order_relaxed);
        orig(this_ptr, delta_time, level_tick, current_thread, completion_event_ref);
    };

    // Only inspect the build-specific actor layout from its owning thread.
    // GC/save flags are an entry guard, not a cross-frame lifetime lock.
    if(!factory_layout_supported || !gc_active || !saving_package ||
       tid()!=*game_thread_id || *gc_active || *saving_package) {
        run_synchronously();
        return;
    }

    // Check if conveyor migration or actor spawning is pending.
    // In AFGBuildableSubsystem, offset 0x400 holds mConveyorGroupsToMigrate.Num().
    // If migration is needed, MigrateConveyorGroupToChainActor will spawn actors via
    // UWorld::SpawnActor. To guarantee thread-safety and preserve UWorld actor integrity,
    // execute this tick synchronously. Custom factory handlers (+0x948) execute
    // before the first ParallelFor and may contain arbitrary Game Thread work.
    void* subsystem = this_ptr ? *reinterpret_cast<void**>(static_cast<char*>(this_ptr) + 0x28) : nullptr;
    if (!subsystem) {
        run_synchronously();
        return;
    }
    const auto* bytes=static_cast<const char*>(subsystem);
    const auto flags=*reinterpret_cast<const std::uint32_t*>(bytes+8);
    // Match the original UObject validity gate before reading larger offsets.
    if((flags & 0x40000000u) ||
       *reinterpret_cast<const std::int32_t*>(bytes+0x400)!=0 ||
       *reinterpret_cast<const std::int32_t*>(bytes+0x948)!=0) {
        run_synchronously();
        return;
    }

    void* engine_task = (completion_event_ref && *completion_event_ref) ? *completion_event_ref : nullptr;
    // Offloading without completion gating can overlap the next frame, save,
    // actor teardown or GC. Reject asynchronous dispatch if any setup step fails.
    if(!engine_task) {
        run_synchronously();
        return;
    }
    satis_completion* comp=satis_completion_create();
    if(!comp) {
        run_synchronously();
        return;
    }
    if(satis_completion_bind_to_task(comp,engine_task)!=0) {
        satis_completion_signal(comp);
        satis_completion_release(comp);
        run_synchronously();
        return;
    }

    auto* ctx = new(std::nothrow) AsyncTickContext{
        orig, this_ptr, delta_time, level_tick, current_thread, engine_task, comp
    };
    if(!ctx) {
        if(comp) {
            satis_completion_signal(comp);
            satis_completion_release(comp);
        }
        run_synchronously();
        return;
    }

    async_dispatches.fetch_add(1, std::memory_order_relaxed);
    int queued = satis_task_enqueue(async_tick_runner, ctx, nullptr, SATIS_WORKER_THREAD);
    if(queued != 0) {
        async_dispatches.fetch_sub(1, std::memory_order_relaxed);
        delete ctx;
        // The completion was bound, so it must be released even on enqueue failure.
        orig(this_ptr, delta_time, level_tick, current_thread, completion_event_ref);
        satis_completion_signal(comp);
        satis_completion_release(comp);
        sync_fallbacks.fetch_add(1,std::memory_order_relaxed);
    }
}

namespace {
struct CompletionProbe {
    std::atomic<bool> started{false};
    std::atomic<std::uint64_t> submitted{0},completed{0},destroyed{0};
    std::atomic<std::uint64_t> watcher_calls{0},watcher_disposals{0},early_watcher{0},wrong_thread{0};
    std::atomic<std::uint64_t> checksum{0},expected{0},signal_calls{0},failures{0},active{0},maximum{0};
    std::uint32_t tasks{},iterations{};
    satis_completion* gate{};
} completion_probe;
void completion_job(void* opaque) {
    const auto i=*static_cast<std::uint32_t*>(opaque);
    if(tid()==*game_thread_id)completion_probe.wrong_thread.fetch_add(1);
    const auto active=completion_probe.active.fetch_add(1)+1;
    auto maximum=completion_probe.maximum.load();
    while(maximum<active && !completion_probe.maximum.compare_exchange_weak(maximum,active)){}
    completion_probe.checksum.fetch_xor(work(i,completion_probe.iterations));
    completion_probe.active.fetch_sub(1);
    completion_probe.completed.fetch_add(1,std::memory_order_release);
}
void completion_job_dispose(void* opaque) {
    delete static_cast<std::uint32_t*>(opaque);
    completion_probe.destroyed.fetch_add(1,std::memory_order_release);
}
void completion_watcher(void*) {
    if(tid()!=*game_thread_id)completion_probe.wrong_thread.fetch_add(1);
    if(completion_probe.signal_calls.load(std::memory_order_acquire)!=1 ||
       completion_probe.completed.load(std::memory_order_acquire)!=completion_probe.tasks ||
       completion_probe.destroyed.load(std::memory_order_acquire)!=completion_probe.tasks)
        completion_probe.early_watcher.fetch_add(1);
    completion_probe.watcher_calls.fetch_add(1,std::memory_order_release);
}
void completion_watcher_dispose(void*) {completion_probe.watcher_disposals.fetch_add(1,std::memory_order_release);}
void completion_publish(void*) {
    completion_probe.signal_calls.fetch_add(1,std::memory_order_release);
    if(satis_completion_signal(completion_probe.gate))completion_probe.failures.fetch_add(1);
    if(satis_completion_signal(completion_probe.gate)!=-3)completion_probe.failures.fetch_add(1);
    satis_completion_release(completion_probe.gate);
    completion_probe.gate=nullptr;
}
}
extern "C" int satis_completion_probe_start(std::uint32_t tasks,std::uint32_t iterations) {
    if(!initialized.load(std::memory_order_acquire))return -1;
    if(tasks<2 || tasks>256 || iterations<1000 || iterations>2000000)return -2;
    bool expected=false;
    if(!completion_probe.started.compare_exchange_strong(expected,true))return -3;
    completion_probe.tasks=tasks;completion_probe.iterations=iterations;
    satis_task_item items[256];
    for(std::uint32_t i=0;i<tasks;++i) {
        auto* job=new(std::nothrow) std::uint32_t(i);
        if(!job){for(std::uint32_t j=0;j<i;++j)delete static_cast<std::uint32_t*>(items[j].context);return -4;}
        items[i]={completion_job,job,completion_job_dispose};
        completion_probe.expected.fetch_xor(work(i,iterations));
    }
    completion_probe.gate=satis_completion_create();
    if(!completion_probe.gate) {
        for(std::uint32_t i=0;i<tasks;++i)delete static_cast<std::uint32_t*>(items[i].context);
        return -4;
    }
    const int watch=satis_completion_then(completion_probe.gate,completion_watcher,nullptr,completion_watcher_dispose,SATIS_GAME_THREAD);
    if(watch) {
        satis_completion_signal(completion_probe.gate);satis_completion_release(completion_probe.gate);
        for(std::uint32_t i=0;i<tasks;++i)delete static_cast<std::uint32_t*>(items[i].context);
        return watch;
    }
    completion_probe.submitted.store(tasks);
    const int result=satis_task_batch(items,tasks,completion_publish,nullptr,nullptr,SATIS_GAME_THREAD);
    if(result) {
        completion_probe.failures.fetch_add(1);
        for(std::uint32_t i=0;i<tasks;++i)delete static_cast<std::uint32_t*>(items[i].context);
        completion_publish(nullptr);
    }
    return result;
}
extern "C" int satis_completion_probe_read(satis_completion_probe_result* r) {
    if(!r)return -1;
    r->submitted=completion_probe.submitted.load();r->completed=completion_probe.completed.load();
    r->destroyed=completion_probe.destroyed.load();r->watcher_calls=completion_probe.watcher_calls.load();
    r->watcher_disposals=completion_probe.watcher_disposals.load();r->early_watcher=completion_probe.early_watcher.load();
    r->wrong_thread=completion_probe.wrong_thread.load();r->checksum=completion_probe.checksum.load();
    r->expected_checksum=completion_probe.expected.load();r->signal_calls=completion_probe.signal_calls.load();
    r->failures=completion_probe.failures.load();r->max_concurrency=completion_probe.maximum.load();
    return 0;
}

