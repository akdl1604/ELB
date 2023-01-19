#pragma once
#include "MaskBase.h"

class MaskPolygon: public MaskBase
{
public:
	bool MouseDown(CPoint pos, const Rigid& trans);  // stop or continue
	bool MouseUp(CPoint pos, const Rigid& trans);    // stop or continue
	bool MouseMove(CPoint pos, const Rigid& trans);  // stop or continue
	bool MouseDbClk(CPoint pos, const Rigid& trans); // stop or continue
	void Render(Gdiplus::Graphics& graphic, const Rigid& imgToView);
	void RenderToMask(Gdiplus::Graphics& graphic);
	bool IsValid() const { return m_Points.size() > 1; }

	MaskPolygon(): MaskPolygon(1) {}
	MaskPolygon(int lineSize);
};

