#pragma once

#include "UI_Control\BBungGrid.h"
// CGridInspectionParamDlg 대화 상자

class CGridInspectionParamDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CGridInspectionParamDlg)

public:
	CGridInspectionParamDlg(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CGridInspectionParamDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_SET_INSPPARA };
#endif
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedOk();
	afx_msg LRESULT OnPropertyChanged(WPARAM wparam, LPARAM lparam);
	afx_msg void OnClickedMfcpropertygridInspection();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

public:
	CLET_AlignClientDlg* m_pMain;
	CBBungGrid m_ctlPropGrid;
	CFont m_font;

	int m_nJob;
	BOOL bUse;
	HBRUSH m_hbrBkg;
public:
	void Init();
	void InitCenterCircle();
	void InitNozzleAlign();
	void InitZGapInsp();
	void InitFilmInspection();
	void InitViewSubInspection();
	void InitAttachAlign();
	void InitScratchInspection();
	// YCS 2022-11-19 필름얼라인 각도 비교 스펙 파라미터 분리
	void InitFilmAlign(); 
	void SaveCenterCircle();
	void SaveNozzleAlign();
	void SaveZGapInsp();
	void SaveFilmInspection();
	void SaveViewSubInspection();
	void SaveAttachAlign();
	void SaveScratchInspection();
	// YCS 2022-11-19 필름얼라인 각도 비교 스펙 파라미터 분리
	void SaveFilmAlign();
	void ChkMasterParam(int _USER_LEVEL);
	void SetMainParent(CLET_AlignClientDlg* pMain, int nJob = 0) 
	{
		m_pMain = pMain; 
		m_nJob	= nJob;
	}

	BOOL InItDialog();

	virtual BOOL PreTranslateMessage(MSG* pMsg);
};
