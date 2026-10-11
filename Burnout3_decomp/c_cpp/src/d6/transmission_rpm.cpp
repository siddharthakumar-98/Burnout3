#include "transmission.h"

/* Select an engine speed below the change-up threshold. The original leaves
 * f3 undefined if mnTopGear <= 1. The locked PS2 compiler preserves that
 * incoming register on the skipped-loop edge; do not invent an initial value.
 * Arguments remain anonymous until their units are established at callers. */
extern "C" s32 func_001C6F30(CTransmission *self, f32 divisor, f32 speed)
{
    s32 gear = 1;
    f32 changeUp = 0.10471975803375244f * self->mChangeUpRPMValue;
    f32 angularVelocity;
    f32 ratio = speed / divisor;
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
