// AxisDlg.cpp : file di implementazione
//

#include "stdafx.h"
#include "resource.h"
#include ".\AxisDlg.h"


// finestra di dialogo CAxisDlg

IMPLEMENT_DYNAMIC(CAxisDlg, CDialog)
CAxisDlg::CAxisDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAxisDlg::IDD, pParent) {
	m_MaxLabel = _T("");
	m_MinLabel = _T("");
	m_Max      = 0.0;
	m_Min      = 0.0;
	m_Auto     = FALSE;
	m_Log      = FALSE;
}

CAxisDlg::~CAxisDlg() {
}

void
CAxisDlg::DoDataExchange(CDataExchange* pDX) {
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MIN, m_edMin);
	DDX_Control(pDX, IDC_MAX, m_edMax);
	DDX_Control(pDX, IDC_AUTO, m_ckAuto);
	DDX_Control(pDX, IDC_LOG, m_ckLog);
	DDX_Text(pDX, IDC_MAXXY, m_MaxLabel);
	DDV_MaxChars(pDX, m_MaxLabel, 4);
	DDX_Text(pDX, IDC_MINXY, m_MinLabel);
	DDV_MaxChars(pDX, m_MinLabel, 4);
	DDX_Text(pDX, IDC_MAX, m_Max);
	DDV_MinMaxDouble(pDX, m_Max, -1.e+099, 1.e+099);
	DDX_Text(pDX, IDC_MIN, m_Min);
	DDV_MinMaxDouble(pDX, m_Min, -1.e+099, 1.e+099);
	DDX_Check(pDX, IDC_AUTO, m_Auto);
	DDX_Check(pDX, IDC_LOG, m_Log);
}


BEGIN_MESSAGE_MAP(CAxisDlg, CDialog)
	ON_BN_CLICKED(IDC_AUTO, OnAutoClicked)
	ON_BN_CLICKED(IDC_LOG, OnLogClicked)
END_MESSAGE_MAP()


// gestori di messaggi CAxisDlg

void
CAxisDlg::OnAutoClicked() {
  UpdateData(TRUE);
  m_edMin.EnableWindow(!m_Auto);
  m_edMax.EnableWindow(!m_Auto);
}


void
CAxisDlg::OnLogClicked() {
  UpdateData(TRUE);
}


BOOL
CAxisDlg::OnInitDialog() {
	CDialog::OnInitDialog();
  m_edMin.EnableWindow(!m_Auto);
  m_edMax.EnableWindow(!m_Auto);
  UpdateData(FALSE);
	return TRUE;
}
