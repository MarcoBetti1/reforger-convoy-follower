# Held15 / native5: useful pace, spacing still fails

September 27, 2026. **Physical/raw/strict FAIL; private, unpromoted.** This single interaction comparison retained the actual-predecessor 15 m target-refresh cadence and changed native moving precision from 20 m to 5 m. Physical moving gap 20 m, stopped real-target approach 10 m, lead 20 km/h, cruise, ownership, course and all acceptance gates stayed unchanged.

| Evidence | Held15 / native20 | Held15 / native5 |
| --- | ---: | ---: |
| Mean speed, selected +10–30 s, 20 samples | 15.90 km/h | **17.55 km/h** |
| Peak actual predecessor gap | 79.1669 m | **66.1315 m FAIL** |
| Continuous follower progress / powered intervals | 251.197 m /35 | **259.4206 m /31** |
| Zero-drift arrival, 180 samples | 180.413 s | **180.346 s** |

The speed windows start from the first exact selected moving-activity sample: native20 at 00:01:15.048, native5 at 00:12:28.747. Their 20 speed samples span 00:01:25.064–44.081 and 00:12:38.763–57.780 respectively. These are single runs, not repeatability estimates or proof of the native planner's internal cause.

## Physical result

First spacing failure was **00:12:58.248**, world 52.3651 s. Terminal **00:16:45.179** retained it. Original follower movement used one exact sequence 1, waypoint `0x60000000000000F9`, proxy `0x60000000000000FA`, and actual predecessor `0x4000000000000006`. Lead progress was **259.0714 m /31 powered samples**. Proxy relocation supplied no movement or route/join credit.

All 51 proxy observations retained ownership; captured-transform error was **0**, maximum observed actual-lead lag **14.8728 m**. Sixteen refreshes occurred at **2.318–2.984 s** intervals. The owned proxy was retired at **00:13:19.280**; the normal stopped approach used the real lead at 10 m. Both vehicles completed 180 arrival samples with zero measured drift, original identities and the exact follower captured Wait retained. This run proves no cargo, explicit Hold/Resume, ordinary input or multiplayer behavior.

## Attribution and closure

The existing analyzer verified **598 frozen sources /three deployed files**, the exact two-file candidate contract and empty attribution `issues`. All five script configurations and packaging passed. Pack **`DBA02C44A2F2B4F3FE64EA1A7F6394C10B82E14187619F518D5FE9130EA61BA2`**; source manifest **`D8289F666F7E48761518E1149A9B22EA2E64F7F8CE1BB97EEB16BF78E30F5C5D`**. Candidate manifest **`2F66C71BB9032E2CF7092E3EC10B370EAEE7DA14A536D3262B143DD9177D2BBC`** is based on the exact [held15/native20](direct-head-sampled-proxy-v1.md) frozen source. The private candidate and full frozen source remain preserved under `.cache/direct-head-sampled-native5-v1` and the run directory.

Full errors: **9 gameplay /0 post-result /62 shutdown**, unwaived. Full log: **1,932,176 bytes**, SHA `11933727D1C38F19CDC7C53BC81BDC0F05956FCA00A3AEE7D31CFFEE2CE81CA1`. Terminal snapshot: exact **1,884,996-byte** prefix, SHA `F3D897FD57C0754A3FCB72A0BAA003517786C3B2FF519230882BC796E165B6CA`, captured **+89 ms**. Native exit was requested after 30.0995 s; cleanup and `Game destroyed` followed. Root consumed client/watcher/recorder with exit 0 and recorded no remaining matching processes/windows.

Root's recording probe reports **420 s /6,299 frames**, silent 2560×1440 at 15 fps. Video SHA **`B6E1AEAC9C06DFB0D27EB27EA060A56AF96C02F9345DA00CB4F17E10E968CF8A`** independently matches. One live travel view was inspected; no terminal screenshot or full playback. The full recording, including its desktop tail, stays private.

Evidence: [existing analyzer output](../../.cache/client/runs/direct-head-sampled-native5-v1/head-independent-analysis.json), SHA `A8DAE5282E63DDDA88F829823798BF4C041AB2EEA264543398905BA8370BD452`; [root closure/video](../../.cache/client/runs/direct-head-sampled-native5-v1/root-closure-video.json).

**Next:** stop isolated parameter tuning and test the useful improvement in the complete 200-supply loop on the current canonical gameplay/Reboard baseline, preserving the tail's actual predecessor route and every cargo, Hold, Resume and spacing gate. This is a private integration comparison, not promotion or release readiness.
