#pragma once

#include <inttypes.h>
#include <vector>

#include <AP_HAL/HAL.h>
#include <AP_HAL/I2CDevice.h>
#include <AP_HAL/utility/OwnPtr.h>

#include "Semaphores.h"

namespace Linux {

class I2CBus;

class I2CDevice : public AP_HAL::I2CDevice {
public:
    static I2CDevice *from(AP_HAL::I2CDevice *dev)
    {
        return static_cast<I2CDevice*>(dev);
    }

    /* AP_HAL::I2CDevice implementation */

    I2CDevice(I2CBus &bus, uint8_t address)
        : _bus(bus)
        , _address(address)
    {
    }

    ~I2CDevice();

    /* See AP_HAL::I2CDevice::set_address() */
    void set_address(uint8_t address) override { _address = address; }

    /* See AP_HAL::I2CDevice::set_retries() */
    void set_retries(uint8_t retries) override { _retries = retries; }

    /* AP_HAL::Device implementation */

    /* See AP_HAL::Device::set_speed(): Empty implementation, not supported. */
    bool set_speed(enum Device::Speed speed) override { return true; }

    /* See AP_HAL::Device::transfer() */
    bool transfer(const uint8_t *send, uint32_t send_len,
                  uint8_t *recv, uint32_t recv_len) override;

    /* See AP_HAL::Device::get_semaphore() */
    AP_HAL::Semaphore *get_semaphore() override;

    /* See AP_HAL::Device::register_periodic_callback() */
    AP_HAL::Device::PeriodicHandle *register_periodic_callback(
        uint32_t period_usec, AP_HAL::MemberProc) override
    {
        /* Not implemented yet */
        return nullptr;
    };

    /* See AP_HAL::Device::get_fd() */
    int get_fd() override;

protected:
    I2CBus &_bus;
    uint8_t _address;
    uint8_t _retries = 0;
};

class I2CDeviceManager : public AP_HAL::I2CDeviceManager {
public:
    friend class I2CDevice;

    static I2CDeviceManager *from(AP_HAL::I2CDeviceManager *i2c_mgr)
    {
        return static_cast<I2CDeviceManager*>(i2c_mgr);
    }

    I2CDeviceManager();

    /*
     * Get device by looking up the I2C bus on the buses from @devpaths.
     *
     * Each string in @devpaths are possible locations for the bus as
     * returned by 'udevadm info -q path /dev/i2c-X'. The first I2C bus
     * matching a prefix in @devpaths is returned.
     */
    AP_HAL::OwnPtr<AP_HAL::I2CDevice> get_device(
            std::vector<const char *> devpaths, uint8_t address);

    /* AP_HAL::I2CDeviceManager implementation */
    AP_HAL::OwnPtr<AP_HAL::I2CDevice> get_device(uint8_t bus, uint8_t address) override;

protected:
    void _unregister(I2CBus &b);
    AP_HAL::OwnPtr<AP_HAL::I2CDevice> _create_device(I2CBus &b, uint8_t address) const;

    std::vector<I2CBus*> _buses;
};

}
