"""Float-pool retargeting must preserve source values when draft instructions move."""
import struct
import tempfile
import unittest
from pathlib import Path
from unittest.mock import patch

import litfix


def literal_object(value):
    """Minimal ELF32 object: a GP load with addend 4 into a two-word literal section."""
    names = b"\0func_00100000\0literal\0"
    symbols = bytes(16) + struct.pack("<IIIBBH", 1, 0, 4, 0x12, 0, 1)
    symbols += struct.pack("<IIIBBH", 15, 0, 8, 0x11, 0, 3)
    sections = [(0, 0, 0, b"", 0, 0, 0),
                (1, 1, 6, struct.pack("<I", 0xC7800004), 0, 0, 0),
                (2, 9, 0, struct.pack("<II", 0, (2 << 8) | litfix.R_MIPS_LITERAL), 4, 1, 8),
                (3, 1, 2, bytes(4) + value, 0, 0, 0),
                (4, 2, 0, symbols, 5, 1, 16),
                (5, 3, 0, names, 0, 0, 0),
                (6, 3, 0, b"\0.text\0.rel.text\0.lit4\0.symtab\0.strtab\0.shstrtab\0", 0, 0, 0)]
    section_names = [0, 1, 7, 17, 23, 31, 39]
    data, headers = bytearray(52), []
    data[:7] = b"\x7fELF\x01\x01\x01"
    for number, kind, flags, content, link, info, entry_size in sections:
        while len(data) % 4:
            data.append(0)
        headers.append((section_names[number], kind, flags, 0, len(data), len(content), link, info, 4, entry_size))
        data += content
    offset = len(data)
    for header in headers:
        data += struct.pack("<10I", *header)
    struct.pack_into("<I", data, 0x20, offset)
    struct.pack_into("<HHH", data, 0x2E, 40, len(headers), 6)
    return bytes(data)


class LiteralValueTests(unittest.TestCase):
    def test_displaced_load_uses_value_not_instruction_position(self):
        pal = struct.pack("<f", 0.04)
        ntsc = struct.pack("<f", 1 / 29.97)
        loads = [(0x100100, "NTSC", ntsc), (0x100104, "PAL", pal)]
        self.assertEqual(litfix.literal_target(pal, 0x100100, loads), "PAL")

    def test_wrong_rounding_and_signed_zero_are_not_substituted(self):
        ntsc = struct.pack("<f", 1 / 29.97)
        self.assertIsNone(litfix.literal_target(struct.pack("<f", 1 / 30), 1, [(1, "NTSC", ntsc)]))
        self.assertIsNone(litfix.literal_target(struct.pack("<f", -0.0), 1, [(1, "ZERO", struct.pack("<f", 0.0))]))

    def test_duplicate_values_prefer_the_original_instruction(self):
        value = struct.pack("<f", 0.5)
        loads = [(10, "FIRST", value), (20, "SECOND", value)]
        self.assertEqual(litfix.literal_target(value, 20, loads), "SECOND")

    def test_global_variables_and_other_functions_do_not_supply_literals(self):
        with tempfile.TemporaryDirectory() as directory:
            asm = Path(directory) / "test.s"
            asm.write_text("glabel first\n"
                           " /* 100 00100100 00000000 */ lwc1 $f0, %gp_rel(D_004E0680)($gp)\n"
                           " /* 104 00100104 00000000 */ lwc1 $f0, %gp_rel(D_004E1400)($gp)\n"
                           "endlabel first\nglabel second\n"
                           " /* 108 00100108 00000000 */ lwc1 $f0, %gp_rel(D_004E0684)($gp)\n"
                           "endlabel second\n")
            rom = bytearray(litfix.LIT4_END - 0x100000)
            rom[litfix.LIT4_START - 0x100000:litfix.LIT4_START - 0x100000 + 8] = struct.pack("<ff", 0.5, 0.25)
            loads = litfix.pooled_loads(asm, rom, {})
            self.assertEqual([entry[1] for entry in loads["first"]], ["D_004E0680"])
            self.assertEqual([entry[1] for entry in loads["second"]], ["D_004E0684"])

    def rewrite_fixture(self, source_value):
        directory = tempfile.TemporaryDirectory()
        self.addCleanup(directory.cleanup)
        folder = Path(directory.name)
        obj, asm, rom = folder / "literal.o", folder / "original.s", folder / "original.rom"
        obj.write_bytes(literal_object(source_value))
        asm.write_text("glabel func_00100000\n"
                       " /* 0 00100000 00000000 */ lwc1 $f0, %gp_rel(D_004E0680)($gp)\n"
                       "endlabel func_00100000\n")
        rom.write_bytes(bytes(litfix.LIT4_START - 0x100000) + struct.pack("<f", 1 / 29.97))
        with patch.object(litfix, "ORIG_ROM", rom), patch.object(litfix, "known_addresses", return_value={}):
            result = litfix.fix(obj, asm)
        return obj, result

    def test_rewrite_relocation_clears_the_resolved_addend(self):
        obj, (fixed, warnings) = self.rewrite_fixture(struct.pack("<f", 1 / 29.97))
        elf = litfix.Elf(obj.read_bytes())
        self.assertEqual((fixed, warnings), (1, []))
        self.assertEqual(struct.unpack("<I", elf.data(1))[0], 0xC7800000)
        _, info = struct.unpack("<II", elf.data(2))
        self.assertEqual(info & 0xFF, litfix.R_MIPS_GPREL16)
        symbol = struct.unpack_from("<IIIBBH", elf.data(4), (info >> 8) * 16)
        self.assertEqual(elf.cstr(5, symbol[0]), "D_004E0680")

    def test_wrong_value_leaves_the_object_and_relocation_unchanged(self):
        wrong = struct.pack("<f", 1 / 30)
        obj, (fixed, warnings) = self.rewrite_fixture(wrong)
        self.assertEqual(fixed, 0)
        self.assertEqual(len(warnings), 1)
        self.assertEqual(obj.read_bytes(), literal_object(wrong))


if __name__ == "__main__":
    unittest.main()
