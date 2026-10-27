
void FUN_0199dcc0(long *param_1,uint *param_2)

{
  uint uVar1;
  int iVar2;
  long lVar3;
  code *pcVar4;
  char cVar5;
  long lVar6;
  ulong uVar7;
  long lVar8;
  
  uVar1 = *param_2;
  uVar7 = (ulong)uVar1;
  lVar3 = *param_1;
  iVar2 = *(int *)(lVar3 + 0x498);
  if (((int)uVar1 < 0) || (iVar2 <= (int)uVar1)) {
    uVar7 = (ulong)(int)uVar1;
    cVar5 = FDebug::CheckVerifyFailedImpl2
                      ("(Index >= 0) & (Index < ArrayNum)","Runtime/Core/Public\\Containers/Array.h"
                       ,0x42b,L"Array index out of bounds: %lld into an array of size %lld",uVar7,
                       (long)iVar2);
    if (cVar5 != '\0') {
      pcVar4 = (code *)swi(3);
      (*pcVar4)();
      return;
    }
  }
  lVar3 = *(long *)(lVar3 + 0x490);
  lVar8 = (long)*(int *)(lVar3 + 8 + uVar7 * 0x10);
  if (lVar8 != 0) {
    lVar3 = *(long *)(lVar3 + uVar7 * 0x10);
    lVar6 = 0;
    do {
      AFGConveyorChainActor::Factory_Tick
                (*(AFGConveyorChainActor **)(lVar3 + lVar6),
                 *(float *)param_1[1] + *(float *)param_1[2]);
      lVar6 = lVar6 + 8;
    } while (lVar8 * 8 != lVar6);
  }
  return;
}

