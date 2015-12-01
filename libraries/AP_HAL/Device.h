#pragma once

#include <inttypes.h>

#include "AP_HAL_Namespace.h"

/*
 * This is an interface abstracting I2C and SPI devices
 */
class AP_HAL::Device {
public:
    enum Speed {
        SPEED_HIGH,
        SPEED_LOW,
    };

    typedef void PeriodicHandle;

    virtual ~Device() { }

    /*
     * Set the speed of future transfers. Depending on the bus the speed may
     * be shared for all devices on the same bus.
     *
     * Return: true if speed was succesfully set or platform doesn't implement
     * it; false otherwise.
     */
    virtual bool set_speed(Speed speed)  = 0;

    /*
     * Core transfer function. This does a single bus transaction which
     * sends send_len bytes and receives recv_len bytes back from the slave.
     *
     * Return: true on a successful transfer, false on failure.
     */
    virtual bool transfer(const uint8_t *send, uint32_t send_len,
                          uint8_t *recv, uint32_t recv_len) = 0;

    bool read_registers(uint8_t reg, uint8_t *recv, uint32_t recv_len)
    {
        return transfer(&reg, 1, recv, recv_len);
    }

    bool write_register(uint8_t reg, uint8_t val)
    {
        uint8_t buf[2] = { reg, val };
        return transfer(buf, sizeof(buf), nullptr, 0);
    }

    /*
     * Get the semaphore for the bus this device is in.  This is intended for
     * drivers to use during initialization phase only.
     */
    virtual AP_HAL::Semaphore *get_semaphore() = 0;

    /*
     * Register a periodic callback for this bus. All callbacks on the
     * same bus are made from the same thread with lock already taken.
     *
     * Return: A handle for this periodic callback. To cancel the callback
     * delete the handle.
     */
    virtual PeriodicHandle *register_periodic_callback(uint32_t period_usec, AP_HAL::MemberProc) = 0;

    virtual int get_fd() = 0;
};
