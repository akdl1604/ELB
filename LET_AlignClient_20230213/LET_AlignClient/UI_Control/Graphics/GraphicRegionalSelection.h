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
class GraphicRegionalSelection: public GraphicBase
{
public:
	bool IsInterative() const { return true; }
	bool IsActive() const { return m_Active; }
	bool IsMouseDown() const { return m_IsMouseDown; }
	GraphicRegionalSelection& SetActive(bool active = true) { m_Active = active; return *this; }
	bool IsRePosition() const { return true; }
	GraphicRegionalSelection& RePosition(CPoint st, CPoint ed, const Rigid& imgToView);

	GraphicRegionalSelection();
	GraphicRegionalSelection(CRect rect);
	GraphicRegionalSelection(cv::Rect2d rect);

	template<class T1>
	GraphicRegionalSelection& Init(cv::Rect_<T1> rect);

	GraphicRegionalSelection& SetMetricTransform(const Rigid& rigid);
	GraphicRegionalSelection& SetShowCross(bool show = true);
	GraphicRegionalSelection& SetShowEllipse(bool show = true);
	GraphicRegionalSelection& SetShowDigonal(bool show = true);
	GraphicRegionalSelection& SetShowCenter(bool show = true);
	GraphicRegionalSelection& SetSelectedRegion(const CRect& rect);

	template<class T0, class T1, class T2, class T3>
	GraphicRegionalSelection& SetSelectedRegion(T0 x, T1 y, T2 w, T3 h);

	template<class T0>
	GraphicRegionalSelection& SetSelectedRegion(const cv::Rect_<T0>& rect);

	template<class T0>
	GraphicRegionalSelection& ChangeSelectedRegion(const cv::Rect_<T0>& rect);

	GraphicBase& Render(Gdiplus::Graphics& graph, const Rigid& imgToView);
	bool MouseDown(CPoint pos, const Rigid& trans);
	bool MouseUp(CPoint pos, const Rigid& trans);
	bool MouseMove(CPoint pos, const Rigid& imgToView);
	bool MouseHover(CPoint pos, const Rigid& trans);

	cv::Rect2f GetSelectedRegion() const;
	cv::Rect GetSelectedRegion(cv::Size ContrainsSize) const;
	
	void ClearMouseState();
	CRect GetSelectedRect() const;
	GraphicRegionalSelection& GetBeginEnd(CPoint& st, CPoint& ed);
	GraphicRegionalSelection& SetBeginEnd(const CPoint& st, const CPoint& ed);
	GraphicRegionalSelection& Settings(const CPoint& st, const CPoint& ed, bool ellipseMODE);
	GraphicRegionalSelection& GetSettings(CPoint& st, CPoint& ed, bool& ellipseMODE);
	~GraphicRegionalSelection();

protected:
private:
	bool m_Active = false;
	bool m_LastHit = false;

	bool m_ShowCross = false;
	bool m_ShowCenter = false;
	bool m_IsMouseDown = false;
	bool m_ShowEllipse = false;
	bool m_ShowDigonal = false;

	HCURSOR m_Cursor;
	HCURSOR m_CursorNormal;

	cv::Rect2f m_SelectedRegion;
	Gdiplus::REAL m_DashPattern[2]{ 6, 6 };

	int m_LastHitId = -1;
	CPoint m_LastMousePos;
	CSize m_Size = CSize(HITTEST_SIZE, HITTEST_SIZE);

	Gdiplus::Pen* m_BoderPen = new Gdiplus::Pen(0xffff6060, 3);
	Gdiplus::Pen* m_DashPen = new Gdiplus::Pen(0xffff6060, 1);
	Gdiplus::Pen* m_SolidPen = new Gdiplus::Pen(0xffff6060, 1);
	Gdiplus::Pen* m_EllipsePen = new Gdiplus::Pen(0xffff6060, 2);

	Rigid m_MetricTrans;
	GraphicHitTest m_HitTest[8];
	GraphicLabel m_SelectionInfo;

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

	GraphicRegionalSelection& OffsetSelectedRegion(float dx, float dy, float dw, float dh)
	{
		return ChangeSelectedRegion(cv::Rect2f(m_SelectedRegion.x + dx, m_SelectedRegion.y + dy, m_SelectedRegion.width + dw, m_SelectedRegion.height + dh));
	}
};

template<class T1>
GraphicRegionalSelection& GraphicRegionalSelection::Init(cv::Rect_<T1> rect)
{
	m_BoderPen->SetDashStyle(Gdiplus::DashStyleDash);
	m_DashPen->SetDashStyle(Gdiplus::DashStyleDash);
	m_DashPen->SetDashPattern(m_DashPattern, 2);

	m_Cursor = LoadCursor(NULL, IDC_SIZEALL);
	m_CursorNormal = LoadCursor(NULL, IDC_ARROW);

	m_ShowCross = true;
	m_ShowCenter = true;
	SetSelectedRegion(rect);

	return *this;
}

template<class T0, class T1>
bool GraphicRegionalSelection::SwapRect(const cv::Rect_<T0>& inRECT, cv::Rect_<T1>& outRECT) const
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
GraphicRegionalSelection& GraphicRegionalSelection::SetSelectedRegion(T0 x, T1 y, T2 w, T3 h)
{
	return SetSelectedRegion(cv::Rect2d(x, y, w, h));
}

template<class T0>
GraphicRegionalSelection& GraphicRegionalSelection::SetSelectedRegion(const cv::Rect_<T0>& rect)
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
GraphicRegionalSelection& GraphicRegionalSelection::ChangeSelectedRegion(const cv::Rect_<T0>& rect)
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