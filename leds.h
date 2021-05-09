#pragma once

enum Led
{
    LED_1 = 0x01,
    LED_ALARM = LED_1,

    LED_2 = 0x02,
    LED_LOS_LCL = LED_2,

    LED_3 = 0x04,
    LED_LOS_RMT = LED_3,

    LED_4 = 0x08,
    LED_LPBK = LED_4,

    LED_5 = 0x10,
    LED_NORM = LED_5,

    LED_6 = 0x20,
    LED_LOS_1 = LED_6,

    LED_7 = 0x40,
    LED_LOS_2 = LED_7
};

void led_init(void);

void led_set_single(enum Led led);
void led_unset_single(enum Led led);