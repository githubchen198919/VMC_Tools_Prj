// CBURN.cpp: 实现文件
//

#include "pch.h"
#include "VMCTools.h"
#include "CBURN.h"
#include "afxdialogex.h"

#include "my_libusb.h"
#include "CDEBUGDlg.h"

#pragma comment(lib,"libusb-1.0.lib")  

static struct _libusb_work_sb libusb_work_sb_burn[BURN_MAX_DEVICE];
static struct _libusb_burn libusb_burn[BURN_MAX_DEVICE][10];
static libusb_device** my_access_list;
static libusb_device* my_usbdev;
static struct libusb_device_descriptor my_dev_desc;

struct _bitmap_dev bitmap_dev;

CMutex global_Mutex(0, 0, 0);
CMutex global_print(0, 0, 0);

// CBURN 对话框

IMPLEMENT_DYNAMIC(CBURN, CDialogEx)

CBURN::CBURN(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_BURN, pParent)
	, m_cstring_debug_print(_T(""))
{

}

CBURN::~CBURN()
{
}

void CBURN::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_STATIC_BURN_DEV1, m_burn_dev1);
	DDX_Control(pDX, IDC_STATIC_BURN_DEV2, m_burn_dev2);
	DDX_Control(pDX, IDC_STATIC_BURN_DEV3, m_burn_dev3);
	DDX_Control(pDX, IDC_STATIC_BURN_DEV4, m_burn_dev4);
	DDX_Control(pDX, IDC_STATIC_BURN_DEV5, m_burn_dev5);
	DDX_Control(pDX, IDC_STATIC_BURN_DEV6, m_burn_dev6);
	DDX_Control(pDX, IDC_EDIT_TOTAL, m_exit_total);
	DDX_Control(pDX, IDC_EDIT_SUCCESS, m_exit_success);
	DDX_Control(pDX, IDC_EDIT_FAIL, m_exit_fail);
	DDX_Control(pDX, IDC_BUTTON_ACCESS_DEVICE, m_button_access_device);
	DDX_Control(pDX, IDC_PROGRESS_DEV1, m_burn_progress_dev0);
	DDX_Control(pDX, IDC_PROGRESS_DEV2, m_burn_progress_dev1);
	DDX_Control(pDX, IDC_PROGRESS_DEV3, m_burn_progress_dev2);
	DDX_Control(pDX, IDC_PROGRESS_DEV4, m_burn_progress_dev3);
	DDX_Control(pDX, IDC_PROGRESS_DEV5, m_burn_progress_dev4);
	DDX_Control(pDX, IDC_PROGRESS_DEV6, m_burn_progress_dev5);
	DDX_Control(pDX, IDC_STATIC_BURN_PLAN1, m_burn_plan_dev0);
	DDX_Control(pDX, IDC_STATIC_BURN_PLAN2, m_burn_plan_dev1);
	DDX_Control(pDX, IDC_STATIC_BURN_PLAN3, m_burn_plan_dev2);
	DDX_Control(pDX, IDC_STATIC_BURN_PLAN4, m_burn_plan_dev3);
	DDX_Control(pDX, IDC_STATIC_BURN_PLAN5, m_burn_plan_dev4);
	DDX_Control(pDX, IDC_STATIC_BURN_PLAN6, m_burn_plan_dev5);
	DDX_Control(pDX, IDC_BUTTON_DELETE_DEVICE, m_button_delete_device);
	DDX_Control(pDX, IDC_BUTTON_START_BURN, m_button_start_burn);
	DDX_Control(pDX, IDC_STATIC_BURN_PIC_DEV0, m_pic_burn_dev0);
	DDX_Control(pDX, IDC_STATIC_BURN_PIC_DEV1, m_pic_burn_dev1);
	DDX_Control(pDX, IDC_STATIC_BURN_PIC_DEV2, m_pic_burn_dev2);
	DDX_Control(pDX, IDC_STATIC_BURN_PIC_DEV3, m_pic_burn_dev3);
	DDX_Control(pDX, IDC_STATIC_BURN_PIC_DEV4, m_pic_burn_dev4);
	DDX_Control(pDX, IDC_STATIC_BURN_PIC_DEV5, m_pic_burn_dev5);
	DDX_Control(pDX, IDC_CHECK_RESET, m_need_reset);
	DDX_Control(pDX, IDC_EDIT_DEBUG_PRINT, m_edit_debug_print);
	DDX_Text(pDX, IDC_EDIT_DEBUG_PRINT, m_cstring_debug_print);
}


BEGIN_MESSAGE_MAP(CBURN, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_ACCESS_DEVICE, &CBURN::OnBnClickedButtonAccessDevice)
	ON_BN_CLICKED(IDC_BUTTON_START_BURN, &CBURN::OnBnClickedButtonStartBurn)
	ON_BN_CLICKED(IDC_BUTTON_DELETE_DEVICE, &CBURN::OnBnClickedButtonDeleteDevice)
END_MESSAGE_MAP()


// CBURN 消息处理程序


/*
* Multiple devices burn function
*/
void CBURN::MyFunctionBurnBitMap()
{
	bitmap_dev.bitmap_connect.LoadBitmapW(IDB_BITMAP1);
	bitmap_dev.bitmap_burning.LoadBitmapW(IDB_BITMAP2);
	bitmap_dev.bitmap_burn_success.LoadBitmapW(IDB_BITMAP3);
	bitmap_dev.bitmap_burn_fail.LoadBitmapW(IDB_BITMAP4);
}

void CBURN::MyFunctionPrintDebug(int dev_num, CString buf)
{
	CString bus_hub_num;

	CEdit* pEdit = (CEdit*)GetDlgItem(IDC_EDIT_DEBUG_PRINT);
	int nLength = pEdit->GetWindowTextLengthW();
	pEdit->SetSel(nLength, nLength);

	CString data;

	switch (dev_num)
	{
		case 0:
			GetDlgItemText(IDC_STATIC_BURN_DEV1, bus_hub_num);
			data = bus_hub_num + _T(" : ") + buf + _T("\r\n");
			pEdit->ReplaceSel(data);
			break;
		case 1:
			GetDlgItemText(IDC_STATIC_BURN_DEV2, bus_hub_num);
			data = bus_hub_num + _T(" : ") + buf + _T("\r\n");
			pEdit->ReplaceSel(data);
			break;
		case 2:
			GetDlgItemText(IDC_STATIC_BURN_DEV3, bus_hub_num);
			data = bus_hub_num + _T(" : ") + buf + _T("\r\n");
			pEdit->ReplaceSel(data);
			break;
		case 3:
			GetDlgItemText(IDC_STATIC_BURN_DEV4, bus_hub_num);
			data = bus_hub_num + _T(" : ") + buf + _T("\r\n");
			pEdit->ReplaceSel(data);
			break;
		case 4:
			GetDlgItemText(IDC_STATIC_BURN_DEV5, bus_hub_num);
			data = bus_hub_num + _T(" : ") + buf + _T("\r\n");
			pEdit->ReplaceSel(data);
			break;
		case 5:
			GetDlgItemText(IDC_STATIC_BURN_DEV6, bus_hub_num);
			data = bus_hub_num + _T(" : ") + buf + _T("\r\n");
			pEdit->ReplaceSel(data);
			break;
	}
}

CString CBURN::MyFunctionBurnGetExePath()
{
	TCHAR  exepath[MAX_PATH];
	CString  exe_dir, tmpdir;
	memset(exepath, 0, MAX_PATH);
	GetModuleFileName(NULL, exepath, MAX_PATH);
	tmpdir = exepath;
	// exe_dir : .exe程序运行路径
	exe_dir = tmpdir.Left(tmpdir.ReverseFind('\\'));

	return exe_dir;
}

CString MyFunctionBurnUTF82WCS(const char* szU8)
{
	//预转换，得到所需空间的大小;
	int wcsLen = ::MultiByteToWideChar(CP_UTF8, NULL, szU8, strlen(szU8), NULL, 0);

	//分配空间要给''留个空间，MultiByteToWideChar不会给''空间
	wchar_t* wszString = new wchar_t[wcsLen + 1];

	//转换
	::MultiByteToWideChar(CP_UTF8, NULL, szU8, strlen(szU8), wszString, wcsLen);

	//最后加上''
	wszString[wcsLen] = '\0';

	CString unicodeString(wszString);

	delete[] wszString;
	wszString = NULL;

	return unicodeString;
}

char* MyFunctionBurnUnicodeToUtf8(const wchar_t* unicode)
{
	int len;
	len = WideCharToMultiByte(CP_UTF8, 0, unicode, -1, NULL, 0, NULL, NULL);
	char* szUtf8 = (char*)malloc(len + 1);
	memset(szUtf8, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, unicode, -1, szUtf8, len, NULL, NULL);
	return szUtf8;
}

CString CBURN::MyFunctionBurnHandshake(struct _libusb_work_sb* myfunc_work_sb)
{
	// TODO: 在此添加控件通知处理程序代码
	int ret;

	myfunc_work_sb->buffer[0] = USTART;
	ret = libusb_bulk_transfer(myfunc_work_sb->handle, myfunc_work_sb->ep_bulkout,
		myfunc_work_sb->buffer, 1,
		&myfunc_work_sb->transfered, TIMEOUT);
	if (ret == 0)
	{
		ret = libusb_bulk_transfer(myfunc_work_sb->handle, myfunc_work_sb->ep_bulkin,
			myfunc_work_sb->buffer,
			sizeof(myfunc_work_sb->buffer), &myfunc_work_sb->transfered, TIMEOUT);

		if (ret == 0)
		{
			if (strncmp(uACK, (char*)myfunc_work_sb->buffer, myfunc_work_sb->transfered) == 0)
			{
				/*MessageBoxTimeout(NULL, _T("握手成功，即将开始烧录......"),
					_T("提示"),
					MB_ICONINFORMATION,
					GetSystemDefaultLangID(),
					1000);*/
			}
			else
			{
				//MessageBox(TEXT("设备握手失败"), TEXT("错误"), MB_OK | MB_ICONWARNING);
				return _T("ERROR");
			}

		}
		else
		{
			//MessageBox(TEXT("[MyFunctionHandshake] start in failed!"), TEXT("错误"), MB_OK | MB_ICONWARNING);
			return _T("ERROR");
		}

	}
	else
	{
		//MessageBox(TEXT("[MyFunctionHandshake] start out failed!"), TEXT("错误"), MB_OK | MB_ICONWARNING);
		return _T("ERROR");
	}

	return _T("OK");
}

CString CBURN::MyFunctionBurnDownloadImage(int dev_num, struct _libusb_work_sb* myfunc_work_sb, CString download_file_path, CString download_file_address_tmp)
{
	// TODO: 在此添加控件通知处理程序代码
	int ret;
	CFile Donwnload_File;
	CFileException Donwnload_File_Error; // 用于保存出错信息

	// Setp 1 : open Donwnload File 
	//if (!Donwnload_File.Open(download_file_path, CFile::modeRead, &Donwnload_File_Error))
	if (!Donwnload_File.Open(download_file_path, CFile::shareDenyNone, &Donwnload_File_Error))
	{
		download_file_path += " open failed!!!";
		//MessageBox(download_file_path, TEXT("错误"), MB_OK | MB_ICONWARNING);
		//TCHAR szError[1024];
		//Donwnload_File_Error.GetErrorMessage(szError, 1024);
		//_tprintf_s(_T("Couldn't open source file: %1024s"), szError);

		global_print.Lock();
		MyFunctionPrintDebug(dev_num, download_file_path);
		global_print.Unlock();

		return _T("ERROR");
	}

	// Setp 2 : Get Donwnload File Address and Get Donwnload File Size 
	ULONGLONG download_file_size, download_file_address;
	CFileStatus fileStatus;

	download_file_address = _tcstoul(download_file_address_tmp, 0, 16);

	CString return_download_file_size;
	if (CFile::GetStatus(download_file_path, fileStatus))
	{
		download_file_size = fileStatus.m_size;
		return_download_file_size.Format(_T("%#x"), download_file_size);
		//CString sizetmp;
		//sizetmp.Format(_T("%#x"), download_file_size);
		//setText(IDC_EDIT_DEBUG_DOWN_SIZE, sizetmp);
		//speed_str.Format(_T("size_tmp: %d"), size_tmp);
		//MessageBox(speed_str, TEXT("LibUSB"), MB_OK | MB_ICONWARNING);
	}

	global_print.Lock();
	MyFunctionPrintDebug(dev_num, _T("set donwnload file address and size started."));
	global_print.Unlock();

	// Setp 3 : Set Donwnload File Address and Size 
	myfunc_work_sb->buffer[0] = UHEAD;
	myfunc_work_sb->buffer[1] = (download_file_size & 0xFF000000) >> 24;
	myfunc_work_sb->buffer[2] = (download_file_size & 0xFF0000) >> 16;
	myfunc_work_sb->buffer[3] = (download_file_size & 0xFF00) >> 8;
	myfunc_work_sb->buffer[4] = (download_file_size & 0xFF);
	myfunc_work_sb->buffer[5] = (download_file_address & 0xFF000000) >> 24;
	myfunc_work_sb->buffer[6] = (download_file_address & 0xFF0000) >> 16;
	myfunc_work_sb->buffer[7] = (download_file_address & 0xFF00) >> 8;
	myfunc_work_sb->buffer[8] = (download_file_address & 0xFF);

	ret = libusb_bulk_transfer(myfunc_work_sb->handle, myfunc_work_sb->ep_bulkout,
		myfunc_work_sb->buffer, 9,
		&myfunc_work_sb->transfered, TIMEOUT);
	if (ret == 0)
	{
		// 读取数据
		ret = libusb_bulk_transfer(myfunc_work_sb->handle, myfunc_work_sb->ep_bulkin,
			myfunc_work_sb->buffer,
			sizeof(myfunc_work_sb->buffer), &myfunc_work_sb->transfered, TIMEOUT);
		if (ret == 0)
		{
			if (strncmp(uACK, (char*)myfunc_work_sb->buffer, myfunc_work_sb->transfered) == 0)
			{
				global_print.Lock();
				MyFunctionPrintDebug(dev_num, _T("(OK)"));
				global_print.Unlock();
			}
			else
			{
				//MessageBox(TEXT("Download地址和长度设置失败"), TEXT("错误"), MB_OK | MB_ICONWARNING);
				global_print.Lock();
				MyFunctionPrintDebug(dev_num, _T("[3] set donwnload file address and size failed."));
				global_print.Unlock();

				goto down_image_error;
			}
		}
		else
		{
			//MessageBox(TEXT("[MyFunctionDownloadImage] start in failed!"), TEXT("错误"), MB_OK | MB_ICONWARNING);
			global_print.Lock();
			MyFunctionPrintDebug(dev_num, _T("[2] set donwnload file address and size failed."));
			global_print.Unlock();

			goto down_image_error;
		}
	}
	else
	{
		//MessageBox(TEXT("[MyFunctionDownloadImage] Set Donwnload File Address and Size failed!"),
		//TEXT("错误"), MB_OK | MB_ICONWARNING);
		global_print.Lock();
		MyFunctionPrintDebug(dev_num, _T("[1] set donwnload file address and size failed."));
		global_print.Unlock();
		goto down_image_error;
	}

	global_print.Lock();
	MyFunctionPrintDebug(dev_num, download_file_path + _T(" downloading started"));
	global_print.Unlock();

	// Setp 4 :  transfer Donwnload File
	while (1)
	{
		myfunc_work_sb->dwRead = Donwnload_File.Read(myfunc_work_sb->buffer, sizeof(myfunc_work_sb->buffer));
		if (myfunc_work_sb->dwRead == 0)
		{
			break;
		}

		ret = libusb_bulk_transfer(myfunc_work_sb->handle, myfunc_work_sb->ep_bulkout,
			myfunc_work_sb->buffer, myfunc_work_sb->dwRead,
			&myfunc_work_sb->transfered, TIMEOUT);
		if (ret == 0)
		{

		}
		else
		{
			//MessageBox(TEXT("[MyFunctionDownloadImage] transfer Donwnload File failed!"), 
			//					TEXT("错误"), MB_OK | MB_ICONWARNING);
			global_print.Lock();
			MyFunctionPrintDebug(dev_num, _T("transfer Donwnload File failed."));
			global_print.Unlock();

			goto down_image_error;
			break;
		}

	}

	global_print.Lock();
	MyFunctionPrintDebug(dev_num, _T("(OK)"));
	global_print.Unlock();

	// Setp 5 :  close Donwnload File
	Donwnload_File.Close();


	return return_download_file_size;

down_image_error:

	Donwnload_File.Close();

	return _T("ERROR");
}

CString CBURN::MyFunctionBurnDownloadCmd(int dev_num, struct _libusb_work_sb* myfunc_work_sb, char* cmd_name, int cmd_len)
{
	int ret;
	CString print_data;
	CString lb(_T("["));
	CString rb(_T("]"));

	myfunc_work_sb->buffer[0] = UCMD;
	memcpy(&myfunc_work_sb->buffer[3], cmd_name, cmd_len);

	ret = libusb_bulk_transfer(myfunc_work_sb->handle, myfunc_work_sb->ep_bulkout,
		myfunc_work_sb->buffer, 3 + cmd_len,
		&myfunc_work_sb->transfered, TIMEOUT);
	if (ret == 0)
	{
		global_print.Lock();
		print_data = lb + MyFunctionBurnUTF82WCS(cmd_name) + rb + _T(" Command send success.");
		MyFunctionPrintDebug(dev_num, print_data);
		global_print.Unlock();

		if (strncmp(cmd_name, "reset", cmd_len))
		{
			ret = libusb_bulk_transfer(myfunc_work_sb->handle, myfunc_work_sb->ep_bulkin,
				myfunc_work_sb->buffer, sizeof(myfunc_work_sb->buffer),
				&myfunc_work_sb->transfered, TIMEOUT * 200);
			if (ret == 0)
			{
				if (strncmp(EOT_OK, (char*)myfunc_work_sb->buffer, myfunc_work_sb->transfered) == 0)
				{
					/*MessageBoxTimeout(NULL, _T("命令处理成功"), _T("提示"),
									MB_ICONINFORMATION,
									GetSystemDefaultLangID(),
									1000);*/
					global_print.Lock();
					print_data = _T("(OK)");
					MyFunctionPrintDebug(dev_num, print_data);
					global_print.Unlock();
				}
				else
				{
					//MessageBox(TEXT("命令处理失败"), TEXT("错误"), MB_OK | MB_ICONWARNING);
					global_print.Lock();
					print_data = _T("（ERROR)");
					MyFunctionPrintDebug(dev_num, print_data);
					global_print.Unlock();
					return _T("ERROR");
				}
			}
			else
			{
				//MessageBox(TEXT("cmd in failed!"), TEXT("错误"), MB_OK | MB_ICONWARNING);	
				global_print.Lock();
				print_data = _T("Command rec failure.");
				MyFunctionPrintDebug(dev_num, print_data);
				global_print.Unlock();
				return _T("ERROR");
			}
		}
	}
	else
	{
		//MessageBox(TEXT("cmd out failed!"), TEXT("错误"), MB_OK | MB_ICONWARNING);
		global_print.Lock();
		print_data = _T("Command send failure.");
		MyFunctionPrintDebug(dev_num, print_data);
		global_print.Unlock();
		return _T("ERROR");
	}

	return _T("OK");
}

CString CBURN::MyFunctionBurnProCmd(int dev_num, struct _libusb_work_sb* myfunc_work_sb, struct _libusb_burn b)
{
	// TODO: 在此添加控件通知处理程序代码
	int cmd_len;
	CString cmd_boot, cmd_status;
	CString burn_plan;
	char cmd_name[MAX_PATH] = { 0 };

	//int npos = m_progress_debug_burn.GetPos();
	int npos = 0;

	switch (dev_num)
	{
		case 0:
			npos = m_burn_progress_dev0.GetPos();
			break;
		case 1:
			npos = m_burn_progress_dev1.GetPos();
			break;
		case 2:
			npos = m_burn_progress_dev2.GetPos();
			break;
		case 3:
			npos = m_burn_progress_dev3.GetPos();
			break;
		case 4:
			npos = m_burn_progress_dev4.GetPos();
			break;
		case 5:
			npos = m_burn_progress_dev5.GetPos();
			break;
	}

	if (!b.BurnImageCmd.Compare(_T("vburn nand")))
	{
		// vburn nand 0x90000000

		cmd_boot.Empty();
		cmd_boot += _T("vburn nand");
		cmd_boot += _T(" ");
		cmd_boot += b.DownloadDdrAddress;

		CString2Char(cmd_boot, cmd_name);
		cmd_status = MyFunctionBurnDownloadCmd(dev_num, myfunc_work_sb, cmd_name, cmd_boot.GetLength());
		if (!cmd_status.Compare(_T("ERROR")))
		{
			return _T("ERROR");
		}

		/*npos += 10;
		m_progress_debug_burn.SetPos(npos);*/
		switch (dev_num)
		{
			case 0:
				npos += 10;
				burn_plan.Format(_T("%d%c"), npos, '%');
				m_burn_plan_dev0.SetWindowTextW(burn_plan);
				m_burn_progress_dev0.SetPos(npos);
				break;
			case 1:
				npos += 10;
				burn_plan.Format(_T("%d%c"), npos, '%');
				m_burn_plan_dev1.SetWindowTextW(burn_plan);
				m_burn_progress_dev1.SetPos(npos);
				break;
			case 2:
				npos += 10;
				burn_plan.Format(_T("%d%c"), npos, '%');
				m_burn_plan_dev2.SetWindowTextW(burn_plan);
				m_burn_progress_dev2.SetPos(npos);
				break;
			case 3:
				npos += 10;
				burn_plan.Format(_T("%d%c"), npos, '%');
				m_burn_plan_dev3.SetWindowTextW(burn_plan);
				m_burn_progress_dev3.SetPos(npos);
				break;
			case 4:
				npos += 10;
				burn_plan.Format(_T("%d%c"), npos, '%');
				m_burn_plan_dev4.SetWindowTextW(burn_plan);
				m_burn_progress_dev4.SetPos(npos);
				break;
			case 5:
				npos += 10;
				burn_plan.Format(_T("%d%c"), npos, '%');
				m_burn_plan_dev5.SetWindowTextW(burn_plan);
				m_burn_progress_dev5.SetPos(npos);
				break;
		}
	}
	else if (!b.BurnImageCmd.Compare(_T("nand write.align")))
	{
		//nand erase 0x1000000 0x800000
		//nand write.align 0x90000000 0x1000000 Size
		cmd_boot.Empty();
		cmd_boot += _T("nand erase");
		cmd_boot += _T(" ");
		cmd_boot += b.PartitionOffset;
		cmd_boot += _T(" ");
		cmd_boot += b.PartitionSize;

		CString2Char(cmd_boot, cmd_name);
		cmd_status = MyFunctionBurnDownloadCmd(dev_num, myfunc_work_sb, cmd_name, cmd_boot.GetLength());
		if (!cmd_status.Compare(_T("ERROR")))
		{
			return _T("ERROR");
		}

		cmd_boot.Empty();
		cmd_boot += _T("nand write.align");
		cmd_boot += _T(" ");
		cmd_boot += b.DownloadDdrAddress;
		cmd_boot += _T(" ");
		cmd_boot += b.PartitionOffset;
		cmd_boot += _T(" ");
		cmd_boot += b.Download_Size;

		CString2Char(cmd_boot, cmd_name);
		cmd_status = MyFunctionBurnDownloadCmd(dev_num, myfunc_work_sb, cmd_name, cmd_boot.GetLength());
		if (!cmd_status.Compare(_T("ERROR")))
		{
			return _T("ERROR");
		}

		/*npos += 10;
		m_progress_debug_burn.SetPos(npos);*/
		switch (dev_num)
		{
			case 0:
				npos += 10;
				burn_plan.Format(_T("%d%c"), npos, '%');
				m_burn_plan_dev0.SetWindowTextW(burn_plan);
				m_burn_progress_dev0.SetPos(npos);
				break;
			case 1:
				npos += 10;
				burn_plan.Format(_T("%d%c"), npos, '%');
				m_burn_plan_dev1.SetWindowTextW(burn_plan);
				m_burn_progress_dev1.SetPos(npos);
				break;
			case 2:
				npos += 10;
				burn_plan.Format(_T("%d%c"), npos, '%');
				m_burn_plan_dev2.SetWindowTextW(burn_plan);
				m_burn_progress_dev2.SetPos(npos);
				break;
			case 3:
				npos += 10;
				burn_plan.Format(_T("%d%c"), npos, '%');
				m_burn_plan_dev3.SetWindowTextW(burn_plan);
				m_burn_progress_dev3.SetPos(npos);
				break;
			case 4:
				npos += 10;
				burn_plan.Format(_T("%d%c"), npos, '%');
				m_burn_plan_dev4.SetWindowTextW(burn_plan);
				m_burn_progress_dev4.SetPos(npos);
				break;
			case 5:
				npos += 10;
				burn_plan.Format(_T("%d%c"), npos, '%');
				m_burn_plan_dev5.SetWindowTextW(burn_plan);
				m_burn_progress_dev5.SetPos(npos);
				break;
		}

		if (b.backup_partition_flag == true)
		{
			cmd_boot.Empty();
			cmd_boot += _T("nand erase");
			cmd_boot += _T(" ");
			cmd_boot += b.PartitionOffset1;
			cmd_boot += _T(" ");
			cmd_boot += b.PartitionSize;

			CString2Char(cmd_boot, cmd_name);
			cmd_status = MyFunctionBurnDownloadCmd(dev_num, myfunc_work_sb, cmd_name, cmd_boot.GetLength());
			if (!cmd_status.Compare(_T("ERROR")))
			{
				return _T("ERROR");
			}

			cmd_boot.Empty();
			cmd_boot += _T("nand write.align");
			cmd_boot += _T(" ");
			cmd_boot += b.DownloadDdrAddress;
			cmd_boot += _T(" ");
			cmd_boot += b.PartitionOffset1;
			cmd_boot += _T(" ");
			cmd_boot += b.Download_Size;

			CString2Char(cmd_boot, cmd_name);
			cmd_status = MyFunctionBurnDownloadCmd(dev_num, myfunc_work_sb, cmd_name, cmd_boot.GetLength());
			if (!cmd_status.Compare(_T("ERROR")))
			{
				return _T("ERROR");
			}

			/*npos += 10;
			m_progress_debug_burn.SetPos(npos);*/
			switch (dev_num)
			{
			case 0:
				npos += 10;
				burn_plan.Format(_T("%d%c"), npos, '%');
				m_burn_plan_dev0.SetWindowTextW(burn_plan);
				m_burn_progress_dev0.SetPos(npos);
				break;
			case 1:
				npos += 10;
				burn_plan.Format(_T("%d%c"), npos, '%');
				m_burn_plan_dev1.SetWindowTextW(burn_plan);
				m_burn_progress_dev1.SetPos(npos);
				break;
			case 2:
				npos += 10;
				burn_plan.Format(_T("%d%c"), npos, '%');
				m_burn_plan_dev2.SetWindowTextW(burn_plan);
				m_burn_progress_dev2.SetPos(npos);
				break;
			case 3:
				npos += 10;
				burn_plan.Format(_T("%d%c"), npos, '%');
				m_burn_plan_dev3.SetWindowTextW(burn_plan);
				m_burn_progress_dev3.SetPos(npos);
				break;
			case 4:
				npos += 10;
				burn_plan.Format(_T("%d%c"), npos, '%');
				m_burn_plan_dev4.SetWindowTextW(burn_plan);
				m_burn_progress_dev4.SetPos(npos);
				break;
			case 5:
				npos += 10;
				burn_plan.Format(_T("%d%c"), npos, '%');
				m_burn_plan_dev5.SetWindowTextW(burn_plan);
				m_burn_progress_dev5.SetPos(npos);
				break;
			}
		}
	}
	else if (!b.BurnImageCmd.Compare(_T("nand write.vburn")))
	{
		//nand write.vburn 0x90000000 0x2000000 Size
		cmd_boot.Empty();
		cmd_boot += _T("nand erase");
		cmd_boot += _T(" ");
		cmd_boot += b.PartitionOffset;
		cmd_boot += _T(" ");
		cmd_boot += b.PartitionSize;

		CString2Char(cmd_boot, cmd_name);
		cmd_status = MyFunctionBurnDownloadCmd(dev_num, myfunc_work_sb, cmd_name, cmd_boot.GetLength());
		if (!cmd_status.Compare(_T("ERROR")))
		{
			return _T("ERROR");
		}

		cmd_boot.Empty();
		cmd_boot += _T("nand write.vburn");
		cmd_boot += _T(" ");
		cmd_boot += b.DownloadDdrAddress;
		cmd_boot += _T(" ");
		cmd_boot += b.PartitionOffset;
		cmd_boot += _T(" ");
		cmd_boot += b.Download_Size;

		CString2Char(cmd_boot, cmd_name);
		cmd_status = MyFunctionBurnDownloadCmd(dev_num, myfunc_work_sb, cmd_name, cmd_boot.GetLength());
		if (!cmd_status.Compare(_T("ERROR")))
		{
			return _T("ERROR");
		}

		/*npos += 25;
		m_progress_debug_burn.SetPos(npos);*/
		switch (dev_num)
		{
			case 0:
				npos += 25;
				burn_plan.Format(_T("%d%c"), npos, '%');
				m_burn_plan_dev0.SetWindowTextW(burn_plan);
				m_burn_progress_dev0.SetPos(npos);
				break;
			case 1:
				npos += 25;
				burn_plan.Format(_T("%d%c"), npos, '%');
				m_burn_plan_dev1.SetWindowTextW(burn_plan);
				m_burn_progress_dev1.SetPos(npos);
				break;
			case 2:
				npos += 25;
				burn_plan.Format(_T("%d%c"), npos, '%');
				m_burn_plan_dev2.SetWindowTextW(burn_plan);
				m_burn_progress_dev2.SetPos(npos);
				break;
			case 3:
				npos += 25;
				burn_plan.Format(_T("%d%c"), npos, '%');
				m_burn_plan_dev3.SetWindowTextW(burn_plan);
				m_burn_progress_dev3.SetPos(npos);
				break;
			case 4:
				npos += 25;
				burn_plan.Format(_T("%d%c"), npos, '%');
				m_burn_plan_dev4.SetWindowTextW(burn_plan);
				m_burn_progress_dev4.SetPos(npos);
				break;
			case 5:
				npos += 25;
				burn_plan.Format(_T("%d%c"), npos, '%');
				m_burn_plan_dev5.SetWindowTextW(burn_plan);
				m_burn_progress_dev5.SetPos(npos);
				break;
		}

	}
	else
	{
		cmd_boot.Empty();
		cmd_boot += _T("nand info");

		CString2Char(cmd_boot, cmd_name);
		cmd_status = MyFunctionBurnDownloadCmd(dev_num, myfunc_work_sb, cmd_name, cmd_boot.GetLength());
		if (!cmd_status.Compare(_T("ERROR")))
		{
			return _T("ERROR");
		}
	}

	return _T("OK");

}

void CBURN::MyFunctionBurnCheckDeleteButton()
{
	CString fial_num;
	CString success_num;
	CString tatol_num;

	GetDlgItemText(IDC_EDIT_FAIL, fial_num);
	GetDlgItemText(IDC_EDIT_SUCCESS, success_num);
	GetDlgItemText(IDC_EDIT_TOTAL, tatol_num);

	int x = _tcstoul(success_num, NULL, 10);
	int y = _tcstoul(fial_num, NULL, 10);
	int z = _tcstoul(tatol_num, NULL, 10);

	if (x + y == z)
	{
		m_button_delete_device.EnableWindow(true);
	}
}

UINT CBURN::MyFunctionBurnSpiNandImage(int dev_num, struct _libusb_work_sb* myfunc_work_sb, struct _libusb_burn* myfunc_burn_sb)
{
	CString fial_num;
	int burn_fail_num;
	CString success_num;
	int burn_success_num;
	CString burn_plan;
	CString bus_hub_num;
		
	CString  exe_dir;
	CString str_boot_dir, str_boot_name;
	char char_boot_name[MAX_PATH] = { 0 };

	exe_dir = MyFunctionBurnGetExePath();

	str_boot_dir = exe_dir;
	str_boot_dir += LIBUSB_IMAGE_PATH;

	int imagecount = GetPrivateProfileInt(TEXT("ImageCount"), TEXT("numOfImage"), 0, exe_dir + TEXT(LIBUSB_SPINAND_INI));
	if (imagecount == 0)
	{
		//MessageBox(TEXT("无镜像文件"), TEXT("错误"), MB_OK | MB_ICONWARNING);
		global_print.Lock();
		MyFunctionPrintDebug(dev_num, _T("No image file."));
		global_print.Unlock();
		//UpdateData(false);
		return -1;
	}
	CString downloadimage;
	CString GetTmp;
	int i;
	int npos;

	switch (dev_num)
	{
		case 0:
			npos = m_burn_progress_dev0.GetPos();
			break;
		case 1:
			npos = m_burn_progress_dev1.GetPos();
			break;
		case 2:
			npos = m_burn_progress_dev2.GetPos();
			break;
		case 3:
			npos = m_burn_progress_dev3.GetPos();
			break;
		case 4:
			npos = m_burn_progress_dev4.GetPos();
			break;
		case 5:
			npos = m_burn_progress_dev5.GetPos();
			break;
	}

	// 一 : 从init文件获取参数
	for (i = 0; i < imagecount; i++)
	{
		downloadimage.Format(_T("DOWNLOADIMAGE%d"), i);

		// 1 : 获取地址参数
		GetPrivateProfileString(downloadimage, TEXT("DownloadDdrAddress"), TEXT(LIBUSB_DOWN_ADDRESS),
			GetTmp.GetBuffer(MAX_PATH), MAX_PATH,
			exe_dir + TEXT(LIBUSB_SPINAND_INI));
		CString2Char(GetTmp, char_boot_name);
		myfunc_burn_sb[i].DownloadDdrAddress = MyFunctionBurnUTF82WCS(char_boot_name);

		//AfxMessageBox(myfunc_burn_sb[i].DownloadDdrAddress);

		// 2 : 获取是否烧录参数
		myfunc_burn_sb[i].NeedBurn = GetPrivateProfileInt(downloadimage, TEXT("NeedBurn"), 0,
			exe_dir + TEXT(LIBUSB_SPINAND_INI));

		// 3 : 获取烧录镜像命令
		/*myfunc_burn_sb[i].ImageFlag = GetPrivateProfileInt(downloadimage, TEXT("ImageFlag"), 0,
			exe_dir + TEXT(LIBUSB_SPINAND_INI));*/
		GetPrivateProfileString(downloadimage, TEXT("BurnImageCmd"), TEXT("NoBurnImageCmd"),
			GetTmp.GetBuffer(MAX_PATH), MAX_PATH,
			exe_dir + TEXT(LIBUSB_SPINAND_INI));

		CString2Char(GetTmp, char_boot_name);
		myfunc_burn_sb[i].BurnImageCmd = MyFunctionBurnUTF82WCS(char_boot_name);

		//AfxMessageBox(myfunc_burn_sb[i].BurnImageCmd);

		// 4 : 获取镜像路径
		GetPrivateProfileString(downloadimage, TEXT("DownloadImageName"), TEXT("NoDownloadImageName"),
			GetTmp.GetBuffer(MAX_PATH), MAX_PATH,
			exe_dir + TEXT(LIBUSB_SPINAND_INI));

		CString2Char(GetTmp, char_boot_name);
		str_boot_name = MyFunctionBurnUTF82WCS(char_boot_name);

		myfunc_burn_sb[i].DownloadImagePath.Empty();
		myfunc_burn_sb[i].DownloadImagePath += str_boot_dir;
		myfunc_burn_sb[i].DownloadImagePath += char_boot_name;

		//AfxMessageBox(myfunc_burn_sb[i].DownloadImagePath);

		// 5 : 内核镜像有个备份分区
		if (strncmp(LIBUSB_UKERNEL, (char*)char_boot_name, strlen(LIBUSB_UKERNEL)) == 0)
		{
			GetPrivateProfileString(downloadimage, TEXT("PartitionOffset1"), TEXT("NoPartitionOffset1"),
				GetTmp.GetBuffer(MAX_PATH), MAX_PATH,
				exe_dir + TEXT(LIBUSB_SPINAND_INI));
			myfunc_burn_sb[i].backup_partition_flag = true;

			CString2Char(GetTmp, char_boot_name);
			myfunc_burn_sb[i].PartitionOffset1 = MyFunctionBurnUTF82WCS(char_boot_name);

			//AfxMessageBox(myfunc_burn_sb[i].PartitionOffset1);
		}

		// 6 : 获取分区大小
		GetPrivateProfileString(downloadimage, TEXT("PartitionSize"), TEXT("NoPartitionSize"),
			GetTmp.GetBuffer(MAX_PATH), MAX_PATH,
			exe_dir + TEXT(LIBUSB_SPINAND_INI));

		CString2Char(GetTmp, char_boot_name);
		myfunc_burn_sb[i].PartitionSize = MyFunctionBurnUTF82WCS(char_boot_name);

		//AfxMessageBox(myfunc_burn_sb[i].PartitionSize);

		// 7 : 获取分区偏移
		GetPrivateProfileString(downloadimage, TEXT("PartitionOffset"), TEXT("NoPartitionOffset"),
			GetTmp.GetBuffer(MAX_PATH), MAX_PATH,
			exe_dir + TEXT(LIBUSB_SPINAND_INI));

		CString2Char(GetTmp, char_boot_name);
		myfunc_burn_sb[i].PartitionOffset = MyFunctionBurnUTF82WCS(char_boot_name);

		//AfxMessageBox(myfunc_burn_sb[i].PartitionOffset);

		/*npos += 5;
		m_progress_debug_burn.SetPos(npos);*/
		switch (dev_num)
		{
			case 0:
				npos += 5;
				burn_plan.Format(_T("%d%c"), npos, '%');
				m_burn_plan_dev0.SetWindowTextW(burn_plan);
				m_burn_progress_dev0.SetPos(npos);
				break;
			case 1:
				npos += 5;
				burn_plan.Format(_T("%d%c"), npos, '%');
				m_burn_plan_dev1.SetWindowTextW(burn_plan);
				m_burn_progress_dev1.SetPos(npos);
				break;
			case 2:
				npos += 5;
				burn_plan.Format(_T("%d%c"), npos, '%');
				m_burn_plan_dev2.SetWindowTextW(burn_plan);
				m_burn_progress_dev2.SetPos(npos);
				break;
			case 3:
				npos += 5;
				burn_plan.Format(_T("%d%c"), npos, '%');
				m_burn_plan_dev3.SetWindowTextW(burn_plan);
				m_burn_progress_dev3.SetPos(npos);
				break;
			case 4:
				npos += 5;
				burn_plan.Format(_T("%d%c"), npos, '%');
				m_burn_plan_dev4.SetWindowTextW(burn_plan);
				m_burn_progress_dev4.SetPos(npos);
				break;
			case 5:
				npos += 5;
				burn_plan.Format(_T("%d%c"), npos, '%');
				m_burn_plan_dev5.SetWindowTextW(burn_plan);
				m_burn_progress_dev5.SetPos(npos);
				break;
		}
	}

	// 二 : 执行烧录
	CString cmd_status;
	for (i = 0; i < imagecount; i++)
	{
		if (myfunc_burn_sb[i].NeedBurn)
		{
			myfunc_burn_sb[i].Download_Size = MyFunctionBurnDownloadImage(dev_num, myfunc_work_sb,
				myfunc_burn_sb[i].DownloadImagePath,
				myfunc_burn_sb[i].DownloadDdrAddress);
			if (!myfunc_burn_sb[i].Download_Size.Compare(_T("ERROR")))
			{
				//MessageBox(TEXT("烧录失败（Download File）"), TEXT("提示"), MB_OK | MB_ICONWARNING);
				//m_burn_status.SetWindowTextW(_T("烧录失败"));

				global_Mutex.Lock();

				GetDlgItemText(IDC_EDIT_FAIL, fial_num);
				burn_fail_num = _tcstoul(fial_num, NULL, 10);
				burn_fail_num += 1;
				fial_num.Format(_T("%d"), burn_fail_num);
				SetDlgItemText(IDC_EDIT_FAIL, fial_num);

				global_Mutex.Unlock();

				global_print.Lock();
				MyFunctionPrintDebug(dev_num, _T("Download File Faild."));
				global_print.Unlock();

				//检查删除设备按钮
				MyFunctionBurnCheckDeleteButton();

				switch (dev_num)
				{
					case 0:
						m_pic_burn_dev0.SetBitmap((HBITMAP)bitmap_dev.bitmap_burn_fail);
						break;
					case 1:
						m_pic_burn_dev1.SetBitmap((HBITMAP)bitmap_dev.bitmap_burn_fail);
						break;
					case 2:
						m_pic_burn_dev2.SetBitmap((HBITMAP)bitmap_dev.bitmap_burn_fail);
						break;
					case 3:
						m_pic_burn_dev3.SetBitmap((HBITMAP)bitmap_dev.bitmap_burn_fail);
						break;
					case 4:
						m_pic_burn_dev4.SetBitmap((HBITMAP)bitmap_dev.bitmap_burn_fail);
						break;
					case 5:
						m_pic_burn_dev5.SetBitmap((HBITMAP)bitmap_dev.bitmap_burn_fail);
						break;
				}

				return -1;
			}

			cmd_status = MyFunctionBurnProCmd(dev_num, myfunc_work_sb, myfunc_burn_sb[i]);
			if (!cmd_status.Compare(_T("ERROR")))
			{
				//MessageBox(TEXT("烧录失败（Download Cmd）"), TEXT("提示"), MB_OK | MB_ICONWARNING);
				//m_burn_status.SetWindowTextW(_T("烧录失败"));

				global_Mutex.Lock();

				GetDlgItemText(IDC_EDIT_FAIL, fial_num);
				burn_fail_num = _tcstoul(fial_num, NULL, 10);
				burn_fail_num += 1;
				fial_num.Format(_T("%d"), burn_fail_num);
				SetDlgItemText(IDC_EDIT_FAIL, fial_num);

				global_Mutex.Unlock();

				global_print.Lock();
				MyFunctionPrintDebug(dev_num, _T("Download Cmd Faild."));
				global_print.Unlock();

				//检查删除设备按钮
				MyFunctionBurnCheckDeleteButton();

				//m_pic_burn_dev0.SetBitmap((HBITMAP)bitmap_dev.bitmap_burn_fail);
				switch (dev_num)
				{
					case 0:
						m_pic_burn_dev0.SetBitmap((HBITMAP)bitmap_dev.bitmap_burn_fail);
						break;
					case 1:
						m_pic_burn_dev1.SetBitmap((HBITMAP)bitmap_dev.bitmap_burn_fail);
						break;
					case 2:
						m_pic_burn_dev2.SetBitmap((HBITMAP)bitmap_dev.bitmap_burn_fail);
						break;
					case 3:
						m_pic_burn_dev3.SetBitmap((HBITMAP)bitmap_dev.bitmap_burn_fail);
						break;
					case 4:
						m_pic_burn_dev4.SetBitmap((HBITMAP)bitmap_dev.bitmap_burn_fail);
						break;
					case 5:
						m_pic_burn_dev5.SetBitmap((HBITMAP)bitmap_dev.bitmap_burn_fail);
						break;
				}

				return -1;
			}
		}
		else
		{
			//MessageBox(TEXT("没有可烧写的镜像"), TEXT("温馨提示"), MB_OK | MB_ICONWARNING);
		}
	}

	if (i == imagecount)
	{
		//m_progress_debug_burn.SetPos(100);
		//m_burn_status.SetWindowTextW(_T("烧录成功"));

		switch (dev_num)
		{
			case 0:
				m_pic_burn_dev0.SetBitmap((HBITMAP)bitmap_dev.bitmap_burn_success);
				m_burn_plan_dev0.SetWindowTextW(_T("100%"));
				m_burn_progress_dev0.SetPos(100);
				break;
			case 1:
				m_pic_burn_dev1.SetBitmap((HBITMAP)bitmap_dev.bitmap_burn_success);
				m_burn_plan_dev1.SetWindowTextW(_T("100%"));
				m_burn_progress_dev1.SetPos(100);
				break;
			case 2:
				m_pic_burn_dev2.SetBitmap((HBITMAP)bitmap_dev.bitmap_burn_success);
				m_burn_plan_dev2.SetWindowTextW(_T("100%"));
				m_burn_progress_dev2.SetPos(100);
				break;
			case 3:
				m_pic_burn_dev3.SetBitmap((HBITMAP)bitmap_dev.bitmap_burn_success);
				m_burn_plan_dev3.SetWindowTextW(_T("100%"));
				m_burn_progress_dev3.SetPos(100);
				break;
			case 4:
				m_pic_burn_dev4.SetBitmap((HBITMAP)bitmap_dev.bitmap_burn_success);
				m_burn_plan_dev4.SetWindowTextW(_T("100%"));
				m_burn_progress_dev4.SetPos(100);
				break;
			case 5:
				m_pic_burn_dev5.SetBitmap((HBITMAP)bitmap_dev.bitmap_burn_success);
				m_burn_plan_dev5.SetWindowTextW(_T("100%"));
				m_burn_progress_dev5.SetPos(100);
				break;
		}

		global_Mutex.Lock();

		GetDlgItemText(IDC_EDIT_SUCCESS, success_num);
		burn_success_num = _tcstoul(success_num, NULL, 10);
		burn_success_num += 1;
		success_num.Format(_T("%d"), burn_success_num);
		SetDlgItemText(IDC_EDIT_SUCCESS, success_num);

		global_Mutex.Unlock();

		global_print.Lock();
		MyFunctionPrintDebug(dev_num, _T("Burn complete."));
		global_print.Unlock();

		//检查删除设备按钮
		MyFunctionBurnCheckDeleteButton();

		/*UINT i = MessageBoxTimeout(NULL, _T("5s 将自动重启，是否重启？"), _T("温馨提示"),
			MB_YESNO | MB_ICONINFORMATION,
			GetSystemDefaultLangID(),
			5000);
		if (i == IDNO)
		{
			goto NandBurnImageEnd;
		}*/

		if (m_need_reset.GetCheck() == 1)
		{	
			global_print.Lock();
			MyFunctionPrintDebug(dev_num, _T("reset device."));
			global_print.Unlock();

			MyFunctionBurnDownloadCmd(dev_num, myfunc_work_sb, "reset", strlen("reset"));
		}

	}

	return 0;
}


void CBURN::OnBnClickedButtonAccessDevice()
{
	// TODO: 在此添加控件通知处理程序代码

	int ret = 0;
	int dev_num = 0;
	int i = 0, j;
	int ep_cnt;
	CString str_vid(_T("0x0AC8"));
	CString str_pid(_T("0x7680"));

	//GetDlgItem(IDC_EDIT_DEBUG_VID)->GetWindowText(str_vid);
	//GetDlgItem(IDC_EDIT_DEBUG_PID)->GetWindowText(str_pid);
	//GetDlgItemText(IDC_EDIT_DEBUG_VID, str_vid);
	//AfxMessageBox(str_vid);

	ret = libusb_init(NULL);
	if (ret < 0)
	{
		MessageBox(TEXT("libusb_init failed!"), TEXT("错误"), MB_OK | MB_ICONWARNING);
		return;
	}

	// get usb device list
	ret = libusb_get_device_list(NULL, &my_access_list);
	if (ret < 0)
	{
		MessageBox(TEXT("libusb_get_device_list failed!"), TEXT("错误"), MB_OK | MB_ICONWARNING);

		goto get_failed;
	}

	/* check the matched device */
	while ((my_usbdev = my_access_list[i++]) != NULL)
	{

		libusb_get_device_descriptor(my_usbdev, &my_dev_desc);

#if 1
		if (my_dev_desc.idVendor == _tcstoul(str_vid, 0, 16) &&
			my_dev_desc.idProduct == _tcstoul(str_pid, 0, 16))
		{
			//CString kk;
			//kk.Format(_T("%d %#x:%#x"), i, my_dev_desc.idVendor, my_dev_desc.idProduct);
			//MessageBox(kk, TEXT("提示"), MB_OK | MB_ICONWARNING);

			/* open usb device */
			ret = libusb_open(my_usbdev, &libusb_work_sb_burn[dev_num].handle);
			if (ret < 0)
			{
				MessageBox(TEXT("libusb_open failed!"), TEXT("错误"), MB_OK | MB_ICONWARNING);
				//goto open_failed;
				continue;
			}

			CString str_bus_hub_num;
			str_bus_hub_num.Format(_T("USB%d:%d"), libusb_get_bus_number(my_usbdev), libusb_get_port_number(my_usbdev));

			/* get config descriptor */
			ret = libusb_get_config_descriptor(my_usbdev, 0, &libusb_work_sb_burn[dev_num].config_desc);
			if (ret < 0)
			{
				MessageBox(TEXT("get config descriptor failed!"), TEXT("错误"), MB_OK | MB_ICONWARNING);
				//goto configdesc_fail;
				dev_num--;
				libusb_close(libusb_work_sb_burn[dev_num].handle);
				
				continue;
			}

			ep_cnt = libusb_work_sb_burn[dev_num].config_desc->interface->altsetting[0].bNumEndpoints;

			/* get bulk in/out ep */
			for (j = 0; j < ep_cnt; j++)
			{
				libusb_work_sb_burn[dev_num].ep_desc = 
					&libusb_work_sb_burn[dev_num].config_desc->interface->altsetting[0].endpoint[j];
				if ((libusb_work_sb_burn[dev_num].ep_desc->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) & LIBUSB_TRANSFER_TYPE_BULK)
				{
					if ((libusb_work_sb_burn[dev_num].ep_desc->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) & LIBUSB_ENDPOINT_IN)
					{
						
						libusb_work_sb_burn[dev_num].ep_bulkin = libusb_work_sb_burn[dev_num].ep_desc->bEndpointAddress;
					}
					else
					{
						libusb_work_sb_burn[dev_num].ep_bulkout = libusb_work_sb_burn[dev_num].ep_desc->bEndpointAddress;
					}
				}
			}

			/* claim the interface */
			libusb_detach_kernel_driver(libusb_work_sb_burn[dev_num].handle, 0);
			ret = libusb_claim_interface(libusb_work_sb_burn[dev_num].handle, 0);
			if (ret < 0)
			{
				MessageBox(TEXT("claim usb interface failed!"), TEXT("错误"), MB_OK | MB_ICONWARNING);
				//goto claim_failed;
				dev_num--;
				libusb_close(libusb_work_sb_burn[dev_num].handle);
				continue;
			}

			//MessageBoxTimeout(NULL, _T("设备绑定成功"), _T("提示"), MB_ICONINFORMATION, GetSystemDefaultLangID(), 1000);

			switch (dev_num)
			{
				case 0:
					m_pic_burn_dev0.SetBitmap((HBITMAP)bitmap_dev.bitmap_connect);
					m_burn_dev1.SetWindowTextW(str_bus_hub_num);
					break;
				case 1:
					m_pic_burn_dev1.SetBitmap((HBITMAP)bitmap_dev.bitmap_connect);
					m_burn_dev2.SetWindowTextW(str_bus_hub_num);
					break;
				case 2:
					m_pic_burn_dev2.SetBitmap((HBITMAP)bitmap_dev.bitmap_connect);
					m_burn_dev3.SetWindowTextW(str_bus_hub_num);
					break;
				case 3:
					m_pic_burn_dev3.SetBitmap((HBITMAP)bitmap_dev.bitmap_connect);
					m_burn_dev4.SetWindowTextW(str_bus_hub_num);
					break;
				case 4:
					m_pic_burn_dev4.SetBitmap((HBITMAP)bitmap_dev.bitmap_connect);
					m_burn_dev5.SetWindowTextW(str_bus_hub_num);
					break;
				case 5:
					m_pic_burn_dev5.SetBitmap((HBITMAP)bitmap_dev.bitmap_connect);
					m_burn_dev6.SetWindowTextW(str_bus_hub_num);
					break;
			}

			dev_num++;

			CString Total_num;
			Total_num.Format(_T("%d"), dev_num);

			SetDlgItemText(IDC_EDIT_TOTAL, Total_num);

			m_button_access_device.EnableWindow(false);
			m_button_start_burn.EnableWindow(true);
			m_button_delete_device.EnableWindow(true);
			
			if (dev_num == 6)
				break;
		}
#endif
		//MessageBox(TEXT("3!"), TEXT("错误"), MB_OK | MB_ICONWARNING);
	}

	return;

open_failed:

	libusb_free_device_list(my_access_list, 1);

get_failed:
	libusb_exit(NULL);

	return;
}



UINT ThreadSpiNandDevice0(LPVOID pParam)
{
	CBURN* pObj = (CBURN*)pParam;

	return pObj->MyFunctionBurnSpiNandImage(0, &libusb_work_sb_burn[0], libusb_burn[0]);
}

UINT ThreadSpiNandDevice1(LPVOID pParam)
{
	CBURN* pObj = (CBURN*)pParam;

	return pObj->MyFunctionBurnSpiNandImage(1, &libusb_work_sb_burn[1], libusb_burn[1]);
}

UINT ThreadSpiNandDevice2(LPVOID pParam)
{
	CBURN* pObj = (CBURN*)pParam;

	return pObj->MyFunctionBurnSpiNandImage(2, &libusb_work_sb_burn[2], libusb_burn[2]);
}

UINT ThreadSpiNandDevice3(LPVOID pParam)
{
	CBURN* pObj = (CBURN*)pParam;

	return pObj->MyFunctionBurnSpiNandImage(3, &libusb_work_sb_burn[3], libusb_burn[3]);
}

UINT ThreadSpiNandDevice4(LPVOID pParam)
{
	CBURN* pObj = (CBURN*)pParam;

	return pObj->MyFunctionBurnSpiNandImage(4, &libusb_work_sb_burn[4], libusb_burn[4]);
}

UINT ThreadSpiNandDevice5(LPVOID pParam)
{
	CBURN* pObj = (CBURN*)pParam;

	return pObj->MyFunctionBurnSpiNandImage(5, &libusb_work_sb_burn[5], libusb_burn[5]);
}

void CBURN::InitBurnProgress()
{
	m_burn_progress_dev0.SetRange(0, 100);
	m_burn_progress_dev0.SetPos(0);
	m_burn_progress_dev1.SetRange(0, 100);
	m_burn_progress_dev1.SetPos(0);
	m_burn_progress_dev2.SetRange(0, 100);
	m_burn_progress_dev2.SetPos(0);
	m_burn_progress_dev3.SetRange(0, 100);
	m_burn_progress_dev3.SetPos(0);
	m_burn_progress_dev4.SetRange(0, 100);
	m_burn_progress_dev4.SetPos(0);
	m_burn_progress_dev5.SetRange(0, 100);
	m_burn_progress_dev5.SetPos(0);

	m_burn_plan_dev0.SetWindowTextW(_T("0%"));
	m_burn_plan_dev1.SetWindowTextW(_T("0%"));
	m_burn_plan_dev2.SetWindowTextW(_T("0%"));
	m_burn_plan_dev3.SetWindowTextW(_T("0%"));
	m_burn_plan_dev4.SetWindowTextW(_T("0%"));
	m_burn_plan_dev5.SetWindowTextW(_T("0%"));

	SetDlgItemText(IDC_EDIT_DEBUG_PRINT, _T(""));
}

void CBURN::OnBnClickedButtonStartBurn()
{
	// TODO: 在此添加控件通知处理程序代码
	CWinThread* m_pThread_dev_0 = nullptr;
	CWinThread* m_pThread_dev_1 = nullptr;
	CWinThread* m_pThread_dev_2 = nullptr;
	CWinThread* m_pThread_dev_3 = nullptr;
	CWinThread* m_pThread_dev_4 = nullptr;
	CWinThread* m_pThread_dev_5 = nullptr;

	CString GetMediaType;
	CString tatol_dev;
	CString Handshake_status;
	int tatol_dev_num, dev_i;

	CString fial_num;
	int burn_fail_num;

	//m_button_access_device.EnableWindow(false);
	m_button_delete_device.EnableWindow(false);

	SetDlgItemText(IDC_EDIT_SUCCESS, _T("0"));
	SetDlgItemText(IDC_EDIT_FAIL, _T("0"));

	InitBurnProgress();

	GetDlgItemText(IDC_EDIT_TOTAL, tatol_dev);
	tatol_dev_num = _tcstoul(tatol_dev, NULL, 10);
	if (tatol_dev_num == 0)
	{
		MessageBox(_T("无烧录设备，请先获取设备！！！"), TEXT("温馨提示"), MB_OK | MB_ICONWARNING);
		return;
	}
	
	CString exe_dir = MyFunctionBurnGetExePath();

	GetPrivateProfileString(TEXT("SystemConf"), TEXT("mediaType"), TEXT(" "),
		GetMediaType.GetBuffer(MAX_PATH), MAX_PATH,
		exe_dir + TEXT(LIBUSB_VMCTOOLS_INI));

	if (!GetMediaType.Compare(_T(LIBUSB_MEDIA_SPINAND)))
	{
		for (dev_i = 0; dev_i < tatol_dev_num; dev_i++)
		{
			global_print.Lock();
			MyFunctionPrintDebug(dev_i, _T("Ready Handshake..."));
			global_print.Unlock();

			switch (dev_i)
			{
				case 0:
					Handshake_status = MyFunctionBurnHandshake(&libusb_work_sb_burn[0]);
					if (!Handshake_status.Compare(_T("ERROR")))
					{
						global_print.Lock();
						MyFunctionPrintDebug(dev_i, _T("Handshake failure, out of the burn."));
						global_print.Unlock();

						GetDlgItemText(IDC_EDIT_FAIL, fial_num);
						burn_fail_num = _tcstoul(fial_num, NULL, 10);
						burn_fail_num += 1;
						fial_num.Format(_T("%d"), burn_fail_num);
						SetDlgItemText(IDC_EDIT_FAIL, fial_num);

						MyFunctionBurnCheckDeleteButton();

						m_pic_burn_dev0.SetBitmap((HBITMAP)bitmap_dev.bitmap_burn_fail);
						break;
					}

					global_print.Lock();
					MyFunctionPrintDebug(dev_i, _T("(OK)"));
					global_print.Unlock();

					m_pic_burn_dev0.SetBitmap((HBITMAP)bitmap_dev.bitmap_burning);
					m_pThread_dev_0 = AfxBeginThread(ThreadSpiNandDevice0, this);
					break;
				case 1:
					Handshake_status = MyFunctionBurnHandshake(&libusb_work_sb_burn[1]);
					if (!Handshake_status.Compare(_T("ERROR")))
					{
						global_print.Lock();
						MyFunctionPrintDebug(dev_i, _T("Handshake failure, out of the burn."));
						global_print.Unlock();

						GetDlgItemText(IDC_EDIT_FAIL, fial_num);
						burn_fail_num = _tcstoul(fial_num, NULL, 10);
						burn_fail_num += 1;
						fial_num.Format(_T("%d"), burn_fail_num);
						SetDlgItemText(IDC_EDIT_FAIL, fial_num);
						MyFunctionBurnCheckDeleteButton();

						m_pic_burn_dev1.SetBitmap((HBITMAP)bitmap_dev.bitmap_burn_fail);
						break;
					}

					global_print.Lock();
					MyFunctionPrintDebug(dev_i, _T("(OK)"));
					global_print.Unlock();

					m_pic_burn_dev1.SetBitmap((HBITMAP)bitmap_dev.bitmap_burning);
					m_pThread_dev_1 = AfxBeginThread(ThreadSpiNandDevice1, this);
					break;
				case 2:
					Handshake_status = MyFunctionBurnHandshake(&libusb_work_sb_burn[2]);
					if (!Handshake_status.Compare(_T("ERROR")))
					{
						global_print.Lock();
						MyFunctionPrintDebug(dev_i, _T("Handshake failure, out of the burn."));
						global_print.Unlock();

						GetDlgItemText(IDC_EDIT_FAIL, fial_num);
						burn_fail_num = _tcstoul(fial_num, NULL, 10);
						burn_fail_num += 1;
						fial_num.Format(_T("%d"), burn_fail_num);
						SetDlgItemText(IDC_EDIT_FAIL, fial_num);
						MyFunctionBurnCheckDeleteButton();

						m_pic_burn_dev2.SetBitmap((HBITMAP)bitmap_dev.bitmap_burn_fail);
						break;
					}

					global_print.Lock();
					MyFunctionPrintDebug(dev_i, _T("(OK)"));
					global_print.Unlock();

					m_pic_burn_dev2.SetBitmap((HBITMAP)bitmap_dev.bitmap_burning);
					m_pThread_dev_2 = AfxBeginThread(ThreadSpiNandDevice2, this);
					break;
				case 3:
					Handshake_status = MyFunctionBurnHandshake(&libusb_work_sb_burn[3]);
					if (!Handshake_status.Compare(_T("ERROR")))
					{
						global_print.Lock();
						MyFunctionPrintDebug(dev_i, _T("Handshake failure, out of the burn."));
						global_print.Unlock();

						GetDlgItemText(IDC_EDIT_FAIL, fial_num);
						burn_fail_num = _tcstoul(fial_num, NULL, 10);
						burn_fail_num += 1;
						fial_num.Format(_T("%d"), burn_fail_num);
						SetDlgItemText(IDC_EDIT_FAIL, fial_num);
						MyFunctionBurnCheckDeleteButton();

						m_pic_burn_dev3.SetBitmap((HBITMAP)bitmap_dev.bitmap_burn_fail);
						break;
					}

					global_print.Lock();
					MyFunctionPrintDebug(dev_i, _T("(OK)"));
					global_print.Unlock();

					m_pic_burn_dev3.SetBitmap((HBITMAP)bitmap_dev.bitmap_burning);
					m_pThread_dev_3 = AfxBeginThread(ThreadSpiNandDevice3, this);
					break;
				case 4:
					Handshake_status = MyFunctionBurnHandshake(&libusb_work_sb_burn[4]);
					if (!Handshake_status.Compare(_T("ERROR")))
					{
						global_print.Lock();
						MyFunctionPrintDebug(dev_i, _T("Handshake failure, out of the burn."));
						global_print.Unlock();

						GetDlgItemText(IDC_EDIT_FAIL, fial_num);
						burn_fail_num = _tcstoul(fial_num, NULL, 10);
						burn_fail_num += 1;
						fial_num.Format(_T("%d"), burn_fail_num);
						SetDlgItemText(IDC_EDIT_FAIL, fial_num);
						MyFunctionBurnCheckDeleteButton();

						m_pic_burn_dev4.SetBitmap((HBITMAP)bitmap_dev.bitmap_burn_fail);
						break;
					}

					global_print.Lock();
					MyFunctionPrintDebug(dev_i, _T("(OK)"));
					global_print.Unlock();

					m_pic_burn_dev4.SetBitmap((HBITMAP)bitmap_dev.bitmap_burning);
					m_pThread_dev_4 = AfxBeginThread(ThreadSpiNandDevice4, this);
					break;
				case 5:
					Handshake_status = MyFunctionBurnHandshake(&libusb_work_sb_burn[5]);
					if (!Handshake_status.Compare(_T("ERROR")))
					{
						global_print.Lock();
						MyFunctionPrintDebug(dev_i, _T("Handshake failure, out of the burn."));
						global_print.Unlock();

						GetDlgItemText(IDC_EDIT_FAIL, fial_num);
						burn_fail_num = _tcstoul(fial_num, NULL, 10);
						burn_fail_num += 1;
						fial_num.Format(_T("%d"), burn_fail_num);
						SetDlgItemText(IDC_EDIT_FAIL, fial_num);
						MyFunctionBurnCheckDeleteButton();

						m_pic_burn_dev5.SetBitmap((HBITMAP)bitmap_dev.bitmap_burn_fail);
						break;
					}

					global_print.Lock();
					MyFunctionPrintDebug(dev_i, _T("(OK)"));
					global_print.Unlock();

					m_pic_burn_dev5.SetBitmap((HBITMAP)bitmap_dev.bitmap_burning);
					m_pThread_dev_5 = AfxBeginThread(ThreadSpiNandDevice5, this);
					break;
			}
		}
	}
	else if (!GetMediaType.Compare(_T(LIBUSB_MEDIA_EMMC)))
	{

		MessageBox(TEXT("emmc烧录，暂未支持！！！"), TEXT("温馨提示"), MB_OK | MB_ICONWARNING);
		//goto error_end;
	}
	else
	{
		MessageBox(TEXT("媒体介质选择有误，请选一种支持的烧录介质！！！"), TEXT("温馨提示"), MB_OK | MB_ICONWARNING);
		//goto error_end;
	}

}





void CBURN::OnBnClickedButtonDeleteDevice()
{
	// TODO: 在此添加控件通知处理程序代码

	CString tatol_dev;
	int tatol_dev_num, dev_i;

	m_button_access_device.EnableWindow(true);
	m_button_start_burn.EnableWindow(false);
	m_button_delete_device.EnableWindow(false);

	GetDlgItemText(IDC_EDIT_TOTAL, tatol_dev);
	tatol_dev_num = _tcstoul(tatol_dev, NULL, 10);
	if (tatol_dev_num == 0)
	{
		return;
	}

	InitBurnProgress();

	CString ll;


	for (dev_i = 0; dev_i < tatol_dev_num; dev_i++)
	{
		libusb_close(libusb_work_sb_burn[dev_i].handle);
		switch (dev_i)
		{
			case 0:
				m_pic_burn_dev0.SetBitmap(NULL);
				m_burn_dev1.SetWindowTextW(_T("Unused"));
				break;
			case 1:
				m_pic_burn_dev1.SetBitmap(NULL);
				m_burn_dev2.SetWindowTextW(_T("Unused"));
				break;
			case 2:
				m_pic_burn_dev2.SetBitmap(NULL);
				m_burn_dev3.SetWindowTextW(_T("Unused"));
				break;
			case 3:
				m_pic_burn_dev3.SetBitmap(NULL);
				m_burn_dev4.SetWindowTextW(_T("Unused"));
				break;
			case 4:
				m_pic_burn_dev4.SetBitmap(NULL);
				m_burn_dev5.SetWindowTextW(_T("Unused"));
				break;
			case 5:
				m_pic_burn_dev5.SetBitmap(NULL);
				m_burn_dev6.SetWindowTextW(_T("Unused"));
				break;
		}
	}

	libusb_free_device_list(my_access_list, 1);

	SetDlgItemText(IDC_EDIT_TOTAL, _T("0"));
	SetDlgItemText(IDC_EDIT_SUCCESS, _T("0"));
	SetDlgItemText(IDC_EDIT_FAIL, _T("0"));

	//libusb_exit(NULL);
}

