#ifndef BSP_SPI_H
#define BSP_SPI_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

void BspSpi_Init(void);
bool BspSpi_Write(bool dataMode, const uint8_t *data, size_t length);
void BspSpi_SetBacklight(bool enabled);
void BspSpi_SetReset(bool asserted);
void BspSpi_DelayMs(uint32_t milliseconds);

#endif
