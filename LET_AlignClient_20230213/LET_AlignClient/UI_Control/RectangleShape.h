#pragma once
#include "ShapeObject.h"
#include "DOGU.h"
#include <vector>

struct RectangleShape : public ShapeObject
{
public:
	virtual BOOL MouseEvent_Down(const Gdiplus::Point& pt, Gdiplus::Graphics& g, float scale = 1.) { return MouseEvent_Down(pt, &g, scale); };
	virtual BOOL MouseEvent_Up(const Gdiplus::Point& pt, Gdiplus::Graphics& g, float scale = 1.) { return MouseEvent_Up(pt, &g, scale); };
	virtual BOOL MouseEvent_Move(const Gdiplus::Point& pt, Gdiplus::Graphics& g, float scale = 1.) { return MouseEvent_Move(pt, &g, scale); };
	virtual BOOL MouseEvent_DblClick(const Gdiplus::Point& pt, Gdiplus::Graphics& g, float scale = 1.) { return MouseEvent_DblClick(pt, &g, scale); };
	virtual void Draw(Gdiplus::Graphics& g, float scale = 1.f) { Draw(&g, scale); };
	virtual void DrawToMask(Gdiplus::Graphics& g) { DrawToMask(&g); };

	virtual BOOL MouseEvent_Down(const Gdiplus::Point& pt, Gdiplus::Graphics* g, float scale = 1.);
	virtual BOOL MouseEvent_Up(const Gdiplus::Point& pt, Gdiplus::Graphics* g, float scale = 1.);
	virtual BOOL MouseEvent_Move(const Gdiplus::Point& pt, Gdiplus::Graphics* g, float scale = 1.);
	virtual BOOL MouseEvent_DblClick(const Gdiplus::Point& pt, Gdiplus::Graphics* g, float scale = 1.);
	virtual void Draw(Gdiplus::Graphics* g, float scale = 1.f);
	virtual void DrawToMask(Gdiplus::Graphics* g);

	RectangleShape() : RectangleShape(1) {}
	RectangleShape(int lineSize);
};