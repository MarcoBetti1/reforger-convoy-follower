# Canonical pending-guide trip: delivery and branch exercised, restart interrupted

September 27, 2026. **Raw/physical/strict FAIL.** The private regression changes only pending-guide pose observation over canonical gameplay, using the six unchanged fixtures from the completed [Road81 integrated trip](road81-integrated-trip-v1.md). No sampled-head targeting, V2 prejoin retention or V4 arrival-credit instrumentation is included.

| Stage | Closed result; later movement separated below |
| --- | --- |
| Native supplies | **200 loaded, transported and delivered**; 482 full-log ledger rows conserve1,800; final **1,600/0/0/200** |
| Baseline travel | Lead264.405m/31 powered; head136.10237m/31; tail241.62181m/49, genuine epoch1 join |
| Arrival / real Hold | **180.51s /180 Wait samples each**, then **51.1982s /51 each**, zero sampled drift |
| Head fresh Resume | Sequence5 earned **38.23612m /10 powered**, first qualified at24.5597m/8 |
| Tail fresh Resume | Sequence6 earned **7.354807m /3 powered** before abort; below20m |
| Peak gaps | **169.795m head /42.5945m tail**; head failed60m at01:28:40.603 |

## Terminal and lifecycle finding

The new pending-guide branch executed at **01:34:40.400**, epoch1/generation3, guide169/waypoint16A. The original tail subsequently selected that guide and moved. **No trail block occurred**, and original identities, route epoch/history and exact cargo containers remained consistent. This supports the narrow observation-lifecycle fix; it does not establish completed Resume or repair spacing.

At **01:34:46.750**, the head retired its moving sequence5 with `controller_clear`, `revoked=true`, **`failed=false`**, and bound the real-predecessor stopped approach, generation4/waypoint16B. At46.767 the group utility still exposed the retired old action as FAILED while the current waypoint was already16B. The fixture's `ResumeMemberContextReady` treated this stale selected action as fatal: `qualified_resume_activity_failed_unit_1`. Terminal reported `pass=false pair_complete=false`.

At46.817 the old lease explicitly logged `revoked_callback_no_request_failure`; the new sequence7 began executing at47.150. This is evidence of a fixture rejection during an owned arrival transition, not a demonstrated native movement-request failure. The raw failed result and earlier spacing failure remain unchanged. No retirement exception or qualification behavior was changed for this run.

## Later movement, excluded from qualification

During the30-second terminal grace, both original trucks continued and captured owned arrival Waits: head **01:34:56.850**, gap10.4539m; tail **01:35:04.916**, gap10.4354m. Final logged positions imply **53.617m head /52.184m tail** net movement along the restart axis from their Hold positions. These later observations receive **no terminal, powered-qualification or pair-completion credit**. They are separately preserved in the [post-terminal observations](../../.cache/client/runs/canonical-pending-guide-trip-v1/post-terminal-observation.json).

## Attribution, errors and closure

All-five validation/packaging passed. **585 frozen sources /three deployments** verified again after closure with no mismatch. All six fixture hashes match the closed Road81 control, and eight other canonical driving/Reboard files are unchanged. The sole gameplay change is Trail controller **`7D3F37FF7007CDE46DDF982804BE1A3964C326BACCA57E39BA08C8F66D0D0C64`**; current addon README is a separate documentation-only delta. Pack **`0CAC7F858F82D2436792230DBDBB4AFD29FDEA17690DCA90C3BA00C7A20D00BA`**; source manifest **`1C0B65BDA16F3EEF6BC5D8F40D0EEA7202EC266715AD9843701C6140D4FD169D`**.

Errors remain **9 gameplay /0 post-result /64 shutdown**. Full log4,405,653 bytes, SHA `F43C4DAAA4408D885BA23308FEE956004907E0E296BDC64D90EEE992B33CD67F`; terminal snapshot is an exact prefix captured **+169ms**, SHA `E7806662D76AA873CF9B0EDCA828A3A6ACA2CA380FD8B1E5712B07B2325D959F`. Native exit followed30.015s grace. Root consumed all three handles at exit0 and verified no remaining game/Workbench/ffmpeg processes or windows.

The silent private video is **530.266667s /7,953 frames /687,551,590 bytes**, SHA independently verified **`AE8203D77C251DCDEC321BFD819ECD03D680BAAC7FBFD4FC9507FAC3C1C93814`**. Root inspected three gameplay views—travel, arrival and unloading—plus a shutdown loading screen. No exact-terminal gameplay image or full playback. Normal player input, multiplayer, repeatability and release readiness remain unproved.

Evidence: [structured review](../../.cache/client/runs/canonical-pending-guide-trip-v1/independent-review.json), [source verification](../../.cache/client/runs/canonical-pending-guide-trip-v1/independent-source-verification.json), [root closure/video](../../.cache/client/runs/canonical-pending-guide-trip-v1/root-closure-video.json). The existing Road81 analyzer and qualification rules were reused unchanged; pending-branch events were added only as diagnostic evidence.

## Narrow integration

Root promoted only the two reviewed pending-guide pose-continuity hunks into the ordinary addon, preserving all eight other driving/Reboard files. No experimental head policy, settings, fixture or qualification change was promoted. The canonical-only source separately passed all five configurations and packaging as `5756BF2529FE850720653316C6354EFBC398E12F17B9E9F49114FCF4C5A7AB84`; the regression above adds six explicitly declared test overlays. The private [promotion record](../../.cache/canonical-pending-guide-pose-v1/promotion.json) pins the before/after source and protected dependencies. This integrates the exercised lifecycle repair; it does not turn this run into a full-trip or release pass.
