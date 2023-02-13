#pragma once
#include "stdafx.h"
#include "GraphicBase.h"
#include "GraphicLabel.h"
#include "Rigid.h"
#include "GraphicDimensional.h"

/*
* - Lincoln Lee
* - x.perfect.vn@gmail.com
* - 2022-08-08
*/

class GraphicInteractiveDimensional: public GraphicBase
{
public:
	bool IsInterative() const { return true; }
	bool IsActive() const { return m_Active; }
	bool IsMouseDown() const { return m_IsMouseDown; }
	GraphicInteractiveDimensional& SetActive(bool active = true) { m_Active = active; return *this; }
	bool IsRePosition() const { return true; }

	GraphicInteractiveDimensional& RePosition(CPoint st, CPoint ed, const Rigid& imgToView)
	{
		auto viewToImage = imgToView.Inverse();
		auto stx = viewToImage.Transform(st);
		auto edx = viewToImage.Transform(ed);
		auto dx = ed.x - st.x;

		m_StPOS = Gdiplus::PointF(stx.x, stx.y);
		m_EdPOS = Gdiplus::PointF(edx.x, edx.y);

		m_IsMouseDown = true;
		m_LastHit = true;
		m_LastHitId = dx >= 0;
		m_LastMousePos = ed;
		CalculateInfo();

		return *this;
	}

	GraphicInteractiveDimensional(Gdiplus::PointF start, Gdiplus::PointF end)
		:m_StPOS(start), m_EdPOS(end), m_Pen(new Gdiplus::Pen(0xff00ff00, 1))
	{
		m_CursorArrow = LoadCursor(NULL, IDC_ARROW);
		m_CursorSizeAll = LoadCursor(NULL, IDC_SIZEALL);
		m_CursorCross = LoadCursor(NULL, IDC_CROSS);
		CalculateInfo();
	}

	bool MouseDown(CPoint pos, const Rigid& imgToView)
	{
		auto hitId = -1;
		auto hit = false;
		auto st = imgToView.Transform(m_StPOS);
		auto ed = imgToView.Transform(m_EdPOS);

		hit = HittingTest(st, ed, cv::Point2f(pos.x, pos.y), hitId);

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
					m_LastHit = true;
					m_LastHitId = hitId;
					DoResizeByHitIds[hitId % 3](dxy);
				}
			}

			m_LastMousePos = pos;
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

	GraphicInteractiveDimensional& Render(Gdiplus::Graphics& graph, const Rigid& trans) override
	{
		m_Dimension.Render(graph, trans);
		return *this;
	}

	GraphicInteractiveDimensional& SetMetricTransform(const Rigid& rigid)
	{
		m_MetricTrans = rigid;
		CalculateInfo();
		return *this;
	}

	CRect GetSelectedRect() const
	{
		return CRect(CPoint(lround(m_StPOS.X), lround(m_StPOS.Y)), CPoint(lround(m_EdPOS.X), lround(m_EdPOS.Y)));
	}

	GraphicInteractiveDimensional& GetBeginEnd(CPoint& st, CPoint& ed)
	{
		st.x = lround(m_StPOS.X);
		st.y = lround(m_StPOS.Y);
		ed.x = lround(m_EdPOS.X);
		ed.y = lround(m_EdPOS.Y);

		return *this;
	}

	GraphicInteractiveDimensional& SetBeginEnd(const CPoint& st, const CPoint& ed)
	{
		m_StPOS.X = st.x;
		m_StPOS.Y = st.y;
		m_EdPOS.X = ed.x;
		m_EdPOS.Y = ed.y;
		CalculateInfo();

		return *this;
	}

	~GraphicInteractiveDimensional()
	{
		delete m_Pen;
	}

private:
	Gdiplus::Pen* m_Pen;
	Gdiplus::PointF m_StPOS;
	Gdiplus::PointF m_EdPOS;

	int m_LastHitId = -1;

	Rigid m_MetricTrans;

	bool m_LastHit = false;
	bool m_Active = false;
	bool m_IsMouseDown = false;

	CPoint m_LastMousePos;

	HCURSOR m_CursorArrow;
	HCURSOR m_CursorSizeAll;
	HCURSOR m_CursorCross;

	GraphicDimensional m_Dimension;

	std::vector<std::function<void(cv::Point2d& dxy)>> DoResizeByHitIds = {
		[&](cv::Point2d& dxy) { ShiftCalipers(dxy, cv::Point2d(0, 0)); },
		[&](cv::Point2d& dxy) { ShiftCalipers(cv::Point2d(0, 0), dxy); },
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

	bool HittingTest(Gdiplus::PointF st, Gdiplus::PointF ed, cv::Point2f& pt, int& hitId, float wh = HITTEST_SIZE)
	{
		bool hit = false;
		Point2Point pp(st, ed);
		auto th = -pp.th * RAD_TO_DEEGREE;

		Gdiplus::PointF center((st.X + ed.X) / 2., (st.Y + ed.Y) / 2.);

		hitId = -1;
		if (IsInsideRotatedRect(pt, ed, th, wh, wh))
		{
			hitId = 1;
			hit = true;
			SetCursor(m_CursorCross);
		}
		else if (IsInsideRotatedRect(pt, st, th, wh, wh))
		{
			hitId = 0;
			hit = true;
			SetCursor(m_CursorCross);
		}
		else if (IsInsideRotatedRect(pt, center, th, MAX((pp.dist - wh), wh), wh))
		{
			hitId = 2;
			hit = true;
			SetCursor(m_CursorSizeAll);
		}
		else SetCursor(m_CursorArrow);

		return hit;
	}

	void ShiftCalipers(cv::Point2d& startOffs, cv::Point2d& endOffs)
	{
		m_StPOS.X += startOffs.x;
		m_StPOS.Y += startOffs.y;

		m_EdPOS.X += endOffs.x;
		m_EdPOS.Y += endOffs.y;

		CalculateInfo();
	}

	void CalculateInfo()
	{
		auto st = m_MetricTrans.Transform(m_StPOS);
		auto ed = m_MetricTrans.Transform(m_EdPOS);
		Point2Point pp(st, ed);

		CStringW str;
		str.Format(L"[%.3f, %.3f, %.3f](W H D)(mm), A: %.3f¡Æ", pp.dx, pp.dy, pp.dist, pp.th * RAD_TO_DEEGREE);

		m_Dimension
			.StartPoint(m_StPOS)
			.EndPoint(m_EdPOS)
			.SetText(str);
	}
};

#pragma region TRASH
//bool IsInsideRotatedRect(Gdiplus::PointF& pt, Gdiplus::PointF& center, float th, float w, float h)
//{
//	auto w2 = w / 2.f;
//	auto h2 = h / 2.f;
//	Rigid trans(center, -th);
//	auto ptx = trans.Transform(Gdiplus::PointF(pt.X - center.X, pt.Y - center.Y));
//	return (pt.X >= (center.X - w2) && pt.X < (center.X + w2)) && (pt.Y >= (center.Y - h2) && pt.Y < (center.Y + h2));
//}

/* Using opencv's function to testing */
/* This is first implement but not using anymore becus it may resource more hunger than new HittingTest function */
//bool HittingTestOCV(Gdiplus::PointF st, Gdiplus::PointF ed, cv::Point2f& pt, int& hitId, float wh = 14.)
//{
//	bool hit = false;

//	auto dx = st.X - ed.X;
//	auto dy = st.Y - ed.Y;
//	auto len = sqrt(dx * dx + dy * dy);
//	auto th = -atan2(dy, dx) * 180. / CV_PI;
//	auto center = Gdiplus::PointF((st.X + ed.X) / 2., (st.Y + ed.Y) / 2.);

//	auto left = GetRotatedRectPoints(st, wh, wh, th);
//	auto midd = GetRotatedRectPoints(center, MAX((len - wh), wh), wh, th);
//	auto right = GetRotatedRectPoints(ed, wh, wh, th);

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

//	return hit;
//}
#pragma endregion
