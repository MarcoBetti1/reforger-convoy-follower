# Straight-arrival v3 — retain observation during arrival selection

September 26, 2026. **Private candidate; physical FAIL.** This [single-file increment](straight-arrival-v3.patch) applies after the preserved [v2 experiment](straight-arrival-v2.md). The canonical addon remains unchanged.

The retained route observer now accepts a sealed, healthy, correctly bound arrival lease while native activity selection is pending. Original driver, truck, group, session, waypoint and predecessor checks remain. A terminal activity is rejected. The change affects actual-pose observation only: native movement authorization, guide updates, pacing receipts and arrival/movement credit still require their existing evidence. A diagnostic logs at most eight pending generations.

## Source and validation

- Only `Scripts/Game/Tests/CF_TrailGuideDriverControllerComponent.c` differs from V2: SHA-256 `2CD5B4B8E9BF3EADC3EBA8F7B81AE3F89F2B1AB59611422E7B5B3A0E27858BA5`, replacing `257C85AB8102BA603865F87319F8AF238FE76B839BAA9EE5AC68A65710BC31D5`.
- Patch SHA-256: `9138C369ED63D8F91BA7D4008035B3A33CDECF6B13834F154D5EC41A79AFF041`. Applicability checked against the exact V2 source. Patch line endings may differ from original source bytes.
- All five configurations and packaging passed in `.cache/workbench-runs/convoy-straight-arrival-v3`.
- Package: `BABAE02E311A5342515FA33C35EA84D3495798DEDD6DEB3266801F9A0E4F8D0C`. The physical run freezes 588 sources; manifest `F71C189D961DE609E0B2836DA130B958CB68A837BD4B6AFC0D3CC6149363CC23`.

## Physical result

`straight-arrival-stop-resume-v3` failed at 19:04:48.144 before the repaired handoff was exercised. Peak link **53.9589 m** stayed within 60 m, but the tail rejected `recorded_goal_not_forward` while approaching the end of its moving route. Its predecessor was already executing the real-target parking approach, but had not yet selected its arrival Wait. No formal arrival observation, Hold or Resume completed. This run neither proves nor disproves the retained-observation repair.

The full log, terminal-prefix snapshot (+207 ms), recording and [independent review](../../.cache/client/runs/straight-arrival-stop-resume-v3/independent-review.md) retain the result and runtime/shutdown errors. The client exited naturally; watcher and recorder completed and a fresh process check was empty. One live departure view was inspected, not full playback. Cargo was explicitly skipped; ordinary input and multiplayer remain unproved.

The next practical change is to coordinate arrival through the predecessor's verified parking phase before advancing an unusable moving guide. Keep the failed control and all physical thresholds. Rebuild from source after applying these experimental patches; no generated package/database is included in this archive.
