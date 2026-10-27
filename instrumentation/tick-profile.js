// Read-only timing/entry observation of the installed 25782485 Linux build.
// Probe instructions are temporarily patched by Frida and restored on detach.
'use strict';
const moduleName='libFactoryServer-FactoryGame-Linux-Shipping.so';
const game=Process.findModuleByName(moduleName);
if(game===null)throw new Error('FactoryGame module not loaded');
const now=new NativeFunction(Module.getGlobalExportByName('clock_gettime'),'int',['int','pointer']);
function ns(){const t=Memory.alloc(16);if(now(1,t)!==0)throw new Error('clock_gettime failed');return Number(t.readS64())*1e9+Number(t.add(8).readS64());}
const specs=[
 ['factory','_ZN21AFGBuildableSubsystem11TickFactoryEf10ELevelTick'],
 ['factoryActors','_ZN21AFGBuildableSubsystem17TickFactoryActorsEf'],
 ['pipes','_ZN16AFGPipeSubsystem4TickEf'],
 ['circuits','_ZN19AFGCircuitSubsystem4TickEf'],
 ['railroad','_ZN20AFGRailroadSubsystem4TickEf'],
 ['vehicles','_ZN19AFGVehicleSubsystem4TickEf']
];
const totals={};
for(const [name,symbol] of specs){
 const address=game.findExportByName(symbol);
 if(address===null)throw new Error('Missing symbol '+symbol);
 totals[name]={calls:0,totalUs:0,maxUs:0,threads:{}};
 Interceptor.attach(address,{onEnter(args){this.started=ns();this.tid=Process.getCurrentThreadId();},onLeave(){const dt=(ns()-this.started)/1000;const row=totals[name];row.calls++;row.totalUs+=dt;row.maxUs=Math.max(row.maxUs,dt);row.threads[this.tid]=(row.threads[this.tid]??0)+1;}});
}
send({kind:'profile-installed',pid:Process.id,base:game.base.toString(),module:moduleName});
setInterval(()=>send({kind:'tick-profile',pid:Process.id,totals}),5000);
