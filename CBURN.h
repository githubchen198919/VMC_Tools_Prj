#pragma once


// CBURN 对话框

class CBURN : public CDialogEx
{
	DECLARE_DYNAMIC(CBURN)

public:
	CBURN(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CBURN();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_BURN };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonAccessDevice();
	CStatic m_burn_dev1;
	CStatic m_burn_dev2;
	CStatic m_burn_dev3;
	CStatic m_burn_dev4;
	CStatic m_burn_dev5;
	CStatic m_burn_dev6;
	CEdit m_exit_total;
	CEdit m_exit_success;
	CEdit m_exit_fail;
	afx_msg void OnBnClickedButtonStartBurn();
	afx_msg void InitBurnProgress();
	afx_msg CString MyFunctionBurnGetExePath();
	afx_msg void MyFunctionBurnBitMap();
	afx_msg void MyFunctionPrintDebug(int dev_num, CString buf);
	afx_msg UINT MyFunctionBurnSpiNandImage(int dev_num, struct _libusb_work_sb* myfunc_work_sb, struct _libusb_burn* myfunc_burn_sb);
	afx_msg CString MyFunctionBurnDownloadImage(int dev_num, struct _libusb_work_sb* myfunc_work_sb, CString download_file_path, CString download_file_address_tmp);
	afx_msg CString MyFunctionBurnDownloadCmd(int dev_num, struct _libusb_work_sb* myfunc_work_sb, char* cmd_name, int cmd_len);
	afx_msg CString MyFunctionBurnProCmd(int dev_num, struct _libusb_work_sb* myfunc_work_sb, struct _libusb_burn b);
	afx_msg CString MyFunctionBurnHandshake(struct _libusb_work_sb* myfunc_work_sb);
	afx_msg void MyFunctionBurnCheckDeleteButton();


	CButton m_button_access_device;
	CProgressCtrl m_burn_progress_dev0;
	CProgressCtrl m_burn_progress_dev1;
	CProgressCtrl m_burn_progress_dev2;
	CProgressCtrl m_burn_progress_dev3;
	CProgressCtrl m_burn_progress_dev4;
	CProgressCtrl m_burn_progress_dev5;
	CStatic m_burn_plan_dev0;
	CStatic m_burn_plan_dev1;
	CStatic m_burn_plan_dev2;
	CStatic m_burn_plan_dev3;
	CStatic m_burn_plan_dev4;
	CStatic m_burn_plan_dev5;
	afx_msg void OnBnClickedButtonDeleteDevice();
	CButton m_button_delete_device;
	CButton m_button_start_burn;
	CStatic m_pic_burn_dev0;
	CStatic m_pic_burn_dev1;
	CStatic m_pic_burn_dev2;
	CStatic m_pic_burn_dev3;
	CStatic m_pic_burn_dev4;
	CStatic m_pic_burn_dev5;
	CButton m_need_reset;
	CEdit m_edit_debug_print;
	CString m_cstring_debug_print;
	afx_msg void OnBnClickedButtonSaveLog();
};
