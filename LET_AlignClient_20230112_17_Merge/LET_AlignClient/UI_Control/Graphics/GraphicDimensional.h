#pragma once
#include "stdafx.h"
#include "GraphicBase.h"
#include "GraphicLabel.h"

/*
* - Lincoln Lee
* - x.perfect.vn@gmail.com
* - 2022-08-08
*/

class GraphicDimensional: public GraphicBase
{
public:
	GraphicDimensional(Gdiplus::PointF startPoint = { 0, 0 }, Gdiplus::PointF endPoint = { 0, 0 }, CString strInfo = CString(), float arrowLen = 11, Gdiplus::Color color = 0xff00ff00, float thickness = 2.)
		: m_Pen(new Gdiplus::Pen(color, thickness)), m_StartPoint(startPoint), m_EndPoint(endPoint), m_ArrowLen(arrowLen)
	{
		lbl = new GraphicLabel();
		lbl->SetText(strInfo)
			.SetDisplayLocation(0, 0)
			.SetAnchor(Anchor::TopCenter);
	}

	GraphicDimensional& SetText(CString str)
	{
		lbl->SetText(str);
		return *this;
	}

	GraphicDimensional& SetText(CStringW str)
	{
		lbl->SetText(str);
		return *this;
	}

	GraphicDimensional& StartPoint(Gdiplus::PointF pos)
	{
		m_StartPoint = pos;
		return *this;
	}

	GraphicDimensional& EndPoint(Gdiplus::PointF pos)
	{
		m_EndPoint = pos;
		return *this;
	}

	GraphicDimensional& SetWidth(float width)
	{
		m_Pen->SetWidth(width);
		return *this;
	}

	GraphicDimensional& Render(Gdiplus::Graphics& graph, const Rigid& trans) override
	{
		Gdiplus::PointF st1, st2;
		Gdiplus::PointF ed1, ed2;

		auto st = trans.Transform(m_StartPoint);
		auto ed = trans.Transform(m_EndPoint);
		Point2Point pp(st, ed);
		auto dspTH = atan(pp.dy / (pp.dx + std::numeric_limits<double>::epsilon())) * 180. / CV_PI;

		GetArrowPoints(ed, m_ArrowLen, pp.th + CV_PI, ed1, ed2);
		GetArrowPoints(st, m_ArrowLen, pp.th, st1, st2);

		auto chk = graph.Save();
		graph.TranslateTransform((st.X+ed.X)/2., (st.Y+ed.Y)/2.);
		graph.RotateTransform(dspTH);
		lbl->Render(graph, Rigid());
		graph.ResetTransform();

		graph.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias);
		graph.DrawLine(m_Pen, st, ed);
		graph.DrawLine(m_Pen, ed, ed1);
		graph.DrawLine(m_Pen, ed, ed2);
		graph.DrawLine(m_Pen, st, st1);
		graph.DrawLine(m_Pen, st, st2);
		
		graph.Restore(chk);
		return *this;
	}

	GraphicDimensional& Render(Gdiplus::Graphics& graph, const Rigid& trans, CSize maxSize) override
	{
		Gdiplus::PointF st1, st2;
		Gdiplus::PointF ed1, ed2;

		auto st = trans.Transform(m_StartPoint);
		auto ed = trans.Transform(m_EndPoint);
		Point2Point pp(st, ed);
		auto dspTH = atan(pp.dy / (pp.dx + std::numeric_limits<double>::epsilon())) * 180. / CV_PI;

		GetArrowPoints(ed, m_ArrowLen, pp.th + CV_PI, ed1, ed2);
		GetArrowPoints(st, m_ArrowLen, pp.th, st1, st2);

		auto chk = graph.Save();
		graph.TranslateTransform((st.X + ed.X) / 2., (st.Y + ed.Y) / 2.);
		graph.RotateTransform(dspTH);
		lbl->Render(graph, Rigid());
		graph.ResetTransform();

		graph.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias);
		graph.DrawLine(m_Pen, st, ed);
		graph.DrawLine(m_Pen, ed, ed1);
		graph.DrawLine(m_Pen, ed, ed2);
		graph.DrawLine(m_Pen, st, st1);
		graph.DrawLine(m_Pen, st, st2);

		graph.Restore(chk);
		return *this;
	}

	~GraphicDimensional()
	{
		delete lbl;
	}

protected:

private:
	Gdiplus::PointF m_StartPoint;
	Gdiplus::PointF m_EndPoint;
	Gdiplus::Pen* m_Pen;
	GraphicLabel* lbl;
	float m_ArrowLen = 11;
	bool m_TwoEnds = false;
};
