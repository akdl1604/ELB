#pragma once
#include "MaskBase.h"

class MaskPen: public MaskBase
{
public:
	MaskPen(): MaskPen(20) {}
	MaskPen(int lineSize)
	{
		m_PenWidth = lineSize;
		m_ShapeMode = DOGU::DPEN;
		CreateDrawingStyle();
	}

	bool MouseDown(CPoint pos, const Rigid& trans)
	{
		m_IsMouseDown = true;
		m_Finished = false;
		m_CurrentTempPoint = trans.Inverse().Transform<Gdiplus::PointF>(pos);
		m_Points.push_back(m_CurrentTempPoint);
		return false;
	}

	bool MouseUp(CPoint pos, const Rigid& trans)
	{
		m_Finished = true;
		m_IsMouseDown = false;
		return true;
	}

	bool MouseMove(CPoint pos, const Rigid& trans)
	{
		m_CurrentTempPoint = trans.Inverse().Transform<Gdiplus::PointF>(pos);
		if (m_IsMouseDown)
		{
			if (m_Points.size() > 0)
			{
				auto last = trans.Transform(m_Points[m_Points.size() - 1]);
				if (distancePointPoint(last, pos) > 3.)
					m_Points.push_back(m_CurrentTempPoint);
			}
			else m_Points.push_back(m_CurrentTempPoint);
		}
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
		auto scale = imgToView.Scale();

		auto m = imgToView.GetGdiMatrix();
		auto chk = graphic.Save();
		graphic.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
		if (!modeAdd) graphic.SetCompositingMode(Gdiplus::CompositingModeSourceCopy);
		if (!m_Finished)
		{
			int l_Size = int(m_Points.size());
			auto pt = imgToView.Transform(m_CurrentTempPoint);
			auto half = float((m_PenWidth / 2.f) * scale);
			auto diam = float(m_PenWidth * scale);
			auto rect = Gdiplus::Rect((pt.X - half), (pt.Y - half), diam, diam);
			if (l_Size > 1)
			{
				CreatePath();
				m_Path.Transform(m);
				graphic.FillPath(brush, &m_Path);
				graphic.FillEllipse(brush, rect);
				graphic.SetCompositingMode(Gdiplus::CompositingModeSourceOver);
				graphic.DrawPath(pen, &m_Path);
				graphic.DrawEllipse(pen, rect);
				m->Invert();
				m_Path.Transform(m);
			}
			else
			{
				if (l_Size > 0)
				{
					pt = imgToView.Transform(m_Points[0]);
					rect = Gdiplus::Rect((pt.X - half), (pt.Y - half), diam, diam);
				}
				graphic.FillEllipse(brush, rect);
				graphic.SetCompositingMode(Gdiplus::CompositingModeSourceOver);
				graphic.DrawEllipse(pen, rect);
			}
		}
		else
		{
			m_Path.Transform(m);
			graphic.FillPath(brush, &m_Path);
			graphic.SetCompositingMode(Gdiplus::CompositingModeSourceOver);
			graphic.DrawPath(pen, &m_Path);
			m->Invert();
			m_Path.Transform(m);
		}
		graphic.Restore(chk);
		delete m;
	}

	void RenderToMask(Gdiplus::Graphics& graphic)
	{
		Gdiplus::Brush* brush = MaskFillMode::ADD == m_FillMode ? m_BrushMask: m_BrushSubtractMask;
		auto count = m_Points.size();
		if (count > 1)
		{
			graphic.FillPath(brush, &m_Path);
		}
		else if (count > 0)
		{
			auto pt = m_Points[0];
			auto half = float(m_PenWidth / 2.f);
			auto diam = float(m_PenWidth);
			auto rect = Gdiplus::Rect((pt.X - half), (pt.Y - half), diam, diam);
			graphic.FillEllipse(brush, rect);
		}
	}

	void Serialize(CArchive& ar) override
	{
		MaskBase::Serialize(ar);
		CreatePath();
		m_Finished = true;
	}

	bool IsValid() const { return m_Points.size() > 0; }
private:
	BOOL m_IsMouseDown = FALSE;
	Gdiplus::GraphicsPath m_Path;

	void CreatePath()
	{
		m_Path.Reset();
		m_Path.SetFillMode(FillMode::FillModeWinding);
		m_Path.AddLines(m_Points.data(), INT(m_Points.size()));
		m_Path.Widen(m_PenWiden);
		m_Path.Outline();
	}
};

