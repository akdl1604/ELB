#pragma once
#include "afxwin.h"
#include "GradientStatic.h"

// CDlgMsg 대화 상자입니다.
class CDlgMsg : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgMsg)

public:
	CDlgMsg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgMsg();

	CFont m_Font;
	CString m_sMessage;
	int m_nBlinking;
	COLORREF m_colorText;
	COLORREF m_colorBk;

// 대화 상자 데이터입니다.
enum { IDD = IDD_DIALOG_MSG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CGradientStatic m_stcMessage;
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnBnClickedButtonOk();
};
