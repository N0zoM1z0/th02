# Handoff

## State

- OP.EXE and MAINE.EXE program images are byte-identical to the originals,
  with equal relocation sets. ZUN.COM is byte-identical as a file.
- MAIN.EXE differs in 13 bytes, all in `sub_B3DA` and `sub_BCB1`
  (MAIN_01_TEXT). These are C functions with inline `asm`; TCC's built-in
  assembler encodes `xor al, al`, `mov al, ah` and `or ah, ch` with the other
  direction bit than TASM. Decompiling them as C++ is the natural fix.
- EXE headers and relocation *order* still differ (whole-product work).
- Work lives on branch `th02-recon` of the ReC98 worktree
  `.analysis/work/rec98` and is exported as `patches/rec98/*.patch`
  (`git format-patch b6ba5b0a52..th02-recon`). `scripts/build_th02.py`
  applies the series to a cold `git archive` and rebuilds everything.

Decompiled so far beyond ReC98: MAIN_06 (`hiscore_init`, game/extra clear
savers, in `regist_m.cpp`), MAIN_04 (`circle_dots_put`,
`circle_tiles_invalidate`, `th02/main_04.cpp`) and the complete Mima boss
block at the end of BOSS_5_TEXT (23 functions, `th02/main/boss/b4.cpp`, TU
`th02/boss_4.cpp`).

## Loop

```sh
# edit .analysis/work/rec98, then:
python3 scripts/th02_quick.py main [--asm] [th02/foo.cpp ...]   # ~3 s
git -C .analysis/work/rec98 commit -am "..."
rm patches/rec98/*.patch && git -C .analysis/work/rec98 format-patch -q \
  --no-signature --zero-commit -N b6ba5b0a529edbb31efdf8c0e939263804f8ee47..th02-recon \
  -o "$PWD/patches/rec98/"
python3 scripts/build_th02.py --run-id NAME --write-ledger && python3 scripts/progress.py
```

`th02_quick.py` relinks from `obj/th02/<product>.@l`; add new objects there
and to `Tupfile.lua`. `th02_sbs.py main START LEN` diffs one function's
disassembly; `th02_asmedit.py` removes/exports procedures and data labels.

## Compiler findings (TCC 4.02, verified on MAIN)

- Within one TU, code with locals in `push bp/mov bp,sp/sub sp` form is `-G`.
  `skill_calculate()` is not `-G` (`imul ax, ax, 3`), so it is a different TU
  from the Mima code before it.
- `-a2` word-aligns switch jump tables relative to the TU start (the `00`
  before a table). Without it, the table is unaligned.
- TLINK only turns a cross-TU far call into `nop / push cs / call near` when
  the target symbol has no group in its PUBDEF and its segment frame equals the
  caller's, which holds for MAIN_03_TEXT. ReC98 splits each original code
  segment into several grouped segments, so C++ calls into a later segment
  of the group spell the sequence out with inline `asm` until the group is one
  segment again (see `mima_update()` → `mima_end()`).
- Two adjacent constant arguments are pushed as one `push imm32` only when both
  parameters are `int`. `bullets_add_16x16()` therefore takes `int patnum`.
- Register variables: TCC's automatic choice depends on use. A result that is
  assigned in the condition (`if((x = f()) != 0)`) stays in memory. Copying
  parameters into `register` locals (`mima_17A7F`) changes TCC's state for
  later functions in the TU (a later `switch` loaded through AX otherwise).
- `x++` / `if((x & 3) == 0)` as separate statements gives `inc mem` /
  `test mem`; `++x & 3` goes through AL.
- `(y << 6) + (y << 4)` is ZUN's spelled-out `y * 80`; `* ROW_SIZE` gives
  `imul` even under `-G`.
- Byte constant subtraction (`x - 0x40`) compiles to `add al, -0x40`.
- A ternary assigning to a byte local shares one store (`mov [v], al`); the
  original's separate stores mean `if`/`else`.
- TCC's cross-jumping merges identical statement tails; write the natural
  duplicated code first. A jump into the middle of another branch needs `goto`
  (`mima_183D0`).

## Next useful actions

1. MAIN_01_TEXT: decompile `sub_B3DA` (stage init, 1460 B) and `sub_BCB1`
   (714 B). This removes the last differing program bytes. They need a split
   of MAIN_01_TEXT into grouped segments around a new TU.
2. Continue backward through BOSS_5_TEXT (`sub_17A55` and earlier). The TU
   before the Mima code ends at 0x17A7F; `sub_17A55` keeps its loop counter
   in AX, so check whether it is C at all.
3. main_03__TEXT starts with `mima_init`, `mima_19C1D`, `mima_19C8D`,
   `mima_end` (a separate TU after `skill_calculate()`).
4. MAIN_05 (`sub_1C3DF`, stage data loader) needs data labels for its raw
   DGROUP addresses (`scripts/th02_datamap.py`).
5. Whole-product identity: header size and relocation order.
