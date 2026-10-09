#include "vehicle_physics.h"

extern "C" void func_001CEE10(VehiclePhysics *);

extern "C" void func_001C5040(VehiclePhysics *self)
{
    if (self->flag210) {
        func_001CEE10(self);
    }
}
