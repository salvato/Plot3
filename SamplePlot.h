#pragma once


class CColorSet;
class CAxisLimits;
class CAxisFrame;
class CDataSet;

// CSamplePlot

class CSamplePlot : public CStatic {
	DECLARE_DYNAMIC(CSamplePlot)

public:
	CSamplePlot();
	virtual ~CSamplePlot();

protected:
	afx_msg void OnPaint();
	DECLARE_MESSAGE_MAP()

protected:
	CFont*   pFont;

public:
	void SetPrintMode(bool PrintMode);
	LOGFONT Font;
  void SetShowDataSet(int Id, bool Show);
  void Update();
  CColorSet ColorSet;

private:
	bool bPrintMode;
  CDataSet* NewDataSet(UINT Id, int PenWidth, COLORREF Color, int Symbol, CString Title);
  void SetLimits(double, double, double, double, BOOL, BOOL, BOOL, BOOL);
  void ClearPlot();
  void NewPoint(int Id, double x, double y);
	void XTicLin(CDC* dc);
	void YTicLin(CDC* dc);
  void DrawData(CDC*);
  void LinePlot(CDC* pDC, CDataSet* pData);
  void DrawLastPoint(CDC* pDC, CDataSet* pData);

  CAxisLimits Ax;
	CAxisFrame Pf;
	double xfact, yfact;
  CDataSet* pData;
};


