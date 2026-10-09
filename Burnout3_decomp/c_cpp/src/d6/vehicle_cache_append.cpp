#include "vehicle_physics.h"

extern u8 D_004E2704;

extern "C" s32 func_001AB090(VehiclePhysics *self, const VehicleCollisionTriangle *triangle)
{
    if (!D_004E2704) {
        s32 count = self->triangleCache->count;
        if (count < 96) {
            VehicleCachedTriangle *destination = &self->triangleCache->triangles[count];
            __int128 normal = triangle->normal.bits;
            __int128 vertex0 = triangle->vertices.value[0].bits;
            __int128 vertex1 = triangle->vertices.value[1].bits;
            __int128 vertex2 = triangle->vertices.value[2].bits;
            destination->vertices.value[0].bits = vertex0;
            destination->vertices.value[1].bits = vertex1;
            destination->vertices.value[2].bits = vertex2;
            destination->normal.bits = normal;
            self->triangleCache->tags[count] = triangle->surface->tag;
            self->triangleCache->count = count + 1;
            return 1;
        }
    }
    D_004E2704 = 1;
    return 0;
}
