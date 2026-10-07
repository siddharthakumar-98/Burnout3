#ifndef VDB_H
#define VDB_H

#include "types.h"

/* Tuning-variable registry (d4/vdb, 0x21B8C0). Variables are registered with a name, a group and the .cfg path
 * that defined them; the key is hash(name + group + "/" + cfg) (tools/vdbhash.py). Names are ours. */

#ifdef __cplusplus
extern "C" {
#endif

enum {
    VDB_NONE = 0, /* free slot */
    VDB_INT = 1,
    VDB_FLOAT = 2,
    VDB_BOOL = 3,
    VDB_VECTOR = 5
};

#define VDB_HAS_RANGE 0x01 /* flags: min or max is nonzero */

typedef union VdbValue {
    s32 i;
    f32 f;
} VdbValue;

typedef struct VdbEntry {
    void *dest;        /* 0x00 the variable */
    VdbValue max;      /* 0x04 */
    VdbValue min;      /* 0x08 */
    const char *name;  /* 0x0C */
    const char *group; /* 0x10 */
    const char *cfg;   /* 0x14 */
    s32 key;           /* 0x18 */
    u8 count;          /* 0x1C */
    u8 flags;          /* 0x1D */
    u8 type;           /* 0x1E VDB_NONE when free */
    u8 pad;            /* 0x1F */
} VdbEntry;            /* 0x20 */

#ifdef __cplusplus
}

/* The registry is a C++ class (its virtual calls load the slot into $t9); its derived class lives in valuedb.
 * The vtable's first two words are MW's header, so the first virtual sits at +0x8. */
class VdbRegistry {
public:
    virtual void add(VdbEntry *entry);              /* vtable +0x08 */
    virtual void remove(VdbEntry *entry);           /* vtable +0x0C */
    virtual void apply(VdbEntry *entry);            /* vtable +0x10 */
    virtual u8 isEnabled(const char *cfg);          /* vtable +0x14 */

    /* 0x00 vtable */
    s32 next;          /* 0x04 where the search for a free slot starts */
    s32 count;         /* 0x08 slots in use */
    VdbEntry *entries; /* 0x0C */
    s32 capacity;      /* 0x10 */
};

/* Data/vdb.xml's tables (tools/vdbhash.py) */
typedef struct VdbFileValue {
    u32 value; /* the variable, or for vectors and arrays an offset into the file */
    s32 key;
} VdbFileValue;

typedef struct VdbFile {
    u32 enabled;
    s32 hash; /* vdbHash(cfg path) */
} VdbFile;

typedef __int128 VdbVector;

/* The database (game/unit_0024FDE0, vtable 0x4DE0F0): the registry with its own entry table and the loaded file. */
class VdbDatabase : public VdbRegistry {
public:
    u8 unk14[0xC14 - 0x14];
    s32 unkC14;                /* 0xC14 */
    s32 unkC18;                /* 0xC18 */
    u8 unkC1C;                 /* 0xC1C */
    VdbEntry table[0x6C8];     /* 0xC20 */
    s32 nValues;               /* 0xE520 */
    VdbFileValue *values;      /* 0xE524 sorted by key */
    s32 nFiles;                /* 0xE528 */
    VdbFile *files;            /* 0xE52C sorted by hash */
    u8 data[1];                /* 0xE530 the file */
};

extern "C" {
/* d4/valuedb: binds a newly loaded file and re-applies every registered variable */
void vdbDbLoad(VdbDatabase *self);
#else
typedef struct VdbRegistry VdbRegistry;
#endif

void vdbSetDatabase(void *db, s32 x);
void vdbFreeEntry(VdbRegistry *reg, VdbEntry *entry);
void vdbUnregister(VdbRegistry *reg, void *dest);
void vdbRegisterVector(VdbRegistry *reg, void *dest, const char *name, const char *group, const char *cfg);
void vdbRegisterFloat(VdbRegistry *reg, f32 *dest, const char *name, const char *group, const char *cfg, u8 flags,
                   u8 count, f32 min, f32 max);
void vdbRegisterInt(VdbRegistry *reg, s32 *dest, const char *name, const char *group, const char *cfg, u8 flags,
                   s32 min, s32 max, s32 count);
void vdbInit(VdbRegistry *reg, VdbEntry *entries, s32 capacity);
s32 vdbHash(const char *str);

#ifdef __cplusplus
}
#endif

#endif
