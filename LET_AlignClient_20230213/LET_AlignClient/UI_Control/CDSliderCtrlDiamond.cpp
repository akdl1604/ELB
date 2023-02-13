// CDSliderCtrlDiamond.cpp : implementation file
//

#include "stdafx.h"
#include "CDSliderCtrlDiamond.h"

// the following are needed for the HLS to RGB 
// (and RGB to HLS) conversion functions
#include <shlwapi.h>		
#pragma comment( lib, "shlwapi.lib" )

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CCDSliderCtrlDiamond

CCDSliderCtrlDiamond::CCDSliderCtrlDiamond()
{
}

CCDSliderCtrlDiamond::~CCDSliderCtrlDiamond()
{
	if (m_normalBrush.m_hObject)
		m_normalBrush.DeleteObject();
	if (m_focusBrush.m_hObject)
		m_focusBrush.DeleteObject();
}


BEGIN_MESSAGE_MAP(CCDSliderCtrlDiamond, CSliderCtrl)
	//{{AFX_MSG_MAP(CCDSliderCtrlDiamond)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, OnCustomDraw)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

void CCDSliderCtrlDiamond::SetPrimaryColor(COLORREF cr)
{
	// sets primary color of control, and derives shadow and hilite colors
	// also initializes brushes that are used in custom draw functions

	m_crPrimary = cr;

	// get hilite and shadow colors

	m_crHilite = ::ColorAdjustLuma( cr, 500, TRUE );	// increase by 50%
	m_crShadow = ::ColorAdjustLuma( cr, -333, TRUE );	// decrease by 33.3%

	// create normal (solid) brush 

	if ( m_normalBrush.m_hObject )
		m_normalBrush.DeleteObject();
	
	m_normalBrush.CreateSolidBrush( cr );


	// create a hatch-patterned pixel pattern for patterned brush 
	// (used when thumb has focus/is selected)	

	// see http://www.codeproject.com/gdi/custom_pattern_brush.asp
	// or look for BrushTool.exe for the code that generates these bits
	
	WORD bitsBrush1[8] = { 0x0055,0x00aa,0x0055,0x00aa,0x0055,0x00aa,0x0055,0x00aa };
	
	CBitmap bm;
	bm.CreateBitmap( 8, 8, 1, 1, bitsBrush1);
	
	LOGBRUSH logBrush;
	logBrush.lbStyle = BS_PATTERN;
	logBrush.lbHatch = (ULONG_PTR) bm.GetSafeHandle();
	logBrush.lbColor = 0;	// ignored anyway; must set DC background and text colors
	
	if ( m_focusBrush.m_hObject )
		m_focusBrush.DeleteObject();

	m_focusBrush.CreateBrushIndirect(&logBrush);

}

/////////////////////////////////////////////////////////////////////////////
// CCDSliderCtrlDiamond message handlers

void CCDSliderCtrlDiamond::OnCustomDraw(NMHDR* pNMHDR, LRESULT* pResult) 
{
	NMCUSTOMDRAW nmcd = *(LPNMCUSTOMDRAW)pNMHDR;
	*pResult = 0;

	if ( nmcd.dwDrawStage == CDDS_PREPAINT )
	{
		// return CDRF_NOTIFYITEMDRAW so that we will 
		// get subsequent CDDS_ITEMPREPAINT notifications

		*pResult = CDRF_NOTIFYITEMDRAW ;		
		return;
	}
	else if ( nmcd.dwDrawStage == CDDS_ITEMPREPAINT )
	{
		if ( nmcd.dwItemSpec == TBCD_THUMB )
		{
			CDC* pDC = CDC::FromHandle( nmcd.hdc );
			int iSaveDC = pDC->SaveDC();
			
			CBrush* pB = &m_normalBrush;

			// if thumb is selected/focussed, switch brushes

			if ( nmcd.uItemState && CDIS_FOCUS )
			{
				pB = &m_focusBrush;
				
				pDC->SetBrushOrg( nmcd.rc.right%8, nmcd.rc.top%8 );
				pDC->SetBkColor( m_crPrimary );
				pDC->SetTextColor( m_crHilite );				
			}
			
			pDC->SelectObject( *pB );
			
			CPen penShadow;
			penShadow.CreatePen( PS_SOLID, 1, m_crShadow );
			pDC->SelectObject( penShadow );

			int xx, yy, dx, dy, cx, cy;
			xx = nmcd.rc.left;
			yy = nmcd.rc.top;
			dx = 2;
			dy = 2;
			cx = nmcd.rc.right - xx - 1;
			cy = nmcd.rc.bottom - yy - 1;
			POINT pts[8] = { {xx+dx, yy}, {xx, yy+dy}, {xx, yy+cy-dy}, {xx+dx, yy+cy},
					{xx+cx-dx, yy+cy}, {xx+cx, yy+cy-dy}, {xx+cx, yy+dy}, {xx+cx-dx, yy} };

			pDC->Polygon( pts, 8 );
			pDC->RestoreDC( iSaveDC );
			pDC->Detach();

			penShadow.DeleteObject();

			CDC::DeleteTempMap();
			::ReleaseDC(this->m_hWnd, nmcd.hdc);

			*pResult = CDRF_SKIPDEFAULT;
		}
	}
}

