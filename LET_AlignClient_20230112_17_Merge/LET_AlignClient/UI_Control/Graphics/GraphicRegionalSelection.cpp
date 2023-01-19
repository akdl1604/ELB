#pragma once
#include "stdafx.h"
#include "GraphicBase.h"
#include "GraphicLabel.h"
#include "GraphicHitTest.h"
#include "GraphicRegionalSelection.h"

/*
* - Lincoln Lee
* - x.perfect.vn@gmail.com
* - 2022-08-08
*/

// Cursor map
//    0      1       2
//    3              4
//    5      6       7

// Hittest by priority
//    6      2       1
//    NW     N      NE
// 4  W              E  5
//    SW     S      SE
//    0      3      7

GraphicRegionalSelection& GraphicRegionalSelection::RePosition(CPoint st, CPoint ed, const Rigid& imgToView)
{
	auto viewToImg = imgToView.Inverse();
	auto stx = viewToImg.Transform(st);
	auto edx = viewToImg.Transform(ed);

	auto dx = ed.x - st.x;
	auto dy = ed.y - st.y;

	auto tlx = MIN(stx.x, edx.x);
	auto tly = MIN(stx.y, edx.y);
	auto brx = MAX(stx.x, edx.x);
	auto bry = MAX(stx.y, edx.y);

	if (dx >= 0)
		m_LastHitId = (dy >= 0) ? 7 : 1;
	else
		m_LastHitId = (dy >= 0) ? 0 : 6;

	m_IsMouseDown = true;
	m_LastHit = true;

	m_LastMousePos = ed;
	ChangeSelectedRegion(cv::Rect2d(tlx, tly, brx - tlx, bry - tly));
	return *this;
};

GraphicRegionalSelection::GraphicRegionalSelection()
{
	Init(cv::Rect2d());
}

GraphicRegionalSelection::GraphicRegionalSelection(CRect rect)
{
	Init(cv::Rect2d(rect.left, rect.top, rect.Width(), rect.Height()));
}

GraphicRegionalSelection::GraphicRegionalSelection(cv::Rect2d rect)
{
	Init(rect);
}

GraphicRegionalSelection& GraphicRegionalSelection::SetMetricTransform(const Rigid& rigid)
{
	m_MetricTrans = rigid;
	return *this;
}

GraphicRegionalSelection& GraphicRegionalSelection::SetShowCross(bool show)
{
	m_ShowCross = show;
	return *this;
}

GraphicRegionalSelection& GraphicRegionalSelection::SetShowEllipse(bool show)
{
	m_ShowEllipse = show;
	return *this;
}

GraphicRegionalSelection& GraphicRegionalSelection::SetShowDigonal(bool show)
{
	m_ShowDigonal = show;
	return *this;
}

GraphicRegionalSelection& GraphicRegionalSelection::SetShowCenter(bool show)
{
	m_ShowCenter = show;
	return *this;
}

GraphicRegionalSelection& GraphicRegionalSelection::SetSelectedRegion(const CRect& rect)
{
	return SetSelectedRegion(cv::Rect(rect.left, rect.top, rect.Width(), rect.Height()));
}

template<class T>
struct LineEQU_
{
	T a;
	T b;

	template<class T0, class T1>
	LineEQU_(T0 _a = 0, T1 _b = 0) : a(_a), b(_b)
	{}

	LineEQU_(const Gdiplus::PointF st, const Gdiplus::PointF ed)
	{
		auto dx = ed.X - st.X;
		auto dy = ed.Y - st.Y;

		if (dx == 0)
		{
			a = 1;
			b = st.X;
		}
		else if (dy == 0)
		{
			a = 0;
			b = st.Y;
		}
		else
		{
			a = dy / dx;
			b = st.Y - (a * st.X);
		}
	}

	LineEQU_(const Gdiplus::Point st, const Gdiplus::Point ed)
	{
		auto dx = ed.X - st.X;
		auto dy = ed.Y - st.Y;

		if (dx == 0)
		{
			a = 0;
			b = 0;
		}
		else if (dy == 0)
		{
			a = 0;
			b = st.Y;
		}
		else
		{
			a = dy / dx;
			b = st.Y - (a * st.X);
		}
	}

	LineEQU_(const CPoint st, const CPoint ed)
	{
		auto dx = ed.x - st.x;
		auto dy = ed.y - st.y;

		if (dx == 0)
		{
			a = 0;
			b = 0;
		}
		else if (dy == 0)
		{
			a = 0;
			b = st.y;
		}
		else
		{
			a = dy / dx;
			b = st.y - (a * st.x);
		}
	}

	template<class T0, class T1>
	LineEQU_(const cv::Point_<T0> st, const cv::Point_<T1> ed)
	{
		auto dx = ed.x - st.x;
		auto dy = ed.y - st.y;

		if (dx == 0)
		{
			a = 0;
			b = 0;
		}
		else if (dy == 0)
		{
			a = 0;
			b = st.y;
		}
		else
		{
			a = dy / dx;
			b = st.y - (a * st.x);
		}
	}
};
using LineEQU = LineEQU_<float>;

GraphicBase& GraphicRegionalSelection::Render(Gdiplus::Graphics& graph, const Rigid& imgToView)
{
	auto cen = GetCenter(m_SelectedRegion);
	auto dspRECT = imgToView.Transform(m_SelectedRegion);
	auto cen_ = imgToView.Transform(cen);
	auto metricRECT = m_MetricTrans.Transform(m_SelectedRegion);

	auto w = metricRECT.width;
	auto h = metricRECT.height;

	auto tl = dspRECT.tl();
	auto br = dspRECT.br();
	auto w4 = dspRECT.width / 4.;
	auto h4 = dspRECT.height / 4.;

	Gdiplus::RectF rect(dspRECT.x, dspRECT.y, dspRECT.width, dspRECT.height);

	//.SetText(fmt("W: %.3fmm, H: %.3fmm, D: %.3fmm", w, h, std::sqrt(w * w + h * h)))
	// Width, Height, Digonal
	m_SelectionInfo.SetDisplayLocation(cen.x, m_SelectedRegion.y)
		.SetText(fmt("[%.3f, %.3f, %.3f](W H D)(mm)", w, h, std::sqrt(w * w + h * h)))
		.Render(graph, imgToView);

	auto chk = graph.Save();

	graph.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias);
	if (m_ShowDigonal || m_ShowEllipse)
	{
		if (m_ShowEllipse || m_ShowDigonal) // Digonal
		{
			Gdiplus::RectF viewSIZE;
			graph.GetVisibleClipBounds(&viewSIZE);

			LineEQU line0(tl, br);
			LineEQU line1(cv::Point2f(tl.x, br.y), cv::Point2f(br.x, tl.y));

			//auto xx = (line1.b - line0.b) / (line0.a - line1.a);
			//auto yy = xx * line0.a + line0.b;


			auto l = tl.x;
			auto t = tl.y;
			auto r = br.x;
			auto b = br.y;

			if (tl.x < 0)
			{
				l = 0;
				t = line0.b;
			}
			else if (br.x > viewSIZE.GetRight())
			{
				r = viewSIZE.GetRight();
				b = line0.a * r + line0.b;
			}

			if (t < 0)
			{
				t = 0;
				l = -line1.b / line1.a;
			}
			else if (b > viewSIZE.GetBottom())
			{
				b = viewSIZE.GetBottom();
				r = (b - line1.b) / line1.a;
			}

			graph.DrawLine(m_DashPen, Gdiplus::PointF(l, t), Gdiplus::PointF(r, b));
			graph.DrawLine(m_DashPen, Gdiplus::PointF(l, b), Gdiplus::PointF(r, t));

			//graph.DrawLine(m_DashPen, Gdiplus::PointF(tl.x, tl.y), Gdiplus::PointF(br.x, br.y));
			//graph.DrawLine(m_DashPen, Gdiplus::PointF(tl.x, br.y), Gdiplus::PointF(br.x, tl.y));
		}
		if (m_ShowEllipse) graph.DrawEllipse(m_EllipsePen, rect);
	}

	if (m_ShowCenter)
	{
		graph.DrawLine(m_SolidPen, Gdiplus::PointF(tl.x, cen_.y), Gdiplus::PointF(br.x, cen_.y));
		graph.DrawLine(m_SolidPen, Gdiplus::PointF(cen_.x, tl.y), Gdiplus::PointF(cen_.x, br.y));

		//Gdiplus::Pen pen(0xff00ff00, 3);
		//Gdiplus::Pen pen1(0xff0000ff, 3);
		//DrawCrossGraphic(graph, &pen, Gdiplus::PointF(cen_.x, cen_.y));


		//Gdiplus::RectF viewSIZE;
		//graph.GetVisibleClipBounds(&viewSIZE);

		//LineEQU line0(tl, br);
		//LineEQU line1(cv::Point2f(tl.x, br.y), cv::Point2f(br.x, tl.y));

		//auto xx = (line1.b - line0.b) / (line0.a - line1.a);
		//auto yy = xx * line0.a + line0.b;

		//DrawCrossGraphic(graph, &pen1, Gdiplus::PointF(xx, yy));
	}

	graph.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeNone);
	if (m_ShowCross)
	{
		DrawCrossGraphic(graph, m_SolidPen, Gdiplus::PointF(tl.x + w4, tl.y + h4));
		DrawCrossGraphic(graph, m_SolidPen, Gdiplus::PointF(tl.x + w4 * 3, tl.y + h4));

		DrawCrossGraphic(graph, m_SolidPen, Gdiplus::PointF(tl.x + w4, tl.y + h4 * 3));
		DrawCrossGraphic(graph, m_SolidPen, Gdiplus::PointF(tl.x + w4 * 3, tl.y + h4 * 3));
	}
	if (!m_ShowEllipse) 
		graph.DrawRectangle(m_BoderPen, rect);

	// Render from lowest to highest to make highest ontop
	for (int i = 0; i < 8; i++)
		m_HitTest[i].Render(graph, imgToView);

	graph.Restore(chk);
	return *this;
}

bool GraphicRegionalSelection::MouseDown(CPoint pos, const Rigid& trans)
{
	bool hit = false;

	m_LastHit = false;
	m_LastHitId = -1;

	// Check for event from highest to lowest to make highest ontop
	for (int i = 0; i < 8; i++)
		if ((hit = m_HitTest[7 - i].MouseDown(pos, trans)))
			break;

	if (!hit)
	{
		auto rect = trans.Transform(m_SelectedRegion);
		hit = IsPointInRect(rect, pos);
	}

	if (hit)
	{
		m_LastMousePos = pos;
		m_IsMouseDown = true;
	}

	return hit;
}

bool GraphicRegionalSelection::MouseUp(CPoint pos, const Rigid& trans)
{
	cv::Rect2f r;
	bool refresh = SwapRect(m_SelectedRegion, r);
	m_IsMouseDown = false;

	if (refresh) ChangeSelectedRegion(r);

	m_LastHitId = -1;
	m_LastHit = false;
	return refresh;
}

bool GraphicRegionalSelection::MouseMove(CPoint pos, const Rigid& imgToView)
{
	bool hit = false;
	int hitId = -1;

	if (m_IsMouseDown && m_LastHit && m_LastHitId > -1)
	{
		auto viewToImg = imgToView.Inverse();
		auto pt0 = viewToImg.Transform(m_LastMousePos);
		auto pt1 = viewToImg.Transform(pos);

		DoResizeByHitIds[m_LastHitId % 9](pt1 - pt0);
		m_LastMousePos = pos;
		hit = true;
	}
	else if (m_IsMouseDown)
	{
		for (int i = 0; i < 8; i++)
		{
			hitId = 7 - i;
			if ((hit = m_HitTest[hitId].MouseMove(pos, imgToView)))
				break;
		}

		if (hit)
		{
			auto viewToImg = imgToView.Inverse();
			auto pt0 = viewToImg.Transform(m_LastMousePos);
			auto pt1 = viewToImg.Transform(pos);

			DoResizeByHitIds[hitId % 9](pt1 - pt0);
			m_LastHitId = hitId;
			m_LastMousePos = pos;
		}
		else
		{
			auto rect = imgToView.Transform(m_SelectedRegion);
			if ((hit = IsPointInRect(rect, pos)))
			{
				auto viewToImg = imgToView.Inverse();
				auto pt0 = viewToImg.Transform(m_LastMousePos);
				auto pt1 = viewToImg.Transform(pos);

				DoResizeByHitIds[8](pt1 - pt0);
				m_LastHitId = 8;
				m_LastMousePos = pos;

				SetCursor(m_Cursor);
			}
			else SetCursor(m_CursorNormal);
		}
		m_LastHit = hit;
	}
	return hit;
}

bool GraphicRegionalSelection::MouseHover(CPoint pos, const Rigid& trans)
{
	bool hit = false;
	for (int i = 0; i < 8; i++)
		if ((hit = m_HitTest[7 - i].MouseHover(pos, trans)))
			break;

	if (!hit)
	{
		auto rect = trans.Transform(m_SelectedRegion);
		hit = IsPointInRect(rect, pos);

		SetCursor(hit ? m_Cursor : m_CursorNormal);
	}

	return false;
}

cv::Rect GraphicRegionalSelection::GetSelectedRegion(cv::Size ContrainsSize) const
{
	return ContrainsRECT3(m_SelectedRegion, ContrainsSize);
}

cv::Rect2f GraphicRegionalSelection::GetSelectedRegion() const
{
	return m_SelectedRegion;
}

void GraphicRegionalSelection::ClearMouseState()
{
	m_IsMouseDown = false;
}

CRect GraphicRegionalSelection::GetSelectedRect() const
{
	auto tl = m_SelectedRegion.tl();
	auto br = m_SelectedRegion.br();
	return CRect(CPoint(lround(tl.x), lround(tl.y)), CPoint(lround(br.x), lround(br.y)));
}

GraphicRegionalSelection& GraphicRegionalSelection::GetBeginEnd(CPoint& st, CPoint& ed)
{
	auto tl = m_SelectedRegion.tl();
	auto br = m_SelectedRegion.br();
	st.x = lround(tl.x);
	st.y = lround(tl.y);
	ed.x = lround(br.x);
	ed.y = lround(br.y);
	return *this;
}

GraphicRegionalSelection& GraphicRegionalSelection::SetBeginEnd(const CPoint& st, const CPoint& ed)
{
	//auto br = m_SelectedRegion.br();
	//auto dx0 = st.x - m_SelectedRegion.x;
	//auto dy0 = st.y - m_SelectedRegion.y;
	//auto dx1 = ed.x - br.x;
	//auto dy1 = ed.y - br.y;

	//OffsetSelectedRegion(dx0, dy0, dx1, dy1);
	ChangeSelectedRegion(cv::Rect(st.x, st.y, ed.x - st.x, ed.y - st.y));

	return *this;
}

GraphicRegionalSelection& GraphicRegionalSelection::Settings(const CPoint& st, const CPoint& ed, bool ellipseMODE)
{
	//auto br = m_SelectedRegion.br();
	//auto dx0 = st.x - m_SelectedRegion.x;
	//auto dy0 = st.y - m_SelectedRegion.y;
	//auto dx1 = ed.x - br.x;
	//auto dy1 = ed.y - br.y;

	m_ShowEllipse = ellipseMODE;
	//OffsetSelectedRegion(dx0, dy0, dx1, dy1);
	ChangeSelectedRegion(cv::Rect(st.x, st.y, ed.x - st.x, ed.y - st.y));
	return *this;
}

GraphicRegionalSelection& GraphicRegionalSelection::GetSettings(CPoint& st, CPoint& ed, bool& ellipseMODE)
{
	auto tl = m_SelectedRegion.tl();
	auto br = m_SelectedRegion.br();
	st.x = lround(tl.x);
	st.y = lround(tl.y);
	ed.x = lround(br.x);
	ed.y = lround(br.y);
	ellipseMODE = m_ShowEllipse;
	return *this;
}

GraphicRegionalSelection::~GraphicRegionalSelection()
{
	delete m_BoderPen;
	delete m_DashPen;
	delete m_SolidPen;
	delete m_EllipsePen;
	DoResizeByHitIds.clear();
}