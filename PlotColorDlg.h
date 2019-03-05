#pragma once


// CPlotColorDlg

class CPlotColorDlg : public CColorDialog
{
	DECLARE_DYNAMIC(CPlotColorDlg)

public:
	CPlotColorDlg(COLORREF clrInit = 0, DWORD dwFlags = 0, CWnd* pParentWnd = NULL);
	virtual ~CPlotColorDlg();

protected:
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
};


