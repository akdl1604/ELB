#pragma once


// CSheetCaliperResult ��ȭ �����Դϴ�.
#include "UI_Control\XListCtrl.h"

class CCaliper;
class CDlgCaliper;
class CSheetCaliperResult: public CDialogEx
{
	DECLARE_DYNAMIC(CSheetCaliperResult)

public:
	CSheetCaliperResult(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CSheetCaliperResult();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_SHEET_CALIPER_RESULT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:	
	BOOL m_bCtrlPush;
	CCaliper *m_pCaliper;
	CDlgCaliper *m_pDlgCaliper;
	CXListCtrl m_listCaliper;
	void updateCaliperList();
	void saveCaliperList();
	void InitList(CXListCtrl &list);
	void ResizeListColumn(CXListCtrl &list, const int *arrWidth);
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
