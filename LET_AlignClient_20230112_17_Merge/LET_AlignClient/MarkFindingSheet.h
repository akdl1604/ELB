#pragma once
#include "afxwin.h"


// CMarkFindingSheet 대화 상자입니다.
class CMaskMarkViewDlg;
class CMarkFindingSheet : public CDialogEx
{
	DECLARE_DYNAMIC(CMarkFindingSheet)

public:
	CMarkFindingSheet(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CMarkFindingSheet();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DIALOG_FINDING };

	CMaskMarkViewDlg *m_pParentDlg;

	BOOL IsClickNumLevelTitle(CPoint pt);
	BOOL IsClickSAngleTitle(CPoint pt);
	BOOL IsClickEAngleTitle(CPoint pt);
	BOOL IsClickMinscoreTitle(CPoint pt);
	BOOL IsClickGreednesslTitle(CPoint pt);
	BOOL IsClickScaleTitle(CPoint pt);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	int m_edtNumMatch;
	double m_edtSangle;
	double m_edtEangle;
	double m_edtMinScore;
	double m_edtGreedness;
	double m_edtNumScale;
	CComboBox m_cmbSubPixel;
	CComboBox m_cmbMethod;
	virtual BOOL OnInitDialog();
	afx_msg void OnStnClickedEditNumlevel();
	afx_msg void OnStnClickedEditSAngle();
	afx_msg void OnStnClickedEditEAngle();
	afx_msg void OnStnClickedEditMinscore();
	afx_msg void OnStnClickedEditContrast();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnStnClickedEditScale();
};
