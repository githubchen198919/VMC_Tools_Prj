// CDEBUGDlg.cpp: 实现文件
//

#include "pch.h"
#include "VMCTools.h"
#include "CDEBUGDlg.h"
#include "afxdialogex.h"
#include "my_libusb.h"

#pragma comment(lib,"libusb-1.0.lib")  

static struct _libusb_work_sb libusb_work_sb_debug;
static struct _libusb_burn libusb_burn[10];

// CDEBUGDlg 对话框

IMPLEMENT_DYNAMIC(CDEBUGDlg, CDialogEx)

CDEBUGDlg::CDEBUGDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DIALOG_DEBUG, pParent)
{

}

CDEBUGDlg::~CDEBUGDlg()
{
}

void CDEBUGDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_BUTTON_DEBUG_CONNECT, m_button_debug_connect);
	DDX_Control(pDX, IDC_BUTTON_DEBUG_DISCONNECT, m_button_debug_disconnect);
	DDX_Control(pDX, IDC_EDIT_DEBUG_VID, m_edit_debug_vid);
	DDX_Control(pDX, IDC_EDIT_DEBUG_PID, m_edit_debug_pid);
	DDX_Control(pDX, IDC_BUTTON_DEBUG_DOWN_CMD, m_button_debug_cmd);
	DDX_Control(pDX, IDC_BUTTON_DEBUG_DOWN, m_button_debug_file);
	DDX_Control(pDX, IDC_BUTTON_START, m_button_debug_start);
	DDX_Control(pDX, IDC_COMBO1, m_combobox_debug_cmd);
	DDX_Control(pDX, IDC_BUTTON_KEY_BURN, m_button_key_burn);
	DDX_Control(pDX, IDC_PROGRESS_DEBUG_BRUN, m_progress_debug_burn);
	DDX_Control(pDX, IDC_COMBO_DEBUG_MEDIA_TYPE, m_combox_debug_media_type);
	DDX_Control(pDX, IDC_STATIC_CONNECT, m_connect_status);
	DDX_Control(pDX, IDC_STATIC_MEDIA_TYPE, m_media_type_status);
	DDX_Control(pDX, IDC_STATIC_TEST, m_text_status);
	DDX_Control(pDX, IDC_STATIC_BURN_STATUS, m_burn_status);
}


BEGIN_MESSAGE_MAP(CDEBUGDlg, CDialogEx)
	ON_BN_CLICKED(IDC_BUTTON_DEBUG_CONNECT, &CDEBUGDlg::OnBnClickedButtonDebugConnect)
	ON_BN_CLICKED(IDC_BUTTON_DEBUG_DISCONNECT, &CDEBUGDlg::OnBnClickedButtonDebugDisconnect)
	ON_BN_CLICKED(IDC_BUTTON_START, &CDEBUGDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_DEBUG_DOWN, &CDEBUGDlg::OnBnClickedButtonDebugDown)
	ON_BN_CLICKED(IDC_BUTTON_DEBUG_DOWN_CMD, &CDEBUGDlg::OnBnClickedButtonDebugDownCmd)
	ON_CBN_SELCHANGE(IDC_COMBO1, &CDEBUGDlg::OnCbnSelchangeCombo1)
	ON_BN_CLICKED(IDC_BUTTON_KEY_BURN, &CDEBUGDlg::OnBnClickedButtonKeyBurn)
	ON_CBN_SELCHANGE(IDC_COMBO_DEBUG_MEDIA_TYPE, &CDEBUGDlg::OnCbnSelchangeComboDebugMediaType)
END_MESSAGE_MAP()


// CDEBUGDlg 消息处理程序

void CDEBUGDlg::setText(int ID, CString s)
{
	SetDlgItemText(ID, s);
}

void CDEBUGDlg::OnBnClickedButtonDebugConnect()
{
	// TODO: 在此添加控件通知处理程序代码

	int ret = 0;
	int i = 0, is_match = 0;
	int ep_cnt;
	CString str_vid;
	CString str_pid;
	CString ep_address;

	m_button_debug_connect.EnableWindow(false);

	GetDlgItem(IDC_EDIT_DEBUG_VID)->GetWindowText(str_vid);
	GetDlgItem(IDC_EDIT_DEBUG_PID)->GetWindowText(str_pid);
	//GetDlgItemText(IDC_EDIT_VID, str_vid);
	//AfxMessageBox(str_vid);

	ret = libusb_init(NULL);
	if (ret < 0) 
	{
		MessageBox(TEXT("libusb_init failed!"), TEXT("错误"), MB_OK | MB_ICONWARNING);

		return;
	}

	// get usb device list
	ret = libusb_get_device_list(NULL, &libusb_work_sb_debug.list);
	if (ret < 0) 
	{
		MessageBox(TEXT("libusb_get_device_list failed!"), TEXT("错误"), MB_OK | MB_ICONWARNING);

		goto get_failed;
	}

	/* print/check the matched device */
	while ((libusb_work_sb_debug.usbdev = libusb_work_sb_debug.list[i++]) != NULL) 
	{
		libusb_get_device_descriptor(libusb_work_sb_debug.usbdev, &libusb_work_sb_debug.dev_desc);

		if (libusb_work_sb_debug.dev_desc.idVendor == _tcstoul(str_vid, 0, 16) &&
			libusb_work_sb_debug.dev_desc.idProduct == _tcstoul(str_pid, 0, 16)) 
		{

			is_match = 1;
			break;
		}
	}

	if (!is_match) 
	{
		MessageBox(TEXT("no matched usb device..."), TEXT("错误"), MB_OK | MB_ICONWARNING);
		goto match_fail;
	}

	/* open usb device */
	ret = libusb_open(libusb_work_sb_debug.usbdev, &libusb_work_sb_debug.handle);
	if (ret < 0) 
	{
		MessageBox(TEXT("libusb_open failed!"), TEXT("错误"), MB_OK | MB_ICONWARNING);
		goto open_failed;
	}
		
	
	/* get config descriptor */
	ret = libusb_get_config_descriptor(libusb_work_sb_debug.usbdev, 0, &libusb_work_sb_debug.config_desc);
	if (ret < 0) 
	{
		MessageBox(TEXT("get config descriptor failed!"), TEXT("错误"), MB_OK | MB_ICONWARNING);

		goto configdesc_fail;
	}

	ep_cnt = libusb_work_sb_debug.config_desc->interface->altsetting[0].bNumEndpoints;

	/* get bulk in/out ep */
	for (i = 0; i < ep_cnt; i++) 
	{
		libusb_work_sb_debug.ep_desc = &libusb_work_sb_debug.config_desc->interface->altsetting[0].endpoint[i];
		if ((libusb_work_sb_debug.ep_desc->bmAttributes & LIBUSB_TRANSFER_TYPE_MASK) & LIBUSB_TRANSFER_TYPE_BULK) 
		{
			if ((libusb_work_sb_debug.ep_desc->bEndpointAddress & LIBUSB_ENDPOINT_DIR_MASK) & LIBUSB_ENDPOINT_IN) 
			{
				//if (!libusb_work_sb_debug.ep_bulkin) {
					libusb_work_sb_debug.ep_bulkin = libusb_work_sb_debug.ep_desc->bEndpointAddress;
				//}
			}
			else 
			{
				//if (!libusb_work_sb_debug.ep_bulkout) {
					libusb_work_sb_debug.ep_bulkout = libusb_work_sb_debug.ep_desc->bEndpointAddress;
				//}
			}
		}
	}

	/* claim the interface */
	libusb_detach_kernel_driver(libusb_work_sb_debug.handle, 0);
	ret = libusb_claim_interface(libusb_work_sb_debug.handle, 0);
	if (ret < 0) 
	{
		MessageBox(TEXT("claim usb interface failed!"), TEXT("错误"), MB_OK | MB_ICONWARNING);
		goto claim_failed;
	}

	MessageBoxTimeout(NULL, _T("设备绑定成功"), _T("提示"), MB_ICONINFORMATION, GetSystemDefaultLangID(), 300);
	
	//m_button_debug_connect.EnableWindow(false);
	m_button_debug_disconnect.EnableWindow(true);
	m_button_debug_start.EnableWindow(true);
	m_button_debug_cmd.EnableWindow(true);
	m_button_debug_file.EnableWindow(true);
	m_button_key_burn.EnableWindow(true);

	m_connect_status.SetWindowTextW(_T("已连接设备"));

	return;

configdesc_fail:
claim_failed:
	libusb_close(libusb_work_sb_debug.handle);

open_failed:
match_fail:
	libusb_free_device_list(libusb_work_sb_debug.list, 1);

get_failed:
	libusb_exit(NULL);

	m_button_debug_connect.EnableWindow(true);

	return;
}


void CDEBUGDlg::OnBnClickedButtonDebugDisconnect()
{
	// TODO: 在此添加控件通知处理程序代码
	m_button_debug_connect.EnableWindow(true);
	m_button_debug_disconnect.EnableWindow(false);
	m_button_debug_start.EnableWindow(false);
	m_button_debug_cmd.EnableWindow(false);
	m_button_debug_file.EnableWindow(false);
	m_button_key_burn.EnableWindow(false);

	libusb_close(libusb_work_sb_debug.handle);

	libusb_free_device_list(libusb_work_sb_debug.list, 1);

	//libusb_exit(NULL);

	MessageBoxTimeout(NULL, _T("设备断开成功"), _T("提示"), MB_ICONINFORMATION, GetSystemDefaultLangID(), 300);

	m_connect_status.SetWindowTextW(_T("已断开设备"));

}


void CDEBUGDlg::OnBnClickedButtonStart()
{
	// TODO: 在此添加控件通知处理程序代码
	int ret;

	m_button_debug_start.EnableWindow(false);

	libusb_work_sb_debug.buffer[0] = USTART;
	ret = libusb_bulk_transfer(libusb_work_sb_debug.handle, libusb_work_sb_debug.ep_bulkout, 
								libusb_work_sb_debug.buffer, 1,
								&libusb_work_sb_debug.transfered, TIMEOUT);
	if (ret == 0) 
	{
		ret = libusb_bulk_transfer(libusb_work_sb_debug.handle, libusb_work_sb_debug.ep_bulkin,
								libusb_work_sb_debug.buffer,
								sizeof(libusb_work_sb_debug.buffer), &libusb_work_sb_debug.transfered, TIMEOUT);

		if (ret == 0) 
		{
			if (strncmp(uACK, (char*)libusb_work_sb_debug.buffer, libusb_work_sb_debug.transfered) == 0) 
			{
				MessageBoxTimeout(NULL, _T("设备握手成功"), 
										_T("提示"),
										MB_ICONINFORMATION,
										GetSystemDefaultLangID(),
										300);
			} 
			else 
			{
				MessageBox(TEXT("设备握手失败"), TEXT("错误"), MB_OK | MB_ICONWARNING);
			}

		} 
		else 
		{
			MessageBox(TEXT("libusb_work_sb_debug start in failed!"), TEXT("错误"), MB_OK | MB_ICONWARNING);
		}

	} 
	else 
	{
		MessageBox(TEXT("libusb_work_sb_debug start failed!"), TEXT("错误"), MB_OK | MB_ICONWARNING);
	}

	m_button_debug_start.EnableWindow(true);

}


void CDEBUGDlg::OnBnClickedButtonDebugDown()
{
	// TODO: 在此添加控件通知处理程序代码
	int ret;
	CFile Donwnload_File;
	CString download_file_path, download_file_path_tmp;
	CFileException Donwnload_File_Error; // 用于保存出错信息
	TCHAR  exepath[MAX_PATH];
	CString  donwnload_file_absolute_path, tmpdir;

	m_button_debug_file.EnableWindow(false);

	// Setp 1 : Get Donwnload File Path
	memset(exepath, 0, MAX_PATH);
	GetModuleFileName(NULL, exepath, MAX_PATH);
	tmpdir = exepath;
	donwnload_file_absolute_path = tmpdir.Left(tmpdir.ReverseFind('\\'));
	download_file_path = donwnload_file_absolute_path;
	download_file_path += LIBUSB_IMAGE_PATH;
	GetDlgItemText(IDC_EDIT_DEBUG_DOWN_FILE, download_file_path_tmp);
	download_file_path += download_file_path_tmp;

	// Setp 2 : open Donwnload File
	if (!Donwnload_File.Open(download_file_path, CFile::modeRead, &Donwnload_File_Error)) 
	{
		download_file_path_tmp += " open failed!!!";
		MessageBox(download_file_path_tmp, TEXT("错误"), MB_OK | MB_ICONWARNING);
		TCHAR szError[1024];
		Donwnload_File_Error.GetErrorMessage(szError, 1024);
		_tprintf_s(_T("Couldn't open source file: %1024s"), szError);
		m_button_debug_file.EnableWindow(true);
		return;
	}
	
	// Setp 3 : Get Donwnload File Address and Get Donwnload File Size 
	CString download_file_address_tmp;
	ULONGLONG download_file_size, download_file_address;
	CFileStatus fileStatus;

	GetDlgItemText(IDC_EDIT_DEBUG_DOWN_DDRADDRESS, download_file_address_tmp);
	download_file_address = _tcstoul(download_file_address_tmp, 0, 16);

	if (CFile::GetStatus(download_file_path, fileStatus)) 
	{
		download_file_size = fileStatus.m_size;
		CString sizetmp;
		sizetmp.Format(_T("%#x"), download_file_size);
		setText(IDC_EDIT_DEBUG_DOWN_SIZE, sizetmp);
		//speed_str.Format(_T("size_tmp: %d"), size_tmp);
		//MessageBox(speed_str, TEXT("LibUSB"), MB_OK | MB_ICONWARNING);
	}

	// Setp 4 : Set Donwnload File Address and Size 
	libusb_work_sb_debug.buffer[0] = UHEAD;
	libusb_work_sb_debug.buffer[1] = (download_file_size & 0xFF000000) >> 24;
	libusb_work_sb_debug.buffer[2] = (download_file_size & 0xFF0000) >> 16;
	libusb_work_sb_debug.buffer[3] = (download_file_size & 0xFF00) >> 8;
	libusb_work_sb_debug.buffer[4] = (download_file_size & 0xFF);
	libusb_work_sb_debug.buffer[5] = (download_file_address & 0xFF000000) >> 24;
	libusb_work_sb_debug.buffer[6] = (download_file_address & 0xFF0000) >> 16;
	libusb_work_sb_debug.buffer[7] = (download_file_address & 0xFF00) >> 8;
	libusb_work_sb_debug.buffer[8] = (download_file_address & 0xFF);

	ret = libusb_bulk_transfer(libusb_work_sb_debug.handle, libusb_work_sb_debug.ep_bulkout,
					libusb_work_sb_debug.buffer, 9,
					&libusb_work_sb_debug.transfered, TIMEOUT);
	if (ret == 0) 
	{
		// 读取数据
		ret = libusb_bulk_transfer(libusb_work_sb_debug.handle, libusb_work_sb_debug.ep_bulkin,
					libusb_work_sb_debug.buffer,
					sizeof(libusb_work_sb_debug.buffer), &libusb_work_sb_debug.transfered, TIMEOUT);
		if (ret == 0) 
		{
			if (strncmp(uACK, (char*)libusb_work_sb_debug.buffer, libusb_work_sb_debug.transfered) == 0) 
			{
				/*MessageBoxTimeout(NULL, _T("Download地址和长度设置成功"), _T("提示"),
									MB_ICONINFORMATION,
									GetSystemDefaultLangID(),
									500);*/
			} 
			else 
			{
				MessageBox(TEXT("Download地址和长度设置失败"), TEXT("错误"), MB_OK | MB_ICONWARNING);
			}
		} 
		else 
		{
			MessageBox(TEXT("libusb_work_sb_debug start in failed!"), TEXT("错误"), MB_OK | MB_ICONWARNING);
			goto exit_dtb;
		}
	} 
	else 
	{
		MessageBox(TEXT("libusb_work_sb_debug Set Donwnload File Address and Size failed!"), TEXT("错误"), MB_OK | MB_ICONWARNING);
		goto exit_dtb;
	}

	// Setp 5 :  transfer Donwnload File
	while (1) 
	{
		libusb_work_sb_debug.dwRead = Donwnload_File.Read(libusb_work_sb_debug.buffer, sizeof(libusb_work_sb_debug.buffer));
		if (libusb_work_sb_debug.dwRead == 0) 
		{
			break;
		}

		ret = libusb_bulk_transfer(libusb_work_sb_debug.handle, libusb_work_sb_debug.ep_bulkout,
						libusb_work_sb_debug.buffer, libusb_work_sb_debug.dwRead,
						&libusb_work_sb_debug.transfered, TIMEOUT);
		if (ret == 0) 
		{

		} 
		else 
		{
			MessageBox(TEXT("libusb_work_sb_debug transfer Donwnload File failed!"), TEXT("错误"), MB_OK | MB_ICONWARNING);
			break;
		}

	}

exit_dtb:
	// Setp 6 :  close Donwnload File
	Donwnload_File.Close();

	m_button_debug_file.EnableWindow(true);
}


void CDEBUGDlg::OnBnClickedButtonDebugDownCmd()
{
	// TODO: 在此添加控件通知处理程序代码
	int ret, cmd_len;
	CString cmd_boot, str_address_dtb, str_address_kernel;
	char cmd_name[MAX_PATH] = { 0 };

	m_button_debug_cmd.EnableWindow(false);

	GetDlgItemText(IDC_EDIT_DEBUG_DOWN_CMD, cmd_boot);
	CString2Char(cmd_boot, cmd_name);

	cmd_len = cmd_boot.GetLength();
	if (cmd_len == 0) 
	{
		MessageBox(TEXT("命令为空"), TEXT("错误"), MB_OK | MB_ICONWARNING);
		goto cmd_end;
	}

	libusb_work_sb_debug.buffer[0] = UCMD;
	memcpy(&libusb_work_sb_debug.buffer[3], cmd_name, cmd_len);

	ret = libusb_bulk_transfer(libusb_work_sb_debug.handle, libusb_work_sb_debug.ep_bulkout,
					libusb_work_sb_debug.buffer, 3 + cmd_len,
					&libusb_work_sb_debug.transfered, TIMEOUT);
	if (ret == 0) 
	{
		ret = libusb_bulk_transfer(libusb_work_sb_debug.handle, libusb_work_sb_debug.ep_bulkin,
						libusb_work_sb_debug.buffer, sizeof(libusb_work_sb_debug.buffer), 
						&libusb_work_sb_debug.transfered, TIMEOUT*50);
		if (ret == 0) 
		{
			if (strncmp(EOT_OK, (char*)libusb_work_sb_debug.buffer, libusb_work_sb_debug.transfered) == 0) 
			{
				/*MessageBoxTimeout(NULL, _T("命令处理成功"), _T("提示"),
								MB_ICONINFORMATION,
								GetSystemDefaultLangID(),
								300);*/
			} 
			else 
			{
				MessageBox(TEXT("命令处理失败"), TEXT("错误"), MB_OK | MB_ICONWARNING);
			}
		} 
		else 
		{
			MessageBox(TEXT("libusb_work_sb_debug start in failed!"), TEXT("错误"), MB_OK | MB_ICONWARNING);
		}
	} 
	else 
	{
		MessageBox(TEXT("libusb_work_sb_debug cmd failed!"), TEXT("错误"), MB_OK | MB_ICONWARNING);
	}

cmd_end:
	m_button_debug_cmd.EnableWindow(true);

}

void CDEBUGDlg::OnCbnSelchangeCombo1()
{
	// TODO: 在此添加控件通知处理程序代码
	int nSel = m_combobox_debug_cmd.GetCurSel();
	if (nSel >= 0)
	{
		CString strSel;
		m_combobox_debug_cmd.GetLBText(nSel, strSel);

		setText(IDC_EDIT_DEBUG_DOWN_CMD, strSel);
	}
}

/* 
* 
* 
* my burn function
* 
* 
*/

CString CDEBUGDlg::MyFunctionGetExePath()
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

CString CDEBUGDlg::MyFunctionHandshake(struct _libusb_work_sb *myfunc_work_sb)
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
				MessageBoxTimeout(NULL, _T("握手成功，即将开始烧录......"), 
										_T("提示"),
										MB_ICONINFORMATION,
										GetSystemDefaultLangID(),
										1000);
			} 
			else 
			{
				MessageBox(TEXT("设备握手失败"), TEXT("错误"), MB_OK | MB_ICONWARNING);
				return _T("ERROR");
			}

		} 
		else 
		{
			MessageBox(TEXT("[MyFunctionHandshake] start in failed!"), TEXT("错误"), MB_OK | MB_ICONWARNING);
			return _T("ERROR");
		}

	} 
	else 
	{
		MessageBox(TEXT("[MyFunctionHandshake] start out failed!"), TEXT("错误"), MB_OK | MB_ICONWARNING);
		return _T("ERROR");
	}

	return _T("OK");
}

CString CDEBUGDlg::MyFunctionDownloadCmd(struct _libusb_work_sb *myfunc_work_sb, char * cmd_name, int cmd_len)
{
	int ret;

	myfunc_work_sb->buffer[0] = UCMD;
	memcpy(&myfunc_work_sb->buffer[3], cmd_name, cmd_len);

	ret = libusb_bulk_transfer(myfunc_work_sb->handle, myfunc_work_sb->ep_bulkout,
		myfunc_work_sb->buffer, 3 + cmd_len,
		&myfunc_work_sb->transfered, TIMEOUT);
	if (ret == 0) 
	{
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
				}
				else
				{
					MessageBox(TEXT("命令处理失败"), TEXT("错误"), MB_OK | MB_ICONWARNING);
					return _T("ERROR");
				}
			}
			else
			{
				MessageBox(TEXT("[MyFunctionDownloadCmd] cmd in failed!"), TEXT("错误"), MB_OK | MB_ICONWARNING);
				return _T("ERROR");
			}
		}
	} 
	else 
	{
		MessageBox(TEXT("[MyFunctionDownloadCmd] cmd out failed!"), TEXT("错误"), MB_OK | MB_ICONWARNING);
		return _T("ERROR");
	}

	return _T("OK");
}


CString CDEBUGDlg::MyFunctionProCmd(struct _libusb_work_sb *myfunc_work_sb, struct _libusb_burn b)
{
	// TODO: 在此添加控件通知处理程序代码
	int cmd_len;
	CString cmd_boot, cmd_status;
	char cmd_name[MAX_PATH] = { 0 };

	int npos = m_progress_debug_burn.GetPos();

	if (!b.BurnImageCmd.Compare(_T("vburn nand"))) 
	{
		// vburn nand 0x90000000

		cmd_boot.Empty();
		cmd_boot += _T("vburn nand");
		cmd_boot += _T(" ");
		cmd_boot += b.DownloadDdrAddress;

		CString2Char(cmd_boot, cmd_name);
		cmd_status = MyFunctionDownloadCmd(myfunc_work_sb, cmd_name, cmd_boot.GetLength());
		if (!cmd_status.Compare(_T("ERROR")))
		{
			return _T("ERROR");
		}

		npos += 10;
		m_progress_debug_burn.SetPos(npos);
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
		cmd_status = MyFunctionDownloadCmd(myfunc_work_sb, cmd_name, cmd_boot.GetLength());
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
		cmd_status = MyFunctionDownloadCmd(myfunc_work_sb, cmd_name, cmd_boot.GetLength());
		if (!cmd_status.Compare(_T("ERROR")))
		{
			return _T("ERROR");
		}

		npos += 10;
		m_progress_debug_burn.SetPos(npos);

		if (b.backup_partition_flag == true)
		{
			cmd_boot.Empty();
			cmd_boot += _T("nand erase");
			cmd_boot += _T(" ");
			cmd_boot += b.PartitionOffset1;
			cmd_boot += _T(" ");
			cmd_boot += b.PartitionSize;

			CString2Char(cmd_boot, cmd_name);
			cmd_status = MyFunctionDownloadCmd(myfunc_work_sb, cmd_name, cmd_boot.GetLength());
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
			cmd_status = MyFunctionDownloadCmd(myfunc_work_sb, cmd_name, cmd_boot.GetLength());
			if (!cmd_status.Compare(_T("ERROR")))
			{
				return _T("ERROR");
			}

			npos += 10;
			m_progress_debug_burn.SetPos(npos);
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
		cmd_status = MyFunctionDownloadCmd(myfunc_work_sb, cmd_name, cmd_boot.GetLength());
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
		cmd_status = MyFunctionDownloadCmd(myfunc_work_sb, cmd_name, cmd_boot.GetLength());
		if (!cmd_status.Compare(_T("ERROR")))
		{
			return _T("ERROR");
		}

		npos += 25;
		m_progress_debug_burn.SetPos(npos);

	} 
	else 
	{
		cmd_boot.Empty();
		cmd_boot += _T("nand info");

		CString2Char(cmd_boot, cmd_name);
		cmd_status = MyFunctionDownloadCmd(myfunc_work_sb, cmd_name, cmd_boot.GetLength());
		if (!cmd_status.Compare(_T("ERROR")))
		{
			return _T("ERROR");
		}
	}

	return _T("OK");

}

CString CDEBUGDlg::MyFunctionDownloadImage(struct _libusb_work_sb *myfunc_work_sb, CString download_file_path, CString download_file_address_tmp)
{
	// TODO: 在此添加控件通知处理程序代码
	int ret;
	CFile Donwnload_File;
	CFileException Donwnload_File_Error; // 用于保存出错信息

	// Setp 1 : open Donwnload File
	if (!Donwnload_File.Open(download_file_path, CFile::modeRead, &Donwnload_File_Error)) 
	{
		download_file_path += " open failed!!!";
		MessageBox(download_file_path, TEXT("错误"), MB_OK | MB_ICONWARNING);
		TCHAR szError[1024];
		Donwnload_File_Error.GetErrorMessage(szError, 1024);
		_tprintf_s(_T("Couldn't open source file: %1024s"), szError);
		
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
				
			} 
			else 
			{
				MessageBox(TEXT("Download地址和长度设置失败"), TEXT("错误"), MB_OK | MB_ICONWARNING);
				goto down_image_error;
			}
		} 
		else 
		{
			MessageBox(TEXT("[MyFunctionDownloadImage] start in failed!"), TEXT("错误"), MB_OK | MB_ICONWARNING);
			goto down_image_error;
		}
	} 
	else 
	{
		MessageBox(TEXT("[MyFunctionDownloadImage] Set Donwnload File Address and Size failed!"), TEXT("错误"), MB_OK | MB_ICONWARNING);
		goto down_image_error;
	}

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
			MessageBox(TEXT("[MyFunctionDownloadImage] transfer Donwnload File failed!"), TEXT("错误"), MB_OK | MB_ICONWARNING);
			goto down_image_error;
			break;
		}

	}

	// Setp 5 :  close Donwnload File
	Donwnload_File.Close();

	
	return return_download_file_size;

down_image_error:

	Donwnload_File.Close();

	return _T("ERROR");
}


CString UTF82WCS(const char* szU8)
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

char* UnicodeToUtf8(const wchar_t* unicode)
{
	int len;
	len = WideCharToMultiByte(CP_UTF8, 0, unicode, -1, NULL, 0, NULL, NULL);
	char* szUtf8 = (char*)malloc(len + 1);
	memset(szUtf8, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, unicode, -1, szUtf8, len, NULL, NULL);
	return szUtf8;
}


UINT CDEBUGDlg::MyFunctionSpiNandBurnImage(struct _libusb_work_sb *myfunc_work_sb, struct _libusb_burn * myfunc_burn_sb)
{
	CString  exe_dir;
	CString str_boot_dir, str_boot_name;
	char char_boot_name[MAX_PATH] = { 0 };

	exe_dir = MyFunctionGetExePath();

	str_boot_dir = exe_dir;
	str_boot_dir += LIBUSB_IMAGE_PATH;

	int imagecount = GetPrivateProfileInt(TEXT("ImageCount"), TEXT("numOfImage"), 0, exe_dir + TEXT(LIBUSB_SPINAND_INI));
	if (imagecount == 0) 
	{
		MessageBox(TEXT("无镜像文件"), TEXT("错误"), MB_OK | MB_ICONWARNING);
		m_button_debug_start.EnableWindow(true);
		m_button_debug_cmd.EnableWindow(true);
		m_button_debug_file.EnableWindow(true);
		m_burn_status.SetWindowTextW(_T("未烧录"));
		return -1;
	}
	CString downloadimage;
	CString GetTmp;
	int i;
	int npos = m_progress_debug_burn.GetPos();

	// 一 : 从init文件获取参数
	for (i = 0; i < imagecount; i++) 
	{
		downloadimage.Format(_T("DOWNLOADIMAGE%d"), i);

		// 1 : 获取地址参数
		GetPrivateProfileString(downloadimage, TEXT("DownloadDdrAddress"), TEXT(LIBUSB_DOWN_ADDRESS),
								GetTmp.GetBuffer(MAX_PATH), MAX_PATH,
								exe_dir + TEXT(LIBUSB_SPINAND_INI));
		CString2Char(GetTmp, char_boot_name);
		myfunc_burn_sb[i].DownloadDdrAddress = UTF82WCS(char_boot_name);

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
		myfunc_burn_sb[i].BurnImageCmd = UTF82WCS(char_boot_name);

		//AfxMessageBox(myfunc_burn_sb[i].BurnImageCmd);
		
		// 4 : 获取镜像路径
		GetPrivateProfileString(downloadimage, TEXT("DownloadImageName"), TEXT("NoDownloadImageName"),
							GetTmp.GetBuffer(MAX_PATH), MAX_PATH,
							exe_dir + TEXT(LIBUSB_SPINAND_INI));

		CString2Char(GetTmp, char_boot_name);
		str_boot_name = UTF82WCS(char_boot_name);

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
			myfunc_burn_sb[i].PartitionOffset1 = UTF82WCS(char_boot_name);

			//AfxMessageBox(myfunc_burn_sb[i].PartitionOffset1);
		}

		// 6 : 获取分区大小
		GetPrivateProfileString(downloadimage, TEXT("PartitionSize"), TEXT("NoPartitionSize"),
			GetTmp.GetBuffer(MAX_PATH), MAX_PATH,
			exe_dir + TEXT(LIBUSB_SPINAND_INI));

		CString2Char(GetTmp, char_boot_name);
		myfunc_burn_sb[i].PartitionSize = UTF82WCS(char_boot_name);

		//AfxMessageBox(myfunc_burn_sb[i].PartitionSize);

		// 7 : 获取分区偏移
		GetPrivateProfileString(downloadimage, TEXT("PartitionOffset"), TEXT("NoPartitionOffset"),
			GetTmp.GetBuffer(MAX_PATH), MAX_PATH,
			exe_dir + TEXT(LIBUSB_SPINAND_INI));

		CString2Char(GetTmp, char_boot_name);
		myfunc_burn_sb[i].PartitionOffset = UTF82WCS(char_boot_name);

		//AfxMessageBox(myfunc_burn_sb[i].PartitionOffset);

		npos += 5;
		m_progress_debug_burn.SetPos(npos);
	}

	// 二 : 执行烧录
	CString cmd_status;
	for (i = 0; i < imagecount; i++) 
	{
		if (myfunc_burn_sb[i].NeedBurn)
		{

			myfunc_burn_sb[i].Download_Size = MyFunctionDownloadImage(myfunc_work_sb,
				myfunc_burn_sb[i].DownloadImagePath,
				myfunc_burn_sb[i].DownloadDdrAddress);
			if (!myfunc_burn_sb[i].Download_Size.Compare(_T("ERROR")))
			{
				MessageBox(TEXT("烧录失败（Download File）"), TEXT("提示"), MB_OK | MB_ICONWARNING);
				m_burn_status.SetWindowTextW(_T("烧录失败"));
				return -1;
			} 
			
			cmd_status = MyFunctionProCmd(myfunc_work_sb, myfunc_burn_sb[i]);
			if (!cmd_status.Compare(_T("ERROR")))
			{
				MessageBox(TEXT("烧录失败（Download Cmd）"), TEXT("提示"), MB_OK | MB_ICONWARNING);
				m_burn_status.SetWindowTextW(_T("烧录失败"));
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
		m_progress_debug_burn.SetPos(100);

		m_burn_status.SetWindowTextW(_T("烧录成功"));

		UINT i = MessageBoxTimeout(NULL, _T("5s 将自动重启，是否重启？"), _T("温馨提示"),
							MB_YESNO | MB_ICONINFORMATION,
							GetSystemDefaultLangID(),
							5000);
		if (i == IDNO)
		{
			goto NandBurnImageEnd;
		}

		MyFunctionDownloadCmd(myfunc_work_sb, "reset", strlen("reset"));
	} 
		
NandBurnImageEnd:
	m_button_debug_start.EnableWindow(true);
	m_button_debug_cmd.EnableWindow(true);
	m_button_debug_file.EnableWindow(true);

	return 0;
}

UINT ThreadButtonKeyBurn(LPVOID pParam)
{
	CDEBUGDlg* pObj = (CDEBUGDlg*)pParam;

	return pObj->MyFunctionSpiNandBurnImage(&libusb_work_sb_debug, libusb_burn);
}

void CDEBUGDlg::OnBnClickedButtonKeyBurn()
{
	// TODO: 在此添加控件通知处理程序代码
	CWinThread* m_pThread = nullptr;

	m_burn_status.SetWindowTextW(_T("正在烧录..."));

	m_button_debug_start.EnableWindow(false);
	m_button_debug_cmd.EnableWindow(false);
	m_button_debug_file.EnableWindow(false);

	m_progress_debug_burn.SetRange(0, 100);
	m_progress_debug_burn.SetPos(0);

	CString GetMediaType;
	int nSel = m_combox_debug_media_type.GetCurSel();
	if (nSel >= 0)
	{
		m_combox_debug_media_type.GetLBText(nSel, GetMediaType);
	}

	if (!GetMediaType.Compare(_T(LIBUSB_MEDIA_SPINAND)))
	{
		CString Handshake_status = MyFunctionHandshake(&libusb_work_sb_debug);
		if (!Handshake_status.Compare(_T("ERROR")))
		{
			goto error_end;
		}

		m_pThread = AfxBeginThread(ThreadButtonKeyBurn, this);
	}
	else if (!GetMediaType.Compare(_T(LIBUSB_MEDIA_EMMC)))
	{

		MessageBox(TEXT("emmc烧录，暂未支持！！！"), TEXT("温馨提示"), MB_OK | MB_ICONWARNING);
		goto error_end;
	}
	else
	{
		MessageBox(TEXT("无选中的媒体介质，请选一种烧录介质！！！"), TEXT("温馨提示"), MB_OK | MB_ICONWARNING);
		goto error_end;
	}

	return;

error_end:

	m_burn_status.SetWindowTextW(_T("未烧录"));
	m_button_debug_start.EnableWindow(true);
	m_button_debug_cmd.EnableWindow(true);
	m_button_debug_file.EnableWindow(true);
}


void CDEBUGDlg::OnCbnSelchangeComboDebugMediaType()
{
	// TODO: 在此添加控件通知处理程序代码

	// 获取到选中项目的 索引
	int nSel = m_combox_debug_media_type.GetCurSel();
	if (nSel >= 0)
	{
		//根据 索引 获取对应的字符串
		CString strSel;
		m_combox_debug_media_type.GetLBText(nSel, strSel);

		m_media_type_status.SetWindowTextW(strSel);

		//将字符串保存到ini文件中
		CString exe_dir = MyFunctionGetExePath();

		WritePrivateProfileString(TEXT("SystemConf"), TEXT("mediaType"), strSel, exe_dir + TEXT(LIBUSB_VMCTOOLS_INI));
	}
	
}
