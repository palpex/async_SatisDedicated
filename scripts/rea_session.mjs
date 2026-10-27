// Keep one native Program open across focused REA queries. JSONL on stdin.
import {Client} from '../.tools/rea/node_modules/@modelcontextprotocol/client/dist/index.mjs';
import {StdioClientTransport} from '../.tools/rea/node_modules/@modelcontextprotocol/client/dist/stdio.mjs';
import {writeFile, mkdir} from 'node:fs/promises';
import {resolve} from 'node:path';
import {createInterface} from 'node:readline';
const transport=new StdioClientTransport({command:resolve('scripts/rea'),args:['mcp'],env:{...process.env,REA_GHIDRA_STARTUP_TIMEOUT_MS:'1800000',REA_MCP_INPUT_SCHEMA_PROFILE:'full'},stderr:'inherit',maxBufferSize:32000000});
const client=new Client({name:'satisfactory-investigation',version:'1.0.0'});
try {
 await client.connect(transport);
 const catalog=await client.listTools();
 await writeFile('evidence/rea-session-tool-schemas.json',JSON.stringify(catalog.tools,null,2)+'\n');
 console.log(JSON.stringify({ready:true,pid:process.pid,server_pid:transport.pid}));
 const lines=createInterface({input:process.stdin,terminal:false});
 for await(const line of lines){
  if(!line.trim()) continue;
  let request;
  try {
   request=JSON.parse(line);
   if(request.name==='exit')break;
   if(!catalog.tools.some(t=>t.name===request.name))throw new Error('Unadvertised tool');
   const response=await client.callTool({name:request.name,arguments:request.args??{}},{timeout:1900000,maxTotalTimeout:1900000});
   if(request.output){
    await mkdir(resolve(request.output,'..'),{recursive:true});
    await writeFile(request.output,JSON.stringify(response,null,2)+'\n',{flag:'wx'});
   }
   const structured=response.structuredContent;
   const preview=JSON.stringify(structured??response);
   console.log(JSON.stringify({name:request.name,output:request.output,isError:response.isError??false,preview:preview.slice(0,2500),bytes:preview.length}));
  }catch(error){console.log(JSON.stringify({name:request?.name,error:String(error)}));}
 }
}finally{await client.close();}
