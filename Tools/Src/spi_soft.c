#include "spi_soft.h"
#include "sys_delay.h"
#include "main.h"

uint8_t spi_soft_readwrite_byte(uint8_t Txdata)
{
    uint8_t i = 0;
    uint8_t Rxdata = 0;
    SPI_SOFT_SCK_LOW();
    for (i = 7; i >= 0; i--)
    {
        SPI_SOFT_SCK_LOW();
        if (Txdata & (1 << i))
        {
            SPI_SOFT_MOSI_HIGH();
        }
        else
        {
            SPI_SOFT_MOSI_LOW();
        }
        sys_delay_us(1);
        SPI_SOFT_SCK_HIGH();
        Rxdata <<= 1;
        Rxdata |= SPI_SOFT_MISO_READ();
        sys_delay_us(1);
    }
    SPI_SOFT_SCK_LOW();
    return Rxdata;
}