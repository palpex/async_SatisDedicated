
/* AFGBuildableSubsystem::RegisterFactoryTickFunction(bool) */

void __thiscall
AFGBuildableSubsystem::RegisterFactoryTickFunction(AFGBuildableSubsystem *this,bool param_1)

{
  char cVar1;
  
  cVar1 = UObjectBaseUtility::IsTemplate(this,0x30);
  if (cVar1 != '\0') {
                    /* WARNING: Subroutine does not return */
    FUN_0223f89e();
  }
  if (param_1) {
    if ((*(ushort *)(this + 0x6e2) & 2) != 0) {
      *(AFGBuildableSubsystem **)(this + 0x700) = this;
      FTickFunction::SetTickFunctionEnable(SUB81((ULevel *)(this + 0x6d8),0));
      AActor::GetLevel();
      FTickFunction::RegisterTickFunction((ULevel *)(this + 0x6d8));
      return;
    }
  }
  else if ((*(byte **)(this + 0x6f8) != (byte *)0x0) && ((**(byte **)(this + 0x6f8) & 1) != 0)) {
    FTickFunction::UnRegisterTickFunction();
    return;
  }
  return;
}

