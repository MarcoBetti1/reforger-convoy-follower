# Earlier prejoin offer: limited effect, spacing still fails

September26,2026. **Physical /raw fixture /strict FAIL. Private comparison; not promoted.** This one-file increment used the same Road81 guide-head world, original actors,20km/h native lead and unchanged physical gates as the [matched head comparison](head-guide-road81-v1.md).

The new branch offered an already-qualified recorded prefix before the existing3m physical entry capture. It preserved the actual first join, epoch, original lease and native steering. An unavailable prefix retained the existing approach; no route or physical credit came from the offer.

| Evidence | Prior guide control | Early offer |
| --- | ---: | ---: |
| First offer after drive begin |20.499s |15.850s |
| Physical entry capture |20.499s |19.084s |
| Genuine join |22.316s |20.900s |
| Guide creation→join |14.216s |13.833s |
| Whole peak gap; limit60m |**100.092m FAIL** |**97.8129m FAIL** |

`before_entry_capture=true` occurred3.234s before actual capture, but follower speed remained about6km/h. Guide creation itself was1.033s earlier under unchanged departure logic, leaving only0.383s improvement from creation to join. This does not establish a useful causal acceleration benefit or repeatability. No further startup tuning or promotion follows from this comparison.

The follower earned **209.58609m /29 powered intervals** in one exact guide activity; the lead earned260.137392m /31. Both retained original identities and completed **180.363s /180 arrival samples with zero drift** and their own selected Waits. No route blocker occurred. Arrival used the inherited real-lead approach; the head-specific handoff was unexercised. Errors remain **9 gameplay /0 post-result /62 shutdown**. No cargo, explicit Hold/Resume or normal-input claim.

## Reproduction

- Exact [one-file patch](head-guide-prejoin-approach-v1.patch), SHA `BDD587AD20BDD5E5D426420211A71636AA88520FC0FC276E0479DE8724B72EBC`, applies after the frozen head-guide control source manifest `DDBA383E4D169681E19B2D70B62E2ADFFD94C708BF431454D613EA932DAD9C36`. Candidate TrailController SHA `493689F56BA76CA8CAD413DB428EB206A465E2E3906E8F9E8BDDEBE6D1E7A5F3`. Canonical addon source is unchanged.
- All five configurations and packaging passed. New pack **`D5D5735F01FF82A06EDBFB6A8680C42456BBB56BEA93D656EDF078A721DCBA59`**;596-source manifest `D9F055F198DA9260C6BCAA5DA83FBD25C5736E6311558D6D06EF040AF7A70FED`. Same world: `Worlds/Tests/ConvoyFollower_Arland_Road81_GuideHead_1Truck.ent`.
- [Closed independent review](../../.cache/client/runs/head-guide-prejoin-approach-v1/independent-review.md): all596 sources/three deployment hashes verified; exact-prefix terminal snapshot+75ms. Natural client exit and watcher/recorder exits0; fresh process checks empty.
- Silent recording354.933333s, SHA `50C55BCB4D1BB5186FA3DA3926F0A6422EAFEA60DFEF5FFCE04EA5A2167A6D0C`. Three sampled live views were inspected with partial tree obstruction; no full playback.
