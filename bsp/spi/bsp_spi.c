#include "bsp_spi.h"

#include "bsp_gpio.h"
#include "car_config.h"

void BspSpi_Init(void)
{
    BspGpio_DisplaySetChipSelect(false);
    BspGpio_DisplaySetDataCommand(true);
    BspGpio_DisplaySetReset(false);
    BspSpi_SetBacklight(false);
}

bool BspSpi_Write(bool dataMode, const uint8_t *data, size_t length)
{
    size_t index;

    if ((data == NULL) && (length != 0U)) {
        return false;
    }

    BspGpio_DisplaySetChipSelect(true);
    BspGpio_DisplaySetDataCommand(dataMode);
    for (index = 0U; index < length; index++) {
        DL_SPI_transmitData8(CAR_ST7735_SPI_INSTANCE, data[index]);
        while (DL_SPI_isBusy(CAR_ST7735_SPI_INSTANCE)) {
        }
        (void)DL_SPI_receiveData8(CAR_ST7735_SPI_INSTANCE);
    }
    BspGpio_DisplaySetChipSelect(false);
    return true;
}

void BspSpi_SetBacklight(bool enabled)
{
    BspGpio_DisplaySetBacklight(enabled);
}

void BspSpi_SetReset(bool asserted)
{
    BspGpio_DisplaySetReset(asserted);
}

void BspSpi_DelayMs(uint32_t milliseconds)
{
    while (milliseconds-- > 0U) {
        delay_cycles(CPUCLK_FREQ / 1000U);
    }
}
