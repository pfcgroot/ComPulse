// ComPulseDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include "Serial.h"

// CComPulseDlg dialog
class CComPulseDlg : public CDialog
{
// Construction
public:
	CComPulseDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_COMPULSE_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	Serial m_serial;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedStart();
	UINT m_iTR;
	UINT m_iCount;
	UINT m_iCount_org;
	CString m_strComPort;
	CButton m_btnOK;
	CComboBox m_cbPort;
	CEdit m_txtTR;
	CEdit m_txtCount;
	CButton m_btnStart;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
