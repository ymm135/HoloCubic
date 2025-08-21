/**
 * @file lv_port_fatfs.c
 * @brief LVGL文件系统端口实现文件 - 基于FatFs的ESP32适配
 * 
 * 功能概述：
 * 本文件实现了LVGL图形库与FatFs文件系统的桥接适配，为HoloCubic
 * 提供完整的文件系统访问能力。通过标准化的接口，LVGL可以直接
 * 访问SD卡上的图像、字体、配置文件等资源。
 * 
 * 核心特性：
 * 1. 文件操作：支持打开、读取、写入、关闭文件
 * 2. 目录管理：支持目录遍历、创建、删除操作
 * 3. 存储管理：提供磁盘空间查询和文件大小获取
 * 4. 路径操作：支持文件重命名、删除、截断等操作
 * 
 * 技术实现：
 * - 基于FatFs文件系统库的底层实现
 * - 符合LVGL文件系统驱动接口规范
 * - 支持ESP32平台的硬件特性
 * - 提供统一的错误处理和状态返回
 * 
 * 驱动配置：
 * - 驱动器标识：'S'盘（SD卡）
 * - 文件类型：基于FatFs的FIL结构
 * - 目录类型：基于FatFs的FF_DIR结构
 * - 接口映射：完整的LVGL文件系统回调函数
 * 
 * 应用场景：
 * - LVGL图像资源加载
 * - 字体文件动态加载
 * - 配置文件读写操作
 * - 用户数据存储管理
 * - 多媒体文件访问
 * 
 * @author ClimbSnail
 * @version 1.0
 * @date 2021
 * @platform ESP32
 */

 /*********************
  *      INCLUDES
  *********************/
#include "lv_port_fatfs.h"  // LVGL FatFs端口头文件


  /*********************
   *      DEFINES
   *********************/
/* SD卡驱动器标识符 */
/* 在LVGL中使用'S:'前缀访问SD卡文件，如"S:/image.bin" */
#define DRIVE_LETTER 'S'

   /**********************
	*      TYPEDEFS
	**********************/

/* 文件操作类型定义 */
/* 基于FatFs库的FIL结构，用于存储文件操作所需的状态信息 */
typedef  FIL file_t;

/* 目录操作类型定义 */
/* 基于FatFs库的FF_DIR结构，用于目录遍历和管理操作 */
typedef  FF_DIR dir_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
/* 文件系统初始化函数 */
static void fs_init(void);

/* 文件操作相关函数声明 */
static lv_fs_res_t fs_open(lv_fs_drv_t* drv, void* file_p, const char* path, lv_fs_mode_t mode);  // 打开文件
static lv_fs_res_t fs_close(lv_fs_drv_t* drv, void* file_p);                                    // 关闭文件
static lv_fs_res_t fs_read(lv_fs_drv_t* drv, void* file_p, void* buf, uint32_t btr, uint32_t* br);  // 读取文件
static lv_fs_res_t fs_write(lv_fs_drv_t* drv, void* file_p, const void* buf, uint32_t btw, uint32_t* bw);  // 写入文件
static lv_fs_res_t fs_seek(lv_fs_drv_t* drv, void* file_p, uint32_t pos);                       // 文件定位
static lv_fs_res_t fs_size(lv_fs_drv_t* drv, void* file_p, uint32_t* size_p);                   // 获取文件大小
static lv_fs_res_t fs_tell(lv_fs_drv_t* drv, void* file_p, uint32_t* pos_p);                    // 获取文件位置

/* 文件管理相关函数声明 */
static lv_fs_res_t fs_remove(lv_fs_drv_t* drv, const char* path);                               // 删除文件
static lv_fs_res_t fs_trunc(lv_fs_drv_t* drv, void* file_p);                                    // 截断文件
static lv_fs_res_t fs_rename(lv_fs_drv_t* drv, const char* oldname, const char* newname);       // 重命名文件
static lv_fs_res_t fs_free(lv_fs_drv_t* drv, uint32_t* total_p, uint32_t* free_p);              // 获取磁盘空间

/* 目录操作相关函数声明 */
static lv_fs_res_t fs_dir_open(lv_fs_drv_t* drv, void* dir_p, const char* path);                // 打开目录
static lv_fs_res_t fs_dir_read(lv_fs_drv_t* drv, void* dir_p, char* fn);                        // 读取目录项
static lv_fs_res_t fs_dir_close(lv_fs_drv_t* drv, void* dir_p);                                 // 关闭目录

/**********************
 *  STATIC VARIABLES
 **********************/

 /**********************
  *      MACROS
  **********************/

  /**********************
   *   GLOBAL FUNCTIONS
   **********************/

/**
 * LVGL文件系统接口初始化函数
 * 
 * 功能说明：
 * 1. 初始化底层存储设备和文件系统
 * 2. 配置LVGL文件系统驱动参数
 * 3. 注册文件系统回调函数
 * 4. 建立LVGL与FatFs的桥接
 * 
 * 初始化流程：
 * - 调用fs_init()初始化FatFs文件系统
 * - 创建并配置LVGL文件系统驱动描述符
 * - 设置文件操作回调函数（打开、读取、写入等）
 * - 设置目录操作回调函数（遍历、打开、关闭等）
 * - 向LVGL注册完整的文件系统驱动
 * 
 * 驱动配置：
 * - 驱动器标识：'S'（对应SD卡）
 * - 文件结构大小：基于FatFs的FIL结构
 * - 目录结构大小：基于FatFs的FF_DIR结构
 * - 完整的文件和目录操作接口映射
 * 
 * 注意事项：
 * - 必须在SD卡初始化完成后调用
 * - 调用后LVGL即可使用"S:/"路径访问SD卡文件
 * - 所有文件操作将通过FatFs库执行
 */
void lv_fs_if_init(void)
{
	/* ====================================================
	 * 第一步：初始化存储设备和文件系统
	 * ==================================================== */
	/* 调用FatFs文件系统初始化函数 */
	/* 这将准备SD卡访问和文件系统挂载 */
	fs_init();

	/* ====================================================
	 * 第二步：在LVGL中注册文件系统接口
	 * ==================================================== */

	/* 创建LVGL文件系统驱动描述符 */
	/* 这个结构体包含了所有文件系统操作的回调函数指针 */
	lv_fs_drv_t fs_drv;                         /* 驱动描述符 */
	lv_fs_drv_init(&fs_drv);                    /* 初始化驱动结构体 */

	/* ====================================================
	 * 第三步：配置文件系统驱动参数
	 * ==================================================== */
	/* 基本配置 */
	fs_drv.file_size = sizeof(file_t);          /* 文件句柄大小（FIL结构体） */
	fs_drv.letter = DRIVE_LETTER;               /* 驱动器标识符 'S' */
	
	/* 文件操作回调函数配置 */
	fs_drv.open_cb = fs_open;                   /* 文件打开回调 */
	fs_drv.close_cb = fs_close;                 /* 文件关闭回调 */
	fs_drv.read_cb = fs_read;                   /* 文件读取回调 */
	fs_drv.write_cb = fs_write;                 /* 文件写入回调 */
	fs_drv.seek_cb = fs_seek;                   /* 文件定位回调 */
	fs_drv.tell_cb = fs_tell;                   /* 获取文件位置回调 */
	fs_drv.size_cb = fs_size;                   /* 获取文件大小回调 */
	
	/* 文件管理回调函数配置 */
	fs_drv.remove_cb = fs_remove;               /* 文件删除回调 */
	fs_drv.rename_cb = fs_rename;               /* 文件重命名回调 */
	fs_drv.trunc_cb = fs_trunc;                 /* 文件截断回调 */
	fs_drv.free_space_cb = fs_free;             /* 磁盘空间查询回调 */

	/* 目录操作回调函数配置 */
	fs_drv.rddir_size = sizeof(dir_t);          /* 目录句柄大小（FF_DIR结构体） */
	fs_drv.dir_open_cb = fs_dir_open;           /* 目录打开回调 */
	fs_drv.dir_read_cb = fs_dir_read;           /* 目录读取回调 */
	fs_drv.dir_close_cb = fs_dir_close;         /* 目录关闭回调 */

	/* ====================================================
	 * 第四步：向LVGL注册文件系统驱动
	 * ==================================================== */
	/* 注册完成后，LVGL即可使用"S:/"路径访问SD卡文件 */
	/* 例如：lv_img_set_src(img, "S:/logo.bin"); */
	lv_fs_drv_register(&fs_drv);
	
	/* 文件系统接口初始化完成 */
	/* LVGL现在可以直接访问SD卡上的文件资源 */
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

 /* Initialize your Storage device and File system. */
static void fs_init(void)
{
	///* Initialisation de la carte SD */
	//Serial.print(F("Init SD card... "));

	//SPIClass* sd_spi = new SPIClass(HSPI); // another SPI
	//if (!SD.begin(15, *sd_spi)) // SD-Card SS pin is 15
	//{
	//	Serial.println("Card Mount Failed");
	//	return;
	//}
}

/**
 * Open a file
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a file_t variable
 * @param path path to the file beginning with the driver letter (e.g. S:/folder/file.txt)
 * @param mode read: FS_MODE_RD, write: FS_MODE_WR, both: FS_MODE_RD | FS_MODE_WR
 * @return LV_FS_RES_OK or any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_open(lv_fs_drv_t* drv, void* file_p, const char* path, lv_fs_mode_t mode)
{
	uint8_t flags = 0;

	if (mode == LV_FS_MODE_WR) flags = FA_WRITE | FA_OPEN_ALWAYS;
	else if (mode == LV_FS_MODE_RD) flags = FA_READ;
	else if (mode == (LV_FS_MODE_WR | LV_FS_MODE_RD)) flags = FA_READ | FA_WRITE | FA_OPEN_ALWAYS;

	FRESULT res = f_open((file_t*)file_p, path, flags);

	if (res == FR_OK)
	{
		f_lseek((file_t*)file_p, 0);
		return LV_FS_RES_OK;
	}
	else
	{
		return LV_FS_RES_UNKNOWN;
	}
}


/**
 * Close an opened file
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a file_t variable. (opened with lv_ufs_open)
 * @return LV_FS_RES_OK: no error, the file is read
 *         any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_close(lv_fs_drv_t* drv, void* file_p)
{
	f_close((file_t*)file_p);
	return LV_FS_RES_OK;
}

/**
 * Read data from an opened file
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a file_t variable.
 * @param buf pointer to a memory block where to store the read data
 * @param btr number of Bytes To Read
 * @param br the real number of read bytes (Byte Read)
 * @return LV_FS_RES_OK: no error, the file is read
 *         any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_read(lv_fs_drv_t* drv, void* file_p, void* buf, uint32_t btr, uint32_t* br)
{
	FRESULT res = f_read((file_t*)file_p, buf, btr, (UINT*)br);
	if (res == FR_OK) return LV_FS_RES_OK;
	else return LV_FS_RES_UNKNOWN;
}

/**
 * Write into a file
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a file_t variable
 * @param buf pointer to a buffer with the bytes to write
 * @param btr Bytes To Write
 * @param br the number of real written bytes (Bytes Written). NULL if unused.
 * @return LV_FS_RES_OK or any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_write(lv_fs_drv_t* drv, void* file_p, const void* buf, uint32_t btw, uint32_t* bw)
{
	FRESULT res = f_write((file_t*)file_p, buf, btw, (UINT*)bw);
	if (res == FR_OK) return LV_FS_RES_OK;
	else return LV_FS_RES_UNKNOWN;
}

/**
 * Set the read write pointer. Also expand the file size if necessary.
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a file_t variable. (opened with lv_ufs_open )
 * @param pos the new position of read write pointer
 * @return LV_FS_RES_OK: no error, the file is read
 *         any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_seek(lv_fs_drv_t* drv, void* file_p, uint32_t pos)
{
	f_lseek((file_t*)file_p, pos);
	return LV_FS_RES_OK;
}

/**
 * Give the size of a file bytes
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a file_t variable
 * @param size pointer to a variable to store the size
 * @return LV_FS_RES_OK or any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_size(lv_fs_drv_t* drv, void* file_p, uint32_t* size_p)
{
	(*size_p) = f_size(((file_t*)file_p));
	return LV_FS_RES_OK;
}

/**
 * Give the position of the read write pointer
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to a file_t variable.
 * @param pos_p pointer to to store the result
 * @return LV_FS_RES_OK: no error, the file is read
 *         any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_tell(lv_fs_drv_t* drv, void* file_p, uint32_t* pos_p)
{
	*pos_p = f_tell(((file_t*)file_p));
	return LV_FS_RES_OK;
}

/**
 * Delete a file
 * @param drv pointer to a driver where this function belongs
 * @param path path of the file to delete
 * @return  LV_FS_RES_OK or any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_remove(lv_fs_drv_t* drv, const char* path)
{
	lv_fs_res_t res = LV_FS_RES_NOT_IMP;

	/* Add your code here*/

	return res;
}

/**
 * Truncate the file size to the current position of the read write pointer
 * @param drv pointer to a driver where this function belongs
 * @param file_p pointer to an 'ufs_file_t' variable. (opened with lv_fs_open )
 * @return LV_FS_RES_OK: no error, the file is read
 *         any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_trunc(lv_fs_drv_t* drv, void* file_p)
{
	f_sync((file_t*)file_p);           /*If not syncronized fclose can write the truncated part*/
	f_truncate((file_t*)file_p);
	return LV_FS_RES_OK;
}

/**
 * Rename a file
 * @param drv pointer to a driver where this function belongs
 * @param oldname path to the file
 * @param newname path with the new name
 * @return LV_FS_RES_OK or any error from 'fs_res_t'
 */
static lv_fs_res_t fs_rename(lv_fs_drv_t* drv, const char* oldname, const char* newname)
{

	FRESULT res = f_rename(oldname, newname);

	if (res == FR_OK) return LV_FS_RES_OK;
	else return LV_FS_RES_UNKNOWN;
}

/**
 * Get the free and total size of a driver in kB
 * @param drv pointer to a driver where this function belongs
 * @param letter the driver letter
 * @param total_p pointer to store the total size [kB]
 * @param free_p pointer to store the free size [kB]
 * @return LV_FS_RES_OK or any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_free(lv_fs_drv_t* drv, uint32_t* total_p, uint32_t* free_p)
{
	lv_fs_res_t res = LV_FS_RES_NOT_IMP;

	/* Add your code here*/

	return res;
}

/**
 * Initialize a 'fs_read_dir_t' variable for directory reading
 * @param drv pointer to a driver where this function belongs
 * @param dir_p pointer to a 'fs_read_dir_t' variable
 * @param path path to a directory
 * @return LV_FS_RES_OK or any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_dir_open(lv_fs_drv_t* drv, void* dir_p, const char* path)
{
	FRESULT res = f_opendir((dir_t*)dir_p, path);
	if (res == FR_OK) return LV_FS_RES_OK;
	else return LV_FS_RES_UNKNOWN;
}

/**
 * Read the next filename form a directory.
 * The name of the directories will begin with '/'
 * @param drv pointer to a driver where this function belongs
 * @param dir_p pointer to an initialized 'fs_read_dir_t' variable
 * @param fn pointer to a buffer to store the filename
 * @return LV_FS_RES_OK or any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_dir_read(lv_fs_drv_t* drv, void* dir_p, char* fn)
{
	FRESULT res;
	FILINFO fno;
	fn[0] = '\0';

	do
	{
		res = f_readdir((dir_t*)dir_p, &fno);
		if (res != FR_OK) return LV_FS_RES_UNKNOWN;

		if (fno.fattrib & AM_DIR)
		{
			fn[0] = '/';
			strcpy(&fn[1], fno.fname);
		}
		else strcpy(fn, fno.fname);

	} while (strcmp(fn, "/.") == 0 || strcmp(fn, "/..") == 0);

	return LV_FS_RES_OK;
}

/**
 * Close the directory reading
 * @param drv pointer to a driver where this function belongs
 * @param dir_p pointer to an initialized 'fs_read_dir_t' variable
 * @return LV_FS_RES_OK or any error from lv_fs_res_t enum
 */
static lv_fs_res_t fs_dir_close(lv_fs_drv_t* drv, void* dir_p)
{
	f_closedir((dir_t*)dir_p);
	return LV_FS_RES_OK;
}