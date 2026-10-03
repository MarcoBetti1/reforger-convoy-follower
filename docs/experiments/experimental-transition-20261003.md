# Experimental transition checkpoint — 3 October 2026

Marco chose Experimental 1.9 for all current Arma work. No further 1.8 campaign is authorized. Finish at the natural checkpoint, retain the exact stable repair/baseline/evidence, and use an isolated Experimental project with matching tools. The separate asset task owns Steam tool installation/version readiness; the convoy executor owns its source and subsequent runtime, with the parent serializing runtime windows.

**Current boundary:** no game, Workbench or recorder remains, and no runtime slot is reserved. Experimental game binary is 1.9.0.10. The only Workbench found in the registered Steam libraries at this check was 1.8.0.13. No Experimental compile, package or gameplay result is claimed. Do not point stable tools at Experimental assets.

The existing branch remains `codex/native-follow-path-experiment-20261001`; the interrupted source and all earlier uncommitted work were preserved. No new Git worktree was created. The isolated Experimental project is a source/resource staging copy, not another checkout.

## Results retained at the stable boundary

Three new V86 attempts extend the prior fourteen automated attempts to **seventeen retained attempts**. Every run kept its full log, first-result snapshot and original receipt and closed normally through its exact owned process. Earlier failures and INVALID/INCOMPLETE results remain unchanged.

| V86 case | Outcome | What it establishes |
|---|---|---|
| Input qualification | PASS input checkpoint | Actual private engine player actions, powered signed motion, steering/braking, public Hold/Resume and neutral detach; no physical-keyboard/manual claim |
| Straight, original LAV plus four ordinary trucks | PASS_CHECKPOINT | All four predecessor pairs, public Hold/Resume and final stopped Hold; peak XZ gap 41.3732 m, trace error 0.331567 m, Hold drift 0.0134277 m |
| Injected exact live head failure | FAIL `quiescent_resume_refused`; final lead release observer INCOMPLETE | Early Resume was refused. Initial Hold was correctly refused while the lead was moving; subsequent Hold was accepted. No fresh movement was credited and no repaired-driving pass was claimed |

V86 contains **two production repairs**, in `CF_DriverControllerComponent.c` and `CF_EntityFollowDriverControllerComponent.c`. Its generic receipts/inherited contract still contained stale “unchanged V80 / follower code unchanged” wording. Additive `contract-label-clarification.json` files correct those labels while preserving original records. Ordinary follower prefabs and global settings were unchanged; controller code was changed.

The injected diagnostic exposed an interrupted-edit mistake: the failed-movement quiescence observer had been placed in unload completion rather than the frame loop. **V88 final** corrects that placement, logs the first exact ownership rejection, retains native zero-speed ownership before waypoint cancellation, verifies handler membership and keeps Resume blocked until native completion, empty path and physical quiescence. The diagnostic now stops the lead through player actions before ordering public Hold. Its stopped failure endpoint uses the same strict detached-neutral measurement, retaining any FAIL result. All five script configurations and packaging pass on stable 1.8.0.13. **V88 has never run**, so automatic stopping, cancellation, quiescence and explicit restart remain unverified. The earlier V86 FAIL is not erased.

The small base-controller guard prevents a handled refusal with no live owned waypoint from emitting a false recovered acknowledgement. V86 normal regression passed; the injected failure log contains no false `FOLLOW_WAYPOINT_RECOVERED`. This guard is the only production hunk selected for the scoped Git checkpoint. The larger experimental retirement controller and fixtures remain preserved local draft work until reviewed against the matching Experimental build. The remote commit alone is not the V88 candidate.

V87 is a preserved, compiled matched baseline using exact manifest-verified V85 production controllers and V86 diagnostic fixtures. It was not run. The final V88 package is distinct from the earlier temporary V88 packaging; only `packed-final` supplied its frozen runtime.

## Exact preserved and prepared state

Stable final repair: `.cache/native-match-v88/frozen-addons/ConvoyFollower_5A5FB20BD40C7C70`.

- `addon.gproj`: `1DFE7E1AE0E2194C703300ED5D60FB41FDD3D365282A0A5FC03E86B622CB3579`
- `data.pak`: `BBC7D35FA238E77341BC615857B0B5D8D01890236F846CF370542167E7B8F79A`
- `resourceDatabase.rdb`: `2EECF37B0294A547F77D82EBF2226B9D53ACF6DBDC0B2FABC2B42DFE78D65DB2`

V80, V85, V86, V87 and V88 frozen trios were rehashed unchanged. The interrupted pre-repair snapshot is `.cache/native-activation-20261002/interrupted-v86-20261003`.

Experimental staging: `.cache/native-experimental-20261003/source/addons/ConvoyFollower/addon.gproj`. All **1,127 source/resource files** (68,642,520 bytes) match V88 before and after copying. `configuration.json`, `source-input-manifest.json`, `safe-transition.json`, `installation-inventory.json` and `api-review.json` in that root hold exact identities and the next action. No package has been produced or loaded on 1.9. The existing `Play-Convoy.cmd` remains a V80 stable rollback entry, not an Experimental or repaired-candidate launcher.

Official [Experimental source revision 6f7d102](https://github.com/BohemiaInteractive/Arma-Reforger-Script-Diff-Experimental/tree/6f7d10273974c9ea716782e0b9cd549124ad751f) is labelled **1.9.0.10**. The used native movement, handler, utility, vehicle action and inherited speed methods remain present. This is a source-contract check, not compile or runtime compatibility. No native path-array setter or cancellation API was invented. [Bohemia’s 1.9.0.5 notes](https://reforger.armaplatform.com/news/experimental-september-15-2026) describe driver dormancy and vehicle simulation changes; they do not establish that this bridge failure is fixed.

## Route evidence and next runtime window

`node tools/convoy-route-sidecar.mjs <closed-run-directory>` adds an offline receipt companion and CSV without altering the log/receipt or launching anything. Six existing runs have sidecars: V80 manual, V83 bridge, V85 static bridge, and all three V86 runs. They verify the actual loaded world line, inherited terrain resource, frozen planned XYZ trace where available, actual periodic vehicle poses, first failure time/positions and receipt/package identities. Public-map context is never treated as driveability evidence.

All five vehicles have periodic XYZ surface samples in the manual log, but those old samples lack world time. They retain their actual wall clock and null `worldMs`; no frame interpolation or fabricated full-resolution tracks were added. The player-proxy logs supply world-timed lead and all four follower tracks. The sidecars preserve reported outcomes and disclose missing clocks/coverage. No uninterrupted moving footage was captured or reviewed in this transition. No master take, collision clearance, full Saint Pierre–Levie result, final 180-second ordered lineup, repeatability batch, natural-ground qualification or later manual acceptance is claimed.

After the setup owner supplies the exact matching tools path/version and the parent grants the serialized runtime slot, the first convoy window is: isolated Experimental validate/package, fresh same-build powered-input qualification, then the smallest exact-live injected retirement diagnostic. Budget roughly ten minutes for those two bounded game runs; observe exact stop refusal, old activity/native request/path, controls and new request generation. Repeated commands must be evaluated against the existing public “already in progress” semantics, not assumed to create another order. Separate an assisted failure diagnostic from ordinary natural navigation.

Then run matched ordinary straight/bend/bridge on the same production follower implementation/global settings and retain every outcome. If the bridge improves, continue to map-informed bends/junctions, grades, narrow crossings, verified rough ground, imperfect lead inputs and held-out routes; a bridge checkpoint does not finish the goal. The bridge-native UNREACHABLE/navigation cause remains unproved. Preserve supported `RequestFollowPathOfEntity` investigation, but never give the player lead an AI route to substitute for player evidence.

The full player-led/five-total filming objective remains **INCOMPLETE**. Multiplayer, unloading/return and later cinematic work are separate. The immediate dependency is matching Experimental tools and an exclusive runtime window; Steam changes belong to the asset setup owner.
