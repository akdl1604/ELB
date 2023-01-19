#include "stdafx.h"
#include "MaskBase.h"
#include "DOGU.h"
#include <iostream>

MaskBase::MaskBase(int lineSize) :
	m_ShapeMode(DOGU::DPEN), m_FillMode(MaskFillMode::ADD), m_PenWidth(lineSize), m_nFillMode(0),
	m_PenAdd(new Gdiplus::Pen(0xFFFFFF00, 2)),
	m_PenSubtract(new Gdiplus::Pen(0xFFFF0000, 2)),
	m_PenWiden(new Gdiplus::Pen(0xFFFFFF00, lineSize)),
	m_BrushAdd(new Gdiplus::SolidBrush(0x70FF0000)),
	m_BrushSubtract(new Gdiplus::SolidBrush(0x00)),
	m_BrushMask(new Gdiplus::SolidBrush(0xFFFF0000)),
	m_BrushSubtractMask(new Gdiplus::SolidBrush(0xFF000000))
{
	CreateDrawingStyle();
}

double MaskBase::distancePointPoint(const Gdiplus::Point& pt0, const Gdiplus::Point& pt1)
{
	double dx = pt1.X - pt0.X;
	double dy = pt1.Y - pt0.Y;

	return std::sqrt(dx * dx + dy * dy);
}

double MaskBase::distancePointPoint(const Gdiplus::PointF& pt0, const CPoint& pt1)
{
	double dx = pt1.x - pt0.X;
	double dy = pt1.y - pt0.Y;

	return std::sqrt(dx * dx + dy * dy);
}

double MaskBase::distancePointPoint(const Gdiplus::PointF& pt0, const Gdiplus::Point& pt1)
{
	double dx = pt1.X - pt0.X;
	double dy = pt1.Y - pt0.Y;

	return std::sqrt(dx * dx + dy * dy);
}

double MaskBase::distancePointPoint(const Gdiplus::Point& pt0, const Gdiplus::PointF& pt1)
{
	double dx = pt1.X - pt0.X;
	double dy = pt1.Y - pt0.Y;

	return std::sqrt(dx * dx + dy * dy);
}

double MaskBase::distancePointPoint(const Gdiplus::PointF& pt0, const Gdiplus::PointF& pt1)
{
	double dx = pt1.X - pt0.X;
	double dy = pt1.Y - pt0.Y;

	return std::sqrt(dx * dx + dy * dy);
}

double MaskBase::distancePointPoint(const CPoint& pt0, const CPoint& pt1)
{
	double dx = pt1.x - pt0.x;
	double dy = pt1.y - pt0.y;

	return std::sqrt(dx * dx + dy * dy);
}

void MaskBase::CreateDrawingStyle()
{
	m_PenAdd->SetDashStyle(Gdiplus::DashStyle::DashStyleDash);
	m_PenSubtract->SetDashStyle(Gdiplus::DashStyle::DashStyleDash);

	m_PenWiden->SetLineJoin(LineJoin::LineJoinRound);
	m_PenWiden->SetStartCap(LineCap::LineCapRound);
	m_PenWiden->SetEndCap(LineCap::LineCapRound);
	m_PenWiden->SetWidth(m_PenWidth);
}

int MaskBase::SetPenWidth(int width)
{
	int oldWidth = m_PenWidth;
	m_PenWidth = width;
	m_PenWiden->SetWidth(width);
	return oldWidth;
}

void MaskBase::Serialize(CArchive& ar)
{
	if (ar.IsStoring())
	{
		ar << int(m_ShapeMode);
		ar << m_PenWidth;
		ar << int(m_Points.size());

		for (const Gdiplus::PointF& pt: m_Points)
		{
			ar << pt.X;
			ar << pt.Y;
		}
		ar << m_nFillMode;
		ar << int(m_FillMode);
	}
	else
	{
		float x, y;
		int items, mFill;

		m_Points.clear();
		ar >> m_PenWidth;
		ar >> items;
		for (int i = 0; i < items; i++)
		{
			ar >> x;
			ar >> y;
			m_Points.push_back(Gdiplus::PointF(x, y));
		}
		ar >> m_nFillMode;
		ar >> mFill;

		m_FillMode = MaskFillMode(mFill);
		m_Finished = true;
	}
}