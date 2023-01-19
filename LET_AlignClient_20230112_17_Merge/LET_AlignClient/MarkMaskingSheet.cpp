// MarkMaskingSheet.cpp: 구현 파일입니다.
//

#include "stdafx.h"
#include "LET_AlignClient.h"
#include "MarkMaskingSheet.h"
#include "afxdialogex.h"

#include "DOGU.h"
#include "MaskFillMode.h"
#include "PenShape.h"
#include "EllipseShape.h"
#include "RectangleShape.h"
#include "PolygonShape.h"


const	TCHAR* lpszHeaderTitle[] = { _T("Use"),			// 0
								_T("Start"),			// 1
								_T("End"),		// 2
								_T("Shape"),	// 3
								NULL };

const int colWidthList[] = { 20,					// 0
								26,					// 1
								26,					// 2
								25 };				// 4

IMPLEMENT_DYNAMIC(CMarkMaskingSheet, CDialogEx)

CMarkMaskingSheet::CMarkMaskingSheet(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMarkMaskingSheet::IDD, pParent)
{
	m_pParentDlg = NULL;
	m_btSelectMode = DOGU::DPEN;
	m_lineColor = RGB(255, 0, 0);
	m_lineColorXor = (RGB(GetRValue(m_lineColor) ^ 255, GetGValue(m_lineColor) ^ 255, GetBValue(m_lineColor) ^ 255));
	m_nLineWidth = 1;
	m_nlineCnt = 0;
	m_bDrawMode = FALSE;
	m_nLineWidth = m_SizeDefault;
}

CMarkMaskingSheet::~CMarkMaskingSheet()
{
}

void CMarkMaskingSheet::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMarkMaskingSheet, CDialogEx)
	//ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, &CMarkMaskingSheet::OnLvnItemchangedList)
	ON_COMMAND(ID_DOGU_PEN, &CMarkMaskingSheet::OnDoguPen)
	ON_COMMAND(ID_DOGU_LINE, &CMarkMaskingSheet::OnDoguLine)
	ON_COMMAND(ID_DOGU_ELLIPSE, &CMarkMaskingSheet::OnDoguEllipse)
	ON_COMMAND(ID_DOGU_RECT, &CMarkMaskingSheet::OnDoguRect)
	ON_COMMAND(ID_DOGU_ERASE, &CMarkMaskingSheet::OnDoguErase)
	ON_COMMAND(ID_PEN_1, &CMarkMaskingSheet::OnWidth1)
	ON_COMMAND(ID_PEN_2, &CMarkMaskingSheet::OnWidth2)
	ON_COMMAND(ID_PEN_3, &CMarkMaskingSheet::OnWidth3)
	ON_COMMAND(ID_PEN_4, &CMarkMaskingSheet::OnWidth5)
	ON_COMMAND(ID_CROSS, &CMarkMaskingSheet::OnBrush)
END_MESSAGE_MAP()

BOOL CMarkMaskingSheet::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	InitToolBar();

	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

void CMarkMaskingSheet::InitList(CXListCtrl& list, const TCHAR** pszTitle)
{
	LV_COLUMN lvcolumn;
	memset(&lvcolumn, 0, sizeof(lvcolumn));
	lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT /*| LVCF_WIDTH | LVCF_IMAGE*/;
	for (int i = 0; ; i++)
	{
		if (pszTitle[i] == NULL)
			break;

		lvcolumn.fmt = LVCFMT_LEFT;
		lvcolumn.pszText = (TCHAR*)pszTitle[i];
		lvcolumn.iSubItem = i;
		list.InsertColumn(i, &lvcolumn);
	}

	lvcolumn.pszText = (TCHAR*)pszTitle[0];
	lvcolumn.iSubItem = 0;
	list.SetColumn(0, &lvcolumn);
	list.SetExtendedStyle(LVS_EX_FULLROWSELECT/*| LVS_EX_GRIDLINES | LVS_EX_FLATSB | LVS_EX_SUBITEMIMAGES*/);
	list.EnableToolTips(TRUE);

	// 리스트 아이템 높이 설정
	list.SetRowHeight(20);
}

void CMarkMaskingSheet::ResizeListColumn(CXListCtrl& list, const int* arrWidth)
{
	SCROLLINFO si;
	CRect rc;
	list.GetWindowRect(&rc);
	int nMargin = 0;

	// 스크롤 유무에 따른 마진 추가
	list.GetScrollInfo(SB_VERT, &si);
	if (si.nPage && si.nPage <= (UINT)si.nMax)
	{
		nMargin = ::GetSystemMetrics(SM_CXVSCROLL);
	}

	// 보더 유무에 따른 마진 추가
	DWORD dwExStyle = list.GetExStyle();
	if (dwExStyle & WS_EX_STATICEDGE ||
		dwExStyle & WS_EX_CLIENTEDGE)
	{
		nMargin += ::GetSystemMetrics(SM_CXEDGE) * 2;
	}

	int nWidth = 0;
	int nLastWidth = rc.Width() - nMargin;
	for (int i = 0; ; i++)
	{
		// 마지막 컬럼일 경우
		if (arrWidth[i] == 0)
		{
			list.SetColumnWidth(i, nLastWidth);
			break;
		}

		nWidth = (rc.Width() * arrWidth[i]) / 100;
		list.SetColumnWidth(i, nWidth);
		nLastWidth -= nWidth;
	}
	list.RedrawWindow();
}

void CMarkMaskingSheet::UseMask()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString str;
	long tmp;

	for (int i = 0; i < 4; i++)
	{
		if (m_pParentDlg->m_Viewer.IsDrawFigure(i + 1))
		{
			CPoint ptStart = m_pParentDlg->m_Viewer.GetPointBegin(i + 1);
			CPoint ptEnd = m_pParentDlg->m_Viewer.GetPointEnd(i + 1);
			if (ptStart.x > ptEnd.x)
			{
				tmp = ptStart.x;
				ptStart.x = ptEnd.x;
				ptEnd.x = tmp;
				if (ptStart.x < 0) ptStart.x = 0;
				if (ptEnd.x >= m_pParentDlg->m_nWidth * m_pParentDlg->m_dSize) ptEnd.x = LONG(m_pParentDlg->m_nWidth * m_pParentDlg->m_dSize - 1);
			}
			if (ptStart.y > ptEnd.y)
			{
				tmp = ptStart.y;
				ptStart.y = ptEnd.y;
				ptEnd.y = tmp;
				if (ptStart.y < 0) ptStart.y = 0;
				if (ptEnd.y >= m_pParentDlg->m_nHeight * m_pParentDlg->m_dSize) ptEnd.y = LONG(m_pParentDlg->m_nHeight * m_pParentDlg->m_dSize - 1);
			}
			m_pParentDlg->m_crMaskRect[i] = CRect(int(ptStart.x / m_pParentDlg->m_dSize), int(ptStart.y / m_pParentDlg->m_dSize), int(ptEnd.x / m_pParentDlg->m_dSize), int(ptEnd.y / m_pParentDlg->m_dSize));

			/*str.Format("%d,%d",ptStart.x,ptStart.y);
			m_list.SetItemText(i, 1, str, RGB(242,149,48), RGB(255,255,255));
			str.Format("%d,%d",ptEnd.x,ptEnd.y);
			m_list.SetItemText(i, 2, str, RGB(242,149,48), RGB(255,255,255));	*/
		}
	}

}

void CMarkMaskingSheet::UseMaskSave()
{
}

void CMarkMaskingSheet::InitToolBar()
{
	if (!m_wndToolBar.CreateEx(this, TBSTYLE_FLAT, WS_CHILD | WS_VISIBLE | CBRS_TOP
		| CBRS_TOOLTIPS | CBRS_FLYBY | CBRS_SIZE_DYNAMIC | CBRS_GRIPPER | CBRS_BORDER_ANY)
		|| !m_wndToolBar.LoadToolBar(IDR_DOGUBAR))
	{
		EndDialog(IDCANCEL);
	}

	CRect rcClientStart;
	CRect rcClientNow;

	GetClientRect(rcClientStart);
	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0, reposQuery, rcClientNow);

	CPoint ptOffset(rcClientNow.left - rcClientStart.left, rcClientNow.top - rcClientStart.top);
	CRect rcChild;
	CWnd* pwndChild = GetWindow(GW_CHILD);
	while (pwndChild)
	{
		pwndChild->GetWindowRect(rcChild);
		ScreenToClient(rcChild);
		rcChild.OffsetRect(ptOffset);
		pwndChild->MoveWindow(rcChild, FALSE);
		pwndChild = pwndChild->GetNextWindow();
	}
	CRect rcWindow;
	GetWindowRect(rcWindow);
	rcWindow.right += rcClientStart.Width() - rcClientNow.Width();
	rcWindow.bottom += rcClientStart.Height() - rcClientNow.Height();
	MoveWindow(rcWindow, FALSE);

	RepositionBars(AFX_IDW_CONTROLBAR_FIRST, AFX_IDW_CONTROLBAR_LAST, 0);
}

void CMarkMaskingSheet::OnDoguPen()
{
	m_pParentDlg->GetMaskDrawer().DoShape(DOGU::DPEN, m_nLineWidth);
	m_pParentDlg->NotifyUpdate();
}

void CMarkMaskingSheet::OnDoguLine()
{
	m_pParentDlg->GetMaskDrawer().DoShape(DOGU::D_POLYGON, m_nLineWidth);
	m_pParentDlg->NotifyUpdate();
}

void CMarkMaskingSheet::OnDoguEllipse()
{
	m_pParentDlg->GetMaskDrawer().DoShape(DOGU::DELLIPSE, m_nLineWidth);
	m_pParentDlg->NotifyUpdate();
}

void CMarkMaskingSheet::OnDoguRect()
{
	m_pParentDlg->GetMaskDrawer().DoShape(DOGU::DRECT, m_nLineWidth);
	m_pParentDlg->NotifyUpdate();
}

void CMarkMaskingSheet::OnDoguErase()
{
	m_FillMode = (MaskFillMode::ADD == m_FillMode) ? MaskFillMode::SUBTRACT: MaskFillMode::ADD;

	m_pParentDlg->GetMaskDrawer().SetFillMode(m_FillMode);
	m_pParentDlg->NotifyUpdate();
}

void CMarkMaskingSheet::OnWidth1()
{
	m_pParentDlg->GetMaskDrawer().ClearShapeObjects();
	m_pParentDlg->NotifyUpdate();
}

void CMarkMaskingSheet::OnWidth2()
{

	m_pParentDlg->GetMaskDrawer().RemoveLastItem();
	m_pParentDlg->NotifyUpdate();
}

void CMarkMaskingSheet::OnWidth3()
{
	m_nLineWidth = MIN(m_SizeMax, MAX(m_nLineWidth - m_SizeInterval, m_SizeMin));

	m_pParentDlg->GetMaskDrawer().SetPenWidth(m_nLineWidth);
	m_pParentDlg->NotifyUpdate();
}

void CMarkMaskingSheet::OnWidth5()
{
	m_nLineWidth = MIN(m_SizeMax, MAX(m_nLineWidth + m_SizeInterval, m_SizeMin));

	m_pParentDlg->GetMaskDrawer().SetPenWidth(m_nLineWidth);
	m_pParentDlg->NotifyUpdate();
}

void CMarkMaskingSheet::OnBrush()
{
	m_pParentDlg->GetMaskDrawer().SetPickPointMode(!m_pParentDlg->GetMaskDrawer().IsPickPointMode());
	m_pParentDlg->NotifyUpdate();
}