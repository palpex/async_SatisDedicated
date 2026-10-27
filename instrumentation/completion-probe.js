'use strict';
const bridge=Module.load('/var/www/async_SatisDedicated/build/libsatis_task_bridge.so');
const init=new NativeFunction(bridge.getExportByName('satis_task_bridge_init'),'int',[]);
const error=new NativeFunction(bridge.getExportByName('satis_task_bridge_error'),'pointer',[]);
const start=new NativeFunction(bridge.getExportByName('satis_completion_probe_start'),'int',['uint','uint']);
const read=new NativeFunction(bridge.getExportByName('satis_completion_probe_read'),'int',['pointer']);
const outstanding=new NativeFunction(bridge.getExportByName('satis_task_outstanding'),'uint64',[]);
const initialized=init();send({kind:'completion-init',result:initialized,error:error().readUtf8String()});
if(initialized!==0)throw new Error('Initialization rejected');
send({kind:'completion-start',result:start(32,500000)});
let checks=0;
const timer=setInterval(()=>{
 const data=Memory.alloc(96);read(data);
 const names=['submitted','completed','destroyed','watcherCalls','watcherDisposals','earlyWatcher','wrongThread','checksum','expectedChecksum','signalCalls','failures','maxConcurrency'];
 const result={kind:'completion-probe',outstanding:outstanding().toString()};
 names.forEach((name,index)=>result[name]=data.add(index*8).readU64().toString());send(result);
 if(result.watcherDisposals==='1' && result.outstanding==='0' || ++checks>=20)clearInterval(timer);
},250);
