#pragma once

#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

typedef struct PWMChannel {
    uint pin;
    uint slice;
    uint channel;
} PWMChannel;

class DCMotorDriver {
public: // Public attributes
    PWMChannel PWMGPIOs[2];

    float pwm_freq;
public: // Public methods
    DCMotorDriver(uint pinA1, uint pinA2, float pwm_freq);

    void init();
    void setPWMsGPIO();
    void setPWMsFreq();
    void setPWMsDC();

private: // Private methods
    void syncPWMsSlices();
};