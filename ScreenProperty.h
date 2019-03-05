#pragma once

class CSamplePlot;
class CColorSet;
class CDataSetProperties;
//#include "MyColorDlg.h"
//#include "afxwin.h"

// finestra di dialogo CScreenProperty

class CScreenProperty : public CPropertyPage {
	DECLARE_DYNAMIC(CScreenProperty)

public:
	CScreenProperty();
	virtual ~CScreenProperty();
	CSamplePlot Plot;

#ifdef _AFXEXT
// Dati della finestra di dialogo
	enum { IDD = IDD_SCREEN_PROPERTIES };
#endif
  CComboBox	cDataSet;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // Supporto DDX/DDV
	virtual BOOL OnInitDialog();
	afx_msg void OnBkg();
	afx_msg void OnFrame();
	afx_msg void OnLabels();
	afx_msg void OnMarker();
	afx_msg void OnZooming();
	afx_msg void OnTicLines();
	afx_msg void OnCurPos();
	afx_msg void OnFont();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void OnDefault();
	afx_msg void OnSelchangeDatasets();

	DECLARE_MESSAGE_MAP()

private:
  CColorSet ColorSet;
#ifdef _UNICODE
	LOGFONTW Font;
#else
	LOGFONT Font;
#endif

public:
	void SetFont(LOGFONT ScreenFont);
	void DelDataSet(int Id);
	void AddDataSet(CDataSetProperties DataSetProperties);
	LOGFONT GetFont();
  void SetColorSet(CColorSet ColorSet);
  CColorSet GetColorSet();

	CTypedPtrList<CPtrList, CDataSetProperties*>  PropertiesList;
};
