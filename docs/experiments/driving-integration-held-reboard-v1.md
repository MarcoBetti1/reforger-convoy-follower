# Driving integration: focused held-Reboard regression

September 26, 2026. **Focused physical PASS; strict full-run FAIL: 9 gameplay /0 post-result /64 shutdown errors.** This checks that the demonstrated private driving cohort preserves the canonical selected-Reboard lifecycle. It is not a new driving or supply-trip pass.

## Candidate and result

All five script configurations and packaging passed for `.cache/packed/convoy-driving-integration-v1`, data SHA-256 **`FA835A46E5A3DC4CDA340839988DB12FE1A7A75CD8E24E520DA1275E85443DA9`**. The private copy starts at canonical `038b87b`, adds five driving product files and reuses exactly four focused canonical Reboard fixture resources. The panel, Session, base Driver and RPC remain byte-identical; Entity merges the exact original-pilot-departure Wait repair into the tested Road81 controller.

The unchanged `Worlds/Tests/ConvoyFollower_Arland_HeldReboard_2Trucks.ent` suppresses departure and exercises real server commands with assisted native exits:

- Both original followers completed **30.0492 seconds /30 samples of Hold**, with zero measured drift/speed; peak link **24.2422 m**.
- Three native exits produced two actual automatic returns, then a selected retry accepted at **22:42:58.248** against the original truck and a fresh waypoint. Execution was observed; the duplicate was rejected without changing attempt, clock or waypoint.
- Original seated Hold returned at22:43:05.848; session completion followed at22:43:06.264. Five later observations retained the original seat and Hold for **5.0160 seconds** through terminal **22:43:11.281**. All138 command identity rows retained the original chain. Transitional seat ownership was logged without granting completed-entry credit.

This clears the focused merged Wait/Reboard regression concern. The five product files remain the promotion scope; the four fixtures, separate qualification correction and head-guide experiment are excluded. Promotion/build status belongs in [current validation](../convoy-validation-current.md). Existing112-case geometry evidence is reused for byte-identical helpers. Head spacing, a complete combined supply-trip regression and driving repeatability remain separate limits.

## Evidence and lifecycle

Independent verification covers **588 frozen source files /3 deployment files**, all five product overlays, four unchanged fixture files, four preserved Reboard files and16 fixture dependencies with four declared driving replacements. Source manifest: `FBECF6245279755D41162FA67A3D88E989D0ADD75AFC0D7E980F14AEA3186FB9`; merged Entity: `3A16D6C5675120CA9983E53BBAB5ADD6E2078A1BB850266DB6AEF00DD9A95088`.

- [Independent review](../../.cache/client/runs/driving-integration-held-reboard-v1/independent-review.md), SHA `C6F9FD79923A5564E3EF0E90AEB2CFD87CE4A1658522B1ED0200D3C5C3FA08E5`; its JSON preserves every error line and the gate evidence.
- Full log:478120bytes, SHA `41D3CA21C17FB4BDC806776394A5FFEBEAFF5CEEEFCCF93B396B05B08E5B3732`. Terminal snapshot:403915bytes, exact prefix captured **+65ms**, SHA `D888B11B855F591450056CD0DB14C62168570701F18DDFBB1D0218DD8DCEB1EA`.
- Native close requested22:43:41.298 after30.0157seconds; cleanup and Game destroyed followed. Root consumed client, watcher and recorder at exit0 and verified no remaining game/Workbench/ffmpeg processes.
- Silent recording: **202.8 seconds /3041frames /105140849bytes**,2560×1440 at15fps, SHA `15224F624340A314CA5E418B893033F4BC00BBC84767BAD253837362201D336E`. Hash and media metadata were independently checked. Root inspected two clear parked-formation live views, the second after terminal; no full playback is claimed.

Assisted exits and server commands do not establish ordinary panel input or multiplayer. This stationary run tests no cargo, travel,180-second arrival or Resume. Physical PASS does not waive the retained runtime/shutdown errors or rewrite earlier failed controls.
