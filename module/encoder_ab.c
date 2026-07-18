#include "encoder_ab.h"

#include "bsp_encoder.h"

#include <limits.h>
#include <stddef.h>

static EncoderAbSnapshot EncoderAb_ToSnapshot(BspEncoderReadout readout)
{
    EncoderAbSnapshot snapshot;

    snapshot.deltaCounts = readout.delta;
    snapshot.invalidTransitions = readout.invalidTransitions;
    return snapshot;
}

EncoderAbSnapshot EncoderAb_ReadA(void)
{
    return EncoderAb_ToSnapshot(BspEncoder_ReadAndClearA());
}

EncoderAbSnapshot EncoderAb_ReadB(void)
{
    return EncoderAb_ToSnapshot(BspEncoder_ReadAndClearB());
}

EncoderAbSnapshot EncoderAb_ReadD(void)
{
    return EncoderAb_ToSnapshot(BspEncoder_ReadAndClearD());
}

EncoderAbSnapshot EncoderAb_ReadC(void)
{
    return EncoderAb_ToSnapshot(BspEncoder_ReadAndClearC());
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
