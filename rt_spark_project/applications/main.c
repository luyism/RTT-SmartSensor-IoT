/*
 * Copyright (c) 2023, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-07-06     Supperthomas first version
 * 2023-12-03     Meco Man     support nano version
 */

#include "app_head.h"


int main(void)
{
    rt_pin_mode(GPIO_LED_R, PIN_MODE_OUTPUT);
    rt_pin_mode(GPIO_LED_B, PIN_MODE_OUTPUT);
    rt_pin_write(GPIO_LED_R, PIN_HIGH);
    rt_pin_write(GPIO_LED_B, PIN_HIGH);

    /* 设置蜂鸣器引脚为输出模式 */
    rt_pin_mode(PIN_BEEP, PIN_MODE_OUTPUT);

    app1_wifi_connect();
    app2_ap3216_example();
    app2_aht21_example();
    led_matrix_example();
    rt_thread_mdelay(8000);
    app3_mqtt_example();


    while (1)
    {
        rt_thread_mdelay(200);

    }
}

// cs:90 (f-a)*16+10
#define WIFI_CS GET_PIN(F,10)
int WIFI_CS_PULL_DOWN(void)
{
    rt_pin_mode(WIFI_CS, PIN_MODE_OUTPUT);
    rt_pin_write(WIFI_CS, PIN_LOW);
    return RT_EOK;
}
INIT_BOARD_EXPORT(WIFI_CS_PULL_DOWN);

