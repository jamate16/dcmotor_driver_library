#include "Driver.h"

DCMotorDriver::DCMotorDriver(uint pinA1, uint pinA2, float _pwm_freq) {
    PWMGPIOs[0].pin = pinA1;
    PWMGPIOs[1].pin = pinA2;

    pwm_freq = _pwm_freq;
}

void DCMotorDriver::init() {
    setPWMsGPIO();
    setPWMsFreq();
    syncPWMsSlices();
}

void DCMotorDriver::setPWMsGPIO() {
    for (auto &gpio: PWMGPIOs){
        gpio_set_function(gpio.pin, GPIO_FUNC_PWM);

        gpio.slice = pwm_gpio_to_slice_num(gpio.pin);
        gpio.channel = pwm_gpio_to_channel(gpio.pin);
    }
}

void DCMotorDriver::setPWMsFreq() {
    uint16_t wrap = 0;

    clock_get_hz()
    pwm_set_clkdiv_int_frac();

    for (auto &gpio: PWMGPIOs) {
        pwm_config my_config = pwm_get_default_config();
        pwm_config_set_phase_correct(&my_config, true);
        pwm_config_set_wrap(&my_config, 10000);
        pwm_init(gpio.slice, &my_config, true); // If the last argument is false, I could do pwm_set_mask_enabled and they'd be synchronized
    }
}

void DCMotorDriver::syncPWMsSlices() {
    uint32_t driver_gpio_slices_mask = 0l; // Mask for syncing slices. All bits set to zero

    for (auto &gpio: PWMGPIOs) {
        // Disable slice and set its counter to 0
		pwm_set_enabled(gpio.slice, false);
		pwm_set_counter(gpio.slice, 0); // Resetting the counter is necessary; otherwise, when the PWM is enabled again, it will start from the value at which it was halted
        
        // Turn corresponding bit to 1
        driver_gpio_slices_mask |= (1u << gpio.slice);
	}
    // Sync the driver's PWM slices taking into account the slices that were enabled before this function's call
	pwm_set_mask_enabled(driver_gpio_slices_mask | (pwm_hw->en));
}
