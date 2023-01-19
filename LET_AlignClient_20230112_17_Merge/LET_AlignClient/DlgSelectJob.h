#pragma once

#include "UI_Control\LabelEx.h"

class CLET_AlignClientDlg;
class CDlgSelectJob : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgSelectJob)

public:
	CDlgSelectJob(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CDlgSelectJob();

	// 대화 상자 데이터입니다.
	enum { IDD = IDD_DLG_SELECT_JOB};

protected:

	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	CLabelEx  m_lbSelect;
	CButtonEx m_btnSelect;
	CListCtrlStyled m_ListResultJob;

	CComboBox m_cmbJob;
	CLET_AlignClientDlg* m_pMain;

	HBRUSH m_hbrBkg;

	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	void MainButtonInit(CButtonEx* pbutton, int size = 15);
	void InitTitle(CLabelEx* pTitle, CString str, float size, COLORREF color);

	afx_msg void OnBnClickedBtnSelect();
};
