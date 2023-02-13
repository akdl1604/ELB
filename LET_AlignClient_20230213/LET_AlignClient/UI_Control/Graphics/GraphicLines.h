#pragma once
#include "stdafx.h"
#include <opencv.hpp>

/*
 * - Lincoln Lee
 * - x.perfect.vn@gmail.com
 * - 2022-08-22
 */

// Lincoln Lee - 2022/08/22 - Adding GraphicPolyline, GraphicLines
class GraphicLines : public GraphicBase
{
public:

public:

	GraphicLines(Gdiplus::Color color = 0xff00ff00, int thickness = 1, Gdiplus::SmoothingMode smooth = Gdiplus::SmoothingMode::SmoothingModeNone)
		: m_Pen(new Gdiplus::Pen(color, thickness)), m_Smooth(smooth)
	{
	}

	GraphicLines(const std::vector<Gdiplus::PointF>& points, Gdiplus::Color color = 0xff00ff00, int thickness = 1, Gdiplus::SmoothingMode smooth = Gdiplus::SmoothingMode::SmoothingModeNone)
		: m_Points(points), m_Pen(new Gdiplus::Pen(color, thickness)), m_Smooth(smooth)
	{
	}


	GraphicLines(const std::vector<Gdiplus::Point>& points, Gdiplus::Color color = 0xff00ff00, int thickness = 1, Gdiplus::SmoothingMode smooth = Gdiplus::SmoothingMode::SmoothingModeNone)
		: m_Pen(new Gdiplus::Pen(color, thickness)), m_Smooth(smooth)
	{
		for (const auto& pt : points)
			m_Points.push_back(Gdiplus::PointF(pt.X, pt.Y));
	}

	template<class T>
	GraphicLines(const std::vector<cv::Point_<T>>& points, Gdiplus::Color color = 0xff00ff00, int thickness = 1, Gdiplus::SmoothingMode smooth = Gdiplus::SmoothingMode::SmoothingModeNone)
		: m_Pen(new Gdiplus::Pen(color, thickness)), m_Smooth(smooth)
	{
		for (const auto& pt : points)
			m_Points.push_back(Gdiplus::PointF(pt.x, pt.y));
	}

	GraphicLines& Render(Gdiplus::Graphics& graph, const Rigid& trans)
	{
		auto pts = trans.Transform(m_Points);
		auto chk = graph.Save();
		graph.SetSmoothingMode(m_Smooth);
		graph.DrawLines(m_Pen, pts.data(),int( pts.size()));
		graph.Restore(chk);
		return *this;
	}

	GraphicLines& AddPoint(Gdiplus::PointF pt)
	{
		m_Points.push_back(pt);
		return *this;
	}

	GraphicLines& AddPoint(CPoint pt)
	{
		m_Points.push_back(Gdiplus::PointF(pt.x, pt.y));
		return *this;
	}

	template<class T0>
	GraphicLines& AddPoint(cv::Point_<T0> pt)
	{
		m_Points.push_back(Gdiplus::PointF(pt.x, pt.y));
		return *this;
	}

	template<class T0, class T1>
	GraphicLines& AddPoint(T0 x, T1 y)
	{
		m_Points.push_back(Gdiplus::PointF(x, y));
		return *this;
	}

	GraphicLines& Clear()
	{
		m_Points.clear();
		return *this;
	}

	GraphicLines& AddPoints(const std::vector<Gdiplus::PointF>& points)
	{
		m_Points.insert(m_Points.end(), points.begin(), points.end());
		return *this;
	}

	~GraphicLines()
	{
		delete m_Pen;
		m_Points.clear();
	}

protected:
private:
	Gdiplus::Pen* m_Pen = NULL;
	Gdiplus::SmoothingMode m_Smooth;
	std::vector<Gdiplus::PointF> m_Points;
};