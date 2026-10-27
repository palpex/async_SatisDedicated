
/* WARNING: Globals starting with '_' overlap smaller symbols at the same address */
/* AFGConveyorChainActor::Factory_Tick(float) */

void __thiscall AFGConveyorChainActor::Factory_Tick(AFGConveyorChainActor *this,float param_1)

{
  AFGConveyorChainActor *pAVar1;
  int iVar2;
  code *pcVar3;
  undefined1 auVar4 [16];
  UObject *pUVar5;
  byte bVar6;
  char cVar7;
  char cVar8;
  int iVar9;
  int iVar10;
  UObject *pUVar11;
  float *pfVar12;
  UWorld *pUVar13;
  AFGBuildableSubsystem *this_00;
  byte bVar14;
  ulong uVar15;
  long lVar16;
  undefined ***pppuVar17;
  long lVar18;
  long lVar19;
  ulong uVar20;
  undefined2 *puVar21;
  int iVar22;
  ulong uVar23;
  long lVar24;
  long *plVar25;
  long lVar26;
  UObject *pUVar27;
  bool bVar28;
  uint uVar29;
  uint uVar30;
  float fVar31;
  float fVar32;
  float fVar33;
  uint in_XMM0_Db;
  uint in_XMM0_Dc;
  uint in_XMM0_Dd;
  float fVar34;
  float fVar35;
  float fVar36;
  float fVar37;
  undefined1 auVar38 [16];
  undefined1 auVar39 [16];
  undefined1 auVar40 [16];
  float fVar41;
  float fVar42;
  undefined4 uVar43;
  undefined4 uVar44;
  undefined4 uVar45;
  float fVar46;
  UObject *local_d8;
  int local_d0;
  undefined1 local_c8 [16];
  undefined4 local_b0;
  undefined4 uStack_ac;
  int local_a8;
  UObject *local_a0;
  int local_98;
  undefined4 uStack_94;
  FFGDynamicStruct local_90 [24];
  long *local_78;
  undefined8 local_70;
  code *local_68 [2];
  undefined **local_58;
  AFGConveyorChainActor *local_50;
  uint local_48;
  undefined ***local_40;
  
  cVar8 = AFGBuildableSubsystem::mCurrentSubStep;
  if ((*(ushort *)(this + 100) & 0xc000) != 0x8000) {
    return;
  }
  if (*(long *)(this + 0x3c0) == 0) {
    return;
  }
  if (*(int *)(this + 0x490) < 1) {
    return;
  }
  if (*(int *)(this + 0x488) < 1) {
    return;
  }
  if (this[0x170] == (AFGConveyorChainActor)0x3) {
    uVar43 = *(undefined4 *)(*(long *)(this + 0x3c0) + 0x308);
    *(undefined4 *)(this + 0x380) = uVar43;
    *(undefined4 *)(this + 0x3a8) = uVar43;
    lVar24 = *(long *)(this + 0x350);
    if (((lVar24 != 0) && ((*(uint *)(lVar24 + 8) & 0x40000000) == 0)) &&
       (*(char *)(lVar24 + 0x268) != '\0')) {
      FInventoryItem::FInventoryItem((FInventoryItem *)&local_a0);
      local_b0 = 0;
      fVar31 = (float)GetAvailableSpace(this);
      if (120.0 < fVar31) {
        pAVar1 = this + 0x418;
        do {
          local_70 = 0;
          cVar7 = UFGFactoryConnectionComponent::Factory_GrabOutput
                            (*(UFGFactoryConnectionComponent **)(this + 0x350),
                             (FInventoryItem *)&local_a0,&local_b0,&local_70);
          if (cVar7 == '\0') break;
          fVar32 = *(float *)(this + 0x4b4);
          uVar29 = -(uint)(fVar32 == -1.0);
          iVar9 = *(int *)(this + 0x490);
          uVar23 = (long)iVar9 - 1;
          if (((long)iVar9 < 1) || (*(int *)(this + 0x2d8) < iVar9)) {
            cVar7 = FDebug::CheckVerifyFailedImpl2
                              ("(Index >= 0) & (Index < ArrayNum)",
                               "Runtime/Core/Public\\Containers/Array.h",0x42b,
                               L"Array index out of bounds: %lld into an array of size %lld",uVar23,
                               (long)*(int *)(this + 0x2d8));
            if (cVar7 != '\0') {
              pcVar3 = (code *)swi(3);
              (*pcVar3)();
              return;
            }
          }
          else {
            uVar23 = uVar23 & 0xffffffff;
          }
          fVar36 = (float)(~uVar29 & (uint)fVar32 | uVar29 & (uint)param_1) *
                   *(float *)(*(long *)(this + 0x2d0) + 0x30 + uVar23 * 0x40);
          fVar32 = fVar31 + -120.0;
          if (fVar36 <= fVar31 + -120.0) {
            fVar32 = fVar36;
          }
          if (fVar32 <= 0.0) {
            fVar32 = 0.0;
          }
          Factory_EnqueueItem(this,(FInventoryItem *)&local_a0,fVar32,0.0);
          iVar9 = *(int *)(this + 0x490);
          uVar23 = (long)iVar9 - 1;
          if (((long)iVar9 < 1) || (*(int *)(this + 0x2d8) < iVar9)) {
            cVar7 = FDebug::CheckVerifyFailedImpl2
                              ("(Index >= 0) & (Index < ArrayNum)",
                               "Runtime/Core/Public\\Containers/Array.h",0x42b,
                               L"Array index out of bounds: %lld into an array of size %lld",uVar23,
                               (long)*(int *)(this + 0x2d8));
            if (cVar7 != '\0') {
              pcVar3 = (code *)swi(3);
              (*pcVar3)();
              return;
            }
          }
          else {
            uVar23 = uVar23 & 0xffffffff;
          }
          lVar24 = *(long *)(this + 0x2d0);
          lVar19 = uVar23 * 0x40;
          uVar29 = *(uint *)(this + 0x368);
          uVar23 = (ulong)(int)uVar29;
          lVar16 = *(long *)(lVar24 + lVar19);
          fVar31 = *(float *)(lVar24 + 0x20 + lVar19);
          iVar9 = *(int *)(lVar16 + 0x338);
          if (((long)uVar23 < 0) || (iVar9 <= (int)uVar29)) {
            cVar7 = FDebug::CheckVerifyFailedImpl2
                              ("(Index >= 0) & (Index < ArrayNum)",
                               "Runtime/Core/Public\\Containers/Array.h",0x42b,
                               L"Array index out of bounds: %lld into an array of size %lld",uVar23,
                               (long)iVar9);
            if (cVar7 != '\0') {
              pcVar3 = (code *)swi(3);
              (*pcVar3)();
              return;
            }
          }
          else {
            uVar23 = (ulong)uVar29;
          }
          lVar16 = *(long *)(lVar16 + 0x330);
          *(float *)(lVar16 + 0x28 + uVar23 * 0x38) =
               fVar31 + *(float *)(lVar16 + 0x28 + uVar23 * 0x38);
          if (*(int *)(lVar24 + 0x34 + lVar19) == -1) {
            *(uint *)(lVar19 + lVar24 + 0x34) = uVar29;
          }
          *(uint *)(lVar24 + 0x38 + lVar19) = uVar29;
          if (0 < *(int *)(this + 0x470)) {
            pUVar27 = (UObject *)CONCAT44(uStack_94,local_98);
            if (((char)UE::GC::GIsIncrementalReachabilityPending != '\0') &&
               (pUVar27 != (UObject *)0x0)) {
              UE::GC::MarkAsReachable(pUVar27);
            }
            uVar29 = *(uint *)(this + 0x3ac);
            uVar23 = (ulong)uVar29;
            if (((int)uVar29 < 0) || (*(int *)(this + 0x3a0) <= (int)uVar29)) {
              uVar23 = (ulong)(int)uVar29;
              cVar7 = FDebug::CheckVerifyFailedImpl2
                                ("(Index >= 0) & (Index < ArrayNum)",
                                 "Runtime/Core/Public\\Containers/Array.h",0x42b,
                                 L"Array index out of bounds: %lld into an array of size %lld",
                                 uVar23,(long)*(int *)(this + 0x3a0));
              if (cVar7 != '\0') {
                pcVar3 = (code *)swi(3);
                (*pcVar3)();
                return;
              }
            }
            lVar24 = *(long *)(this + 0x398);
            pUVar11 = pUVar27;
            if (((char)UE::GC::GIsIncrementalReachabilityPending != '\0') &&
               (pUVar11 = (UObject *)0x0, pUVar27 != (UObject *)0x0)) {
              UE::GC::MarkAsReachable(pUVar27);
              pUVar11 = pUVar27;
            }
            *(UObject **)(lVar24 + uVar23 * 0x10 + 8) = pUVar11;
            uVar43 = *(undefined4 *)(this + 0x3a8);
            uVar29 = *(uint *)(this + 0x3ac);
            uVar23 = (ulong)uVar29;
            if (((int)uVar29 < 0) || (*(int *)(this + 0x3a0) <= (int)uVar29)) {
              uVar23 = (ulong)(int)uVar29;
              cVar7 = FDebug::CheckVerifyFailedImpl2
                                ("(Index >= 0) & (Index < ArrayNum)",
                                 "Runtime/Core/Public\\Containers/Array.h",0x42b,
                                 L"Array index out of bounds: %lld into an array of size %lld",
                                 uVar23,(long)*(int *)(this + 0x3a0));
              if (cVar7 != '\0') {
                pcVar3 = (code *)swi(3);
                (*pcVar3)();
                return;
              }
            }
            *(undefined4 *)(*(long *)(this + 0x398) + uVar23 * 0x10) = uVar43;
            iVar9 = *(int *)(this + 0x3ac) + 1;
            if (iVar9 == *(int *)(this + 0x3b4)) {
              iVar9 = 0;
            }
            *(int *)(this + 0x3ac) = iVar9;
            iVar9 = *(int *)(this + 0x470);
            if ((long)iVar9 != 0) {
              lVar24 = *(long *)(this + 0x468);
              lVar16 = 0;
              do {
                pUVar27 = *(UObject **)(lVar24 + lVar16);
                if ((pUVar27 != (UObject *)0x0) &&
                   ((char)UE::GC::GIsIncrementalReachabilityPending != '\0')) {
                  UE::GC::MarkAsReachable(pUVar27);
                }
                if ((pUVar27 != (UObject *)0x0) &&
                   ((char)UE::GC::GIsIncrementalReachabilityPending != '\0')) {
                  UE::GC::MarkAsReachable(pUVar27);
                }
                local_d8 = pUVar27;
                if ((pUVar27 != (UObject *)0x0) &&
                   ((char)UE::GC::GIsIncrementalReachabilityPending != '\0')) {
                  UE::GC::MarkAsReachable(pUVar27);
                }
                pUVar11 = local_d8;
                if (((char)UE::GC::GIsIncrementalReachabilityPending != '\0') &&
                   (local_d8 != (UObject *)0x0)) {
                  UE::GC::MarkAsReachable(local_d8);
                }
                uVar29 = ((uint)((ulong)pUVar11 >> 0x14) & 0xffff ^ (uint)((ulong)pUVar11 >> 4)) *
                         -0x7a143595;
                uVar29 = (uVar29 >> 0xd ^ uVar29) * -0x3d4d51cb;
                iVar10 = FUN_018765e0(pAVar1,uVar29 >> 0x10 ^ uVar29,&local_d8);
                if ((iVar10 == -1) || (lVar19 = (long)iVar10 * 0x18 + *(long *)pAVar1, lVar19 == 0))
                {
                  cVar7 = FDebug::CheckVerifyFailedImpl2
                                    ("Pair != nullptr","Runtime/Core/Public\\Containers/Map.h",0x2d9
                                     ,L"");
                  iVar10 = _DAT_00000008;
                  if (cVar7 != '\0') {
                    pcVar3 = (code *)swi(3);
                    (*pcVar3)();
                    return;
                  }
                }
                else {
                  iVar10 = *(int *)(lVar19 + 8);
                }
                if (iVar10 != -1) {
                  if ((pUVar27 != (UObject *)0x0) &&
                     ((char)UE::GC::GIsIncrementalReachabilityPending != '\0')) {
                    UE::GC::MarkAsReachable(pUVar27);
                  }
                  if ((pUVar27 != (UObject *)0x0) &&
                     ((char)UE::GC::GIsIncrementalReachabilityPending != '\0')) {
                    UE::GC::MarkAsReachable(pUVar27);
                  }
                  local_d8 = pUVar27;
                  if ((pUVar27 != (UObject *)0x0) &&
                     ((char)UE::GC::GIsIncrementalReachabilityPending != '\0')) {
                    UE::GC::MarkAsReachable(pUVar27);
                  }
                  pUVar11 = local_d8;
                  if (((char)UE::GC::GIsIncrementalReachabilityPending != '\0') &&
                     (local_d8 != (UObject *)0x0)) {
                    UE::GC::MarkAsReachable(local_d8);
                  }
                  uVar29 = ((uint)((ulong)pUVar11 >> 0x14) & 0xffff ^ (uint)((ulong)pUVar11 >> 4)) *
                           -0x7a143595;
                  uVar29 = (uVar29 >> 0xd ^ uVar29) * -0x3d4d51cb;
                  iVar10 = FUN_018765e0(pAVar1,uVar29 >> 0x10 ^ uVar29,&local_d8);
                  if ((iVar10 == -1) ||
                     (lVar19 = (long)iVar10 * 0x18 + *(long *)pAVar1, lVar19 == 0)) {
                    cVar7 = FDebug::CheckVerifyFailedImpl2
                                      ("Pair != nullptr","Runtime/Core/Public\\Containers/Map.h",
                                       0x2d9,L"");
                    iVar10 = _DAT_00000008;
                    if (cVar7 != '\0') {
                      pcVar3 = (code *)swi(3);
                      (*pcVar3)();
                      return;
                    }
                  }
                  else {
                    iVar10 = *(int *)(lVar19 + 8);
                  }
                  if (iVar10 != -1) {
                    if ((pUVar27 != (UObject *)0x0) &&
                       ((char)UE::GC::GIsIncrementalReachabilityPending != '\0')) {
                      UE::GC::MarkAsReachable(pUVar27);
                    }
                    if ((pUVar27 != (UObject *)0x0) &&
                       ((char)UE::GC::GIsIncrementalReachabilityPending != '\0')) {
                      UE::GC::MarkAsReachable(pUVar27);
                    }
                    local_d8 = pUVar27;
                    if ((pUVar27 != (UObject *)0x0) &&
                       ((char)UE::GC::GIsIncrementalReachabilityPending != '\0')) {
                      UE::GC::MarkAsReachable(pUVar27);
                    }
                    pUVar11 = local_d8;
                    if (((char)UE::GC::GIsIncrementalReachabilityPending != '\0') &&
                       (local_d8 != (UObject *)0x0)) {
                      UE::GC::MarkAsReachable(local_d8);
                    }
                    uVar29 = ((uint)((ulong)pUVar11 >> 0x14) & 0xffff ^ (uint)((ulong)pUVar11 >> 4))
                             * -0x7a143595;
                    uVar29 = (uVar29 >> 0xd ^ uVar29) * -0x3d4d51cb;
                    iVar10 = FUN_018765e0(pAVar1,uVar29 >> 0x10 ^ uVar29,&local_d8);
                    if ((iVar10 == -1) ||
                       (lVar19 = (long)iVar10 * 0x18 + *(long *)pAVar1, lVar19 == 0)) {
                      cVar7 = FDebug::CheckVerifyFailedImpl2
                                        ("Pair != nullptr","Runtime/Core/Public\\Containers/Map.h",
                                         0x2d9,L"");
                      if (cVar7 != '\0') {
                        pcVar3 = (code *)swi(3);
                        (*pcVar3)();
                        return;
                      }
                      lVar19 = 0;
                    }
                    iVar10 = *(int *)(this + 0x3ac) + 1;
                    if (iVar10 == *(int *)(this + 0x3b4)) {
                      iVar10 = 0;
                    }
                    if (*(int *)(lVar19 + 8) == iVar10) {
                      if ((pUVar27 != (UObject *)0x0) &&
                         ((char)UE::GC::GIsIncrementalReachabilityPending != '\0')) {
                        UE::GC::MarkAsReachable(pUVar27);
                      }
                      if ((pUVar27 != (UObject *)0x0) &&
                         ((char)UE::GC::GIsIncrementalReachabilityPending != '\0')) {
                        UE::GC::MarkAsReachable(pUVar27);
                      }
                      local_d8 = pUVar27;
                      if ((pUVar27 != (UObject *)0x0) &&
                         ((char)UE::GC::GIsIncrementalReachabilityPending != '\0')) {
                        UE::GC::MarkAsReachable(pUVar27);
                      }
                      pUVar27 = local_d8;
                      if (((char)UE::GC::GIsIncrementalReachabilityPending != '\0') &&
                         (local_d8 != (UObject *)0x0)) {
                        UE::GC::MarkAsReachable(local_d8);
                      }
                      uVar29 = ((uint)((ulong)pUVar27 >> 0x14) & 0xffff ^
                               (uint)((ulong)pUVar27 >> 4)) * -0x7a143595;
                      uVar29 = (uVar29 >> 0xd ^ uVar29) * -0x3d4d51cb;
                      iVar10 = FUN_018765e0(pAVar1,uVar29 >> 0x10 ^ uVar29,&local_d8);
                      if ((iVar10 == -1) ||
                         (lVar19 = (long)iVar10 * 0x18 + *(long *)pAVar1, lVar19 == 0)) {
                        cVar7 = FDebug::CheckVerifyFailedImpl2
                                          ("Pair != nullptr","Runtime/Core/Public\\Containers/Map.h"
                                           ,0x2d9,L"");
                        if (cVar7 != '\0') {
                          pcVar3 = (code *)swi(3);
                          (*pcVar3)();
                          return;
                        }
                        lVar19 = 0;
                      }
                      *(undefined4 *)(lVar19 + 8) = 0xffffffff;
                    }
                  }
                }
                lVar16 = lVar16 + 8;
              } while ((long)iVar9 << 3 != lVar16);
            }
          }
          fVar31 = (float)GetAvailableSpace(this);
        } while (120.0 < fVar31);
      }
      FFGDynamicStruct::~FFGDynamicStruct(local_90);
    }
  }
  else {
    if (this[0x494] != (AFGConveyorChainActor)0x0) {
      local_a0 = (UObject *)0x0;
      FUN_014238c0(this + 0x478,&local_a0);
      iVar9 = *(int *)(this + 0x2d8);
      if ((long)iVar9 != 0) {
        lVar24 = *(long *)(this + 0x2d0);
        lVar16 = 0;
        do {
          *(undefined8 *)(lVar24 + 8 + lVar16) = 0;
          lVar16 = lVar16 + 0x40;
        } while ((long)iVar9 * 0x40 != lVar16);
      }
      if ((long)*(int *)(this + 0x480) == 0) {
        bVar14 = 1;
      }
      else {
        lVar24 = *(long *)(this + 0x478);
        lVar16 = (long)*(int *)(this + 0x480) << 3;
        lVar19 = 0;
        bVar6 = 1;
        do {
          while( true ) {
            bVar14 = bVar6;
            pUVar27 = *(UObject **)(lVar24 + lVar19);
            uVar29 = *(uint *)(pUVar27 + 0x6e0);
            uVar23 = (ulong)uVar29;
            if (((int)uVar29 < 0) || (*(int *)(this + 0x2d8) <= (int)uVar29)) break;
            lVar18 = *(long *)(this + 0x2d0);
            if ((pUVar27 != (UObject *)0x0) &&
               ((char)UE::GC::GIsIncrementalReachabilityPending != '\0')) {
              UE::GC::MarkAsReachable(pUVar27);
            }
            lVar26 = uVar23 * 0x40;
            *(UObject **)(lVar26 + lVar18 + 8) = pUVar27;
            if ((*(int *)(this + 0x2d8) <= (int)uVar29) &&
               (cVar7 = FDebug::CheckVerifyFailedImpl2
                                  ("(Index >= 0) & (Index < ArrayNum)",
                                   "Runtime/Core/Public\\Containers/Array.h",0x42b,
                                   L"Array index out of bounds: %lld into an array of size %lld",
                                   uVar23), cVar7 != '\0')) {
              pcVar3 = (code *)swi(3);
              (*pcVar3)();
              return;
            }
            if (0 < *(int *)(*(long *)(*(long *)(this + 0x2d0) + 8 + lVar26) + 0x6d0)) {
              local_40 = (undefined ***)0x0;
              local_58 = &PTR_FUN_02668a90;
              local_68[0] = FUN_01ab3260;
              local_50 = this;
              local_48 = uVar29;
              FUN_017dfcb0(&local_78,local_68,0,1);
              if (local_78 != (long *)0x0) {
                LOCK();
                plVar25 = local_78 + 1;
                *(int *)plVar25 = (int)*plVar25 + -1;
                UNLOCK();
                if ((int)*plVar25 == 0) {
                  (**(code **)(*local_78 + 8))();
                }
              }
              if (local_68[0] != (code *)0x0) {
                pppuVar17 = local_40;
                if (local_40 == (undefined ***)0x0) {
                  pppuVar17 = &local_58;
                }
                (*(code *)(*pppuVar17)[2])();
              }
            }
            if ((*(int *)(this + 0x2d8) <= (int)uVar29) &&
               (cVar7 = FDebug::CheckVerifyFailedImpl2
                                  ("(Index >= 0) & (Index < ArrayNum)",
                                   "Runtime/Core/Public\\Containers/Array.h",0x42b,
                                   L"Array index out of bounds: %lld into an array of size %lld",
                                   uVar23), cVar7 != '\0')) {
              pcVar3 = (code *)swi(3);
              (*pcVar3)();
              return;
            }
            lVar18 = *(long *)(*(long *)(this + 0x2d0) + 8 + lVar26);
            FUN_0143e690(*(undefined8 *)(lVar18 + 0x680),*(undefined4 *)(lVar18 + 0x688));
            *(undefined4 *)(lVar18 + 0x688) = 0;
            if (*(int *)(lVar18 + 0x68c) != 0) {
              FUN_013e3f40(0x38,8,0,lVar18 + 0x680,0,lVar18 + 0x68c);
            }
            lVar19 = lVar19 + 8;
            bVar6 = bVar14;
            if (lVar16 == lVar19) goto LAB_01a867dd;
          }
          bVar14 = 0;
          lVar19 = lVar19 + 8;
          bVar6 = 0;
        } while (lVar16 != lVar19);
      }
LAB_01a867dd:
      this[0x494] = (AFGConveyorChainActor)(~bVar14 & 1);
    }
    HandleClientAdditions(this);
    HandleClientRemovals(this);
  }
  uVar29 = *(uint *)(this + 0x364);
  uVar23 = (ulong)uVar29;
  if (uVar23 == 0xffffffff) {
    if (this[0x4b0] == (AFGConveyorChainActor)0x0) {
      return;
    }
    UpdateAttachedMonitors(this,param_1);
    return;
  }
  uVar20 = (ulong)(int)uVar29;
  fVar31 = 0.0;
  if ((this[0x170] != (AFGConveyorChainActor)0x3) && (DAT_026eab4c != '\0')) {
    if ((((int)uVar29 < 0) || (uVar15 = uVar23, *(int *)(this + 0x338) <= (int)uVar29)) &&
       (cVar7 = FDebug::CheckVerifyFailedImpl2
                          ("(Index >= 0) & (Index < ArrayNum)",
                           "Runtime/Core/Public\\Containers/Array.h",0x42b,
                           L"Array index out of bounds: %lld into an array of size %lld",uVar20,
                           (long)*(int *)(this + 0x338)), uVar15 = uVar20, cVar7 != '\0')) {
      pcVar3 = (code *)swi(3);
      (*pcVar3)();
      return;
    }
    fVar32 = *(float *)(*(long *)(this + 0x330) + 0x30 + uVar15 * 0x38);
    fVar31 = 0.0;
    if ((fVar32 != 0.0) || (NAN(fVar32))) {
      pfVar12 = (float *)(*(long *)(this + 0x330) + uVar15 * 0x38 + 0x30);
      fVar36 = -fVar32;
      fVar31 = (float)(-(uint)(fVar36 < fVar32) & 0x3f800000 |
                      ~-(uint)(fVar36 < fVar32) & -(uint)(fVar32 < fVar36) & 0xbf800000);
      fVar35 = fVar32 * fVar31;
      if (param_1 < fVar35) {
        fVar32 = fVar32 - fVar31 * param_1;
      }
      else {
        fVar32 = 0.0;
      }
      uVar30 = -(uint)(param_1 < fVar35);
      *pfVar12 = fVar32;
      fVar31 = (float)(~uVar30 & (uint)fVar36 | uVar30 & (uint)param_1);
      if (ABS(fVar32) <= 0.001) {
        *pfVar12 = 0.0;
      }
    }
  }
  if (*(int *)(this + 0x2d8) < 1) {
    cVar7 = FDebug::CheckVerifyFailedImpl2
                      ("(Index >= 0) & (Index < ArrayNum)","Runtime/Core/Public\\Containers/Array.h"
                       ,0x42b,L"Array index out of bounds: %lld into an array of size %lld",0);
    if (cVar7 != '\0') {
      pcVar3 = (code *)swi(3);
      (*pcVar3)();
      return;
    }
    fVar32 = *(float *)(*(long *)(this + 0x2d0) + 0x30);
    if ((*(int *)(this + 0x2d8) < 1) &&
       (cVar7 = FDebug::CheckVerifyFailedImpl2
                          ("(Index >= 0) & (Index < ArrayNum)",
                           "Runtime/Core/Public\\Containers/Array.h",0x42b,
                           L"Array index out of bounds: %lld into an array of size %lld",0),
       cVar7 != '\0')) {
      pcVar3 = (code *)swi(3);
      (*pcVar3)();
      return;
    }
    lVar24 = *(long *)(this + 0x2d0);
    iVar9 = *(int *)(lVar24 + 0x34);
    if (iVar9 == -1) goto LAB_01a86a1a;
LAB_01a86954:
    iVar10 = *(int *)(lVar24 + 0x38) - iVar9;
    if (*(int *)(lVar24 + 0x38) < iVar9) {
      iVar10 = iVar10 + *(int *)(this + 0x338);
    }
    uVar15 = 0;
    uVar30 = *(uint *)(this + 0x2d8);
    if (0 < iVar10 + 1) goto LAB_01a86a86;
  }
  else {
    fVar32 = *(float *)(*(long *)(this + 0x2d0) + 0x30);
    lVar24 = *(long *)(this + 0x2d0);
    iVar9 = *(int *)(lVar24 + 0x34);
    if (iVar9 != -1) goto LAB_01a86954;
LAB_01a86a1a:
    uVar30 = *(uint *)(this + 0x2d8);
  }
  uVar15 = 0;
  if (0 < (int)uVar30) {
    uVar15 = 0xffffffff;
    lVar16 = 0;
    do {
      iVar9 = *(int *)(lVar24 + 0x34 + lVar16);
      iVar10 = 0;
      if (iVar9 != -1) {
        iVar22 = *(int *)(lVar24 + 0x38 + lVar16);
        iVar10 = iVar22 - iVar9;
        if (iVar22 < iVar9) {
          iVar10 = iVar10 + *(int *)(this + 0x338) + 1;
        }
        else {
          iVar10 = iVar10 + 1;
        }
      }
      uVar15 = (ulong)((int)uVar15 + 1);
    } while ((iVar10 < 1) &&
            (bVar28 = (ulong)uVar30 * 0x40 + -0x40 != lVar16, lVar16 = lVar16 + 0x40, bVar28));
  }
LAB_01a86a86:
  iVar9 = (int)uVar15;
  if (((int)uVar30 <= iVar9) &&
     (cVar7 = FDebug::CheckVerifyFailedImpl2
                        ("(Index >= 0) & (Index < ArrayNum)",
                         "Runtime/Core/Public\\Containers/Array.h",0x42b,
                         L"Array index out of bounds: %lld into an array of size %lld",uVar15,
                         (long)(int)uVar30), cVar7 != '\0')) {
    pcVar3 = (code *)swi(3);
    (*pcVar3)();
    return;
  }
  lVar24 = uVar15 * 0x40;
  fVar36 = *(float *)(*(long *)(this + 0x2d0) + 0x30 + lVar24);
  if ((((int)uVar29 < 0) || (*(int *)(this + 0x338) <= (int)uVar29)) &&
     (cVar7 = FDebug::CheckVerifyFailedImpl2
                        ("(Index >= 0) & (Index < ArrayNum)",
                         "Runtime/Core/Public\\Containers/Array.h",0x42b,
                         L"Array index out of bounds: %lld into an array of size %lld",uVar20,
                         (long)*(int *)(this + 0x338)), uVar23 = uVar20, cVar7 != '\0')) {
    pcVar3 = (code *)swi(3);
    (*pcVar3)();
    return;
  }
  lVar16 = *(long *)(this + 0x330);
  lVar19 = uVar23 * 0x38;
  fVar35 = *(float *)(lVar16 + 0x28 + lVar19);
  fVar36 = (float)(~-(uint)(*(float *)(this + 0x360) < fVar35) & (uint)(fVar36 * param_1) |
                  -(uint)(*(float *)(this + 0x360) < fVar35) & 0x80000000) + fVar35;
  *(float *)(lVar16 + 0x28 + lVar19) = fVar36;
  if ((*(int *)(this + 0x2d8) <= iVar9) &&
     (cVar7 = FDebug::CheckVerifyFailedImpl2
                        ("(Index >= 0) & (Index < ArrayNum)",
                         "Runtime/Core/Public\\Containers/Array.h",0x42b,
                         L"Array index out of bounds: %lld into an array of size %lld"),
     cVar7 != '\0')) {
    pcVar3 = (code *)swi(3);
    (*pcVar3)();
    return;
  }
  fVar41 = *(float *)(*(long *)(this + 0x2d0) + 0x24 + lVar24);
  if (*(int *)(this + 0x2d8) <= iVar9) {
    local_c8._0_4_ = fVar41;
    cVar7 = FDebug::CheckVerifyFailedImpl2
                      ("(Index >= 0) & (Index < ArrayNum)","Runtime/Core/Public\\Containers/Array.h"
                       ,0x42b,L"Array index out of bounds: %lld into an array of size %lld",uVar15);
    fVar41 = (float)local_c8._0_4_;
    if (cVar7 != '\0') {
      pcVar3 = (code *)swi(3);
      (*pcVar3)();
      return;
    }
  }
  *(float *)(lVar16 + 0x2c + lVar19) =
       fVar36 - (fVar41 + *(float *)(*(long *)(this + 0x2d0) + 0x20 + lVar24));
  fVar36 = *(float *)(lVar19 + lVar16 + 0x28);
  uVar43 = 0;
  uVar44 = 0;
  uVar45 = 0;
  if (cVar8 == '\0') {
    fVar41 = 0.0;
  }
  else {
    fVar41 = 1.0;
  }
  *(float *)(lVar16 + 0x34 + lVar19) =
       (*(float *)(lVar16 + 0x34 + lVar19) * fVar41 + fVar36) - fVar35;
  uVar23 = (ulong)(uVar29 + 1);
  if (uVar29 + 1 == *(uint *)(this + 0x488)) {
    uVar23 = 0;
  }
  if (this[0x170] == (AFGConveyorChainActor)0x3) {
    iVar9 = 1;
    do {
      if (((long)*(int *)(this + 0x2d8) <= (long)uVar15) &&
         (cVar8 = FDebug::CheckVerifyFailedImpl2
                            ("(Index >= 0) & (Index < ArrayNum)",
                             "Runtime/Core/Public\\Containers/Array.h",0x42b,
                             L"Array index out of bounds: %lld into an array of size %lld"),
         cVar8 != '\0')) {
        pcVar3 = (code *)swi(3);
        (*pcVar3)();
        return;
      }
      lVar24 = uVar15 * 0x40;
      fVar35 = *(float *)(*(long *)(this + 0x2d0) + 0x30 + lVar24);
      if (((long)*(int *)(this + 0x2d8) <= (long)uVar15) &&
         (cVar8 = FDebug::CheckVerifyFailedImpl2
                            ("(Index >= 0) & (Index < ArrayNum)",
                             "Runtime/Core/Public\\Containers/Array.h",0x42b,
                             L"Array index out of bounds: %lld into an array of size %lld"),
         cVar8 != '\0')) {
        pcVar3 = (code *)swi(3);
        (*pcVar3)();
        return;
      }
      iVar10 = *(int *)(*(long *)(this + 0x2d0) + 0x34 + lVar24);
      if (iVar10 == -1) {
        iVar22 = 0;
        iVar10 = *(int *)(this + 0x2d8);
      }
      else {
        iVar2 = *(int *)(*(long *)(this + 0x2d0) + 0x38 + lVar24);
        iVar22 = iVar2 - iVar10;
        if (iVar2 < iVar10) {
          iVar22 = iVar22 + *(int *)(this + 0x338) + 1;
          iVar10 = *(int *)(this + 0x2d8);
        }
        else {
          iVar22 = iVar22 + 1;
          iVar10 = *(int *)(this + 0x2d8);
        }
      }
      if (((long)iVar10 <= (long)uVar15) &&
         (cVar8 = FDebug::CheckVerifyFailedImpl2
                            ("(Index >= 0) & (Index < ArrayNum)",
                             "Runtime/Core/Public\\Containers/Array.h",0x42b,
                             L"Array index out of bounds: %lld into an array of size %lld"),
         cVar8 != '\0')) {
        pcVar3 = (code *)swi(3);
        (*pcVar3)();
        return;
      }
      fVar33 = *(float *)(*(long *)(this + 0x2d0) + 0x24 + lVar24);
      if (((long)*(int *)(this + 0x2d8) <= (long)uVar15) &&
         (cVar8 = FDebug::CheckVerifyFailedImpl2
                            ("(Index >= 0) & (Index < ArrayNum)",
                             "Runtime/Core/Public\\Containers/Array.h",0x42b,
                             L"Array index out of bounds: %lld into an array of size %lld"),
         cVar8 != '\0')) {
        pcVar3 = (code *)swi(3);
        (*pcVar3)();
        return;
      }
      iVar10 = iVar22 - iVar9;
      if (iVar10 != 0 && iVar9 <= iVar22) {
        fVar37 = *(float *)(*(long *)(this + 0x2d0) + 0x20 + lVar24);
        do {
          iVar9 = (int)uVar23;
          if ((iVar9 < 0) || (*(int *)(this + 0x338) <= iVar9)) {
            uVar23 = (ulong)iVar9;
            cVar8 = FDebug::CheckVerifyFailedImpl2
                              ("(Index >= 0) & (Index < ArrayNum)",
                               "Runtime/Core/Public\\Containers/Array.h",0x42b,
                               L"Array index out of bounds: %lld into an array of size %lld",uVar23,
                               (long)*(int *)(this + 0x338));
            if (cVar8 != '\0') {
              pcVar3 = (code *)swi(3);
              (*pcVar3)();
              return;
            }
          }
          lVar16 = *(long *)(this + 0x330);
          lVar19 = uVar23 * 0x38;
          fVar34 = *(float *)(lVar16 + 0x28 + lVar19);
          fVar46 = fVar35 * param_1 + fVar34 + fVar31 * fVar32;
          fVar42 = fVar36 + -120.0;
          fVar36 = fVar46;
          if (fVar42 <= fVar46) {
            fVar36 = fVar42;
          }
          *(float *)(lVar16 + 0x28 + lVar19) = fVar36;
          *(float *)(lVar16 + 0x2c + lVar19) = fVar36 - (fVar33 + fVar37);
          *(float *)(lVar16 + 0x34 + lVar19) =
               (*(float *)(lVar16 + 0x34 + lVar19) * fVar41 + fVar36) - fVar34;
          uVar23 = (ulong)(iVar9 + 1U);
          if (iVar9 + 1U == *(uint *)(this + 0x488)) {
            uVar23 = 0;
          }
          iVar10 = iVar10 + -1;
        } while (iVar10 != 0);
      }
      if (0 < iVar22) {
        if (((long)*(int *)(this + 0x2d8) <= (long)uVar15) &&
           (cVar8 = FDebug::CheckVerifyFailedImpl2
                              ("(Index >= 0) & (Index < ArrayNum)",
                               "Runtime/Core/Public\\Containers/Array.h",0x42b,
                               L"Array index out of bounds: %lld into an array of size %lld"),
           cVar8 != '\0')) {
          pcVar3 = (code *)swi(3);
          (*pcVar3)();
          return;
        }
        fVar36 = fVar36 - *(float *)(*(long *)(this + 0x2d0) + 0x20 + lVar24);
      }
      uVar15 = uVar15 + 1;
      iVar10 = *(int *)(this + 0x490);
      iVar9 = 0;
    } while ((int)uVar15 < iVar10);
  }
  else {
    iVar9 = 1;
    do {
      if ((long)*(int *)(this + 0x2d8) <= (long)uVar15) {
        cVar8 = FDebug::CheckVerifyFailedImpl2
                          ("(Index >= 0) & (Index < ArrayNum)",
                           "Runtime/Core/Public\\Containers/Array.h",0x42b,
                           L"Array index out of bounds: %lld into an array of size %lld");
        uVar43 = 0;
        uVar44 = 0;
        uVar45 = 0;
        if (cVar8 != '\0') {
          pcVar3 = (code *)swi(3);
          (*pcVar3)();
          return;
        }
      }
      lVar24 = uVar15 * 0x40;
      fVar31 = *(float *)(*(long *)(this + 0x2d0) + 0x30 + lVar24);
      if ((long)*(int *)(this + 0x2d8) <= (long)uVar15) {
        cVar8 = FDebug::CheckVerifyFailedImpl2
                          ("(Index >= 0) & (Index < ArrayNum)",
                           "Runtime/Core/Public\\Containers/Array.h",0x42b,
                           L"Array index out of bounds: %lld into an array of size %lld");
        uVar43 = 0;
        uVar44 = 0;
        uVar45 = 0;
        if (cVar8 != '\0') {
          pcVar3 = (code *)swi(3);
          (*pcVar3)();
          return;
        }
      }
      iVar10 = *(int *)(*(long *)(this + 0x2d0) + 0x34 + lVar24);
      if (iVar10 == -1) {
        iVar22 = 0;
        iVar10 = *(int *)(this + 0x2d8);
      }
      else {
        iVar2 = *(int *)(*(long *)(this + 0x2d0) + 0x38 + lVar24);
        iVar22 = iVar2 - iVar10;
        if (iVar2 < iVar10) {
          iVar22 = iVar22 + *(int *)(this + 0x338) + 1;
          iVar10 = *(int *)(this + 0x2d8);
        }
        else {
          iVar22 = iVar22 + 1;
          iVar10 = *(int *)(this + 0x2d8);
        }
      }
      if ((long)iVar10 <= (long)uVar15) {
        cVar8 = FDebug::CheckVerifyFailedImpl2
                          ("(Index >= 0) & (Index < ArrayNum)",
                           "Runtime/Core/Public\\Containers/Array.h",0x42b,
                           L"Array index out of bounds: %lld into an array of size %lld");
        uVar43 = 0;
        uVar44 = 0;
        uVar45 = 0;
        if (cVar8 != '\0') {
          pcVar3 = (code *)swi(3);
          (*pcVar3)();
          return;
        }
      }
      fVar32 = *(float *)(*(long *)(this + 0x2d0) + 0x24 + lVar24);
      if ((long)*(int *)(this + 0x2d8) <= (long)uVar15) {
        cVar8 = FDebug::CheckVerifyFailedImpl2
                          ("(Index >= 0) & (Index < ArrayNum)",
                           "Runtime/Core/Public\\Containers/Array.h",0x42b,
                           L"Array index out of bounds: %lld into an array of size %lld");
        uVar43 = 0;
        uVar44 = 0;
        uVar45 = 0;
        if (cVar8 != '\0') {
          pcVar3 = (code *)swi(3);
          (*pcVar3)();
          return;
        }
      }
      iVar10 = iVar22 - iVar9;
      if (iVar10 == 0 || iVar22 < iVar9) {
        auVar38._4_4_ = uVar43;
        auVar38._0_4_ = fVar36;
        auVar38._8_4_ = uVar44;
        auVar38._12_4_ = uVar45;
      }
      else {
        fVar35 = *(float *)(*(long *)(this + 0x2d0) + 0x20 + lVar24);
        do {
          iVar9 = (int)uVar23;
          if (DAT_026eab4c == '\0') {
            auVar39 = ZEXT816(0);
          }
          else {
            if ((iVar9 < 0) || (uVar20 = uVar23, *(int *)(this + 0x338) <= iVar9)) {
              cVar8 = FDebug::CheckVerifyFailedImpl2
                                ("(Index >= 0) & (Index < ArrayNum)",
                                 "Runtime/Core/Public\\Containers/Array.h",0x42b,
                                 L"Array index out of bounds: %lld into an array of size %lld",
                                 (long)iVar9,(long)*(int *)(this + 0x338));
              uVar20 = (long)iVar9;
              if (cVar8 != '\0') {
                pcVar3 = (code *)swi(3);
                (*pcVar3)();
                return;
              }
            }
            fVar33 = *(float *)(*(long *)(this + 0x330) + 0x30 + uVar20 * 0x38);
            auVar39 = ZEXT816(0);
            if ((fVar33 != 0.0) || (NAN(fVar33))) {
              fVar34 = -fVar33;
              fVar37 = (float)(-(uint)(fVar34 < fVar33) & 0x3f800000 |
                              ~-(uint)(fVar34 < fVar33) & -(uint)(fVar33 < fVar34) & 0xbf800000);
              fVar42 = fVar33 * fVar37;
              if (param_1 < fVar42) {
                fVar33 = fVar33 - fVar37 * param_1;
              }
              else {
                fVar33 = 0.0;
              }
              pfVar12 = (float *)(*(long *)(this + 0x330) + uVar20 * 0x38 + 0x30);
              uVar29 = -(uint)(param_1 < fVar42);
              auVar39._0_8_ = CONCAT44(~in_XMM0_Db,~uVar29 & (uint)fVar34) & 0x80000000ffffffff;
              auVar39._8_4_ = ~in_XMM0_Dc & 0x80000000;
              auVar39._12_4_ = ~in_XMM0_Dd & 0x80000000;
              auVar4._4_4_ = in_XMM0_Db;
              auVar4._0_4_ = uVar29 & (uint)param_1;
              auVar4._8_4_ = in_XMM0_Dc;
              auVar4._12_4_ = in_XMM0_Dd;
              auVar39 = auVar39 | auVar4;
              *pfVar12 = fVar33;
              if (ABS(fVar33) <= 0.001) {
                *pfVar12 = 0.0;
              }
            }
          }
          if (((long)*(int *)(this + 0x2d8) <= (long)uVar15) &&
             (local_c8 = auVar39,
             cVar8 = FDebug::CheckVerifyFailedImpl2
                               ("(Index >= 0) & (Index < ArrayNum)",
                                "Runtime/Core/Public\\Containers/Array.h",0x42b,
                                L"Array index out of bounds: %lld into an array of size %lld"),
             auVar39 = local_c8, cVar8 != '\0')) {
            pcVar3 = (code *)swi(3);
            (*pcVar3)();
            return;
          }
          auVar40._4_12_ = auVar39._4_12_;
          auVar40._0_4_ = auVar39._0_4_ * *(float *)(*(long *)(this + 0x2d0) + 0x30 + lVar24);
          if ((iVar9 < 0) || (*(int *)(this + 0x338) <= iVar9)) {
            uVar23 = (ulong)iVar9;
            local_c8 = auVar40;
            cVar8 = FDebug::CheckVerifyFailedImpl2
                              ("(Index >= 0) & (Index < ArrayNum)",
                               "Runtime/Core/Public\\Containers/Array.h",0x42b,
                               L"Array index out of bounds: %lld into an array of size %lld",uVar23,
                               (long)*(int *)(this + 0x338));
            auVar40 = local_c8;
            if (cVar8 != '\0') {
              pcVar3 = (code *)swi(3);
              (*pcVar3)();
              return;
            }
          }
          lVar16 = *(long *)(this + 0x330);
          lVar19 = uVar23 * 0x38;
          fVar33 = *(float *)(lVar16 + 0x28 + lVar19);
          auVar38._4_12_ = auVar40._4_12_;
          fVar37 = auVar40._0_4_ + fVar31 * param_1 + fVar33;
          fVar34 = fVar36 + -120.0;
          fVar36 = fVar37;
          if (fVar34 <= fVar37) {
            fVar36 = fVar34;
          }
          auVar38._0_4_ = fVar36;
          *(float *)(lVar16 + 0x28 + lVar19) = fVar36;
          *(float *)(lVar16 + 0x2c + lVar19) = fVar36 - (fVar32 + fVar35);
          *(float *)(lVar16 + 0x34 + lVar19) =
               (*(float *)(lVar16 + 0x34 + lVar19) * fVar41 + fVar36) - fVar33;
          uVar23 = (ulong)(iVar9 + 1U);
          if (iVar9 + 1U == *(uint *)(this + 0x488)) {
            uVar23 = 0;
          }
          iVar10 = iVar10 + -1;
        } while (iVar10 != 0);
      }
      if (0 < iVar22) {
        if (((long)*(int *)(this + 0x2d8) <= (long)uVar15) &&
           (local_c8 = auVar38,
           cVar8 = FDebug::CheckVerifyFailedImpl2
                             ("(Index >= 0) & (Index < ArrayNum)",
                              "Runtime/Core/Public\\Containers/Array.h",0x42b,
                              L"Array index out of bounds: %lld into an array of size %lld"),
           auVar38 = local_c8, cVar8 != '\0')) {
          pcVar3 = (code *)swi(3);
          (*pcVar3)();
          return;
        }
        auVar38._0_4_ = auVar38._0_4_ - *(float *)(*(long *)(this + 0x2d0) + 0x20 + lVar24);
      }
      uVar15 = uVar15 + 1;
      iVar10 = *(int *)(this + 0x490);
      iVar9 = 0;
      fVar36 = auVar38._0_4_;
      uVar43 = auVar38._4_4_;
      uVar44 = auVar38._8_4_;
      uVar45 = auVar38._12_4_;
    } while ((int)uVar15 < iVar10);
  }
  if (1 < iVar10) {
    lVar16 = 0;
    lVar24 = 0;
    do {
      lVar19 = lVar24 + 1;
      if ((*(int *)(this + 0x2d8) <= lVar19) &&
         (cVar8 = FDebug::CheckVerifyFailedImpl2
                            ("(Index >= 0) & (Index < ArrayNum)",
                             "Runtime/Core/Public\\Containers/Array.h",0x42b,
                             L"Array index out of bounds: %lld into an array of size %lld",lVar19),
         cVar8 != '\0')) {
        pcVar3 = (code *)swi(3);
        (*pcVar3)();
        return;
      }
      if (*(int *)(*(long *)(this + 0x2d0) + 0x74 + lVar16) != -1) {
        if ((*(int *)(this + 0x2d8) <= lVar19) &&
           (cVar8 = FDebug::CheckVerifyFailedImpl2
                              ("(Index >= 0) & (Index < ArrayNum)",
                               "Runtime/Core/Public\\Containers/Array.h",0x42b,
                               L"Array index out of bounds: %lld into an array of size %lld",lVar19)
           , cVar8 != '\0')) {
          pcVar3 = (code *)swi(3);
          (*pcVar3)();
          return;
        }
        uVar29 = *(uint *)(*(long *)(this + 0x2d0) + 0x74 + lVar16);
        uVar23 = (ulong)uVar29;
        if (((int)uVar29 < 0) || (*(int *)(this + 0x338) <= (int)uVar29)) {
          uVar23 = (ulong)(int)uVar29;
          cVar8 = FDebug::CheckVerifyFailedImpl2
                            ("(Index >= 0) & (Index < ArrayNum)",
                             "Runtime/Core/Public\\Containers/Array.h",0x42b,
                             L"Array index out of bounds: %lld into an array of size %lld",uVar23,
                             (long)*(int *)(this + 0x338));
          if (cVar8 != '\0') {
            pcVar3 = (code *)swi(3);
            (*pcVar3)();
            return;
          }
        }
        fVar31 = *(float *)(*(long *)(this + 0x330) + 0x28 + uVar23 * 0x38);
        if ((*(int *)(this + 0x2d8) <= lVar19) &&
           (cVar8 = FDebug::CheckVerifyFailedImpl2
                              ("(Index >= 0) & (Index < ArrayNum)",
                               "Runtime/Core/Public\\Containers/Array.h",0x42b,
                               L"Array index out of bounds: %lld into an array of size %lld",lVar19)
           , cVar8 != '\0')) {
          pcVar3 = (code *)swi(3);
          (*pcVar3)();
          return;
        }
        pfVar12 = (float *)(*(long *)(this + 0x2d0) + 0x68 + lVar16);
        if (*pfVar12 <= fVar31 && fVar31 != *pfVar12) {
          if ((*(int *)(this + 0x2d8) <= lVar19) &&
             (cVar8 = FDebug::CheckVerifyFailedImpl2
                                ("(Index >= 0) & (Index < ArrayNum)",
                                 "Runtime/Core/Public\\Containers/Array.h",0x42b,
                                 L"Array index out of bounds: %lld into an array of size %lld",
                                 lVar19), cVar8 != '\0')) {
            pcVar3 = (code *)swi(3);
            (*pcVar3)();
            return;
          }
          uVar29 = *(uint *)(*(long *)(this + 0x2d0) + 0x74 + lVar16);
          uVar23 = (ulong)uVar29;
          if (((int)uVar29 < 0) || (*(int *)(this + 0x338) <= (int)uVar29)) {
            uVar23 = (ulong)(int)uVar29;
            cVar8 = FDebug::CheckVerifyFailedImpl2
                              ("(Index >= 0) & (Index < ArrayNum)",
                               "Runtime/Core/Public\\Containers/Array.h",0x42b,
                               L"Array index out of bounds: %lld into an array of size %lld",uVar23,
                               (long)*(int *)(this + 0x338));
            if (cVar8 != '\0') {
              pcVar3 = (code *)swi(3);
              (*pcVar3)();
              return;
            }
          }
          *(undefined4 *)(*(long *)(this + 0x330) + 0x2c + uVar23 * 0x38) = 0;
          if ((*(int *)(this + 0x2d8) < lVar19) &&
             (cVar8 = FDebug::CheckVerifyFailedImpl2
                                ("(Index >= 0) & (Index < ArrayNum)",
                                 "Runtime/Core/Public\\Containers/Array.h",0x42b,
                                 L"Array index out of bounds: %lld into an array of size %lld"),
             cVar8 != '\0')) {
            pcVar3 = (code *)swi(3);
            (*pcVar3)();
            return;
          }
          lVar18 = *(long *)(this + 0x2d0);
          if ((*(int *)(this + 0x2d8) <= lVar19) &&
             (cVar8 = FDebug::CheckVerifyFailedImpl2
                                ("(Index >= 0) & (Index < ArrayNum)",
                                 "Runtime/Core/Public\\Containers/Array.h",0x42b,
                                 L"Array index out of bounds: %lld into an array of size %lld",
                                 lVar19), cVar8 != '\0')) {
            pcVar3 = (code *)swi(3);
            (*pcVar3)();
            return;
          }
          uVar29 = *(uint *)(*(long *)(this + 0x2d0) + 0x74 + lVar16);
          uVar23 = (ulong)(int)uVar29;
          lVar26 = *(long *)(lVar18 + lVar16);
          fVar31 = *(float *)(lVar18 + 0x20 + lVar16);
          iVar9 = *(int *)(lVar26 + 0x338);
          if (((long)uVar23 < 0) || (iVar9 <= (int)uVar29)) {
            cVar8 = FDebug::CheckVerifyFailedImpl2
                              ("(Index >= 0) & (Index < ArrayNum)",
                               "Runtime/Core/Public\\Containers/Array.h",0x42b,
                               L"Array index out of bounds: %lld into an array of size %lld",uVar23,
                               (long)iVar9);
            if (cVar8 != '\0') {
              pcVar3 = (code *)swi(3);
              (*pcVar3)();
              return;
            }
          }
          else {
            uVar23 = (ulong)uVar29;
          }
          lVar26 = *(long *)(lVar26 + 0x330);
          *(float *)(lVar26 + 0x28 + uVar23 * 0x38) =
               fVar31 + *(float *)(lVar26 + 0x28 + uVar23 * 0x38);
          if (*(int *)(lVar18 + 0x34 + lVar16) == -1) {
            *(uint *)(lVar18 + 0x34 + lVar16) = uVar29;
          }
          *(uint *)(lVar18 + 0x38 + lVar16) = uVar29;
          if ((*(int *)(this + 0x2d8) <= lVar19) &&
             (cVar8 = FDebug::CheckVerifyFailedImpl2
                                ("(Index >= 0) & (Index < ArrayNum)",
                                 "Runtime/Core/Public\\Containers/Array.h",0x42b,
                                 L"Array index out of bounds: %lld into an array of size %lld",
                                 lVar19), cVar8 != '\0')) {
            pcVar3 = (code *)swi(3);
            (*pcVar3)();
            return;
          }
          lVar18 = *(long *)(this + 0x2d0);
          iVar9 = *(int *)(lVar18 + 0x74 + lVar16);
          if (iVar9 == *(int *)(lVar18 + 0x78 + lVar16)) {
            *(undefined4 *)(lVar18 + 0x78 + lVar16) = 0xffffffff;
            iVar9 = -1;
          }
          else {
            iVar9 = iVar9 + 1;
            if (iVar9 == *(int *)(*(long *)(lVar18 + 0x40 + lVar16) + 0x488)) {
              iVar9 = 0;
            }
          }
          *(int *)(lVar18 + 0x74 + lVar16) = iVar9;
        }
      }
      lVar16 = lVar16 + 0x40;
      lVar18 = lVar24 + 2;
      lVar24 = lVar19;
    } while (lVar18 < *(int *)(this + 0x490));
  }
  if (this[0x49c] != (AFGConveyorChainActor)0x0) {
    iVar9 = *(int *)(this + 0x364);
    if ((iVar9 != -1) && (iVar10 = *(int *)(this + 0x368), iVar10 != -1)) {
      iVar22 = iVar10 - iVar9;
      if (iVar10 < iVar9) {
        iVar22 = iVar22 + *(int *)(this + 0x488);
      }
      if (0 < iVar22 + 1) {
        uVar29 = *(uint *)(this + 0x368);
        iVar9 = 0;
        do {
          if ((*(uint *)(this + 0x364) == 0xffffffff) || (*(int *)(this + 0x368) == -1)) {
            if (4 < LogConveyorChain) {
              UE::Logging::Private::BasicLog
                        ((FLogCategoryBase *)&LogConveyorChain,
                         (FStaticBasicLogRecord *)&PTR_u_Failed_to_find_segment_for_itemI_026652d0,
                         (ulong)uVar29,(ulong)*(uint *)(this + 0x364),(ulong)*(uint *)(this + 0x368)
                        );
            }
          }
          else if ((long)*(int *)(this + 0x2d8) != 0) {
            plVar25 = *(long **)(this + 0x2d0);
            lVar24 = (long)*(int *)(this + 0x2d8) << 6;
            do {
              iVar10 = *(int *)((long)plVar25 + 0x34);
              if (iVar10 != -1) {
                if ((int)plVar25[7] < iVar10) {
                  if (iVar10 <= (int)uVar29) goto LAB_01a87dd0;
                }
                else if ((int)uVar29 < iVar10) goto LAB_01a87d78;
                if ((int)uVar29 <= (int)plVar25[7]) goto LAB_01a87dd0;
              }
LAB_01a87d78:
              plVar25 = plVar25 + 8;
              lVar24 = lVar24 + -0x40;
            } while (lVar24 != 0);
          }
          plVar25 = (long *)0x0;
LAB_01a87dd0:
          iVar10 = *(int *)((long)plVar25 + 0x3c);
          if (((int)uVar29 < 0) || (*(int *)(this + 0x338) <= (int)uVar29)) {
            uVar23 = (ulong)(int)uVar29;
            cVar8 = FDebug::CheckVerifyFailedImpl2
                              ("(Index >= 0) & (Index < ArrayNum)",
                               "Runtime/Core/Public\\Containers/Array.h",0x42b,
                               L"Array index out of bounds: %lld into an array of size %lld",uVar23,
                               (long)*(int *)(this + 0x338));
            if (cVar8 != '\0') {
              pcVar3 = (code *)swi(3);
              (*pcVar3)();
              return;
            }
          }
          else {
            uVar23 = (ulong)uVar29;
          }
          fVar31 = *(float *)((long)plVar25 + 0x24) + *(float *)(plVar25 + 4);
          pfVar12 = (float *)(*(long *)(this + 0x330) + 0x28 + uVar23 * 0x38);
          if (((*pfVar12 <= fVar31 && fVar31 != *pfVar12) && (-2 < iVar10)) &&
             (uVar30 = iVar10 + 1, (int)uVar30 < *(int *)(this + 0x2d8))) {
            lVar24 = *(long *)(this + 0x2d0);
            lVar16 = (ulong)uVar30 * 0x40;
            if (*(int *)(lVar24 + 0x34 + lVar16) == -1) {
              *(uint *)(lVar24 + 0x38 + lVar16) = uVar29;
            }
            *(uint *)(lVar24 + lVar16 + 0x34) = uVar29;
            iVar10 = (int)plVar25[7];
            if (*(int *)((long)plVar25 + 0x34) == iVar10) {
              *(undefined4 *)((long)plVar25 + 0x34) = 0xffffffff;
              iVar10 = -1;
            }
            else {
              if (iVar10 == 0) {
                iVar10 = *(int *)(*plVar25 + 0x488);
              }
              iVar10 = iVar10 + -1;
            }
            *(int *)(plVar25 + 7) = iVar10;
          }
          if (uVar29 == 0) {
            uVar29 = *(uint *)(this + 0x488);
          }
          iVar9 = iVar9 + 1;
          uVar29 = uVar29 - 1;
        } while (iVar9 != iVar22 + 1);
      }
    }
    this[0x49c] = (AFGConveyorChainActor)0x0;
  }
  if (this[0x170] == (AFGConveyorChainActor)0x3) {
    uVar43 = GetAvailableSpace(this);
    *(undefined4 *)(this + 0x498) = uVar43;
    if (this[0x4b0] != (AFGConveyorChainActor)0x0) {
      UpdateAttachedMonitors(this,param_1);
    }
  }
  if (AFGBuildableSubsystem::mCurrentSubStep == AFGBuildableSubsystem::mCurrentSubStepMax) {
    for (pUVar27 = *(UObject **)(this + 0x340); pUVar27 != (UObject *)0x0;
        pUVar27 = *(UObject **)(pUVar27 + 0x6b8)) {
      pAVar1 = *(AFGConveyorChainActor **)(pUVar27 + 0x6d8);
      if (pAVar1 != this) {
        if (2 < LogConveyorChain) {
          UObjectBaseUtility::GetFullName(&local_a0,pUVar27,0,0);
          pUVar27 = local_a0;
          if (local_98 == 0) {
            pUVar27 = (UObject *)&DAT_010895c6;
          }
          if ((pAVar1 == (AFGConveyorChainActor *)0x0) || (*(long *)(pAVar1 + 0x10) == 0)) {
            FString::FString((FString *)&local_d8,L"None");
          }
          else {
            UObjectBaseUtility::GetFullName(&local_d8,pAVar1,0,0);
          }
          puVar21 = &DAT_010895c6;
          pUVar11 = (UObject *)&DAT_010895c6;
          if (local_d0 != 0) {
            pUVar11 = local_d8;
          }
          UObjectBaseUtility::GetFullName(&local_b0,this,0,0);
          if (local_a8 != 0) {
            puVar21 = (undefined2 *)CONCAT44(uStack_ac,local_b0);
          }
          UE::Logging::Private::BasicLog
                    ((FLogCategoryBase *)&LogConveyorChain,
                     (FStaticBasicLogRecord *)&PTR_u_Detected_conveyor_chain_tick_gro_02665130,
                     pUVar27,pUVar11,puVar21);
          if ((void *)CONCAT44(uStack_ac,local_b0) != (void *)0x0) {
            FMemory::Free((void *)CONCAT44(uStack_ac,local_b0));
          }
          if (local_d8 != (UObject *)0x0) {
            FMemory::Free(local_d8);
          }
          if (local_a0 != (UObject *)0x0) {
            FMemory::Free(local_a0);
          }
        }
        this_00 = (AFGBuildableSubsystem *)AFGBuildableSubsystem::Get((UObject *)this);
        if (this_00 == (AFGBuildableSubsystem *)0x0) {
          FUN_022408e8();
          pcVar3 = (code *)swi(3);
          (*pcVar3)();
          return;
        }
        AFGBuildableSubsystem::ForceDestroyChainActor(this_00,this);
        if (pAVar1 == (AFGConveyorChainActor *)0x0) {
          return;
        }
        if ((*(uint *)(pAVar1 + 8) & 0x40000000) != 0) {
          return;
        }
        AFGBuildableSubsystem::ForceDestroyChainActor(this_00,pAVar1);
        return;
      }
      if (pUVar27[0x710] != (UObject)0x0) {
        pUVar13 = (UWorld *)AActor::GetWorld();
        lVar24 = AFGRadioactivitySubsystem::Get(pUVar13);
        if (lVar24 != 0) {
          pUVar11 = pUVar27;
          pUVar5 = pUVar27;
          if ((((char)UE::GC::GIsIncrementalReachabilityPending != '\0') &&
              (UE::GC::MarkAsReachable(pUVar27),
              (char)UE::GC::GIsIncrementalReachabilityPending != '\0')) &&
             ((local_a0 = pUVar27, UE::GC::MarkAsReachable(pUVar27), pUVar11 = local_a0,
              pUVar5 = local_a0, (char)UE::GC::GIsIncrementalReachabilityPending != '\0' &&
              (local_a0 != (UObject *)0x0)))) {
            UE::GC::MarkAsReachable(local_a0);
            pUVar5 = local_a0;
          }
          local_a0 = pUVar5;
          uVar29 = ((uint)((ulong)pUVar11 >> 0x14) & 0xffff ^ (uint)((ulong)pUVar11 >> 4)) *
                   -0x7a143595;
          uVar29 = (uVar29 >> 0xd ^ uVar29) * -0x3d4d51cb;
          iVar9 = FUN_017e1d50((long *)(lVar24 + 0x2c8),uVar29 >> 0x10 ^ uVar29,&local_a0);
          if (((iVar9 != -1) &&
              (lVar16 = (long)iVar9 * 0x28 + *(long *)(lVar24 + 0x2c8), lVar16 != 0)) &&
             (*(undefined4 *)(lVar16 + 0x18) = 0, *(int *)(lVar16 + 0x1c) < 0)) {
            FUN_013e3f40(0x40,8,0,lVar16 + 0x10,0,lVar16 + 0x1c);
          }
          (**(code **)(*(long *)pUVar27 + 0xb20))(pUVar27,lVar24);
        }
      }
    }
  }
  *(float *)(this + 0x4b4) = param_1;
  return;
}

