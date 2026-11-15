# ReC98 compatibility boundary

ReC98 infrastructure that TH02 needed changed, at its upstream path:
`Tupfile.lua` (TH02's link lists, ONGCHK/ZUNINIT sources, the MAINE header
rewrite), `ReC98.inc` (`nopcall` for TLINK's header sizing), `planar.h`
(TH02's separate plane pointers) and `platform/array.hpp`.
`config/rec98_overlay.toml` copies them over the pinned tree. They are ReC98
files with local edits, not reconstructed TH02 source; product source lives in
`src/`.
