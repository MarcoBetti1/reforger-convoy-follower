# Reforger local test workspace

Local tools and notes for repeatable Arma Reforger mod research, scenario tests, and addon validation. The older MCP helper remains available for inspecting local files and documentation.

## Current focus

This repository contains the Reforger test harness and the [Convoy Follower addon](addons/ConvoyFollower/README.md). The harness supports a Workbench-first loop: open a prepared Arland or Everon scene, validate and pack the addon, launch a local world, and inspect logs. An autonomous one-, two-, and three-truck gameplay test and video capture loop is under development; it is not yet evidence of reliable driving. Start with the [map-maker quickstart](docs/convoy-mapmaker-quickstart.md) for the addon, the [automated smoke test](docs/convoy-automated-smoke.md) for the test matrix, or the [harness handoff](docs/harness-handoff.md) for the tooling. The [operations playbook](docs/operations-playbook.md) records observed UI behavior.

## What is in this repo

- `docs/research-summary.md`: saved research and source links.
- `docs/roadmap.md`: phased plan and the current workflow prerequisite.
- `docs/operations-playbook.md`: tested paths, UI quirks, server setup, and evaluation protocol.
- `docs/harness-handoff.md`: short operating loop, verified paths, and remaining limits.
- `docs/raven-field-notes.md`: Raven's documented feature claims versus local gameplay observations and remaining test blockers.
- `docs/first-mod-workflow.md`: the recommended first learning path.
- `docs/agent-loop.md`: how to use Git plus an agent for mod edits.
- `src/`: a minimal `reforger-mcp` server.
- `reforger-agent.config.example.json`: example local config.

## Quick start

1. Create a local config file from `reforger-agent.config.example.json`.
2. Adjust the relative addon roots and add local documentation or sample roots if available. The personal config is excluded from Git.
3. Install the locked dependencies with `npm ci` on Node.js 24 or newer.
4. Run `npm run knowledge:prepare` to index the installed Workbench API docs and reuse existing official web snapshots.
5. Run `npm run typecheck`, `npm test`, and `npm run build`.
6. Start the MCP server with `npm run dev` during development or `npm start` after building.

For the current workflow, run `npm run cli -- doctor --mod <Workshop ID>` to check local paths and mod presence in configured roots. `npm run mod:fetch -- --help` shows one-command Workshop acquisition through the local dedicated server. `npm run conflict:host -- --help` shows the private Conflict host with a reusable cache and optional `--keep-open` for player handoffs. `npm run workbench -- --help` shows dry-run validation and packaging; `npm run workbench:open -- --help` opens an editor directly. `npm run server:config -- --help` generates a private scenario config. `npm run server:run -- --help` and `npm run client:run -- --help` show guarded, isolated launchers. They dry-run until `--execute` is given. `npm run logs:summary -- --log <console.log>` summarizes one run without treating startup as a verified delivery. Server downloads stay in the chosen server profile; they do not appear automatically in the client Workshop cache.

The addon source and radio assets are in this repository; generated audio provenance is documented in [the voice-pack notes](docs/convoy-generated-voice.md). Original unprocessed player recordings and model weights are not included. A public GitHub repository is source control, not a Workshop release or a claim that the current gameplay passes acceptance.

GitHub Actions runs the TypeScript typecheck, unit tests, and build. Enfusion validation, addon packaging, and driving tests require a Windows machine with Arma Reforger Tools installed; a green GitHub check does not cover those steps.

## Licensing

No project-wide reuse license has been selected for the harness, addon, or player-recorded audio. Public GitHub visibility allows viewing and forking under GitHub's terms; it does not by itself grant broader permission to reuse or redistribute this work. See [GitHub's licensing guidance](https://docs.github.com/en/repositories/managing-your-repositorys-settings-and-features/customizing-your-repository/licensing-a-repository). The generated-voice model and inference wrapper have separate upstream licenses described in the [voice-pack notes](docs/convoy-generated-voice.md). A project license and Workshop terms remain decisions for the owner before inviting reuse or publishing the addon to the Workshop.

## CLI

This repository also exposes a CLI so you can inspect the same data without a chat UI.

- `npm run cli -- project-info`
- `npm run cli -- prepare-knowledge`
- `npm run cli -- scan-mod`
- `npm run cli -- search-text Bradley --rootType all`
- `npm run cli -- list-packaged-resources "C:\\Path\\To\\data.pak"`
- `npm run cli -- search-packaged-data "C:\\Path\\To\\data.pak" "M2A2_Turret_OD.et"`
- `npm run cli -- doctor --mod 6A3A112604EF7286`
- `npm run mod:fetch -- --mod 6A3A112604EF7286=Raven --duration-seconds 30 --execute`
- `npm run workbench -- validate --project "C:\\Path\\To\\addon.gproj"`
- `npm run workbench:open -- --editor script` (dry run)
- `npm run logs:summary -- --log .cache/client/runs/<run>/logs/console.log`
- `npm run server:config -- --scenario "{C41618FD18E9D714}Missions/23_Campaign_Arland.conf" --mod 6A3A112604EF7286=Raven`
- `npm run conflict:host -- --mod '6A3A112604EF7286=Raven AI Commander'` (dry run)
- `npm run server:run -- --config .cache/local-server/config.json` (dry run)
- `npm run server:run -- --config .cache/local-server/config.json --duration-seconds 30 --execute`
- `npm run client:run -- --profile .cache/client/profiles/logistics` (dry run)
- `npm run client:run -- --profile .cache/client/profiles/vanilla-gm --world worlds/GameMaster/GM_Arland.ent --duration-seconds 600 --expect-game --execute` (direct local Game Master)

## Initial MCP tools

- `project_info`: show the active config and accessible roots.
- `scan_mod`: summarize likely mod files and script module folders.
- `list_files`: list files under configured roots.
- `search_text`: search text content in configured roots.
- `read_text_file`: read text files inside configured roots.
- `list_packaged_resources`: list asset-like paths discovered in `.pak` or `.rdb` files.
- `search_packaged_data`: search binary package content for printable text windows.

## Verified so far

- The dedicated server can download a selected Workshop mod from JSON and load Conflict Arland. The guarded runner verified a loopback-only listener and shut it down after a bounded run.
- `mod:fetch` completed a fresh Raven download into an isolated reusable cache, verified the packed addon, and shut down the local server.
- Workbench validated and packed the existing empty addon once the base game `addons` directory was supplied. The pack log reported success and produced `data.pak` in the ignored test output directory.
- The existing client has a large enabled mod set. A featured Game Master launch failed with a duplicate script declaration, so gameplay tests need an isolated enabled set or a clean profile.
- Launching the isolated client with the Game Master Arland `--world` path reached playable Game Master controls twice, bypassing the unreliable menu splash.
- The automatic client join to a loopback dedicated server timed out during authentication in both Raven and vanilla no-mods controls; direct loading of the server's local addon cache into a clean client did work.
- Raven loaded into the directly launched Game Master Arland world from the server's local addon cache; the Game Master UI responded. Raven's supply commands and delivery recovery remain gameplay tests.
