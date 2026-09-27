# Short supply trip: Arland daylight

This scene brings the mod back to its intended use: recruit a driver, load a truck, lead it to a destination, unload and resume with the same driver. It is a development scene, not a completed demonstration or release acceptance.

- World: `Worlds/Showcase/ConvoyFollower_Arland_SupplyDay.ent`.
- Scenario: **Convoy Follower - Arland Supply Day** (`Missions/ConvoyFollower_Arland_SupplyDay.conf`).
- Three ordinary US Convoy Driver groups, three M923A1 supply trucks and a separate player truck.
- Native supply storage starts with 1,800 supplies between the first two trucks. Their authored resource origins are about 10.7 m from it, within the native 12 m search range. An empty destination with capacity 3,000 is about 200 m east along the road. Values are set on the actual child storage containers, whose quantities are aggregated by the storage parent.
- The third truck starts outside loading range and needs separate staging. Leave it unused for the first trip.
- The scenario requests noon. A bare world launch may inherit the Game Master time.

There is no automatic recruitment, AI lead, hidden follower controller, scripted cargo transfer or forced seat possession in the scene. Driver behavior comes from the ordinary addon prefabs. Check [current validation](convoy-validation-current.md) for which backend is integrated and which behavior has actually run.

The scene explicitly enables native supplies through `CF_SupplyScenarioSettingsComponent`. The inherited Game Master mode otherwise disables this resource type, which hides the truck's native loading action. This is a one-time scenario setting, not an addon-wide override; later administrator changes remain respected. It does not add or transfer supplies.

## First trip: one follower, then two

1. Spawn at **Convoy supply staging - daylight** and use the front driver/truck pair first. Enter character play and close the Game Master editor before interacting; stay on foot for loading.
2. At the truck's rear, run the native continuous load action, then stop it at the desired amount. For two trucks, load both before recruiting their drivers. Check that the source decrease equals the combined truck increase and that counts stay stable after stopping. Loading 100 into each should leave source **1,600**, truck one **100** and truck two **100**. If storage is initially unavailable, allow native discovery a moment and check actual range and eligibility; this scene's ordinary interaction remains unverified.
3. Approach that driver on foot and choose **Join my convoy in closest empty vehicle**. This single action recruits the driver and assigns the nearby truck. Wait for the driver to occupy the intended truck, then enter the separate player truck's driver seat and drive east to the destination stack.
4. Stop beyond the stack so the follower's rear can approach it. Open the map and choose **HOLD ALL SEATED**. Wait for the panel to report **completed: all trucks holding in vehicles**, check that the truck remains still, then use **CLOSE MAP** and get out.
5. On foot at the assigned truck's rear, run the native continuous unload action and stop it at the desired amount. Verify that truck supplies decrease and destination supplies increase by the same amount, then check that counts remain stable after stopping the action.
6. Return to the same lead truck, open the map and choose **RESUME ALL**. The owner may issue Resume from its driver seat or a settled passenger seat when someone else drives. On foot, during a seat transition, or from a different truck, the order is rejected. Use **CLOSE MAP**, drive on, and confirm that membership and truck assignments survive. The rear-menu **Resume after pull-off** action handles recovery from a pull-off maneuver; explicit **HOLD ALL SEATED** uses the map's **RESUME ALL**.

After one follower works, repeat with two: load both, then recruit front to back using the same player. Wait for each driver to occupy its intended truck before recruiting the next. Unit One follows your lead vehicle; Unit Two follows Unit One. Confirm both trucks hold, unload and resume with their original assignments. After fully delivering both 100-supply loads, expect trucks **0 / 0**, destination **200**, source **1,600**. Each truck must be within native destination range; arrival alone does not prove delivery.

For a later three-truck attempt, manually stage the still-unassigned third truck near the source, load it, then recruit it in chain order. Its original position is about 36.7 m from storage. Three-follower delivery and advanced maneuvers remain separate validation cases.

## Recover a held driver without recruiting again

If a driver unexpectedly leaves their truck during an explicit Hold, the controller first attempts a bounded automatic reboard. If that recovery stops, keep the other trucks held, open the convoy map panel, select the affected unit and choose **REBOARD SELECTED**. This command becomes available only for an eligible blocked driver on foot with their original, stationary truck's driver seat free. The panel explains why it is unavailable otherwise.

Wait for **completed: Unit … seated in original truck; Hold retained**. Accepted or executing means the driver is still working on the order. Repeated requests do not restart an active attempt. A completed reboard preserves membership, the assigned truck and Hold; use **RESUME ALL** separately when ready. This is recovery for an unexpected exit, not a temporary dismount command or a way to choose a different truck.

The [focused recovery comparison](experiments/reboard-canonical-v1.md) physically verified two automatic returns, the selected retry, duplicate rejection and the original-seat held result. It issued the real server command from a fixture; opening/clicking the new button through normal input and multiplayer behavior still need verification.

## Placement and test boundary

The road and vehicle coordinates reuse the existing OpenRoad course. The storage stacks sit beside the lane at approximately `(1337, 3336)` and `(1542, 3346)` in XZ. The revised source midpoint loaded both trucks in the separate scripted loaded-trip fixture. Supply Day's live terrain clearance, native queues and ordinary interactions remain unverified; the native resource search is separate from the player's rear interaction reach. The unchanged destination placement is also provisional.

The source correction and passenger Resume admission are integrated and passed all five script configurations and packaging in `.cache/workbench-runs/convoy-player-trip-controls-v1`, pack `DA64B3A01FBA62A72472736F8A67C69231BF5A63664170B4A89F47DAC55529B0`. That combined canonical package has not had a separate live run. The [V3 experiment](experiments/loaded-straight-trip-v3.md) verifies actual passenger Resume admission and subsequent movement, but its overall spacing and qualified restart gates failed. Its private driving changes are not part of this canonical promotion.

The September 26 supply-setting and actual-container corrections passed all five configurations and packaging in `.cache/workbench-runs/convoy-supply-day-settings-v1`, pack `BAFCCAFB46F0FF8D5D805DD9CDCD206A95EC4761E0D0CA3E311C3CE679D21F7F`. The separate static native-action fixture has now loaded and unloaded 100 supplies with conservation; this establishes that action path, not this scene's final placement or completed delivery.

Ordinary desktop input is currently an unresolved environment boundary. Automated preset driving results do not establish player menu opening, clicking or restored driving controls.
