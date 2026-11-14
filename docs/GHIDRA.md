# Headless Ghidra

`config/analysis_toolchain.toml` pins Ghidra 12.1.3 and Temurin 21.0.12.1+1.
`.tools/ghidra` and `.tools/jdk` select the Factory shared installations.

```sh
python3 scripts/attest_analysis_toolchain.py
python3 scripts/ghidra.py th02-main import --max-cpu 1
python3 scripts/ghidra.py th02-main check
```

Projects exist for `th02-op`, `th02-main` and `th02-maine` in
`ghidra-project/TH02-*.gpr`. `ZUN.COM` is not an MZ image; analyze its
payloads from the ReC98 build outputs instead. Imports refuse existing
projects. Exports and receipts live below `.analysis/ghidra`.

The MZ loader uses load segment 0x1000 and applies relocation words. Full
file bytes, header/load mapping, relocations, entry and samples are checked
independently. Automatic function boundaries are provisional.

`scripts/factory_ghidra.py [--artifact ID] check|decompile|query` is the
read-only Factory wrapper.
