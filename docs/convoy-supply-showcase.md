# Short supply trip: Arland daylight

This scene brings the mod back to its intended use: recruit a driver, load a truck, lead it to a destination, unload and resume with the same driver. It is a development scene, not a completed demonstration or release acceptance.

- World: `Worlds/Showcase/ConvoyFollower_Arland_SupplyDay.ent`.
- Scenario: **Convoy Follower - Arland Supply Day** (`Missions/ConvoyFollower_Arland_SupplyDay.conf`).
- Three ordinary US Convoy Driver groups, three M923A1 supply trucks and a separate player truck.
- Native supply storage is authored with 1,800 supplies near the first truck; an empty destination with capacity 3,000 is about 195 m east along the road. Values are set on the actual child storage containers, whose quantities are aggregated by the storage parent.
- The scenario requests noon. A bare world launch may inherit the Game Master time.

There is no automatic recruitment, AI lead, hidden follower controller, scripted cargo transfer or forced seat possession in the scene. Driver behavior comes from the ordinary addon prefabs. Check [current validation](convoy-validation-current.md) for which backend is integrated and which behavior has actually run.

The scene explicitly enables native supplies through `CF_SupplyScenarioSettingsComponent`. The inherited Game Master mode otherwise disables this resource type, which hides the truck's native loading action. This is a one-time scenario setting, not an addon-wide override; later administrator changes remain respected. It does not add or transfer supplies.

## First trip: one follower, then two

1. Spawn at **Convoy supply staging - daylight** and use the front driver/truck pair first. Enter character play and close the Game Master editor before interacting; stay on foot for loading.
2. At the truck's rear, run the native continuous load action, then stop it at the desired amount. Check that source supplies decrease by exactly the amount added to the truck. If storage is initially unavailable, allow discovery a moment and check that the truck is close enough to the source; the scene's loading reach is still unverified.
3. Approach that driver on foot and choose **Join my convoy in closest empty vehicle**. This single action recruits the driver and assigns the nearby truck. Wait for the driver to occupy the intended truck, then enter the separate player truck's driver seat and drive east to the destination stack.
4. Stop beyond the stack so the follower's rear can approach it. Open the map and choose **HOLD ALL SEATED**. Wait for the panel to report **completed: all trucks holding in vehicles**, check that the truck remains still, then use **CLOSE MAP** and get out.
5. On foot at the assigned truck's rear, run the native continuous unload action and stop it at the desired amount. Verify that truck supplies decrease and destination supplies increase by the same amount, then check that counts remain stable after stopping the action.
6. Return to the separate player truck's driver seat, open the map and choose **RESUME ALL**. Use **CLOSE MAP**, drive on, and confirm that membership and truck assignment survive. The rear-menu **Resume after pull-off** action handles recovery from a pull-off maneuver; explicit **HOLD ALL SEATED** uses the map's **RESUME ALL**.

After one follower works, repeat with two: load and assign each truck separately using the same player, check loading reach for each, and recruit front to back. Unit One follows your lead vehicle; Unit Two follows Unit One. Confirm both trucks hold, unload and resume with their original assignments. Three followers, advanced pull-off and turning remain separate follow-up cases.

## Placement and test boundary

The road and vehicle coordinates reuse the existing OpenRoad course. The storage stacks sit beside the lane at approximately `(1343, 3337)` and `(1542, 3346)` in XZ. Their shoulder heights, clearance and native loading reach are provisional until inspected in the live world. This scene passed five-configuration validation and packaging in `.cache/workbench-runs/convoy-trip-v2/`, pack `E3A0EB37EA06A12AFA7AAB20C23380E91261698DD0E723AA4C3E693D3C711420`; it has not yet been inspected or played. Do not call a vehicle arriving at a stack a delivery without actual supply transfer.

The September 26 supply-setting and actual-container corrections passed all five configurations and packaging in `.cache/workbench-runs/convoy-supply-day-settings-v1`, pack `BAFCCAFB46F0FF8D5D805DD9CDCD206A95EC4761E0D0CA3E311C3CE679D21F7F`. The separate static native-action fixture has now loaded and unloaded 100 supplies with conservation; this establishes that action path, not this scene's final placement or completed delivery.

Ordinary desktop input is currently an unresolved environment boundary. Automated preset driving results do not establish player menu opening, clicking or restored driving controls.
