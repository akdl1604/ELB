#include "stdafx.h"
#include "ShapeObject.h"
#include "DOGU.h"

#include <iostream>

ShapeObject::ShapeObject(int lineSize) :
	m_ShapeMode(DOGU::DPEN), m_FillMode(MaskFillMode::ADD), m_lineSize(lineSize), m_nFillMode(0),
	m_PenAdd(new Gdiplus::Pen(0xFFFFFF00, 2)),
	m_PenSubtract(new Gdiplus::Pen(0xFFFF0000, 2)),
	m_PenWiden(new Gdiplus::Pen(0xFFFFFF00, lineSize)),
	m_BrushAdd(new Gdiplus::SolidBrush(0x80FF0000)),
	m_BrushSubtract(new Gdiplus::SolidBrush(0x80000000)),
	m_BrushMask(new Gdiplus::SolidBrush(0xFFFF0000)),
	m_BrushSubtractMask(new Gdiplus::SolidBrush(0xFF000000))
{
	CreateDrawingStyle();
}

void ShapeObject::CreateDrawingStyle()
{
	m_PenAdd->SetDashStyle(Gdiplus::DashStyle::DashStyleDash);
	m_PenSubtract->SetDashStyle(Gdiplus::DashStyle::DashStyleDash);

	m_PenWiden->SetLineJoin(LineJoin::LineJoinRound);
	m_PenWiden->SetStartCap(LineCap::LineCapRound);
	m_PenWiden->SetEndCap(LineCap::LineCapRound);
	m_PenWiden->SetWidth(m_lineSize);
}

int ShapeObject::SetPenWidth(int width)
{
	int oldWidth = m_lineSize;
	m_lineSize = width;
	m_PenWiden->SetWidth(width);
	return oldWidth;
}

void ShapeObject::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << int(m_ShapeMode);
		ar << m_lineSize;
		ar << int(m_Points.size());

		for (const Gdiplus::Point& pt : m_Points)
		{
			ar << pt.X;
			ar << pt.Y;
		}
		ar << m_nFillMode;
		ar << int(m_FillMode);
	}
	else
	{
		int items, mFill, x, y;

		m_Points.clear();
		ar >> m_lineSize;
		ar >> items;
		for (int i = 0; i < items; i++)
		{
			ar >> x;
			ar >> y;
			m_Points.push_back({x, y});
		}
		ar >> m_nFillMode;
		ar >> mFill;

		m_FillMode = MaskFillMode(mFill);
	}
}