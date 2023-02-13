/////////////////////////////////////////////////////////////////////////////
// ProgressBar.cpp : source file
//
// MFC ProgressBar - ProgressBar class implementation
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

#include "stdafx.h"
#include "ProgressBar.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CProgressBar
IMPLEMENT_DYNAMIC(CProgressBar, CWnd)

CProgressBar::CProgressBar() : 
m_nEndPos( 100 ),
m_nStartPos( 0 ),
m_fPos( 0 ),
m_fStepAmount( 1 ),
m_fFontSize( 14 ),
m_crBarColor( RGB( 255, 0, 128 ) ),
m_crBkColor( RGB( 255, 255, 255 ) ),
m_crTextOverColor( RGB( 255, 255, 255 ) ),
m_crTextOutColor( RGB( 0, 0, 0 ) ),
m_bShowPercentage( true )
{ }

CProgressBar::~CProgressBar()
{ }


BEGIN_MESSAGE_MAP(CProgressBar, CWnd)
	//{{AFX_MSG_MAP(CProgressBar)
	ON_WM_PAINT()
	ON_MESSAGE(PBM_FLOAT_SETSTEP, OnSetStepF)
	ON_MESSAGE(PBM_FLOAT_SETPOS, OnSetPosF)
	ON_MESSAGE(PBM_GETPOS, OnGetPos)
	ON_MESSAGE(PBM_SETRANGE32, OnSetRange)
	ON_MESSAGE(PBM_STEPIT, OnStep)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()



/////////////////////////////////////////////////////////////////////////////
// CProgressBar message handlers

void CProgressBar::OnPaint() 
{
	CPaintDC dc(this); // device context for painting

	Update( &dc );
}

BOOL CProgressBar::Create(DWORD dwExStyle, DWORD dwStyle, const RECT &rect, CWnd *pParentWnd, COLORREF crBarColor, COLORREF crBkColor, COLORREF crTextOverColor, COLORREF crTextOutColor, BOOL bShowPercentage, CFont *pFont)
{
	// register control class
	CString strClassName = AfxRegisterWndClass( CS_VREDRAW | CS_HREDRAW | CS_OWNDC, LoadCursor( NULL, IDC_ARROW ), CreateSolidBrush( crBkColor ), NULL );

	// set and determine display options
	m_brushBar.DeleteObject( );
	if( !m_brushBar.CreateSolidBrush( crBarColor ) )
		return false;

	m_brushBackground.DeleteObject( );
	if( !m_brushBackground.CreateSolidBrush( crBkColor ) )
		return false;

	m_crBarColor = crBarColor;
	m_crBkColor = crBkColor;
	m_crTextOverColor = crTextOverColor;
	m_crTextOutColor = crTextOutColor;

	m_bShowPercentage = bShowPercentage;

	m_Font.DeleteObject( );
	if( pFont )
	{
		ASSERT_VALID(pFont);
		LOGFONT logFont;
		pFont->GetLogFont( &logFont );
		VERIFY( m_Font.CreateFontIndirect( &logFont ) );
	}
	else
	{
		VERIFY( m_Font.CreateFont( m_fFontSize, 0, 0, 0, FW_NORMAL, 0, 0, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, PROOF_QUALITY, VARIABLE_PITCH | FF_ROMAN, _T( "MS Sans Serif" ) ) );
	}

	// create control window
	return CreateEx( dwExStyle, strClassName, NULL, dwStyle, rect, pParentWnd, NULL, NULL );
}

void CProgressBar::SetRange(UINT nStartPos, UINT nEndPos)
{
	ASSERT( nStartPos < nEndPos );
	ASSERT( nStartPos >= 0 );
	ASSERT( nEndPos >= 0 );

	m_nStartPos = nStartPos;
	m_nEndPos = nEndPos;
	
	m_fPos = ( float )nStartPos;

	// update
	CDC *pDC = GetDC( );
	Update( pDC );
	VERIFY( ReleaseDC( pDC ) );
}

float CProgressBar::GetPosition() const
{
	return m_fPos;
}

void CProgressBar::SetPosition(float fPos)
{
	if( fPos < m_nStartPos )
		fPos = ( float )m_nStartPos;
	if( fPos > m_nEndPos )
		fPos = ( float )m_nEndPos;

	m_fPos = fPos;

	// update
	CDC *pDC = GetDC( );
	Update( pDC );
	VERIFY( ReleaseDC( pDC ) );
}

void CProgressBar::SetStepAmount(float fStepAmount)
{
	m_fStepAmount = fStepAmount;
}

void CProgressBar::SetFontSize( int size )
{
	m_fFontSize=size;
}

void CProgressBar::Step()
{
	m_fPos += m_fStepAmount;

	if( m_fPos < m_nStartPos )
		m_fPos = ( float )m_nStartPos;
	if( m_fPos > m_nEndPos )
		m_fPos = ( float )m_nEndPos;

	// update
	CDC *pDC = GetDC( );
	Update( pDC );
	VERIFY( ReleaseDC( pDC ) );
}

void CProgressBar::Update(CDC *pDC)
{
	RECT rc;
	RECT store;
	char output[ 20 ];
	CDC dcCompatible, dcBuffer;
	CBitmap pBmpCompatible, pBmpBuffer;
	CBitmap *pBmpOld, *pBmpOldBuffer;
	CFont *pFontOld;
	CSize pTextSize;
	CPoint pTextPos;
	const UINT nRange = m_nEndPos - m_nStartPos;
	
	// get dimensions
	GetClientRect( &rc );
	
	// store dimensions
	store = rc;
	
	// create and prepare buffer for text output
	VERIFY( dcBuffer.CreateCompatibleDC( pDC ) );
	
	// create a string buffer for output
	if( m_bShowPercentage )
		sprintf( output, "%.f%%", m_fPos ); // no decimal
	
	// prepare device-context for text output
	pFontOld = dcBuffer.SelectObject( &m_Font );
	dcBuffer.SetBkColor( m_crBarColor );
	dcBuffer.SetTextColor( m_crTextOverColor );
	
	// store dimensions of text output
	pTextSize = dcBuffer.GetTextExtent( output );
	pTextPos.x = ( store.right / 2 ) - ( pTextSize.cx / 2 );
	pTextPos.y = ( store.bottom / 2 ) - ( pTextSize.cy / 2 );
	
	// create storage bitmap buffer
	VERIFY( pBmpBuffer.CreateCompatibleBitmap( pDC, store.right + ( pTextSize.cx * 2 ) + 1, store.bottom + ( pTextSize.cy * 2 ) + 1 ) );
	pBmpOldBuffer = dcBuffer.SelectObject( &pBmpBuffer );
	
	// compute background painting dimensions
	rc.left = rc.left + ( ( rc.right / nRange ) * ( long )m_fPos );
	
	// paint background
	dcBuffer.FillRect( &rc, &m_brushBackground );
	
	// refill rect
	rc = store;
	
	// compute bar painting dimensions

	// if it is not at the end yet ...
	if( m_fPos != m_nEndPos )
	{
		float calcRight = ( float )rc.right;
		calcRight = calcRight / nRange;
		rc.right = rc.left + ( long )( calcRight * m_fPos );
	}
	// ... otherwise keep it the same (the entire width of the window)
	
	// paint bar
	dcBuffer.FillRect( &rc, &m_brushBar );
		
	// output text
	if( m_bShowPercentage )
		VERIFY( dcBuffer.TextOut( 0, rc.bottom + 1, output ) );
	
	// prepare device-context for next text output
	dcBuffer.SetBkColor( m_crBkColor );
	dcBuffer.SetTextColor( m_crTextOutColor );
	
	// output text
	if( m_bShowPercentage )
		VERIFY( dcBuffer.TextOut( 0, rc.bottom + pTextSize.cy + 1, output, int(strlen( output )) ) );
	
	// get rid of the font
	dcBuffer.SelectObject( &pFontOld );
	
	// create bitmap storage device-context
	VERIFY( dcCompatible.CreateCompatibleDC( pDC ) );
	
	// create bitmap for text-effect storage
	VERIFY( pBmpCompatible.CreateCompatibleBitmap( pDC, pTextSize.cx, pTextSize.cy ) );
	
	// prepare and store the text-effect
	pBmpOld = dcCompatible.SelectObject( &pBmpCompatible );
	VERIFY( dcCompatible.BitBlt( 0, 0, pTextSize.cx, pTextSize.cy, &dcBuffer, 0, rc.bottom + 1, SRCCOPY ) );
	VERIFY( dcCompatible.BitBlt( rc.right - pTextPos.x, 0, pTextSize.cx - ( rc.right - pTextPos.x ), pTextSize.cy, &dcBuffer, rc.right - pTextPos.x, rc.bottom + pTextSize.cy + 1, SRCCOPY ) );
	VERIFY( dcBuffer.BitBlt( pTextPos.x, pTextPos.y, pTextSize.cx, pTextSize.cy, &dcCompatible, 0, 0, SRCCOPY ) );
	
	// clean and delete dc
	dcCompatible.SelectObject( pBmpOld );
	VERIFY( dcCompatible.DeleteDC( ) );
	
	// delete text-effect bitmap
	VERIFY( pBmpCompatible.DeleteObject( ) );
	
	// output text effect
	VERIFY( pDC->BitBlt( store.left, store.top, store.right, store.bottom, &dcBuffer, 0, 0, SRCCOPY ) );
	
	// clean and delete buffer dc
	dcBuffer.SelectObject( pBmpOldBuffer );
	VERIFY( dcBuffer.DeleteDC( ) );
	
	// delete buffer bitmap
	VERIFY( pBmpBuffer.DeleteObject( ) );
}

float CProgressBar::GetStepAmount() const
{
	return m_fStepAmount;
}

LRESULT CProgressBar::OnGetPos(WPARAM, LPARAM)
{
	return (UINT)GetPosition();
}

LRESULT CProgressBar::OnSetRange(WPARAM w, LPARAM l)
{
	SetRange(int(w), int(l));
	return 1;
}

LRESULT CProgressBar::OnStep(WPARAM, LPARAM)
{
	Step();
	return 1;
}

LRESULT CProgressBar::OnSetStepF(WPARAM w, LPARAM)
{
	SetStepAmount(*(float*)w);
	return 1;
}

LRESULT CProgressBar::OnSetPosF(WPARAM w, LPARAM)
{
	SetPosition(*(float*)w);
	return 1;
}
