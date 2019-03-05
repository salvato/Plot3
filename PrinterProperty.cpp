// PrinterProperty.cpp : file di implementazione
//

#include "stdafx.h"
#include "resource.h"
#include ".\AxisFrame.h"
#include ".\AxisLimits.h"
#include ".\ColorSet.h"
#include ".\SamplePlot.h"
#include ".\PlotColorDlg.h"
#include ".\PrinterProperty.h"


// finestra di dialogo CPrinterProperty

IMPLEMENT_DYNAMIC(CPrinterProperty, CPropertyPage)
CPrinterProperty::CPrinterProperty()
	: CPropertyPage(CPrinterProperty::IDD) {
  memset(&Font, 0, sizeof Font);
	lstrcpy(Font.lfFaceName, _T("Arial"));
	Font.lfHeight         =-24;
	Font.lfWeight					= FW_SEMIBOLD;
	Font.lfOutPrecision   = OUT_TT_PRECIS;
	Font.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
	Font.lfQuality        = PROOF_QUALITY;
	Font.lfPitchAndFamily = FF_SWISS | VARIABLE_PITCH;

	ColorSet.Background = RGB(255,255,255),
  ColorSet.Labels     = RGB(0, 0, 0),
  ColorSet.Axis       = RGB(0, 0, 0),
  ColorSet.CursorPos  = RGB(255, 255, 255),
  ColorSet.Marker     = RGB(255, 255, 255),
  ColorSet.TicLines   = RGB(128, 128, 128),
  ColorSet.Zooming    = RGB(255, 255, 255);

  CWinApp* pApp = AfxGetApp();
  CString strSection = _T("Printer Preferences");
  CString strStringItem = _T("Colors");
  CString strValue = pApp->GetProfileString(strSection, strStringItem);
  int curPos = 0;
  if(strValue != _T("")) {
    //char buf[1025];
    //strcpy(buf, strValue.Left(sizeof(buf)-1));
    ColorSet.Background = COLORREF(_ttoi(strValue.Tokenize(_T(","), curPos)));
    ColorSet.Labels     = COLORREF(_ttoi(strValue.Tokenize(_T(","), curPos)));
    ColorSet.Axis       = COLORREF(_ttoi(strValue.Tokenize(_T(","), curPos)));
    ColorSet.TicLines   = COLORREF(_ttoi(strValue.Tokenize(_T("\0"), curPos)));
    //ColorSet.Background = COLORREF(atoi(strtok(buf, ",")));
    //ColorSet.Labels     = COLORREF(atoi(strtok(0,   ",")));
    //ColorSet.Axis       = COLORREF(atoi(strtok(0,   ",")));
    //ColorSet.TicLines   = COLORREF(atoi(strtok(0,   "\0")));
  }

  curPos = 0;
  strStringItem = _T("Font");
  strValue = pApp->GetProfileString(strSection, strStringItem);
  if(strValue != "") {
    //char buf[1025];
    //strcpy(buf, strValue.Left(sizeof(buf)-1));
    Font.lfHeight         = LONG(_ttoi(strValue.Tokenize(_T(","), curPos)));
    Font.lfWidth          = LONG(_ttoi(strValue.Tokenize(_T(","), curPos)));
    Font.lfEscapement     = LONG(_ttoi(strValue.Tokenize(_T(","), curPos))); 
    Font.lfOrientation    = LONG(_ttoi(strValue.Tokenize(_T(","), curPos))); 
    Font.lfWeight         = LONG(_ttoi(strValue.Tokenize(_T(","), curPos)));
    Font.lfItalic         = BYTE(_ttoi(strValue.Tokenize(_T(","), curPos)));
    Font.lfUnderline      = BYTE(_ttoi(strValue.Tokenize(_T(","), curPos)));
    Font.lfStrikeOut      = BYTE(_ttoi(strValue.Tokenize(_T(","), curPos)));
    Font.lfCharSet        = BYTE(_ttoi(strValue.Tokenize(_T(","), curPos)));
    Font.lfOutPrecision   = BYTE(_ttoi(strValue.Tokenize(_T(","), curPos)));
    Font.lfClipPrecision  = BYTE(_ttoi(strValue.Tokenize(_T(","), curPos)));
    Font.lfQuality        = BYTE(_ttoi(strValue.Tokenize(_T(","), curPos)));
    Font.lfPitchAndFamily = BYTE(_ttoi(strValue.Tokenize(_T(","), curPos)));
    _tcscpy(Font.lfFaceName, strValue.Tokenize(_T("\0"), curPos));
    //Font.lfHeight         = LONG(atoi(strtok(buf, ","))); 
    //Font.lfWidth          = LONG(atoi(strtok(0,   ",")));
    //Font.lfEscapement     = LONG(atoi(strtok(0,   ","))); 
    //Font.lfOrientation    = LONG(atoi(strtok(0,   ","))); 
    //Font.lfWeight         = LONG(atoi(strtok(0,   ","))); 
    //Font.lfItalic         = BYTE(atoi(strtok(0,   ",")));
    //Font.lfUnderline      = BYTE(atoi(strtok(0,   ","))); 
    //Font.lfStrikeOut      = BYTE(atoi(strtok(0,   ","))); 
    //Font.lfCharSet        = BYTE(atoi(strtok(0,   ","))); 
    //Font.lfOutPrecision   = BYTE(atoi(strtok(0,   ","))); 
    //Font.lfClipPrecision  = BYTE(atoi(strtok(0,   ","))); 
    //Font.lfQuality        = BYTE(atoi(strtok(0,   ","))); 
    //Font.lfPitchAndFamily = BYTE(atoi(strtok(0,   ",")));
    //strcpy(Font.lfFaceName, strtok(0, "\0")); 
  }
	
  Plot.ColorSet    = ColorSet;
  Plot.Font = Font;
  Plot.SetPrintMode(true);
}

CPrinterProperty::~CPrinterProperty() {
  CString strValue;
  CWinApp* pApp = AfxGetApp();
  CString strSection = _T("Printer Preferences");

  CString strStringItem = _T("Colors");
  strValue.Format(_T("%d,%d,%d,%d"), 
                  ColorSet.Background,
                  ColorSet.Labels,  
                  ColorSet.Axis,
                  ColorSet.TicLines
                 );
  pApp->WriteProfileString(strSection, strStringItem, strValue);

  strStringItem = _T("Font");
  strValue.Format(_T("%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%d,%s"),
                  Font.lfHeight,
                  Font.lfWidth,
                  Font.lfEscapement,
                  Font.lfOrientation,
                  Font.lfWeight,
                  Font.lfItalic,
                  Font.lfUnderline,
                  Font.lfStrikeOut,
                  Font.lfCharSet,
                  Font.lfOutPrecision,
                  Font.lfClipPrecision,
                  Font.lfQuality,
                  Font.lfPitchAndFamily,
                  Font.lfFaceName
                 );
  pApp->WriteProfileString(strSection, strStringItem, strValue);
}

void 
CPrinterProperty::DoDataExchange(CDataExchange* pDX) {
	CPropertyPage::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PLOT, Plot);
}


BEGIN_MESSAGE_MAP(CPrinterProperty, CPropertyPage)
	ON_BN_CLICKED(IDC_BKG, OnBkg)
	ON_BN_CLICKED(IDC_FRAME, OnFrame)
	ON_BN_CLICKED(IDC_LABELS, OnLabels)
	ON_BN_CLICKED(IDC_TICLINES, OnTicLines)
	ON_BN_CLICKED(IDC_FONT, OnFont)
	ON_WM_CREATE()
END_MESSAGE_MAP()



void 
CPrinterProperty::SetColorSet(CColorSet newColorSet) {
  ColorSet = newColorSet;
}


CColorSet 
CPrinterProperty::GetColorSet() {
  return ColorSet;
}


// gestori di messaggi CPrinterProperty

BOOL
CPrinterProperty::OnInitDialog() {
	CDialog::OnInitDialog();
  UpdateData(false);
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}


void 
CPrinterProperty::OnBkg() {
  CPlotColorDlg ColorDlg(ColorSet.Background, 0, this);
  if(ColorDlg.DoModal() == IDOK) {
    ColorSet.Background = ColorDlg.GetColor();
    Plot.ColorSet = ColorSet;
  }
  RedrawWindow();
}


void 
CPrinterProperty::OnFrame() {
  CPlotColorDlg ColorDlg(ColorSet.Axis, 0, this);
  if(ColorDlg.DoModal() == IDOK) {
    ColorSet.Axis = ColorDlg.GetColor();
    Plot.ColorSet = ColorSet;
  }
  RedrawWindow();
}


void 
CPrinterProperty::OnLabels() {
  CPlotColorDlg ColorDlg(ColorSet.Labels, 0, this);
  if(ColorDlg.DoModal() == IDOK) {
    ColorSet.Labels = ColorDlg.GetColor();
    Plot.ColorSet = ColorSet;
  }
  RedrawWindow();
}


void 
CPrinterProperty::OnTicLines() {
  CPlotColorDlg ColorDlg(ColorSet.TicLines, 0, this);
  if(ColorDlg.DoModal() == IDOK) {
    ColorSet.TicLines = ColorDlg.GetColor();
    Plot.ColorSet = ColorSet;
  }
  RedrawWindow();
}


void 
CPrinterProperty::OnFont() {
  LOGFONT tmpFont = Font;
	CFontDialog Dlg(&tmpFont);
  Dlg.m_cf.rgbColors = ColorSet.Labels;
  if(Dlg.DoModal() == IDOK) {
    Font = tmpFont;
    Plot.Font = Font;
  }
  RedrawWindow();
}


LOGFONT 
CPrinterProperty::GetFont() {
  return Font;
}


int 
CPrinterProperty::OnCreate(LPCREATESTRUCT lpCreateStruct) {
	if (CDialog::OnCreate(lpCreateStruct) == -1)
		return -1;
	return 0;
}


void
CPrinterProperty::SetFont(LOGFONT PrinterFont) {
  Font = PrinterFont;
}

