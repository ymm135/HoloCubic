/**
 * @file setup_scr_scenes.c
 * @brief HoloCubic场景界面（Scenes）设置文件
 * 
 * 功能概述：
 * 本文件实现HoloCubic的场景界面，用于播放存储在SD卡中的3D全息动画序列。
 * 通过连续播放预渲染的帧图像，创造出令人惊艳的伪全息显示效果。
 * 
 * 界面组件：
 * 1. 场景画布：用于显示动画帧的图像控件
 * 2. 动画播放：支持连续帧动画播放
 * 3. 文件系统：从SD卡加载预渲染的图像序列
 * 4. 全屏显示：充分利用240x240分辨率
 * 
 * 动画系统：
 * - 帧格式：二进制图像文件（.bin格式）
 * - 帧数量：138帧循环播放（frame000.bin ~ frame137.bin）
 * - 存储路径：S:/Scenes/Holo3D/目录
 * - 播放方式：连续循环播放，营造无缝动画效果
 * 
 * 技术特点：
 * - 高效的图像解码和显示
 * - 流畅的动画播放体验
 * - 内存优化的帧缓冲管理
 * - 支持多种动画场景切换
 * 
 * 视觉效果：
 * - 3D立体视觉效果
 * - 透明全息投影感
 * - 科幻感的动态展示
 * - 吸引眼球的视觉冲击
 * 
 * 应用场景：
 * - 桌面装饰和展示
 * - 科技产品演示
 * - 艺术创作和展览
 * - 教育和娱乐应用
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
 * 场景界面（Scenes）设置函数
 * 
 * @param ui GUI界面结构体指针，用于存储界面对象
 * 
 * 功能说明：
 * 1. 创建场景界面屏幕对象
 * 2. 添加图像显示控件作为动画画布
 * 3. 配置画布的样式和显示属性
 * 4. 设置默认显示的动画帧
 * 
 * 界面布局：
 * - 全屏显示：充分利用240x240像素屏幕
 * - 居中对齐：动画内容居中显示
 * - 黑色背景：突出动画内容，增强视觉效果
 * - 无边框设计：提供沉浸式观看体验
 * 
 * 动画配置：
 * - 初始帧：frame000.bin（动画序列的第一帧）
 * - 文件路径：S:/Scenes/Holo3D/（SD卡中的动画目录）
 * - 图像格式：二进制格式，针对ESP32优化
 * - 显示模式：全屏居中显示
 * 
 * 样式设计：
 * - 默认状态：黑色背景，突出动画内容
 * - 按压状态：灰色背景，提供交互反馈
 * - 聚焦状态：黑色背景，保持一致性
 * - 简洁设计：不干扰动画播放效果
 * 
 * 注意事项：
 * - 需要SD卡中存在对应的动画文件
 * - 动画播放逻辑在主循环中实现
 * - 支持通过IMU手势切换到其他界面
 */
void setup_scr_scenes(lv_ui* ui)
{
	/* 创建场景界面屏幕对象 */
	/* 作为动画播放的主容器 */
	ui->scenes = lv_obj_create(NULL, NULL);

	/* 创建图像控件作为动画画布 */
	/* 用于显示从SD卡加载的动画帧 */
	ui->scenes_canvas = lv_img_create(ui->scenes, NULL);

	/* 配置动画画布的主要样式 */
	static lv_style_t style_scenes_canvas_main;
	lv_style_init(&style_scenes_canvas_main);  // 初始化样式对象
	
	/* 设置不同状态下的背景颜色 */
	/* 默认状态：黑色背景，突出动画内容 */
	lv_style_set_bg_color(&style_scenes_canvas_main, LV_STATE_DEFAULT, LV_COLOR_BLACK);
	/* 按压状态：灰色背景，提供用户交互反馈 */
	lv_style_set_bg_color(&style_scenes_canvas_main, LV_STATE_PRESSED, LV_COLOR_GRAY);
	/* 聚焦状态：黑色背景，保持视觉一致性 */
	lv_style_set_bg_color(&style_scenes_canvas_main, LV_STATE_FOCUSED, LV_COLOR_BLACK);

	/* 应用样式到场景界面 */
	/* 注意：这里应用到scenes对象而不是canvas对象 */
	lv_obj_add_style(ui->scenes, LV_BTN_PART_MAIN, &style_scenes_canvas_main);
	
	/* 设置动画画布的初始图像源 */
	/* 加载动画序列的第一帧作为默认显示 */
	lv_img_set_src(ui->scenes_canvas, "S:/Scenes/Holo3D/frame000.bin");
	
	/* 设置画布在屏幕中央对齐 */
	/* 确保动画内容完美居中显示 */
	lv_obj_align(ui->scenes_canvas, NULL, LV_ALIGN_CENTER, 0, 0);
	
	/* 场景界面现在已准备就绪 */
	/* 动画播放逻辑将在主循环中通过更新图像源实现 */
	/* 用户可以通过IMU手势在不同界面间切换 */
}