#pragma once
#include "stdafx.h"
#include "GraphicBase.h"
#include "GraphicLabel.h"
#include "GraphicHitTest.h"

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
class GraphicRotateRectSelection : public GraphicBase
{
public:
	bool IsInterative() const { return true; }
	bool IsActive() const { return m_Active; }
	bool IsMouseDown() const { return m_IsMouseDown; }
	GraphicRotateRectSelection& SetActive(bool active = true) { m_Active = active; return *this; }
	bool IsRePosition() const { return true; }
	GraphicRotateRectSelection& RePosition(CPoint st, CPoint ed, const Rigid& imgToView);

	GraphicRotateRectSelection();
	GraphicRotateRectSelection(CRect rect);
	GraphicRotateRectSelection(cv::Rect2d rect);

	template<class T1>
	GraphicRotateRectSelection& Init(cv::Rect_<T1> rect);

	GraphicRotateRectSelection& SetMetricTransform(const Rigid& rigid);
	GraphicRotateRectSelection& SetShowDigonal(bool show = true);
	GraphicRotateRectSelection& SetShowCenter(bool show = true);
	GraphicRotateRectSelection& SetShowRotate(bool show = true);
	GraphicRotateRectSelection& SetSelectedRegion(const CRect& rect);

	template<class T0, class T1, class T2, class T3>
	GraphicRotateRectSelection& SetSelectedRegion(T0 x, T1 y, T2 w, T3 h);

	template<class T0>
	GraphicRotateRectSelection& SetSelectedRegion(const cv::Rect_<T0>& rect);

	template<class T0>
	GraphicRotateRectSelection& ChangeSelectedRegion(const cv::Rect_<T0>& rect);

	GraphicBase& Render(Gdiplus::Graphics& graph, const Rigid& imgToView);
	bool MouseDown(CPoint pos, const Rigid& trans);
	bool MouseUp(CPoint pos, const Rigid& trans);
	bool MouseMove(CPoint pos, const Rigid& imgToView);
	bool MouseHover(CPoint pos, const Rigid& trans);

	cv::Rect2f GetSelectedRegion() const;
	cv::Rect GetSelectedRegion(cv::Size ContrainsSize) const;

	void SetPoints(bool bSetCenter);
	
	void ClearMouseState();
	CRect GetSelectedRect() const;
	GraphicRotateRectSelection& GetBeginEnd(CPoint& st, CPoint& ed);
	GraphicRotateRectSelection& SetBeginEnd(const CPoint& st, const CPoint& ed);
	GraphicRotateRectSelection& Settings(const CPoint& st, const CPoint& ed, bool ellipseMODE);
	GraphicRotateRectSelection& GetSettings(CPoint& st, CPoint& ed, bool& ellipseMODE);
	~GraphicRotateRectSelection();

protected:
private:
	bool m_Active = false;
	bool m_LastHit = false;

	bool m_ShowCenter = false;
	bool m_IsMouseDown = false;
	bool m_ShowDigonal = false;
	bool m_ShowRotate = false;

	HCURSOR m_Cursor;
	HCURSOR m_CursorNormal;
	HCURSOR m_RotateCursor;
	cv::Rect2f m_SelectedRegion;
	Gdiplus::REAL m_DashPattern[2]{ 6, 6 };

	int m_LastHitId = -1;
	CPoint m_LastMousePos;
	CSize m_Size = CSize(HITTEST_SIZE, HITTEST_SIZE);

	Gdiplus::Pen* m_BoderPen = new Gdiplus::Pen(0xffff6060, 3);
	Gdiplus::Pen* m_DashPen = new Gdiplus::Pen(0xffff6060, 1);
	Gdiplus::Pen* m_SolidPen = new Gdiplus::Pen(0xffff6060, 1);
	Gdiplus::Pen* m_EllipsePen = new Gdiplus::Pen(0xffffff00, 2);
	Gdiplus::Pen* m_EllipseInPen = new Gdiplus::Pen(0xffffff00, 2);

	Rigid m_MetricTrans;
	GraphicHitTest m_HitTest[8];
	GraphicLabel m_SelectionInfo;
	GraphicDimensional m_Dimension;

	POINT m_PointSrc[5];
	POINT m_PointDest[5];
	CPoint m_FixedSrc;
	CPoint m_FixedDest;

	Gdiplus::PointF PointDest[5];

	Gdiplus::Matrix m_Transform;
	Gdiplus::REAL m_StartPhi;
	Gdiplus::REAL m_CurPhi;

	std::vector<std::function<void(cv::Point2d& dxy)>> DoResizeByHitIds = {
		[&](cv::Point2d& dxy) { OffsetSelectedRegion(dxy.x, 0, -dxy.x, dxy.y); },
		[&](cv::Point2d& dxy) { OffsetSelectedRegion(0, dxy.y, dxy.x, -dxy.y); },
		[&](cv::Point2d& dxy) { OffsetSelectedRegion(0, dxy.y, 0, -dxy.y); },
		[&](cv::Point2d& dxy) { OffsetSelectedRegion(0, 0, 0, dxy.y); },
		[&](cv::Point2d& dxy) { OffsetSelectedRegion(dxy.x, 0, -dxy.x, 0); },
		[&](cv::Point2d& dxy) { OffsetSelectedRegion(0, 0, dxy.x, 0); },
		[&](cv::Point2d& dxy) { OffsetSelectedRegion(dxy.x, dxy.y, -dxy.x, -dxy.y); },
		[&](cv::Point2d& dxy) { OffsetSelectedRegion(0, 0, dxy.x, dxy.y); },
		[&](cv::Point2d& dxy) { OffsetSelectedRegion(dxy.x, dxy.y, 0, 0); }
	};

private: // functions
	template<class T0, class T1>
	bool SwapRect(const cv::Rect_<T0>& inRECT, cv::Rect_<T1>& outRECT) const;

	GraphicRotateRectSelection& OffsetSelectedRegion(float dx, float dy, float dw, float dh)
	{
		return ChangeSelectedRegion(cv::Rect2f(m_SelectedRegion.x + dx, m_SelectedRegion.y + dy, m_SelectedRegion.width + dw, m_SelectedRegion.height + dh));
	}

	void OffsetRotateRegion(float dx, float dy,int id)
	{
		switch (id)
		{
		case 0:	{ m_PointDest[3].x += dx;	m_PointDest[3].y += dy;	}		break;
		case 1:	{ m_PointDest[1].x += dx;	m_PointDest[1].y += dy;	}		break;
		case 2:	{ m_PointDest[0].x += dx;	m_PointDest[1].x += dx;	
			      m_PointDest[0].y += dy;	m_PointDest[1].y += dy; }		break;
		case 3:	{ m_PointDest[2].x += dx;	m_PointDest[3].x += dx;
			      m_PointDest[2].y += dy;	m_PointDest[3].y += dy; }		break;
		case 4:	{ m_PointDest[0].x += dx;	m_PointDest[3].x += dx;
			      m_PointDest[0].y += dy;	m_PointDest[3].y += dy; }		break;
		case 5:	{ m_PointDest[1].x += dx;	m_PointDest[2].x += dx;	
			      m_PointDest[1].y += dy;	m_PointDest[2].y += dy; }		break;
		case 6:	{ m_PointDest[0].x += dx;	m_PointDest[0].y += dy;	}		break;
		case 7:	{ m_PointDest[2].x += dx;	m_PointDest[2].y += dy;	}		break;
		case 8:	{
			for (int i = 0; i < 4; i++)
			{
				m_PointDest[i].x += dx;
				m_PointDest[i].y += dy;
			}
		}	break;
		}

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
	void CalculateInfo(Gdiplus::PointF m_StPOS, Gdiplus::PointF m_EdPOS, Gdiplus::PointF vStPOS, Gdiplus::PointF vEdPOS)
	{
		auto st = m_MetricTrans.Transform(m_StPOS);
		auto ed = m_MetricTrans.Transform(m_EdPOS);
		auto vst = m_MetricTrans.Transform(vStPOS);
		auto ved = m_MetricTrans.Transform(vEdPOS);

		Point2Point pp(st, ed);
		Point2Point pp2(vst, ved);

		CStringW str;
		str.Format(L"[%.3f, %.3f](W H)(mm), A: %.3f¡Æ", pp.dist, pp2.dist, -pp.th * RAD_TO_DEEGREE);

		m_Dimension
			.StartPoint(m_StPOS)
			.EndPoint(m_EdPOS)
			.SetText(str);
	}
};

template<class T1>
GraphicRotateRectSelection& GraphicRotateRectSelection::Init(cv::Rect_<T1> rect)
{
	m_BoderPen->SetDashStyle(Gdiplus::DashStyleDash);
	m_DashPen->SetDashStyle(Gdiplus::DashStyleDash);
	m_EllipsePen->SetDashStyle(Gdiplus::DashStyleDash);
	m_EllipsePen->SetDashPattern(m_DashPattern, 2);
	m_DashPen->SetDashPattern(m_DashPattern, 2);

	m_Cursor = LoadCursor(NULL, IDC_SIZEALL);
	m_CursorNormal = LoadCursor(NULL, IDC_ARROW);
	m_RotateCursor = LoadCursor(NULL, IDC_PIN);

	m_ShowCenter = true;
	m_CurPhi = 0;
	SetSelectedRegion(rect);

	return *this;
}

template<class T0, class T1>
bool GraphicRotateRectSelection::SwapRect(const cv::Rect_<T0>& inRECT, cv::Rect_<T1>& outRECT) const
{
	auto tl = inRECT.tl();
	auto br = inRECT.br();
	auto tlx = MIN(tl.x, br.x);
	auto tly = MIN(tl.y, br.y);
	auto brx = MAX(tl.x, br.x);
	auto bry = MAX(tl.y, br.y);

	outRECT = cv::Rect_<T0>(tlx, tly, brx - tlx, bry - tly);
	return (inRECT.width < 0) || (inRECT.height < 0) || (br.x < tl.x) || (br.y < tl.y);
}

template<class T0, class T1, class T2, class T3>
GraphicRotateRectSelection& GraphicRotateRectSelection::SetSelectedRegion(T0 x, T1 y, T2 w, T3 h)
{
	return SetSelectedRegion(cv::Rect2d(x, y, w, h));
}

template<class T0>
GraphicRotateRectSelection& GraphicRotateRectSelection::SetSelectedRegion(const cv::Rect_<T0>& rect)
{
	SwapRect(rect, m_SelectedRegion);
	auto w = m_SelectedRegion.width;
	auto h = m_SelectedRegion.height;
	auto w2 = w / 2.;
	auto h2 = h / 2.;
	auto tl = m_SelectedRegion.tl();
	auto br = m_SelectedRegion.br();

	m_HitTest[0].Init(cv::Point2f(tl.x, tl.y + h), m_Size, g_HitTestCursors[5]);
	m_HitTest[1].Init(cv::Point2f(tl.x + w, tl.y), m_Size, g_HitTestCursors[2]);

	m_HitTest[2].Init(cv::Point2f(tl.x + w2, tl.y), m_Size, g_HitTestCursors[1]);
	m_HitTest[3].Init(cv::Point2f(tl.x + w2, tl.y + h), m_Size, g_HitTestCursors[6]);

	m_HitTest[4].Init(cv::Point2f(tl.x, tl.y + h2), m_Size, g_HitTestCursors[3]);
	m_HitTest[5].Init(cv::Point2f(tl.x + w, tl.y + h2), m_Size, g_HitTestCursors[4]);

	m_HitTest[6].Init(tl, m_Size, g_HitTestCursors[0]);
	m_HitTest[7].Init(br, m_Size, g_HitTestCursors[7]);

	m_SelectionInfo.SetFont(GraphicLabel::GetDefaultFont())
		.SetAnchor(Anchor::TopCenter)
		.SetTextColor(0xffffffff);

	return *this;
}

template<class T0>
GraphicRotateRectSelection& GraphicRotateRectSelection::ChangeSelectedRegion(const cv::Rect_<T0>& rect)
{
	bool isSwap = SwapRect(rect, m_SelectedRegion);
	auto w = m_SelectedRegion.width;
	auto h = m_SelectedRegion.height;
	auto w2 = w / 2.;
	auto h2 = h / 2.;
	auto tl = m_SelectedRegion.tl();
	auto br = m_SelectedRegion.br();

	if (isSwap && m_LastHit)
		if (m_LastHitId > -1 && m_LastHitId < 8)
			m_LastHitId ^= 1; // Toggle even odd

	m_HitTest[0].ChangeCenterPoint(cv::Point2f(tl.x, tl.y + h));
	m_HitTest[1].ChangeCenterPoint(cv::Point2f(tl.x + w, tl.y));

	m_HitTest[2].ChangeCenterPoint(cv::Point2f(tl.x + w2, tl.y));
	m_HitTest[3].ChangeCenterPoint(cv::Point2f(tl.x + w2, tl.y + h));

	m_HitTest[4].ChangeCenterPoint(cv::Point2f(tl.x, tl.y + h2));
	m_HitTest[5].ChangeCenterPoint(cv::Point2f(tl.x + w, tl.y + h2));

	m_HitTest[6].ChangeCenterPoint(tl);
	m_HitTest[7].ChangeCenterPoint(br);

	return *this;
}