#include "app_head.h"

#include "dev_sign_api.h"
#include "mqtt_api.h"

// 引入CJSON库
#include "cJSON.h"


#define GPIO_LED_B    GET_PIN(F, 11)


#define LOG_TAG              "app_mqtt"
#define LOG_LVL              LOG_LVL_DBG
#include <ulog.h>

extern rt_mq_t mq_hum;
extern rt_mq_t mq_tem;
extern rt_mq_t mq_brightness;
extern rt_mq_t mq_distance;

static char DEMO_PRODUCT_KEY[IOTX_PRODUCT_KEY_LEN + 1] = {0};
static char DEMO_DEVICE_NAME[IOTX_DEVICE_NAME_LEN + 1] = {0};
static char DEMO_DEVICE_SECRET[IOTX_DEVICE_SECRET_LEN + 1] = {0};

void *HAL_Malloc(uint32_t size);
void HAL_Free(void *ptr);
void HAL_Printf(const char *fmt, ...);
int HAL_GetProductKey(char product_key[IOTX_PRODUCT_KEY_LEN + 1]);
int HAL_GetDeviceName(char device_name[IOTX_DEVICE_NAME_LEN + 1]);
int HAL_GetDeviceSecret(char device_secret[IOTX_DEVICE_SECRET_LEN]);
uint64_t HAL_UptimeMs(void);
int HAL_Snprintf(char *str, const int len, const char *fmt, ...);

#define EXAMPLE_TRACE(fmt, ...)  \
    do { \
        HAL_Printf("%s|%03d :: ", __func__, __LINE__); \
        HAL_Printf(fmt, ##__VA_ARGS__); \
        HAL_Printf("%s", "\r\n"); \
    } while(0)

/* 数据解析函数, 从云端接收到的数据为json格式，解析后放入消息队列中 */
/* 进而控制红色led、蓝色led、beep、led矩阵的状态，数据格式如下 */
/* {"cmd":"lednoff"} 后面的lednoff为具体控制字段*/
/* {"cmd":"redledoff"} {"cmd":"redledoff"} {"cmd":"blueledon"} {"cmd":"blueledoff"}
{"cmd":"beepon"} {"cmd":"beepoff"}
 {"cmd":"ledn0"}{"cmd":"ledn0"}----{"cmd":"ledn5"} 代表led矩阵的不同模式*/
static void example_message_arrive(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    iotx_mqtt_topic_info_t     *topic_info = (iotx_mqtt_topic_info_pt) msg->msg;

    switch (msg->event_type) {
        case IOTX_MQTT_EVENT_PUBLISH_RECEIVED:
            /* print topic name and topic message */
            EXAMPLE_TRACE("Message Arrived:");
            EXAMPLE_TRACE("Topic  : %.*s", topic_info->topic_len, topic_info->ptopic);
            EXAMPLE_TRACE("Payload: %.*s", topic_info->payload_len, topic_info->payload);
            EXAMPLE_TRACE("\n");
            // 解析json数据
            cJSON *root = cJSON_Parse(topic_info->payload);
            if (root == NULL) {
                EXAMPLE_TRACE("json parse error");
                return;
            }
            cJSON *cmd = cJSON_GetObjectItem(root, "cmd");
            if (cmd == NULL) {
                EXAMPLE_TRACE("json parse error");
                cJSON_Delete(root);
                return;
            }
            // 打印解析后的数据
            EXAMPLE_TRACE("cmd: %s",cmd->valuestring);
            // 控制红色led
            if (strcmp(cmd->valuestring, "redledon") == 0) {
                rt_pin_write(GPIO_LED_R, PIN_LOW);
            } else if (strcmp(cmd->valuestring, "redledoff") == 0) {
                rt_pin_write(GPIO_LED_R, PIN_HIGH);
            }
            // 控制蓝色led
            else if (strcmp(cmd->valuestring, "blueledon") == 0) {
                rt_pin_write(GPIO_LED_B, PIN_LOW);
            } else if (strcmp(cmd->valuestring, "blueledoff") == 0) {
                rt_pin_write(GPIO_LED_B, PIN_HIGH);
            }
            // 控制beep
            else if (strcmp(cmd->valuestring, "beepon") == 0) {
                rt_pin_write(PIN_BEEP,PIN_HIGH);
            } else if (strcmp(cmd->valuestring, "beepoff") == 0) {
                rt_pin_write(PIN_BEEP,PIN_LOW);
            }
            // 控制led矩阵,修改led_matrix_status的值
             else if (strcmp(cmd->valuestring, "lednoff") == 0) {
                led_matrix_status=0;
            }else if (strcmp(cmd->valuestring, "ledn0") == 0) {
                led_matrix_status=1;
            } else if (strcmp(cmd->valuestring, "ledn1") == 0) {
                led_matrix_status=2;
            } else if (strcmp(cmd->valuestring, "ledn2") == 0) {
                led_matrix_status=3;
            } else if (strcmp(cmd->valuestring, "ledn3") == 0) {
                led_matrix_status=4;
            } else if (strcmp(cmd->valuestring, "ledn4") == 0) {
                led_matrix_status=5;
            } else if (strcmp(cmd->valuestring, "ledn5") == 0) {
                led_matrix_status=6;
            }else if (strcmp(cmd->valuestring, "ledn6") == 0) {
                led_matrix_status=7;
            }else if (strcmp(cmd->valuestring, "ledn7") == 0) {
                led_matrix_status=8;
            }else if (strcmp(cmd->valuestring, "ledn8") == 0) {
                led_matrix_status=9;
            }

            break;
        default:
            break;
    }
}

static int example_subscribe(void *handle)
{
    int res = 0;
    const char *fmt = "/%s/%s/user/get";
    char *topic = NULL;
    int topic_len = 0;

    topic_len = strlen(fmt) + strlen(DEMO_PRODUCT_KEY) + strlen(DEMO_DEVICE_NAME) + 1;
    topic = HAL_Malloc(topic_len);
    if (topic == NULL) {
        EXAMPLE_TRACE("memory not enough");
        return -1;
    }
    memset(topic, 0, topic_len);
    HAL_Snprintf(topic, topic_len, fmt, DEMO_PRODUCT_KEY, DEMO_DEVICE_NAME);

    res = IOT_MQTT_Subscribe(handle, topic, IOTX_MQTT_QOS0, example_message_arrive, NULL);
    if (res < 0) {
        EXAMPLE_TRACE("subscribe failed");
        HAL_Free(topic);
        return -1;
    }

    HAL_Free(topic);
    return 0;
}


// 设备模拟器发送的数据格式为json格式，云平台到设备消息
// 该函数用来从消息队列中读取温湿度数据组合成payload数据格式并传入example_publish函数
static void mqtt_tranfer_sensor(void *pclient)
{
    float humidity;
    float temperature;
    float brightness;
    rt_uint16_t distance;
    char *payload = NULL;
    int payload_len = 0,res = 0;

    const char     *fmt = "/sys/%s/%s/thing/event/property/post";
    char           *topic = NULL;
    int topic_len = 0;

    topic_len = strlen(fmt) + strlen(DEMO_PRODUCT_KEY) + strlen(DEMO_DEVICE_NAME) + 1;
    topic = HAL_Malloc(topic_len);
    if (topic == NULL) {
        EXAMPLE_TRACE("memory not enough");
        return;
    }
    memset(topic, 0, topic_len);
    HAL_Snprintf(topic, topic_len, fmt, DEMO_PRODUCT_KEY, DEMO_DEVICE_NAME);

    rt_mq_recv(mq_hum, &humidity, sizeof(humidity), RT_WAITING_NO);
    // LOG_D("humidity   : %d.%d %%", (int)humidity, (int)(humidity * 10) % 10);
    /* 从队列中读取温度并打印 */
    rt_mq_recv(mq_tem, &temperature, sizeof(temperature), RT_WAITING_NO);
    // LOG_D("temperature: %d.%d", (int)temperature, (int)(temperature * 10) % 10);

    rt_mq_recv(mq_brightness, &brightness, sizeof(brightness), RT_WAITING_NO);
    // LOG_D("brightness: %d.%d", (int)brightness, (int)(brightness * 10) % 10);

    rt_mq_recv(mq_distance, &distance, sizeof(distance), RT_WAITING_NO);
    // LOG_D("distance: %d.%d", (int)distance, (int)(distance * 10) % 10);

    // 判断数据是否合法
    if (humidity < 0 || humidity > 100 || temperature < -40 || temperature > 85) {
        EXAMPLE_TRACE("data error");
        return;
    }

    //计算payload所需长度，温度湿度数据保留一位小数上传，"params":{"CurrentTemperature":16.5,"CurrentHumidity":56.3,"LightValue":1000.0,"DetectDistance":1000.0}
    payload_len = strlen("{\"params\":{\"CurrentTemperature\":16.5,\"CurrentHumidity\":56.3,\"LightValue\":1000.0,\"DetectDistance\":1000}}") + 5;
    payload = HAL_Malloc(payload_len);
    if (payload == NULL) {
        EXAMPLE_TRACE("memory not enough");
        return;
    }
    memset(payload, 0, payload_len);
    HAL_Snprintf(payload, payload_len, "{\"params\":{\"CurrentTemperature\":%.1f,\"CurrentHumidity\":%.1f,\"LightValue\":%.1f,\"DetectDistance\":%d}}", temperature, humidity, brightness, (int)distance);
    
    res = IOT_MQTT_Publish_Simple(0, topic, IOTX_MQTT_QOS0, payload, strlen(payload));
    if (res < 0) {
        EXAMPLE_TRACE("publish failed, res = %d", res);
        HAL_Free(topic);
        HAL_Free(payload);
        return;
    }
    HAL_Free(topic);
    HAL_Free(payload);
}


static void example_event_handle(void *pcontext, void *pclient, iotx_mqtt_event_msg_pt msg)
{
    EXAMPLE_TRACE("msg->event_type : %d", msg->event_type);
}


static void mqtt_example_main(void *parameter)
{
    // 获取WiFi连接成功信号量，等待WiFi连接成功后再进行mqtt连接
    rt_sem_take(wifi_connect_sem, RT_WAITING_FOREVER);
    rt_sem_take(ap3216_connect_sem, RT_WAITING_FOREVER);
    rt_sem_take(aht32_connect_sem, RT_WAITING_FOREVER);

    void                   *pclient = NULL;
    int                     res = 0;
    int                     loop_cnt = 0;
    iotx_mqtt_param_t       mqtt_params;

    HAL_GetProductKey(DEMO_PRODUCT_KEY);
    HAL_GetDeviceName(DEMO_DEVICE_NAME);
    HAL_GetDeviceSecret(DEMO_DEVICE_SECRET);

    EXAMPLE_TRACE("mqtt example");

    /* Initialize MQTT parameter */

    memset(&mqtt_params, 0x0, sizeof(mqtt_params));

    mqtt_params.handle_event.h_fp = example_event_handle;

    pclient = IOT_MQTT_Construct(&mqtt_params);
    if (NULL == pclient) {
        EXAMPLE_TRACE("MQTT construct failed");
        return;
    }

    res = example_subscribe(pclient);
    if (res < 0) {
        IOT_MQTT_Destroy(&pclient);
        return;
    }

    while (1) {
        if (0 == loop_cnt % 10) {
            mqtt_tranfer_sensor(pclient);
        }

        IOT_MQTT_Yield(pclient, 200);

        loop_cnt += 1;
    }

    return;
}
// #ifdef FINSH_USING_MSH
// MSH_CMD_EXPORT_ALIAS(mqtt_example_main, ali_mqtt_sample, ali coap sample);
// #endif

static char mqtt_thread_stack[4096];
static struct rt_thread mqtt_thread;

void app3_mqtt_example(void)
{
    // 提示信息
    rt_kprintf("creating mqtt thread\n");

    rt_thread_init(&mqtt_thread, "mqtt_thread", mqtt_example_main, RT_NULL, mqtt_thread_stack, sizeof(mqtt_thread_stack), 20, 10);
    rt_thread_startup(&mqtt_thread);

    // 提示信息
    rt_kprintf("creating mqtt rt_thread_startup\n");

}

MSH_CMD_EXPORT(app3_mqtt_example, app_ali_mqtt_sample);

