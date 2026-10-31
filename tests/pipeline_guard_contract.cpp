// Exercise the actual standalone guard against synthetic Unreal ABI fixtures.
// This establishes branch/ownership behavior, not gameplay or binary compatibility.
#include "../src/satis_pipeline_guard.cpp"
#include <algorithm>
#include <array>
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <vector>
#include <sys/syscall.h>
#include <unistd.h>

namespace {
template<std::size_t N> struct alignas(16) Storage {
    std::array<unsigned char, N> bytes{};
    void* data() { return bytes.data(); }
    template<class T> void put(std::size_t offset, T value) {
        assert(offset + sizeof(value) <= N);
        std::memcpy(bytes.data() + offset, &value, sizeof(value));
    }
};
unsigned original_calls{}, base_calls{}, frees{}, enumerations{};
void* expected_actor{};
std::vector<void*> components;
Storage<0x80> pipe_class, wrong_class, component_class;
void* pipe_ancestry[1]{};
void* wrong_ancestry[1]{};
const std::uint64_t first_name=101, second_name=202;
std::uint32_t game_tid{};

void original(void* actor) { assert(actor==expected_actor); ++original_calls; }
void base(void* actor) { assert(actor==expected_actor); ++base_calls; }
void* get_component_class() { return component_class.data(); }
void* get_pipeline_class() { return pipe_class.data(); }
void enumerate(void* actor, void* const* klass, bool include_children,
               InlineComponents* result) {
    assert(actor==expected_actor && klass && *klass==component_class.data());
    assert(!include_children);
    ++enumerations;
    result->count=static_cast<std::int32_t>(components.size());
    if (components.size()>4) {
        result->capacity=result->count;
        result->heap=static_cast<void**>(std::malloc(components.size()*sizeof(void*)));
        assert(result->heap);
        std::memcpy(result->heap, components.data(), components.size()*sizeof(void*));
    } else {
        assert(!result->heap && result->capacity==4);
        std::copy(components.begin(),components.end(),result->inline_data);
    }
}
void free_memory(void* allocation) { assert(allocation); ++frees; std::free(allocation); }

struct Fixture {
    Storage<0x900> actor;
    Storage<0x280> c0,c1,p0,p1,extra;
    Storage<0x40> owner0,owner1;
    Fixture() {
        expected_actor=actor.data();
        owner0.put<void*>(0x10,pipe_class.data());
        owner1.put<void*>(0x10,pipe_class.data());
        p0.put<void*>(0x20,owner0.data());
        p1.put<void*>(0x20,owner1.data());
        c0.put<std::uint64_t>(0x18,first_name);
        c1.put<std::uint64_t>(0x18,second_name);
        c0.put<void*>(0x260,p0.data());
        c1.put<void*>(0x260,p1.data());
        components={c0.data(),c1.data()};
    }
};
using Hook=void(*)(void*);
Hook hook=&_ZN30AFGBuildablePipelineAttachment24Dismantle_ImplementationEv;
void check(Fixture& f, bool fallback, unsigned expected_frees=0) {
    original_calls=base_calls=frees=enumerations=0;
    hook(f.actor.data());
    assert(original_calls==unsigned(!fallback));
    assert(base_calls==unsigned(fallback));
    assert(enumerations==1 && frees==expected_frees);
}
}

int main() {
    pipe_ancestry[0]=static_cast<unsigned char*>(pipe_class.data())+0x30;
    wrong_ancestry[0]=static_cast<unsigned char*>(wrong_class.data())+0x30;
    pipe_class.put<void*>(0x30,pipe_ancestry);
    pipe_class.put<std::int32_t>(0x38,0);
    wrong_class.put<void*>(0x30,wrong_ancestry);
    wrong_class.put<std::int32_t>(0x38,0);
    game_tid=static_cast<std::uint32_t>(::syscall(SYS_gettid));
    bindings.original=original;
    bindings.base=base;
    bindings.pipe_component_class=get_component_class;
    bindings.pipeline_class=get_pipeline_class;
    bindings.enumerate=enumerate;
    bindings.free_memory=free_memory;
    bindings.name0=&first_name;
    bindings.name1=&second_name;
    bindings.game_thread_id=&game_tid;
    supported=true;
    initialized.store(true);

    { Fixture f; check(f,false); }
    { Fixture f; f.p0.put<void*>(0x20,nullptr); check(f,true); }
    { Fixture f; f.p1.put<void*>(0x20,nullptr); check(f,true); }
    { Fixture f; f.p0.put<void*>(0x20,nullptr); f.p1.put<void*>(0x20,nullptr); check(f,true); }
    { Fixture f; f.owner0.put<void*>(0x10,wrong_class.data()); check(f,true); }
    { Fixture f; f.owner1.put<void*>(0x10,wrong_class.data()); check(f,true); }
    { Fixture f; f.owner0.put<void*>(0x10,nullptr); check(f,true); }
    { Fixture f; f.owner1.put<std::uint32_t>(8,0x40000000u); check(f,true); }
    { Fixture f; f.owner0.put<void*>(0x10,wrong_class.data()); f.owner1.put<void*>(0x10,wrong_class.data()); check(f,true); }
    // Missing peer already skips the engine's optional merge. Preserve that path.
    { Fixture f; f.c0.put<void*>(0x260,nullptr); check(f,false); }
    { Fixture f; f.c1.put<void*>(0x260,nullptr); check(f,false); }
    { Fixture f; components={f.c0.data()}; check(f,false); }
    { Fixture f; components.clear(); check(f,false); }
    // The engine discards peers carrying this object flag before checking owners.
    { Fixture f; f.p0.put<std::uint32_t>(8,0x40000000u); f.p1.put<void*>(0x20,nullptr); check(f,false); }
    { Fixture f; f.p1.put<std::uint32_t>(8,0x40000000u); f.p0.put<void*>(0x20,nullptr); check(f,false); }
    // Unknown names must not replace either named connection.
    { Fixture f; f.extra.put<std::uint64_t>(0x18,303); components.push_back(f.extra.data()); check(f,false); }
    // Like the engine, use the final matching component, including a missing peer.
    { Fixture f; f.p0.put<void*>(0x20,nullptr); f.extra.put<std::uint64_t>(0x18,first_name); f.extra.put<void*>(0x260,f.p1.data()); components.push_back(f.extra.data()); check(f,false); }
    { Fixture f; f.extra.put<std::uint64_t>(0x18,first_name); components.push_back(f.extra.data()); check(f,false); }
    { Fixture f; f.extra.put<std::uint64_t>(0x18,second_name); f.extra.put<void*>(0x260,f.p0.data()); f.p0.put<void*>(0x20,nullptr); components.push_back(f.extra.data()); check(f,true); }
    // Derived pipeline types must pass the same ancestry-array test as Cast<>.
    {
        Fixture f;
        Storage<0x80> derived;
        void* ancestors[2]={static_cast<unsigned char*>(pipe_class.data())+0x30,
                            static_cast<unsigned char*>(derived.data())+0x30};
        derived.put<void*>(0x30,ancestors);
        derived.put<std::int32_t>(0x38,1);
        f.owner0.put<void*>(0x10,derived.data());
        check(f,false);
    }
    // An unsupported build or a non-Game-Thread call retains the original path.
    {
        Fixture f;
        f.p0.put<void*>(0x20,nullptr);
        original_calls=base_calls=enumerations=0;
        supported=false;
        hook(f.actor.data());
        supported=true;
        assert(original_calls==1 && base_calls==0 && enumerations==0);
        original_calls=0;
        ++game_tid;
        hook(f.actor.data());
        --game_tid;
        assert(original_calls==1 && base_calls==0 && enumerations==0);
    }
    // Live publication cannot recurse into the replaced module entry while the
    // trampoline is being handed over. It temporarily uses stock base dismantle.
    {
        Fixture f;
        original_calls=base_calls=enumerations=0;
        satis_pipeline_guard_prepare_live();
        assert(satis_pipeline_guard_live_installed()==0);
        hook(f.actor.data());
        assert(original_calls==0 && base_calls==1 && enumerations==0);
        satis_pipeline_guard_set_original(reinterpret_cast<void*>(original));
        assert(satis_pipeline_guard_live_installed()==1);
        check(f,false);
        f.p0.put<void*>(0x20,nullptr);
        check(f,true);
        // Failed live installation can reset its publication state for retry.
        satis_pipeline_guard_cancel_live();
        assert(satis_pipeline_guard_live_installed()==0);
        f.p0.put<void*>(0x20,f.owner0.data());
        check(f,false);
    }
    // Heap storage allocated by the engine helper must be released on both branches.
    { Fixture f; f.extra.put<std::uint64_t>(0x18,303); components.insert(components.end(),4,f.extra.data()); check(f,false,1); }
    { Fixture f; f.p1.put<void*>(0x20,nullptr); f.extra.put<std::uint64_t>(0x18,303); components.insert(components.end(),4,f.extra.data()); check(f,true,1); }
}
