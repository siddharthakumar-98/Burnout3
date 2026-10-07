/* Extra linker script, passed after splat's generated one. */
ENTRY(_start);

/* Absolute offsets used by config/reloc_addrs.txt (not addresses). */
ofs_00122A10 = 0x00122A10;
ofs_00122B1C = 0x00122B1C;
ofs_00125F68 = 0x00125F68;
ofs_0012ACE0 = 0x0012ACE0;
ofs_0012AD18 = 0x0012AD18;
ofs_0012AD24 = 0x0012AD24;
ofs_0012C0E0 = 0x0012C0E0;
ofs_00143840 = 0x00143840;

/* CodeWarrior's soft-float helper names, which the original linker resolved to libgcc's functions. Only the ones
   game code is seen calling are listed (a C unit calling another one fails to link, rather than linking wrongly). */
dptoli = __fixdfsi;
dptofp = __truncdfsf2;
fptodp = __extendsfdf2;
