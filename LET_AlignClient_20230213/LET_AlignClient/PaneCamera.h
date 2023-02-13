#pragma once
#include "afxwin.h"
#include "CheckEx.h"
#include "TabCtrlEx.h"
#include "TabCameraPage.h"
#include "TabInspPage.h"

#define _INSP_CREATE_TAB_RANGE 35 // 검사 탭 추가시 확인

// CPaneCamera 폼 뷰입니다.
class CPaneCamera : public CFormView
{
	DECLARE_DYNCREATE(CPaneCamera)

protected:
	CPaneCamera();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~CPaneCamera();

public:
	enum { IDD = IDD_PANE_CAMERA };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif
	CButtonEx m_btnCameraClose;
	CButtonEx m_btnPatternSave;

	CTabCtrlEx m_hSelectTab;
	TabCameraPage *c_TabCameraPage[MAX_CAMERA];
	TabInspPage  *c_TabInspPage[MAX_CAMERA];

	CWnd *c_pWnd;
	CLET_AlignClientDlg *m_pMain;
	HBRUSH m_hbrBkg;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
private:
public:
	int old_select_job;

	virtual void OnInitialUpdate();	

	afx_msg void OnSelchangeTabList(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedBtnPatternSave();
	afx_msg void OnBnClickedBtnCameraClose();

	void InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color);	
	void MainButtonInit(CButtonEx *pbutton, int size = 15);
	void CreateGrid();
	void UpdateFrameDlg();
	void CameraCaliperClose();
	void ReferenceImageSave();
};


