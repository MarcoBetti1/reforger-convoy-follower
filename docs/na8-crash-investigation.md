# NA8 W.C.S client crash investigation

Observed on Windows with Arma Reforger 1.8.0.13 (engine 192142). This is an investigation record, not an attribution to a particular mod.

## Confirmed native crash signature

Six sessions connected to `[NA8] W.C.S. Realism NATO/RUS New Everon AAS` ended with `ENGINE (F): Crashed` and a minidump:

| Local time (CDT) | Session folder | Inventory activity near crash |
| --- | --- | --- |
| 2026-09-22 21:59:58 | `logs_2026-09-22_21-39-26` | Inventory closed at 21:59:53. |
| 2026-09-23 18:45:42 | `logs_2026-09-23_17-24-41` | Last logged inventory close at 18:43:20. |
| 2026-09-23 20:15:02 | `logs_2026-09-23_18-46-08` | Inventory closed at 20:14:53. |
| 2026-09-23 22:36:20 | `logs_2026-09-23_21-55-12` | Last logged inventory close at 22:21:40. |
| 2026-09-24 22:01:57 | `logs_2026-09-24_19-42-50` | W.C.S inventory opened at 22:01:44 and again at 22:01:55. The player recalls moving bandages or epinephrine, but cannot identify the exact item or transfer. |
| 2026-09-24 22:07:06 | `logs_2026-09-24_22-02-31` | Player opened the map. `MapMenu` loaded at 22:07:02; the native crash followed about four seconds later. No inventory interaction occurred near this crash. |

The first five `crash.log` files say `Access violation. Illegal read ... at 0x18`; the sixth says illegal read at `0xFFFFFFFFFFFFFFFF`. Parsing each minidump's exception and module streams shows exception code `0xC0000005` at RVA `0x8197CA` in `ArmaReforgerSteam.exe`. The absolute instruction address changes with process loading; the relative address is identical. This is strong evidence of the same native failure path despite differing input actions and target addresses. It does not identify the function or the data that caused the invalid read. The local game installation has no matching PDB or native source.

The 1.8.0.13 executable bytes at RVA `0x8197CA` are `49 8B 50 18`, an x64 `mov rdx, [r8+0x18]` instruction. The preceding `4C 8B 40 10` loads `r8` from `[rax+0x10]`. In the first five minidump contexts, `r8` is zero and the recorded read address is `0x18`, confirming an immediate null-pointer dereference in this native routine. The sixth context still points to the same instruction but records `r8 = 0x8080808080808080` while its exception record gives read address `0xFFFFFFFFFFFFFFFF`; those disagree, so do not infer a particular corruption mechanism from that dump. Without game symbols, the routine's function or object type remains unknown. Adding a null check at this instruction would only mask the symptom unless the invalid pointer's origin is identified.

The September 24 crash was preceded by `UI/layouts/Menus/Inventory/WCS_InventoryMain.layout` loading and `OverlayWidget Frame0 has invalid slot GridWidgetSlot ... expected: LayoutSlot`. The same session opened inventory 36 times and logged 102 `Frame0` slot errors, mostly without a crash. That error should be reported, but it is not proven to cause the access violation. The September 24 log also contains 53 `RHS_SwitchNearbyIRLights` script null-pointer exceptions at 21:33, roughly 29 minutes before the native crash; timing does not establish a connection.

The 22:07 map crash loaded `MapMenu.layout` and logged a `ToolMenu.layout` unknown `name` keyword plus a mouse-icon mask error. Map opens in the preceding session produced the mask error without a crash. The second crash weakens the inventory-specific hypothesis; neither UI warning should be treated as the root cause from timing alone.

No bandage, epinephrine, or storage-transfer error is logged between the 22:01:44 inventory open and the 22:01:57 crash. The console contains earlier storage warnings for saline bags, but those occurred minutes or hours before the crash and cannot be tied to this player's action.

## Evidence and routing

The original session folders are under `%USERPROFILE%\Documents\My Games\ArmaReforger\logs`. The September 22 folder was no longer present in that directory when checked after the September 24 crash; its logs and dump were preserved earlier in a private desktop archive named `ArmaReforger-NA8-crash-evidence-2026-09-24.zip`.

Ask the W.C.S NA8 maintainers to correlate the crashes with their server and mod state. Ask Bohemia Interactive to symbolize the dump instruction at build 1.8.0.13, RVA `0x8197CA`, because the faulting instruction is in the game executable. A code fix recommendation should wait for a symbolized native stack or a reproducible server/mod condition. The downloaded W.C.S addons on this machine are packaged `data.pak` files, not the development source.

On recurrence, record the exact action, local time, and any crash-reporter text. Preserve the newest `logs_*` folder and minidump. The September 24 Crash Reporter screenshots showed only the generic report form; the specific exceptions were in `crash.log` and the dumps.
