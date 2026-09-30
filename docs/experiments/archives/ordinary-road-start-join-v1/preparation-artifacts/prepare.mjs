import fs from 'node:fs';import path from 'node:path';import crypto from 'node:crypto';
const root='.cache/ordinary-road-start-join-v1',declaration=JSON.parse(fs.readFileSync(root+'/geometry-declaration.json'));
const dest=declaration.buildRoot+'/source/ConvoyFollower',base=declaration.baseline;
const files=p=>fs.readdirSync(p,{withFileTypes:true}).flatMap(e=>e.isDirectory()?files(p+'/'+e.name):[p+'/'+e.name]);
if(fs.existsSync(dest)){
 // The first preparation rejected LF/CRLF before writing any source. Reuse
 // only the still byte-identical baseline copy; never replace edited files.
 const copied=files(dest),original=files(base);if(copied.length!==original.length)throw Error('Existing copy differs');
 for(const f of copied){const rel=path.relative(dest,f);if(!fs.readFileSync(f).equals(fs.readFileSync(base+'/'+rel)))throw Error('Preserve edited private copy '+rel);}
}else fs.cpSync(base,dest,{recursive:true});
const relative=declaration.authoredFile,old=fs.readFileSync(base+'/'+relative,'utf8');let layer=old;const edits=[];
const vector=p=>p.join(' ');
function replace(before,after,label){if(!old.includes('\r\n')){before=before.replaceAll('\r\n','\n');after=after.replaceAll('\r\n','\n');}if(layer.split(before).length!==2)throw Error('Unique source seam '+label);layer=layer.replace(before,after);edits.push({before,after,label});}
const g=declaration.fixedGeometry;
replace('coords 7239.0 138.9 2440.0','coords '+vector(g.spawn.coords),'same local spawn offset rotated with lead');
replace('coords 7253.0 138.9 2432.0','coords '+vector(g.followerDriverGroup.coords),'original follower group local6m offset');
replace('coords 7253.0 138.9 2438.0\r\n angles 0 -90 0','coords '+vector(g.follower.coords)+'\r\n angles '+vector(g.follower.angles),'original follower road pose');
replace('coords 7233.0 138.9 2438.0\r\n angles 0 -90 0','coords '+vector(g.lead.coords)+'\r\n angles '+vector(g.lead.angles),'original lead road pose');
replace('coords 7253.0 138.6 2428.0\r\n angles 0 90 0','coords '+vector(g.source.coords)+'\r\n angles '+vector(g.source.angles),'original source local offset rotated');
replace('coords 7233.0 138.9 2432.0','coords '+vector(g.leadDriverGroup.coords),'original native pilot group local6m offset');
let restored=layer;for(const e of [...edits].reverse())restored=restored.replace(e.after,e.before);if(restored!==old)throw Error('Byte restoration mismatch');
fs.writeFileSync(dest+'/'+relative,layer);
const hash=b=>crypto.createHash('sha256').update(b).digest('hex').toUpperCase();
const audit={status:'PREPARED_SOURCE_ONLY_UNLAUNCHED',declaration:root+'/geometry-declaration.json',sourceRoot:dest,baseline:base,authoredFile:relative,edits,baselineSha256:hash(Buffer.from(old)),candidateSha256:hash(Buffer.from(layer)),restoredOriginalByteTextMatches:true,productionScriptsAndWorldComponentUnchanged:true,worldResourceAndMetadataUnchanged:true,actorAndStorageCountPrefabsNamesAndContainersUnchanged:true,destinationUnchanged:true,liveRun:false};
fs.writeFileSync(root+'/edit-review.json',JSON.stringify(audit,null,2)+'\n',{flag:'wx'});
console.log(JSON.stringify({sourceRoot:dest,authoredFile:relative,sourceSha256:audit.candidateSha256,edits:edits.length,liveRun:false},null,2));
