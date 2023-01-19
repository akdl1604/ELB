#include "stdafx.h"
#include "CircleProgress.h"

#define  PI 3.1415926535897932384626433832795028

IMPLEMENT_DYNAMIC(CCircleProgress, CStatic)

CCircleProgress::CCircleProgress()
{
	GdiplusStartupInput gdiplusStartupInput;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

	m_colors = NULL;
	m_percent = 0;
	m_penWidth = 0;
	m_centerPoint = 0;
	m_elementCount = 0;
	m_insideRadius = 0;
	m_outsideRadius = 0;
	m_rect = 0;
	m_offSet = 0;
	m_strPercent = "0%";
}

CCircleProgress::~CCircleProgress()
{
	GdiplusShutdown(gdiplusToken);
	if (m_colors) delete m_colors;
}


BEGIN_MESSAGE_MAP(CCircleProgress, CStatic)
	ON_WM_PAINT()
	ON_WM_TIMER()
	ON_WM_ERASEBKGND()
	ON_WM_CTLCOLOR_REFLECT()
END_MESSAGE_MAP()

HBRUSH CCircleProgress::CtlColor(CDC* pDC, UINT nCtlColor)
{
	pDC->SetBkMode(TRANSPARENT);
	
	return (HBRUSH)GetStockObject(NULL_BRUSH);
}

BOOL CCircleProgress::OnEraseBkgnd(CDC* pDC)
{
	return FALSE;
}
void CCircleProgress::OnPaint()
{
	CPaintDC dc(this); // device context for painting

	CDC memdc;
	memdc.CreateCompatibleDC(NULL);
	CBitmap memBitmap;
	memBitmap.CreateCompatibleBitmap(&dc, m_rect.Width(), m_rect.Height());
	memdc.SelectObject(memBitmap);
	memdc.FillSolidRect(CRect(0, 0, m_rect.Width(), m_rect.Height()), RGB(240, 240, 240));

	Graphics graphics(memdc);
	graphics.SetSmoothingMode(SmoothingModeDefault); //Anti-aliasing when stroked

	BYTE byIncrement = (BYTE)(255 / 12);
	BYTE iPercent = 255;
	double angle = (double)360 / m_elementCount;

	Pen initPen(Darken(GetPixel(dc, 1, 1), 255), m_penWidth);
	initPen.SetStartCap(LineCapRound); //Set the line cap style
	initPen.SetEndCap(LineCapRound);
	for (int iCounter = 0; iCounter < m_elementCount; iCounter++)
	{
		Point startP, endP;
		startP.X = m_centerPoint.x + m_insideRadius * (float)cos((PI * (1 + iCounter) * angle) / 180);
		startP.Y = m_centerPoint.y + m_insideRadius * (float)sin((PI * (1 + iCounter) * angle) / 180);
		endP.X = m_centerPoint.x + m_outsideRadius * (float)cos((PI * (1 + iCounter) * angle) / 180);
		endP.Y = m_centerPoint.y + m_outsideRadius * (float)sin((PI * (1 + iCounter) * angle) / 180);

		graphics.DrawLine(&initPen, startP, endP);
	}

	for (int iCounter = 0; iCounter < m_elementCount; iCounter++)
	{
		Point startP, endP;
		startP.X = m_centerPoint.x + m_insideRadius * (float)cos((PI * (1 + iCounter) * angle) / 180);
		startP.Y = m_centerPoint.y + m_insideRadius * (float)sin((PI * (1 + iCounter) * angle) / 180);
		endP.X = m_centerPoint.x + m_outsideRadius * (float)cos((PI * (1 + iCounter) * angle) / 180);
		endP.Y = m_centerPoint.y + m_outsideRadius * (float)sin((PI * (1 + iCounter) * angle) / 180);
		int index = (iCounter + m_offSet) % m_elementCount;
		Pen pen(m_colors[index], m_penWidth);
		pen.SetStartCap(LineCapRound); //Set the line cap style
		pen.SetEndCap(LineCapRound);
		graphics.DrawLine(&pen, startP, endP);
	}

	//Draw percentage
	Gdiplus::FontFamily fontFamily(L"Times new roman");
	Gdiplus::Font font(&fontFamily, 40, FontStyleRegular, UnitPixel);
	StringFormat stringformat;
	stringformat.SetAlignment(StringAlignmentCenter);
	stringformat.SetLineAlignment(StringAlignmentCenter);

	SolidBrush brush(Color(255, 0x66, 0xcc, 0x66));
	//LinearGradientBrush brush(Point(10, 0), Point(600, 0), Color(0, 255, 0), Color(0, 0, 255));
	graphics.DrawString(towstring(m_strPercent).c_str(), -1, &font, RectF(0, 0, m_rect.Width(), 100), &stringformat, &brush);

	TransparentBlt(dc.m_hDC, 0, 0, m_rect.Width(), m_rect.Height(), memdc.m_hDC, 0, 0, m_rect.Width(), m_rect.Height(), RGB(240, 240, 240));

	memBitmap.DeleteObject();
	ReleaseDC(&memdc);
	DeleteDC(memdc);
}

void CCircleProgress::Init(CWnd* parentWnd, int nCount, int elementWidth, CRect rect, COLORREF color)
{
	m_rect = rect;
	m_penWidth = elementWidth;
	m_centerPoint.SetPoint(rect.Width() / 2, rect.Height() / 2);
	m_elementCount = nCount;
	m_colors = new Color[nCount];
	SetColors(color);
	RECT rc;
	rc.top = rect.top; rc.bottom = rect.bottom; rc.left = rect.left; rc.right = rect.right;

	Create("", SS_OWNERDRAW | WS_CHILDWINDOW, rc, parentWnd);
	ModifyStyle(0, WS_CLIPCHILDREN );
	ShowWindow(SW_SHOWNOACTIVATE);	
}

void CCircleProgress::SetPercent(int percent)
{
	m_percent = percent;
	m_strPercent.Format("%d%%", percent);
	if (percent == 100)
	{
		StopProgress();
	}
	else
	{
		RedrawWindow();
	}

}
void CCircleProgress::SetColors(COLORREF color)
{
	BYTE byteIncrement = BYTE(255 / m_elementCount);

	Color argb;
	argb.SetFromCOLORREF(color);
	for (int i = 0; i < m_elementCount; i++)
	{
		m_colors[i] = Darken(argb, byteIncrement * (i + 1));
	}

}

Color CCircleProgress::Darken(Color color, int percent)
{
	return Color(percent, color.GetR(), color.GetG(), color.GetB());
}

void CCircleProgress::SetCircleAppearance(int insideRadius, int outsideRadius)
{
	m_insideRadius = insideRadius;
	m_outsideRadius = outsideRadius;
	SetWindowPos(this, m_rect.left, m_rect.top, outsideRadius * 2 + 26, outsideRadius * 2 + 26, SWP_NOZORDER);
	m_rect = CRect(m_rect.left, m_rect.top, m_rect.left + outsideRadius * 2 + 26, m_rect.top + outsideRadius * 2 + 26);
	m_centerPoint = CPoint(m_rect.Width() / 2, m_rect.Height() / 2);
	StopProgress();
	Invalidate(TRUE);
}

void CCircleProgress::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 1)
	{
		m_offSet = (m_offSet + 1) % m_elementCount;
		m_strPercent.Format("%d%%", m_percent++);
		Invalidate(FALSE);
	}

	CStatic::OnTimer(nIDEvent);
}

void CCircleProgress::StartProgress()
{
	m_percent=0;
	SetTimer(1, 20, NULL);

	MoveWindow(m_rect.left, m_rect.top, m_rect.Width(), m_rect.Height());
	//20210910 Tkyuha 최상위 윈도우 만들기
	GetTopMostWnd();
}
void CCircleProgress::StopProgress()
{
	KillTimer(1);
	MoveWindow(-1000, 0, m_rect.Width(), m_rect.Height());
}

//20210910 Tkyuha 최상위 윈도우 만들기
void CCircleProgress::GetTopMostWnd()
{
	if (::GetForegroundWindow() != this->m_hWnd) {
		HWND h_active_wnd = ::GetForegroundWindow();
		if (h_active_wnd != NULL) {
			DWORD thread_id = GetWindowThreadProcessId(h_active_wnd, NULL);
			DWORD current_thread_id = GetCurrentThreadId();
			if (current_thread_id != thread_id) {
				if (AttachThreadInput(current_thread_id, thread_id, TRUE)) {
					BringWindowToTop();
					AttachThreadInput(current_thread_id, thread_id, FALSE);
				}
			}
		}
	}
}