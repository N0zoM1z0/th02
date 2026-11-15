# Architecture

TH02 (東方封魔録 ～ the Story of Eastern Wonderland) ships four programs in
`HUMAROKU/`:

| Artifact | Role | Format | Size |
| --- | --- | --- | ---: |
| `OP.EXE` | title, menus, music room | MZ | 63,736 |
| `MAIN.EXE` | gameplay | MZ | 134,758 |
| `MAINE.EXE` | ending and staff roll | MZ | 59,106 |
| `ZUN.COM` | launcher: ZUNGEN-packed ONGCHK, ZUNINIT, ZUN_RES, ZUNSOFT | COM | 19,984 |

Unlike TH03, `ZUN.COM` is a plain COM image. Its four payloads are built from
source (ONGCHK and ZUNINIT in assembly, ZUN_RES and ZUNSOFT in C++; ZUNSOFT is
shared with TH01) and stored by ReC98's `zungen` packer behind its launcher
stub (`Pipeline/zun_stub.asm`).

## Identity

`config/targets.toml` pins the four extracted programs. The carrier is the
user's Anex86 `jp.hdi` (FAT12 at byte 38912, label `TOUHOU`). The HDI hash is
not identity, because the game writes `HUUHI.DAT`/`HUUMA.CFG` into it; each
import records it in `.analysis/target-import.json`. The TH02 hashes are equal
to those pinned independently by the TH03 and TH04 repositories from
`zun.hdi`. TH01/03/04/05 programs from the same image are optional calibration
targets and add nothing to TH02 progress.

## Toolchain and runtime

`config/toolchain.toml` is the TH03/TH04 lock: Turbo C++ 4.0J (TCC 4.02),
TASM32 5.0, TLINK 6.10, run under ReC98's MS-DOS Player P0281 through Wine.
`.analysis/toolchain/wineprefix` is this game's own prefix; Ghidra, JDK and
Wine executables come from the Factory shared store (`docs/SHARED_TOOLS.md`).

## ReC98 relationship

ReC98 builds all four TH02 programs from a mix of decompiled C/C++ and
remaining assembly. The pinned revision is checked out in `_reference/ReC98`
(ignored) and materialized with `git archive` for every cold build. The import
ledger `config/rec98_th02_units.csv` records each translation unit of each
product link, whether it is C/C++ or ASM, and the CODE bytes it contributes
in the linker map. Details and the verification result are in
`docs/REC98_IMPORT.md`.

All reconstruction work is a patch series on the pinned ReC98 tree
(`patches/rec98/`), so it stays reviewable against ReC98 and can go upstream.
`src/` holds no product source yet.
