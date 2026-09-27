# Road81 head comparison: steadier guide travel, spacing still fails

September 26, 2026. **Both runs: physical FAIL /raw fixture FAIL /strict FAIL.** One empty follower used the same authored Original Follow graph, original trucks, native lead pilot with passenger owner, 20 km/h lead request and Road81 destination. Rear pacing was disabled. The guide followed the actual lead's recorded trail; no future route was supplied.

| Evidence | Direct head | Recorded-trail guide head |
| --- | --- | --- |
| Continuous exact follower activity | 190.4065 m /33 powered intervals | 213.9806 m /31 powered intervals |
| Lead travel | 262.5012 m /32 powered intervals | 262.9111 m /32 powered intervals |
| Whole-run peak gap; limit 60 m | **128.812 m — FAIL** | **100.092 m — FAIL** |
| Stationary arrival; both trucks, limit 2 m drift | 180.396 s /180 samples, zero drift | 180.346 s /180 samples, zero drift |
| Gameplay /post-result /shutdown errors | 9 /0 /64 | 9 /0 /64 |

The guide genuinely joined epoch 1 **22.316 s after drive begin**, after native approach to the first recorded point. Before that join, its follower averaged **3.22 km/h**, versus **6.96** for the direct control over the matched elapsed window; its gap had grown to **83.151 m**. After joining and before lead parking, it averaged **19.57 km/h**, versus direct **12.38**, with both leads near **20.59**. These are sample means, not replacement gates. The guide gap still grew **87.10→95.74 m** in that second window: entry cost is substantial, but does not explain the entire failure. One comparison establishes neither repeatability nor a spacing fix.

Both retained their original assignments and selected their own captured arrival Wait. The new head-specific arrival handoff branch was **not exercised**: the guide instead used the inherited real-lead 10 m arrival path while recorded-route remainder was 44.7832 m, outside the new branch's 30 m bound. No cargo or explicit Hold/Resume was tested. Head-guide retained-pose and pending-Resume paths still require an addon predecessor; ordinary input, multiplayer and production use remain unproved.

## Reproduction and evidence

- Worlds: `Worlds/Tests/ConvoyFollower_Arland_Road81_DirectHead_1Truck.ent` and `Worlds/Tests/ConvoyFollower_Arland_Road81_GuideHead_1Truck.ent`. Five configurations and packaging passed in `.cache/workbench-runs/convoy-head-guide-road81-v1`.
- Shared package: **`9B58385623DFDED5B8E5FB1B61BEB263C84EEFF85E16380ABBDC44626270DFD0`**. Each run verified **596 source /3 deployment files**; source file records match exactly. Direct manifest `D4B452EE9FC921779C1E197FBF69867EC637E2135BAE3A3F589BE63D47A1AF51`; guide `DDBA383E4D169681E19B2D70B62E2ADFFD94C708BF431454D613EA932DAD9C36`.
- The exact [ten-file patch](head-guide-road81-v1.patch), SHA **`23B2F13B8F3EDA320CEFEB4FB5A6177D95F77ADF97DD85C13F7A21BEB9D57FF4`**, applies after the frozen [Road81 arrival comparison](road81-arrival-trip-v1.md), manifest `7BEE0B7DCAD3E3E3A1CB854C8F4277CF44BE404F617A8158BD4D4FF654D5866C`. Isolated apply-check and byte-exact reconstruction passed. Two scripts change and eight opt-in resources are added; ordinary roles remain unchanged.
- Private independent reviews: [direct](../../.cache/client/runs/head-guide-road81-direct-v1/independent-review.md) and [guide](../../.cache/client/runs/head-guide-road81-guide-v1/independent-review.md), with full logs, structured analysis and exact-prefix snapshots captured **+50 /+116 ms**. The guide analyzer requires genuine join and exact guide activity; direct 20 m activity cannot earn guide moving credit.
- Both closed naturally; all process handles completed and fresh checks found no remaining applications. Silent videos: **357 /373.066667 s**, SHA `E8C4FB983EDF8845C79EAC5E8DC5D64EE3C3F03CF35D90FE5164E0C5BB56D792` / `8D62AED63B1857F272F395A10F611CD5266EAAD38043F0ECFA530A3BCD5BFCC9`. Sampled live views and terminal screenshots were inspected, with partial tree obstruction; no full playback. A separate headless integration compile ran during guide stationary observation only; the frozen comparison package stayed unchanged.
