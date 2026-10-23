#!/usr/bin/env python3
"""Compile original player.c and trace its init/sleep references, without execution."""
import argparse
from pathlib import Path
import re
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parents[3]
ROOTS = ("PlayerInit", "PlayerSleep")


def run(*args):
    return subprocess.check_output(args, text=True)


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--elf", type=Path, help="Optional built ARM9 ELF to compare symbol presence")
    parser.add_argument("--nm", default="nm", help="Use arm-none-eabi-nm with an ARM9 ELF")
    args = parser.parse_args()
    if args.elf is not None and not args.elf.is_file():
        parser.error("--elf must be an existing file")
    with tempfile.TemporaryDirectory(prefix="picori-player-audit-") as temp:
        obj = str(Path(temp) / "player.o")
        command = ["cc", "-std=gnu11", "-O0", "-w", "-ffunction-sections",
                   "-fdata-sections", "-fno-strict-aliasing", "-fwrapv",
                   "-DPC_PORT", "-DTMC_DSI", "-DNON_MATCHING", "-DUSE_HDMA",
                   "-DEU", "-DENGLISH", "-DREVISION=0"]
        for path in ("", "include", "port", "platform/dsi/source", "platform/dsi-engine/source"):
            command += ["-I", str(ROOT / path)]
        command += ["-include", str(ROOT / "include/region.h"), "-c",
                    str(ROOT / "src/player.c"), "-o", obj]
        subprocess.run(command, check=True)
        # Function/data sections preserve references through dispatch tables.
        # No source regex guessing and no fake external implementations.
        edges = {}
        section = None
        for line in run("objdump", "-r", obj).splitlines():
            match = re.match(r"RELOCATION RECORDS FOR \[(.+)\]:", line)
            if match:
                section = match[1]
                edges.setdefault(section, set())
            elif section and re.match(r"^[0-9a-fA-F]+\s", line):
                target = re.sub(r"[+-]0x[0-9a-fA-F]+$", "", line.split()[2])
                edges[section].add(target)
        # Symbols can be referenced by name or directly by their local section.
        symbols = {}
        for line in run("objdump", "-t", obj).splitlines():
            parts = line.split()
            if len(parts) >= 5 and re.fullmatch(r"[0-9a-fA-F]+", parts[0]):
                if parts[-3].startswith("."):
                    symbols[parts[-1]] = parts[-3]
        present = None
        if args.elf:
            present = {line.split()[-1] for line in
                       run(args.nm, "--defined-only", str(args.elf)).splitlines()
                       if len(line.split()) >= 3}
            if not present:
                parser.error("--elf has no readable defined symbols; use an unstripped ELF")
        print("# Original player init/sleep dependency audit")
        print("Host compilation only. Does not run or enable gameplay.")
        print("Reachability stops at externals outside player.c; it is not the full engine closure.")
        print("A symbol present in an ELF can still be a stub or blocked service.\n")
        for root in ROOTS:
            start = ".text." + root
            if start not in edges:
                raise RuntimeError("Missing original section: " + start)
            visited, external, pending = set(), set(), [start]
            while pending:
                current = pending.pop()
                if current in visited:
                    continue
                visited.add(current)
                for target in edges.get(current, ()):
                    local = target if target.startswith(".") else symbols.get(target)
                    if local is not None:
                        pending.append(local)
                    else:
                        external.add(target)
            print("## " + root)
            print("Direct references: " + ", ".join(sorted(edges[start])))
            print("Reachable local functions: " + ", ".join(
                sorted(s.removeprefix(".text.") for s in visited if s.startswith(".text."))))
            print("External references (functions AND data):")
            for name in sorted(external):
                status = "" if present is None else (
                    " [present; behavior unverified]" if name in present else " [absent from ELF]")
                print("- " + name + status)
            print()


if __name__ == "__main__":
    main()
