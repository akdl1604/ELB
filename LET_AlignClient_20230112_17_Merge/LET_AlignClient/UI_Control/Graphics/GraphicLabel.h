#pragma once
#include <gdiplus.h>
#include <string>
#include <opencv.hpp>
#include "GraphicBase.h"
#include <functional>

/*
* - Lincoln Lee
* - x.perfect.vn@gmail.com
* - 2022-08-08
*/

enum class Anchor
{
	TopLeft,
	TopRight,
	TopCenter,
	Middle,
	BottomLeft,
	BottomRight,
	BottomCenter,
	XCenterMiddleTop,
	XCenterMiddleBottom,
	YCenterMiddleTop,
	YCenterMiddleBottom
};

struct GraphicLabel: public GraphicBase
{
public:
	float x = 0;
	float y = 0;

public:
	GraphicLabel() :
		GraphicLabel(0, 0, CString("Label1"), DEF_BG_COLOR, DEF_FG_COLOR, Anchor::BottomRight, true)
	{}

	GraphicLabel(float _x, float _y, CString strContent) :
		GraphicLabel(_x, _y, strContent, DEF_BG_COLOR, DEF_FG_COLOR, Anchor::BottomRight, true)
	{
	}

	GraphicLabel(float _x, float _y, CString strContent, Anchor anchor) :
		GraphicLabel(_x, _y, strContent, DEF_BG_COLOR, DEF_FG_COLOR, anchor, true)
	{
	}

	GraphicLabel(float _x, float _y, CString strContent, Gdiplus::Color bg, Gdiplus::Color fg) :
		GraphicLabel(_x, _y, strContent, bg, fg, Anchor::BottomRight)
	{
	}

	GraphicLabel(Gdiplus::Point loc, CString strContent, Gdiplus::Color bg, Gdiplus::Color fg, Anchor anchor = Anchor::BottomRight) :
		GraphicLabel(loc.X, loc.Y, strContent, bg, fg, anchor)
	{
	}

	GraphicLabel(Gdiplus::PointF loc, CString strContent, Gdiplus::Color bg, Gdiplus::Color fg, Anchor anchor = Anchor::BottomRight) :
		GraphicLabel(loc.X, loc.Y, strContent, bg, fg, anchor)
	{
	}

	GraphicLabel(float _x, float _y, CString strContent, Gdiplus::Color fg, bool withBG = true) :
		GraphicLabel(_x, _y, strContent, DEF_BG_COLOR, fg, Anchor::BottomRight, withBG)
	{
	}

	GraphicLabel(float _x, float _y, CString strContent, Gdiplus::Color fg, Anchor anchor, bool withBG = true) :
		GraphicLabel(_x, _y, strContent, DEF_BG_COLOR, fg, anchor, withBG)
	{
	}

	GraphicLabel(Gdiplus::Point loc, CString strContent, Gdiplus::Color fg, Anchor anchor = Anchor::BottomRight, bool withBG = true) :
		GraphicLabel(loc.X, loc.Y, strContent, DEF_BG_COLOR, fg, anchor, withBG)
	{
	}

	GraphicLabel(Gdiplus::PointF loc, CString strContent, Gdiplus::Color fg, Anchor anchor = Anchor::BottomRight, bool withBG = true) :
		GraphicLabel(loc.X, loc.Y, strContent, DEF_BG_COLOR, fg, anchor, withBG)
	{
	}

	template<class T0>
	GraphicLabel(cv::Point_<T0> loc, CStringW strContent, Gdiplus::Color fg, Anchor anchor = Anchor::BottomRight) :
		GraphicLabel(loc, strContent, DEF_BG_COLOR, fg, anchor, true)
	{
	}

	GraphicLabel(float _x, float _y, CString strContent, Gdiplus::Color bg, Gdiplus::Color fg, Anchor anchor = Anchor::BottomRight, bool withBG = true) :
		x(_x), y(_y), m_Content(strContent), m_Anchor(anchor),
		m_bgBrush(new Gdiplus::SolidBrush(bg)), m_fgBrush(new Gdiplus::SolidBrush(fg)), m_WithBackground(withBG), m_Font(GraphicLabel::g_Font)
	{
	}

	template<class T0>
	GraphicLabel(cv::Point_<T0> loc, CStringW strContent, Gdiplus::Color bg, Gdiplus::Color fg, Anchor anchor = Anchor::BottomRight, bool withBG = true) :
		x(loc.x), y(loc.y), m_Content(strContent), m_Anchor(anchor),
		m_bgBrush(new Gdiplus::SolidBrush(bg)), m_fgBrush(new Gdiplus::SolidBrush(fg)), m_WithBackground(withBG), m_Font(GraphicLabel::g_Font)
	{
	}

	GraphicLabel& SetDisplayLocation(const cv::Point2f loc)
	{
		x = loc.x;
		y = loc.y;
		return *this;
	}

	GraphicLabel& SetDisplayLocation(float _x, float _y)
	{
		x = _x;
		y = _y;
		return *this;
	}

	GraphicLabel& SetDisplayLocation(CPoint pt)
	{
		x = pt.x;
		y = pt.y;
		return *this;
	}

	GraphicLabel& SetWithBackground(bool withBg)
	{
		m_WithBackground = withBg;
		return *this;
	}

	GraphicLabel& SetFont(Gdiplus::Font* fnt)
	{
		m_Font = fnt;
		return *this;
	}

	GraphicLabel& SetTextColor(const CString str)
	{
		m_Content = str;
		return *this;
	}

	GraphicLabel& SetTextColor(const Gdiplus::Color color)
	{
		m_fgBrush->SetColor(color);
		return *this;
	}

	GraphicLabel& SetBgColor(const Gdiplus::Color color)
	{
		m_bgBrush->SetColor(color);
		return *this;
	}

	Gdiplus::Color GetBgColor() const
	{
		Gdiplus::Color color;
		m_bgBrush->GetColor(&color);
		return color;
	}

	Gdiplus::Color GetFgColor() const
	{
		Gdiplus::Color color;
		m_fgBrush->GetColor(&color);
		return color;
	}

	GraphicLabel& SetText(const CString str)
	{
		m_Content = str;
		return *this;
	}

	GraphicLabel& SetText(const CStringW str)
	{
		m_Content = str;
		return *this;
	}

	Gdiplus::SizeF GetSize() const
	{
		Gdiplus::Graphics g(GetDC(NULL));
		const auto* str = m_Content.GetString();
		const auto len = lstrlenW(str);

		Gdiplus::RectF rec;
		g.MeasureString(str, len, m_Font, Gdiplus::PointF(0, 0), &rec);
		return Gdiplus::SizeF(rec.Width, rec.Height);
	}

	GraphicLabel& SetAnchor(Anchor anchor)
	{
		m_Anchor = anchor;
		return *this;
	}

	virtual GraphicLabel& Render(Gdiplus::Graphics& graph, const Rigid& trans)
	{
		Gdiplus::RectF rect;
		auto loc = trans.Transform(Gdiplus::PointF(x, y));
		const auto* str = m_Content.GetString();
		const auto len = lstrlenW(str);

		graph.MeasureString(str, len, m_Font, loc, &rect);

		m_CalRect[int(m_Anchor)](rect, 0, 0);

		auto chk = graph.Save();
		graph.SetSmoothingMode(Gdiplus::SmoothingModeNone);
		if (m_WithBackground)
			graph.FillRectangle(m_bgBrush, rect);

		graph.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
		graph.DrawString(str, len, m_Font, rect, NULL, m_fgBrush);
		graph.Restore(chk);

		return *this;
	}

	virtual GraphicLabel& Render(Gdiplus::Graphics& graph, const Rigid& trans, CSize maxSize)
	{
		Gdiplus::RectF rect;
		auto xx = x < 0 ? maxSize.cx + x: x;
		auto yy = y < 0 ? maxSize.cy + y: y;
		auto w2 = maxSize.cx / 2.f;
		auto h2 = maxSize.cy / 2.f;

		auto loc = trans.Transform(Gdiplus::PointF(xx, yy));
		const auto* str = m_Content.GetString();
		const auto len = lstrlenW(str);

		graph.MeasureString(str, len, m_Font, loc, &rect);

		m_CalRect[int(m_Anchor)](rect, w2, h2);

		auto chk = graph.Save();
		graph.SetSmoothingMode(Gdiplus::SmoothingModeNone);
		if (m_WithBackground)
			graph.FillRectangle(m_bgBrush, rect);

		graph.SetSmoothingMode(Gdiplus::SmoothingModeAntiAlias);
		graph.DrawString(str, len, m_Font, rect, NULL, m_fgBrush);
		graph.Restore(chk);

		return *this;
	}

	bool GetWithBackground() const { return m_WithBackground; }

	static void SetDefaultFont(Gdiplus::Font* fnt)
	{
		if (NULL != g_Font) delete g_Font;
		g_Font = fnt;
	}

	static Gdiplus::Font* GetDefaultFont() { return g_Font; }

	static void Init()
	{
		// YCS 2022-11-30 폰트 사이즈 변경 13.5f -> 16.5f
		//g_Font = new Gdiplus::Font(CStringW("Consolas"), 13.5f, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
		g_Font = new Gdiplus::Font(CStringW("Consolas"), 16.5f, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
	}

	~GraphicLabel()
	{
		if (NULL != m_Font && m_Font != GraphicLabel::g_Font) delete m_Font;
		delete m_bgBrush;
		delete m_fgBrush;
		m_CalRect.clear();
	}
protected:


protected:
	static Gdiplus::Font* g_Font;

private:
	Anchor m_Anchor = Anchor::BottomRight;
	bool m_WithBackground = true;

	Gdiplus::Font* m_Font;
	Gdiplus::SolidBrush* m_bgBrush = NULL;
	Gdiplus::SolidBrush* m_fgBrush = NULL;

	CStringW m_Content;

	std::vector<std::function<void(Gdiplus::RectF&, float, float)>> m_CalRect
	{
		[&](Gdiplus::RectF& rect, float w2, float h2) { rect.X -= rect.Width; rect.Y -= rect.Height; },
		[&](Gdiplus::RectF& rect, float w2, float h2) { rect.Y -= rect.Height; },
		[&](Gdiplus::RectF& rect, float w2, float h2) { rect.X -= (rect.Width / 2.f); rect.Y -= rect.Height; },
		[&](Gdiplus::RectF& rect, float w2, float h2) { rect.Y -= (rect.Height / 2.f); rect.X -= (rect.Width / 2.); },
		[&](Gdiplus::RectF& rect, float w2, float h2) { rect.X -= rect.Width; },
		[&](Gdiplus::RectF& rect, float w2, float h2) {},
		[&](Gdiplus::RectF& rect, float w2, float h2) { rect.X -= (rect.Width / 2.f); },
		[&](Gdiplus::RectF& rect, float w2, float h2) { rect.X = (w2 - (rect.Width / 2.f)); rect.Y -= rect.Height; },
		[&](Gdiplus::RectF& rect, float w2, float h2) { rect.X = (w2 - (rect.Width / 2.f)); },
		[&](Gdiplus::RectF& rect, float w2, float h2) { rect.Y = h2 - rect.Height; },
		[&](Gdiplus::RectF& rect, float w2, float h2) { rect.Y = h2; }
	};
};
