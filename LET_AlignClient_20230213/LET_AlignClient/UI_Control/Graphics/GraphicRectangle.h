#pragma once
#include "stdafx.h"
#include "GraphicBase.h"

/*
* - Lincoln Lee
* - x.perfect.vn@gmail.com
* - 2022-08-08
*/

class GraphicRectangle: public GraphicBase
{
public:
	GraphicRectangle(Gdiplus::PointF startPoint = { 0, 0 }, Gdiplus::PointF endPoint = { 0, 0 }, Gdiplus::Color color = 0xff00ff00, float thickness = 1.)
		: GraphicRectangle(Gdiplus::RectF(startPoint.X, startPoint.Y, endPoint.X - startPoint.X, endPoint.Y - startPoint.Y), color, thickness)
	{}

	GraphicRectangle(CPoint startPoint, CPoint endPoint, Gdiplus::Color color = 0xff00ff00, float thickness = 1.)
		: GraphicRectangle(Gdiplus::RectF(startPoint.x, startPoint.y, endPoint.x - startPoint.x, endPoint.y - startPoint.y), color, thickness)
	{}

	template<class T0, class T1>
	GraphicRectangle(cv::Point_<T0> startPoint, cv::Point_<T1> endPoint, Gdiplus::Color color = 0xff00ff00, float thickness = 1.)
		: GraphicRectangle(Gdiplus::RectF(startPoint.x, startPoint.y, endPoint.x - startPoint.x, endPoint.y - startPoint.y), color, thickness)
	{}

	GraphicRectangle(int x1, int y1, int x2, int y2, Gdiplus::Color color = 0xff00ff00, float thickness = 1.)
		: GraphicRectangle(Gdiplus::RectF(x1, y1, x2 - x1, y2 - y1), color, thickness)
	{}

	GraphicRectangle& SetDashStyle(Gdiplus::DashStyle dashStyle)
	{
		m_Pen->SetDashStyle(dashStyle);
		return *this;
	}

	GraphicRectangle& SetDashPattern(Gdiplus::REAL* pattern, int count)
	{
		m_Pen->SetDashPattern(pattern, count);
		return *this;
	}

	GraphicRectangle& SetColor(Gdiplus::Color color)
	{
		m_Pen->SetColor(color);
		return *this;
	}

	GraphicRectangle& SetThickness(float thickness)
	{
		m_Pen->SetWidth(thickness);
		return *this;
	}

	GraphicRectangle(Gdiplus::PointF centerPoint, float width, float height, Gdiplus::Color color = 0xff00ff00, float thickness = 1.)
		: GraphicRectangle(GetRectGDI(centerPoint, cv::Size2f(width, height)), color, thickness)
	{
	}

	GraphicRectangle(Gdiplus::RectF rect, Gdiplus::Color color = 0xff00ff00, float thickness = 1.)
		: m_Pen(new Gdiplus::Pen(color, thickness)), m_Rect(rect)
	{
	}

	GraphicRectangle(CRect rect, Gdiplus::Color color = 0xff00ff00, float thickness = 1.)
		: m_Pen(new Gdiplus::Pen(color, thickness)), m_Rect(rect.left, rect.top, rect.Width(), rect.Height())
	{
	}

	GraphicRectangle& Render(Gdiplus::Graphics& graph, const Rigid& trans) override
	{
		auto rect = trans.Transform(m_Rect);

		auto chk = graph.Save();
		graph.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeNone);
		graph.DrawRectangle(m_Pen, rect);
		graph.Restore(chk);
		return *this;
	}

	GraphicRectangle& Render(Gdiplus::Graphics& graph, const Rigid& trans, CSize maxSize) override
	{
		auto rect = trans.Transform(m_Rect);

		auto chk = graph.Save();
		graph.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeNone);
		graph.DrawRectangle(m_Pen, rect);
		graph.Restore(chk);
		return *this;
	}

	~GraphicRectangle()
	{
		delete m_Pen;
	}
protected:
private:
	Gdiplus::RectF m_Rect;
	Gdiplus::Pen* m_Pen;
};
