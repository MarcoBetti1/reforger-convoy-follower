# Loaded straight trip v1 — combined two-truck supply comparison

September 26, 2026. **Private candidate; physical result pending. Not a release candidate.** This [three-file increment](loaded-straight-trip-v1.patch) applies after [straight-arrival v5](straight-arrival-v5.md). It preserves earlier source/evidence and leaves the canonical addon unchanged.

## Product intent and changes

Complete the existing native workflow: load 100 into each ordinary assigned truck, follow the real predecessor chain to a distinct destination, park, Hold, unload 200 and Resume with both original drivers.

- Keep actual-pose queries running while native avoidance temporarily preempts the healthy owned pending Resume Wait. Session/owner/epoch/world/seat and original-chain checks remain. Actual departure still requires the selected Wait, genuine TRACKING and the existing predecessor-motion/forward-goal checks. No geometry or movement-credit change.
- During upward head-cap recovery, promptly restore the existing pressure-shaped cap only when the front gap is larger than the rear and twice the moving gap, the front is opening and the rear is closing. Other recovery remains limited to 3 km/h/s; tightening stays immediate. The existing cruise owner remains the sole writer.
- Use the existing loaded-pair world with desired native lead goal `1610.5 19.9302 3365.83`, 45 m restart walk and explicit 3.5 m minimum mapped width. Move only the destination storage to `1592 21.25 3368`, about 256.755 m from the source. The terrain height was observed; clearance and native 12 m transfer reach remain hypotheses for the actual run.

This is a combined product-loop comparison, so it cannot isolate the contribution of each change. The cargo coordinator and native action eligibility, cancellation, original-container conservation and transfer-range checks are unchanged. Required ledger: `1800/0/0/0 → 1600/100/100/0 → 1600/0/0/200` for source / cargo1 / cargo2 / destination. No resource-count writes, replacement cargo or vehicle teleport supplies the result.

## Validation and limits

All five script configurations and packaging passed. Package SHA-256: `5BE37BAE52010EEA457666A70CC07A5B965D5B0E234AAC8515A2C610389F53FF`; 588-file frozen source manifest: `33F28DE4B3CF602F818066499E62E254BF9FC11F422B73DCD6E8B757C602F75F`. The preserved acceptance gates require powered travel, both peak links ≤60 m, arrival for 180 s with ≤2 m drift, real Hold ≥30 s and fresh Resume ≥20 m / three powered intervals per original follower. Route geometry and the existing 100 native cases are unchanged.

Recruitment, owner-seat staging and native lead/server commands are test assistance. Ordinary keyboard/mouse control and multiplayer remain unproved. V5's arrival and Hold success, spacing failure and incomplete Resume remain visible in its separate archive; this candidate has no physical verdict yet.

## Apply and complete source mapping

Apply archived straight-arrival V1–V5 in order to their documented baseline, then:

```powershell
git apply --check docs/experiments/loaded-straight-trip-v1.patch
git apply docs/experiments/loaded-straight-trip-v1.patch
```

Applicability was checked read-only against exact composed V5. Rebuild and generate fresh database/package files before running. This archive contains source only; no binaries, generated resource database or user data. Hashes pin the approved source bytes; patch text uses LF line endings.

| File (relative to `addons/ConvoyFollower/`) | V5 SHA-256 | Candidate SHA-256 |
| --- | --- | --- |
| `Scripts/Game/Tests/CF_EntityFollowDriverControllerComponent.c` | `B98FFA44967D0EA9D8FBB0190D86B20A6CE792C2BD6D35021CDA0F254690537A` | `CC35EAD550B158EC8D9BFB18DDB2D55495A55AB006D91B2F1B679D08EBA1D1F6` |
| `Scripts/Game/Tests/CF_TrailGuideDriverControllerComponent.c` | `1E5442CBD0DA309B710730D94E23DAC62122998B492FBA6E326E7DCF79B7807C` | `6FCC3A59E8191357BCAF0D72F9F02AE19DE27CE23C33BB626E566F56FB854513` |
| `Worlds/Tests/ConvoyFollower_Arland_LoadedSupplyTrip_2Trucks_Layers/default.layer` | `4D9B27AC6930D2B1111334DB8A7DC6A989B4E00AA13C3C7E28BF6B7A17AD8D97` | `2A925D5662C11C33F311681C04686041535A12DD9B099B205DCE0D542E38295E` |

Patch SHA-256: `0DA26B37EFE11A92C4C864A52F7AE82205BB272F6192710337990EA003A47EEA`.
