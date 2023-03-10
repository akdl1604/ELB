#pragma once
//----------------------------------------------------------
// EditEx Control
//----------------------------------------------------------
// Programmed by William Kim
//----------------------------------------------------------
// Last Update : 2016-09-02 14:48
//----------------------------------------------------------

// CEditEx
class CEditEx : public CEdit
{
	DECLARE_DYNAMIC(CEditEx)

public:
	CEditEx();
	virtual ~CEditEx();

public :
	void SetEnable(bool bEnable) { m_bEnable = bEnable; EnableWindow(m_bEnable); Invalidate(); }

	void SetAlignRight() { ModifyStyle(ES_LEFT | ES_CENTER,ES_RIGHT); Invalidate(); }
	void SetAlignLeft()  { ModifyStyle(ES_RIGHT | ES_CENTER,ES_LEFT); Invalidate(); }
	void SetAlignCenter(){ ModifyStyle(ES_LEFT | ES_RIGHT,ES_CENTER); Invalidate(); }

	void SetText(CString strText) { SetWindowText(strText); Invalidate(); }
	void SetTextInt(int nValue) { CString tmp=_T(""); tmp.Format(_T("%d"), nValue); SetWindowText(tmp); Invalidate(); }
	void SetTextDouble(double dValue) { CString tmp=_T(""); tmp.Format(_T("%.3f"), dValue); SetWindowText(tmp); Invalidate(); }
	int GetTextInt() { CString tmp; GetWindowText(tmp); return _ttoi(tmp); }
	CString GetText() { CString tmp; GetWindowText(tmp); return tmp; }

	void SetColorBorder(COLORREF clrColor) { m_clrBorder = clrColor; Invalidate(); }
	void SetColorBkg(COLORREF clrColor);
	void SetColorText(COLORREF clrColor) { m_clrText = clrColor; Invalidate(); }
	void SetTextMargins(int nLeft, int nRight) { SetMargins(nLeft, nRight); Invalidate(); }
	
	void SetSizeText(int nSize);
	void SetStyleTextBold(bool bEnable);

private :
	bool m_bEnable;
	COLORREF m_clrBkg;
	COLORREF m_clrBorder;
	COLORREF m_clrText;
	HBRUSH m_hBrush;
	bool m_bBold;
	int m_nSizeText;
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg HBRUSH CtlColor(CDC* pDC, UINT nCtlColor);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnEnUpdate();
	afx_msg void OnNcPaint();
};


