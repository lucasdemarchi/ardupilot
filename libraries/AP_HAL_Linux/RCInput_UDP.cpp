#include <stddef.h>
#include <stdio.h>

#include <AP_HAL/AP_HAL.h>

#include "RCInput_UDP.h"

extern const AP_HAL::HAL& hal;

using namespace Linux;

RCInput_UDP::RCInput_UDP() :
    _port(0),
    _last_buf_ts(0),
    _last_buf_seq(0)
{}

void RCInput_UDP::init()
{
    _port = RCINPUT_UDP_DEF_PORT;
    if(!_socket.bind("0.0.0.0", _port)) {
        hal.console->printf("failed to bind UDP socket\n");
    }

    _socket.set_blocking(false);

    return;
}

void RCInput_UDP::_timer_tick(void)
{
    uint64_t delay;
    uint16_t seq_inc;
    ssize_t size = -1;
    unsigned int n_channels;
    ssize_t header_size = offsetof(struct rc_udp_packet, pwms);

    /*
     * Read all pending packets from udp without blocking - we may have more
     * than one if we are not reading them faster than the RC is sending: only
     * the last one is going to really take effect
     */
    do {
        ssize_t r = _socket.recv(&_buf, sizeof(_buf), 0);
        if (r == -1)
            break;

        size = r;
    } while (true);

    if (size < header_size)
        return;

    /*
     * v2 and v3 are compatible, with the only difference being the number
     * of channels.  We require at least 4 channels to be compatible with
     * the upper layers and don't care about making enforcing 8 channels
     * for v2
     */
    n_channels = (size - header_size) / sizeof(uint16_t);
    if (n_channels < 4)
        return;

    if (_last_buf_ts != 0 &&
        (delay = _buf.timestamp_us - _last_buf_ts) > 100000) {
        hal.console->printf("no rc cmds received for %llu\n", delay);
    }

    _last_buf_ts = _buf.timestamp_us;

    seq_inc = _buf.sequence - _last_buf_seq;

    if (seq_inc > 10) {
        hal.console->printf("gap in rc cmds > 10: %u\n", seq_inc);
    }

    _update_periods(_buf.pwms, n_channels);
}
