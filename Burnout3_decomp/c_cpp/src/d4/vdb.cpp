/* d4/vdb (0x21B8C0-0x21C010): tuning-variable registry. C++: virtual calls through a C function-pointer struct load
 * the slot into $v0, the original uses $t9, which only a real virtual call gives. The functions keep their
 * unmangled func_ names (extern "C") until the class and its methods are named. */

#include "vdb.h"

extern "C" {

char *strcpy(char *dst, const char *src);
char *strcat(char *dst, const char *src);
u32 strlen(const char *s);
int sprintf(char *buf, const char *fmt, ...);
void *memset(void *dst, int c, u32 n);

void vdbDbInit(void *p);
s32 packName40(const char *s);

extern void *gVdbDatabase;
extern s32 D_004E291C;
extern char D_004B9C88[]; /* "/" */
extern char D_004B9C90[]; /* "GtCommVa%04d" */
extern u32 crc32Table[256]; /* CRC-32 table */

void vdbSetDatabase(void *db, s32 x)
{
    gVdbDatabase = db;
    vdbDbInit((u8 *)db + 0x10);
    D_004E291C = x;
}

void vdbFreeEntry(VdbRegistry *reg, VdbEntry *entry)
{
    entry->type = VDB_NONE;
    reg->count--;
}

void vdbUnregister(VdbRegistry *reg, void *dest)
{
    VdbEntry *entry;
    s32 i = 0;
    s32 found = 0;

    while (found < reg->count && i < reg->capacity) {
        entry = &reg->entries[i];
        if (entry->type != VDB_NONE) {
            if (entry->dest == dest) {
                goto done;
            }
            found++;
        }
        i++;
    }
    entry = NULL;
done:
    if (entry != NULL) {
        reg->remove(entry);
    }
}

void vdbRegisterVector(VdbRegistry *reg, void *dest, const char *name, const char *group, const char *cfg)
{
    char tag[16];
    char path[512];
    s32 key;
    s32 i;
    VdbEntry *entry;

    strcpy(path, name);
    strcat(path, group);
    if (path[strlen(path) - 1] != '/') {
        strcat(path, D_004B9C88);
    }
    strcat(path, cfg);
    key = vdbHash(path);

    i = reg->next;
    while (reg->entries[i].type != VDB_NONE) {
        if (++i >= reg->capacity) {
            i = 0;
        }
    }
    reg->next = i;
    sprintf(tag, D_004B9C90, i);
    packName40(tag);

    entry = &reg->entries[i];
    entry->name = name;
    entry->group = group;
    entry->cfg = cfg;
    entry->key = key;
    entry->flags = 0;
    reg->count++;
    entry->type = VDB_VECTOR;
    entry->dest = dest;

    reg->apply(entry);
    if (reg->isEnabled(cfg) == 1) {
        reg->add(entry);
    } else {
        entry->type = VDB_NONE;
        reg->count--;
    }
}

void vdbRegisterFloat(VdbRegistry *reg, f32 *dest, const char *name, const char *group, const char *cfg, u8 flags,
                   u8 count, f32 min, f32 max)
{
    char tag[16];
    char path[512];
    s32 key;
    s32 i;
    VdbEntry *entry;

    strcpy(path, name);
    strcat(path, group);
    if (path[strlen(path) - 1] != '/') {
        strcat(path, D_004B9C88);
    }
    strcat(path, cfg);
    key = vdbHash(path);

    i = reg->next;
    while (reg->entries[i].type != VDB_NONE) {
        if (++i >= reg->capacity) {
            i = 0;
        }
    }
    reg->next = i;
    sprintf(tag, D_004B9C90, i);
    packName40(tag);

    entry = &reg->entries[i];
    entry->name = name;
    entry->group = group;
    entry->cfg = cfg;
    entry->key = key;
    entry->flags = flags;
    reg->count++;
    entry->type = VDB_FLOAT;
    entry->dest = dest;
    entry->count = count;
    entry->min.f = min;
    entry->max.f = max;
    if (max != 0.0f || min != 0.0f) {
        entry->flags |= VDB_HAS_RANGE;
    }

    reg->apply(entry);
    if (reg->isEnabled(cfg) == 1) {
        reg->add(entry);
    } else {
        entry->type = VDB_NONE;
        reg->count--;
    }
}

void vdbRegisterInt(VdbRegistry *reg, s32 *dest, const char *name, const char *group, const char *cfg, u8 flags,
                   s32 min, s32 max, s32 count)
{
    char tag[16];
    char path[512];
    s32 key;
    s32 i;
    VdbEntry *entry;

    strcpy(path, name);
    strcat(path, group);
    if (path[strlen(path) - 1] != '/') {
        strcat(path, D_004B9C88);
    }
    strcat(path, cfg);
    key = vdbHash(path);

    i = reg->next;
    while (reg->entries[i].type != VDB_NONE) {
        if (++i >= reg->capacity) {
            i = 0;
        }
    }
    reg->next = i;
    sprintf(tag, D_004B9C90, i);
    packName40(tag);

    entry = &reg->entries[i];
    entry->name = name;
    entry->group = group;
    entry->cfg = cfg;
    entry->key = key;
    entry->flags = flags;
    reg->count++;
    entry->type = VDB_INT;
    entry->dest = dest;
    entry->count = count;
    entry->min.i = min;
    entry->max.i = max;
    if (max != 0 || min != 0) {
        entry->flags |= VDB_HAS_RANGE;
    }

    reg->apply(entry);
    if (reg->isEnabled(cfg) == 1) {
        reg->add(entry);
    } else {
        entry->type = VDB_NONE;
        reg->count--;
    }
}

void vdbInit(VdbRegistry *reg, VdbEntry *entries, s32 capacity)
{
    reg->entries = entries;
    reg->capacity = capacity;
    reg->next = 0;
    memset(reg->entries, 0, capacity << 5);
}

s32 vdbHash(const char *str)
{
    s32 n = strlen(str);
    s32 crc = -1;
    s32 i;

    for (i = 0; i < n; i++) {
        crc = (crc >> 8) ^ crc32Table[(signed char)*str++ ^ (crc & 0xFF)];
    }
    return crc;
}

} /* extern "C" */
