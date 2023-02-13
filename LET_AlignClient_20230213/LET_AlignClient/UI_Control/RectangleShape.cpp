#include "stdafx.h"
#include "RectangleShape.h"

RectangleShape::RectangleShape(int lineSize)
{
	m_lineSize = lineSize;
	m_ShapeMode = DOGU::DRECT;
	CreateDrawingStyle();
}

BOOL RectangleShape::MouseEvent_Down(const Gdiplus::Point& pt, Gdiplus::Graphics* g, float scale)
{
	if (m_Points.size() < 2) // If number of point is not yet enough to form the rectangle
		m_Points.push_back(pt);

	Draw(g, scale);

	BOOL enough = m_Points.size() >= 2;

	return enough;
}

BOOL RectangleShape::MouseEvent_Up(const Gdiplus::Point& pt, Gdiplus::Graphics* g, float scale)
{
	if (m_Points.size() == 1)
		m_Points.push_back(pt);

	Draw(g, scale);

	return TRUE;
}

BOOL RectangleShape::MouseEvent_Move(const Gdiplus::Point& pt, Gdiplus::Graphics* g, float scale)
{
	if (m_Points.size() >= 2) return TRUE;

	m_Points.push_back(pt);
	
	Draw(g, scale);

	m_Points.erase(m_Points.end() - 1, m_Points.end());
	return FALSE;
}

BOOL RectangleShape::MouseEvent_DblClick(const Gdiplus::Point& pt, Gdiplus::Graphics* g, float scale)
{
	Draw(g, scale);
	return TRUE;
}

// Lincoln Lee - 2022/04/15
void RectangleShape::Draw(Gdiplus::Graphics* g, float scale)
{
	Gdiplus::Pen* pen = MaskFillMode::ADD == m_FillMode ? m_PenAdd : m_PenSubtract;
	Gdiplus::Brush* brush = MaskFillMode::ADD == m_FillMode ? m_BrushAdd : m_BrushSubtract;

	if (m_Points.size() > 0) // Do Paint
	{
		const Gdiplus::Point& pt00 = m_Points[0];
		const Gdiplus::Point& pt01 = m_Points[m_Points.size() - 1];

		const Gdiplus::Point pt0 = { MIN(pt00.X, pt01.X), MIN(pt00.Y, pt01.Y) };
		const Gdiplus::Point pt1 = { MAX(pt00.X, pt01.X), MAX(pt00.Y, pt01.Y) };

		Gdiplus::Rect rect(pt0.X * scale, pt0.Y * scale, (pt1.X - pt0.X) * scale, (pt1.Y - pt0.Y) * scale);

		g->FillRectangle(brush, rect);
		g->DrawRectangle(pen, rect);
	}
}

// Lincoln Lee - 2022/04/15
void RectangleShape::DrawToMask(Gdiplus::Graphics* g)
{
	Gdiplus::Brush* brush = MaskFillMode::ADD == m_FillMode ? m_BrushMask : m_BrushSubtractMask;

	if (m_Points.size() > 1) // Do Paint
	{
		const Gdiplus::Point& pt00 = m_Points[0];
		const Gdiplus::Point& pt01 = m_Points[m_Points.size() - 1];

		const Gdiplus::Point pt0 = { MIN(pt00.X, pt01.X), MIN(pt00.Y, pt01.Y) };
		const Gdiplus::Point pt1 = { MAX(pt00.X, pt01.X), MAX(pt00.Y, pt01.Y) };

		Gdiplus::Rect rect(pt0.X, pt0.Y, (pt1.X - pt0.X), (pt1.Y - pt0.Y));

		g->FillRectangle(brush, rect);
	}
}
