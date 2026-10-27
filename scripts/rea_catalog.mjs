import {Client} from '../.tools/rea/node_modules/@modelcontextprotocol/client/dist/index.mjs';
import {StdioClientTransport} from '../.tools/rea/node_modules/@modelcontextprotocol/client/dist/stdio.mjs';
import {writeFile} from 'node:fs/promises';
import {resolve} from 'node:path';
const transport=new StdioClientTransport({command:resolve('scripts/rea'),args:['mcp'],env:{...process.env,REA_MCP_INPUT_SCHEMA_PROFILE:'full'},stderr:'inherit',maxBufferSize:32000000});
const client=new Client({name:'satisfactory-investigation',version:'1.0.0'});
try {
 await client.connect(transport);
 const response=await client.listTools();
 const names=['open_binary','binary_session','procedure_info','analyze_function','procedure_pseudo_code','export_evidence_bundle','close_binary','list_procedures','inspect_native_load_image'];
 const selected=response.tools.filter(t=>names.includes(t.name));
 await writeFile('evidence/rea-native-tool-schemas.json',JSON.stringify(selected,null,2)+'\n');
 console.log(JSON.stringify(selected.map(t=>({name:t.name,inputSchema:t.inputSchema})),null,2));
}finally{await client.close();}
