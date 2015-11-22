#pragma once

#include <inttypes.h>

#include "AP_HAL_Namespace.h"
#include "Device.h"
#include "utility/OwnPtr.h"

namespace AP_HAL {

class I2CDevice : public Device {
public:
    enum Speed {
        SPEED_100KHZ = 100,
        SPEED_400KHZ = 400,
    };

    virtual ~I2CDevice() { }

    /*
     * Change device address. Note that this is the 7 bit address, it
     * does not include the bit for read/write.
     */
    virtual void set_address(uint8_t address) = 0;

    /* set number of retries on transfers */
    virtual void set_retries(uint8_t retries) = 0;

    /*
     * Set the speed of future transfers. This speed is shared by all
     * devices on the same bus. The lowest requested speed across all
     * devices on the bus is used.
     *
     * Return: false in case of errors; not supporting the method is not an
     * error.
     */
    virtual bool set_speed(enum Speed speed) = 0;


    /* Device implementation */

    /* See Device::set_speed() */
    bool set_speed(Device::Speed speed) override
    {
        switch (speed) {
        case Device::SPEED_HIGH:
            return set_speed(SPEED_400KHZ);
        case Device::SPEED_LOW:
            return set_speed(SPEED_100KHZ);
        }
        /* not reached */
        return true;
    }

    /* See Device::transfer() */
    virtual bool transfer(const uint8_t *send, uint32_t send_len,
                          uint8_t *recv, uint32_t recv_len) override = 0;

    /* See Device::get_semaphore() */
    virtual Semaphore *get_semaphore() override = 0;

    /* See Device::register_periodic_callback() */
    virtual Device::PeriodicHandle *register_periodic_callback(
        uint32_t period_usec, MemberProc) override = 0;

    /* Temporary conversion functions. TODO: remove */
    virtual int get_fd() override = 0;
};

class I2CDeviceManager {
public:
    /* Get a device handle */
    virtual OwnPtr<AP_HAL::I2CDevice> get_device(uint8_t bus, uint8_t address) = 0;
};

}
