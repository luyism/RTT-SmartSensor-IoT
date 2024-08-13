#include "app_head.h"

#include <drv_matrix_led.h>
// 声明menset头文件
#include <stdio.h>
#include <string.h>

#define LED_NUM     19

volatile u_int8_t led_matrix_status;
static RGBColor_TypeDef RGB_Data[LED_NUM] = {0};

// 辅助函数：HSV 转 RGB（用于彩虹渐变显示）
RGBColor_TypeDef HSV_to_RGB(uint8_t hue) {
    uint8_t region = hue / 43;
    uint8_t remainder = (hue - (region * 43)) * 6;

    uint8_t p = 0;
    uint8_t q = (255 * (255 - remainder)) / 255;
    uint8_t t = (255 * remainder) / 255;

    switch (region) {
        case 0:
            return (RGBColor_TypeDef){255, t, p};
        case 1:
            return (RGBColor_TypeDef){q, 255, p};
        case 2:
            return (RGBColor_TypeDef){p, 255, t};
        case 3:
            return (RGBColor_TypeDef){p, q, 255};
        case 4:
            return (RGBColor_TypeDef){t, p, 255};
        default:
            return (RGBColor_TypeDef){255, p, q};
    }
}




// 定义 LED 矩阵控制模式函数:滚动显示效果
static void led_matrix_mode1(void) {
    static int offset = 0;
    RGBColor_TypeDef colors[] = {RED, GREEN, BLUE, WHITE};
    for (int i = 0; i < LED_NUM; i++) {
        Set_LEDColor(i, colors[(i + offset) % 4]);
    }
    offset = (offset + 1) % LED_NUM;
    RGB_Reflash();
}

static void led_matrix_mode2(void) {
    // 模式2的实现，例如：流水灯效果
    static int current_led = 0;
    RGBColor_TypeDef color = {255, 0, 0}; // 红色
    memset(RGB_Data, 0, sizeof(RGB_Data)); // 清空所有LED
    Set_LEDColor(current_led, color);
    current_led = (current_led + 1) % LED_NUM;
    RGB_Reflash();
}

static void led_matrix_mode3(void) {
    // 模式3的实现，例如：逐点点亮效果
    static int current_led = 0;
    RGBColor_TypeDef color = {0, 255, 0}; // 绿色
    if (current_led < LED_NUM) {
        Set_LEDColor(current_led, color);
        current_led++;
    } else {
        memset(RGB_Data, 0, sizeof(RGB_Data)); // 清空所有LED
        current_led = 0;
    }
    RGB_Reflash();
}

static void led_matrix_mode4(void) {
    // 模式4的实现，例如：红绿蓝循环显示
    static int color = 0;
    RGBColor_TypeDef colors[] = {RED, GREEN, BLUE};
    for (int i = 0; i < LED_NUM; i++) {
        Set_LEDColor(i, colors[color]);
    }
    color = (color + 1) % 3;
    RGB_Reflash();
}

static void led_matrix_mode5(void) {
    // 模式5的实现，例如：彩虹流动效果
    static int hue = 0;
    for (int i = 0; i < LED_NUM; i++) {
        Set_LEDColor(i, HSV_to_RGB((hue + i * 10) % 256));
    }
    hue = (hue + 1) % 256;
    RGB_Reflash();
}

static void led_matrix_mode6(void) {
    // 模式6的实现，例如：呼吸灯效果
    static uint8_t brightness = 0;
    static int8_t direction = 1;
    for (int i = 0; i < LED_NUM; i++) {
        RGBColor_TypeDef color = {brightness, brightness, brightness};
        Set_LEDColor(i, color);
    }
    brightness += direction * 5;
    if (brightness == 255 || brightness == 0) {
        direction = -direction;
    }
    RGB_Reflash();
}

static void led_matrix_mode7(void) {
    // 模式7的实现，例如：彩虹渐变显示
    static uint8_t hue = 0;
    for (int i = 0; i < LED_NUM; i++) {
        Set_LEDColor(i, HSV_to_RGB(hue + i * 10));
    }
    hue = (hue + 1) % 256;
    RGB_Reflash();
}

// 波浪效果
static void led_matrix_mode8(void) {
    static int wave_position = 0;
    for (int i = 0; i < LED_NUM; i++) {
        int brightness = (int)(127.5 * (1 + sin((i + wave_position) * 0.5)));
        RGBColor_TypeDef color = {brightness, brightness, brightness}; // 灰色波浪
        Set_LEDColor(i, color);
    }
    wave_position = (wave_position + 1) % LED_NUM;
    RGB_Reflash();
}


// 定义函数指针数组
typedef void (*LedMatrixModeFunc)(void);
LedMatrixModeFunc led_matrix_modes[] = {
    led_matrix_rst,
    led_matrix_mode1,
    led_matrix_mode2,
    led_matrix_mode3,
    led_matrix_mode4,
    led_matrix_mode5,
    led_matrix_mode6,
    led_matrix_mode7,
    led_matrix_mode8
};

// led_matrix线程入口函数
static void led_matrix_thread_entry(void *parameter) {
    int status = 0;
    while (1) {
        // 根据status切换模式
        if(led_matrix_status>8)
        {
            led_matrix_status=0;
        }
        led_matrix_modes[led_matrix_status]();
        rt_thread_mdelay(50); 
    }
}

// 创建并启动线程
void led_matrix_example(void) {
    // 提示信息
    rt_kprintf("creating led_matrix thread\n");
    rt_thread_t led_matrix_thread = rt_thread_create("led_matrix_thread", led_matrix_thread_entry, RT_NULL, 1024, 20, 20);
    if (led_matrix_thread == RT_NULL) {
        rt_kprintf("led matrix thread create failed!\n");
    } else {
        rt_thread_startup(led_matrix_thread);
    }
    rt_kprintf("creating led_matrix rt_thread_startup\n");
}





