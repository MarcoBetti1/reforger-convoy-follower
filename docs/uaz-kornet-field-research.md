# UAZ Kornet Field Mod Research

## Why the Ural was dropped

The Ural SPG-9 proof of concept proved the agent could inspect packed assets and assemble a new addon, but it failed as a finished vehicle:

- the cargo-bed entity spawned separately from the truck
- the SPG-9 tripod did not integrate cleanly with the vehicle physics chain
- the result was visibly wrong in Workbench

That made it the wrong platform for the next iteration.

## Why the UAZ-469 was chosen

The installed dependency set already contains a much stronger donor:

- `WCS_Vehicles` exposes working armed `UAZ-469` weapon-slot patterns
- `WCS_Armaments` contains a complete `UAZ469_Kornet` implementation internally
- the `UAZ-469` is a much better physical match for a light missile carrier than a `Ural-4320`

## Historical basis

The mounted-Kornet-on-light-vehicle concept is historically grounded. Missilery describes self-propelled `Kornet-T` systems as being mounted on `UAZ-3151` vehicles, and the US Army's Worldwide Equipment Guide describes the Kornet family as fielded in several alternate mounts including jeeps. From those sources, a jeep-mounted Kornet is a defensible direction rather than an invented fantasy build.

Inference:

- a `UAZ-469` Kornet carrier is historically plausible
- building it as a local editable derivative of installed assets is a better learning target than trying to invent a BMP from scratch

## Technical basis

Package inspection showed three key pieces already exist in your installed mods:

- `Prefabs/Vehicles/Wheeled/UAZ469/UAZ469_base.et`
- `Prefabs/Vehicles/Wheeled/UAZ469/VehParts/WeapMounts/UAZ469_weapon_mount_Kornet.et`
- `Prefabs/Weapons/Magazines/9M133_Kornet/Ammo_Magazine_Kornet.et`

The local addon therefore only needs to:

- inherit from the unarmed `UAZ469_base`
- attach the Kornet weapon mount in the `Weapon` slot
- expose the result in editor placement

## Source links

- Missilery Kornet page: http://en.missilery.info/missile/kornet
- Worldwide Equipment Guide, Kornet entry: https://odin.tradoc.army.mil/WEG/Asset/AT-14_SPRIGGAN_9P163M-1_KORNET-EM_ATGM_Launcher%2C_Russian
- WCS Armaments Workshop page: https://reforger.armaplatform.com/workshop/629B2BA37EFFD577
- WCS Vehicles Workshop page: https://reforger.armaplatform.com/workshop/615A80027C5C9170
