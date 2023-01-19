#pragma once

#include "MaskMarkViewDlg.h"
#include "afxwin.h"
#include "UI_Control\CDSliderCtrlDiamond.h"
#include "UI_Control\CheckEx.h"

class CMaskMarkViewDlg;

class CMarkSearchTesterSheet : public CDialogEx
{
	DECLARE_DYNAMIC(CMarkSearchTesterSheet)

public:
	CMarkSearchTesterSheet(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CMarkSearchTesterSheet();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DIALOG_SEARCH_TESTER};

	CMaskMarkViewDlg *m_pParentDlg;

	CCheckEx m_bEdgeView;
	CCDSliderCtrlDiamond m_ctrlLowThresh;
	CCDSliderCtrlDiamond m_ctrlHighThresh;

	int m_edtSimMincontrast;
	int m_edtSimContrast;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	
	afx_msg void OnBnClickedCheckEdgeView();
	afx_msg void OnBnClickedButtonGetparam();
	afx_msg void OnBnClickedButtonSearch();
	afx_msg void OnBnClickedButtonApply();
	
	// Lincoln Lee - 220221
	virtual void ClearShapeModel();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
