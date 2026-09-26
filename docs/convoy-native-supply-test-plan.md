# Native supply transfer: verified action test and delivery plan

**September 26 execution update:** `native-cargo-action-v6` completed the static native-action check. Source/truck quantities moved **1,800/0 → 1,700/100 → 1,800/0**, using the original native load/unload actions and exact storage/cargo containers. Both cancellations were followed by at least three seconds of unchanged counts. Pack: `CC39384DD7C1A9DFFE64EC180A781FD03B0C5AFBBC313F8736F0A31E10A7EE4C`. This is transfer proof at one location; physical delivery and ordinary input remain separate gates. The [current validation record](convoy-validation-current.md) retains errors and closure limits.

The verified regression world is `Worlds/Tests/ConvoyFollower_Arland_StaticCargoAction.ent`; its `CF_NativeCargoActionProbeComponent` is opt-in and does not run in ordinary worlds. It uses the real controlled character, closes the native Game Master editor, explicitly enables scenario supplies, waits for physical/native-action readiness, and dispatches native actions. It does not write resource quantities or vehicle controls. The fixture deliberately unloads into its original storage; the full delivery plan below still needs travel to a distinct destination.

Three setup lessons resolved the failures: Game Master disables SUPPLIES by default; a spawned truck needs observed settling before the static baseline; and native `CanBePerformed` creates a spatial subscription whose queue updates on a later frame. Unload's first `NoStorage`/empty queue resolved after **33.1 ms** in v6. The bounded readiness wait preserves all eligibility and conservation checks. Earlier v1–v5 failures remain in their run directories.

## Installed contract

Sources were selectively read from `D:/SteamLibrary/steamapps/common/Arma Reforger/addons/data/data007.pak` (1,994,525,893 bytes; modified `2026-09-07T19:53:00.629Z`). No installed files were extracted, changed, or executed. The package index SHA-256 was `EBD2BB2B7E1561CD8F2D773155870E8D77C03B2112D4BE8601639CC982183B3E`.

Exact mounted resource paths:

| Source | Relevant contract |
| --- | --- |
| `Prefabs/Vehicles/Wheeled/M923A1/M923A1_transport.et` | Standard transport registers its cargo child and that child's actions. |
| `Prefabs/Vehicles/Wheeled/M923A1/VehParts/M923A1_cargo.et` | Cargo resource component at lines 713–751; native load/unload actions at 933/943 use `door_rear`. The cargo compartment manager has `m_bBlockSuppliesIfOccupied=1` at 700. Discover the action's actual owner; do not assume the truck root owns the cargo resources. |
| `Configs/Resources/Supplies/Actions/Supplies_Vehicle_Load.conf` and `Supplies_Vehicle_Unload.conf` | Both configure continuous actions, duration `-1`, and a maximum 100 supplies per execution. They are shown when the user is outside a vehicle. |
| `scripts/Game/Sandbox/Resources/UserActions/SCR_ResourceContainerVehicleLoadAction.c` | Checks the game-mode master, resolves the controlled user's player/controller inventory component, computes available supply and capacity, then calls native consumption and generation. Loading eligibility also checks the cargo occupancy restriction. |
| `scripts/Game/Sandbox/Resources/UserActions/SCR_ResourceContainerVehicleUnloadAction.c` | Corresponding master-only unload path, with capacity/availability checks. Both actions invoke the native player resource-interaction hooks. |
| `scripts/Game/Interactions/SCR_InteractionHandlerComponent.c` | Lines 163–216 set the action context, call `DoStartObjectAction`, and tick `DoPerformContinuousObjectAction`; line 254 cancels an interrupted action. Eligibility is refreshed while using the action. |
| `scripts/Game/generated/Character/ChimeraCharacter.c` | Public native action-dispatch methods at lines 45–52. |
| `scripts/Game/generated/UserAction/BaseActionsManagerComponent.c` and `BaseUserAction.c` | `GetActionsList`, `GetContext`, action owner/manager/context, `CanBeShown`, `CanBePerformed`, and actual visibility range. |
| `scripts/Game/Sandbox/Resources/SCR_ResourceComponent.c` | Runtime `GetContainer(SUPPLIES)`, container list, consumer and generator accessors. |
| `scripts/Game/Sandbox/Resources/Container/SCR_ResourceContainer.c` | `GetResourceValue`/`GetMaxResourceValue`; lines 229–265 apply resource type and resource rights. |
| `scripts/Game/Sandbox/Resources/Consumer/SCR_ResourceConsumer.c` | Lines 894–924 consume actual queued resources and replicate the change. |
| `scripts/Game/Sandbox/Resources/Generator/SCR_ResourceGenerator.c` | Lines 245–266 perform native generation/storage and replicate the change. |
| `scripts/Game/Sandbox/Resources/Encapsulator/SCR_ResourceEncapsulator.c` | Cargo child containers update their representative's aggregate values. Do not double-count representative plus children. |

The four exact queue configuration paths are:

- `Configs/Resources/Supplies/Consumers/Consumer_VehicleLoad.conf`: range 12 m, rights `ALL`, ignores itself.
- `Configs/Resources/Supplies/Generators/Generator_VehicleLoad.conf`: range 12 m, rights `SELF`, stores into the vehicle.
- `Configs/Resources/Supplies/Consumers/Consumer_VehicleUnload.conf`: range 12 m, rights `SELF`, consumes the vehicle's cargo.
- `Configs/Resources/Supplies/Generators/Generator_VehicleUnload.conf`: range 12 m, rights `ALL`, ignores itself; its queue targets `STORED` resources.

Twelve metres is the configured resource-search range, **not player interaction reach** or proof that a particular storage is eligible. Read the native queue membership and rear context at runtime. Container/interactor resource rights still apply. The cargo representative is faction restricted; the generic supply-storage prefab uses `ALL`. These actions contain no Convoy Follower session-owner permission check.

Selected source SHA-256 values: load action `D64D9397108552BDFD239685828828EE41EA0E4D107200D0B51602D772113344`; unload action `6813732DB7B5EE1B89A01D292AEC56A48C77C5321FE9D302EEAAD6DA72E5F0E2`; cargo prefab `125F56BB16E89036047BD8B3730130CEE2365C36F55319CA984D61291EFDB4F6`; interaction handler `4273C266067EB965A3E4269D53BE6E75CD20978EF8D1C0E7ACCC1D0BE52BD354`.

## Complete delivery fixture still to validate

1. **Bind the original objects.** Use one standard M923 transport, one source and one destination, with no competing accessible storage or unrelated resource consumers. The [Supply Day scene](convoy-supply-showcase.md) declares source 1,800/destination 0, but actual loading reach remains unverified. Record the live truck, action owner/cargo child, storage entities, resource types, capacities, player/controller and world. Require stable initial counts before starting. Initial world values are setup; no runtime resource setters are permitted during the measurement.
2. **Establish native eligibility.** Use the real local controlled player character on foot beside the actual rear context, alive and outside any seat transition. Enumerate the initialized native action objects and select the exact load action belonging to this truck. Require enabled manager, `CanBeShown` and `CanBePerformed`, appropriate interaction reach, and the expected storage in the native queue. The action's eligibility code obtains the local player's resource-inventory replication ID, so an arbitrary AI or headless dedicated-server caller is not equivalent. Start with a standalone/listen-owner fixture.
3. **Invoke the native action dispatcher.** Bind the real context, call `DoStartObjectAction`, and tick `DoPerformContinuousObjectAction` with actual elapsed slices while rechecking identity and eligibility. This mirrors the stock interaction handler below its input-selection layer. Cancel on the first observed positive transfer or bounded timeout using `DoCancelObjectAction`. Do not call resource setters, replace the actions, or bypass rejection with a direct transfer helper.
4. **Verify loading.** Read exact source and destination containers through `GetContainer(EResourceType.SUPPLIES).GetResourceValue()`. For cargo, corroborate its representative with the native self-only `VEHICLE_LOAD` generator / `VEHICLE_UNLOAD` consumer aggregate. Enumerate contributing container owners so a nearby unrelated store cannot satisfy the check. Require source decrease = cargo increase > 0, destination unchanged, and total conserved. Check the actual capacity and partial-transfer amount; do not assume every execution must move 100.
5. **Transport and unload.** Drive the same loaded truck physically to the destination; retain identities and cargo counts during travel. After a real stable stop and rear-context eligibility, invoke the existing unload action through the same native dispatcher. Require cargo decrease = destination increase > 0, source unchanged, and total conserved. Observe unchanged counts after cancellation. Preserve physical travel evidence separately from transfer evidence.

These native resources may be virtual or encapsulated game cargo; a successful count check does not imply that loose inventory items were moved. Visual cargo appearance and the native displayed count are useful corroboration, while exact authoritative container deltas establish the transfer.

## Boundaries and failure reporting

- Predeclare the transfer amount and timeout, record each action start/cancel and each actual count change. A request or completed progress bar is not success.
- A missing player-inventory component, occupied cargo restriction, wrong queue, disabled resource type, absent capacity, or no delta is a fixture/action failure to diagnose. Do not manufacture a count or waive the native condition.
- Native `PerformAction` does consumption followed by generation and does not inspect both returned responses. Independently requiring conservation detects a partial failure; calling the action alone is insufficient.
- `SCR_ScenarioFrameworkResourceComponentActionTransfer` uses `SCR_ResourceSystemHelper.SimpleResourceTransfer`, and the scenario unload action can drop containers. Those are different paths and are not substitutes for this cargo-menu-action test.
- Preserve full console, terminal snapshot, source/package provenance, video and shutdown errors. Report native transfer, physical transport, ordinary input, and multiplayer as separate gates.
