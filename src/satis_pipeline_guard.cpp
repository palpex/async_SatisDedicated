// Steam build 25782485: pipeline attachments may have connected components
// whose owners are not pipelines. The stock optional merge path casts those
// owners to null, then calls Execute_CanDismantle(null), asserting on the Game
// Thread. Keep the stock base dismantle path for this case. No async scheduling.
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <dlfcn.h>
#include <link.h>
#include <mutex>
#include <sys/syscall.h>
#include <unistd.h>

namespace {
constexpr const char* attachment_symbol =
    "_ZN30AFGBuildablePipelineAttachment24Dismantle_ImplementationEv";
constexpr std::uintptr_t attachment_offset = 0x17166f0;
constexpr std::uintptr_t enumerate_offset = 0x1730110;
using Dismantle = void (*)(void*);
using GetClass = void* (*)();
using Free = void (*)(void*);

// The actual GetComponents<T, TInlineAllocator<4>> closure in this build.
struct InlineComponents {
    void* inline_data[4]{};
    void** heap{};
    std::int32_t count{}, capacity{4};
    void** data() { return heap ? heap : inline_data; }
};
static_assert(sizeof(InlineComponents) == 48);
static_assert(offsetof(InlineComponents, heap) == 32);
static_assert(offsetof(InlineComponents, count) == 40);
using Enumerate = void (*)(void*, void* const*, bool, InlineComponents*);
struct Bindings {
    Dismantle original{}, base{};
    GetClass pipe_component_class{}, pipeline_class{};
    Enumerate enumerate{};
    Free free_memory{};
    const std::uint64_t* name0{};
    const std::uint64_t* name1{};
    const std::uint32_t* game_thread_id{};
} bindings;
std::atomic<bool> initialized{false};
std::mutex init_mutex;
bool supported{};
const char* status = "not initialized";
std::atomic<std::uint64_t> checks{0}, fallbacks{0};
std::atomic<bool> live_mode{false};
std::atomic<Dismantle> live_original{nullptr};

template<class T> T read_at(const void* object, std::size_t offset) {
    T value{};
    std::memcpy(&value, static_cast<const char*>(object) + offset, sizeof(value));
    return value;
}

struct BuildCheck { std::uintptr_t base; bool matched{}; };
int check_build(dl_phdr_info* image, std::size_t, void* opaque) {
    auto& check = *static_cast<BuildCheck*>(opaque);
    if (image->dlpi_addr != check.base) return 0;
    constexpr unsigned char expected[] = {0x7d,0xd1,0x1e,0xd7,0xc1,0xcc,0x5e,0x43};
    for (std::size_t i = 0; i < image->dlpi_phnum; ++i) {
        const auto& ph = image->dlpi_phdr[i];
        if (ph.p_type != PT_NOTE) continue;
        const auto* p = reinterpret_cast<const unsigned char*>(check.base + ph.p_vaddr);
        const auto* end = p + ph.p_memsz;
        while (static_cast<std::size_t>(end - p) >= sizeof(ElfW(Nhdr))) {
            ElfW(Nhdr) note{};
            std::memcpy(&note, p, sizeof(note));
            p += sizeof(note);
            const std::size_t name_size = (std::size_t(note.n_namesz) + 3) & ~std::size_t(3);
            const std::size_t desc_size = (std::size_t(note.n_descsz) + 3) & ~std::size_t(3);
            if (name_size > static_cast<std::size_t>(end - p) ||
                desc_size > static_cast<std::size_t>(end - p) - name_size) break;
            if (note.n_type == NT_GNU_BUILD_ID && note.n_namesz == 4 &&
                note.n_descsz == sizeof(expected) && !std::memcmp(p, "GNU", 4) &&
                !std::memcmp(p + name_size, expected, sizeof(expected))) check.matched = true;
            p += name_size + desc_size;
        }
    }
    return 1;
}

void initialize() {
    if (initialized.load(std::memory_order_acquire)) return;
    std::lock_guard<std::mutex> lock(init_mutex);
    if (initialized.load(std::memory_order_relaxed)) return;
    void* factory = dlopen("libFactoryServer-FactoryGame-Linux-Shipping.so", RTLD_NOW | RTLD_NOLOAD);
    if (!factory) { status = "FactoryGame is not loaded"; return; }
    bindings.original = reinterpret_cast<Dismantle>(dlsym(factory, attachment_symbol));
    Dl_info info{};
    if (!bindings.original || !dladdr(reinterpret_cast<void*>(bindings.original), &info)) {
        status = "attachment entry is unavailable";
        dlclose(factory);
        return;
    }
    const auto base = reinterpret_cast<std::uintptr_t>(info.dli_fbase);
    BuildCheck build{base};
    dl_iterate_phdr(check_build, &build);
    const unsigned char enumerate_prefix[] = {
        0x55,0x41,0x57,0x41,0x56,0x41,0x54,0x53,0x48,0x83,0xec,0x20,0x48,0x89,0xcb
    };
    // Validate the specific owner-cast/null-call path as well as module identity.
    const unsigned char cast_prefix[] = {0x4d,0x85,0xff,0x74,0x20};
    if (!build.matched || reinterpret_cast<std::uintptr_t>(bindings.original) - base != attachment_offset ||
        std::memcmp(reinterpret_cast<void*>(base + enumerate_offset), enumerate_prefix, sizeof(enumerate_prefix)) ||
        std::memcmp(reinterpret_cast<void*>(base + 0x17168ba), cast_prefix, sizeof(cast_prefix))) {
        status = "unsupported FactoryGame build or instructions";
    } else {
        bindings.base = reinterpret_cast<Dismantle>(dlsym(factory,
            "_ZN19AFGBuildableFactory24Dismantle_ImplementationEv"));
        bindings.pipe_component_class = reinterpret_cast<GetClass>(dlsym(factory,
            "_Z60Z_Construct_UClass_UFGPipeConnectionComponentBase_NoRegisterv"));
        bindings.pipeline_class = reinterpret_cast<GetClass>(dlsym(factory,
            "_Z50Z_Construct_UClass_AFGBuildablePipeline_NoRegisterv"));
        bindings.name0 = static_cast<const std::uint64_t*>(dlsym(factory,
            "_ZN25AFGPipeAttachmentHologram12mConnection0E"));
        bindings.name1 = static_cast<const std::uint64_t*>(dlsym(factory,
            "_ZN25AFGPipeAttachmentHologram12mConnection1E"));
        bindings.free_memory = reinterpret_cast<Free>(dlsym(RTLD_DEFAULT, "_ZN7FMemory4FreeEPv"));
        bindings.game_thread_id = static_cast<const std::uint32_t*>(dlsym(RTLD_DEFAULT, "GGameThreadId"));
        bindings.enumerate = reinterpret_cast<Enumerate>(base + enumerate_offset);
        supported = bindings.base && bindings.pipe_component_class && bindings.pipeline_class &&
            bindings.name0 && bindings.name1 && bindings.free_memory && bindings.game_thread_id;
        status = supported ? "ready (synchronous pipeline dismantle guard)" : "required engine binding missing";
    }
    dlclose(factory);
    initialized.store(true, std::memory_order_release);
    std::fprintf(stderr, "[satis_pipeline_guard] %s\n", status);
}

bool is_pipeline(const void* owner, const void* pipeline_class) {
    if (!owner || !pipeline_class || (read_at<std::uint32_t>(owner, 8) & 0x40000000u)) return false;
    const auto* actual_class = read_at<const void*>(owner, 0x10);
    if (!actual_class) return false;
    // This is the same class-ancestry-array test used by the stock Cast<>.
    const auto index = read_at<std::int32_t>(pipeline_class, 0x38);
    if (index < 0 || index > read_at<std::int32_t>(actual_class, 0x38)) return false;
    const auto* ancestors = read_at<const void* const*>(actual_class, 0x30);
    return ancestors && ancestors[index] == static_cast<const char*>(pipeline_class) + 0x30;
}

bool needs_base_dismantle(void* attachment) {
    void* component_class = bindings.pipe_component_class();
    void* pipeline_class = bindings.pipeline_class();
    if (!component_class || !pipeline_class) return true;
    InlineComponents components;
    bindings.enumerate(attachment, &component_class, false, &components);
    void* peers[2]{};
    const bool valid_array = components.count >= 0 && components.count <= components.capacity &&
        components.count <= 65536 && (components.heap || components.count <= 4);
    bool malformed = !valid_array;
    if (valid_array) {
        for (std::int32_t i = 0; i < components.count; ++i) {
            const auto* component = components.data()[i];
            if (!component) { malformed = true; break; }
            const auto name = read_at<std::uint64_t>(component, 0x18);
            int which = name == read_at<std::uint64_t>(bindings.name0, 0) ? 0 :
                (name == read_at<std::uint64_t>(bindings.name1, 0) ? 1 : -1);
            if (which < 0) continue;
            auto* peer = read_at<void*>(component, 0x260);
            if (peer && (read_at<std::uint32_t>(peer, 8) & 0x40000000u)) peer = nullptr;
            peers[which] = peer; // Same last-name-wins behavior as the original.
        }
        if (peers[0] && peers[1]) {
            malformed = !is_pipeline(read_at<void*>(peers[0], 0x20), pipeline_class) ||
                !is_pipeline(read_at<void*>(peers[1], 0x20), pipeline_class);
        }
    }
    if (components.heap) bindings.free_memory(components.heap);
    return malformed;
}
} // namespace

extern "C" int satis_pipeline_guard_init() { initialize(); return supported ? 0 : -1; }
extern "C" const char* satis_pipeline_guard_status() { return status; }
extern "C" std::uint64_t satis_pipeline_guard_check_count() { return checks.load(); }
extern "C" std::uint64_t satis_pipeline_guard_fallback_count() { return fallbacks.load(); }
// Live installation supplies Frida's original trampoline before publishing the
// replacement. Startup LD_PRELOAD instead uses the unchanged module entry.
extern "C" void satis_pipeline_guard_set_original(void* original) {
    live_original.store(reinterpret_cast<Dismantle>(original), std::memory_order_release);
}
extern "C" void satis_pipeline_guard_prepare_live() { live_mode.store(true, std::memory_order_release); }
extern "C" void satis_pipeline_guard_cancel_live() {
    live_mode.store(false, std::memory_order_release);
    live_original.store(nullptr, std::memory_order_release);
}
extern "C" int satis_pipeline_guard_live_installed() {
    return live_mode.load(std::memory_order_acquire) && live_original.load(std::memory_order_acquire) ? 1 : 0;
}

extern "C" void _ZN30AFGBuildablePipelineAttachment24Dismantle_ImplementationEv(void* attachment) {
    initialize();
    const auto original = live_mode.load(std::memory_order_acquire) ?
        live_original.load(std::memory_order_acquire) : bindings.original;
    // Between publishing a live replacement and receiving its trampoline, use
    // the stock base path, never recursively call the patched entry point.
    if (!original) {
        if (supported && attachment) bindings.base(attachment);
        return;
    }
    if (!supported || !attachment ||
        static_cast<std::uint32_t>(syscall(SYS_gettid)) != *bindings.game_thread_id) {
        original(attachment);
        return;
    }
    checks.fetch_add(1, std::memory_order_relaxed);
    if (needs_base_dismantle(attachment)) {
        const auto count = fallbacks.fetch_add(1, std::memory_order_relaxed) + 1;
        if (count <= 8 || !(count & (count - 1))) {
            std::fprintf(stderr, "[satis_pipeline_guard] skipped invalid pipe auto-merge; base dismantle, count=%llu\n",
                static_cast<unsigned long long>(count));
        }
        bindings.base(attachment);
    } else {
        original(attachment);
    }
}
