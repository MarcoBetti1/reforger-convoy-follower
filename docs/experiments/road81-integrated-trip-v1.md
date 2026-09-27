# Integrated Road81: complete native supply loop, spacing still fails

September 26, 2026. **The scripted supply/Hold/Resume stages completed. Overall physical, raw fixture and strict results remain FAIL.** The integrated driving cohort preserved the original two-truck chain, delivered 200 supplies and earned both fresh powered Resume qualifications. The head's earlier spacing breach remains visible.

| Stage | Observed result |
| --- | --- |
| Native load, transport and delivery | 100 per truck; distinct destination 276.438 m away; final source/trucks/destination 1600/0/0/200; all 497 ledger rows conserve 1800 |
| Continuous follower travel | Head 124.2957 m /31 powered intervals; genuinely joined tail 240.0035 m /50 |
| Arrival and real Hold | 180.279 s /180 arrival samples, then 51.1149 s /51 Hold samples; zero measured drift |
| Fresh powered Resume | Head sequence 5 best 35.1900 m /7; tail sequence 6 21.2586 m /8; native lead 54.5371 m /10 |
| Whole-run spacing, 60 m limit | **Head 172.483 m FAIL /rear 44.9963 m PASS** |
| Gameplay /post-result /shutdown errors | **9 /0 /62**, retained |

Both negative Resume calls were rejected with Holds intact. Owner-passenger Resume followed the fully settled assisted seat transfer. The corrected fixture retained the head's earned qualification while it entered its healthy stopped-target approach, then completed when the tail independently qualified. Each event was reproduced from its own fresh continuous activity; no baseline, replacement or projected-route distance was pooled. Terminal 23:08:01.920 correctly records **`pair_complete=true`, `pass=false`**. This resolves the preceding [same-poll fixture timeout](road81-arrival-trip-v1.md), without fixing the head lag or proving repeatability.

## Reproduction and limits

- Baseline: canonical **58c36a2**, including the promoted five driving files and preserved selected-Reboard. The exact [six-fixture patch](road81-integrated-trip-v1.patch), SHA **`6E413916E28CEB570F39DB1A1A52156FED009F3BE02EB319498ED5C6A93B3A33`**, passed isolated apply-check and byte-exact reconstruction. It restores the tested Road81 observer/world resources, adds reviewed per-member qualification retention and changes only the camera's viewing offsets for a steeper shot. No head-guide, driving-policy, actor or physical-gate change is included.
- World: `Worlds/Tests/ConvoyFollower_Arland_LoadedSupplyTrip_2Trucks.ent`; native lead 20 km/h and existing 60 m restart road walk. Five configurations and packaging passed. Package **`0813A7E99FE8861DF6F3EF71791FC8CA6D62FB4642881FFCBE84AB797069E77C`**; manifest **`B7D630C8B679573EEBC3F81F6D5754A3B0AF195E181BA7D2AE3B93CFB39C2AEE`**. All 585 frozen sources, three deployment files and nine protected gameplay/Reboard files verified.
- [Independent review](../../.cache/client/runs/road81-integrated-trip-v1/independent-review.md) preserves the raw failures, full log and exact-prefix terminal snapshot captured +259 ms. Natural closure, completed handles and an empty process check are verified. The 62 shutdown diagnostics remain; this is not a clean-lifecycle claim.
- Silent full recording: 524.2 s /7,862 frames, SHA **`119B9B8F4ACA7A257682A7346794B01B1701EEB137637F0D7FB95C30836F9BFA`**. Five live views including terminal were inspected; all trucks are visible at arrival/unload/restart, while trees obscure some travel. No full playback is claimed.

This is useful integrated automated-trip evidence with explicit owner staging and a native AI lead. Ordinary keyboard/menu input, human driving, selected-Reboard execution in this trip, multiplayer, repeatability and release readiness remain unproved.

## Development demonstration

The local [90-second game-only edit](../../.cache/test-videos/road81-integrated-trip-v1-demo.mp4) shows loaded staging, travel, arrival, Hold, unloading and resumed movement. Captions disclose the automated lead/server commands, log-backed supply counts and failed head spacing. It is silent and plays selected intervals at original speed. The private desktop recording is not intended for publication.

- Edit SHA `D9A0A70FED52DD0250393D6B5A6458D7BF4003145F52D7404A17ED9EFCE13DC8`; 1272×704, 15 fps, 1350 frames, 45,252,002 bytes.
- Original-video intervals: 55–63, 105–127, 214–222, 400–406, 431–447 and 450–477 seconds; three-second frozen end card. Crop `1272:704:644:684` excludes the desktop and taskbar.
- Root inspected the observed game crop, a nine-frame contact sheet spanning the edit, and the end card. Five original live views were also inspected. This is sampled visual review, not full playback. No ordinary menu or player-driving claim is made.
