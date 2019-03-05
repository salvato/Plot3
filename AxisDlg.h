#pragma once


// finestra di dialogo CAxisDlg

class
CAxisDlg : public CDialog {
	DECLARE_DYNAMIC(CAxisDlg)

public:
	CAxisDlg(CWnd* pParent = NULL);   // costruttore standard
	virtual ~CAxisDlg();

// Dati della finestra di dialogo
	enum { IDD = IDD_AXISDLG };
	CEdit	m_edMin;
	CEdit	m_edMax;
	CButton	m_ckAuto;
	CButton	m_ckLog;
	CString	m_MaxLabel;
	CString	m_MinLabel;
	double	m_Max;
	double	m_Min;
	BOOL	m_Auto;
	BOOL	m_Log;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Supporto DDX/DDV
	afx_msg void OnAutoClicked();
	afx_msg void OnLogClicked();
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
};
