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
| `rec98_th02_ledger.py SRC` | Per-contribution comparison from TLINK detailed maps |
| `progress.py` | `docs/PROGRESS.md` and `resources/progress.svg` from the ledger |
| `ghidra.py ARTIFACT import\|check` | Headless Ghidra project import / attestation |
| `factory_ghidra.py` | Read-only Factory query/decompile wrapper |
| `inspect_omf.py`, `compare_artifacts.py` | OMF and MZ inspection helpers |
| `shared_runtime.py` | Factory shared Ghidra/JDK/Wine launcher |
| `preflight.py` | Environment, toolchain and target checks |
| `ci.py` | Syntax and whitespace only |
