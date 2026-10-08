#!/usr/bin/env python3
"""Read-only Ghidra queries for matcher agents (runs on the host, not in tools/dock).

    python3 tools/ghidra_read.py fn 0x13CE20 [FIELDS]     get_functions; FIELDS default decompiled_code,callers,callees
    python3 tools/ghidra_read.py fn 0x13CE20,0x13CFA0 callers,callees,refs     several functions at once
    python3 tools/ghidra_read.py xrefs 0x4DDD90           references to an address
    python3 tools/ghidra_read.py mem 0x4DDD90 64           bytes at an address

Calls GhidraMCP's HTTP server (127.0.0.1:8089) directly, so an agent needs no MCP tool schemas in its context.
Only these read endpoints are reachable; renaming and retyping go through the repo and `ghidra_sync.py push`.
"""

import json
import sys
import urllib.parse
import urllib.request

URL = "http://127.0.0.1:8089"
PROGRAM = "SLUS_210.50"


def get(endpoint: str, **params) -> str:
    query = urllib.parse.urlencode({"program": PROGRAM, **params})
    try:
        with urllib.request.urlopen(f"{URL}/{endpoint}?{query}", timeout=120) as r:
            return r.read().decode()
    except OSError as e:
        sys.exit(f"{URL}/{endpoint}: {e}. Is Ghidra open on {PROGRAM} with the GhidraMCP server running?")


def show(text: str) -> None:
    """Print JSON compactly, with decompiled code as plain text rather than an escaped string."""
    try:
        data = json.loads(text)
    except ValueError:
        print(text)
        return
    code = []

    def strip(node):
        if isinstance(node, dict):
            for k in list(node):
                if k == "decompiled_code" and isinstance(node[k], str):
                    code.append(node.pop(k))
                else:
                    strip(node[k])
        elif isinstance(node, list):
            for v in node:
                strip(v)

    strip(data)
    if isinstance(data, dict):
        data.pop("revision", None)
        for f in (data.get("functions") or {}).values():
            if isinstance(f, dict):
                f.pop("revision", None)
    print(json.dumps(data, separators=(",", ":")))
    for c in code:
        print(c)


def main() -> None:
    if len(sys.argv) < 3:
        sys.exit(__doc__)
    cmd, arg, rest = sys.argv[1], sys.argv[2], sys.argv[3:]
    if cmd == "fn":
        fields = rest[0] if rest else "decompiled_code,callers,callees"
        key = "functions" if "," in arg else "function"
        show(get("get_functions", **{key: arg, "fields": fields, "include_call_context": "false"}))
    elif cmd == "xrefs":
        show(get("get_xrefs_to", address=arg))
    elif cmd == "mem":
        show(get("read_memory", address=arg, length=rest[0] if rest else "64"))
    else:
        sys.exit(__doc__)


if __name__ == "__main__":
    main()
