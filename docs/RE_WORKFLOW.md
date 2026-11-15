# TH02 reconstruction workflow

> Follow the **Highest priority** section in `AGENTS.md`: affected exact
> comparisons and useful Markdown knowledge, no ceremony.

1. Inspect Git and the [handoff](RE_HANDOFF.md). Run `scripts/preflight.py`
   after environment or target changes, not for documentation edits.
2. Pick a complete function or source question. The pinned Japanese target is
   the truth; ReC98, TH03/TH04 and Ghidra boundaries are hypotheses.
3. Preserve 16-bit widths, segments, near/far/Pascal/cdecl ABI, packing, x87,
   TU boundaries and compile/link order. Remember that TCC's inline assembler
   and TASM choose different encodings for the same instruction.
4. Edit source under `src/` (and `compat/rec98/`), never in the ReC98 tree.
   `scripts/rec98_overlay.py` copies it into the private work tree for
   `th02_quick.py`; `scripts/build_th02.py` materializes pinned ReC98, copies
   the same files over it and writes the per-contribution ledger. A change is
   exact only if every contribution it touches is byte-equal, the relocation
   set is unchanged and no other contribution regresses.
5. Record useful findings (in `docs/` or the source), run
   `git diff --check`, and commit substantial progress locally.

All tools are headless. Serialize Borland/Wine writers. Keep exports and
build runs under `.analysis`.
