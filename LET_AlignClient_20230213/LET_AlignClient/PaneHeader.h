#pragma once
#include "afxwin.h"

// CPaneHeader �� ���Դϴ�.
class CPaneHeader : public CFormView
{
	DECLARE_DYNCREATE(CPaneHeader)

protected:
	CPaneHeader();           // ���� ����⿡ ���Ǵ� protected �������Դϴ�.
	virtual ~CPaneHeader();

public:
	enum { IDD = IDD_PANE_HEADER };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	CLET_AlignClientDlg *m_pMain;

	CRect m_rcForm;
	HBRUSH m_hbrBkg;

	CLabelEx m_LbNowTime;
	CLabelEx m_LbProgramTitle;
	CLabelEx m_LbProgramVersion;
	CLabelEx m_LbConnectServer;
	CLabelEx m_lblPlcConnected;
	CLabelEx m_LbConnectCamera[MAX_CAMERA];
	CLabelEx m_lblTrigger[4];
	CLabelEx m_lbLight[4];
	CLabelEx m_lbEquipTitle; // hsj 2023-01-02 Equipment Name �߰�

	virtual void OnInitialUpdate();

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);	
	afx_msg LRESULT OnViewControl(WPARAM wParam, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnStnDblclickStaticProgramTitle();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);

	void OnUpdateTime();
	void InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color);

	BOOL IsClickHeaderTitle(CPoint pt);
	BOOL IsClickServerTitle(CPoint pt);
	BOOL IsClickVersionTitle(CPoint pt);
	BOOL IsClickPLCTitle(CPoint pt);
};


