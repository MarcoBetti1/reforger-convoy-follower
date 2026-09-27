# Loaded straight trip V4 — longer onward restart

September 26, 2026. **Closed physical FAIL; strict full-run FAIL.** All five script configurations and packaging passed in `.cache/workbench-runs/convoy-loaded-straight-trip-v4`. This comparison preserved native delivery and passenger Resume, then exercised a longer restart. Head spacing failed before commands; tail route guidance later blocked during restart. No candidate is promoted by this result.

## Declared change and exact source scope

The [two-file fixture patch](loaded-straight-trip-v4.patch) adds a default-off explicit restart target to the existing pair coordinator and enables it only in the loaded-pair layer. The goal is **1697.17 / 25.722 / 3301.33**, previously used by a successful one-follower uphill restart from a different start. The legacy road-walk branch remains unchanged.

The explicit branch checks finite authored coordinates, requested and natively resolved XZ distance of 90–140 m, starting-road width/distance, native reachability within radius 5, snap within 3 m and the existing forward-dot condition. V4 resolved the goal to **1697.17 / 25.7218 / 3301.33**, 112.572 m from the stopped lead. The 5.24466 m endpoint rise does not prove the intermediate grade, clearance or two-truck junction behavior. Native lead MOVE, 20 km/h request, radius 5, owner-seat barriers and follower policies are unchanged.

This is **two fixture overlays, not only two changes in the full build**. Comparison of all 588 frozen source files against V3 found exactly four changed files:

1. `Scripts/Game/Tests/CF_LoadedPairTripProbeComponent.c` — explicit-goal fixture branch.
2. `Worlds/Tests/ConvoyFollower_Arland_LoadedSupplyTrip_2Trucks_Layers/default.layer` — enable the declared goal.
3. `README.md` — canonical integration documentation.
4. `Worlds/Showcase/ConvoyFollower_Arland_SupplyDay_Layers/default.layer` — separately integrated source-storage position; this test did not load that world.

Session and all follower movement sources are byte-identical to V3. The canonical input resource-database pin changed during composition; the final packaged database matched V3. Full source hashes are retained in the private verification record.

## Completed evidence

| Gate | V4 observation |
| --- | --- |
| Original cargo | 435 exact four-container ledger samples conserved 1800; native load 100 per truck and distinct-destination unload 200; each transfer followed by ≥3 s stable cancellation window |
| Baseline exact moving activity | Head 250.297 m / 28 powered intervals; tail 211.22 m / 25; genuine predecessor-route epoch 1 |
| Original identity | 795 baseline seat/identity samples passed; original chain retained up to the route failure |
| Peak links, limit 60 m | **63.0681 / 55.0173 m: FAIL** |
| Arrival | **180.56 s**, 180 exact arrival Wait samples per follower, zero measured drift across all three vehicles |
| Real Hold and delivery | **51.247 s / 51 samples**, zero measured drift/speed; Hold retained through unloading and complete native-pilot/owner-passenger handoff |
| Negative Resume calls | Owner on foot and unowned AI both rejected; both original assignments remained held after each |
| Passenger Resume | Accepted after exact seated handoff; actual onward movement followed |
| Fresh lead movement | 58.5026 m projected progress / 5 powered intervals: individual gate passed |
| Fresh head movement | Sequence 5: 38.2294 m continuous physical progress / 3 powered; projected net 30.1304 m: individual gate passed |
| Fresh tail movement | Sequence 6: 7.91244 m / 1 powered; projected net 8.91579 m: **FAIL** |

Original cargo identities and counts remained valid through the terminal and post-result interval. Public server command acceptance is reported separately from the stricter physical restart gate. No old moving activity, later stopped approach or post-terminal movement is pooled into restart evidence.

## Actual failure sequence

First failure was **head spacing at 20:38:16.222**, world 95.3468 s, before any explicit onward command. That failure remains sticky.

At **20:43:07.202**, Unit Two's authored route query rejected **ADVANCE_LIMIT / route_state_8**: prior station 258.754, candidate 259.288, budget end 258.815, excess 0.472473 m, measured step 0.0599613 m, allowed budget 0.0616265 m, segment 210→211 and cross-track 2.57462 m. The controller retired its owned guide lease and set the fallback-failure flag. Its retirement recorded `controller_clear`, revoked=true, failed=false; this was not a logged native UNREACHABLE result.

The fixture's `command_original_identity_or_session_lost` at 20:43:07.205 is secondary because that check also rejects the fallback flag. The last original identity sample at 20:43:07.019 was valid; no physical driver/truck reassignment or lost player session was established. Exact failing geometry is retained without claiming a rounding-only cause or a proven repair.

The longer leg addressed V3's insufficient lead/head power opportunity, but did not establish a complete two-follower onward trip. Keep both the spacing failure and the later production route failure.

## Reproduction and evidence integrity

- Private run: [independent review](../../.cache/client/runs/loaded-straight-trip-v4/independent-review.md); structured review and closure record are beside it. Review SHA-256: `250C3CD39AC3C89DB4A483BC146777B3CA392A54B519B8276F88EFFB75E1A8F9`.
- Data SHA-256: `A40590B23BB1C59D4F517F28F71796B77FBBF6118E1F8EDA6B9D6F435B8B2619`.
- Source manifest: `40DF1ADC01979F24F86D5280D6EF6CEE1612563FE8A9D12A28673601760017EE`; all **588 sources and three deployed files** verified.
- Full console and terminal snapshot retained. Snapshot is an exact prefix, captured **+237 ms**, SHA-256 `C48E456701AC7952E6BA221E36450EB901E9F2AD017320EA3EE2598566919795`.
- **9 gameplay / 0 post-result / 64 shutdown errors** remain. Normal closure does not mean an error-free run.
- Root consumed natural client, watcher and recorder completion, all exit 0, and confirmed no game/Workbench/ffmpeg processes. Native close followed the 30.0145 s grace; cleanup and Game destroyed were logged.
- Full private silent 2560×1440 recording: **461.2 s / 6917 frames / 677870370 bytes**, SHA-256 `89EFEA78E601D602C0FDACA9A5A6A1AD3E78A0ABF09554FAC8A5A7B115B5FAB7`. Root inspected two live views: stopped destination and renewed travel/failure. No full playback or ordinary-input claim.

### Fixture patch provenance

The archived patch is byte-identical to the reviewed private patch, SHA-256 `1E58965E182ECAB35DC52D80A8D24F1C9DEC386FF06CD92B44D1159A2C74003D`. It is the fixture-only increment against the exact V3 files below, not a complete canonical-to-runtime patch or a packaged addon. No new apply-check was run for this archival copy. Use an isolated, matching source tree and rebuild the package/database together; do not apply it blindly to current canonical source.

| File relative to `addons/ConvoyFollower/` | V3 SHA-256 | V4 SHA-256 |
| --- | --- | --- |
| `Scripts/Game/Tests/CF_LoadedPairTripProbeComponent.c` | `E64A9A817304E3CD72A313766AF9772BD9C7E3BDD31A3AA8222655BBE80D14F1` | `C12D8A79E05716DA3DADA6123DC3F74A0B0F5F1CD4014D40685594FFEEE57C32` |
| `Worlds/Tests/ConvoyFollower_Arland_LoadedSupplyTrip_2Trucks_Layers/default.layer` | `2A925D5662C11C33F311681C04686041535A12DD9B099B205DCE0D542E38295E` | `DE28B91F1DB657BC72793E78E6CFA992DFC75F278D9466EFBD633779BCD07F1A` |

All cargo, original identity, genuine join, ≤60 m links, ≥180 s/≤2 m arrival, real ≥30 s Hold and fresh continuous ≥20 m/three-powered-interval requirements remain. Automated recruitment, native lead driving, seat staging and native-action dispatch do not establish normal keyboard/menu workflow, human driving, repeatability or multiplayer release acceptance.
