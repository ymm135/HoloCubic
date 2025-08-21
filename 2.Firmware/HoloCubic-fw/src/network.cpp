/*
 * HoloCubic 网络功能模块
 * 
 * 功能说明：
 * 1. 管理ESP32的WiFi连接和网络状态
 * 2. 提供HTTP客户端功能，支持RESTful API调用
 * 3. 实现网络应用接口（如获取B站粉丝数等）
 * 4. 处理JSON数据解析和网络错误处理
 * 5. 支持多种网络服务的集成扩展
 * 
 * 网络特性：
 * - 支持2.4GHz WiFi（802.11 b/g/n）
 * - 自动重连机制
 * - HTTP/HTTPS客户端
 * - JSON数据处理
 * - 网络状态监控
 * 
 * 应用场景：
 * - 获取实时数据（天气、股价、粉丝数等）
 * - 物联网数据上传
 * - 远程控制和配置
 * - 固件OTA更新
 */

#include "network.h"
#include <WiFi.h>        // ESP32 WiFi库
#include <HTTPClient.h>  // HTTP客户端库
#include <ArduinoJson.h> // JSON解析库


/**
 * WiFi网络初始化函数
 * 
 * 功能描述：
 * 1. 扫描周围可用的WiFi网络
 * 2. 显示网络列表（SSID、信号强度、加密状态）
 * 3. 连接到指定的WiFi网络
 * 4. 等待连接成功并显示IP地址
 * 
 * @param ssid WiFi网络名称
 * @param password WiFi密码
 */
void Network::init(String ssid, String password)
{
	// 开始扫描周围的WiFi网络
	Serial.println("开始扫描WiFi网络...");
	int n = WiFi.scanNetworks();
	Serial.println("WiFi扫描完成");
	
	// 检查扫描结果
	if (n == 0)
	{
		Serial.println("未发现任何WiFi网络");
	}
	else
	{
		// 显示发现的网络数量
		Serial.print("发现 ");
		Serial.print(n);
		Serial.println(" 个WiFi网络:");
		
		// 遍历并显示每个网络的详细信息
		for (int i = 0; i < n; ++i)
		{
			Serial.print(i + 1);
			Serial.print(": ");
			Serial.print(WiFi.SSID(i));        // 网络名称
			Serial.print(" (");
			Serial.print(WiFi.RSSI(i));        // 信号强度(dBm)
			Serial.print("dBm)");
			// 显示加密状态：开放网络显示空格，加密网络显示*
			Serial.println((WiFi.encryptionType(i) == WIFI_AUTH_OPEN) ? " [开放]" : " [加密]*");
			delay(10);  // 短暂延时，避免输出过快
		}
	}
	
	Serial.println("");
	Serial.print("正在连接WiFi: ");
	Serial.print(ssid.c_str());
	Serial.print(" 密码: ");
	Serial.println(password.c_str());

	// 开始连接WiFi
	WiFi.begin(ssid.c_str(), password.c_str());
	
	// 等待连接成功，每500ms检查一次状态
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(500);
		Serial.print(".");  // 显示连接进度
	}
	
	// 连接成功，显示网络信息
	Serial.println("");
	Serial.println("WiFi连接成功!");
	Serial.print("设备IP地址: ");
	Serial.println(WiFi.localIP());
}

/**
 * 获取B站用户粉丝数
 * 
 * 功能描述：
 * 1. 通过B站开放API获取指定用户的粉丝数据
 * 2. 解析JSON响应数据提取粉丝数量
 * 3. 处理网络请求异常和数据解析错误
 * 
 * API接口：http://api.bilibili.com/x/relation/stat?vmid={uid}
 * 返回格式：JSON格式，包含粉丝数、关注数等统计信息
 * 
 * @param uid B站用户UID（用户唯一标识符）
 * @return 粉丝数量，获取失败返回0
 */
unsigned int Network::getBilibiliFans(String uid)
{
	String fansCount = "";  // 存储解析出的粉丝数字符串
	HTTPClient http;        // HTTP客户端对象
	
	// 构建B站API请求URL
	String apiUrl = "http://api.bilibili.com/x/relation/stat?vmid=" + uid;
	http.begin(apiUrl);
	Serial.println("正在请求B站API: " + apiUrl);

	// 发送HTTP GET请求
	int httpCode = http.GET();
	Serial.printf("HTTP响应码: %d\n", httpCode);

	// 检查HTTP响应状态
	if (httpCode > 0)
	{
		// 请求成功，检查响应码
		if (httpCode == HTTP_CODE_OK)
		{
			// 获取响应数据
			String payload = http.getString();
			Serial.println("API响应数据: " + payload);
			
			// 简单的字符串解析方式查找粉丝数
			// 在JSON中查找"follower"字段
			int pos = payload.indexOf("follower");
			if (pos != -1)
			{
				// 提取粉丝数数值（简化解析，实际应使用JSON库）
				fansCount = payload.substring(pos + 10, payload.length() - 2);
				Serial.println("解析到的粉丝数: " + fansCount);
			}
			else
			{
				Serial.println("未找到follower字段");
			}
		}
		else
		{
			Serial.printf("HTTP请求失败，响应码: %d\n", httpCode);
		}
	}
	else
	{
		// HTTP请求出错
		Serial.printf("[HTTP] GET请求失败，错误: %s\n", http.errorToString(httpCode).c_str());
	}
	
	// 关闭HTTP连接，释放资源
	http.end();

	// 将字符串转换为数字并返回
	unsigned int result = atol(fansCount.c_str());
	Serial.printf("最终粉丝数: %u\n", result);
	return result;
}

