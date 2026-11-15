# Scripts

| Script | Purpose |
| --- | --- |
| `import_targets.py IMAGE` | Extract `HUMAROKU/` (and `--include-all-games-smoke`) from an HDI or RAR; verify pins |
| `verify_targets.py` | Re-check private targets against `config/targets.toml` |
| `attest_toolchain.py` | Hash every Borland/runner surface and run compile/assemble/link probes |
| `cold_build_rec98.py` | Unmodified pinned ReC98 cold build (calibration) |
| `survey_rec98_outputs.py SRC --gate exact` | Strict raw comparison of all 20 outputs |
| `build_th02.py` | Pinned ReC98 + our `src/`/`compat/rec98` overlay, build, write ledger |
| `rec98_overlay.py [TREE]` | Copy `src/` and `compat/rec98/` into a ReC98 tree (default: the work tree) |
| `prepare_game_hdi.py` | Install a build's programs into a copy of the original game image |
| `run_game_hdi.py DIR` | Boot a prepared image in DOSBox-X (PC-98) under Xvfb; keys and screenshots |
| `rec98_th02_ledger.py SRC` | Per-contribution comparison from TLINK detailed maps |
| `progress.py` | `docs/PROGRESS.md` and `resources/progress.svg` from the ledger |
| `ghidra.py ARTIFACT import\|check` | Headless Ghidra project import / attestation |
| `factory_ghidra.py` | Read-only Factory query/decompile wrapper |
| `th02_quick.py PRODUCT [--asm] [TU ...]` | Sync `src/`, compile, relink and compare one product in the work tree |
| `th02_sbs.py PRODUCT START LEN` | Diff one function's target vs built disassembly (needs capstone) |
| `inspect_omf.py` | OMF object inspection |
| `shared_runtime.py` | Factory shared Ghidra/JDK/Wine launcher |
| `preflight.py` | Environment, toolchain and target checks |
| `ci.py` | Syntax and whitespace only |
