#pragma once
#include "stdafx.h"
#include "GraphicBase.h"
#include "GraphicLabel.h"
#include "GraphicHitTest.h"
#include "Rigid.h"
#include "GraphicArrow.h"
#include "GraphicDimensional.h"
#include "PolyPoints.h"
#include <opencv.hpp>

/*
* - Lincoln Lee
* - x.perfect.vn@gmail.com
* - 2022-08-08
*/


class GraphicInteractiveCalipers: public GraphicBase
{
public:
	bool IsInterative() const { return true; }
	bool IsActive() const { return m_Active; }
	bool IsMouseDown() const { return m_IsMouseDown; }
	GraphicInteractiveCalipers& SetActive(bool active = true) { m_Active = active; return *this; }

	bool IsRePosition() const { return true; }
	GraphicInteractiveCalipers& RePosition(CPoint st, CPoint ed, const Rigid& imgToView)
	{
		auto viewToImage = imgToView.Inverse();
		auto stx = viewToImage.Transform(st);
		auto edx = viewToImage.Transform(ed);
		auto dx = ed.x - st.x;

		m_InternalReverse = dx < 0;
		m_IsMouseDown = true;
		m_LastHit = true;
		m_LastHitId = m_InternalReverse ? 0: 1;
		m_LastMousePos = ed;

		CalculateCalipers(Gdiplus::PointF(stx.x, stx.y), Gdiplus::PointF(edx.x, edx.y));

		return *this;
	}

	bool IsCircleMode() const { return m_CircleMode; }
	bool IsReverse() const { return m_ReverseDirection; }
	Gdiplus::PointF StartPoint() const { return m_StPOS; }
	Gdiplus::PointF EndPoint() const { return m_EdPOS; }

	GraphicInteractiveCalipers& SetCircleMode(bool val = true) { m_CircleMode = val; CalculateCalipers(); return *this; }
	GraphicInteractiveCalipers& ShowCenterCross(bool val = true) { m_CenterCross = val; return *this; }
	GraphicInteractiveCalipers& SetReverseMode(bool val = true) { m_ReverseDirection = val; CalculateCalipers(); return *this; }


	GraphicInteractiveCalipers(Gdiplus::PointF start, Gdiplus::PointF end, int nCaliper, float projLen, float searchLen, bool showCapilers = true, bool showBaseLine = true, bool circleMode = false)
		:m_StPOS(start), m_EdPOS(end), m_NumOfCalipers(nCaliper), m_ProjectionLength(projLen), m_SearchLength(searchLen),
		m_ShowBaseLine(showBaseLine), m_ShowCalipers(showCapilers), m_Pen(new Gdiplus::Pen(0xff00ff00, 1)), m_CircleMode(circleMode)
	{
		m_CursorArrow = LoadCursor(NULL, IDC_ARROW);
		m_CursorSizeAll = LoadCursor(NULL, IDC_SIZEALL);
		m_CursorCross = LoadCursor(NULL, IDC_CROSS);
		m_ArrowOne = new GraphicArrow(Gdiplus::PointF(0, 0), Gdiplus::PointF(0, 0), 11, 0xfff3b5e6, false, 5);
		m_ArrowTwo = new GraphicArrow(Gdiplus::PointF(0, 0), Gdiplus::PointF(0, 0), 11, 0xfff3b5e6, false, 5);

		CalculateCalipers(start, end, nCaliper, projLen, searchLen);
	}

	bool MouseDown(CPoint pos, const Rigid& imgToView)
	{
		auto hitId = -1;
		auto hit = false;
		auto st = imgToView.Transform(m_StPOS);
		auto ed = imgToView.Transform(m_EdPOS);

		hit = HittingTest(st, ed, cv::Point2f(pos.x, pos.y), hitId);
		hit &= m_Active;

		if (hit)
		{
			m_LastMousePos = pos;
			m_IsMouseDown = true;
		}

		return hit;
	}

	bool MouseUp(CPoint pos, const Rigid& trans)
	{
		m_LastHitId = -1;
		m_LastHit = false;
		m_IsMouseDown = false;
		return false;
	}

	bool MouseMove(CPoint pos, const Rigid& imgToView)
	{
		auto hitId = -1;
		auto hit = false;
		auto st = imgToView.Transform(m_StPOS);
		auto ed = imgToView.Transform(m_EdPOS);

		if (m_Active)
		{
			hit = HittingTest(st, ed, cv::Point2f(pos.x, pos.y), hitId);

			if (m_IsMouseDown)
			{
				auto viewToImg = imgToView.Inverse();
				auto pt0 = viewToImg.Transform(m_LastMousePos);
				auto pt1 = viewToImg.Transform(pos);
				auto dxy = pt1 - pt0;

				if (m_LastHit && m_LastHitId > -1)
				{
					hit = true;
					DoResizeByHitIds[m_LastHitId % 3](dxy);
				}
				else
				{
					if (hit)
					{
						m_LastHit = hit;
						m_LastHitId = hitId;
						DoResizeByHitIds[hitId % 3](dxy);
					}
				}

				m_LastMousePos = pos;
			}
		}

		return hit;
	}

	bool MouseHover(CPoint pos, const Rigid& imgToView)
	{
		auto hitId = 0;
		bool hit = false;

		auto st = imgToView.Transform(m_StPOS);
		auto ed = imgToView.Transform(m_EdPOS);
		HittingTest(st, ed, cv::Point2f(pos.x, pos.y), hitId);

		return false;
	}

	GraphicInteractiveCalipers& Render(Gdiplus::Graphics& graph, const Rigid& trans) override
	{
		auto st = trans.Transform(m_StPOS);
		auto ed = trans.Transform(m_EdPOS);

		auto chk = graph.Save();

		if (m_ShowBaseLine)
		{
			m_Pen->SetWidth(2);
			graph.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
			graph.DrawLine(m_Pen, st, ed);
		}

		if (m_ShowCalipers)
		{
			m_Pen->SetWidth(1);
			graph.SetSmoothingMode(Gdiplus::SmoothingModeNone);
			int i = 1;
			for (auto& poly: m_CaliperRects)
			{
				GraphicLabel Info;
				auto pts = trans.Transform(poly);
				graph.DrawPolygon(m_Pen, ((Gdiplus::PointF*)pts.data()), 4);	

				Info.SetFont(GraphicLabel::GetDefaultFont())
					.SetAnchor(Anchor::TopCenter)
					.SetTextColor(0xff0000ff)
					.SetWithBackground(false);

				Info.SetDisplayLocation((poly.tl.X+ poly.tr.X)/2.f, (poly.tl.Y + poly.tr.Y) / 2.f)
					.SetText(fmt("%d", i++))
					.Render(graph, trans);
			}
		}
		if (m_ShowArrow)
			m_ArrowOne->Render(graph, trans);

		if (m_CircleMode)
		{
			auto dist = DistancePointPoint(st, ed);
			cv::Point2f center((st.X + ed.X) / 2., (st.Y + ed.Y) / 2.);
			auto rect = GetRectGDI(center, cv::Size2f(dist, dist));

			m_ArrowTwo->Render(graph, trans);
			if(m_CenterCross) DrawCrossGraphic(graph, m_Pen, Gdiplus::PointF(center.x, center.y), 5);
			m_Pen->SetWidth(2);
			graph.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
			graph.DrawEllipse(m_Pen, rect);
		}

		if (m_ShowBaseLine)
		{
			auto r0 = GetRectGDI(cv::Point2f(st.X, st.Y), cv::Size(11, 11));
			auto r1 = GetRectGDI(cv::Point2f(ed.X, ed.Y), cv::Size(11, 11));

			m_Pen->SetWidth(1);
			graph.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
			graph.FillEllipse(m_HittestBrush, r0);
			graph.FillEllipse(m_HittestBrush, r1);
			graph.DrawEllipse(m_Pen, r0);
			graph.DrawEllipse(m_Pen, r1);
		}

		graph.Restore(chk);

		return *this;
	}

	GraphicInteractiveCalipers& SetMetricTransform(const Rigid& rigid)
	{
		return *this;
	}

	CRect GetSelectedRect() const
	{
		return CRect(CPoint(lround(m_StPOS.X), lround(m_StPOS.Y)), CPoint(lround(m_EdPOS.X), lround(m_EdPOS.Y)));
	}

	GraphicInteractiveCalipers& GetBeginEnd(CPoint& st, CPoint& ed)
	{
		st.x = lround(m_StPOS.X);
		st.y = lround(m_StPOS.Y);
		ed.x = lround(m_EdPOS.X);
		ed.y = lround(m_EdPOS.Y);

		return *this;
	}

	GraphicInteractiveCalipers& SetBeginEnd(const CPoint& st, const CPoint& ed)
	{
		m_StPOS.X = st.x;
		m_StPOS.Y = st.y;
		m_EdPOS.X = ed.x;
		m_EdPOS.Y = ed.y;
		CalculateCalipers();
		return *this;
	}

	GraphicInteractiveCalipers& Settings(const CPoint& st, const CPoint& ed, int nCount, int projLEN, int searchLEN, bool reverse, bool lineMODE)
	{
		m_StPOS.X = st.x;
		m_StPOS.Y = st.y;
		m_EdPOS.X = ed.x;
		m_EdPOS.Y = ed.y;
		m_NumOfCalipers = MAX(1, nCount);
		m_ProjectionLength = projLEN;
		m_SearchLength = searchLEN;
		m_ReverseDirection = reverse;
		m_CircleMode = !lineMODE;

		CalculateCalipers();
		return *this;
	}

	GraphicInteractiveCalipers& setNumOfCalipers(int nCount)
	{
		m_NumOfCalipers = MAX(1, nCount);
		CalculateCalipers();
		return *this;
	}

	GraphicInteractiveCalipers& setProjectionLength(int nLength)
	{
		m_ProjectionLength = MAX(1, nLength);
		CalculateCalipers();
		return *this;
	}

	GraphicInteractiveCalipers& setSearchLength(int nLength)
	{
		m_SearchLength = MAX(1, nLength);
		CalculateCalipers();
		return *this;
	}

	GraphicInteractiveCalipers& GetSettings(CPoint& st, CPoint& ed, int& nCount, int& projLEN, int& searchLEN, bool& reverse, bool& lineMODE)
	{
		st.x = lround(m_StPOS.X);
		st.y = lround(m_StPOS.Y);
		ed.x = lround(m_EdPOS.X);
		ed.y = lround(m_EdPOS.Y);
		nCount = m_NumOfCalipers;
		projLEN = m_ProjectionLength;
		searchLEN = m_SearchLength;
		reverse = m_ReverseDirection;
		lineMODE = !m_CircleMode;
		return *this;
	}

	~GraphicInteractiveCalipers()
	{
		delete m_Pen;
		delete m_HittestBrush;
		delete m_ArrowOne;
		delete m_ArrowTwo;
		DoResizeByHitIds.clear();
	}

private:
	Gdiplus::Pen* m_Pen;
	Gdiplus::Brush* m_HittestBrush = new Gdiplus::SolidBrush(0xC0404040); // 0xB0804040, 0xB0404040
	Gdiplus::PointF m_StPOS;
	Gdiplus::PointF m_EdPOS;

	int m_LastHitId = -1;
	int m_NumOfCalipers;
	float m_SearchLength;
	float m_ProjectionLength;

	bool m_LastHit = false;
	bool m_Active = false;
	bool m_ShowArrow = true;
	bool m_IsMouseDown = false;
	bool m_ShowCalipers = false;
	bool m_ShowBaseLine = true;
	bool m_CircleMode = true;
	bool m_InternalReverse = false;
	bool m_ReverseDirection = false;
	bool m_CenterCross = true;

	CPoint m_LastMousePos;

	HCURSOR m_CursorArrow;
	HCURSOR m_CursorSizeAll;
	HCURSOR m_CursorCross;

	GraphicArrow* m_ArrowOne;
	GraphicArrow* m_ArrowTwo;
	std::vector<PolyPoints> m_CaliperRects;

	std::vector<std::function<void(cv::Point2d& dxy)>> DoResizeByHitIds = {
		[&](cv::Point2d& dxy) { if (m_InternalReverse) ShiftCalipers(cv::Point2d(0, 0), dxy); else ShiftCalipers(dxy, cv::Point2d(0, 0)); },
		[&](cv::Point2d& dxy) { if (m_InternalReverse) ShiftCalipers(dxy, cv::Point2d(0, 0)); else ShiftCalipers(cv::Point2d(0, 0), dxy); },
		[&](cv::Point2d& dxy) { ShiftCalipers(dxy, dxy); }
	};
private:
	template<class T0>
	bool IsInsideRotatedRect(cv::Point_<T0>& pt, Gdiplus::PointF& center, float th, float w, float h)
	{
		auto w2 = w / 2.f;
		auto h2 = h / 2.f;
		auto ptx = Rigid(center, th).Transform(cv::Point2f(pt.x - center.X, pt.y - center.Y));
		return (ptx.x >= (center.X - w2) && ptx.x < (center.X + w2)) && (ptx.y >= (center.Y - h2) && ptx.y < (center.Y + h2));
	}

	template<class T0>
	bool IsOverInnerCircle(cv::Point_<T0>& pt, Gdiplus::PointF& center, float rad, float wh)
	{
		auto wh2 = wh / 2.;
		auto dist = DistancePointPoint(pt, center);
		return (dist >= (rad - wh2)) && (dist < (rad + wh2));
	}

	bool HittingTest(Gdiplus::PointF st, Gdiplus::PointF ed, cv::Point2f& pt, int& hitId, float wh = HITTEST_SIZE)
	{
		bool hit = false;
		Point2Point pp(st, ed);
		auto th = -pp.th * RAD_TO_DEEGREE;
		Gdiplus::PointF center((st.X + ed.X) / 2., (st.Y + ed.Y) / 2.);

		hitId = -1;
		if (IsInsideRotatedRect(pt, ed, th, wh, wh))
		{
			hitId = m_InternalReverse ? 0: 1;
			hit = true;
			SetCursor(m_CursorCross);
		}
		else if (IsInsideRotatedRect(pt, st, th, wh, wh))
		{
			hitId = m_InternalReverse ? 1: 0;
			hit = true;
			SetCursor(m_CursorCross);
		}
		else if (IsInsideRotatedRect(pt, center, th, MAX((pp.dist - wh), wh), wh))
		{
			hitId = 2;
			hit = true;
			SetCursor(m_CursorSizeAll);
		}
		else if (m_CircleMode && IsOverInnerCircle(pt, center, pp.dist / 2., wh))
		{
			hitId = 2;
			hit = true;
			SetCursor(m_CursorSizeAll);
		}
		else SetCursor(m_CursorArrow);

		return hit;
	}

	void CalculateCalipers(Gdiplus::PointF start, Gdiplus::PointF end, int nCaliper, float projLen, float searchLen)
	{
		m_NumOfCalipers = MAX(1, nCaliper);
		m_ProjectionLength = projLen;
		m_SearchLength = searchLen;

		CalculateCalipers(start, end);
	}

	void CalculateCalipers(Gdiplus::PointF start, Gdiplus::PointF end)
	{
		m_StPOS = start;
		m_EdPOS = end;
		CalculateCalipers();
	}

	void CalculateCalipers()
	{
		Point2Point pp(m_StPOS, m_EdPOS);
		auto nCount = MAX(1, m_NumOfCalipers);
		auto rad2 = pp.th + CV_05PI;
		auto cx = (m_StPOS.X + m_EdPOS.X) / 2.f;
		auto cy = (m_StPOS.Y + m_EdPOS.Y) / 2.f;

		m_CaliperRects.clear();
		if (m_CircleMode)
		{
			auto r = pp.dist / 2.;
			auto deltaTh = CV_2PI / nCount;
			auto search = MAX(r / 4., 40);
			auto xx = cos(rad2) * search;
			auto yy = sin(rad2) * search;
			auto x05 = xx / 2.f;
			auto y05 = yy / 2.f;
			auto x15 = xx * 1.5f;
			auto y15 = yy * 1.5f;

			for (int i = 0; i < nCount; i++)
			{
				auto rad = ((deltaTh * i) + pp.th);
				auto x = cos(rad) * r + cx;
				auto y = sin(rad) * r + cy;
				m_CaliperRects.push_back(GetRotatedRectPoints2(cv::Point2f(x, y), m_SearchLength, m_ProjectionLength, rad * RAD_TO_DEEGREE));
			}

			if (m_ReverseDirection)
			{
				m_ArrowOne->StartPOS = Gdiplus::PointF(cx + x15, cy + y15);
				m_ArrowOne->EndPOS = Gdiplus::PointF(cx + x05, cy + y05);

				m_ArrowTwo->StartPOS = Gdiplus::PointF(cx - x15, cy - y15);
				m_ArrowTwo->EndPOS = Gdiplus::PointF(cx - x05, cy - y05);
			}
			else
			{
				m_ArrowOne->StartPOS = Gdiplus::PointF(cx + x05, cy + y05);
				m_ArrowOne->EndPOS = Gdiplus::PointF(cx + x15, cy + y15);

				m_ArrowTwo->StartPOS = Gdiplus::PointF(cx - x05, cy - y05);
				m_ArrowTwo->EndPOS = Gdiplus::PointF(cx - x15, cy - y15);
			}
		}
		else
		{
			auto th180 = pp.th * RAD_TO_DEEGREE;
			Rigid trans(m_StPOS, th180);
			auto nCount1 = MAX(1, nCount - 1);
			auto step_ = (pp.dist - m_ProjectionLength) / nCount1;
			auto pj2 = m_ProjectionLength / 2.f;
			auto search = m_SearchLength / 2. + 10;
			auto xx = cos(rad2) * search;
			auto yy = sin(rad2) * search;

			for (int i = 0; i < nCount; i++)
				m_CaliperRects.push_back(GetRotatedRectPoints2(trans.Transform(cv::Point2f(pj2 + (step_ * i), 0)), m_ProjectionLength, m_SearchLength, th180));

			if (m_ReverseDirection)
			{
				m_ArrowOne->StartPOS = Gdiplus::PointF(cx - xx, cy - yy);
				m_ArrowOne->EndPOS = Gdiplus::PointF(cx + xx, cy + yy);
			}
			else
			{
				m_ArrowOne->StartPOS = Gdiplus::PointF(cx + xx, cy + yy);
				m_ArrowOne->EndPOS = Gdiplus::PointF(cx - xx, cy - yy);
			}
		}
	}

	void ShiftCalipers(cv::Point2d& startOffs, cv::Point2d& endOffs)
	{
		Gdiplus::PointF start(m_StPOS.X + startOffs.x, m_StPOS.Y + startOffs.y);
		Gdiplus::PointF end(m_EdPOS.X + endOffs.x, m_EdPOS.Y + endOffs.y);
		CalculateCalipers(start, end);
	}
};

#pragma region TRASH
/* Using opencv's function to testing */
/* This is first implement but not using anymore becus it may resource more hunger than new HittingTest function */
//bool HittingTestOCV(Gdiplus::PointF st, Gdiplus::PointF ed, cv::Point2f& pt, int& hitId, float wh = 14.)
//{
//	bool hit = false;
//
//	auto dx = st.X - ed.X;
//	auto dy = st.Y - ed.Y;
//	auto len = sqrt(dx * dx + dy * dy);
//	auto th = -atan2(dy, dx) * 180. / CV_PI;
//	auto center = Gdiplus::PointF((st.X + ed.X) / 2., (st.Y + ed.Y) / 2.);
//
//	auto left = GetRotatedRectPoints(st, wh, wh, th);
//	auto midd = GetRotatedRectPoints(center, MAX((len - wh), wh), wh, th);
//	auto right = GetRotatedRectPoints(ed, wh, wh, th);
//
//	hitId = -1;
//	if (cv::pointPolygonTest(right, pt, false) >= 0)
//	{
//		hitId = 1;
//		hit = true;
//		SetCursor(m_CursorCross);
//	}
//	else if (cv::pointPolygonTest(left, pt, false) >= 0)
//	{
//		hitId = 0;
//		hit = true;
//		SetCursor(m_CursorCross);
//	}
//	else if (cv::pointPolygonTest(midd, pt, false) >= 0)
//	{
//		hitId = 2;
//		hit = true;
//		SetCursor(m_CursorSizeAll);
//	}
//	else SetCursor(m_CursorArrow);
//
//	return hit;
//}
#pragma endregion
