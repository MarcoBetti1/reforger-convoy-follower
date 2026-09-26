# Straight-arrival v1 — unpromoted source checkpoint

September 26, 2026. **Development experiment, not a release candidate.** The accompanying [patch](straight-arrival-v1.patch) preserves reusable source outside the local cache. It does not change the canonical addon until explicitly applied.

## Baseline and contents

Baseline commit: `53972a4358976dfbb51d73bf4272cda68a8f0818` (`53972a4`); the addon was unchanged at this checkpoint. The patch contains exactly 12 approved text overlays: nine existing scripts and three new world resources. It excludes resource databases, packages, binaries, recordings and local user data. Candidate hashes below are the original approved file-byte hashes; the patch stores normalized LF text, so checkout line endings can affect raw hashes.

The candidate separates local route station from monotonic high-water evidence, admits physically supported local reverse adjustments, retains route observation through arrival/Hold, and waits for genuine predecessor departure before a tail Resume. Native geometry guards and independent physical evidence remain separate.

The command-only scene uses ordinary US drivers and a native AI lead at 20 km/h. It requests the connected goal `1598.08 20.7374 3363.82`, before the problematic junction, then surveys a 45 m restart. Initial assignments and the real predecessor chain remain intact. Gates remain 150 m lead / 100 m and three powered intervals per follower, 60 m peak links, 180 s arrival within 2 m, real Hold at least 30 s, then fresh 20 m and three powered intervals per follower.

## Evidence at archival checkpoint

- Reverse-recovery v5 native helper: **98/98 PASS** at 18:26:13.851. Geometry assertions are not physical driving acceptance; runtime/lifecycle error gates remain separate.
- V5 junction driving: **FAIL**, peak link 81.7859 m and `local_turn_over_90_degrees`; no formal arrival observation, Hold or Resume. Preserve that failure.
- New straight-arrival physical comparison: **FAIL** at 18:37:39.845. Peak link 56.635 m stayed within the spacing limit, but the tail rejected `reverse_previous_segment_unavailable_or_step_spans_vertices` before formal arrival, Hold or Resume. The clearer stopping point did not resolve route tracking. Pack `497B6E8806966A7B573F80C2148A0F1D6CAA5A58CA413DB6236EEA28563F2039`; full evidence is in `.cache/client/runs/straight-arrival-stop-resume-v1/`. The client closed naturally; gameplay and shutdown errors remain in the full log.

Script preview and compilation alone do not establish gameplay. Recruitment, native lead driving, server commands and owner seat transfers are test assistance. This course explicitly sets `cargo_tested=false`; it proves neither a loaded delivery nor ordinary keyboard/mouse use, Soviet behavior, three-follower reliability or multiplayer. The successful full supply workflow still requires its own final integration run.

## Review or reuse

From the baseline repository root, `git apply --check docs/experiments/straight-arrival-v1.patch` verifies applicability without changing the addon. Apply in a separate checkout with `git apply docs/experiments/straight-arrival-v1.patch`, then validate/build through Workbench and generate a fresh resource database/package. The new world is `Worlds/Tests/ConvoyFollower_Arland_OpenRoad_StopHoldResume_2Trucks.ent`. Applying this patch is a source experiment, not promotion of its behavior.

## Approved overlay hashes

Paths are relative to `addons/ConvoyFollower/`.

| File | SHA-256 |
| --- | --- |
| `Scripts/Game/CF_ConvoyFollowDriverControllerComponent.c` | `DB1255D5B23C611A8A0BD13292F558DC34134046449206C6BCB4DE7D5A5A0284` |
| `Scripts/Game/CF_DrivenRoute.c` | `6D8A4203526F95FE06A4ED1DBAE1E1C4B1B64C6A9806C1E60B1379538C59355E` |
| `Scripts/Game/Tests/CF_DrivenRouteGeometryProbeComponent.c` | `060333761CF74A40E95EFEF4BE48617808E8DA3AFA8B03988C628C2A9EDEC03A` |
| `Scripts/Game/Tests/CF_LoadedPairTripProbeComponent.c` | `CB78B0F2FF70CB474879A6938849533326B0BA3C6ED3BCC098F6B7DBDC974393` |
| `Scripts/Game/Tests/CF_OrdinaryMixedRoadProbeComponent.c` | `16BF6A063C1541C7DDE9072E704BB76EA2B5CD8021DB76A9D975D223CB6369F1` |
| `Scripts/Game/Tests/CF_OriginalTailGuideProbeComponent.c` | `32AE1DFF3F50E1E106C201AE06AAB7336BBA16962A6144A1336712FB38EC8B49` |
| `Scripts/Game/Tests/CF_TrailGuideDriverControllerComponent.c` | `257C85AB8102BA603865F87319F8AF238FE76B839BAA9EE5AC68A65710BC31D5` |
| `Scripts/Game/Tests/CF_TrailGuideEntity.c` | `A504BDE44265FE3B6A8ADDEE1E13211CD3542BB4AE631ECEBDC83ABA170EC34B` |
| `Worlds/Tests/ConvoyFollower_Arland_OpenRoad_StopHoldResume_2Trucks_Layers/default.layer` | `3C7A408A9E793CADE5196229FF855236A963BC54BBBA7DAF0C3FD46A7776811B` |
| `Worlds/Tests/ConvoyFollower_Arland_OpenRoad_StopHoldResume_2Trucks.ent` | `0F89D1AF2D744A27DEC4921589717368851C8D32594CCB972F783B71D87EA4E8` |
| `Worlds/Tests/ConvoyFollower_Arland_OpenRoad_StopHoldResume_2Trucks.ent.meta` | `905B8FAF65CD82F3D93B4DDEBB593A74DD3512F763EDFD3CA20E30731FCBD47D` |
| `Scripts/Game/Tests/CF_SmokeProbeComponent.c` | `BC0F8F5B9DA545E4C15E72D9EA97CD344B3B0FAC2D7BE64588B9C27E736E445E` |
