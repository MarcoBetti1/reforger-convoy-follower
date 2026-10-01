# Convoy control point — October 1, 2026

Testing is capped at **five followers plus the lead**, following Marco's latest direction. The implementation is on `codex/convoy-player-trip-20260929`, production code commit `883fe0d`. Source, voices and driver clothing are pushed; this checkpoint records validation and exact-package backup separately. Larger configurable fleets and numbered clips are capabilities, not tested fleet sizes.

The explicit bay sequence is **Save bay / Hold queue → Set waiting area → Admit → native rear Unload → Park unloaded truck**, repeated for each truck, then **Depart / return convoy** from the original lead. On-foot admission/parking near the original stopped lead is supported. The loading area can be off-road if native navigation can reach it. Native supplies are enabled in the five-truck Everon scene, with two900-supply source stacks for front/rear access and a3000-capacity destination. The source count is1800, not automatic cargo delivery.

Start, Resume, bay readiness, parking, departure and Finish acknowledgements are included in188 generated clips; original recordings remain selectable. Spacing/range warnings share a90s cooldown and sustained-pressure/hysteresis checks. Drivers use ground infantry clothing. Finish means seated Hold; healthy owned drivers suppress combat dismount behavior, while destroyed/incapacitated/ownership-ending safety behavior remains.

## Verified results and unresolved work

The focused one-truck V8 bay course passed100-supply native delivery, on-foot parking with30s zero drift and public Depart followed by26.6949m/8 powered intervals under one exact original movement order. It does not establish five sequential unloads or home arrival.

The stop-episode fix retains the native Wait when the predecessor settles. The corrected road-aligned V16 fixture established all-five original powered arrival and180.959s common stationary observation with zero measured drift. Its overall result still fails81.862m peak spacing against60m. V11's earlier LOST fifth truck,390.448m peak gap and2.37491m captured-Wait drift remain failed history; that fixture's last truck started11.5m off-road and repeatedly reversed. Marco's successful manual route remains useful separate evidence. The worse all-link pacing candidate is excluded from the player package. Complete sequential unloading/home return and repeatability are separate checks.

Combat results, including failed enemy fixtures and the projectile crash, are preserved in [the fleet report](convoy-fleet-unload-2026-09-30.md). A confirmed hit and a surviving driver continuing powered travel must both be observed before claiming the combat feature passed. Ordinary rear-menu visibility, the complete five-truck bay/return cycle, smoother native approach, repeatability and video capture remain open.

## Current player session

The ordinary Player V3 session reached GAME at11:16:22 on October1, registered the command listener and enabled native supplies. It is open **without a time limit** in Game Master at Regina. Marco owns deployment and all player input; no deployment was automated. The bare world is daylight (12:01 observed), despite its night-world name; the mission header22:00 was not applied. See [startup evidence](experiments/convoy-fleet-player-startup-20261001.json). This startup is not completion of a rear-action, convoy, bay or return test.

## Backup and cache audit — October 1

The exact archive **convoy-fleet-unload-v2.zip** is backed up in the repository's [draft takeover release](https://github.com/MarcoBetti1/reforger-convoy-follower/releases/tag/untagged-92cd715b1ebbf43e8666). It contains12,312 logical files in975 verified blobs, including current Player V3, frozen successful/failed courses, build logs,188 voice assets and full closed console/error/crash text. All payload hashes passed locally; downloading the uploaded878,464,333-byte archive produced the same SHA-256 `CD352EBFDF99495AC808E12198E20A78B56C20DB74EC66E13C8F947B0FA31C07`. See [backup verification](experiments/convoy-fleet-unload-backup-20261001.json).

After that verification,33 superseded cache/source/runtime folders were removed, freeing2,220,436,680 bytes (2.22GB). Every removed file was matched to the archive by hash before deletion. Current Player V3, the passing V8 bay fixture, the corrected V16 five-truck fixture, full logs/evidence and prior rollback packages remain. The [deletion/restoration index](cleanup/convoy-fleet-archive-cleanup-20261001.json) maps every old local path to its archive path. Extract `restore.py` from the archive and run it with the ZIP and a **new empty destination**; it verifies all restored content and refuses to overwrite an existing directory. No active player files were removed.

## Working locations

| Purpose | Location |
|---|---|
| Addon implementation | `C:/Users/marco/.codex/worktrees/175a/REFORGER` |
| Control/docs/tools | `C:/Users/marco/Desktop/REFORGER`, `codex/release-hardening` |
| Independent generic harness | `C:/Users/marco/Desktop/reforger-agent-harness` |
| Current frozen ordinary player source | Implementation `.cache/fleet-workflow-player-v3/source/ConvoyFollower` |
| Current frozen ordinary runtime parent | Implementation `.cache/player-addons/fleet-workflow-player-v3` |
| Closed private full logs | `D:/ReforgerAgentRuns/fleet-*` |
| Exact source/runtime/evidence archive | `D:/ConvoyTakeoverBackups/2026-09-30/convoy-fleet-unload-v2.zip` (verified locally and after GitHub download) |
| Previous verified rollback archive | `convoy-bay-voices-uniform-v4.zip` in the same backup root / draft takeover release |

The ordinary runtime contains no private fleet or attack orchestrators. Player V3 passed five-configuration compilation, packaging and231 packed-text comparisons against its852 source files. Its unique runtime child is `ConvoyFollower_5A5FB20BD40C7C70` and its trio must stay together:

| File | Bytes | SHA-256 |
|---|---:|---|
| addon.gproj | 135 | `1DFE7E1AE0E2194C703300ED5D60FB41FDD3D365282A0A5FC03E86B622CB3579` |
| data.pak | 66650993 | `5F831B62CBECA399ACB8D92661420947297ED70A502BA36DF862C07173AF72A7` |
| resourceDatabase.rdb | 43291 | `6AA9ABA1FC4E96F00B70177428985DFC5418F470E5FA3026D55304E4AA7C402B` |

Private courses use bounded clients and normal native AI leads. Ordinary user testing uses `--keep-open`; Marco owns deployment and player input. No source Workbench is open. Do not edit frozen packages. A GAME startup marker verifies loading, never the scenario result.

Start with [current validation](convoy-validation-current.md), [the fleet report](convoy-fleet-unload-2026-09-30.md), and [player steps](convoy-player-guide.md). Earlier dated builds and running-session descriptions are historical. The [storage audit](cleanup/convoy-storage-audit-2026-09-30.md) records71.88GB removed and the deletion/restoration index. Broad other-chat goals remain paused; this focused development and validation is Marco-authorized.
