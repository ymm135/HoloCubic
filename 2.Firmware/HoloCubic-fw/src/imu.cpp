/*
 * HoloCubic IMU传感器模块
 * 
 * 功能说明：
 * 1. 管理MPU6050六轴惯性测量单元（加速度计+陀螺仪）
 * 2. 通过I2C接口读取传感器数据
 * 3. 将IMU数据转换为LVGL编码器输入事件
 * 4. 实现基于重力感应的手势识别
 * 5. 提供旋转检测和按压检测功能
 * 
 * 硬件接口：
 * - I2C通信：SDA=GPIO32, SCL=GPIO33
 * - 传感器：MPU6050（InvenSense）
 * - 工作电压：3.3V
 * - 采样频率：可配置，默认1kHz
 * 
 * 手势识别逻辑：
 * - Y轴加速度变化 → 旋转方向检测
 * - X轴加速度阈值 → 按压状态检测
 * - 数据滤波和防抖处理
 */

#include "imu.h"
#include <MPU6050_tockn.h>  // MPU6050传感器库
#include <Wire.h>           // I2C通信库

// MPU6050传感器对象实例，使用Wire I2C总线
MPU6050 imu(Wire);

/**** LVGL编码器输入设备全局变量 ****/
// 编码器旋转差值，用于LVGL滚动事件
// 正值表示顺时针旋转，负值表示逆时针旋转
int16_t encoder_diff = 0;

// 编码器按键状态，用于LVGL按键事件
// LV_INDEV_STATE_PR: 按下状态
// LV_INDEV_STATE_REL: 释放状态
lv_indev_state_t encoder_state = LV_INDEV_STATE_REL;

/**
 * IMU传感器初始化函数
 * 
 * 功能：
 * 1. 初始化I2C总线，配置SDA和SCL引脚
 * 2. 设置I2C时钟频率为400kHz（快速模式）
 * 3. 检测MPU6050连接状态，确保通信正常
 * 4. 初始化MPU6050寄存器配置
 */
void IMU::init()
{
	// 初始化I2C总线，指定SDA和SCL引脚
	Wire.begin(IMU_I2C_SDA, IMU_I2C_SCL);
	
	// 设置I2C时钟频率为400kHz，提高数据传输速度
	Wire.setClock(400000);
	
	// 等待MPU6050连接成功，确保硬件通信正常
	while (!imu.testConnection());
	
	// 初始化MPU6050传感器，配置默认参数
	imu.initialize();
}

/**
 * IMU数据更新和手势识别函数
 * 
 * 参数：
 * @param interval 更新间隔时间（毫秒）
 * 
 * 功能：
 * 1. 读取MPU6050的六轴数据（3轴加速度 + 3轴角速度）
 * 2. 基于Y轴加速度变化检测旋转手势
 * 3. 基于X轴加速度阈值检测按压手势
 * 4. 转换为LVGL编码器事件格式
 */
void IMU::update(int interval)
{
	// 读取MPU6050六轴数据：加速度计(ax,ay,az) + 陀螺仪(gx,gy,gz)
	imu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

	// 调试输出陀螺仪数据（已注释）
	//Serial.print(gx);
	//Serial.print(" ");
	//Serial.print(gy);
	//Serial.print(" ");
	//Serial.print(gz);
	//Serial.println(" ");

	// 按设定间隔更新手势识别，避免过于频繁的状态变化
	if (millis() - last_update_time > interval)
	{
		// Y轴加速度手势识别：检测上下倾斜动作
		if (ay > 3000 && flag)  // Y轴正向倾斜超过阈值
		{
			encoder_diff--;  // 编码器计数减1（向上滚动）
			flag = 0;        // 设置防抖标志
		}
		else if (ay < -3000 && flag)  // Y轴负向倾斜超过阈值
		{
			encoder_diff++;  // 编码器计数加1（向下滚动）
			flag = 0;        // 设置防抖标志
		}
		else
		{
			flag = 1;  // 重置防抖标志，允许下次检测
		}

		// X轴加速度按压检测：检测前后倾斜或震动
		if (ax > 10000)  // X轴加速度超过按压阈值
		{
			encoder_state = LV_INDEV_STATE_PR;   // 设置为按下状态
		}
		else
		{
			encoder_state = LV_INDEV_STATE_REL;  // 设置为释放状态
		}

		// 更新时间戳
		last_update_time = millis();
	}
}

/**
 * 获取X轴加速度值
 * @return X轴加速度原始数据（16位有符号整数）
 */
int16_t IMU::getAccelX()
{
	return ax;
}

/**
 * 获取Y轴加速度值
 * @return Y轴加速度原始数据（16位有符号整数）
 */
int16_t IMU::getAccelY()
{
	return ay;
}

/**
 * 获取Z轴加速度值
 * @return Z轴加速度原始数据（16位有符号整数）
 */
int16_t IMU::getAccelZ()
{
	return az;
}

/**
 * 获取X轴角速度值
 * @return X轴陀螺仪原始数据（16位有符号整数）
 */
int16_t IMU::getGyroX()
{
	return gx;
}

/**
 * 获取Y轴角速度值
 * @return Y轴陀螺仪原始数据（16位有符号整数）
 */
int16_t IMU::getGyroY()
{
	return gy;
}

/**
 * 获取Z轴角速度值
 * @return Z轴陀螺仪原始数据（16位有符号整数）
 */
int16_t IMU::getGyroZ()
{
	return gz;
}
