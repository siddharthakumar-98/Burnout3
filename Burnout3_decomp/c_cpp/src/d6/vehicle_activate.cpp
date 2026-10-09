#include "vehicle_physics.h"

extern "C" void func_001ABE40(VehiclePhysics *self, u8 state)
{
    if (self->flag210) {
        return;
    }
    self->state218 = state;
    self->flag210 = 1;
    self->flag20E = 0;
}
