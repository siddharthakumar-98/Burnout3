#include "vehicle_physics.h"

/* The shared cache's three fields retain their distinct original symbols. */
extern VehicleTriangleCache D_0066C580;
extern VehicleCachedTriangle *D_0066C584[];
extern u16 *D_0066C588[];
extern VehicleCachedTriangle D_0066C590[];
extern u16 D_0066DD90[];
extern "C" void func_001AD380(VehiclePhysics *);

extern "C" void func_001AD850(VehiclePhysics *self)
{
    self->bodyType = 0;
    self->transform = 0;
    self->triangleCache = &D_0066C580;
    D_0066C580.count = 0;
    D_0066C584[0] = D_0066C590;
    D_0066C588[0] = D_0066DD90;
    func_001AD380(self);
}
