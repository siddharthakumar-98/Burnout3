#include "vehicle_physics.h"

extern "C" void func_001CBE50(VehiclePhysics *self, s32 index)
{
    s8 *state = (s8 *)(index + (s32)self->bodyState) + 0x4B2;
    /* Signed byte: negative/sentinel states must remain unchanged. */
    if (*state == 0 || *state == 1) {
        *state = 2;
    }
}
