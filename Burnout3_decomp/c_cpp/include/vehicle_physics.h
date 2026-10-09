#ifndef VEHICLE_PHYSICS_H
#define VEHICLE_PHYSICS_H

#include "transmission.h"
#include "vu0.h"

union VehicleVector {
    __int128 bits;
    Vec4 value;
};

struct VehicleTriangleVertices {
    VehicleVector value[3];
};

struct VehicleCachedTriangle {
    VehicleTriangleVertices vertices;
    VehicleVector normal;
};

struct VehicleTriangleCache {
    s32 count; /* +0x0, capacity 96 */
    VehicleCachedTriangle *triangles; /* +0x4 */
    u16 *tags; /* +0x8, separate from the normal's fourth word */
};

struct VehicleCollisionSurface {
    u8 unknown00[4];
    u16 tag; /* +0x4 */
};

struct VehicleCollisionTriangle {
    u8 unknown00[0x10];
    VehicleVector normal; /* +0x10 */
    VehicleTriangleVertices vertices; /* +0x20 */
    u8 unknown50[0x10];
    VehicleCollisionSurface *surface; /* +0x60 */
};

typedef char VehicleCachedTriangleSizeCheck[sizeof(VehicleCachedTriangle) == 0x40 ? 1 : -1];

/* Partial retail views: offsets checked against their owning functions.
 * Neutral field names mark identities not yet established from the beta. */
struct RaceCarBoostState {
    u8 unknown0000[0x10CC];
    f32 time10CC; /* +0x10CC */
    u8 unknown10D0[0xE0];
    f32 time11B0; /* +0x11B0 */
    u8 unknown11B4[0x2D];
    u8 flag11E1; /* +0x11E1 */
    u8 unknown11E2[0x732];
    s32 state1914; /* +0x1914 */
};

struct VehiclePhysics {
    u32 vtable; /* +0x0 */
    u8 unknown0004[0xAC];
    VehicleVector speedSample; /* +0xB0 */
    u8 unknown00C0[0x110];
    VehicleVector queryExtents; /* +0x1D0 */
    u8 unknown01E0[0x20];
    VehicleTriangleCache *triangleCache; /* +0x200 */
    Vec4 *transform; /* +0x204 */
    u8 unknown0208[6];
    u8 flag20E; /* +0x20E */
    u8 unknown020F;
    u8 flag210; /* +0x210 */
    u8 unknown0211[5];
    s8 currentUnit; /* +0x216, -1 is a valid no-unit result */
    s8 collisionWorld; /* +0x217 */
    u8 state218; /* +0x218 */
    u8 unknown0219[0xAAB];
    u8 *bodyState; /* +0xCC4 */
    u8 unknown0CC8[0x4A1];
    u8 state1169; /* +0x1169 */
    u8 state116A; /* +0x116A */
    u8 flag116B; /* +0x116B */
    f32 initialHeight; /* +0x116C */
    u8 unknown1170[0x60];
    VehicleCachedTriangle extraTriangles[6]; /* +0x11D0 */
    u8 unknown1350;
    u8 extraTrianglesEnabled; /* +0x1351 */
    u8 unknown1352[0x82];
    f32 boostMaxSpeed; /* +0x13D4 */
    u8 unknown13D8[0x1C];
    RaceCarBoostState *raceCar; /* +0x13F4 */
    void *resource; /* +0x13F8 */
    u8 unknown13FC[0x4C];
    CTransmission transmission; /* +0x1448 */
    u8 unknown14DC[0x94];
    f32 deadline; /* +0x1570 */
};

/* These also guard offsets used by the inline quadword translation getter. */
typedef char VehicleTransformOffsetCheck[(u32)&((VehiclePhysics *)0)->transform == 0x204 ? 1 : -1];
typedef char VehicleCacheOffsetCheck[(u32)&((VehiclePhysics *)0)->triangleCache == 0x200 ? 1 : -1];
typedef char VehicleExtraTrianglesOffsetCheck[(u32)&((VehiclePhysics *)0)->extraTriangles == 0x11D0 ? 1 : -1];
typedef char VehicleTransmissionOffsetCheck[(u32)&((VehiclePhysics *)0)->transmission == 0x1448 ? 1 : -1];
typedef char VehicleRaceStateOffsetCheck[(u32)&((RaceCarBoostState *)0)->state1914 == 0x1914 ? 1 : -1];

extern "C" {
s32 func_001AB090(VehiclePhysics *, const VehicleCollisionTriangle *);
void func_001ABE40(VehiclePhysics *, u8);
void func_001C0E80(VehiclePhysics *);
void func_001C5040(VehiclePhysics *);
void func_001C5080(VehiclePhysics *, s32);
s32 func_001C5330(const VehicleCollisionTriangle *, VehiclePhysics *);
void func_001C6EF0(VehiclePhysics *);
void func_001CBC70(VehiclePhysics *, u8);
void func_001CBCE0(VehiclePhysics *);
void func_001CBD80(VehiclePhysics *, s32);
void func_001CBE50(VehiclePhysics *, s32);
void func_001CC4C0(VehiclePhysics *, u32, void *);
void func_001CC520(VehiclePhysics *, u32, void *);
void func_001D17B0(VehiclePhysics *, f32);
f32 func_001C6F90(CTransmission *, f32, f32, s32, s32);
}

#endif
