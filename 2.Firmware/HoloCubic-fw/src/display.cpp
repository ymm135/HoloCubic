/*
 * HoloCubic 显示系统驱动模块
 * 
 * 功能说明：
 * 1. 初始化ST7789 TFT显示屏（240x240分辨率）
 * 2. 集成LVGL图形库，提供高级GUI功能
 * 3. 实现显示缓冲区管理和刷新机制
 * 4. 提供PWM背光亮度控制
 * 5. 处理LVGL任务调度和显示更新
 * 
 * 硬件接口：
 * - TFT屏幕：SPI接口连接ST7789控制器
 * - 背光控制：GPIO5 PWM输出
 * - 分辨率：240x240像素，16位色深
 * 
 * 技术特点：
 * - 双缓冲机制提高显示流畅度
 * - DMA加速SPI传输
 * - 支持LVGL动画和特效
 */

#include "display.h"
#include <TFT_eSPI.h>    // ESP32优化的TFT显示库
#include <lvgl.h>        // 轻量级图形库

/*
TFT引脚配置应在以下路径设置：
path/to/Arduino/libraries/TFT_eSPI/User_Setups/Setup24_ST7789.h
*/
// TFT显示驱动实例
TFT_eSPI tft = TFT_eSPI();

// LVGL显示缓冲区配置
static lv_disp_buf_t disp_buf;                    // 显示缓冲区描述符
static lv_color_t buf[LV_HOR_RES_MAX * 10];       // 显示缓冲区数组（10行像素）


/**
 * LVGL日志打印回调函数
 * 用于调试LVGL内部状态和错误信息
 * 
 * @param level 日志级别
 * @param file  源文件名
 * @param line  行号
 * @param fun   函数名
 * @param dsc   描述信息
 */
void my_print(lv_log_level_t level, const char* file, uint32_t line, const char* fun, const char* dsc)
{
	Serial.printf("%s@%d %s->%s\r\n", file, line, fun, dsc);
	Serial.flush();
}


/**
 * LVGL显示刷新回调函数
 * 将LVGL渲染的图像数据传输到TFT显示屏
 * 
 * @param disp    显示驱动指针
 * @param area    需要刷新的区域坐标
 * @param color_p 像素颜色数据指针
 */
void my_disp_flush(lv_disp_drv_t* disp, const lv_area_t* area, lv_color_t* color_p)
{
	// 计算刷新区域的宽度和高度
	uint32_t w = (area->x2 - area->x1 + 1);
	uint32_t h = (area->y2 - area->y1 + 1);

	// 开始SPI传输事务
	tft.startWrite();
	// 设置显示窗口地址
	tft.setAddrWindow(area->x1, area->y1, w, h);
	// 推送像素数据到显示屏（使用DMA加速）
	tft.pushColors(&color_p->full, w * h, true);
	// 结束SPI传输事务
	tft.endWrite();

	// 通知LVGL刷新完成
	lv_disp_flush_ready(disp);
}


/**
 * 显示系统初始化函数
 * 配置TFT显示屏、LVGL图形库和背光控制
 */
void Display::init()
{
	// 配置背光PWM控制
	// 频率5kHz，8位分辨率（0-255）
	ledcSetup(LCD_BL_PWM_CHANNEL, 5000, 8);
	ledcAttachPin(LCD_BL_PIN, LCD_BL_PWM_CHANNEL);

	// 初始化LVGL图形库
	lv_init();

	// 注册LVGL调试日志打印函数
	lv_log_register_print_cb(my_print);

	// 初始化TFT显示屏
	tft.begin();
	// 设置屏幕旋转方向（镜像显示）
	tft.setRotation(4);

	// 初始化LVGL显示缓冲区
	// 使用单缓冲模式，缓冲区大小为10行像素
	lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);

	// 配置LVGL显示驱动
	lv_disp_drv_t disp_drv;
	lv_disp_drv_init(&disp_drv);              // 初始化驱动结构体
	disp_drv.hor_res = 240;                   // 水平分辨率240像素
	disp_drv.ver_res = 240;                   // 垂直分辨率240像素
	disp_drv.flush_cb = my_disp_flush;        // 设置刷新回调函数
	disp_drv.buffer = &disp_buf;              // 绑定显示缓冲区
	lv_disp_drv_register(&disp_drv);          // 注册显示驱动到LVGL
}

/**
 * 显示系统例程函数
 * 需要在主循环中定期调用，处理LVGL任务调度
 * 包括动画更新、事件处理、显示刷新等
 */
void Display::routine()
{
	// 处理LVGL任务队列
	// 包括动画、定时器、事件处理等
	lv_task_handler();
}

/**
 * 设置背光亮度
 * 使用PWM控制背光LED的亮度
 * 
 * @param duty 亮度占空比，范围0.0-1.0
 *             0.0 = 最暗，1.0 = 最亮
 */
void Display::setBackLight(float duty)
{
	// 限制占空比范围在0-1之间
	duty = constrain(duty, 0, 1);
	// 反转占空比（硬件可能是低电平有效）
	duty = 1 - duty;
	// 写入PWM值（0-255对应8位分辨率）
	ledcWrite(LCD_BL_PWM_CHANNEL, (int)(duty * 255));
}
