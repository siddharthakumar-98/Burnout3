"""Burnout 2 side of bo2map.py: the game functions of a b3dllc/burnout2 checkout, with what their bodies call.

Only what helps name Burnout 3 is read: each game function's address, size, qualified name, source file, the
names its body calls, its string and float literals, and the vtable slot order from the generated class headers.
A function counts as game code when its body sits in src/gamesource, src/GameShared or src/nodebug, under the
"// 0xSTART - 0xEND" comment the Burnout 2 sources put above every definition.
"""

import csv
import re
import struct
from dataclasses import dataclass, field
from pathlib import Path

GAME_DIRS = ("src/gamesource", "src/GameShared", "src/nodebug")

ADDR_COMMENT = re.compile(r"^// 0x([0-9A-Fa-f]+) - 0x([0-9A-Fa-f]+)", re.M)
CALL = re.compile(r"(?:(\w+)::)?(~?\w+)\s*\(")
STRING = re.compile(r'"((?:[^"\\\n]|\\.)*)"')
FLOAT = re.compile(r"(?<![\w.])(\d+\.\d*(?:[eE][-+]?\d+)?|\.\d+(?:[eE][-+]?\d+)?)[fF]?(?![\w.])")
VIRTUAL = re.compile(r"^\s*virtual [^;(]*?(~?\w+)\s*\([^;]*;\s*//\s*(?:0x([0-9A-Fa-f]+))?", re.M)
CLASS = re.compile(r"^(?:class|struct) (\w+)\s*(?::\s*(?:public\s+)?(\w+))?", re.M)
NOT_CALLS = {"if", "for", "while", "switch", "return", "sizeof", "do", "else", "case", "defined", "__asm__", "asm"}


@dataclass
class Fn2:
    addr: int
    size: int
    name: str                    # Class::method, or a plain function name
    file: str = ""               # path of the defining source file, relative to the checkout
    mangled: str = ""
    callees: set = field(default_factory=set)   # "Class::method" when qualified, else the bare name
    strings: set = field(default_factory=set)
    floats: set = field(default_factory=set)


def demangle(sym: str) -> str:
    """Qualified name of a CodeWarrior-mangled symbol, without its parameters ("m__9CGameModeFv" -> "CGameMode::m").

    Constructors and destructors become Class::Class and Class::~Class; unmangled names come back unchanged."""
    m = re.match(r"^(__ct|__dt|\w+?)__(\d+)(\w+)$", sym)
    if not m or len(m.group(3)) < int(m.group(2)):
        return re.sub(r"__F\w*$", "", sym)       # plain C++ function: drop the parameter signature
    method, cls = m.group(1), m.group(3)[:int(m.group(2))]
    if method == "__ct":
        method = cls
    elif method == "__dt":
        method = "~" + cls
    return f"{cls}::{method}"


def qualname(signature: str) -> str:
    """'CGameMode * CTwoPlayerSSGameMode::Update()' -> 'CTwoPlayerSSGameMode::Update'."""
    head = signature.split("(", 1)[0].strip()
    return head.split()[-1].lstrip("*&") if head else ""


def strip_comments(text: str) -> str:
    """Blank out comments and the contents of string/char literals, keeping offsets (so braces inside are inert)."""
    out, i, n = list(text), 0, len(text)
    while i < n:
        c = text[i]
        if text.startswith("//", i):
            j = text.find("\n", i)
            j = n if j < 0 else j
        elif text.startswith("/*", i):
            j = text.find("*/", i + 2)
            j = n if j < 0 else j + 2
        elif c in "\"'":
            j = i + 1
            while j < n and text[j] != c and text[j] != "\n":
                j += 2 if text[j] == "\\" else 1
            for k in range(i + 1, min(j, n)):
                out[k] = " "
            i = j + 1
            continue
        else:
            i += 1
            continue
        for k in range(i, j):
            if out[k] != "\n":
                out[k] = " "
        i = j
    return "".join(out)


def body_at(text: str, clean: str, start: int) -> str:
    """Raw text of the first brace-matched block at or after `start` (the function body), or ''."""
    i = clean.find("{", start)
    if i < 0:
        return ""
    depth = 0
    for j in range(i, len(clean)):
        if clean[j] == "{":
            depth += 1
        elif clean[j] == "}":
            depth -= 1
            if depth == 0:
                return text[i:j + 1]
    return text[i:]


def definitions(text: str) -> dict[int, str]:
    """Function bodies of one source file, keyed by the start address in the comment above each definition."""
    clean = strip_comments(text)
    return {int(m.group(1), 16): body_at(text, clean, m.end()) for m in ADDR_COMMENT.finditer(text)}


def scan_body(fn: Fn2, body: str) -> None:
    clean = strip_comments(body)
    for m in CALL.finditer(clean):
        cls, name = m.group(1), m.group(2)
        if name in NOT_CALLS or name.isdigit():
            continue
        fn.callees.add(f"{cls}::{name}" if cls else name)
    fn.strings.update(s for s in STRING.findall(body) if len(s) >= 4 and "\\" not in s)
    code = re.sub(r'"(?:[^"\\\n]|\\.)*"', '""', body)
    for lit in FLOAT.findall(strip_comments(code)):
        fn.floats.add(struct.unpack("<f", struct.pack("<f", float(lit)))[0])


def vtables(bo2: Path) -> dict[str, list[int]]:
    """Class -> Burnout 2 function address per vtable slot (0 for a pure virtual), from include/types/*.h.

    The generated headers declare a class's own virtuals in __vt__ slot order (slot 0 = vtable +0x08) but leave out
    inherited ones that are not overridden, so each class starts from its base's slots: an override takes its
    base slot by name, a new virtual is appended."""
    own, base = {}, {}
    for h in sorted((bo2 / "include/types").glob("*.h")):
        text = h.read_text(errors="ignore")
        cm = CLASS.search(text)
        if not cm:
            continue
        base[cm.group(1)] = cm.group(2)
        own[cm.group(1)] = [(m.group(1), int(m.group(2), 16) if m.group(2) else 0) for m in VIRTUAL.finditer(text)]
    full: dict[str, list[tuple[str, int]]] = {}

    def resolve(cls: str, seen=()) -> list:
        if cls in full:
            return full[cls]
        b = base.get(cls)
        slots = list(resolve(b, seen + (cls,))) if b in own and b not in seen else []
        for name, addr in own.get(cls, []):
            idx = next((i for i, (n, _) in enumerate(slots) if n == name), None)
            if idx is None:
                slots.append((name, addr))
            else:
                slots[idx] = (name, addr)
        full[cls] = slots
        return slots

    return {cls: [a for _, a in resolve(cls)] for cls in own if own[cls] or base.get(cls) in own and resolve(cls)}


def load(bo2: Path) -> list[Fn2]:
    """Burnout 2 game functions sorted by address."""
    for name in ("docs/functions.csv", "docs/symtab_functions.csv"):
        if not (bo2 / name).exists():
            raise SystemExit(f"{bo2 / name}: not found (is --bo2 a b3dllc/burnout2 checkout?)")
    fns: dict[int, Fn2] = {}
    with open(bo2 / "docs/symtab_functions.csv") as f:
        for r in csv.DictReader(f):
            a = int(r["address"], 16)
            name = qualname(r["demangled"]) if r["demangled"] else demangle(r["symbol"])
            fns[a] = Fn2(a, int(r["size"], 16), name, mangled=r["symbol"])
    with open(bo2 / "docs/functions.csv") as f:
        for r in csv.DictReader(f):
            a = int(r["address"], 16)
            old = fns.get(a)
            fns[a] = Fn2(a, int(r["size"], 16), qualname(r["signature"]), mangled=old.mangled if old else "")
    game = []
    for d in GAME_DIRS:
        for src in sorted((bo2 / d).rglob("*")):
            if src.suffix not in (".c", ".cpp") or not src.is_file():
                continue
            rel = str(src.relative_to(bo2))
            for addr, body in definitions(src.read_text(errors="ignore")).items():
                fn = fns.get(addr)
                if fn is None or fn.file:
                    continue
                fn.file = rel
                scan_body(fn, body)
                game.append(fn)
    return sorted(game, key=lambda f: f.addr)
