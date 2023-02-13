#pragma once


// CDlgAlarm ��ȭ �����Դϴ�.

class CDlgAlarm : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgAlarm)

public:
	CDlgAlarm(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CDlgAlarm();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_DLG_ALARM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

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
