#pragma once
#include "stdafx.h"
#include <afxwin.h>
#include <gdiplus.h>
#include <vector>
#include "DOGU.h"
#include "MaskFillMode.h"


class MaskBase: public CObject
{
public:
	MaskBase(): MaskBase(10) {};
	MaskBase(int penWidth);

	virtual bool IsValid() const { return false; }
	virtual bool MouseDown(CPoint pos, const Rigid& trans) { return false; }  // need update graphic or not
	virtual bool MouseUp(CPoint pos, const Rigid& trans) { return false; }    // need update graphic or not
	virtual bool MouseMove(CPoint pos, const Rigid& trans) { return false; }  // need update graphic or not
	virtual bool MouseDbClk(CPoint pos, const Rigid& trans) { return false; } // need update graphic or not
	virtual void Render(Gdiplus::Graphics& graphic, const Rigid& imgToView) {};
	virtual void RenderToMask(Gdiplus::Graphics& graphic) {};
	virtual void Serialize(CArchive& ar);
	virtual void SetMaxSize(Gdiplus::Size size) { m_MaxSize = size; }
	virtual void SetMaxSize(cv::Size size) { m_MaxSize = Gdiplus::Size(size.width, size.height); }
	virtual void SetMaxSize(CSize size) { m_MaxSize = Gdiplus::Size(size.cx, size.cy); }
	virtual void SetMaxSize(int width, int height) { m_MaxSize = Gdiplus::Size(width, height); }
	virtual void DoFinish(){};
	virtual void SetFillMode(MaskFillMode fillMode) { m_FillMode = fillMode; }
	virtual MaskFillMode GetFillMode() { return m_FillMode; }
	virtual DOGU GetShapeMode() const { return m_ShapeMode; }
	virtual int SetPenWidth(int width);
	virtual int GetPenWidth() const { return m_PenWidth; }

protected:
	Gdiplus::Pen* m_PenAdd;
	Gdiplus::Pen* m_PenSubtract;
	Gdiplus::Pen* m_PenWiden;

	Gdiplus::Brush* m_BrushAdd;
	Gdiplus::Brush* m_BrushSubtract;
	Gdiplus::Brush* m_BrushMask;
	Gdiplus::Brush* m_BrushSubtractMask;

	int m_PenWidth;
	int m_nFillMode;
	DOGU m_ShapeMode;
	MaskFillMode m_FillMode;
	bool m_Finished = false;
	Gdiplus::Size m_MaxSize;
	Gdiplus::PointF m_CurrentTempPoint;
	std::vector<Gdiplus::PointF> m_Points;
	
protected:
	virtual Gdiplus::Pen* GetPen() const { return MaskFillMode::ADD == m_FillMode ? m_PenAdd: m_PenSubtract; }
	virtual Gdiplus::Brush* GetBrush() const { return MaskFillMode::ADD == m_FillMode ? m_BrushAdd: m_BrushSubtract; }
	virtual void CreateDrawingStyle();
	virtual double distancePointPoint(const CPoint& pt0, const CPoint& pt1);
	virtual double distancePointPoint(const Gdiplus::PointF& pt0, const CPoint& pt1);
	virtual double distancePointPoint(const Gdiplus::Point& pt0, const Gdiplus::Point& pt1);
	virtual double distancePointPoint(const Gdiplus::Point& pt0, const Gdiplus::PointF& pt1);
	virtual double distancePointPoint(const Gdiplus::PointF& pt0, const Gdiplus::Point& pt1);
	virtual double distancePointPoint(const Gdiplus::PointF& pt0, const Gdiplus::PointF& pt1);
};
