# Handoff

## State

- OP.EXE and MAIN.EXE are identical to the originals as whole files (header
  and relocation table included). ZUN.COM is identical as a file. MAINE.EXE
  has an identical program image and relocation table; only its header
  format differs (see "Linking" below). Verified by cold run
  `session5-cold2`.
- Work lives on branch `th02-recon` of the ReC98 worktree
  `.analysis/work/rec98` and is exported as `patches/rec98/*.patch`
  (`git format-patch b6ba5b0a52..th02-recon`). `scripts/build_th02.py`
  applies the series to a cold `git archive` and rebuilds everything.

All ZUN C code in MAIN is C++. What remains assembly:
- `pointnum_put()` (`th02/main/pointnum/num_put_m.asm`) and `pfopen()`
  (`th02/main/pf.asm`): hand-written by ZUN, like their TH04/TH05 siblings.
- `sub_3F8A()` (`th02/main/dots_upd.asm`): its call to `vector2()` in SHARED
  is `push cs / call near` in the original, which TLINK 6.10 only produces
  within one physical segment. A natural far call gives an extra relocation.
- `shots_hittest()` (`th02/main/hittest.asm`): the C++ version differs only
  in the final reload of `damage`.
- master.lib code, and the data of TUs that still declare it `extern`, stay
  in `th02_main.asm` / `th02/main_dtl.asm`. `main_01a.cpp` and
  `regist_m.cpp` define their own data; string literals are still extern.

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

## Linking

The whole-file match depends on reproducing ZUN's object order and TU
boundaries, which the target's relocation table records:
- TLINK writes relocations module by module in link order. Within a TCC
  object, each 0x400-byte LEDATA chunk's fixups come out highest address
  first; TASM objects list them in ascending order. Mapping every target
  relocation to its module in the `.map` file therefore shows where original
  TUs began. Chunk breaks closer than 0x400 bytes are TU boundaries.
- DGROUP data follows the same order, so a TU's data must be defined in that
  TU when its relocations or position require it (`main_01a.cpp`,
  `regist_m.cpp`). TCC moves `const` objects (and `const` one-dimensional
  character arrays into the literal pool) behind the non-const ones; ZUN's
  data was not `const`.
- `th02/main_seg.asm` only declares MAIN's segments and goes first, so that
  `main()`'s TU (with its data at `DGROUP:0090`) can precede
  `th02_main.asm` while code segment #1 stays word-aligned.
- TLINK sizes the header from the far fixups it sees before shortening them.
  In the original, master.lib's calls into other library objects were such
  far calls, so `nopcall` emits a real far call for TH02 (under `NOSMART`,
  which keeps TASM from shortening it). This fixed OP's header size.
- MAINE.EXE's original header starts the relocation table at `0x1C` and is
  only paragraph-aligned, without TLINK's `0x3E` layout: it was rewritten by
  another tool after linking. Everything after the header matches.
- Original TUs found this way: `main_01a.cpp` plus the stage loop up to
  `DemoPlay()`; the random number ring; `cfg_load()`; the game over menu;
  `GameExecl()` with `sub_C5B0()`; `player_invalidate()`; bombanim with the
  bomb renderers; Mima with `skill_calculate()` and her intro/defeat code.

## Next useful actions

1. `shots_hittest()`'s final reload; `sub_3F8A()` if a TLINK setup that
   shortens across segments turns up.
2. Move the remaining TU data and string literals out of `th02_main.asm`
   into their TUs, using the data order as the guide.
3. Identify the tool that rewrote MAINE.EXE's header.
