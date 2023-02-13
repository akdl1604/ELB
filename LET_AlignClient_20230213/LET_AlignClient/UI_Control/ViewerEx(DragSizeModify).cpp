// CumstomPicture.cpp : implementation file
//

#include "stdafx.h"
#include "ViewerEx.h"
#include <math.h>

// CViewerEx
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
IMPLEMENT_DYNAMIC(CViewerEx, CWnd)


CViewerEx::CViewerEx()
{
	RegisterWindowClass();
	m_gdiplusToken = 0;
	m_pBmp = NULL;
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);

	//----- Move Image -----//
	m_ptResult	= (0,0);
	m_ptLDown	= (0,0);
	m_ptLup		= (0,0);

	//----- Image Information -----//
	m_nWidth		 = 0;
	m_nHeight		 = 0;
	m_nBpp			 = 0;
	m_bClear		 = true;
	m_fMagnification = 1;
	m_bInit			 = false;
	m_bScroll		 = false;
	m_bNavi			 = false;
	m_strPath		 = _T("");
	m_fCameraResolutionX = 1;
	m_fCameraResolutionY = 1;
	//m_fCameraResolution = 1;

	//----- Overlay Drawing -----//
	m_nCurrFigure		= 0;
	m_bDrawCrossLine	= false;
	m_bDrawingFigure	= false;
	m_bDrawAllFigures	= true;
	m_bDrawFitImage		= false;
	m_bModifyFigure		= false;

	//----- Mouse Moving -----//
	m_isHitNW		= false;
	m_isHitN		= false;
	m_isHitNE		= false;
	m_isHitE		= false;
	m_isHitSE		= false;
	m_isHitS		= false;
	m_isHitSW		= false;
	m_isHitW		= false;
	m_isMoving		= false;

	m_bDrawRectangleInside = true;
	m_bDrawStart = true;
	//----- Menu -----//
	m_bMenu	= false;
	m_pMenu	= NULL;
	m_pSubZoom = NULL;
	m_pSubMode = NULL;

	//----- All Figures -----//
	for (int i=0; i<MAX_DRAW; i++)
	{
		m_stFigure[i].isDraw		 = false;
		m_stFigure[i].isLine		 = true;
		m_stFigure[i].isRectangle	 = false;
		m_stFigure[i].isCircle		 = false;
		m_stFigure[i].nWidth		 = 1;
		m_stFigure[i].ptBegin		 = (0,0);
		m_stFigure[i].ptEnd			 = (0,0);
		m_stFigure[i].clrColor		 = RGB(220 + 1.5*i,50 + 5*i,20 + 10*i);
		m_stFigure[i].fLength		 = 0.0;
		for (int j=0; j<8; j++)
			m_stFigure[i].rcHit[j]	 = CRect(0,0,0,0);

		m_stFigureText[i].bDraw=false;
	}

	m_clrRGB = 0;
	m_nY	 = 0;
	m_hWnd = NULL;
	m_bPatternLine = FALSE;

	m_ptPatternPos.x = -1;
	m_ptPatternPos.y = -1;

	m_bMaskClick = FALSE;
	m_bUseMask = FALSE;	
	m_bMaskPen = TRUE;	
	m_bMaskRect = FALSE;
}

CViewerEx::~CViewerEx()
{
	if (m_pSubMode != NULL)
	{
		delete m_pSubMode;
		m_pSubMode = NULL;
	}

	if (m_pSubZoom != NULL)
	{
		delete m_pSubZoom;
		m_pSubZoom = NULL;
	}

	if (m_pMenu != NULL)
	{
		delete m_pMenu;
		m_pMenu = NULL;
	}
	
	if(m_pBmpInfo8BitGray.Valid()!=NULL)		m_OverlayMemDC.DeleteDC();
	if(m_pBmpInfo8BitGray.Valid()!=NULL)		m_MaskOverlayMemDC.DeleteDC();
	if(m_pBmpInfo8BitGray.Valid()!=NULL)		m_RectOverlayMemDC.DeleteDC();

	m_AlignTargetLine.clear();	
	GdiplusShutdown(m_gdiplusToken);
}


BEGIN_MESSAGE_MAP(CViewerEx, CWnd)
	ON_WM_MOUSEMOVE()
	ON_WM_PAINT()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDOWN()
	ON_COMMAND_RANGE(ID_MENU_ZOOM_X0125, ID_MENU_ZOOM_X32, OnSetZoom)
	ON_COMMAND(ID_MENU_SAVE, OnSaveImage)
	ON_COMMAND(ID_MENU_LOAD, OnLoadImage)
	ON_COMMAND(ID_MENU_FIT, OnFitImage)
	ON_COMMAND(ID_MENU_NEVIGATION, OnNavigation)
	ON_COMMAND_RANGE(ID_MENU_MOUSE, ID_MENU_SCROLL, OnSetMode)
    ON_WM_RBUTTONUP()
    ON_WM_CONTEXTMENU()
	ON_WM_HSCROLL()
	ON_WM_VSCROLL()
	ON_WM_DROPFILES()
END_MESSAGE_MAP()

void CViewerEx::RegisterWindowClass(void)
{
	WNDCLASS wndcls;
	HINSTANCE hInst = AfxGetInstanceHandle();

	wndcls.style            = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
	wndcls.lpfnWndProc      = ::DefWindowProc;
	wndcls.cbClsExtra       = wndcls.cbWndExtra = 0;
	wndcls.hInstance        = hInst;
	wndcls.hIcon            = NULL;
	wndcls.hCursor          = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
	wndcls.hbrBackground    = (HBRUSH) (COLOR_3DFACE + 1);
	wndcls.lpszMenuName     = NULL;
	wndcls.lpszClassName    = VIEWER_WINCLASSNAME;

	AfxRegisterClass(&wndcls);
}


bool CViewerEx::InitControl(CWnd* pWnd, bool bMenu)
{
	if (pWnd == NULL) return false;

	m_bMenu = bMenu;

	CRect rect;
	GetWindowRect(&rect);
	pWnd->ScreenToClient(&rect);

	UINT id = GetDlgCtrlID();
	BOOL ret = DestroyWindow();
	if (ret == FALSE) return false;

	ret = Create(NULL,NULL,WS_CHILD|WS_VISIBLE|SS_CENTER|SS_NOTIFY, rect, pWnd, id);

	GetClientRect(&rect);
	rect.top = rect.bottom - SIZE_SCROLLBAR;
	rect.right -= SIZE_SCROLLBAR;
	m_wndSBH.CreateContol(this,true,rect,HORIZONTAL_SCROLL_ID);

	GetClientRect(&rect);
	rect.left = rect.right - SIZE_SCROLLBAR;
	rect.bottom = rect.bottom - SIZE_SCROLLBAR;
	m_wndSBV.CreateContol(this,false,rect,VERTICAL_SCROLL_ID);

	m_wndSBH.ShowWindow(SW_HIDE);
	m_wndSBV.ShowWindow(SW_HIDE);

	ResetScrollbarInfo();

	return true;
}

bool CViewerEx::OnLoad(CString path)
{
	if (path.IsEmpty()) return false;
	
	if (m_ImgSrc.IsNull() == false)
		m_ImgSrc.Destroy();
	
	
	m_ImgSrc.Load(path);
	m_strPath = path;

	m_ptResult		 = (0,0);
	m_ptLDown		 = (0,0);
	m_ptLup			 = (0,0);
	m_fMagnification = 1;
	m_bDrawFitImage	 = false;
	m_bDrawCrossLine = false;
	m_bDrawAllFigures= true;

	m_nWidth = m_ImgSrc.GetWidth();
	m_nHeight = m_ImgSrc.GetHeight();
	m_nBpp = m_ImgSrc.GetBPP();

	for (int i = 0; i < MAX_DRAW; i++)
	{
		m_stFigure[i].isDraw		 = false;
		m_stFigure[i].isLine		 = true;
		m_stFigure[i].isRectangle	 = false;
		m_stFigure[i].isCircle		 = false;
		m_stFigure[i].nWidth		 = 1;
		m_stFigure[i].ptBegin		 = (0,0);
		m_stFigure[i].ptEnd			 = (0,0);
		m_stFigure[i].clrColor		 = RGB(220 + 1.5*i,50 + 5*i,20 + 10*i);
		for (int j=0; j<8; j++)
			m_stFigure[i].rcHit[j]	 = CRect(0,0,0,0);
	}

	m_bClear = true;

	m_pMenu->EnableMenuItem(ID_MENU_LOAD, MF_ENABLED);
	m_pMenu->EnableMenuItem(ID_MENU_SAVE, MF_ENABLED);
	m_pMenu->EnableMenuItem(ID_MENU_FIT , MF_ENABLED);
	m_pMenu->EnableMenuItem(ID_MENU_NEVIGATION , MF_ENABLED);
	m_pMenu->EnableMenuItem((UINT)m_pSubZoom->m_hMenu, MF_ENABLED);
	CheckMenuItem(ID_MENU_ZOOM_X1,false);

	if (m_bScroll)
		ResetScrollbarInfo();

	Invalidate();

	return true;
}

bool CViewerEx::OnLoadImageFromPtr(BYTE* pBuffer)
{
	if (pBuffer == NULL) return false;
	// 649 x 484 8bit 기준 약 0.1ms 소요 확인.
	if (m_bInit == false) return false;
		
	if (m_ImgSrc.IsNull()) return false;

	int lineSize;
	if (m_nBpp == 8)
	{
		lineSize = m_nWidth * 1;
	}
	else if (m_nBpp == 24)
		lineSize = m_nWidth * 1 * 3;

	for ( int i = 0; i < m_nHeight; i++ )
	{
		const void* src;
		void* dst = m_ImgSrc.GetPixelAddress(0, i);
		if (m_nBpp == 8)
			src = pBuffer + (i*m_nWidth);
		else if (m_nBpp == 24)
			src = pBuffer + (i*m_nWidth*3);

		memcpy(dst, src, lineSize);
	}

	Invalidate();

	return true;
}

void CViewerEx::OnInitWithCamera(int nWidth, int nHeight, int nBpp)
{
	m_ptResult		 = (0,0);
	m_ptLDown		 = (0,0);
	m_ptLup			 = (0,0);
	m_fMagnification = 1;
	m_nWidth		 = nWidth;
	m_nHeight		 = nHeight;
	m_nBpp			 = nBpp;
	m_bDrawFitImage	 = false;
	m_bDrawCrossLine = false;
	m_bDrawAllFigures= true;

	if (m_ImgSrc.IsNull() == false)
		m_ImgSrc.Destroy();

	m_ImgSrc.Create(nWidth,nHeight,nBpp);

	/*if(m_pBmpInfo8BitGray.Valid()!=NULL)		m_OverlayMemDC.DeleteDC();
	if(m_pBmpInfo8BitGray.Valid()!=NULL)		m_MaskOverlayMemDC.DeleteDC();
	if(m_pBmpInfo8BitGray.Valid()!=NULL)		m_RectOverlayMemDC.DeleteDC();*/

	if (nBpp == 8)
	{
		// Define the color table
		RGBQUAD* tab = new RGBQUAD[256];
		for (int i = 0; i < 256; ++i)
		{
			tab[i].rgbRed = i;
			tab[i].rgbGreen = i;
			tab[i].rgbBlue = i;
			tab[i].rgbReserved = 0;
		}
		m_ImgSrc.SetColorTable(0, 256, tab);
		delete[] tab;
	}

	m_pMenu->EnableMenuItem(ID_MENU_SAVE, MF_DISABLED);
	m_pMenu->EnableMenuItem(ID_MENU_LOAD, MF_ENABLED);
	m_pMenu->EnableMenuItem(ID_MENU_FIT , MF_ENABLED);
	m_pMenu->EnableMenuItem(ID_MENU_NEVIGATION , MF_ENABLED);
	m_pMenu->EnableMenuItem((UINT)m_pSubZoom->m_hMenu, MF_ENABLED);
	CheckMenuItem(ID_MENU_ZOOM_X1,false);

	//Invalidate();

	m_bInit	= true;
	m_bClear = true;

	CDC m_OverlayMemDC_OLD;
	CDC m_MaskOverlayMemDC_OLD;
	CDC m_RectOverlayMemDC_OLD;

	if( m_MaskOverlayMemDC.m_hDC )
	{
		int nWidth2 = m_MaskOverlayMemDC.GetDeviceCaps(HORZSIZE);
		int nHeight2 = m_MaskOverlayMemDC.GetDeviceCaps(VERTSIZE);

		CBitmap Bmp; 
		m_MaskOverlayMemDC_OLD.CreateCompatibleDC(NULL); 
		Bmp.CreateBitmap(m_nWidth, m_nHeight, m_MaskOverlayMemDC.GetDeviceCaps(PLANES), m_MaskOverlayMemDC.GetDeviceCaps(BITSPIXEL), NULL); 
		m_MaskOverlayMemDC_OLD.SelectObject(&Bmp); 
		//m_MaskOverlayMemDC_OLD.BitBlt(0, 0, m_MaskOverlayMemDC.GetDeviceCaps(HORZSIZE), m_MaskOverlayMemDC.GetDeviceCaps(VERTSIZE), &m_MaskOverlayMemDC, m_MaskOverlayMemDC.GetDeviceCaps(HORZSIZE), m_MaskOverlayMemDC.GetDeviceCaps(VERTSIZE), SRCCOPY);
		
	}
	/*
	if( m_RectOverlayMemDC.m_hDC )
	{
		int nWidth2 = m_RectOverlayMemDC.GetDeviceCaps(HORZSIZE);
		int nHeight2 = m_RectOverlayMemDC.GetDeviceCaps(VERTSIZE);
		m_RectOverlayMemDC_OLD.BitBlt(0, 0, m_RectOverlayMemDC.GetDeviceCaps(HORZSIZE), m_RectOverlayMemDC.GetDeviceCaps(VERTSIZE), &m_RectOverlayMemDC, m_RectOverlayMemDC.GetDeviceCaps(HORZSIZE), m_RectOverlayMemDC.GetDeviceCaps(VERTSIZE), SRCCOPY);
	}*/

	m_OverlayMemDC.DeleteDC();
	m_MaskOverlayMemDC.DeleteDC();
	m_RectOverlayMemDC.DeleteDC();
	
	CBitmap Bmp; 
	m_OverlayMemDC.CreateCompatibleDC(NULL); 
	Bmp.CreateBitmap(m_nWidth, m_nHeight, m_OverlayMemDC.GetDeviceCaps(PLANES), m_OverlayMemDC.GetDeviceCaps(BITSPIXEL), NULL); 
	m_OverlayMemDC.SelectObject(&Bmp); 


	CBitmap MaskBmp; 
	m_MaskOverlayMemDC.CreateCompatibleDC(NULL); 
	MaskBmp.CreateBitmap(m_nWidth,m_nHeight,m_MaskOverlayMemDC.GetDeviceCaps(PLANES), m_MaskOverlayMemDC.GetDeviceCaps(BITSPIXEL),NULL); 
	m_MaskOverlayMemDC.SelectObject(&MaskBmp); 

	if( m_MaskOverlayMemDC_OLD.m_hDC )
		m_MaskOverlayMemDC.BitBlt(0, 0, m_MaskOverlayMemDC.GetDeviceCaps(HORZSIZE), m_MaskOverlayMemDC.GetDeviceCaps(VERTSIZE), &m_MaskOverlayMemDC_OLD, m_MaskOverlayMemDC_OLD.GetDeviceCaps(HORZSIZE), m_MaskOverlayMemDC_OLD.GetDeviceCaps(VERTSIZE), SRCCOPY);


	CBitmap RectMaskBmp; 
	m_RectOverlayMemDC.CreateCompatibleDC(NULL); 
	RectMaskBmp.CreateBitmap(m_nWidth,m_nHeight,m_RectOverlayMemDC.GetDeviceCaps(PLANES), m_RectOverlayMemDC.GetDeviceCaps(BITSPIXEL),NULL); 
	m_RectOverlayMemDC.SelectObject(&RectMaskBmp); 



	if( m_MaskOverlayMemDC_OLD.m_hDC )	m_MaskOverlayMemDC_OLD.DeleteDC();
	if( m_RectOverlayMemDC_OLD.m_hDC )	m_RectOverlayMemDC_OLD.DeleteDC();
}

void CViewerEx::ClearOverlayDC()
{
	m_OverlayMemDC.FillSolidRect(0, 0, (int)m_nWidth,(int)m_nHeight, RGB(0, 0, 0) );
}

void CViewerEx::ClearMaskOverlayDC()
{
	m_MaskOverlayMemDC.FillSolidRect(0,0,(int)m_nWidth,(int)m_nHeight,RGB(0,0,0));
}

void CViewerEx::ClearRectMaskOverlayDC()
{
	m_RectOverlayMemDC.FillSolidRect(0,0,(int)m_nWidth,(int)m_nHeight,RGB(0,0,0));
}

void CViewerEx::PreSubclassWindow()
{
	ModifyStyle( 0, SS_NOTIFY );

	DragAcceptFiles(TRUE);
	if (m_bMenu) CreateMousePopupMenu();
	CWnd::PreSubclassWindow();
}


void CViewerEx::OnMouseMove(UINT nFlags, CPoint point)
{
	GetCursorPos( &point ); 
	ScreenToClient( &point ); 

	CRect rect;
	GetClientRect(&rect);

	if (m_bScroll)
	{
		rect.right -= SIZE_SCROLLBAR;
		rect.bottom -= SIZE_SCROLLBAR;
	}

	if (m_ImgSrc.IsNull()) return;

	if (!m_bDrawFitImage && (point.x > m_nWidth * m_fMagnification - 1 || point.y > m_nHeight * m_fMagnification - 1) )
		return;

	// Fit Mode or Scroll Mode 에서 Scroll 사이즈를 뺀 영역까지만 마우스 포인트를 받을 수 있도록 보정.
	if (m_bDrawFitImage || m_bScroll)
		if (point.x > rect.Width() || point.y > rect.Height()) return;
	
	float fH = 0.0, fV = 0.0;
	if (m_bDrawFitImage)
	{
		fH = (float)rect.Width () / m_nWidth;
		fV = (float)rect.Height() / m_nHeight;
	}
	else
	{
		fH = m_fMagnification;
		fV = m_fMagnification;
	}
	
	point.x /= fH;
	point.y /= fV;

	if (m_bModifyFigure && !m_bDrawStart)
	{
		if (nFlags == MK_LBUTTON)
		{
			CPoint offset;	
			offset = point - m_ptLDown ;

			if (m_isHitNW)
			{
				m_stFigure[m_nCurrFigure].ptBegin.x = m_rcPreFigure.left + offset.x;
				m_stFigure[m_nCurrFigure].ptBegin.y = m_rcPreFigure.top  + offset.y;
			}
			if (m_isHitN)
			{
				if (m_stFigure[m_nCurrFigure].isLine)
				{
					m_stFigure[m_nCurrFigure].ptBegin.x = m_rcPreFigure.left   + offset.x;
					m_stFigure[m_nCurrFigure].ptBegin.y = m_rcPreFigure.top    + offset.y;
					m_stFigure[m_nCurrFigure].ptEnd.x   = m_rcPreFigure.right  + offset.x;
					m_stFigure[m_nCurrFigure].ptEnd.y   = m_rcPreFigure.bottom + offset.y;
				}
				else
				{
					m_stFigure[m_nCurrFigure].ptBegin.y = m_rcPreFigure.top + offset.y;
				}
			}
			if (m_isHitNE)
			{
				if (m_stFigure[m_nCurrFigure].isLine)
				{
					m_stFigure[m_nCurrFigure].ptEnd.x = m_rcPreFigure.right  + offset.x;
					m_stFigure[m_nCurrFigure].ptEnd.y = m_rcPreFigure.bottom + offset.y;
				}
				else
				{
					m_stFigure[m_nCurrFigure].ptEnd.x = m_rcPreFigure.right + offset.x;
					m_stFigure[m_nCurrFigure].ptBegin.y = m_rcPreFigure.top + offset.y;
				}
			}
			if (m_isHitE)
			{
				m_stFigure[m_nCurrFigure].ptEnd.x = m_rcPreFigure.right + offset.x;
			}
			if (m_isHitSE)
			{
				m_stFigure[m_nCurrFigure].ptEnd.x = m_rcPreFigure.right  + offset.x;
				m_stFigure[m_nCurrFigure].ptEnd.y = m_rcPreFigure.bottom + offset.y;
			}
			if (m_isHitS)
			{
				m_stFigure[m_nCurrFigure].ptEnd.y = m_rcPreFigure.bottom + offset.y;
			}
			if (m_isHitSW)
			{
				m_stFigure[m_nCurrFigure].ptBegin.x = m_rcPreFigure.left + offset.x;
				m_stFigure[m_nCurrFigure].ptEnd.y = m_rcPreFigure.bottom + offset.y;
			}
			if (m_isHitW)
			{
				m_stFigure[m_nCurrFigure].ptBegin.x = m_rcPreFigure.left + offset.x;
			}
			if ( m_isMoving && (!m_isHitNW || !m_isHitN  || !m_isHitNE || !m_isHitE  || !m_isHitSE || !m_isHitS || !m_isHitSW || !m_isHitW) )
			{
				m_stFigure[m_nCurrFigure].ptBegin.x = m_rcPreFigure.left   + offset.x;
				m_stFigure[m_nCurrFigure].ptBegin.y = m_rcPreFigure.top    + offset.y;
				m_stFigure[m_nCurrFigure].ptEnd.x   = m_rcPreFigure.right  + offset.x;
				m_stFigure[m_nCurrFigure].ptEnd.y   = m_rcPreFigure.bottom + offset.y;
			}

			Invalidate();
		}
		//Mouse Hit check
		// 도형이 그려지는 시작점과 끝점의 위치에 따라 조건문을 설정.
		else if (
				(point.x + m_ptResult.x >= m_stFigure[m_nCurrFigure].ptBegin.x -5 && point.x + m_ptResult.x <= m_stFigure[m_nCurrFigure].ptEnd.x +5 && point.y + m_ptResult.y >= m_stFigure[m_nCurrFigure].ptBegin.y -5 && point.y + m_ptResult.y <= m_stFigure[m_nCurrFigure].ptEnd.y   +5)
			||  (point.x + m_ptResult.x >= m_stFigure[m_nCurrFigure].ptBegin.x -5 && point.x + m_ptResult.x <= m_stFigure[m_nCurrFigure].ptEnd.x +5 && point.y + m_ptResult.y >= m_stFigure[m_nCurrFigure].ptEnd.y   -5 && point.y + m_ptResult.y <= m_stFigure[m_nCurrFigure].ptBegin.y +5)
			||  (point.x + m_ptResult.x >= m_stFigure[m_nCurrFigure].ptEnd.x   -5 && point.x + m_ptResult.x <= m_stFigure[m_nCurrFigure].ptBegin.x +5 && point.y + m_ptResult.y >= m_stFigure[m_nCurrFigure].ptEnd.y -5 && point.y + m_ptResult.y <= m_stFigure[m_nCurrFigure].ptBegin.y +5)
			||  (point.x + m_ptResult.x >= m_stFigure[m_nCurrFigure].ptEnd.x   -5 && point.x + m_ptResult.x <= m_stFigure[m_nCurrFigure].ptBegin.x +5 && point.y + m_ptResult.y >= m_stFigure[m_nCurrFigure].ptBegin.y -5 && point.y + m_ptResult.y <= m_stFigure[m_nCurrFigure].ptEnd.y   +5)
			)
		{
			::SetCursor(LoadCursor(NULL, IDC_ARROW));
			m_isMoving = false;
			m_isHitNW  = false;
			m_isHitN   = false;
			m_isHitNE  = false;
			m_isHitE   = false;
			m_isHitSE  = false;
			m_isHitS   = false;
			m_isHitSW  = false;
			m_isHitW   = false;

			if (
					(point.x + m_ptResult.x >= m_stFigure[m_nCurrFigure].ptBegin.x  && point.x + m_ptResult.x <= m_stFigure[m_nCurrFigure].ptEnd.x && point.y + m_ptResult.y >= m_stFigure[m_nCurrFigure].ptBegin.y  && point.y + m_ptResult.y <= m_stFigure[m_nCurrFigure].ptEnd.y )
				||	(point.x + m_ptResult.x >= m_stFigure[m_nCurrFigure].ptBegin.x  && point.x + m_ptResult.x <= m_stFigure[m_nCurrFigure].ptEnd.x && point.y + m_ptResult.y >= m_stFigure[m_nCurrFigure].ptEnd.y  && point.y + m_ptResult.y <= m_stFigure[m_nCurrFigure].ptBegin.y )
				||	(point.x + m_ptResult.x >= m_stFigure[m_nCurrFigure].ptEnd.x  && point.x + m_ptResult.x <= m_stFigure[m_nCurrFigure].ptBegin.x && point.y + m_ptResult.y >= m_stFigure[m_nCurrFigure].ptEnd.y  && point.y + m_ptResult.y <= m_stFigure[m_nCurrFigure].ptBegin.y )
				||  (point.x + m_ptResult.x >= m_stFigure[m_nCurrFigure].ptEnd.x  && point.x + m_ptResult.x <= m_stFigure[m_nCurrFigure].ptBegin.x && point.y + m_ptResult.y >= m_stFigure[m_nCurrFigure].ptBegin.y  && point.y + m_ptResult.y <= m_stFigure[m_nCurrFigure].ptEnd.y )
				)
			{
				::SetCursor(LoadCursor(NULL, IDC_SIZEALL));
				m_isMoving = true;
				m_isHitNW = false;
				m_isHitN  = false;
				m_isHitNE = false;
				m_isHitE  = false;
				m_isHitSE = false;
				m_isHitS  = false;
				m_isHitSW = false;
				m_isHitW  = false;
			}
			
			for (int i=0; i<8; i++)
			{
				if (m_stFigure[m_nCurrFigure].rcHit[i].left <= point.x * fH && point.x * fH <= m_stFigure[m_nCurrFigure].rcHit[i].right  &&
					m_stFigure[m_nCurrFigure].rcHit[i].top  <= point.y * fV && point.y * fV <= m_stFigure[m_nCurrFigure].rcHit[i].bottom )
				{
					switch(i)
					{
							// NW
						case 0 :
							::SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
							m_isHitNW  = true;
							m_isMoving = false;
							break;
							// N
						case 1 :
							::SetCursor(LoadCursor(NULL, IDC_SIZENS));
							m_isHitN  = true;
							m_isMoving = false;
							break;
							//NE
						case 2 :
							::SetCursor(LoadCursor(NULL, IDC_SIZENESW));
							m_isHitNE  = true;
							m_isMoving = false;
							break;
							// E
						case 3 :
							::SetCursor(LoadCursor(NULL, IDC_SIZEWE));
							m_isHitE  = true;
							m_isMoving = false;
							break;
							// SE
						case 4 :
							::SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
							m_isHitSE  = true;
							m_isMoving = false;
							break;
							// S
						case 5 :
							::SetCursor(LoadCursor(NULL, IDC_SIZENS));
							m_isHitS  = true;
							m_isMoving = false;
							break;
							// SW
						case 6 :
							::SetCursor(LoadCursor(NULL, IDC_SIZENESW));
							m_isHitSW  = true;
							m_isMoving = false;
							break;
							// W
						case 7 :
							::SetCursor(LoadCursor(NULL, IDC_SIZEWE));
							m_isHitW   = true;
							m_isMoving = false;
							break;
					}
				}
			}
		}
		else
		{
			m_isMoving = false;
			m_isHitNW  = false;
			m_isHitN   = false;
			m_isHitNE  = false;
			m_isHitE   = false;
			m_isHitSE  = false;
			m_isHitS   = false;
			m_isHitSW  = false;
			m_isHitW   = false;
		}
	}
	else
	{
		if (m_bDrawingFigure)
			::SetCursor(LoadCursor(NULL, IDC_HAND));
		else
			::SetCursor(LoadCursor(NULL, IDC_ARROW));
		
		if(nFlags == MK_LBUTTON)
		{	
			CPoint temp;
			temp = m_ptResult;

			// 도형을 그린 후 마우스 포인트가 좌상단으로 올라갈 경우 이미지의 옵셋이 변하게 된다.
			//아래 조건문은 보정하는 코드.
			if (!m_bDrawingFigure)
			{
				m_ptResult = m_ptLup + m_ptLDown - point;
			}

			//우하단 이미지 크기보다 못 넘어가게 막음.
			if(m_ptResult.x >  (m_nWidth - rect.Width()/m_fMagnification))
				m_ptResult.x = (m_nWidth - rect.Width()/ m_fMagnification);
			if(m_ptResult.y >  (m_nHeight - rect.Height()/m_fMagnification))
				m_ptResult.y = (m_nHeight - rect.Height()/m_fMagnification);

			//좌상단 0,0이하로 못 넘어가게 막음.
			if(m_ptResult.x < 0)	m_ptResult.x = 0;
			if(m_ptResult.y < 0 )	m_ptResult.y = 0;

			if (m_bDrawingFigure)
				m_stFigure[m_nCurrFigure].ptEnd = m_ptResult + point;

			if (m_bScroll)
				m_ptResult = temp;

			Invalidate();
		}
	}
	
	//이미지 상의 현재 마우스 좌표.
	m_ptMousePoint = point + m_ptResult;

	
	if( m_bUseMask && m_bMaskClick )
	{
		if( m_bMaskPen ) SetMaskPos(point);

		if( m_bMaskRect )
		{
			CRect rect;
			GetClientRect(&rect);

			if( (rect.right - point.x) <= 10 ) point.x = rect.right;
			if( point.x <= 10 ) point.x = 0;

			if( (rect.bottom - point.y) <= 10 ) point.y = rect.bottom;
			if( point.y <= 10 ) point.y = 0;

			DrawRect(m_ptLDown, point);
		}

		Invalidate();
	//	loadGrayRaw();
	}

	if( m_hWnd ) ::PostMessage(m_hWnd, WM_VIEW_CONTROL, MSG_FCV_SET_MOUSE_POS, MAKELPARAM(m_ptMousePoint.x, m_ptMousePoint.y));

	if (m_nBpp == 8)
		m_nY = GetValueY(m_ptMousePoint.x, m_ptMousePoint.y);
	else if (m_nBpp == 24)
		m_clrRGB = GetValueRGB(m_ptMousePoint.x, m_ptMousePoint.y);

	CWnd::OnMouseMove(nFlags, point);
}


void CViewerEx::OnPaint()
{
	CPaintDC dc(this); // device context for painting
	
	CRect rect;
	GetClientRect(&rect);

	//////////////////////////////////////////////////////////////////////////
	CDC memDC;
	CBitmap *pOldBitmap, bitmap;

	memDC.CreateCompatibleDC(&dc);

	bitmap.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());

	pOldBitmap = memDC.SelectObject(&bitmap);
	memDC.PatBlt(0, 0, rect.Width(), rect.Height(), SRCCOPY);
	//////////////////////////////////////////////////////////////////////////

	if (m_bScroll)
	{
		rect.right -= SIZE_SCROLLBAR;
		rect.bottom -= SIZE_SCROLLBAR;
	}

	// 2016-04-06. 
	// 상시 그리도록 변경 
	//if (m_bClear == true)
	{
		int w = 32;
		int h = 32;
		HDC hDC = CreateCompatibleDC(dc.m_hDC);
		HBITMAP hBmp = CreateCompatibleBitmap(dc.m_hDC, w, h);
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hDC, hBmp);

		BitBlt(hDC, 0, 0, w, h, hDC, 0, 0, SRCCOPY);
		CBrush Brush1(RGB(100,100,100));
		CBrush Brush2(RGB(110,110,110));

		FillRect(hDC,CRect(0,0,w/2,h/2),Brush1);
		FillRect(hDC,CRect(w/2,0,w,h/2),Brush2);
		FillRect(hDC,CRect(0,h/2,w/2,h),Brush2);
		FillRect(hDC,CRect(w/2,h/2,w,h),Brush1);
		
		CBitmap bm;
		bm.Attach(hBmp);

		CBrush brush;
		brush.CreatePatternBrush(&bm);

		/*CBrush *pOldBrush = (CBrush*)dc.SelectObject(&brush);
		dc.Rectangle(&rect);

		dc.SelectObject(pOldBrush);*/

		CBrush *pOldBrush = (CBrush*)memDC.SelectObject(&brush);
		memDC.Rectangle(&rect);

		memDC.SelectObject(pOldBrush);

		DeleteObject(hDC);
		DeleteObject(hBmp);
		DeleteObject(hOldBitmap);

		m_bClear = false;
	}

	if (m_ImgSrc.IsNull()) 
	{
		dc.BitBlt(0, 0, rect.Width(), rect.Height(), &memDC, 0, 0, SRCCOPY);

		memDC.SelectObject(pOldBitmap);
		memDC.DeleteDC();
		bitmap.DeleteObject();
		return;
	}

	//큰 배율에서 작은 배율로 축소 시 이미지의 크기를 넘어서 그리게 되는 것을 방지한다.
	if (m_ptResult.x + (float)rect.Width()/m_fMagnification > m_nWidth)
	{
		m_ptResult.x = m_nWidth - (float)rect.Width()/m_fMagnification;
		if (m_ptResult.x < 0) m_ptResult.x = 0;
		m_ptLup.x = m_ptResult.x;
	}
	if (m_ptResult.y + (float)rect.Height()/m_fMagnification > m_nHeight)
	{
		m_ptResult.y = m_nHeight - (float)rect.Height()/m_fMagnification;
		if (m_ptResult.y < 0) m_ptResult.y = 0;
		m_ptLup.y = m_ptResult.y;
	}

	// 컨트롤 안에서 이미지 크기가 작을 경우, 이미지를 벗어나 도형을 그리면 잔상이 남는다. 이를 보정하는 부분.
	CRgn rgn;
	if (m_bDrawFitImage)
	{
		/*rgn.CreateRectRgnIndirect(&rect);
		dc.SelectClipRgn(&rgn);*/
		rgn.CreateRectRgnIndirect(&rect);
		memDC.SelectClipRgn(&rgn);
	}
	else
	{
		if (m_nWidth*m_fMagnification < rect.Width() || m_nHeight*m_fMagnification < rect.Height())
		{
			CRect rc;
			rc = rect;

			rc.right = m_nWidth*m_fMagnification;
			rc.bottom = m_nHeight*m_fMagnification;

			/*rgn.CreateRectRgnIndirect(&rc);
			dc.SelectClipRgn(&rgn);*/
			rgn.CreateRectRgnIndirect(&rc);
			memDC.SelectClipRgn(&rgn);
		}
		else
		{
			/*rgn.CreateRectRgnIndirect(&rect);
			dc.SelectClipRgn(&rgn);*/
			rgn.CreateRectRgnIndirect(&rect);
			memDC.SelectClipRgn(&rgn);
		}
	}

	if (m_bDrawFitImage)
	{
		/*dc.SetStretchBltMode(COLORONCOLOR);
		m_ImgSrc.Draw(dc.m_hDC,rect);*/
		memDC.SetStretchBltMode(COLORONCOLOR);
		m_ImgSrc.Draw(memDC.m_hDC,rect);

		m_ptResult = (0,0); 
		m_fMagnification = 1.0;
	}
	else
	{
		// 이미지 축소 시 정상적인 그리기가 안되므로 옵션 설정.
		if (m_fMagnification < 1)
			memDC.SetStretchBltMode(COLORONCOLOR);
			//dc.SetStretchBltMode(COLORONCOLOR);

		//m_ImgSrc.Draw(dc.m_hDC,0,0,rect.Width(),rect.Height(),m_ptResult.x, m_ptResult.y, rect.Width()/m_fMagnification, rect.Height()/m_fMagnification);
		m_ImgSrc.Draw(memDC.m_hDC,0,0,rect.Width(),rect.Height(),m_ptResult.x, m_ptResult.y, rect.Width()/m_fMagnification, rect.Height()/m_fMagnification);
	}

	if (m_bDrawAllFigures)
	{
		float fH = m_fMagnification,fV = m_fMagnification;
		if (m_bDrawFitImage)
		{
			fH = (float)rect.Width()/m_nWidth;
			fV = (float)rect.Height()/m_nHeight;
		}

		for (int i=0; i<MAX_DRAW; i++)
		{
			if (!m_stFigure[i].isDraw) continue;

			CPen FigurePen, WhitePen, *pOldPen;
			FigurePen.CreatePen(PS_SOLID,m_stFigure[i].nWidth,m_stFigure[i].clrColor);
			WhitePen.CreatePen(PS_SOLID,1,RGB(255,255,255));
			//pOldPen = (CPen*)dc.SelectObject(&FigurePen);
			pOldPen = (CPen*)memDC.SelectObject(&FigurePen);

			CPoint ptNewBegin, ptNewEnd;

			ptNewBegin.x = m_stFigure[i].ptBegin.x * fH- m_ptResult.x * fH;
			ptNewBegin.y = m_stFigure[i].ptBegin.y * fV- m_ptResult.y * fV;
			ptNewEnd.x   = m_stFigure[i].ptEnd.x   * fH- m_ptResult.x * fH;
			ptNewEnd.y   = m_stFigure[i].ptEnd.y   * fV- m_ptResult.y * fV;
					
			// Index 및 직선을 그린다.
			if (m_stFigure[i].isLine)
			{
				CString Index;
				/*dc.SetBkMode(TRANSPARENT);
				dc.SetTextColor(RGB(0,255,0));*/
				memDC.SetBkMode(TRANSPARENT);
				memDC.SetTextColor(RGB(0,255,0));

				//Index.Format(L"%d Figure", i);
				//dc.TextOut(ptNewBegin.x, ptNewBegin.y-20, Index);

				/*dc.MoveTo(ptNewBegin);
				dc.LineTo(ptNewEnd);*/
				memDC.MoveTo(ptNewBegin);
				memDC.LineTo(ptNewEnd);

				m_stFigure[i].fLength = CalculateLength(ptNewBegin, ptNewEnd);

				NMHDR nmHdr;
				::ZeroMemory(&nmHdr, sizeof(NMHDR));

				nmHdr.hwndFrom = m_hWnd;
				nmHdr.idFrom = GetDlgCtrlID();
				nmHdr.code = WM_DRAWING_LINE_MESSAGE;
				GetParent()->SendMessage(WM_NOTIFY, (LPARAM)NULL, (WPARAM)&nmHdr);
			}
			// Index 및 사각형을 그린다.
			else if (m_stFigure[i].isRectangle)
			{
				CString Index;
				/*dc.SetBkMode(TRANSPARENT);
				dc.SetTextColor(RGB(0,255,0));*/
				memDC.SetBkMode(TRANSPARENT);
				memDC.SetTextColor(RGB(0,255,0));

				if(m_bDrawRectangleInside)
				{
					Index.Format("%4.2f , %4.2f (mm)", (GetPointEnd(i).x - GetPointBegin(i).x)*m_fCameraResolutionX,(GetPointEnd(i).y - GetPointBegin(i).y)*m_fCameraResolutionY);
					memDC.TextOut(ptNewBegin.x, ptNewBegin.y-20, Index);
				}
				/*dc.SelectStockObject(NULL_BRUSH);
				dc.Rectangle(ptNewBegin.x, ptNewBegin.y, ptNewEnd.x, ptNewEnd.y);*/
				memDC.SelectStockObject(NULL_BRUSH);
				memDC.Rectangle(ptNewBegin.x, ptNewBegin.y, ptNewEnd.x, ptNewEnd.y);

				if(m_bDrawRectangleInside)
				{
					memDC.MoveTo(ptNewBegin.x, (ptNewBegin.y + ptNewEnd.y) / 2.0);
					memDC.LineTo(ptNewEnd.x, (ptNewBegin.y + ptNewEnd.y) / 2.0);

					memDC.MoveTo((ptNewBegin.x + ptNewEnd.x) / 2.0, ptNewBegin.y);
					memDC.LineTo((ptNewBegin.x + ptNewEnd.x) / 2.0, ptNewEnd.y);
				}

			}
			// Index 및 원을 그린다.
			else if (m_stFigure[i].isCircle)
			{
				CString Index;
				/*dc.SetBkMode(TRANSPARENT);
				dc.SetTextColor(RGB(0,255,0));*/
				memDC.SetBkMode(TRANSPARENT);
				memDC.SetTextColor(RGB(0,255,0));

				//Index.Format(L"%d Figure", i);
				//dc.TextOut(ptNewBegin.x, ptNewBegin.y-20, Index);

				/*dc.SelectStockObject(NULL_BRUSH);
				dc.Ellipse(ptNewBegin.x, ptNewBegin.y, ptNewEnd.x, ptNewEnd.y);*/
				memDC.SelectStockObject(NULL_BRUSH);
				memDC.Ellipse(ptNewBegin.x, ptNewBegin.y, ptNewEnd.x, ptNewEnd.y);
			}

			if (m_bModifyFigure && (m_stFigure[i].isCircle || m_stFigure[i].isRectangle) && (i == m_nCurrFigure))
			{
				m_stFigure[i].rcHit[0].left   = ptNewBegin.x -5;
				m_stFigure[i].rcHit[0].right  = ptNewBegin.x +5;
				m_stFigure[i].rcHit[0].top	  = ptNewBegin.y -5;
				m_stFigure[i].rcHit[0].bottom = ptNewBegin.y +5;
				/*dc.SelectStockObject(WHITE_BRUSH);
				dc.Rectangle(m_stFigure[i].rcHit[0]);*/
				memDC.SelectStockObject(WHITE_BRUSH);
				memDC.Rectangle(m_stFigure[i].rcHit[0]);

				/*m_stFigure[i].rcHit[1].left   = (abs(ptNewBegin.x) + abs(ptNewEnd.x))/2 -5;
				m_stFigure[i].rcHit[1].right  = (abs(ptNewBegin.x) + abs(ptNewEnd.x))/2 +5;*/
				m_stFigure[i].rcHit[1].left   = ((ptNewBegin.x) + (ptNewEnd.x))/2 -5;
				m_stFigure[i].rcHit[1].right  = ((ptNewBegin.x) + (ptNewEnd.x))/2 +5;
				m_stFigure[i].rcHit[1].top	  = ptNewBegin.y -5;
				m_stFigure[i].rcHit[1].bottom = ptNewBegin.y +5;
				//dc.Rectangle(m_stFigure[i].rcHit[1]);
				memDC.Rectangle(m_stFigure[i].rcHit[1]);

				m_stFigure[i].rcHit[2].left   = ptNewEnd.x -5;
				m_stFigure[i].rcHit[2].right  = ptNewEnd.x +5;
				m_stFigure[i].rcHit[2].top	  = ptNewBegin.y -5;
				m_stFigure[i].rcHit[2].bottom = ptNewBegin.y +5;
				//dc.Rectangle(m_stFigure[i].rcHit[2]);
				memDC.Rectangle(m_stFigure[i].rcHit[2]);

				m_stFigure[i].rcHit[3].left   = ptNewEnd.x -5;
				m_stFigure[i].rcHit[3].right  = ptNewEnd.x +5;
				/*m_stFigure[i].rcHit[3].top	  = (abs(ptNewBegin.y) + abs(ptNewEnd.y))/2 -5;
				m_stFigure[i].rcHit[3].bottom = (abs(ptNewBegin.y) + abs(ptNewEnd.y))/2 +5;*/
				m_stFigure[i].rcHit[3].top	  = ((ptNewBegin.y) + (ptNewEnd.y))/2 -5;
				m_stFigure[i].rcHit[3].bottom = ((ptNewBegin.y) + (ptNewEnd.y))/2 +5;
				//dc.Rectangle(m_stFigure[i].rcHit[3]);
				memDC.Rectangle(m_stFigure[i].rcHit[3]);

				m_stFigure[i].rcHit[4].left   = ptNewEnd.x -5;
				m_stFigure[i].rcHit[4].right  = ptNewEnd.x +5;
				m_stFigure[i].rcHit[4].top	  = ptNewEnd.y -5;
				m_stFigure[i].rcHit[4].bottom = ptNewEnd.y +5;
				//dc.Rectangle(m_stFigure[i].rcHit[4]);
				memDC.Rectangle(m_stFigure[i].rcHit[4]);

				/*m_stFigure[i].rcHit[5].left   = (abs(ptNewBegin.x) + abs(ptNewEnd.x))/2 -5;
				m_stFigure[i].rcHit[5].right  = (abs(ptNewBegin.x) + abs(ptNewEnd.x))/2 +5;*/
				m_stFigure[i].rcHit[5].left   = ((ptNewBegin.x) + (ptNewEnd.x))/2 -5;
				m_stFigure[i].rcHit[5].right  = ((ptNewBegin.x) + (ptNewEnd.x))/2 +5;
				m_stFigure[i].rcHit[5].top	  = ptNewEnd.y -5;
				m_stFigure[i].rcHit[5].bottom = ptNewEnd.y +5;
				//dc.Rectangle(m_stFigure[i].rcHit[5]);
				memDC.Rectangle(m_stFigure[i].rcHit[5]);

				m_stFigure[i].rcHit[6].left   = ptNewBegin.x -5;
				m_stFigure[i].rcHit[6].right  = ptNewBegin.x +5;
				m_stFigure[i].rcHit[6].top	  = ptNewEnd.y -5;
				m_stFigure[i].rcHit[6].bottom = ptNewEnd.y +5;
				//dc.Rectangle(m_stFigure[i].rcHit[6]);
				memDC.Rectangle(m_stFigure[i].rcHit[6]);

				m_stFigure[i].rcHit[7].left   = ptNewBegin.x -5;
				m_stFigure[i].rcHit[7].right  = ptNewBegin.x +5;
				/*m_stFigure[i].rcHit[7].top	  = (abs(ptNewBegin.y) + abs(ptNewEnd.y))/2 -5;
				m_stFigure[i].rcHit[7].bottom = (abs(ptNewBegin.y) + abs(ptNewEnd.y))/2 +5;*/
				m_stFigure[i].rcHit[7].top	  = ((ptNewBegin.y) + (ptNewEnd.y))/2 -5;
				m_stFigure[i].rcHit[7].bottom = ((ptNewBegin.y) + (ptNewEnd.y))/2 +5;
				//dc.Rectangle(m_stFigure[i].rcHit[7]);
				memDC.Rectangle(m_stFigure[i].rcHit[7]);
			}
			if (m_bModifyFigure && m_stFigure[i].isLine && (i == m_nCurrFigure))
			{
				m_stFigure[i].rcHit[0].left   = ptNewBegin.x -5;
				m_stFigure[i].rcHit[0].right  = ptNewBegin.x +5;
				m_stFigure[i].rcHit[0].top	  = ptNewBegin.y -5;
				m_stFigure[i].rcHit[0].bottom = ptNewBegin.y +5;

				/*dc.SelectStockObject(WHITE_BRUSH);
				dc.Rectangle(m_stFigure[i].rcHit[0]);*/
				memDC.SelectStockObject(WHITE_BRUSH);
				memDC.Rectangle(m_stFigure[i].rcHit[0]);

				m_stFigure[i].rcHit[2].left   = ptNewEnd.x -5;
				m_stFigure[i].rcHit[2].right  = ptNewEnd.x +5;
				m_stFigure[i].rcHit[2].top	  = ptNewEnd.y -5;
				m_stFigure[i].rcHit[2].bottom = ptNewEnd.y +5;
				//dc.Rectangle(m_stFigure[i].rcHit[2]);
				memDC.Rectangle(m_stFigure[i].rcHit[2]);
			}
			/*dc.SelectObject(pOldPen);*/
			memDC.SelectObject(pOldPen);
			FigurePen.DeleteObject();
		}

		for (int i=0; i<MAX_DRAW; i++)
		{
			if (!m_stFigureText[i].bDraw) continue;			

			int x =int(m_stFigureText[i].ptBegin.x * fH- m_ptResult.x * fH);
			int y = int(m_stFigureText[i].ptBegin.y * fV- m_ptResult.y * fV);

			memDC.SetBkMode(TRANSPARENT);
			memDC.SetTextColor(m_stFigureText[i].clrColor);
			memDC.TextOut(x, y, m_stFigureText[i].textString);			
		}	
	}

	CPen *pOldPen;

	int centerX = m_nWidth /2*m_fMagnification;
	int centerY = m_nHeight/2*m_fMagnification;
	if (m_bDrawCrossLine) 
	{
		//두 포인트를 이용하여 선을 그린다.
		CPen ScalePen,ScalePenBig;
		ScalePen.CreatePen(PS_DOT,1,RGB(0, 255, 255));
		//pOldPen = (CPen*)dc.SelectObject(&ScalePen);
		pOldPen = (CPen*)memDC.SelectObject(&ScalePen);

		if (m_bDrawFitImage)
		{
			/*dc.SetBkMode(TRANSPARENT);
			dc.MoveTo(0, rect.Height()/2);
			dc.LineTo(rect.Width(), rect.Height()/2);

			dc.MoveTo(rect.Width()/2, 0);
			dc.LineTo(rect.Width()/2, rect.Height());*/
			memDC.SetBkMode(TRANSPARENT);
			memDC.MoveTo(0, rect.Height()/2);
			memDC.LineTo(rect.Width(), rect.Height()/2);
			memDC.MoveTo(rect.Width()/2, 0);
			memDC.LineTo(rect.Width()/2, rect.Height());

			if(m_bDrawAlignTargetLine)
			{
				float fH = (float)rect.Width () / m_nWidth;
				float fV = (float)rect.Height() / m_nHeight;

				for(int i=0;i<m_AlignTargetLine.size();i++)
				{
					memDC.MoveTo(m_AlignTargetLine.at(i).x*fH, 0);
					memDC.LineTo(m_AlignTargetLine.at(i).x*fH, rect.Height());
				}
			}
		}
		else
		{
			/*dc.SetBkMode(TRANSPARENT);
			dc.MoveTo(0, (m_nHeight*m_fMagnification)/2 - m_ptResult.y*m_fMagnification);
			dc.LineTo(m_nWidth*m_fMagnification, (m_nHeight*m_fMagnification)/2 - m_ptResult.y*m_fMagnification);

			dc.MoveTo((m_nWidth*m_fMagnification)/2 - m_ptResult.x*m_fMagnification, 0);
			dc.LineTo((m_nWidth*m_fMagnification)/2 - m_ptResult.x*m_fMagnification, m_nHeight*m_fMagnification);*/
			memDC.SetBkMode(TRANSPARENT);
			memDC.MoveTo(0, (m_nHeight*m_fMagnification)/2 - m_ptResult.y*m_fMagnification);
			memDC.LineTo(m_nWidth*m_fMagnification, (m_nHeight*m_fMagnification)/2 - m_ptResult.y*m_fMagnification);
			memDC.MoveTo((m_nWidth*m_fMagnification)/2 - m_ptResult.x*m_fMagnification, 0);
			memDC.LineTo((m_nWidth*m_fMagnification)/2 - m_ptResult.x*m_fMagnification, m_nHeight*m_fMagnification);

			if(m_bDrawAlignTargetLine)
			{
				for(int i=0;i<m_AlignTargetLine.size();i++)
				{
					memDC.MoveTo(m_AlignTargetLine.at(i).x*m_fMagnification, 0);
					memDC.LineTo(m_AlignTargetLine.at(i).x*m_fMagnification, rect.Height());
				}
			}
		}

		//dc.SelectObject(pOldPen);
		memDC.SelectObject(pOldPen);
		ScalePen.DeleteObject();
	}

	
	if (m_bNavi)
	{
		CBrush brsFrame(RGB(255,0,0));

		//dc.SetStretchBltMode(COLORONCOLOR);
		memDC.SetStretchBltMode(COLORONCOLOR);
		int w = rect.Width()/4;
		int h = rect.Height()/4;
		int orgx = rect.Width()/2-w/2;
		int orgy = rect.Height()-h-20;
		float fH = (float)w/m_nWidth;
		float fV = (float)h/m_nHeight;
		
		//m_ImgSrc.AlphaBlend(dc.m_hDC, rect.Width()/2-w/2, rect.Height()-h-20,w,h,0,0,m_nWidth,m_nHeight,100);
		m_ImgSrc.AlphaBlend(memDC.m_hDC, rect.Width()/2-w/2, rect.Height()-h-20,w,h,0,0,m_nWidth,m_nHeight,100);

		if (m_bDrawFitImage)
		{
			CRect rc;
			rc.left = orgx;
			rc.top = orgy;
			rc.right = rc.left + w;
			rc.bottom = rc.top + h;
			memDC.FrameRect(rc,&brsFrame);
		}
		else
		{
			CRect rc;
			rc.left = orgx + m_ptResult.x*fH;
			rc.top =  orgy + m_ptResult.y*fV;

			if (m_nWidth*m_fMagnification < rect.Width())
				rc.right = rc.left + w;
			else
				rc.right = rc.left + rect.Width() / m_fMagnification*fH;

			if (m_nHeight*m_fMagnification < rect.Height())
				rc.bottom = rc.top + h;
			else
				rc.bottom = rc.top + rect.Height() / m_fMagnification * fV;

			//dc.FrameRect(rc,&brsFrame);
			memDC.FrameRect(rc,&brsFrame);
		}
	}

	if( m_bPatternLine ) 
	{
		//두 포인트를 이용하여 선을 그린다.
		CPen ScalePen,ScalePenBig;
		ScalePen.CreatePen(PS_DOT, 1, RGB(0, 255, 0));
		pOldPen = (CPen*)memDC.SelectObject(&ScalePen);

		if (m_bDrawFitImage)
		{
			memDC.SetBkMode(TRANSPARENT);
			memDC.MoveTo(0, m_ptPatternPos.y);
			memDC.LineTo(rect.Width(), m_ptPatternPos.y);
			memDC.MoveTo(m_ptPatternPos.x, 0);
			memDC.LineTo(m_ptPatternPos.x, rect.Height());

			/*
			memDC.MoveTo(0, rect.Height()/2);
			memDC.LineTo(rect.Width(), rect.Height()/2);
			memDC.MoveTo(rect.Width()/2, 0);
			memDC.LineTo(rect.Width()/2, rect.Height());
			*/
		}
		else
		{
			memDC.SetBkMode(TRANSPARENT);
			memDC.MoveTo(0, (m_nHeight*m_fMagnification)/2 - m_ptResult.y*m_fMagnification);
			memDC.LineTo(m_nWidth*m_fMagnification, (m_nHeight*m_fMagnification)/2 - m_ptResult.y*m_fMagnification);
			memDC.MoveTo((m_nWidth*m_fMagnification)/2 - m_ptResult.x*m_fMagnification, 0);
			memDC.LineTo((m_nWidth*m_fMagnification)/2 - m_ptResult.x*m_fMagnification, m_nHeight*m_fMagnification);
		}

		memDC.SelectObject(pOldPen);
		ScalePen.DeleteObject();
	}

	//dc.SelectClipRgn(NULL);
	memDC.SelectClipRgn(NULL);
	
	
			

	if (m_bDrawFitImage)
	{
		if( m_bUseMask )
			TransparentBlt(memDC.m_hDC,0,0,(int)rect.Width(), (int)rect.Height(),	m_MaskOverlayMemDC.m_hDC, 0,0,(int)m_nWidth, (int)m_nHeight,RGB(0,0,0));

		TransparentBlt(memDC.m_hDC, 0, 0, (int)rect.Width(), (int)rect.Height(), m_OverlayMemDC.m_hDC, 0,0,(int)m_nWidth, (int)m_nHeight, RGB(0, 0, 0));

		if( m_bUseMask && m_bMaskRect )
			TransparentBlt(memDC.m_hDC,0,0,(int)rect.Width(), (int)rect.Height(),	m_RectOverlayMemDC.m_hDC, 0,0,(int)m_nWidth, (int)m_nHeight,RGB(0,0,0));
	}
		//TransparentBlt(memDC.m_hDC, 0, 0, (int)m_nWidth, (int)m_nHeight, m_OverlayMemDC.m_hDC, 0,0,(int)rect.Width(), (int)rect.Height(), RGB(0, 0, 0));
	else
	{
		m_ptResult.x;
		m_ptResult.y;
		TransparentBlt(memDC.m_hDC, -m_ptResult.x, -m_ptResult.y, (int)m_nWidth, (int)m_nHeight, m_OverlayMemDC.m_hDC, 0, 0, (int)(m_nWidth * m_fMagnification), (int)(m_nHeight * m_fMagnification), RGB(0, 0, 0));

		//TransparentBlt(memDC.m_hDC, 0, 0, (int)m_nWidth, (int)m_nHeight, m_OverlayMemDC.m_hDC, 0,0,(int)(m_nWidth * m_fMagnification), (int)(m_nHeight * m_fMagnification), RGB(0, 0, 0));
	}

	


//	dc.SetStretchBltMode(COLORONCOLOR);
	//////////////////////////////////////////////////////////////////////////
	dc.BitBlt(0, 0, rect.Width(), rect.Height(), &memDC, 0, 0, SRCCOPY);

	if (m_bScroll)
	{
		CBrush brs(RGB(41,41,41));
		dc.FillRect(CRect(rect.right, rect.bottom, rect.right+SIZE_SCROLLBAR, rect.bottom+SIZE_SCROLLBAR),&brs);
	}

	memDC.SelectObject(pOldBitmap);
	memDC.DeleteDC();
	bitmap.DeleteObject();
	//////////////////////////////////////////////////////////////////////////
}

void CViewerEx::OnLButtonUp(UINT nFlags, CPoint point)
{
	CRect rect;
	GetClientRect(&rect);

	if (m_bScroll)
	{
		rect.right -= SIZE_SCROLLBAR;
		rect.bottom -= SIZE_SCROLLBAR;
	}

	if(point.x < 0 || point.x > rect.Width() || point.y <0 || point.y > rect.Height())
		return;

	if (m_bDrawFitImage)
	{
		float fH = (float)rect.Width () / m_nWidth;
		float fV = (float)rect.Height() / m_nHeight;

		point.x /= fH;
		point.y /= fV;
	}
	else
	{
		point.x /= m_fMagnification;
		point.y /= m_fMagnification;
	}

	if (m_bDrawingFigure)
	{
	//	m_bDrawingFigure = false;

		NMHDR nmHdr;
		::ZeroMemory(&nmHdr, sizeof(NMHDR));

		nmHdr.hwndFrom = m_hWnd;
		nmHdr.idFrom = GetDlgCtrlID();
		nmHdr.code = WM_DRAWING_FINISH_MESSAGE;
		GetParent()->SendMessage(WM_NOTIFY, (LPARAM)NULL, (WPARAM)&nmHdr);
	}

	
	m_ptLup = m_ptResult;

	if (m_bModifyFigure)
	{
		m_rcPreFigure.left	 = m_stFigure[m_nCurrFigure].ptBegin.x;
		m_rcPreFigure.top	 = m_stFigure[m_nCurrFigure].ptBegin.y;
		m_rcPreFigure.right	 = m_stFigure[m_nCurrFigure].ptEnd.x;
		m_rcPreFigure.bottom = m_stFigure[m_nCurrFigure].ptEnd.y;
	}

	if( m_bUseMask && m_bMaskClick )
	{
		m_bMaskClick = FALSE;

		if( m_bMaskRect )
		{
			CRect rect;
			CPoint ptTemp;
			GetClientRect(&rect);

			if( (rect.right - point.x) <= 10 ) point.x = rect.right;
			if( point.x <= 10 ) point.x = 0;

			if( (rect.bottom - point.y) <= 10 ) point.y = rect.bottom;
			if( point.y <= 10 ) point.y = 0;

			CPoint pt1, pt2;

			pt1 = m_ptLDown;
			pt2 = point;

			if( pt2.x < pt1.x )
			{
				ptTemp = pt2;
				pt2 = pt1;
				pt1 = ptTemp;
			}

			DrawRectMask(pt1, pt2);
			Invalidate();
			//loadGrayRaw();
		}
	}

	m_bDrawStart=false;

	CWnd::OnLButtonUp(nFlags, point);
}


void CViewerEx::OnLButtonDown(UINT nFlags, CPoint point)
{
	CRect rect;
	GetClientRect(&rect);

	if (m_bScroll)
	{
		rect.right -= SIZE_SCROLLBAR;
		rect.bottom -= SIZE_SCROLLBAR;
	}

	if(point.x < 0 || point.x > rect.Width() || point.y <0 || point.y > rect.Height())
		return;

	if (m_bDrawFitImage)
	{
		float fH = (float)rect.Width () / m_nWidth;
		float fV = (float)rect.Height() / m_nHeight;

		point.x /= fH;
		point.y /= fV;
	}
	else
	{
		point.x /= m_fMagnification;
		point.y /= m_fMagnification;
	}

	m_ptLDown = point;

	if ( m_bDrawingFigure && !m_isMoving && !m_isHitNW && !m_isHitN  && !m_isHitNE && !m_isHitE  && !m_isHitSE && !m_isHitS && !m_isHitSW && !m_isHitW) 
	{
		m_stFigure[m_nCurrFigure].ptBegin = m_ptLDown + m_ptResult;
		m_bDrawStart=true;
	}
	else m_bDrawStart=false;


	if( m_bUseMask && m_bMaskClick == FALSE )
	{
		m_bMaskClick = TRUE;

		if( m_bMaskPen ) SetMaskPos(point);

	//	if( m_bMaskRect ) m_ptStart = point;

		Invalidate();
	}

	CWnd::OnLButtonDown(nFlags, point);
}

void CViewerEx::OnRButtonUp(UINT nFlags, CPoint point)
{
	CWnd::OnRButtonUp(nFlags, point);
}


void CViewerEx::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
{
	if (m_bMenu)
		m_pMenu->TrackPopupMenu(TPM_RIGHTBUTTON | TPM_LEFTALIGN, point.x, point.y, this);
}

void CViewerEx::CreateMousePopupMenu()
{
	m_pSubZoom = new CMenu();
	m_pSubZoom->CreatePopupMenu();
	m_pSubZoom->InsertMenu(-1,MF_BYPOSITION, ID_MENU_ZOOM_X0125 , _T("x0.125"	));
	m_pSubZoom->InsertMenu(-1,MF_BYPOSITION, ID_MENU_ZOOM_X025	 , _T("x0.25"	));
	m_pSubZoom->InsertMenu(-1,MF_BYPOSITION, ID_MENU_ZOOM_X05	 , _T("x0.5"	));
	m_pSubZoom->InsertMenu(-1,MF_BYPOSITION, ID_MENU_ZOOM_X1	 , _T("x1"		));
	m_pSubZoom->InsertMenu(-1,MF_BYPOSITION, ID_MENU_ZOOM_X2	 , _T("x2"		));
	m_pSubZoom->InsertMenu(-1,MF_BYPOSITION, ID_MENU_ZOOM_X4	 , _T("x4"		));
	m_pSubZoom->InsertMenu(-1,MF_BYPOSITION, ID_MENU_ZOOM_X8	 , _T("x8"		));
	m_pSubZoom->InsertMenu(-1,MF_BYPOSITION, ID_MENU_ZOOM_X16	 , _T("x16"		));
	m_pSubZoom->InsertMenu(-1,MF_BYPOSITION, ID_MENU_ZOOM_X32	 , _T("x32"		));

	m_pSubMode = new CMenu();
	m_pSubMode->CreatePopupMenu();
	m_pSubMode->InsertMenu(-1,MF_BYPOSITION, ID_MENU_MOUSE , _T("Mouse"));
	m_pSubMode->InsertMenu(-1,MF_BYPOSITION, ID_MENU_SCROLL, _T("Scroll"));

	m_pMenu = new CMenu();
	m_pMenu->CreatePopupMenu();
	m_pMenu->InsertMenu(-1,MF_BYPOSITION, ID_MENU_SAVE, _T("Save..."));
	m_pMenu->InsertMenu(-1,MF_BYPOSITION, ID_MENU_LOAD, _T("Load..."));
	m_pMenu->AppendMenu(MF_POPUP,(UINT)m_pSubZoom->m_hMenu, _T("Zoom"));
	m_pMenu->InsertMenu(-1,MF_BYPOSITION, ID_MENU_FIT, _T("Fit Image"));
	m_pMenu->InsertMenu(-1,MF_BYPOSITION, ID_MENU_NEVIGATION, _T("Navigation"));
	m_pMenu->AppendMenu(MF_POPUP,(UINT)m_pSubMode->m_hMenu, _T("Mode"));

	m_pSubZoom->CheckMenuItem  (ID_MENU_ZOOM_X1	, MF_CHECKED);		//x1
	m_pSubMode->CheckMenuItem  (ID_MENU_MOUSE	, MF_CHECKED);		//Mouse
	m_pMenu->EnableMenuItem(ID_MENU_SAVE, MF_DISABLED);			//Save
	m_pMenu->EnableMenuItem(ID_MENU_FIT , MF_DISABLED);			//Fit
	m_pMenu->EnableMenuItem(ID_MENU_NEVIGATION, MF_DISABLED);			//Navigation
	m_pMenu->EnableMenuItem((UINT)m_pSubZoom->m_hMenu, MF_DISABLED);

	CheckMenuItem(ID_MENU_ZOOM_X1,false);
	CheckMenuItem(ID_MENU_MOUSE,true);
}

void CViewerEx::OnFitImage()
{
	m_bDrawFitImage = true;

	CheckMenuItem(ID_MENU_FIT,false);
	
	m_wndSBH.EnableWindow(FALSE);
	m_wndSBV.EnableWindow(FALSE);

	Invalidate();
	return;
}

void CViewerEx::OnNavigation()
{
	if (m_bNavi == false)
	{
		m_bNavi = true;
		m_pMenu->CheckMenuItem(ID_MENU_NEVIGATION, MF_CHECKED);
	}
	else
	{
		m_bNavi = false;
		m_pMenu->CheckMenuItem(ID_MENU_NEVIGATION, MF_UNCHECKED);
	}

	Invalidate();
	return;
}

void CViewerEx::OnSaveImage()
{
	if (m_ImgSrc.IsNull()) return;

	CString strFilter = _T("All Files (*.*)|*.*||");

	CFileDialog FileDlg(FALSE, _T(".bmp"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, strFilter);
	if( FileDlg.DoModal() == IDOK )
	{
		m_ImgSrc.Save(FileDlg.GetPathName());
	}
}

void CViewerEx::OnLoadImage()
{
	CString strFilter = _T("All Files (*.*)|*.*||");

	CFileDialog FileDlg(TRUE, _T(".BMP"), NULL, 0, strFilter);
	
	if( FileDlg.DoModal() == IDOK )
	{
		OnLoad(FileDlg.GetPathName());

		NMHDR nmHdr;
		::ZeroMemory(&nmHdr, sizeof(NMHDR));

		nmHdr.hwndFrom = m_hWnd;
		nmHdr.idFrom = GetDlgCtrlID();
		nmHdr.code = WM_LOAD_IMAGE_MESSAGE;
		GetParent()->SendMessage(WM_NOTIFY, (LPARAM)NULL, (WPARAM)&nmHdr);
	}
}

void CViewerEx::OnSetZoom(UINT wParam)
{
	int nMenuID = (int)wParam;
	switch(nMenuID)
	{
	case ID_MENU_ZOOM_X0125:
		m_fMagnification=0.125;
		break;
	case ID_MENU_ZOOM_X025:
		m_fMagnification=0.25;
		break;
	case ID_MENU_ZOOM_X05:
		m_fMagnification=0.5;
		break;
	case ID_MENU_ZOOM_X1:
		m_fMagnification=1;
		break;
	case ID_MENU_ZOOM_X2:
		m_fMagnification=2;
		break;
	case ID_MENU_ZOOM_X4:
		m_fMagnification=4;
		break;
	case ID_MENU_ZOOM_X8:
		m_fMagnification=8;
		break;
	case ID_MENU_ZOOM_X16:
		m_fMagnification=16;
		break;
	case ID_MENU_ZOOM_X32:
		m_fMagnification=32;
		break;
	}

	m_bDrawFitImage = false;

	CheckMenuItem(nMenuID,false);
	m_bClear = true;

	ResetScrollbarInfo();

	Invalidate();
}

void CViewerEx::OnSetMode(UINT wParam)
{
	int nMenuID = (int)wParam;
	switch(nMenuID)
	{
		// Mouse
	case ID_MENU_MOUSE:
		m_bScroll = false;	
		m_wndSBH.ShowWindow(SW_HIDE);
		m_wndSBV.ShowWindow(SW_HIDE);

		//Mouse <-> Scroll 변환 후 이미지 이동시 보정하는 부분.
		m_ptLup = m_ptResult;
		m_ptLDown = m_ptResult;
		break;
		// Scroll
	case ID_MENU_SCROLL:
		m_bScroll = true;
		m_wndSBH.ShowWindow(SW_SHOW);
		m_wndSBV.ShowWindow(SW_SHOW);

		//Mouse <-> Scroll 변환 후 이미지 이동시 보정하는 부분.
		ResetScrollbarInfo();
		break;
	}

	CheckMenuItem(nMenuID,true);
	m_bClear = true;

	Invalidate();
}

void CViewerEx::CheckMenuItem(int nID, bool bMode)
{
	if (bMode)
	{
		for (int i=ID_MENU_MOUSE; i<ID_MENU_MAX; i++)
		{
			if (i == nID)
				m_pSubMode->CheckMenuItem(i, MF_CHECKED);
			else
				m_pSubMode->CheckMenuItem(i, MF_UNCHECKED);
		}
	}
	else
	{
		for (int i=ID_MENU_ZOOM_X0125; i<ID_MENU_FIT+1; i++)
		{
			if (i < ID_MENU_ZOOM_MAX)
			{
				if (i == nID)
					m_pSubZoom->CheckMenuItem(i, MF_CHECKED);
				else
					m_pSubZoom->CheckMenuItem(i, MF_UNCHECKED);
			}
			else
			{
				if (i == nID)
					m_pMenu->CheckMenuItem(i, MF_CHECKED);
				else
					m_pMenu->CheckMenuItem(i, MF_UNCHECKED);
			}
		}
	}
}

void CViewerEx::SetEnableFigureLine(int iIndex)
{
	if (iIndex<0 || iIndex>=MAX_DRAW) return;

	m_stFigure[iIndex].isLine		= true;
	m_stFigure[iIndex].isRectangle	= false;
	m_stFigure[iIndex].isCircle		= false;
}

void CViewerEx::SetEnableFigureRectangle(int iIndex)
{
	if(iIndex < 0 || iIndex >= MAX_DRAW) return;

	m_stFigure[iIndex].isLine		= false;
	m_stFigure[iIndex].isRectangle	= true;
	m_stFigure[iIndex].isCircle		= false;
}

void CViewerEx::SetEnableFigureCircle(int iIndex)
{
	if(iIndex < 0 || iIndex >= MAX_DRAW) return;

	m_stFigure[iIndex].isLine		= false;
	m_stFigure[iIndex].isRectangle	= false;
	m_stFigure[iIndex].isCircle		= true;
}

void CViewerEx::SetEnableModifyFigure(int iIndex, bool isEnable) 
{ 
	if (iIndex < 0 || iIndex >= MAX_DRAW) return; 

	m_nCurrFigure = iIndex; 
	m_bModifyFigure = isEnable; 
	
	m_rcPreFigure.left   = m_stFigure[m_nCurrFigure].ptBegin.x;
	m_rcPreFigure.top    = m_stFigure[m_nCurrFigure].ptBegin.y;
	m_rcPreFigure.right  = m_stFigure[m_nCurrFigure].ptEnd.x;
	m_rcPreFigure.bottom = m_stFigure[m_nCurrFigure].ptEnd.y;

	Invalidate();
}

void CViewerEx::ResetScrollbarInfo()
{
	CRect rect;
	GetWindowRect(rect);

	//rect.right -= SIZE_SCROLLBAR;
	//rect.bottom -= SIZE_SCROLLBAR;

	//int nScrollBarSize = m_nWidth*m_fMagnification - rect.Width();
	int nScrollBarSize = m_nWidth*m_fMagnification - rect.Width() + SIZE_SCROLLBAR;
	nScrollBarSize = nScrollBarSize > 0 ? nScrollBarSize : 0;

	SCROLLINFO si;
	si.fMask = SIF_ALL;
	si.nPage = nScrollBarSize/10;
	//si.nMax = nScrollBarSize + 34 + si.nPage;
	si.nMax = nScrollBarSize;//+ si.nPage;
	si.nMin = 0;
	si.nTrackPos = 0;
	si.nPos = m_ptResult.x*m_fMagnification;

	m_wndSBH.SetScrollRange(0,si.nMax);
	m_wndSBH.SetScrollPos(si.nPos);

	//nScrollBarSize = m_nHeight*m_fMagnification - rect.Height();
	nScrollBarSize = m_nHeight*m_fMagnification - rect.Height() + SIZE_SCROLLBAR;
	nScrollBarSize = nScrollBarSize > 0 ? nScrollBarSize : 0;

	si.fMask = SIF_ALL;
	si.nPage = nScrollBarSize/10;
	//si.nMax = nScrollBarSize + 34 + si.nPage;	
	si.nMax = nScrollBarSize;// + si.nPage;	
	si.nMin = 0;
	si.nTrackPos = 0;
	si.nPos = m_ptResult.y*m_fMagnification;

	m_wndSBV.SetScrollRange(0,si.nMax);
	m_wndSBV.SetScrollPos(si.nPos);

	m_nWidth  * m_fMagnification < rect.Width()  ? m_wndSBH.EnableWindow(FALSE) : m_wndSBH.EnableWindow(TRUE);
	m_nHeight * m_fMagnification < rect.Height() ? m_wndSBV.EnableWindow(FALSE) : m_wndSBV.EnableWindow(TRUE);

	if (m_bDrawFitImage)
	{
		m_wndSBH.EnableWindow(FALSE);
		m_wndSBV.EnableWindow(FALSE);
	}
}

void CViewerEx::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar->m_hWnd == m_wndSBH.m_hWnd)
	{
		int pos = m_wndSBH.GetScrollPos();
		m_ptResult.x = pos;
		m_ptResult.x /= m_fMagnification;
		Invalidate();
	}

	CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CViewerEx::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	if (pScrollBar->m_hWnd == m_wndSBV.m_hWnd)
	{
		int pos = m_wndSBV.GetScrollPos();
		m_ptResult.y = pos;
		m_ptResult.y /= m_fMagnification;
		Invalidate();
	}

	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
}

// 두 점 사이의 거리 계산.
float CViewerEx::CalculateLength(CPoint pt1, CPoint pt2)
{
	CRect rect;
	GetClientRect(rect);
	
	float a=0.0, b=0.0, c=0.0;
	if (m_bDrawFitImage)
	{
		float fX = (float)rect.Width()/m_nWidth;
		float fY = (float)rect.Height()/m_nHeight;
		pt1.x /= fX;
		pt1.y /= fY;
		pt2.x /= fX;
		pt2.y /= fY;
	}
	else
	{
		pt1.x /= m_fMagnification;
		pt1.y /= m_fMagnification;
		pt2.x /= m_fMagnification;
		pt2.y /= m_fMagnification;
	}
	
	a = (float)(pt1.x - pt2.x);
	b = (float)(pt1.y - pt2.y);
	c = sqrt(a*a+b*b);

	return c;
}

void CViewerEx::SetEnableMenuSave(bool bEnable)
{
	if (bEnable)
		m_pMenu->EnableMenuItem(ID_MENU_SAVE, MF_ENABLED);
	else
		m_pMenu->EnableMenuItem(ID_MENU_SAVE, MF_DISABLED);
}

void CViewerEx::SetEnableMenuLoad(bool bEnable)
{
	if (bEnable)
		m_pMenu->EnableMenuItem(ID_MENU_LOAD, MF_ENABLED);
	else
		m_pMenu->EnableMenuItem(ID_MENU_LOAD, MF_DISABLED);
}

void CViewerEx::SetSizeRectangle(int iIndex, CPoint pt1, CPoint pt2)
{
	if (iIndex<0 || iIndex>=MAX_DRAW) return;

	if (m_stFigure[iIndex].isRectangle == false) return;

	m_stFigure[iIndex].ptBegin = pt1;
	m_stFigure[iIndex].ptEnd   = pt2;
}

void CViewerEx::SetSizeRectangle(int iIndex, int nOrgX, int nOrgY, int nWidth, int nHeight)
{
	if (iIndex<0 || iIndex>=MAX_DRAW) return;

	if (m_stFigure[iIndex].isRectangle == false) return;

	m_stFigure[iIndex].ptBegin = CPoint(nOrgX,nOrgY);
	m_stFigure[iIndex].ptEnd   = CPoint(nOrgX+nWidth-1,nOrgY+nHeight-1);
}

int CViewerEx::GetValueY(int nPosX, int nPosY)
{
	if (m_ImgSrc.IsNull()) return 0;

	if (nPosX < 0 || nPosX >= m_nWidth)  return 0;
	if (nPosY < 0 || nPosY >= m_nHeight) return 0;

	BYTE* ptr = (BYTE*)m_ImgSrc.GetPixelAddress(nPosX,nPosY);

	int value = *ptr;

	return value;
}

COLORREF CViewerEx::GetValueRGB(int nPosX, int nPosY)
{
	if (m_ImgSrc.IsNull() == true) return RGB(0,0,0);
	if (nPosX < 0 || nPosX >= m_nWidth)  return 0;
	if (nPosY < 0 || nPosY >= m_nHeight) return 0;

	COLORREF clr = *((COLORREF*)m_ImgSrc.GetPixelAddress(nPosX,nPosY));

	//BYTE r = GetBValue(clr);
	//BYTE g = GetGValue(clr);
	//BYTE b = GetRValue(clr);

	return clr;
}

void CViewerEx::OnDropFiles(HDROP hDropInfo)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	int nFiles,n=0;		
	char m_sFilePath[MAX_PATH];
	CString path;

	nFiles = ::DragQueryFileA( hDropInfo, 0xFFFFFFFF, (LPSTR)(LPCTSTR)m_sFilePath, MAX_PATH );	

	for(int i = 0; i < nFiles; i++)
	{
		::DragQueryFileA( hDropInfo, i, (LPSTR)(LPCTSTR) m_sFilePath, MAX_PATH);
		path = m_sFilePath;

		if( path.Find(".jpg") > 0 || path.Find(".bmp") > 0 || path.Find(".JPG") > 0 || path.Find(".BMP") > 0 ||
			path.Find(".png") > 0 || path.Find(".PNG") > 0 )
		{
			OnLoad((LPCTSTR)m_sFilePath);

			NMHDR nmHdr;
			::ZeroMemory(&nmHdr, sizeof(NMHDR));

			nmHdr.hwndFrom = m_hWnd;
			nmHdr.idFrom = GetDlgCtrlID();
			nmHdr.code = WM_LOAD_IMAGE_MESSAGE;
			GetParent()->SendMessage(WM_NOTIFY, (LPARAM)NULL, (WPARAM)&nmHdr);
			n++;
		}
		else
			AfxMessageBox("BMP 또는 JPG 파일만 가능 합니다.");
	}


	Invalidate();

	CWnd::OnDropFiles(hDropInfo);
}


void CViewerEx::SetMaskPos(CPoint point)
{
	CDC *pDC = getMaskOverlayDC();
	CPen pen( PS_SOLID, 1, RGB(255, 0, 0));
	CBrush br(RGB(255, 0, 0)), *oldBr;
	CPen *oldPen;
	
	

	oldPen = pDC->SelectObject( &pen );
	oldBr = pDC->SelectObject(&br);
	
	int nSize = 10;
	pDC->Ellipse((int)(point.x / (double)m_fMagnification + 0.5) - nSize, (int)(point.y / (double)m_fMagnification + 0.5) - nSize, 
		(int)(point.x / (double)m_fMagnification + 0.5) + nSize, (int)(point.y / (double)m_fMagnification + 0.5) + nSize);

	pDC->SelectObject(oldBr);
	pDC->SelectObject( oldPen );
}



BOOL CViewerEx::DrawRect(CPoint pt1, CPoint pt2)
{
	BOOL bSuccess = TRUE;
	ClearRectMaskOverlayDC();
	CDC * pDC = getRectMaskOverlayDC();
	
	CPen pen(PS_DOT, 1, RGB(0, 0, 255)), *oldPen;
	CBrush br(NULL_BRUSH), *oldBr;
	oldPen = pDC->SelectObject( &pen );
	//oldBr = pDC->SelectObject( &br );
	oldBr = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);
	pDC->Rectangle((int)(pt1.x / (double)m_fMagnification + 0.5), (int)(pt1.y / (double)m_fMagnification + 0.5), 
		(int)(pt2.x / (double)m_fMagnification + 0.5), (int)(pt2.y / (double)m_fMagnification + 0.5));

	pDC->SelectObject( oldPen );
	pDC->SelectObject( oldBr );

	return bSuccess;
}



void CViewerEx::DrawRectMask(CPoint pt1, CPoint pt2)
{
	ClearRectMaskOverlayDC();

	CDC *pDC = getMaskOverlayDC();
	
	CBrush br(RGB(255, 0, 0)), *oldBr;
	CPen *oldPen;
	oldPen = (CPen*)pDC->SelectStockObject(NULL_PEN);
	oldBr = pDC->SelectObject( &br );

	pDC->Rectangle((int)(pt1.x / (double)m_fMagnification + 0.5) - 5, (int)(pt1.y / (double)m_fMagnification + 0.5) - 5, 
		(int)(pt2.x / (double)m_fMagnification + 0.5) + 5, (int)(pt2.y / (double)m_fMagnification + 0.5) + 5);

	pDC->SelectObject( oldBr );
	pDC->SelectObject( oldPen );

}
