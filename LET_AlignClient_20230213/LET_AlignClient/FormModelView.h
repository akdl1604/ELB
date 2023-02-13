#pragma once
#include "afxwin.h"

#include "EditEx.h"
#include "CheckEx.h"
#include "ListCtrlStyled.h"

#include "GroupEx.h"
#include "TabCtrlEx.h"
#include "TabModelAlgoPage.h"

// CFormModelView 폼 뷰입니다.
class CLET_AlignClientDlg;

class CFormModelView : public CFormView
{
	DECLARE_DYNCREATE(CFormModelView)

protected:
	CFormModelView();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~CFormModelView();

public:
	enum { IDD = IDD_FORM_MODEL };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

	CWnd *c_pWnd;
	CLET_AlignClientDlg *m_pMain;

	TabModelAlgoPage *c_TabModelPage[MAX_CAMERA];
	int m_nOldTabSelect;

	CTabCtrlEx m_hSelectAlgoTab;
	CListCtrlStyled m_ListModelID;
	HBRUSH m_hbrBkg;

	CGroupEx m_grpCalibration;
	CGroupEx m_grpGlass;

	CLabelEx m_LbModelViewTitle;
	CLabelEx m_stt_ModelTitle;
	CLabelEx  m_stt_GlassWidth;
	CLabelEx  m_stt_GlassHeight;
	CLabelEx  m_stt_FrameWidth;
	CLabelEx  m_stt_FrameHeight;
	CLabelEx  m_stt_ModelPanelType;
	CLabelEx  m_stt_FMarkPitch;
	CLabelEx  m_lblSecondGrab;
	CLabelEx  m_stt_CalibrationCountX;
	CLabelEx  m_stt_CalibrationCountY;
	CLabelEx  m_stt_CalibrationRangeX;
	CLabelEx  m_stt_CalibrationRangeY;
	CLabelEx  m_stt_RotateRange;
	CLabelEx  m_stt_RotateCount;
	CLabelEx  m_lblCalibrationType;

	CButtonEx m_btnDataSave;
	CButtonEx m_btnDataModify;
	CButtonEx m_btnModelChange;

	CEditEx m_edt_ModelTitle;
	CEditEx m_edt_GlassWidth;
	CEditEx m_edt_GlassHeight;
	CEditEx m_edt_FrameWidth;
	CEditEx m_edt_FrameHeight;
	CEditEx m_edt_FMarkPitch;
	CEditEx m_edtSecondMarkPitchY;
	CEditEx m_edt_CalibrationCountX;
	CEditEx m_edt_CalibrationCountY;
	CEditEx m_edt_CalibrationRangeX;
	CEditEx m_edt_CalibrationRangeY;
	CEditEx m_edt_RotateRange;
	CEditEx m_edt_RotateCount;
	CEditEx m_edt_RotateStartPos;
	CEditEx m_edt_RotateEndPos;

	int m_nJobID;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual void OnInitialUpdate();

	afx_msg void OnSelchangeTabList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnViewControl(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedBtnModelDataModify();
	afx_msg void OnBnClickedBtnModelDataSave();
	afx_msg void OnBnClickedBtnModelChange();
	afx_msg void OnEnSetfocusEditCalibCountX();
	afx_msg void OnEnSetfocusEditCalibCountY();
	afx_msg void OnEnSetfocusEditCalibRangeX();
	afx_msg void OnEnSetfocusEditCalibRangeY();
	afx_msg void OnEnSetfocusEditRotateRange();
	afx_msg void OnEnSetfocusEditRotateCount();
	afx_msg void OnEnSetfocusEditRotateStartPos();
	afx_msg void OnEnSetfocusEditRotateEndPos();
	afx_msg void OnEnSetfocusEditGlassWidth();
	afx_msg void OnEnSetfocusEditGlassHeight();
	afx_msg void OnEnSetfocusEditFrameWidth();
	afx_msg void OnEnSetfocusEditFrameHeight();
	afx_msg void OnEnSetfocusEditMarkPitchX();
	afx_msg void OnEnSetfocusEditInspGrabSecond();
	afx_msg void OnNMClickListModel(NMHDR *pNMHDR, LRESULT *pResult);

	void InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color);
	void MainButtonInit(CButtonEx *pbutton, int size = 15);
	void EditButtonInit(CEditEx *pbutton, int size, COLORREF color = RGB(255, 0, 0));
	void CreateGrid();
	void initListCtrl();
	BOOL getModelList();
	BOOL deleteModelData(CString strPath);
	void enableEditControls(BOOL bEnable);	
	void setEditBox(int nID);
	void showModelInfo(CString strModel);
	void showGlassInfo(CString strModel);	
	void showMachineInfo(CString strModel);
	void showAlignInfo(CString strModel);
	void showInspSpecPara(CString strModel);		// dh.jung 2021-08-02 add 
	void updateAlignInfoData();
	void applyModelData();
	void EditGroupInit(CGroupEx *pGroup, CString str, float size, COLORREF color);
	virtual BOOL PreTranslateMessage(MSG* pMsg);
};


