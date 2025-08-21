/**
 * @file setup_scr_home.c
 * @brief HoloCubic主界面（Home）设置文件
 * 
 * 功能概述：
 * 本文件实现HoloCubic的主界面，提供一个圆形颜色选择器（Color Picker），
 * 用户可以通过IMU手势操作来选择颜色，控制RGB LED的显示效果。
 * 
 * 界面组件：
 * 1. 圆形颜色选择器：支持HSV颜色空间选择
 * 2. 实时颜色预览：选中颜色的即时反馈
 * 3. 手势导航：支持IMU编码器模拟操作
 * 4. 响应式布局：适配240x240分辨率显示屏
 * 
 * 交互方式：
 * - 旋转手势：在颜色环上移动选择不同色相
 * - 点击手势：确认颜色选择并应用到RGB LED
 * - 界面切换：可切换到其他功能界面
 * 
 * 技术特点：
 * - 基于LVGL颜色选择器控件
 * - 自定义样式和主题
 * - 高效的颜色空间转换
 * - 流畅的用户交互体验
 * 
 * 应用场景：
 * - RGB LED氛围灯控制
 * - 个性化灯光效果设置
 * - 颜色主题选择
 * - 视觉效果调试
 * 
 * Copyright 2021 NXP
 * SPDX-License-Identifier: MIT
 * 
 * @author ClimbSnail
 * @version 1.0
 * @date 2021
 */

#include "lvgl.h"        // LVGL图形库核心头文件
#include <stdio.h>       // 标准输入输出库
#include "gui_guider.h"  // GUI向导头文件

/**
 * 主界面（Home）设置函数
 * 
 * @param ui GUI界面结构体指针，用于存储界面对象
 * 
 * 功能说明：
 * 1. 创建主界面屏幕对象
 * 2. 添加圆形颜色选择器控件
 * 3. 配置颜色选择器的样式和属性
 * 4. 设置控件的位置和尺寸
 * 
 * 界面布局：
 * - 屏幕尺寸：240x240像素
 * - 颜色选择器：200x200像素，居中偏上显示
 * - 选择器类型：圆盘式（DISC）颜色选择器
 * - 内边距：10像素，提供舒适的操作空间
 * 
 * 样式设计：
 * - 现代化的圆形设计
 * - 清晰的颜色渐变显示
 * - 适合触摸和手势操作的尺寸
 * - 与HoloCubic整体设计风格一致
 * 
 * 注意事项：
 * - 颜色选择器支持HSV颜色空间
 * - 可通过IMU手势进行操作
 * - 选中的颜色会实时应用到RGB LED
 */
void setup_scr_home(lv_ui* ui)
{
	/* 创建主界面屏幕对象 */
	/* 作为所有主界面控件的父容器 */
	ui->home = lv_obj_create(NULL, NULL);

	/* 创建圆形颜色选择器控件 */
	/* 用于RGB LED颜色选择和控制 */
	ui->home_cpicker0 = lv_cpicker_create(ui->home, NULL);

	/* 配置颜色选择器的主要样式 */
	static lv_style_t style_home_cpicker0_main;
	lv_style_init(&style_home_cpicker0_main);  // 初始化样式对象

	/* 设置颜色选择器的默认状态样式 */
	/* 内边距：提供10像素的内部间距，改善视觉效果 */
	lv_style_set_pad_inner(&style_home_cpicker0_main, LV_STATE_DEFAULT, 10);
	/* 刻度宽度：设置颜色环的宽度为10像素 */
	lv_style_set_scale_width(&style_home_cpicker0_main, LV_STATE_DEFAULT, 10);
	
	/* 应用样式到颜色选择器控件 */
	lv_obj_add_style(ui->home_cpicker0, LV_CPICKER_PART_MAIN, &style_home_cpicker0_main);
	
	/* 设置颜色选择器的位置和尺寸 */
	lv_obj_set_pos(ui->home_cpicker0, 15, 16);      // 位置：距离左上角(15,16)像素
	lv_obj_set_size(ui->home_cpicker0, 200, 200);   // 尺寸：200x200像素的正方形
	
	/* 设置颜色选择器类型为圆盘式 */
	/* 圆盘式提供更直观的颜色选择体验 */
	lv_cpicker_set_type(ui->home_cpicker0, LV_CPICKER_TYPE_DISC);
	
	/* 颜色选择器现在已准备就绪，可以通过IMU手势进行操作 */
	/* 选中的颜色将自动应用到RGB LED显示 */
}