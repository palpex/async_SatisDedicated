
void FUN_0199dd80(long *param_1,uint *param_2)

{
  long *plVar1;
  uint uVar2;
  int iVar3;
  long *plVar4;
  code *pcVar5;
  char cVar6;
  long lVar7;
  long lVar8;
  long lVar9;
  long *plVar10;
  ulong uVar11;
  
  uVar2 = *param_2;
  uVar11 = (ulong)uVar2;
  lVar8 = *param_1;
  iVar3 = *(int *)(lVar8 + 0x470);
  if (((int)uVar2 < 0) || (iVar3 <= (int)uVar2)) {
    uVar11 = (ulong)(int)uVar2;
    cVar6 = FDebug::CheckVerifyFailedImpl2
                      ("(Index >= 0) & (Index < ArrayNum)","Runtime/Core/Public\\Containers/Array.h"
                       ,0x42b,L"Array index out of bounds: %lld into an array of size %lld",uVar11,
                       (long)iVar3);
    if (cVar6 != '\0') {
      pcVar5 = (code *)swi(3);
      (*pcVar5)();
      return;
    }
  }
  lVar8 = *(long *)(lVar8 + 0x468);
  lVar7 = (long)*(int *)(lVar8 + 8 + uVar11 * 0x10);
  if (lVar7 != 0) {
    plVar10 = *(long **)(lVar8 + uVar11 * 0x10);
    plVar1 = plVar10 + lVar7;
    do {
      lVar8 = *plVar10;
      if (*(AFGConveyorChainActor **)(lVar8 + 0x18) == (AFGConveyorChainActor *)0x0) {
        if ((long)*(int *)(lVar8 + 0x10) != 0) {
          lVar7 = *(long *)(lVar8 + 8);
          lVar9 = (long)*(int *)(lVar8 + 0x10) * 8;
          lVar8 = 0;
          do {
            while (((plVar4 = *(long **)(lVar7 + lVar8), (~*(byte *)((long)plVar4 + 0x619) & 3) != 0
                    || ((int)plVar4[0xd1] == 0)) || (*(char *)param_1[2] == '\0'))) {
              (**(code **)(*plVar4 + 0x930))(*(undefined4 *)param_1[1]);
              lVar8 = lVar8 + 8;
              if (lVar9 == lVar8) goto LAB_0199dddd;
            }
            *(float *)(plVar4 + 0xea) = *(float *)param_1[1] + *(float *)(plVar4 + 0xea);
            lVar8 = lVar8 + 8;
          } while (lVar9 != lVar8);
        }
      }
      else {
        AFGConveyorChainActor::Factory_Tick
                  (*(AFGConveyorChainActor **)(lVar8 + 0x18),*(float *)param_1[1]);
      }
LAB_0199dddd:
      plVar10 = plVar10 + 1;
    } while (plVar10 != plVar1);
  }
  return;
}

