# Handoff

## State

- OP.EXE, MAIN.EXE and MAINE.EXE program images are byte-identical to the
  originals, with equal relocation sets. ZUN.COM is byte-identical as a file.
  Only EXE headers and relocation *order* still differ (whole-product work).
- Work lives on branch `th02-recon` of the ReC98 worktree
  `.analysis/work/rec98` and is exported as `patches/rec98/*.patch`
  (`git format-patch b6ba5b0a52..th02-recon`). `scripts/build_th02.py`
  applies the series to a cold `git archive` and rebuilds everything.

Decompiled beyond ReC98 so far (all byte-exact; cold run `session4-cold1`):
- MAIN code group 1 is entirely C++ except ReC98's `pointnum_put()` include:
  `main()` and init, stage init/loop/pause, boss/BGM/demo callbacks, config
  loading, game over/continue, `GameExecl()`, player init
  (`th02/main_01a.cpp`, `main_01b.cpp`, `main_01c.cpp`), all shot code
  (`th02/main/player/shot.cpp`) and bombs (`th02/main/player/bombanim.cpp`).
- MAIN_03: the boss explosion/reset functions at the start of BULLET_TEXT
  (`th02/main/boss/explode.cpp`), the pillar objects before the dialog code
  (`th02/main/pillar.cpp`), the Mima block in BOSS_5_TEXT and Marisa from
  `marisa_1AB35` on (`th02/mima.cpp`, `th02/marisa.cpp`).
- MAIN_04 circle helpers, MAIN_05 stage data loader, MAIN_06 score savers,
  player controls in `player.cpp`.
- MAINE's game code is entirely C++.

Still assembly in MAIN: DIALOG_TEXT before the pillars (stage 3 midboss and
the Five Magic Stones, `sub_10E4F`/`sub_10E95`), `shots_hittest()` (own
module `th02/main/hittest.asm`, see below), BOSS_5_TEXT before Mima (Rika,
Meira, midbosses 1/2/x, Sigma), main_03__TEXT head (`mima_init` …
`marisa_1AA60`), and ZUN code inside `_TEXT`.

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
- A `nop / push cs / call near` does *not* prove a TU boundary: TCC emits a
  plain far call for a function that is only declared so far, even in the
  same TU, and TLINK shortens it. Only calls to functions already *defined*
  earlier in the TU become `push cs / call near` without the `nop`
  (`end.cpp` → `extra_unlock_animate()`). String-literal placement (TCC `-d`
  merging, `aAll_pi = ($ - 7)`) is the better TU-membership evidence.
- `resident_t::rank` and `::bgm_mode` are unsigned: comparisons against
  nonzero constants are bytewise. MAIN_05's loader also compares `rank`
  bytewise, unlike every other MAIN TU.
- TLINK turns a far call into `nop / push cs / call near` by itself when
  caller and callee share one physical segment. The three C++ units after
  `main()` use `-zCmain_01_TEXT` and get `demo_load()`, `GameExecl()` and
  `DemoPlay()` calls right, arguments included. Calls into segments that
  ReC98 still keeps separate need the inline-asm stand-in, which only works
  without stack arguments (TCC merges consecutive cdecl cleanups).
- To decompile a function in the middle of an assembly segment, move the
  assembly after it into its own module (`th02/main/hittest.asm`) and link
  it after the new C++ object; segment contributions follow object order.
  Such a module must not use `.model` (it adds an empty `<FILE>_TEXT`
  segment that shifts DGROUP).
- `shots_hittest()` was compiled with `-2` (inverted short jumps over near
  JMPs). Its C++ version (`/tmp` copy not kept; rebuild from the asm) matches
  with `_BL = shot_level; _BH = 0; _BX += _BX;` table reads, `_BX = y_offset`
  coordinate reads, `-= 0x80u`, and a do-while; only the final reload of
  `damage` before `return` was not reproduced.
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
- Code after `if(...) continue;` starts at a jump label, which resets TCC's
  register knowledge (`push si` instead of `push ax`, `sub_E2D9()`).
- Locals get stack slots in declaration order, first declared at the highest
  address; a block-scoped local comes after all function-level ones.
- `++p->age` and `p->age += 1` load through AL; `p->age++` is `inc mem`.
- Leaf code without calls keeps loop counters and copies in AX/CX/DX
  (`shots_clear()`, `sub_C5B0()`); a plain `int page = page_back * 2` local
  lands in CX/DX, while `register` changes the choice.
- TCC 4.02 rejects `-Oc`/`-Og` etc. in `#pragma option`.
- Byte pairs written by one 16-bit store (`shot_t` flag/age,
  patnum/from_option) need an explicit 16-bit write.
- Struct copies from an initialized constant (`bomb_reimu_c()` speeds,
  `stage_loop()` scroll lines) are `T local = EXTERN_CONST;`.
- TCC's cross-jumping merges identical statement tails; write the natural
  duplicated code first. A jump into the middle of another branch needs `goto`
  (`mima_183D0`).

## Next useful actions

1. DIALOG_TEXT backwards from `stones_12778`: `stones_update`, `stones_end`
   and `stones_init` are read; the trailing `db 0` before `shots_hittest`
   becomes word alignment of `hittest.asm`. `stones_init` passes an argument
   through a nopcall into BOSS_5 (`sub_13ABB`).
2. BOSS_5_TEXT backwards from `sub_17A55`; main_03__TEXT head.
3. Merge each original code segment group into one physical segment (split
   `th02_main.asm` into modules) to drop the remaining nopcall stand-ins.
4. Whole-product identity: header size and relocation order.
