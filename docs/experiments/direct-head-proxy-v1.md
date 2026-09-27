# Direct-head current-position proxy: sustained lag remains

**Physical FAIL; strict FAIL. Private candidate, not promoted.** Native desired distance 20 m, physical gap 20 m, stopped approach 10 m, lead 20 km/h and all physical gates stayed unchanged. An owned nonphysical proxy mirrored the actual predecessor every controller frame. The real lead remained the ownership, spacing, cruise and stopped-arrival target. Camera was the approved steeper presentation overlay.

- Terminal **23:44:39.274**, peak **125.481 m** against 60 m. First spacing failure **23:40:27.475**, world 37.5881 s.
- Lead **262.067 m / 32 powered samples**; original follower **205.997 m / 29 powered intervals** in exact moving sequence 1, waypoint `0x60000000000000F9`, native proxy `0x60000000000000FA`, actual predecessor `0x4000000000000006`.
- Normal stopped approach retired the proxy at **23:41:05.391**, then used the real lead at distance 10. Arrival completed **180.23 s / 180 samples, zero drift**, original identities retained and one exact captured follower Wait.
- **9 gameplay / 0 post-result / 64 shutdown errors**, unwaived. Natural exit requested after **30.0165 s**; cleanup and Game destroyed logged.

## Moving comparison

| Measurement | Direct20 control | Current-position proxy20 |
| --- | ---: | ---: |
| DRIVE_BEGIN to selected activity | 8.166 s | 8.151 s |
| Mean speed, first 10 selected seconds (10 samples) | 10.74 km/h | 11.60 km/h |
| Mean speed, next 20 selected seconds (20 samples) | 11.16 km/h | 11.29 km/h |
| Braking samples in that 20-second window | 5 | 7 |
| First 60 m breach after DRIVE_BEGIN | 21.467 s | 22.734 s |
| Peak gap | 128.812 m | 125.481 m |
| DRIVE_BEGIN to arrival observation | 94.216 s | 94.300 s |

Both single runs sustained roughly 11 km/h in the matched window. A generic entity at the current predecessor pose did not resolve slowdown. This does not identify the opaque native planning/braking cause. The prior native5 comparison also failed (118.396 m); no precision sweep or entity-type success is claimed.

Proxy readback covered **52 samples**, maximum update age **17.6055 ms** and actual-lead position offset **0.115742 m**. These bound observed frame-phase lag, not the planner's evaluation order or native velocity semantics. Full-ID MIXED/lease observations and exact frozen ownership checks bind the real lead, waypoint, proxy and generation. Creator decimal IDs 249 and 6 were resolved only to unique full IDs already observed in the complete log; ambiguous mappings are rejected. No route history/join or proxy displacement supplied movement credit.

## Attribution and closure

All **598 frozen sources / 3 deployment files** matched, with zero attribution issues. The exact direct20 baseline plus six replacements/two additions is preserved in the eight-overlay patch. Pack **83D314E0174776B02775F83C60D814F372FC7B9E0614180390B6C125975C5756**; source manifest **2FE7E8CCE9D9C44BEFBBD1CF41535EF2C523A7329BE39B3482C8E2DE4DC4DF1C**.

Terminal snapshot is an exact **1889125-byte** prefix, SHA **9DD75FBFD41DD3DB13DF7DF8EFCD31E767A13607ABC73E8E6CBDA9968AB99CB4**, captured +18 ms. Full log **1936769 bytes**, SHA **69D5B51B5480153AFFFE0D8A80384520EF999414C005581701429EBA67B4A52B**. Root consumed client/watcher/recorder with exit 0 and verified no matching processes/windows. Silent video **521.6 s / 7823 frames / 472200136 bytes**, 2560×1440 at 15 fps, SHA **C8FAAEAB05C9E2EAF6A44DFC62A94D2B4318EF5CBF3AA82A9860C0E6509FDF67**, independently hashed and probed. Root inspected two live travel/arrival views; no terminal screenshot or full playback. Post-game desktop remains private.

No cargo, explicit Hold/Resume, normal-input or multiplayer claim. The separate pending 15 m sampled-predecessor candidate tests held endpoints plus target lag; it changes neither this result nor this frozen source.

## Exact reproduction

[Eight-overlay patch](direct-head-proxy-v1.patch), SHA 9A3D577C023FB7178F12CB4C8DEEB950C25DCA650A17206A3DCC9A760322C3EE, applies to historical direct20 frozen manifest D4B452EE9FC921779C1E197FBF69867EC637E2135BAE3A3F589BE63D47A1AF51 / pack 9B58385623DFDED5B8E5FB1B61BEB263C84EEFF85E16380ABBDC44626270DFD0. Candidate manifest 2C4EE01F62E11C030AE09ECB4A7B74D78335312BC0F45B64940713B812EC3398. It includes the controller, explicit waypoint proxy admission, observer, two flags, empty proxy prefab/meta, and approved camera. Apply to that exact private baseline, not blindly to current canonical source. All five configurations and packaging passed; world remains Worlds/Tests/ConvoyFollower_Arland_Road81_DirectHead_1Truck.ent. [Independent report](../../.cache/client/runs/direct-head-proxy-v1/independent-review.md).
