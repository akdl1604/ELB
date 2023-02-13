#pragma once
#include "stdafx.h"
#include <opencv.hpp>

/*
* - Lincoln Lee
* - x.perfect.vn@gmail.com
* - 2022-08-08
*/

class GraphicPoints: public GraphicBase
{
public:

public:

	GraphicPoints(Gdiplus::Color color = 0xff00ff00, int crossSize = 3, int thickness = 1)
		: m_CrossSize(crossSize), m_Pen(new Gdiplus::Pen(color, thickness))
	{
	}

	GraphicPoints(const std::vector<Gdiplus::PointF>& points, Gdiplus::Color color = 0xff00ff00, int crossSize = 3, int thickness = 1)
		: m_Points(points), m_CrossSize(crossSize), m_Pen(new Gdiplus::Pen(color, thickness))
	{
	}

	GraphicPoints& Render(Gdiplus::Graphics& graph, const Rigid& trans)
	{
		auto chk = graph.Save();
		
		graph.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeNone);
		for (const auto& pt: m_Points)
		{
			auto ptx = trans.Transform(pt);
			PlotPoint(graph, m_Pen, Gdiplus::Point(std::lround(ptx.X), std::lround(ptx.Y)), m_CrossSize);
		}
		graph.Restore(chk);
		return *this;
	}

	GraphicPoints& AddPoint(Gdiplus::PointF pt)
	{
		m_Points.push_back(pt);
		return *this;
	}

	GraphicPoints& AddPoint(CPoint pt)
	{
		m_Points.push_back(Gdiplus::PointF(pt.x, pt.y));
		return *this;
	}

	template<class T0>
	GraphicPoints& AddPoint(cv::Point_<T0> pt)
	{
		m_Points.push_back(Gdiplus::PointF(pt.x, pt.y));
		return *this;
	}

	template<class T0, class T1>
	GraphicPoints& AddPoint(T0 x, T1 y)
	{
		m_Points.push_back(Gdiplus::PointF(x, y));
		return *this;
	}

	GraphicPoints& Clear()
	{
		m_Points.clear();
		return *this;
	}

	GraphicPoints& AddPoints(const std::vector<Gdiplus::PointF>& points)
	{
		m_Points.insert(m_Points.end(), points.begin(), points.end());
		return *this;
	}

	~GraphicPoints()
	{
		delete m_Pen;
		m_Points.clear();
	}

protected:
private:
	int m_CrossSize = 3;
	Gdiplus::Pen* m_Pen = NULL;
	std::vector<Gdiplus::PointF> m_Points;
};