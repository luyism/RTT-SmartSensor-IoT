#ifndef APP_HEAD_H
#define APP_HEAD_H

#include <rtthread.h>
#include <board.h>
#include <rtthread.h>
#include <drv_gpio.h>
#include <rtdevice.h>

#define GPIO_LED_B    GET_PIN(F, 11)
#define GPIO_LED_R    GET_PIN(F, 12)

/* 配置 KEY 输入引脚  */
#define PIN_KEY1        GET_PIN(C, 1)      // PC1:  KEY1         --> KEY
#define PIN_WK_UP       GET_PIN(C, 5)      // PC5:  WK_UP        --> KEY

/* 配置蜂鸣器引脚 */
#define PIN_BEEP        GET_PIN(B, 0)      // PA1:  BEEP         --> BEEP (PB1)


extern rt_sem_t wifi_connect_sem;
extern rt_sem_t ap3216_connect_sem;
extern rt_sem_t aht32_connect_sem;

extern volatile u_int8_t led_matrix_status;


void app1_wifi_connect(void);
void app2_ap3216_example(void);
void app2_aht21_example(void);
void app3_mqtt_example(void);
void led_matrix_example(void);



/* define LED  */
enum{
    EXTERN_LED_0,
    EXTERN_LED_1,
    EXTERN_LED_2,
    EXTERN_LED_3,
    EXTERN_LED_4,
    EXTERN_LED_5,
    EXTERN_LED_6,
    EXTERN_LED_7,
    EXTERN_LED_8,
    EXTERN_LED_9,
    EXTERN_LED_10,
    EXTERN_LED_11,
    EXTERN_LED_12,
    EXTERN_LED_13,
    EXTERN_LED_14,
    EXTERN_LED_15,
    EXTERN_LED_16,
    EXTERN_LED_17,
    EXTERN_LED_18,
};


#endif // APP_HEAD_H