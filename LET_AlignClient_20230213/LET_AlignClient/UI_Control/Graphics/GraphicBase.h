#pragma once
#include "LincUtils.h"
#include "Rigid.h"
#include <gdiplus.h>
#include <opencv.hpp>

/*
* - Lincoln Lee
* - x.perfect.vn@gmail.com
* - 2022-08-08
*/


/* Abstract class for all graphic object (included interative and non-interative graphic) */
class GraphicBase
{
public:
	// Reserve for checking whether this graphic present on Active window or not
	virtual CRect BoundingBox() const { return CRect(); }

	// using for Interative graphic
	virtual bool IsActive() const { return false; }
	virtual bool IsSkipMouseDown() const { return false; }
	virtual bool IsInside(const CRect& rect, const Rigid& trans) { return false; }
	virtual bool IsInterative() const { return false; } // Normaly non interative graphic
	virtual bool MouseDown(CPoint pos, const Rigid& trans) { return false; }  // need update graphic or not
	virtual bool MouseUp(CPoint pos, const Rigid& trans) { return false; }    // need update graphic or not
	virtual bool MouseMove(CPoint pos, const Rigid& trans) { return false; }  // need update graphic or not
	virtual bool MouseHover(CPoint pos, const Rigid& trans) { return false; } // need update graphic or not
	virtual bool MouseDbClk(CPoint pos, const Rigid& trans) { return false; } // need update graphic or not
	virtual bool IsRePosition() const { return false; }
	virtual GraphicBase& RePosition(CPoint st, CPoint ed, const Rigid& imgToView) { return *this; };

	virtual CRect GetSelectedRect() const { return CRect(); }
	virtual GraphicBase& GetBeginEnd(CPoint& st, CPoint& ed) { return *this; }
	virtual GraphicBase& SetBeginEnd(const CPoint& st, const CPoint& ed) { return *this; }

	virtual GraphicBase& SetActive(bool act = true) { return *this; }
	virtual GraphicBase& SetMetricTransform(const Rigid& rigid) { return *this; }

	GraphicBase(){}
	virtual GraphicBase& Render(Gdiplus::Graphics& graph, const Rigid& trans) { return *this; } //Soft layer rendering
	virtual GraphicBase& Render(Gdiplus::Graphics& graph, const Rigid& trans, CSize maxSize) { return *this; } // Hard layer rendering
	virtual ~GraphicBase() {}

protected:
private:
};