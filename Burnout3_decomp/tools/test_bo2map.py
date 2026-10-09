"""bo2map.py / bo2src.py on synthetic fixtures (no Burnout 2 checkout or generated asm needed)."""

import struct
import tempfile
import unittest
from pathlib import Path

import bo2map
import bo2src
from bo2map import Fn3, align, match
from bo2src import Fn2


def f32(x: float) -> float:
    return struct.unpack("<f", struct.pack("<f", x))[0]


class Demangle(unittest.TestCase):
    def test_names(self):
        self.assertEqual(bo2src.demangle("FExist__13CGTFileSystemFPCc"), "CGTFileSystem::FExist")
        self.assertEqual(bo2src.demangle("__ct__9CGameModeFv"), "CGameMode::CGameMode")
        self.assertEqual(bo2src.demangle("__dt__9CGameModeFv"), "CGameMode::~CGameMode")
        self.assertEqual(bo2src.demangle("GtLListRemoveAllLinks__FP10GtLListTag"), "GtLListRemoveAllLinks")
        self.assertEqual(bo2src.demangle("main"), "main")

    def test_qualname(self):
        self.assertEqual(bo2src.qualname("CGameMode * CTwoPlayerSSGameMode::Update()"),
                         "CTwoPlayerSSGameMode::Update")


SOURCE = """
// 0x100 - 0x120
void CFoo::A(int x)
{
    /* a comment with { brace */
    if (x) Bar(x);
    mpThing->Baz("text with } brace", 0.85f);
    CQux::Init();
    return;
}

// 0x200 - 0x210
int B() { return sizeof(int); }
"""


class Bodies(unittest.TestCase):
    def test_definitions_and_scan(self):
        defs = bo2src.definitions(SOURCE)
        self.assertEqual(sorted(defs), [0x100, 0x200])
        self.assertTrue(defs[0x100].rstrip().endswith("return;\n}"))
        fn = Fn2(0x100, 0x20, "CFoo::A")
        bo2src.scan_body(fn, defs[0x100])
        self.assertEqual(fn.callees, {"Bar", "Baz", "CQux::Init"})
        self.assertEqual(fn.strings, {"text with } brace"})
        self.assertEqual(fn.floats, {f32(0.85)})
        fb = Fn2(0x200, 0x10, "B")
        bo2src.scan_body(fb, defs[0x200])
        self.assertEqual(fb.callees, set())

    def test_vtables_inherit(self):
        with tempfile.TemporaryDirectory() as d:
            t = Path(d) / "include/types"
            t.mkdir(parents=True)
            (t / "CBase.h").write_text("class CBase {\n    virtual void Init(); // 0x10\n"
                                       "    virtual void Pause(); // 0x20 (symtab; return type unknown)\n"
                                       "    virtual void Draw() = 0; // pure (NULL vtable slot)\n};\n")
            (t / "CKid.h").write_text("class CKid : public CBase {\n    virtual void Init(); // 0x30\n"
                                      "    virtual void Extra(); // 0x40\n};\n")
            vt = bo2src.vtables(Path(d))
        self.assertEqual(vt["CBase"], [0x10, 0x20, 0])
        self.assertEqual(vt["CKid"], [0x30, 0x20, 0, 0x40])


class Align(unittest.TestCase):
    def test_gap_for_added_function(self):
        self.assertEqual(align([0x40, 0x100, 0x8, 0x200], [0x40, 0x8, 0x1F0]), [(0, 0), (2, 1), (3, 2)])

    def test_no_forced_pair(self):
        self.assertEqual(align([0x400], [0x10]), [])


def f3(addr, size, name=None, unit="game/u", **kw):
    return Fn3(addr, size, name or f"func_{addr:08X}", unit, **kw)


def f2(addr, size, name, file="src/nodebug/X.cpp", **kw):
    return Fn2(addr, size, name, file, **kw)


class Matching(unittest.TestCase):
    def test_name_seed_is_high(self):
        rows = match([f3(0x1000, 0x40, "Init__4CFooFv")], [f2(0x10, 0x40, "CFoo::Init")], [], {})
        self.assertEqual(rows[0x1000].confidence, "high")
        self.assertEqual(rows[0x1000].kinds, {"name"})

    def test_order_extends_from_seed(self):
        b3 = [f3(0x1000, 0x40, "Init__4CFooFv"), f3(0x1040, 0x100), f3(0x1140, 0x500)]
        b2 = [f2(0x10, 0x40, "CFoo::Init"), f2(0x50, 0x110, "CFoo::Update"), f2(0x160, 0x800, "CFoo::Big")]
        rows = match(b3, b2, [], {})
        self.assertEqual(rows[0x1040].fn2.name, "CFoo::Update")
        self.assertEqual(rows[0x1040].confidence, "medium")      # order only, sizes within 25%
        self.assertEqual(rows[0x1140].confidence, "low")         # order only, 0x500 vs 0x800 is outside 25%
        self.assertEqual(rows[0x1140].kinds, {"order"})

    def test_order_plus_call_is_high(self):
        b3 = [f3(0x1000, 0x40, "Init__4CFooFv", calls={0x1040}), f3(0x1040, 0x100)]
        b2 = [f2(0x10, 0x40, "CFoo::Init", callees={"Update"}), f2(0x50, 0x110, "CFoo::Update")]
        rows = match(b3, b2, [], {})
        self.assertEqual(rows[0x1040].kinds, {"order", "call"})
        self.assertEqual(rows[0x1040].confidence, "high")

    def test_ambiguous_call_skipped(self):
        b3 = [f3(0x1000, 0x40, "Init__4CFooFv", unit="game/a", calls={0x2000}), f3(0x2000, 0x100, unit="game/b")]
        b2 = [f2(0x10, 0x40, "CFoo::Init", callees={"Run"}, file="src/a.cpp"),
              f2(0x50, 0x100, "CA::Run", file="src/b.cpp"), f2(0x150, 0x100, "CB::Run", file="src/c.cpp")]
        rows = match(b3, b2, [], {})
        self.assertNotIn(0x2000, rows)

    def test_conflict_demotes(self):
        b3 = [f3(0x1000, 0x40, strings={"alpha"}, unit="game/a"), f3(0x2000, 0x40, unit="game/b")]
        b2 = [f2(0x10, 0x40, "CFoo::A", strings={"alpha"}, file="src/a.cpp"),
              f2(0x60, 0x40, "CFoo::B", file="src/b.cpp")]
        m = bo2map.Matcher(b3, b2, [], {})
        m.seed()
        m.propose(0x1000, 0x60, "call")              # disagreeing evidence for the same function
        row = m.rows()[0x1000]
        self.assertTrue(row.conflict)
        self.assertEqual(row.confidence, "low")

    def test_vtable_slots(self):
        b3 = [f3(0x1000, 0x40, "Init__4CFooFv", unit="game/a"), f3(0x2000, 0x80, unit="game/b"),
              f3(0x3000, 0x20, unit="game/c")]
        b2 = [f2(0x10, 0x40, "CFoo::Init", file="src/a.cpp"), f2(0x50, 0x80, "CFoo::Update", file="src/b.cpp"),
              f2(0x90, 0x20, "CFoo::Exit", file="src/c.cpp")]
        rows = match(b3, b2, [[0x1000, 0x2000, 0x3000]], {"CFoo": [0x10, 0x50, 0x90]})
        self.assertEqual(rows[0x2000].fn2.name, "CFoo::Update")
        self.assertEqual(rows[0x3000].kinds, {"vtable"})


if __name__ == "__main__":
    unittest.main()
