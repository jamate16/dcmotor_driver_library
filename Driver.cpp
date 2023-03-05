#include "Driver.h"

DCMotorDriver::DCMotorDriver(uint pinA1, uint pinA2, float _pwm_freq, bool _pwm_phase_correct) {
    PWMGPIOs[0].pin = pinA1;
    PWMGPIOs[1].pin = pinA2;

    pwm_freq = _pwm_freq;
    pwm_phase_correct = _pwm_phase_correct;

    pwm_dc = 0;
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
    calculatePWMsClockTopAndDiv();

    // Configure a config struct
    pwm_config PWMs_config = pwm_get_default_config();
    pwm_config_set_phase_correct(&PWMs_config, pwm_phase_correct);
    pwm_config_set_clkdiv_int_frac(&PWMs_config, PWMDiv.int_, PWMDiv.fract);
    pwm_config_set_wrap(&PWMs_config, pwm_top);

    // Initialize PWM's slices with PWMs_config
    for (auto &gpio: PWMGPIOs) {
        pwm_init(gpio.slice, &PWMs_config, true); // If the last argument were false, the syncPWMsSlices function would only need to do pwm_set_mask_enabled()
        pwm_set_chan_level(gpio.slice, gpio.channel, pwm_dc);
    }
}

// Iterates over values of PWM top(wrap pint) searching for the first clock divider value that satisfies the desired frequency
void DCMotorDriver::calculatePWMsClockTopAndDiv() {
    uint32_t sys_clock_freq = clock_get_hz(clk_sys);

    /// \todo Implement guard clauses forr avoiding innecesary calculations based on MIN and MAX frequencies
    const float MIN_FREQ = static_cast<float>(sys_clock_freq) / ((TOP_MAX + 1) * (pwm_phase_correct + 1) * (DIV_INT_MAX + DIV_FRAC_MAX/16));
    const float MAX_FREQ = static_cast<float>(sys_clock_freq) / ((1 + 1) * (pwm_phase_correct + 1) * (DIV_INT_MIN));

    pwm_top = TOP_MAX; // Start with the max resolution for the PWMs' DC
    uint16_t top_decrement = 1; // To make the loop quicker, make this number bigger at the cost of frequency and DC inaccuracy.

    // Decrement the resolution of the PWM's DC until the desired frequency is achieved
    while (true) {
        if (pwm_top >= top_decrement) {
            calculateClockDiv(sys_clock_freq);
            if (!(PWMDiv.int_ < DIV_INT_MIN)) break; // Frecuency achieved with the highest DC resolution possible
            pwm_top -= top_decrement;
        } else {
            // std::cout << "\nCan't achieve that desired frequency" << std::endl;
            break;
        }
    }
}

void DCMotorDriver::calculateClockDiv(uint32_t sys_clock_freq) {
    // Calculate the value for the divider
    float div_float = sys_clock_freq / ((pwm_top + 1) * (pwm_phase_correct + 1) * pwm_freq);
    
    PWMDiv.int_only = static_cast<uint8_t>(div_float + 0.5);

    // Split the divider's integal part and decimal part into an 8 bit integer and a 4 bit integer respectively
    PWMDiv.int_ = static_cast<uint8_t>(div_float);
    uint8_t temp_div_frac = static_cast<uint8_t>((div_float - static_cast<int>(div_float))*16 + 0.5);
    PWMDiv.fract = (temp_div_frac <= DIV_FRAC_MAX) ? temp_div_frac: DIV_FRAC_MAX; // Max accuracy is 15/16 with this 8.4 clock divider

    // std::cout << "Div float: " << div_float << " number of posible DCs of: " << pwm_top+1 << '\n'; 
    // Frecuency values for comparison between not using the fract part of the divider and using it
    // float pwm_freq_real_div_int = (float)sys_clock_freq / ((top + 1) * (phase_correct + 1) * div.int_only); // Only with int part (fract rounded to closest int)
    // float pwm_freq_real_div_frac = (float)sys_clock_freq / ((top + 1) * (phase_correct + 1) * (div.int_ + div.fract/16.0)); // With int and fract
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
    // Enable the driver's PWM slices together with the slices that were enabled before this function's call
	pwm_set_mask_enabled(driver_gpio_slices_mask | (pwm_hw->en));
}

void DCMotorDriver::setPWMsDC(PWMDir dir, float dc) {
    // Saves the values to the object's attributes just in case they are needed
    pwm_dc = dc;
    pwm_dir = dir;

    /// \todo Create a variable that allows to set min and max value for each channel
    uint16_t pwm_level_sp = static_cast<uint16_t>(dc * pwm_top / 100 + 0.5); // Rounds the scaled dc float to int, casting the value will truncate. Also, dc is always positive
    switch (dir) {
        case PWMDir::cw:
            pwm_set_chan_level(PWMGPIOs[0].slice, PWMGPIOs[0].channel, pwm_level_sp);
            pwm_set_chan_level(PWMGPIOs[1].slice, PWMGPIOs[1].channel, 0);
            break;
        case PWMDir::ccw:
            pwm_set_chan_level(PWMGPIOs[0].slice, PWMGPIOs[0].channel, 0);
            pwm_set_chan_level(PWMGPIOs[1].slice, PWMGPIOs[1].channel, pwm_level_sp);
            break;
    }
}

