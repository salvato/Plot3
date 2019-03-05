// PlotWindow.cpp : file di implementazione
//

#include "stdafx.h"
#include "resource.h"
#include <fstream>
#include <math.h>
#include <float.h>
#include "HtmlHelp.h"
#include ".\AxisDlg.h"
#include ".\AxesDlg.h"
#include ".\AxisFrame.h"
#include ".\AxisLimits.h"
#include ".\ColorSet.h"
#include ".\DataSetProperties.h"
#include ".\DataSet.h"
#include ".\SamplePlot.h"
#include ".\PrinterProperty.h"
#include ".\ScreenProperty.h"
#include ".\PlotPoint.h"
#include ".\PlotWindow.h"


#define MSG_PREFERENCES WM_USER + 1
#define MSG_PRINT       WM_USER + 2

#define NINT(x) ((x) > 0.0 ? (int)floor((x)+0.5) : (int)ceil((x)-0.5))

#define MAXPOINTS 100000

#define MOUSETIMER       1
#define CURSORUPDATETIME 100
#define UPDATETIMER      2
#define UPDATETIME       300

#define FIRSTDROPPEDFILENUMBER 0xFFFF

const int CPlotWindow::iline  = 0;
const int CPlotWindow::ipoint = 1;
const int CPlotWindow::iplus  = 2;
const int CPlotWindow::iper   = 3;
const int CPlotWindow::istar  = 4;
const int CPlotWindow::iuptri = 5;
const int CPlotWindow::idntri = 6;
const int CPlotWindow::icirc  = 7;


// CPlotWindow

IMPLEMENT_DYNAMIC(CPlotWindow, CWnd)
CPlotWindow::CPlotWindow(CString Title/*="PlotWindow"*/) {
	while (!dataSetList.IsEmpty()) {
		delete dataSetList.RemoveHead();
	}

  pFont        = NULL;

  xMarker      = 0.0;
  yMarker      = 0.0;
  bShowMarker  = false;
  Zooming      = false;
  bHelpEnabled = false;
  bPrinting    = false;

  x0   = 0;
  y0   = 0;
  wdth = 560;
  hght = 425;

	xCoord.Format(_T("X=% -015.7g"), 0.0);
	yCoord.Format(_T("Y=% -015.7g"), 0.0);

  UINT cStyle= CS_DBLCLKS | CS_HREDRAW | CS_NOCLOSE | CS_OWNDC | CS_VREDRAW;
  CBrush myBrush(ScreenColorSet.Background);
  HBRUSH hbrBackground = HBRUSH(myBrush);
  HICON hIcon = AfxGetApp()->LoadIcon(IDI_ICONPLOT);
  ClassName = AfxRegisterWndClass(cStyle, NULL, hbrBackground, hIcon); 
  DWORD exStyle = WS_EX_DLGMODALFRAME;
  DWORD Style = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
  BOOL result = CreateEx(exStyle, ClassName, Title, Style, x0, y0, wdth, 425, NULL, NULL);
  if(!result) {
    AfxMessageBox(_T("CreteEx failed"));
  }
}

CPlotWindow::~CPlotWindow() {
  KillTimer(MOUSETIMER);
  KillTimer(UPDATETIMER);
	while (!dataSetList.IsEmpty()) {
		delete dataSetList.RemoveHead();
	}
  if(!IsIconic()) {
    CWinApp* pApp = AfxGetApp();
    strSection = "Plot";
    GetWindowText(strStringItem);
    CRect Rect;
    GetWindowRect(&Rect);
    strValue.Format(_T("%d,%d,%d,%d"), Rect.left, Rect.top, Rect.Width(), Rect.Height());  
    pApp->WriteProfileString(strSection, strStringItem, strValue);
  }
  if(pFont != NULL) delete pFont;
  pFont = NULL;
}


BEGIN_MESSAGE_MAP(CPlotWindow, CWnd)
	ON_WM_PAINT()
  ON_WM_MOUSEMOVE()
  ON_WM_LBUTTONDBLCLK()
  ON_WM_LBUTTONDOWN()
  ON_WM_LBUTTONUP()
	ON_WM_SIZE()
	ON_WM_CREATE()
	ON_WM_SETCURSOR()
	ON_WM_CONTEXTMENU()
	ON_WM_MENUSELECT()
	ON_WM_HELPINFO()
  ON_MESSAGE(MSG_PREFERENCES, OnChangePreferences)
  ON_MESSAGE(MSG_PRINT, OnPrintMsg)
	ON_WM_TIMER()
	ON_WM_DROPFILES()
END_MESSAGE_MAP()



// gestori di messaggi CPlotWindow


bool 
CPlotWindow::DelDataSet(int Id) {
  bool result = false;
  if(dataSetList.IsEmpty()) {
    return result;
  }
	CTypedPtrList<CPtrList,CDataSet*> tmpList;  
	POSITION pos = dataSetList.GetHeadPosition();
	while(pos != NULL) {
		CDataSet* pData = dataSetList.GetNext(pos);
    if(pData->GetId() == Id) {
      ScreenProperty.DelDataSet(Id);
		  delete dataSetList.RemoveHead();
      result = true;
    } else {
      tmpList.AddTail(pData);
      dataSetList.RemoveHead();
    }
  }
  pos = tmpList.GetHeadPosition();
	while(pos != NULL) {
    dataSetList.AddTail(tmpList.GetNext(pos));
  }
  return result;
}


void
CPlotWindow::ChangeDataSetColor(int Id, COLORREF Color) {
  if(dataSetList.IsEmpty()) {
    return;
  }
	POSITION pos = dataSetList.GetHeadPosition();
	while(pos != NULL) {
		CDataSet* pData = dataSetList.GetNext(pos);
    if(pData->GetId() == Id) {
      pData->SetColor(Color);
      break;
    }
	}
}


void
CPlotWindow::DrawData(CDC* pDC) {
  COLORREF OldColor;
  if(dataSetList.IsEmpty()) {
    return;
  }
	CBrush brBackground = pDC->GetBkColor();
  pDC->SelectObject(GetStockObject(NULL_BRUSH));
	POSITION pos = dataSetList.GetHeadPosition();
	while(pos != NULL) {
		CDataSet* pData = dataSetList.GetNext(pos);
    if(pData->isShowed) {
      if(pDC->GetDeviceCaps(TECHNOLOGY) == DT_RASPRINTER) {
        OldColor = pData->GetProperties().Color;
        pData->SetColor(RGB(0,0,0));
      }
      if(pData->GetProperties().Symbol == iline) {
        CPen dPen(PS_SOLID, pData->GetProperties().PenWidth, pData->GetProperties().Color);
        DeleteObject(pDC->SelectObject(dPen));
        LinePlot(pDC, pData);
      } else if(pData->GetProperties().Symbol == ipoint) {
        CPen dPen(PS_SOLID, pData->GetProperties().PenWidth, pData->GetProperties().Color);
        DeleteObject(pDC->SelectObject(dPen));
        PointPlot(pDC, pData);
      } else {
        CSize Size(1, 1);
        pDC->DPtoLP(&Size);
        CPen dPen(PS_SOLID, __max(Size.cx, Size.cy), pData->GetProperties().Color);
        DeleteObject(pDC->SelectObject(dPen));
        ScatterPlot(pDC, pData);
      }
      if(pData->bShowCurveTitle) {
        ShowTitle(pDC, pData);
      }
      if(pDC->GetDeviceCaps(TECHNOLOGY) == DT_RASPRINTER) {
        pData->SetColor(OldColor);
      }
    }
  }
}


void 
CPlotWindow::ScatterPlot(CDC* pDC, CDataSet* pData) {
  int iMax = int(pData->m_pointArray.GetUpperBound());
  if(iMax == -1)
    return;
	int ix, iy;
  
  double xlmin, ylmin;
  if(Ax.XMin > 0.0) 
    xlmin = log10(Ax.XMin);
  else
    xlmin = FLT_MIN;
  if(Ax.YMin > 0.0)
    ylmin = log10(Ax.YMin);
  else ylmin = FLT_MIN;

  int SYMBOLS_DIM = 8;
  if(pDC->GetDeviceCaps(TECHNOLOGY) == DT_RASPRINTER) {
    SYMBOLS_DIM = 20;// 3mm in MM_LOMETRIC
  }
  CSize Size(SYMBOLS_DIM, SYMBOLS_DIM);

  for (int i=0; i <= iMax; i++) {
    if(pData->m_pointArray[i].x>=Ax.XMin && 
       pData->m_pointArray[i].x<=Ax.XMax && 
       pData->m_pointArray[i].y>=Ax.YMin && 
       pData->m_pointArray[i].y<=Ax.YMax) {
      if(Ax.LogX)
	      if(pData->m_pointArray[i].x > 0.0)
          ix = int(((log10(pData->m_pointArray[i].x) - xlmin)*xfact) + Pf.l);
        else
          ix =-INT_MAX; // Solo per escludere il punto
      else//Asse X Lineare
  	    ix= int(((pData->m_pointArray[i].x - Ax.XMin)*xfact) + Pf.l);
      if(Ax.LogY) {
	      if(pData->m_pointArray[i].y > 0.0)
          iy = int(((log10(pData->m_pointArray[i].y) - ylmin)*yfact) + Pf.b);
        else
          iy =-INT_MAX; // Solo per escludere il punto
      } else
  	    iy = int(((pData->m_pointArray[i].y - Ax.YMin)*yfact) + Pf.b);
      if(pData->GetProperties().Symbol == iplus) {
        pDC->MoveTo(ix, iy-Size.cy/2);
        pDC->LineTo(ix, iy+Size.cy/2+1);
        pDC->MoveTo(ix-Size.cx/2, iy);
        pDC->LineTo(ix+Size.cx/2+1, iy);
      } else if(pData->GetProperties().Symbol == iper) {
        pDC->MoveTo(ix-Size.cx/2+1, iy+Size.cy/2-1);
        pDC->LineTo(ix+Size.cx/2-1, iy-Size.cy/2);
        pDC->MoveTo(ix+Size.cx/2-1, iy+Size.cy/2-1);
        pDC->LineTo(ix-Size.cx/2+1, iy-Size.cy/2);
      } else if(pData->GetProperties().Symbol == istar) {
        pDC->MoveTo(ix, iy-Size.cy/2);
        pDC->LineTo(ix, iy+Size.cy/2+1);
        pDC->MoveTo(ix-Size.cx/2, iy);
        pDC->LineTo(ix+Size.cx/2+1, iy);
        pDC->MoveTo(ix-Size.cx/2+1, iy+Size.cy/2-1);
        pDC->LineTo(ix+Size.cx/2-1, iy-Size.cy/2);
        pDC->MoveTo(ix+Size.cx/2-1, iy+Size.cy/2-1);
        pDC->LineTo(ix-Size.cx/2+1, iy-Size.cy/2);
      } else if(pData->GetProperties().Symbol == iuptri) {
        pDC->MoveTo(ix, iy-Size.cy/2);
        pDC->LineTo(ix+Size.cx/2, iy+Size.cy/2);
        pDC->LineTo(ix-Size.cx/2, iy+Size.cy/2);
        pDC->LineTo(ix, iy-Size.cy/2);
      } else if(pData->GetProperties().Symbol == idntri) {
        pDC->MoveTo(ix, iy+Size.cy/2);
        pDC->LineTo(ix+Size.cx/2, iy-Size.cy/2);
        pDC->LineTo(ix-Size.cx/2, iy-Size.cy/2);
        pDC->LineTo(ix, iy+Size.cy/2);
      } else if(pData->GetProperties().Symbol == icirc) {
        pDC-> Ellipse(ix-Size.cx/2, iy+Size.cy/2, ix+Size.cx/2, iy-Size.cy/2);
      } else {
        pDC->MoveTo(ix-Size.cx/2, iy);
        pDC->LineTo(ix-Size.cx/2, iy-Size.cy);
        pDC->MoveTo(ix, iy-Size.cy/2);
        pDC->LineTo(ix-Size.cx, iy-Size.cy/2);
      }
    }
  }
}


void 
CPlotWindow::ShowTitle(CDC *pDC, CDataSet* pData) {
  int iYsign = 1;
  if(pDC->GetMapMode() == MM_TEXT) iYsign = -1;
  pDC->SetTextColor(pData->GetProperties().Color);
  CSize Size = pDC->GetOutputTextExtent(pData->GetTitle());
  pDC->TextOut(int(Pf.l+10), int(Pf.t-iYsign*Size.cy*(pData->GetId()+1)), 
               pData->GetTitle(), pData->GetTitle().GetLength());
}


void 
CPlotWindow::PointPlot(CDC * pDC, CDataSet* pData) {
  int iMax = int(pData->m_pointArray.GetUpperBound());
  if(iMax == -1)
    return;
	int ix, iy;
  double xlmin, ylmin;
  if(Ax.XMin > 0.0) 
    xlmin = log10(Ax.XMin);
  else
    xlmin = FLT_MIN;
  if(Ax.YMin > 0.0)
    ylmin = log10(Ax.YMin);
  else ylmin = FLT_MIN;

  for (int i=0; i <= iMax; i++) {
    if(!(pData->m_pointArray[i].x < Ax.XMin ||
       pData->m_pointArray[i].x > Ax.XMax ||
       pData->m_pointArray[i].y < Ax.YMin ||
       pData->m_pointArray[i].y > Ax.YMax )) {
      if(Ax.LogX) {
        if(pData->m_pointArray[i].x > 0.0)
	        ix = int(((log10(pData->m_pointArray[i].x) - xlmin)*xfact) + Pf.l);
        else
          ix =-INT_MAX; // Solo per escludere il punto
      } else
  	    ix = int(((pData->m_pointArray[i].x - Ax.XMin)*xfact) + Pf.l);
      if(Ax.LogY) {
        if(pData->m_pointArray[i].y > 0.0)
          iy = int((Pf.b + (log10(pData->m_pointArray[i].y) - ylmin)*yfact));
        else
          iy =-INT_MAX; // Solo per escludere il punto
      } else
  	    iy = int((Pf.b + (pData->m_pointArray[i].y - Ax.YMin)*yfact));

      if(pDC->GetMapMode() == MM_TEXT) {
        pDC->SetPixelV(ix, iy, pData->GetProperties().Color);
      } else {
        pDC->MoveTo(ix, iy);
        pDC->LineTo(ix+1, iy+1);
      }
    }
	}//for (int i=0; i <= iMax; i++)
}


void 
CPlotWindow::LinePlot(CDC* pDC, CDataSet* pData) {
  int iMax = int(pData->m_pointArray.GetUpperBound());
  if(iMax == -1)
    return;
	int ix, iy;
  double xlmin, ylmin;
  if(Ax.XMin > 0.0) 
    xlmin = log10(Ax.XMin);
  else
    xlmin = FLT_MIN;
  if(Ax.YMin > 0.0)
    ylmin = log10(Ax.YMin);
  else ylmin = FLT_MIN;

  if(Ax.LogX) {
    if(pData->m_pointArray[0].x > 0.0) 
	    ix = int(((log10(pData->m_pointArray[0].x) - xlmin)*xfact) + Pf.l);
    else 
      ix =-INT_MAX; // Solo per escludere il punto
  } else
	  ix = int(((pData->m_pointArray[0].x - Ax.XMin)*xfact) + Pf.l);
  if(Ax.LogY) {
    if(pData->m_pointArray[0].y > 0.0)
  	  iy = int((Pf.b + (log10(pData->m_pointArray[0].y) - ylmin)*yfact));
    else 
      iy =-INT_MAX; // Solo per escludere il punto
  } else
    iy = int((Pf.b + (pData->m_pointArray[0].y - Ax.YMin)*yfact));

  pDC->MoveTo(ix, iy);

  for(int i=1; i <= iMax; i++) {
    if(Ax.LogX)
      if(pData->m_pointArray[i].x > 0.0)
	      ix = int(((log10(pData->m_pointArray[i].x) - xlmin)*xfact) + Pf.l);
    else 
        ix =-INT_MAX; // Solo per escludere il punto
    else
  	  ix = int(((pData->m_pointArray[i].x - Ax.XMin)*xfact) + Pf.l);
    if(Ax.LogY)
      if(pData->m_pointArray[i].y > 0.0)
        iy = int((Pf.b + (log10(pData->m_pointArray[i].y) - ylmin)*yfact));
      else 
        iy =-INT_MAX; // Solo per escludere il punto
    else
  	  iy = int((Pf.b + (pData->m_pointArray[i].y - Ax.YMin)*yfact));
    if(pDC->GetMapMode() == MM_TEXT) {
      if(ix<Pf.l || iy<Pf.t || iy>Pf.b)
        pDC->MoveTo(ix, iy);
      else
        pDC->LineTo(ix, iy);
    } else {
      if(ix<Pf.l || iy>Pf.t || iy<Pf.b)
        pDC->MoveTo(ix, iy);
      else
        pDC->LineTo(ix, iy);
    }
    if(ix>Pf.r){
      break;
    }
    DrawLastPoint(pDC, pData);
	}
}


void
CPlotWindow::DrawLastPoint(CDC* pDC, CDataSet* pData) {
  if(!pData->isShowed)
    return;
	int ix, iy, i;
  i = int(pData->m_pointArray.GetUpperBound());

  double xlmin, ylmin;
  if(Ax.XMin > 0.0) 
    xlmin = log10(Ax.XMin);
  else
    xlmin = FLT_MIN;
  if(Ax.YMin > 0.0)
    ylmin = log10(Ax.YMin);
  else ylmin = FLT_MIN;

  if(Ax.LogX) {
    if(pData->m_pointArray[i].x > 0.0)
	    ix = int(((log10(pData->m_pointArray[i].x) - xlmin)*xfact) + Pf.l);
    else
      return;
  } else
 	  ix = int(((pData->m_pointArray[i].x - Ax.XMin)*xfact) + Pf.l);
  if(Ax.LogY) {
    if(pData->m_pointArray[i].y > 0.0)
	    iy = int((Pf.b + (log10(pData->m_pointArray[i].y) - ylmin)*yfact));
    else
      return;
  } else
   	iy = int((Pf.b + (pData->m_pointArray[i].y - Ax.YMin)*yfact));
    if(ix<=Pf.r && ix>=Pf.l && iy>=Pf.t && iy<=Pf.b)
      pDC->SetPixelV(ix, iy, pData->GetProperties().Color);
	return;
}


void
CPlotWindow::NewPoint(int Id, double x, double y) {
  if(_isnan(x) || _isnan(y)) return;
  if(dataSetList.IsEmpty()) {
    return;
  }
	CDataSet* pData = NULL;
  CDataSet* pTemp;
	POSITION pos = dataSetList.GetHeadPosition();
	while (pos != NULL) {
	  pTemp = dataSetList.GetNext(pos);
    if(pTemp->GetId() == Id) {
      pData = pTemp;
      break;
    }
  }
  if(pData != NULL) {
    if(pData->m_pointArray.GetSize() > MAXPOINTS) {
      CString Title;
      CString Removed = _T(" (Some Data Removed)");
      GetWindowText(Title);
      if(Title.Find(Removed, 0) == -1) {
        SetWindowText(Title + Removed);
      }
       pData->RemoveElements(0, MAXPOINTS/4);// from, howMany
    }
    pData->AddPoint(CPlotPoint(x, y));
  }
}


void
CPlotWindow::SetLimits (double XMin, double XMax, double YMin, double YMax,
                        BOOL AutoX, BOOL AutoY, BOOL LogX, BOOL LogY) {
	Ax.XMin  = XMin;
	Ax.XMax  = XMax;
	Ax.YMin  = YMin;
	Ax.YMax  = YMax;
	Ax.AutoX = AutoX;
	Ax.AutoY = AutoY;
	Ax.LogX  = LogX;
	Ax.LogY  = LogY;

  if(dataSetList.IsEmpty()) goto check;

  if(AutoX | AutoY) {
    bool EmptyData = true;
    if(Ax.AutoX) {
      if(Ax.LogX) {
        XMin = FLT_MAX;
        XMax = FLT_MIN;
      } else {
        XMin = FLT_MAX;
        XMax =-FLT_MAX;
      }
    }
    if(Ax.AutoY) {
      if(Ax.LogY) {
        YMin = FLT_MAX;
        YMax = FLT_MIN;
      } else {
        YMin = FLT_MAX;
        YMax =-FLT_MAX;
      }
    }
	  POSITION pos = dataSetList.GetHeadPosition();
    CDataSet* pData;
	  while(pos != NULL) {
		  pData = dataSetList.GetNext(pos);
      if(pData->isShowed) {
        if(pData->m_pointArray.GetSize() != 0) {
          EmptyData = false;
          if(Ax.AutoX) {
            if(XMin > pData->minx) {
              XMin = pData->minx;
            }
            if(XMax < pData->maxx) {
              XMax = pData->maxx;
            }
          }// if(Ax.AutoX)
          if(Ax.AutoY) {
            if(YMin > pData->miny) {
              YMin = pData->miny;
            }
            if(YMax < pData->maxy) {
              YMax = pData->maxy;
            }
          }// if(Ax.AutoY)
        }// if(pData->m_pointArray.GetSize() != 0)
      }// if(pData->isShowed)
	  }// while (pos != NULL)
    if(EmptyData) {
      XMin = Ax.XMin;
      XMax = Ax.XMax;
      YMin = Ax.YMin;
      YMax = Ax.YMax;
    }
  }
check:
  if(XMin == XMax) {
	  XMin  -= 0.05*(XMax+XMin)+FLT_MIN;
    XMax  += 0.05*(XMax+XMin)+FLT_MIN;
  }

  if(YMin == YMax) {
    YMin  -= 0.05*(YMax+YMin)+FLT_MIN;
    YMax  += 0.05*(YMax+YMin)+FLT_MIN;
  }

  if(XMin > XMax) {
    double tmp = XMin;
 	  XMin = XMax;
    XMax = tmp;
  }
  if(YMin > YMax) {
    double tmp = YMin;
	  YMin = YMax;
    YMax = tmp;
  }
  if(LogX) {
    if(XMin <= 0.0) XMin = FLT_MIN;
    if(XMax <= 0.0) XMax = 2.0*FLT_MIN;
  }
  if(LogY) {
    if(YMin <= 0.0) YMin = FLT_MIN;
    if(YMax <= 0.0) YMax = 2.0*FLT_MIN;
  }
  Ax.XMin  = XMin;
  Ax.XMax  = XMax;
  Ax.YMin  = YMin;
  Ax.YMax  = YMax;
}


void
CPlotWindow::GetLimits (double* XMin, double* XMax, double* YMin, double* YMax, BOOL *AutoX, BOOL *AutoY, BOOL *LogX, BOOL *LogY) {
	*XMin  = Ax.XMin;
	*XMax  = Ax.XMax;
	*YMin  = Ax.YMin;
	*YMax  = Ax.YMax;
  *AutoX = Ax.AutoX;
  *AutoY = Ax.AutoY;
  *LogX  = Ax.LogX;
  *LogY  = Ax.LogY;
}


CDataSet*
CPlotWindow::NewDataSet(UINT Id, int PenWidth, COLORREF Color, int Symbol, CString Title) {
  CDataSet* pDataItem = new CDataSet(Id, PenWidth, Color, Symbol, Title);
  dataSetList.AddTail(pDataItem);
  ScreenProperty.AddDataSet(CDataSetProperties(Id, PenWidth, Color, Symbol, Title));
  return pDataItem;
}


CDataSet*
CPlotWindow::NewDataSet(CDataSetProperties myProperties) {
  CDataSet* pDataItem = new CDataSet(myProperties);
  dataSetList.AddTail(pDataItem);
  ScreenProperty.AddDataSet(myProperties);
  return pDataItem;
}


void
CPlotWindow::DelayedUpdate() {
  if(bPrinting){
    return;
  }
  if(IsIconic())
    return;
  SetTimer(UPDATETIMER, UPDATETIME, NULL);
}


void
CPlotWindow::UpdatePlot() {
  if(bPrinting){
    return;
  }
  if(IsIconic())
    return;
  if(m_Plot.m_hObject != NULL)
    m_Plot.DeleteObject(); //Cancella la vecchia Bitmap del Plot

  if(Ax.AutoX || Ax.AutoY) {
    SetLimits (Ax.XMin, Ax.XMax, Ax.YMin, Ax.YMax, Ax.AutoX, Ax.AutoY, Ax.LogX, Ax.LogY);
  }

  CClientDC dc(this);
  dc.SetMapMode(MM_TEXT);
  dc.SetBkColor(ScreenColorSet.Background);
  CSize PointSize = CSize(1,1);
  dc.DPtoLP(&PointSize);// Quanti Logical Points per Pixel ?
  int PenSize = int(__max(abs(PointSize.cx), abs(PointSize.cy)));

	CDC dcMem;
	dcMem.CreateCompatibleDC(&dc);
  dcMem.SetBkColor(dc.GetBkColor());
	CRect rect;
  GetClientRect(rect);

	m_Plot.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
	ASSERT(m_Plot.m_hObject != NULL);

  dcMem.SetMapMode(MM_TEXT);
  dcMem.SetBkMode(OPAQUE);
	dcMem.SetTextColor(ScreenColorSet.Labels);
  dcMem.SetTextAlign(TA_TOP | TA_LEFT);

  CBitmap* pOldPlot = dcMem.SelectObject(&m_Plot);

  // draw a rectangle in the background color
  CBrush brBackground = dcMem.GetBkColor();
  dcMem.FillRect(rect, &brBackground);

  if(pFont == NULL) pFont = new CFont();
  pFont->CreateFontIndirect(&ScreenFont);

  pOldFont = dcMem.SelectObject(pFont);
  LabelSize = dcMem.GetTextExtent(_T("-0.0000"));
  dcMem.DPtoLP(&LabelSize);

  Pf.l = LabelSize.cx + rect.left + 10;
  Pf.r = rect.right   - LabelSize.cx;
  Pf.t = rect.top     + 3*LabelSize.cy;
  Pf.b = rect.bottom  - 3*LabelSize.cy;

  CPen Pen(PS_SOLID, PenSize, ScreenColorSet.TicLines);
  dcMem.SelectObject(Pen);
  if(Ax.LogX)
    XTicLog(&dcMem);
  else
    XTicLin(&dcMem);
  if(Ax.LogY)
    YTicLog(&dcMem);
  else
    YTicLin(&dcMem);

  CPen aPen(PS_SOLID, PenSize, ScreenColorSet.Axis);
  dcMem.SelectObject(aPen);

  DeleteObject(Pen);
  dcMem.MoveTo(int(Pf.l), int(Pf.b));
  dcMem.LineTo(int(Pf.r), int(Pf.b));
  dcMem.LineTo(int(Pf.r), int(Pf.t));
  dcMem.LineTo(int(Pf.l), int(Pf.t));
  dcMem.LineTo(int(Pf.l), int(Pf.b));

  if(!dataSetList.IsEmpty()) {
    DrawData(&dcMem);
  }
  dcMem.SetTextColor(ScreenColorSet.CursorPos);
  LabelSize = dcMem.GetTextExtent(_T("00000000000000000"));
  dcMem.TextOut(int(Pf.l), rect.bottom-LabelSize.cy, xCoord, int(_tcslen(xCoord)));
  dcMem.TextOut(int(Pf.l+LabelSize.cx), rect.bottom-LabelSize.cy, yCoord, int(_tcslen(yCoord)));

  if(bShowMarker) {
	  int ix, iy;
    double xlmin, ylmin;
    if(Ax.XMin > 0.0) 
      xlmin = log10(Ax.XMin);
    else
      xlmin = FLT_MIN;
    if(Ax.YMin > 0.0)
      ylmin = log10(Ax.YMin);
    else ylmin = FLT_MIN;

    if(xMarker>=Ax.XMin && xMarker<=Ax.XMax && yMarker>=Ax.YMin && yMarker<=Ax.YMax) {
      if(Ax.LogX)
	      ix = int(((log10(xMarker) - xlmin)*xfact) + Pf.l);
      else
       ix = int(((xMarker - Ax.XMin)*xfact) + Pf.l);
      if(Ax.LogY)
        iy = int(((log10(yMarker) - ylmin)*yfact) + Pf.b);
      else
       iy = int(((yMarker - Ax.YMin)*yfact)+ Pf.b);
      CPen mPen (PS_SOLID, 1, ScreenColorSet.Marker);
      dcMem.SelectObject(mPen);
      DeleteObject(aPen);
      dcMem.MoveTo(ix-4, iy-4);
      dcMem.LineTo(ix, iy);
      dcMem.MoveTo(ix+4, iy-4);
      dcMem.LineTo(ix, iy);
      dcMem.MoveTo(ix-4, iy+4);
      dcMem.LineTo(ix, iy);
      dcMem.MoveTo(ix+4, iy+4);
      dcMem.LineTo(ix, iy);
      dcMem.MoveTo(ix, iy-15);
      dcMem.LineTo(ix, iy);
      dcMem.MoveTo(ix, iy+15);
      dcMem.LineTo(ix, iy);
    }
  }
  dc.BitBlt(0, 0,rect.Width(), rect.Height(), &dcMem, 0, 0, SRCCOPY);

  dcMem.SelectObject(pOldFont);
  DeleteObject(pFont);
  delete pFont;
  pFont = NULL;
  dcMem.DeleteDC();
}


void
CPlotWindow::RedrawPlot() {
	CRect rc;
	GetClientRect(rc);
	InvalidateRect(rc, TRUE);
}


void
CPlotWindow::SetShowDataSet(int Id, bool Show) {
  if(!dataSetList.IsEmpty()) {
	  POSITION pos = dataSetList.GetHeadPosition();
	  while(pos != NULL) {
		  CDataSet* pData = dataSetList.GetNext(pos);
      if(pData->GetId() == Id) {
        pData->SetShow(Show);
        break;
      }
	  }
  }
}


void
CPlotWindow::ClearPlot() {
  if(dataSetList.IsEmpty()) {
    return;
  }
	POSITION pos = dataSetList.GetHeadPosition();
	while(!dataSetList.IsEmpty()) {
		CDataSet* pData = dataSetList.GetAt(pos);
    ScreenProperty.DelDataSet(pData->GetId());
    delete dataSetList.RemoveHead();
    pos = dataSetList.GetHeadPosition();
  }
  nFilesDropped = 0;
  UpdatePlot();
}


void 
CPlotWindow::SetShowMarker(bool Show) {
  bShowMarker = Show;
}


void 
CPlotWindow::SetMarkerPos(double x, double y) {
  xMarker = x;
  yMarker = y;
}


/////////////////////////////////////////////////////////////////////////////
///////////////////////// Fine Membri Pubblici //////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////// Membri Privati //////////////////////////////////
/////////////////////////////////////////////////////////////////////////////


void
CPlotWindow::XTicLin(CDC* dc) {

  CSize Size;
	double xmax, xmin;
	double dx, dxx, b, fmant;
	int isx, ic, iesp, jy, isig, ix, ix0, iy0;
	CString Label;

  int iYsign = 1;
  if(dc->GetMapMode() == MM_TEXT) iYsign = -1;

	if (Ax.XMax <= 0.0) {
		xmax = -Ax.XMin;	xmin= -Ax.XMax; isx= -1;
	} else {
		xmax = Ax.XMax; xmin= Ax.XMin; isx= 1;
	}
	dx = xmax - xmin;
	b = log10(dx);
	ic = NINT(b) - 2;
	dx = (double)NINT(pow(10.0, (b-ic-1.0)));

	if(dx < 11.0) dx = 10.0;
	else if(dx < 28.0) dx = 20.0;
	else if(dx < 70.0) dx = 50.0;
	else dx = 100.0;

	dx = dx * pow(10.0, (double)ic);
	xfact = (Pf.r-Pf.l) / (xmax-xmin);
	dxx = (xmax+dx) / dx;
	dxx = floor(dxx) * dx;
  Size = dc->GetTextExtent(_T("X"));
	iy0 = int(Pf.b - iYsign*Size.cy*0.5);
	iesp = int(floor(log10(dxx)));
	if (dxx > xmax) dxx = dxx - dx;
	do {
		if(isx == -1)
			ix = int(Pf.r-(dxx-xmin) * xfact);
		else
			ix = int((dxx-xmin) * xfact + Pf.l);
		jy = int(Pf.b - iYsign*5);// Perche' 5 ?
    dc->MoveTo(ix, int(Pf.t));
		dc->LineTo(ix, jy);
		isig = 0;
		if(dxx == 0.0)
			fmant= 0.0;
		else {
			isig = int(dxx/fabs(dxx));
			dxx = fabs(dxx);
			fmant = log10(dxx) - double(iesp);
			fmant = pow(10.0, fmant)*10000.0 + 0.5;
			fmant = floor(fmant)/10000.0;
			fmant = isig * fmant;
		}
    if((double)isx*fmant <= -10.0)
      Label.Format(_T("% 6.2f"), (double)isx*fmant);
    else
		  Label.Format(_T("% 6.3f"), (double)isx*fmant);
    Size = dc->GetTextExtent(Label);
		ix0 = ix - Size.cx/2;
		dc->TextOut(ix0, iy0, Label, int(_tcslen(Label)));
		dxx = isig*dxx - dx;
	}	while(dxx >= xmin);
  Size = dc->GetTextExtent(_T("*10 "));
	dc->TextOut(int(Pf.r + 2),	int(Pf.b + iYsign*Size.cy), _T("*10"), 3);
  int icx = Size.cx;
	Label.Format(_T("%-3i"), iesp);
  Size = dc->GetTextExtent(Label);
	dc->TextOut(int(Pf.r + icx), int(Pf.b+iYsign*1.5*Size.cy), Label, Label.GetLength());
}


void
CPlotWindow::YTicLin(CDC* dc) {
  CSize Size;
	double ymax, ymin;
	double dy, dyy, b, fmant;
	int isy, icc, iesp, jx, isig, iy, ix0, iy0;
	CString Label;

  int iYsign = 1;
  if(dc->GetMapMode() == MM_TEXT) iYsign = -1;

	if (Ax.YMax <= 0.0) {
		ymax = -Ax.YMin; ymin= -Ax.YMax; isy= -1;
	} else {
		ymax = Ax.YMax; ymin= Ax.YMin; isy= 1;
	}
	dy = ymax - ymin;
	b = log10(dy);
	icc = NINT(b) - 2;
	dy = (double)NINT(pow(10.0, (b-icc-1.0)));

	if(dy < 11.0) dy = 10.0;
	else if(dy < 28.0) dy = 20.0;
	else if(dy < 70.0) dy = 50.0;
	else dy = 100.0;

	dy = dy * pow(10.0, (double)icc);
	yfact = (Pf.t-Pf.b) / (ymax-ymin);
	dyy = (ymax+dy) / dy;
	dyy = floor(dyy) * dy;
	iesp = int(floor(log10(dyy)));
	if(dyy > ymax) dyy = dyy - dy;
	do {
		if(isy == -1)
			iy = int(Pf.t - (dyy-ymin) * yfact);
		else
			iy = int((dyy-ymin) * yfact + Pf.b);
		jx = int(Pf.r);
		dc->MoveTo(int(Pf.l-5), iy);
		dc->LineTo(jx, iy);
		isig = 0;
		if(dyy == 0.0)
			fmant = 0.0;
		else{
			isig = int(dyy/fabs(dyy));
			dyy = fabs(dyy);
			fmant = log10(dyy) - double(iesp);
			fmant = pow(10.0, fmant)*10000.0 + 0.5;
			fmant = floor(fmant)/10000.0;
			fmant = isig * fmant;
		}
    if((double)isy*fmant <= -10.0)
		  Label.Format(_T("% 7.3f"), (double)isy*fmant);
    else
  		Label.Format(_T("% 7.4f"), (double)isy*fmant);
    Size = dc->GetTextExtent(Label);
	  ix0 = int(Pf.l - Size.cx - 5);
		iy0 = iy + iYsign*Size.cy/2;
		dc->TextOut(ix0, iy0, Label, Label.GetLength());
		dyy = isig*dyy - dy;
	}	while (dyy >= ymin);
  ix0 = int(Pf.l);
  Size = dc->GetTextExtent(_T("*10 "));
	dc->TextOut(ix0, int(Pf.t+iYsign*(1.5*Size.cy)),_T("*10"), 3);
  int icx = Size.cx;
	Label.Format(_T("%-3i"), iesp);
	dc->TextOut(int(ix0+icx),int(Pf.t+2*iYsign*Size.cy),Label, Label.GetLength());
}


void
CPlotWindow::XTicLog(CDC* dc) {
  CSize Size;
  int i, ix, ix0, iy0, jy, j;
  double dx;
	CString Label;

  int iYsign = 1;
  if(dc->GetMapMode() == MM_TEXT) iYsign = -1;
  jy = int(Pf.b - iYsign*5);// Perche' 5 ?

  Size = dc->GetTextExtent(_T("X"));
	iy0 = int(Pf.b - iYsign*Size.cy*0.5);

  if(Ax.XMin < FLT_MIN) Ax.XMin = FLT_MIN;
  if(Ax.XMax < FLT_MIN) Ax.XMax = 10.0*FLT_MIN;

  double xlmin = log10(Ax.XMin);
  int minx = int(xlmin);
  if((xlmin < 0.0) && (xlmin != minx)) minx= minx - 1;

  double xlmax = log10(Ax.XMax);
  int maxx = int(xlmax);
  if((xlmax > 0.0) && (xlmax != maxx)) maxx= maxx + 1;
  
  xfact= (Pf.r-Pf.l) / ((xlmax-xlmin)+FLT_MIN);

  bool init = true;
  int decades = maxx - minx;
  double x = pow(10.0, minx);
  if(decades < 6) {
    for(i=0; i<decades; i++) {
      dx = pow(10.0, (minx + i));
      if(x >= Ax.XMin) {
        ix = int(Pf.l + (log10(x)-xlmin)*xfact);
        Label.Format(_T("%7.0e"), x);
        Size = dc->GetTextExtent(Label);
		    ix0 = ix - Size.cx/2;
	      dc->TextOut(ix0, iy0, Label, Label.GetLength());
        init = false;
      }
      for(j=1; j<10; j++){
        x = x + dx;
        if((x >= Ax.XMin) && (x <= Ax.XMax)) {
          ix = int(Pf.l + (log10(x)-xlmin)*xfact);
          dc->MoveTo(ix, int(Pf.t));
          dc->LineTo(ix, jy);
          Label.Format(_T("%7.0e"), x);
          if(init || (j == 9 && decades == 1)) {
            Size = dc->GetTextExtent(Label);
		        ix0 = ix - Size.cx/2;
  	        dc->TextOut(ix0, iy0, Label, Label.GetLength());
            init = false;
          } else if (decades == 1) {
            Label = Label.Left(2);
            Size = dc->GetTextExtent(Label);
		        ix0 = ix - Size.cx/2;
            dc->TextOut(ix0, iy0, Label, Label.GetLength());
          }
        }
      }
    }// for(i=0; i<decades; i++)
    if((decades != 1) && (x <= Ax.XMax)) {
      Label.Format(_T("%7.0e"), x);
      ix = int(Pf.l + (log10(x)-xlmin)*xfact);
      Size = dc->GetTextExtent(Label);
		  ix0 = ix - Size.cx/2;
      dc->TextOut(ix0, iy0, Label, Label.GetLength());
    }
  } else {// decades > 5
    for(i=1; i<=decades; i++) {
      x = pow(10.0, minx + i);
      if((x >= Ax.XMin) && (x <= Ax.XMax)) {
        ix = int(Pf.l + (log10(x)-xlmin)*xfact);
        dc->MoveTo(ix, int(Pf.t));
        dc->LineTo(ix, jy);
        Label.Format(_T("%7.0e"), x);
        Size = dc->GetTextExtent(Label);
        ix0 = ix - Size.cx/2;
        dc->TextOut(ix0, iy0, Label, Label.GetLength());
      }
    }
  }//if(decades < 6)
}


void
CPlotWindow::YTicLog(CDC* dc) {
  CSize Size;
  int i, iy, ix0, iy0, j;
  double dy;
	CString Label;

  int iYsign = 1;
  if(dc->GetMapMode() == MM_TEXT) iYsign = -1;

  if(Ax.YMin < FLT_MIN) Ax.YMin = FLT_MIN;
  if(Ax.YMax < FLT_MIN) Ax.YMax = 10.0*FLT_MIN;

  double ylmin = log10(Ax.YMin);
  int miny = int(ylmin);
  if((ylmin < 0.0) && (ylmin != miny)) miny= miny - 1;

  double ylmax = log10(Ax.YMax);
  int maxy = int(ylmax);
  if((ylmax > 0.0) && (ylmax != maxy)) maxy= maxy + 1;

  yfact = (Pf.t-Pf.b) / ((ylmax-ylmin)+FLT_MIN);

  bool init = true;
  int decades = maxy - miny;
  double y = pow(10.0, miny);
  if(decades < 6) {
    for(i=0; i<decades; i++) {
      dy = pow(10.0, (miny + i));
      if(y >= Ax.YMin) {
        iy = int(Pf.b + (log10(y)-ylmin)*yfact);
	      Label.Format(_T("%7.0e"), y);
        Size = dc->GetTextExtent(Label);
        ix0 = int(Pf.l - Size.cx - 5);
        iy0 = iy +iYsign*Size.cy/2;
	      dc->TextOut(ix0, iy0, Label, Label.GetLength());
        init = false;
      }
      for(j=1; j<10; j++){
        y = y + dy;
        if((y >= Ax.YMin) && (y <= Ax.YMax)) {
          iy = int(Pf.b + (log10(y)-ylmin)*yfact);
          dc->MoveTo(int(Pf.l-5), iy);
          dc->LineTo(int(Pf.r), iy);
          Label.Format(_T("%7.0e"), y);
          if(init || (j == 9 && decades == 1)) {
            Size = dc->GetTextExtent(Label);
            ix0 = int(Pf.l - Size.cx - 5);
            iy0 = iy + iYsign*Size.cy/2;
  	        dc->TextOut(ix0, iy0, Label, Label.GetLength());
            init = false;
          } else if (decades == 1) {
            Label = Label.Left(2);
            Size = dc->GetTextExtent(Label);
            ix0 = int(Pf.l - Size.cx - 5);
            iy0 = iy + iYsign*Size.cy/2;
            dc->TextOut(ix0, iy0, Label, Label.GetLength());
          }
        }
      }
    }// for(i=0; i<decades; i++)
    if((decades != 1) && (y <= Ax.YMax)) {
      Label.Format(_T("%7.0e"), y);
      iy = int(Pf.b - (log10(y)-ylmin)*yfact);
      Size = dc->GetTextExtent(Label);
      ix0 = int(Pf.l - Size.cx - 5);
      iy0 = iy + iYsign*Size.cy/2;
      dc->TextOut(ix0, iy0, Label, Label.GetLength());
    }
  } else {// decades > 5
    for(i=1; i<=decades; i++) {
      y = pow(10.0, miny + i);
      if((y >= Ax.YMin) && (y <= Ax.YMax)) {
        iy = int(Pf.b + (log10(y)-ylmin)*yfact);
        dc->MoveTo(int(Pf.l-5), iy);
        dc->LineTo(int(Pf.r), iy);
        Label.Format(_T("%7.0e"), y);
        Size = dc->GetTextExtent(Label);
        ix0 = int(Pf.l - Size.cx - 5);
        iy0 = iy + iYsign*Size.cy/2;
        dc->TextOut(ix0, iy0, Label, Label.GetLength());
      }
    }
  }//if(decades < 6)
}

/////////////////////////////////////////////////////////////////////////////
////////////////////////// Fine Membri Privati //////////////////////////////
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////// Messaggi ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

afx_msg void
CPlotWindow::OnMouseMove(UINT nFlags, CPoint point) {
  if((nFlags & MK_LBUTTON) && !Zooming) {
    double xmin, xmax, ymin, ymax;
    double dxPix = point.x - xold;
    double dyPix = point.y - yold;
    double dx    = dxPix / xfact;
    double dy    = dyPix / yfact;
    if(Ax.LogX) {
      xmin = pow(10.0, log10(Ax.XMin)-dx);
      xmax = pow(10.0, log10(Ax.XMax)-dx);
    } else {
      xmin = Ax.XMin - dx;
      xmax = Ax.XMax - dx;
    }
    if(Ax.LogY) {
      ymin = pow(10.0, log10(Ax.YMin)-dy);
      ymax = pow(10.0, log10(Ax.YMax)-dy);
    } else {
      ymin = Ax.YMin - dy;
      ymax = Ax.YMax - dy;
    }
    xold = point.x;
    yold = point.y;
    SetLimits (xmin, xmax, ymin, ymax, Ax.AutoX, Ax.AutoY, Ax.LogX, Ax.LogY);
    UpdatePlot();
  } else if((nFlags & MK_LBUTTON) && Zooming) {
    UpdatePlot();
  	CClientDC dc(this);
    CPen Pen(PS_SOLID, 1, ScreenColorSet.Zooming);
    dc.SelectObject(Pen);
    dc.MoveTo(xr, yr);
    dc.LineTo(point.x, yr);
    dc.LineTo(point.x, point.y);
    dc.LineTo(xr, point.y);
    dc.LineTo(xr, yr);
    DeleteObject(Pen);
  } else {
    double xval, yval;
    if(Ax.LogX) {
      xval = pow(10.0, log10(Ax.XMin)+(point.x-Pf.l)/xfact);
    }else {
      xval =Ax.XMin + (point.x-Pf.l) / xfact;
    }
    if(Ax.LogY) {
      yval = pow(10.0, log10(Ax.YMin)+(point.y-Pf.b)/yfact);
    } else {
      yval =Ax.YMin + (point.y-Pf.b) / yfact;
    }
	  xCoord.Format(_T("X=% -010.7g"), xval);
	  yCoord.Format(_T("Y=% -010.7g"), yval);
    SetTimer(MOUSETIMER, CURSORUPDATETIME, NULL);
  }
}


afx_msg void
CPlotWindow::OnLButtonDblClk(UINT nFlags, CPoint point) {
  bool ScaleChanged = false;
  if(abs(point.x-Pf.l) < 5) {
    CAxisDlg AxisDlg(this);
    AxisDlg.m_MaxLabel = "YMax";
    AxisDlg.m_MinLabel = "YMin";
    AxisDlg.m_Max      = Ax.YMax;
    AxisDlg.m_Min      = Ax.YMin;
    AxisDlg.m_Auto     = Ax.AutoY;
    AxisDlg.m_Log      = Ax.LogY;
    if(AxisDlg.DoModal() == IDOK) {
      if(AxisDlg.m_Max > AxisDlg.m_Min) {
        Ax.YMax  = AxisDlg.m_Max;
        Ax.YMin  = AxisDlg.m_Min;
        Ax.AutoY = AxisDlg.m_Auto;
        Ax.LogY  = AxisDlg.m_Log;
        ScaleChanged = true;
      } else {
        MessageBeep(MB_ICONHAND);
      }
    }// if(AxisDlg.DoModal() == IDOK)
  } else if(abs(point.y-Pf.b) < 5){
    CAxisDlg AxisDlg(this);
    AxisDlg.m_MaxLabel = "XMax";
    AxisDlg.m_MinLabel = "XMin";
    AxisDlg.m_Max      = Ax.XMax;
    AxisDlg.m_Min      = Ax.XMin;
    AxisDlg.m_Auto     = Ax.AutoX;
    AxisDlg.m_Log      = Ax.LogX;
    if(AxisDlg.DoModal() == IDOK) {
      if(AxisDlg.m_Max > AxisDlg.m_Min) {
        Ax.XMax  = AxisDlg.m_Max;
        Ax.XMin  = AxisDlg.m_Min;
        Ax.AutoX = AxisDlg.m_Auto;
        Ax.LogX  = AxisDlg.m_Log;
        ScaleChanged = true;
      } else {
        MessageBeep(MB_ICONHAND);
     }
    }// if(AxisDlg.DoModal() == IDOK)
  } else {
    CAxesDlg AxesDlg(this);
    AxesDlg.m_XMax = Ax.XMax;
    AxesDlg.m_XMin = Ax.XMin;
    AxesDlg.m_YMax = Ax.YMax;
    AxesDlg.m_YMin = Ax.YMin;
    AxesDlg.m_AutoX= Ax.AutoX;
    AxesDlg.m_AutoY= Ax.AutoY;
    AxesDlg.m_LogX = Ax.LogX;
    AxesDlg.m_LogY = Ax.LogY;
    if(AxesDlg.DoModal() == IDOK) {
      if(AxesDlg.m_XMax > AxesDlg.m_XMin &&
        AxesDlg.m_YMax > AxesDlg.m_YMin) {
        Ax.XMax = AxesDlg.m_XMax;
        Ax.XMin = AxesDlg.m_XMin;
        Ax.YMax = AxesDlg.m_YMax;
        Ax.YMin = AxesDlg.m_YMin;
        Ax.AutoX = AxesDlg.m_AutoX;
        Ax.AutoY = AxesDlg.m_AutoY;
        Ax.LogX  = AxesDlg.m_LogX;
        Ax.LogY  = AxesDlg.m_LogY;
        ScaleChanged = true;
      } else {
        MessageBeep(MB_ICONHAND);
      }
    }
  }
  if(ScaleChanged) {
    SetLimits(Ax.XMin, Ax.XMax, Ax.YMin, Ax.YMax, Ax.AutoX, Ax.AutoY, Ax.LogX, Ax.LogY);
    UpdatePlot();
  }
}


afx_msg void
CPlotWindow::OnLButtonDown(UINT nFlags, CPoint point) {
  if(nFlags & MK_SHIFT) {
    xr = point.x;
    yr = point.y;
    myCursor = AfxGetApp()->LoadCursor(IDC_PLOTLENS);
    SetCursor(myCursor);
    Zooming = true;
  } else {
    myCursor = AfxGetApp()->LoadCursor(IDC_PLOTHAND);
    SetCursor(myCursor);
    xold = point.x;
    yold = point.y;
  }
}


afx_msg void
CPlotWindow::OnLButtonUp(UINT nFlags, CPoint point) {
  if(Zooming) {
    Zooming = false;
    myCursor = AfxGetApp()->LoadCursor(IDC_PLOTCROSS);
    SetCursor(myCursor);
    if(abs(xr-point.x) < 10 || abs(yr-point.y) < 10) return;
    double x1, x2, y1, y2, tmp;
    if(Ax.LogX) {
      x1 = pow(10.0, log10(Ax.XMin)+(point.x-Pf.l)/xfact);
      x2 = pow(10.0, log10(Ax.XMin)+(xr-Pf.l)/xfact);
    } else {
      x1 = (point.x-Pf.l)/xfact + Ax.XMin;
      x2 = (xr-Pf.l)/xfact + Ax.XMin;
    }
    if(Ax.LogY) {
      y1 = pow(10.0, log10(Ax.YMin)+(point.y-Pf.b)/yfact);
      y2 = pow(10.0, log10(Ax.YMin)+(yr-Pf.b)/yfact);
    } else {
      y1 = (point.y-Pf.b) / yfact + Ax.YMin;
      y2 = (yr-Pf.b) / yfact + Ax.YMin;
    }
    if(x2<x1) {
      tmp = x2;
      x2 = x1;
      x1 = tmp;
    }
    if(y2<y1) {
      tmp = y2;
      y2 = y1;
      y1 = tmp;
    }
    SetLimits (x1, x2, y1, y2, Ax.AutoX, Ax.AutoY, Ax.LogX, Ax.LogY);
    UpdatePlot();
  } else {
    myCursor = AfxGetApp()->LoadCursor(IDC_PLOTCROSS);
    SetCursor(myCursor);
  }
}


afx_msg void
CPlotWindow::OnSize( UINT nType, int cx, int cy ) {
	CWnd::OnSize(nType, cx, cy);
	CPaintDC dc(this); // device context for painting
	UpdatePlot();
}


afx_msg void
CPlotWindow::OnPaint() {
	CPaintDC dc(this); // device context for painting
  UpdatePlot();	
}


int 
CPlotWindow::OnCreate(LPCREATESTRUCT lpCreateStruct) {
  
  if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	myCursor = AfxGetApp()->LoadCursor(IDC_PLOTCROSS);
  RECT rDeskTop;
  ::GetWindowRect(::GetDesktopWindow(), &rDeskTop);
  
  int xx0, yy0, xwdth, yhght;
  CWinApp* pApp = AfxGetApp();

  strSection = _T("Plot");
  GetWindowText(strStringItem);
  strValue = pApp->GetProfileString(strSection, strStringItem);
  int curPos=0;
  try {
    if(strValue != _T("")) {
      xx0   = _ttoi(strValue.Tokenize(_T(","), curPos));
      yy0   = _ttoi(strValue.Tokenize(_T(","), curPos));
      xwdth = _ttoi(strValue.Tokenize(_T(","), curPos));
      yhght = _ttoi(strValue.Tokenize(_T("\0"), curPos));
      //char buf[1025];
      //strcpy(buf, strValue.Left(sizeof(buf)-1));
      //xx0   = atoi(strtok(buf, ","));
      //yy0   = atoi(strtok(0, ","));
      //xwdth = atoi(strtok(0, ","));
      //yhght = atoi(strtok(0, "\0"));
      if((xx0>0) && (yy0>0) && (xx0<rDeskTop.right-20) && (yy0<rDeskTop.bottom-20)) {
        x0 = xx0;
        y0 = yy0;
        wdth = xwdth;
        hght = yhght;
      }
      strValue.Format(_T("%d,%d,%d,%d"), x0, y0, wdth, hght);  
      pApp->WriteProfileString(strSection, strStringItem, strValue);
    }
  } catch (...) {
  }


  ScreenProperty.m_psp.dwFlags &= ~PSP_HASHELP;
  PrinterProperty.m_psp.dwFlags &= ~PSP_HASHELP;
  Preferences.AddPage(&ScreenProperty);
  Preferences.AddPage(&PrinterProperty);
  ScreenFont      = ScreenProperty.GetFont();
  PrinterFont     = PrinterProperty.GetFont();
  ScreenColorSet  = ScreenProperty.GetColorSet();
  PrinterColorSet = PrinterProperty.GetColorSet();

  nFilesDropped = 0;
  DragAcceptFiles(TRUE);

  CRect Rect;
  Rect.SetRect(x0, y0, x0+wdth, y0+hght);
  MoveWindow(&Rect, true);
  xold = yold = 0;
  SetLimits(1.0, 2.0, 1.0, 2.0, true, true, false, false);
  return 0;
}


BOOL 
CPlotWindow::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) {
  SetCursor(myCursor);
	return CWnd::OnSetCursor(pWnd, nHitTest, message);
}


LRESULT 
CPlotWindow::OnChangePreferences(WPARAM wParam, LPARAM lParam) {
  if(Preferences.DoModal() == IDOK) {
    ScreenFont      = ScreenProperty.GetFont();
    PrinterFont     = PrinterProperty.GetFont();
    ScreenColorSet  = ScreenProperty.GetColorSet();
    PrinterColorSet = PrinterProperty.GetColorSet();
    //Aggiorna le Proprietà dei DataSet
    POSITION pos = ScreenProperty.PropertiesList.GetHeadPosition();
    while(pos != NULL) {
		  CDataSetProperties* pProp = ScreenProperty.PropertiesList.GetNext(pos);
      ChangeDataSetProp(*pProp);
    }
  } else {
    ScreenProperty.SetFont(ScreenFont);
    PrinterProperty.SetFont(PrinterFont);
    ScreenProperty.SetColorSet(ScreenColorSet);
    PrinterProperty.SetColorSet(PrinterColorSet);
  }
  UpdatePlot();
  return 0;
}


LRESULT 
CPlotWindow::OnPrintMsg(WPARAM wParam, LPARAM lParam) {
  Print();
  UpdatePlot();
  return 0;
}


/////////////////////////////////////////////////////////////////////////////
////////////////////////// Fine Messaggi ////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

bool 
CPlotWindow::SaveDataSet(int Id) {
  if(dataSetList.IsEmpty()) {
    return false;
  }
	POSITION pos = dataSetList.GetHeadPosition();
	while (pos != NULL) {
		CDataSet* pData = dataSetList.GetNext(pos);
    if(pData->GetId() == Id) {
      return pData->Save();
	  }
  }
  return false;
}


bool 
CPlotWindow::SaveDataSet(int Id, int len, BYTE* Header/*=NULL*/) {
  if(Header == NULL || len <= 0)
    return false;
	POSITION pos = dataSetList.GetHeadPosition();
	while (pos != NULL) {
		CDataSet* pData = dataSetList.GetNext(pos);
    if(pData->GetId() == Id) {
      return pData->Save(len, Header);
	  }
  }
  return false;
}


void 
CPlotWindow::GetDataSet(int Id, int *nPoints, double *x, double *y) {
  *nPoints = 0;
	POSITION pos = dataSetList.GetHeadPosition();
	while (pos != NULL) {
		CDataSet* pData = dataSetList.GetNext(pos);
    if(pData->GetId() == Id) {
      pData->GetData(nPoints, x, y);
	  }
  }
}


int 
CPlotWindow::DataSetPoints(int Id) {
  int nPoints = 0;
  POSITION pos = dataSetList.GetHeadPosition();
	while (pos != NULL) {
		CDataSet* pData = dataSetList.GetNext(pos);
    if(pData->GetId() == Id) {
      nPoints = pData->GetNPoints();
	  }
  }
  return nPoints;
}


void 
CPlotWindow::NewArray(int Id, double *x, double *y, int nElem) {
  if(dataSetList.IsEmpty()) {
    return;
  }
	CDataSet* pData = NULL;
  CDataSet* pTemp;
	POSITION pos = dataSetList.GetHeadPosition();
	while(pos != NULL) {
	  pTemp = dataSetList.GetNext(pos);
    if(pTemp->GetId() == Id) {
      pData = pTemp;
      break;
    }
  }
  if(pData != NULL) {
    pData->AddArray(x, y, nElem);
  }
}


void 
CPlotWindow::SetCurveTitle(int Id, CString Title) {
	POSITION pos = dataSetList.GetHeadPosition();
	while (pos != NULL) {
		CDataSet* pData = dataSetList.GetNext(pos);
    if(pData->GetId() == Id) {
      pData->SetTitle(Title);
      return;
	  }
  }
}


void 
CPlotWindow::SetShowTitle(int Id, bool show) {
	POSITION pos = dataSetList.GetHeadPosition();
	while (pos != NULL) {
		CDataSet* pData = dataSetList.GetNext(pos);
    if(pData->GetId() == Id) {
      pData->SetShowTitle(show);
      return;
	  }
  }
}


void 
CPlotWindow::OnContextMenu(CWnd* pWnd, CPoint point) {
   CMenu menu;
   if(menu.LoadMenu(IDR_PLOTMENU)) {
     CMenu* pPopup = menu.GetSubMenu(0);
     pPopup->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,
                            point.x, point.y, this);
   }
}


void 
CPlotWindow::OnMenuSelect(UINT nItemID, UINT nFlags, HMENU hSysMenu) {
	CWnd::OnMenuSelect(nItemID, nFlags, hSysMenu);
  if(nItemID == ID_DATAFILESAVE) {
    ;
  } else {
    ;
  }
}


BOOL 
CPlotWindow::OnCommand(WPARAM wParam, LPARAM lParam) {

  if(LOWORD(wParam) == ID_DATAFILESAVE) {
  
    if(dataSetList.IsEmpty()) {
      return CWnd::OnCommand(wParam, lParam);
    }
	  POSITION pos = dataSetList.GetHeadPosition();
	  while (pos != NULL) {
		  CDataSet* pData = dataSetList.GetNext(pos);
      pData->Save();
	  }
    return CWnd::OnCommand(wParam, lParam);

  } else if(LOWORD(wParam) == ID_CLEARPLOT) {

    if(AfxMessageBox(_T("Clear Plot:\nAre you Sure?"), MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2)== IDNO) 
      return CWnd::OnCommand(wParam, lParam);

	  POSITION pos = dataSetList.GetHeadPosition();
	  while (pos != NULL) {
		  dataSetList.GetNext(pos)->RemoveAllPoints();
    }
    UpdatePlot();

  } else if(LOWORD(wParam) == ID_PREFERENCES) {
    PostMessage(MSG_PREFERENCES);
  } else if(LOWORD(wParam) == ID_PRINT) {
    PostMessage(MSG_PRINT);
  }

	return CWnd::OnCommand(wParam, lParam);
}


BOOL 
CPlotWindow::OnHelpInfo(HELPINFO* pHelpInfo) {
  if(bHelpEnabled) {
    char FileHelp[] = "Grafici.htm";
    HtmlHelp(DWORD_PTR(FileHelp), HH_DISPLAY_TOPIC); 
  }
  return false;
}


void 
CPlotWindow::EnableHelp(bool enabled) {
  bHelpEnabled = enabled;
}



void 
CPlotWindow::Print() {
  bPrinting = true;
	CRect rect;
  CPrintDialog dlg(FALSE);
  if(dlg.DoModal () != IDOK) {
    bPrinting = false;
    return;
  } else {
    CDC dc;
    dc.Attach (dlg.GetPrinterDC());
    DOCINFO di;
    ::ZeroMemory (&di, sizeof (DOCINFO));
    di.cbSize = sizeof (DOCINFO);
    CString Title;
    GetWindowText(Title);
    di.lpszDocName = Title;
    dc.StartDoc(&di);
    dc.StartPage();

    dc.SetMapMode(MM_LOMETRIC); // Each logical unit is converted to 0.1 millimeter
    dc.SetBkMode(OPAQUE);

    dc.SetBkColor(RGB(255, 255, 255));
	  dc.SetTextColor(RGB(0, 0, 0));

	  dc.SetTextAlign(TA_TOP | TA_LEFT);

    rect.SetRect(0, 0, dc.GetDeviceCaps(HORZRES), dc.GetDeviceCaps(VERTRES));// In mm.

    if(pFont == NULL) pFont = new CFont();
	  pFont->CreateFontIndirect(&PrinterFont);

    pOldFont = dc.SelectObject(pFont);
    LabelSize = dc.GetTextExtent(_T("-0.0000"));

    dc.DPtoLP(&LabelSize);
    dc.DPtoLP(rect);

    int LeftMargin   = 300;
    int RightMargin  = 200;
    int TopMargin    = 300;
    int BottomMargin = 300;

    Pf.l = LeftMargin + LabelSize.cx + rect.left;
    Pf.r = rect.right - RightMargin -  LabelSize.cx;
    Pf.t = rect.top - 3 * LabelSize.cy - TopMargin;
    Pf.b = rect.bottom + BottomMargin + 3 * LabelSize.cy;

    CSize Size = CSize(1,1);
    dc.DPtoLP(&Size);// Quanti Logical Points per Pixel ?
    CPen Pen(PS_SOLID, int(__max(abs(Size.cx), abs(Size.cy))), RGB(0, 0, 0));
    dc.SelectObject(Pen);

    if(Ax.LogX)
      XTicLog(&dc);
    else
      XTicLin(&dc);
    if(Ax.LogY)
      YTicLog(&dc);
    else
      YTicLin(&dc);

    Size = CSize(4,4);
    dc.DPtoLP(&Size);
    CPen fPen(PS_SOLID, int(__max(abs(Size.cx), abs(Size.cy))), RGB(0, 0, 0));
    DeleteObject(dc.SelectObject(fPen));

    dc.MoveTo(int(Pf.l), int(Pf.b));
    dc.LineTo(int(Pf.r), int(Pf.b));
    dc.LineTo(int(Pf.r), int(Pf.t));
    dc.LineTo(int(Pf.l), int(Pf.t));
    dc.LineTo(int(Pf.l), int(Pf.b));

    if(!dataSetList.IsEmpty()) {
      DrawData(&dc);
    }

    dc.SelectObject(pOldFont);
    DeleteObject(pFont);
    delete pFont;
    pFont = NULL;

    dc.EndPage ();
    dc.EndDoc();
  }
  bPrinting = false;
  UpdatePlot();
}


void 
CPlotWindow::OnTimer(UINT nIDEvent) {
  KillTimer(nIDEvent);
  if(nIDEvent == MOUSETIMER) {
	  CRect rc;
	  GetClientRect(rc);
    rc.top = int(rc.bottom-LabelSize.cy);
	  InvalidateRect(rc, true);
  }
  if(nIDEvent == UPDATETIMER) {
    if(m_Plot.m_hObject != NULL)
      m_Plot.DeleteObject(); //Cancella la vecchia Bitmap del Plot

    if(Ax.AutoX || Ax.AutoY) {
      SetLimits (Ax.XMin, Ax.XMax, Ax.YMin, Ax.YMax, Ax.AutoX, Ax.AutoY, Ax.LogX, Ax.LogY);
    }

    CClientDC dc(this);
    dc.SetMapMode(MM_TEXT);
    dc.SetBkColor(ScreenColorSet.Background);
    CSize PointSize = CSize(1,1);
    dc.DPtoLP(&PointSize);// Quanti Logical Points per Pixel ?
    int PenSize = int(__max(abs(PointSize.cx), abs(PointSize.cy)));

	  CDC dcMem;
	  dcMem.CreateCompatibleDC(&dc);
    dcMem.SetBkColor(dc.GetBkColor());
	  CRect rect;
    GetClientRect(rect);

	  m_Plot.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
	  ASSERT(m_Plot.m_hObject != NULL);

    dcMem.SetMapMode(MM_TEXT);
    dcMem.SetBkMode(OPAQUE);
	  dcMem.SetTextColor(ScreenColorSet.Labels);
	  dcMem.SetTextAlign(TA_TOP | TA_LEFT);

	  CBitmap* pOldPlot = dcMem.SelectObject(&m_Plot);

    // draw a rectangle in the background color
	  CBrush brBackground = dcMem.GetBkColor();
	  dcMem.FillRect(rect, &brBackground);

    if(pFont == NULL) pFont = new CFont();
	  pFont->CreateFontIndirect(&ScreenFont);

    pOldFont = dcMem.SelectObject(pFont);
    LabelSize = dcMem.GetTextExtent(_T("-0.0000"));
    dcMem.DPtoLP(&LabelSize);

    Pf.l = LabelSize.cx + rect.left + 10;
    Pf.r = rect.right   - LabelSize.cx;
    Pf.t = rect.top     + 3*LabelSize.cy;
    Pf.b = rect.bottom  - 3*LabelSize.cy;

    CPen Pen(PS_SOLID, PenSize, ScreenColorSet.TicLines);
    dcMem.SelectObject(Pen);

    if(Ax.LogX)
      XTicLog(&dcMem);
    else
      XTicLin(&dcMem);
    if(Ax.LogY)
      YTicLog(&dcMem);
    else
      YTicLin(&dcMem);

    CPen aPen(PS_SOLID, PenSize, ScreenColorSet.Axis);
    dcMem.SelectObject(aPen);
    DeleteObject(Pen);

    dcMem.MoveTo(int(Pf.l), int(Pf.b));
    dcMem.LineTo(int(Pf.r), int(Pf.b));
    dcMem.LineTo(int(Pf.r), int(Pf.t));
    dcMem.LineTo(int(Pf.l), int(Pf.t));
    dcMem.LineTo(int(Pf.l), int(Pf.b));

    if(!dataSetList.IsEmpty()) {
		  DrawData(&dcMem);
    }

    dcMem.SetTextColor(ScreenColorSet.CursorPos);
    LabelSize = dcMem.GetTextExtent(_T("00000000000000000"));
    dcMem.TextOut(int(Pf.l), rect.bottom-LabelSize.cy, xCoord, xCoord.GetLength());
    dcMem.TextOut(int(Pf.l+LabelSize.cx), rect.bottom-LabelSize.cy, yCoord, yCoord.GetLength());

    if(bShowMarker) {
	    int ix, iy;
      double xlmin, ylmin;
      if(Ax.XMin > 0.0) 
        xlmin = log10(Ax.XMin);
      else
        xlmin = FLT_MIN;
      if(Ax.YMin > 0.0)
        ylmin = log10(Ax.YMin);
      else ylmin = FLT_MIN;

      if(xMarker>=Ax.XMin && xMarker<=Ax.XMax && yMarker>=Ax.YMin && yMarker<=Ax.YMax) {
        if(Ax.LogX)
	        ix = int(((log10(xMarker) - xlmin)*xfact) + Pf.l);
        else
  	      ix = int(((xMarker - Ax.XMin)*xfact) + Pf.l);
        if(Ax.LogY)
          iy = int(((log10(yMarker) - ylmin)*yfact) + Pf.b);
        else
  	      iy = int(((yMarker - Ax.YMin)*yfact)+ Pf.b);
        CPen mPen (PS_SOLID, 1, ScreenColorSet.Marker);
        dcMem.SelectObject(mPen);
        DeleteObject(aPen);
        dcMem.MoveTo(ix-4, iy-4);
        dcMem.LineTo(ix, iy);
        dcMem.MoveTo(ix+4, iy-4);
        dcMem.LineTo(ix, iy);
        dcMem.MoveTo(ix-4, iy+4);
        dcMem.LineTo(ix, iy);
        dcMem.MoveTo(ix+4, iy+4);
        dcMem.LineTo(ix, iy);
        dcMem.MoveTo(ix, iy-15);
        dcMem.LineTo(ix, iy);
        dcMem.MoveTo(ix, iy+15);
        dcMem.LineTo(ix, iy);
      }
    }
    dc.BitBlt(0, 0,rect.Width(), rect.Height(), &dcMem, 0, 0, SRCCOPY);

    dcMem.SelectObject(pOldFont);
    DeleteObject(pFont);
    delete pFont;
    pFont = NULL;
	  dcMem.DeleteDC();
  }//Update Timer
	CWnd::OnTimer(nIDEvent);
}


void
CPlotWindow::ChangeDataSetProp(CDataSetProperties Prop) {
	POSITION pos = dataSetList.GetHeadPosition();
	while (pos != NULL) {
		CDataSet* pData = dataSetList.GetNext(pos);
    if(pData->GetId() == Prop.GetId()) {
		  pData->SetProperties(Prop);
      return;
    }
  }
}


CString
CPlotWindow::GetDataSetTitle(int Id) {
	POSITION pos = dataSetList.GetHeadPosition();
	while (pos != NULL) {
		CDataSet* pData = dataSetList.GetNext(pos);
    if(pData->GetId() == Id) {
      return pData->GetTitle();
    }
  }
  return CString("No Such Data Set");
}


void 
CPlotWindow::OnDropFiles(HDROP hDropInfo) {
  UINT nFiles = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);
  UINT LenBuf = DragQueryFile(hDropInfo, 0, NULL, 0);
  _TCHAR* FilePath = new _TCHAR[LenBuf+1];
  double x=0.0, y=0.0;
  FILE* FileIn;

  CString sLine;
  for(int i=0; i<int(nFiles); i++) {
    if(DragQueryFile(hDropInfo, i, FilePath, LenBuf+1) > 2) {
      FileIn = _tfopen(FilePath, _T("rt"));
      if(FileIn == NULL) {
        sLine.Format(_T("Impossibile Aprire il File:\n%s"), FilePath);
        AfxMessageBox(sLine);
        continue;
      } // if(FileIn == NULL)

      sLine = FilePath;
      int iStart = sLine.ReverseFind('\\')+1;
      int iEnd   = sLine.ReverseFind('.');
      if(iEnd > iStart) {
        sLine = sLine.Mid(iStart, iEnd-iStart);
      } else {
        sLine = sLine.Mid(iStart);
      }
      NewDataSet(FIRSTDROPPEDFILENUMBER+nFilesDropped+i, 1, RGB(255,255,0), CPlotWindow::ipoint, sLine);

      int nRead;
      CString sLine;
      while(!feof(FileIn)) {
        try {
          nRead = _ftscanf(FileIn, _T("%lf %lf"), &x, &y);
          if(nRead == 2)
            NewPoint(FIRSTDROPPEDFILENUMBER+nFilesDropped+i, x, y);
        } catch(...) {
          continue;
        }
      }
      SetShowDataSet(FIRSTDROPPEDFILENUMBER+nFilesDropped+i, true);
      nFilesDropped++;
      fclose(FileIn);
    }
  }
  delete[] FilePath;
  UpdatePlot();
  CWnd::OnDropFiles(hDropInfo);
}
