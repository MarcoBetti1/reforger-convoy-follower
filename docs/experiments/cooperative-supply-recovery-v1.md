# Full supply regression with command retry available

**27 September 2026 — closed FAIL.** The complete supply course included the tested explicit-retry/passenger-Hold product changes and cooperative leader pacing. It injected no failure. A natural tail order failure stopped the test before delivery; the earlier [focused command-retry success](explicit-retry-2truck-v2.md) remains separate evidence.

| Gate | Result |
|---|---|
| Native loading | 100 into each original truck |
| Conservation | 190 full-log records, same four containers, total 1800; no discrepancies |
| Exact moving activities | Head 249.06415 m /57 powered intervals; genuinely joined tail 220.58529 m /35, original epoch 1/predecessor |
| Adjacent gap peaks | 50.3638 m head /40.2523 m rear; below60 before early termination |
| Delivery | 0; source 1600, trucks 100+100, destination 0 |
| Arrival180 s /Hold /Resume | Not reached |
| Raw lifecycle errors | 9 gameplay / 0 post-result / 62 shutdown |

At **03:01:51.084**, the tail's original generation 1/sequence 2 lease failed with `guard_or_request_failure_admitted_move_3_handler_1`. The fallback failure followed at 51.102 and terminal at**51.551**. Original actor/vehicle/predecessor identities remained intact. There was no route-block event, admitted arrival-recovery branch or explicit retry command. Head arrival capture during post-terminal grace grants no arrival or Hold credit. Inclusion of the retry and arrival changes therefore proves neither branch in this run.

The next useful comparison is one explicitly observed command recovery from the natural failure, continuing toward the **original delivery destination**. It must retain the failed attempt and all cargo,180 s arrival, Hold and fresh per-member Resume gates. Extra lead travel must not be added merely to make a pending Resume pass.

## Reproduction and inspection

[Incremental source patch](cooperative-supply-recovery-v1-source.patch) and [manifest](cooperative-supply-recovery-v1-source.json) apply after the byte-exact [cooperative V2 source archive](cooperative-cohesion-trip-v2-source.json). Nine resources changed: three retry/passenger-Hold product files, README and five ordinary Road81 resources. The seven supply fixtures and private arrival controller were unchanged. Isolated application reproduced **all 590 frozen source hashes**; 3/3 deployment files also matched. World: `Worlds/Tests/ConvoyFollower_Arland_LoadedSupplyTrip_2Trucks.ent`. Pack: `82753824BCF978D1E3030EB206A646C22BC73B0910FD281B8B53D93D02E0568C`.

The terminal snapshot was an exact full-log prefix at+221 ms. Client, watcher, video and audio closed with exit 0; native game shutdown and empty process/window checks were retained. Private video 274.066667 s/audio 286.1 s were hash-verified. Two travel views were inspected; a late post-terminal capture failed after native close. No full playback or audio-content/audibility/synchronization review was performed; raw system-output media is not published here.

This failed regression does not negate the narrower command-recovery result or establish a completed supply loop with this build. Ordinary input, multiplayer, native navigation reliability and clean lifecycle acceptance remain open.
