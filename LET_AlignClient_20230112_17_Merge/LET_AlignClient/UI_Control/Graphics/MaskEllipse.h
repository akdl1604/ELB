#pragma once
#include "MaskBase.h"

class MaskEllipse: public MaskBase
{
public:
	MaskEllipse(): MaskEllipse(1) {}
	MaskEllipse(int lineSize)
	{
		m_PenWidth = lineSize;
		m_ShapeMode = DOGU::DELLIPSE;
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
			const auto& pt0 = pts[0];
			const auto& pt1 = pts[pts.size() - 1];
			const auto mid = Gdiplus::PointF((pt0.X + pt1.X) / 2.f, (pt0.Y + pt1.Y) / 2.f);
			const auto len = distancePointPoint(pt0, pt1);
			const auto len2 = len / 2.f;
			const auto len3 = lround(len);

			Gdiplus::Rect rect(lround(mid.X - len2), lround(mid.Y - len2), len3, len3);

			auto chk = graphic.Save();
			graphic.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
			if (!modeAdd) graphic.SetCompositingMode(Gdiplus::CompositingModeSourceCopy);
			graphic.FillEllipse(brush, rect);
			graphic.SetCompositingMode(Gdiplus::CompositingModeSourceOver);
			graphic.DrawEllipse(pen, rect);
			graphic.Restore(chk);
		}
		pts.clear();
	}

	void RenderToMask(Gdiplus::Graphics& graphic)
	{
		Gdiplus::Brush* brush = MaskFillMode::ADD == m_FillMode ? m_BrushMask: m_BrushSubtractMask;

		if (m_Points.size() > 1)
		{
			const auto& pt0 = m_Points[0];
			const auto& pt1 = m_Points[m_Points.size() - 1];
			const auto mid = Gdiplus::PointF((pt0.X + pt1.X) / 2.f, (pt0.Y + pt1.Y) / 2.f);
			const auto len = distancePointPoint(pt0, pt1);
			const auto len2 = len / 2.f;
			const auto len3 = lround(len);

			Gdiplus::Rect rect(lround(mid.X - len2), lround(mid.Y - len2), len3, len3);
			graphic.FillEllipse(brush, rect);
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

