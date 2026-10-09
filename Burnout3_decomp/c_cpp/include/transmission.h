#ifndef TRANSMISSION_H
#define TRANSMISSION_H

#include "types.h"

struct VehiclePhysics;

/* CTransmission correspondence: Burnout 2's ratios/RPM prefix and update
 * algorithm, checked against 0x1C6F90/0x1C7880. Burnout 3 adds boost torque
 * fields and replaces the large beta CRandom with two 32-bit words. */
class CTransmission {
public:
    f32 mnGearRatioReverseValue;       /* 0x00 */
    f32 maGearRatioValues[7];          /* 0x04 */
    f32 mnGearRatioFinalValue;         /* 0x20 */
    f32 mEngineIdleRPMValue;            /* 0x24 */
    f32 mChangeUpRPMValue;              /* 0x28 */
    f32 mChangeDownRPMValue;            /* 0x2C */
    u8 unknown30[8];
    f32 mEngineMaxRPMValue;             /* 0x38 */
    f32 mEngineTorqueValue;             /* 0x3C */
    f32 mrMaxRevs;                     /* 0x40 */
    f32 mrPeakTorqueRevs;              /* 0x44 */
    f32 mrFallOffTorqueRevs;            /* 0x48 */
    f32 boostTorqueScale;              /* 0x4C */
    f32 boostTorqueWindow;             /* 0x50 */
    f32 mrEngineAngularVel;            /* 0x54 */
    f32 mrClutchReleaseTime;            /* 0x58 */
    s32 mbClutchDown;                  /* 0x5C */
    s32 mbLockInGear;                  /* 0x60 */
    u8 unknown64[4];
    u32 randomState;                   /* 0x68 */
    u32 randomCarry;                   /* 0x6C */
    s32 mbManual;                      /* 0x70 */
    s32 mbManualSwitched;              /* 0x74 */
    s32 mbGearUpSwitched;              /* 0x78 */
    s32 mbGearDownSwitched;            /* 0x7C */
    s32 mnGear;                        /* 0x80 */
    s32 mnTopGear;                     /* 0x84 */
    f32 mrLastChangeDownTime;           /* 0x88 */
    f32 mrLastChangeUpTime;             /* 0x8C */
    VehiclePhysics *physics;           /* 0x90 */
};

typedef char TransmissionSizeCheck[sizeof(CTransmission) == 0x94 ? 1 : -1];

/* Keep address names: the beta has separate Reset/Setup methods, while this
 * retail routine initializes already-loaded tuning and resets it twice. */
extern "C" s32 func_001C7880(CTransmission *, VehiclePhysics *);
/* All three callers store the selected gear from v0 into vehicle +0x14C8. */
extern "C" s32 func_001C6F30(CTransmission *, f32, f32);

#endif
