/**
 * @file lv_cubic_gui.c
 * @brief HoloCubic自定义GUI界面实现文件
 * 
 * 功能概述：
 * 本文件实现HoloCubic的自定义图形用户界面，提供简洁而现代的
 * 启动界面和Logo显示功能。作为系统的视觉入口，展现产品的
 * 品牌形象和设计理念。
 * 
 * 界面特色：
 * 1. 极简设计：黑色背景突出主要内容
 * 2. 品牌展示：居中显示HoloCubic Logo
 * 3. 响应式交互：支持多种状态的视觉反馈
 * 4. 高对比度：确保在各种环境下的可视性
 * 
 * 技术实现：
 * - 基于LVGL图形库的现代GUI设计
 * - 自定义样式系统，统一视觉风格
 * - 内嵌图像资源，减少外部依赖
 * - 灵活的布局系统，适配不同屏幕
 * 
 * 视觉设计：
 * - 主色调：深黑色背景，营造科技感
 * - 交互反馈：灰色按压状态，红色聚焦状态
 * - Logo展示：居中对齐，突出品牌形象
 * - 简洁布局：避免视觉干扰，专注核心内容
 * 
 * 应用场景：
 * - 系统启动界面
 * - 品牌Logo展示
 * - 产品演示和展览
 * - 用户界面入口
 * 
 * @author ClimbSnail
 * @version 1.0
 * @date 2021
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_cubic_gui.h"  // HoloCubic GUI头文件
#include "images.h"        // 图像资源头文件

/**
 * 全局屏幕对象指针
 * 用于存储当前活动的屏幕对象，便于后续操作和管理
 */
lv_obj_t* scr;

/**
 * HoloCubic自定义GUI初始化函数
 * 
 * 功能说明：
 * 1. 创建并配置默认的界面样式
 * 2. 设置多种交互状态的视觉反馈
 * 3. 加载并显示HoloCubic品牌Logo
 * 4. 建立统一的视觉设计语言
 * 
 * 样式配置：
 * - 默认状态：纯黑背景，营造专业科技感
 * - 按压状态：灰色背景，提供触觉反馈
 * - 聚焦状态：黑色背景，保持视觉一致性
 * - 组合状态：红色背景，突出重要交互
 * 
 * 布局设计：
 * - Logo居中：确保品牌形象的最佳展示效果
 * - 全屏适配：充分利用240x240像素显示空间
 * - 响应式设计：适应不同的交互方式
 * 
 * 资源管理：
 * - 内嵌Logo：使用编译时嵌入的图像资源
 * - 可选外部：支持从SD卡加载外部图像
 * - 内存优化：高效的图像显示和缓存机制
 * 
 * 注意事项：
 * - 必须在LVGL和显示系统初始化后调用
 * - Logo图像资源需要在images.h中定义
 * - 样式设置会影响整个屏幕的视觉效果
 */
void lv_holo_cubic_gui(void)
{
	/* 创建并初始化默认样式 */
	static lv_style_t default_style;
	lv_style_init(&default_style);  // 初始化样式对象
	
	/* 配置不同状态下的背景颜色 */
	/* 默认状态：纯黑背景，营造专业的科技感氛围 */
	lv_style_set_bg_color(&default_style, LV_STATE_DEFAULT, LV_COLOR_BLACK);
	/* 按压状态：灰色背景，为用户提供清晰的触觉反馈 */
	lv_style_set_bg_color(&default_style, LV_STATE_PRESSED, LV_COLOR_GRAY);
	/* 聚焦状态：黑色背景，保持与默认状态的视觉一致性 */
	lv_style_set_bg_color(&default_style, LV_STATE_FOCUSED, LV_COLOR_BLACK);
	/* 聚焦+按压组合状态：红色背景，突出重要的交互操作 */
	lv_style_set_bg_color(&default_style, LV_STATE_FOCUSED | LV_STATE_PRESSED, lv_color_hex(0xf88));

	/* 将默认样式应用到当前活动屏幕 */
	/* 这将影响整个屏幕的视觉表现和交互反馈 */
	lv_obj_add_style(lv_scr_act(), LV_BTN_PART_MAIN, &default_style);

	/* 获取当前活动屏幕的引用 */
	/* 便于后续的界面操作和管理 */
	scr = lv_scr_act();
	
	/* 创建图像控件用于显示Logo */
	lv_obj_t* img = lv_img_create(lv_scr_act(), NULL);
	
	/* 设置Logo图像源 */
	/* 使用内嵌的Logo图像资源，确保快速加载和显示 */
	lv_img_set_src(img, &logo);
	/* 可选：从SD卡加载外部图像文件 */
	// lv_img_set_src(img, "S:/pic.bin");
	
	/* 设置Logo在屏幕中央对齐 */
	/* 确保品牌形象得到最佳的展示效果 */
	lv_obj_align(img, NULL, LV_ALIGN_CENTER, 0, 0);
	
	/* HoloCubic自定义GUI现在已准备就绪 */
	/* Logo将在屏幕中央显示，展现产品的品牌形象 */
}