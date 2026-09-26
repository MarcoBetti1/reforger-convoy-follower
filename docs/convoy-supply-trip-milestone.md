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
