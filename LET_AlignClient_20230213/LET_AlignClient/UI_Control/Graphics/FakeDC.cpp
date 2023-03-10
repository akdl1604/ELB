#include "stdafx.h"
#include "FakeDC.h"
#include "LincUtils.h"

/*
* - Lincoln Lee
* - x.perfect.vn@gmail.com
* - 2022-08-08
*/


CPen* FakeDC::SelectObject(CPen* pen)
{
	LOGPEN lp;
	pen->GetLogPen(&lp);
	m_CurrentColor = ColorRefToColor(lp.lopnColor);
	return pen;
}

CPoint FakeDC::MoveTo(int x, int y)
{
	m_LastPoint.x = x;
	m_LastPoint.y = y;
	return m_LastPoint;
}

CPoint FakeDC::MoveTo(CPoint pt)
{
	m_LastPoint = pt;
	return m_LastPoint;
}

BOOL FakeDC::Rectangle(CRect rect)
{
	m_dsp->AddSoftGraphic(new GraphicRectangle(rect, m_CurrentColor));
	return TRUE;
}

BOOL FakeDC::Rectangle(int x1, int y1, int x2, int y2)
{
	m_dsp->AddSoftGraphic(new GraphicRectangle(CRect(x1, y1, x2, y2), m_CurrentColor));
	return TRUE;
}

BOOL FakeDC::Rectangle(int x1, int y1, int x2, int y2, float thickness)
{
	m_dsp->AddSoftGraphic(new GraphicRectangle(CRect(x1, y1, x2, y2), m_CurrentColor, thickness));
	return TRUE;
}

void FakeDC::SetColor(Gdiplus::Color color)
{
	m_CurrentColor = color;
}

void FakeDC::SetTextColor(Gdiplus::Color color)
{
	m_CurrentTextColor = color;
}

COLORREF FakeDC::SetColor(COLORREF color)
{
	m_CurrentColor = ColorRefToColor(color);
	return color;
}

COLORREF FakeDC::SetTextColor(COLORREF color)
{
	m_CurrentTextColor = ColorRefToColor(color);
	return color;
}

BOOL FakeDC::LineTo(int x, int y)
{
	m_dsp->AddSoftGraphic(new GraphicLine(Gdiplus::PointF(m_LastPoint.x, m_LastPoint.y), Gdiplus::PointF(x, y), m_CurrentColor, 1, Gdiplus::DashStyleSolid, Gdiplus::SmoothingModeNone));
	m_LastPoint.x = x;
	m_LastPoint.y = y;

	return true;
}

BOOL FakeDC::LineTo(CPoint pt)
{
	m_dsp->AddSoftGraphic(new GraphicLine(Gdiplus::PointF(m_LastPoint.x, m_LastPoint.y), Gdiplus::PointF(pt.x, pt.y), m_CurrentColor, 1, Gdiplus::DashStyleSolid, Gdiplus::SmoothingModeNone));
	m_LastPoint = pt;

	return true;
}

void FakeDC::Ellipse(int x1, int y1, int x2, int y2)
{
	m_dsp->AddSoftGraphic(new GraphicCircle(x1, y1, x2, y2, m_CurrentColor));
}

void FakeDC::Ellipse(CRect rect)
{
	m_dsp->AddSoftGraphic(new GraphicCircle(rect.left, rect.top, rect.right, rect.bottom, m_CurrentColor));
}

BOOL FakeDC::TextOut(int x, int y, const CString& str)
{
	m_dsp->AddSoftGraphic(new GraphicLabel(x, y, str, m_CurrentTextColor, Anchor::BottomRight, true));
	return true;
}

void FakeDC::AddPoint(int x, int y, int wh, int thickness)
{
	m_dsp->AddSoftGraphic(new GraphicPoint(x, y, m_CurrentColor, wh, thickness));
}


void FakeDC::AddGraphic(GraphicBase* graph)
{
	m_dsp->AddSoftGraphic(graph);
}
void FakeDC::AddHardGraphic(GraphicBase* graph)
{
	m_dsp->AddHardGraphic(graph);
}