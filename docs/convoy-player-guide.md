# Convoy Follower: a short supply trip

**Development build.** An automated two-truck load/deliver/Hold/Resume trip is complete. Large head gaps and the normal-input workflow remain unresolved. See [current validation](convoy-validation-current.md).

## Set up and deliver

1. **Place drivers and trucks separately.** In Game Master, place a one-member **Convoy Driver** group (US) or **Convoy Driver (USSR)** beside each empty wheeled vehicle. Keep a separate lead vehicle for yourself. [Road81 Supply Day](convoy-road81-supply-day.md) provides two US drivers, follower trucks, your lead and storage for a short trip; start with one follower, then two. The earlier [Arland Supply Day](convoy-supply-showcase.md) is also available; its third truck starts outside loading range.
2. **Load before recruiting.** Enter character play and close the GM editor. On foot at each truck's rear, use the native continuous supply-loading action, then stop it. Verify matching source decrease/truck increase. Supplies must be enabled and storage in range; cargo transfer is manual.
3. **Recruit front to back.** On the driver, choose **Join my convoy in closest empty vehicle**. The nearest eligible empty wheeled vehicle wins—including a Jeep. Wait for the intended truck's driver seat to be occupied before recruiting another. **Follow me on foot** can stage an idle driver first. Then enter your separate lead.
4. **Drive, stop and Hold.** The chain is **your lead → Unit One → Unit Two → later units**, with five followers maximum, excluding your lead, and one spokesperson. Stop near the destination with room behind you. Open the normal map (normally **M**), choose **HOLD ALL SEATED**, and wait for **completed: all trucks holding in vehicles**. Check they are still before exiting.
5. **Unload and Resume.** Use **CLOSE MAP**, then the native continuous unloading action at each truck's rear. Stop it and verify matching truck decrease/destination increase; each truck needs actual storage reach. Return to the **same lead**, settle into its driver or passenger seat, choose **RESUME ALL**, close the map and drive on.

## Know what each control does

### Keep the line together

The map panel shows each truck's gap to the vehicle ahead. **Ease up** means a link is stretching; **wait safely** asks you to choose a safe stopping place and let it close. The leader gives restrained warning calls, while the panel identifies the affected unit. You control your lead vehicle's speed.

At an ordinary stop, **convoy closing up** or **convoy spacing together** describes spacing. Check each driver's order status before moving. **Pace: not assessed** means advice is unavailable; it gives no permission to accelerate. Explicit Hold, boarding, recovery and unloading maneuvers can suspend assessment.

Advice does not order Hold. Use **HOLD ALL SEATED** before unloading and **RESUME ALL** afterward. This feedback is integrated, but ordinary input and reliable full-trip cohesion still need validation.

### Commands

The panel appears with the normal map for the active convoy owner, seated or on foot. **Selecting a row does not make Hold or Resume an individual order.** Read server feedback: accepted/executing means work is underway; completed reports the observed result, and blocked gives a reason.

| Control | Meaning and conditions |
| --- | --- |
| **HOLD ALL SEATED** | Applies to the whole active convoy, preserving assignments. Stop the lead first; drivers must be seated and no conflicting maneuver may be active. The owner can also order Hold on foot within 25 m of the established stopped lead. Wait for completion before unloading. |
| **RESUME ALL** | Applies to the whole held convoy. The original owner must be fully seated in the same established lead; on foot, another vehicle or a seat transition is rejected. All assigned drivers must be seated and able to resume, with no conflicting maneuver. |
| **REBOARD SELECTED** | Select an affected row after an unexpected driver exit during explicit Hold and after automatic recovery becomes blocked. Requires the original, usable truck to be stopped with its driver seat free, the driver on foot and under AI control, and the other members held. The panel shows the eligibility reason. Wait for **completed: Unit … seated in original truck; Hold retained**; then issue Resume separately. Duplicate requests do not restart an active attempt. |
| **Stand down** | An interaction on the owned driver that dismisses them and releases the truck assignment; remaining members reconnect. This is not temporary dismount or a way to retain that driver's place. |
| **CLOSE MAP** | Closes the native map and panel. The complete ordinary opening/clicking/restored-driving-input loop remains a validation requirement. |

Temporary dismount with retained assignments is unavailable; there is no **DISMOUNT ALL** button. Selected Reboard recovers an unexpected exit, not a deliberate dismount order.

**Optional experimental maneuvers:** **PULL AHEAD AND WAIT** and **PULL OFF / REGROUP** have separate placement/recovery rules. They are unnecessary for basic held delivery; see [advanced unloading/return](../addons/ConvoyFollower/README.md#destination-unloading-and-return).

Source: [recruit action](../addons/ConvoyFollower/Scripts/Game/CF_AssignTruckAction.c), [panel](../addons/ConvoyFollower/Scripts/Game/CF_ConvoyMapPanel.c), [session](../addons/ConvoyFollower/Scripts/Game/CF_ConvoySession.c). Server-command tests do not certify normal input, Soviet parity or multiplayer.
