# Short supply trip: Arland daylight

This scene brings the mod back to its intended use: recruit a driver, load a truck, lead it to a destination, unload and resume with the same driver. It is a development scene, not a completed demonstration or release acceptance.

- World: `Worlds/Showcase/ConvoyFollower_Arland_SupplyDay.ent`.
- Scenario header: `Missions/ConvoyFollower_Arland_SupplyDay.conf`.
- Three ordinary US Convoy Driver groups, three M923A1 supply trucks and a separate player truck.
- Native supply storage starts with 1,800 supplies near the first truck; an empty destination with capacity 3,000 is about 195 m east along the road.
- The scenario requests noon. A bare world launch may inherit the Game Master time.

There is no automatic recruitment, AI lead, hidden follower controller, scripted cargo transfer or forced seat possession in the scene. Driver behavior comes from the ordinary addon prefabs. Check [current validation](convoy-validation-current.md) for which backend is integrated and which behavior has actually run.

## First test

1. Spawn at **Convoy supply staging - daylight** and use the front driver/truck pair first.
2. Use the truck's native rear cargo interaction to load a nonzero amount. Record source and truck quantities.
3. Recruit and assign that driver, then drive the separate player truck east to the destination stack.
4. Stop beyond the stack so the follower's rear can approach it. Use **Hold all seated** and check that the truck remains still.
5. Unload through the native cargo menu. Verify that truck supplies decrease and destination supplies increase by the same amount.
6. Resume, drive on and confirm that membership and truck assignment survive.

After that works, repeat with two and three followers. Demonstrate command use and retained assignments as well as visible driving. Advanced pull-off and turning are separate follow-up cases.

## Placement and test boundary

The road and vehicle coordinates reuse the existing OpenRoad course. The storage stacks sit beside the lane at approximately `(1343, 3337)` and `(1542, 3346)` in XZ. Their shoulder heights, clearance and native loading reach are provisional until inspected in the live world. This scene passed five-configuration validation and packaging in `.cache/workbench-runs/convoy-trip-v2/`, pack `E3A0EB37EA06A12AFA7AAB20C23380E91261698DD0E723AA4C3E693D3C711420`; it has not yet been inspected or played. Do not call a vehicle arriving at a stack a delivery without actual supply transfer.

Ordinary desktop input is currently an unresolved environment boundary. Automated preset driving results do not establish player menu opening, clicking or restored driving controls.
