# Direct-head native precision 5: sustained lag remains

September 26, 2026. **Physical / raw fixture / strict FAIL. Private comparison; not promoted.** This matched Road81 one-follower experiment changed native moving precision from 20 m to 5 m. The physical moving gap remained 20 m, stopped approach 10 m, and native lead cap 20 km/h. Original actors, course, camera and acceptance gates were unchanged from the [direct-head control](head-guide-road81-v1.md).

| Evidence | Direct20 control | Native5 |
| --- | ---: | ---: |
| Moving activity after DRIVE_BEGIN | 8.166 s | 7.233 s |
| Mean sampled speed, first 10 selected seconds | 10.74 km/h | 12.07 km/h |
| Mean sampled speed, next 20 selected seconds | 11.16 km/h | 11.33 km/h |
| First 60 m breach after DRIVE_BEGIN | 21.467 s | 24.318 s |
| Whole peak gap; limit 60 m | **128.812 m FAIL** | **118.396 m FAIL** |
| Arrival observation begins after DRIVE_BEGIN | 94.216 s | 89.267 s |

The candidate started its native activity 0.933 s sooner under unchanged departure logic. Both single runs then sustained roughly 11 km/h in the matched 20-second window; smaller precision did not resolve the lag. This is not a repeatability estimate or proof that precision caused the whole peak difference. At 23:19:09.980 the candidate was braking at 13.471 km/h despite a nearby cruise allowance of 41.7455 km/h; the same exact native activity/request remained. Native moving-target planning/formation behavior is the next useful boundary to inspect. Returned path arrays do not establish the internal braking cause. No further precision sweep is justified here.

The original follower earned **210.297 m / 29 powered intervals** in one exact moving sequence with actual lead target and desired distance 5; the lead earned **260.535 m / 31**. Normal arrival used a fresh desired-10 approach. Both retained original identity and completed **180.33 s / 180 arrival samples with zero drift** and their own selected Waits. Errors remain **9 gameplay / 0 post-result / 64 shutdown**, also present as those counts in the direct control. No cargo, explicit Hold/Resume, normal-input or multiplayer claim.

## Exact reproduction and evidence

- [Four-overlay patch](direct-head-native5-v1.patch), byte-preserved SHA `0ABD58FC43715524A34AFE141D60D5230FD8E5A85B0264317221B29BC284A31C`. It changes only the private Entity controller moving precision hook, observer declaration/readback, original test character flag and direct-head world-layer flag. Stopped distance, physical cruise and movement/arrival gates remain unchanged.
- Apply to frozen direct-control manifest `D4B452EE9FC921779C1E197FBF69867EC637E2135BAE3A3F589BE63D47A1AF51`, pack `9B58385623DFDED5B8E5FB1B61BEB263C84EEFF85E16380ABBDC44626270DFD0`. Candidate manifest: `BEBE1B091EAFE265040674EEB42014067A28668F87C4CF043672D00DFCE6F6B8`. This is a historical private source baseline, not a patch to apply blindly to current canonical source.
- All five configurations and packaging passed. Candidate pack **`71CFCFD2477AEB84F887F61678D37DDFFC76E48183BD853B56DA9F381403C7E9`**; 596-source run manifest `7B73F9501C53FF62CFB72475F88BEEEA26C629A29E0C98F88201DF4E36D38EF4`. World: `Worlds/Tests/ConvoyFollower_Arland_Road81_DirectHead_1Truck.ent`.
- [Closed independent review](../../.cache/client/runs/direct-head-native5-v1/independent-review.md): all 596 sources/three deployment hashes matched; terminal snapshot exact-prefix SHA `0800108AA6E9D1E93708681B9255DF2D41134FAD5B76B4D99224BE27C7E20E63`. Natural client closure after 30.0163 s; client, watcher and recorder exits 0; root process/window checks empty.
- Silent recording 352.933333 s, SHA `0017362ABB28B2552CD0D4A61366807284A9A49F4E1CDA3C7CC31F36A11768BD`. Three sampled live views were inspected with partial tree obstruction; no full playback.
