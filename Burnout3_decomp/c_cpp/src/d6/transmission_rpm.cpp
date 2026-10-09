#include "transmission.h"

/* Select an engine speed below the change-up threshold. The original leaves
 * f3 undefined if mnTopGear <= 1; retain that edge pending caller validation.
 * Arguments remain anonymous until their units are established at callers. */
extern "C" s32 func_001C6F30(CTransmission *self, f32 divisor, f32 speed)
{
    s32 gear = 1;
    f32 ratio = speed / divisor;
    f32 angularVelocity;
    f32 changeUp = 0.10471975803375244f * self->mChangeUpRPMValue;
    while (gear < self->mnTopGear) {
        angularVelocity = self->mnGearRatioFinalValue * (ratio * self->maGearRatioValues[gear]);
        if (angularVelocity <= changeUp) {
            break;
        }
        gear++;
    }
    self->mrEngineAngularVel = angularVelocity;
    return gear;
}
