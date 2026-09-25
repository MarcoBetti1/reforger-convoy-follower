# Pickup Truck Mod Research

## Goal

Build a pickup-truck family for Arma Reforger that starts with a usable default truck and later branches into:

- a supply/cargo pickup
- an MG technical
- an anti-air pickup or light gun truck

The practical constraints are:

- We want a truck smaller than the Ural.
- We want something visually close to a Toyota-style single-cab pickup if possible.
- We want a path that still works with agent-driven editing, Git, and a clean local development loop.

## Current Local Context

Installed locally right now:

- `RHS-StatusQuo`
- `RHS-ContentPack01`
- `RHS-ContentPack02`
- `WCS_Armaments`
- `WCS_ZU-23-2`

Not installed locally right now:

- `Gatoma Pickup Truck`
- `TRex Pickup Truck HAZARD`
- `THE TOYO`
- `Toyota 1983 REVAMPED`
- `Cold War M880`

That matters because we already have weapon donors for later variants, but not yet a pickup donor we can build on.

## Workshop Donor Candidates

### 1. THE TOYO

- Workshop ID: `651B162EF31AA2BA`
- Link: https://reforger.armaplatform.com/workshop/651B162EF31AA2BA
- Summary: "a collection of toyota hilux based technicals"
- License: `APL`
- Last modified: `2025-08-25`
- Game version listed: `1.4.0.48`
- Dependencies: none shown on the workshop page

Assessment:

- Best visual fit for the kind of truck we want.
- License is workable for derivative mods.
- Risk is currency: it is not listed against `1.6.0.119`, so we should expect some forward-port or cleanup work.

### 2. TRex Pickup Truck HAZARD

- Workshop ID: `65C694F214BF091D`
- Link: https://reforger.armaplatform.com/workshop/65C694F214BF091D-TRexPickupTruckHAZARD
- License: `APL`
- Last modified: `2026-01-21`
- Game version listed: `1.6.0.108`
- Dependency: `MFD Framework`

Assessment:

- Strongest current donor from a maintenance standpoint.
- License is workable.
- Likely the safest "it should load and drive now" donor.
- Drawback: it is a modern pickup, not a classic Hilux-style single-cab technical by default.

### 3. Cold War M880

- Workshop ID: `66C2AAEACA445073`
- Link: https://reforger.armaplatform.com/workshop/66C2AAEACA445073-ColdWarM880
- License: `APL`
- Last modified: `2025-11-20`
- Game version listed: `1.6.0.68`

Assessment:

- Good fallback if we prioritize a 2-seat, cargo-bed, utility truck that is current and legally clean.
- Less "Toyota technical" and more U.S. military utility truck.
- Better fit for a heavier gun truck or ZU-23-style experiment than for a light insurgent-style pickup.

### 4. Toyota 1983 REVAMPED

- Workshop ID: `5EE162A9B07BAAA7`
- Link: https://reforger.armaplatform.com/workshop/5EE162A9B07BAAA7
- License: `APL`
- Last modified: `2023-12-02`
- Game version listed: `1.0.0.59`
- Notes from page: six civilian colour combinations, cargo version, armored versions, M2-mounted variants

Assessment:

- Very close to the target theme.
- License is usable.
- But it is stale. The listed target game version is old enough that I would treat it as a porting project, not a drop-in donor.

### 5. Gatoma Pickup Truck

- Workshop ID: `5D0D623BB27CC015`
- Link: https://reforger.armaplatform.com/workshop/5D0D623BB27CC015-GatomaPickupTruck
- License: `APL-ND`
- Last modified: `2026-01-12`
- Game version listed: `1.6.0.95`
- Notes from page: updated for Reforger `1.6`, tuned transmission, suspension, hitboxes, braking, repair

Assessment:

- Technically one of the strongest and most actively maintained pickup donors in the ecosystem.
- Not legally clean for derivative work because of the `No Derivatives` license.
- Good reference or private proof-of-concept base only if permission is obtained from the author.

### 6. Integrity - Coyota Offroad

- Workshop ID: `68F0286BB2B783E4`
- Link: https://reforger.armaplatform.com/workshop/68F0286BB2B783E4-Integrity-CoyotaOffroad
- License: custom no-derivatives / no-redistribution license
- Last modified: `2026-03-28`
- Game version listed: `1.6.0.119`
- Notes from page: explicitly Hilux-like, includes civilian transport/supply variants and optional weapon integrations

Assessment:

- Probably the closest thing to the exact visual target on the current workshop.
- Not suitable as a derivative donor without explicit permission.

## Vehicle Family Variants Found In The Community

Relevant existing examples:

- `SAS_ZU23-2_Gatoma` mounts a `ZU-23-2` onto the Gatoma pickup.
- `PhaToma 4x4` mounts a CIWS-style system onto Gatoma.
- `Gatoma Mortar Truck` adds mortar and supplies to the base pickup.
- `GatomaPickupTruckToUS` and `Add Gatoma to US` show depot integration patterns.

These are useful because they prove that the ecosystem already treats pickup trucks as a valid host for:

- rear-bed weapons
- supply-capable utility variants
- faction/depot registration

## Real-World Reference

### Pickup Shape And Scale

Current Toyota Hilux single-cab references are close to the shape we want even if we do not reproduce one exact model year.

Sources show:

- single-cab capacity of `2` in some work/cab-chassis trims
- overall length around `5265 mm` to `5380 mm`
- width around `1800 mm` to `1855 mm`
- wheelbase `3085 mm`
- Toyota Australia spec sheet snippet indicates single-cab cab-chassis length around `1880 mm`, which is effectively a `~6.17 ft` tray length

That means a "2-seat cab + ~6-foot bed" design target is historically and mechanically reasonable.

### Technical Use

The real-world technical pattern is well established:

- technicals are commonly open-backed 4x4 pickup trucks
- the most common bases are Toyota Hilux and Toyota Land Cruiser
- common armaments include heavy machine guns, recoilless rifles, anti-tank missiles, and anti-aircraft autocannons

### What Is Historically Plausible On A Pickup

Most plausible for a light pickup:

- `DShK`
- `NSV/KORD`-style HMG
- `M2 Browning`
- `SPG-9`
- light ATGM launchers

Plausible but mechanically harsher on a true light pickup:

- `ZU-23-2`
- `ZPU` family

The historical record absolutely includes ZU-23-2 systems mounted on vehicles and technicals, but in practice they are more comfortable on larger trucks or stronger commercial chassis than on a small 6-foot-bed pickup. If we want the AA variant to feel defensible rather than just dramatic, an HMG with high elevation is the safer first AA variant on a true pickup. A ZU-23 variant is still possible, but it should be treated as the "heavier technical" branch.

## Recommendation

### Recommended Primary Path

1. Download `THE TOYO` first.
2. If it loads and indexes on Reforger `1.6.0.119`, use it as the visual donor for the family.
3. If it does not load cleanly, fall back to `TRex Pickup Truck HAZARD` for the first full working implementation.

Reason:

- `THE TOYO` is the best balance of theme and usable licensing.
- `TRex Pickup Truck HAZARD` is the best balance of maintenance and legality.

### Recommended Variant Order

1. `Default Utility Pickup`
   - single-cab
   - supply storage in bed
   - bed passenger seating or bench stubs
   - 3-4 paint schemes
2. `MG Technical`
   - bed-mounted HMG
   - most likely donor: existing HMG tripod or vehicle HMG mount from installed mods
3. `AA Technical`
   - first-choice practical version: elevated HMG / DShK-style AA truck
   - second-choice dramatic version: ZU-23 on a heavier donor or reinforced pickup

### Recommendation On The AA Variant

For the first working AA branch, I recommend:

- do **not** start with `ZU-23-2` on the light pickup
- start with an HMG or DShK-style AA truck

Reason:

- It is more believable on a small pickup.
- It avoids a bed-weight/recoil mismatch in the first iteration.
- It is more likely to work cleanly in Reforger without major seat, physics, and mount hacks.

If we want a ZU-23 truck after that, we should consider making it a separate "heavy technical" based on a larger donor like `Cold War M880` rather than forcing it into the first pickup family.

## Personal Creative Direction

Suggested family theme:

- `Coyota Utility`
- `Coyota Raider`
- `Coyota Skyguard`

Suggested paint schemes:

- faded sand
- chalk white
- oxidized blue
- green farm truck

Suggested default truck details:

- rear jerrycan
- spare tire in bed
- tied-down supply crates
- field antenna option
- simple steel wheel look

Suggested MG variant details:

- ring or pintle mount
- ammo crates in bed corners
- one stripped-down variant and one shielded variant

Suggested AA variant details:

- gunshield only if needed
- bed reinforcement plates
- visibly reduced cargo capacity

## Implementation Decision

If we want the most realistic and productive next step:

- try `THE TOYO` first
- keep `TRex Pickup Truck HAZARD` as the fallback donor
- use installed `WCS_ZU-23-2` only as a local reference/dependency check, not as the first design anchor

## Sources

- Gatoma Pickup Truck: https://reforger.armaplatform.com/workshop/5D0D623BB27CC015-GatomaPickupTruck
- PhaToma 4x4: https://reforger.armaplatform.com/workshop/667B178339E77ACE
- SAS_ZU23-2_Gatoma: https://reforger.armaplatform.com/workshop/6555564E88081310
- Sample Mod - New Car: https://reforger.armaplatform.com/workshop/5614E482BF83E310-SampleMod-NewCar
- Sample Mod - New Car changelog: https://reforger.armaplatform.com/workshop/5614E482BF83E310/changelog
- Toyota 1983 REVAMPED: https://reforger.armaplatform.com/workshop/5EE162A9B07BAAA7
- THE TOYO: https://reforger.armaplatform.com/workshop/651B162EF31AA2BA
- TRex Pickup Truck HAZARD: https://reforger.armaplatform.com/workshop/65C694F214BF091D-TRexPickupTruckHAZARD
- Cold War M880: https://reforger.armaplatform.com/workshop/66C2AAEACA445073-ColdWarM880
- Integrity - Coyota Offroad: https://reforger.armaplatform.com/workshop/68F0286BB2B783E4-Integrity-CoyotaOffroad
- Technical (vehicle): https://en.wikipedia.org/wiki/Technical_%28vehicle%29
- ZU-23-2: https://en.wikipedia.org/wiki/ZU-23-2
- Toyota Hilux single-cab example dimensions: https://taylortoyota.dealer.toyota.com.au/test-drive/hilux/workmate/4x2/2025/2L48300DYFC20040
- Toyota Hilux spec references: https://www.toyota.com.au/hilux/specs
- Toyota Hilux Kuwait reference: https://www.toyota.com.kw/en/explore-range/hilux/
