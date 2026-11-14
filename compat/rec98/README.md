# ReC98 compatibility boundary

Product source under `src/` may need ReC98 headers before their declarations
are localized. Such a dependency goes through a forwarding file here, named
`compat/rec98/<upstream-path>`, containing only
`#include "<upstream-relative-path>"` (or `include <path>` for TASM `.inc`).
Do not copy declarations. Forwarders keep upstream provenance visible and are
not counted as reconstructed TH02 source. ReC98 itself is not vendored; see
`docs/REC98_IMPORT.md`.
