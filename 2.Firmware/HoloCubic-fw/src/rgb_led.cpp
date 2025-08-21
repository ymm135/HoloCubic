/*
 * HoloCubic RGB LED控制模块
 * 
 * 功能说明：
 * 1. 控制板载WS2812可编程RGB LED灯珠
 * 2. 提供颜色设置、亮度调节和动画效果
 * 3. 支持多LED独立控制和同步显示
 * 4. 实现系统状态指示和视觉反馈
 * 5. 集成FastLED库，支持丰富的灯光效果
 * 
 * 硬件特性：
 * - LED类型：WS2812B（集成控制芯片）
 * - 数据接口：单线串行通信（GPIO27）
 * - 颜色深度：24位真彩色（RGB各8位）
 * - LED数量：2颗（可扩展）
 * - 工作电压：3.3V/5V兼容
 * 
 * 控制协议：
 * - 数据格式：GRB顺序（绿-红-蓝）
 * - 时序要求：严格的高低电平时序
 * - 级联控制：支持多LED串联
 * - 刷新频率：可达400Hz
 */

#include "rgb_led.h"
#include <FastLED.h>     // 高性能LED控制库

// RGB LED颜色缓冲区数组
// 每个元素对应一颗LED的RGB颜色值
// CRGB结构体包含红、绿、蓝三个8位颜色分量
CRGB color_buffers[RGB_LED_NUM];


/**
 * @brief 初始化RGB LED控制器
 * 
 * 配置FastLED库参数：
 * - LED类型：WS2812（可编程RGB LED）
 * - 数据引脚：RGB_LED_PIN（通常为GPIO27）
 * - 颜色顺序：GRB（WS2812的标准格式）
 * - LED数量：RGB_LED_NUM（定义的LED总数）
 * - 默认亮度：200/255（约78%亮度）
 */
void Pixel::init()
{
	// 添加WS2812 LED灯带，指定引脚和颜色缓冲区
	FastLED.addLeds<WS2812, RGB_LED_PIN, GRB>(color_buffers, RGB_LED_NUM);
	// 设置全局亮度，避免过亮影响视觉体验
	FastLED.setBrightness(200);
}

/**
 * @brief 设置指定LED的RGB颜色值
 * 
 * @param id LED索引（0开始）
 * @param r 红色分量（0-255）
 * @param g 绿色分量（0-255）
 * @param b 蓝色分量（0-255）
 * @return Pixel& 返回自身引用，支持链式调用
 * 
 * 注意：调用后立即刷新显示，适合单次颜色设置
 */
Pixel& Pixel::setRGB(int id, int r, int g, int b)
{
	// 将RGB值写入指定LED的颜色缓冲区
	color_buffers[id] = CRGB(r, g, b);
	// 立即将缓冲区数据发送到LED硬件显示
	FastLED.show();

	return *this;
}

/**
 * @brief 设置LED全局亮度
 * 
 * @param duty 亮度占空比（0.0-1.0浮点数）
 * @return Pixel& 返回自身引用，支持链式调用
 * 
 * 功能说明：
 * - 使用PWM调光原理控制整体亮度
 * - 不改变颜色比例，只调节输出强度
 * - 有效降低功耗和发热
 */
Pixel& Pixel::setBrightness(float duty)
{
	// 限制亮度值在有效范围内（0-1）
	duty = constrain(duty, 0, 1);
	// 将浮点亮度转换为8位整数（0-255）
	FastLED.setBrightness((uint8_t)(255 * duty));
	// 应用新的亮度设置到所有LED
	FastLED.show();

	return *this;
}
