# Handoff

## State

- OP.EXE, MAIN.EXE and MAINE.EXE program images are byte-identical to the
  originals, with equal relocation sets. ZUN.COM is byte-identical as a file.
  Only EXE headers and relocation *order* still differ (whole-product work).
- Work lives on branch `th02-recon` of the ReC98 worktree
  `.analysis/work/rec98` and is exported as `patches/rec98/*.patch`
  (`git format-patch b6ba5b0a52..th02-recon`). `scripts/build_th02.py`
  applies the series to a cold `git archive` and rebuilds everything.

Decompiled beyond ReC98 so far (all byte-exact):
- MAIN_01: stage init, post-continue reset and pause menu
  (`th02/main_01a.cpp`), and the stage loop (`th02/main_01b.cpp`). The rest of
  MAIN_01_TEXT continues in ASM segment `main_01c_TEXT`.
- BOSS_5_TEXT: the complete Mima (stage 5 boss) block, 23 functions
  (`th02/main/boss/b4.cpp`, TU `th02/boss_4.cpp`).
- MAIN_03__TEXT tail: Marisa (stage 4 boss) from `marisa_1B19D` through
  `marisa_end`, 30 functions (`th02/main/boss/marisa.cpp`, TU
  `th02/marisa.cpp`, `-zCMAIN_03__TEXT -G -a2`). Still ASM before it:
  `marisa_init` (blocked by a `nopcall` with arguments), `marisa_1AA60`,
  `1AB35`, `1AC7B`, `1AD80`, `1AE98`, `1B025`. Keep the TU start at an odd
  address (now `0x1B19D`) or the `-a2` switch-table pad in `marisa_update()`
  moves; `1AB35` and `1AC7B` are the next odd starting points.
- MAIN_04 circle helpers (`th02/main_04.cpp`), MAIN_06 score savers
  (`regist_m.cpp`).

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
- The inline `nop / push cs / call near` stand-in only works for calls
  without stack arguments: TCC merges the cleanup of consecutive cdecl calls
  (`mima_init()` → `sub_13ABB()` has one `add sp, 6` for two calls). To
  decompile such callers exactly, the `main_03` group has to become one
  physical segment again, i.e. split `th02_main.asm` into modules interleaved
  with the C++ objects, all using the same code segment name.
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
- `!(x & 1)` loads through AL; `(x & 1) == 0` tests memory directly.
- A ternary into a register variable goes through AX; separate `if`/`else`
  stores (`mov si, imm`) mean `if`/`else`.
- An I/O delay `jmp short $+2` is `__emit__(0xEB, 0x00)`; BASM has no `$`,
  and an inline jump to a C label on the next line makes TCC add a jump.
- Locals that inline `asm` reads (`mov ax, gdc_start`) stay in memory.
- Nested `if`s in an inlined function leave a `jmp short $+2` (see
  `playperf_raise_over_time()`).
- `char far *` mangles as `nc` in the large model. ZUN's pascal C symbols
  (`TEXT_WIPE`, `DEMOPLAY`) are `extern "C" pascal` in C++.
- `STAGE_TITLES` and similar tables hold near pointers: declare them
  `near`. `[bx+1EDAh]` is master.lib's `super_patsize[]`.
- Byte constant subtraction (`x - 0x40`) compiles to `add al, -0x40`.
- A ternary assigning to a byte local shares one store (`mov [v], al`); the
  original's separate stores mean `if`/`else`.
- TCC's cross-jumping merges identical statement tails; write the natural
  duplicated code first. A jump into the middle of another branch needs `goto`
  (`mima_183D0`).

## Next useful actions

1. Continue backward through BOSS_5_TEXT (`sub_17A55` and earlier). The TU
   before the Mima code ends at 0x17A7F; `sub_17A55` keeps its loop counter
   in AX, so check whether it is C at all.
2. main_03__TEXT starts with `mima_init`, `mima_19C1D`, `mima_19C8D`,
   `mima_end` (a separate TU after `skill_calculate()`), then midboss 4 and
   the rest of Marisa listed above.
3. The rest of MAIN_01 (`_main`, `sub_B2AB`, `main_01c_TEXT` onward).
4. MAIN_05 (`sub_1C3DF`, stage data loader) needs data labels for its raw
   DGROUP addresses (`scripts/th02_datamap.py`).
5. Whole-product identity: header size and relocation order.
