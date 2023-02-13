#include "stdafx.h"
#include "PenShape.h"
#include <cmath>

PenShape::PenShape(int lineSize)
{
	m_lineSize = lineSize;
	m_ShapeMode = DOGU::DPEN;
	m_MinDist = MAX(3, lineSize * 0.2f);
	CreateDrawingStyle();
}

BOOL PenShape::MouseEvent_Down(const Gdiplus::Point& pt, Gdiplus::Graphics* g, float scale)
{
	m_IsMouseDown = TRUE;
	m_Points.push_back(pt);
	Draw(g, scale);

	return FALSE;
}

BOOL PenShape::MouseEvent_Up(const Gdiplus::Point& pt, Gdiplus::Graphics* g, float scale)
{
	m_IsMouseDown = FALSE;
	Draw(g, scale);

	return TRUE;
}

BOOL PenShape::MouseEvent_Move(const Gdiplus::Point& pt, Gdiplus::Graphics* g, float scale)
{
	int l_Size = int(m_Points.size());
	if (l_Size > 0)
	{
		if (m_IsMouseDown) {
			const auto& pt0 = m_Points[l_Size - 1];
			double dist = distancePointPoint(pt0, pt);
			if (dist >= m_MinDist)
				m_Points.push_back(pt);
		}
		Draw(g, scale);
	}

	return FALSE;
}

void PenShape::Draw(Gdiplus::Graphics* g, float scale)
{
	Gdiplus::Pen* pen = MaskFillMode::ADD == m_FillMode ? m_PenAdd : m_PenSubtract;
	Gdiplus::Brush* brush = MaskFillMode::ADD == m_FillMode ? m_BrushAdd : m_BrushSubtract;

	int l_Size = int(m_Points.size());
	if (l_Size > 1)
	{
		std::vector<Gdiplus::Point> pts;
		Gdiplus::GraphicsPath* Path = new Gdiplus::GraphicsPath(FillMode::FillModeWinding);

		for (const auto& pt : m_Points)
			pts.push_back(Gdiplus::Point(pt.X * scale, pt.Y * scale));

		m_PenWiden->SetWidth(m_lineSize * scale);

		Path->AddLines(pts.data(), l_Size);
		Path->Widen(m_PenWiden);
		Path->Outline();

		g->FillPath(brush, Path);
		g->DrawPath(pen, Path);

		::DeleteObject(Path);
		pts.clear();
	}
	else if (l_Size > 0)
	{
		const auto& pt = m_Points[0];
		const int half = m_lineSize / 2.;
		const float d = m_lineSize * scale;
		Gdiplus::Rect rect((pt.X - half) * scale, (pt.Y - half) * scale, d, d);

		g->FillEllipse(brush, rect);
		g->DrawEllipse(pen, rect);
	}
}

BOOL PenShape::MouseEvent_DblClick(const Gdiplus::Point& pt, Gdiplus::Graphics* g, float scale)
{
	Draw(g, scale);
	return TRUE;
}

void PenShape::DrawToMask(Gdiplus::Graphics* g)
{
	Gdiplus::Brush* brush = MaskFillMode::ADD == m_FillMode ? m_BrushMask : m_BrushSubtractMask;

	int l_Size = int(m_Points.size());
	if (l_Size > 1) // Do Paint
	{
		Gdiplus::GraphicsPath* Path = new Gdiplus::GraphicsPath();

		m_PenWiden->SetWidth(m_lineSize);
		Path->AddLines(m_Points.data(), l_Size);
		Path->Widen(m_PenWiden);
		Path->Outline();

		g->FillPath(brush, Path);
		
		::DeleteObject(Path);
	}
	else if (l_Size > 0)
	{
		const auto& pt = m_Points[0];
		const int half = m_lineSize >> 1;
		Gdiplus::Rect rect(pt.X - half, pt.Y - half, m_lineSize, m_lineSize);

		g->FillEllipse(brush, rect);
	}
}

double PenShape::distancePointPoint(const Gdiplus::Point& pt0, const Gdiplus::Point& pt1)
{
	double dx = pt1.X - pt0.X;
	double dy = pt1.Y - pt0.Y;

	return std::sqrt(dx * dx + dy * dy);
}