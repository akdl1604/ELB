#pragma once
#include "stdafx.h"
#include "GraphicBase.h"

/*
* - Lincoln Lee
* - x.perfect.vn@gmail.com
* - 2022-08-08
*/

class GraphicLine: public GraphicBase
{
public:
	GraphicLine(int x0, int y0, int x1, int y1, Gdiplus::Color color = 0xff00ff00, float thickness = 2., Gdiplus::DashStyle dashStyle = Gdiplus::DashStyle::DashStyleSolid, Gdiplus::SmoothingMode smooth = Gdiplus::SmoothingMode::SmoothingModeAntiAlias) :
		GraphicLine(Gdiplus::PointF(x0, y0), Gdiplus::PointF(x1, y1), color, thickness, dashStyle, smooth)
	{}

	GraphicLine(float x0, float y0, float x1, float y1, Gdiplus::Color color = 0xff00ff00, float thickness = 2., Gdiplus::DashStyle dashStyle = Gdiplus::DashStyle::DashStyleSolid, Gdiplus::SmoothingMode smooth = Gdiplus::SmoothingMode::SmoothingModeAntiAlias) :
		GraphicLine(Gdiplus::PointF(x0, y0), Gdiplus::PointF(x1, y1), color, thickness, dashStyle, smooth)
	{}

	GraphicLine(double x0, double y0, double x1, double y1, Gdiplus::Color color = 0xff00ff00, float thickness = 2., Gdiplus::DashStyle dashStyle = Gdiplus::DashStyle::DashStyleSolid, Gdiplus::SmoothingMode smooth = Gdiplus::SmoothingMode::SmoothingModeAntiAlias) :
		GraphicLine(Gdiplus::PointF(x0, y0), Gdiplus::PointF(x1, y1), color, thickness, dashStyle, smooth)
	{}

	GraphicLine(CPoint pt0, CPoint pt1, Gdiplus::Color color = 0xff00ff00, float thickness = 2., Gdiplus::DashStyle dashStyle = Gdiplus::DashStyle::DashStyleSolid, Gdiplus::SmoothingMode smooth = Gdiplus::SmoothingMode::SmoothingModeAntiAlias) :
		GraphicLine(Gdiplus::PointF(pt0.x, pt0.y), Gdiplus::PointF(pt1.x, pt1.y), color, thickness, dashStyle, smooth)
	{}

	template<class T0, class T1>
	GraphicLine(cv::Point_<T0> pt0, cv::Point_<T1> pt1, Gdiplus::Color color = 0xff00ff00, float thickness = 2., Gdiplus::DashStyle dashStyle = Gdiplus::DashStyle::DashStyleSolid, Gdiplus::SmoothingMode smooth = Gdiplus::SmoothingMode::SmoothingModeAntiAlias) :
		GraphicLine(Gdiplus::PointF(pt0.x, pt0.y), Gdiplus::PointF(pt1.x, pt1.y), color, thickness, dashStyle, smooth)
	{}

	GraphicLine(Gdiplus::PointF startPoint, Gdiplus::PointF endPoint, Gdiplus::Color color = 0xff00ff00, float thickness = 2., Gdiplus::DashStyle dashStyle = Gdiplus::DashStyle::DashStyleSolid, Gdiplus::SmoothingMode smooth = Gdiplus::SmoothingMode::SmoothingModeAntiAlias)
		: m_Pen(new Gdiplus::Pen(color, thickness)), m_StartPoint(startPoint), m_EndPoint(endPoint), m_SmoothingMode(smooth)
	{
		if (Gdiplus::DashStyle::DashStyleSolid != dashStyle)
		{
			m_Pen->SetDashStyle(dashStyle);
			m_Pen->SetDashPattern(m_DashPattern, 2);
		}
	}

	GraphicLine& SetDashStyle(Gdiplus::DashStyle dashStyle)
	{
		m_Pen->SetDashStyle(dashStyle);
		return *this;
	}

	GraphicLine& SetDashPattern(Gdiplus::REAL* pattern, int count)
	{
		m_Pen->SetDashPattern(pattern, count);
		return *this;
	}

	GraphicLine& SetColor(Gdiplus::Color color)
	{
		m_Pen->SetColor(color);
		return *this;
	}

	GraphicLine& SetThickness(float thickness)
	{
		m_Pen->SetWidth(thickness);
		return *this;
	}

	GraphicLine& Render(Gdiplus::Graphics& graph, const Rigid& trans) override
	{
		auto st = trans.Transform(m_StartPoint);
		auto ed = trans.Transform(m_EndPoint);

		auto chk = graph.Save();
		graph.SetSmoothingMode(m_SmoothingMode); //SmoothingMode::SmoothingModeAntiAlias
		graph.DrawLine(m_Pen, st, ed);
		graph.Restore(chk);
		return *this;
	}

	GraphicLine& Render(Gdiplus::Graphics& graph, const Rigid& trans, CSize maxSize) override
	{
		auto st = trans.Transform(m_StartPoint);
		auto ed = trans.Transform(m_EndPoint);

		auto chk = graph.Save();
		graph.SetSmoothingMode(m_SmoothingMode);
		graph.DrawLine(m_Pen, st, ed);
		graph.Restore(chk);
		return *this;
	}

	Gdiplus::REAL* GetDashPattern()
	{
		return m_DashPattern;
	}

	~GraphicLine()
	{
		delete m_Pen;
	}
protected:


private:
	Gdiplus::PointF m_StartPoint;
	Gdiplus::PointF m_EndPoint;
	Gdiplus::SmoothingMode m_SmoothingMode = Gdiplus::SmoothingMode::SmoothingModeAntiAlias;
	Gdiplus::Pen* m_Pen;
	Gdiplus::REAL m_DashPattern[2]{6, 6};
};
