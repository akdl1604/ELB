#pragma once
#include "MaskMarkViewDlg.h"
#include "UI_Control\XListCtrl.h"
#include "DOGU.h"
#include "ShapeObject.h"
#include <gdiplus.h>
#include <cstring>

// CMarkMaskingSheet 대화 상자입니다.

class CMaskMarkViewDlg;

class CMarkMaskingSheet: public CDialogEx
{
	DECLARE_DYNAMIC(CMarkMaskingSheet)

public:
	CMarkMaskingSheet(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CMarkMaskingSheet();

	// 대화 상자 데이터입니다.
	enum { IDD = IDD_DIALOG_MASKING };

	//CXListCtrl m_list;
	CMaskMarkViewDlg* m_pParentDlg;
	CToolBar m_wndToolBar;

	COLORREF m_lineColor, m_lineColorXor;

	DOGU m_btSelectMode = DOGU::DPEN;
	int m_nLineWidth;
	int m_nlineCnt;
	BOOL m_bDrawMode;

	MaskFillMode m_FillMode = MaskFillMode::ADD;

	void InitList(CXListCtrl& list, const TCHAR** pszTitle);
	void ResizeListColumn(CXListCtrl& list, const int* arrWidth);
	void UseMask();
	void UseMaskSave();
	void InitToolBar();

	// Lincoln Lee - 220221
	const int m_SizeMin = 10;
	const int m_SizeMax = 80;
	const int m_SizeDefault = 40;
	const int m_SizeInterval = 5;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

	//afx_msg void OnLvnItemchangedList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDoguPen();
	afx_msg void OnDoguLine();
	afx_msg void OnDoguEllipse();
	afx_msg void OnDoguRect();
	afx_msg void OnDoguErase();
	afx_msg void OnWidth1(); // Clear
	afx_msg void OnWidth2(); // Undo
	afx_msg void OnWidth3(); // Minus Size
	afx_msg void OnWidth5(); // Plus Size
	afx_msg void OnBrush();
};
