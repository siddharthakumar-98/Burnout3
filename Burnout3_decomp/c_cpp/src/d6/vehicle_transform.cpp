#include "vehicle_physics.h"

/* Transpose the orthonormal rotation and rotate/negate its translation.
 * The VU implementation preserves each rotation row's fourth component. */
static inline void inverseOrthonormal(register VehicleMatrix *matrix)
{
    asm {
        lqc2 vf4, 0x10(matrix)
        lqc2 vf2, 0x00(matrix)
        lqc2 vf3, 0x20(matrix)
        vaddx.y vf2, vf0, vf4x
        lqc2 vf1, 0x00(matrix)
        vaddx.z vf2, vf0, vf3x
        sqc2 vf2, 0x00(matrix)
        lqc2 vf2, 0x10(matrix)
        vaddy.x vf2, vf0, vf1y
        vaddy.z vf2, vf0, vf3y
        sqc2 vf2, 0x10(matrix)
        lqc2 vf2, 0x20(matrix)
        vaddz.x vf2, vf0, vf1z
        vaddz.y vf2, vf0, vf4z
        sqc2 vf2, 0x20(matrix)
        lqc2 vf3, 0x30(matrix)
        lqc2 vf1, 0x00(matrix)
        vmulz.xyz vf4, vf2, vf3z
        vmulx.xyz vf2, vf1, vf3x
        lqc2 vf1, 0x10(matrix)
        vmuly.xyz vf1, vf1, vf3y
        vadd.xyz vf1, vf2, vf1
        vadd.xyz vf1, vf1, vf4
        vsub.xyz vf1, vf0, vf1
        sqc2 vf1, 0x30(matrix)
    }
}

extern "C" void func_001AB610(VehiclePhysics *self, const VehicleMatrix *transform)
{
    VehicleMatrix *destination = (VehicleMatrix *)self->transform;
    __int128 right = transform->right.bits;
    __int128 up = transform->up.bits;
    __int128 at = transform->at.bits;
    __int128 position = transform->position.bits;
    destination->right.bits = right;
    destination->up.bits = up;
    destination->at.bits = at;
    destination->position.bits = position;
    self->inverseTransform.right.bits = right;
    self->inverseTransform.up.bits = up;
    self->inverseTransform.at.bits = at;
    self->inverseTransform.position.bits = position;
    inverseOrthonormal(&self->inverseTransform);
}
