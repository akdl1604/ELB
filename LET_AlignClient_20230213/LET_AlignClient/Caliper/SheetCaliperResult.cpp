// SheetCaliperResult.cpp: ���� �����Դϴ�.
//

#include "stdafx.h"
#include "..\LET_AlignClient.h"
#include "SheetCaliperResult.h"
#include "afxdialogex.h"
#include "..\LET_AlignClientDlg.h"
#include "Caliper\Caliper.h"

// CSheetCaliperResult ��ȭ �����Դϴ�.

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


// CSheetCaliperResult �޽��� ó�����Դϴ�.
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

	// ����Ʈ ������ ���� ����
	list.SetRowHeight(20);
}


void CSheetCaliperResult::ResizeListColumn(CXListCtrl &list, const int *arrWidth)
{
	SCROLLINFO si;
	CRect rc;
	list.GetWindowRect(&rc);
	int nMargin = 0;

	// ��ũ�� ������ ���� ���� �߰�
	list.GetScrollInfo(SB_VERT, &si);	
	if( si.nPage && si.nPage <= (UINT)si.nMax ) 
	{
		nMargin = ::GetSystemMetrics(SM_CXVSCROLL);
	}

	// ���� ������ ���� ���� �߰�
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
		// ������ �÷��� ���
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

	// TODO:  ���⿡ �߰� �ʱ�ȭ �۾��� �߰��մϴ�.
	if( m_pDlgCaliper != NULL )	m_pCaliper = &m_pDlgCaliper->m_Caliper[0][0][0];
	m_bCtrlPush = FALSE;



	return TRUE;  // return TRUE unless you set the focus to a control
	// ����: OCX �Ӽ� �������� FALSE�� ��ȯ�ؾ� �մϴ�.
}

BOOL CSheetCaliperResult::PreTranslateMessage(MSG* pMsg)
{
	// TODO: ���⿡ Ư��ȭ�� �ڵ带 �߰� ��/�Ǵ� �⺻ Ŭ������ ȣ���մϴ�.

	if( pMsg->message == WM_KEYDOWN )
	{
		if( pMsg->wParam == VK_CONTROL ) m_bCtrlPush = TRUE;

		// ��Ʈ�� + A ���� ��� ��ü ���� �ǵ���..
		if( m_bCtrlPush && (pMsg->wParam == 'A') )
		{
			for(int i = 0; i < m_pCaliper->getNumOfCalipers(); i++)
				m_listCaliper.SetItemState(i, LVIS_SELECTED, 0xFFFFFFFF);
		}

		// �����̽� ������ ��� ���� ���õǾ��� �ִ� ���� ��������
		// Check�� ������ Uncheck
		// Uncheck�� ������ Check �ǵ���..
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

