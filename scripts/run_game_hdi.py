#!/usr/bin/env python3
"""Run a prepared TH02 game image in DOSBox-X (PC-98) and save timed frames.

Run under a private X server: `xvfb-run -a -s "-screen 0 1024x768x24"
python3 scripts/run_game_hdi.py DIR ...`, with DIR from prepare_game_hdi.py.
The image is mounted with DOSBox-X's own DOS, like the user's th02_jp.conf,
and GAME.BAT starts ZUN.COM's programs, KAJA's drivers and OP.EXE. Keys are
sent with xdotool (`--key 30:Return`). Each run works on a copy of game.hdi.
"""

from __future__ import annotations

import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess
import time

from lib.pc98 import digest_file

ROOT = Path(__file__).resolve().parents[1]
CONFIG = ROOT / "config" / "runtime" / "dosbox-x-pc98.conf"


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__.splitlines()[0])
    parser.add_argument("prepared", type=Path, help="directory with game.hdi")
    parser.add_argument("--name", default="run", help="run subdirectory")
    parser.add_argument("--frame", type=float, action="append", default=[], help="screenshot second")
    parser.add_argument("--key", action="append", default=[], help="SECOND:XDOTOOL_KEY")
    parser.add_argument("--time-limit", type=int, default=60)
    args = parser.parse_args()
    if not os.environ.get("DISPLAY"):
        parser.error("needs an X display; run under xvfb-run")

    run = args.prepared.resolve() / args.name
    run.mkdir()
    image = run / "game.hdi"
    shutil.copyfile(args.prepared.resolve() / "game.hdi", image)
    command = [
        "dosbox-x", "-defaultconf", "-defaultmapper", "-conf", str(CONFIG),
        "-fastlaunch", "-nogui", "-nomenu", "-time-limit", str(args.time_limit),
        "-c", f'imgmount c "{image}"', "-c", "c:", "-c", "cd humaroku", "-c", "game",
    ]
    env = dict(os.environ, SDL_VIDEODRIVER="x11", SDL_AUDIODRIVER="dummy",
               XDG_CONFIG_HOME=str(run / "xdg"), XDG_CACHE_HOME=str(run / "xdg"))
    events = sorted([(float(s), "frame", None) for s in args.frame]
                    + [(float(k.split(":", 1)[0]), "key", k.split(":", 1)[1]) for k in args.key])
    log = (run / "dosbox-x.log").open("w")
    process = subprocess.Popen(command, cwd=run, env=env, stdout=log, stderr=subprocess.STDOUT,
                               start_new_session=True)
    started = time.monotonic()
    done = []
    try:
        for second, kind, value in events:
            while time.monotonic() - started < second:
                if process.poll() is not None:
                    raise SystemExit(f"DOSBox-X exited after {time.monotonic() - started:.1f} s")
                time.sleep(0.05)
            if kind == "frame":
                frame = run / f"frame-{second:05.1f}s.png"
                subprocess.run(["import", "-window", "root", str(frame)], check=True, timeout=10)
                done.append({"second": second, "frame": frame.name, "sha256": digest_file(frame)})
            else:
                windows = subprocess.run(["xdotool", "search", "--name", "DOSBox-X"],
                                         capture_output=True, text=True).stdout.split()
                for window in windows:
                    subprocess.run(["xdotool", "key", "--window", window, value], check=False)
                done.append({"second": second, "key": value, "windows": len(windows)})
    finally:
        if process.poll() is None:
            os.killpg(process.pid, 15)
            process.wait(timeout=10)
        log.close()
    receipt = {"prepared": json.loads((args.prepared / "receipt.json").read_text()),
               "command": command, "events": done,
               "config_sha256": digest_file(CONFIG), "dosbox_x": shutil.which("dosbox-x")}
    (run / "receipt.json").write_text(json.dumps(receipt, indent=2) + "\n", encoding="utf-8")
    for event in done:
        print(event)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
