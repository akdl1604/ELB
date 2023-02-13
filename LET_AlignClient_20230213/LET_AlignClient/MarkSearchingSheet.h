#pragma once

#include "MaskMarkViewDlg.h"
#include "afxwin.h"
// CMarkSearchingSheet 대화 상자입니다.

class CMaskMarkViewDlg;

class CMarkSearchingSheet : public CDialogEx
{
	DECLARE_DYNAMIC(CMarkSearchingSheet)

public:
	CMarkSearchingSheet(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CMarkSearchingSheet();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DIALOG_SEARCH };

	CMaskMarkViewDlg *m_pParentDlg;

	BOOL IsClickNumLevelTitle(CPoint pt);
	BOOL IsClickSangleTitle(CPoint pt);
	BOOL IsClickEangleTitle(CPoint pt);
	BOOL IsClickStepAngleTitle(CPoint pt);
	BOOL IsClickContrastTitle(CPoint pt);
	BOOL IsClickMinContrastTitle(CPoint pt);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	int m_edtNumLevel;
	double m_edtSangle;
	double m_edtEangle;
	double m_edtStepAngle;
	double m_edtContrast;
	double m_edtMinContrast;
	CComboBox m_cmbOptimize;
	CComboBox m_cmbMetric;
	virtual BOOL OnInitDialog();
	afx_msg void OnStnClickedEditNumlevel();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnStnClickedEditSAngle();
	afx_msg void OnStnClickedEditEAngle();
	afx_msg void OnStnClickedEditStepAngle();
	afx_msg void OnStnClickedEditContrast();
	afx_msg void OnStnClickedEditContrast2();
};
