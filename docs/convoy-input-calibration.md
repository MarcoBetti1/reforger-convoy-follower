# Player action calibration and physics measurement

September 26, 2026. These are private fixtures, not proof of keyboard/mouse delivery, panel usability, general driving or a completed supply trip. All failed runs remain failed. The [current validation record](convoy-validation-current.md) tracks product acceptance.

## Why this work matters

Earlier scripted vehicle-control writes were overwritten before physics. A different, narrowly owned path writes `CarThrust` in the exact local player's character `OnPrepareControls` callback before calling the native implementation. The actual truck now accelerates uphill with AI inactive and the original pilot retained. This can support a more controllable test lead; it does not establish the same callback behavior for an AI driver or replace the ordinary-input release gate.

The controller uses only the callback-provided action manager, checks original player/pilot/truck/world ownership, and releases its action back to neutral. It does not teleport the truck, write its physics transform, force an input context, or operate desktop controls.

## Preserved comparisons

| Run / frozen package | Physical observations | Fixture verdict |
| --- | --- | --- |
| `action-input-calibration-v3` / `78046F6B...` | 453 positive writes/readbacks; 20.0152 m signed uphill travel, +2.49131 m elevation. Same-pair entity origins were unchanged despite movement between pairs. | FAIL: zero measured powered intervals and one unmatched callback. |
| `action-input-calibration-v4` / `B5F2395E...` | 454 writes/readbacks; 20.0632 m signed travel, +2.49815 m elevation; 405 consecutive completed-post direct-body intervals contributing 20.062 m. | FAIL: one unmatched callback across baseline and drive phases. |
| `action-input-calibration-v5` / `3E241FA4...` | 453 writes/readbacks; 20.0648 m signed travel, +2.49839 m elevation; 405 powered intervals contributing 20.062 m. Read-only active-state events identify the baseline suspension. | FAIL: the same unmatched-callback gate remains unchanged. |
| `action-input-calibration-v6` / `5EA78318...` | 453 writes/readbacks; 20.0071 m signed travel, +2.49014 m elevation; 405 powered intervals / 20.062 m. One prospectively classified neutral suspension, zero unexplained gaps. | Engine-action calibration PASS; strict full-run FAIL on retained errors. |

V4's [independent report](../.cache/client/runs/action-input-calibration-v4/action-input-independent-result.md) retains sparse samples, instrumented aggregates, exact identities, all errors and provenance. V5's immutable console, terminal snapshot, source/deployment manifests and experiment note are in `.cache/client/runs/action-input-calibration-v5/`.

The numeric acceptance thresholds remain at least 30 matched callbacks, 20 powered intervals, 5 m powered contribution, 30 positive writes/readbacks, 10 m signed drive travel, 0.25 m elevation gain, neutral release and zero unexpected unmatched callbacks. A later correction must declare its observation boundary prospectively and retain all raw interrupted-step evidence; it must not recalculate these failures into passes.

## What the measurement corrected

Entity `GetOrigin()` publication did not expose the within-step displacement in these runs. A read-only `Physics.GetDirectWorldTransform` lookup did. The sole credited metric is now the signed displacement between adjacent completed POST observations, within one drive epoch, with exact original entity/pilot, valid controls at both complete pairs, consecutive ordinals and no intervening readback fault. Within-pair values remain diagnostics. Sparse log samples cannot independently reconstruct every credited aggregate interval.

The first measurement build passed compilation but warned that `Physics` pointers can only be local variables. That candidate was not launched. The corrected build uses fresh local getters and persists numeric observations only. Original entity continuity is checked; physics-body generation continuity between callbacks is explicitly unverified. The warning and initial candidate remain in `.cache/workbench-runs/convoy-action-measurement-v1/`; the corrected v4 and diagnostic v5 packages each passed all five configurations and packaging.

## Baseline suspension observed in v5

At world time **7030.68 ms**, the original truck emitted `EOnPhysicsActive(false)` during neutral phase 1. PRE ordinal 75 was pending, POST ordinal 74 was the last completed observation, and the pending PRE had that same world timestamp. The event's local `Physics.IsActive()` read was false. At phase 2 entry, **7814.03 ms**, the body was still observed inactive. A new PRE then found it active; the wake event followed at the same world time. The old pending PRE caused the unchanged pairing gate to record one gap.

This directly links the unfinished baseline observation to a reported sleep/wake boundary. It does not establish a universal engine event-order guarantee or justify ignoring gaps during powered driving. V5 retains its FAIL. Any future handling must be restricted to an explicitly observed, neutral stationary baseline suspension, preserve a separate interruption count, never credit motion across it, and continue to fail unknown or powered-phase gaps.

## Closure and visual limits

V3 and v4 have independently recorded natural client exits, terminal snapshots and completed recorders. V4 retained nine runtime and 64 shutdown errors. For v5, root consumed natural client exit 0, then stopped the sole remaining recorder with `q` and observed its exit 0; no game, Workbench or ffmpeg process remained. The initial exit callback fired at 29999.3 ms, exercised its single 100 ms deferral, then requested native exit at 30.0993 seconds. Cleanup and `Game destroyed` followed. This verifies the isolated action fixture's early-timer branch; it does not transfer that result to the separate paced probe. V5 retains nine runtime and 64 shutdown errors.

V5 full console SHA-256: `0471D498CB2F2CB9606806FD3C2BAA520FED70D0AB3ADB62A6833FD00DCE4ED6`. Terminal snapshot: `8411C080CF23A8687470BE42DF61EB4628B1CF3649BE403D32A789184A3D713B`, 108,790 bytes, captured 63 ms after terminal. V5 video: 101.2 seconds / 1,517 frames / 91,906,969 bytes, video only. Root inspected one early live capture, not full playback. V4's visual scope is two live captures. Recording/container validity does not prove continuous visibility, ordinary input or precise stopped motion.

## V6 completed after explicit goal resumption

The frozen v6 source `4C754B64F0AECC2BBCF3DB8C37D076B43F0D360C2AC670DF11E9544FD1E5B25C` passed independent review, all five configurations and packaging, then its declared physical calibration. See the [independent report](../.cache/client/runs/action-input-calibration-v6/action-input-independent-result.md). At world 7094.44 ms, the owned inactive event classified pending PRE76 after completed POST75: zero action writes, fresh neutral controls, 0.242739 km/h, and no pair/motion credit. Raw unfinished/classified counts remain 1/1, with zero unexpected gaps or POSTs. V3/v4/v5 remain failed under their original contracts. Rejection branches are source-reviewed, not all live-exercised.

Neutral release left the truck coasting at 2.21427 km/h in the last printed sample; this is no braking or precise-stop proof. Original pilot/entity continuity is supported, but physics-body generation continuity and ordinary desktop input remain unproved. The result enables further bounded engine-action fixtures without authorizing injection over an active NPC movement controller.

Natural client exit, cleanup and recorder `q`/exit 0 were confirmed. Nine runtime and 64 shutdown errors remain, so strict full-run status is FAIL. Full console SHA-256 `3571A527DA788050CADC1762D6E1732BFB30EE4FF20460DA83468A81A99E1946`; terminal snapshot `9BF1B4812B7D4A38B66A4F08D09696A2EF5311554175DF7BBB0C91A64D4ADF46`, verified prefix captured 50 ms after terminal. Private video: 122.8 seconds, 1,841 frames, 100,825,209 bytes; SHA-256 `D7F755CFE527F7B12165A9B61083E9AEB8FFC22F73B8AC66BA804B4CA2F79FEE`. Root inspected two live captures, not full playback.
