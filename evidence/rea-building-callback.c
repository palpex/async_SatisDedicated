
void FUN_0199dbe0(undefined8 *param_1,int *param_2)

{
  long lVar1;
  code *pcVar2;
  int iVar3;
  char cVar4;
  long lVar5;
  int iVar6;
  long lVar7;
  int iVar8;
  
  lVar1 = param_1[1];
  lVar5 = (long)*param_2 * (long)*(int *)*param_1;
  iVar3 = (int)lVar5;
  iVar6 = *(int *)*param_1 + iVar3;
  if (*(int *)(lVar1 + 0x3c8) < iVar6) {
    iVar6 = *(int *)(lVar1 + 0x3c8);
  }
  iVar8 = iVar6 - iVar3;
  lVar7 = lVar5;
  if (iVar8 != 0 && iVar3 <= iVar6) {
    do {
      if (((lVar7 < 0) || (*(int *)(lVar1 + 0x3c8) <= lVar7)) &&
         (cVar4 = FDebug::CheckVerifyFailedImpl2
                            ("(Index >= 0) & (Index < ArrayNum)",
                             "Runtime/Core/Public\\Containers/Array.h",0x42b,
                             L"Array index out of bounds: %lld into an array of size %lld",lVar7),
         cVar4 != '\0')) {
        pcVar2 = (code *)swi(3);
        (*pcVar2)();
        return;
      }
      iVar8 = iVar8 + -1;
      lVar7 = lVar7 + 1;
    } while (iVar8 != 0);
    iVar6 = iVar6 - iVar3;
    do {
      (**(code **)(**(long **)(*(long *)(lVar1 + 0x3c0) + lVar5 * 8) + 0x930))
                (*(undefined4 *)param_1[2]);
      lVar5 = lVar5 + 1;
      iVar6 = iVar6 + -1;
    } while (iVar6 != 0);
  }
  return;
}

