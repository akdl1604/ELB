#pragma once


// CSheetCaliperResult 대화 상자입니다.
#include "UI_Control\XListCtrl.h"

class CCaliper;
class CDlgCaliper;
class CSheetCaliperResult: public CDialogEx
{
	DECLARE_DYNAMIC(CSheetCaliperResult)

public:
	CSheetCaliperResult(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CSheetCaliperResult();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_SHEET_CALIPER_RESULT };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

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
