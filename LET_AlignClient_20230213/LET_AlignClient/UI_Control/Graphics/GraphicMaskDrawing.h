#pragma once
#include "stdafx.h"
#include "LincUtils.h"
#include "DOGU.h"
#include "MaskBase.h"
#include "MaskPolygon.h"
#include "MaskEllipse.h"
#include "MaskPen.h"
#include "MaskPickPoint.h"
#include "MaskRectangle.h"
#include "MaskFillMode.h"

/*
* - Lincoln Lee
* - x.perfect.vn@gmail.com
* - 2022-08-08
*/

class GraphicMaskDrawing: public GraphicBase
{
public:
	// using for Interative graphic
	bool IsActive() const { return true; }
	bool IsSkipMouseDown() const { return true; }
	bool IsInterative() const { return true; } // Normaly non interative graphic

	GraphicMaskDrawing();
	~GraphicMaskDrawing();

	bool MouseDown(CPoint pos, const Rigid& trans);
	bool MouseUp(CPoint pos, const Rigid& trans);
	bool MouseMove(CPoint pos, const Rigid& trans);
	bool MouseDbClk(CPoint pos, const Rigid& trans);

	GraphicMaskDrawing& SetActive(bool act = true);
	GraphicMaskDrawing& SetMetricTransform(const Rigid& rigid);
	GraphicMaskDrawing& Render(Gdiplus::Graphics& graph, const Rigid& trans);
	GraphicMaskDrawing& ImportShapes(CString strFile);
	GraphicMaskDrawing& ExportShapes(CString strFile);

	GraphicMaskDrawing& SetPickPointMode(bool ena = true);
	GraphicMaskDrawing& SetFillMode(MaskFillMode fillMode);
	GraphicMaskDrawing& SetMaxSize(Gdiplus::Size size);
	GraphicMaskDrawing& SetMaxSize(cv::Size size);
	GraphicMaskDrawing& SetMaxSize(CSize size);
	GraphicMaskDrawing& SetMaxSize(int width, int height);
	
	GraphicMaskDrawing& OffsetOrign(float dx, float dy);
	GraphicMaskDrawing& DoShape(DOGU shapeMode);
	GraphicMaskDrawing& DoShape(DOGU shapeMode, int size);
	GraphicMaskDrawing& DoShape();
	GraphicMaskDrawing& ClearShapeObjects();
	GraphicMaskDrawing& RemoveLastItem();
	GraphicMaskDrawing& IgnoreMouse(bool val = true);
	GraphicMaskDrawing& SetPickRectMode(bool brect = false);

	bool IsPickPointMode() const;
	int GetPenWidth() const;
	GraphicMaskDrawing& SetPenWidth(int w);

	Gdiplus::PointF GetSelectedPoint() const;
	GraphicMaskDrawing& SetSelectedPoint(float x, float y);

	cv::Mat GetMarkMask();
	GraphicMaskDrawing& SaveMask(CString strFile);
protected:
private:
	int m_PenWidth = 1;
	bool m_StopPassingEvent = false;
	
	MaskPickPoint m_PickPoint;
	MaskBase* m_CurrentActive = NULL;
	std::vector<MaskBase*> m_MaskObjects;

	Gdiplus::Size m_ImageSize{0, 0};
	DOGU m_ShapeMode = DOGU::DNONE;
	MaskFillMode m_FillMode = MaskFillMode::ADD;
private:
};