#pragma once
#include "stdafx.h"
#include <opencv.hpp>
#include "GraphicBase.h"

/*
* - Lincoln Lee
* - x.perfect.vn@gmail.com
* - 2022-08-08
*/


class GraphicHitTest: public GraphicBase
{
public:
	GraphicHitTest()
	{
		Init(cv::Point(0, 0), cv::Size(0, 0), IDC_ARROW);
	}

	GraphicHitTest(CPoint center, CSize size, TCHAR* cursorShape = IDC_ARROW)
	{
		Init(cv::Point(center.x, center.y), cv::Size(size.cx, size.cy), cursorShape);
	}

	template<class T0>
	GraphicHitTest(cv::Point_<T0> center, CSize size, TCHAR* cursorShape)
	{
		Init(center, cv::Size(size.cx, size.cy), cursorShape);
	}

	template<class T0>
	GraphicHitTest& Init(cv::Point_<T0> center, CSize size, TCHAR* cursorShape)
	{
		return Init(center, cv::Size(size.cx, size.cy), cursorShape);
	}

	template<class T0, class T1>
	GraphicHitTest& Init(cv::Point_<T0> center, cv::Size_<T1> size, TCHAR* cursorShape)
	{
		m_CenterPoint.x = center.x;
		m_CenterPoint.y = center.y;

		m_Size.width = size.width;
		m_Size.height = size.height;

		m_Cursor = LoadCursor(NULL, cursorShape);

		return *this;
	}

	template<class T0>
	GraphicHitTest& ChangeCenterPoint(cv::Point_<T0> center)
	{
		m_CenterPoint.x = center.x;
		m_CenterPoint.y = center.y;
		return *this;
	}

	virtual GraphicBase& Render(Gdiplus::Graphics& graph, const Rigid& trans)
	{
		auto pt = trans.Transform(m_CenterPoint);
		auto w = m_Size.width;
		auto h = m_Size.height;
		auto w2 = w / 2.;
		auto h2 = h / 2.;
		Gdiplus::RectF rect(pt.x - w2, pt.y - h2, w, h);

		graph.FillRectangle(m_Brush, rect);
		graph.DrawRectangle(m_Pen, rect);

		return *this;
	}

	virtual bool MouseDown(CPoint pos, const Rigid& trans)
	{
		auto rect = GetRect(trans.Transform(m_CenterPoint), m_Size);		bool ret = false;
		if ((ret = IsPointInRect(rect, pos)))
			SetCursor(m_Cursor);

		return ret;
	}

	virtual bool MouseUp(CPoint pos, const Rigid& trans) { return false; }
	virtual bool MouseMove(CPoint pos, const Rigid& trans)
	{
		auto rect = GetRect(trans.Transform(m_CenterPoint), m_Size);
		bool ret = false;
		if ((ret = IsPointInRect(rect, pos)))
			SetCursor(m_Cursor);

		return ret;
	}

	virtual bool MouseHover(CPoint pos, const Rigid& trans)
	{
		auto rect = GetRect(trans.Transform(m_CenterPoint), m_Size);

		bool ret = false;
		if ((ret = IsPointInRect(rect, pos)))
			SetCursor(m_Cursor);

		return ret;
	}

	virtual bool IsInterative() const { return true; }

	~GraphicHitTest()
	{
		delete m_Pen;
		delete m_Brush;
	}
protected:
private:
	cv::Size2f m_Size;
	cv::Point2f m_CenterPoint;

	HCURSOR m_Cursor;
	Gdiplus::Pen* m_Pen = new Gdiplus::Pen(0xff000000, 2);
	Gdiplus::Brush* m_Brush = new Gdiplus::SolidBrush(Gdiplus::Color(0xd0d0d000));
};

/*
MouseEventCallback m_OnMouseUp;
MouseEventCallback m_OnMouseDown;
MouseEventCallback m_OnMouseMove;
MouseEventCallback m_OnMouseHover;
GraphicHitTest& SetOnMouseUp(MouseEventCallback ev)
{
	m_OnMouseUp = ev;
	return *this;
}

GraphicHitTest& SetOnMouseDown(MouseEventCallback ev)
{
	m_OnMouseDown = ev;
	return *this;
}

GraphicHitTest& SetOnMouseMove(MouseEventCallback ev)
{
	m_OnMouseMove = ev;
	return *this;
}

GraphicHitTest& SetOnMouseHover(MouseEventCallback ev)
{
	m_OnMouseHover = ev;
	return *this;
}
*/