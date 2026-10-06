#!/usr/bin/env python3
"""Keep the Ghidra project in step with the repo, through the ghidra-mcp plugin's HTTP API.

The repo is the source of truth: function boundaries come from the generated assembly (assembly/asm/), names from
config/symbol_addrs.txt and units from assembly/splat/b3.yaml. Ghidra mirrors them. Plain Python 3, run on the host
(not in the build container) with Ghidra open on SLUS_210.50 and the GhidraMCP HTTP server running:

    python3 tools/ghidra_sync.py status            compare Ghidra's functions and names with the repo's
    python3 tools/ghidra_sync.py push              create missing functions, then apply names and unit tags
    python3 tools/ghidra_sync.py push --no-tags    names and functions only
    python3 tools/ghidra_sync.py push --reset      also rename Ghidra names the repo doesn't have back to func_XXXXXXXX
    python3 tools/ghidra_sync.py pull              list names given in Ghidra that symbol_addrs.txt doesn't have

`pull` only prints candidate lines in symbol_addrs.txt format; review them before adding them. With Strict Naming
Enforcement on (Edit > Tool Options > GhidraMCP HTTP Server), Ghidra rejects names such as `memcpy`; push passes
strict_mode=off for each rename, but turning the option off also stops struct fields from being renamed.
"""

import argparse
import json
import re
import sys
import urllib.parse
import urllib.request
from pathlib import Path

ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(ROOT / "tools"))
import dataslice  # noqa: E402
import xref  # noqa: E402

URL = "http://127.0.0.1:8089"
PROGRAM = "SLUS_210.50"
SYMBOL_ADDRS = ROOT / "config/symbol_addrs.txt"
AUTO_NAME = re.compile(r"(FUN|thunk_FUN|LAB|SUB)_[0-9a-fA-F]{8}$|func_[0-9A-F]{8}$|entry$")


def call(endpoint: str, params: dict | None = None, body: dict | None = None):
    query = urllib.parse.urlencode({"program": PROGRAM, **(params or {})})
    req = urllib.request.Request(f"{URL}/{endpoint}?{query}")
    if body is not None:
        req.data = json.dumps({"program": PROGRAM, **body}).encode()
        req.add_header("Content-Type", "application/json")
    try:
        with urllib.request.urlopen(req, timeout=120) as r:
            return json.loads(r.read().decode())
    except OSError as e:
        sys.exit(f"{URL}/{endpoint}: {e}. Is Ghidra open on {PROGRAM} with the GhidraMCP server running?")


def ghidra_listing() -> list[dict]:
    out = call("find_functions", {"limit": 0})
    if "functions" not in out:
        sys.exit(f"find_functions failed: {out}")
    return out["functions"]


def ghidra_functions() -> dict[int, str]:
    return {int(f["address"], 16): f["name"] for f in ghidra_listing()}


def repo_names() -> dict[int, str]:
    """Function names from symbol_addrs.txt (entries marked type:func, or inside .text/.init)."""
    out = {}
    for line in SYMBOL_ADDRS.read_text().splitlines():
        m = re.match(r"\s*(\S+)\s*=\s*0x([0-9A-Fa-f]+)\s*;(.*)", line)
        if not m:
            continue
        addr = int(m.group(2), 16)
        if "type:func" in m.group(3) or xref.section_of(addr) in ("text", "init"):
            out[addr] = m.group(1)
    return out


def repo_functions() -> list[int]:
    return sorted(f.addr for f in xref.load())


def alternate_entries() -> set[int]:
    """Addresses of alternate entry points (`alabel`) inside repo functions; Ghidra makes functions of them."""
    out = set()
    for path in (ROOT / "assembly/asm").rglob("*.s"):
        for m in re.finditer(r"^\s*alabel func_([0-9A-F]{8})", path.read_text(), re.M):
            out.add(int(m.group(1), 16))
    return out


def unit_of() -> dict[int, str]:
    starts = dataslice.text_units()
    out = {}
    for a in repo_functions():
        if a >= 0x469E00:
            out[a] = "sinit"
            continue
        k = max(i for i, (s, _) in enumerate(starts) if s <= a)
        out[a] = starts[k][1]
    return out


def cmd_status() -> None:
    g, ours, names = ghidra_functions(), repo_functions(), repo_names()
    missing = [a for a in ours if a not in g]
    alt = alternate_entries()
    extra = sorted(a for a in g if a not in set(ours) and a not in alt)
    wrong = [(a, g[a], n) for a, n in sorted(names.items()) if a in g and g[a] != n]
    custom = [a for a, n in g.items() if not AUTO_NAME.match(n) and a not in names]
    print(f"Ghidra has {len(g):,} functions; the repo has {len(ours):,}.")
    print(f"  {len(missing):,} repo functions are missing in Ghidra (push creates them)")
    print(f"  {len(alt & set(g)):,} more are alternate entry points inside repo functions (fine)")
    print(f"  {len(extra):,} Ghidra functions don't start a repo function: "
          + ", ".join(f"{a:08X} {g[a]}" for a in extra[:10]) + (" …" if len(extra) > 10 else ""))
    print(f"  {len(wrong):,} repo names differ in Ghidra (push applies them)")
    print(f"  {len(custom):,} names exist only in Ghidra (pull lists them)")


def cmd_push(tags: bool, reset: bool) -> None:
    g, names = ghidra_functions(), repo_names()
    created = renamed = tagged = 0
    for a in repo_functions():
        if a not in g:
            r = call("create_function", body={"address": f"0x{a:08X}", "disassemble_first": "true"})
            if r.get("success") or "error" not in r:
                created += 1
            else:
                print(f"create {a:08X}: {r.get('error')}", file=sys.stderr)
    g = ghidra_functions()
    for a, name in sorted(names.items()):
        if a in g and g[a] != name:
            r = call("rename_function", body={"old_name": f"0x{a:08X}", "new_name": name, "strict_mode": "off"})
            if "error" in r:
                print(f"rename {a:08X} -> {name}: {r['error']}", file=sys.stderr)
            else:
                renamed += 1
    if reset:
        # names given in Ghidra that the repo doesn't have (run pull first to keep any worth keeping)
        for a, name in sorted(g.items()):
            if a not in names and not AUTO_NAME.match(name):
                r = call("rename_function", body={"old_name": f"0x{a:08X}", "new_name": f"func_{a:08X}",
                                                  "strict_mode": "off"})
                if "error" in r:
                    print(f"reset {a:08X} {name}: {r['error']}", file=sys.stderr)
                else:
                    renamed += 1
    if tags:
        # every function carries one "unit:<unit>" tag; a function whose unit changed loses its old tag
        have = {int(f["address"], 16): [t for t in f.get("tags", []) if t.startswith("unit:")]
                for f in ghidra_listing()}
        todo = []
        for a, unit in unit_of().items():
            want = f"unit:{unit}"
            if a not in have or have[a] == [want]:
                continue
            for old in have[a]:
                if old != want:
                    call("remove_function_tag", body={"function": f"0x{a:08X}", "tags": old})
            todo.append({"function": f"0x{a:08X}", "tags": want})
        for k in range(0, len(todo), 500):
            r = call("add_function_tag", body={"assignments": todo[k:k + 500]})
            if "error" in r:
                print(f"tagging: {r['error']}", file=sys.stderr)
            else:
                tagged += len(todo[k:k + 500])
    print(f"created {created} functions, renamed {renamed}, tagged {tagged}")


def cmd_pull() -> None:
    g, names = ghidra_functions(), repo_names()
    new = [(a, n) for a, n in sorted(g.items()) if not AUTO_NAME.match(n) and names.get(a) != n]
    for a, n in new:
        note = f"  // repo: {names[a]}" if a in names else ""
        print(f"{n} = 0x{a:08X}; // type:func{note}")
    print(f"// {len(new)} names in Ghidra that symbol_addrs.txt doesn't have", file=sys.stderr)


def main() -> None:
    p = argparse.ArgumentParser(description=__doc__, formatter_class=argparse.RawDescriptionHelpFormatter)
    sub = p.add_subparsers(dest="cmd", required=True)
    sub.add_parser("status")
    s = sub.add_parser("push")
    s.add_argument("--no-tags", action="store_true", help="skip tagging every function with its unit")
    s.add_argument("--reset", action="store_true", help="rename names only Ghidra has back to func_XXXXXXXX")
    sub.add_parser("pull")
    a = p.parse_args()
    if a.cmd == "status":
        cmd_status()
    elif a.cmd == "push":
        cmd_push(not a.no_tags, a.reset)
    else:
        cmd_pull()


if __name__ == "__main__":
    main()
