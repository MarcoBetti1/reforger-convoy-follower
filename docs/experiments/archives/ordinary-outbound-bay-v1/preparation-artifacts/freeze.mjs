import fs from 'node:fs';
import path from 'node:path';
import crypto from 'node:crypto';
import {verifyOriginalFollowSources} from '../startup-pair-v1/analysis/original-follow-report.mjs';
const local='.cache/ordinary-outbound-bay-v1';
const build='D:/ReforgerAgentBuilds/ordinary-outbound-bay-v1';
const source=build+'/source/ConvoyFollower';
const base='.cache/frozen-source/return-predicate-observer-v1/source';
const finalFreeze=process.argv.includes('--final');
const archive=build+(finalFreeze?'/frozen-source-final/source':'/frozen-source/source'), runtime=build+(finalFreeze?'/frozen-addons-final':'/frozen-addons');
const deployment=runtime+'/ConvoyFollower_5A5FB20BD40C7C70';
const pack=build+(finalFreeze?'/packed-resume-baseline':'/packed-parking-release');
const hash=b=>crypto.createHash('sha256').update(b).digest('hex').toUpperCase();
const files=root=>fs.readdirSync(root,{withFileTypes:true}).flatMap(e=>e.isDirectory()?files(root+'/'+e.name):[root+'/'+e.name]);
const manifest=root=>files(root).map(file=>{const b=fs.readFileSync(file);return {path:path.relative(root,file).replaceAll('\\','/'),bytes:b.length,sha256:hash(b)};}).sort((a,b)=>a.path.localeCompare(b.path));
const baseline=manifest(base), final=manifest(source), baselineMap=new Map(baseline.map(e=>[e.path,e]));
const changes=final.filter(e=>baselineMap.get(e.path)?.sha256!==e.sha256).map(e=>({...e,baselineSha256:baselineMap.get(e.path)?.sha256??null}));
const deleted=baseline.filter(e=>!final.some(f=>f.path===e.path));
const allowed=new Set(['resourceDatabase.rdb','Scripts/Game/Tests/CF_ExplicitBayFeatureProbeComponent.c','Scripts/Game/Tests/CF_NativeParkingVisual.c','Scripts/Game/Tests/CF_OrdinaryOutboundBayCourse.c','Worlds/Tests/ConvoyFollower_Everon_LoadedOrdinary_1Truck_Layers/default.layer']);
if(deleted.length||changes.some(e=>!allowed.has(e.path))||changes.length!==allowed.size)throw Error('Unexpected private source delta: '+JSON.stringify({changes,deleted}));
const read=(root,file)=>fs.readFileSync(root+'/'+file,'utf8');
const restorationChecks=[];
for(const file of allowed){
 if(file==='resourceDatabase.rdb'||file.endsWith('CF_OrdinaryOutboundBayCourse.c'))continue;
 let restored=read(source,file);
 if(file.endsWith('default.layer'))restored=restored.replace('CF_OrdinaryOutboundBayCourse','CF_ExplicitBayFeatureProbeComponent');
 if(file.endsWith('CF_NativeParkingVisual.c'))restored=restored.replaceAll('camera_unchanged=false','camera_unchanged=true');
 if(file.endsWith('CF_ExplicitBayFeatureProbeComponent.c'))restored=restored.replace('1650000','420000').replace('1650world_total_timeout','420world_total_timeout').replace('observation_samples=" + m_iObservationSamples + " first_failure=','observation_samples=0 first_failure=');
 const equal=restored===read(base,file);restorationChecks.push({file,restoredByteTextMatchesBaseline:equal});if(!equal)throw Error('Unexpected observer/world edit: '+file);
}
const added=read(source,'Scripts/Game/Tests/CF_OrdinaryOutboundBayCourse.c');
if(/CF_ResetInitialDeparture\s*\(|m_bInitialDeparturePending\s*=(?!=)|\.SetOrigin\s*\(|\.SetTransform\s*\(/.test(added))throw Error('Forbidden guard/actor mutation');
if(added.match(/\.SetWorldTransform\(/g)?.length!==2||!added.includes('m_JourneyCamera.SetWorldTransform'))throw Error('Camera transform scope');
if(!added.includes('ReleaseCommandParking("ordinary_first_native_drive_order_started")'))throw Error('Missing exact first-drive parking release');
if(hash(Buffer.from(added))!==hash(fs.readFileSync(local+'/CF_OrdinaryOutboundBayCourse.c')))throw Error('Review script differs from validated source');
if(fs.existsSync(archive)||fs.existsSync(runtime))throw Error('Freeze destinations already exist; preserve them');
fs.mkdirSync(path.dirname(archive),{recursive:true});fs.cpSync(source,archive,{recursive:true});fs.mkdirSync(deployment,{recursive:true});
const trio={};for(const file of ['addon.gproj','data.pak','resourceDatabase.rdb']){fs.copyFileSync(pack+'/'+file,deployment+'/'+file);const b=fs.readFileSync(deployment+'/'+file);trio[file]={bytes:b.length,sha256:hash(b)};}
for(const file of ['addon.gproj','resourceDatabase.rdb'])if(hash(fs.readFileSync(source+'/'+file))!==trio[file].sha256)throw Error('Authoring/deployment mismatch '+file);
fs.writeFileSync(runtime+'/source-manifest.json',JSON.stringify(final,null,2)+'\n',{flag:'wx'});
const verification=await verifyOriginalFollowSources({manifestPath:runtime+'/source-manifest.json',copiedRoot:archive,packPath:deployment+'/data.pak',expectedPack:trio['data.pak'].sha256});
fs.writeFileSync(runtime+'/source-pack-verification.json',JSON.stringify(verification,null,2)+'\n',{flag:'wx'});
let contract=JSON.parse(fs.readFileSync(local+'/course-contract.json'));
Object.assign(contract,{
 setupOwnership:'Actual stopped owner pilot seat lets the unchanged session capture original lead staging; original native pilot then reenters and owner returns passenger before driving. First exact parking lease releases only after successful native first-drive order; later native legs use StartCommandRestart.',
 motionProof:'One continuous exact original activity/waypoint/lease and original driver/truck/predecessor. Signed projection onto previous actual NativeTarget goal axis, including negative progress. Independent net approach to the fixed route endpoint must reach 100m outbound or20m fresh Resume within that same continuous activity, each with at least3 powered intervals.',
 freshResumeEligibility:'Greatest observed sequence across all pre-Resume phases1–6 includes inherited stopping/arrival samples and current exact activity. Frozen immediately before real Resume; post-Resume samples excluded. Resume must be strictly greater and satisfy the same continuous20m/net20m/3powered evidence.',
 cameraScope:'Anonymous CameraBase/CameraManager only. Lead/tail midpoint when span<=100m; follower-centered at45m height when span>100m, explicitly tail_only=true and no lead framing claim. During parking use actual tail/planned-slot midpoint and actual span. Camera transform/FOV only; rendered visibility/occlusion/overhead remain unqualified.',
 cameraNativeCaptureMaximum:15,
 inheritedBayAnnotations:'Conservative inherited BAY_FEATURE fields still say full_trip/following/arrival claims false. The new ORDINARY_BAY_PREFIX_RESULT separately owns actual outbound/180s arrival/Hold/Resume evidence. Old lead_bay_approach fields are zero because the old short approach phases are bypassed, not measured by that legacy counter.',
 handoffAudit:{firstDriveReleaseAfterOrder:true,laterNativeDrivesReleaseViaStartCommandRestart:true,terminalHandlingUnchanged:true,failedStartNotReleasedByNewSuccessPath:true},
 validation:{allFiveConfigurationsPassed:true,packPassed:true,logs:build+(finalFreeze?'/workbench-resume-baseline':'/workbench-parking-release'),packedSource:pack,initialFailedValidation:'workbench/validate: observer field Sequence conflicted with engine type; repaired to QualifiedSequence. Failed log retained.',subsequentRebuildsRetained:true},
 frozenSource:archive,runtimeParent:runtime,deployment,sourceFiles:final.length,packedTextResources:verification.checks.filter(e=>e.packMatches!==undefined).length,trio,
 sourceAudit:{changes,deleted,restorationChecks,originalDriverSha256:final.find(e=>e.path==='Scripts/Game/CF_DriverControllerComponent.c').sha256,newWrapperReadOnlyDepartureObservation:true,guardMutationScanPassed:true,actorTransformMutationScanPassed:true},
 liveRun:false,reviewBeforeLiveExecution:true
});
fs.writeFileSync(local+'/course-contract.json',JSON.stringify(contract,null,2)+'\n');
fs.writeFileSync(runtime+'/course-contract.json',JSON.stringify(contract,null,2)+'\n',{flag:'wx'});
const review={status:'PREPARED_UNLAUNCHED',...contract,sourceVerification:runtime+'/source-pack-verification.json',runtimeGuidChildren:fs.readdirSync(runtime,{withFileTypes:true}).filter(e=>e.isDirectory()).map(e=>e.name),ordinaryV8UnchangedAndClosed:true};
if(review.runtimeGuidChildren.length!==1)throw Error('GUID parent isolation failed');
fs.writeFileSync(local+(finalFreeze?'/frozen-final-review.json':'/frozen-review.json'),JSON.stringify(review,null,2)+'\n',{flag:'wx'});
fs.writeFileSync(runtime+'/frozen-review.json',JSON.stringify(review,null,2)+'\n',{flag:'wx'});
console.log(JSON.stringify({status:review.status,sourceFiles:review.sourceFiles,packedTextResources:review.packedTextResources,trio,changes:changes.map(e=>e.path),sourceVerification:review.sourceVerification},null,2));
