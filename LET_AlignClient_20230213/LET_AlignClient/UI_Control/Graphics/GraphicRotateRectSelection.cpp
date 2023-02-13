#pragma once
#include "stdafx.h"
#include "GraphicBase.h"
#include "GraphicLabel.h"
#include "GraphicHitTest.h"
#include "GraphicRotateRectSelection.h"

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

GraphicRotateRectSelection& GraphicRotateRectSelection::RePosition(CPoint st, CPoint ed, const Rigid& imgToView)
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

GraphicRotateRectSelection::GraphicRotateRectSelection()
{
	Init(cv::Rect2d());
}

GraphicRotateRectSelection::GraphicRotateRectSelection(CRect rect)
{
	Init(cv::Rect2d(rect.left, rect.top, rect.Width(), rect.Height()));
}

GraphicRotateRectSelection::GraphicRotateRectSelection(cv::Rect2d rect)
{
	Init(rect);
}

GraphicRotateRectSelection& GraphicRotateRectSelection::SetMetricTransform(const Rigid& rigid)
{
	m_MetricTrans = rigid;
	return *this;
}

GraphicRotateRectSelection& GraphicRotateRectSelection::SetShowDigonal(bool show)
{
	m_ShowDigonal = show;
	return *this;
}

GraphicRotateRectSelection& GraphicRotateRectSelection::SetShowCenter(bool show)
{
	m_ShowCenter = show;
	return *this;
}

GraphicRotateRectSelection& GraphicRotateRectSelection::SetShowRotate(bool show)
{
	m_ShowRotate = show;
	return *this;
}

GraphicRotateRectSelection& GraphicRotateRectSelection::SetSelectedRegion(const CRect& rect)
{
	return SetSelectedRegion(cv::Rect(rect.left, rect.top, rect.Width(), rect.Height()));
}

GraphicBase& GraphicRotateRectSelection::Render(Gdiplus::Graphics& graph, const Rigid& imgToView)
{
	auto cen = GetCenter(m_SelectedRegion);
	auto dspRECT = imgToView.Transform(m_SelectedRegion);
	auto cen_ = imgToView.Transform(cen);
	auto metricRECT = m_MetricTrans.Transform(m_SelectedRegion);
	auto viewToImg = imgToView.Inverse();

	auto w = metricRECT.width;
	auto h = metricRECT.height;

	auto tl = dspRECT.tl();
	auto br = dspRECT.br();
	auto w4 = dspRECT.width / 4.;
	auto h4 = dspRECT.height / 4.;
	
	Gdiplus::RectF rect(dspRECT.x, dspRECT.y, dspRECT.width, dspRECT.height);

	m_SelectionInfo.SetDisplayLocation(cen.x, m_SelectedRegion.y)
		.SetText(fmt("[%.3f, %.3f, %.3f , %.3f¡Æ](W H D)(mm)", w, h, std::sqrt(w * w + h * h), m_CurPhi))
		.Render(graph, imgToView);

	auto chk = graph.Save();

	graph.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeAntiAlias);
	if (m_PointDest[0].x == 0 && m_PointDest[0].y == 0)
	{
		if (m_ShowCenter)
		{
			graph.DrawLine(m_SolidPen, Gdiplus::PointF(tl.x, cen_.y), Gdiplus::PointF(br.x, cen_.y));
			graph.DrawLine(m_SolidPen, Gdiplus::PointF(cen_.x, tl.y), Gdiplus::PointF(cen_.x, br.y));
		}

		graph.DrawRectangle(m_BoderPen, rect);
	}
	
	if (m_ShowRotate)
	{
		double pi = 4.0 * atan(1.0);		
		auto pt0 = viewToImg.Transform(m_LastMousePos);

		CSize sz = CSize(pt0.x - m_FixedDest.x, pt0.y - m_FixedDest.y);
		
		m_CurPhi = 180.0f * atan2((Gdiplus::REAL)sz.cy, (Gdiplus::REAL)sz.cx) / (Gdiplus::REAL)pi - m_StartPhi;
		while (m_CurPhi <= -180.0f) m_CurPhi += 360.0f;	// (-180, 180]
		m_Transform.Reset();
		m_Transform.Translate((Gdiplus::REAL)-m_FixedSrc.x, (Gdiplus::REAL)-m_FixedSrc.y);
		m_Transform.Rotate(m_CurPhi, Gdiplus::MatrixOrderAppend);
		m_Transform.Translate((Gdiplus::REAL)m_FixedDest.x, (Gdiplus::REAL)m_FixedDest.y, Gdiplus::MatrixOrderAppend);

		::CopyMemory(m_PointDest, m_PointSrc, 5 * sizeof(POINT));
		m_Transform.TransformPoints((Gdiplus::Point*)m_PointDest, 5);

		cv::Rect2f r;
		int min_x = 99999, max_x = 0, min_y = 99999, max_y = 0;

		for (int j = 0; j < 4; j++)
		{
			min_x = MIN(min_x, m_PointDest[j].x);
			max_x = MAX(max_x, m_PointDest[j].x);
			min_y = MIN(min_y, m_PointDest[j].y);
			max_y = MAX(max_y, m_PointDest[j].y);
		}
		r.x = min_x;
		r.y = min_y;
		r.width = max_x - min_x;
		r.height = max_y - min_y;

		ChangeSelectedRegion(r);
	}
	
	if (m_PointDest[0].x != 0 || m_PointDest[0].y != 0)
	{
		Gdiplus::PointF PointDest[5];
		graph.SetSmoothingMode(Gdiplus::SmoothingMode::SmoothingModeNone);

		for (int j = 0; j < 4; j++)
		{
			auto dspPt = imgToView.Transform(m_PointDest[j]);
			PointDest[j].X = dspPt.x;
			PointDest[j].Y = dspPt.y;
		}

		auto pt1 = Gdiplus::PointF((PointDest[0].X + PointDest[1].X) / 2, (PointDest[0].Y + PointDest[1].Y) / 2);
		auto pt2 = Gdiplus::PointF((PointDest[2].X + PointDest[3].X) / 2, (PointDest[2].Y + PointDest[3].Y) / 2);
		auto pt3 = Gdiplus::PointF((PointDest[0].X + PointDest[3].X) / 2, (PointDest[0].Y + PointDest[3].Y) / 2);
		auto pt4 = Gdiplus::PointF((PointDest[2].X + PointDest[1].X) / 2, (PointDest[2].Y + PointDest[1].Y) / 2);

		graph.DrawPolygon(m_EllipsePen, (Gdiplus::PointF*)PointDest, 4);
		graph.DrawLine(m_EllipseInPen, pt1, pt2);
		graph.DrawLine(m_EllipseInPen, pt3, pt4);

		CalculateInfo(viewToImg.Transform(pt3), viewToImg.Transform(pt4), viewToImg.Transform(pt1), viewToImg.Transform(pt2));
		m_Dimension.Render(graph, imgToView);
	}

	// Render from lowest to highest to make highest ontop
	for (int i = 0; i < 8; i++)
		m_HitTest[i].Render(graph, imgToView);

	graph.Restore(chk);
	return *this;
}

bool GraphicRotateRectSelection::MouseDown(CPoint pos, const Rigid& trans)
{
	bool hit = false;
	int i = 0;
	m_LastHit = false;
	m_LastHitId = -1;

	// Check for event from highest to lowest to make highest ontop
	for (i = 0; i < 8; i++)
		if ((hit = m_HitTest[7 - i].MouseDown(pos, trans)))
			break;

	if (!hit)
	{
		auto rect = trans.Transform(m_SelectedRegion);
		hit = IsPointInRect(rect, pos);
	}

	m_LastMousePos = pos;

	if (hit)
	{
		m_IsMouseDown = true;

		bool bCtrl = ::GetKeyState(VK_CONTROL) < 0;
		int handle = 7 - i;

		if (bCtrl && (handle < 2 || handle > 5))
		{
			SetCursor(m_RotateCursor);

			if (m_CurPhi == 0)		SetPoints(true);								
			else
			{
				auto p1 = cv::Point2f((m_PointDest[0].x + m_PointDest[1].x) / 2, (m_PointDest[0].y + m_PointDest[1].y) / 2);
				auto p2 = cv::Point2f((m_PointDest[2].x + m_PointDest[3].x) / 2, (m_PointDest[2].y + m_PointDest[3].y) / 2);
				auto p3 = cv::Point2f((m_PointDest[0].x + m_PointDest[3].x) / 2, (m_PointDest[0].y + m_PointDest[3].y) / 2);
				auto p4 = cv::Point2f((m_PointDest[2].x + m_PointDest[1].x) / 2, (m_PointDest[2].y + m_PointDest[1].y) / 2);

				cv::Point2f ret = cv::Point2f(0, 0);

				if (((p1.x - p2.x) * (p3.y - p4.y) - (p1.y - p2.y) * (p3.x - p4.x)) != 0)
				{
					ret.x = ((p1.x * p2.y - p1.y * p2.x) * (p3.x - p4.x) - (p1.x - p2.x) * (p3.x * p4.y - p3.y * p4.x)) / ((p1.x - p2.x) * (p3.y - p4.y) - (p1.y - p2.y) * (p3.x - p4.x));
					ret.y = ((p1.x * p2.y - p1.y * p2.x) * (p3.y - p4.y) - (p1.y - p2.y) * (p3.x * p4.y - p3.y * p4.x)) / ((p1.x - p2.x) * (p3.y - p4.y) - (p1.y - p2.y) * (p3.x - p4.x));
				}

				m_PointSrc[0].x = m_PointSrc[3].x = m_SelectedRegion.tl().x;
				m_PointSrc[1].x = m_PointSrc[2].x = m_SelectedRegion.br().x;
				m_PointSrc[0].y = m_PointSrc[1].y = m_SelectedRegion.tl().y;
				m_PointSrc[2].y = m_PointSrc[3].y = m_SelectedRegion.br().y;

				m_PointSrc[4] = m_PointDest[4] = CPoint(ret.x, ret.y);				
			}

			m_FixedDest = m_FixedSrc = m_PointSrc[4];

			double pi = 4.0 * atan(1.0);
			CSize sz = CPoint(m_PointSrc[2]) - m_FixedSrc;

			if (handle == 6)	    sz = CPoint(m_PointSrc[0]) - m_FixedSrc;
			else if (handle == 1)	sz = CPoint(m_PointSrc[1]) - m_FixedSrc;
			else if (handle == 0)	sz = CPoint(m_PointSrc[3]) - m_FixedSrc;

			m_StartPhi = 180.0f * (Gdiplus::REAL)atan2((Gdiplus::REAL)sz.cy, (Gdiplus::REAL)sz.cx) / (Gdiplus::REAL)pi;

			if (m_CurPhi != 0)	
				m_StartPhi = m_StartPhi - m_CurPhi;

			m_ShowRotate = true;
		}
	}
	else
	{
		m_CurPhi = 0;
		memset(m_PointDest, 0, 5 * sizeof(POINT));
	}

	return hit;
}

bool GraphicRotateRectSelection::MouseUp(CPoint pos, const Rigid& trans)
{
	cv::Rect2f r;

	bool refresh = SwapRect(m_SelectedRegion, r);
	m_IsMouseDown = false;
	m_ShowRotate = false;

	if (refresh) ChangeSelectedRegion(r);

	m_LastHitId = -1;
	m_LastHit = false;
	return refresh;
}

bool GraphicRotateRectSelection::MouseMove(CPoint pos, const Rigid& imgToView)
{
	bool hit = false;
	int hitId = -1;

	auto viewToImg = imgToView.Inverse();
	auto pt0 = viewToImg.Transform(m_LastMousePos);
	auto pt1 = viewToImg.Transform(pos);
	auto pt2 = pt1 - pt0;

	if (m_IsMouseDown && m_LastHit && m_LastHitId > -1)
	{
		if(m_PointDest[0].x!=0 || m_PointDest[0].y != 0)	
			OffsetRotateRegion(pt2.x, pt2.y, m_LastHitId % 9);
		else	DoResizeByHitIds[m_LastHitId % 9](pt2);
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
			if (m_PointDest[0].x != 0 || m_PointDest[0].y != 0)	
				OffsetRotateRegion(pt2.x, pt2.y, hitId % 9);
			else	DoResizeByHitIds[hitId % 9](pt2);
			m_LastHitId = hitId;
			m_LastMousePos = pos;
		}
		else
		{
			auto rect = imgToView.Transform(m_SelectedRegion);
			if ((hit = IsPointInRect(rect, pos)))
			{
				if (m_PointDest[0].x != 0 || m_PointDest[0].y != 0)	
					OffsetRotateRegion(pt2.x, pt2.y,8);
				else	DoResizeByHitIds[8](pt2);
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

bool GraphicRotateRectSelection::MouseHover(CPoint pos, const Rigid& trans)
{
	bool hit = false;
	int i = 0;
	for (i = 0; i < 8; i++)
		if ((hit = m_HitTest[7 - i].MouseHover(pos, trans)))
			break;

	if (!hit)
	{
		auto rect = trans.Transform(m_SelectedRegion);
		hit = IsPointInRect(rect, pos);

		SetCursor(hit ? m_Cursor : m_CursorNormal);
	}
	else
	{
		bool bCtrl = ::GetKeyState(VK_CONTROL) < 0;

		if (bCtrl && ((7-i)<2 || (7 - i) >5))
			SetCursor(m_RotateCursor);
	}

	return false;
}

void GraphicRotateRectSelection::SetPoints(bool bSetCenter)
{
	m_PointSrc[0].x = m_PointSrc[3].x = m_SelectedRegion.tl().x;
	m_PointSrc[1].x = m_PointSrc[2].x = m_SelectedRegion.br().x;
	m_PointSrc[0].y = m_PointSrc[1].y = m_SelectedRegion.tl().y;
	m_PointSrc[2].y = m_PointSrc[3].y = m_SelectedRegion.br().y;

	auto cen = GetCenter(m_SelectedRegion);

	if (bSetCenter) m_PointSrc[4] = CPoint(cen.x, cen.y);
	else m_PointSrc[4] = m_PointDest[4];

	::CopyMemory(m_PointDest, m_PointSrc, 5 * sizeof(POINT));
}

cv::Rect GraphicRotateRectSelection::GetSelectedRegion(cv::Size ContrainsSize) const
{
	return ContrainsRECT3(m_SelectedRegion, ContrainsSize);
}

cv::Rect2f GraphicRotateRectSelection::GetSelectedRegion() const
{
	return m_SelectedRegion;
}

void GraphicRotateRectSelection::ClearMouseState()
{
	m_IsMouseDown = false;
}

CRect GraphicRotateRectSelection::GetSelectedRect() const
{
	auto tl = m_SelectedRegion.tl();
	auto br = m_SelectedRegion.br();
	return CRect(CPoint(lround(tl.x), lround(tl.y)), CPoint(lround(br.x), lround(br.y)));
}

GraphicRotateRectSelection& GraphicRotateRectSelection::GetBeginEnd(CPoint& st, CPoint& ed)
{
	auto tl = m_SelectedRegion.tl();
	auto br = m_SelectedRegion.br();
	st.x = lround(tl.x);
	st.y = lround(tl.y);
	ed.x = lround(br.x);
	ed.y = lround(br.y);
	return *this;
}

GraphicRotateRectSelection& GraphicRotateRectSelection::SetBeginEnd(const CPoint& st, const CPoint& ed)
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

GraphicRotateRectSelection& GraphicRotateRectSelection::Settings(const CPoint& st, const CPoint& ed, bool ellipseMODE)
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

GraphicRotateRectSelection& GraphicRotateRectSelection::GetSettings(CPoint& st, CPoint& ed, bool& ellipseMODE)
{
	auto tl = m_SelectedRegion.tl();
	auto br = m_SelectedRegion.br();
	st.x = lround(tl.x);
	st.y = lround(tl.y);
	ed.x = lround(br.x);
	ed.y = lround(br.y);
	return *this;
}

GraphicRotateRectSelection::~GraphicRotateRectSelection()
{
	delete m_BoderPen;
	delete m_DashPen;
	delete m_SolidPen;
	delete m_EllipsePen;
	delete m_EllipseInPen;
	DoResizeByHitIds.clear();
}