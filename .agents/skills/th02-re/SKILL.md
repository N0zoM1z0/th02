---
name: th02-re
description: Recover one bounded TH02 PC-98 source unit and verify exact native output.
---

Read AGENTS.md, the handoff, `docs/REC98_IMPORT.md` and the relevant
architecture/workflow sections. The pinned Japanese targets are the truth;
ReC98 code and Ghidra boundaries are hypotheses until the ledger shows the
affected contributions byte-equal.

Recover natural source: preserve ABI, 16-bit widths, segments and TU/link
order. TCC inline `asm` and TASM encode some instructions differently; match
the original's encoder instead of patching bytes.

Edit our files under `src/` (mapped to ReC98 paths by
`config/rec98_overlay.toml`). Check one product with `scripts/th02_quick.py`,
then cold-build with `scripts/build_th02.py --write-ledger` and check every
affected contribution in the ledger. Record compiler findings in
`docs/FINDINGS.md`, run `git diff --check`, and commit.
Use direct headless tools, one CPU at nice10, serialized Borland writers.
