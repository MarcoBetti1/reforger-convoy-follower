# First complete native supply trip

**September 26, 2026 — physical PASS; strict full-run FAIL. The mod is not release ready.** One ordinary Convoy Driver and its original truck completed native loading, a road trip, stopped arrival, Hold, delivery to a distinct storage and powered Resume. The client then closed naturally. [Independent review](../.cache/client/runs/loaded-supply-trip-v1/independent-loaded-trip-review.md)

## What happened

| Step | Measured result |
| --- | --- |
| Load | Native rear action moved **100 supplies** from source to truck, then cancelled and remained stable. |
| Follow | The original follower sustained **321.054 m / 41 powered intervals** under the same activity; the native lead requested **20 km/h**. Peak separation was **45.8258 m**, below the unchanged 60 m gate. |
| Arrive | **180.413 seconds** of selected arrival Wait, with zero measured drift. |
| Hold and deliver | Real server Hold lasted **40.0991 seconds**, with zero measured drift. Native unload delivered **100** to the distinct destination while the truck was **326.469 m from the source** and 9.49148 m from that destination. |
| Resume | The same original driver/truck resumed under fresh sequence 3: **about 22.956 m / nine powered intervals**. Command acceptance was followed by measured movement. |

Source / truck / destination counts were **1,800 / 0 / 0 → 1,700 / 100 / 0 → 1,700 / 0 / 100**. All 400 cargo observations retained the exact original containers and conserved the total of 1,800, including after the result. Both native transfers passed exact queue checks and cancellation stability. No runtime resource-count setter or truck repositioning supplied the result.

**Errors remain: 9 gameplay / 0 post-result / 59 shutdown error lines.** The full log preserves them. Native exit followed the result by 30.0155 seconds; process exit, watcher completion and recording completion were verified without the safety timer terminating the client.

## Reproduce the frozen case

The [probe source](../addons/ConvoyFollower/Scripts/Game/Tests/CF_LoadedSupplyTripProbeComponent.c) and [world](../addons/ConvoyFollower/Worlds/Tests/ConvoyFollower_Arland_LoadedSupplyTrip_1Truck.ent) are separate from the manual Supply Day scene. Use a fresh run directory from the repository root:

```powershell
npm run client:run -- --run-dir .cache/client/runs/loaded-supply-trip-repeat-01 --world Worlds/Tests/ConvoyFollower_Arland_LoadedSupplyTrip_1Truck.ent --addon 5A5FB20BD40C7C70 --addons-dir .cache/standalone-loaded-supply-trip-v1-addons --duration-seconds 780 --expect-game --execute
```

This uses the frozen tested package, which passed all five script configurations and packaging. **Data SHA-256:** `6361D179EA75890F0D4C2BFF1A11A53A98A4BD64622CBAFDE037257740353D50`. The 780-second timer is a safety bound; normal fixture exit is automatic. `--expect-game` checks startup only. The final physical marker is `LOADED_TRIP_RESULT`.

## Demonstration and evidence

- [146-second highlights](../.cache/test-videos/loaded-supply-trip-v1-highlights.mp4): edited raw-recording intervals **42–95, 130–175 and 370–418 seconds**.
- [390-second gameplay crop](../.cache/test-videos/loaded-supply-trip-v1-gameplay.mp4): starts 35 seconds into the [453.6-second private full recording](../.cache/test-videos/loaded-supply-trip-v1-full.mp4). These videos are **silent**. Review covered sampled live views and frames, not complete playback; the full recording includes desktop footage.
- [Complete console](../.cache/client/runs/loaded-supply-trip-v1/logs/console.log), [terminal snapshot](../.cache/client/runs/loaded-supply-trip-v1/gameplay-at-terminal.log), [source manifest](../.cache/client/runs/loaded-supply-trip-v1/source-manifest.json), [three deployment hashes](../.cache/client/runs/loaded-supply-trip-v1/deployment-hashes.json), and [closure record](../.cache/client/runs/loaded-supply-trip-v1/closure-review.json). The review verified 577 frozen sources and an exact-prefix snapshot captured 200 ms after the result.

## Scope

This proves an autonomous **native-action supply trip** under the stated route and pace. The fixture loads before recruitment, stages the same test owner at the rear and in lead seats, uses a native AI lead, and invokes real server Hold/Resume. Ordinary keyboard/mouse menus, walking and human driving remain unproved. It does not establish repeated runs, two/three loaded followers, multiplayer or general release readiness. The [ordinary Supply Day instructions](convoy-supply-showcase.md) describe the player workflow still to inspect.

## Two loaded trucks: delivery achieved, restart still fails

The implemented [two-truck world](../addons/ConvoyFollower/Worlds/Tests/ConvoyFollower_Arland_LoadedSupplyTrip_2Trucks.ent) and coordinated probes now provide the next regression case. `loaded-pair-trip-v2` loaded100 each, transported both original loads over328m, held both drivers seated and unloaded200 to the common distinct destination. Arrival lasted180.313s; explicit Hold lasted48.1325s, both with zero measured drift. All451 four-container observations conserved1800 supplies.

**This is a partial milestone, not a full-trip pass.** Unit Two's peak gap81.625m exceeded60m. Resume was accepted but its retained route cursor rejected movement accumulated during the final approach; no fresh Resume movement was completed. The [independent review](../.cache/client/runs/loaded-pair-trip-v2/independent-review.md) preserves source/deployment verification,9 gameplay/0post/64shutdown errors and natural closure. The [130-second highlights](../.cache/test-videos/loaded-pair-trip-v2-highlights.mp4) show selected game footage; they are silent and edited, not a complete normal-input demonstration.

The tested pack is `037F30C6D6E18A284146536B7CEEBE6A08B547A4DE3AEC213C79A2F39701AE89`, deployed under `.cache/standalone-loaded-pair-trip-v2-addons`. Its six regression resources and generated database are canonical. The production controller remains the integrated arrival-recovery version. The next comparison repairs the actual Resume transition while preserving the tests below.

### Retained test design

Build the next isolated trip from the existing ordinary **two-follower mixed observer**, preserving this one-driver regression. Its current Hold/Resume observer deliberately requires one follower; adding a second truck alone would not prove both trucks' behavior.

Keep the ordinary groups, native lead, route, recruitment and per-unit travel/180-second arrival checks. Add two coordinated native cargo workers and sequential owner staging: load100 into each original truck, drive, issue real Hold to both for at least30 seconds, unload each into the same distinct destination, then Resume and prove at least20m/three powered intervals independently for both original assignments. Unit Two must retain its immediate predecessor and genuinely resume its recorded route. Preserve baseline and resumed evidence separately.

Use one shared source beside the midpoint of the actual starting cargo origins and the existing destination at `1667.4 19.8 3360.7`. Prior two-truck settled positions were roughly7.9m and7.0m from that destination; installed native ranges are12m. This makes a simple unload while Held plausible without requiring pull-off maneuvers, but native discovery/eligibility must verify the placement. No truck repositioning, range override or transfer fallback should rescue it.

Track the exact original source/cargo1/cargo2/destination containers throughout: **1800/0/0/0 →1600/100/100/0 →1600/0/100/100 →1600/0/0/200**. Only one transfer runs at a time; cancellation and three-second stability must precede the next transfer. Keep the existing native subscription warmup and exact contributor/destination checks. Two autonomous copies of the current worker would conflict over owner staging and conservation snapshots, so the coordinator must activate each explicitly.

The existing60m spacing and all movement/hold gates remain. A delivery can be observed while an overall driving gate still fails; report both. This fixture is now implemented and tested with the partial result above. Ordinary keyboard/menu use and multiplayer remain separate requirements.
