#include "vehicle_physics.h"

/* Incomplete array forces the original lui/addiu addressing. */
extern f32 D_00522680[];

extern "C" void func_001D17B0(VehiclePhysics *self, f32 duration)
{
    self->deadline = duration + D_00522680[0];
}
