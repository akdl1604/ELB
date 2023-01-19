#pragma once

#include "LabelEx.h"
// CCamLightControlDlg 대화 상자

class CCamLightControlDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CCamLightControlDlg)

public:
	CCamLightControlDlg(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CCamLightControlDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_CAMLIGHT_CONTROL };
#endif

	HBRUSH m_hbrBkg;
	CLET_AlignClientDlg* m_pMain;

	CLabelEx m_lblLightChannel;
	CLabelEx m_lblLightIndex;
	CLabelEx m_lblExposureTime;

	CComboBox m_cmbLightSelect;
	CComboBox m_cmbLightIndex;
	CComboBox m_cbUserSet;

	CEdit m_edtExposureTime;	
	CEdit m_edtLightValue;

	CSliderCtrl m_sliderExposureTime;
	CSliderCtrl m_sliderLightSetting;

	BOOL m_bExposureTimeChange[MAX_CAMERA];

private:
	int m_nJobID;
	int m_nCam;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();

	afx_msg void OnCbnSelchangeCbSelectLightChannel();
	afx_msg void OnCbnSelchangeCbLightIndex();
	afx_msg void OnEnSetfocusEditLightSetting();
	afx_msg void OnEnSetfocusEditExposureSetting();
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	void InitTitle(CLabelEx* pTitle, CString str, float size, COLORREF color);
	void updateFrameDialog(int nJob,int nCam);
};
