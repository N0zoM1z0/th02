# Headless Borland toolchain

`config/toolchain.toml` and `probes/toolchain/` are the TH03 lock, unchanged:
Turbo C++ 4.0J (4.02), TASM32 5.0, TLINK 6.10 and ReC98 MS-DOS Player P0281.

The bootstrap copied TH03's canonical unpacked `installed/tc40j` and
`installed/tasm50` trees into `.analysis/toolchain/installed`, created a fresh
`.analysis/toolchain/wineprefix` through the shared Wine launcher, and placed
`C:\TC4` and `C:\TASM50` inside it. Every surface hash and both probe rounds
are re-checked by:

```sh
python3 scripts/attest_toolchain.py
```

The acquisition media (`tc40j-media`, TASM archive) are bootstrap-only and are
not present; the attestation reports them as such. On an empty machine, clone
ReC98 and run `bash scripts/bootstrap_toolchain.sh`, which downloads and
verifies them.

```sh
git clone https://github.com/nmlgc/ReC98.git _reference/ReC98
git -C _reference/ReC98 checkout --detach b6ba5b0a529edbb31efdf8c0e939263804f8ee47
PATH="$PWD/.tools/factory-bin:$PATH" nice -n 10 python3 scripts/cold_build_rec98.py --run-id UNIQUE
python3 scripts/survey_rec98_outputs.py .analysis/builds/rec98-b6ba5b0a52/UNIQUE/source --gate exact --compact
```

The host `wine64` hash differs from the TH03 recording because Wine now comes
from the Factory store; it is an observation, not a required surface.
