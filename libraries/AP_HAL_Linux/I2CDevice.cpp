#include "I2CDevice.h"

#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <vector>

#include <AP_HAL/AP_HAL.h>

namespace Linux {

static const AP_HAL::HAL &hal = AP_HAL::get_HAL();

/* Private struct to maintain for each bus */
class I2CBus {
public:
    ~I2CBus()
    {
        if (fd >= 0) {
            ::close(fd);
        }
    }

    int open(uint8_t n)
    {
        char path[sizeof("/dev/i2c-XXX")];
        int r;

        if (fd >= 0) {
            return -EBUSY;
        }

        // TODO: open the bus

        bus = n;

        return fd;
    }

    Semaphore sem;
    int fd = -1;
    uint8_t bus;

    uint8_t ref;
};

I2CDevice::~I2CDevice()
{
    // Unregister itself from the I2CDeviceManager
    I2CDeviceManager::from(hal.i2c_mgr)->_unregister(_bus);
}

bool I2CDevice::transfer(const uint8_t *send, uint32_t send_len,
                         uint8_t *recv, uint32_t recv_len)
{
    // TODO: implement I2C transfer
    return false;
}

AP_HAL::Semaphore *I2CDevice::get_semaphore()
{
    return &_bus.sem;
}

int I2CDevice::get_fd()
{
    return _bus.fd;
}

I2CDeviceManager::I2CDeviceManager()
{
    /* Reserve space up-front for 4 buses */
    _buses.reserve(4);
}

AP_HAL::OwnPtr<AP_HAL::I2CDevice>
I2CDeviceManager::get_device(std::vector<const char *> devpaths, uint8_t address)
{
    // TODO: implement device search on sysfs
}

AP_HAL::OwnPtr<AP_HAL::I2CDevice>
I2CDeviceManager::get_device(uint8_t bus, uint8_t address)
{
    I2CBus *b = nullptr;

    for (uint8_t i = 0, n = _buses.size(); i < n; i++) {
        if (_buses[i]->bus == bus) {
            b = _buses[i];
            break;
        }
    }

    /*
     * Device has already a bus registered. Create a new device using the same
     * bus instance
     */
    if (b) {
        return _create_device(*b, address);
    }

    b = new I2CBus();
    if (!b) {
        return nullptr;
    }

    if (b->open(bus) < 0) {
        delete b;
        return nullptr;
    }

    auto dev = _create_device(*b, address);
    if (!dev) {
        delete b;
        return nullptr;
    }

    _buses.push_back(b);

    return dev;
}

/* Create a new device increasing the bus reference */
AP_HAL::OwnPtr<AP_HAL::I2CDevice>
I2CDeviceManager::_create_device(I2CBus &b, uint8_t address) const
{
    auto dev = AP_HAL::OwnPtr<AP_HAL::I2CDevice>(new I2CDevice(b, address));
    if (!dev) {
        return nullptr;
    }
    b.ref++;
    return dev;
}

void I2CDeviceManager::_unregister(I2CBus &b)
{
    if (b.ref == 0 || --b.ref > 0) {
        return;
    }

    for (auto it = _buses.begin(); it != _buses.end(); it++) {
        if ((*it)->bus == b.bus) {
            _buses.erase(it);
            delete &b;
            break;
        }
    }
}

}
