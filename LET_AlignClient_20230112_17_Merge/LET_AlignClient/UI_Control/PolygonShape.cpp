#include "stdafx.h"
#include "PolygonShape.h"

PolygonShape::PolygonShape(int lineSize)
{
	m_lineSize = lineSize;
	m_ShapeMode = DOGU::D_POLYGON;
	CreateDrawingStyle();
}

BOOL PolygonShape::MouseEvent_Down(const Gdiplus::Point& pt, Gdiplus::Graphics* g, float scale)
{
	m_Points.push_back(pt);

	Draw(g, scale);

	return FALSE;
}

BOOL PolygonShape::MouseEvent_Up(const Gdiplus::Point& pt, Gdiplus::Graphics* g, float scale)
{
	Draw(g, scale);

	return FALSE;
}

BOOL PolygonShape::MouseEvent_Move(const Gdiplus::Point& pt, Gdiplus::Graphics* g, float scale)
{
	m_Points.push_back(pt);
	Draw(g, scale);
	m_Points.erase(m_Points.end() - 1, m_Points.end());

	return FALSE;
}

BOOL PolygonShape::MouseEvent_DblClick(const Gdiplus::Point& pt, Gdiplus::Graphics* g, float scale)
{
	Draw(g, scale);
	return TRUE;
}

void PolygonShape::Draw(Gdiplus::Graphics* g, float scale)
{
	Gdiplus::Pen* pen = MaskFillMode::ADD == m_FillMode ? m_PenAdd : m_PenSubtract;
	Gdiplus::Brush* brush = MaskFillMode::ADD == m_FillMode ? m_BrushAdd : m_BrushSubtract;

	int l_Size = int(m_Points.size());
	if (l_Size > 0) // Do Paint
	{
		Gdiplus::Point* pts = new Gdiplus::Point[l_Size];

		for (int i = 0; i < l_Size; i++)
		{
			const auto& pt = m_Points[i];
			pts[i].X = pt.X * scale;
			pts[i].Y = pt.Y * scale;
		}

		g->FillPolygon(brush, pts, l_Size);
		g->DrawPolygon(pen, pts, l_Size);

		delete[] pts;
	}
}

void PolygonShape::DrawToMask(Gdiplus::Graphics* g)
{
	Gdiplus::Brush* brush = MaskFillMode::ADD == m_FillMode ? m_BrushMask : m_BrushSubtractMask;

	int l_Size = int(m_Points.size());
	if (l_Size > 0) // Do Paint
	{
		Gdiplus::Point* pts = m_Points.data();

		g->FillPolygon(brush, pts, l_Size);
	}
}
