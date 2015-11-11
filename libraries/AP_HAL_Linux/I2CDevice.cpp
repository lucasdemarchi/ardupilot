#include "I2CDevice.h"

#include <algorithm>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

#include <AP_HAL/AP_HAL.h>

#include "Util.h"

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

        r = snprintf(path, sizeof(path), "/dev/i2c-%u", n);
        if (r < 0 || r >= (int)sizeof(path)) {
            return -EINVAL;
        }

        fd = ::open(path, O_RDWR | O_CLOEXEC);
        if (fd < 0) {
            return -errno;
        }

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
    const char *dirname = "/sys/class/i2c-dev";
    struct dirent *de = nullptr;
    DIR *d;

    d = opendir(dirname);
    if (!d) {
        AP_HAL::panic("Could not get list of I2C buses");
    }

    for (de = readdir(d); de; de = readdir(d)) {
        char *str_device, *abs_str_device;

        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
            continue;
        }

        if (asprintf(&str_device, "%s/%s", dirname, de->d_name) < 0) {
            continue;
        }

        abs_str_device = realpath(str_device, nullptr);
        if (!abs_str_device || strncmp(abs_str_device, "/sys", sizeof("/sys") - 1)) {
            free(abs_str_device);
            free(str_device);
            continue;
        }

        auto t = std::find_if(std::begin(devpaths), std::end(devpaths),
                              [abs_str_device](const char *v) {
                                  return strncmp(abs_str_device + sizeof("/sys") - 1, v, strlen(v)) == 0;
                              });

        free(abs_str_device);
        free(str_device);

        if (t != std::end(devpaths)) {
            unsigned int n;

            /* Found the bus, try to create the device now */
            if (sscanf(de->d_name, "i2c-%u", &n) != 1) {
                AP_HAL::panic("I2CDevice: can't parse %s", de->d_name);
            }
            if (n > UINT8_MAX) {
                AP_HAL::panic("I2CDevice: bus with number n=%u higher than %u",
                              n, UINT8_MAX);
            }

            closedir(d);
            return get_device(n, address);
        }
    }

    /* not found */
    closedir(d);
    return nullptr;
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
