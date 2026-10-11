#include "vehicle_physics.h"

extern "C" void func_001CFD20(VehiclePhysics *);

/* Retail Init path: initialize the base vehicle, then select four wheels.
 * The historical filename predates verification of the base callee. */
extern "C" void func_001C6EF0(VehiclePhysics *self)
{
    func_001CFD20(self);
    self->state1169 = 4;
    self->state116A = 4;
    self->resource = NULL;
}
