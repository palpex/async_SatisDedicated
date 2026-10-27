
/* AFGBuildableSubsystem::TickFactoryActors(float) */

void __thiscall AFGBuildableSubsystem::TickFactoryActors(AFGBuildableSubsystem *this,float param_1)

{
  int iVar1;
  UObject *pUVar2;
  AFGBuildableSubsystem *pAVar3;
  char cVar4;
  uint uVar5;
  uint uVar6;
  int iVar7;
  int iVar8;
  float fVar9;
  long *plVar10;
  UWorld *pUVar11;
  AFGConveyorChainSubsystem *this_00;
  UObject *pUVar12;
  code *pcVar13;
  long lVar14;
  UObject *pUVar15;
  UObject *pUVar16;
  ulong uVar17;
  ulong uVar18;
  UObject *pUVar19;
  int iVar20;
  ulong uVar21;
  long lVar22;
  AFGBuildableSubsystem *pAVar23;
  float fVar24;
  char local_a9;
  ulong local_a8;
  float local_a0;
  float local_9c;
  code *local_98;
  AFGBuildableSubsystem **ppAStack_90;
  AFGBuildableSubsystem *local_88;
  ulong local_80;
  AFGBuildableSubsystem *local_78;
  AFGBuildableSubsystem *local_70;
  float *local_68;
  uint local_5c;
  AFGConveyorChainSubsystem *local_58;
  UObject *local_50;
  undefined4 local_48;
  AFGBuildableSubsystem *local_40;
  float local_34;
  
  local_88 = (AFGBuildableSubsystem *)CONCAT44(local_88._4_4_,param_1);
  iVar7 = *(int *)(this + 0x948);
  local_a0 = param_1;
  if ((long)iVar7 != 0) {
    lVar14 = *(long *)(this + 0x940);
    lVar22 = 0;
    do {
      if ((*(long *)(lVar14 + lVar22) != 0) &&
         (plVar10 = *(long **)(lVar14 + 8 + lVar22), plVar10 != (long *)0x0)) {
        (**(code **)(*plVar10 + 0x18))(local_88._0_4_,plVar10,this);
      }
      lVar22 = lVar22 + 0x10;
    } while ((long)iVar7 * 0x10 != lVar22);
  }
  iVar7 = *DAT_02742a68;
  iVar8 = *DAT_02742a80;
  iVar20 = *DAT_02742af8;
  local_a8 = CONCAT44(local_a8._4_4_,*DAT_02742b28);
  iVar1 = *DAT_02742b88;
  AActor::GetWorld();
  local_a9 = '\x01';
  uVar6 = *DAT_02742b10;
  if ((int)*DAT_02742b10 < 2) {
    uVar6 = 1;
  }
  if (uVar6 != *(uint *)(this + 0x708)) {
    this[0x3e0] = (AFGBuildableSubsystem)0x1;
    this[0x4c0] = (AFGBuildableSubsystem)0x1;
    this[0x478] = (AFGBuildableSubsystem)0x1;
    *(uint *)(this + 0x708) = uVar6;
    if (4 < LogFactory) {
      local_40 = (AFGBuildableSubsystem *)CONCAT44(local_40._4_4_,iVar20);
      plVar10 = (long *)FTaskGraphInterface::Get();
      uVar5 = (**(code **)(*plVar10 + 0x28))(plVar10);
      UE::Logging::Private::BasicLog
                ((FLogCategoryBase *)&LogFactory,
                 (FStaticBasicLogRecord *)&PTR_u_Num_worker_threads___i_02657b98,(ulong)uVar5);
      iVar20 = (int)local_40;
    }
  }
  local_9c = (float)((int)(*(int *)(this + 0x3c8) + uVar6 + -1) / (int)uVar6);
  local_78 = (AFGBuildableSubsystem *)&local_9c;
  local_68 = &local_a0;
  uVar5 = (uint)(iVar1 != 0) * 2;
  if (iVar8 == 0) {
    uVar5 = 1;
  }
  if (iVar7 == 0) {
    uVar5 = 1;
  }
  local_98 = FUN_0199dbe0;
  ppAStack_90 = &local_78;
  local_50 = (UObject *)0x0;
  local_48 = 0;
  local_80 = (ulong)uVar6;
  local_70 = this;
  FUN_0194eea0(L"ParallelFor Task",(ulong)uVar6,1,&local_98,uVar5,&local_50);
  pUVar11 = (UWorld *)AActor::GetWorld();
  this_00 = (AFGConveyorChainSubsystem *)AFGConveyorChainSubsystem::Get(pUVar11);
  local_88 = this;
  if (this[0x170] == (AFGBuildableSubsystem)0x3) {
    if (this_00 != (AFGConveyorChainSubsystem *)0x0) {
      *(float *)(this_00 + 0x308) = local_a0 + *(float *)(this_00 + 0x308);
    }
    iVar7 = *(int *)(this + 0x400);
    if ((long)iVar7 != 0) {
      lVar14 = *(long *)(this + 0x3f8);
      lVar22 = 0;
      do {
        MigrateConveyorGroupToChainActor(this,*(FConveyorTickGroup **)(lVar14 + lVar22));
        lVar22 = lVar22 + 8;
      } while ((long)iVar7 * 8 != lVar22);
    }
    *(undefined4 *)(this + 0x400) = 0;
    if (*(int *)(this + 0x404) != 0) {
      FUN_013e3f40(8,8,0,this + 0x3f8,0,this + 0x404);
    }
  }
  else if ((this_00 != (AFGConveyorChainSubsystem *)0x0) &&
          (((*(float *)(this_00 + 0x30c) != 0.0 || (NAN(*(float *)(this_00 + 0x30c)))) &&
           (local_58 = this_00,
           cVar4 = AFGConveyorChainSubsystem::IsUnusuallyLargeTickDelta(this_00), cVar4 == '\0'))))
  {
    local_9c = (float)AFGConveyorChainSubsystem::GetAndConsumeClientTimeDebt(local_58,local_a0);
    if (this[0x4c1] != (AFGBuildableSubsystem)0x0) {
      local_40 = (AFGBuildableSubsystem *)CONCAT44(local_40._4_4_,iVar20);
      iVar7 = *(int *)(this + 0x49c);
      local_5c = uVar5;
      FUN_013e4890(*(undefined8 *)(this + 0x490),*(undefined4 *)(this + 0x498));
      *(undefined4 *)(this + 0x498) = 0;
      if ((iVar7 < (int)local_80) && (*(int *)(this + 0x49c) != (int)local_80)) {
        FUN_013e3f40(0x10,8,local_80,this + 0x490,0);
      }
      FUN_01829320(this + 0x490,local_80,1);
      if (0 < *(int *)(this + 0x488)) {
        lVar14 = 0;
        uVar18 = 0;
        do {
          uVar17 = (uVar18 & 0xffffffff) % (local_80 & 0xffffffff);
          if (((int)*(float *)(this + 0x498) <= (int)uVar17) &&
             (cVar4 = FDebug::CheckVerifyFailedImpl2
                                ("(Index >= 0) & (Index < ArrayNum)",
                                 "Runtime/Core/Public\\Containers/Array.h",0x42b,
                                 L"Array index out of bounds: %lld into an array of size %lld",
                                 uVar17), cVar4 != '\0')) {
            pcVar13 = (code *)swi(3);
            (*pcVar13)();
            return;
          }
          pUVar12 = *(UObject **)(this + 0x490);
          if (((long)(int)*(float *)(this + 0x488) <= (long)uVar18) &&
             (cVar4 = FDebug::CheckVerifyFailedImpl2
                                ("(Index >= 0) & (Index < ArrayNum)",
                                 "Runtime/Core/Public\\Containers/Array.h",0x42b,
                                 L"Array index out of bounds: %lld into an array of size %lld",
                                 uVar18), cVar4 != '\0')) {
            pcVar13 = (code *)swi(3);
            (*pcVar13)();
            return;
          }
          lVar22 = uVar17 * 0x10;
          pUVar19 = pUVar12 + lVar22;
          pUVar16 = *(UObject **)(this + 0x480);
          pUVar15 = pUVar16 + lVar14;
          pUVar2 = *(UObject **)pUVar19;
          if (((pUVar2 <= pUVar15) &&
              (pUVar15 < pUVar2 + (long)*(int *)(pUVar12 + lVar22 + 0xc) * 8)) &&
             (cVar4 = FDebug::CheckVerifyFailedImpl2
                                ("Addr < GetData() || Addr >= (GetData() + ArrayMax)",
                                 "Runtime/Core/Public\\Containers/Array.h",0x7ac,
                                 L"Attempting to use a container element (%p) which already comes from the container being modified (%p, ArrayMax: %lld, ArrayNum: %lld, SizeofElement: %zu)!"
                                 ,pUVar15,pUVar2,(long)*(int *)(pUVar12 + lVar22 + 0xc),
                                 (long)*(int *)(pUVar12 + lVar22 + 8),8), cVar4 != '\0')) {
            pcVar13 = (code *)swi(3);
            (*pcVar13)();
            return;
          }
          iVar7 = *(int *)(pUVar12 + lVar22 + 8);
          if (iVar7 == *(int *)(pUVar12 + lVar22 + 0xc)) {
            iVar7 = FUN_01423d90(0x808,pUVar19,pUVar12 + lVar22 + 0xc);
          }
          *(int *)(pUVar12 + lVar22 + 8) = iVar7 + 1;
          *(undefined8 *)(*(ulong *)pUVar19 + (long)iVar7 * 8) =
               *(undefined8 *)(pUVar16 + uVar18 * 8);
          uVar18 = uVar18 + 1;
          lVar14 = lVar14 + 8;
          this = local_88;
        } while ((long)uVar18 < (long)(int)*(float *)(local_88 + 0x488));
      }
      this[0x4c1] = (AFGBuildableSubsystem)0x0;
      iVar20 = (int)local_40;
      uVar5 = local_5c;
    }
    local_34 = local_9c;
    fVar24 = (float)AFGConveyorChainSubsystem::GetAverageServerTickDeltaTime(local_58);
    if (-fVar24 <= local_34) {
      *(float *)(local_58 + 0x30c) = local_a0 + local_9c + *(float *)(local_58 + 0x30c);
      local_70 = (AFGBuildableSubsystem *)&local_a0;
      local_68 = &local_9c;
      local_98 = FUN_0199dcc0;
      local_50 = (UObject *)0x0;
      local_48 = 0;
      ppAStack_90 = &local_78;
      local_78 = this;
      FUN_0194eea0(L"ParallelFor Task",*(float *)(this + 0x498),1,&local_98,uVar5,&local_50);
    }
  }
  if (iVar20 == 0) {
    fVar24 = *(float *)(this + 0x3f0);
    local_70 = (AFGBuildableSubsystem *)&local_a9;
    local_68 = &local_a0;
    local_98 = FUN_0199dec0;
  }
  else {
    if (this[0x478] != (AFGBuildableSubsystem)0x0) {
      fVar24 = *(float *)(this + 0x474);
      local_5c = uVar5;
      FUN_013e4890(*(UObject **)(this + 0x468),*(float *)(this + 0x470));
      *(float *)(this + 0x470) = 0.0;
      if (((int)fVar24 < (int)(float)local_80) && (*(float *)(this + 0x474) != (float)local_80)) {
        FUN_013e3f40(0x10,8,local_80,this + 0x468,0);
      }
      uVar18 = local_80;
      FUN_01829320(this + 0x468,local_80,1);
      local_98 = (code *)0x0;
      ppAStack_90 = (AFGBuildableSubsystem **)0x0;
      FUN_01607440(&local_98,uVar18 & 0xffffffff,1);
      uVar18 = (ulong)(uint)*(float *)(this + 0x3f0);
      if (0 < (int)*(float *)(this + 0x3f0)) {
        if ((int)local_a8 == 0) {
          lVar14 = 0;
          uVar18 = 0;
          do {
            if (**(long **)(*(UObject **)(this + 1000) + uVar18 * 8) == 0) {
              uVar17 = (uVar18 & 0xffffffff) % (local_80 & 0xffffffff);
              if (((int)*(float *)(this + 0x470) <= (int)uVar17) &&
                 (cVar4 = FDebug::CheckVerifyFailedImpl2
                                    ("(Index >= 0) & (Index < ArrayNum)",
                                     "Runtime/Core/Public\\Containers/Array.h",0x42b,
                                     L"Array index out of bounds: %lld into an array of size %lld",
                                     uVar17), cVar4 != '\0')) {
                pcVar13 = (code *)swi(3);
                (*pcVar13)();
                return;
              }
              pUVar12 = *(UObject **)(this + 0x468);
              if (((long)(int)*(float *)(this + 0x3f0) <= (long)uVar18) &&
                 (cVar4 = FDebug::CheckVerifyFailedImpl2
                                    ("(Index >= 0) & (Index < ArrayNum)",
                                     "Runtime/Core/Public\\Containers/Array.h",0x42b,
                                     L"Array index out of bounds: %lld into an array of size %lld",
                                     uVar18), cVar4 != '\0')) {
                pcVar13 = (code *)swi(3);
                (*pcVar13)();
                return;
              }
              lVar22 = uVar17 * 0x10;
              pUVar19 = pUVar12 + lVar22;
              pUVar16 = *(UObject **)(this + 1000);
              pUVar15 = pUVar16 + lVar14;
              pUVar2 = *(UObject **)pUVar19;
              if (((pUVar2 <= pUVar15) &&
                  (pUVar15 < pUVar2 + (long)*(int *)(pUVar12 + lVar22 + 0xc) * 8)) &&
                 (cVar4 = FDebug::CheckVerifyFailedImpl2
                                    ("Addr < GetData() || Addr >= (GetData() + ArrayMax)",
                                     "Runtime/Core/Public\\Containers/Array.h",0x7ac,
                                     L"Attempting to use a container element (%p) which already comes from the container being modified (%p, ArrayMax: %lld, ArrayNum: %lld, SizeofElement: %zu)!"
                                     ,pUVar15,pUVar2,(long)*(int *)(pUVar12 + lVar22 + 0xc),
                                     (long)*(int *)(pUVar12 + lVar22 + 8),8), cVar4 != '\0')) {
                pcVar13 = (code *)swi(3);
                (*pcVar13)();
                return;
              }
              pUVar16 = pUVar16 + uVar18 * 8;
              pUVar15 = pUVar12 + lVar22 + 8;
              iVar7 = *(int *)(pUVar12 + lVar22 + 8);
              if (iVar7 == *(int *)(pUVar12 + lVar22 + 0xc)) goto LAB_01980c10;
            }
            else {
              if (((int)*(float *)(this + 0x470) < 1) &&
                 (cVar4 = FDebug::CheckVerifyFailedImpl2
                                    ("(Index >= 0) & (Index < ArrayNum)",
                                     "Runtime/Core/Public\\Containers/Array.h",0x42b,
                                     L"Array index out of bounds: %lld into an array of size %lld",0
                                    ), cVar4 != '\0')) {
                pcVar13 = (code *)swi(3);
                (*pcVar13)();
                return;
              }
              pUVar19 = *(UObject **)(this + 0x468);
              if (((long)(int)*(float *)(this + 0x3f0) <= (long)uVar18) &&
                 (cVar4 = FDebug::CheckVerifyFailedImpl2
                                    ("(Index >= 0) & (Index < ArrayNum)",
                                     "Runtime/Core/Public\\Containers/Array.h",0x42b,
                                     L"Array index out of bounds: %lld into an array of size %lld",
                                     uVar18), cVar4 != '\0')) {
                pcVar13 = (code *)swi(3);
                (*pcVar13)();
                return;
              }
              pUVar16 = *(UObject **)(this + 1000) + lVar14;
              pUVar12 = *(UObject **)pUVar19;
              if (((pUVar12 <= pUVar16) && (pUVar16 < pUVar12 + (long)*(int *)(pUVar19 + 0xc) * 8))
                 && (cVar4 = FDebug::CheckVerifyFailedImpl2
                                       ("Addr < GetData() || Addr >= (GetData() + ArrayMax)",
                                        "Runtime/Core/Public\\Containers/Array.h",0x7ac,
                                        L"Attempting to use a container element (%p) which already comes from the container being modified (%p, ArrayMax: %lld, ArrayNum: %lld, SizeofElement: %zu)!"
                                        ,pUVar16,pUVar12,(long)*(int *)(pUVar19 + 0xc),
                                        (long)*(int *)(pUVar19 + 8),8), cVar4 != '\0')) {
                pcVar13 = (code *)swi(3);
                (*pcVar13)();
                return;
              }
              pUVar15 = pUVar19 + 8;
              iVar7 = *(int *)(pUVar19 + 8);
              if (iVar7 == *(int *)(pUVar19 + 0xc)) {
LAB_01980c10:
                iVar7 = FUN_01423d90(0x808,pUVar19,pUVar19 + 0xc);
              }
            }
            *(int *)pUVar15 = iVar7 + 1;
            *(undefined8 *)(*(ulong *)pUVar19 + (long)iVar7 * 8) = *(undefined8 *)pUVar16;
            uVar18 = uVar18 + 1;
            lVar14 = lVar14 + 8;
            this = local_88;
          } while ((long)uVar18 < (long)(int)*(float *)(local_88 + 0x3f0));
        }
        else {
          pAVar23 = this + 0x408;
          uVar17 = 0;
          local_40 = pAVar23;
          do {
            local_a8 = uVar17;
            if (**(long **)(*(UObject **)(this + 1000) + uVar17 * 8) == 0) {
              if ((int)ppAStack_90 < 1) {
LAB_01980810:
                pAVar3 = local_88;
                uVar18 = (uVar17 & 0xffffffff) % (local_80 & 0xffffffff);
                if (((int)*(float *)(local_88 + 0x470) <= (int)uVar18) &&
                   (cVar4 = FDebug::CheckVerifyFailedImpl2
                                      ("(Index >= 0) & (Index < ArrayNum)",
                                       "Runtime/Core/Public\\Containers/Array.h",0x42b,
                                       L"Array index out of bounds: %lld into an array of size %lld"
                                       ,uVar18), uVar17 = local_a8, cVar4 != '\0')) {
                  pcVar13 = (code *)swi(3);
                  (*pcVar13)();
                  return;
                }
                pUVar12 = *(UObject **)(pAVar3 + 0x468);
                if (((long)(int)*(float *)(pAVar3 + 0x3f0) <= (long)uVar17) &&
                   (cVar4 = FDebug::CheckVerifyFailedImpl2
                                      ("(Index >= 0) & (Index < ArrayNum)",
                                       "Runtime/Core/Public\\Containers/Array.h",0x42b,
                                       L"Array index out of bounds: %lld into an array of size %lld"
                                       ,local_a8), uVar17 = local_a8, cVar4 != '\0')) {
                  pcVar13 = (code *)swi(3);
                  (*pcVar13)();
                  return;
                }
                lVar14 = uVar18 * 0x10;
                pUVar19 = pUVar12 + lVar14;
                pUVar16 = *(UObject **)(pAVar3 + 1000) + uVar17 * 8;
                pUVar15 = *(UObject **)pUVar19;
                if (((pUVar15 <= pUVar16) &&
                    (pUVar16 < pUVar15 + (long)*(int *)(pUVar12 + lVar14 + 0xc) * 8)) &&
                   (cVar4 = FDebug::CheckVerifyFailedImpl2
                                      ("Addr < GetData() || Addr >= (GetData() + ArrayMax)",
                                       "Runtime/Core/Public\\Containers/Array.h",0x7ac,
                                       L"Attempting to use a container element (%p) which already comes from the container being modified (%p, ArrayMax: %lld, ArrayNum: %lld, SizeofElement: %zu)!"
                                       ,pUVar16,pUVar15,(long)*(int *)(pUVar12 + lVar14 + 0xc),
                                       (long)*(int *)(pUVar12 + lVar14 + 8),8), uVar17 = local_a8,
                   cVar4 != '\0')) {
                  pcVar13 = (code *)swi(3);
                  (*pcVar13)();
                  return;
                }
              }
              else {
                uVar18 = 0xffffffff;
                iVar7 = 0x7fffffff;
                uVar21 = 0;
                pcVar13 = local_98;
                iVar8 = (int)ppAStack_90;
                do {
                  if (*(int *)(pcVar13 + uVar21 * 4) < iVar7) {
                    if (((long)iVar8 <= (long)uVar21) &&
                       (cVar4 = FDebug::CheckVerifyFailedImpl2
                                          ("(Index >= 0) & (Index < ArrayNum)",
                                           "Runtime/Core/Public\\Containers/Array.h",0x42b,
                                           L"Array index out of bounds: %lld into an array of size %lld"
                                           ,uVar21), uVar17 = local_a8, cVar4 != '\0')) {
                      pcVar13 = (code *)swi(3);
                      (*pcVar13)();
                      return;
                    }
                    iVar7 = *(int *)(local_98 + uVar21 * 4);
                    uVar18 = uVar21 & 0xffffffff;
                    pcVar13 = local_98;
                    iVar8 = (int)ppAStack_90;
                  }
                  pAVar3 = local_88;
                  uVar21 = uVar21 + 1;
                } while ((long)uVar21 < (long)iVar8);
                iVar7 = (int)uVar18;
                if (iVar7 == -1) goto LAB_01980810;
                if (((long)(int)*(float *)(local_88 + 0x3f0) <= (long)uVar17) &&
                   (cVar4 = FDebug::CheckVerifyFailedImpl2
                                      ("(Index >= 0) & (Index < ArrayNum)",
                                       "Runtime/Core/Public\\Containers/Array.h",0x42b,
                                       L"Array index out of bounds: %lld into an array of size %lld"
                                       ,local_a8), uVar17 = local_a8, cVar4 != '\0')) {
                  pcVar13 = (code *)swi(3);
                  (*pcVar13)();
                  return;
                }
                iVar8 = *(int *)(*(long *)(*(UObject **)(pAVar3 + 1000) + uVar17 * 8) + 0x10);
                if ((iVar7 < 0) || ((int)ppAStack_90 <= iVar7)) {
                  uVar18 = (ulong)iVar7;
                  cVar4 = FDebug::CheckVerifyFailedImpl2
                                    ("(Index >= 0) & (Index < ArrayNum)",
                                     "Runtime/Core/Public\\Containers/Array.h",0x42b,
                                     L"Array index out of bounds: %lld into an array of size %lld",
                                     uVar18,(long)(int)ppAStack_90);
                  uVar17 = local_a8;
                  if (cVar4 != '\0') {
                    pcVar13 = (code *)swi(3);
                    (*pcVar13)();
                    return;
                  }
                }
                *(int *)(local_98 + uVar18 * 4) = *(int *)(local_98 + uVar18 * 4) + iVar8;
                if (((iVar7 < 0) || ((int)*(float *)(local_88 + 0x470) <= iVar7)) &&
                   (cVar4 = FDebug::CheckVerifyFailedImpl2
                                      ("(Index >= 0) & (Index < ArrayNum)",
                                       "Runtime/Core/Public\\Containers/Array.h",0x42b,
                                       L"Array index out of bounds: %lld into an array of size %lld"
                                       ,uVar18,(long)(int)*(float *)(local_88 + 0x470)),
                   uVar17 = local_a8, cVar4 != '\0')) {
                  pcVar13 = (code *)swi(3);
                  (*pcVar13)();
                  return;
                }
                pAVar3 = local_88;
                pUVar12 = *(UObject **)(local_88 + 0x468);
                if (((long)(int)*(float *)(local_88 + 0x3f0) <= (long)uVar17) &&
                   (cVar4 = FDebug::CheckVerifyFailedImpl2
                                      ("(Index >= 0) & (Index < ArrayNum)",
                                       "Runtime/Core/Public\\Containers/Array.h",0x42b,
                                       L"Array index out of bounds: %lld into an array of size %lld"
                                       ,local_a8), uVar17 = local_a8, cVar4 != '\0')) {
                  pcVar13 = (code *)swi(3);
                  (*pcVar13)();
                  return;
                }
                lVar14 = uVar18 * 0x10;
                pUVar19 = pUVar12 + lVar14;
                pUVar16 = *(UObject **)(pAVar3 + 1000) + uVar17 * 8;
                pUVar15 = *(UObject **)pUVar19;
                if (((pUVar15 <= pUVar16) &&
                    (pUVar16 < pUVar15 + (long)*(int *)(pUVar12 + lVar14 + 0xc) * 8)) &&
                   (cVar4 = FDebug::CheckVerifyFailedImpl2
                                      ("Addr < GetData() || Addr >= (GetData() + ArrayMax)",
                                       "Runtime/Core/Public\\Containers/Array.h",0x7ac,
                                       L"Attempting to use a container element (%p) which already comes from the container being modified (%p, ArrayMax: %lld, ArrayNum: %lld, SizeofElement: %zu)!"
                                       ,pUVar16,pUVar15,(long)*(int *)(pUVar12 + lVar14 + 0xc),
                                       (long)*(int *)(pUVar12 + lVar14 + 8),8), uVar17 = local_a8,
                   cVar4 != '\0')) {
                  pcVar13 = (code *)swi(3);
                  (*pcVar13)();
                  return;
                }
              }
              iVar7 = *(int *)(pUVar12 + lVar14 + 8);
              if (iVar7 == *(int *)(pUVar12 + lVar14 + 0xc)) {
                iVar7 = FUN_01423d90(0x808,pUVar19,pUVar12 + lVar14 + 0xc);
                uVar17 = local_a8;
              }
              *(int *)(pUVar12 + lVar14 + 8) = iVar7 + 1;
              *(undefined8 *)(*(long *)pUVar19 + (long)iVar7 * 8) = *(undefined8 *)pUVar16;
              this = local_88;
            }
            else {
              if (((long)(int)uVar18 <= (long)uVar17) &&
                 (cVar4 = FDebug::CheckVerifyFailedImpl2
                                    ("(Index >= 0) & (Index < ArrayNum)",
                                     "Runtime/Core/Public\\Containers/Array.h",0x42b,
                                     L"Array index out of bounds: %lld into an array of size %lld",
                                     uVar17), cVar4 != '\0')) {
                pcVar13 = (code *)swi(3);
                (*pcVar13)();
                return;
              }
              pUVar12 = (UObject *)**(undefined8 **)(*(UObject **)(this + 1000) + local_a8 * 8);
              if ((pUVar12 != (UObject *)0x0) &&
                 ((char)UE::GC::GIsIncrementalReachabilityPending != '\0')) {
                UE::GC::MarkAsReachable(pUVar12);
              }
              local_78 = (AFGBuildableSubsystem *)pUVar12;
              if ((pUVar12 != (UObject *)0x0) &&
                 ((char)UE::GC::GIsIncrementalReachabilityPending != '\0')) {
                UE::GC::MarkAsReachable(pUVar12);
              }
              pAVar3 = local_78;
              if (((char)UE::GC::GIsIncrementalReachabilityPending != '\0') &&
                 (local_78 != (AFGBuildableSubsystem *)0x0)) {
                UE::GC::MarkAsReachable((UObject *)local_78);
              }
              uVar6 = ((uint)((ulong)pAVar3 >> 0x14) & 0xffff ^ (uint)((ulong)pAVar3 >> 4)) *
                      -0x7a143595;
              uVar6 = (uVar6 >> 0xd ^ uVar6) * -0x3d4d51cb;
              iVar7 = FUN_018765e0(pAVar23,uVar6 >> 0x10 ^ uVar6,&local_78);
              if (iVar7 == -1) {
                uVar18 = 0xffffffff;
                fVar24 = -NAN;
                if (0 < (int)ppAStack_90) {
                  iVar7 = 0x7fffffff;
                  uVar17 = 0;
                  pcVar13 = local_98;
                  iVar8 = (int)ppAStack_90;
                  do {
                    if (*(int *)(pcVar13 + uVar17 * 4) < iVar7) {
                      if (((long)iVar8 <= (long)uVar17) &&
                         (cVar4 = FDebug::CheckVerifyFailedImpl2
                                            ("(Index >= 0) & (Index < ArrayNum)",
                                             "Runtime/Core/Public\\Containers/Array.h",0x42b,
                                             L"Array index out of bounds: %lld into an array of size %lld"
                                             ,uVar17), cVar4 != '\0')) {
                        pcVar13 = (code *)swi(3);
                        (*pcVar13)();
                        return;
                      }
                      iVar7 = *(int *)(local_98 + uVar17 * 4);
                      uVar18 = uVar17 & 0xffffffff;
                      pcVar13 = local_98;
                      iVar8 = (int)ppAStack_90;
                    }
                    fVar24 = (float)uVar18;
                    uVar17 = uVar17 + 1;
                  } while ((long)uVar17 < (long)iVar8);
                }
                local_9c = fVar24;
                local_50 = pUVar12;
                if ((pUVar12 != (UObject *)0x0) &&
                   ((char)UE::GC::GIsIncrementalReachabilityPending != '\0')) {
                  UE::GC::MarkAsReachable(pUVar12);
                }
                local_78 = (AFGBuildableSubsystem *)&local_50;
                local_70 = (AFGBuildableSubsystem *)&local_9c;
                FUN_0187c060(pAVar23,&local_78,0);
                fVar9 = *(float *)(local_88 + 0x470);
                this = local_88;
                fVar24 = local_9c;
                if (-1 < (int)local_9c) goto LAB_019806fc;
LAB_01980966:
                uVar18 = (ulong)(int)fVar24;
                cVar4 = FDebug::CheckVerifyFailedImpl2
                                  ("(Index >= 0) & (Index < ArrayNum)",
                                   "Runtime/Core/Public\\Containers/Array.h",0x42b,
                                   L"Array index out of bounds: %lld into an array of size %lld",
                                   uVar18,(long)(int)fVar9);
                if (cVar4 != '\0') {
                  pcVar13 = (code *)swi(3);
                  (*pcVar13)();
                  return;
                }
              }
              else {
                if ((pUVar12 != (UObject *)0x0) &&
                   ((char)UE::GC::GIsIncrementalReachabilityPending != '\0')) {
                  UE::GC::MarkAsReachable(pUVar12);
                }
                if ((pUVar12 != (UObject *)0x0) &&
                   ((char)UE::GC::GIsIncrementalReachabilityPending != '\0')) {
                  UE::GC::MarkAsReachable(pUVar12);
                }
                local_78 = (AFGBuildableSubsystem *)pUVar12;
                if ((pUVar12 != (UObject *)0x0) &&
                   ((char)UE::GC::GIsIncrementalReachabilityPending != '\0')) {
                  UE::GC::MarkAsReachable(pUVar12);
                }
                pAVar3 = local_78;
                if (((char)UE::GC::GIsIncrementalReachabilityPending != '\0') &&
                   (local_78 != (AFGBuildableSubsystem *)0x0)) {
                  UE::GC::MarkAsReachable((UObject *)local_78);
                }
                uVar6 = ((uint)((ulong)pAVar3 >> 0x14) & 0xffff ^ (uint)((ulong)pAVar3 >> 4)) *
                        -0x7a143595;
                uVar6 = (uVar6 >> 0xd ^ uVar6) * -0x3d4d51cb;
                iVar7 = FUN_018765e0(pAVar23,uVar6 >> 0x10 ^ uVar6,&local_78);
                if ((iVar7 == -1) ||
                   (pUVar12 = *(UObject **)pAVar23 + (long)iVar7 * 0x18, pUVar12 == (UObject *)0x0))
                {
                  cVar4 = FDebug::CheckVerifyFailedImpl2
                                    ("Pair != nullptr","Runtime/Core/Public\\Containers/Map.h",0x2d9
                                     ,L"");
                  if (cVar4 != '\0') {
                    pcVar13 = (code *)swi(3);
                    (*pcVar13)();
                    return;
                  }
                  pUVar12 = (UObject *)0x0;
                }
                fVar24 = *(float *)(pUVar12 + 8);
                fVar9 = *(float *)(this + 0x470);
                if ((int)fVar24 < 0) goto LAB_01980966;
LAB_019806fc:
                if ((int)fVar9 <= (int)fVar24) goto LAB_01980966;
                uVar18 = (ulong)(uint)fVar24;
              }
              pUVar12 = *(UObject **)(this + 0x468);
              if (((long)(int)*(float *)(this + 0x3f0) <= (long)local_a8) &&
                 (cVar4 = FDebug::CheckVerifyFailedImpl2
                                    ("(Index >= 0) & (Index < ArrayNum)",
                                     "Runtime/Core/Public\\Containers/Array.h",0x42b,
                                     L"Array index out of bounds: %lld into an array of size %lld",
                                     local_a8), cVar4 != '\0')) {
                pcVar13 = (code *)swi(3);
                (*pcVar13)();
                return;
              }
              lVar14 = uVar18 * 0x10;
              pUVar19 = pUVar12 + lVar14;
              pUVar16 = *(UObject **)(this + 1000) + local_a8 * 8;
              pUVar15 = *(UObject **)pUVar19;
              if (((pUVar15 <= pUVar16) &&
                  (pUVar16 < pUVar15 + (long)*(int *)(pUVar12 + lVar14 + 0xc) * 8)) &&
                 (cVar4 = FDebug::CheckVerifyFailedImpl2
                                    ("Addr < GetData() || Addr >= (GetData() + ArrayMax)",
                                     "Runtime/Core/Public\\Containers/Array.h",0x7ac,
                                     L"Attempting to use a container element (%p) which already comes from the container being modified (%p, ArrayMax: %lld, ArrayNum: %lld, SizeofElement: %zu)!"
                                     ,pUVar16,pUVar15,(long)*(int *)(pUVar12 + lVar14 + 0xc),
                                     (long)*(int *)(pUVar12 + lVar14 + 8),8), cVar4 != '\0')) {
                pcVar13 = (code *)swi(3);
                (*pcVar13)();
                return;
              }
              pAVar23 = local_40;
              iVar7 = *(int *)(pUVar12 + lVar14 + 8);
              if (iVar7 == *(int *)(pUVar12 + lVar14 + 0xc)) {
                local_58 = (AFGConveyorChainSubsystem *)CONCAT44(local_58._4_4_,fVar24);
                iVar7 = FUN_01423d90(0x808,pUVar19,pUVar12 + lVar14 + 0xc);
                fVar24 = local_58._0_4_;
              }
              this = local_88;
              *(int *)(pUVar12 + lVar14 + 8) = iVar7 + 1;
              *(undefined8 *)(*(ulong *)pUVar19 + (long)iVar7 * 8) = *(undefined8 *)pUVar16;
              if (((long)(int)*(float *)(local_88 + 0x3f0) <= (long)local_a8) &&
                 (cVar4 = FDebug::CheckVerifyFailedImpl2
                                    ("(Index >= 0) & (Index < ArrayNum)",
                                     "Runtime/Core/Public\\Containers/Array.h",0x42b,
                                     L"Array index out of bounds: %lld into an array of size %lld",
                                     local_a8), cVar4 != '\0')) {
                pcVar13 = (code *)swi(3);
                (*pcVar13)();
                return;
              }
              iVar7 = *(int *)(*(long *)(*(UObject **)(this + 1000) + local_a8 * 8) + 0x10);
              if ((((int)fVar24 < 0) || ((int)ppAStack_90 <= (int)fVar24)) &&
                 (cVar4 = FDebug::CheckVerifyFailedImpl2
                                    ("(Index >= 0) & (Index < ArrayNum)",
                                     "Runtime/Core/Public\\Containers/Array.h",0x42b,
                                     L"Array index out of bounds: %lld into an array of size %lld",
                                     uVar18,(long)(int)ppAStack_90), cVar4 != '\0')) {
                pcVar13 = (code *)swi(3);
                (*pcVar13)();
                return;
              }
              *(int *)(local_98 + uVar18 * 4) = *(int *)(local_98 + uVar18 * 4) + iVar7;
              uVar17 = local_a8;
            }
            uVar17 = uVar17 + 1;
            uVar18 = (ulong)(int)*(float *)(this + 0x3f0);
          } while ((long)uVar17 < (long)uVar18);
        }
      }
      this[0x478] = (AFGBuildableSubsystem)0x0;
      uVar5 = local_5c;
      if (local_98 != (code *)0x0) {
        FMemory::Free(local_98);
        uVar5 = local_5c;
      }
    }
    fVar24 = *(float *)(this + 0x470);
    local_70 = (AFGBuildableSubsystem *)&local_a0;
    local_68 = (float *)&local_a9;
    local_98 = FUN_0199dd80;
  }
  ppAStack_90 = &local_78;
  local_50 = (UObject *)0x0;
  local_48 = 0;
  local_78 = this;
  FUN_0194eea0(L"ParallelFor Task",fVar24,1,&local_98,uVar5,&local_50);
  if ((long)(int)*(float *)(this + 0x460) != 0) {
    pUVar12 = *(UObject **)(this + 0x458);
    lVar14 = (long)(int)*(float *)(this + 0x460) * 8;
    lVar22 = 0;
    do {
      while (((plVar10 = *(long **)(pUVar12 + lVar22), (~*(byte *)((long)plVar10 + 0x619) & 3) != 0
              || ((int)plVar10[0xd1] == 0)) || (local_a9 == '\0'))) {
        (**(code **)(*plVar10 + 0x930))(local_a0);
        lVar22 = lVar22 + 8;
        if (lVar14 == lVar22) goto LAB_01980fa8;
      }
      *(float *)(plVar10 + 0xea) = local_a0 + *(float *)(plVar10 + 0xea);
      lVar22 = lVar22 + 8;
    } while (lVar14 != lVar22);
  }
LAB_01980fa8:
  fVar24 = *(float *)(this + 0x948);
  if ((long)(int)fVar24 != 0) {
    pUVar12 = *(UObject **)(this + 0x940);
    lVar14 = 0;
    do {
      if ((*(long *)(pUVar12 + lVar14) != 0) &&
         (plVar10 = *(long **)(pUVar12 + lVar14 + 8), plVar10 != (long *)0x0)) {
        (**(code **)(*plVar10 + 0x20))(local_a0,plVar10,this);
      }
      lVar14 = lVar14 + 0x10;
    } while ((long)(int)fVar24 * 0x10 != lVar14);
  }
  return;
}

