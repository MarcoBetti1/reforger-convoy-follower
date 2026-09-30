import fs from 'node:fs';
import crypto from 'node:crypto';
import {setTimeout as delay} from 'node:timers/promises';
const run='D:/ReforgerAgentRuns/ordinary-outbound-bay-v2';
const source=run+'/logs/console.log',output=run+'/gameplay-at-terminal.log';
if(fs.existsSync(output)||fs.existsSync(output+'.provenance.json'))throw Error('Existing snapshot evidence');
const startedUtc=new Date().toISOString(), deadline=performance.now()+5100000;
const marker=Buffer.from('PACED_RESULT:'),hash=b=>crypto.createHash('sha256').update(b).digest('hex').toUpperCase();
let attempts=0;
while(performance.now()<deadline){
 attempts++;let bytes;
 const readStartedUtc=new Date().toISOString();
 try{const info=fs.statSync(source);if(!info.isFile()||info.size>64*1024*1024)throw Error('Source exceeds snapshot bound');bytes=fs.readFileSync(source);}catch(error){if(error.code!=='ENOENT')throw error;}
 if(bytes&&bytes.includes(marker)){
  const capturedUtc=new Date().toISOString(),sha256=hash(bytes);
  const provenance={method:'first observed marker; up to poll latency; complete successful read, not atomic terminal boundary',startedUtc,readStartedUtc,capturedUtc,markerLiteral:'PACED_RESULT:',firstMarkerOffsetBytes:bytes.indexOf(marker),pollMs:50,timeoutMs:5100000,attempts,source:{path:source,bytes:bytes.length,sha256,hashScope:'captured read only'},output:{path:output,bytes:bytes.length,sha256},provenancePath:output+'.provenance.json',scope:'Private long-course watch; same64MiB/50ms/read-only/exclusive evidence discipline; installed generic CLI has3600s maximum.'};
  fs.mkdirSync(run,{recursive:true});fs.writeFileSync(output,bytes,{flag:'wx'});fs.writeFileSync(output+'.provenance.json',JSON.stringify(provenance,null,2)+'\n',{flag:'wx'});
  console.log(JSON.stringify(provenance,null,2));process.exit(0);
 }
 await delay(50);
}
throw Error('Declared5100s terminal snapshot timeout; no result inferred');
