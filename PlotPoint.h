#pragma once


class AFX_EXT_CLASS CPlotPoint {
public:
 	CPlotPoint(double _x, double _y);
  CPlotPoint(void);
  virtual ~CPlotPoint(void);

  double x;
	double y;
};
