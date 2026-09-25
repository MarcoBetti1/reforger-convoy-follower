# Ural SPG-9 Field Conversion Research

## Why this vehicle

The first tracked-IFV idea was rejected because a true BMP-style mod would need a hull, suspension, turret, and weapon stack that do not exist cleanly in the locally installed source assets.

I then narrowed the problem to a vehicle that could be built with:

- an installed truck chassis
- an installed heavy weapon
- a real historical pattern for field mounting

That led to the SPG-9 on a utility or cargo vehicle.

## Sources and interpretation

- Weaponsystems.net describes the SPG-9 as a recoilless gun that can be pedestal-mounted on vehicles and shows a 4x4 utility-vehicle mount.
- GlobalSecurity's recoilless-weapons overview specifically calls out the SPG-9 as suitable for vehicle mounts and notes it has been seen pintle-mounted on a UAZ-469.
- Washington Institute reporting on Syrian rebel convoys documents both SPG-9 use and truck-mounted heavy weapons in improvised mobile fire-support roles.

Inference:

- A truck-mounted SPG-9 is historically grounded.
- A Ural-4320 cargo-truck conversion is plausible and consistent with the kinds of field-expedient weapon carriers seen in Soviet-aligned and post-Soviet contexts.

## Local asset basis

Installed assets confirm all required building blocks are already present:

- RHS `Ural4320_transport.et`
- RHS `Tripod_SPG9.et`
- RHS `Rocket_SPG_1rnd_Base.et`

The missing piece was the host cargo module. For that, I used the truck-bed mounting pattern demonstrated by the installed `WCS_ZU-23-2` addon and replaced the anti-aircraft turret with the RHS SPG-9 tripod.

## Design choice

I intentionally named the final vehicle a `field conversion`, not a formal military designation. That keeps the historical claim honest. The mod is representing a real style of improvised or unit-level mounting rather than inventing an official catalog vehicle that the source material does not support directly.

## Source links

- Weaponsystems.net SPG-9: https://weaponsystems.net/system/646-73mm%2BSPG-9
- GlobalSecurity recoilless-weapons overview: https://www.globalsecurity.org/military/world/artillery-rcl.htm
- GlobalSecurity SPG-9 overview: https://www.globalsecurity.org/military/library/policy/army/accp/in0546/lsn2.htm
- Washington Institute, "Syria's Rebels Gain Heavy Weapons": https://www.washingtoninstitute.org/policy-analysis/syrias-rebels-gain-heavy-weapons
- RHS Ural-4320 reference: https://www.rhsmods.org/w/ural4320
- RHS: Status Quo Reforger docs: https://docs.rhsmods.org/rhs-status-quo-user-documentation/arma-reforger/rhs-status-quo
- WCS ZU-23-2 Workshop page: https://reforger.armaplatform.com/workshop/63294BA2D9F0339B
