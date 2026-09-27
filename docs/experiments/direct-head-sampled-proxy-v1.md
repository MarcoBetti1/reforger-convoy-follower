# Held15 / native20 proxy: closed independent review

September 27, 2026. **Physical/raw/strict FAIL; private, unpromoted.** Holding the proxy until 15 m of real predecessor displacement materially improved speed, but the peak gap remained **79.1669 m**, above 60 m. Native desired 20 m, actual physical gap 20 m, stopped real-target 10 m, lead 20 km/h, course, actors, camera and acceptance gates were unchanged from continuous-proxy v1. Refresh cadence and target lag changed together.

| Matched evidence | Continuous proxy20 | Held15 / native20 |
| --- | ---: | ---: |
| First 10 selected seconds, 10 samples | 11.60 km/h | 13.57 km/h |
| Next 20 selected seconds, 20 samples | 11.29 km/h | 15.90 km/h |
| Selected +30–45 s, 14 samples | 14.48 km/h | 19.95 km/h |
| Peak actual link gap | 125.481 m | **79.1669 m FAIL** |

These are single-run comparisons, not repeatability estimates. First spacing failure was **00:01:37.398**, world 46.2679 s. Terminal **00:05:35.613** retained that failure.

## Physical proof and remaining slowdown

- Lead **259.455 m / 31 powered samples**. Original follower **251.197 m / 35 powered intervals** within the same exact moving sequence 1, waypoint `0x60000000000000F8`, proxy `0x60000000000000F9`, actual predecessor `0x4000000000000006`.
- All 51 proxy observations retained exact ownership. Captured-origin/basis readback error was **0**; maximum actual lead lag **14.7821 m**, no sampled lag above 15 m. Sixteen refreshes followed **15.003–15.089 m** real displacement, at **2.316–2.966 s** intervals. The last target remained fixed after the lead stopped; maximum age **8.484 s**. Refresh-threshold overshoot is reported rather than treated as physical credit.
- Moving brake samples often occurred shortly after refresh with the current proxy still far away: at 00:01:39, target 58.32 m/speed 17.09/brake 0.767; at 01:44, target 65.00 m/speed 21.15/brake 1; at 01:51, target 76.80 m/speed 20.74/brake 1. Nearby preceding cruise allowances were 44.626 km/h; proxy ages were 517/416/350 ms. Same original activity persisted. This supports a native refresh-related slowdown; it does **not** prove braking simply starts at the current target's 20 m radius. Native internal/stale-path semantics remain unknown.
- The proxy was retired at **00:02:05.664**; sequence 2 used the **real predecessor at 10 m**. Arrival completed **180.413 s / 180 samples, zero drift**, original seats/chain retained and one exact selected follower Wait. No route/history/join or proxy motion supplied travel credit.

## Attribution, errors and closure

All **598 frozen sources / 3 deployment files** and exact two-overlay contract verified; analyzer issues were empty. Pack **64C0B211434679C70CAB9F29EA5019B580BD835D292AEA9CD64D8D0D60E036FE**; source manifest **BEB32AF92563BF5DA84304F9FAA1ACF6B18FC283C74A153A2963FEF6B9A119AD**. Source candidate 88CFBAE5…03BF22 repaired only an overlong diagnostic concatenation after the preserved first compile failure; all five configurations and pack2 subsequently passed. Full-ID observations bind the actors; decimal creator IDs are accepted only with unique matching full IDs observed in this log.

Errors remain **9 gameplay / 0 post-result / 64 shutdown**, all retained. Terminal snapshot is an exact 1907928-byte prefix, SHA **26E2BB311B0912D5F33BBDF7BD1B7154F7FA3693564DCA6047CF5F5C7BCA3451**, captured +256 ms. Full log 1955120 bytes, SHA **ABBAB76477C42A9D400D51F9835AB042FDAAD34DB003C7BCF931F224B703D1CA**. Native exit requested after 30.1162 s; cleanup and Game destroyed logged. Root consumed client/watcher/recorder exit 0 and checked no matching processes/windows.

Silent video **353.466667 s / 5301 frames / 456789406 bytes**, 2560×1440 at 15 fps; SHA **96192BEFB6E8D9DB5ABA455AC1470F059A9DBB86CC3F0930D3F56E7AEDB6C05F**, independently hashed/probed. Root inspected three live travel/arrival/terminal views; no full playback.

No cargo, explicit Hold/Resume, ordinary-input or multiplayer claim. One separately staged held15/native5 interaction comparison is justified by the speed improvement and remaining slowdown; it is not a demonstrated fix or another parameter sweep.

## Exact reproduction

[Two-overlay patch](direct-head-sampled-proxy-v1.patch), SHA D63CDBB49B32DBA1067080515DCEF730D673DEB9ED6129718DE12A96BEB1371B, reconstructs both exact candidate hashes against the historical proxy-v1 frozen baseline manifest 2FE7E8CCE9D9C44BEFBBD1CF41535EF2C523A7329BE39B3482C8E2DE4DC4DF1C / pack 83D314E0174776B02775F83C60D814F372FC7B9E0614180390B6C125975C5756. Candidate manifest 88CFBAE546962BC3FD7D44532D206A306EF910DF06966D3DC9B81AA24D03BF22. Use git -c core.autocrlf=false apply --directory=<exact-addon-copy> direct-head-sampled-proxy-v1.patch; paths begin Scripts/. Existing controller and observer only; no new assets or geometry. Apply to that private baseline, not blindly to canonical source. World remains Worlds/Tests/ConvoyFollower_Arland_Road81_DirectHead_1Truck.ent. [Independent report](../../.cache/client/runs/direct-head-sampled-proxy-v1/independent-review.md).
