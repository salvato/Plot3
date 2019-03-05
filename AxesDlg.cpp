// AxesDlg.cpp : file di implementazione
//

#include "stdafx.h"
#include "resource.h"
#include ".\axesdlg.h"


// finestra di dialogo CAxesDlg

IMPLEMENT_DYNAMIC(CAxesDlg, CDialog)
CAxesDlg::CAxesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAxesDlg::IDD, pParent) {
	m_XMax  = 0.0;
	m_XMin  = 0.0;
	m_YMax  = 0.0;
	m_YMin  = 0.0;
	m_AutoX = FALSE;
	m_AutoY = FALSE;
	m_LogX  = FALSE;
	m_LogY  = FALSE;
}

CAxesDlg::~CAxesDlg() {
}

void 
CAxesDlg::DoDataExchange(CDataExchange* pDX) {
  CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_XMIN, m_edXMin);
	DDX_Control(pDX, IDC_XMAX, m_edXMax);
	DDX_Control(pDX, IDC_YMIN, m_edYMin);
	DDX_Control(pDX, IDC_YMAX, m_edYMax);
	DDX_Text(pDX, IDC_XMIN, m_XMin);
	DDV_MinMaxDouble(pDX, m_XMin, -1.e+099, 1.e+099);
	DDX_Text(pDX, IDC_XMAX, m_XMax);
	DDV_MinMaxDouble(pDX, m_XMax, -1.e+099, 1.e+099);
	DDX_Text(pDX, IDC_YMIN, m_YMin);
	DDV_MinMaxDouble(pDX, m_YMin, -1.e+099, 1.e+099);
	DDX_Text(pDX, IDC_YMAX, m_YMax);
	DDV_MinMaxDouble(pDX, m_YMax, -1.e+099, 1.e+099);
	DDX_Check(pDX, IDC_AUTOX, m_AutoX);
	DDX_Check(pDX, IDC_AUTOY, m_AutoY);
	DDX_Check(pDX, IDC_LOGX, m_LogX);
	DDX_Check(pDX, IDC_LOGY, m_LogY);
}


BEGIN_MESSAGE_MAP(CAxesDlg, CDialog)
	ON_BN_CLICKED(IDC_AUTOX, OnAutox)
	ON_BN_CLICKED(IDC_AUTOY, OnAutoy)
	ON_BN_CLICKED(IDC_LOGX, OnLogx)
	ON_BN_CLICKED(IDC_LOGY, OnLogy)
  ON_BN_CLICKED(IDOK, OnBnClickedOk)
  ON_BN_CLICKED(IDCANCEL, OnBnClickedCancel)
END_MESSAGE_MAP()


// gestori di messaggi CAxesDlg

void
CAxesDlg::OnAutox() {
  UpdateData(TRUE);
  m_edXMin.EnableWindow(!m_AutoX);
  m_edXMax.EnableWindow(!m_AutoX);
}


void 
CAxesDlg::OnLogx() {
  UpdateData(TRUE);
}


void
CAxesDlg::OnLogy() {
  UpdateData(TRUE);
}


void
CAxesDlg::OnAutoy() {
  UpdateData(TRUE);
  m_edYMin.EnableWindow(!m_AutoY);
  m_edYMax.EnableWindow(!m_AutoY);
}


BOOL
CAxesDlg::OnInitDialog() {
	CDialog::OnInitDialog();
  m_edXMin.EnableWindow(!m_AutoX);
  m_edXMax.EnableWindow(!m_AutoX);
  m_edYMin.EnableWindow(!m_AutoY);
  m_edYMax.EnableWindow(!m_AutoY);
  UpdateData(FALSE);
	return TRUE;
}

void CAxesDlg::OnBnClickedOk()
{
  // TODO: aggiungere qui il codice per la gestione della notifica del controllo.
  OnOK();
}

void CAxesDlg::OnBnClickedCancel()
{
  // TODO: aggiungere qui il codice per la gestione della notifica del controllo.
  OnCancel();
}
