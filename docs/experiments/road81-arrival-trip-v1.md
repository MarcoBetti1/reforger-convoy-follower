# Road81: 200 delivered and both trucks resumed; spacing still fails

September 26, 2026. **Overall physical FAIL; raw fixture FAIL; strict FAIL.** The assisted native supply-trip stages completed individually. Head spacing remains a product failure; a separate fixture bookkeeping problem prevented its completion marker.

## Observed progress

- Both original trucks loaded **100 supplies each**, followed their real predecessor chain and reached the destination. The tail genuinely joined the original recorded route; no later route block occurred.
- Rear pacing remained active during the head's approach to its stopped lead. The rear link peaked at **44.6887 m**, improved from [the preceding run's](road81-entry-approach-v1.md) 88.2234 m. The head still peaked at **163.823 m**, failing the unchanged 60 m gate before this arrival branch began. This is one comparison, not repeatability proof.
- Both drivers completed **180.347 s /180 arrival Wait samples**, then real server Hold for **51.1162 s /51 samples**, all with zero measured drift. On-foot-owner and unowned-AI Resume attempts were rejected safely.
- The separately authored destination shift put the trucks **11.3042 /6.38131 m** from storage. Both natively unloaded 100 and canceled stably. All **559 original four-container ledger rows** conserve 1800; final counts are **source 1600 /truck one 0 /truck two 0 /destination 200**.
- Passenger Resume was accepted. The head's fresh sequence 5 earned **36.0823 m /11 powered intervals**; the tail's fresh sequence 6 earned **24.1697 m /seven**, retaining its genuine join and original predecessor. The native lead earned **55.2302 m /11**. No baseline, gap or replacement intervals were pooled.

The head qualified at 22:13:04.938–07.937, then naturally approached its next stop. The tail qualified at 22:13:15.955–17.954. The fixture requires both to remain eligible **in the same poll**, so it discarded the head's earned proof and timed out at **22:14:18.104**. Both original trucks subsequently held their own captured arrival Waits, with final net movement **54.9043 /53.6063 m**. Preserve this raw failure; retaining independently earned member qualification is the next narrow fixture correction. It must preserve every physical threshold, identity/failure check and the real spacing failure.

## Exact reproduction and limits

The [two-file patch](road81-arrival-trip-v1.patch), SHA `AF48CF54E56C80FE280861A348AC22ADE2E6B823AAEE417C6426CD9D3B8F355C`, applies after the exact frozen Road81 entry-approach source. Isolated apply-check and byte-exact reconstruction passed. Only the Entity controller's stopped-predecessor rear pacing and the destination layer's X coordinate change. No Reboard integration or other driving policy is included.

- World: `Worlds/Tests/ConvoyFollower_Arland_LoadedSupplyTrip_2Trucks.ent`; native lead remains capped at 20 km/h. Five configurations and packaging passed in `.cache/workbench-runs/convoy-road81-arrival-trip-v1`.
- Package: `86B0F6A880F2339EC846030217FB3AF9801D6F0514A6E7FF2D4CDBAE108449D3`; manifest: `7BEE0B7DCAD3E3E3A1CB854C8F4277CF44BE404F617A8158BD4D4FF654D5866C`. All **588 source /3 deployment files** verified.
- [Independent review](../../.cache/client/runs/road81-arrival-trip-v1/independent-review.md), SHA `094BA7BB979267CE60D9AF26C8C2D739E71D5C2B7572243D938058FB3436A042`. Raw terminal and **9 gameplay /0 post-result /64 shutdown errors** remain intact. Snapshot is an exact log prefix captured +88 ms; natural closure and all consumed process handles were verified.
- Silent recording: **590.133333 s /8,851 frames**, SHA `227A5467477046EC8D2A57E1F77F081BB8387BDBF6B8ADED44DE0E9A6B0EDB56`. Three live views were inspected; trees obscure some vehicles. No terminal screenshot or full playback is claimed.

This demonstrates useful scripted native cargo and command stages. Explicit owner staging and native AI lead assistance remain; ordinary keyboard/menu input, multiplayer, repeatability and release readiness are not proved. These private driving candidates are not promoted by this record.
