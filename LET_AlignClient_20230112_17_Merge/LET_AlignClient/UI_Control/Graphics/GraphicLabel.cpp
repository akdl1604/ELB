#include "stdafx.h"
#include "GraphicLabel.h"

Gdiplus::Font* GraphicLabel::g_Font = new Gdiplus::Font(CStringW("Consolas"), 14, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint);