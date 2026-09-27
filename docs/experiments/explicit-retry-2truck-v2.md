# Two-truck explicit command recovery

**27 September 2026 — V2 physical recovery PASS; strict lifecycle FAIL.** Two ordinary assigned drivers recovered through real server Hold/Resume after one deliberately induced authored-order failure. This supports explicit command recovery, not a native navigation fix or release readiness.

| Evidence | V1 | V2 |
|---|---:|---:|
| Induced failure and exact lease retirement | Observed once | Observed once |
| Server Hold | Rejected | Accepted;30.0995 s,30 samples each,0 m drift |
| Fresh head Resume proof | Not reached | Sequence3:21.32367 m /4 powered intervals |
| Fresh tail Resume proof | Not reached | Sequence4:22.007909 m /4 powered intervals; original epoch1/join |
| Maximum adjacent gap | 40.2855 m before early termination | 58.8781 m;60 m gate passed |
| Outcome | FAIL at Hold admission | Focused physical recovery PASS |
| Gameplay /post-result /shutdown errors | 9 /0 /64 | 9 /0 /64 |

## What changed and what was proved

V2 changes only Session's Hold admission to reuse the existing exact established-lead passenger resolver. All593 other frozen source files, fixture phases and physical gates match V1. No global piloting rule changed. The product candidate also contains the previously reviewed Entity/Trail explicit retry preflight: active failure can recover on an authorized command while the historical failed attempt remains recorded.

The fixture first requires each follower's real20 m /3 powered proof and a genuinely joined tail. It calls the exact executing lease's `Block` sink once, labeled **SYNTHETIC AUTHORED ORDER FAILURE**. It does not fabricate a native `UNREACHABLE` result, exercise a typed native callback, or inject successful recovery. The owner stays in the original lead vehicle; existing native AI drives that lead. There are no cargo operations or extra seat transfers.

V2 held both original drivers in their original trucks, then accepted actual Resume. A duplicate command was rejected without resetting the pending Wait, deadline or generation. Each follower earned its own fresh continuous proof, with independent signed-step/counter checks and unchanged ownership, actual predecessor and route history. No distance was pooled across activities or the failed generation. By terminal, the same activities had reached36.21849 m /7 powered intervals head and49.598859 m /8 tail; the native lead reached21.4742 m /4. Old tail generation1 remained failed/revoked while generation2 executed. Terminal: **02:51:08.979**; V1 remains failed at02:45:02.912.

## Reproduction and evidence boundaries

[Exact source patch](explicit-retry-2truck-v2-source.patch) and [manifest](explicit-retry-2truck-v2-source.json) apply to raw Git commit `78a17900723f7842a5127c10263cdb95d05b4b5c`. Use `core.autocrlf=false`; isolated `git apply --check`, application and **all594 reconstructed byte hashes passed**. Three product files and ten private fixture resources carry authored changes;123 additional differences preserve frozen line endings. V1 reconstructs by restoring only Session from that raw baseline; all594 V1 hashes also matched. Launch world: `Worlds/Tests/ConvoyFollower_Arland_ExplicitRetry_2Trucks.ent`. The fixture remains experimental.

- V1 pack: `F113FACAF9904F24FA8B3AA168E058FE543AE603542D6063E63959FAA8B1A804`.
- V2 pack: `94C1C0B577FEA279145E9CE91798F45F611EEF3CDC4EF7737C6CE52583FEFF2B`.
- Both runs:594 frozen source and3 deployed files independently verified. Terminal snapshots matched full-log prefixes at+137 ms /+136 ms. Natural closure and all client/watcher/recording exits were0; raw lifecycle errors remain counted.

Private recordings were hash-verified after closure: V1 video163.666667 s/audio175.6 s; V2 video182.8 s/audio194.8 s. Inspection covered two/three sampled live views, including V2 moving restart and post-terminal; neither full playback nor audio audibility/sync was verified. System-output audio and raw media are not published here. This course establishes no cargo delivery,180 s arrival, ordinary keyboard/menu use, multiplayer, automatic navigation recovery or general reliability claim. The next useful gate is the complete supply loop with these narrow product changes.
