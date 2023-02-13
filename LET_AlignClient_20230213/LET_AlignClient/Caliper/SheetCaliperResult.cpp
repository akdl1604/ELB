// SheetCaliperResult.cpp: 구현 파일입니다.
//

#include "stdafx.h"
#include "..\LET_AlignClient.h"
#include "SheetCaliperResult.h"
#include "afxdialogex.h"
#include "..\LET_AlignClientDlg.h"
#include "Caliper\Caliper.h"

// CSheetCaliperResult 대화 상자입니다.

IMPLEMENT_DYNAMIC(CSheetCaliperResult, CDialogEx)

CSheetCaliperResult::CSheetCaliperResult(CWnd* pParent /*=NULL*/)
	: CDialogEx(CSheetCaliperResult::IDD, pParent)
{
	m_pDlgCaliper = NULL;
	m_pCaliper = NULL;
}

CSheetCaliperResult::~CSheetCaliperResult()
{
}

void CSheetCaliperResult::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_CALIPER, m_listCaliper);
}


BEGIN_MESSAGE_MAP(CSheetCaliperResult, CDialogEx)
END_MESSAGE_MAP()


// CSheetCaliperResult 메시지 처리기입니다.
void CSheetCaliperResult::updateCaliperList()
{
	CString str;
	m_listCaliper.DeleteAllItems();	
	
	for(int nItem = 0; nItem < m_pCaliper->getNumOfCalipers(); nItem++)
	{
		m_listCaliper.InsertItem(nItem, _T("0"));

		str.Format("%d", nItem + 1);
		m_listCaliper.SetItemText(nItem, 0, str, COLOR_BLACK, COLOR_WHITE);
		m_listCaliper.SetCheckbox(nItem, 1, m_pCaliper->getUseCaliper(nItem));
		
		if( m_pCaliper->getFindInfo(nItem).dist == -1 )
		{
			str.Format("-");
			m_listCaliper.SetItemText(nItem, 2, str);

			str.Format("-");
			m_listCaliper.SetItemText(nItem, 3, str);

			str.Format("-");
			m_listCaliper.SetItemText(nItem, 4, str);

			str.Format("-");
			m_listCaliper.SetItemText(nItem, 5, str);
		}
		else
		{
			str.Format("%.3f", m_pCaliper->getFindInfo(nItem).x);
			m_listCaliper.SetItemText(nItem, 2, str);

			str.Format("%.3f", m_pCaliper->getFindInfo(nItem).y);
			m_listCaliper.SetItemText(nItem, 3, str);

			str.Format("%.3f", m_pCaliper->getFindInfo(nItem).dist);
			m_listCaliper.SetItemText(nItem, 4, str);

			str.Format("%d", m_pCaliper->getFindInfo(nItem).contrast);
			m_listCaliper.SetItemText(nItem, 5, str);
		}
	}
}

void CSheetCaliperResult::saveCaliperList()
{
	for (int i = 0; i < m_pCaliper->getNumOfCalipers(); i++)
	{
		m_pCaliper->setUseCaliper(i, m_listCaliper.GetCheckbox(i, 1));		
	}
}

void CSheetCaliperResult::InitList(CXListCtrl &list)
{
	const TCHAR *lpszHeader[] = {	_T("No."),			// 0
		_T("Use"),			// 1
		_T("X"),		// 2
		_T("Y"),			// 3
		_T("Distance"),			// 4
		_T("Contrast"),			// 4
		NULL};	

	const int colWidthList[] = {	45,					// 0
		60,					// 1
		80,					// 2
		80,					// 3
		80,					// 4
		80,					// 5
		0
	};				

	LV_COLUMN lvcolumn;
	memset(&lvcolumn, 0, sizeof(lvcolumn));
	lvcolumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT /*| LVCF_WIDTH | LVCF_IMAGE*/;
	for( int i=0; ; i++ )
	{
		if( lpszHeader[i] == NULL )
			break;

		lvcolumn.fmt = LVCFMT_CENTER;
		lvcolumn.pszText = (TCHAR *)lpszHeader[i];
		lvcolumn.iSubItem = i;
		list.InsertColumn(i, &lvcolumn);	
		list.SetColumnWidth(i, colWidthList[i]);
	}	

	lvcolumn.pszText = (TCHAR *)lpszHeader[0];
	lvcolumn.iSubItem = 0;
	list.SetColumn(0, &lvcolumn);
	list.SetExtendedStyle(LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT/*| LVS_EX_GRIDLINES | LVS_EX_FLATSB | LVS_EX_SUBITEMIMAGES*/);
	list.EnableToolTips(TRUE);

	// 리스트 아이템 높이 설정
	list.SetRowHeight(20);
}


void CSheetCaliperResult::ResizeListColumn(CXListCtrl &list, const int *arrWidth)
{
	SCROLLINFO si;
	CRect rc;
	list.GetWindowRect(&rc);
	int nMargin = 0;

	// 스크롤 유무에 따른 마진 추가
	list.GetScrollInfo(SB_VERT, &si);	
	if( si.nPage && si.nPage <= (UINT)si.nMax ) 
	{
		nMargin = ::GetSystemMetrics(SM_CXVSCROLL);
	}

	// 보더 유무에 따른 마진 추가
	DWORD dwExStyle = list.GetExStyle();
	if( dwExStyle&WS_EX_STATICEDGE || 
		dwExStyle&WS_EX_CLIENTEDGE )
	{
		nMargin += ::GetSystemMetrics(SM_CXEDGE)*2;
	}

	int nWidth = 0;
	int nLastWidth = rc.Width() - nMargin;
	for( int i = 0; ; i++ )
	{
		// 마지막 컬럼일 경우
		if( arrWidth[i] == 0 )
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


BOOL CSheetCaliperResult::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	if( m_pDlgCaliper != NULL )	m_pCaliper = &m_pDlgCaliper->m_Caliper[0][0][0];
	m_bCtrlPush = FALSE;



	return TRUE;  // return TRUE unless you set the focus to a control
	// 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}

BOOL CSheetCaliperResult::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.

	if( pMsg->message == WM_KEYDOWN )
	{
		if( pMsg->wParam == VK_CONTROL ) m_bCtrlPush = TRUE;

		// 컨트롤 + A 누를 경우 전체 선택 되도록..
		if( m_bCtrlPush && (pMsg->wParam == 'A') )
		{
			for(int i = 0; i < m_pCaliper->getNumOfCalipers(); i++)
				m_listCaliper.SetItemState(i, LVIS_SELECTED, 0xFFFFFFFF);
		}

		// 스페이스 눌렀을 경우 현재 선택되어져 있는 행을 기준으로
		// Check가 많으면 Uncheck
		// Uncheck가 많으면 Check 되도록..
		if( pMsg->wParam == VK_SPACE )
		{
			int nState[MAX_CALIPER] = {0, };
			int nValue = 0;
			for(int i = 0; i < m_pCaliper->getNumOfCalipers(); i++)
			{
				nState[i] = m_listCaliper.GetItemState(i, LVIS_SELECTED);
				if( nState[i] != 0 )
				{
					if( m_listCaliper.GetCheckbox(i, 1) )	nValue++;
					else									nValue--;
				}
			}

			for(int i = 0; i < m_pCaliper->getNumOfCalipers(); i++)
			{
				if( nState[i] != 0 )
				{
					if( nValue > 0 )	m_listCaliper.SetCheckbox(i, 1, FALSE);
					else				m_listCaliper.SetCheckbox(i, 1, TRUE);
				}
			}
		}
	}
	else if( pMsg->message == WM_KEYUP)
	{
		if( pMsg->wParam == VK_CONTROL ) m_bCtrlPush = FALSE;
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}

