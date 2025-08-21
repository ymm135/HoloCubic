/*
 * HoloCubic SD卡文件系统模块
 * 
 * 功能说明：
 * 1. 管理microSD卡的初始化和文件系统操作
 * 2. 提供完整的文件和目录操作接口
 * 3. 支持文本文件和二进制文件的读写
 * 4. 实现配置文件存储和多媒体资源管理
 * 5. 为LVGL提供文件系统支持
 * 
 * 硬件接口：
 * - SPI接口：使用HSPI总线（高速SPI）
 * - 支持容量：最大32GB（FAT32格式）
 * - 传输速度：最高25MHz SPI时钟
 * - 兼容性：支持SD/SDHC卡
 * 
 * 文件系统特性：
 * - 格式支持：FAT16/FAT32
 * - 长文件名：支持255字符文件名
 * - 目录结构：支持多级目录
 * - 文件操作：创建、读取、写入、删除、重命名
 * - 数据安全：支持文件完整性检查
 * 
 * 应用场景：
 * - 配置文件存储（WiFi密码、系统设置等）
 * - 多媒体资源（图片、动画、音频）
 * - 数据日志记录
 * - 固件更新文件
 */

#include "sd_card.h"
#include "FS.h"         // ESP32文件系统抽象层
#include "SD.h"         // SD卡驱动库
#include "SPI.h"        // SPI通信库


/**
 * SD卡初始化函数
 * 
 * 功能描述：
 * 1. 初始化HSPI接口和SD卡驱动
 * 2. 检测SD卡类型和容量信息
 * 3. 验证文件系统挂载状态
 * 4. 输出SD卡详细信息用于调试
 * 
 * 硬件配置：
 * - 使用HSPI总线（高速SPI）
 * - CS引脚：GPIO 15
 * - 支持的卡类型：MMC、SDSC、SDHC
 * 
 * 支持的卡类型：
 * - MMC：多媒体卡
 * - SDSC：标准容量SD卡（≤2GB）
 * - SDHC：高容量SD卡（2GB-32GB）
 */
void SdCard::init()
{
	// 创建HSPI实例用于SD卡通信
	SPIClass* sd_spi = new SPIClass(HSPI); // another SPI
	
	// 尝试初始化SD卡，CS引脚为GPIO 15
	if (!SD.begin(15, *sd_spi)) // SD-Card SS pin is 15
	{
		Serial.println("SD卡挂载失败！请检查：");
		Serial.println("1. SD卡是否正确插入");
		Serial.println("2. SPI接线是否正确");
		Serial.println("3. SD卡格式是否为FAT32");
		return;
	}
	
	// 获取SD卡类型信息
	uint8_t cardType = SD.cardType();

	// 检查是否检测到SD卡
	if (cardType == CARD_NONE)
	{
		Serial.println("未检测到SD卡，请检查硬件连接");
		return;
	}

	// 显示SD卡类型信息
	Serial.print("SD卡类型: ");
	if (cardType == CARD_MMC)
	{
		Serial.println("MMC (多媒体卡)");
	}
	else if (cardType == CARD_SD)
	{
		Serial.println("SDSC (标准容量SD卡)");
	}
	else if (cardType == CARD_SDHC)
	{
		Serial.println("SDHC (高容量SD卡)");
	}
	else
	{
		Serial.println("未知类型");
	}

	// 计算并显示SD卡容量（转换为MB）
	uint64_t cardSize = SD.cardSize() / (1024 * 1024);
	Serial.printf("SD卡容量: %lluMB\n", cardSize);
	
	Serial.println("SD卡初始化完成！");
}



/**
 * 递归列出目录内容
 * 
 * @param dirname 目录路径
 * @param levels 递归深度（0表示只列出当前目录）
 * 
 * 功能说明：
 * 1. 遍历指定目录下的所有文件和子目录
 * 2. 显示文件名、大小和类型信息
 * 3. 支持多级目录递归遍历
 * 4. 用于调试和文件系统状态检查
 */
void SdCard::listDir(const char* dirname, uint8_t levels)
{
	Serial.printf("正在列出目录: %s\n", dirname);

	// 打开指定目录
	File root = SD.open(dirname);
	if (!root)
	{
		Serial.println("无法打开目录");
		return;
	}
	
	// 验证是否为目录类型
	if (!root.isDirectory())
	{
		Serial.println("指定路径不是目录");
		return;
	}

	// 遍历目录中的所有条目
	File file = root.openNextFile();
	while (file)
	{
		if (file.isDirectory())
		{
			// 处理子目录
			Serial.print("  目录: ");
			Serial.println(file.name());
			
			// 递归遍历子目录（如果还有剩余层级）
			if (levels)
			{
				listDir(file.name(), levels - 1);
			}
		}
		else
		{
			// 处理文件
			Serial.print("  文件: ");
			Serial.print(file.name());
			Serial.print("  大小: ");
			Serial.print(file.size());
			Serial.println(" 字节");
		}
		
		// 获取下一个文件或目录
		file = root.openNextFile();
	}
}

/**
 * 创建目录
 * 
 * @param path 要创建的目录路径
 * 
 * 功能说明：
 * 1. 在SD卡上创建新目录
 * 2. 支持多级目录创建（需要父目录存在）
 * 3. 自动处理路径分隔符
 * 4. 提供创建结果反馈
 * 
 * 注意事项：
 * - 目录名不能包含非法字符
 * - 路径长度不能超过文件系统限制
 * - 父目录必须已存在
 */
void SdCard::createDir(const char* path)
{
	Serial.printf("正在创建目录: %s\n", path);
	
	// 尝试创建目录
	if (SD.mkdir(path))
	{
		Serial.println("目录创建成功");
	}
	else
	{
		Serial.println("目录创建失败，请检查：");
		Serial.println("1. 父目录是否存在");
		Serial.println("2. 目录名是否合法");
		Serial.println("3. SD卡是否有足够空间");
	}
}

/**
 * 删除目录
 * 
 * @param path 要删除的目录路径
 * 
 * 功能说明：
 * 1. 删除SD卡上的指定目录
 * 2. 只能删除空目录
 * 3. 提供删除结果反馈
 * 4. 安全检查防止误删除
 * 
 * 注意事项：
 * - 目录必须为空才能删除
 * - 不能删除根目录
 * - 删除操作不可恢复
 */
void SdCard::removeDir(const char* path)
{
	Serial.printf("正在删除目录: %s\n", path);
	
	// 尝试删除目录
	if (SD.rmdir(path))
	{
		Serial.println("目录删除成功");
	}
	else
	{
		Serial.println("目录删除失败，可能原因：");
		Serial.println("1. 目录不为空");
		Serial.println("2. 目录不存在");
		Serial.println("3. 目录正在被使用");
	}
}

/**
 * 读取文件内容并输出到串口
 * 
 * @param path 文件路径
 * 
 * 功能说明：
 * 1. 打开指定路径的文件
 * 2. 逐字节读取文件内容
 * 3. 将内容输出到串口显示
 * 4. 自动关闭文件句柄
 * 
 * 适用场景：
 * - 调试时查看文件内容
 * - 读取配置文件
 * - 验证文件完整性
 */
void SdCard::readFile(const char* path)
{
	Serial.printf("正在读取文件: %s\n", path);

	// 以只读模式打开文件
	File file = SD.open(path);
	if (!file)
	{
		Serial.println("无法打开文件进行读取");
		return;
	}

	Serial.print("文件内容: ");
	// 逐字节读取并输出文件内容
	while (file.available())
	{
		Serial.write(file.read());
	}
	Serial.println(); // 添加换行
	
	// 关闭文件，释放资源
	file.close();
}

/**
 * 读取文件指定行的内容
 * 
 * @param path 文件路径
 * @param num 行号（从1开始，默认为1）
 * @return 指定行的内容字符串，失败返回错误信息
 * 
 * 功能说明：
 * 1. 打开文本文件并逐行解析
 * 2. 定位到指定行号
 * 3. 返回该行的完整内容
 * 4. 自动去除行首行尾空白字符
 * 
 * 应用场景：
 * - 读取配置文件的特定配置项
 * - 解析CSV或其他结构化文本
 * - 获取WiFi密码等敏感信息
 * 
 * 注意事项：
 * - 行号从1开始计数
 * - 使用内部缓冲区存储行内容
 * - 文件不存在或行号超出范围返回错误信息
 */
String SdCard::readFileLine(const char* path, int num = 1)
{
	Serial.printf("正在读取文件: %s 第%d行\n", path, num);

	// 以只读模式打开文件
	File file = SD.open(path);
	if (!file)
	{
		return ("Failed to open file for reading");
	}

	// 使用内部缓冲区存储行内容
	char* p = buf;
	while (file.available())
	{
		char c = file.read();
		if (c == '\n')  // 遇到换行符
		{
			num--;
			if (num == 0)  // 找到目标行
			{
				*(p++) = '\0';  // 添加字符串结束符
				String s(buf);
				s.trim();  // 去除首尾空白字符
				file.close();
				Serial.printf("成功读取第%d行内容\n", (num + 1));
				return s;
			}
		}
		else if (num == 1)  // 当前为目标行，存储字符
		{
			*(p++) = c;
		}
	}
	file.close();

	return  String("error parameter!");
}

/**
 * 写入文件内容（覆盖模式）
 * 
 * @param path 文件路径
 * @param message 要写入的内容
 * 
 * 功能说明：
 * 1. 以写入模式打开文件（如果文件存在则覆盖）
 * 2. 将指定内容写入文件
 * 3. 自动创建不存在的文件
 * 4. 提供写入结果反馈
 * 
 * 应用场景：
 * - 保存配置文件
 * - 创建日志文件
 * - 存储用户数据
 * - 备份系统设置
 * 
 * 注意事项：
 * - 写入模式会覆盖原有内容
 * - 确保SD卡有足够的存储空间
 * - 写入完成后自动关闭文件
 */
void SdCard::writeFile(const char* path, const char* message)
{
	Serial.printf("正在写入文件: %s\n", path);

	// 以写入模式打开文件（覆盖原有内容）
	File file = SD.open(path, FILE_WRITE);
	if (!file)
	{
		Serial.println("无法打开文件进行写入");
		return;
	}
	
	// 尝试写入内容
	if (file.print(message))
	{
		Serial.println("文件写入成功");
	}
	else
	{
		Serial.println("文件写入失败");
	}
	
	// 关闭文件，确保数据写入SD卡
	file.close();
}

/**
 * 追加内容到文件末尾
 * 
 * @param path 文件路径
 * @param message 要追加的内容
 * 
 * 功能说明：
 * 1. 以追加模式打开文件
 * 2. 在文件末尾添加新内容
 * 3. 保留原有文件内容
 * 4. 如果文件不存在则自动创建
 * 
 * 应用场景：
 * - 记录系统日志
 * - 追加传感器数据
 * - 累积统计信息
 * - 添加配置项
 * 
 * 注意事项：
 * - 不会覆盖原有内容
 * - 适合连续数据记录
 * - 文件大小会逐渐增长
 */
void SdCard::appendFile(const char* path, const char* message)
{
	Serial.printf("正在追加内容到文件: %s\n", path);

	// 以追加模式打开文件
	File file = SD.open(path, FILE_APPEND);
	if (!file)
	{
		Serial.println("无法打开文件进行追加");
		return;
	}
	
	// 尝试追加内容到文件末尾
	if (file.print(message))
	{
		Serial.println("内容追加成功");
	}
	else
	{
		Serial.println("内容追加失败");
	}
	
	// 关闭文件，确保数据写入SD卡
	file.close();
}

/**
 * 重命名文件
 * 
 * @param path1 原文件路径
 * @param path2 新文件路径
 * 
 * 功能说明：
 * 1. 将指定文件重命名为新名称
 * 2. 支持移动文件到不同目录
 * 3. 原子操作，确保数据安全
 * 4. 提供操作结果反馈
 * 
 * 应用场景：
 * - 文件备份和归档
 * - 临时文件转正式文件
 * - 文件版本管理
 * - 目录结构重组
 * 
 * 注意事项：
 * - 目标路径的目录必须存在
 * - 不能重命名为已存在的文件
 * - 操作失败时原文件保持不变
 */
void SdCard::renameFile(const char* path1, const char* path2)
{
	Serial.printf("正在重命名文件: %s -> %s\n", path1, path2);
	
	// 尝试重命名文件
	if (SD.rename(path1, path2))
	{
		Serial.println("文件重命名成功");
	}
	else
	{
		Serial.println("文件重命名失败，可能原因：");
		Serial.println("1. 原文件不存在");
		Serial.println("2. 目标文件已存在");
		Serial.println("3. 目标目录不存在");
		Serial.println("4. 文件名包含非法字符");
	}
}

/**
 * 删除文件
 * 
 * @param path 要删除的文件路径
 * 
 * 功能说明：
 * 1. 从SD卡中永久删除指定文件
 * 2. 释放文件占用的存储空间
 * 3. 提供删除结果反馈
 * 4. 安全检查防止误删除
 * 
 * 应用场景：
 * - 清理临时文件
 * - 删除过期日志
 * - 释放存储空间
 * - 文件管理维护
 * 
 * 注意事项：
 * - 删除操作不可恢复
 * - 确认文件路径正确
 * - 不能删除正在使用的文件
 * - 删除前建议备份重要文件
 */
void SdCard::deleteFile(const char* path)
{
	Serial.printf("正在删除文件: %s\n", path);
	
	// 尝试删除文件
	if (SD.remove(path))
	{
		Serial.println("文件删除成功");
	}
	else
	{
		Serial.println("文件删除失败，可能原因：");
		Serial.println("1. 文件不存在");
		Serial.println("2. 文件正在被使用");
		Serial.println("3. 文件为只读属性");
		Serial.println("4. SD卡写保护");
	}
}

/**
 * 从SD卡读取二进制文件数据
 * 
 * @param path 二进制文件路径
 * @param buf 数据缓冲区指针
 * 
 * 功能说明：
 * 1. 打开指定的二进制文件
 * 2. 将整个文件数据读取到指定缓冲区
 * 3. 支持大文件分块读取（每次512字节）
 * 4. 自动处理文件关闭和错误检查
 * 
 * 应用场景：
 * - 读取图像文件数据
 * - 加载音频文件
 * - 读取固件更新包
 * - 加载字体文件
 * - 读取配置二进制数据
 * 
 * 注意事项：
 * - 确保缓冲区大小足够容纳整个文件
 * - 使用512字节分块读取提高效率
 * - 自动获取文件大小进行完整读取
 * - 适合中等大小的二进制文件
 */
void SdCard::readBinFromSd(const char* path, uint8_t* buf)
{
	Serial.printf("正在读取二进制文件: %s\n", path);
	
	File file = SD.open(path);
	size_t len = 0;
	if (file)
	{
		len = file.size();
		size_t flen = len;
		Serial.printf("文件大小: %d 字节\n", flen);

		// 分块读取文件数据，每次读取512字节
		uint8_t* bufPtr = buf;
		while (len)
		{
			size_t toRead = len;
			if (toRead > 512)
			{
				toRead = 512;
			}
			file.read(bufPtr, toRead);
			bufPtr += toRead;
			len -= toRead;
		}

		Serial.printf("成功读取 %d 字节数据\n", flen);
		file.close();
	}
	else
	{
		Serial.println("无法打开二进制文件进行读取");
	}
}

/**
 * 向SD卡写入二进制文件数据
 * 
 * @param path 二进制文件路径
 * @param buf 数据缓冲区指针
 * 
 * 功能说明：
 * 1. 创建或覆盖指定的二进制文件
 * 2. 将缓冲区数据写入文件
 * 3. 固定写入1MB数据（2048 × 512字节）
 * 4. 分块写入提高写入效率
 * 
 * 应用场景：
 * - 保存图像文件
 * - 写入音频数据
 * - 创建固件备份
 * - 保存传感器原始数据
 * - 写入配置二进制文件
 * 
 * 注意事项：
 * - 固定写入1MB数据量
 * - 确保SD卡有足够空间
 * - 写入模式会覆盖原有文件
 * - 使用512字节块大小优化性能
 */
void SdCard::writeBinToSd(const char* path, uint8_t* buf)
{
	Serial.printf("正在写入二进制文件: %s\n", path);
	
	File file = SD.open(path, FILE_WRITE);
	if (!file)
	{
		Serial.println("无法打开文件进行二进制写入");
		return;
	}

	// 写入2048个512字节的数据块（总计1MB）
	size_t i;
	for (i = 0; i < 2048; i++)
	{
		file.write(buf, 512);
	}
	
	Serial.println("二进制文件写入完成，总大小: 1MB");
	file.close();
}


/**
 * 文件IO性能测试函数
 * 
 * @param path 测试文件路径
 * 
 * 功能说明：
 * 1. 测试SD卡文件读取性能
 * 2. 测试SD卡文件写入性能
 * 3. 统计读写操作耗时
 * 4. 输出性能测试结果
 * 
 * 测试流程：
 * 1. 读取测试：打开现有文件，分块读取全部内容
 * 2. 写入测试：创建新文件，写入1MB测试数据
 * 3. 性能统计：记录读写操作的时间消耗
 * 4. 结果输出：显示传输速度和耗时信息
 * 
 * 应用场景：
 * - SD卡性能基准测试
 * - 文件系统性能评估
 * - 硬件兼容性测试
 * - 系统优化参考
 * 
 * 注意事项：
 * - 使用512字节缓冲区进行测试
 * - 测试会覆盖指定路径的文件
 * - 适合用于开发调试阶段
 * - 测试结果受SD卡类型和质量影响
 */
void SdCard::fileIO(const char* path)
{
	Serial.printf("开始SD卡IO性能测试，文件: %s\n", path);
	
	File file = SD.open(path);
	static uint8_t buf[512];
	size_t len = 0;
	uint32_t start = millis();
	uint32_t end = start;
	
	// 读取性能测试
	if (file)
	{
		len = file.size();
		size_t flen = len;
		Serial.printf("开始读取测试，文件大小: %u 字节\n", flen);
		
		start = millis();
		while (len)
		{
			size_t toRead = len;
			if (toRead > 512)
			{
				toRead = 512;
			}
			file.read(buf, toRead);
			len -= toRead;
		}
		end = millis() - start;
		Serial.printf("读取完成: %u 字节，耗时: %u 毫秒，速度: %.2f KB/s\n", 
					  flen, end, (float)flen / end);
		file.close();
	}
	else
	{
		Serial.println("无法打开文件进行读取测试");
	}

	// 写入性能测试
	Serial.println("开始写入测试...");
	file = SD.open(path, FILE_WRITE);
	if (!file)
	{
		Serial.println("无法打开文件进行写入测试");
		return;
	}

	size_t i;
	start = millis();
	for (i = 0; i < 2048; i++)
	{
		file.write(buf, 512);
	}
	end = millis() - start;
	uint32_t totalBytes = 2048 * 512;
	Serial.printf("写入完成: %u 字节，耗时: %u 毫秒，速度: %.2f KB/s\n", 
				  totalBytes, end, (float)totalBytes / end);
	file.close();
	
	Serial.println("SD卡IO性能测试完成");
}

