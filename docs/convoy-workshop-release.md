# Convoy Follower Workshop release preparation

This is a preparation checklist. The addon has **not** been uploaded.

## Before publishing

1. Finish a fresh gameplay pass on the final packaged build: one, two, and three-truck following; stopped arrival while the player remains seated or exits; middle-unit loss and rechain; passenger staging; important radio reports; routine call debounce; and the selected squelch audio. Check `m_bVoiceEnabled 0` in a separately built control run and confirm driver interactions never show a voice toggle. The hard cap is five trucks; the planned live matrix is one through three trucks. Check that **Pull off and regroup** appears beside vanilla supply actions at the rear of only the owner's front occupied truck, and that releasing Unit One does not advance Unit Two until Unit One physically clears the bay. Cargo transfers remain player actions.
2. Test both ways of leaving the stop. When the owner's lead vehicle drives homeward past the parked return line, parked trucks and any unreleased outbound trucks should merge into one following convoy. Also test the explicit **Regroup convoy for return** action on a parked truck. When the player continues outbound or chooses **Resume convoy following** after a failed unload, unreleased trucks should follow while parked return trucks stay put. Test a blocked/narrow turn: the queue should hold, log the route failure truthfully, and allow rear-menu recovery without a false stuck voice call. Record actual vehicle positions, rear-menu visibility, voice playback, and logs. The new route and interaction have not yet passed an in-game acceptance run. Keep the [map-maker quickstart](convoy-mapmaker-quickstart.md) aligned with what the player actually observes.
3. Keep the editable source project and the player's original voice recording backed up. Bohemia notes that the published package cannot be recovered as editable source. Review the [Workshop terms and rights](https://community.bistudio.com/wiki/Arma_Reforger%3AMod_Publishing_Process) for all included audio, code, and images.
4. Prepare a Workshop title, short summary, full description, change notes, category, preview image, and optional screenshots. Bohemia's [publishing guide](https://community.bistudio.com/wiki/Arma_Reforger%3AMod_Publishing_Process) lists a 30-character project-name limit, 1,024-character summary, 5,000-character description, 30,000-character change notes, at least one category, and JPG/PNG images no larger than 2 MB each.
5. Check that the packaged addon has only its intended Arma Reforger dependency and that the example scenes are clearly identified as test maps. A separate Workshop scenario can declare Convoy Follower as a dependency and place the US or Soviet Convoy Driver group; see the [map-maker quickstart](convoy-mapmaker-quickstart.md) and Bohemia's [project dependency guide](https://community.bistudio.com/wiki/Arma_Reforger%3AMod_Project_Setup). The rear unloading option currently covers M923A1 transport and standard-body Ural4320 transport cargo parts, not every wheeled vehicle the boarding action can select. Smoke-test the Soviet group and Ural menu before publication; covered Ural inheritance is unverified.
6. Decide and test who can command a driver in mixed-faction scenarios. The current action checks ownership and vehicle availability but does not require the player's faction to match the driver's. Also test the owner disconnect/death cleanup and a blocked driver exit; a timed-out get-out order may leave the AI seated after its convoy session ends.

## Workbench publishing path

Bohemia's [official process](https://community.bistudio.com/wiki/Arma_Reforger%3AMod_Publishing_Process) requires a working mod and a Bohemia account linked in Workbench. In Resource Manager, use **Workbench → Publish Project**, fill the Workshop fields, bundle the project, review the size/confirmation prompt, then publish. Workbench manages later updates to the same item. The stable and Experimental Workshops are separate. Publishing is an external action and should happen only after the player-facing release pass and final review.

## Suggested listing draft

**Title:** Convoy Follower

**Short summary:** Add simple player-led AI vehicle convoys to Game Master and custom scenarios.

**Description draft:** Place a Convoy Driver group near an empty wheeled vehicle. A player can order the first driver to follow their vehicle, then add more drivers into a chain where each follows the truck ahead. Drivers can be staged on foot, ride as passengers, and report useful convoy events through optional private radio voice calls. Players load and unload supplies manually. Scenario makers can place the group prefab directly and tune documented addon settings in Workbench. See the included quickstart for setup and controls.

After the unloading test passes, the listing can also describe the M923A1 rear **Pull off and regroup** order and the player-led return sequence. Keep that feature out of the public listing until its menu, five-truck road maneuver, and return crossing are observed in game.

Check the final claims against the last in-game test before copying this description into Workbench. Do not promise autonomous cargo delivery, support for every truck, or an in-game settings menu.

## Player control beyond the rear menu

Bohemia documents both [ScriptedUserAction](https://community.bistudio.com/wiki/Arma_Reforger%3ACreating_a_User_Action) opening a custom menu and [Commanding Menu Modding](https://community.bistudio.com/wiki/Arma_Reforger%3ACommanding_Menu_Modding). A player-held command menu is technically possible, but it needs a separate interface and server-authoritative order routing. The current release candidate uses the truck's existing rear cargo context for unloading controls, with a manual return order as a backup to the drive-past event. A remote/phone-style menu is a later feature, not a game limitation.
