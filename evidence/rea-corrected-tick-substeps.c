
/* AFGBuildableSubsystem::TickFactory(float, ELevelTick) */

void __thiscall
AFGBuildableSubsystem::TickFactory(float param_1,AFGBuildableSubsystem *this,int param_3)

{
  long lVar1;
  long lVar2;
  int iVar3;
  int iVar4;
  float fVar5;
  float fVar6;
  
  if ((((this != (AFGBuildableSubsystem *)0x0) && (param_3 != 1)) &&
      ((*(uint *)(this + 8) & 0x40000000) == 0)) && (lVar1 = AActor::GetWorld(), lVar1 != 0)) {
    lVar2 = Z_Construct_UClass_UFGFactoryTickSettings_NoRegister();
    lVar1 = *(long *)(lVar2 + 0x110);
    if (lVar1 == 0) {
      UClass::InternalCreateDefaultObjectWrapper();
      lVar1 = *(long *)(lVar2 + 0x110);
    }
    fVar5 = *(float *)(lVar1 + 0x38);
    lVar2 = Z_Construct_UClass_UFGFactoryTickSettings_NoRegister();
    lVar1 = *(long *)(lVar2 + 0x110);
    if (lVar1 == 0) {
      UClass::InternalCreateDefaultObjectWrapper();
      lVar1 = *(long *)(lVar2 + 0x110);
    }
    iVar4 = *(int *)(lVar1 + 0x3c);
    fVar6 = (float)iVar4 * fVar5;
    if (param_1 <= fVar6) {
      fVar6 = param_1;
    }
    fVar5 = (1.0 / fVar5) * fVar6;
    iVar3 = -((int)ROUND(-0.5 - (fVar5 + fVar5)) >> 1);
    if (iVar4 <= iVar3) {
      iVar3 = iVar4;
    }
    iVar4 = 1;
    if (1 < iVar3) {
      iVar4 = iVar3;
    }
    *(float *)(this + 0x7c8) = fVar6 + *(float *)(this + 0x7c8);
    mCurrentSubStepMax = (char)iVar4 + -1;
    iVar3 = 0;
    do {
      mCurrentSubStep = (undefined1)iVar3;
      TickFactoryActors(this,fVar6 / (float)iVar4);
      iVar3 = iVar3 + 1;
    } while (iVar4 != iVar3);
  }
  return;
}

