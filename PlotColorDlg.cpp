// MyColorDlg.cpp : file di implementazione
//

#include "stdafx.h"
#include "resource.h"
#include ".\PlotColorDlg.h"


// CPlotColorDlg

IMPLEMENT_DYNAMIC(CPlotColorDlg, CColorDialog)
CPlotColorDlg::CPlotColorDlg(COLORREF clrInit, DWORD dwFlags, CWnd* pParentWnd) :
	CColorDialog(clrInit, dwFlags, pParentWnd) {
}

CPlotColorDlg::~CPlotColorDlg() {
}


BEGIN_MESSAGE_MAP(CPlotColorDlg, CColorDialog)
END_MESSAGE_MAP()



// gestori di messaggi CPlotColorDlg


BOOL 
CPlotColorDlg::OnInitDialog() {
	CColorDialog::OnInitDialog();
	SetWindowPos(&wndTopMost, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOMOVE|SWP_SHOWWINDOW);
	return TRUE;
}
