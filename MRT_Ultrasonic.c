#include "hardware/pwm.h"
#include "hardware/gpio.h"
#include "pico/time.h"
#include "pico/stdlib.h"

#include <stdio.h>

#define PWM_PIN 0
#define TRIGGER_PIN 4
#define ECHO_PIN 5
#define WRAP 41666 // 125 MHz clock / 3 kHz PWM frequency

#define DIST1 100
#define DIST2 1000
#define DIST3 2000

/**
 * @brief Determines the high time of the buzzer based off linear interpolation.
 *
 * @param distance_mm Distance in mm.
 * @return int High time in ms.
 */
int determine_high_time(int distance_mm)
{
    if (distance_mm < DIST1)
    {
        return 300;
    }
    else if (distance_mm < DIST2 && distance_mm >= DIST1)
    {
        return (int)((double)distance_mm * 2 / 9 + 277.7);
    }
    else if (distance_mm < DIST3 && distance_mm >= DIST2)
    {
        return (int)((double)distance_mm * 0.5);
    }
    else
    {
        return 1000;
    }
}

/**
 * @brief Determines the low time of the buzzer based off linear interpolation.
 *
 * @param distance_mm Distance in mm.
 * @return int Low time in ms.
 */
int determine_low_time(int distance_mm)
{
    if (distance_mm < DIST1)
    {
        return 300;
    }
    else if (distance_mm < DIST2 && distance_mm >= DIST1)
    {
        return (int)((double)distance_mm * 4 / 9 + 255.6);
    }
    else if (distance_mm < DIST3 && distance_mm >= DIST2)
    {
        return (int)((double)distance_mm * 13 / 10 - 600);
    }
    else
    {
        return 2000;
    }
}

/**
 * @brief Initializes the GPIO and PWM.
 *
 * @param slice_num Pointer to the slice number.
 * @param chan Pointer to the channel number.
 */
void init(uint *slice_num, uint *chan)
{
    // For serial output
    stdio_init_all();
    // GPIO init:
    gpio_init(TRIGGER_PIN);
    gpio_set_dir(TRIGGER_PIN, GPIO_OUT);
    gpio_init(ECHO_PIN);
    gpio_set_dir(ECHO_PIN, GPIO_IN);
    // PWM Init:
    gpio_set_function(PWM_PIN, GPIO_FUNC_PWM);
    *slice_num = pwm_gpio_to_slice_num(PWM_PIN);
    *chan = pwm_gpio_to_channel(PWM_PIN);
    pwm_set_wrap(*slice_num, WRAP);
    pwm_set_enabled(*slice_num, true);
}

/**
 * @brief Reads the distance from the ultrasonic sensor.
 *
 * @return int Distance in mm.
 */
int readSensor()
{
    gpio_put(TRIGGER_PIN, 0);
    sleep_us(3); // ensure Pin is really low
    gpio_put(TRIGGER_PIN, 1);
    sleep_us(10); // sending ultrasound signal
    gpio_put(TRIGGER_PIN, 0);

    // measure travel time of the ultrasound which is represented by the high time of echo pin
    uint64_t start = time_us_64();

    // wait until echo pin is high
    while (gpio_get(ECHO_PIN) == 0)
    {
        if (time_us_64() - start > 10000000)
        {
            return -1; // if no echo is received after 10s, return -1
        }
    }
    while (gpio_get(ECHO_PIN) == 1)
    {
        continue;
    }
    uint64_t duration = time_us_64() - start;

    return (int)((double)duration / 2 * 0.343); // value in mm (0.5 time for half way * speed of sound)
}

/**
 * @brief Sets the volume of the buzzer by changing the pwm compare value.
 * @brief PWM compare influences duty cycle which influences volume.
 * @brief The volume is interpolated linearly proportional to the distance.
 *
 * @param distance_mm Distance in mm.
 * @param pwm_compare Pointer to the pwm compare value.
 */
void set_volume(int distance_mm, float *pwm_compare)
{
    if (distance_mm < DIST1)
    {
        *pwm_compare = 2e-4f * (float)DIST3;
    }
    else if (distance_mm > DIST3)
    {
        *pwm_compare = 2e-4f * (float)DIST1;
    }
    *pwm_compare = 2e-4f * (float)DIST3 - 2e-4f * (float)distance_mm;
}

/**
 * @brief Main function.
 */
int main()
{
    int distance_mm = 1000;
    uint slice_num, chan;
    float pwm_compare;
    init(&slice_num, &chan);
    pwm_set_chan_level(slice_num, PWM_CHAN_A, WRAP / 2);

    while (true)
    {
        distance_mm = readSensor();
        set_volume(distance_mm, &pwm_compare);
        pwm_set_chan_level(slice_num, PWM_CHAN_A, WRAP * pwm_compare);
        sleep_ms(determine_high_time(distance_mm));
        distance_mm = readSensor(); // update distance to be more responsive
        pwm_set_chan_level(slice_num, PWM_CHAN_A, 0);
        sleep_ms(determine_low_time(distance_mm));
    }
    return 0;
}