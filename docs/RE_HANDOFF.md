# Handoff

## State

- OP.EXE, MAIN.EXE, MAINE.EXE and ZUN.COM are identical to the originals as
  whole files, from a cold build of the patch series (`session5-cold15`).
- Work lives on branch `th02-recon` of the ReC98 worktree
  `.analysis/work/rec98` and is exported as `patches/rec98/*.patch`
  (`git format-patch b6ba5b0a52..th02-recon`). `scripts/build_th02.py`
  applies the series to a cold `git archive` and rebuilds everything.

All ZUN C code in MAIN is C++. What remains assembly:
- `pointnum_put()` (`th02/main/pointnum/num_put_m.asm`) and `pfopen()`
  (`th02/main/pf.asm`): hand-written by ZUN, like their TH04/TH05 siblings.
- `sub_3F8A()` is C++ at the end of `dots.cpp`. Its near call into SHARED
  uses `spark.cpp`'s inline-asm form with a `near` `vector2()` declaration.
- `shots_hittest()` (`th02/main/hittest.asm`): ZUN's own assembly module. The
  target lists its relocations in ascending (TASM) order, and its code
  segment is word-aligned (a `00` pad after stage 3), which TCC never emits.
  The final `damage` reload follows the 16-bit `add` into `score_delta`.
- `pointnum_put()` and `shots_hittest()` are both preceded by a `00` pad,
  i.e. word-aligned modules, which TCC never emits.
- All of ZUN's initialized data, string literals included, is defined in C++
  (or in ZUN's own asm modules) in all three EXEs; the `_DATA` of
  `th02_*.asm` is master.lib only. Shared TUs are compiled once for every
  binary, so shared data had to sit where it fits all of them
  (`keydelay.cpp`, `frmdely1`'s neighbours `maine022.cpp`/`main02_2.cpp`,
  pfopen's data in `pf.asm`). All of ZUN's uninitialized variables are
  C++ too; `.data?` in `th02_*.asm` is master.lib only. The blocks shared
  with TH01/TH03-05 got TH02-only owner TUs: `th02/vplanset.cpp` (the four
  plane pointers as separate variables; `planar.h` makes `VRAM_PLANE`
  `&VRAM_PLANE_B` for TH02, since array subscripts change codegen),
  `th02/pi_load.cpp`, `th02/input_rs.cpp` (`key_det`, then the `snd_*` flags;
  in C they would become communals), `th02/snd_ld_2.cpp`, and pfint21's BSS
  in `pf.asm`. Unlabeled gaps were real data:
  oversized arrays (the bomb TU's 50-element tables) or unused variables
  (`map_unused`, `tile_ring_unused`); TCC aligns neither byte-sized enums
  nor character arrays under `-a2`, but does align structs.

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
- `shots_hittest()`'s body is TCC `-2` output (inverted short jumps over near
  JMPs; `_BL = shot_level; _BH = 0; _BX += _BX;` table reads, `_BX = y_offset`
  coordinate reads, `-= 0x80u`, a do-while), but the module is ZUN's
  assembly (see "State"). An inline `asm` statement in TCC code also makes
  TCC reload registers afterwards, which is how the `damage` reload arises.
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
  only paragraph-aligned, unlike TLINK's `0x3E` layout (UNLZEXE writes this
  layout). `Pipeline/exemin.c` rewrites TLINK's output that way after linking;
  image, relocation order, `SS:SP` and allocation fields stay TLINK's.
- Moving data into TUs works tail-first: the last ZUN block in
  `th02_main.asm`'s data belongs to the latest-linked TU still declaring it
  `extern`, so no new modules are needed.
- TCC's data order within a TU: variables in definition order (including
  the anonymous data of local aggregate initializers, at their function),
  then the literal pool in source order. `const` one-dimensional character
  arrays join the literal pool (`POWER_RESET_FOR`, the stones' ring orders).
  Under `-a2`, word-element data and *structs* are word-aligned, so ZUN's
  byte tables were plain arrays, not `Array<>` (dialog faces).
- ZUN's `"m\0\0"`: `stage_fn_set_ext()` copies three bytes.
- Original TUs found this way: `main_01a.cpp` plus the stage loop up to
  `DemoPlay()`; the random number ring; `cfg_load()`; the game over menu;
  `GameExecl()` with `sub_C5B0()`; `player_invalidate()`; bombanim with the
  bomb renderers; Mima with `skill_calculate()` and her intro/defeat code.

## Next useful actions


1. None for ZUN's three hand-written routines. A C++ wrapper of
   `pointnum_put()`'s asm fails twice: TCC with `-k-` omits the frame, but it
   saves SI/DI itself *before* the inline `enter 0, 0` (also with `-r-`), and
   its code segments are byte-aligned (ACBP 28, while the target's module is
   ACBP 48 after `pointnum.cpp` ends at the odd offset `0B16:1845`). Matching
   either would need byte-encoded instructions or a `codestring` pad.
   `shots_hittest()` (after `stage3.cpp` ends at `0FAC:2D7B`) and TH02's
   modified master.lib `pfopen()` (after pfint21 ends at `0000:3BFF`) show the
   same word-aligned module pad; TCC emits byte-aligned code segments with
   or without `-a2`, never `enter 0, 0`, and drops unused locals.
