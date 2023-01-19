#pragma once
#include "stdafx.h"

/*
* - Lincoln Lee
* - x.perfect.vn@gmail.com
* - 2022-08-08
*/


class CViewerEx;

class FakeDC: public CDC
{
public:
	FakeDC(CViewerEx* dsp): CDC(), m_dsp(dsp), m_CurrentColor(0xff00ff00), m_CurrentTextColor(0xff00ff00) 
	{
		CreateCompatibleDC(NULL);
	}
	CPen* SelectObject(CPen* pen);

	CBrush* SelectObject(CBrush* brush) { return brush; };
	HGDIOBJ SelectObject(HGDIOBJ hi) { return hi; }
	CFont* SelectObject(CFont* font) { return font; };

	CPoint MoveTo(int x, int y);
	CPoint MoveTo(CPoint pt);
	void SetColor(Gdiplus::Color color);
	void SetTextColor(Gdiplus::Color color);
	COLORREF SetColor(COLORREF color);
	COLORREF SetTextColor(COLORREF color);
	BOOL LineTo(int x, int y);
	BOOL LineTo(CPoint pt);
	void Ellipse(int x1, int y1, int x2, int y2);
	void Ellipse(CRect rect);
	Gdiplus::Color GetCurrentColor() const { return m_CurrentColor; }
	Gdiplus::Color GetCurrentTextColor() const { return m_CurrentTextColor; }

	BOOL TextOut(int x, int y, const CString& str);
	BOOL TextOut(int x, int y, const CString& str, int len) { return TextOut(x, y, str); };
	BOOL Rectangle(CRect rect);
	BOOL Rectangle(int x1, int y1, int x2, int y2);
	BOOL Rectangle(int x1, int y1, int x2, int y2, float thickness);
	void AddPoint(int x, int y, int wh = 3, int thickness = 2);
	template<class T0>
	void AddPoint(cv::Point_<T0> pt, int wh = 3, int thickness = 2);
	void AddGraphic(GraphicBase* graph);
	void AddHardGraphic(GraphicBase* graph);

private:
	CPoint m_LastPoint;
	Gdiplus::Color m_CurrentColor = Gdiplus::Color(0xff00ff00);
	Gdiplus::Color m_CurrentTextColor = Gdiplus::Color(0xff00ff00);
	CViewerEx* m_dsp = NULL;
};

template<class T0>
void FakeDC::AddPoint(cv::Point_<T0> pt, int wh, int thickness)
{
	m_dsp->AddSoftGraphic(new GraphicPoint(pt, m_CurrentColor, wh, thickness));
}