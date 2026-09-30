import fs from 'node:fs';import crypto from 'node:crypto';
const root='.cache/ordinary-outbound-bay-v2',build='D:/ReforgerAgentBuilds/ordinary-outbound-bay-v2';
const base='D:/ReforgerAgentBuilds/ordinary-outbound-bay-v1/frozen-source-final/source';
const dest=build+'/source/ConvoyFollower',file='Scripts/Game/Tests/CF_OrdinaryOutboundBayCourse.c';
if(fs.existsSync(dest))throw Error('Preserve existing V2 source');
const original=fs.readFileSync(base+'/'+file,'utf8');let code=original;
const edits=[];
function replace(before,after,label){before=before.replaceAll('\r\n','\n');after=after.replaceAll('\r\n','\n');if(code.split(before).length!==2)throw Error('Unique seam '+label);code=code.replace(before,after);edits.push({label,before,after});}
replace(' protected void JourneyPhase(int phase)\r\n {\r\n',' protected void JourneyPhase(int phase)\r\n {\r\n  if (m_bPacedTerminal) return;\r\n','journey phase stops at terminal');
const observer=` override protected void BayPhase(int phase)
 {
  if (m_bPacedTerminal) return;
  super.BayPhase(phase);
 }
 // Copy/read the unchanged native seat-handoff conjunction; no actor/control writes.
 protected bool JourneyHandoffReadback(bool toOwner, string seam)
 {
  bool parkAllowed = CanCommandPark(); float speed = CommandSpeed(m_Lead);
  bool speedRejected = speed > 2; // Preserve the existing rejection comparison.
  bool directionReady; bool nativeControl; bool selectedWait; bool ownerPilot; bool nativeOnFoot;
  if (toOwner)
  {
   nativeControl = CanControlLead(); selectedWait = LeadWaitSelected();
   directionReady = nativeControl && selectedWait;
  }
  else
  {
   ownerPilot = CommandSeat(m_PacedOwner, true); nativeOnFoot = CommandOnFoot(m_Pilot);
   directionReady = ownerPilot && nativeOnFoot;
  }
  bool accepted = parkAllowed && !speedRejected && directionReady;
  string line = "[ConvoyFollower] ORDINARY_BAY_HANDOFF_PRECONDITIONS: phase=" + m_iJourneyPhase + " bay_phase=" + m_iBayPhase + " seam=" + seam + " to_owner=" + toOwner;
  line += " can_command_park=" + parkAllowed + " speed_kmh=" + speed + " speed_over2_rejected=" + speedRejected + " speed_nan=" + (speed != speed);
  line += " native_control_required=" + toOwner + " native_control=" + nativeControl + " selected_wait_required=" + toOwner + " selected_owned_wait=" + selectedWait;
  line += " owner_pilot_required=" + !toOwner + " owner_pilot=" + ownerPilot + " native_onfoot_required=" + !toOwner + " native_onfoot=" + nativeOnFoot;
  line += " unchanged_conjunction_accepted=" + accepted + " original_identities=" + BayIdentities() + " read_only=true";
  Print(line);
  return accepted;
 }
 override protected void BeginCommandSeatTransfer(bool toOwner)
 {
  if (m_bPacedTerminal) return;
  JourneyHandoffReadback(toOwner, "native_begin_call");
  super.BeginCommandSeatTransfer(toOwner);
 }
` .replaceAll('\n','\r\n');
replace(' protected void JourneyCapture(string tag)\r\n',observer+' protected void JourneyCapture(string tag)\r\n','read-only handoff diagnostics and terminal guarded bay phase');
replace('total_world_s=1650 setup_s=95 outbound_s=240','version=2 startup_wait_selection_s=10 total_world_s=1650 setup_s=95 outbound_s=240','version and selected-Wait subbound metadata');
replace('m_iJourneyPhase == 0 || m_iJourneyPhase == 11 || m_iJourneyPhase == 12','m_iJourneyPhase == 0 || m_iJourneyPhase == 13 || m_iJourneyPhase == 11 || m_iJourneyPhase == 12','selected-Wait stage within unchanged95world setup');
replace('    HoldLeadAtRoadGoal(); BeginCommandSeatTransfer(true); JourneyPhase(11); return;','    HoldLeadAtRoadGoal();\r\n    if (m_bPacedTerminal) return;\r\n    JourneyPhase(13); return;','one Wait request; no immediate seat transfer');
const selectedStage=`  if (m_iJourneyPhase == 13)
  {
   if (!JourneyHandoffReadback(true, "startup_selected_wait_poll"))
   { if (now - m_fJourneyPhaseMs > 10000) JourneyFail("startup_selected_owned_wait_handoff_10world_timeout"); return; }
   BeginCommandSeatTransfer(true);
   if (m_bPacedTerminal) return;
   JourneyPhase(11); return;
  }
`.replaceAll('\n','\r\n');
replace('  if (m_iJourneyPhase == 11)\r\n',selectedStage+'  if (m_iJourneyPhase == 11)\r\n','bounded selected-Wait handoff phase');
replace('   JourneyMotion(m_JourneyOutbound, false);','   JourneyMotion(m_JourneyOutbound, false);\r\n   if (m_bPacedTerminal) return;','stop outbound callback after terminal motion failure');
replace('   JourneyMotion(m_JourneyResumeEvidence, true);','   JourneyMotion(m_JourneyResumeEvidence, true);\r\n   if (m_bPacedTerminal) return;','stop Resume callback after terminal motion failure');
let restored=code;for(const e of [...edits].reverse()){if(restored.split(e.after).length!==2)throw Error('Restore seam '+e.label);restored=restored.replace(e.after,e.before);}if(restored!==original)throw Error('Restored V1 source differs');
fs.mkdirSync(root,{recursive:true});fs.mkdirSync(build,{recursive:true});fs.cpSync(base,dest,{recursive:true});fs.writeFileSync(dest+'/'+file,code);fs.writeFileSync(root+'/CF_OrdinaryOutboundBayCourse.c',code,{flag:'wx'});
const hash=b=>crypto.createHash('sha256').update(b).digest('hex').toUpperCase();
const audit={baseSource:base,sourceRoot:dest,buildRoot:build,changedFile:file,baseSha256:hash(Buffer.from(original)),candidateSha256:hash(Buffer.from(code)),restoredV1ByteTextMatches:true,edits,
 seamAudit:[{seam:'Journey startup phase0',before:'Wait request and BeginCommandSeatTransfer in samecallback',after:'One request then phase13 polls unchanged exact conjunction;10world subbound within95world setup'},
 {seam:'Journey arrival phase3',condition:'JourneyArrivalReady requires exact selected lead/follower Wait continuously for180world before handoff',unchanged:true},
 {seam:'Journey resumed corridor phase8',condition:'LeadWaitSelected and speed<=2 before handoff',unchanged:true},
 {seam:'Inherited bay first stop phase1',condition:'LeadWaitSelected and speed<=2 before handoff',unchanged:true},
 {seam:'Inherited bay clear phases6/16',condition:'LeadWaitSelected and speed<=2 and powered native motion before handoff',unchanged:true},
 {seam:'Inherited bay/return owner-to-native',condition:'Existing owner pilot/native pilot onfoot and parked ownership predicates; original helper retained',unchanged:true},
 {seam:'All JourneyPhase and inherited BayPhase transitions',after:'Private terminal guard prevents phase writes after failures; native handoff logs unchanged reject terms then calls original helper'}],
 productionSourceChanged:false,originalDriverSourceUnchanged:true,guardResetRearm:false,routeGeometryCargoUnchanged:true,numericAcceptanceUnchanged:true,totalWorldSeconds:1650,setupWorldSeconds:95,selectedWaitSubboundWorldSeconds:10,launcherWallSeconds:5000,snapshotWallSeconds:5100,liveRun:false};
fs.writeFileSync(root+'/edit-review.json',JSON.stringify(audit,null,2)+'\n',{flag:'wx'});
console.log(JSON.stringify({sourceRoot:dest,changedFile:file,restoredV1ByteTextMatches:true,candidateSha256:audit.candidateSha256,liveRun:false},null,2));
