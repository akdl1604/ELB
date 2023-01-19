#pragma once
// CDlgMessage 대화 상자입니다.
#include "UI_Control\ButtonEx.h"
#include "UI_Control\LabelEx.h"

class CDlgMessage : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgMessage)

public:
	CDlgMessage(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgMessage();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DLG_MESSAGE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
private:
	CString m_strText1;
	CString m_strText2;
	int m_nType;
	int m_nResult;
	CButtonEx m_btnOK;
	CButtonEx m_btnCancel;
	CLabelEx m_lbText1;
	CLabelEx m_lbText2;
	HBRUSH m_hbrBkg;
	void MainButtonInit(CButtonEx *pbutton,int size = 15);
	void InitTitle(CLabelEx *pTitle,CString str,float size,COLORREF color);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
public:	
	BOOL IsOK() { return (m_nResult == 1) ? TRUE : FALSE; };
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnMessageOk();
	afx_msg void OnBnClickedBtnMessageCancel();
	void setType(int nType) { m_nType = nType; }
	void setMessage(CString strText1, CString strText2 = _T("")) { m_strText1 = strText1; m_strText2 = strText2;}
	void setResult(int nResult) { m_nResult = nResult; }
	int getResult() { return m_nResult; }
	void setMessageText(CString strText1, CString strText2 = _T("")) { m_lbText1.SetText(strText1); m_lbText2.SetText(strText2); }
	void setMessageModaless(int nType, CString strText1, CString strText2 = _T(""));
};

