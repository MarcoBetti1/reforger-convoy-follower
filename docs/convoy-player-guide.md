# Convoy Follower: five-truck supply test

Development build, September 30, 2026. Marco has observed five original followers reach a destination. Arrival creeping, unloading and departure are being checked independently; this is not a release or completed ten-truck test. Testing is capped at five followers, excluding your lead.

## Recruit and start

Place a one-member Convoy Driver group beside each empty transport truck and keep a separate lead vehicle. Recruit front to back with **Join my convoy in closest empty vehicle**; wait for each intended driver seat before recruiting the next. Newly recruited drivers wait for **Start convoy**. Enter your original lead, open the assigned convoy menu key, choose Start and drive forward with a clear gap.

**Hold all trucks** keeps assignments and seated drivers. **Resume all trucks** restarts a held convoy from the original lead. Start is the initial authorization; Resume is for an existing held assignment. Read the blocked reason and wait for physical movement or a stationary completed state. Commands do not repair an impossible native route.

## Native loading and unloading

On foot, approach the M923A1 or standard Ural transport's rear cargo interaction. Use the native continuous Load/Unload supplies action, stop it, and verify the truck and nearby storage counts. The addon does not transfer cargo automatically. Supplies must be enabled, real storage must be within range, and the truck must contain supplies to unload. The refreshed Everon night scene enables native supplies and uses physical supply-stack containers; its five follower trucks start near two source stacks of900 supplies each, positioned for front and rear access (1800 total). A LAV lead is not a substitute for a cargo truck.

## Unload one truck at a time

1. Stop the lead where you want trucks unloaded. Choose **Unload bay → 1. Save bay / Hold queue**. The original position is saved automatically; no later “mark moved out” action is needed.
2. Unload your lead if applicable. Drive clear to a wide, flat waiting area and stop. For five trucks, allow roughly 70 metres from the bay and ample clear space beside and behind the lead. Choose **2. Set waiting area**. The command checks every future parking slot before accepting the area; read its specific reason if blocked.
3. Choose **3. Admit next truck**. Wait for the original truck to reach the loading area, stop and report ready. An exact heading match is not required. A road is not mandatory: off-road native navigation can be attempted, but buildings, rocks, steep ground and tight turns can still block it.
4. Get out and use that truck's native rear unloading action. When done, choose **4. Park unloaded truck** from the bay menu. It sends the truck to a fixed slot near the saved waiting area; you may give this order on foot near your original stopped lead. Wait until it is physically parked and held before admitting the next truck.
5. Repeat Admit, native Unload and Park for each truck. Do not move the waiting-area anchor after parking begins. After the last truck parks, enter your original lead and choose **Depart / return convoy**. This releases the whole original roster and reconnects its order; drive forward safely, then lead it home. Resume does not complete an active bay maneuver.

If an approach or parking route fails, stop and read the reason. **Options / recovery → Cancel bay** is a recovery path; it retains Hold, so Resume is a separate action. Cancel is not the normal completion step. Parking preflight checks slopes, vehicle occupancy and bay clearance, not every static obstacle.

## Controls, voices and driver behavior

The main wheel has convoy commands and a single **Units** submenu; numbered drivers do not crowd the main wheel. Hover explanations describe eligibility. Options includes Finish / Hold, debug and recovery. **Finish / Hold** requests a seated stop and an end call; it does not dismiss the drivers or unload cargo.

Generated voices are the default. Start, Resume, saving a bay, physical bay readiness, parking, physical parking completion, departure and Finish have varied acknowledgements. Confirmed damaging hits can report under fire. Spacing advice and actual range warnings share a 90-second cooldown, require sustained pressure and rotate wording; repeated acceleration alone does not rearm an episode. Original recordings remain selectable in the server profile.

Healthy owned transport drivers retain their seats during combat behavior. Death, incapacitation, a destroyed truck, possession, dismissal and ended ownership retain native safety behavior. This is not invulnerability or a promise to drive a disabled vehicle.

Use the debug HUD to compare actual speed, brake/throttle, native path, gap and the cruise request. A speed request is a ceiling, not proof that the AI is accelerating. Native steering and braking can still cause delays.

For the next player test, check all five Start, ordinary stops stay still, native rear supplies appear, every admitted truck parks, the final whole-convoy departure works, and voices vary. Keep enemy-fire testing separate from the cargo sequence so a damaged truck does not obscure an unloading failure. See [current validation](convoy-validation-current.md) for measured results and limits.
