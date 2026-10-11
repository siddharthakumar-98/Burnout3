#include "vehicle_physics.h"

extern f32 D_004E0DC4; /* surface normal rejection threshold, -0.7 */

/* The vector library carries scalar results in the low word of a quadword. */
struct CollisionScalar {
    union { __int128 bits; Vec4 value; };
    CollisionScalar() {}
    CollisionScalar(const CollisionScalar &other) { bits = other.bits; }
    CollisionScalar &operator=(const CollisionScalar &other)
    {
        bits = other.bits;
        return *this;
    }
    inline operator f32() const
    {
        register __int128 value = bits;
        register f32 result;
        asm volatile { mtc1 value, result }
        return result;
    }
};

/* The multiply stage leaves xyz products in vf2 and returns scalar one
 * for the VU0 horizontal sum. Both stages are always inlined together. */
static inline CollisionScalar collisionProducts(register const Vec4 *a, register const Vec4 *b)
{
    CollisionScalar scalar;
    register __int128 bits;
    asm volatile {
        lqc2 vf3, 0(a)
        lqc2 vf2, 0(b)
        vaddw.x vf1, vf0, vf0w
        vmul.xyz vf2, vf3, vf2
        qmfc2.ni bits, vf1
    }
    scalar.bits = bits;
    return scalar;
}

static inline CollisionScalar collisionDot(register const Vec4 *a, register const Vec4 *b)
{
    register __int128 unit = collisionProducts(a, b).bits;
    register __int128 bits;
    CollisionScalar result;
    asm volatile {
        vadday.x ACC, vf2, vf2y
        qmtc2.ni unit, vf1
        vmaddz.x vf1, vf1, vf2z
        qmfc2.ni bits, vf1
    }
    result.bits = bits;
    return result;
}

static inline VehicleVector triangleNormal(register const VehicleCollisionTriangle *triangle)
{
    VehicleVector normal;
    register __int128 bits = triangle->normal.bits;
    register VehicleVector *destination = &normal;
    asm volatile { sq bits, 0(destination) }
    return normal;
}

static inline f32 triangleNormalY(const VehicleCollisionTriangle *triangle)
{
    return triangleNormal(triangle).value.y;
}

static inline f32 normalY(const VehicleCollisionTriangle *triangle)
{
    return triangleNormalY(triangle);
}

/* Retained draft: 99.69%; only the normal-y/cutoff FPR allocation differs. */
extern "C" s32 func_001C5330(const VehicleCollisionTriangle *triangle, VehiclePhysics *self)
{
    u16 tag = triangle->surface->tag;
    s32 surface = tag & 0xFF;
    bool directional;
    if (surface == 0x23 || surface == 0x22 || ((tag & 0x1000) != 0) & 1) goto reject;
    if (surface < 0x15 || surface > 0x20) goto nondirectional;
    directional = true;
    goto checkDirection;
reject:
    return 1;
checkDirection:
    if (directional) goto dot;
checkNormal:
    {
        f32 cutoff = D_004E0DC4;
        if (normalY(triangle) < cutoff) return 1;
        goto append;
    }
nondirectional:
    directional = false;
    goto checkDirection;
dot:
    if (collisionDot(&self->speedSample.value, &triangle->normal.value) <= 0.5f) goto checkNormal;
    return 1;
append:
    return func_001AB090(self, triangle);
}
