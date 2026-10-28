// Exercise the actual hook using a synthetic ABI boundary, without proprietary
// binaries. These tests verify bridge control flow, not Unreal gameplay safety.
#include "../src/satis_task_bridge.cpp"
#include <cassert>
#include <vector>

namespace {
unsigned original_calls{}, binds{}, signals{};
std::uint32_t observed_lane{};
UniqueFunction queued_function{};
bool reject_binding{};
std::vector<void*> allocated_events;

void original_tick(void*,float,std::uint32_t,std::uint32_t lane,void* const*) {
    ++original_calls;
    observed_lane=lane;
}
void* allocate_event(std::size_t size) {
    void* result=new unsigned char[size]{};
    allocated_events.push_back(result);
    return result;
}
void construct_event(void* event) {
    // Mock engine retains a reference until this test's teardown.
    *reinterpret_cast<std::uint32_t*>(static_cast<char*>(event)+8)=2;
}
bool unlock_event(void*,bool*) { ++signals; return true; }
bool add_prerequisite(void*,void*) { ++binds; return !reject_binding; }
void add_nested(void*,void*) { ++binds; }
void enqueue(std::uint32_t lane,UniqueFunction* function) {
    assert(lane==255u);
    assert(!queued_function.heap);
    queued_function=*function;
    function->call=nullptr;
    function->heap=nullptr;
}
void drain() {
    auto* callback=static_cast<OwnedCallback*>(queued_function.heap);
    queued_function.call(callback);
    callback->destroy();
    queued_function={};
}
using Hook=ExecuteTickSignature;
Hook hook=&_ZN20FFactoryTickFunction11ExecuteTickEf10ELevelTickN13ENamedThreads4TypeERK12TRefCountPtrI14FBaseGraphTaskE;
}

int main() {
    alignas(16) unsigned char subsystem[0x960]{};
    alignas(16) unsigned char tick[0x30]{};
    alignas(16) unsigned char engine_task[0xa8]{};
    *reinterpret_cast<void**>(tick+0x28)=subsystem;
    void* completion=engine_task;
    std::uint32_t owner=tid();
    game_thread_id=&owner;
    bool inactive=false;
    gc_active=&inactive;
    saving_package=&inactive;
    incremental_reachability_pending=&inactive;
    factory_layout_supported=true;
    initialized.store(true);
    async_tick_enabled.store(true);
    original_execute_tick.store(original_tick);
    event_allocate=allocate_event;
    event_construct=construct_event;
    task_unlock=unlock_event;
    task_add_nested=add_nested;
    task_add_prerequisites=add_prerequisite;
    async_task=enqueue;

    // Missing completion: run inline instead of starting an untracked worker.
    hook(tick,0.03f,0,1,nullptr);
    assert(original_calls==1 && !queued_function.heap && signals==0);
    assert(satis_async_sync_fallback_count()==1);

    // Actor migration and extensible tick handlers retain their original lane.
    *reinterpret_cast<std::int32_t*>(subsystem+0x400)=1;
    hook(tick,0.03f,0,1,&completion);
    assert(original_calls==2 && !queued_function.heap && observed_lane==1);
    *reinterpret_cast<std::int32_t*>(subsystem+0x400)=0;
    *reinterpret_cast<std::int32_t*>(subsystem+0x948)=1;
    hook(tick,0.03f,0,1,&completion);
    assert(original_calls==3 && !queued_function.heap);
    *reinterpret_cast<std::int32_t*>(subsystem+0x948)=0;

    // A missing executing-task binder must never call AddPrerequisites.
    task_add_nested=nullptr;
    hook(tick,0.03f,0,1,&completion);
    assert(original_calls==4 && !queued_function.heap && binds==0 && signals==1);
    task_add_nested=add_nested;
    engine_task[0x1c]=2;
    reject_binding=true;
    hook(tick,0.03f,0,1,&completion);
    assert(original_calls==5 && !queued_function.heap && binds==1 && signals==2);
    reject_binding=false;
    engine_task[0x1c]=0;

    bool gc=true;
    gc_active=&gc;
    hook(tick,0.03f,0,1,&completion);
    assert(original_calls==6 && !queued_function.heap);
    gc_active=&inactive;
    bool incremental_pending=true;
    incremental_reachability_pending=&incremental_pending;
    hook(tick,0.03f,0,1,&completion);
    assert(original_calls==7 && !queued_function.heap);
    incremental_reachability_pending=&inactive;
    factory_layout_supported=false;
    hook(tick,0.03f,0,1,&completion);
    assert(original_calls==8 && !queued_function.heap);
    factory_layout_supported=true;

    // Eligible work returns before executing or signaling its completion gate.
    hook(tick,0.03f,0,1,&completion);
    assert(original_calls==8 && queued_function.heap && binds==2 && signals==2);
    assert(satis_async_dispatch_count()==1 && satis_async_completed_count()==0);
    drain();
    assert(original_calls==9 && observed_lane==255u && signals==3);
    assert(satis_async_completed_count()==1 && satis_task_outstanding()==0);
    assert(satis_async_sync_fallback_count()==8);

    // Verify buildable mutation guard and synchronization
    assert(satis_async_active_mutations() == 0);
    assert(satis_async_mutation_wait_count() == 0);

    // When an active mutation is in progress, ExecuteTick must fall back synchronously.
    {
        ScopedBuildableMutation mutation;
        assert(satis_async_active_mutations() == 1);
        hook(tick, 0.03f, 0, 1, &completion);
        assert(original_calls == 10 && !queued_function.heap && observed_lane == 1);
        assert(satis_async_sync_fallback_count() == 9);
    }
    assert(satis_async_active_mutations() == 0);

    // Verify calling all hooks directly
    _ZN21AFGBuildableSubsystem12AddBuildableEP12AFGBuildable(subsystem, nullptr);
    _ZN21AFGBuildableSubsystem15RemoveBuildableEP12AFGBuildable(subsystem, nullptr);
    _ZN21AFGBuildableSubsystem11AddConveyorEP24AFGBuildableConveyorBase(subsystem, nullptr);
    _ZN21AFGBuildableSubsystem14RemoveConveyorEP24AFGBuildableConveyorBase(subsystem, nullptr);
    _ZN21AFGBuildableSubsystem14AddToTickGroupEP12AFGBuildable(subsystem, nullptr);
    _ZN21AFGBuildableSubsystem19RemoveFromTickGroupEP12AFGBuildable(subsystem, nullptr);
    _ZN19UFGInventoryLibrary17MoveInventoryItemEP21UFGInventoryComponentiS1_i(nullptr, 0, nullptr, 0);
    _ZN19UFGInventoryLibrary25GrabAllItemsFromInventoryEP21UFGInventoryComponentS1_11TSubclassOfI17UFGItemDescriptorE(nullptr, nullptr, nullptr);
    assert(satis_async_active_mutations() == 0);

    for(void* event:allocated_events) delete[] static_cast<unsigned char*>(event);
}
