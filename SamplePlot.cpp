// SamplePlot.cpp : file di implementazione
//

#include "stdafx.h"
#include <math.h>
#include <float.h>
#include "resource.h"

#include ".\ColorSet.h"
#include ".\AxisLimits.h"
#include ".\AxisFrame.h"
#include ".\PlotPoint.h"
#include ".\DataSetProperties.h"
#include ".\DataSet.h"
#include ".\SamplePlot.h"


#define NINT(x) ((x) > 0.0 ? (int)floor((x)+0.5) : (int)ceil((x)-0.5))

#define FrameLineWidth  4    //In 0.1mm
#define MarkerLineWidth 4    //In 0.1mm
#define TicLineWidth    2    //In 0.1mm
#define DataLineWidth   2    //In 0.1mm
#define FrameWidth      1300 //In 0.1mm

// CSamplePlot

IMPLEMENT_DYNAMIC(CSamplePlot, CStatic)
CSamplePlot::CSamplePlot() {
  bPrintMode = false;
  pData      = NULL;
  pFont      = NULL;
}

CSamplePlot::~CSamplePlot() {
	if(pData != NULL)
    delete pData;
	if(pFont != NULL)
    delete pFont;
}


BEGIN_MESSAGE_MAP(CSamplePlot, CStatic)
	ON_WM_PAINT()
END_MESSAGE_MAP()



// gestori di messaggi CSamplePlot

void
CSamplePlot::SetLimits (double XMin, double XMax, double YMin, double YMax,
                        BOOL AutoX, BOOL AutoY, BOOL LogX, BOOL LogY) {
	Ax.XMin  = XMin;
	Ax.XMax  = XMax;
	Ax.YMin  = YMin;
	Ax.YMax  = YMax;
	Ax.AutoX = AutoX;
	Ax.AutoY = AutoY;
	Ax.LogX  = LogX;
	Ax.LogY  = LogY;

  if(pData == NULL) goto check;

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
    if(pData != NULL){
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
      if(EmptyData) {
        XMin = Ax.XMin;
        XMax = Ax.XMax;
        YMin = Ax.YMin;
        YMax = Ax.YMax;
      }
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
  Ax.XMin  = XMin;
  Ax.XMax  = XMax;
  Ax.YMin  = YMin;
  Ax.YMax  = YMax;
}


CDataSet*
CSamplePlot::NewDataSet(UINT Id, int PenWidth, COLORREF Color, int Symbol, CString Title) {
  CDataSet* pDataItem = new CDataSet(Id, PenWidth, Color, Symbol, Title);
  return pDataItem;
}


void
CSamplePlot::SetShowDataSet(int Id, bool Show) {
  if(!pData != NULL) {
    pData->SetShow(Show);
  }
}


void
CSamplePlot::DrawData(CDC* pDC) {
  if(pData == NULL) {
    return;
  }
	CBrush brBackground = pDC->GetBkColor();
  pDC->SelectObject(GetStockObject(NULL_BRUSH));
  if(pData->isShowed) {
    CPen dPen;
    if(bPrintMode) {
      dPen.CreatePen(PS_SOLID, DataLineWidth, RGB(0, 0, 0));
      DeleteObject(pDC->SelectObject(dPen));
    } else {
      dPen.CreatePen(PS_SOLID, 1, pData->GetProperties().Color);
      DeleteObject(pDC->SelectObject(dPen));
    }
    LinePlot(pDC, pData);
  }
}


void 
CSamplePlot::LinePlot(CDC* pDC, CDataSet* pData) {
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
      ix = INT_MIN; // Solo per escludere il punto
  } else
	  ix = int(((pData->m_pointArray[0].x - Ax.XMin)*xfact) + Pf.l);
  if(Ax.LogY) {
    if(pData->m_pointArray[0].y > 0.0)
  	  iy = int((Pf.b + (log10(pData->m_pointArray[0].y) - ylmin)*yfact));
    else 
      iy = INT_MIN; // Solo per escludere il punto
  } else
    iy = int((Pf.b + (pData->m_pointArray[0].y - Ax.YMin)*yfact));

  pDC->MoveTo(ix, iy);

  for(int i=1; i <= iMax; i++) {
    if(Ax.LogX)
      if(pData->m_pointArray[i].x > 0.0)
	      ix = int(((log10(pData->m_pointArray[i].x) - xlmin)*xfact) + Pf.l);
    else 
        ix = INT_MIN; // Solo per escludere il punto
    else
  	  ix = int(((pData->m_pointArray[i].x - Ax.XMin)*xfact) + Pf.l);
    if(Ax.LogY)
      if(pData->m_pointArray[i].y > 0.0)
        iy = int((Pf.b + (log10(pData->m_pointArray[i].y) - ylmin)*yfact));
      else 
        iy = INT_MIN; // Solo per escludere il punto
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
CSamplePlot::DrawLastPoint(CDC* pDC, CDataSet* pData) {
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
  if(bPrintMode) {
    pDC->LineTo(ix+1, iy+1);
  } else {
    pDC->SetPixelV(ix, iy, pData->GetProperties().Color);
  }
	return;
}


void
CSamplePlot::XTicLin(CDC* dc) {

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
		  Label.Format(_T("% 6.2f"), (double)isx*fmant);
    Size = dc->GetTextExtent(Label);
		ix0 = ix - Size.cx/2;
		dc->TextOut(ix0, iy0, Label, Label.GetLength());
		dxx = isig*dxx - dx;
	}	while (dxx >= xmin);
  Size = dc->GetTextExtent(_T("*10 "));
	dc->TextOut(int(Pf.r + 2),	int(Pf.b + iYsign*Size.cy), _T("*10"), 3);
  int icx = Size.cx;
	Label.Format(_T("%-3i"), iesp);
  Size = dc->GetTextExtent(Label);
	dc->TextOut(int(Pf.r + icx), int(Pf.b+iYsign*1.5*Size.cy),Label,Label.GetLength());
}


void
CSamplePlot::YTicLin(CDC* dc) {
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
	dc->TextOut(int(ix0+icx),int(Pf.t+2*iYsign*Size.cy),Label,Label.GetLength());
}


/////////////////////////////////////////////////////////////////////////////
// CSamplePlot message handlers
/////////////////////////////////////////////////////////////////////////////


void 
CSamplePlot::OnPaint() {
	CPaintDC dc(this); // device context for painting
  Update();
}


void
CSamplePlot::Update() {
  CSize A4 = CSize(2100, 2970);// In 0.1 mm

  if(pData == NULL) {
    pData = NewDataSet(0, 2, RGB(255, 255, 0), 0, _T("Dati di Test"));
    for(int i=-7; i<8; i++) {
      pData->AddPoint(CPlotPoint(double(i), double(i*i)));
    }
    pData->SetShow(true);
  }
  SetLimits (0.0, 1.0, 0.0, 1.0, true, true, false, false);

  CClientDC dc(this);

  dc.SetBkColor(ColorSet.Background);
  if(bPrintMode) {
    dc.SetMapMode(MM_LOMETRIC);
  } else {
    dc.SetMapMode(MM_TEXT);
  }

	CRect rectClient;
  GetClientRect(rectClient);
  dc.DPtoLP(rectClient);

  int tmp = Font.lfHeight;
  if(bPrintMode) {
    Font.lfHeight = -MulDiv(Font.lfHeight/*In Punti*/, rectClient.Height(), A4.cy);
  } else {
    Font.lfHeight = -MulDiv(Font.lfHeight/*In Punti*/, GetDeviceCaps(dc, LOGPIXELSY), 72);
  }

  if(pFont == NULL) pFont = new CFont();
  pFont->CreateFontIndirect(&Font);
  Font.lfHeight = tmp;

  CSize PointSize = CSize(1,1);
  dc.DPtoLP(&PointSize);// Quanti Logical Points per Pixel ?
  int PenSize = int(__max(abs(PointSize.cx), abs(PointSize.cy)));

  int xScale = rectClient.Width()  / A4.cx;
  int yScale = rectClient.Height() / A4.cy;

  dc.SetBkMode(OPAQUE);
	dc.SetTextColor(ColorSet.Labels);
	dc.SetTextAlign(TA_TOP | TA_LEFT);

  // draw a rectClientangle in the background color
	CBrush brBackground = ColorSet.Background;
	dc.FillRect(rectClient, &brBackground);

  CFont* pOldFont = dc.SelectObject(pFont);
  CSize LabelSize = dc.GetTextExtent(_T("-0.0000"));

  int LeftMargin   = 0;
  int RightMargin  = 0;
  int TopMargin    = 0;
  int BottomMargin = 0;

  if(dc.GetMapMode() == MM_TEXT) {
    Pf.l = LeftMargin + LabelSize.cx + rectClient.left + 10;
    Pf.r = rectClient.right - RightMargin -  LabelSize.cx;
    Pf.t = rectClient.top + 3*LabelSize.cy + TopMargin;
    Pf.b = rectClient.bottom - BottomMargin - 3*LabelSize.cy;
  } else {
    Pf.l = LeftMargin + LabelSize.cx + rectClient.left + 10;
    Pf.r = rectClient.right - RightMargin -  LabelSize.cx;
    Pf.t = rectClient.top - 3*LabelSize.cy - TopMargin;
    Pf.b = rectClient.bottom + BottomMargin + 3*LabelSize.cy;
  }
  CPen Pen;
  if(bPrintMode) {
    Pen.CreatePen(PS_SOLID, TicLineWidth, ColorSet.TicLines);
    DeleteObject(dc.SelectObject(Pen));
  } else {
    Pen.CreatePen(PS_SOLID, PenSize, ColorSet.TicLines);
    DeleteObject(dc.SelectObject(Pen));
  }
  XTicLin(&dc);
  YTicLin(&dc);

  CPen aPen;
  if(bPrintMode) {
    aPen.CreatePen(PS_SOLID, FrameLineWidth, ColorSet.Axis);
    DeleteObject(dc.SelectObject(aPen));
  } else {
    aPen.CreatePen(PS_SOLID, PenSize, ColorSet.Axis);
    DeleteObject(dc.SelectObject(aPen));
  }
  dc.MoveTo(int(Pf.l), int(Pf.b));
  dc.LineTo(int(Pf.r), int(Pf.b));
  dc.LineTo(int(Pf.r), int(Pf.t));
  dc.LineTo(int(Pf.l), int(Pf.t));
  dc.LineTo(int(Pf.l), int(Pf.b));

  if(pData != NULL) {
		DrawData(&dc);
  }

  if(!bPrintMode) {
    dc.SetTextColor(ColorSet.CursorPos);
    LabelSize = dc.GetTextExtent(_T("X=12345   Y=67890"));
    dc.TextOut(int(Pf.l), rectClient.bottom-LabelSize.cy, _T("X=12345   Y=67890"), int(_tcslen(_T("X=12345   Y=67890"))));

    int ix = int((Pf.r+Pf.l)*0.5);
    int iy = int((Pf.t+Pf.b)*0.5);

    CPen mPen;
    if(bPrintMode) {
      mPen.CreatePen(PS_SOLID, MarkerLineWidth, ColorSet.Marker);
      DeleteObject(dc.SelectObject(mPen));
    } else {
      mPen.CreatePen(PS_SOLID, 1, ColorSet.Marker);
      DeleteObject(dc.SelectObject(mPen));
    }
    dc.MoveTo(ix-4, iy-4);
    dc.LineTo(ix, iy);
    dc.MoveTo(ix+4, iy-4);
    dc.LineTo(ix, iy);
    dc.MoveTo(ix-4, iy+4);
    dc.LineTo(ix, iy);
    dc.MoveTo(ix+4, iy+4);
    dc.LineTo(ix, iy);
    dc.MoveTo(ix, iy-15);
    dc.LineTo(ix, iy);
    dc.MoveTo(ix, iy+15);
    dc.LineTo(ix, iy);
  }

  DeleteObject(pFont);
  delete pFont;
  pFont = NULL;
}


void 
CSamplePlot::SetPrintMode(bool PrintMode) {
  bPrintMode = PrintMode;
}

