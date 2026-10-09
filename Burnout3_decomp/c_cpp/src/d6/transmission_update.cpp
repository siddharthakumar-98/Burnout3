#include "vehicle_physics.h"

/* Separate reads preserve the original timer accesses between state stores. */
extern volatile f32 D_0052267C[]; /* game timestep in seconds */

static const f32 RPM_TO_RADS = 0.10471975803375244f;
static const f32 MPS_TO_MPH = 2.236936330795288f;

/* The original uses an arithmetic high-half shift, not a logical rotate.
 * Unsigned storage preserves wraparound without signed-overflow assumptions. */
static inline f32 RandomNorm(CTransmission *self)
{
    self->randomState = (self->randomState << 16) + ((s32)self->randomState >> 16);
    self->randomState += self->randomCarry;
    self->randomCarry += self->randomState;
    return (f32)self->randomState * 2.3283064365386963e-10f;
}

/* Reverse is a separate member before the forward array. Address through the
 * object's byte view so gear -1 does not index outside a C++ array. */
static inline f32 GearRatio(CTransmission *self, s32 gear)
{
    return *(const f32 *)((const u8 *)self + 4 + gear * 4);
}

static inline f32 SpeedMPH(VehiclePhysics *physics)
{
    VehicleVector sample;
    sample.bits = physics->speedSample.bits;
    return MPS_TO_MPH * sample.value.w;
}

/* Burnout 2's CTransmission algorithm with retail offsets and control flow.
 * The fourth argument is the driving-control boost bit; the fifth doubles
 * torque, with the caller deriving it from race-car +0x178C == 0. */
extern "C" f32 func_001C6F90(CTransmission *self, f32 accelerator, f32 wheelSpeed,
                            s32 boost, s32 doubleTorque)
{
    if (boost && accelerator < 0.9f) {
        accelerator = 0.9f;
    }
    s32 gear = self->mnGear;
    if (gear == 1 && wheelSpeed < 0.0f) {
        wheelSpeed *= 100.0f;
    }
    /* Ratios include reverse at index -1, contiguous with the forward array. */
    f32 targetSpeed = self->mnGearRatioFinalValue * (wheelSpeed * GearRatio(self, gear));
    f32 idleSpeed = RPM_TO_RADS * self->mEngineIdleRPMValue;
    f32 changeDown = RPM_TO_RADS * self->mChangeDownRPMValue;
    f32 changeUp = RPM_TO_RADS * self->mChangeUpRPMValue;
    if (gear == 0) {
        targetSpeed = accelerator * (RPM_TO_RADS * self->mrMaxRevs);
    }

    if (self->mbClutchDown) {
        f32 clutchTime = self->mrClutchReleaseTime - D_0052267C[0];
        self->mrClutchReleaseTime = clutchTime;
        if (clutchTime > 0.0f) {
            accelerator *= 0.1f;
        } else {
            self->mbClutchDown = 0;
        }
    } else if (!self->mbManual) {
        self->mrLastChangeUpTime -= D_0052267C[0];
        self->mrLastChangeDownTime -= D_0052267C[0];
        if (targetSpeed < changeDown && self->mnGear >= 2 && accelerator < 0.5f) {
            goto changeDownTimer;
        }
        if (targetSpeed < changeDown * 0.75f && self->mnGear >= 2) {
        changeDownTimer:
            if (self->mrLastChangeUpTime <= 0.0f) {
                self->mnGear--;
                self->mbClutchDown = 1;
                self->mrClutchReleaseTime = 0.35f;
                self->mrLastChangeDownTime = 1.0f;
            }
        } else if (targetSpeed > changeUp && self->mnGear > 0 && self->mnGear < self->mnTopGear &&
                   !self->mbLockInGear && self->mrLastChangeDownTime <= 0.0f) {
            /* Preserve the old gear's ratio during an upshift. */
            f32 ratio = GearRatio(self, self->mnGear);
            self->mnGear++;
            self->mbClutchDown = 1;
            targetSpeed = self->mnGearRatioFinalValue * (wheelSpeed * ratio);
            if (boost) {
                self->mrClutchReleaseTime = 0.1f;
                self->mrLastChangeUpTime = 0.1f;
            } else {
                self->mrClutchReleaseTime = 0.35f;
                self->mrLastChangeUpTime = 1.0f;
            }
        }
    }

    gear = self->mnGear;
    f32 maxSpeed = RPM_TO_RADS * self->mrMaxRevs;
    if (gear == -1) {
        maxSpeed *= 0.8f;
    } else if (gear != 0) {
        if (boost) {
            maxSpeed *= 1.1f;
        } else if (self->mbManual) {
            maxSpeed *= 1.05f;
        }
    }

    if (self->mrEngineAngularVel >= maxSpeed - 0.001f) {
        if (self->mrMaxRevs == self->mEngineMaxRPMValue && !boost) {
            if (gear == 0) {
                self->mrMaxRevs = (0.0f + self->mEngineMaxRPMValue) -
                                 (0.1f * self->mEngineMaxRPMValue) * RandomNorm(self);
            } else {
                self->mrMaxRevs = ((0.0f + self->mEngineMaxRPMValue) -
                                  RandomNorm(self) * 50.0f) - 70.0f;
            }
        }
        accelerator = (-1.0f * targetSpeed) / maxSpeed;
        self->mrEngineAngularVel -= 20.0f;
        if (accelerator > 0.0f) {
            accelerator = 0.0f;
        }
    } else {
        self->mrMaxRevs = self->mEngineMaxRPMValue;
        f32 accelRate, decelRate;
        if (self->mnGear == 0) {
            if (self->mEngineMaxRPMValue < 6000.0f) {
                accelRate = 22.5f;
                decelRate = 9.6f;
            } else {
                accelRate = 45.0f;
                decelRate = 16.0f;
            }
            targetSpeed *= (0.0f + 0.97f) + 0.06f * RandomNorm(self);
        } else {
            accelRate = boost ? 45.0f : 15.0f;
            decelRate = 8.0f;
        }
        f32 engineSpeed = self->mrEngineAngularVel;
        if (targetSpeed <= engineSpeed + accelRate) {
            if (targetSpeed < engineSpeed - decelRate) {
                self->mrEngineAngularVel = self->mrEngineAngularVel - decelRate;
            } else {
                self->mrEngineAngularVel = targetSpeed;
            }
        } else {
            self->mrEngineAngularVel = engineSpeed + accelRate;
        }
        if (self->mrEngineAngularVel < idleSpeed || SpeedMPH(self->physics) < 1.0f) {
            if (self->mrEngineAngularVel <= idleSpeed + 0.1f) {
                self->mrEngineAngularVel = (0.0f + idleSpeed) + 16.0f * RandomNorm(self);
            }
            if (accelerator <= 0.0f &&
                (self->mnGear == 1 || self->mnGear == -1 || self->mbManual)) {
                self->mnGear = 0;
                self->mbClutchDown = 1;
                self->mrClutchReleaseTime = 0.035f;
            }
        }
    }

    f32 engineSpeed = self->mrEngineAngularVel;
    f32 torqueScale;
    if (engineSpeed < self->mrPeakTorqueRevs) {
        torqueScale = (0.0f + 0.5f) + 0.5f * (engineSpeed / self->mrPeakTorqueRevs);
    } else if (engineSpeed > self->mrFallOffTorqueRevs) {
        torqueScale = (0.0f + 0.75f) +
                      0.25f * ((maxSpeed - engineSpeed) / (maxSpeed - self->mrFallOffTorqueRevs));
    } else {
        torqueScale = 1.0f;
    }
    if (doubleTorque) {
        torqueScale = 2.0f;
    }
    if (self->mnGear == -1) {
        torqueScale = (maxSpeed - engineSpeed) / maxSpeed;
    }
    if (boost && SpeedMPH(self->physics) < self->physics->boostMaxSpeed) {
        RaceCarBoostState *car = self->physics->raceCar;
        f32 time = car->time10CC - car->time11B0;
        if (car->flag11E1) {
            torqueScale = self->boostTorqueScale + 1.0f;
        } else if (time < self->boostTorqueWindow) {
            torqueScale = 1.0f + self->boostTorqueScale * (1.0f - time / self->boostTorqueWindow);
        }
    }
    f32 random = RandomNorm(self);
    f32 torque = accelerator * self->mEngineTorqueValue;
    torque = torqueScale * torque;
    torque = GearRatio(self, self->mnGear) * torque;
    torque = self->mnGearRatioFinalValue * torque;
    return torque * ((0.0f + 0.95f) + 0.1f * random);
}
