/// -*- tab-width: 4; Mode: C++; c-basic-offset: 4; indent-tabs-mode: nil -*-
/*
 * Copyright (C) 2016  Intel Corporation. All rights reserved.
 *
 * This file is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This file is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once

#include <inttypes.h>

#include <AP_HAL/Device.h>

#include "Poller.h"

namespace Linux {

class BusPollable : public Pollable {
public:
    using PeriodicCb = AP_HAL::Device::PeriodicCb;

    BusPollable(PeriodicCb cb_) : cb(cb_) { }

    void on_can_read() override;

    PeriodicCb cb;
    bool removeme;

    bool setup_timer(uint32_t timeout_usec);
    bool adjust_timer(uint32_t timeout_usec);
    uint32_t get_events();

protected:
    bool _adjust_timer(int fd, uint32_t timeout_usec);
};

}
