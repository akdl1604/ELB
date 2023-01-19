// SimpleSplashWnd.cpp : implementation file
//

#include "stdafx.h"
#include "SimpleSplashWnd.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CSimpleSplashWnd window
/////////////////////////////////////////////////////////////////////////////
// CSimpleSplashWnd

#if _MFC_VER <= 0x710
	#define __EXT_MFC_ON_WM_NCHITTEST() ON_WM_NCHITTEST()
#else
	#define __EXT_MFC_ON_WM_NCHITTEST() \
		{ WM_NCHITTEST, 0, 0, 0, AfxSig_l_p, \
			(AFX_PMSG)(AFX_PMSGW) \
			(static_cast < UINT (AFX_MSG_CALL CWnd::*)(CPoint) > (&ThisClass :: OnNcHitTest)) },
#endif

static bool g_bSplashWndClassRegistered = false;

CSimpleSplashWnd::CSimpleSplashWnd()
	: m_sStatusText( _T("Initializing ...") )
{
	m_nProgress = 1;
	VERIFY( RegisterSplashWndClass() );
	__initControls();
}

CSimpleSplashWnd::CSimpleSplashWnd(
	CWnd * pWndParent,
	UINT nBitmapID)
	: m_sStatusText( _T("Initializing ...") )
{
	VERIFY( Create( pWndParent, nBitmapID ) );
	__initControls();
}

CSimpleSplashWnd::~CSimpleSplashWnd()
{
	m_Font.DeleteObject();
}


BEGIN_MESSAGE_MAP(CSimpleSplashWnd, CWnd)
	//{{AFX_MSG_MAP(CSimpleSplashWnd)
	ON_WM_ERASEBKGND()
	ON_WM_PAINT()
	ON_WM_NCCALCSIZE()
	ON_WM_NCHITTEST()
	ON_WM_CLOSE()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()


/////////////////////////////////////////////////////////////////////////////
// CSimpleSplashWnd message handlers

void CSimpleSplashWnd::SetStatusText(LPCTSTR sStatusText, int nProgress)
{
	ASSERT_VALID( this );
	ASSERT( GetSafeHwnd() != NULL );
	ASSERT( ::IsWindow( GetSafeHwnd() ) );
	LPCTSTR sText = (sStatusText == NULL) ? _T("") : sStatusText;
	if( m_sStatusText == sText )
		return;
	m_sStatusText = sText;
	if( (GetStyle() & WS_VISIBLE) == 0 )
		return;
	if (nProgress < 1 || nProgress > 100)
		nProgress = 100;
	m_nProgress = nProgress;
	Invalidate();
	UpdateWindow();
}

bool CSimpleSplashWnd::Create(	CWnd * pWndParent,	UINT nBitmapID	)
{
	ASSERT_VALID( this );
	ASSERT( GetSafeHwnd() == NULL );
	ASSERT( m_bitmap.GetSafeHandle() == NULL );

	if( !RegisterSplashWndClass() )
	{
		ASSERT( FALSE );
		return false;
	}

	if( !m_bitmap.LoadBitmap(nBitmapID) )
	{
		ASSERT( FALSE );
		return false;
	}
BITMAP _bmpInfo;
	::memset( &_bmpInfo, 0, sizeof(BITMAP) );
	m_bitmap.GetBitmap( &_bmpInfo );
	m_sizeBitmap.cx = _bmpInfo.bmWidth;
	m_sizeBitmap.cy = _bmpInfo.bmHeight;

CRect rcDesktop;
	VERIFY(
		::SystemParametersInfo(
			SPI_GETWORKAREA,
			0,
			PVOID(&rcDesktop),
			0
			)
		);
CRect rcWnd( 0, 0, m_sizeBitmap.cx, m_sizeBitmap.cy );
	rcWnd.OffsetRect(
		( rcDesktop.Width() - m_sizeBitmap.cx ) / 2,
		//( rcDesktop.Height() - m_sizeBitmap.cy ) / 2
		200
		);
	if( !CWnd::CreateEx(
			0,
			GL_VIEWS_SPLASHWND_WNDCLASS,
			_T("GLViews initizlizing ..."),
			WS_POPUP|WS_VISIBLE,
			rcWnd.left, rcWnd.top, m_sizeBitmap.cx, m_sizeBitmap.cy,
			pWndParent->GetSafeHwnd(),
			(HMENU)NULL
			)
		)
	{
		ASSERT( FALSE );
		m_bitmap.DeleteObject();
		return false;
	}

	VERIFY(
		::SetWindowPos(
			m_hWnd, HWND_TOP, 0, 0, 0, 0,
			SWP_NOSIZE|SWP_NOMOVE|SWP_SHOWWINDOW
			)
		);

	CSimpleSplashWnd::PassMsgLoop( false );

	return true;
}

void CSimpleSplashWnd::__initControls()
{
	VERIFY(m_Font.CreateFont(
		(INT)(17*SCALE_X),                        // nHeight
		0,                         // nWidth
		0,                         // nEscapement
		0,                         // nOrientation
		FW_BOLD,	               // nWeight
		FALSE,                     // bItalic
		FALSE,                     // bUnderline
		0,                         // cStrikeOut
		ANSI_CHARSET,              // nCharSet
		OUT_DEFAULT_PRECIS,        // nOutPrecision
		CLIP_DEFAULT_PRECIS,       // nClipPrecision
		DEFAULT_QUALITY,           // nQuality
		DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
		_T("Arial")));

	VERIFY(m_FontTitle.CreateFont(
		(INT)(36*SCALE_X),         // nHeight
		0,                         // nWidth
		0,                         // nEscapement
		0,                         // nOrientation
		FW_BOLD,	               // nWeight
		FALSE,                     // bItalic
		FALSE,                     // bUnderline
		0,                         // cStrikeOut
		ANSI_CHARSET,              // nCharSet
		OUT_DEFAULT_PRECIS,        // nOutPrecision
		CLIP_DEFAULT_PRECIS,       // nClipPrecision
		DEFAULT_QUALITY,           // nQuality
		DEFAULT_PITCH | FF_SWISS,  // nPitchAndFamily
		_T("Arial")));
}

BOOL CSimpleSplashWnd::OnEraseBkgnd(CDC* pDC)
{
	pDC;
	return TRUE;
}

void CSimpleSplashWnd::OnPaint()
{
	ASSERT( m_bitmap.GetSafeHandle() != NULL );
CPaintDC dcPaint( this );
CDC dcMem;
	if( !dcMem.CreateCompatibleDC( &dcPaint ) )
	{
		ASSERT( FALSE );
		return;
	}
CBitmap * pOldBmp = dcMem.SelectObject( &m_bitmap );
	dcPaint.BitBlt(
		0, 0, m_sizeBitmap.cx, m_sizeBitmap.cy,
		&dcMem,
		0, 0,
		SRCCOPY
		);
	if( !m_sStatusText.IsEmpty() )
	{
		CRect rcText;
		GetClientRect( &rcText );
		rcText.bottom -= 6;
		rcText.top = rcText.bottom - (INT)(80*SCALE_Y);
		rcText.DeflateRect( (INT)(16 * SCALE_X), 0 );
		int nOldBkMode = dcPaint.SetBkMode( TRANSPARENT );

		//draw Provider text
		CString strMachine, strVersion, strUpdateDate;
		strMachine.Format(_T("Round Side Sealing"));
		strVersion.Format(_T("[ Version ] %s"), "ver. 1.0.0");
		strUpdateDate.Format(_T("[ Update date ] %s"), "2017.06.27");

		CRect rcProvider = rcText;
		rcProvider.top = (INT)(10*SCALE_Y);
		COLORREF clrTextOld = dcPaint.SetTextColor( RGB(255,255,255) );
		CFont * pOldFont = dcPaint.SelectObject( &m_Font );
		/*
		dcPaint.DrawText(_T("Copyright (C) LET All Rights Reserved."), &rcProvider, DT_SINGLELINE|DT_LEFT|DT_END_ELLIPSIS );
		rcProvider.top += (INT)(20*SCALE_Y);
		rcProvider.top += (INT)(20*SCALE_Y);
		dcPaint.DrawText(strVersion, &rcProvider, DT_SINGLELINE|DT_LEFT|DT_END_ELLIPSIS );
		rcProvider.top += (INT)(20*SCALE_Y);
		dcPaint.DrawText(strUpdateDate, &rcProvider, DT_SINGLELINE|DT_LEFT|DT_END_ELLIPSIS );
		dcPaint.SelectObject( &m_FontTitle );

		rcProvider.top = 30;
		rcProvider.left = 30;
		
		dcPaint.DrawText(strMachine, &rcProvider, DT_SINGLELINE|DT_LEFT|DT_END_ELLIPSIS );
		dcPaint.SelectObject( &m_Font );
		*/

		//draw text
		//clrTextOld = dcPaint.SetTextColor( RGB(255,230,0) );
		clrTextOld = dcPaint.SetTextColor( RGB(0,230,0) );
		dcPaint.DrawText(m_sStatusText,	&rcText, DT_SINGLELINE|DT_LEFT|DT_VCENTER|DT_END_ELLIPSIS );

		//DrawProgress
		CRect rcProgress = rcText;
		rcProgress.top += (INT)(50*SCALE_Y);
		rcProgress.bottom -= (INT)(10*SCALE_Y);
		dcPaint.Rectangle(rcProgress);
		rcProgress.right = rcProgress.left + (int)(rcProgress.Width() * m_nProgress / 100.0);
		CBrush *pOldBrush, tmpBrush;
		tmpBrush.CreateSolidBrush(RGB(128,100,230));
		pOldBrush = dcPaint.SelectObject(&tmpBrush);
		dcPaint.Rectangle(rcProgress);
		dcPaint.SelectObject(pOldBrush);

		dcPaint.SelectObject( pOldFont );
		dcPaint.SetTextColor( clrTextOld );
		dcPaint.SetBkMode( nOldBkMode );
	} // if( !m_sStatusText.IsEmpty() )
	dcMem.SelectObject( pOldBmp );
}

void CSimpleSplashWnd::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp)
{
	bCalcValidRects;
	lpncsp;
}

LRESULT CSimpleSplashWnd::OnNcHitTest(CPoint point)
{
	point;
	return HTCLIENT;
}

void CSimpleSplashWnd::OnClose()
{
}

BOOL CSimpleSplashWnd::PreCreateWindow(CREATESTRUCT& cs)
{
	if( ( !RegisterSplashWndClass() )
		|| ( !CWnd::PreCreateWindow(cs) )
		)
	{
		ASSERT( FALSE );
		return FALSE;
	}

	cs.lpszClass = GL_VIEWS_SPLASHWND_WNDCLASS;

	return TRUE;
}

bool CSimpleSplashWnd::RegisterSplashWndClass()
{
	if( g_bSplashWndClassRegistered )
		return true;

WNDCLASS _wndClassInfo;
HINSTANCE hInst = AfxGetInstanceHandle();
	if( ! ::GetClassInfo(
			hInst,
			GL_VIEWS_SPLASHWND_WNDCLASS,
			&_wndClassInfo
			)
		)
	{
		// otherwise we need to register a new class
		_wndClassInfo.style =
				CS_GLOBALCLASS
				//|CS_DBLCLKS
				|CS_HREDRAW|CS_VREDRAW
				|CS_NOCLOSE
				|CS_SAVEBITS
				;
		_wndClassInfo.lpfnWndProc = ::DefWindowProc;
		_wndClassInfo.cbClsExtra = _wndClassInfo.cbWndExtra = 0;
		_wndClassInfo.hInstance = hInst;
		_wndClassInfo.hIcon = NULL;
		_wndClassInfo.hCursor =
				::LoadCursor(
					NULL, //hInst,
					IDC_WAIT
					)
				;
		ASSERT( _wndClassInfo.hCursor != NULL );
		_wndClassInfo.hbrBackground = NULL;
		_wndClassInfo.lpszMenuName = NULL;
		_wndClassInfo.lpszClassName = GL_VIEWS_SPLASHWND_WNDCLASS;
		if( !::AfxRegisterClass( &_wndClassInfo ) )
		{
			ASSERT( FALSE );
			//AfxThrowResourceException();
			return false;
		}
	}

	g_bSplashWndClassRegistered = true;
	return true;
}

BOOL CSimpleSplashWnd::DestroyWindow()
{
	ShowWindow( SW_HIDE );
	CSimpleSplashWnd::PassMsgLoop( false );

	return CWnd::DestroyWindow();
}

void CSimpleSplashWnd::PassMsgLoop(
	bool bEnableOnIdleCalls
	)
{
	//__PROF_UIS_MANAGE_STATE;
	AFX_MANAGE_STATE( ::AfxGetAppModuleState() );

MSG msg;
	// Process all the messages in the message queue
	while( PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) )
	{
		if( !AfxGetThread()->PumpMessage() )
		{
			PostQuitMessage(0);
			return;
		} // if( !AfxGetThread()->PumpMessage() )
	} // while( PeekMessage(&msg, NULL, 0, 0, PM_NOREMOVE) )
	if( bEnableOnIdleCalls )
	{
		for(	LONG nIdleCounter = 0L;
				::AfxGetThread()->OnIdle(nIdleCounter);
				nIdleCounter ++
				);
	}
}
