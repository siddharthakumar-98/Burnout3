#include "vehicle_physics.h"

extern u8 D_004E2704;

extern "C" s32 func_001AB090(VehiclePhysics *self, const VehicleCollisionTriangle *triangle)
{
    if (!D_004E2704) {
        s32 count = self->triangleCache->count;
        if (count < 96) {
            VehicleCachedTriangle *destination = &self->triangleCache->triangles[count];
            destination->vertices.value[0].bits = triangle->vertices.value[0].bits;
            destination->vertices.value[1].bits = triangle->vertices.value[1].bits;
            destination->vertices.value[2].bits = triangle->vertices.value[2].bits;
            destination->normal.bits = triangle->normal.bits;
            self->triangleCache->tags[count] = triangle->surface->tag;
            self->triangleCache->count = count + 1;
            return 1;
        }
    }
    D_004E2704 = 1;
    return 0;
}
