
// VMCToolsDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "VMCTools.h"
#include "VMCToolsDlg.h"
#include "afxdialogex.h"
#include "my_libusb.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CVMCToolsDlg 对话框



CVMCToolsDlg::CVMCToolsDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_VMCTOOLS_DIALOG, pParent)
{
	//m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_hIcon = AfxGetApp()->LoadIcon(IDI_ICON1);
}

void CVMCToolsDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TAB1, m_tab);
}

BEGIN_MESSAGE_MAP(CVMCToolsDlg, CDialogEx)
	//ON_WM_CLOSE()
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, &CVMCToolsDlg::OnTcnSelchangeTab1)
END_MESSAGE_MAP()


// CVMCToolsDlg 消息处理程序

//void CVMCToolsDlg::OnClose()
//{
//	// TODO: Add your message handler code here and/or call default
//	//MessageBox(_T("12345656456"), TEXT("错误"), MB_OK | MB_ICONWARNING);
//	m_debugDlg.OnBnClickedButtonDebugDisconnect();
//	CDialog::OnClose();
//}

BOOL CVMCToolsDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	//MoveWindow

	m_tab.InsertItem(0, _T("烧录"));
	m_tab.InsertItem(1, _T("调试"));

	m_burnDlg.Create(IDD_DIALOG_BURN, &m_tab);
	m_debugDlg.Create(IDD_DIALOG_DEBUG, &m_tab);

	HDC hdc = ::GetDC(NULL);
	int hdpi = GetDeviceCaps(hdc, LOGPIXELSX);//水平方向每逻辑英寸多少个像素点
	int wdpi = GetDeviceCaps(hdc, LOGPIXELSY);//垂直方向每逻辑英寸多少个像素点

	CRect tab_crect;
	m_tab.GetClientRect(&tab_crect);
	tab_crect.left += 1;
	tab_crect.right -= 1;
	tab_crect.bottom -= 1;

	switch (hdpi)
	{
		case 96:
			tab_crect.top += 22;
			break;
		case 120:
			tab_crect.top += 26;
			break;
		case 144:
			tab_crect.top += 30;
			break;
		case 168:
			tab_crect.top += 33;
			break;
		case 192:
			tab_crect.top += 38;
			break;
		default:
			tab_crect.top += 26;
			break;
	}

	//tab_crect.top += 26;

	m_burnDlg.SetWindowPos(NULL, tab_crect.left, tab_crect.top, 
								tab_crect.Width() - 2, tab_crect.Height() - 1, SWP_SHOWWINDOW);
	m_debugDlg.SetWindowPos(NULL, tab_crect.left, tab_crect.top,
								tab_crect.Width() - 2, tab_crect.Height() - 1, SWP_HIDEWINDOW);
	
	m_debugDlg.m_button_debug_connect.EnableWindow(true);
	m_debugDlg.m_button_debug_disconnect.EnableWindow(false);
	m_debugDlg.m_button_debug_start.EnableWindow(false);
	m_debugDlg.m_button_debug_cmd.EnableWindow(false);
	m_debugDlg.m_button_debug_file.EnableWindow(false);
	m_debugDlg.m_button_key_burn.EnableWindow(false);
	m_debugDlg.setText(IDC_EDIT_DEBUG_VID, _T(LIBUSB_VID));
	m_debugDlg.setText(IDC_EDIT_DEBUG_PID, _T(LIBUSB_PID));
	m_debugDlg.setText(IDC_EDIT_DEBUG_DOWN_FILE, _T(LIBUSB_UKERNEL));
	m_debugDlg.setText(IDC_EDIT_DEBUG_DOWN_DDRADDRESS, _T(LIBUSB_DOWN_ADDRESS));
	m_debugDlg.setText(IDC_EDIT_DEBUG_DOWN_SIZE, _T("0x0"));

	m_debugDlg.m_combox_debug_media_type.AddString(_T("spi nand"));
	m_debugDlg.m_combox_debug_media_type.AddString(_T("emmc"));

	m_burnDlg.SetDlgItemText(IDC_EDIT_TOTAL, _T("0"));
	m_burnDlg.SetDlgItemText(IDC_EDIT_SUCCESS, _T("0"));
	m_burnDlg.SetDlgItemText(IDC_EDIT_FAIL, _T("0"));

	m_burnDlg.m_button_start_burn.EnableWindow(false);
	m_burnDlg.m_button_delete_device.EnableWindow(false);

	m_burnDlg.MyFunctionBurnBitMap();
	
	CString exe_dir = m_debugDlg.MyFunctionGetExePath();
	CString GetTmp, GetUboot_Command;

	int commandcount = GetPrivateProfileInt(TEXT("CommandCount"), TEXT("numOfCommand"), 0, exe_dir + TEXT(LIBUSB_VMCTOOLS_INI));

	GetPrivateProfileString(TEXT("SystemConf"), TEXT("mediaType"), TEXT(" "),
						GetTmp.GetBuffer(MAX_PATH), MAX_PATH,
						exe_dir + TEXT(LIBUSB_VMCTOOLS_INI));

	if (!GetTmp.Compare(_T(LIBUSB_MEDIA_SPINAND)))
	{
		m_debugDlg.m_combox_debug_media_type.SetCurSel(0);
		m_debugDlg.m_media_type_status.SetWindowTextW(GetTmp);
	}
	if (!GetTmp.Compare(_T(LIBUSB_MEDIA_EMMC)))
	{
		m_debugDlg.m_combox_debug_media_type.SetCurSel(1);
		m_debugDlg.m_media_type_status.SetWindowTextW(GetTmp);
	}

	for (int i = 0; i < commandcount; i++)
	{
		GetUboot_Command.Format(_T("CMD%d"), i);

		GetPrivateProfileString(TEXT("UBOOTCOMMAND"), GetUboot_Command, TEXT("NoCmd"),
						GetTmp.GetBuffer(MAX_PATH), MAX_PATH,
						exe_dir + TEXT(LIBUSB_VMCTOOLS_INI));

		if (GetTmp.Compare(TEXT("NoCmd")))
		{
			m_debugDlg.m_combobox_debug_cmd.AddString(GetTmp);
		}
	}

	if (commandcount != 0)
	{
		//默认选中第一项
		m_debugDlg.m_combobox_debug_cmd.SetCurSel(0);
		// 获取到选中项目的 索引
		int nSel = m_debugDlg.m_combobox_debug_cmd.GetCurSel();
		if (nSel >= 0)
		{
			//根据 索引 获取对应的字符串
			CString strSel;
			m_debugDlg.m_combobox_debug_cmd.GetLBText(nSel, strSel);
			//将字符串显示到文本框中
			m_debugDlg.setText(IDC_EDIT_DEBUG_DOWN_CMD, strSel);
		}
	}

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CVMCToolsDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CVMCToolsDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CVMCToolsDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CVMCToolsDlg::OnTcnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 在此添加控件通知处理程序代码
	*pResult = 0;

	CRect tabRect;

	HDC hdc = ::GetDC(NULL);
	int hdpi = GetDeviceCaps(hdc, LOGPIXELSX);//水平方向每逻辑英寸多少个像素点
	int wdpi = GetDeviceCaps(hdc, LOGPIXELSY);//垂直方向每逻辑英寸多少个像素点

	//获取标签页控件的宽高信息
	m_tab.GetClientRect(&tabRect);
	tabRect.left += 1;
	tabRect.right -= 1;
	tabRect.bottom -= 1;

	switch (hdpi)
	{
		case 96:
			tabRect.top += 22;
			break;
		case 120:
			tabRect.top += 26;
			break;
		case 144:
			tabRect.top += 30;
			break;
		case 168:
			tabRect.top += 33;
			break;
		case 192:
			tabRect.top += 38;
			break;
		default:
			tabRect.top += 26;
			break;
	}
	//tabRect.top += 26;

	switch (m_tab.GetCurSel()) 
	{
		case 0:
			m_burnDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width()-2, tabRect.Height()- 1, SWP_SHOWWINDOW);
			m_debugDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width()-2, tabRect.Height()- 1, SWP_HIDEWINDOW);
			break;
		case 1:
			m_burnDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width()-2, tabRect.Height()- 1, SWP_HIDEWINDOW);
			m_debugDlg.SetWindowPos(NULL, tabRect.left, tabRect.top, tabRect.Width()-2, tabRect.Height()- 1, SWP_SHOWWINDOW);
			break;
		default:
			break;
	}
}
