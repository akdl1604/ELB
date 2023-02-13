#if !defined(AFX_CDSLIDERCTRLDIAMOND_H__83C2A7B8_42D1_47AE_B2B7_51D5DDFAFEC0__INCLUDED_)
#define AFX_CDSLIDERCTRLDIAMOND_H__83C2A7B8_42D1_47AE_B2B7_51D5DDFAFEC0__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// CDSliderCtrl.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CCDSliderCtrlDiamond window

class CCDSliderCtrlDiamond : public CSliderCtrl
{
// Construction
public:
	CCDSliderCtrlDiamond();
	void SetPrimaryColor(COLORREF cr);


// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCDSliderCtrlDiamond)
	//}}AFX_VIRTUAL

// Implementation
public:
	virtual ~CCDSliderCtrlDiamond();

	// Generated message map functions
protected:
	COLORREF m_crPrimary;
	COLORREF m_crShadow;
	COLORREF m_crHilite;

	CBrush m_normalBrush;
	CBrush m_focusBrush;
	//{{AFX_MSG(CCDSliderCtrlDiamond)
	afx_msg void OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult);
	//}}AFX_MSG

	DECLARE_MESSAGE_MAP()
};

/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CDSLIDERCTRLDIAMOND_H__83C2A7B8_42D1_47AE_B2B7_51D5DDFAFEC0__INCLUDED_)
