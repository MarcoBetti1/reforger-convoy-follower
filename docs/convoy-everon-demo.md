# Everon night convoy demo

This is a separate Convoy Follower test scene. The Arland test scene remains available. The Everon world inherits the installed `GM_Eden.ent` Game Master world; Everon is named **Eden** in the game resources, and the map labels the destination **Levie**.

## Open and play

1. Open `Worlds/Tests/ConvoyFollower_Everon_Night.ent` in Workbench:

   ```powershell
   npm run workbench:open -- --editor world --project addons/ConvoyFollower/addon.gproj --world Worlds/Tests/ConvoyFollower_Everon_Night.ent --execute
   ```

2. Press **F5** and select the US spawn named **Convoy staging - Regina sports ground**. The custom spawn point attempts to put the player in the preplaced **CF_EveronLeadLAV** driver seat. If that attempt does not work, the player should appear on foot beside the LAV; enter its driver seat manually. The seat attempt needs a gameplay check.
3. Three Convoy Driver groups stand beside three M923A1 trucks in a clear line behind the LAV on the open sports ground. The covered supply stack west of the line is the source. Use each truck's normal rear cargo menu to load supplies manually, then give the convoy follow order. No cargo transfer is automated by Convoy Follower.
4. Leave the sports ground through its open side toward the Regina road, then use the in-game map for a road route toward **Green Valley**, **Peters Pass**, and **Levie**. This waypoint sequence bends east and back west through the heights and gives a drive of several kilometres. The exact field exit and road turns need a player check. The destination is the covered stack and floodlight just southwest of the Levie map label, around **(7445, 4724)**. Stop the LAV near that marked unload area and test the arrival, rear-menu release, and return-line behavior.

The packaged scenario is `Missions/ConvoyFollower_Everon_Night.conf` (scenario ID `{E2C7751867D04A19}Missions/ConvoyFollower_Everon_Night.conf`). It requests **22:00**, fixed time speed, and fixed weather through the mission header. Launch this scenario from the game's **Scenarios** menu for the intended night start. A bare Workbench F5 world preview may use the parent Game Master time settings instead of the scenario header. If F5 opens in daylight, enter Game Master, open **Scenario properties** with **V**, then set **Time and Date → Time** to **22:00** and leave time progression at **1.0×**. Bohemia documents those Game Master controls; their exact behavior in this preview still needs a player check. The F5 preview was previously blocked at a vanilla **Script Authorization Required** prompt, so neither launch path has yet been visually confirmed at night.

## Scene inventory

| Item | Approximate world position | Purpose |
| --- | --- | --- |
| US spawn | (7239, 2440) | Attempts a LAV driver-seat spawn, with on-foot fallback |
| LAV-25 | (7233, 2438) | Named `CF_EveronLeadLAV` for the custom spawn point |
| Three M923A1 transports | (7233, 2452), (7233, 2467), (7233, 2482) | South-facing convoy line on the clear sports ground, each with a nearby driver group |
| Covered source stack | (7218, 2467) | Resource storage seeded with 1,800 supplies for manual truck loading |
| Covered destination stack | (7445, 4724) | Empty storage with room for 3,000 supplies near Levie |
| Floodlights | Source and destination | Make the work areas recognizable at night |

The sports ground was inspected in World Editor after the original sawmill-edge truck line was found intersecting brush, a tree, and rocks. A terrain point near the replacement line measured about `(7233, 138.83, 2463)`; the replacement entities use a common approximate ground height of `138.9`. The covered Levie destination stack appears on a paved apron directly beside the village road. The apron is relatively narrow, so a truck stop and U-turn there require a live check. Exact ground alignment, field exit, spawn selection and seat transfer, resource transfer, and floodlight visibility also require first-person gameplay verification.

For the first player check, confirm the selected spawn seats the player in the named LAV or falls back on foot beside it, the vehicles rest on clear ground, all three trucks can load from the source stack, and the LAV can leave the sports ground. At Levie, confirm the lead truck stops by the paved apron without clipping the stack. Check that the release option appears at the truck's rear, each released unit can turn and clear the bay, and the homeward crossing merges the return line with any unreleased trucks. Record whether the F5 preview or scenario menu actually starts at 22:00.

## Headless check

The revised Everon scene passed Workbench script validation for WORKBENCH, PC, XBOX, PS4, and PS5 and packaged successfully. Its isolated probe pack is `.cache/probe-packs/everon-staging-clear/data.pak`, with logs under `.cache/workbench-runs/everon-staging-clear/`. These checks establish resource packaging and script compilation, not the seat spawn, final vehicle positions, cargo menus, night start, or convoy driving in game.

The world parent and place-name coordinates were read from the installed base-game resources. The [official Everon terrain reference](https://community.bistudio.com/wiki/Arma_Reforger%3AEveron) confirms the internal map name Eden, and the [official mission-header API](https://community.bistudio.com/wikidata/external-data/arma-reforger/ArmaReforgerScriptAPIPublic/interfaceSCR__MissionHeader.html) defines the night-start fields used here.
