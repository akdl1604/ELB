#if !defined(AFX_SIMPLESPLASHWND_H__BC470DEA_FB74_425A_B860_34C9D4CAF6E8__INCLUDED_)
#define AFX_SIMPLESPLASHWND_H__BC470DEA_FB74_425A_B860_34C9D4CAF6E8__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// SimpleSplashWnd.h : header file
//
#define		SCALE_X		1
#define		SCALE_Y		1

class CSimpleSplashWnd : public CWnd
{
	CBitmap m_bitmap;
	CSize m_sizeBitmap;
	CString m_sStatusText;

// Construction
public:
	CSimpleSplashWnd();
	CSimpleSplashWnd(CWnd * pWndParent, UINT nBitmapID);

// Attributes
public:
	CFont m_Font;
	CFont m_FontTitle;
// Operations
public:
	bool Create(CWnd * pWndParent, UINT nBitmapID);

	static bool RegisterSplashWndClass();
	static void PassMsgLoop(bool bEnableOnIdleCalls);
	void SetStatusText(LPCTSTR sStatusText, int nProgress = 1);

private:
	int		m_nProgress;
	void __initControls();


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSimpleSplashWnd)
	public:
	virtual BOOL DestroyWindow();
	protected:
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CSimpleSplashWnd();

	// Generated message map functions
protected:
	//{{AFX_MSG(CSimpleSplashWnd)
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnPaint();
	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnClose();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
}; // class CSimpleSplashWnd

#define GL_VIEWS_SPLASHWND_WNDCLASS _T("GLViewsSplashWnd")

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SIMPLESPLASHWND_H__BC470DEA_FB74_425A_B860_34C9D4CAF6E8__INCLUDED_)
