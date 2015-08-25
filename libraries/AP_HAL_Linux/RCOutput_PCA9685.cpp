
#include <AP_HAL/AP_HAL.h>
#include "GPIO.h"

#if CONFIG_HAL_BOARD_SUBTYPE == HAL_BOARD_SUBTYPE_LINUX_NAVIO

#include "RCOutput_PCA9685.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define PCA9685_ADDRESS            0x40 // All address pins low, Navio default

#define PCA9685_RA_MODE1           0x00
#define PCA9685_RA_MODE2           0x01
#define PCA9685_RA_LED0_ON_L       0x06
#define PCA9685_RA_LED0_ON_H       0x07
#define PCA9685_RA_LED0_OFF_L      0x08
#define PCA9685_RA_LED0_OFF_H      0x09
#define PCA9685_RA_ALL_LED_ON_L    0xFA
#define PCA9685_RA_ALL_LED_ON_H    0xFB
#define PCA9685_RA_ALL_LED_OFF_L   0xFC
#define PCA9685_RA_ALL_LED_OFF_H   0xFD
#define PCA9685_RA_PRE_SCALE       0xFE

#define PCA9685_MODE1_RESTART_BIT  (1 << 7)
#define PCA9685_MODE1_EXTCLK_BIT   (1 << 6)
#define PCA9685_MODE1_AI_BIT       (1 << 5)
#define PCA9685_MODE1_SLEEP_BIT    (1 << 4)
#define PCA9685_MODE1_SUB1_BIT     (1 << 3)
#define PCA9685_MODE1_SUB2_BIT     (1 << 2)
#define PCA9685_MODE1_SUB3_BIT     (1 << 1)
#define PCA9685_MODE1_ALLCALL_BIT  (1 << 0)
#define PCA9685_ALL_LED_OFF_H_SHUT (1 << 4)
#define PCA9685_MODE2_INVRT_BIT    (1 << 4)
#define PCA9685_MODE2_OCH_BIT      (1 << 3)
#define PCA9685_MODE2_OUTDRV_BIT   (1 << 2)
#define PCA9685_MODE2_OUTNE1_BIT   (1 << 1)
#define PCA9685_MODE2_OUTNE0_BIT   (1 << 0)

/*
 * Drift for internal oscillator
 * see: https://github.com/diydrones/ardupilot/commit/50459bdca0b5a1adf95
 * and https://github.com/adafruit/Adafruit-PWM-Servo-Driver-Library/issues/11
 */
#define PCA9685_INTERNAL_CLOCK (1.04f * 25000000.f)
#define PCA9685_EXTERNAL_CLOCK 24576000.f

using namespace Linux;

#define PWM_CHAN_COUNT 13

static const AP_HAL::HAL& hal = AP_HAL_BOARD_DRIVER;

RCOutput_PCA9685::RCOutput_PCA9685(bool external_clock,
                                   uint8_t channel_offset,
                                   uint8_t oe_pin_number) :
    _i2c_sem(NULL),
    _enable_pin(NULL),
    _frequency(50),
    _pulses_buffer(new uint16_t[PWM_CHAN_COUNT]),
    _external_clock(external_clock),
    _channel_offset(channel_offset),
    _oe_pin_number(oe_pin_number)
{
    if (_external_clock)
        _osc_clock = PCA9685_EXTERNAL_CLOCK;
    else
        _osc_clock = PCA9685_INTERNAL_CLOCK;
}

RCOutput_PCA9685::~RCOutput_PCA9685()
{
    delete [] _pulses_buffer;
}

void RCOutput_PCA9685::init(void* machtnicht)
{
    _i2c_sem = hal.i2c->get_semaphore();
    if (_i2c_sem == NULL) {
        hal.scheduler->panic(PSTR("PANIC: RCOutput_PCA9685 did not get "
                                  "valid I2C semaphore!"));
        return; /* never reached */
    }

    reset_all_channels();

    /* Set the initial frequency */
    set_freq(0, 50);

    /* Enable PCA9685 PWM */
    _enable_pin = hal.gpio->channel(_oe_pin_number);
    _enable_pin->mode(HAL_GPIO_OUTPUT);
    _enable_pin->write(0);
}

void RCOutput_PCA9685::reset_all_channels()
{
    if (!_i2c_sem->take(10)) {
        return;
    }

    uint8_t data[4] = {0x00, 0x00, 0x00, 0x00};
    hal.i2c->writeRegisters(PCA9685_ADDRESS, PCA9685_RA_ALL_LED_ON_L, 4, data);

    /* Wait for the last pulse to end */
    hal.scheduler->delay(2);

    _i2c_sem->give();
}

void RCOutput_PCA9685::set_freq(uint32_t chmask, uint16_t freq_hz)
{
    /* Correctly finish last pulses */
    for (int i = 0; i < PWM_CHAN_COUNT; i++) {
        write(i, _pulses_buffer[i]);
    }

    if (!_i2c_sem->take(10)) {
        return;
    }

    /* Shutdown before sleeping.
     * see p.14 of PCA9685 product datasheet
     */
    hal.i2c->writeRegister(PCA9685_ADDRESS, PCA9685_RA_ALL_LED_OFF_H, PCA9685_ALL_LED_OFF_H_SHUT);

    /* Put PCA9685 to sleep (required to write prescaler) */
    hal.i2c->writeRegister(PCA9685_ADDRESS, PCA9685_RA_MODE1, PCA9685_MODE1_SLEEP_BIT);

    /* Calculate prescale and save frequency using this value: it may be
     * different from @freq_hz due to rounding/ceiling. We use ceil() rather
     * than round() so the resulting frequency is never greater than @freq_hz
     */
    uint8_t prescale = ceil(_osc_clock / (4096 * freq_hz)) - 1;
    _frequency = _osc_clock / (4096 * (prescale + 1));

    /* Write prescale value to match frequency */
    hal.i2c->writeRegister(PCA9685_ADDRESS, PCA9685_RA_PRE_SCALE, prescale);

    if (_external_clock) {
        /* Enable external clocking */
        hal.i2c->writeRegister(PCA9685_ADDRESS, PCA9685_RA_MODE1,
                               PCA9685_MODE1_SLEEP_BIT | PCA9685_MODE1_EXTCLK_BIT);
    }

    /* Restart the device to apply new settings and enable auto-incremented write */
    hal.i2c->writeRegister(PCA9685_ADDRESS, PCA9685_RA_MODE1,
                            PCA9685_MODE1_RESTART_BIT | PCA9685_MODE1_AI_BIT);

    _i2c_sem->give();
}

uint16_t RCOutput_PCA9685::get_freq(uint8_t ch)
{
    return _frequency;
}

void RCOutput_PCA9685::enable_ch(uint8_t ch)
{

}

void RCOutput_PCA9685::disable_ch(uint8_t ch)
{
    write(ch, 0);
}

/* calls to write() and flush() must occur on the same thread */
void RCOutput_PCA9685::write(uint8_t ch, uint16_t period_us, int flags)
{
    if (ch >= PWM_CHAN_COUNT) {
        return;
    }

    _pulses_buffer[ch] = period_us;
    _pending_write_mask |= (1U << ch);

    if (flags & AP_HAL::RCOutput::FLAGS_ASYNC)
        return;

    flush();
}

/* calls to write() and flush() must occur on the same thread */
void RCOutput_PCA9685::flush()
{
    uint8_t data[PWM_CHAN_COUNT * 4] = { };
    uint8_t max_ch, min_ch;

    if (_pending_write_mask == 0)
        return;

    max_ch = (sizeof(unsigned) * 8) - __builtin_clz(_pending_write_mask);
    min_ch = __builtin_ctz(_pending_write_mask);
    _pending_write_mask = 0;

    for (unsigned ch = min_ch; ch < max_ch; ch++) {
        uint16_t width;

        if (_pulses_buffer[ch] == 0)
            width = 0;
        else
            width = round((_pulses_buffer[ch] * 4096) / (1000000.f / _frequency)) - 1;

        uint8_t *d = &data[ch * 4 + 2];
        *d++ = width && 0xFF;
        *d = width >> 8;
    }

    if (!_i2c_sem->take(100)) {
        hal.console->printf("RCOutput: Unable to get bus semaphore");
        return;
    }

    hal.i2c->writeRegisters(PCA9685_ADDRESS,
                            PCA9685_RA_LED0_ON_L + 4 * (_channel_offset + min_ch),
                            (max_ch - min_ch) * 4,
                            &data[min_ch * 4]);

    _i2c_sem->give();
}

uint16_t RCOutput_PCA9685::read(uint8_t ch)
{
    return _pulses_buffer[ch];
}

void RCOutput_PCA9685::read(uint16_t* period_us, uint8_t len)
{
    for (int i = 0; i < len; i++)
        period_us[i] = read(0 + i);
}

#endif // CONFIG_HAL_BOARD_SUBTYPE == HAL_BOARD_SUBTYPE_LINUX_NAVIO
