#pragma once
#include "GraphicBase.h"

/*
* - Lincoln Lee
* - x.perfect.vn@gmail.com
* - 2022-08-08
*/

class GraphicCircle: public GraphicBase
{
public:
	GraphicCircle(Gdiplus::PointF loc = { 0, 0 }, float radius = 1., Gdiplus::Color color = 0xff00ff00, float thickness = 2.) :
		GraphicCircle(loc.X - radius, loc.Y - radius, loc.X + radius, loc.Y + radius, color, thickness)
	{
	}

	GraphicCircle(Gdiplus::PointF pt0, Gdiplus::PointF pt1, Gdiplus::Color color = 0xff00ff00, float thickness = 2.) :
		GraphicCircle(pt0.X, pt0.Y, pt1.X, pt1.Y, color, thickness)
	{
	}

	GraphicCircle(int x1, int y1, int x2, int y2, Gdiplus::Color color = 0xff00ff00, float thickness = 2.) :
		m_Rect(Gdiplus::RectF(x1, y1, x2 - x1, y2 - y1)), m_Pen(new Gdiplus::Pen(color, thickness))
	{
	}

	GraphicCircle(float x1, float y1, float x2, float y2, Gdiplus::Color color = 0xff00ff00, float thickness = 2.) :
		m_Rect(Gdiplus::RectF(x1, y1, x2 - x1, y2 - y1)), m_Pen(new Gdiplus::Pen(color, thickness))
	{
	}

	GraphicCircle& Render(Gdiplus::Graphics& graph, const Rigid& trans)
	{
		auto rect = trans.Transform(m_Rect);
		auto chk = graph.Save();
		graph.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias);
		graph.DrawEllipse(m_Pen, rect);
		graph.Restore(chk);
		return *this;
	}

	~GraphicCircle()
	{
		delete m_Pen;
	}

protected:


private:
	Gdiplus::Pen* m_Pen;
	Gdiplus::RectF m_Rect;
};


