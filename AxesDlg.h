#pragma once
#include "afxwin.h"


// finestra di dialogo CAxesDlg

class
CAxesDlg : public CDialog {
	DECLARE_DYNAMIC(CAxesDlg)

public:
	CAxesDlg(CWnd* pParent = NULL);   // costruttore standard
	virtual ~CAxesDlg();

// Dati della finestra di dialogo
	enum { IDD = IDD_AXESDLG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Supporto DDX/DDV
	afx_msg void OnAutox();
	afx_msg void OnAutoy();
	afx_msg void OnLogx();
	afx_msg void OnLogy();
	virtual BOOL OnInitDialog();

	DECLARE_MESSAGE_MAP()
public:
	CEdit	m_edYMin;
	CEdit	m_edYMax;
	CEdit	m_edXMax;
	CEdit	m_edXMin;
	double	m_XMax;
	double	m_XMin;
	double	m_YMax;
	double	m_YMin;
	BOOL	m_AutoX;
	BOOL	m_AutoY;
	BOOL	m_LogX;
	BOOL	m_LogY;
  afx_msg void OnBnClickedOk();
  afx_msg void OnBnClickedCancel();
           };
