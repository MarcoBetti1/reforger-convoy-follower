// Offline receipt companion. It does not launch, move actors or certify a run.
import fs from 'node:fs';import path from 'node:path';import crypto from 'node:crypto';
const run=path.resolve(process.argv[2]??'');if(!process.argv[2])throw Error('Usage: node tools/convoy-route-sidecar.mjs <closed-run-directory>');
const receiptPath=path.join(run,'receipt.json'),receipt=JSON.parse(fs.readFileSync(receiptPath));
if(!receipt.finishedUtc && !receipt.cleanup && !receipt.exitCode && !receipt.firstFailure)throw Error('Closed receipt required');
const log=path.join(run,'logs/console.log'),raw=fs.readFileSync(log,'utf8'),lines=raw.split(/\r?\n/);
const hash=p=>crypto.createHash('sha256').update(fs.readFileSync(p)).digest('hex').toUpperCase();
if(receipt.logHash && hash(log)!==receipt.logHash)throw Error('Receipt log identity mismatch');
const out=path.join(run,'route-sidecar.json');if(fs.existsSync(out))throw Error('Preserve existing sidecar');
const buildRoot=path.dirname(run),overlay=path.join(buildRoot,'source-overlay');
const number=(s,k)=>{const m=s.match(new RegExp('(?:^|\\s)'+k+'=([-+\\d.eE]+)'));return m?Number(m[1]):null;};
const vec=(s,k)=>{const m=s.match(new RegExp(k+'=<([^>]+)>'));return m?m[1].trim().split(/[ ,]+/).map(Number):null;};
const id=(s,k)=>{const m=s.match(new RegExp('(?:^|\\s)'+k+'=(0x[\\da-fA-F]+|\\d+|[A-Za-z_][\\w]*)'));return m?.[1]??null;};
const vectors=s=>[...s.matchAll(/Vector\(\s*([-\d.]+)\s*,\s*([-\d.]+)\s*,\s*([-\d.]+)\s*\)/g)].map(m=>m.slice(1).map(Number));
const scene=receipt.scenario??null,loaded=lines.find(s=>s.includes("Entities load '"+scene+"'"))??null;
const sceneFile=scene?path.join(overlay,'addons/ConvoyFollower',scene):null;
const sceneSource=sceneFile&&fs.existsSync(sceneFile)?fs.readFileSync(sceneFile,'utf8'):null;
const parentWorld=sceneSource?.match(/Parent\s+"([^"]+)"/)?.[1]??null;
let planned=[],routeBasis='No fixed planned trace in receipt; no geometry invented';
const bridgeSource=path.join(overlay,'addons/ConvoyFollower/Scripts/Game/Tests/CF_PlayerRegressionProbeComponent.c');
const bendSource=path.join(overlay,'addons/ConvoyFollower/Scripts/Game/Tests/CF_PlayerRoadTransitionProbeComponent.c');
if(['Straight','Bridge','RetireBridge','RetireInjected'].includes(receipt.course)&&fs.existsSync(bridgeSource)){
 const src=fs.readFileSync(bridgeSource,'utf8'),start=src.indexOf('m_BridgeTrace='),end=src.indexOf('};',start);planned=vectors(src.slice(start,end));
 if(receipt.course==='Straight')planned=planned.slice(0,9);if(receipt.course==='RetireInjected')planned=planned.slice(0,12);routeBasis='Exact frozen recorded trace source (player actions only, no lead AI route)';
}else if(receipt.course==='Bend'&&fs.existsSync(bendSource)){const src=fs.readFileSync(bendSource,'utf8');planned=vectors(src.slice(src.indexOf('m_RoadTrace='),src.indexOf('};')));routeBasis='Exact frozen bend trace source';}
else if(receipt.course==='StaticBridge'){planned=[[9644.6598,21.4982,1656.48],[9487.33,35.4142,1807.2]];routeBasis='Declared fixed native waypoint endpoints; connecting line is not native path or driven centerline';}
const tracks=[],surfaces=[],failures=[];
for(let i=0;i<lines.length;i++){
 const s=lines[i],gameTime=s.match(/^\d\d:\d\d:\d\d\.\d+/)?.[0]??null,worldMs=number(s,'world_ms');
 let type=null,vehicle=null,unit=null;
 if(s.includes('[PlayerRegression] TELEMETRY')){type='player-lead';vehicle=id(s,'lead_id');unit=0;}
 if(s.includes('[PlayerRegression] PAIR')){type='follower';vehicle=id(s,'truck_id');unit=number(s,'unit');}
 if(s.includes('[StaticBridge] TELEMETRY')&&vec(s,'position')){type='solo-static-truck';vehicle=id(s,'truck');unit=1;}
 if(s.includes('[ManualConvoy] SAMPLE')&&vec(s,'position')){type='manual-follower';vehicle=id(s,'truck');unit=number(s,'unit');}
 if(type){const position=vec(s,'position');if(position)tracks.push({line:i+1,gameTime,worldMs,type,vehicle,unit,predecessor:id(s,'predecessor_id')??id(s,'predecessor'),position,speedKmh:number(s,'speed'),throttle:number(s,'throttle'),brake:number(s,'brake'),phase:number(s,'phase'),pathPoints:number(s,'path_points'),generation:number(s,'generation')});}
 if(s.includes('TEST_SURFACE_SAMPLE:')){const p=vec(s,'position');if(p){surfaces.push({line:i+1,gameTime,worldMs,vehicleName:id(s,'truck'),position:p,speedKmh:number(s,'speed_kmh')});}}
 if(/ORIGINAL_REQUEST_FAILURE_CONTEXT:|FIRST_SPACING_FAILURE|FIRST_BINDING_FAILURE|\[RetirementRegression\] FAILURE/.test(s)){
  const truck=s.match(/original_truck=.*? at <([^>]+)>/)?.[1],predecessor=s.match(/predecessor=.*? at <([^>]+)>/)?.[1];
  failures.push({line:i+1,gameTime,worldMs,phase:number(s,'phase'),nativeResult:number(s,'native_result'),handler:number(s,'handler'),generation:number(s,'generation'),truckPosition:truck?truck.trim().split(/[ ,]+/).map(Number):null,predecessorPosition:predecessor?predecessor.trim().split(/[ ,]+/).map(Number):null,raw:s});
 }
}
const videoFile=path.join(run,'recording-provenance.json');let recording=null;if(fs.existsSync(videoFile))recording=JSON.parse(fs.readFileSync(videoFile));
const terminal=lines.find(s=>s.includes('[PlayerRegression] RESULT'))??lines.find(s=>s.includes('[StaticBridge] RESULT'))??null;
const result={schema:1,createdUtc:new Date().toISOString(),runLabel:receipt.runLabel,build:receipt.build,engineVersion:lines.find(s=>/ENGINE.*(?:Game Version|Version:|Build:)/.test(s))??null,loadedWorld:{requested:scene,actualLoadLine:loaded,confirmed:!!loaded,parentResource:parentWorld,terrain:parentWorld?.includes('GM_Eden')?'Everon':parentWorld?.includes('GM_Arland')?'Arland':'unestablished',frozenWorldSource:sceneFile&&fs.existsSync(sceneFile)?{path:sceneFile,sha256:hash(sceneFile)}:null},identity:{receipt:receiptPath,receiptSha256:hash(receiptPath),log,logSha256:hash(log),packHashes:receipt.packHashes},route:{basis:routeBasis,plannedXYZ:planned,landmarks:parentWorld?.includes('GM_Eden')?[{name:'Saint Pierre staged approach',xyz:[9635.78,23.0627,1674.4]},{name:'Bridge exit reference from retained trace',xyz:[9531.38,30.3276,1761.97]},{name:'Static diagnostic fixed goal',xyz:[9487.33,35.4142,1807.2]}]:[],publicMapOnlyContext:true},clock:{gameTime:'Desktop-local HH:mm:ss.mmm from the same console line; not assumed UTC',worldMs:'Exact logged world time where present; no interpolation',video:recording,videoClockAligned:false,uninterruptedVideoReviewed:false},coverage:{planned:'Only listed trace/waypoint or requested manual course; no held-out/hill/rough qualification inferred',observed:'Periodic physical XYZ samples plus exact native failure poses. Surface samples without world_ms keep wall clock and null world time; they are not fabricated frame tracks',qualifiedOutcome:receipt.status??'See original receipt',leadValidity:receipt.inputValidity??receipt.leadValidity??null,followerOutcome:receipt.followerOutcome??null,perFramePeakFromTerminal:number(terminal??'','peak_gap'),originalVehicleIds:[...new Set(tracks.map(x=>x.vehicle).filter(Boolean))],surfaceVehicleNames:[...new Set(surfaces.map(x=>x.vehicleName).filter(Boolean))],missingXYZTracks:tracks.length===0,firstFailure:failures[0]??null,terminal,noPromotion:true},tracks,surfacePositions:surfaces,failures};
fs.writeFileSync(out,JSON.stringify(result,null,2)+'\n');
fs.writeFileSync(path.join(run,'route-tracks.csv'),'game_time,world_ms,type,vehicle,unit,predecessor,x,y,z,speed_kmh,throttle,brake,path_points,generation\n'+tracks.map(x=>[x.gameTime,x.worldMs,x.type,x.vehicle,x.unit,x.predecessor,...x.position,x.speedKmh,x.throttle,x.brake,x.pathPoints,x.generation].join(',')).join('\n')+'\n');
console.log(JSON.stringify({sidecar:out,loadedWorldConfirmed:!!loaded,plannedPoints:planned.length,tracks:tracks.length,surfacePositions:surfaces.length,failures:failures.length,status:receipt.status??null}));
