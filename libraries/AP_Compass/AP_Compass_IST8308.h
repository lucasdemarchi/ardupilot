// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once

#include <AP_Common/AP_Common.h>
#include <AP_HAL/AP_HAL.h>
#include <AP_HAL/I2CDevice.h>
#include <AP_Math/AP_Math.h>

#include "AP_Compass.h"
#include "AP_Compass_Backend.h"

#ifndef HAL_COMPASS_IST8308_I2C_ADDR
#define HAL_COMPASS_IST8308_I2C_ADDR 0x0C
#endif

class AP_Compass_IST8308 : public AP_Compass_Backend
{
public:
    static AP_Compass_Backend *probe(Compass &compass,
                                     AP_HAL::OwnPtr<AP_HAL::I2CDevice> dev,
                                     bool force_external = false,
                                     enum Rotation rotation = ROTATION_NONE);

    void read() override;

    static constexpr const char *name = "IST8308";

private:
    AP_Compass_IST8308(Compass &compass,
                       AP_HAL::OwnPtr<AP_HAL::Device> dev,
                       bool force_external,
                       enum Rotation rotation);

    void timer();
    bool init();

    AP_HAL::OwnPtr<AP_HAL::Device> _dev;
    AP_HAL::Device::PeriodicHandle _periodic_handle;
    AP_HAL::Util::perf_counter_t _perf_xfer_err;

    Vector3f _accum = Vector3f();
    uint32_t _accum_count = 0;

    enum Rotation _rotation;
    uint8_t _instance;
    bool _force_external;
};
