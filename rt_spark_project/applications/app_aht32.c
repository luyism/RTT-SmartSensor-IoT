#include "app_head.h"

#include "aht10.h"

#define LOG_TAG              "app_aht21"
#define LOG_LVL              LOG_LVL_DBG
#include <ulog.h>

static float humidity, temperature;
static aht10_device_t dev;
static int count;
/* 总线名称 */
static const char *i2c_bus_name = "i2c3";

//新建一个线程专门用于读取传感器温湿度数据并将数据分别放入两个消息队列中
static rt_thread_t thread1 = RT_NULL;
rt_mq_t mq_hum = RT_NULL;
rt_mq_t mq_tem = RT_NULL;

// 新建信号量，用于线程创建成功后的回调启动mqtt连接
rt_sem_t aht32_connect_sem = RT_NULL;


/* 定义一个线程入口 */
static void thread_aht10_entry(void *parameter)
{
    // 释放信号量
    rt_sem_release(aht32_connect_sem);

    while (1)
    {
        /* 读取湿度 */
        humidity = aht10_read_humidity(dev);
        // LOG_D("humidity   : %d.%d %%", (int)humidity, (int)(humidity * 10) % 10);
        rt_mq_urgent(mq_hum, &humidity, sizeof(humidity));

        /* 读取温度 */
        temperature = aht10_read_temperature(dev);
        // LOG_D("temperature: %d.%d", (int)temperature, (int)(temperature * 10) % 10);
        rt_mq_urgent(mq_tem, &temperature, sizeof(temperature));
        rt_thread_mdelay(200);
    }
}



void app2_aht21_example(void)
{
    // 提示信息
    rt_kprintf("creating aht21 thread\n");

    // 初始化信号量
    aht32_connect_sem = rt_sem_create("aht32_sem", 0, RT_IPC_FLAG_FIFO);
    if (aht32_connect_sem == RT_NULL)
    {
        rt_kprintf("create aht32_connect_sem failed\n");
        return;
    }

    /* 初始化 aht10 */
    dev = aht10_init(i2c_bus_name);
    if (dev == RT_NULL)
    {
        LOG_E(" The sensor initializes failure");
        return;
    }

    // 初始化消息队列
    mq_hum = rt_mq_create("mq_hum", 10, sizeof(humidity), RT_IPC_FLAG_FIFO);
    mq_tem = rt_mq_create("mq_tem", 10, sizeof(temperature), RT_IPC_FLAG_FIFO);

    // 创建线程
    thread1 = rt_thread_create("thread_aht10", thread_aht10_entry, RT_NULL, 1024, 25, 10);
    if (thread1 != RT_NULL)
    {
        rt_thread_startup(thread1);
        rt_kprintf("creating aht21 rt_thread_startup\n");
    }
    else
    {
        LOG_E("thread1 create failed");
    }
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(app2_aht21_example, app_aht21_example);