# Loaded straight trip V5 — native geometry passed, travel failed

September 26, 2026. **Closed physical FAIL; strict full-run FAIL.** All five script configurations and packaging passed. This comparison changed only the recorded-route helper and its native geometry probe against frozen [V4](loaded-straight-trip-v4.md); controllers, course, cargo workflow and physical gates remained unchanged. The new connected-chain projection passed **112/112 native geometry assertions**, but the full trip failed during initial travel. No candidate is promoted by this result.

## Observed trip

- Both original trucks natively loaded **100 supplies** and completed cancellation stability windows of 3.01617 and 3.01597 seconds. All **121 four-container ledger rows** (91 through terminal) conserved 1800. Final counts were source 1600, cargos 100+100, destination 0. **Nothing was delivered.**
- All **105 drive-to-terminal identity samples** retained original pilots, trucks and predecessor chain. Best single-activity movement was head 109.943 m / 22 powered intervals and tail 54.4177 m / 6; the complete baseline did not pass.
- Peak links were **60.6782 /64.5144 m** against the unchanged 60 m gate. The first failure was tail spacing at **21:13:46.484**, world 79.7536 s.
- Tail `reverse_corridor_lost` followed at **21:13:47.100**. Terminal **21:13:47.783 / world 81.053 s** retained both failures. Formal arrival observation, real Hold, unloading and Resume were **not reached**.

## Native movement boundary

The tail braked near x1405.78, then physically reversed while the same sequence 2, owned guide/waypoint and native formation behavior remained selected. Consecutive one-second samples showed **12.506 m signed reverse before the block**. The last pre-block sample was near 1394.76 / 3324.97 at 9.98234 km/h, with cross-track 4.42621 m. Returned native car-path points folded backward towardx1395 before turning forward again; their internal steering semantics are not exposed.

The unchanged reverse guard rejects a current or prior pose residual beyond 5 m. Its exact failed residual/pose pair was not logged. The owned lease was retired as `controller_clear revoked=true failed=false`, not a native UNREACHABLE result. One projection-correction marker appeared, but that shared marker cannot distinguish the new chain path from the existing one-corner correction. It provides no extra physical credit.

V4 braked near the same hill and continued forward, subsequently delivering 200, completing 180.56 s arrival and 51.247 s Hold, and qualifying lead/head restart before a separate tail projection failure. Those useful V4 boundaries remain. V5 is not proof that the new helper caused the native reversal, nor evidence that the intended onward repair succeeded. The next useful investigation is the native movement/path boundary at this recurrent location and an alternate representative course, with the failed course retained.

## Geometry and evidence

The first three inert geometry runs each remained **111/112 FAIL**. Readback showed a nominal .5 m native distance compared below the .5 m sample minimum, yielding four recorded points instead of seven; an explicit-float hypothesis did not change that. The fourth fixture used .51 m samples and passed **112/112** with seven points and the expected segment 3 / local 2.2. Production helper bytes stayed unchanged through those fixture iterations. Each inert run retained nine runtime errors and a timed closure without normal shutdown proof.

- Data SHA-256: `3DBA6E1B5E132E105868AE658FA0EACB7DA1C89B09EEFC8DCCF51C078D8E1919`.
- Manifest: `29689DF0533B6607622279BC648DCF93056E6690D42120F50341B9E108DB12D8`; **588 source files and three deployment files** independently verified. Exactly two source changes versus V4: helper 82B25F47…F9787620 and probe 95C1D1E4…85D293D.
- [Independent trip review](../../.cache/client/runs/loaded-straight-trip-v5/independent-review.md), SHA `D2F58494F518E219B24C1BB6E62A3520B839950753FCD15898D18CFC05ACDEF4`; structured review, full console and closure evidence are adjacent.
- Exact-prefix terminal snapshot: 820909 bytes, +222 ms, SHA `D300049BB4B1E45D398C4C6BF8929530FF0348E686F8B9EB91A5604F357EC46D`.
- **9 gameplay /0 post-result /62 shutdown errors** remain. Root consumed natural client, watcher and recorder completion, all exit 0; fresh process check was empty. Native close followed 30.0151 s grace, with world cleanup and Game destroyed logged.
- Silent recording: **134.733333 s / 2020 frames / 120430764 bytes**, SHA `012124467E392503CC2B2AF950179557700F66D8528B86811757032CAA1CC93F`. Initial display enumeration failed and the replacement recording began after client launch, so early setup may be absent. Two live views were inspected; no full playback.

No ordinary keyboard/menu, human-driving, multiplayer or release-readiness claim follows from this automated fixture. All cargo, identity, genuine join, 60 m spacing, 180 s / 2 m arrival, 30 s Hold and fresh 20 m / three-powered-interval gates remain unchanged.
