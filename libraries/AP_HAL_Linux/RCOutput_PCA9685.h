
#ifndef __AP_HAL_LINUX_RCOUTPUT_NAVIO_H__
#define __AP_HAL_LINUX_RCOUTPUT_NAVIO_H__

#include "AP_HAL_Linux.h"

class Linux::RCOutput_PCA9685 : public AP_HAL::RCOutput {
public:
    RCOutput_PCA9685(bool external_clock, uint8_t channel_offset,
                     uint8_t oe_pin_number);
    RCOutput_PCA9685(bool external_clock, uint8_t channel_offset,
                     AP_HAL::GPIO::VirtualPin oe_pin_number);
    ~RCOutput_PCA9685();
    void     init(void* machtnichts);
    void     reset_all_channels();
    void     set_freq(uint32_t chmask, uint16_t freq_hz);
    uint16_t get_freq(uint8_t ch);
    void     enable_ch(uint8_t ch);
    void     disable_ch(uint8_t ch);
    void     write(uint8_t ch, uint16_t period_us, int flags = 0) override;
    void     flush() override;
    uint16_t read(uint8_t ch);
    void     read(uint16_t* period_us, uint8_t len);

private:
    RCOutput_PCA9685(bool external_clock, uint8_t channel_offset);
    void reset();

    AP_HAL::Semaphore *_i2c_sem;
    AP_HAL::DigitalSource *_enable_pin;
    uint16_t _frequency;
    float _osc_clock;

    uint16_t _pending_write_mask;
    uint16_t *_pulses_buffer;

    bool _external_clock;
    uint8_t _channel_offset;

    struct {
        bool virt;
        union {
            AP_HAL::GPIO::VirtualPin vpin;
            uint8_t pin;
        };
    } _oe_pin;
};

#endif // __AP_HAL_LINUX_RCOUTPUT_NAVIO_H__
