// #include <wlan_mgnt.h>
#include <rtthread.h>

#define WIFI_SSID "43324"
#define WIFI_PASSWORD "433433433"


rt_err_t rt_wlan_connect(const char *ssid, const char *password);


static void app1_wifi_connect(void)
{
    rt_err_t result;
    result = rt_wlan_connect(WIFI_SSID, WIFI_PASSWORD);
    if (result != RT_EOK)
    {
        rt_kprintf("wifi connect failed\n");
    }
    else
    {
        rt_kprintf("wifi connect success\n");
    }
}

MSH_CMD_EXPORT(app1_wifi_connect, wifi connect);
