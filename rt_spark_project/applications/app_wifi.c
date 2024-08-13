// #include <wlan_mgnt.h>

#include "app_head.h"

#define WIFI_SSID "43324"   //wiif名字，需要根据实际情况修改（2.4G）
#define WIFI_PASSWORD "433433433"   //wifi密码，需要根据实际情况修改


#define THREAD_PRIORITY         16
#define THREAD_STACK_SIZE       1024

#define THREAD_TIMESLICE        5


// 新建信号量，用于wifi连接成功后的回调启动mqtt连接
rt_sem_t wifi_connect_sem = RT_NULL;

// 函数声明
rt_err_t rt_wlan_connect(const char *ssid, const char *password);

/* 定义一个线程控制块 */
static rt_thread_t thread_wifi = RT_NULL;
/* 定义一个线程入口 */
static void thread_wifi_entry(void *parameter)
{
    rt_kprintf("entry wifi thread\n");
    // 尝试连接wifi，连接成功后释放信号量删除线程，若连接不成功则间隔5s一直尝试
    while (1)
    {
        rt_kprintf("wifi thread connecting \n");

        rt_err_t result;
        // 建立临界区


        result = rt_wlan_connect(WIFI_SSID, WIFI_PASSWORD);

        if (result != RT_EOK)
        {
            rt_kprintf("wifi connect failed\n");
            rt_thread_mdelay(1000);
        }
        else
        {
            rt_kprintf("wifi connected, checking IP...\n");
            // 检查是否获取到IP地址
            while (rt_wlan_is_ready() != RT_EOK)
            {
                rt_kprintf("waiting for IP address...\n");
                rt_thread_mdelay(1000);
            }

            rt_kprintf("IP acquired, wifi connect success\n");
            rt_sem_release(wifi_connect_sem);
            rt_thread_delete(thread_wifi);
        } 
    }
}


void app1_wifi_connect(void)
{
    // 提示信息：wifi线程开始创建
    rt_kprintf("creating wifi thread\n");
    // 初始化信号量
    wifi_connect_sem = rt_sem_create("wifi_sem", 0, RT_IPC_FLAG_FIFO);
    if (wifi_connect_sem == RT_NULL)
    {
        rt_kprintf("create wifi_connect_sem failed\n");
        return;
    }
    // 初始化wifi线程
    thread_wifi = rt_thread_create("wifi_thread", thread_wifi_entry, RT_NULL, THREAD_STACK_SIZE, THREAD_PRIORITY, THREAD_TIMESLICE);
        /* 如果获得线程控制块，启动这个线程 */
    if (thread_wifi != RT_NULL)
    {
        rt_thread_startup(thread_wifi);
        rt_kprintf("create wifi thread sucess\n");
    }
    else
    {
        rt_kprintf("create thread_wifi failed\n");
    }    
}

MSH_CMD_EXPORT(app1_wifi_connect, wifi connect);
