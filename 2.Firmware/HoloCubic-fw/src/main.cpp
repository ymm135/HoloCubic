/*
 * HoloCubic 主程序文件
 * 
 * 功能说明：
 * 1. 初始化所有硬件模块（显示屏、IMU、RGB LED、SD卡等）
 * 2. 配置LVGL图形库和输入设备
 * 3. 建立WiFi连接并支持网络应用
 * 4. 运行主循环，处理显示刷新和传感器数据
 * 
 * 硬件平台：ESP32-PICO-D4
 * 开发框架：Arduino + PlatformIO
 * GUI库：LVGL
 * 
 * 作者：稚晖君
 * 项目：HoloCubic - 多功能透明显示屏桌面站
 */

#include <Arduino.h>
#include "display.h"        // 显示屏驱动模块
#include "imu.h"            // IMU传感器模块（MPU6050）
#include "ambient.h"        // 环境光传感器模块
#include "network.h"        // WiFi网络功能模块
#include "sd_card.h"        // SD卡文件系统模块
#include "rgb_led.h"        // RGB LED控制模块
#include "lv_port_indev.h"  // LVGL输入设备端口
#include "lv_port_fatfs.h"  // LVGL文件系统端口
#include "lv_cubic_gui.h"   // HoloCubic自定义GUI
#include "gui_guider.h"     // GUI向导和界面管理

/**** 硬件组件对象实例化 ****/
Display screen;     // 显示屏对象 - 管理ST7789 TFT显示屏
IMU mpu;           // IMU传感器对象 - 管理MPU6050六轴传感器
Pixel rgb;         // RGB LED对象 - 管理板载WS2812 LED
SdCard tf;         // SD卡对象 - 管理文件系统和数据存储
Network wifi;      // WiFi网络对象 - 管理无线连接和网络应用

// LVGL GUI管理对象
lv_ui guider_ui;   // GUI向导界面结构体

/**
 * 系统初始化函数
 * 
 * 执行顺序：
 * 1. 串口通信初始化
 * 2. 显示系统初始化（TFT屏幕 + LVGL）
 * 3. 输入设备初始化（IMU传感器作为编码器）
 * 4. 状态指示初始化（RGB LED）
 * 5. 存储系统初始化（SD卡 + FAT文件系统）
 * 6. 用户界面初始化
 * 7. 网络功能初始化（可选）
 */
void setup()
{
    // 初始化串口通信，波特率115200
    Serial.begin(115200);
    Serial.println("HoloCubic System Starting...");

    /**** 显示系统初始化 ****/
    screen.init();              // 初始化ST7789 TFT显示屏和LVGL
    screen.setBackLight(0.2);   // 设置背光亮度为20%（PWM控制）

    /**** 输入设备初始化 ****/
    lv_port_indev_init();       // 初始化LVGL输入设备端口
    mpu.init();                 // 初始化MPU6050 IMU传感器（I2C接口）

    /**** RGB状态指示灯初始化 ****/
    rgb.init();                 // 初始化WS2812 RGB LED
    // 设置两颗LED为蓝色，亮度10%，表示系统启动状态
    rgb.setBrightness(0.1).setRGB(0, 0, 122, 204).setRGB(1, 0, 122, 204);

    /**** 存储系统初始化 ****/
    tf.init();                  // 初始化SD卡（HSPI接口）
    lv_fs_if_init();           // 初始化LVGL文件系统接口

    // 从SD卡读取WiFi配置信息
    String ssid = tf.readFileLine("/wifi.txt", 1);        // 第1行：WiFi SSID
    String password = tf.readFileLine("/wifi.txt", 2);    // 第2行：WiFi密码

    /**** 用户界面初始化 ****/
    lv_holo_cubic_gui();        // 加载HoloCubic自定义GUI界面
    // setup_ui(&guider_ui);    // 可选：使用GUI向导生成的界面

    /**** 网络功能初始化（当前已禁用）****/
#if 0
    wifi.init(ssid, password);  // 连接WiFi网络

    // 示例：获取B站粉丝数（需要修改为你的UID）
    Serial.println(wifi.getBilibiliFans("20259914"));
#endif

    Serial.println("System initialization completed!");
}

// 动画帧计数器（用于播放SD卡中的动画序列）
int frame_id = 0;
char buf[100];  // 文件路径缓冲区

/**
 * 主循环函数
 * 
 * 功能说明：
 * 1. 处理LVGL图形库任务（显示刷新、动画更新等）
 * 2. 更新IMU传感器数据并转换为输入事件
 * 3. 处理动画播放（从SD卡加载帧图像）
 * 4. 处理网络任务和其他后台任务
 * 
 * 执行频率：尽可能高频率运行，确保界面流畅
 */
void loop()
{
    /**** 显示系统任务处理 ****/
    // LVGL任务处理器 - 处理界面刷新、动画、事件等
    // 这个函数需要尽可能频繁调用以保证界面流畅
    screen.routine();

    /**** 输入设备数据更新 ****/
    // 每200ms更新一次IMU数据，避免过于频繁的传感器读取
    // IMU数据会被转换为LVGL编码器事件用于界面导航
    mpu.update(200);

    // 调试输出（可以移除）
    Serial.println("System running...");

    /**** 动画播放功能（当前已注释）****/
    // 以下代码演示如何从SD卡播放动画序列
    // 动画文件命名格式：S:/Scenes/Holo3D/frame000.bin ~ frame137.bin
//    int len = sprintf(buf, "S:/Scenes/Holo3D/frame%03d.bin", frame_id++);
//    buf[len] = 0;
//    lv_img_set_src(guider_ui.scenes_canvas, buf);  // 设置图像源
//    Serial.println(buf);
//
//    if (frame_id == 138) frame_id = 0;  // 循环播放动画

    /**** 可选延时 ****/
    // 如果需要降低CPU使用率，可以添加小延时
    // delay(10);
}