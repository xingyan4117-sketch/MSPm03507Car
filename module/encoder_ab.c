#include "encoder_ab.h"

#include "bsp_encoder.h"

#include <limits.h>
#include <stddef.h>
#include <stdbool.h>

static int32_t EncoderAb_NegateSaturated(int32_t value)
{
    if (value == INT32_MIN) {
        return INT32_MAX;
    }
    return -value;
}

static EncoderAbSnapshot EncoderAb_ToSnapshot(BspEncoderReadout readout,
    bool invertDirection)
{
    EncoderAbSnapshot snapshot;

    snapshot.deltaCounts = invertDirection ?
        EncoderAb_NegateSaturated(readout.delta) : readout.delta;
    snapshot.invalidTransitions = readout.invalidTransitions;
    return snapshot;
}

EncoderAbSnapshot EncoderAb_ReadA(void)
{
    return EncoderAb_ToSnapshot(BspEncoder_ReadAndClearA(), false);
}

EncoderAbSnapshot EncoderAb_ReadB(void)
{
    return EncoderAb_ToSnapshot(BspEncoder_ReadAndClearB(), true);
}

EncoderAbSnapshot EncoderAb_ReadD(void)
{
    return EncoderAb_ToSnapshot(BspEncoder_ReadAndClearD(), true);
}

EncoderAbSnapshot EncoderAb_ReadC(void)
{
    return EncoderAb_ToSnapshot(BspEncoder_ReadAndClearC(), false);
}

int32_t EncoderAb_CountsToRpm(const EncoderAbSnapshot *snapshot,
    const EncoderAbRpmConfig *config)
{
    uint64_t magnitudeCounts;
    uint64_t denominator;
    uint64_t magnitudeRpm;

    if ((snapshot == NULL) || (config == NULL) ||
        (config->countsPerRevolution == 0U || config->samplePeriodMs == 0U)) {
        return 0;
    }

    if (snapshot->deltaCounts < 0) {
        magnitudeCounts = (uint64_t) (-(snapshot->deltaCounts + 1)) + 1U;
    } else {
        magnitudeCounts = (uint64_t) snapshot->deltaCounts;
    }

    denominator = (uint64_t) config->countsPerRevolution * config->samplePeriodMs;
    magnitudeRpm = (magnitudeCounts * 60000U) / denominator;
    if (magnitudeRpm > (uint64_t) INT32_MAX) {
        return snapshot->deltaCounts < 0 ? INT32_MIN : INT32_MAX;
    }
    return snapshot->deltaCounts < 0 ? -(int32_t) magnitudeRpm : (int32_t) magnitudeRpm;
}
