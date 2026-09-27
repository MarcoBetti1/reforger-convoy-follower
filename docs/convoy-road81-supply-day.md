# Road81 Supply Day

This prepared US scene places two convoy drivers, two follower trucks, a separate player truck and nearby supply storage on Arland. It is integrated and passes validation/packaging; the complete ordinary player workflow remains untested.

Open scenario **Convoy Follower - Road81 Supply Day**, or world `Worlds/Showcase/ConvoyFollower_Arland_Road81SupplyDay.ent` in the addon project. The scenario requests noon; a bare World Editor preview may inherit another time. Use the nearby **Convoy Road81 supply staging - daylight** US spawn and close the GM editor before interacting.

1. **Load the two rear trucks.** The foremost empty truck is yours. Use each follower truck's native rear loading action and stop it at 100 supplies. Expected counts: source **1,600**, trucks **100/100**, destination **0**. Allow native storage discovery a moment if the action is initially unavailable.
2. **Recruit front to back.** Choose **Join my convoy in closest empty vehicle** on the front driver, wait for seating, then repeat for the rear driver. Enter your separate lead truck. For a one-follower test, leave the second pair untouched.
3. **Drive east to the empty storage.** It is about 276 m away near map XZ **1539/3076**. Follow the road, let the trucks close up, and use panel gap/advice to choose a suitable pace. Stop with space behind you for both trucks to reach the destination storage.
4. **Hold and unload.** Open the map, choose **HOLD ALL SEATED**, and wait for completed status and stationary trucks. Close the map and use each truck's native rear unloading action. Verify source **1,600**, trucks **0/0**, destination **200**; every truck must actually be in storage range.
5. **Resume the same convoy.** Re-enter the original lead, settle into the seat, choose **RESUME ALL**, close the map and drive on. Check both original drivers and trucks follow in their original order.

Keep assignments during unloading. **Stand down** dismisses a driver; it is not a temporary stop. See the [player guide](convoy-player-guide.md) for the distinct controls and selected Reboard recovery.

The scene has no scripted pilot, recruitment, cargo transfer or follower assistance. Its storage placement comes from a measured automated delivery, but ordinary spawning, native action reach, input and parking still require live checks. The standalone automated supply tests and their failed spacing/recovery cases remain in [current validation](convoy-validation-current.md).
