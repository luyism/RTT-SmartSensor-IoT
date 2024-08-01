//记得在menuconfig中开启支持旧版本功能（Support legacy version）
#include <board.h>
#include <rtthread.h>
#include <drv_gpio.h>
#include <dfs_posix.h>//需要添加软件包进这里

//定义要写入的内容
char String[] = "Hello, RT-Thread.Welcom to RSOC!";

//定义接受文件内容的缓冲区
char buffer[100] = {};

void FileSystem_Test(void *parameter)
{
    //文件描述符
    int fd;

    //用只写方式打开文件,如果没有该文件,则创建一个文件
    fd = open("/fal/FileTest.txt", O_WRONLY | O_CREAT);

    //如果打开成功
    if (fd >= 0)
    {
        //写入文件
        write(fd, String, sizeof(String));

        rt_kprintf("Write done.\n");

        //关闭文件
        close(fd);
    }
    else
    {
        rt_kprintf("File Open Fail.\n");
    }

    //用只读方式打开文件
    fd = open("/fal/FileTest.txt", O_RDONLY);

    if (fd>= 0)
    {
        //读取文件内容
        rt_uint32_t size = read(fd, buffer, sizeof(buffer));
    
        if (size < 0)
        {
            rt_kprintf("Read File Fail.\n");
            return ;
        }

        //输出文件内容
        rt_kprintf("Read from file test.txt : %s \n", buffer);

        //关闭文件
        close(fd);
    }
    else
    {
        rt_kprintf("File Open Fail.\n");
    }
}
//导出命令
MSH_CMD_EXPORT(FileSystem_Test, FileSystem_Test);

static void readdir_sample(void)
{
    DIR *dirp;
    struct dirent *d;

    /* 打开 / dir_test 目录 */
    dirp = opendir("/fal");
    if (dirp == RT_NULL)
    {
        rt_kprintf("open directory error!\n");
    }
    else
    {
        /* 读取目录 */
        while ((d = readdir(dirp)) != RT_NULL)
        {
            rt_kprintf("found %s\n", d->d_name);
        }

        /* 关闭目录 */
        closedir(dirp);
    }
}
/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(readdir_sample, readdir sample);


// 在温湿度上传上传云端的同时，将数据同步放在文件系统处，文件名为：Data.txt；
// 文件内容： Temp：XX ; Humi：XX ; Count： 1（自上电起所采集的数据次数）

extern rt_mq_t mq_hum;
extern rt_mq_t mq_tem;

// 定义文件名
char file_name[] = "/fal/Data.txt";
// 定义文件内容
static char file_content[100] = {};

// 定义一个函数用来从队列中获取数据并写入文件
void write_data_to_file(void *parameter)
{
    // 定义文件描述符
    int fd;
    // 定义文件内容的长度
    int len = 0;
    // 定义温度和湿度
    float temperature, humidity;
    // 定义次数
    static int count = 1;
    
    memset(file_content, 0, sizeof(file_content));

    // 从队列中获取温度和湿度
    rt_mq_recv(mq_hum, &humidity, sizeof(humidity), RT_WAITING_NO);
    rt_mq_recv(mq_tem, &temperature, sizeof(temperature), RT_WAITING_NO);
    len = rt_snprintf(file_content, sizeof(file_content), "Temp: %.2f ; Humi: %.2f ; Count: %d\n", temperature, humidity, count);


    // 创建文件
    fd = open(file_name, O_WRONLY | O_CREAT);
    if (fd >= 0)
    {
        // rt_kprintf("Openfile done.\n");
        // 将数据写入文件末尾
        lseek(fd, 0, SEEK_END);
        write(fd, file_content, len);
        // 关闭文件
        close(fd);
        count++;
        // rt_kprintf("Write done.\n");
    }
    else
    {
        rt_kprintf("File Open Fail.\n");
    }
}

// 定义一个线程用来写入文件
static rt_thread_t thread2 = RT_NULL;
static char thread2_stack[2048];
void write_data_to_file_entry(void *parameter)
{
    while (1)
    {
        write_data_to_file(RT_NULL);
        rt_thread_mdelay(5000);
    }
}

// 定义一个函数用来创建线程
void app4_write_data(void)
{
    thread2 = rt_thread_create("thread2", write_data_to_file_entry, RT_NULL, sizeof(thread2_stack), 25, 10);
    if (thread2 != RT_NULL)
    {
        rt_thread_startup(thread2);
    }
}

// 导出命令
MSH_CMD_EXPORT(app4_write_data, write_data_thread);