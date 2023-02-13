#pragma once
#include "MaskBase.h"

class MaskRectangle: public MaskBase
{
public:
	MaskRectangle(): MaskRectangle(1) {}
	MaskRectangle(int lineSize)
	{
		m_PenWidth = lineSize;
		m_ShapeMode = DOGU::DRECT;
		CreateDrawingStyle();
	}

	bool MouseDown(CPoint pos, const Rigid& trans)
	{
		m_Finished = false;
		m_IsMouseDown = true;
		m_Points.push_back(trans.Inverse().Transform<Gdiplus::PointF>(pos));
		return false;
	}

	bool MouseUp(CPoint pos, const Rigid& trans)
	{
		m_Finished = true;
		m_IsMouseDown = false;
		m_Points.push_back(trans.Inverse().Transform<Gdiplus::PointF>(pos));
		return true;
	}

	bool MouseMove(CPoint pos, const Rigid& trans)
	{
		m_CurrentTempPoint = trans.Inverse().Transform<Gdiplus::PointF>(pos);
		return false;
	}

	bool MouseDbClk(CPoint pos, const Rigid& trans)
	{
		m_Finished = true;
		m_IsMouseDown = false;
		return true;
	}

	void Render(Gdiplus::Graphics& graphic, const Rigid& imgToView)
	{
		bool modeAdd = MaskFillMode::ADD == m_FillMode;
		Gdiplus::Pen* pen = modeAdd ? m_PenAdd: m_PenSubtract;
		Gdiplus::Brush* brush = modeAdd ? m_BrushAdd: m_BrushSubtract;

		auto pts = imgToView.Transform(m_Points);
		if (!m_Finished) pts.push_back(imgToView.Transform(m_CurrentTempPoint));

		if (pts.size() > 1)
		{
			const auto& pt00 = pts[0];
			const auto& pt01 = pts[pts.size() - 1];
			const auto pt0 = Gdiplus::PointF(MIN(pt00.X, pt01.X), MIN(pt00.Y, pt01.Y));
			const auto pt1 = Gdiplus::PointF(MAX(pt00.X, pt01.X), MAX(pt00.Y, pt01.Y));
			Gdiplus::RectF rect(pt0.X, pt0.Y, (pt1.X - pt0.X), (pt1.Y - pt0.Y));

			auto chk = graphic.Save();
			graphic.SetSmoothingMode(Gdiplus::SmoothingModeNone);
			if (!modeAdd) graphic.SetCompositingMode(Gdiplus::CompositingModeSourceCopy);
			graphic.FillRectangle(brush, rect);
			graphic.SetCompositingMode(Gdiplus::CompositingModeSourceOver);
			graphic.DrawRectangle(pen, rect);
			graphic.Restore(chk);
		}
		pts.clear();
	}

	void RenderToMask(Gdiplus::Graphics& graphic)
	{
		Gdiplus::Brush* brush = MaskFillMode::ADD == m_FillMode ? m_BrushMask: m_BrushSubtractMask;

		if (m_Points.size() > 1)
		{
			const auto& pt00 = m_Points[0];
			const auto& pt01 = m_Points[m_Points.size() - 1];
			const auto pt0 = Gdiplus::PointF(MIN(pt00.X, pt01.X), MIN(pt00.Y, pt01.Y));
			const auto pt1 = Gdiplus::PointF(MAX(pt00.X, pt01.X), MAX(pt00.Y, pt01.Y));
			Gdiplus::RectF rect(pt0.X, pt0.Y, (pt1.X - pt0.X), (pt1.Y - pt0.Y));

			graphic.FillRectangle(brush, rect);
		}
	}

	bool IsValid() const { return m_Points.size() > 1; }
private:
	BOOL m_IsMouseDown = FALSE;

	double distancePointPoint(const Gdiplus::PointF& pt0, const Gdiplus::PointF& pt1)
	{
		double dx = pt1.X - pt0.X;
		double dy = pt1.Y - pt0.Y;
		return std::sqrt(dx * dx + dy * dy);
	}
};
