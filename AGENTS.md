# TH02 reconstruction

## Highest priority: lightweight exact reconstruction and useful knowledge

These are the user's current workflow requirements. They override older
instructions that mandate broad CI, repeated preflights, document/source-pin
refreshes, per-checkpoint audits or extra workflow machinery. Preserve the
actual exactness, target identity, natural-source and state-isolation rules.

- Prioritize exact reconstruction and reusable knowledge. Use the shortest
  useful loop: inspect relevant state, analyze/edit, compile and compare the
  affected scope, retain useful learning, and create a coherent checkpoint.
- Use existing repository scripts and composable Bash. Add only the smallest
  change needed for an observed problem. Do not introduce a framework, schema,
  registry, state machine, generalized validator, test family or reporting
  process merely to make a local reconstruction task look more engineered.
- Documentation, knowledge and handoff edits require no compiler, analyzer,
  full CI, cold replay or receipt refresh. Inspect the diff. Check syntax only
  for machine-readable inputs actually changed.
- Source changes need the complete affected exact comparison, including its
  owned extents and relocations. Recheck known consumers of changed shared
  inputs. Tool changes need relevant positive/negative cases and a bounded
  real smoke. Run a broad suite only for an explicit request or a demonstrated
  impact that cannot be narrowed; state that reason first.
- Do not add tests or live freshness gates for prose hashes, prompt wording,
  progress totals, handoff source-SHA tables or ordinary source spelling.
  Bind exact evidence to executable inputs and relevant verification settings.
  Keep Git metadata as provenance. A document genuinely consumed by a build
  remains a build input. Never edit historical receipts to manufacture a pass.
- Plain Markdown is sufficient for knowledge: record the finding, applicable
  target/compiler/scope, evidence links, useful failed attempts and open
  questions. No mandatory JSON conversion, promotion workflow or test per note.
  Search relevant sections; do not read an entire large knowledge journal.
- Write documentation and knowledge in natural, fluent English. Lead with the
  concrete finding and connect the evidence and next action clearly. Remove
  filler, circular restatements, repeated conclusions and unnecessary jargon.
  Do not add defensive-claim boilerplate. State a real uncertainty or scope
  limit once when it changes a decision; do not repeat disclaimers about what
  a note does not prove, certify or authorize. Prefer a small useful note over
  an exhaustive report that is difficult to read.
- Keep handoff focused on current work, evidence references, blockers and the
  next useful action. Generate progress from canonical ledgers; avoid manual
  duplicate counts, recurring full reports and unrelated documentation chores.
- Preserve dirty/untracked work, targets, useful evidence, game-specific Wine
  prefixes and analyzer projects. Exactness still requires a complete strict
  comparison; knowledge and successful commands alone confer no exact credit.


Prioritize natural source recovery and exact native output. Read the short
`docs/RE_HANDOFF.md`, relevant parts of `docs/RE_WORKFLOW.md`,
`docs/ARCHITECTURE.md`, `docs/REC98_IMPORT.md` and `docs/FINDINGS.md`, and
the local `th02-re` skill. Inspect Git; run preflight before target-dependent
reconstruction or after target/runtime changes. Documentation and CI edits do
not need it.

- Use the Japanese targets pinned in `config/targets.toml` (`HUMAROKU/`
  OP.EXE, MAIN.EXE, MAINE.EXE, ZUN.COM), with `candidate-local-attested`
  provenance. Never substitute translated (`cn.hdi`), patched or
  reference-game binaries. Re-attest a Ghidra database before using it.
- ReC98 (nmlgc and contributors) already decompiled much of TH02. Its pinned
  revision is the starting point, credited in README and built from the
  reference checkout. ReC98 has no top-level license, so do not bulk-copy its
  source into this tree: only files we wrote or had to change live here.
  Every file in a link is checked per contribution against the original in
  `config/rec98_th02_units.csv`. All four programs are whole-file identical;
  ZUN's remaining assembly is listed in the handoff.
- Our source belongs in `src/op`, `src/main`, `src/maine`, `src/zun` and
  `src/shared` (used by more than one TH02 program); `units/` holds the
  files a link names. ReC98 infrastructure we changed (`Tupfile.lua`,
  `ReC98.inc`, `planar.h`, ...) goes in `compat/rec98/` at its upstream path.
  `config/rec98_overlay.toml` maps every file to its ReC98 path; edit
  `src/`, never only the private work tree. MAINE is the ending/staff roll.
- Preserve 16-bit widths, ABI, segments, packing, x87 and compile/link order.
  Exact requires complete raw bytes, literal ordered relocations, reviewed
  ownership/layout and reproducible native output. No padding, target-byte
  arrays or object patches to manufacture equality.
- Tests serve a specific reconstruction question. Add a small targeted probe
  only when it resolves an actual source, ABI or codegen doubt. Preserve
  original unchecked behavior; do not add defensive product fixes.
- Rebuild/check the affected scope. Reuse an unchanged cold build; do not
  repeat full cold replay every turn.
- Use direct local headless tools with DISPLAY/WAYLAND_DISPLAY empty; only
  DOSBox-X game runs use a private Xvfb display (`docs/DOS_BUILD.md`). Limit
  heavy work to one CPU at nice10; serialize Borland/Wine writers and preserve
  the local prefix. Keep private tools/targets/assets out of Git.
- Keep useful findings in source, ledgers and the concise handoff. Do not turn
  diagnostic counts or reference builds into source-exact progress. Build
  outputs are regenerable; historical archives, duplicate evidence and old
  trials need no permanent retention.

Finish with affected checks and `git diff --check`. `python3 scripts/ci.py`
is a fast, standard-library-only syntax/whitespace check, not a native gate.
Documentation-only changes need only diff inspection and whitespace checking.
Commit substantial progress with the prefix the user requires for your model
(`claude-opus-5.5-high: xxx` for Claude Opus 5.5, `gpt-6.1-sol: xxx` for GPT
agents).

## Shared Factory runtime

Follow [the shared tools instructions](docs/SHARED_TOOLS.md). Ghidra/JDK and
Wine executables are shared through Factory; repository scripts also work
without MCP. Use `scripts/shared_runtime.py` and the existing compiler/analysis
frontends. Keep this game's prefix, analyzer project, Java home/cache, target
and compiler profile isolated. Do not install duplicate tools or inherit
another game's `WINEPREFIX`.
