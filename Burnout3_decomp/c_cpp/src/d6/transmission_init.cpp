#include "transmission.h"

extern "C" s32 func_001C7880(CTransmission *self, VehiclePhysics *physics)
{
    self->mrEngineAngularVel = 0.0f;
    self->mrClutchReleaseTime = 0.0f;
    self->mbClutchDown = 0;
    self->mbLockInGear = 0;
    self->mrMaxRevs = self->mEngineMaxRPMValue;
    self->randomCarry = 0x02B9D6F8;
    self->randomState = 0xFD462907;
    self->mbManualSwitched = 0;
    self->mbGearUpSwitched = 0;
    self->mbGearDownSwitched = 0;
    self->mnGear = 0;
    self->mnTopGear = 0;
    while (self->maGearRatioValues[self->mnTopGear + 1] > 0.0f && self->mnTopGear < 6) {
        self->mnTopGear++;
    }
    self->mrEngineAngularVel = 0.0f;
    self->mrClutchReleaseTime = 0.0f;
    self->mbClutchDown = 0;
    self->mbLockInGear = 0;
    self->mrMaxRevs = self->mEngineMaxRPMValue;
    self->randomCarry = 0x02B9D6F8;
    self->randomState = 0xFD462907;
    self->mbManualSwitched = 0;
    self->mbGearUpSwitched = 0;
    self->mbGearDownSwitched = 0;
    self->mnGear = 0;
    self->mrLastChangeUpTime = 0.0f;
    self->mrLastChangeDownTime = 0.0f;
    self->physics = physics;
    return 1;
}
