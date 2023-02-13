#include "stdafx.h"
#include "MaskPolygon.h"

MaskPolygon::MaskPolygon(int lineSize)
{
	m_PenWidth = lineSize;
	m_ShapeMode = DOGU::D_POLYGON;
	CreateDrawingStyle();
}

bool MaskPolygon::MouseDown(CPoint pos, const Rigid& trans)
{
	m_CurrentTempPoint = trans.Inverse().Transform<Gdiplus::PointF>(pos);
	m_Points.push_back(m_CurrentTempPoint);
	return false;
}

bool MaskPolygon::MouseUp(CPoint pos, const Rigid& trans)
{
	return false;
}

bool MaskPolygon::MouseMove(CPoint pos, const Rigid& trans)
{
	m_CurrentTempPoint = trans.Inverse().Transform<Gdiplus::PointF>(pos);
	return false;
}

bool MaskPolygon::MouseDbClk(CPoint pos, const Rigid& trans)
{
	auto imPOS = trans.Inverse().Transform<Gdiplus::PointF>(pos);
	m_Points.push_back(imPOS);
	m_Finished = true;

	return true;
}

void MaskPolygon::Render(Gdiplus::Graphics& graphic, const Rigid& imgToView)
{
	bool modeAdd = MaskFillMode::ADD == m_FillMode;
	Gdiplus::Pen* pen = modeAdd ? m_PenAdd: m_PenSubtract;
	Gdiplus::Brush* brush = modeAdd ? m_BrushAdd: m_BrushSubtract;

	auto pts = imgToView.Transform(m_Points);
	if (!m_Finished) pts.push_back(imgToView.Transform(m_CurrentTempPoint));
	int l_Size = int(pts.size());

	auto chk = graphic.Save();
	graphic.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
	
	if (l_Size > 2)
	{
		if (!modeAdd) graphic.SetCompositingMode(Gdiplus::CompositingModeSourceCopy);
		graphic.FillPolygon(brush, pts.data(), l_Size);
		graphic.SetCompositingMode(Gdiplus::CompositingModeSourceOver);
		graphic.DrawPolygon(pen, pts.data(), l_Size);
	}
	else if (l_Size > 1) // Do Paint
		graphic.DrawLine(pen, pts[0], pts[1]);

	graphic.Restore(chk);
}

void MaskPolygon::RenderToMask(Gdiplus::Graphics& graphic)
{
	Gdiplus::Brush* brush = MaskFillMode::ADD == m_FillMode ? m_BrushMask: m_BrushSubtractMask;
	auto count = m_Points.size();
	if (int(count) > 2) graphic.FillPolygon(brush, m_Points.data(), INT(count));
}