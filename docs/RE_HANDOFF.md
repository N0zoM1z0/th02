# Handoff

## State

- OP.EXE, MAIN.EXE, MAINE.EXE and ZUN.COM rebuild identical to the originals
  as whole files (cold build `session6-cleanup1`), and the rebuilt game boots
  and plays in DOSBox-X (`docs/DOS_BUILD.md`).
- Our source lives in `src/{op,main,maine,shared,zun}`; the ReC98
  infrastructure we changed is in `compat/rec98/`. `config/rec98_overlay.toml`
  maps each file to its ReC98 path, and builds copy them over a `git archive`
  of the pinned ReC98. Everything else is ReC98 unchanged.
- All of ZUN's compiled code and data is C/C++. ZUN's own assembly:
  `pointnum_put()` (`src/main/pointnum/num_put_m.asm`), `shots_hittest()`
  (`src/main/hittest.asm`), TH02's modified master.lib `pfopen()`
  (`src/shared/pf.asm`) and ZUN.COM's ZUNINIT (`src/zun/zuninit.asm`, also
  TH03's `-2`). The first three cannot come from TCC; see
  [FINDINGS](FINDINGS.md#zuns-assembly-modules). `th02_{op,main,maine}.asm`
  hold only master.lib and the segment declarations.
- ZUN.COM's ONGCHK is KAJA's code, assembled from `src/zun/ongchk.asm`
  (recovered in our TH03 reconstruction); TH03-TH05 still use ReC98's binary.
- Shared TUs compile once for all three EXEs, so TH02-only data had to go
  into TH02-only owners (`src/shared/units/`, `keydelay.cpp`, `pf.asm`).

## Loop

```sh
# edit src/ (a new file also goes into config/rec98_overlay.toml), then:
python3 scripts/th02_quick.py main [--asm] [th02/foo.cpp ...]   # ~3 s, ReC98 paths
python3 scripts/build_th02.py --run-id NAME --write-ledger && python3 scripts/progress.py
```

`th02_quick.py` copies `src/` into `.analysis/work/rec98`, compiles the named
TUs, relinks from `obj/th02/<product>.@l` and compares the program image. A
new object goes into that `.@l` file and `compat/rec98/Tupfile.lua`.
`th02_sbs.py main START LEN` diffs one function's disassembly. Compiler and
linker behavior is collected in [FINDINGS](FINDINGS.md).

## Next useful actions

1. Optional runtime check of MAINE (ending and staff roll), which the DOSBox-X
   runs have not reached: an invincible test build of MAIN as in TH04, or a
   manual playthrough of a prepared image.
2. Optional: a TH04-style build that links from our own manifests instead of
   ReC98's `Tupfile.lua`.
