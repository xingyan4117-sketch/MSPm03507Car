#ifndef SCREEN_RENDER_HOST_BSP_SPI_H
#define SCREEN_RENDER_HOST_BSP_SPI_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void BspSpi_Init(void);
bool BspSpi_Write(bool dataMode, const uint8_t *data, size_t length);
void BspSpi_SetBacklight(bool enabled);
void BspSpi_SetReset(bool asserted);
void BspSpi_DelayMs(uint32_t milliseconds);

void ScreenStub_Reset(void);
void ScreenStub_FailWriteCall(size_t callIndex);
bool ScreenStub_BacklightEnabled(void);
uint32_t ScreenStub_ResetAssertCalls(void);
uint32_t ScreenStub_ResetReleaseCalls(void);
uint32_t ScreenStub_DelayCalls(void);
size_t ScreenStub_WriteCount(void);
bool ScreenStub_WriteDataMode(size_t index);
size_t ScreenStub_WriteLength(size_t index);
uint8_t ScreenStub_WriteFirstByte(size_t index);
size_t ScreenStub_LastDataLength(void);
uint8_t ScreenStub_LastDataByte(size_t index);

#endif
