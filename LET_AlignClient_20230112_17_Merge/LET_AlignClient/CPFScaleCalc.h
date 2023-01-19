#pragma once

#include "LabelEx.h"
// CPFScaleCalc 대화 상자

class CPFScaleCalc : public CDialogEx
{
	DECLARE_DYNAMIC(CPFScaleCalc)

public:
	CPFScaleCalc(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CPFScaleCalc();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_PF_SCALE_CALC };
#endif

	HBRUSH m_hbrBkg;
	CLET_AlignClientDlg* m_pMain;

	CLabelEx m_stt_Insp_LX;
	CLabelEx m_stt_Insp_LY;
	CLabelEx m_stt_Insp_RX;
	CLabelEx m_stt_Insp_RY;

	CLabelEx m_stt_Scope_LX;
	CLabelEx m_stt_Scope_LY;
	CLabelEx m_stt_Scope_RX;
	CLabelEx m_stt_Scope_RY;

	CLabelEx m_stt_Scale_LX;
	CLabelEx m_stt_Scale_LY;
	CLabelEx m_stt_Scale_RX;
	CLabelEx m_stt_Scale_RY;

	CEditEx	 m_edt_Insp_LX;
	CEditEx	 m_edt_Insp_LY;
	CEditEx	 m_edt_Insp_RX;
	CEditEx	 m_edt_Insp_RY;

	CEditEx	 m_edt_Scope_LX;
	CEditEx	 m_edt_Scope_LY;
	CEditEx	 m_edt_Scope_RX;
	CEditEx	 m_edt_Scope_RY;

	CEditEx	 m_edt_Scale_LX;
	CEditEx	 m_edt_Scale_LY;
	CEditEx	 m_edt_Scale_RX;
	CEditEx	 m_edt_Scale_RY;

	CButtonEx  m_btnCalc;
	CButtonEx  m_btnSave;
	
	CCheckEx m_chkATTACH1;
	CCheckEx m_chkATTACH2;

	double Insp_LX;
	double Insp_LY;
	double Insp_RX;
	double Insp_RY;

	double Scope_LX;
	double Scope_LY;
	double Scope_RX;
	double Scope_RY;

	double Scale_LX;
	double Scale_LY;
	double Scale_RX;
	double Scale_RY;

	BOOL m_bCalcCheck;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:

	virtual BOOL OnInitDialog();

	afx_msg void OnBnClickedCalc();
	afx_msg void OnBnClickedSave();

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	void InitTitle(CLabelEx* pTitle, CString str, float size, COLORREF color);
	void MainButtonInit(CButtonEx* pbutton, int size = 15);
	void EditButtonInit(CEditEx* pbutton, int size, COLORREF color = RGB(255, 0, 0));


	afx_msg void OnEnSetfocusEditInspLx();
	afx_msg void OnEnSetfocusEditInspLy();
	afx_msg void OnEnSetfocusEditInspRx();
	afx_msg void OnEnSetfocusEditInspRy();
	afx_msg void OnEnSetfocusEditScopeLx();
	afx_msg void OnEnSetfocusEditScopeLy();
	afx_msg void OnEnSetfocusEditScopeRx();
	afx_msg void OnEnSetfocusEditScopeRy();
};
