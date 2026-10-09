#include "vehicle_physics.h"

/* The six pointers belong to the parent pool at +0xAE0, preserving the
 * original relocation; the pool implementation remains assembly. */
struct VehicleResourcePool {
    u8 entries[0xAE0];
    void *active[6];
};
extern VehicleResourcePool D_01D9C350[];
extern "C" void func_00254090(void *);

extern "C" void func_001C0E80(VehiclePhysics *self)
{
    void *resource = self->resource;
    for (s32 i = 0; i < 6; i++) {
        if (D_01D9C350[0].active[i] == resource) {
            func_00254090(D_01D9C350[0].active[i]);
            D_01D9C350[0].active[i] = NULL;
            break;
        }
    }
    self->resource = NULL;
}
