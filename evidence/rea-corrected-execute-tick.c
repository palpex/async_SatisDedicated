
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* FFactoryTickFunction::ExecuteTick(float, ELevelTick, ENamedThreads::Type,
   TRefCountPtr<FBaseGraphTask> const&) */

void FFactoryTickFunction::ExecuteTick(float param_1,long param_2,undefined4 param_3)

{
  uint uVar1;
  code *pcVar2;
  char cVar3;
  uint uVar4;
  long lVar5;
  timespec local_40;
  double local_30;
  
  lVar5 = *(long *)(param_2 + 0x28);
  if ((lVar5 != 0) && ((*(uint *)(lVar5 + 8) & 0x40000000) == 0)) {
    uVar1 = *(uint *)(lVar5 + 0xc);
    if (((int)uVar1 < 0) &&
       (cVar3 = FDebug::CheckVerifyFailedImpl2
                          ("Index >= 0","Runtime/CoreUObject/Public\\UObject/UObjectArray.h",0x40d,
                           L""), cVar3 != '\0')) {
      pcVar2 = (code *)swi(3);
      (*pcVar2)();
      return;
    }
    if ((int)uVar1 < _DAT_027529dc) {
      uVar4 = uVar1 >> 0x10;
      if (((int)uVar1 < 0) &&
         (cVar3 = FDebug::CheckVerifyFailedImpl2
                            ("IsValidIndex(Index)",
                             "Runtime/CoreUObject/Public\\UObject/UObjectArray.h",0x32c,
                             L"IsValidIndex(%d)",(ulong)uVar1), cVar3 != '\0')) {
        pcVar2 = (code *)swi(3);
        (*pcVar2)();
        return;
      }
      if ((_DAT_027529e4 <= uVar4) &&
         (cVar3 = FDebug::CheckVerifyFailedImpl2
                            ("ChunkIndex < (uint32)NumChunks",
                             "Runtime/CoreUObject/Public\\UObject/UObjectArray.h",0x32d,
                             L"ChunkIndex (%d) < NumChunks (%d)",(ulong)uVar4), cVar3 != '\0')) {
        pcVar2 = (code *)swi(3);
        (*pcVar2)();
        return;
      }
      if ((_GetContainerName <= (int)uVar1) &&
         (cVar3 = FDebug::CheckVerifyFailedImpl2
                            ("Index < MaxElements",
                             "Runtime/CoreUObject/Public\\UObject/UObjectArray.h",0x32e,
                             L"Index (%d) < MaxElements (%d)",(ulong)uVar1), cVar3 != '\0')) {
        pcVar2 = (code *)swi(3);
        (*pcVar2)();
        return;
      }
      lVar5 = *(long *)(_Z_Construct_UClass_UPhysicalMaterial_NoRegister + (ulong)uVar4 * 8);
      if ((lVar5 == 0) &&
         (cVar3 = FDebug::CheckVerifyFailedImpl2
                            ("Chunk","Runtime/CoreUObject/Public\\UObject/UObjectArray.h",0x330,L"")
         , cVar3 != '\0')) {
        pcVar2 = (code *)swi(3);
        (*pcVar2)();
        return;
      }
      lVar5 = lVar5 + (ulong)((uVar1 & 0xffff) * 0x18);
      if (lVar5 == 0) {
        lVar5 = 0;
        cVar3 = FDebug::CheckVerifyFailedImpl2
                          ("ItemPtr","Runtime/CoreUObject/Public\\UObject/UObjectArray.h",0x34b,L"")
        ;
        if (cVar3 != '\0') {
          pcVar2 = (code *)swi(3);
          (*pcVar2)();
          return;
        }
      }
    }
    else {
      lVar5 = 0;
    }
    if ((*(uint *)(lVar5 + 8) & 0x10000000) == 0) {
      _GTickFactoryTime = 0.0;
      if ((int)FUnixTime::ClockSource < 0) {
        FUnixTime::ClockSource._0_4_ = FUnixTime::CalibrateAndSelectClock();
      }
      clock_gettime((int)FUnixTime::ClockSource,&local_40);
      local_30 = (double)local_40.tv_nsec / 1000000000.0 + (double)local_40.tv_sec;
      AFGBuildableSubsystem::TickFactory
                ((AFGBuildableSubsystem *)(param_1 * *(float *)(*(long *)(param_2 + 0x28) + 0x70)),
                 *(long *)(param_2 + 0x28),param_3);
      if ((int)FUnixTime::ClockSource < 0) {
        FUnixTime::ClockSource._0_4_ = FUnixTime::CalibrateAndSelectClock();
      }
      clock_gettime((int)FUnixTime::ClockSource,&local_40);
      _GTickFactoryTime =
           (((double)local_40.tv_nsec / 1000000000.0 + (double)local_40.tv_sec) - local_30) +
           _GTickFactoryTime;
    }
  }
  return;
}

