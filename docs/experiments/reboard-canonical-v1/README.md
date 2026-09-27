# Exact held-Reboard reproduction archive

These are source records for the successful local stationary test, not active addon resources. World files use a .txt suffix so the documentation tree cannot be mistaken for a loaded resource directory. The manifest preserves their exact original bytes.

## Reproduction boundary

1. Use the canonical baseline recorded by product-source-manifest.json (commit 93f22bda004df1d32a2355ff5408c0d7d68188af), or verify the five product hashes are already integrated. Apply product.patch only when its pinned baselines match.
2. In a separate copied addon, apply fixture.patch against its pinned original pair-probe baseline 2AF50353CCD9BE10ADF285E28F6171B72579D9F04174DC3A8F956701E2F1706D. It supplies the complete tested four-resource overlay, including the new world. Do not apply canonical-readback-adaptation.patch on top: it is an explanatory three-deletion diff from the prior private V3 probe, already incorporated in fixture.patch.
3. Verify the four final fixture hashes and sixteen dependencies in fixture-source-manifest.json. The new world is Worlds/Tests/ConvoyFollower_Arland_HeldReboard_2Trucks.ent, GUID7069BCC7F13648F6. If reconstructing the three world resources manually, restore their filenames from the manifest; .txt archive bytes are exact.
4. Use the repository's supported isolated five-configuration validation/pack/freeze process, launch that explicit world and package, and watch HELD_REBOARD_RESULT. The fixture uses the native lead/passenger setup, real server Hold/Reboard, assisted native exits and a guarded30-second close grace. Capture full logs, first-terminal prefix and video; report gameplay/post-result/shutdown errors separately.

The fixture is intentionally separate from the proposed five-product promotion. It also changes the existing pair cargo fixture's negative Resume calls and seat/Resume order outside held mode; those branches were not exercised here. Do not replace the canonical pair probe as an incidental gameplay integration. No cargo, travel, arrival, Resume, ordinary-input or multiplayer proof follows from the held-only result.

The exact tested deployment is documented in [the experiment](../reboard-canonical-v1.md); rebuilt packages may hash differently and require new attribution.
