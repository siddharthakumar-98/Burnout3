#ifndef VEHICLE_PHYSICS_H
#define VEHICLE_PHYSICS_H

#include "transmission.h"
#include "vu0.h"

union VehicleVector {
    __int128 bits;
    Vec4 value;
};

/* Partial retail views: offsets checked against their owning functions.
 * Neutral field names mark identities not yet established from the beta. */
struct RaceCarBoostState {
    u8 unknown0000[0x10CC];
    f32 time10CC; /* +0x10CC */
    u8 unknown10D0[0xE0];
    f32 time11B0; /* +0x11B0 */
    u8 unknown11B4[0x2D];
    u8 flag11E1; /* +0x11E1 */
};

struct VehiclePhysics {
    u32 vtable; /* +0x0 */
    u8 unknown0004[0xAC];
    VehicleVector speedSample; /* +0xB0 */
    u8 unknown00C0[0x144];
    Vec4 *transform; /* +0x204 */
    u8 unknown0208[0x8];
    u8 flag210; /* +0x210 */
    u8 unknown0211[0xAB3];
    u8 *bodyState; /* +0xCC4 */
    u8 unknown0CC8[0x4A1];
    u8 state1169; /* +0x1169 */
    u8 state116A; /* +0x116A */
    u8 unknown116B[0x269];
    f32 boostMaxSpeed; /* +0x13D4 */
    u8 unknown13D8[0x1C];
    RaceCarBoostState *raceCar; /* +0x13F4 */
    void *resource; /* +0x13F8 */
    u8 unknown13FC[0x4C];
    CTransmission transmission; /* +0x1448 */
    u8 unknown14DC[0x94];
    f32 deadline; /* +0x1570 */
};

extern "C" {
void func_001C0E80(VehiclePhysics *);
void func_001C5040(VehiclePhysics *);
void func_001C6EF0(VehiclePhysics *);
void func_001CBE50(VehiclePhysics *, s32);
void func_001CC4C0(VehiclePhysics *, u32, void *);
void func_001CC520(VehiclePhysics *, u32, void *);
void func_001D17B0(VehiclePhysics *, f32);
f32 func_001C6F90(CTransmission *, f32, f32, s32, s32);
}

#endif
