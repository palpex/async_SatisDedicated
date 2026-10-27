
/* AFGPipeSubsystem::SimulatePipeNetworks(float) */

void __thiscall AFGPipeSubsystem::SimulatePipeNetworks(AFGPipeSubsystem *this,float param_1)

{
  code *pcVar1;
  char cVar2;
  int iVar3;
  uint uVar4;
  int iVar5;
  int iVar6;
  int iVar7;
  long lVar8;
  int iVar9;
  AFGPipeSubsystem *pAVar10;
  ulong uVar11;
  long lVar12;
  ulong uVar13;
  ulong uVar14;
  uint uVar15;
  float fVar16;
  AFGPipeNetwork **local_88;
  undefined8 uStack_80;
  float local_78;
  float local_74;
  float local_70;
  int local_6c;
  AFGPipeNetwork *local_68;
  int *local_60;
  float *local_58;
  undefined8 local_50;
  undefined4 local_48;
  code *local_40;
  AFGPipeNetwork **local_38;
  
  local_88 = (AFGPipeNetwork **)0x0;
  uStack_80 = 0;
  iVar5 = *(int *)(this + 0x2e8);
  local_74 = param_1;
  if (iVar5 != 0) {
    pAVar10 = *(AFGPipeSubsystem **)(this + 0x2e0);
    if (*(AFGPipeSubsystem **)(this + 0x2e0) == (AFGPipeSubsystem *)0x0) {
      pAVar10 = this + 0x2d0;
    }
    uVar15 = *(uint *)pAVar10;
    if (uVar15 == 0) {
      iVar3 = iVar5 + 0x1e;
      if (-1 < iVar5 + -1) {
        iVar3 = iVar5 + -1;
      }
      uVar14 = 0;
      uVar11 = (ulong)(uint)(iVar3 >> 5);
      if (iVar3 >> 5 < 1) {
        uVar11 = uVar14;
      }
      iVar3 = 0x3e;
      do {
        iVar9 = iVar3;
        if (uVar11 == uVar14) goto LAB_01c25c0e;
        uVar15 = *(uint *)(pAVar10 + uVar14 * 4 + 4);
        uVar14 = uVar14 + 1;
        iVar3 = iVar9 + 0x20;
      } while (uVar15 == 0);
      iVar9 = iVar9 + -0x1e;
    }
    else {
      iVar3 = 0x3e;
      uVar14 = 0;
      iVar9 = 0;
    }
    uVar13 = (ulong)(-uVar15 & uVar15);
    uVar11 = uVar13 * 2 + 1;
    lVar12 = 0x3f;
    if (uVar11 != 0) {
      for (; uVar11 >> lVar12 == 0; lVar12 = lVar12 + -1) {
      }
    }
    iVar3 = iVar3 - ((uint)lVar12 ^ 0x3f);
    if (iVar5 <= iVar3) {
      iVar3 = iVar5;
    }
    if (iVar3 < iVar5) {
      uVar15 = 0xffffffff;
      do {
        local_68 = *(AFGPipeNetwork **)(*(long *)(this + 0x2c0) + 8 + (long)iVar3 * 0x18);
        cVar2 = AFGPipeNetwork::ShouldRunSimulation(local_68);
        if (cVar2 != '\0') {
          if (local_88 <= &local_68) {
            if (&local_68 < local_88 + uStack_80._4_4_) {
              cVar2 = FDebug::CheckVerifyFailedImpl2
                                ("Addr < GetData() || Addr >= (GetData() + ArrayMax)",
                                 "Runtime/Core/Public\\Containers/Array.h",0x7ac,
                                 L"Attempting to use a container element (%p) which already comes from the container being modified (%p, ArrayMax: %lld, ArrayNum: %lld, SizeofElement: %zu)!"
                                 ,&local_68,local_88,(long)uStack_80._4_4_,(long)(int)uStack_80,8);
              if (cVar2 != '\0') {
                pcVar1 = (code *)swi(3);
                (*pcVar1)();
                return;
              }
            }
          }
          iVar5 = (int)uStack_80;
          if ((int)uStack_80 == uStack_80._4_4_) {
            iVar5 = FUN_01423d90(0x808,&local_88,(long)&uStack_80 + 4);
          }
          uStack_80 = CONCAT44(uStack_80._4_4_,iVar5 + 1);
          local_88[iVar5] = local_68;
        }
        uVar15 = uVar15 & ~(uint)uVar13;
        pAVar10 = *(AFGPipeSubsystem **)(this + 0x2e0);
        if (*(AFGPipeSubsystem **)(this + 0x2e0) == (AFGPipeSubsystem *)0x0) {
          pAVar10 = this + 0x2d0;
        }
        iVar5 = *(int *)(this + 0x2e8);
        iVar3 = (int)uVar14;
        lVar12 = (long)iVar3;
        uVar4 = *(uint *)(pAVar10 + lVar12 * 4) & uVar15;
        if (uVar4 == 0) {
          iVar6 = iVar5 + 0x1e;
          if (-1 < iVar5 + -1) {
            iVar6 = iVar5 + -1;
          }
          iVar6 = iVar6 >> 5;
          iVar7 = iVar6;
          if (iVar6 < iVar3) {
            iVar7 = iVar3;
          }
          if (iVar6 <= iVar3) break;
          lVar8 = 0;
          while( true ) {
            iVar9 = iVar9 + 0x20;
            uVar4 = *(uint *)(pAVar10 + lVar8 * 4 + lVar12 * 4 + 4);
            if (uVar4 != 0) break;
            lVar8 = lVar8 + 1;
            if (iVar7 - lVar12 == lVar8) goto LAB_01c25c0e;
          }
          uVar14 = (ulong)(iVar3 + (int)lVar8 + 1);
          uVar15 = 0xffffffff;
        }
        uVar13 = (ulong)(-uVar4 & uVar4);
        uVar11 = uVar13 * 2 + 1;
        lVar12 = 0x3f;
        if (uVar11 != 0) {
          for (; uVar11 >> lVar12 == 0; lVar12 = lVar12 + -1) {
          }
        }
        iVar3 = (iVar9 - ((uint)lVar12 ^ 0x3f)) + 0x3e;
      } while (iVar3 < iVar5);
    }
  }
LAB_01c25c0e:
  iVar5 = *DAT_02747298;
  lVar8 = Z_Construct_UClass_UFGFactoryTickSettings_NoRegister();
  lVar12 = *(long *)(lVar8 + 0x110);
  if (lVar12 == 0) {
    UClass::InternalCreateDefaultObjectWrapper();
    lVar12 = *(long *)(lVar8 + 0x110);
  }
  local_78 = *(float *)(lVar12 + 0x38);
  lVar8 = Z_Construct_UClass_UFGFactoryTickSettings_NoRegister();
  lVar12 = *(long *)(lVar8 + 0x110);
  if (lVar12 == 0) {
    UClass::InternalCreateDefaultObjectWrapper();
    lVar12 = *(long *)(lVar8 + 0x110);
  }
  iVar3 = *(int *)(lVar12 + 0x3c);
  local_70 = (float)iVar3 * local_78;
  if (local_74 <= (float)iVar3 * local_78) {
    local_70 = local_74;
  }
  fVar16 = (1.0 / local_78) * local_70;
  iVar9 = -((int)ROUND(-0.5 - (fVar16 + fVar16)) >> 1);
  if (iVar3 <= iVar9) {
    iVar9 = iVar3;
  }
  local_6c = 1;
  if (1 < iVar9) {
    local_6c = iVar9;
  }
  local_70 = local_70 / (float)local_6c;
  local_60 = &local_6c;
  local_58 = &local_70;
  local_40 = FUN_01c5d1f0;
  local_38 = &local_68;
  local_50 = 0;
  local_48 = 0;
  local_68 = (AFGPipeNetwork *)&local_88;
  FUN_0194eea0(L"ParallelFor Task",uStack_80 & 0xffffffff,1,&local_40,2 - (uint)(iVar5 == 0),
               &local_50);
  if (local_88 != (AFGPipeNetwork **)0x0) {
    FMemory::Free(local_88);
  }
  return;
}

