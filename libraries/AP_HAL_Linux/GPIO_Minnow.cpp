#include <AP_Common/AP_Common.h>

#include "GPIO_Minnow.h"

#if CONFIG_HAL_BOARD_SUBTYPE == HAL_BOARD_SUBTYPE_LINUX_MINLURE

const unsigned Linux::LinuxGPIO_Sysfs::pin_table[] = {
    [AP_HAL::GPIO::VirtualPin::MINNOW_SPI_CS] =    476,
    [AP_HAL::GPIO::VirtualPin::MINNOW_SPI_MISO] =  477,
    [AP_HAL::GPIO::VirtualPin::MINNOW_SPI_MOSI] =  478,
    [AP_HAL::GPIO::VirtualPin::MINNOW_SPI_CLK] =   479,
    [AP_HAL::GPIO::VirtualPin::MINNOW_I2C_SCL] =   499,
    [AP_HAL::GPIO::VirtualPin::MINNOW_I2C_SDA] =   498,
    [AP_HAL::GPIO::VirtualPin::MINNOW_UART2_TXD] = 485,
    [AP_HAL::GPIO::VirtualPin::MINNOW_UART2_RXD] = 484,
    [AP_HAL::GPIO::VirtualPin::MINNOW_S5_0] =      338,
    [AP_HAL::GPIO::VirtualPin::MINNOW_S5_1] =      339,
    [AP_HAL::GPIO::VirtualPin::MINNOW_S5_2] =      340,
    [AP_HAL::GPIO::VirtualPin::MINNOW_UART1_TXD] = 481,
    [AP_HAL::GPIO::VirtualPin::MINNOW_UART1_RXD] = 480,
    [AP_HAL::GPIO::VirtualPin::MINNOW_UART1_CTS] = 483,
    [AP_HAL::GPIO::VirtualPin::MINNOW_UART1_RTS] = 482,
    [AP_HAL::GPIO::VirtualPin::MINNOW_I2S_CLK] =   472,
    [AP_HAL::GPIO::VirtualPin::MINNOW_I2S_FRM] =   473,
    [AP_HAL::GPIO::VirtualPin::MINNOW_I2S_DO] =    475,
    [AP_HAL::GPIO::VirtualPin::MINNOW_I2S_DI] =    474,
    [AP_HAL::GPIO::VirtualPin::MINNOW_PWM0] =      504,
    [AP_HAL::GPIO::VirtualPin::MINNOW_PWM1] =      505,
    [AP_HAL::GPIO::VirtualPin::MINNOW_IBL_8254] =  464,
};

static_assert(ARRAY_SIZE(Linux::LinuxGPIO_Sysfs::pin_table) == AP_HAL::GPIO::VirtualPin::_MINNOW_MAX,
              "GPIO pin_table must have the same size of entries in AP_HAL::GPIO::VirtualPin");

#endif
