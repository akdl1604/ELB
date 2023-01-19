#pragma once
#include "stdafx.h"
#include "GraphicBase.h"

/*
* - Lincoln Lee
* - x.perfect.vn@gmail.com
* - 2022-08-08
*/


class GraphicArrow: public GraphicBase
{
public:
	Gdiplus::PointF StartPOS;
	Gdiplus::PointF EndPOS;

public:
	GraphicArrow(Gdiplus::PointF startPoint = { 0, 0 }, Gdiplus::PointF endPoint = { 0, 0 }, float arrowLen = 11, Gdiplus::Color color = Gdiplus::Color::Yellow, bool twoEnds = false, float thickness = 1.)
		: m_Pen(new Gdiplus::Pen(color, thickness)), StartPOS(startPoint), EndPOS(endPoint), m_TwoEnds(twoEnds), m_ArrowLen(arrowLen)
	{
	}

	GraphicArrow& Render(Gdiplus::Graphics& graph, const Rigid& trans) override
	{
		Gdiplus::PointF ed1, ed2;
		
		auto st = trans.Transform(StartPOS);
		auto ed = trans.Transform(EndPOS);
		Point2Point pp(st, ed);

		GetArrowPoints(ed, m_ArrowLen, pp.th + CV_PI, ed1, ed2);

		auto chk = graph.Save();
		graph.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias);
		graph.DrawLine(m_Pen, st, ed);
		graph.DrawLine(m_Pen, ed, ed1);
		graph.DrawLine(m_Pen, ed, ed2);

		if (m_TwoEnds)
		{
			Gdiplus::PointF st1, st2;
			GetArrowPoints(st, m_ArrowLen, pp.th, st1, st2);
			graph.DrawLine(m_Pen, st, st1);
			graph.DrawLine(m_Pen, st, st2);
		}
		graph.Restore(chk);
		return *this;
	}

	GraphicArrow& Render(Gdiplus::Graphics& graph, const Rigid& trans, CSize maxSize) override
	{
		Gdiplus::PointF ed1, ed2;

		auto st = trans.Transform(StartPOS);
		auto ed = trans.Transform(EndPOS);
		Point2Point pp(st, ed);

		GetArrowPoints(ed, m_ArrowLen, pp.th + CV_PI, ed1, ed2);

		auto chk = graph.Save();
		graph.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias);
		graph.DrawLine(m_Pen, st, ed);
		graph.DrawLine(m_Pen, ed, ed1);
		graph.DrawLine(m_Pen, ed, ed2);

		if (m_TwoEnds)
		{
			Gdiplus::PointF st1, st2;
			GetArrowPoints(st, m_ArrowLen, pp.th, st1, st2);
			graph.DrawLine(m_Pen, st, st1);
			graph.DrawLine(m_Pen, st, st2);
		}
		graph.Restore(chk);
		return *this;
	}

	GraphicArrow& SetWidth(float w)
	{
		m_Pen->SetWidth(w);
		return *this;
	}

	~GraphicArrow()
	{
		delete m_Pen;
	}

protected:

private:
	Gdiplus::Pen* m_Pen;
	float m_ArrowLen = 11;
	bool m_TwoEnds = false;
};
