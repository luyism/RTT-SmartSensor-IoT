#include <rtthread.h>
#include <rtdevice.h>
#include <board.h>

#include <ap3216c.h>

#define LOG_TAG             "ap3216c"
#define LOG_LVL              LOG_LVL_DBG
#include <ulog.h>


static float brightness, distance;
static ap3216c_device_t dev;

//新建一个线程专门用于读取传感器数据并将数据分别放入两个消息队列中
static rt_thread_t thread1 = RT_NULL;
rt_mq_t mq_brightness = RT_NULL;
rt_mq_t mq_distance = RT_NULL;

// 新建信号量，用于线程创建成功后的回调启动mqtt连接
rt_sem_t ap3216_connect_sem = RT_NULL;

/* 定义一个线程入口 */
static void thread_ap3216_entry(void *parameter)
{
    // 释放信号量
    rt_sem_release(ap3216_connect_sem);
    

    while (1)
    {
        static int i =0;
        i++;
        rt_uint16_t ps_data;
        float brightness;

        ps_data = ap3216c_read_ps_data(dev);
        // LOG_D("current ps data: %d.", ps_data);
        rt_mq_urgent(mq_distance, &ps_data, sizeof(ps_data));

        brightness = ap3216c_read_ambient_light(dev);
        // LOG_D("current brightness: %d.%d(lux).", (int)brightness, ((int)(10 * brightness) % 10));
        rt_mq_urgent(mq_brightness, &brightness, sizeof(brightness));

        // if(i%10 == 0)
        // {
        //     LOG_D("current brightness: %d.%d(lux).", (int)brightness, ((int)(10 * brightness) % 10));
        //     LOG_D("current ps data: %d", ps_data);
        // }

        rt_thread_mdelay(200);
    }

}

void app2_ap3216_example(void)
{
    // 初始化信号量
    ap3216_connect_sem = rt_sem_create("ap3216_sem", 0, RT_IPC_FLAG_FIFO);
    if (ap3216_connect_sem == RT_NULL)
    {
        rt_kprintf("create ap3216_connect_sem failed\n");
        return;
    }

    // 提示信息
    rt_kprintf("creating ap3216 thread\n");

    const char* i2c_bus_name = "i2c2";
    dev = ap3216c_init(i2c_bus_name);

    // 初始化消息队列
    mq_brightness = rt_mq_create("mq_bright", 10, sizeof(brightness), RT_IPC_FLAG_FIFO);
    mq_distance = rt_mq_create("mq_distance", 10, sizeof(distance), RT_IPC_FLAG_FIFO);

    // 创建线程
    thread1 = rt_thread_create("thread_ap3216", thread_ap3216_entry, RT_NULL, 1024, 25, 10);
    if (thread1 != RT_NULL)
    {
        rt_thread_startup(thread1);
        rt_kprintf("creating ap3216 rt_thread_startup\n");
    }
    else
    {
        LOG_E("app2_ap3216_example thread create failed");
    }
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(app2_ap3216_example, app2_ap3216_example);