# Alternate Road81 supply trip: entry approach blocked

September 26, 2026. **Closed physical and strict FAIL.** This comparison kept the frozen loaded-trip V5 gameplay code and changed only the loaded-course fixture. It tested a second surveyed road after repeated native backward path folds on the original road.

Both original trucks loaded **100 supplies** through native actions. The source and destination were 283.15 m apart. The two storage props were aligned to native terrain once before binding; no truck pose, quantity or native eligibility was changed. The same ownership, cargo, spacing, arrival, Hold and powered Resume gates remained.

The test ended before sustained travel at `unsupported_initial_entry_geometry` for Unit Two. Its original predecessor, route epoch and truck assignment were retained. No delivery, arrival, Hold or Resume occurred. This is a different failure from the original course's backward navigation loop.

## Useful finding

The initial-entry guard extrapolates the predecessor's first recorded segment backward to the waiting truck. It rejects an unjoined truck when its along coordinate exceeds that segment, lateral distance exceeds 2 m, or facing alignment is below 0.9. On this curved starting road, that strict extrapolation rejected an ordinary staged follower before a guide was created.

The exact first sample, tangent and failing disjunct were not logged. Sampled motion supports a lateral-alignment explanation, but does not prove which predicate failed. The next candidate should permit a bounded native approach to the actual recorded entry while preserving the unjoined cursor, predecessor and epoch. It must earn physical route capture; widening progress credit or searching for a later nearby segment would not demonstrate that behavior.

## Evidence

- Five script configurations and packaging passed. Pack SHA `1819F8E1C987019372CB6382D2C0B2C8E3D4E6011D6F0F1FC62757C9B60CA8EA`; frozen manifest `CF2B5F71201894ED5586ABB22314B599A91007EFC4CD1D4A5A68FCA860BF1753`.
- All 588 source files and three deployed files verified. Only `CF_LoadedPairTripProbeComponent.c` and the loaded-pair world layer differ from frozen V5. [Archived fixture patch](road81-loaded-trip-v1.patch) applies to that frozen baseline, not directly to an arbitrary canonical checkout.
- 99 original four-container rows, including 69 through terminal, conserved 1,800 supplies. Final counts were source 1,600, cargos 100/100, destination 0. Both cancellation/stability checks passed.
- 39 driving identity rows retained their original actors. Peak links were 38.8966 / 27.3733 m before early termination; those short observations are not a full spacing pass.
- Terminal at 21:33:58.533; exact-prefix snapshot +210 ms, 413,451 bytes, SHA `9500E62D35448ED61BE18D901CCBDDF6D1A8C14F248472FA81590C13D1112250`.
- **9 gameplay / 0 post-result / 64 shutdown errors** remain. Native closure, consumed client/watcher/recorder handles and an empty fresh process check were verified.
- Silent recording: 147.066667 s, 2,205 frames, SHA `D76E023411DE1AF75CF2CCCB09E67E3B6AEAE41C1E4D9431870FFF259FF9F59A`. Staging and terminal gameplay views were inspected; full playback and ordinary player input were not.
- [Independent review](../../.cache/client/runs/road81-loaded-trip-v1/independent-review.md), SHA `6CDA7BE91D0A5AAECF2AB21A4BE95AE0745A5305CF65B2F247C08A97F19938C5`. Full private evidence is adjacent.

Preserve this course and the original road failure as separate regressions. No driving candidate was promoted by this test.
