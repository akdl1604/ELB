#include "stdafx.h"
#include "TabCtrlEx.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CTabCtrlEx

CTabCtrlEx::CTabCtrlEx()
{
	m_crSelColour = RGB(0, 0, 255);
	m_crUnselColour = RGB(50, 50, 50);
}

CTabCtrlEx::~CTabCtrlEx()
{
	m_SelFont.DeleteObject();
	m_UnselFont.DeleteObject();
}

BEGIN_MESSAGE_MAP(CTabCtrlEx, CTabCtrl)
	//{{AFX_MSG_MAP(CTabCtrlEx)
	ON_WM_CREATE()
	//}}AFX_MSG_MAP
	ON_WM_ERASEBKGND()
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTabCtrlEx message handlers

int CTabCtrlEx::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CTabCtrl::OnCreate(lpCreateStruct) == -1) return -1;
	ModifyStyle(0, TCS_OWNERDRAWFIXED);

	return 0;
}

void CTabCtrlEx::PreSubclassWindow()
{
	CTabCtrl::PreSubclassWindow();

	ModifyStyle(0, TCS_OWNERDRAWFIXED);
}

void CTabCtrlEx::DrawItem(LPDRAWITEMSTRUCT lpDrawItemStruct)
{
	COLORREF m_select_border_color = RGB(255, 87, 51);
	COLORREF m_select_text_color = RGB(255, 255, 255);
	COLORREF m_unselect_border_color = RGB(195, 155, 211);
	COLORREF m_unselect_text_color = RGB(255, 255, 255);


	int select_index = lpDrawItemStruct->itemID;
	if (select_index < 0) return;
	BOOL bSelected = (select_index == GetCurSel());

	CDC *pDC = CDC::FromHandle(lpDrawItemStruct->hDC);
	CRect rect = lpDrawItemStruct->rcItem;

	// Tab이 그려진 테두리의 두께만큼 위치를 보정한다.
	rect.top += ::GetSystemMetrics(SM_CYEDGE);

	pDC->SetBkMode(TRANSPARENT);

	char tab_text[40];

	TC_ITEM data;
	data.mask = TCIF_TEXT | TCIF_IMAGE;
	data.pszText = tab_text;
	data.cchTextMax = 39;

	// 탭이 선택된 정보에 따라 배경색을 칠해준다.
	if (select_index == GetCurSel()) pDC->FillSolidRect(rect, m_select_border_color);
	else pDC->FillSolidRect(rect, m_unselect_border_color);

	// 선택된 탭의 정보를 얻는다.
	if (!GetItem(select_index, &data)) return;

	// 이미지를 출력한다.
	CImageList *p_image_list = GetImageList();
	if (p_image_list != NULL && data.iImage >= 0) {
		rect.left += pDC->GetTextExtent(" ").cx;

		IMAGEINFO image_info;
		p_image_list->GetImageInfo(data.iImage, &image_info);
		CRect image_rect(image_info.rcImage);

		p_image_list->Draw(pDC, data.iImage, CPoint(rect.left, rect.top), ILD_TRANSPARENT);
		rect.left += image_rect.Width();
	}

	CFont *p_old_font = NULL;
	if (bSelected == 1) p_old_font = pDC->SelectObject(&m_SelFont);
	else p_old_font = pDC->SelectObject(GetFont());

	if (select_index == GetCurSel()) {
		// 선택된 탭이라면...
		pDC->SetTextColor(m_select_text_color);
		// 텍스트의 위치를 보정하여 선택된 느낌이 강조되도록 만든다.
		rect.top -= ::GetSystemMetrics(SM_CYEDGE);
		pDC->DrawText(tab_text, rect, DT_SINGLELINE | DT_VCENTER | DT_CENTER);
	}
	else {
		// 선택되지 않은 탭이라면...
		pDC->SetTextColor(m_unselect_text_color);
		pDC->DrawText(tab_text, rect, DT_SINGLELINE | DT_BOTTOM | DT_CENTER);
	}

	pDC->SelectObject(p_old_font);
}

/////////////////////////////////////////////////////////////////////////////
// CTabCtrlEx operations

void CTabCtrlEx::SetColours(COLORREF bSelColour, COLORREF bUnselColour)
{
	m_crSelColour = bSelColour;
	m_crUnselColour = bUnselColour;
	Invalidate();
}

void CTabCtrlEx::SetFonts(CFont* pSelFont, CFont* pUnselFont)
{
	ASSERT(pSelFont && pUnselFont);

	LOGFONT lFont;
	int nSelHeight, nUnselHeight;

	m_SelFont.DeleteObject();
	m_UnselFont.DeleteObject();

	pSelFont->GetLogFont(&lFont);
	m_SelFont.CreateFontIndirect(&lFont);
	nSelHeight = lFont.lfHeight;

	pUnselFont->GetLogFont(&lFont);
	m_UnselFont.CreateFontIndirect(&lFont);
	nUnselHeight = lFont.lfHeight;

	SetFont((nSelHeight > nUnselHeight) ? &m_SelFont : &m_UnselFont);
}


void CTabCtrlEx::SetFonts(int nSelWeight, BOOL bSelItalic, BOOL bSelUnderline,
	int nUnselWeight, BOOL bUnselItalic, BOOL bUnselUnderline)
{
	// Free any memory currently used by the fonts.
	m_SelFont.DeleteObject();
	m_UnselFont.DeleteObject();

	// Get the current font
	LOGFONT lFont;
	CFont *pFont = GetFont();
	if (pFont)
		pFont->GetLogFont(&lFont);
	else {
		NONCLIENTMETRICS ncm;
		ncm.cbSize = sizeof(NONCLIENTMETRICS);
		VERIFY(SystemParametersInfo(SPI_GETNONCLIENTMETRICS,
			sizeof(NONCLIENTMETRICS), &ncm, 0));
		lFont = ncm.lfMessageFont;
	}

	// Create the "Selected" font
	lFont.lfWeight = nSelWeight;
	lFont.lfItalic = bSelItalic;
	lFont.lfUnderline = bSelUnderline;
	m_SelFont.CreateFontIndirect(&lFont);

	// Create the "Unselected" font
	lFont.lfWeight = nUnselWeight;
	lFont.lfItalic = bUnselItalic;
	lFont.lfUnderline = bUnselUnderline;
	m_UnselFont.CreateFontIndirect(&lFont);

	SetFont((nSelWeight > nUnselWeight) ? &m_SelFont : &m_UnselFont);
}

BOOL CTabCtrlEx::OnEraseBkgnd(CDC* pDC)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	CRect rect; 
	GetClientRect(&rect); 

	CBrush myBrush;
	myBrush.CreateStockObject(NULL_BRUSH);

	CBrush *pOld = pDC->SelectObject(&myBrush);
	pDC->FillSolidRect(rect, 0);
	BOOL bRes = pDC->PatBlt(0, 0, rect.Width(), rect.Height(), PATCOPY); 
	pDC->SelectObject(pOld); // restore old brush 
	myBrush.DeleteObject();

	return bRes; // return CTabCtrl::OnEraseBkgnd(pDC);
}
