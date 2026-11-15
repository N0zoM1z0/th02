# DOS build and test run

This is the original-platform PC-98 build: the four programs from a cold
build, installed into a copy of the Japanese game image with its original
data, KAJA's PMD/MMD drivers and DOS.

```sh
python3 scripts/build_th02.py --run-id NAME
python3 scripts/prepare_game_hdi.py --build-run .analysis/builds/th02-b6ba5b0a52/NAME \
  --output-dir .analysis/runtime/NAME
xvfb-run -a -s "-screen 0 1024x768x24" python3 scripts/run_game_hdi.py \
  .analysis/runtime/NAME --name play --key 14:Return --key 16:Return \
  --key 18:Return --key 20:Return --key 22:Return --frame 15 --frame 26 --frame 55
```

`prepare_game_hdi.py` needs the image retained by
`import_targets.py --retain-runtime-image`. It checks that the image still holds
the pinned original programs, replaces them, reads them back and records
whether each one equals the original. `--original` prepares a baseline with
the original programs. The prepared `game.hdi` also runs on Windows with the
user's DOSBox-X and `th02_jp.conf` (change its `imgmount` line).

`run_game_hdi.py` boots DOSBox-X 2024.03.01 in PC-98 mode
(`config/runtime/dosbox-x-pc98.conf`), starts `GAME.BAT` from DOSBox-X's own
DOS like `th02_jp.conf`, sends xdotool keys and saves screenshots. Each run
works on its own copy of the image.

Result for `session6-overlay1` (`.analysis/runtime/game-overlay1`, all four
programs equal to the originals): ZUN.COM's programs and the sound driver load,
OP shows the logo and title menu (10-15 s) and starts the attract demo in
MAIN (30 s). With Return pressed through the menu, stage 1 starts at 26 s and
plays (enemies, scoring, the idle player losing lives by 55 s). The ending
(MAINE) was not reached in these runs.
