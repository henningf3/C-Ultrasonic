#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "pico/time.h"

#define PWM_PIN 0
#define TRIGGER_PIN 4
#define ECHO_PIN 5
#define WRAP 41666 // 125 MHz clock / 3 kHz PWM frequency

/*
 * This function generates buzzer output.
 * Signal and break times are set by PWM.
 * Volume is set by reference voltage.
 */
//return value is in milliseconds
int determine_high_time(int distance_mm) {
    if (distance_mm < 100) {
        return 300;
    } else if (distance_mm < 1000 && distance_mm >= 100) {
        return (int) ((double) distance_mm * 2 / 9 + 277.7);
    } else if (distance_mm < 2000 && distance_mm >= 1000) {
        return (int) ((double) distance_mm * 0.5);
    } else {
        return 1000;
    }
}

int determine_low_time(int distance_mm) {
    if (distance_mm < 100) {
        return 300;
    } else if (distance_mm < 1000 && distance_mm >= 100) {
        return (int) ((double) distance_mm * 4 / 9 + 255.6);
    } else if (distance_mm < 2000 && distance_mm >= 1000) {
        return (int) ((double) distance_mm * 13 / 10 - 600);
    } else {
        return 2000;
    }
}


void init(uint *slice_num, uint *chan) {
    //GPIO init:
    gpio_init(TRIGGER_PIN);
    gpio_set_dir(TRIGGER_PIN, GPIO_OUT);
    gpio_init(ECHO_PIN);
    gpio_set_dir(ECHO_PIN, GPIO_IN);
    //PWM Init:
    gpio_set_function(PWM_PIN, GPIO_FUNC_PWM);
    *slice_num = pwm_gpio_to_slice_num(PWM_PIN);
    *chan = pwm_gpio_to_channel(PWM_PIN);
    pwm_set_wrap(*slice_num, WRAP);
    pwm_set_chan_level(*slice_num, *chan, WRAP);
    pwm_set_enabled(*slice_num, true);

}

//returns distance in mm
int read_buzzer() {
    gpio_put(TRIGGER_PIN, 0);
    sleep_ms(1); //ensure Pin is really low
    gpio_put(TRIGGER_PIN, 1);
    sleep_ms(10);
    gpio_put(TRIGGER_PIN, 0);
    uint64_t start = time_us_64();
    //wait until echo pin is high
    while (gpio_get(ECHO_PIN) == 0) {
        if (time_us_64() - start > 10000000) {
            return -1;
        }
    }
    while (gpio_get(ECHO_PIN) == 1) {
        continue;
    }
    uint64_t duration = time_us_64()-start;
    return (int) ((double) duration/2 * 0.343); //value in mm (0.5 time for half way * speed of sound)
}

int main() {
    int distance_mm=1000;
    uint slice_num, chan;
    init(&slice_num, &chan);
    while (true) {
        distance_mm = read_buzzer();
        pwm_set_chan_level(slice_num, PWM_CHAN_A, WRAP/2);
        sleep_ms(determine_high_time(distance_mm));
        pwm_set_chan_level(slice_num, PWM_CHAN_A, 0);
        sleep_ms(determine_low_time(distance_mm));
    }
    return 0;
}