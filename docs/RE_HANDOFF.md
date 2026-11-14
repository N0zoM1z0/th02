# Handoff

## State (2026-11-14 bootstrap)

- Targets: four `HUMAROKU/` programs from the user's `jp.hdi`, pinned in
  `config/targets.toml`; `python3 scripts/verify_targets.py` passes.
- Toolchain: TC4J/TASM32/TLINK lock from TH03 in this repository's own Wine
  prefix; `python3 scripts/attest_toolchain.py` is READY.
- ReC98 `b6ba5b0a52` cold build reproduces TH03's recorded output hashes.
- Import ledger `config/rec98_th02_units.csv`: ZUN.COM is byte-identical, and
  every ReC98 C/C++ unit in OP/MAIN/MAINE is byte-equal with patch 0001.
  The remaining 19 differing bytes are in ReC98 assembly; see
  [REC98_IMPORT.md](REC98_IMPORT.md).
- Ghidra projects: `TH02-th02-op`, `TH02-th02-main`, `TH02-th02-maine`.
- `src/` is empty: nothing has been rewritten locally yet.

## Next useful actions

1. Decompile MAIN `sub_B3DA` and `sub_BCB1` (MAIN_01_TEXT) as C++ with inline
   `asm`. This removes 13 of the 19 differing bytes naturally and starts MAIN
   decompilation. Overlay mechanism: add the new TU to `src/main/` and a patch
   that removes the procedures from `th02_main.asm` and links the TU.
2. Split the ZUN-written procedures out of the asm files' `_TEXT` in the ledger
   (by public symbol), so the progress numbers stop counting them as library.
3. Continue MAIN decompilation from `th02_main.asm` (58,286 game CODE bytes).
4. Investigate the original header and relocation order for whole-product
   identity.

## Commands

```sh
python3 scripts/verify_targets.py
python3 scripts/attest_toolchain.py
python3 scripts/build_th02.py --run-id NAME [--write-ledger]
python3 scripts/progress.py
python3 scripts/ghidra.py th02-main check
```
