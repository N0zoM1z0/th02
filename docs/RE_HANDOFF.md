# Handoff

## State

- OP.EXE, MAIN.EXE and MAINE.EXE program images are byte-identical to the
  originals, with equal relocation sets. ZUN.COM is byte-identical as a file.
  MAIN's EXE header now matches too; only the relocation table *order*
  differs (see "Relocation order" below).
- Work lives on branch `th02-recon` of the ReC98 worktree
  `.analysis/work/rec98` and is exported as `patches/rec98/*.patch`
  (`git format-patch b6ba5b0a52..th02-recon`). `scripts/build_th02.py`
  applies the series to a cold `git archive` and rebuilds everything.

All ZUN C code in MAIN is now C++ (byte-exact, quick loop; last cold run
`session4-cold1` predates the latest patches). What remains assembly:
- `pointnum_put()` (`th02/main/pointnum/num_put_m.asm`) and `pfopen()`
  (`th02/main/pf.asm`): hand-written by ZUN, like their TH04/TH05 siblings.
- `sub_3F8A()` (`th02/main/dots_upd.asm`): its call to `vector2()` in SHARED
  is `push cs / call near` in the original, which TLINK 6.10 only produces
  within one physical segment. A natural far call gives an extra relocation.
- `shots_hittest()` (`th02/main/hittest.asm`): the C++ version differs only
  in the final reload of `damage`.
- master.lib code and all data stay in `th02_main.asm`.

Recent structure changes: main_01 and main_03 are each one code segment
(`-zCmain_01_TEXT`, `-zCmain_03_TEXT`), so all intra-group calls are natural.
The original has no unshortened far call inside either group, which makes
the merge safe. `Tupfile.lua`'s MAIN list now follows `obj/th02/main.@l`.

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
- Without `-G`, TCC opens frames with locals using `enter`; `-G` gives
  `push bp / mov bp, sp / sub sp`. `-G` also cleans one cdecl argument with
  `add sp, 2` instead of `pop cx` (Mima's defeat code is therefore not `-G`).
- `-a2` does not align functions. Inter-function `nop`s in `_TEXT` follow
  ReC98's `#pragma codestring "\x90"` (`spark.cpp`, `dots.cpp`).
- `p = dots` into a register pointer is `mov si, offset`; the original's
  `mov ax, offset / mov si, ax` is `_AX = FP_OFF(dots); p = (T near *)_AX;`.
- Byte pairs written by one 16-bit store (`shot_t` flag/age,
  patnum/from_option) need an explicit 16-bit write.
- Struct copies from an initialized constant (`bomb_reimu_c()` speeds,
  `stage_loop()` scroll lines) are `T local = EXTERN_CONST;`.
- Two statements `x = a; x += b;` into a register variable give
  `mov si, a / add si, b`; one expression goes through AX.
- A comparison assigned to a byte (`dir = (a < b);`) is computed in AX; a
  ternary `(a < b) ? 1 : 0` uses AL.
- `if(...) goto label;` and `if(...) continue;` both end the condition at a
  jump label, which resets TCC's register and ES tracking. A nested `if`
  keeps it.
- Under `-a2`, a TU's switch tables are word-aligned relative to the TU
  start, so a misaligned table means the TU starts at another function
  (`stage2_lightning_update()` before Meira, the null callback before the
  enemy code).
- An `if`/`else if` chain with copy-pasted bodies compiles to one linear
  compare sequence after cross-jumping; a `switch` with as many sparse cases
  uses a binary search.
- TCC's cross-jumping merges identical statement tails; write the natural
  duplicated code first. A jump into the middle of another branch needs `goto`
  (`mima_183D0`).

## Relocation order

TLINK writes relocations module by module in link order; within a TCC object,
each LEDATA chunk's fixups come out highest address first. The target's
table therefore shows ZUN's original object order and TU boundaries when
each target relocation is mapped to its module in `obj/th02/main.map`:
- After `c0`, the first object is one TU spanning `main_01a.cpp` and most of
  `main_01b.cpp`, followed by its own `_DATA` (the far pointers at
  `DGROUP:00FE`-`0110`, pause/quit strings).
- Then `_TEXT` code from `GRAPH_PI_FREE` to `VSYNC_START`, the SHARED objects
  `keydelay` … `vector`, `pfint21`/`PFEND`, then `exit` … `pi_put`.
- The next TU starts at the end of `main_01b.cpp` and continues with
  `main_01c.cpp`; `hud_ovrl.cpp`'s MAIN_03 code sits *inside* it, so ZUN's
  TUs switched code segments.
- Further interleavings: `explode`/`bullet` (MAIN_03) between `shot` and
  `item`; `bombanim` and `player_b` are one TU; `main_04` before `player`;
  `stage3`, `hittest`, `main_05`, `pillar`, `dialog`, `rika`, `meira`,
  `regist_m`, one `_DATA` relocation, `sigma`, `enemy`, then `mima` and
  `boss_5` as one TU, then `marisa`, then the C runtime.
Reproducing this needs `th02_main.asm`'s relocation-bearing code and data
moved into modules at those link positions, while keeping each segment's
contributor order.

## Next useful actions

1. Relocation order: reorder `main.@l` and split `th02_main.asm` as above,
   checking the order with the relocation-to-module mapping.
2. `shots_hittest()`'s final reload.
3. Cold rebuild (`build_th02.py --write-ledger`) and progress regeneration.
