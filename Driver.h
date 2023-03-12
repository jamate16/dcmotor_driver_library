#pragma once

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/clocks.h"

#define TOP_MAX 65534
#define DIV_INT_MIN 1.f
#define DIV_INT_MAX 255.f
#define DIV_FRAC_MAX 15.f

typedef struct PWMGPIO {
    uint pin;
    uint slice;
    uint channel;
} PWMGPIO;

typedef struct PWMClockDivider {
    uint8_t int_;
    uint8_t fract;
    uint8_t int_only; // Use this just in case the frac part of the divider is not going to be used (less PWM frequency precision)
} PWMClockDivider;

enum class MotorDir {
    cw,
    ccw
};

class DCMotorDriver {
public: // Public attributes
    PWMGPIO PWMGPIOs[2];
    PWMClockDivider PWMDiv;
    uint16_t pwm_top; // Warp value, this value determines the resolution of the PWM's Duty Cycle

    float pwm_freq;
    bool pwm_phase_correct; // Only set true if phase correct is needed. Remember DC resolution is less in this mode
public: // Public methods
    DCMotorDriver(uint pinA1, uint pinA2, float _pwm_freq, bool _pwm_phase_correct=false);
    /**
     * @brief Initializes gpios and pwms. Current core's clock frequency shouldn't be changed after calling this method
     * 
     */
    void init();
    void setPWMsGPIO();
    void setPWMsFreq();
    /**
     * @brief Sets one GPIO ouput to DC% > 0 and the other to DC% = 0 depending on the direction
     * 
     * @param dir Direction of Motor. cw: PWMGPIOs[1] at zero% dc, ccw: PWMGPIOs[0] at zero% dc 
     * @param dc Duty cycle of the GPIO that isn't at zero% dc. Only positive values
     */
    void setPWMsDC(MotorDir dir, float dc);

    float getPWMDC();
    MotorDir getMotorDir();

private: // Private attributes
    float pwm_dc;
    MotorDir pwm_dir; // Since only one GPIO is outputting pwm at one time, we have pwm_dir and pwm_dc instead of a dc value for each GPIO

private: // Private methods
    void syncPWMsSlices();
    void calculatePWMsClockTopAndDiv();
    void calculateClockDiv(uint32_t sys_clock_freq);
};