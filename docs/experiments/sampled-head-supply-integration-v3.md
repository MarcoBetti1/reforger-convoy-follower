# Sampled-head supply V3: delivered and restarted, declared gates failed

September 27, 2026. **Raw/physical/strict FAIL; private, unpromoted.** V3 changes only the pending-guide physical-observation lifecycle over [V2](sampled-head-supply-integration-v2.md). The native supply sequence, sampled head policy, original assignments, course and physical gates remain unchanged.

| Stage | Independent result |
| --- | --- |
| Native supplies | **200 loaded, transported and delivered**; 531 original-container rows conserve 1,800; final **1,600/0/0/200** |
| Baseline exact travel | Head **208.1263 m /25 powered intervals**, tail **241.5065 m /32**, genuine epoch 1 join |
| Arrival / real Hold | **180.427 s /180 Wait samples each**, then **51.1648 s /51 each**, zero drift |
| Head fresh Resume | Qualified; same-sequence best **50.888123 m /11 powered** |
| Tail fresh moving guide | **16.51392 m /6 powered**; does not meet 20 m gate |
| Actual final onward displacement | Head **54.9914 m**, tail **53.7483 m**, lead **55.2533 m /11 powered** |
| Peak spacing | Head **111.888 m**, tail **69.3399 m** — both exceed 60 m |

## What worked and what failed

The pending-guide branch executed at **00:59:15.791**, epoch 1/generation3, with the original predecessor and exact guide/waypoint. It observed actual pose without advancing the guide, issuing controls or granting activity credit. The guide then became selected and moved. **No trail block occurred**. The separate prejoin-retention branch still has no event and remains unexercised.

Tail's exact moving sequence 6 earned 16.51392 m /six powered intervals from **00:59:17.275–25.275**. It then switched to sequence 8, the legitimate **real immediate-predecessor 10 m stopped approach**, and continued physically to its final parking position. That onward movement cannot be pooled with sequence 6 to satisfy V3's declared continuous moving-guide gate. Head's qualification independently reproduces 25.399873 m /eight powered intervals, subsequently reaching 50.888123 m /11 within the same fresh sequence 5.

Terminal **01:00:35.508** is the 90-second restart timeout: `pass=false pair_complete=false`. The trucks delivered and physically restarted, but the tail's declared qualification and both spacing gates failed. The earlier [completed supply/Hold/Resume loop and demo](road81-integrated-trip-v1.md) remain separate evidence. No ordinary-input, Reboard-invocation, multiplayer or release claim follows from this run.

## Attribution and recording

All-five validation/packaging passed. **587 frozen sources /three deployments** verified without mismatch; only Trail controller **`CB477EA9339C79DE1E496D81482F448FC60841F4FDEDCE8D2D4BFB46FE395B55`** differs from V2. Pack **`60BE19638A673C35B539368D96605F38C8C657D8F3A904A9C6D11205E53FA99D`**; manifest **`04B12ECF280999053ECCF15286D37CE9A87D42D121251A6FA6FAB4EEAE259792`**.

Errors remain **9 gameplay /0 post-result /64 shutdown**. Exact-prefix snapshot **+122 ms**, native exit after 30.0156 s, all three root handles exit 0 and empty process/window checks are retained. The private silent recording is **565.933333 s /8,488 frames**, independently verified SHA **`E7A1ACC3534090F2675506F3F67118727F04FDD36C5601C2CA1407C71AE6334C`**. Root inspected six live views including a post-terminal view; no full playback. Raw recording includes desktop footage.

The [76-second game-only demonstration](../../.cache/test-videos/sampled-head-supply-integration-v3-demo.mp4) discloses automated interaction, incomplete formal restart, both spacing failures and unverified ordinary input/multiplayer. Root inspected its six-frame contact sheet and full end card, not entire playback. Its 1,140-frame 1272×704 output independently matches SHA **`B14DC2316CF3CA9771FE6D6618CD58D9EF0696AA0A9E0EBB1A20CB8D6313996C`**.

Evidence: [independent review](../../.cache/client/runs/sampled-head-supply-integration-v3/independent-review.md), [analysis](../../.cache/client/runs/sampled-head-supply-integration-v3/independent-review.json), [attribution](../../.cache/client/runs/sampled-head-supply-integration-v3/independent-source-verification.json), [closure/video](../../.cache/client/runs/sampled-head-supply-integration-v3/root-closure-video.json). Full log SHA `270AFC2F6E9782D562E851E6395057200BC47020247356A451E0D33453A11C84`; snapshot SHA `925309C6C6B89CF1120999FF228CE0671F3BFF50CB3BCB26B84219E355127B78`.

**Next:** prepare a private V4 candidate for the declared one-way transition to the exact real-predecessor approach, requiring that leg independently earn 20 m /three powered intervals. No pooled credit, lowered threshold or retroactive V3 pass. Source work is in progress; instrumentation/runtime scope awaits peer review, with no new build or live result.
