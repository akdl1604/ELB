/////////////////////////////////////////////////////////////////////////////
// ProgressBar.h : header file
//
// MFC ProgressBar - ProgressBar class header file
//
// Written by Daniel Caldwell <dbc@acedev.net>
// Copyright (c) 2005-2015. All Rights Reserved.
//
// This code may be used in compiled form in any way you desire. This
// file may be redistributed unmodified by any means PROVIDING it is 
// not sold for profit without the authors written consent, and 
// providing that this notice and the authors name and all copyright 
// notices remains intact. 
//
// An email letting me know how you are using it would be nice as well. 
//
// This file is provided "as is" with no expressed or implied warranty.
// The author accepts no liability for any damage/loss of business that
// this product may cause.
//
//////////////////////////////////////////////////////////////////////



#ifndef __CONTROL_PROGRESSBAR_H_
#define __CONTROL_PROGRESSBAR_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// ProgressBar.h : header file
//

/////////////////////////////////////////////////////////////////////////////
// CProgressBar special messages

// float compatible version of PBM_SETSTEP
// IMPORTANT: WPARAM = valid pointer to a float variable, therefore you should only use this message in SendMessage to ensure there is no access violation
#define PBM_FLOAT_SETSTEP (WM_USER+4) // WPARAM must be a pointer to a valid float variable

// float compatible version of PBM_SETPOS
// IMPORTANT: WPARAM = valid pointer to a float variable, therefore you should only use this message in SendMessage to ensure there is no access violation
#define PBM_FLOAT_SETPOS (WM_USER+3) // WPARAM must be a pointer to a valid float variable


/////////////////////////////////////////////////////////////////////////////
// CProgressBar window

class CProgressBar : public CWnd
{
	DECLARE_DYNAMIC(CProgressBar)
// Construction
public:
	CProgressBar();

// Attributes
public:

// Operations
public:

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CProgressBar)
	//}}AFX_VIRTUAL

// Implementation
public:

	// NON-THREAD SAFE FUNCTIONS (use messages in order to be thread safe, see source code for implemented messages)

	// get step amount
	float GetStepAmount( ) const;

	// increment the position by the step amount
	void Step();

	// set step amount
	void SetStepAmount( float fStepAmount );

	// set the position
	void SetPosition( float fPos );

	// get the current position
	float GetPosition( ) const;

	// set range (maximum and minimum)
	void SetRange( UINT nStartPos, UINT nEndPos );

	void SetFontSize( int size );

	BOOL Create( DWORD dwExStyle,
				DWORD dwStyle,
				const RECT& rect,
				CWnd *pParentWnd,
				COLORREF crBarColor = 0xFF, // progress color
				COLORREF crBkColor = 0xFFFFFF, // no progress color
				COLORREF crTextOverColor = 0xFFFFFF, // text color the progress is overlapping
				COLORREF crTextOutColor = 0, // text color the progress hasnt reached yet
				BOOL bShowPercentage = 1,
				CFont *pFont = 0 );
	virtual ~CProgressBar();

protected:
	virtual afx_msg LRESULT OnStep(WPARAM, LPARAM); // response to PBM_STEPIT
	virtual afx_msg LRESULT OnSetRange(WPARAM, LPARAM); // response to PBM_SETRANGE32
	virtual afx_msg LRESULT OnGetPos(WPARAM, LPARAM); // response to PBM_GETPOS
	virtual afx_msg LRESULT OnSetPosF(WPARAM, LPARAM); // response to PBM_FLOAT_SETPOS - see message description for warning
	virtual afx_msg LRESULT OnSetStepF(WPARAM, LPARAM); // response to PBM_FLOAT_SETSTEP - see message description for warning
	void Update( CDC *pDC );
	BOOL m_bShowPercentage;
	UINT m_nStartPos, m_nEndPos;
	float m_fPos;
	float m_fStepAmount;
	int   m_fFontSize;
	COLORREF m_crBarColor, m_crBkColor, m_crTextOverColor, m_crTextOutColor;
	CBrush m_brushBar, m_brushBackground;
	CFont m_Font;

	//{{AFX_MSG(CProgressBar)
	afx_msg void OnPaint();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};



/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif
