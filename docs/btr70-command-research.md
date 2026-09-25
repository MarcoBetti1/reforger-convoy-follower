# BTR-70 Command Variant Research

## Why this vehicle

I scanned the installed addon set first. There was no unpacked BMP-family vehicle available for direct derivative work, but RHS ships a BTR-70 asset set with placeable prefabs and content packs already installed locally.

That makes the BTR-70 the best candidate for a first "real" vehicle mod:

- It uses assets already present on the machine.
- It has a known placeable prefab in the RHS package.
- It supports a derivative learning workflow without replacing the original Workshop download.

## Vehicle references used

- FAS BTR-70 reference:
  - crew and passengers
  - armament
  - dimensions
  - amphibious capability
- Wikipedia BTR-70 variants page:
  - `BTR-70K` described as a command vehicle with additional radios, whip antennas, navigation equipment, and a portable generator
  - `BTR-70KShM` described as a mobile command post
  - `BTR-70MS` described as a communications support vehicle

## Mod design decision

The installed RHS package provides the BTR-70 hull, turret, and AFRF materials, but not editable source assets for a brand-new command-body mesh. Because of that, the first implementation focuses on what can be changed cleanly and safely:

- editor-visible prefab variants
- fixed hull numbers instead of randomized markings
- distinct command-role names and descriptions
- clean Workbench registration in a new addon folder

## Local prefab basis

RHS packaged data confirms these local references:

- `Prefabs/Vehicles/Wheeled/BTR70/BTR70.et`
- `Prefabs/Vehicles/Wheeled/BTR70/VehParts/Turret/BTR70_Turret_AFRF.et`
- `Assets/Vehicles/Wheeled/BTR70/Data/BTR_70_base_exterior_Body_AFRF.emat`

## Next upgrades

- Add a custom thumbnail and editor preview image.
- Attach visible communications props if suitable reusable assets are found.
- Add a small scenario or showcase composition that spawns the vehicle with HQ props.
- If publishing, add the correct RHS derivative-license language and external EULA link.
