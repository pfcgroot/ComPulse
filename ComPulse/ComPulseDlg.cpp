// ComPulseDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ComPulse.h"
#include "ComPulseDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
END_MESSAGE_MAP()


// CComPulseDlg dialog




CComPulseDlg::CComPulseDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CComPulseDlg::IDD, pParent)
	, m_iTR(3000)
	, m_iCount(100)
	, m_strComPort(_T("COM1"))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CComPulseDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_TR, m_iTR);
	DDX_Text(pDX, IDC_N, m_iCount);
	DDX_CBString(pDX, IDC_COMPORT, m_strComPort);
	DDX_Control(pDX, IDOK, m_btnOK);
	DDX_Control(pDX, IDC_COMPORT, m_cbPort);
	DDX_Control(pDX, IDC_TR, m_txtTR);
	DDX_Control(pDX, IDC_N, m_txtCount);
	DDX_Control(pDX, IDC_START, m_btnStart);
}

BEGIN_MESSAGE_MAP(CComPulseDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDOK, &CComPulseDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_START, &CComPulseDlg::OnBnClickedStart)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CComPulseDlg message handlers

BOOL CComPulseDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CComPulseDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CComPulseDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CComPulseDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CComPulseDlg::OnBnClickedOk()
{
	if (m_serial.IsConnected())
	{
		KillTimer(0);
		m_serial.Disconnect();
		m_btnOK.SetWindowText("Connect");
		m_btnStart.EnableWindow(FALSE);
		m_cbPort.EnableWindow(TRUE);
	}
	else
	{
		UpdateData(TRUE);
		if (m_serial.Connect(m_strComPort)==ERROR_SUCCESS)
		{
			m_btnOK.SetWindowText("Disconnect");
			m_btnStart.EnableWindow(TRUE);
			m_cbPort.EnableWindow(FALSE);
		}
		else
		{
			AfxMessageBox("Couldn't connect to this port");
		}
	}
}

void CComPulseDlg::OnBnClickedStart()
{
	UpdateData(TRUE);
	m_iCount_org = m_iCount;
	OnTimer(0); // kick off now once immediately
	if (m_iCount>0)
		SetTimer(0,m_iTR,NULL);
	else
		UpdateData(FALSE);
}

void CComPulseDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (m_serial.IsConnected())
	{
		DCB dcb = m_serial.GetState();
		if (dcb.fDtrControl==DTR_CONTROL_ENABLE)
			dcb.fDtrControl = DTR_CONTROL_DISABLE;
		else
			dcb.fDtrControl = DTR_CONTROL_ENABLE;
		m_serial.SetState(dcb);
		if (m_iCount==0 || (--m_iCount)==0)
		{
			KillTimer(0);
			m_iCount = m_iCount_org;
		}
		UpdateData(FALSE);
	}
	CDialog::OnTimer(nIDEvent);
}
