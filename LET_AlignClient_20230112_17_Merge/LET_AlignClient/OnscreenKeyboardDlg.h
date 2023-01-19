#pragma once


struct KEYDEF
{
	char cNormal;
	char cShifted;
	int cWidth;
};

// COnscreenKeyboardDlg 대화 상자입니다.

class COnscreenKeyboardDlg : public CDialogEx
{
	DECLARE_DYNAMIC(COnscreenKeyboardDlg)

public:
	COnscreenKeyboardDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~COnscreenKeyboardDlg();

public:
	int pKeyWidth;
	int pKeyHeight;
	HWND gFocus;
	BOOL m_bCreateKeyBoard;

	CTypedPtrList<CPtrList, KEYDEF*> cKeys;

	void AddKey(char pNormal,char pShifted,	int pWidth);
	void CalcWidthHeight();
	void DrawKey(CDC * dc, CRect & rc, KEYDEF * key, BOOL cHilight=FALSE);
	void SendKey(KEYDEF * key);
	void ReleaseFocus();
	int DescribeKeyState();
	void DrawKeyboard();
	void InitKeyBoard();

	int cCurrentKeyState;
	HBRUSH m_hbrBkg;

// 대화 상자 데이터입니다.
	enum { IDD = IDD_ONSCREENKEYBOARD_DIALOG };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual BOOL OnInitDialog();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg int OnMouseActivate(CWnd* pDesktopWnd, UINT nHitTest, UINT message);
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnMove(int x, int y);
};
