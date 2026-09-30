import fs from 'node:fs';
import path from 'node:path';
import crypto from 'node:crypto';
import {verifyOriginalFollowSources} from '../startup-pair-v1/analysis/original-follow-report.mjs';
const hash=b=>crypto.createHash('sha256').update(b).digest('hex').toUpperCase();
const source=path.resolve('.cache/everon-journey-scene-v8/source/ConvoyFollower'),packed=path.resolve('.cache/packed/everon-journey-scene-v8'),parent=path.resolve('.cache/frozen-addons/everon-journey-scene-v8');
if(fs.existsSync(parent))throw Error('Destination already exists');
const child=path.join(parent,'ConvoyFollower_5A5FB20BD40C7C70'),copied=path.resolve('.cache/frozen-source/everon-journey-scene-v8/source');
fs.mkdirSync(child,{recursive:true});
function walk(dir,prefix=''){return fs.readdirSync(dir,{withFileTypes:true}).flatMap(e=>e.isDirectory()?walk(path.join(dir,e.name),prefix+e.name+'/'):[prefix+e.name]);}
const manifest=walk(source).sort().map(file=>{
 const b=fs.readFileSync(path.join(source,file)),target=path.join(copied,file);
 fs.mkdirSync(path.dirname(target),{recursive:true});fs.writeFileSync(target,b);return {path:file,bytes:b.length,sha256:hash(b)};
});
fs.writeFileSync(path.join(parent,'source-manifest.json'),JSON.stringify(manifest,null,2)+'\n');
const deployment=['addon.gproj','data.pak','resourceDatabase.rdb'].map(file=>{
 const b=fs.readFileSync(path.join(packed,file)),target=path.join(child,file);fs.writeFileSync(target,b);return {file,path:target,bytes:b.length,sha256:hash(b)};
});
fs.writeFileSync(path.join(parent,'deployment-hashes.json'),JSON.stringify(deployment,null,2)+'\n');
const pack=deployment.find(e=>e.file==='data.pak');
const verified=await verifyOriginalFollowSources({manifestPath:path.join(parent,'source-manifest.json'),copiedRoot:copied,packPath:pack.path,expectedPack:pack.sha256});
fs.writeFileSync(path.join(parent,'verified-provenance.json'),JSON.stringify({deployment,sourceArchiveRoot:copied,source:verified,scope:'Ordinary manual Everon scene, no automatic driver/menu/cargo observer. Main panel framing fix and instruction-preserving menu; static native render result is separate, normal input and full journey pending.'},null,2)+'\n');
console.log(JSON.stringify({sourceFiles:manifest.length,packedText:verified.checks.filter(e=>e.packMatches).length,deployment},null,2));
