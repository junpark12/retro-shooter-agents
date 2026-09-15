#!/usr/bin/env python3
"""Optional native input smoke test; requires an isolated DISPLAY, Pillow and xdotool."""

import argparse
import json
import os
from pathlib import Path
import shutil
import subprocess
import sys
import time

from PIL import ImageChops, ImageGrab, ImageStat


class NativePlaytest:
    def __init__(self, executable, output):
        self.executable = executable
        self.output = output
        self.process = None
        self.window = None
        self.held = set()
        self.report = {
            "executable": str(executable),
            "display": os.environ["DISPLAY"],
            "audio_driver": os.environ.get("SDL_AUDIODRIVER"),
            "audio_listening_verified": False,
            "captures": [],
            "checks": [],
            "coverage": {},
        }

    def xdo(self, *args):
        return subprocess.check_output(
            ["xdotool", *args], text=True, stderr=subprocess.PIPE, timeout=10
        ).strip()

    def alive(self):
        if self.process.poll() is not None:
            raise RuntimeError(f"Game exited unexpectedly: {self.process.returncode}")

    def start(self):
        with (self.output / "game.log").open("w") as log:
            self.process = subprocess.Popen(
                [str(self.executable)],
                cwd=self.executable.parent,
                stdout=log,
                stderr=subprocess.STDOUT,
            )
        self.report["pid"] = self.process.pid
        deadline = time.monotonic() + 15
        while time.monotonic() < deadline:
            self.alive()
            result = subprocess.run(
                ["xdotool", "search", "--onlyvisible", "--pid", str(self.process.pid)],
                text=True, capture_output=True, timeout=5,
            )
            if result.returncode == 0 and result.stdout.strip():
                windows = result.stdout.splitlines()
                if len(windows) != 1:
                    raise RuntimeError(f"Expected one game window, found {windows}")
                self.window = windows[0]
                break
            if result.returncode not in (0, 1):
                raise RuntimeError(f"Window lookup failed: {result.stderr}")
            time.sleep(0.1)
        else:
            raise TimeoutError("No visible window belonging to the game PID within 15s")
        self.xdo("windowfocus", "--sync", self.window)
        self.report["window"] = self.window
        self.report["window_title"] = self.xdo("getwindowname", self.window)
        self.wait(0.4)

    def wait(self, seconds):
        deadline = time.monotonic() + seconds
        while time.monotonic() < deadline:
            self.alive()
            time.sleep(min(0.05, max(0, deadline - time.monotonic())))

    def down(self, *keys):
        self.held.update(keys)
        self.xdo("keydown", *keys)

    def up(self, *keys):
        self.xdo("keyup", *keys)
        self.held.difference_update(keys)

    def tap(self, *keys):
        for key in keys:
            self.down(key)
            self.wait(0.06)
            self.up(key)
            self.wait(0.15)

    def capture(self, name):
        self.alive()
        geometry = dict(
            line.split("=", 1)
            for line in self.xdo("getwindowgeometry", "--shell", self.window).splitlines()
        )
        x, y, width, height = (int(geometry[k]) for k in ("X", "Y", "WIDTH", "HEIGHT"))
        if (width, height) != (480, 640):
            raise RuntimeError(f"Expected a 480x640 client window, found {width}x{height}")
        image = ImageGrab.grab(
            bbox=(x, y, x + width, y + height), xdisplay=os.environ["DISPLAY"]
        ).convert("RGB")
        if image.size != (480, 640):
            raise RuntimeError(f"Unexpected screenshot shape: {image.size}")
        if all(low == high for low, high in image.getextrema()):
            raise RuntimeError(f"Blank screenshot: {name}")
        path = self.output / f"{name}.png"
        image.save(path)
        self.report["captures"].append({"name": name, "path": str(path), "size": list(image.size)})
        return image

    @staticmethod
    def difference(first, second, box=None):
        if box:
            first, second = first.crop(box), second.crop(box)
        return sum(ImageStat.Stat(ImageChops.difference(first, second)).mean) / 3

    @staticmethod
    def yellow_pixels(image, box):
        pixels = image.load()
        left, top, right, bottom = box
        return sum(
            red > 180 and green > 180 and blue < 100
            for y in range(top, bottom)
            for x in range(left, right)
            for red, green, blue in (pixels[x, y],)
        )

    @classmethod
    def selection_marker(cls, image):
        # These fixed-size headings distinguish menus from the animated playfield.
        return cls.yellow_pixels(image, (120, 60, 360, 90))

    @classmethod
    def pause_marker(cls, image):
        return cls.yellow_pixels(image, (195, 290, 285, 312))

    @staticmethod
    def gameover_marker(image):
        pixels = image.load()
        red_heading = sum(
            red > 180 and green < 100 and blue < 100
            for y in range(268, 282)
            for x in range(140, 340)
            for red, green, blue in (pixels[x, y],)
        )
        white_prompt = sum(
            min(pixels[x, y]) > 180
            for y in range(340, 360)
            for x in range(140, 340)
        )
        return red_heading > 300 and white_prompt > 400

    def check(self, name, passed, **evidence):
        self.report["checks"].append({"name": name, "passed": passed, **evidence})
        if not passed:
            raise AssertionError(f"{name}: {evidence}")

    def run(self, baseline, idle_seconds):
        self.start()
        title = self.capture("01-title")
        self.down("Return")
        self.wait(0.12)
        selection = self.capture("02-enter-down-selection")
        initial_marker = self.selection_marker(selection)
        self.check("enter_opens_ship_selection", initial_marker > 100, yellow_pixels=initial_marker)
        self.wait(1.1)
        held = self.capture("03-enter-held")
        self.up("Return")
        self.wait(0.2)
        released = self.capture("04-enter-released")
        held_marker = self.selection_marker(held)
        released_marker = self.selection_marker(released)
        header_diff = self.difference(selection, released, (120, 60, 360, 90))
        stays_selected = held_marker > 100 and released_marker > 100 and header_diff < 2
        self.report["coverage"]["enter_hold_seconds"] = 1.22
        if baseline:
            self.report["checks"].append({
                "name": "held_enter_stays_in_selection",
                "passed": stays_selected,
                "expected_regression_allowed": True,
                "held_yellow_pixels": held_marker,
                "released_yellow_pixels": released_marker,
                "header_mean_difference": header_diff,
            })
            self.report["coverage"]["scope"] = "Baseline title/held-Enter only; remaining inputs not run"
            return
        self.check(
            "held_enter_stays_in_selection", stays_selected,
            held_yellow_pixels=held_marker, released_yellow_pixels=released_marker,
            header_mean_difference=header_diff,
        )
        self.tap("Right")
        next_ship = self.capture("05-next-ship")
        self.check(
            "ship_selection_changes",
            self.selection_marker(next_ship) > 100
            and self.difference(released, next_ship, (20, 150, 460, 410)) > 2,
            mean_difference=self.difference(released, next_ship, (20, 150, 460, 410)),
        )
        self.tap("Left")
        self.capture("06-previous-ship")
        self.tap("Return")
        self.wait(0.5)
        combat = self.capture("07-combat-start")
        self.check("fresh_enter_leaves_selection", self.selection_marker(combat) < 100)
        self.down("Right", "space")
        self.wait(0.5)
        self.up("Right", "space")
        self.capture("08-movement-fire")
        self.down("space")
        self.wait(1.4)
        self.capture("09-fire-charge-held")
        self.up("space")
        self.wait(0.2)
        self.capture("10-charge-released")
        self.down("z")
        self.wait(0.8)
        self.capture("11-lock-held")
        self.up("z")
        self.tap("c")
        self.capture("12-bomb-input")
        self.report["coverage"]["combat_inputs"] = {
            "sent": ["Right+space 0.5s", "space held 1.4s/released", "z held 0.8s/released", "c"],
            "effect_verification": "Screenshots only; damage, charge, lock and bomb effects not automatically classified",
        }
        self.down("p")
        self.wait(0.15)
        paused = self.capture("13-pause-key-down")
        pause_samples = [self.pause_marker(paused)]
        for index in range(5):
            self.wait(0.22)
            sample = self.capture(f"14-pause-hold-sample-{index + 1}")
            pause_samples.append(self.pause_marker(sample))
        pause_held = self.capture("14-pause-key-held")
        self.up("p")
        self.wait(0.3)
        pause_released = self.capture("15-pause-key-released")
        self.check(
            "held_p_keeps_pause_overlay_visible",
            min(pause_samples) > 100 and self.pause_marker(pause_released) > 100,
            sampled_pause_yellow_pixels=pause_samples,
            released_pause_yellow_pixels=self.pause_marker(pause_released),
            animated_background_mean_difference=self.difference(paused, pause_held),
        )
        self.tap("p")
        self.wait(0.3)
        resumed = self.capture("16-fresh-p-resume")
        self.check(
            "fresh_p_resumes_scene",
            self.pause_marker(resumed) < 100 and self.difference(paused, resumed) > 1,
        )
        self.xdo("windowfocus", "0")
        self.wait(0.2)
        self.check("focus_moved_away", self.xdo("getwindowfocus", "-f") != self.window)
        self.xdo("windowfocus", "--sync", self.window)
        self.wait(0.2)
        focus_paused = self.capture("17-focus-return")
        self.wait(0.5)
        focus_still = self.capture("18-focus-remains-paused")
        self.check(
            "focus_return_keeps_pause_overlay_visible",
            self.pause_marker(focus_paused) > 100 and self.pause_marker(focus_still) > 100,
            pause_yellow_pixels=[self.pause_marker(focus_paused), self.pause_marker(focus_still)],
        )
        self.tap("p")
        self.wait(0.3)
        focus_resumed = self.capture("19-focus-fresh-resume")
        self.check(
            "fresh_p_after_focus_resumes_scene",
            self.pause_marker(focus_resumed) < 100 and self.difference(focus_still, focus_resumed) > 1,
        )
        elapsed = 0
        gameover = False
        while elapsed < idle_seconds:
            interval = min(2, idle_seconds - elapsed)
            self.wait(interval)
            elapsed += interval
            idle = self.capture(f"20-idle-{elapsed:03d}s")
            if self.gameover_marker(idle):
                gameover = True
                break
        self.report["coverage"]["stationary_play_seconds"] = elapsed
        self.report["coverage"]["gameover"] = {
            "observed": gameover,
            "evidence": "Red GAME OVER heading region and white PRESS ENTER prompt region",
        }
        self.report["coverage"]["continue"] = "Unverified; intermediate idle frames retained"
        self.report["coverage"]["restart_flow"] = {
            "sent": [],
            "verified_new_run": False,
        }
        if not gameover:
            self.report["coverage"]["restart_flow"]["reason"] = "No game-over screen detected within idle limit"
            return
        # Escape exits on GAME OVER; follow the captured on-screen Enter prompt instead.
        self.tap("Return")
        restart = self.capture("21-gameover-enter")
        self.report["coverage"]["restart_flow"]["sent"].append("Return")
        if self.selection_marker(restart) < 100:
            self.check(
                "gameover_enter_returns_to_title",
                self.difference(title, restart, (40, 90, 440, 230)) < 2,
                title_region_mean_difference=self.difference(title, restart, (40, 90, 440, 230)),
            )
            self.tap("Return")
            restart = self.capture("22-restart-selection")
            self.report["coverage"]["restart_flow"]["sent"].append("Return")
        self.check("restart_reaches_selection", self.selection_marker(restart) > 100)
        self.tap("Return")
        self.report["coverage"]["restart_flow"]["sent"].append("Return")
        self.wait(0.3)
        restarted = self.capture("23-restarted-combat")
        self.check(
            "restart_leaves_selection",
            self.selection_marker(restarted) < 100 and not self.gameover_marker(restarted),
        )
        self.report["coverage"]["restart_flow"]["verified_new_run"] = True

    def close(self):
        errors = []
        if self.held:
            try:
                self.xdo("keyup", *sorted(self.held))
                self.held.clear()
            except (subprocess.SubprocessError, OSError) as error:
                errors.append(f"Key release failed: {error}")
        if self.process is not None:
            if self.process.poll() is None:
                self.process.terminate()
                try:
                    self.process.wait(timeout=5)
                except subprocess.TimeoutExpired:
                    self.process.kill()
                    self.process.wait(timeout=5)
            self.report["game_exit_code"] = self.process.returncode
            self.report["own_game_stopped"] = self.process.poll() is not None
        self.report["held_keys_released"] = not self.held
        self.report["cleanup_errors"] = errors
        if errors:
            self.report["status"] = "failed"


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--executable", required=True, type=Path)
    parser.add_argument("--output", required=True, type=Path)
    parser.add_argument("--baseline", action="store_true", help="Capture only held-Enter behavior; allow its known regression")
    parser.add_argument("--idle-seconds", type=int, default=60, help="Stationary play duration, 0..100 (default: 60)")
    args = parser.parse_args()
    if not args.executable.is_absolute() or not args.executable.is_file() or not os.access(args.executable, os.X_OK):
        parser.error("--executable must name an existing executable by absolute path")
    if not 0 <= args.idle_seconds <= 100:
        parser.error("--idle-seconds must be between 0 and 100")
    if not os.environ.get("DISPLAY") or not shutil.which("xdotool"):
        parser.error("An existing isolated DISPLAY and xdotool on PATH are required")
    args.output = args.output.resolve()
    if (args.output / "report.json").exists():
        parser.error("--output already contains report.json; choose a new evidence directory")
    args.output.mkdir(parents=True, exist_ok=True)
    test = NativePlaytest(args.executable.resolve(), args.output)
    try:
        test.run(args.baseline, args.idle_seconds)
        test.report["status"] = "completed"
    except (Exception, KeyboardInterrupt) as error:
        test.report["status"] = "failed"
        test.report["error"] = f"{type(error).__name__}: {error}"
        if isinstance(error, subprocess.CalledProcessError):
            test.report["command_stderr"] = error.stderr
        print(test.report["error"], file=sys.stderr)
    finally:
        try:
            test.close()
        except (Exception, KeyboardInterrupt) as error:
            test.report["status"] = "failed"
            test.report["cleanup_error"] = f"{type(error).__name__}: {error}"
            print(test.report["cleanup_error"], file=sys.stderr)
        (args.output / "report.json").write_text(json.dumps(test.report, indent=2) + "\n")
    print(json.dumps(test.report, indent=2))
    return 0 if test.report["status"] == "completed" else 1


if __name__ == "__main__":
    sys.exit(main())
