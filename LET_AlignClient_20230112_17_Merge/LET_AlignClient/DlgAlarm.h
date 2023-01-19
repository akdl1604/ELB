#pragma once


// CDlgAlarm 대화 상자입니다.

class CDlgAlarm : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgAlarm)

public:
	CDlgAlarm(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgAlarm();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DLG_ALARM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	COLORREF m_crBackGround[5];
	int m_nLastIndex;
	int m_nCount;
	BOOL m_bIsUse[5];
	CString m_strTitle;
	CString m_strAlarmMessage;
	HBRUSH m_hbrBkg;
	CLabelEx m_LbAlarmMessage;
	CButtonEx m_BtnAlarmMessage;

	void MainButtonInit(CButtonEx *pbutton,int size = 15);
	void InitTitle(CLabelEx *pTitle,CString str,float size, COLORREF color);
	void showAlarmMessage(CString strMessage);

	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedBtnConfirm();
	afx_msg void OnClose();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
