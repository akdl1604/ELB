#pragma once
#include "MaskBase.h"

class MaskPickPoint: public MaskBase
{
public:
	MaskPickPoint(): MaskPickPoint(Gdiplus::PointF(10, 10), 2) {}
	MaskPickPoint(Gdiplus::PointF lastPoint, int lineSize): m_LastSelected(lastPoint)
	{
		m_PenWidth = lineSize;
		m_SelectedPen = new Gdiplus::Pen(0xff00ff00, m_PenWidth);
		m_SelectedPen->SetDashStyle(Gdiplus::DashStyleDash);
		m_SelectedPen->SetWidth(m_PenWidth);

		m_PenAdd = new Gdiplus::Pen(0xffff00ff, m_PenWidth);
		m_PenAdd->SetDashStyle(Gdiplus::DashStyleDash);
		m_PenAdd->SetWidth(m_PenWidth);
	}

	bool MouseDown(CPoint pos, const Rigid& trans)
	{
		m_Finished = true;
		m_LastSelected = trans.Inverse().Transform<Gdiplus::PointF>(pos);
		return true;
	}

	bool MouseUp(CPoint pos, const Rigid& trans)
	{
		m_Finished = true;
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
		return true;
	}

	Gdiplus::PointF GetSelectedPoint() const { return m_LastSelected; }
	MaskPickPoint& SetSelectedPoint(const Gdiplus::PointF& pt) { return SetSelectedPoint(pt.X, pt.Y); }
	MaskPickPoint& SetSelectedPoint(const CPoint& pt) { return SetSelectedPoint(pt.x, pt.y); }
	MaskPickPoint& SetSelectedPoint(float x, float y) { m_LastSelected = Gdiplus::PointF(MIN(m_MaxSize.Width, MAX(x, 0)), MIN(m_MaxSize.Height, MAX(y, 0))); return *this; }
	MaskPickPoint& OffsetSelectedPoint(float x, float y) { return SetSelectedPoint(m_LastSelected.X + x, m_LastSelected.Y + y); }
	MaskPickPoint& SetActive(bool act = true) { m_Finished = !act; return *this; }
	MaskPickPoint& SetActiveRectMode(bool act = true) { m_IsRectTangleMode = act; return *this; }

	bool IsActive() const { return !m_Finished; }

	void Render(Gdiplus::Graphics& graphic, const Rigid& imgToView)
	{
		Gdiplus::Pen* pen = MaskFillMode::ADD == m_FillMode ? m_PenAdd: m_PenSubtract;
		Gdiplus::Brush* brush = MaskFillMode::ADD == m_FillMode ? m_BrushAdd: m_BrushSubtract;
		auto pt = imgToView.Transform(m_LastSelected);
		auto tl = imgToView.Transform(cv::Point2f(0, 0));
		auto br = imgToView.Transform(cv::Point2f(m_MaxSize.Width, m_MaxSize.Height));
		auto chk = graphic.Save();
		graphic.SetSmoothingMode(Gdiplus::SmoothingModeNone);

		if (m_IsRectTangleMode)
		{
			auto pt1 = imgToView.Transform(m_CurrentTempPoint);
			if (m_Finished)			pt = imgToView.Transform(m_LastTmpSelected);

			graphic.DrawLine(pen, Gdiplus::PointF((pt1.X + pt.X) / 2, tl.y), Gdiplus::PointF((pt1.X + pt.X) / 2, br.y));
			graphic.DrawLine(pen, Gdiplus::PointF(tl.x, (pt1.Y + pt.Y) / 2), Gdiplus::PointF(br.x, (pt1.Y + pt.Y) / 2));

			if (m_Finished)			m_LastSelected = Gdiplus::PointF((m_CurrentTempPoint.X + m_LastTmpSelected.X) / 2, (m_CurrentTempPoint.Y + m_LastTmpSelected.Y) / 2);
		}
		else
		{
			graphic.DrawLine(m_SelectedPen, Gdiplus::PointF(pt.X, tl.y), Gdiplus::PointF(pt.X, br.y));
			graphic.DrawLine(m_SelectedPen, Gdiplus::PointF(tl.x, pt.Y), Gdiplus::PointF(br.x, pt.Y));
		}
		if (!m_Finished)
		{
			auto pt1 = imgToView.Transform(m_CurrentTempPoint);

			if (m_IsRectTangleMode)
			{
				auto t_pt = pt;
				auto t_pt1 = pt1;

				if (t_pt.X > t_pt1.X) swap(t_pt.X , t_pt1.X);
				if (t_pt.Y > t_pt1.Y) swap(t_pt.Y, t_pt1.Y);

				graphic.DrawRectangle(pen, Gdiplus::RectF(t_pt.X, t_pt.Y, t_pt1.X- t_pt.X, t_pt1.Y- t_pt.Y));
				graphic.DrawLine(pen, Gdiplus::PointF((pt1.X+pt.X)/2, tl.y), Gdiplus::PointF((pt1.X + pt.X) / 2, br.y));
				graphic.DrawLine(pen, Gdiplus::PointF(tl.x, (pt1.Y + pt.Y) / 2), Gdiplus::PointF(br.x, (pt1.Y + pt.Y) / 2));
				m_LastTmpSelected = m_LastSelected;
			}
			else
			{
				graphic.DrawLine(pen, Gdiplus::PointF(pt1.X, tl.y), Gdiplus::PointF(pt1.X, br.y));
				graphic.DrawLine(pen, Gdiplus::PointF(tl.x, pt1.Y), Gdiplus::PointF(br.x, pt1.Y));
			}
		}

		graphic.Restore(chk);
	}

	~MaskPickPoint()
	{
		MaskBase::~MaskBase();
		delete m_SelectedPen;
	}
private:
	Gdiplus::Pen* m_SelectedPen;
	Gdiplus::PointF m_LastSelected;
	Gdiplus::PointF m_LastTmpSelected;
	BOOL m_IsRectTangleMode = FALSE;
	BOOL m_IsMouseDown = FALSE;

	double distancePointPoint(const Gdiplus::PointF& pt0, const Gdiplus::PointF& pt1)
	{
		double dx = pt1.X - pt0.X;
		double dy = pt1.Y - pt0.Y;
		return std::sqrt(dx * dx + dy * dy);
	}
};

