#pragma once


// CDEBUGDlg 对话框

class CDEBUGDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CDEBUGDlg)

public:
	CDEBUGDlg(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CDEBUGDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_DEBUG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonDebugConnect();
	afx_msg void OnBnClickedButtonDebugDisconnect();
	CButton m_button_debug_connect;
	CButton m_button_debug_disconnect;
	CEdit m_edit_debug_vid;
	CEdit m_edit_debug_pid;

	void CDEBUGDlg::setText(int ID, CString s);
	CButton m_button_debug_cmd;
	CButton m_button_debug_file;
	CButton m_button_debug_start;
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonDebugDown();
	afx_msg void OnBnClickedButtonDebugDownCmd();
	CComboBox m_combobox_debug_cmd;
	afx_msg void OnCbnSelchangeCombo1();
	CButton m_button_key_burn;
	afx_msg void OnBnClickedButtonKeyBurn();
	afx_msg UINT MyFunctionSpiNandBurnImage(struct _libusb_work_sb *myfunc_work_sb, struct _libusb_burn* myfunc_burn_sb);
	afx_msg CString MyFunctionHandshake(struct _libusb_work_sb *myfunc_work_sb);
	afx_msg CString MyFunctionGetExePath();
	afx_msg CString MyFunctionDownloadImage(struct _libusb_work_sb *myfunc_work_sb, CString download_file_path, CString download_file_address_tmp);
	afx_msg CString MyFunctionProCmd(struct _libusb_work_sb *myfunc_work_sb, struct _libusb_burn b);
	afx_msg CString MyFunctionDownloadCmd(struct _libusb_work_sb *myfunc_work_sb, char* cmd_name, int cmd_len);
	CProgressCtrl m_progress_debug_burn;
	//afx_msg void OnEnChangeEditDebugDownCmd();
	CComboBox m_combox_debug_media_type;
	afx_msg void OnCbnSelchangeComboDebugMediaType();
	CStatic m_connect_status;
	CStatic m_media_type_status;
	CStatic m_text_status;
	CStatic m_burn_status;
};
