/* game/unit_0024FDE0 (0x24FDE0-0x250160): the tuning database, VdbRegistry's derived class that applies the values
 * of Data/vdb.xml (layout in tools/vdbhash.py). Its virtual methods keep their func_ names: they are written as
 * extern "C" functions taking `self` (same code as methods), and the vtable is emitted as plain data. */

#include "vdb.h"

extern "C" {

void *memcpy(void *dst, const void *src, u32 n);

void vdbDbAdd(VdbDatabase *self, VdbEntry *entry);
void vdbDbRemove(VdbDatabase *self, VdbEntry *entry);
void vdbDbApply(VdbDatabase *self, VdbEntry *entry);
u8 vdbDbIsEnabled(VdbDatabase *self, const char *cfg);

/* vtable: MW header, add, remove, apply, isEnabled */
void *vtVdbDatabase[8] = {
    0, 0, (void *)vdbDbAdd, (void *)vdbDbRemove, (void *)vdbDbApply, (void *)vdbDbIsEnabled, 0, 0,
};

/* isEnabled: is the .cfg file that defined a variable switched on in the database (unknown files are) */
u8 vdbDbIsEnabled(VdbDatabase *self, const char *cfg)
{
    s32 hash = vdbHash(cfg);
    VdbFile *file;
    s32 lo = -1;
    s32 hi = self->nFiles;

    while (hi - lo > 1) {
        s32 mid = (hi + lo) / 2;
        file = &self->files[mid];
        if (file->hash == hash) {
            return file->enabled != 0;
        }
        if (hash < file->hash) {
            hi = mid;
        } else {
            lo = mid;
        }
    }
    return 1;
}

/* load: point the tables into the loaded file, apply every variable, drop those whose file is disabled */
void vdbDbLoad(VdbDatabase *self)
{
    u8 *data = self->data;
    s32 i = 0;
    s32 found = 0;
    s32 count;
    VdbEntry *entry;

    self->nValues = ((s32 *)data)[1];
    self->values = (VdbFileValue *)(data + 0x14);
    self->nFiles = ((s32 *)data)[3];
    self->files = (VdbFile *)(data + ((s32 *)data)[4]);

    count = self->count;
    while (found < count && i < self->capacity) {
        entry = &self->table[i];
        if (entry->type != VDB_NONE) {
            self->apply(entry);
            if (self->isEnabled(entry->cfg) == 0) {
                vdbFreeEntry(self, entry);
            }
            found++;
        }
        i++;
    }
}

/* apply: copy a variable's value from the database into it */
void vdbDbApply(VdbDatabase *self, VdbEntry *entry)
{
    s32 key = entry->key;
    VdbFileValue *value;
    s32 lo = -1;
    s32 hi = self->nValues;

    while (hi - lo > 1) {
        s32 mid = (hi + lo) / 2;
        value = &self->values[mid];
        if (value->key == key) {
            goto found;
        }
        if (key < value->key) {
            hi = mid;
        } else {
            lo = mid;
        }
    }
    value = NULL;
found:
    if (value != NULL) {
        if (entry->count < 2) {
            switch (entry->type) {
            case VDB_INT:
            case VDB_FLOAT:
                *(u32 *)entry->dest = value->value;
                break;
            case VDB_BOOL:
                *(u8 *)entry->dest = value->value != 0;
                break;
            case VDB_VECTOR:
                *(VdbVector *)entry->dest = *(VdbVector *)&self->data[value->value];
                break;
            }
        } else {
            memcpy(entry->dest, &self->data[value->value], entry->count * 4);
        }
    }
}

/* remove */
void vdbDbRemove(VdbDatabase *self, VdbEntry *entry)
{
    vdbFreeEntry(self, entry);
}

/* add */
void vdbDbAdd(VdbDatabase *self, VdbEntry *entry)
{
}

/* init, from vdbSetDatabase */
void vdbDbInit(VdbDatabase *self)
{
    vdbInit(self, self->table, 0x6C8);
    self->unkC1C = 0;
    self->unkC14 = 0;
    self->unkC18 = 0;
}

}
