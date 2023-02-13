#pragma once
#include "stdafx.h"
#include <opencv.hpp>

/*
* - Lincoln Lee
* - x.perfect.vn@gmail.com
* - 2022-08-08
*/

class GraphicPoint: public GraphicBase
{
public:
	float X;
	float Y;

public:
	template<class T0>
	GraphicPoint(cv::Point_<T0> center, Gdiplus::Color color = 0xff00ff00, int crossSize = 3, int thickness = 1)
		: GraphicPoint(float(center.x), float(center.y), color, crossSize, thickness)
	{}

	GraphicPoint(CPoint center, Gdiplus::Color color = 0xff00ff00, int crossSize = 3, int thickness = 1)
		: GraphicPoint(center.x, center.y, color, crossSize, thickness)
	{}

	GraphicPoint(Gdiplus::PointF center, Gdiplus::Color color = 0xff00ff00, int crossSize = 3, int thickness = 1)
		: GraphicPoint(center.X, center.Y, color, crossSize, thickness)
	{}
	
	GraphicPoint(float x, float y, Gdiplus::Color color = 0xff00ff00, int crossSize = 3, int thickness = 1)
		: X(x), Y(y), m_CrossSize(crossSize), m_Pen(new Gdiplus::Pen(color, thickness))
	{
	}

	GraphicPoint& Render(Gdiplus::Graphics& graph, const Rigid& trans)
	{
		auto chk = graph.Save();
		auto pt = trans.Transform(cv::Point2f(X, Y));
		graph.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeNone);
		PlotPoint(graph, m_Pen, Gdiplus::Point(std::lround(pt.x), std::lround(pt.y)), m_CrossSize);
		//PlotPoint(graph, m_Pen, Gdiplus::PointF(pt.x, pt.y), m_CrossSize);
		graph.Restore(chk);
		return *this;
	}

	~GraphicPoint()
	{
		delete m_Pen;
	}

protected:
private:
	Gdiplus::Pen* m_Pen = NULL;
	int m_CrossSize = 12;
};