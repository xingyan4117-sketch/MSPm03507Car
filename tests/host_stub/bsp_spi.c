#include "bsp_spi.h"

#include <string.h>

#define SCREEN_STUB_MAX_WRITES (256U)
#define SCREEN_STUB_CAPTURE_BYTES (16384U)

typedef struct {
    bool dataMode;
    size_t length;
    uint8_t firstByte;
} ScreenStubWrite;

static ScreenStubWrite g_writes[SCREEN_STUB_MAX_WRITES];
static uint8_t g_lastData[SCREEN_STUB_CAPTURE_BYTES];
static size_t g_lastDataLength;
static size_t g_maxWhiteBytes;
static size_t g_writeCount;
static size_t g_failWriteCall;
static bool g_backlightEnabled;
static uint32_t g_resetAssertCalls;
static uint32_t g_resetReleaseCalls;
static uint32_t g_delayCalls;

void ScreenStub_Reset(void)
{
    (void) memset(g_writes, 0, sizeof(g_writes));
    (void) memset(g_lastData, 0, sizeof(g_lastData));
    g_lastDataLength = 0U;
    g_maxWhiteBytes = 0U;
    g_writeCount = 0U;
    g_failWriteCall = 0U;
    g_backlightEnabled = false;
    g_resetAssertCalls = 0U;
    g_resetReleaseCalls = 0U;
    g_delayCalls = 0U;
}

void ScreenStub_FailWriteCall(size_t callIndex)
{
    g_failWriteCall = callIndex;
}

void BspSpi_Init(void)
{
}

bool BspSpi_Write(bool dataMode, const uint8_t *data, size_t length)
{
    if ((data == NULL) && (length != 0U)) {
        return false;
    }
    if (g_writeCount < SCREEN_STUB_MAX_WRITES) {
        g_writes[g_writeCount].dataMode = dataMode;
        g_writes[g_writeCount].length = length;
        g_writes[g_writeCount].firstByte = (length == 0U) ? 0U : data[0];
    }
    g_writeCount++;
    if ((g_failWriteCall != 0U) && (g_writeCount == g_failWriteCall)) {
        return false;
    }
    if (dataMode && (length <= sizeof(g_lastData))) {
        size_t whiteBytes = 0U;
        size_t index;

        for (index = 0U; index < length; index++) {
            if (data[index] == 0xFFU) whiteBytes++;
        }
        if (whiteBytes > g_maxWhiteBytes) g_maxWhiteBytes = whiteBytes;
        (void) memcpy(g_lastData, data, length);
        g_lastDataLength = length;
    }
    return true;
}

void BspSpi_SetBacklight(bool enabled)
{
    g_backlightEnabled = enabled;
}

void BspSpi_SetReset(bool asserted)
{
    if (asserted) {
        g_resetAssertCalls++;
    } else {
        g_resetReleaseCalls++;
    }
}

void BspSpi_DelayMs(uint32_t milliseconds)
{
    (void) milliseconds;
    g_delayCalls++;
}

bool ScreenStub_BacklightEnabled(void)
{
    return g_backlightEnabled;
}

uint32_t ScreenStub_ResetAssertCalls(void) { return g_resetAssertCalls; }
uint32_t ScreenStub_ResetReleaseCalls(void) { return g_resetReleaseCalls; }
uint32_t ScreenStub_DelayCalls(void) { return g_delayCalls; }

size_t ScreenStub_WriteCount(void)
{
    return g_writeCount;
}

bool ScreenStub_WriteDataMode(size_t index)
{
    return g_writes[index].dataMode;
}

size_t ScreenStub_WriteLength(size_t index)
{
    return g_writes[index].length;
}

uint8_t ScreenStub_WriteFirstByte(size_t index)
{
    return g_writes[index].firstByte;
}

size_t ScreenStub_LastDataLength(void)
{
    return g_lastDataLength;
}

uint8_t ScreenStub_LastDataByte(size_t index)
{
    return g_lastData[index];
}

size_t ScreenStub_MaxWhiteBytes(void)
{
    return g_maxWhiteBytes;
}
