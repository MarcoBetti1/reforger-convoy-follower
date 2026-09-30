import fs from 'node:fs';import path from 'node:path';import crypto from 'node:crypto';import {spawnSync} from 'node:child_process';
import {verifyOriginalFollowSources} from '../startup-pair-v1/analysis/original-follow-report.mjs';
const root='.cache/ordinary-outbound-bay-v2',build='D:/ReforgerAgentBuilds/ordinary-outbound-bay-v2',source=build+'/source/ConvoyFollower';
const parent=JSON.parse(fs.readFileSync('.cache/ordinary-outbound-bay-v1/frozen-final-review.json'));
const audit=JSON.parse(fs.readFileSync(root+'/edit-review.json')),base=parent.frozenSource;
const archive=build+'/frozen-source/source',runtime=build+'/frozen-addons',deployment=runtime+'/ConvoyFollower_5A5FB20BD40C7C70';
const hash=b=>crypto.createHash('sha256').update(b).digest('hex').toUpperCase();
const files=p=>fs.readdirSync(p,{withFileTypes:true}).flatMap(e=>e.isDirectory()?files(p+'/'+e.name):[p+'/'+e.name]);
const manifest=p=>files(p).map(file=>{const b=fs.readFileSync(file);return{path:path.relative(p,file).replaceAll('\\','/'),bytes:b.length,sha256:hash(b)};}).sort((a,b)=>a.path.localeCompare(b.path));
const baseline=manifest(base),final=manifest(source),map=new Map(baseline.map(e=>[e.path,e]));
const changes=final.filter(e=>map.get(e.path)?.sha256!==e.sha256).map(e=>({...e,baselineSha256:map.get(e.path)?.sha256??null}));
const deleted=baseline.filter(e=>!final.some(f=>f.path===e.path));
if(deleted.length||final.length!==baseline.length||changes.some(e=>!['resourceDatabase.rdb',audit.changedFile].includes(e.path)))throw Error('Unexpected V2 source delta');
const code=fs.readFileSync(source+'/'+audit.changedFile,'utf8');let restored=code;
for(const edit of [...audit.edits].reverse()){if(restored.split(edit.after).length!==2)throw Error('Restore seam '+edit.label);restored=restored.replace(edit.after,edit.before);}
if(restored!==fs.readFileSync(base+'/'+audit.changedFile,'utf8'))throw Error('V1 restoration mismatch');
if(hash(Buffer.from(code))!==audit.candidateSha256||hash(fs.readFileSync(root+'/CF_OrdinaryOutboundBayCourse.c'))!==audit.candidateSha256)throw Error('V2 reviewed bytes differ');
if(fs.existsSync(runtime)||fs.existsSync(archive))throw Error('Frozen destination already exists');
fs.mkdirSync(path.dirname(archive),{recursive:true});fs.cpSync(source,archive,{recursive:true});fs.mkdirSync(deployment,{recursive:true});
const trio={};for(const file of ['addon.gproj','data.pak','resourceDatabase.rdb']){fs.copyFileSync(build+'/packed/'+file,deployment+'/'+file);const b=fs.readFileSync(deployment+'/'+file);trio[file]={bytes:b.length,sha256:hash(b)};}
for(const file of ['addon.gproj','resourceDatabase.rdb'])if(hash(fs.readFileSync(source+'/'+file))!==trio[file].sha256)throw Error('Source/deployment '+file+' mismatch');
fs.writeFileSync(runtime+'/source-manifest.json',JSON.stringify(final,null,2)+'\n',{flag:'wx'});
const verify=await verifyOriginalFollowSources({manifestPath:runtime+'/source-manifest.json',copiedRoot:archive,packPath:deployment+'/data.pak',expectedPack:trio['data.pak'].sha256});
fs.writeFileSync(runtime+'/source-pack-verification.json',JSON.stringify(verify,null,2)+'\n',{flag:'wx'});
const diff=spawnSync('git',['diff','--no-index','--',base+'/'+audit.changedFile,archive+'/'+audit.changedFile],{encoding:'utf8'});
if(diff.status!==1||!diff.stdout)throw Error('Missing exact source diff');fs.writeFileSync(root+'/source-diff.patch',diff.stdout,{flag:'wx'});
const production=final.find(e=>e.path==='Scripts/Game/CF_DriverControllerComponent.c');
if(production.sha256!=='224A97D271D9F43820AE4E1B30C3BBC5FF3514C4E5570BC9BB075F1292A5418B')throw Error('Production driver changed');
const review={...parent,status:'PREPARED_UNLAUNCHED_V2',version:2,base:base,buildRoot:build,sourceRoot:source,frozenSource:archive,runtimeParent:runtime,deployment,sourceFiles:final.length,packedTextResources:verify.checks.filter(e=>e.packMatches!==undefined).length,trio,
 validation:{allFiveConfigurationsPassed:true,packPassed:true,logs:build+'/workbench',packedSource:build+'/packed'},
 sourceVerification:runtime+'/source-pack-verification.json',sourceAudit:{changes,deleted,sourceFileCountUnchanged:true,restoredV1ByteTextMatches:true,originalDriverSha256:production.sha256,editAudit:root+'/edit-review.json',exactSourceDiff:root+'/source-diff.patch',productionAndWorldFilesUnchanged:true},
 startupCorrection:'Request original lead Wait once, phase13 polls original parking/control/speed/exact-selected-Wait conjunction with10world subbound within unchanged95world setup; then invokes unchanged native handoff. All other Wait/seat seams audited. JourneyPhase/BayPhase/native transfer entry stop after terminal; motion callers return after terminal.',
 preconditionDiagnostics:'Read-only original CanCommandPark, speed>2 rejection (sameNaN comparison semantics), owner-direction native-control/selected-Wait or native-direction owner-pilot/native-pilot-onfoot. All required terms, failed terms and original identities logged. Original helper remains the actuator/rejection authority.',
 selectedWaitSubboundWorldSeconds:10,liveRun:false,reviewBeforeLiveExecution:true,changedExistingObserverFiles:[audit.changedFile],newObserver:null,
 runtimeGuidChildren:fs.readdirSync(runtime,{withFileTypes:true}).filter(e=>e.isDirectory()).map(e=>e.name),ordinaryV8UnchangedAndClosed:true};
if(review.runtimeGuidChildren.length!==1)throw Error('Duplicate GUID parent');
fs.writeFileSync(root+'/frozen-review.json',JSON.stringify(review,null,2)+'\n',{flag:'wx'});fs.writeFileSync(runtime+'/frozen-review.json',JSON.stringify(review,null,2)+'\n',{flag:'wx'});
fs.writeFileSync(runtime+'/edit-review.json',JSON.stringify(audit,null,2)+'\n',{flag:'wx'});
console.log(JSON.stringify({status:review.status,sourceFiles:review.sourceFiles,packedText:review.packedTextResources,changes:changes.map(e=>e.path),trio,sourceDiff:root+'/source-diff.patch',liveRun:false},null,2));
