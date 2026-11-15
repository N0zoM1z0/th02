# 東方封魔録 ～ the Story of Eastern Wonderland

Agent workflow: follow the highest-priority requirements in [AGENTS.md](AGENTS.md). Use affected exact checks and searchable Markdown knowledge; ordinary notes do not require full CI or replay.

<p align="center">
  <img src="resources/progress.svg" alt="TH02 authored byte exact reconstruction progress">
</p>

This repository reconstructs the original Japanese PC-98 **Touhou 2: the
Story of Eastern Wonderland** (`OP.EXE`, `MAIN.EXE`, `MAINE.EXE`, `ZUN.COM`).

[Progress](docs/PROGRESS.md) · [Handoff](docs/RE_HANDOFF.md) · [ReC98 import](docs/REC98_IMPORT.md) · [Workflow](docs/RE_WORKFLOW.md) · [Architecture](docs/ARCHITECTURE.md)

## State

All four files (`OP.EXE`, `MAIN.EXE`, `MAINE.EXE`, `ZUN.COM`) rebuild
byte-identical to the originals from a cold build of the pinned
[ReC98](https://github.com/nmlgc/ReC98) revision plus this repository's
source in `src/`. ReC98 had already decompiled most of TH02 outside MAIN's
gameplay; this project decompiled the rest of MAIN, including the stage and
boss code, and moved ZUN's remaining data into C++.

All of ZUN's compiled code and data is C/C++. Three routines stay as ZUN's
own assembly because TCC cannot produce them: `pointnum_put()`,
`shots_hittest()` and TH02's modified master.lib `pfopen()`, each a
word-aligned TASM module (see [the handoff](docs/RE_HANDOFF.md)). master.lib
and the Borland runtime are linked as libraries.

Details are in [the import notes](docs/REC98_IMPORT.md).

## Build

Private inputs (original programs, Borland tools, Ghidra projects) are not in
Git. With them in place:

```sh
python3 scripts/import_targets.py --retain-runtime-image /path/to/jp.hdi   # extracts and verifies HUMAROKU/
python3 scripts/attest_toolchain.py
python3 scripts/build_th02.py --run-id NAME --write-ledger
python3 scripts/progress.py
```

To play the rebuilt game on the original platform (PC-98 under DOSBox-X), see
[DOS_BUILD.md](docs/DOS_BUILD.md). See also [TOOLCHAIN.md](docs/TOOLCHAIN.md)
and [GHIDRA.md](docs/GHIDRA.md).

## Credits

**ZUN** ([上海アリス幻樂団 / Team Shanghai Alice](https://www16.big.or.jp/~zun/))
created Touhou Project and the original TH02 game, characters, artwork and music.

- **[ReC98](https://github.com/nmlgc/ReC98)**, by **nmlgc and contributors**:
  the TH02 decompilation this project starts from, plus PC-98 research,
  hardware definitions and the DOS build pipeline. ReC98 is used from a pinned
  checkout and is not redistributed here.
- **[master.lib](https://github.com/nmlgc/ReC98/tree/master/libs/master.lib)**:
  the PC-98 system library linked into TH02.
- **KAJA / PMD**: the PC-98 music driver shipped with the game.
- Workflow and tooling follow our [TH03](https://github.com/N0zoM1z0/th03)
  and [TH04](https://github.com/N0zoM1z0/th04) reconstructions.
- **[Ghidra](https://github.com/NationalSecurityAgency/ghidra)**,
  **[Wine](https://www.winehq.org/)**, **[mtools](https://www.gnu.org/software/mtools/)**
  and ReC98's **[MS-DOS Player](https://github.com/nmlgc/msdos-player)** build:
  analysis, DOS toolchain execution and disk-image extraction.

Project-authored code and documentation use the [MIT License](LICENSE).
Files under `src/` and `compat/rec98/` that started as ReC98 files keep
ReC98's credit and are offered for upstream use; ReC98 itself is not
redistributed here.
