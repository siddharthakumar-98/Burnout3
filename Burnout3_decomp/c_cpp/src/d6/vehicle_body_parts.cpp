#include "vehicle_physics.h"

extern u8 D_0055EF70[];
extern u8 D_01E78510[];
extern "C" VehiclePhysics *func_001B86A0(void *, VehiclePhysics *, s32, s32);
extern "C" void func_00333990(VehiclePhysics *);
extern "C" void func_0027D240(void *, const Vec4 *, s32, f32);

extern "C" void func_001CBCE0(VehiclePhysics *self)
{
    u8 *flag = self->bodyState + 0x1023;
    if (!*flag && *(s8 **)(self->bodyState + 0x1024)) {
        *flag = 1;
        s32 i = 0;
        while (i < **(s8 **)(self->bodyState + 0x1024)) {
            VehiclePhysics *part = func_001B86A0(D_0055EF70, self, i, 2);
            if (!part) {
                break;
            }
            func_00333990(part);
            ++i;
        }
    }
}

extern "C" void func_001CBD80(VehiclePhysics *self, s32 index)
{
    s8 *state = (s8 *)(index + (s32)self->bodyState) + 0x4B2;
    if (*state == 2) {
        *state = 3;
        VehiclePhysics *part = func_001B86A0(D_0055EF70, self, index, 1);
        if (part) {
            func_00333990(part);
            func_0027D240(D_01E78510, &part->transform[3], index, 100.0f);
        } else {
            self->bodyState[0x4B2 + index] = 2;
            return;
        }
    }
}
