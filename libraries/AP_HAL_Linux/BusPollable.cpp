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
#include "BusPollable.h"

#include <poll.h>
#include <sys/epoll.h>
#include <sys/timerfd.h>

#include <AP_HAL/Device.h>
#include <AP_Math/AP_Math.h>

namespace Linux {

void BusPollable::on_can_read()
{
    if (removeme) {
        return;
    }

    uint64_t nevents = 0;
    int r = read(_fd, &nevents, sizeof(nevents));
    if (r < 0) {
        return;
    }

    if (!cb()) {
        removeme = true;
    }
}

bool BusPollable::setup_timer(uint32_t timeout_usec)
{
    int fd;

    if (_fd >= 0) {
        return false;
    }

    fd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC|TFD_NONBLOCK);
    if (fd < 0) {
        return false;
    }

    if (!_adjust_timer(timeout_usec, fd)) {
        ::close(fd);
        return false;
    }

    _fd = fd;

    return true;
}

bool BusPollable::adjust_timer(uint32_t timeout_usec)
{
    if (_fd < 0) {
        return false;
    }

    return _adjust_timer(timeout_usec, _fd);
}

bool BusPollable::_adjust_timer(int fd, uint32_t timeout_usec)
{
    struct itimerspec spec = { };

    spec.it_interval.tv_nsec = timeout_usec * NSEC_PER_USEC;
    spec.it_value.tv_nsec = timeout_usec * NSEC_PER_USEC;

    if (timerfd_settime(fd, 0, &spec, nullptr) < 0) {
        return false;
    }

    return true;
}

uint32_t BusPollable::get_events()
{
    return POLLIN | POLLPRI | POLLERR;
}
}
