#pragma once

using namespace Gdiplus;

class CCircleProgress : public CStatic
{
	DECLARE_DYNAMIC(CCircleProgress)

public:
	CCircleProgress();
	virtual ~CCircleProgress();
public:
	ULONG_PTR gdiplusToken;
	Color* m_colors;
	int m_percent;
	int m_penWidth;
	CPoint m_centerPoint;
	int m_elementCount;
	int m_insideRadius;
	int m_outsideRadius;
	CRect m_rect;
	int m_offSet;
	CString m_strPercent;

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	//afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	void Init(CWnd* parentWnd, int nCount, int elementWidth, CRect rect = CRect(840, 420, 1080, 660),COLORREF color=RGB(0,0,255));
	void SetPercent(int percent);
	void SetColors(COLORREF color);
	Color Darken(Color color, int percent);
	void SetCircleAppearance(int insideRadius, int outsideRadius);
	void StartProgress();
	void StopProgress();
	void GetTopMostWnd(); //20210910 Tkyuha 최상위 윈도우 만들기
};