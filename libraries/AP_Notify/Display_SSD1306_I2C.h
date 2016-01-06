/// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-

#pragma once

#include "Display.h"

class Display_SSD1306_I2C: public Display {
public:
    virtual bool hw_init();
    virtual bool hw_update();
    virtual bool set_pixel(uint16_t x, uint16_t y);
    virtual bool clear_pixel(uint16_t x, uint16_t y);

private:
    uint8_t _displaybuffer[SSD1306_ROWS * SSD1306_COLUMNS_PER_PAGE];
};
