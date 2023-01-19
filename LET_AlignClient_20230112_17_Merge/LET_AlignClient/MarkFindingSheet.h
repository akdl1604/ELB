#pragma once
#include "afxwin.h"


// CMarkFindingSheet ��ȭ �����Դϴ�.
class CMaskMarkViewDlg;
class CMarkFindingSheet : public CDialogEx
{
	DECLARE_DYNAMIC(CMarkFindingSheet)

public:
	CMarkFindingSheet(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CMarkFindingSheet();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_DIALOG_FINDING };

	CMaskMarkViewDlg *m_pParentDlg;

	BOOL IsClickNumLevelTitle(CPoint pt);
	BOOL IsClickSAngleTitle(CPoint pt);
	BOOL IsClickEAngleTitle(CPoint pt);
	BOOL IsClickMinscoreTitle(CPoint pt);
	BOOL IsClickGreednesslTitle(CPoint pt);
	BOOL IsClickScaleTitle(CPoint pt);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

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
