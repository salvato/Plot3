#pragma once

class CSamplePlot;
class CColorSet;

// finestra di dialogo CPrinterProperty

class CPrinterProperty : public CPropertyPage {
	DECLARE_DYNAMIC(CPrinterProperty)

public:
	CPrinterProperty();
	virtual ~CPrinterProperty();
	CSamplePlot Plot;

#ifdef _AFXEXT
// Dati della finestra di dialogo
	enum { IDD = IDD_PRINTER_PROPERTIES };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Supporto DDX/DDV

	virtual BOOL OnInitDialog();
	afx_msg void OnBkg();
	afx_msg void OnFrame();
	afx_msg void OnLabels();
	afx_msg void OnTicLines();
	afx_msg void OnFont();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	DECLARE_MESSAGE_MAP()

private:
	int nDataSet;
  CColorSet ColorSet;
#ifdef _UNICODE
	LOGFONTW Font;
#else
	LOGFONT Font;
#endif

public:
	void SetFont(LOGFONT PrinterFont);
	LOGFONT GetFont();
  void SetColorSet(CColorSet ColorSet);
  CColorSet GetColorSet();
};
