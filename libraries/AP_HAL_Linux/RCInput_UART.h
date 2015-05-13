#pragma once

#include "AP_HAL_Linux.h"
#include "RCInput.h"

#define CHANNELS 8

class Linux::LinuxRCInput_UART : public Linux::LinuxRCInput
{
public:
    void init(void*);
    void _timer_tick(void);
private:
    uint8_t _count;
    int8_t _direction;

    int _fd;
    uint8_t *_pdata;
    ssize_t _remain;
    struct __attribute__((__packed__)) {
        uint16_t magic;
        uint16_t values[CHANNELS];
    } _data;
};
