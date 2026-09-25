# Testing Raven AI Commander in Conflict

This is a player guide for the local Raven test. It distinguishes steps we actually completed from behavior described by the mod author. [Field notes](raven-field-notes.md) hold the evidence and open questions.

## Enter the prepared session

The verified setup used **Conflict - Arland** with **Raven AI Commander** as the only enabled mod in a private player-hosted session. At **Deployment Setup**, choose **US Army** for the tested path: confirm the faction, join an available US group (**Atlas Red 2** in our run), continue to the deployment map, and deploy at **Main Operating Base**. This reached first-person play in the base tent. Raven also initialized a Soviet logistics context, but the US path is the one exercised here.

If recreating the test, download **Raven AI Commander** through Reforger's in-game Workshop first. Its Workshop ID is `6A3A112604EF7286`. Then go to **Multiplayer → Host → Host new server**, select **Conflict - Arland**, and move Raven into the **Enabled** Host Mods column. The verified private host form used **Visibility No**, **Host LAN Yes**, **Bind IP 127.0.0.1**, and port `2001`. Check the complete Bind IP after leaving the field, then Host. The isolated profile already used for this test has Raven registered, so reuse it to avoid another download. See the [operations playbook](operations-playbook.md) for the profile path and UI quirks.

## What Raven does automatically

The [Workshop description](https://reforger.armaplatform.com/workshop/6A3A112604EF7286-RavenAICommander) calls Raven an autonomous Conflict logistics system. Its [changelog](https://reforger.armaplatform.com/workshop/6A3A112604EF7286-RavenAICommander/changelog) describes automatic HQ and depot discovery, vehicle and crew creation, supply-run task claiming, and convoy transport. Read-only inspection of the installed scripts indicates that a faction context queues a supply run from its nearest depot **to its HQ** when cached HQ stock is below 500 and no run is open. The planned amount is 750 supplies. **You do not need to issue a delivery order:** no player command, menu, or radio input was found in the installed addon. In the local Arland session, Raven detected Conflict and initialized vehicles and crews. It later claimed and started one convoy after the player placed transport trucks, but no completed transfer was observed.

## First useful check

1. Open the map and identify your faction's Main Operating Base and a nearby supply depot. Note both displayed supply counts before any movement. This is for observation; Raven's delivery decision is automatic. The [official Conflict guide](https://community.bistudio.com/wiki/Arma_Reforger%3AConflict) explains that supplies are held at bases and depots; main-base replenishment can be slow.
2. Watch for a friendly Raven logistics vehicle leaving a depot and returning to HQ. Do not treat a spawned vehicle or crew alone as a delivery.
3. Recheck depot and HQ counts. A completed delivery needs a corresponding stock change, alongside the observed trip or a clearly identified virtual-cargo result.
4. Tell Codex the faction, depot and HQ names, starting and ending counts, vehicle behavior, and elapsed time. The log summary command in the [operations playbook](operations-playbook.md) can then look for `SUPPLY_RUN CLAIMED`, `SUPPLY_RUN BLOCKED`, and transfer messages without mistaking startup for delivery.

The local Arland session initially logged **zero** supplies at the main base and no `SUPPLY_RUN` events for about 15 minutes. The player later placed two M923A1 transport trucks near the US main base. Roughly 25 seconds later Raven logged **SUPPLY_RUN CLAIMED**, a convoy start, and a switch to an outbound waypoint. The installed code shows Raven uses its own vehicle bound at startup, so those placed trucks were **not** a confirmed trigger. No physical movement, loading, arrival, or resource transfer was verified before the game closed. Raven's author says version 1.0.6 was designed for testing alongside the companion **Raven Conflict PvE** scenario on Everon. Use that companion scenario as a separate compatibility check if Arland does not complete the run, and record that its additional mod was enabled.
