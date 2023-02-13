#include "stdafx.h"
#include "EllipseShape.h"

double EllipseShape::distancePointPoint(const Gdiplus::Point& pt0, const Gdiplus::Point& pt1)
{
	double dx = pt1.X - pt0.X;
	double dy = pt1.Y - pt0.Y;

	return std::sqrt(dx * dx + dy * dy);
}

EllipseShape::EllipseShape(int lineSize)
{
	m_lineSize = lineSize;
	m_ShapeMode = DOGU::DELLIPSE;
	CreateDrawingStyle();
}

BOOL EllipseShape::MouseEvent_Down(const Gdiplus::Point& pt, Gdiplus::Graphics* g, float scale)
{
	if (m_Points.size() < 2) // If number of point is not yet enough to form the line
		m_Points.push_back(pt);

	Draw(g, scale);

	BOOL enough = m_Points.size() >= 2;

	return enough;
}

BOOL EllipseShape::MouseEvent_Up(const Gdiplus::Point& pt, Gdiplus::Graphics* g, float scale)
{
	// Lincoln Lee - 220210
	if (m_Points.size() == 1)
		m_Points.push_back(pt);

	Draw(g, scale);

	return TRUE;
}

BOOL EllipseShape::MouseEvent_Move(const Gdiplus::Point& pt, Gdiplus::Graphics* g, float scale)
{
	if (m_Points.size() >= 2) return TRUE;

	m_Points.push_back(pt);
	
	Draw(g, scale);

	m_Points.erase(m_Points.end() - 1, m_Points.end());
	return FALSE;
}

BOOL EllipseShape::MouseEvent_DblClick(const Gdiplus::Point& pt, Gdiplus::Graphics* g, float scale)
{
	Draw(g, scale);
	//ReleaseCapture();
	return TRUE;
}

// Lincoln Lee - 2022/04/15
void EllipseShape::Draw(Gdiplus::Graphics* g, float scale)
{
	Gdiplus::Pen* pen = MaskFillMode::ADD == m_FillMode ? m_PenAdd : m_PenSubtract;
	Gdiplus::Brush* brush = MaskFillMode::ADD == m_FillMode ? m_BrushAdd : m_BrushSubtract;

	if (m_Points.size() > 1) // Do Paint
	{
		const Gdiplus::Point& pt0 = m_Points[0];
		const Gdiplus::Point& pt1 = m_Points[m_Points.size() - 1];
		const Gdiplus::Point mid = { lround((pt0.X + pt1.X) / 2.), lround((pt0.Y + pt1.Y) / 2.) };
		const double len = distancePointPoint(pt0, pt1);
		const double len2 = len / 2.;

		//Gdiplus::Rect rect(pt0.X * scale, pt0.Y * scale, (pt1.X - pt0.X) * scale, (pt1.Y - pt0.Y) * scale);
		Gdiplus::Rect rect( (mid.X - len2)* scale, (mid.Y - len2) * scale, len * scale, len * scale);
		
		g->FillEllipse(brush, rect);
		g->DrawEllipse(pen, rect);
	}
}

// Lincoln Lee - 2022/04/15
void EllipseShape::DrawToMask(Gdiplus::Graphics* g)
{
	Gdiplus::Brush* brush = MaskFillMode::ADD == m_FillMode ? m_BrushMask : m_BrushSubtractMask;
	if (m_Points.size() > 1) // Do Paint
	{
		const Gdiplus::Point& pt0 = m_Points[0];
		const Gdiplus::Point& pt1 = m_Points[m_Points.size() - 1];

		//Gdiplus::Rect rect(pt0.X, pt0.Y, (pt1.X - pt0.X), (pt1.Y - pt0.Y));
		const Gdiplus::Point mid = { lround((pt0.X + pt1.X) / 2.), lround((pt0.Y + pt1.Y) / 2.) };
		const double len = distancePointPoint(pt0, pt1);
		const double len2 = len / 2.;

		Gdiplus::Rect rect(mid.X - len2, mid.Y - len2, len, len);

		g->FillEllipse(brush, rect);
	}
}