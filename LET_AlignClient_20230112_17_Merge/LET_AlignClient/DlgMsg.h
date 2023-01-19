#pragma once
#include "afxwin.h"
#include "GradientStatic.h"

// CDlgMsg ��ȭ �����Դϴ�.
class CDlgMsg : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgMsg)

public:
	CDlgMsg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CDlgMsg();

	CFont m_Font;
	CString m_sMessage;
	int m_nBlinking;
	COLORREF m_colorText;
	COLORREF m_colorBk;

// ��ȭ ���� �������Դϴ�.
enum { IDD = IDD_DIALOG_MSG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CGradientStatic m_stcMessage;
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnBnClickedButtonOk();
};
