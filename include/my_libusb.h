#ifndef MY_LIBUSB_H
#define MY_LIBUSB_H

#include "libusb.h"
#include <assert.h>  
#include "AutoClose.h"

extern "C"
{
	#define 	BURN_MAX_DEVICE 		6
	#define 	TIMEOUT 				2000
	#define 	STR_LEN 				512 * 1000
	#define 	LIBUSB_VID 				"0x0AC8"
	#define 	LIBUSB_PID 				"0x7680"
	#define 	LIBUSB_KERNEL 			"zImage"
	#define 	LIBUSB_UKERNEL 			"uImage"
	#define 	LIBUSB_DTB 				"vc0768-sv.dtb"
	
	#define 	LIBUSB_DOWN_ADDRESS 	"0x90000000"
	#define 	LIBUSB_BOOT_ADDRESS 	"0x90000400"
	#define 	LIBUSB_KERNEL_ADDRESS 	"0x80008000"
	#define 	LIBUSB_DTB_ADDRESS 		"0x80000400"

	#define 	LIBUSB_MEDIA_SPINAND	"spi nand"
	#define 	LIBUSB_MEDIA_EMMC		"emmc"
	#define 	LIBUSB_IMAGE_PATH		"\\boot\\"
	#define 	LIBUSB_VMCTOOLS_INI 	"\\VMCTools.ini"
	#define 	LIBUSB_SPINAND_INI 		"\\VMCToolsSpiNand.ini"
	#define 	LIBUSB_EMMC_INI 		"\\VMCToolsEmmc.ini"

	#define USTART  0xFA
	#define UHEAD   0xFE
	#define UDATA   0xDA
	#define UTAIL   0xED
	#define UCMD    0XAB

	#define KB 1024
	#define MB (1024*KB)
	#define GB (1024*MB)

	static char uACK[2] = { 0xAA,'\0' };
	static char uNAK[2] = { 0x55,'\0' };
	static char	EOT_OK[] = "[EOT](OK)";
	static char	EOT_ERROR[] = "[EOT](ERROR)";

	static struct _libusb_work_sb {
		BYTE buffer[STR_LEN];
		DWORD dwRead;

		unsigned char ep_bulkin;
		unsigned char ep_bulkout;
		int transfered;
		libusb_device_handle* handle;
		libusb_device** list;
		libusb_device* usbdev;
		struct libusb_device_descriptor dev_desc;
		struct libusb_config_descriptor* config_desc;
		const struct libusb_endpoint_descriptor* ep_desc;

	};

	static struct _libusb_burn {
		CString DownloadDdrAddress;
		int NeedBurn;
		CString BurnImageCmd;
		CString DownloadImagePath;
		CString PartitionSize;
		CString PartitionOffset;
		CString PartitionOffset1;

		CString Download_Size;
		bool backup_partition_flag;
	};

	static struct _bitmap_dev {
		CBitmap bitmap_connect;
		CBitmap bitmap_burning;
		CBitmap bitmap_burn_success;
		CBitmap bitmap_burn_fail;
	};
	

	/*
	* 函数名: CString2Char
	* 参数1: CString str                 待转换字符串
	* 参数2: char ch[]                       转换后将要储存的位置
	* 将Unicode下的CString转换为char*
	*/
	static void CString2Char(CString str, char ch[]) {
		int i;
		char* tmpch;
		int wLen = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);//得到Char的长度
		tmpch = new char[wLen + 1];                                             //分配变量的地址大小
		WideCharToMultiByte(CP_ACP, 0, str, -1, tmpch, wLen, NULL, NULL);       //将CString转换成char*

		for (i = 0; tmpch[i] != '\0'; i++) 
			ch[i] = tmpch[i];
		ch[i] = '\0';

	}
}

#endif