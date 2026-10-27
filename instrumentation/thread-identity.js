const name='libFactoryServer-Core-Linux-Shipping.so';
const core=Process.getModuleByName(name);
for(const symbol of ['GGameThreadId','GIsGameThreadIdInitialized']){
 const local=core.findExportByName(symbol),global=Module.findGlobalExportByName(symbol);
 send({symbol,local:local?.toString(),localValue:local?.readU32(),global:global?.toString(),globalValue:global?.readU32()});
}
const game=Process.getModuleByName('libFactoryServer-FactoryGame-Linux-Shipping.so');
const target=game.getExportByName('_ZN21AFGBuildableSubsystem11TickFactoryEf10ELevelTick');
let emitted=false;Interceptor.attach(target,{onEnter(){if(!emitted){send({kind:'actual-game-thread',tid:Process.getCurrentThreadId()});emitted=true;}}});
