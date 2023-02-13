#pragma once
#include "stdafx.h"
#include <afxwin.h>
#include <gdiplus.h>
#include <vector>
#include "DOGU.h"
#include "MaskFillMode.h"

using namespace Gdiplus;

struct ShapeObject : public CObject
{
protected:
	Gdiplus::Pen* m_PenAdd;
	Gdiplus::Pen* m_PenSubtract;
	Gdiplus::Pen* m_PenWiden;

	Gdiplus::Brush* m_BrushAdd;
	Gdiplus::Brush* m_BrushSubtract;
	Gdiplus::Brush* m_BrushMask;
	Gdiplus::Brush* m_BrushSubtractMask;

	DOGU m_ShapeMode;
	int m_lineSize;
	int m_nFillMode;
	MaskFillMode m_FillMode;
	std::vector<Gdiplus::Point> m_Points;

public:
	ShapeObject() : ShapeObject(40) {};
	ShapeObject(int lineSize);

	virtual BOOL MouseEvent_Down(const Gdiplus::Point& pt, Gdiplus::Graphics& dc, float scale = 1.) { return FALSE; };
	virtual BOOL MouseEvent_Up(const Gdiplus::Point& pt, Gdiplus::Graphics& dc, float scale = 1.) { return FALSE; };
	virtual BOOL MouseEvent_Move(const Gdiplus::Point& pt, Gdiplus::Graphics& dc, float scale = 1.) { return FALSE; };
	virtual BOOL MouseEvent_DblClick(const Gdiplus::Point& pt, Gdiplus::Graphics& dc, float scale = 1.) { return FALSE; };
	virtual void Draw(Gdiplus::Graphics& g, float scale = 1.f) {};
	virtual void DrawToMask(Gdiplus::Graphics& g) {};

	virtual BOOL MouseEvent_Down(const Gdiplus::Point& pt, Gdiplus::Graphics* dc, float scale = 1.) { return FALSE; };
	virtual BOOL MouseEvent_Up(const Gdiplus::Point& pt, Gdiplus::Graphics* dc, float scale = 1.) { return FALSE; };
	virtual BOOL MouseEvent_Move(const Gdiplus::Point& pt, Gdiplus::Graphics* dc, float scale = 1.) { return FALSE; };
	virtual BOOL MouseEvent_DblClick(const Gdiplus::Point& pt, Gdiplus::Graphics* dc, float scale = 1.) { return FALSE; };
	virtual void Draw(Gdiplus::Graphics* g, float scale = 1.f) {};
	virtual void DrawToMask(Gdiplus::Graphics* g) {};

	virtual int SetPenWidth(int width);
	virtual int GetPenWidth() { return m_lineSize; }

	virtual void SetFillMode(MaskFillMode fillMode) { m_FillMode = fillMode; }
	virtual MaskFillMode GetFillMode() { return m_FillMode; }
	virtual Gdiplus::Pen* GetPen() const { return MaskFillMode::ADD == m_FillMode ? m_PenAdd : m_PenSubtract; }
	virtual Gdiplus::Brush* GetBrush() const { return MaskFillMode::ADD == m_FillMode ? m_BrushAdd : m_BrushSubtract; }
	virtual void DoFinish() { };
	virtual void Serialize(CArchive& ar);
	virtual void CreateDrawingStyle();
	virtual DOGU GetShapeMode() const { return m_ShapeMode; }
	virtual int GetLineSize() const { return m_lineSize; }

	virtual ~ShapeObject()
	{
		m_Points.clear();
		::DeleteObject(m_PenAdd);
		::DeleteObject(m_PenSubtract);
		::DeleteObject(m_PenWiden);
		::DeleteObject(m_BrushAdd);
		::DeleteObject(m_BrushSubtract);
		::DeleteObject(m_BrushMask);
		::DeleteObject(m_BrushSubtractMask);
	};
};