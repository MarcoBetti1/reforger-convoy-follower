# Loaded straight trip v3 — Resume from the original lead passenger seat

September 26, 2026. **Closed physical FAIL; strict full-run FAIL.** All five script configurations and packaging passed. Native delivery, arrival, Hold and passenger Resume acceptance worked; spacing and the complete fresh powered restart gate failed. This [two-file increment](loaded-straight-trip-v3.patch) applies after [loaded straight trip v2](loaded-straight-trip-v2.md). The Session change was subsequently integrated separately; private driving candidates and the pair fixture were not promoted. [Current validation](../convoy-validation-current.md) owns the latest result.

V2 delivered 200 supplies, passed peak links of 46.5385/55.1042 m, completed 180.446 s of stationary arrival and 48.0482 s of real Hold, then failed the test-only parking guard during its post-Resume seat transfer. The exact failing guard term was not recorded. V3 addresses that command/fixture sequence; it does not establish that any earlier failure was harmless.

## Development video

The private [132-second highlights](../../.cache/test-videos/loaded-straight-trip-v3-highlights.mp4) show loaded travel, stopped arrival, the native-unload interval and resumed movement. Persistent captions identify automated assistance and the failed acceptance gates. Cargo counts are corroborated by the conserved native-action ledger, rather than visible player UI. Arrival/restart views are partly obscured by trees.

Highlights SHA-256: `0466FB7FC085B0F39035F422AC3CBF176D048834A536C9B6C4457781E78AD7F1`. The run's `highlight-filter.txt` and `highlights.json` preserve exact raw intervals and crop. Five live views and sampled rendered frames were inspected; this is not a full-playback or normal-input demonstration. The full raw recording remains private and unchanged.

## Exact changes

- A Resume-only session helper accepts the actual controlled convoy owner in any fully settled seat of the head's cached lead vehicle, matching the session's original lead when recorded. It checks server/session authority, live owner and vehicle, world, exact occupancy, seat transitions and convoy reservations. All member eligibility and existing Resume tracking remain unchanged. Other maneuvers keep their pilot-only helper.
- The pair fixture keeps both original followers in real Hold throughout the existing native-pilot-first/owner-passenger handoff. After the original seats and entry checks pass, it calls public server Resume and starts the native lead in that same poll. Hold measurement continues until actual Resume. Once-only negative calls during on-foot unloading must reject the owner on foot and the unowned AI pilot, with both original assignments still held after each. These are local checks, not multiplayer authentication tests.
- The unchanged test parking guard now logs its first rejecting term, speed and drift. No movement policy, follower control writer, world or cargo machinery changes.

Retain native load/unload of 100 per truck and exact four-container conservation `1800/0/0/0 → 1600/100/100/0 → 1600/0/0/200`; native range and ≥250 m source/destination separation; original predecessor chain and powered travel; ≤60 m peak links; ≥180 s arrival/≤2 m drift; ≥30 s real Hold; and fresh continuous ≥20 m/three powered intervals per follower after Resume. Failures remain sticky. Recruitment, owner staging, native lead driving and scripted cargo/server actions remain disclosed assistance; ordinary input, human driving and multiplayer are unproved.

## Frozen build and evidence

- Build: `.cache/workbench-runs/convoy-loaded-straight-trip-v3`.
- Data package SHA-256: `515335F7452D9A9B3E1F9B0D7D91730AEE65B1F299E1AC7C0DB3FDAB69B9D783`.
- Frozen source manifest SHA-256: `922EEE71757B1A85C3FE0BA67B122133458A1FABC3ADA48134E96A2D8864FA1D`.
- Run: `.cache/client/runs/loaded-straight-trip-v3`; [closed independent review](../../.cache/client/runs/loaded-straight-trip-v3/independent-review.md), SHA `797CCB93ED080D6005B21B274C525FF28ABFAFE151F9578B105FA264574E9101`.
- Private source, hypothesis and original patch remain intact in `.cache/loaded-straight-trip-v3`.

## Closed result and next step

All **510 ledger rows** conserved the original 1,800 supplies; sequential native transfers delivered **200** to the distinct original destination. All three vehicles completed **180.228 s** stationary arrival, then both followers remained in real Hold for **51.1474 s /51 samples**, with zero measured drift, through unloading and the full lead-seat restoration. Actual on-foot-owner and unowned-AI Resume calls were rejected with both Holds and identities unchanged. Passenger Resume was accepted at 20:20:53.849, and the native lead restarted in the same callback. No parking rejection, parked-lead rejection, route block or failed original lease occurred before terminal.

Actual net restart movement was **39.5874 m lead /40.0264 m head /38.4547 m tail**. This is useful movement, but the unchanged proof requires continuous fresh moving activity and powered intervals:

| Member | Qualifying restart evidence | Gate |
| --- | --- | --- |
| Native lead | 39.5874 m, one powered interval | Three powered intervals not met |
| Head | Fresh sequence 5: 32.3173 m /four powered intervals | Individual 20 m/three gate passed |
| Tail | Fresh sequence 6: 11.6428 m /one powered interval | 20 m/three not met |

The lead reached its short downhill goal before the tail could retain enough moving-follow evidence. Subsequent stopped-target travel is not pooled into that segment. The production session's own completion message uses separate thresholds and cannot substitute for the fixture gate. The run ended at its 90 s restart timeout; **peak links 64.6878/54.6578 m** preserve the earlier head-spacing failure, which preceded all commands. A separately declared longer useful native restart leg is the next comparison, retaining every physical gate. No new candidate exists yet.

The review verifies 588 sources, three deployments and an exact terminal-prefix snapshot captured +68 ms. **9 gameplay /0 post-result /64 shutdown errors** remain. Natural closure, watcher/recorder completion and an empty process check are recorded. The silent 524.066667 s recording has SHA `CAF74CA5532F7C50EE56CBA2B0E3CDFEADD066A061B7739AD99AFD17199B5761`; five live views were inspected, not full playback or ordinary input.

Only Session `C8781654…43C9485` and the ordinary Supply Day source-position layer `6E05066B…EB33CA4` were integrated, recorded in `.cache/player-trip-controls-v1/promotion.json`. Canonical five-configuration validation and packaging passed in `.cache/workbench-runs/convoy-player-trip-controls-v1`, data SHA `DA64B3A01FBA62A72472736F8A67C69231BF5A63664170B4A89F47DAC55529B0`. The ordinary scene's destination is unchanged; its native loading reach and player workflow still need validation. Private driving changes remain separate, and this canonical package has no integration/regression or ordinary-input pass.

## Apply after V2

```powershell
git apply --check docs/experiments/loaded-straight-trip-v3.patch
git apply docs/experiments/loaded-straight-trip-v3.patch
```

The patch has normalized `addons/ConvoyFollower/` paths and LF text. `git apply --check` passed against isolated, hash-verified exact V2 copies in `.cache/loaded-straight-trip-v3/archive-apply-check`; that check applied no patch. Candidate bytes match this run's frozen source. The later Session-only integration means this increment must not be blindly reapplied to current canonical source. Rebuild matching package/database files before use; no binaries or database are archived here.

| File relative to `addons/ConvoyFollower/` | V2 SHA-256 | V3 SHA-256 |
| --- | --- | --- |
| `Scripts/Game/CF_ConvoySession.c` | `729B457B9792B73168EDA438D8D800DDE88C4EA7773943D4B5140ECC14FC68A8` | `C87816547191185B98468B6690EE6483C85FA128FC0CC82DDF76B4DAB43C9485` |
| `Scripts/Game/Tests/CF_LoadedPairTripProbeComponent.c` | `23E6F4A2D3BD120BB58EFD807084B1796F47586A23962D8F3593FC7C931C6E60` | `E64A9A817304E3CD72A313766AF9772BD9C7E3BDD31A3AA8222655BBE80D14F1` |

Patch SHA-256: `85C5B34E87A9D54F359376145DEEB01466B702FBABA047F5BAF049980D77E80E`.
