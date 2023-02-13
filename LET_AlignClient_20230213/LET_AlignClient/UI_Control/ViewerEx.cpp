//// CumstomPicture.cpp: implementation file
////
//
#include "stdafx.h"
//#include "ViewerEx.h"
//#include <math.h>
//#include "Caliper\DlgCaliper.h"
//#include "..\FastMemcpy.h"
//
///*
//// 메모리 DC의 비트맵을 현재 DC에 고속 복사
//BOOL BitBlt(
//	int x,			// Bitmap이 출력될 클라이언트 윈도우 기준 좌표
//	int y,
//	int nWidth,		// 출력될 Bitmap의 폭과 높이
//	int nHeight,	
//	CDC *pSrcDC,	// 메모리 DC
//	int xSrc,		// 메모리 DC의 비트맵을 어느 좌표부터 출력할 건지
//	int ySrc,
//	DWORD dwRop		// 출력 스타일 래스터 오퍼레이션 코드
//	};
//*/
//
///*
//// 이미지를 확대하거나 축소하여 출력.
//BOOL StretchBlt(
//	int x,			// 비트맵이 출력될 클라이언트 윈도우 기준 좌표
//	int y,
//	int nWidth,		// 출력될 비트맵의 폭과 높이
//	int nHeight,
//	CDC *pSrcDC,	// 메모리 DC
//	int xSrc,		// 출력할 비트맵을 어디서(기준 좌표) 부터
//	int ySrc,
//	int nSrcWidth,	// 얼마(크기)만큼 얻어올 것인지 지정
//	int nSrcHeight,
//	DWORD dwRop		// ROP 코드
//);
//*/
//
//
//// CViewerEx
//#ifdef _DEBUG
//#define new DEBUG_NEW
//#endif
//IMPLEMENT_DYNAMIC(CViewerEx, CWnd)
//
//
//void CalcRotate(double x1, double y1, double cx,double cy,double rangle, double *x2, double *y2)
//{
//	double tmpx,tmpy;
//	tmpx = x1 - cx;
//	tmpy = y1 - cy;
//
//	*x2 = (tmpx * cos(rangle) - tmpy * sin(rangle) + cx);
//	*y2 = (tmpx * sin(rangle) + tmpy * cos(rangle) + cy);
//}
//
//CViewerEx::CViewerEx()
//{
//	RegisterWindowClass();
//	m_gdiplusToken = 0;
//	m_pBmp = NULL;
//	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
//	Gdiplus::GdiplusStartup(&m_gdiplusToken, &gdiplusStartupInput, NULL);
//
//	//----- Move Image -----//
//	m_ptResult	= (0,0);
//	m_ptLDown	= (0,0);
//	m_ptLup		= (0,0);
//
//	//----- Image Information -----//
//	m_nWidth		 = 0;
//	m_nHeight		 = 0;
//	m_nBpp			 = 0;
//	m_bClear		 = true;
//	m_fMagnification = 1;
//	m_bInit			 = false;
//	m_bScroll		 = false;
//	m_bNavi			 = false;
//	m_strPath		 = _T("");
//	m_fCameraResolutionX = 1;
//	m_fCameraResolutionY = 1;
//	//m_fCameraResolution = 1;
//
//	//----- Overlay Drawing -----//
//	m_nCurrFigure		= 0;
//	m_bDrawCrossLine	= false;
//	m_bDrawingFigure	= false;
//	m_bDrawAllFigures	= true;
//	m_bDrawFitImage		= false;
//	m_bModifyFigure		= false;
//
//	//----- Mouse Moving -----//
//	m_isHitNW		= false;
//	m_isHitN		= false;
//	m_isHitNE		= false;
//	m_isHitE		= false;
//	m_isHitSE		= false;
//	m_isHitS		= false;
//	m_isHitSW		= false;
//	m_isHitW		= false;
//	m_isMoving		= false;
//
//	m_bDrawRectangleInside = true;
//	//HTK 2022-06-29 Mark View창에 Tracker 기능 추가
//	m_bDrawRectangleInsideText = true;
//	//HTK 2022-04-28 초기 Drag&Drop 기능 활성화
//	m_bDrawStart = false;
//	//----- Menu -----//
//	m_bMenu	= false;
//	m_pMenu	= NULL;
//	m_pSubZoom = NULL;
//	m_pSubMode = NULL;
//
//	//----- All Figures -----//
//	for (int i=0; i<MAX_DRAW; i++)
//	{
//		m_stFigure[i].isDraw		 = false;
//		m_stFigure[i].isLine		 = true;
//		m_stFigure[i].isRectangle	 = false;
//		m_stFigure[i].isCircle		 = false;
//		m_stFigure[i].nWidth		 = 1;
//		m_stFigure[i].ptBegin		 = (0,0);
//		m_stFigure[i].ptEnd			 = (0,0);
//		m_stFigure[i].clrColor		 = RGB(220 + 1.5*i,50 + 5*i,20 + 10*i);
//		m_stFigure[i].fLength		 = 0.0;
//		m_stFigure[i].isDot			 = false;
//		for (int j=0; j<8; j++)
//			m_stFigure[i].rcHit[j]	 = CRect(0,0,0,0);
//
//		m_stFigureText[i].bDraw=false;
//	}
//
//	m_clrRGB = 0;
//	m_nY	 = 0;
//	m_hWnd = NULL;
//	m_bPatternLine = FALSE;
//
//	m_ptPatternPos.x = -1;
//	m_ptPatternPos.y = -1;
//	m_bDrawOverlay = TRUE;
//	m_bMaskClick = FALSE;
//	m_bUseMask = FALSE;	
//	m_bMaskPen = TRUE;	
//	m_bMaskRect = FALSE;
//	m_bDrawAlignTargetLine=true;
//	m_AlignTargetAngle = 0;
//	m_nDrawAlignLineoffset = 0.0;
//
//	m_bDrawCaliper = FALSE;
//	m_bUseCaliper = FALSE;
//	m_pDlgCaliper = NULL;
//
//	m_bRatioFit = TRUE;
//
//	m_UseSemiTransparent = FALSE;
//	m_bf.BlendOp = AC_SRC_OVER;
//	m_bf.BlendFlags = 0;
//	m_bf.AlphaFormat = AC_SRC_ALPHA;
//	m_bf.SourceConstantAlpha = 255;
//
//	InitializeSRWLock(&g_srwStrlock); // 초기화 동기화 객체 Tkyuha 20211027
//}
//
//CViewerEx::~CViewerEx()
//{
//	if (m_pSubMode != NULL)
//	{
//		delete m_pSubMode;
//		m_pSubMode = NULL;
//	}
//
//	if (m_pSubZoom != NULL)
//	{
//		delete m_pSubZoom;
//		m_pSubZoom = NULL;
//	}
//
//	if (m_pMenu != NULL)
//	{
//		delete m_pMenu;
//		m_pMenu = NULL;
//	}
//	
//	if(m_pBmpInfo8BitGray.Valid()!=NULL)		m_OverlayMemDC.DeleteDC();
//	if(m_pBmpInfo8BitGray.Valid()!=NULL)		m_MaskOverlayMemDC.DeleteDC();
//	if(m_pBmpInfo8BitGray.Valid()!=NULL)		m_RectOverlayMemDC.DeleteDC();
//	if (m_pBmpInfo8BitGray.Valid() != NULL)		m_CaliperMemDC.DeleteDC();
//	//KJH 2022-07-12 View 저장기능 활용을 위한 TotalOverla관련인자 추가
//	if (m_pBmpInfo8BitGray.Valid() != NULL)		m_TotalOverlayMemDC.DeleteDC();
//
//	m_AlignTargetLine.clear();	
//	GdiplusShutdown(m_gdiplusToken);
//
//	
//	m_bUseRefPos = FALSE;
//
//	memset(m_dbRefPosX, 0, sizeof(m_dbRefPosX));
//	memset(m_dbRefPosY, 0, sizeof(m_dbRefPosY));
//}
//
//
//BEGIN_MESSAGE_MAP(CViewerEx, CWnd)
//	ON_WM_MOUSEMOVE()
//	ON_WM_PAINT()
//	ON_WM_LBUTTONUP()
//	ON_WM_LBUTTONDOWN()
//	ON_COMMAND_RANGE(ID_MENU_ZOOM_X0125, ID_MENU_ZOOM_X32, OnSetZoom)
//	ON_COMMAND(ID_MENU_SAVE, OnSaveImage)
//	ON_COMMAND(ID_MENU_LOAD, OnLoadImage)
//	ON_COMMAND(ID_MENU_FIT, OnFitImage)
//	ON_COMMAND(ID_MENU_NEVIGATION, OnNavigation)
//	ON_COMMAND_RANGE(ID_MENU_MOUSE, ID_MENU_SCROLL, OnSetMode)
//    ON_WM_RBUTTONUP()
//    ON_WM_CONTEXTMENU()
//	ON_WM_HSCROLL()
//	ON_WM_VSCROLL()
//	ON_WM_DROPFILES()
//END_MESSAGE_MAP()
//
//void CViewerEx::RegisterWindowClass(void)
//{
//	WNDCLASS wndcls;
//	HINSTANCE hInst = AfxGetInstanceHandle();
//
//	wndcls.style            = CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW;
//	wndcls.lpfnWndProc      = ::DefWindowProc;
//	wndcls.cbClsExtra       = wndcls.cbWndExtra = 0;
//	wndcls.hInstance        = hInst;
//	wndcls.hIcon            = NULL;
//	wndcls.hCursor          = AfxGetApp()->LoadStandardCursor(IDC_ARROW);
//	wndcls.hbrBackground    = (HBRUSH) (COLOR_3DFACE + 1);
//	wndcls.lpszMenuName     = NULL;
//	wndcls.lpszClassName    = VIEWER_WINCLASSNAME;
//
//	AfxRegisterClass(&wndcls);
//}
//
//bool CViewerEx::InitControl(CWnd* pWnd, bool bMenu)
//{
//	if (pWnd == NULL) return false;
//
//	m_bMenu = bMenu;
//
//	CRect rect;
//	GetWindowRect(&rect);
//	pWnd->ScreenToClient(&rect);
//
//	UINT id = GetDlgCtrlID();
//	BOOL ret = DestroyWindow();
//	if (ret == FALSE) return false;
//
//	ret = Create(NULL, NULL, WS_CHILD | WS_VISIBLE | SS_CENTER | SS_NOTIFY, rect, pWnd, id);
//
//	GetClientRect(&rect);
//	rect.top = rect.bottom - SIZE_SCROLLBAR;
//	rect.right -= SIZE_SCROLLBAR;
//	m_wndSBH.CreateContol(this,true,rect,HORIZONTAL_SCROLL_ID);
//
//	GetClientRect(&rect);
//	rect.left = rect.right - SIZE_SCROLLBAR;
//	rect.bottom = rect.bottom - SIZE_SCROLLBAR;
//	m_wndSBV.CreateContol(this,false,rect,VERTICAL_SCROLL_ID);
//
//	m_wndSBH.ShowWindow(SW_HIDE);
//	m_wndSBV.ShowWindow(SW_HIDE);
//
//	ResetScrollbarInfo();	
//
//	return true;
//}
//
//bool CViewerEx::OnLoad(CString path)
//{
//	if (path.IsEmpty()) return false;
//	
//	if (m_ImgSrc.IsNull() == false)
//		m_ImgSrc.Destroy();
//	
//	
//	m_ImgSrc.Load(path);
//	m_strPath = path;
//
//	m_ptResult		 = (0,0);
//	m_ptLDown		 = (0,0);
//	m_ptLup			 = (0,0);
//	m_fMagnification = 1;
//	m_bDrawFitImage	 = true; //m_bDrawFitImage	 = false;
//	m_bDrawCrossLine = false;
//	m_bDrawAllFigures= true;
//
//	m_nWidth = m_ImgSrc.GetWidth();
//	m_nHeight = m_ImgSrc.GetHeight();
//	m_nBpp = m_ImgSrc.GetBPP();
//
//	for (int i = 0; i < MAX_DRAW; i++)
//	{
//		m_stFigure[i].isDraw		 = false;
//		m_stFigure[i].isLine		 = true;
//		m_stFigure[i].isRectangle	 = false;
//		m_stFigure[i].isCircle		 = false;
//		m_stFigure[i].nWidth		 = 1;
//		m_stFigure[i].ptBegin		 = (0,0);
//		m_stFigure[i].ptEnd			 = (0,0);
//		m_stFigure[i].clrColor		 = RGB(220 + 1.5*i,50 + 5*i,20 + 10*i);
//		for (int j=0; j<8; j++)
//			m_stFigure[i].rcHit[j]	 = CRect(0,0,0,0);
//	}
//
//	m_bClear = true;
//
//	m_pMenu->EnableMenuItem(ID_MENU_LOAD, MF_ENABLED);
//	m_pMenu->EnableMenuItem(ID_MENU_SAVE, MF_ENABLED);
//	m_pMenu->EnableMenuItem(ID_MENU_FIT, MF_ENABLED);
//	m_pMenu->EnableMenuItem(ID_MENU_NEVIGATION, MF_ENABLED);
//	m_pMenu->EnableMenuItem((UINT_PTR)(m_pSubZoom->m_hMenu), MF_ENABLED);
//	CheckMenuItem(ID_MENU_ZOOM_X1,false);
//
//	if (m_bScroll)
//		ResetScrollbarInfo();
//
//	Invalidate();
//
//	return true;
//}
//
//bool CViewerEx::OnLoadImageFromPtr(BYTE* pBuffer)
//{
//	if (pBuffer == NULL) return false;
//	// 649 x 484 8bit 기준 약 0.1ms 소요 확인.
//	if (m_bInit == false) return false;		
//	if (m_ImgSrc.IsNull()) return false;
//
//	int lineSize;
//	if (m_nBpp == 8)		lineSize = m_nWidth * 1;
//	else if (m_nBpp == 24)	lineSize = m_nWidth * 1 * 3;
//
//	for ( int i = 0; i < m_nHeight; i++ )
//	{
//		const void* src;
//		void* dst = m_ImgSrc.GetPixelAddress(0, i);
//		if (m_nBpp == 8)		src = pBuffer + (i*m_nWidth);
//		else if (m_nBpp == 24)	src = pBuffer + (i*m_nWidth*3);
//
//		memcpy_fast(dst, src, lineSize);
//	}
//
//	Invalidate(FALSE);
//
//	return true;
//}
//
//BYTE *CViewerEx::GetImaagePtr()
//{
//	if (m_ImgSrc.IsNull()) return NULL;
//
//	int pitch = m_ImgSrc.GetPitch();
//    int height = m_ImgSrc.GetHeight();
//
//    BYTE* pBits = (BYTE*)m_ImgSrc.GetBits();
//    if (pitch < 0)
//        pBits += (pitch *(height -1));
//
//    return pBits;
//}
//
//void CViewerEx::OnInitWithCamera(int nWidth, int nHeight, int nBpp)
//{
//	m_ptResult		 = (0,0);
//	m_ptLDown		 = (0,0);
//	m_ptLup			 = (0,0);
//	m_fMagnification = 1;
//	m_nWidth		 = nWidth;
//	m_nHeight		 = nHeight;
//	m_nBpp			 = nBpp;
//	m_bDrawFitImage	 = true;
//	m_bDrawCrossLine = false;
//	m_bModifyFigure = false;
//	m_bDrawAllFigures = false;
//	m_bDrawingFigure = false;
//
//	if (m_ImgSrc.IsNull() == false)
//		m_ImgSrc.Destroy();
//
//	m_ImgSrc.Create(nWidth,nHeight,nBpp);
//
//	/*if(m_pBmpInfo8BitGray.Valid()!=NULL)		m_OverlayMemDC.DeleteDC();
//	if(m_pBmpInfo8BitGray.Valid()!=NULL)		m_MaskOverlayMemDC.DeleteDC();
//	if(m_pBmpInfo8BitGray.Valid()!=NULL)		m_RectOverlayMemDC.DeleteDC();*/
//
//	if (nBpp == 8)
//	{
//		// Define the color table
//		RGBQUAD* tab = new RGBQUAD[256];
//		for (int i = 0; i < 256; ++i)
//		{
//			tab[i].rgbRed = i;
//			tab[i].rgbGreen = i;
//			tab[i].rgbBlue = i;
//			tab[i].rgbReserved = 0;
//		}
//		m_ImgSrc.SetColorTable(0, 256, tab);
//		delete[] tab;
//	}
//
//	m_pMenu->EnableMenuItem(ID_MENU_SAVE, MF_DISABLED);
//	m_pMenu->EnableMenuItem(ID_MENU_LOAD, MF_ENABLED);
//	m_pMenu->EnableMenuItem(ID_MENU_FIT, MF_ENABLED);
//	m_pMenu->EnableMenuItem(ID_MENU_NEVIGATION, MF_ENABLED);
//	m_pMenu->EnableMenuItem((UINT_PTR)m_pSubZoom->m_hMenu, MF_ENABLED);
//	CheckMenuItem(ID_MENU_ZOOM_X1,false);
//
//	//Invalidate();
//
//	m_bInit	= true;
//	m_bClear = true;
//
//	CDC m_OverlayMemDC_OLD;
//	CDC m_TotalOverlayMemDC_OLD;
//	CDC m_MaskOverlayMemDC_OLD;
//	CDC m_RectOverlayMemDC_OLD;
//	CDC m_CaliperMemDC_OLD;
//
//	if( m_MaskOverlayMemDC.m_hDC )
//	{
//		int nWidth2 = m_MaskOverlayMemDC.GetDeviceCaps(HORZSIZE);
//		int nHeight2 = m_MaskOverlayMemDC.GetDeviceCaps(VERTSIZE);
//
//		CBitmap Bmp; 
//		m_MaskOverlayMemDC_OLD.CreateCompatibleDC(NULL); 
//		Bmp.CreateBitmap(m_nWidth, m_nHeight, m_MaskOverlayMemDC.GetDeviceCaps(PLANES), m_MaskOverlayMemDC.GetDeviceCaps(BITSPIXEL), NULL); 
//		m_MaskOverlayMemDC_OLD.SelectObject(&Bmp); 
//		//m_MaskOverlayMemDC_OLD.BitBlt(0, 0, m_MaskOverlayMemDC.GetDeviceCaps(HORZSIZE), m_MaskOverlayMemDC.GetDeviceCaps(VERTSIZE), &m_MaskOverlayMemDC, m_MaskOverlayMemDC.GetDeviceCaps(HORZSIZE), m_MaskOverlayMemDC.GetDeviceCaps(VERTSIZE), SRCCOPY);
//		
//	}
//
//	if (m_CaliperMemDC.m_hDC)
//	{
//		int nWidth2 = m_CaliperMemDC.GetDeviceCaps(HORZSIZE);
//		int nHeight2 = m_CaliperMemDC.GetDeviceCaps(VERTSIZE);
//
//		CBitmap Bmp;
//		m_CaliperMemDC_OLD.CreateCompatibleDC(NULL);
//		Bmp.CreateBitmap(m_nWidth, m_nHeight, m_CaliperMemDC.GetDeviceCaps(PLANES), m_CaliperMemDC.GetDeviceCaps(BITSPIXEL), NULL);
//		m_CaliperMemDC_OLD.SelectObject(&Bmp);
//	}
//
//
//	/*
//	if( m_RectOverlayMemDC.m_hDC )
//	{
//		int nWidth2 = m_RectOverlayMemDC.GetDeviceCaps(HORZSIZE);
//		int nHeight2 = m_RectOverlayMemDC.GetDeviceCaps(VERTSIZE);
//		m_RectOverlayMemDC_OLD.BitBlt(0, 0, m_RectOverlayMemDC.GetDeviceCaps(HORZSIZE), m_RectOverlayMemDC.GetDeviceCaps(VERTSIZE), &m_RectOverlayMemDC, m_RectOverlayMemDC.GetDeviceCaps(HORZSIZE), m_RectOverlayMemDC.GetDeviceCaps(VERTSIZE), SRCCOPY);
//	}*/
//	m_TotalOverlayMemDC.DeleteDC();
//	m_OverlayMemDC.DeleteDC();
//	m_MaskOverlayMemDC.DeleteDC();
//	m_RectOverlayMemDC.DeleteDC();
//	m_CaliperMemDC.DeleteDC();
//
//	CBitmap Bmp; 
//	m_OverlayMemDC.CreateCompatibleDC(NULL); 
//	Bmp.CreateBitmap(m_nWidth, m_nHeight, m_OverlayMemDC.GetDeviceCaps(PLANES), m_OverlayMemDC.GetDeviceCaps(BITSPIXEL), NULL); 
//	m_OverlayMemDC.SelectObject(&Bmp); 
//
//	CBitmap Bmp_Total;
//	m_TotalOverlayMemDC.CreateCompatibleDC(NULL);
//	Bmp_Total.CreateBitmap(m_nWidth, m_nHeight, m_TotalOverlayMemDC.GetDeviceCaps(PLANES), m_TotalOverlayMemDC.GetDeviceCaps(BITSPIXEL), NULL);
//	m_TotalOverlayMemDC.SelectObject(&Bmp_Total);
//
//	CBitmap MaskBmp; 
//	m_MaskOverlayMemDC.CreateCompatibleDC(NULL); 
//	MaskBmp.CreateBitmap(m_nWidth,m_nHeight,m_MaskOverlayMemDC.GetDeviceCaps(PLANES), m_MaskOverlayMemDC.GetDeviceCaps(BITSPIXEL),NULL); 
//	m_MaskOverlayMemDC.SelectObject(&MaskBmp); 
//
//	if( m_MaskOverlayMemDC_OLD.m_hDC )
//		m_MaskOverlayMemDC.BitBlt(0, 0, m_MaskOverlayMemDC.GetDeviceCaps(HORZSIZE), m_MaskOverlayMemDC.GetDeviceCaps(VERTSIZE), &m_MaskOverlayMemDC_OLD, m_MaskOverlayMemDC_OLD.GetDeviceCaps(HORZSIZE), m_MaskOverlayMemDC_OLD.GetDeviceCaps(VERTSIZE), SRCCOPY);
//
//
//	CBitmap RectMaskBmp; 
//	m_RectOverlayMemDC.CreateCompatibleDC(NULL); 
//	RectMaskBmp.CreateBitmap(m_nWidth,m_nHeight,m_RectOverlayMemDC.GetDeviceCaps(PLANES), m_RectOverlayMemDC.GetDeviceCaps(BITSPIXEL),NULL); 
//	m_RectOverlayMemDC.SelectObject(&RectMaskBmp); 
//
//	CBitmap CaliperBmp;
//	m_CaliperMemDC.CreateCompatibleDC(NULL);
//	CaliperBmp.CreateBitmap(m_nWidth, m_nHeight, m_CaliperMemDC.GetDeviceCaps(PLANES), m_CaliperMemDC.GetDeviceCaps(BITSPIXEL), NULL);
//	m_CaliperMemDC.SelectObject(&CaliperBmp);
//
//	if (m_CaliperMemDC_OLD.m_hDC)
//		m_CaliperMemDC.BitBlt(0, 0, m_CaliperMemDC.GetDeviceCaps(HORZSIZE), m_CaliperMemDC.GetDeviceCaps(VERTSIZE), &m_CaliperMemDC_OLD, m_CaliperMemDC_OLD.GetDeviceCaps(HORZSIZE), m_CaliperMemDC_OLD.GetDeviceCaps(VERTSIZE), SRCCOPY);
//
//	if( m_MaskOverlayMemDC_OLD.m_hDC )	m_MaskOverlayMemDC_OLD.DeleteDC();
//	if( m_RectOverlayMemDC_OLD.m_hDC )	m_RectOverlayMemDC_OLD.DeleteDC();
//	if (m_CaliperMemDC_OLD.m_hDC)		m_CaliperMemDC_OLD.DeleteDC();
//}
//
//void CViewerEx::ClearOverlayDC()
//{
//	if (NULL != m_OverlayMemDC)
//		m_OverlayMemDC.FillSolidRect(0, 0, (int)m_nWidth, (int)m_nHeight, RGB(0, 0, 0));
//}
//
//void CViewerEx::ClearMaskOverlayDC()
//{
//	if (NULL != m_MaskOverlayMemDC)
//		m_MaskOverlayMemDC.FillSolidRect(0, 0, (int)m_nWidth, (int)m_nHeight, RGB(0, 0, 0));
//}
//
//void CViewerEx::ClearCaliperOverlayDC()
//{
//	if (NULL != m_CaliperMemDC)
//		m_CaliperMemDC.FillSolidRect(0, 0, (int)m_nWidth, (int)m_nHeight, RGB(0, 0, 0));
//}
//
//void CViewerEx::ClearRectMaskOverlayDC()
//{
//	if (NULL != m_RectOverlayMemDC)
//		m_RectOverlayMemDC.FillSolidRect(0, 0, (int)m_nWidth, (int)m_nHeight, RGB(0, 0, 0));
//}
//
//void CViewerEx::PreSubclassWindow()
//{
//	ModifyStyle( 0, SS_NOTIFY );
//
//	::DragAcceptFiles(this->GetParent()->GetSafeHwnd(),TRUE);
//
//	if (m_bMenu) CreateMousePopupMenu();
//	CWnd::PreSubclassWindow();
//}
//
//
//void CViewerEx::OnMouseMove(UINT nFlags, CPoint point)
//{
//	GetCursorPos( &point ); 
//	ScreenToClient( &point ); 
//
//	CRect rect;
//	GetClientRect(&rect);
//
//	if( m_bRatioFit )
//	{
//		int dx = 0;
//		int dy = 0;
//		if (m_bDrawFitImage)
//		{
//			dx = (rect.Width() - m_expectedRect.Width()) / 2;
//			dy = (rect.Height() - m_expectedRect.Height()) / 2;
//
//			point.x -= dx;
//			point.y -= dy;
//		}
//	}
//
//	if (m_bScroll)
//	{
//		rect.right -= SIZE_SCROLLBAR;
//		rect.bottom -= SIZE_SCROLLBAR;
//	}
//
//	if (m_ImgSrc.IsNull()) return;
//
//	if (!m_bDrawFitImage && (point.x > m_nWidth * m_fMagnification - 1 || point.y > m_nHeight * m_fMagnification - 1) )
//		return;
//
//	// Fit Mode or Scroll Mode 에서 Scroll 사이즈를 뺀 영역까지만 마우스 포인트를 받을 수 있도록 보정.
//	if (m_bDrawFitImage || m_bScroll)
//		if (point.x > rect.Width() || point.y > rect.Height()) return;
//	
//	float fH = 0.0, fV = 0.0;
//	if (m_bDrawFitImage)
//	{
//		if( m_bRatioFit )
//		{
//			fH = (float)m_expectedRect.Width () / m_nWidth;
//			fV = (float)m_expectedRect.Height() / m_nHeight;
//		}
//		else
//		{
//			fH = (float)rect.Width () / m_nWidth;
//			fV = (float)rect.Height() / m_nHeight;
//		}
//	}
//	else
//	{
//		fH = m_fMagnification;
//		fV = m_fMagnification;
//	}
//	
//	point.x = LONG(point.x /fH);
//	point.y = LONG(point.y /fV);
//
//	if (m_pDlgCaliper && m_bUseCaliper)
//	{
//		ClearCaliperOverlayDC();
//
//		point.x += m_ptResult.x;
//		point.y += m_ptResult.y;
//
//		m_pDlgCaliper->MouseMove(point, getCaliperOverlayDC());
//		Invalidate();
//		return;
//	}
//
//	if (m_bModifyFigure && !m_bDrawStart)
//	{
//		if (nFlags == MK_LBUTTON)
//		{
//			CPoint offset;	
//			offset = point - m_ptLDown ;
//
//			if (m_isHitNW)
//			{
//				m_stFigure[m_nCurrFigure].ptBegin.x = m_rcPreFigure.left + offset.x;
//				m_stFigure[m_nCurrFigure].ptBegin.y = m_rcPreFigure.top  + offset.y;
//			}
//			if (m_isHitN)
//			{
//				if (m_stFigure[m_nCurrFigure].isLine)
//				{
//					m_stFigure[m_nCurrFigure].ptBegin.x = m_rcPreFigure.left   + offset.x;
//					m_stFigure[m_nCurrFigure].ptBegin.y = m_rcPreFigure.top    + offset.y;
//					m_stFigure[m_nCurrFigure].ptEnd.x   = m_rcPreFigure.right  + offset.x;
//					m_stFigure[m_nCurrFigure].ptEnd.y   = m_rcPreFigure.bottom + offset.y;
//				}
//				else
//				{
//					m_stFigure[m_nCurrFigure].ptBegin.y = m_rcPreFigure.top + offset.y;
//				}
//			}
//			if (m_isHitNE)
//			{
//				if (m_stFigure[m_nCurrFigure].isLine)
//				{
//					m_stFigure[m_nCurrFigure].ptEnd.x = m_rcPreFigure.right  + offset.x;
//					m_stFigure[m_nCurrFigure].ptEnd.y = m_rcPreFigure.bottom + offset.y;
//				}
//				else
//				{
//					m_stFigure[m_nCurrFigure].ptEnd.x = m_rcPreFigure.right + offset.x;
//					m_stFigure[m_nCurrFigure].ptBegin.y = m_rcPreFigure.top + offset.y;
//				}
//			}
//			if (m_isHitE)
//			{
//				m_stFigure[m_nCurrFigure].ptEnd.x = m_rcPreFigure.right + offset.x;
//			}
//			if (m_isHitSE)
//			{
//				m_stFigure[m_nCurrFigure].ptEnd.x = m_rcPreFigure.right  + offset.x;
//				m_stFigure[m_nCurrFigure].ptEnd.y = m_rcPreFigure.bottom + offset.y;
//			}
//			if (m_isHitS)
//			{
//				m_stFigure[m_nCurrFigure].ptEnd.y = m_rcPreFigure.bottom + offset.y;
//			}
//			if (m_isHitSW)
//			{
//				m_stFigure[m_nCurrFigure].ptBegin.x = m_rcPreFigure.left + offset.x;
//				m_stFigure[m_nCurrFigure].ptEnd.y = m_rcPreFigure.bottom + offset.y;
//			}
//			if (m_isHitW)
//			{
//				m_stFigure[m_nCurrFigure].ptBegin.x = m_rcPreFigure.left + offset.x;
//			}
//			if ( m_isMoving && (!m_isHitNW || !m_isHitN  || !m_isHitNE || !m_isHitE  || !m_isHitSE || !m_isHitS || !m_isHitSW || !m_isHitW) )
//			{
//				m_stFigure[m_nCurrFigure].ptBegin.x = m_rcPreFigure.left   + offset.x;
//				m_stFigure[m_nCurrFigure].ptBegin.y = m_rcPreFigure.top    + offset.y;
//				m_stFigure[m_nCurrFigure].ptEnd.x   = m_rcPreFigure.right  + offset.x;
//				m_stFigure[m_nCurrFigure].ptEnd.y   = m_rcPreFigure.bottom + offset.y;
//			}
//
//			Invalidate();
//		}
//		//Mouse Hit check
//		// 도형이 그려지는 시작점과 끝점의 위치에 따라 조건문을 설정.
//		else if (
//				(point.x + m_ptResult.x >= m_stFigure[m_nCurrFigure].ptBegin.x -5 && point.x + m_ptResult.x <= m_stFigure[m_nCurrFigure].ptEnd.x +5 && point.y + m_ptResult.y >= m_stFigure[m_nCurrFigure].ptBegin.y -5 && point.y + m_ptResult.y <= m_stFigure[m_nCurrFigure].ptEnd.y   +5)
//			||  (point.x + m_ptResult.x >= m_stFigure[m_nCurrFigure].ptBegin.x -5 && point.x + m_ptResult.x <= m_stFigure[m_nCurrFigure].ptEnd.x +5 && point.y + m_ptResult.y >= m_stFigure[m_nCurrFigure].ptEnd.y   -5 && point.y + m_ptResult.y <= m_stFigure[m_nCurrFigure].ptBegin.y +5)
//			||  (point.x + m_ptResult.x >= m_stFigure[m_nCurrFigure].ptEnd.x   -5 && point.x + m_ptResult.x <= m_stFigure[m_nCurrFigure].ptBegin.x +5 && point.y + m_ptResult.y >= m_stFigure[m_nCurrFigure].ptEnd.y -5 && point.y + m_ptResult.y <= m_stFigure[m_nCurrFigure].ptBegin.y +5)
//			||  (point.x + m_ptResult.x >= m_stFigure[m_nCurrFigure].ptEnd.x   -5 && point.x + m_ptResult.x <= m_stFigure[m_nCurrFigure].ptBegin.x +5 && point.y + m_ptResult.y >= m_stFigure[m_nCurrFigure].ptBegin.y -5 && point.y + m_ptResult.y <= m_stFigure[m_nCurrFigure].ptEnd.y   +5)
//			)
//		{
//			::SetCursor(LoadCursor(NULL, IDC_ARROW));
//			m_isMoving = false;
//			m_isHitNW  = false;
//			m_isHitN   = false;
//			m_isHitNE  = false;
//			m_isHitE   = false;
//			m_isHitSE  = false;
//			m_isHitS   = false;
//			m_isHitSW  = false;
//			m_isHitW   = false;
//
//			if (
//					(point.x + m_ptResult.x >= m_stFigure[m_nCurrFigure].ptBegin.x  && point.x + m_ptResult.x <= m_stFigure[m_nCurrFigure].ptEnd.x && point.y + m_ptResult.y >= m_stFigure[m_nCurrFigure].ptBegin.y  && point.y + m_ptResult.y <= m_stFigure[m_nCurrFigure].ptEnd.y )
//				||	(point.x + m_ptResult.x >= m_stFigure[m_nCurrFigure].ptBegin.x  && point.x + m_ptResult.x <= m_stFigure[m_nCurrFigure].ptEnd.x && point.y + m_ptResult.y >= m_stFigure[m_nCurrFigure].ptEnd.y  && point.y + m_ptResult.y <= m_stFigure[m_nCurrFigure].ptBegin.y )
//				||	(point.x + m_ptResult.x >= m_stFigure[m_nCurrFigure].ptEnd.x  && point.x + m_ptResult.x <= m_stFigure[m_nCurrFigure].ptBegin.x && point.y + m_ptResult.y >= m_stFigure[m_nCurrFigure].ptEnd.y  && point.y + m_ptResult.y <= m_stFigure[m_nCurrFigure].ptBegin.y )
//				||  (point.x + m_ptResult.x >= m_stFigure[m_nCurrFigure].ptEnd.x  && point.x + m_ptResult.x <= m_stFigure[m_nCurrFigure].ptBegin.x && point.y + m_ptResult.y >= m_stFigure[m_nCurrFigure].ptBegin.y  && point.y + m_ptResult.y <= m_stFigure[m_nCurrFigure].ptEnd.y )
//				)
//			{
//				::SetCursor(LoadCursor(NULL, IDC_SIZEALL));
//				m_isMoving = true;
//				m_isHitNW = false;
//				m_isHitN  = false;
//				m_isHitNE = false;
//				m_isHitE  = false;
//				m_isHitSE = false;
//				m_isHitS  = false;
//				m_isHitSW = false;
//				m_isHitW  = false;
//			}
//			
//			
//			float fH2 = (float)rect.Width () / m_nWidth;
//			float fV2 = (float)rect.Height() / m_nHeight;
//
//			if( !m_bDrawFitImage )
//			{
//				fH2 = fH;
//				fV2 = fV;
//			}
//
//			for (int i = 0; i < 8; i++)
//			{
//				/*if (m_stFigure[m_nCurrFigure].rcHit[i].left <= point.x * fH && point.x * fH <= m_stFigure[m_nCurrFigure].rcHit[i].right  &&
//					m_stFigure[m_nCurrFigure].rcHit[i].top  <= point.y * fV && point.y * fV <= m_stFigure[m_nCurrFigure].rcHit[i].bottom )*/
//				if (m_stFigure[m_nCurrFigure].rcHit[i].left <= point.x * fH2 && point.x * fH2 <= m_stFigure[m_nCurrFigure].rcHit[i].right  &&
//					m_stFigure[m_nCurrFigure].rcHit[i].top  <= point.y * fV2 && point.y * fV2 <= m_stFigure[m_nCurrFigure].rcHit[i].bottom )
//				{
//					switch(i)
//					{
//							// NW
//						case 0 :
//							::SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
//							m_isHitNW  = true;
//							m_isMoving = false;
//							break;
//							// N
//						case 1 :
//							::SetCursor(LoadCursor(NULL, IDC_SIZENS));
//							m_isHitN  = true;
//							m_isMoving = false;
//							break;
//							//NE
//						case 2 :
//							::SetCursor(LoadCursor(NULL, IDC_SIZENESW));
//							m_isHitNE  = true;
//							m_isMoving = false;
//							break;
//							// E
//						case 3 :
//							::SetCursor(LoadCursor(NULL, IDC_SIZEWE));
//							m_isHitE  = true;
//							m_isMoving = false;
//							break;
//							// SE
//						case 4 :
//							::SetCursor(LoadCursor(NULL, IDC_SIZENWSE));
//							m_isHitSE  = true;
//							m_isMoving = false;
//							break;
//							// S
//						case 5 :
//							::SetCursor(LoadCursor(NULL, IDC_SIZENS));
//							m_isHitS  = true;
//							m_isMoving = false;
//							break;
//							// SW
//						case 6 :
//							::SetCursor(LoadCursor(NULL, IDC_SIZENESW));
//							m_isHitSW  = true;
//							m_isMoving = false;
//							break;
//							// W
//						case 7 :
//							::SetCursor(LoadCursor(NULL, IDC_SIZEWE));
//							m_isHitW   = true;
//							m_isMoving = false;
//							break;
//					}
//				}
//			}
//		}
//		else
//		{
//			m_isMoving = false;
//			m_isHitNW  = false;
//			m_isHitN   = false;
//			m_isHitNE  = false;
//			m_isHitE   = false;
//			m_isHitSE  = false;
//			m_isHitS   = false;
//			m_isHitSW  = false;
//			m_isHitW   = false;
//		}
//	}
//	else
//	{
//		if (m_bDrawingFigure)
//			::SetCursor(LoadCursor(NULL, IDC_HAND));
//		else
//			::SetCursor(LoadCursor(NULL, IDC_ARROW));
//		
//		if(nFlags == MK_LBUTTON)
//		{	
//			CPoint temp;
//			temp = m_ptResult;
//
//			// 도형을 그린 후 마우스 포인트가 좌상단으로 올라갈 경우 이미지의 옵셋이 변하게 된다.
//			//아래 조건문은 보정하는 코드.
//			if (!m_bDrawingFigure)
//			{
//				m_ptResult = m_ptLup + m_ptLDown - point;
//			}
//
//			//우하단 이미지 크기보다 못 넘어가게 막음.
//			if(m_ptResult.x >  (m_nWidth - rect.Width()/m_fMagnification))
//				m_ptResult.x = LONG(m_nWidth - rect.Width()/ m_fMagnification);
//			if(m_ptResult.y >  (m_nHeight - rect.Height()/m_fMagnification))
//				m_ptResult.y = LONG(m_nHeight - rect.Height()/m_fMagnification);
//
//			//좌상단 0,0이하로 못 넘어가게 막음.
//			if(m_ptResult.x < 0)	m_ptResult.x = 0;
//			if(m_ptResult.y < 0 )	m_ptResult.y = 0;
//
//			if (m_bDrawingFigure)
//				m_stFigure[m_nCurrFigure].ptEnd = m_ptResult + point;
//
//			if (m_bScroll)
//				m_ptResult = temp;
//
//			Invalidate();
//		}
//	}
//	
//	//이미지 상의 현재 마우스 좌표.
//	m_ptMousePoint = point + m_ptResult;
//
//	
//	if( m_bUseMask && m_bMaskClick )
//	{
//		if( m_bMaskPen ) SetMaskPos(point);
//
//		if( m_bMaskRect )
//		{
//			CRect rect;
//			GetClientRect(&rect);
//
//			if( (rect.right - point.x) <= 10 ) point.x = rect.right;
//			if( point.x <= 10 ) point.x = 0;
//
//			if( (rect.bottom - point.y) <= 10 ) point.y = rect.bottom;
//			if( point.y <= 10 ) point.y = 0;
//
//			DrawRect(m_ptLDown, point);
//		}
//
//		Invalidate();
//	//	loadGrayRaw();
//	}
//
//	if( m_hWnd ) ::PostMessage(m_hWnd, WM_VIEW_CONTROL, MSG_FCV_SET_MOUSE_POS, MAKELPARAM(m_ptMousePoint.x, m_ptMousePoint.y));
//
//	if (m_nBpp == 8)
//		m_nY = GetValueY(m_ptMousePoint.x, m_ptMousePoint.y);
//	else if (m_nBpp == 24)
//		m_clrRGB = GetValueRGB(m_ptMousePoint.x, m_ptMousePoint.y);
//
//	::SendMessage(GetParent()->m_hWnd, WM_MOUSEMOVE, 0, 0);
//
//	CWnd::OnMouseMove(nFlags, point);
//}
//
//
//void CViewerEx::OnPaint()
//{
//	CPaintDC dc(this); // device context for painting
//
//	CRect rect;
//	GetClientRect(&rect);
//
//	m_expectedRect = rect;
//	if( m_bRatioFit )
//	{
//		double aspectRatio = m_nHeight / (double)m_nWidth;
//		double aspectRatio2 = m_expectedRect.Height() / (double)m_expectedRect.Width();
//
//		int expectedX = m_expectedRect.Height() * m_nWidth / (double)m_nHeight;
//		int expectedY = m_expectedRect.Width() * m_nHeight / (double)m_nWidth;
//
//		if( aspectRatio < aspectRatio2 )
//		{
//			int nDiff = rect.Height() - expectedY;
//			m_expectedRect.top = rect.top + nDiff / 2;
//			m_expectedRect.left = rect.left;
//			m_expectedRect.right = rect.right;
//			m_expectedRect.bottom = rect.bottom - nDiff / 2;
//		}
//		else
//		{
//			int nDiff = rect.Width() - expectedX;
//			m_expectedRect.top = rect.top;
//			m_expectedRect.left = rect.left + nDiff / 2;
//			m_expectedRect.right = rect.right - nDiff / 2;
//			m_expectedRect.bottom = rect.bottom;
//		}
//	}
//
//	//////////////////////////////////////////////////////////////////////////
//	CDC memDC;
//	CBitmap *pOldBitmap, bitmap;
//
//	memDC.CreateCompatibleDC(&dc);
//
//	bitmap.CreateCompatibleBitmap(&dc, rect.Width(), rect.Height());
//
//	pOldBitmap = memDC.SelectObject(&bitmap);
//	memDC.PatBlt(0, 0, rect.Width(), rect.Height(), SRCCOPY);
//	//////////////////////////////////////////////////////////////////////////
//
//	if (m_bScroll)
//	{
//		rect.right -= SIZE_SCROLLBAR;
//		rect.bottom -= SIZE_SCROLLBAR;
//	}
//
//	// 2016-04-06. 
//	// 상시 그리도록 변경 
//	if (m_bClear == true)
//	{
//		int w = 32;
//		int h = 32;
//		HDC hDC = CreateCompatibleDC(dc.m_hDC);
//		HBITMAP hBmp = CreateCompatibleBitmap(dc.m_hDC, w, h);
//		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hDC, hBmp);
//
//		BitBlt(hDC, 0, 0, w, h, hDC, 0, 0, SRCCOPY);
//		CBrush Brush1(RGB(130,100,100));
//		CBrush Brush2(RGB(140,100,100));
//
//		FillRect(hDC, CRect(0, 0, w / 2, h / 2), Brush1);
//		FillRect(hDC, CRect(w / 2, 0, w, h / 2), Brush2);
//		FillRect(hDC, CRect(0, h / 2, w / 2, h), Brush2);
//		FillRect(hDC, CRect(w / 2, h / 2, w, h), Brush1);
//		
//		CBitmap bm;
//		bm.Attach(hBmp);
//
//		CBrush brush;
//		brush.CreatePatternBrush(&bm);
//		CBrush *pOldBrush = (CBrush*)memDC.SelectObject(&brush);
//		memDC.Rectangle(&rect);
//
//		memDC.SelectObject(pOldBrush);
//
//		DeleteObject(hDC);
//		DeleteObject(hBmp);
//		DeleteObject(hOldBitmap);
//
//		m_bClear = false;
//	}
//
//	if (m_ImgSrc.IsNull()) 
//	{
//		dc.BitBlt(0, 0, rect.Width(), rect.Height(), &memDC, 0, 0, SRCCOPY);
//
//		memDC.SelectObject(pOldBitmap);
//		memDC.DeleteDC();
//		bitmap.DeleteObject();
//		return;
//	}
//
//	//큰 배율에서 작은 배율로 축소 시 이미지의 크기를 넘어서 그리게 되는 것을 방지한다.
//	if (m_ptResult.x + (float)rect.Width() / m_fMagnification > m_nWidth)
//	{
//		m_ptResult.x = LONG(m_nWidth - (float)rect.Width() / m_fMagnification);
//		if (m_ptResult.x < 0) m_ptResult.x = 0;
//		m_ptLup.x = m_ptResult.x;
//	}
//	if (m_ptResult.y + (float)rect.Height() / m_fMagnification > m_nHeight)
//	{
//		m_ptResult.y = LONG(m_nHeight - (float)rect.Height() / m_fMagnification);
//		if (m_ptResult.y < 0) m_ptResult.y = 0;
//		m_ptLup.y = m_ptResult.y;
//	}
//
//	// 컨트롤 안에서 이미지 크기가 작을 경우, 이미지를 벗어나 도형을 그리면 잔상이 남는다. 이를 보정하는 부분.
//	CRgn rgn;
//	if (m_bDrawFitImage)
//	{
//		rgn.CreateRectRgnIndirect(&rect);
//		memDC.SelectClipRgn(&rgn);
//	}
//	else
//	{
//		if(m_nWidth * m_fMagnification < rect.Width() || m_nHeight * m_fMagnification < rect.Height())
//		{
//			CRect rc;
//			rc = rect;
//
//			rc.right = LONG(m_nWidth * m_fMagnification);
//			rc.bottom = LONG(m_nHeight * m_fMagnification);
//
//			rgn.CreateRectRgnIndirect(&rc);
//			memDC.SelectClipRgn(&rgn);
//		}
//		else
//		{
//			rgn.CreateRectRgnIndirect(&rect);
//			memDC.SelectClipRgn(&rgn);
//		}
//	}
//
//	if (m_bDrawFitImage)
//	{
//		memDC.SetStretchBltMode(COLORONCOLOR);
//		m_ImgSrc.Draw(memDC.m_hDC,rect);
//
//		m_ptResult = (0,0);
//		//HTK 2022-04-04 Fit이미지 Overlay 배율따라가게 변경
//		m_fMagnification = m_nWidth>0?float(rect.Width()/float(m_nWidth)):1.0f;
//	}
//	else
//	{
//		// 이미지 축소 시 정상적인 그리기가 안되므로 옵션 설정.
//		if (m_fMagnification < 1)	memDC.SetStretchBltMode(COLORONCOLOR);
//		m_ImgSrc.Draw(memDC.m_hDC, 0, 0, rect.Width(), rect.Height(), m_ptResult.x, m_ptResult.y, int(rect.Width() / m_fMagnification), int(rect.Height() / m_fMagnification));
//	}
//
//	if (m_bDrawAllFigures)
//	{
//		drawAllFigures(memDC);
//		drawAllFigures(m_TotalOverlayMemDC);
//	}
//	if (m_bDrawOverlay)
//	{
//		drawFigure_NEW(memDC);
//		drawFigure_NEW(m_TotalOverlayMemDC);
//	}
//
//	CPen *pOldPen;
//
//	int centerX = int(m_nWidth / 2 * m_fMagnification);
//	int centerY = int(m_nHeight / 2 * m_fMagnification);
//
//	if( m_bUseRefPos && m_bDrawOverlay ) {
//		for(int i = 0; i < 4; i++)
//		{
//			if (m_dbRefPosX[i] != 0 && m_dbRefPosY[i] != 0)
//			{
//				//21.03.12
//				//drawReferencePos(memDC, i);
//			}
//		}
//	}
//
//	if (m_bDrawCrossLine) 
//	{
//		//두 포인트를 이용하여 선을 그린다.
//		CPen ScalePen,ScalePenBig;
//
//		if( m_bDrawOverlay )
//		{
//			if (m_bDrawFitImage)
//			{
//				ScalePen.CreatePen(PS_DOT, 1, RGB(0, 255, 255));
//				//ScalePen.CreatePen(PS_SOLID, 1, RGB(0, 255, 255));
//				pOldPen = (CPen*)memDC.SelectObject(&ScalePen);
//
//				memDC.SetBkMode(TRANSPARENT);
//				memDC.MoveTo(0, rect.Height() / 2);
//				memDC.LineTo(rect.Width(), rect.Height() / 2);
//
//				memDC.MoveTo(rect.Width() / 2, 0);
//				memDC.LineTo(rect.Width() / 2, rect.Height());
//
//				if(m_bDrawAlignTargetLine)
//				{
//					float fH = (float)rect.Width () / m_nWidth;
//					float fV = (float)rect.Height() / m_nHeight;
//					double x2, y2;
//
//					for(int i = 0; i < m_AlignTargetLine.size(); i++)
//					{
//						if(m_AlignTargetLine.at(i).y!=0)
//						{ 
//							if (m_nDrawAlignLineoffset != 0)
//							{
//								CalcRotate((m_AlignTargetLine.at(i).x - m_nDrawAlignLineoffset) * fH, m_AlignTargetLine.at(i).y * fV, m_nWidth * fH / 2, m_nHeight * fV / 2, 0, &x2, &y2);
//								memDC.MoveTo(int(x2), int(y2));
//								CalcRotate((m_AlignTargetLine.at(i).x + m_nDrawAlignLineoffset) * fH, m_AlignTargetLine.at(i).y * fV, m_nWidth * fH / 2, m_nHeight * fV / 2, 0, &x2, &y2);
//								memDC.LineTo(int(x2), int(y2));
//							}
//							else
//							{
//								CalcRotate(0, m_AlignTargetLine.at(i).y * fV, m_nWidth * fH / 2, m_nHeight * fV / 2, 0, &x2, &y2);
//								memDC.MoveTo(int(x2), int(y2));
//								CalcRotate(rect.Width(), m_AlignTargetLine.at(i).y * fV, m_nWidth * fH / 2, m_nHeight * fV / 2, 0, &x2, &y2);
//								memDC.LineTo(int(x2), int(y2));
//							}
//						}
//						else
//						{
//							if (m_nDrawAlignLineoffset != 0)
//							{
//								CalcRotate(m_AlignTargetLine.at(i).x* fH, (m_AlignTargetLine.at(i).y - m_nDrawAlignLineoffset )* fV, m_nWidth* fH / 2, m_nHeight* fV / 2, 0, &x2, &y2);
//								memDC.MoveTo(int(x2), int(y2));
//								CalcRotate(m_AlignTargetLine.at(i).x* fH, (m_AlignTargetLine.at(i).y + m_nDrawAlignLineoffset)* fV, m_nWidth* fH / 2, m_nHeight* fV / 2, 0, &x2, &y2);
//								memDC.LineTo(int(x2), int(y2));
//							}
//							else
//							{
//								CalcRotate(m_AlignTargetLine.at(i).x* fH, 0, m_nWidth* fH / 2, m_nHeight* fV / 2, 0, &x2, &y2);
//								memDC.MoveTo(int(x2), int(y2));
//								CalcRotate(m_AlignTargetLine.at(i).x* fH, rect.Height(), m_nWidth* fH / 2, m_nHeight* fV / 2, 0, &x2, &y2);
//								memDC.LineTo(int(x2), int(y2));
//							}
//						}						
//					}
//
//					int xp, yp, s = 20;
//
//					for(int i = 0; i < 4; i++)
//					{
//						switch(i)
//						{
//						case 0: xp = rect.Width() / 4,		yp = rect.Height() / 4;			break;
//						case 1: xp = rect.Width() * 3 / 4,	yp = rect.Height() / 4;			break;
//						case 2: xp = rect.Width() / 4,		yp = rect.Height() * 3 / 4;		break;
//						case 3: xp = rect.Width() * 3 / 4,	yp = rect.Height() * 3 / 4;		break;
//						}
//
//						memDC.MoveTo(xp - s, yp);
//						memDC.LineTo(xp + s, yp);
//						memDC.MoveTo(xp, yp - s);
//						memDC.LineTo(xp, yp + s);
//					}
//				}
//			}
//			else
//			{
//				ScalePen.CreatePen(PS_DOT, 1, RGB(0, 255, 255));
//				pOldPen = (CPen*)memDC.SelectObject(&ScalePen);
//
//				memDC.SetBkMode(TRANSPARENT);
//				memDC.MoveTo(0, int((m_nHeight * m_fMagnification) / 2 - m_ptResult.y * m_fMagnification));
//				memDC.LineTo(int(m_nWidth * m_fMagnification), int((m_nHeight * m_fMagnification) / 2 - m_ptResult.y * m_fMagnification));
//				memDC.MoveTo(int((m_nWidth * m_fMagnification) / 2 - m_ptResult.x * m_fMagnification), 0);
//				memDC.LineTo(int((m_nWidth * m_fMagnification) / 2 - m_ptResult.x * m_fMagnification), int(m_nHeight * m_fMagnification));
//
//				if(m_bDrawAlignTargetLine)
//				{
//					for(int i = 0; i < m_AlignTargetLine.size(); i++)
//					{
//						if (m_AlignTargetLine.at(i).y != 0)
//						{
//							if (m_nDrawAlignLineoffset != 0)
//							{
//								memDC.MoveTo(int((m_AlignTargetLine.at(i).x - m_ptResult.x - m_nDrawAlignLineoffset) * m_fMagnification), int((m_AlignTargetLine.at(i).y - m_ptResult.y) * m_fMagnification));
//								memDC.LineTo(int((m_AlignTargetLine.at(i).x - m_ptResult.x + m_nDrawAlignLineoffset) * m_fMagnification), int((m_AlignTargetLine.at(i).y - m_ptResult.y) * m_fMagnification));
//							}
//							else
//							{
//								memDC.MoveTo(0, int((m_AlignTargetLine.at(i).y - m_ptResult.y) * m_fMagnification));
//								memDC.LineTo(int(m_nWidth * m_fMagnification), int((m_AlignTargetLine.at(i).y - m_ptResult.y) * m_fMagnification));
//							}
//						}
//						else
//						{
//							if (m_nDrawAlignLineoffset != 0)
//							{
//								memDC.MoveTo(int((m_AlignTargetLine.at(i).x - m_ptResult.x)* m_fMagnification), int((m_AlignTargetLine.at(i).y - m_ptResult.y - m_nDrawAlignLineoffset)* m_fMagnification));
//								memDC.LineTo(int((m_AlignTargetLine.at(i).x - m_ptResult.x)* m_fMagnification), int((m_AlignTargetLine.at(i).y - m_ptResult.y + m_nDrawAlignLineoffset)* m_fMagnification));
//							}
//							else
//							{
//								memDC.MoveTo(int((m_AlignTargetLine.at(i).x - m_ptResult.x)* m_fMagnification), 0);
//								memDC.LineTo(int((m_AlignTargetLine.at(i).x - m_ptResult.x)* m_fMagnification), int(m_nHeight* m_fMagnification));
//							}
//						}
//					}
//
//					int xp, yp, s = 20;
//
//					for(int i = 0; i < 4; i++)
//					{
//						switch(i)	{
//						case 0: xp = int(m_nWidth / 4), yp = int(m_nHeight / 4);			break;
//						case 1: xp = int(m_nWidth * 3 / 4), yp = int(m_nHeight / 4);		break;
//						case 2: xp = int(m_nWidth / 4), yp = int(m_nHeight * 3 / 4);		break;
//						case 3: xp = int(m_nWidth * 3 / 4), yp = int(m_nHeight * 3 / 4);	break;
//						}
//
//						xp = (xp - m_ptResult.x) * m_fMagnification;
//						yp = (yp - m_ptResult.y) * m_fMagnification;
//
//						memDC.MoveTo(xp - s, yp);
//						memDC.LineTo(xp + s, yp);
//						memDC.MoveTo(xp, yp - s);
//						memDC.LineTo(xp, yp + s);
//					}
//				}
//			}
//
//			memDC.SelectObject(pOldPen);
//			ScalePen.DeleteObject();
//		}
//	}
//
//	
//	if (m_bNavi)
//	{
//		CBrush brsFrame(RGB(255,0,0));
//
//		memDC.SetStretchBltMode(COLORONCOLOR);
//		int w = rect.Width() / 4;
//		int h = rect.Height() / 4;
//		int orgx = rect.Width() / 2 - w / 2;
//		int orgy = rect.Height() - h - 20;
//		float fH = (float)w / m_nWidth;
//		float fV = (float)h / m_nHeight;
//		
//		//2021-05-11 KJH 카메라 이미지가 Rect 사이즈 보다 작을떄 네비게이션 위치 조정
//		int naviw = m_nWidth * m_fMagnification / 4;
//		int navih = m_nHeight * m_fMagnification / 4;
//		int magniWidth = m_nWidth * m_fMagnification;
//		int magniHeight = m_nHeight * m_fMagnification;
//
//		if (!m_bDrawFitImage && (magniWidth < rect.Width() || magniHeight < rect.Height()))
//		{
//			m_ImgSrc.AlphaBlend(memDC.m_hDC, magniWidth / 2 - naviw / 2, magniHeight - navih - 20, naviw, navih, 0, 0, m_nWidth, m_nHeight, 150);
//		}
//		else
//		{
//			m_ImgSrc.AlphaBlend(memDC.m_hDC, orgx, orgy, w, h, 0, 0, m_nWidth, m_nHeight, 150);
//		}
//
//		if (m_bDrawFitImage)
//		{
//			CRect rc;
//			rc.left = orgx;
//			rc.top = orgy;
//			rc.right = rc.left + w;
//			rc.bottom = rc.top + h;
//			memDC.FrameRect(rc, &brsFrame);
//		}
//		else
//		{
//			CRect rc;
//			//2021-05-11 KJH 카메라 이미지가 Rect 사이즈 보다 작을떄 네비게이션 위치 조정
//			if (!m_bDrawFitImage || magniWidth < rect.Width() || magniHeight < rect.Height())
//			{
//				rc.left = magniWidth / 2 - naviw / 2;
//				rc.top = magniHeight - navih - 20;
//				rc.right = rc.left + naviw;
//				rc.bottom = rc.top + navih;
//				memDC.FrameRect(rc, &brsFrame);
//			}
//			else
//			{
//				rc.left = LONG(orgx + m_ptResult.x * fH);
//				rc.top = LONG(orgy + m_ptResult.y * fV);
//
//				if (m_nWidth * m_fMagnification < rect.Width())
//					rc.right = rc.left + w;
//				else
//					rc.right = LONG(rc.left + rect.Width() / m_fMagnification * fH);
//
//				if (m_nHeight * m_fMagnification < rect.Height())
//					rc.bottom = rc.top + h;
//				else
//					rc.bottom = LONG(rc.top + rect.Height() / m_fMagnification * fV);
//
//				memDC.FrameRect(rc, &brsFrame);
//			}
//		}
//	}
//
//	if( m_bPatternLine ) 
//	{
//		//두 포인트를 이용하여 선을 그린다.
//		CPen ScalePen,ScalePenBig,ScalePenAngle;
//		ScalePen.CreatePen(PS_DOT, 1, RGB(0, 255, 0));
//		ScalePenAngle.CreatePen(PS_DOT, 1, RGB(255, 255, 0));
//		pOldPen = (CPen*)memDC.SelectObject(&ScalePen);
//
//		if (m_bDrawFitImage)
//		{
//			memDC.SetBkMode(TRANSPARENT);
//			memDC.MoveTo(0, m_ptPatternPos.y);
//			memDC.LineTo(rect.Width(), m_ptPatternPos.y);
//			memDC.MoveTo(m_ptPatternPos.x, 0);
//			memDC.LineTo(m_ptPatternPos.x, rect.Height());
//
//			if(m_AlignTargetAngle != 0)
//			{
//				memDC.SelectObject(&ScalePenAngle);
//				CPoint r = RotatePt(m_AlignTargetAngle, CPoint(0,m_ptPatternPos.y),m_ptPatternPos);
//			
//				memDC.MoveTo(0, r.y); r = RotatePt(m_AlignTargetAngle, CPoint(rect.Width(), m_ptPatternPos.y), m_ptPatternPos);
//				memDC.LineTo(rect.Width(), r.y);
//			
//				r = RotatePt(m_AlignTargetAngle, CPoint(m_ptPatternPos.x, 0), m_ptPatternPos);
//				memDC.MoveTo(r.x, 0);
//				r = RotatePt(m_AlignTargetAngle, CPoint(m_ptPatternPos.x,rect.Height()), m_ptPatternPos);
//				memDC.LineTo(r.x, rect.Height());
//			}
//		}
//		else
//		{
//			memDC.SetBkMode(TRANSPARENT);
//			memDC.MoveTo(0, int((m_nHeight * m_fMagnification) / 2 - m_ptResult.y * m_fMagnification));
//			memDC.LineTo(int(m_nWidth * m_fMagnification), int((m_nHeight * m_fMagnification) / 2 - m_ptResult.y * m_fMagnification));
//			memDC.MoveTo(int((m_nWidth * m_fMagnification) / 2 - m_ptResult.x * m_fMagnification), 0);
//			memDC.LineTo(int((m_nWidth * m_fMagnification) / 2 - m_ptResult.x * m_fMagnification), int(m_nHeight * m_fMagnification));
//		}
//
//		memDC.SelectObject(pOldPen);
//		ScalePen.DeleteObject();
//	}
//
//	memDC.SelectClipRgn(NULL);
//
//	if( m_bDrawOverlay )
//	{
//		if (m_bDrawFitImage)
//		{
//			if( m_bUseMask )
//				TransparentBlt(memDC.m_hDC, 0, 0, (int)rect.Width(), (int)rect.Height(), m_MaskOverlayMemDC.m_hDC, 0, 0, (int)m_nWidth, (int)m_nHeight, RGB(0, 0, 0));
//
//			// Lincoln Lee - 220210
//			if (m_UseSemiTransparent)
//				AlphaBlend(memDC.m_hDC, 0, 0, (int)rect.Width(), (int)rect.Height(), m_OverlayMemDC.m_hDC, 0, 0, (int)m_nWidth, (int)m_nHeight, m_bf);
//			else
//				TransparentBlt(memDC.m_hDC, 0, 0, (int)rect.Width(), (int)rect.Height(), m_OverlayMemDC.m_hDC, 0, 0, (int)m_nWidth, (int)m_nHeight, RGB(0, 0, 0));
//
//			if( m_bUseMask && m_bMaskRect )
//				TransparentBlt(memDC.m_hDC, 0, 0, (int)rect.Width(), (int)rect.Height(), m_RectOverlayMemDC.m_hDC, 0, 0, (int)m_nWidth, (int)m_nHeight, RGB(0, 0, 0));
//
//			if (m_bUseCaliper || m_bDrawCaliper)
//				TransparentBlt(memDC.m_hDC, 0, 0, (int)rect.Width(), (int)rect.Height(), m_CaliperMemDC.m_hDC, 0, 0, (int)m_nWidth, (int)m_nHeight, RGB(0, 0, 0));
//		}
//		else
//		{
//			// Lincoln Lee - 220210
//			if (m_UseSemiTransparent)
//				AlphaBlend(memDC.m_hDC, -m_ptResult.x * m_fMagnification, -m_ptResult.y * m_fMagnification, (int)(m_nWidth * m_fMagnification), (int)(m_nHeight * m_fMagnification), m_OverlayMemDC.m_hDC, 0, 0, (int)(m_nWidth), (int)(m_nHeight), m_bf);
//			else
//				TransparentBlt(memDC.m_hDC, -m_ptResult.x * m_fMagnification, -m_ptResult.y * m_fMagnification, (int)(m_nWidth * m_fMagnification), (int)(m_nHeight * m_fMagnification), m_OverlayMemDC.m_hDC, 0, 0, (int)(m_nWidth), (int)(m_nHeight), RGB(0, 0, 0));
//
//
//			if (m_bUseCaliper || m_bDrawCaliper)
//				TransparentBlt(memDC.m_hDC, -m_ptResult.x, -m_ptResult.y, (int)m_nWidth, (int)m_nHeight, m_CaliperMemDC.m_hDC, 0, 0, (int)(m_nWidth * m_fMagnification), (int)(m_nHeight * m_fMagnification), RGB(0, 0, 0));
//		}
//		
//		TransparentBlt(m_TotalOverlayMemDC.m_hDC, -m_ptResult.x, -m_ptResult.y, (int)m_nWidth, (int)m_nHeight, memDC.m_hDC, 0, 0, (int)(m_nWidth* m_fMagnification), (int)(m_nHeight* m_fMagnification), RGB(0, 0, 0));
//	}
//
//
//	if( m_bRatioFit )
//	{
//		if( m_bDrawFitImage )
//		{
//			CDC memDC2;
//			CBitmap bitmap2;
//			memDC2.CreateCompatibleDC(&dc);
//			bitmap2.CreateCompatibleBitmap(&dc, m_expectedRect.Width(), m_expectedRect.Height());
//			memDC2.SelectObject(&bitmap2);
//			memDC2.SetStretchBltMode(HALFTONE);
//		//	memDC2.SetStretchBltMode(COLORONCOLOR);
//			StretchBlt(memDC2.m_hDC,0,0,(int)m_expectedRect.Width(), (int)m_expectedRect.Height(), memDC.m_hDC, 0, 0, (int)rect.Width(), (int)rect.Height(),SRCCOPY);
//
//			int dx = (rect.Width() - m_expectedRect.Width()) / 2;
//			int dy = (rect.Height() - m_expectedRect.Height()) / 2;
//
//			CBrush br(RGB(140, 100, 100));
//			dc.BitBlt(dx, dy, m_expectedRect.Width(), m_expectedRect.Height(), &memDC2, 0, 0, SRCCOPY);
//
//			double aspectRatio = m_nHeight / (double)m_nWidth;
//			double aspectRatio2 = m_expectedRect.Height() / (double)m_expectedRect.Width();
//
//			double dxx = abs(m_expectedRect.Width() - rect.Width());
//			double dyy = abs(m_expectedRect.Height() - rect.Height());
//
//			if( dxx > dyy )
//			{
//				FillRect(dc.m_hDC, CRect(0, 0, dx, m_expectedRect.Height()), br);
//				FillRect(dc.m_hDC, CRect(dx + m_expectedRect.Width(), 0, dx + dx + m_expectedRect.Width(), m_expectedRect.Height()), br);
//			}
//			else
//			{
//				FillRect(dc.m_hDC, CRect(0, 0, m_expectedRect.Width(), dy), br);
//				FillRect(dc.m_hDC, CRect(0, dy + m_expectedRect.Height(), m_expectedRect.Width(), rect.Height()), br);
//			}
//
//			bitmap2.DeleteObject();
//			ReleaseDC(&memDC2);
//			//DeleteDC(memDC2);
//		}
//		else
//		{
//			int w = m_nWidth * m_fMagnification;
//			int h = m_nHeight * m_fMagnification;
//
//			int x = 0;
//			int y = 0;
//
//			//2021-05-11 KJH Viewer 사이즈보다 Camera 해상도가 작을때 예외처리 추가 
//			if (w < rect.Width() && h < rect.Height())
//			{
//				CBrush br(RGB(140, 100, 100));
//				FillRect(dc.m_hDC, CRect(0, 0, rect.Width(), rect.Height()), br);
//
//				// Lincoln Lee - 220210
//				if (m_UseSemiTransparent)
//					AlphaBlend(memDC.m_hDC, 0, 0, (int)rect.Width(), (int)rect.Height(), m_OverlayMemDC.m_hDC, 0, 0, (int)m_nWidth, (int)m_nHeight, m_bf);
//				else
//					TransparentBlt(memDC.m_hDC, 0, 0, (int)rect.Width(), (int)rect.Height(), m_OverlayMemDC.m_hDC, 0, 0, (int)m_nWidth, (int)m_nHeight, RGB(0, 0, 0));
//
//				dc.BitBlt((rect.Width() - w) / 2, (rect.Height() - h) / 2, w, h, &memDC, 0, 0, SRCCOPY);
//			}
//			else
//			{
//				CBrush br(RGB(140, 100, 100));
//				if (w < rect.Width())	x = (rect.Width() - w) / 2;
//				if (h < rect.Height())	y = (rect.Height() - h) / 2;
//
//				if (w < rect.Width())
//				{
//					FillRect(dc.m_hDC, CRect(0, 0, x, rect.Height()), br);
//					FillRect(dc.m_hDC, CRect(rect.Width() - x, 0, rect.Width(), rect.Height()), br);
//				}
//
//				if (h < rect.Height())
//				{
//					FillRect(dc.m_hDC, CRect(0, 0, rect.Width(), y), br);
//					FillRect(dc.m_hDC, CRect(0, rect.Height() - y, rect.Width(), y), br);
//				}
//
//				// 2022-02-25 LinCole 
//				dc.BitBlt(x, y, rect.Width(), rect.Height(), &memDC, 0, 0, SRCCOPY);
//			}
//		}
//	}
//	else
//	{
//		if (m_bDrawFitImage)
//		{
//			if( m_bUseMask )
//				TransparentBlt(memDC.m_hDC, 0, 0, (int)rect.Width(), (int)rect.Height(), m_MaskOverlayMemDC.m_hDC, 0, 0, (int)m_nWidth, (int)m_nHeight, RGB(0,0,0));
//
//			// Lincoln Lee - 220210
//			if (m_UseSemiTransparent)
//				AlphaBlend(memDC.m_hDC, 0, 0, (int)rect.Width(), (int)rect.Height(), m_OverlayMemDC.m_hDC, 0, 0, (int)m_nWidth, (int)m_nHeight, m_bf);
//			else
//				TransparentBlt(memDC.m_hDC, 0, 0, (int)rect.Width(), (int)rect.Height(), m_OverlayMemDC.m_hDC, 0, 0, (int)m_nWidth, (int)m_nHeight, RGB(0, 0, 0));
//
//			if( m_bUseMask && m_bMaskRect )
//				TransparentBlt(memDC.m_hDC,0,0,(int)rect.Width(), (int)rect.Height(),	m_RectOverlayMemDC.m_hDC, 0,0,(int)m_nWidth, (int)m_nHeight,RGB(0,0,0));
//
//			if (m_bUseCaliper || m_bDrawCaliper)
//				TransparentBlt(memDC.m_hDC, 0, 0, (int)rect.Width(), (int)rect.Height(), m_CaliperMemDC.m_hDC, 0, 0, (int)m_nWidth, (int)m_nHeight, RGB(0, 0, 0));
//		}
//		else
//		{
//			// Lincoln Lee - 220210
//			if (m_UseSemiTransparent)
//				AlphaBlend(memDC.m_hDC, -m_ptResult.x, -m_ptResult.y, (int)m_nWidth, (int)m_nHeight, m_OverlayMemDC.m_hDC, 0, 0, (int)(m_nWidth * m_fMagnification), (int)(m_nHeight * m_fMagnification), m_bf);
//			else
//				TransparentBlt(memDC.m_hDC, -m_ptResult.x, -m_ptResult.y, (int)m_nWidth, (int)m_nHeight, m_OverlayMemDC.m_hDC, 0, 0, (int)(m_nWidth * m_fMagnification), (int)(m_nHeight * m_fMagnification), RGB(0, 0, 0));
//
//			if (m_bUseCaliper || m_bDrawCaliper)
//				TransparentBlt(memDC.m_hDC, -m_ptResult.x, -m_ptResult.y, (int)m_nWidth, (int)m_nHeight, m_CaliperMemDC.m_hDC, 0, 0, (int)(m_nWidth * m_fMagnification), (int)(m_nHeight * m_fMagnification), RGB(0, 0, 0));
//		}
//		dc.BitBlt(0, 0, rect.Width(), rect.Height(), &memDC, 0, 0, SRCCOPY);
//	}
//	
//	if (m_bScroll)
//	{
//		CBrush brs(RGB( 41, 41, 41));
//		dc.FillRect(CRect(rect.right, rect.bottom, rect.right + SIZE_SCROLLBAR, rect.bottom + SIZE_SCROLLBAR), &brs);
//	}
//
//	memDC.SelectObject(pOldBitmap);
//	/*memDC.DeleteDC();
//	 bitmap.DeleteObject();*/
//
//	 bitmap.DeleteObject();
//	 ReleaseDC(&memDC);
//	 //DeleteDC(memDC);
//
//	//////////////////////////////////////////////////////////////////////////
//}
//
// CPoint CViewerEx::RotatePt(double dA, CPoint point,CPoint cpt)
//{
//	CPoint rpt = point;
//
//	double X,Y;
//	double ra = dA * PI / 180.;
//
//	rpt.x = point.x - cpt.x;
//	rpt.y = point.y - cpt.y;
//
//	X = cos(ra) * rpt.x - sin(ra) * rpt.y;
//	Y = sin(ra) * rpt.x + cos(ra) * rpt.y;
//
//	rpt.x = int(X + cpt.x) ;
//	rpt.y = int(Y + cpt.y) ;
//
//	return rpt;
//}
//
//void CViewerEx::OnLButtonUp(UINT nFlags, CPoint point)
//{
//	CRect rect;
//	GetClientRect(&rect);
//
//	if (m_bScroll)
//	{
//		rect.right -= SIZE_SCROLLBAR;
//		rect.bottom -= SIZE_SCROLLBAR;
//	}
//
//	if(point.x < 0 || point.x > rect.Width() || point.y <0 || point.y > rect.Height())
//		return;
//	
//	if (m_bDrawFitImage)
//	{
//		float fH = 0.0;
//		float fV = 0.0;
//		if( m_bRatioFit )
//		{
//			fH = (float)m_expectedRect.Width () / m_nWidth;
//			fV = (float)m_expectedRect.Height() / m_nHeight;
//		}
//		else
//		{
//			fH = (float)rect.Width () / m_nWidth;
//			fV = (float)rect.Height() / m_nHeight;
//		}
//
//		point.x = LONG(point.x / fH);
//		point.y = LONG(point.y / fV);
//	}
//	else
//	{
//		point.x = LONG(point.x / m_fMagnification);
//		point.y = LONG(point.y / m_fMagnification);
//	}
//
//	if (m_pDlgCaliper && m_bUseCaliper)
//	{
//		ClearCaliperOverlayDC();
//		m_pDlgCaliper->LButtonUp(point);
//		Invalidate();
//		return;
//	}
//
//	if (m_bDrawingFigure)
//	{
//		NMHDR nmHdr;
//		::ZeroMemory(&nmHdr, sizeof(NMHDR));
//
//		nmHdr.hwndFrom = m_hWnd;
//		nmHdr.idFrom = GetDlgCtrlID();
//		nmHdr.code = WM_DRAWING_FINISH_MESSAGE;
//		GetParent()->SendMessage(WM_NOTIFY, (LPARAM)NULL, (WPARAM)&nmHdr);
//	}
//
//	
//	m_ptLup = m_ptResult;
//
//	if (m_bModifyFigure)
//	{
//		m_rcPreFigure.left	 = m_stFigure[m_nCurrFigure].ptBegin.x;
//		m_rcPreFigure.top	 = m_stFigure[m_nCurrFigure].ptBegin.y;
//		m_rcPreFigure.right	 = m_stFigure[m_nCurrFigure].ptEnd.x;
//		m_rcPreFigure.bottom = m_stFigure[m_nCurrFigure].ptEnd.y;
//	}
//
//	if( m_bUseMask && m_bMaskClick )
//	{
//		m_bMaskClick = FALSE;
//
//		if( m_bMaskRect )
//		{
//			CRect rect;
//			CPoint ptTemp;
//			GetClientRect(&rect);
//
//			if( (rect.right - point.x) <= 10 ) point.x = rect.right;
//			if( point.x <= 10 ) point.x = 0;
//
//			if( (rect.bottom - point.y) <= 10 ) point.y = rect.bottom;
//			if( point.y <= 10 ) point.y = 0;
//
//			CPoint pt1, pt2;
//
//			pt1 = m_ptLDown;
//			pt2 = point;
//
//			if( pt2.x < pt1.x )
//			{
//				ptTemp = pt2;
//				pt2 = pt1;
//				pt1 = ptTemp;
//			}
//
//			DrawRectMask(pt1, pt2);
//			Invalidate();
//			//loadGrayRaw();
//		}
//	}
//
//	m_bDrawStart = false;
//
//	::SendMessage(GetParent()->m_hWnd, WM_LBUTTONUP, 0, 0);
//
//	CWnd::OnLButtonUp(nFlags, point);
//}
//
//void CViewerEx::OnLButtonDown(UINT nFlags, CPoint point)
//{
//	CRect rect;
//	GetClientRect(&rect);
//
//	if (m_bScroll)
//	{
//		rect.right -= SIZE_SCROLLBAR;
//		rect.bottom -= SIZE_SCROLLBAR;
//	}
//
//	// 21.12.24 htk 
//	if (m_bNavi)
//	{
//		int w = rect.Width() / 4;
//		int h = rect.Height() / 4;
//		int orgx = rect.Width() / 2 - w / 2;
//		int orgy = rect.Height() - h - 20;
//		float fH = (float)w / m_nWidth;
//		float fV = (float)h / m_nHeight;
//
//		CRect rc = CRect(orgx, orgy, orgx + w, orgy + h);
//		CPoint naviPt = CPoint(LONG(point.x * m_fMagnification), LONG(point.y * m_fMagnification));
//
//		if (rc.PtInRect(naviPt))
//		{
//			naviPt.x = naviPt.x - orgx;
//			naviPt.y = naviPt.y - orgy;
//
//			m_ptResult.x = LONG(naviPt.x / fH - (w * 2) / m_fMagnification);
//			m_ptResult.y = LONG(naviPt.y / fV - (h * 2) / m_fMagnification);
//			Invalidate();
//			return;
//		}
//	}
//
//	if( m_bRatioFit )
//	{
//		if (m_bDrawFitImage)
//		{
//			int dx = (rect.Width() - m_expectedRect.Width()) / 2;
//			int dy = (rect.Height() - m_expectedRect.Height()) / 2;
//
//			point.x -= dx;
//			point.y -= dy;
//		}
//	}
//
//	if(point.x < 0 || point.x > rect.Width() || point.y <0 || point.y > rect.Height())
//		return;
//
//	if (m_bDrawFitImage)
//	{
//		float fH, fV;
//		if( m_bRatioFit )
//		{
//			fH = (float)m_expectedRect.Width () / m_nWidth;
//			fV = (float)m_expectedRect.Height() / m_nHeight;
//		}
//		else
//		{
//			fH = (float)rect.Width () / m_nWidth;
//			fV = (float)rect.Height() / m_nHeight;
//		}
//
//		point.x = LONG(point.x / fH);
//		point.y = LONG(point.y / fV);
//	}
//	else
//	{
//		point.x = LONG(point.x / m_fMagnification);
//		point.y = LONG(point.y / m_fMagnification);
//	}
//
//	if (m_pDlgCaliper && m_bUseCaliper)
//	{
//		ClearCaliperOverlayDC();
//		m_pDlgCaliper->LButtonDown(point);
//		Invalidate();
//		return;
//	}
//
//	m_ptLDown = point;
//
//	if ( m_bDrawingFigure && !m_isMoving && !m_isHitNW && !m_isHitN  && !m_isHitNE && !m_isHitE  && !m_isHitSE && !m_isHitS && !m_isHitSW && !m_isHitW) 
//	{
//		m_stFigure[m_nCurrFigure].ptBegin = m_ptLDown + m_ptResult;
//		m_bDrawStart = true;
//	}
//	else m_bDrawStart = false;	
//
//	//GetParent()->SendMessage(WM_LINE_TRACKER_MESSAGE,(WPARAM)nFlags, (LPARAM)&point);
//	::SendMessage(GetParent()->m_hWnd, WM_LBUTTONDOWN, 0, 0);
//	CWnd::OnLButtonDown(nFlags, point);
//}
//
//void CViewerEx::OnRButtonUp(UINT nFlags, CPoint point)
//{
//	CWnd::OnRButtonUp(nFlags, point);
//}
//
//
//void CViewerEx::OnContextMenu(CWnd* /*pWnd*/, CPoint point)
//{
//	if (m_bMenu)
//		m_pMenu->TrackPopupMenu(TPM_RIGHTBUTTON | TPM_LEFTALIGN, point.x, point.y, this);
//}
//
//void CViewerEx::CreateMousePopupMenu()
//{
//	m_pSubZoom = new CMenu();
//	m_pSubZoom->CreatePopupMenu();
//	m_pSubZoom->InsertMenu(-1,MF_BYPOSITION, ID_MENU_ZOOM_X0125, _T("x0.125"	));
//	m_pSubZoom->InsertMenu(-1,MF_BYPOSITION, ID_MENU_ZOOM_X025	, _T("x0.25"	));
//	m_pSubZoom->InsertMenu(-1,MF_BYPOSITION, ID_MENU_ZOOM_X05	, _T("x0.5"	));
//	m_pSubZoom->InsertMenu(-1,MF_BYPOSITION, ID_MENU_ZOOM_X1	, _T("x1"		));
//	m_pSubZoom->InsertMenu(-1,MF_BYPOSITION, ID_MENU_ZOOM_X2	, _T("x2"		));
//	m_pSubZoom->InsertMenu(-1,MF_BYPOSITION, ID_MENU_ZOOM_X4	, _T("x4"		));
//	m_pSubZoom->InsertMenu(-1,MF_BYPOSITION, ID_MENU_ZOOM_X8	, _T("x8"		));
//	m_pSubZoom->InsertMenu(-1,MF_BYPOSITION, ID_MENU_ZOOM_X16	, _T("x16"		));
//	m_pSubZoom->InsertMenu(-1,MF_BYPOSITION, ID_MENU_ZOOM_X32	, _T("x32"		));
//
//	m_pSubMode = new CMenu();
//	m_pSubMode->CreatePopupMenu();
//	m_pSubMode->InsertMenu(-1,MF_BYPOSITION, ID_MENU_MOUSE, _T("Mouse"));
//	m_pSubMode->InsertMenu(-1,MF_BYPOSITION, ID_MENU_SCROLL, _T("Scroll"));
//
//	m_pMenu = new CMenu();
//	m_pMenu->CreatePopupMenu();
//	m_pMenu->InsertMenu(-1,MF_BYPOSITION, ID_MENU_SAVE, _T("Save..."));
//	m_pMenu->InsertMenu(-1,MF_BYPOSITION, ID_MENU_LOAD, _T("Load..."));
//	m_pMenu->AppendMenu(MF_POPUP,(UINT_PTR)m_pSubZoom->m_hMenu, _T("Zoom"));
//	m_pMenu->InsertMenu(-1,MF_BYPOSITION, ID_MENU_FIT, _T("Fit Image"));
//	m_pMenu->InsertMenu(-1,MF_BYPOSITION, ID_MENU_NEVIGATION, _T("Navigation"));
//	m_pMenu->AppendMenu(MF_POPUP,(UINT_PTR)m_pSubMode->m_hMenu, _T("Mode"));
//
//	m_pSubZoom->CheckMenuItem  (ID_MENU_ZOOM_X1	, MF_CHECKED);		//x1
//	m_pSubMode->CheckMenuItem  (ID_MENU_MOUSE	, MF_CHECKED);		//Mouse
//	m_pMenu->EnableMenuItem(ID_MENU_SAVE, MF_DISABLED);			//Save
//	m_pMenu->EnableMenuItem(ID_MENU_FIT, MF_DISABLED);			//Fit
//	m_pMenu->EnableMenuItem(ID_MENU_NEVIGATION, MF_DISABLED);			//Navigation
//	m_pMenu->EnableMenuItem((UINT_PTR)m_pSubZoom->m_hMenu, MF_DISABLED);
//
//	CheckMenuItem(ID_MENU_ZOOM_X1, false);
//	CheckMenuItem(ID_MENU_MOUSE, true);
//}
//
//void CViewerEx::OnFitImage()
//{
//	m_bDrawFitImage = true;
//
//	CheckMenuItem(ID_MENU_FIT, false);
//	
//	m_wndSBH.EnableWindow(FALSE);
//	m_wndSBV.EnableWindow(FALSE);
//
//	Invalidate();
//	return;
//}
//
//void CViewerEx::OnNavigation()
//{
//	if (m_bNavi == false)
//	{
//		m_bNavi = true;
//		m_pMenu->CheckMenuItem(ID_MENU_NEVIGATION, MF_CHECKED);
//	}
//	else
//	{
//		m_bNavi = false;
//		m_pMenu->CheckMenuItem(ID_MENU_NEVIGATION, MF_UNCHECKED);
//	}
//
//	Invalidate();
//	return;
//}
//
//void CViewerEx::OnSaveImage()
//{
//	if (m_ImgSrc.IsNull()) return;
//
//	CString strFilter = _T("All Files (*.*)|*.*||");
//
//	CFileDialog FileDlg(FALSE, _T(".bmp"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, strFilter);
//	if( FileDlg.DoModal() == IDOK )
//	{
//		m_ImgSrc.Save(FileDlg.GetPathName());
//	}
//}
//
//void CViewerEx::OnLoadImage()
//{
//	CString strFilter = _T("All Files (*.*)|*.*||");
//
//	CFileDialog FileDlg(TRUE, _T(".BMP"), NULL, 0, strFilter);
//	
//	if( FileDlg.DoModal() == IDOK )
//	{
//		OnLoad(FileDlg.GetPathName());
//
//		NMHDR nmHdr;
//		::ZeroMemory(&nmHdr, sizeof(NMHDR));
//
//		nmHdr.hwndFrom = m_hWnd;
//		nmHdr.idFrom = GetDlgCtrlID();
//		nmHdr.code = WM_LOAD_IMAGE_MESSAGE;
//		GetParent()->SendMessage(WM_NOTIFY, (LPARAM)NULL, (WPARAM)&nmHdr);
//	}
//}
//void CViewerEx::OnLoadImage(CString strpath)
//{
//	OnLoad(strpath);
//
//	NMHDR nmHdr;
//	::ZeroMemory(&nmHdr, sizeof(NMHDR));
//
//	nmHdr.hwndFrom = m_hWnd;
//	nmHdr.idFrom = GetDlgCtrlID();
//	nmHdr.code = WM_LOAD_IMAGE_MESSAGE;
//	GetParent()->SendMessage(WM_NOTIFY, (LPARAM)NULL, (WPARAM)&nmHdr);
//}
//void CViewerEx::OnSetZoom(UINT wParam)
//{
//	int nMenuID = (int)wParam;
//	switch(nMenuID)
//	{
//	case ID_MENU_ZOOM_X0125:
//		m_fMagnification=0.125;
//		break;
//	case ID_MENU_ZOOM_X025:
//		m_fMagnification=0.25;
//		break;
//	case ID_MENU_ZOOM_X05:
//		m_fMagnification=0.5;
//		break;
//	case ID_MENU_ZOOM_X1:
//		m_fMagnification=1;
//		break;
//	case ID_MENU_ZOOM_X2:
//		m_fMagnification=2;
//		break;
//	case ID_MENU_ZOOM_X4:
//		m_fMagnification=4;
//		break;
//	case ID_MENU_ZOOM_X8:
//		m_fMagnification=8;
//		break;
//	case ID_MENU_ZOOM_X16:
//		m_fMagnification=16;
//		break;
//	case ID_MENU_ZOOM_X32:
//		m_fMagnification=32;
//		break;
//	}
//
//	m_bDrawFitImage = false;
//
//	CheckMenuItem(nMenuID,false);
//	m_bClear = true;
//
//	ResetScrollbarInfo();
//
//	Invalidate();
//}
//
//void CViewerEx::OnSetMode(UINT wParam)
//{
//	int nMenuID = (int)wParam;
//	switch(nMenuID)
//	{
//		// Mouse
//	case ID_MENU_MOUSE:
//		m_bScroll = false;	
//		m_wndSBH.ShowWindow(SW_HIDE);
//		m_wndSBV.ShowWindow(SW_HIDE);
//
//		//Mouse <-> Scroll 변환 후 이미지 이동시 보정하는 부분.
//		m_ptLup = m_ptResult;
//		m_ptLDown = m_ptResult;
//		break;
//		// Scroll
//	case ID_MENU_SCROLL:
//		m_bScroll = true;
//		m_wndSBH.ShowWindow(SW_SHOW);
//		m_wndSBV.ShowWindow(SW_SHOW);
//
//		//Mouse <-> Scroll 변환 후 이미지 이동시 보정하는 부분.
//		ResetScrollbarInfo();
//		break;
//	}
//
//	CheckMenuItem(nMenuID,true);
//	m_bClear = true;
//
//	Invalidate();
//}
//
//void CViewerEx::CheckMenuItem(int nID, bool bMode)
//{
//	if (bMode)
//	{
//		for (int i=ID_MENU_MOUSE; i<ID_MENU_MAX; i++)
//		{
//			if (i == nID)
//				m_pSubMode->CheckMenuItem(i, MF_CHECKED);
//			else
//				m_pSubMode->CheckMenuItem(i, MF_UNCHECKED);
//		}
//	}
//	else
//	{
//		for (int i=ID_MENU_ZOOM_X0125; i<ID_MENU_FIT+1; i++)
//		{
//			if (i < ID_MENU_ZOOM_MAX)
//			{
//				if (i == nID)
//					m_pSubZoom->CheckMenuItem(i, MF_CHECKED);
//				else
//					m_pSubZoom->CheckMenuItem(i, MF_UNCHECKED);
//			}
//			else
//			{
//				if (i == nID)
//					m_pMenu->CheckMenuItem(i, MF_CHECKED);
//				else
//					m_pMenu->CheckMenuItem(i, MF_UNCHECKED);
//			}
//		}
//	}
//}
//
//void CViewerEx::SetEnableFigureLine(int iIndex)
//{
//	if (iIndex<0 || iIndex>=MAX_DRAW) return;
//
//	m_stFigure[iIndex].isLine		= true;
//	m_stFigure[iIndex].isRectangle	= false;
//	m_stFigure[iIndex].isCircle		= false;
//}
//
//void CViewerEx::SetEnableFigureRectangle(int iIndex)
//{
//	if(iIndex < 0 || iIndex >= MAX_DRAW) return;
//
//	m_stFigure[iIndex].isLine		= false;
//	m_stFigure[iIndex].isRectangle	= true;
//	m_stFigure[iIndex].isCircle		= false;
//}
//
//void CViewerEx::SetEnableFigureCircle(int iIndex)
//{
//	if(iIndex < 0 || iIndex >= MAX_DRAW) return;
//
//	m_stFigure[iIndex].isLine		= false;
//	m_stFigure[iIndex].isRectangle	= false;
//	m_stFigure[iIndex].isCircle		= true;
//}
//
//void CViewerEx::SetEnableModifyFigure(int iIndex, bool isEnable) 
//{ 
//	if (iIndex < 0 || iIndex >= MAX_DRAW) return; 
//
//	m_nCurrFigure = iIndex; 
//	m_bModifyFigure = isEnable; 
//	
//	m_rcPreFigure.left   = m_stFigure[m_nCurrFigure].ptBegin.x;
//	m_rcPreFigure.top    = m_stFigure[m_nCurrFigure].ptBegin.y;
//	m_rcPreFigure.right  = m_stFigure[m_nCurrFigure].ptEnd.x;
//	m_rcPreFigure.bottom = m_stFigure[m_nCurrFigure].ptEnd.y;
//
//	Invalidate();
//}
//
//void CViewerEx::ResetScrollbarInfo()
//{
//	CRect rect;
//	GetWindowRect(rect);
//
//	//rect.right -= SIZE_SCROLLBAR;
//	//rect.bottom -= SIZE_SCROLLBAR;
//
//	//int nScrollBarSize = m_nWidth*m_fMagnification - rect.Width();
//	int nScrollBarSize = int(m_nWidth*m_fMagnification - rect.Width() + SIZE_SCROLLBAR);
//	nScrollBarSize = nScrollBarSize > 0 ? nScrollBarSize: 0;
//
//	SCROLLINFO si;
//	si.fMask = SIF_ALL;
//	si.nPage = nScrollBarSize/10;
//	//si.nMax = nScrollBarSize + 34 + si.nPage;
//	si.nMax = nScrollBarSize;//+ si.nPage;
//	si.nMin = 0;
//	si.nTrackPos = 0;
//	si.nPos = int(m_ptResult.x*m_fMagnification);
//
//	m_wndSBH.SetScrollRange(0,si.nMax);
//	m_wndSBH.SetScrollPos(si.nPos);
//
//	//nScrollBarSize = m_nHeight*m_fMagnification - rect.Height();
//	nScrollBarSize = int(m_nHeight*m_fMagnification - rect.Height() + SIZE_SCROLLBAR);
//	nScrollBarSize = nScrollBarSize > 0 ? nScrollBarSize: 0;
//
//	si.fMask = SIF_ALL;
//	si.nPage = nScrollBarSize/10;
//	//si.nMax = nScrollBarSize + 34 + si.nPage;	
//	si.nMax = nScrollBarSize;// + si.nPage;	
//	si.nMin = 0;
//	si.nTrackPos = 0;
//	si.nPos = int(m_ptResult.y*m_fMagnification);
//
//	m_wndSBV.SetScrollRange(0,si.nMax);
//	m_wndSBV.SetScrollPos(si.nPos);
//
//	m_nWidth  * m_fMagnification < rect.Width()  ? m_wndSBH.EnableWindow(FALSE): m_wndSBH.EnableWindow(TRUE);
//	m_nHeight * m_fMagnification < rect.Height() ? m_wndSBV.EnableWindow(FALSE): m_wndSBV.EnableWindow(TRUE);
//
//	if (m_bDrawFitImage)
//	{
//		m_wndSBH.EnableWindow(FALSE);
//		m_wndSBV.EnableWindow(FALSE);
//	}
//}
//
//void CViewerEx::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
//{
//	if (pScrollBar->m_hWnd == m_wndSBH.m_hWnd)
//	{
//		int pos = m_wndSBH.GetScrollPos();
//		m_ptResult.x = pos;
//		m_ptResult.x = LONG(m_ptResult.x /m_fMagnification);
//		Invalidate();
//	}
//
//	CWnd::OnHScroll(nSBCode, nPos, pScrollBar);
//}
//
//
//void CViewerEx::OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
//{
//	if (pScrollBar->m_hWnd == m_wndSBV.m_hWnd)
//	{
//		int pos = m_wndSBV.GetScrollPos();
//		m_ptResult.y = pos;
//		m_ptResult.y = LONG(m_ptResult.y /m_fMagnification);
//		Invalidate();
//	}
//
//	CWnd::OnVScroll(nSBCode, nPos, pScrollBar);
//}
//
//// 두 점 사이의 거리 계산.
//float CViewerEx::CalculateLength(CPoint pt1, CPoint pt2)
//{
//	CRect rect;
//	GetClientRect(rect);
//	
//	float a=0.0, b=0.0, c=0.0;
//	if (m_bDrawFitImage)
//	{
//		float fX = (float)rect.Width()/m_nWidth;
//		float fY = (float)rect.Height()/m_nHeight;
//		pt1.x = LONG(pt1.x /fX);
//		pt1.y = LONG(pt1.y /fY);
//		pt2.x = LONG(pt2.x /fX);
//		pt2.y = LONG(pt2.y /fY);
//	}
//	else
//	{
//		pt1.x = LONG(pt1.x /m_fMagnification);
//		pt1.y = LONG(pt1.y /m_fMagnification);
//		pt2.x = LONG(pt2.x /m_fMagnification);
//		pt2.y = LONG(pt2.y /m_fMagnification);
//	}
//	
//	a = (float)(pt1.x - pt2.x);
//	b = (float)(pt1.y - pt2.y);
//	c = sqrt(a*a+b*b);
//
//	return c;
//}
//
//void CViewerEx::SetEnableMenuSave(bool bEnable)
//{
//	if (bEnable)
//		m_pMenu->EnableMenuItem(ID_MENU_SAVE, MF_ENABLED);
//	else
//		m_pMenu->EnableMenuItem(ID_MENU_SAVE, MF_DISABLED);
//}
//
//void CViewerEx::SetEnableMenuLoad(bool bEnable)
//{
//	if (bEnable)
//		m_pMenu->EnableMenuItem(ID_MENU_LOAD, MF_ENABLED);
//	else
//		m_pMenu->EnableMenuItem(ID_MENU_LOAD, MF_DISABLED);
//}
//
//void CViewerEx::SetSizeRectangle(int iIndex, CPoint pt1, CPoint pt2)
//{
//	if (iIndex<0 || iIndex>=MAX_DRAW) return;
//
//	if (m_stFigure[iIndex].isRectangle == false) return;
//
//	m_stFigure[iIndex].ptBegin = pt1;
//	m_stFigure[iIndex].ptEnd   = pt2;
//}
//void CViewerEx::SetSizeLine(int iIndex, CPoint pt1, CPoint pt2)
//{
//	if (iIndex < 0 || iIndex >= MAX_DRAW) return;
//
//	if (m_stFigure[iIndex].isLine == false) return;
//
//	m_stFigure[iIndex].ptBegin = pt1;
//	m_stFigure[iIndex].ptEnd = pt2;
//}
//void CViewerEx::SetSizeCircle(int iIndex, CPoint pt1, CPoint pt2)
//{
//	if (iIndex<0 || iIndex>=MAX_DRAW) return;
//
//	if (m_stFigure[iIndex].isCircle == false) return;
//
//	m_stFigure[iIndex].ptBegin = pt1;
//	m_stFigure[iIndex].ptEnd   = pt2;
//}
//void CViewerEx::SetSizeRectangle(int iIndex, int nOrgX, int nOrgY, int nWidth, int nHeight)
//{
//	if (iIndex<0 || iIndex>=MAX_DRAW) return;
//
//	if (m_stFigure[iIndex].isRectangle == false) return;
//
//	m_stFigure[iIndex].ptBegin = CPoint(nOrgX,nOrgY);
//	m_stFigure[iIndex].ptEnd   = CPoint(nOrgX+nWidth-1,nOrgY+nHeight-1);
//}
//
//int CViewerEx::GetValueY(int nPosX, int nPosY)
//{
//	if (m_ImgSrc.IsNull()) return 0;
//
//	if (nPosX < 0 || nPosX >= m_nWidth)  return 0;
//	if (nPosY < 0 || nPosY >= m_nHeight) return 0;
//
//	BYTE* ptr = (BYTE*)m_ImgSrc.GetPixelAddress(nPosX,nPosY);
//
//	int value = *ptr;
//
//	return value;
//}
//
//COLORREF CViewerEx::GetValueRGB(int nPosX, int nPosY)
//{
//	if (m_ImgSrc.IsNull() == true) return RGB(0,0,0);
//	if (nPosX < 0 || nPosX >= m_nWidth)  return 0;
//	if (nPosY < 0 || nPosY >= m_nHeight) return 0;
//
//	COLORREF clr = *((COLORREF*)m_ImgSrc.GetPixelAddress(nPosX,nPosY));
//
//	//BYTE r = GetBValue(clr);
//	//BYTE g = GetGValue(clr);
//	//BYTE b = GetRValue(clr);
//
//	return clr;
//}
//
//void CViewerEx::OnDropFiles(HDROP hDropInfo)
//{
//	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
//
//	int nFiles,n=0;		
//	char m_sFilePath[MAX_PATH];
//	CString path;
//
//	nFiles = ::DragQueryFileA( hDropInfo, 0xFFFFFFFF, (LPSTR)(LPCTSTR)m_sFilePath, MAX_PATH );	
//
//	for(int i = 0; i < nFiles; i++)
//	{
//		::DragQueryFileA( hDropInfo, i, (LPSTR)(LPCTSTR) m_sFilePath, MAX_PATH);
//		path = m_sFilePath;
//
//		if( path.Find(".jpg") > 0 || path.Find(".bmp") > 0 || path.Find(".JPG") > 0 || path.Find(".BMP") > 0 ||
//			path.Find(".png") > 0 || path.Find(".PNG") > 0 )
//		{
//			OnLoad((LPCTSTR)m_sFilePath);
//
//			NMHDR nmHdr;
//			::ZeroMemory(&nmHdr, sizeof(NMHDR));
//
//			nmHdr.hwndFrom = m_hWnd;
//			nmHdr.idFrom = GetDlgCtrlID();
//			nmHdr.code = WM_LOAD_IMAGE_MESSAGE;
//			GetParent()->SendMessage(WM_NOTIFY, (LPARAM)NULL, (WPARAM)&nmHdr);
//			n++;
//		}
//		else
//			AfxMessageBox("Only BMP or JPG files are available.");
//	}
//
//	DragFinish(hDropInfo);
//
//	Invalidate();
//
//	CWnd::OnDropFiles(hDropInfo);
//}
//
//
//void CViewerEx::SetMaskPos(CPoint point)
//{
//	CDC *pDC = getMaskOverlayDC();
//	CPen pen( PS_SOLID, 1, RGB(255, 0, 0));
//	CBrush br(RGB(255, 0, 0)), *oldBr;
//	CPen *oldPen;
//	
//	
//
//	oldPen = pDC->SelectObject( &pen );
//	oldBr = pDC->SelectObject(&br);
//	
//	int nSize = 10;
//	pDC->Ellipse((int)(point.x / (double)m_fMagnification + 0.5) - nSize, (int)(point.y / (double)m_fMagnification + 0.5) - nSize, 
//		(int)(point.x / (double)m_fMagnification + 0.5) + nSize, (int)(point.y / (double)m_fMagnification + 0.5) + nSize);
//
//	pDC->SelectObject(oldBr);
//	pDC->SelectObject( oldPen );
//}
//
//
//
//BOOL CViewerEx::DrawRect(CPoint pt1, CPoint pt2)
//{
//	BOOL bSuccess = TRUE;
//	ClearRectMaskOverlayDC();
//	CDC * pDC = getRectMaskOverlayDC();
//	
//	CPen pen(PS_DOT, 1, RGB(0, 0, 255)), *oldPen;
//	CBrush br(NULL_BRUSH), *oldBr;
//	oldPen = pDC->SelectObject( &pen );
//	//oldBr = pDC->SelectObject( &br );
//	oldBr = (CBrush*)pDC->SelectStockObject(NULL_BRUSH);
//	pDC->Rectangle((int)(pt1.x / (double)m_fMagnification + 0.5), (int)(pt1.y / (double)m_fMagnification + 0.5), 
//		(int)(pt2.x / (double)m_fMagnification + 0.5), (int)(pt2.y / (double)m_fMagnification + 0.5));
//
//	pDC->SelectObject( oldPen );
//	pDC->SelectObject( oldBr );
//
//	return bSuccess;
//}
//
//
//
//void CViewerEx::DrawRectMask(CPoint pt1, CPoint pt2)
//{
//	ClearRectMaskOverlayDC();
//
//	CDC *pDC = getMaskOverlayDC();
//	
//	CBrush br(RGB(255, 0, 0)), *oldBr;
//	CPen *oldPen;
//	oldPen = (CPen*)pDC->SelectStockObject(NULL_PEN);
//	oldBr = pDC->SelectObject( &br );
//
//	pDC->Rectangle((int)(pt1.x / (double)m_fMagnification + 0.5) - 5, (int)(pt1.y / (double)m_fMagnification + 0.5) - 5, 
//		(int)(pt2.x / (double)m_fMagnification + 0.5) + 5, (int)(pt2.y / (double)m_fMagnification + 0.5) + 5);
//
//	pDC->SelectObject( oldBr );
//	pDC->SelectObject( oldPen );
//
//}
//
//
//double CViewerEx::getMinMagnification()
//{
//	CRect rect;
//	GetClientRect( &rect );
//
//	double aspectRatio = m_nHeight / (double)m_nWidth;
//	double aspectRatio2 = rect.Height() / (double)rect.Width();
//
//	if( aspectRatio < aspectRatio2 )	return rect.Width() / (double)m_nWidth;
//	else								return rect.Height() / (double)m_nHeight;
//}
//
//void CViewerEx::setZoomIn()
//{
//	float mag = GetMagnification();
//
//	if( IsFitImage() )
//	{
//		setOriginalImage();
//		mag = GetExpectedRect().Width() / (double)GetWidth();
//	}
//		
//	// 화면 중앙 기준으로 확대/ 축소 되도록 - 1
//	CRect rect;
//	CPoint pt = getResult();
//	GetClientRect(&rect);
//	/////////////
//	pt.x = pt.x + (rect.Width() / 2) / mag;
//	pt.y = pt.y + (rect.Height() / 2) / mag;
//
//	mag *= 1.25;
//	if( mag > 2.0 ) mag = 2.0;
//	if( mag < getMinMagnification() ) mag = getMinMagnification();
//	
//	if (mag < 0.125) mag = 0.125;
//	// 화면 중앙 기준으로 확대/ 축소 되도록 - 2
//	pt.x = pt.x - (rect.Width() / 2) / mag;
//	pt.y = pt.y - (rect.Height() / 2) / mag;
//	setResult(pt);
//	setLup(pt);
//	setLDown(pt);
//	/////////////
//
//	SetMagnification(mag);
//	if (mag >= 1.0 && mag < 1.25)
//	{
//		m_bNavi = true; 
//		m_pMenu->CheckMenuItem(ID_MENU_NEVIGATION, MF_CHECKED); //200208
//	}
//
//	m_bDrawFitImage = false;
//	m_bClear = true;
//
//	Invalidate();
//}
//
//
//void CViewerEx::setZoomOut()
//{
//	float mag = GetMagnification();
//	
//	// 화면 중앙 기준으로 확대/ 축소 되도록 - 1
//	CRect rect;
//	CPoint pt = getResult();
//	GetClientRect(&rect);
//	pt.x = pt.x + (rect.Width() / 2) / mag;
//	pt.y = pt.y + (rect.Height() / 2) / mag;
//	/////////////
//
//	mag *= 0.75;
//
//	if( mag < getMinMagnification() ) 
//	{
//		mag = getMinMagnification();
//		SetFitMode();
//	}
//
//	// 화면 중앙 기준으로 확대/ 축소 되도록 - 2
//	pt.x = pt.x - (rect.Width() / 2) / mag;
//	pt.y = pt.y - (rect.Height() / 2) / mag;
//	setResult(pt);
//	setLup(pt);
//	setLDown(pt);
//	/////////////
//
//	SetMagnification(mag);
//	if (mag < 1.0 && mag >= 0.75)
//	{
//		m_bNavi = false;
//		m_pMenu->CheckMenuItem(ID_MENU_NEVIGATION, MF_UNCHECKED);  //200208
//	}
//	Invalidate();
//}
//
//void CViewerEx::drawAllFigures(CDC &memDC)
//{
//	CRect rect;
//	GetClientRect(&rect);
//
//	float fH = m_fMagnification, fV = m_fMagnification;
//	if (m_bDrawFitImage)
//	{
//		fH = (float)rect.Width() / m_nWidth;
//		fV = (float)rect.Height() / m_nHeight;
//	}
//
//	for (int i = 0; i < MAX_DRAW; i++)
//	{
//		if (!m_stFigure[i].isDraw) continue;
//
//		CPen FigurePen, WhitePen, *pOldPen;
//		CPen realPen(PS_DOT, 1, COLOR_LIME);
//		if (m_stFigure[i].isDot)	FigurePen.CreatePen(PS_DOT, m_stFigure[i].nWidth, m_stFigure[i].clrColor);
//		else
//		{
//			if (m_bDrawFitImage)	FigurePen.CreatePen(PS_SOLID, m_stFigure[i].nWidth + 1, m_stFigure[i].clrColor);
//			else					FigurePen.CreatePen(PS_SOLID, m_stFigure[i].nWidth, m_stFigure[i].clrColor);
//		}
//
//		WhitePen.CreatePen(PS_SOLID, 1, RGB(255, 255, 255));
//		pOldPen = (CPen*)memDC.SelectObject(&FigurePen);
//
//		CPoint ptNewBegin, ptNewEnd;
//
//		ptNewBegin.x = LONG(m_stFigure[i].ptBegin.x * fH - m_ptResult.x * fH);
//		ptNewBegin.y = LONG(m_stFigure[i].ptBegin.y * fV - m_ptResult.y * fV);
//		ptNewEnd.x = LONG(m_stFigure[i].ptEnd.x   * fH - m_ptResult.x * fH);
//		ptNewEnd.y = LONG(m_stFigure[i].ptEnd.y   * fV - m_ptResult.y * fV);
//
//		// Index 및 직선을 그린다.
//		if (m_stFigure[i].isLine && m_bModifyFigure)
//		{
//			CString Index;
//			memDC.SetBkMode(TRANSPARENT);
//			memDC.SetTextColor(RGB(0, 255, 0));
//			memDC.MoveTo(ptNewBegin);
//			memDC.LineTo(ptNewEnd);
//
//			CPoint realBeginPt;
//			CPoint realEndPt;
//			int dx = abs(m_stFigure[i].ptEnd.x - m_stFigure[i].ptBegin.x);
//			int dy = abs(m_stFigure[i].ptEnd.y - m_stFigure[i].ptBegin.y);
//
//			if (dx > dy)
//			{
//				realBeginPt = ptNewBegin;
//				realEndPt = ptNewEnd;
//				realBeginPt.y = (ptNewEnd.y + ptNewBegin.y) / 2;
//				realEndPt.y = (ptNewEnd.y + ptNewBegin.y) / 2;
//			}
//			else
//			{
//				realBeginPt = ptNewBegin;
//				realEndPt = ptNewEnd;
//				realBeginPt.x = (ptNewEnd.x + ptNewBegin.x) / 2;
//				realEndPt.x = (ptNewEnd.x + ptNewBegin.x) / 2;
//			}
//
//			memDC.SelectObject(&realPen);
//			memDC.MoveTo(realBeginPt);
//			memDC.LineTo(realEndPt);
//
//			m_stFigure[i].fLength = CalculateLength(ptNewBegin, ptNewEnd);
//
//			double x = dx * m_fCameraResolutionX;
//			double y = dy * m_fCameraResolutionY;
//
//			double diagonal = sqrt(x * x + y * y);
//			double theta = 0.0;
//			if (dx != 0)	theta = atan(dy / (double)dx) *180.0 / 3.141592;
//
//			Index.Format("L = %.3fmm, T: %.3f°", diagonal, theta);
//			memDC.TextOut(ptNewBegin.x, ptNewBegin.y - 20, Index);
//
//			NMHDR nmHdr;
//			::ZeroMemory(&nmHdr, sizeof(NMHDR));
//			memDC.SelectObject(pOldPen);
//
//			nmHdr.hwndFrom = m_hWnd;
//			nmHdr.idFrom = GetDlgCtrlID();
//			nmHdr.code = WM_DRAWING_LINE_MESSAGE;
//			GetParent()->SendMessage(WM_NOTIFY, (LPARAM)NULL, (WPARAM)&nmHdr);
//		}
//		// Index 및 사각형을 그린다.
//		//else if (m_stFigure[i].isRectangle && m_bModifyFigure)
//		else if (m_stFigure[i].isRectangle)
//		{
//			CString Index;
//			memDC.SetBkMode(TRANSPARENT);
//			memDC.SetTextColor(RGB(0, 255, 0));
//
//			//HTK 2022-06-29 Mark View창에 Tracker 기능 추가
//			if (m_bDrawRectangleInside && m_bDrawRectangleInsideText)
//			{
//				//hsj 생산팀 요청으로 소수점 아래 3자리까지 디스플레이
//				double _x = (GetPointEnd(i).x - GetPointBegin(i).x) * m_fCameraResolutionX;
//				double _y = (GetPointEnd(i).y - GetPointBegin(i).y) * m_fCameraResolutionY;
//
//				Index.Format("%4.3f, %4.3f, %4.3f (mm)",_x, _y,sqrt(pow(_x,2)+pow(_y,2)));
//				memDC.TextOut(ptNewBegin.x, ptNewBegin.y - 20, Index);
//			}
//
//			memDC.SelectStockObject(NULL_BRUSH);
//			memDC.Rectangle(ptNewBegin.x, ptNewBegin.y, ptNewEnd.x, ptNewEnd.y);
//
//			if (m_bModifyFigure && m_bDrawRectangleInside)
//			{
//				memDC.MoveTo(ptNewBegin.x, int((ptNewBegin.y + ptNewEnd.y) / 2.0));
//				memDC.LineTo(ptNewEnd.x, int((ptNewBegin.y + ptNewEnd.y) / 2.0));
//
//				memDC.MoveTo(int((ptNewBegin.x + ptNewEnd.x) / 2.0), ptNewBegin.y);
//				memDC.LineTo(int((ptNewBegin.x + ptNewEnd.x) / 2.0), ptNewEnd.y);
//
//				CPen dotpen;
//				dotpen.CreatePen(PS_DOT, m_stFigure[i].nWidth, m_stFigure[i].clrColor);
//				memDC.SelectObject(&dotpen);
//
//				memDC.MoveTo(ptNewBegin.x, ptNewBegin.y);
//				memDC.LineTo(ptNewEnd.x, ptNewEnd.y);
//
//				memDC.MoveTo(ptNewEnd.x, ptNewBegin.y);
//				memDC.LineTo(ptNewBegin.x, ptNewEnd.y);
//
//				dotpen.DeleteObject();
//				memDC.SelectObject(&FigurePen);
//			}
//
//		}
//		// Index 및 원을 그린다.
//		else if (m_stFigure[i].isCircle)
//		{
//			CString Index;
//			memDC.SetBkMode(TRANSPARENT);
//			memDC.SetTextColor(RGB(0, 255, 0));
//
//			memDC.SelectStockObject(NULL_BRUSH);
//			memDC.Ellipse(ptNewBegin.x, ptNewBegin.y, ptNewEnd.x, ptNewEnd.y);
//
//			if (m_bDrawRectangleInside)
//			{
//				memDC.MoveTo(ptNewBegin.x, int((ptNewBegin.y + ptNewEnd.y) / 2.0));
//				memDC.LineTo(ptNewEnd.x, int((ptNewBegin.y + ptNewEnd.y) / 2.0));
//
//				memDC.MoveTo(int((ptNewBegin.x + ptNewEnd.x) / 2.0), ptNewBegin.y);
//				memDC.LineTo(int((ptNewBegin.x + ptNewEnd.x) / 2.0), ptNewEnd.y);
//
//				CPen dotpen;
//				dotpen.CreatePen(PS_DOT, m_stFigure[i].nWidth, m_stFigure[i].clrColor);
//				memDC.SelectObject(&dotpen);
//
//				memDC.MoveTo(ptNewBegin.x, ptNewBegin.y);
//				memDC.LineTo(ptNewEnd.x, ptNewEnd.y);
//
//				memDC.MoveTo(ptNewEnd.x, ptNewBegin.y);
//				memDC.LineTo(ptNewBegin.x, ptNewEnd.y);
//
//				dotpen.DeleteObject();
//				memDC.SelectObject(&FigurePen);
//			}
//		}
//
//		if (m_bModifyFigure && (m_stFigure[i].isCircle || m_stFigure[i].isRectangle) && (i == m_nCurrFigure))
//		{
//			int nSize = 4;
//			if (m_bDrawFitImage) nSize = 7;
//
//			m_stFigure[i].rcHit[0].left = ptNewBegin.x - nSize;
//			m_stFigure[i].rcHit[0].right = ptNewBegin.x + nSize;
//			m_stFigure[i].rcHit[0].top = ptNewBegin.y - nSize;
//			m_stFigure[i].rcHit[0].bottom = ptNewBegin.y + nSize;
//			memDC.SelectStockObject(WHITE_BRUSH);
//			memDC.Rectangle(m_stFigure[i].rcHit[0]);
//
//			m_stFigure[i].rcHit[1].left = ((ptNewBegin.x) + (ptNewEnd.x)) / 2 - nSize;
//			m_stFigure[i].rcHit[1].right = ((ptNewBegin.x) + (ptNewEnd.x)) / 2 + nSize;
//			m_stFigure[i].rcHit[1].top = ptNewBegin.y - nSize;
//			m_stFigure[i].rcHit[1].bottom = ptNewBegin.y + nSize;
//			memDC.Rectangle(m_stFigure[i].rcHit[1]);
//
//			m_stFigure[i].rcHit[2].left = ptNewEnd.x - nSize;
//			m_stFigure[i].rcHit[2].right = ptNewEnd.x + nSize;
//			m_stFigure[i].rcHit[2].top = ptNewBegin.y - nSize;
//			m_stFigure[i].rcHit[2].bottom = ptNewBegin.y + nSize;
//			memDC.Rectangle(m_stFigure[i].rcHit[2]);
//
//			m_stFigure[i].rcHit[3].left = ptNewEnd.x - nSize;
//			m_stFigure[i].rcHit[3].right = ptNewEnd.x + nSize;
//			m_stFigure[i].rcHit[3].top = ((ptNewBegin.y) + (ptNewEnd.y)) / 2 - nSize;
//			m_stFigure[i].rcHit[3].bottom = ((ptNewBegin.y) + (ptNewEnd.y)) / 2 + nSize;
//			memDC.Rectangle(m_stFigure[i].rcHit[3]);
//
//			m_stFigure[i].rcHit[4].left = ptNewEnd.x - nSize;
//			m_stFigure[i].rcHit[4].right = ptNewEnd.x + nSize;
//			m_stFigure[i].rcHit[4].top = ptNewEnd.y - nSize;
//			m_stFigure[i].rcHit[4].bottom = ptNewEnd.y + nSize;
//			memDC.Rectangle(m_stFigure[i].rcHit[4]);
//
//			m_stFigure[i].rcHit[5].left = ((ptNewBegin.x) + (ptNewEnd.x)) / 2 - nSize;
//			m_stFigure[i].rcHit[5].right = ((ptNewBegin.x) + (ptNewEnd.x)) / 2 + nSize;
//			m_stFigure[i].rcHit[5].top = ptNewEnd.y - nSize;
//			m_stFigure[i].rcHit[5].bottom = ptNewEnd.y + nSize;
//			//dc.Rectangle(m_stFigure[i].rcHit[5]);
//			memDC.Rectangle(m_stFigure[i].rcHit[5]);
//
//			m_stFigure[i].rcHit[6].left = ptNewBegin.x - nSize;
//			m_stFigure[i].rcHit[6].right = ptNewBegin.x + nSize;
//			m_stFigure[i].rcHit[6].top = ptNewEnd.y - nSize;
//			m_stFigure[i].rcHit[6].bottom = ptNewEnd.y + nSize;
//			//dc.Rectangle(m_stFigure[i].rcHit[6]);
//			memDC.Rectangle(m_stFigure[i].rcHit[6]);
//
//			m_stFigure[i].rcHit[7].left = ptNewBegin.x - nSize;
//			m_stFigure[i].rcHit[7].right = ptNewBegin.x + nSize;
//			m_stFigure[i].rcHit[7].top = ((ptNewBegin.y) + (ptNewEnd.y)) / 2 - nSize;
//			m_stFigure[i].rcHit[7].bottom = ((ptNewBegin.y) + (ptNewEnd.y)) / 2 + nSize;
//			memDC.Rectangle(m_stFigure[i].rcHit[7]);
//		}
//
//		if (m_bModifyFigure && m_stFigure[i].isLine && (i == m_nCurrFigure))
//		{
//			int nSize = 4;
//			if (m_bDrawFitImage) nSize = 7;
//
//			m_stFigure[i].rcHit[0].left = ptNewBegin.x - nSize;
//			m_stFigure[i].rcHit[0].right = ptNewBegin.x + nSize;
//			m_stFigure[i].rcHit[0].top = ptNewBegin.y - nSize;
//			m_stFigure[i].rcHit[0].bottom = ptNewBegin.y + nSize;
//
//			memDC.SelectStockObject(WHITE_BRUSH);
//			memDC.Rectangle(m_stFigure[i].rcHit[0]);
//
//			m_stFigure[i].rcHit[2].left = ptNewEnd.x - nSize;
//			m_stFigure[i].rcHit[2].right = ptNewEnd.x + nSize;
//			m_stFigure[i].rcHit[2].top = ptNewEnd.y - nSize;
//			m_stFigure[i].rcHit[2].bottom = ptNewEnd.y + nSize;
//			memDC.Rectangle(m_stFigure[i].rcHit[2]);
//		}
//
//		memDC.SelectObject(pOldPen);
//		FigurePen.DeleteObject();
//		WhitePen.DeleteObject();
//		realPen.DeleteObject();
//	}
//
//	for (int i = 0; i < MAX_DRAW; i++)
//	{
//		if (!m_stFigureText[i].bDraw) continue;
//
//		int x = m_stFigureText[i].ptBegin.x;
//		int y = m_stFigureText[i].ptBegin.y;
//
//		if (m_bDrawFitImage)
//		{
//			memDC.SetBkMode(TRANSPARENT);
//			memDC.SetTextColor(m_stFigureText[i].clrColor);
//			memDC.TextOut(x, y, m_stFigureText[i].textString);
//		}
//	}
//}
//
//void CViewerEx::drawFigure_NEW(CDC &memDC)
//{
//	AcquireSRWLockExclusive(&g_srwStrlock);
//
//	struct stFigure tempFigure;
//	CRect rect;
//	GetClientRect(&rect);
//
//	float fH = m_fMagnification, fV = m_fMagnification;
//	if (m_bDrawFitImage)
//	{
//		fH = (float)rect.Width() / m_nWidth;
//		fV = (float)rect.Height() / m_nHeight;
//	}
//
//	for (int i = 0; i < m_vtFigure.size(); i++)
//	{
//		tempFigure = m_vtFigure[i];
//
//		CPen FigurePen, *pOldPen;
//
//		
//		if (tempFigure.isDot)
//		{
//			if (m_bDrawFitImage)	FigurePen.CreatePen(PS_DOT, tempFigure.nFitWidth, tempFigure.clrColor);
//			else					FigurePen.CreatePen(PS_DOT, tempFigure.nWidth, tempFigure.clrColor);
//		}
//		else
//		{
//			if (m_bDrawFitImage)	FigurePen.CreatePen(PS_SOLID, tempFigure.nFitWidth, tempFigure.clrColor);
//			else					FigurePen.CreatePen(PS_SOLID, tempFigure.nWidth, tempFigure.clrColor);
//		}
//		
//
//		pOldPen = (CPen*)memDC.SelectObject(&FigurePen);
//
//		CPoint ptNewBegin, ptNewEnd;
//
//		if (tempFigure.isLine)
//		{
//			ptNewBegin.x = LONG(tempFigure.ptBegin.x * fH - m_ptResult.x * fH);
//			ptNewBegin.y = LONG(tempFigure.ptBegin.y * fV - m_ptResult.y * fV);
//			ptNewEnd.x = LONG(tempFigure.ptEnd.x   * fH - m_ptResult.x * fH);
//			ptNewEnd.y = LONG(tempFigure.ptEnd.y   * fV - m_ptResult.y * fV);
//			memDC.MoveTo(ptNewBegin);
//			memDC.LineTo(ptNewEnd);
//		}
//
//		if (tempFigure.isRectangle)
//		{
//			ptNewBegin.x = LONG(tempFigure.ptBegin.x * fH - m_ptResult.x * fH);
//			ptNewBegin.y = LONG(tempFigure.ptBegin.y * fV - m_ptResult.y * fV);
//			ptNewEnd.x = LONG(tempFigure.ptBegin.x   * fH - m_ptResult.x * fH);
//			ptNewEnd.y = LONG(tempFigure.ptEnd.y   * fV - m_ptResult.y * fV);
//			memDC.MoveTo(ptNewBegin);
//			memDC.LineTo(ptNewEnd);
//
//			ptNewBegin.x = LONG(tempFigure.ptBegin.x * fH - m_ptResult.x * fH);
//			ptNewBegin.y = LONG(tempFigure.ptBegin.y * fV - m_ptResult.y * fV);
//			ptNewEnd.x = LONG(tempFigure.ptEnd.x   * fH - m_ptResult.x * fH);
//			ptNewEnd.y = LONG(tempFigure.ptBegin.y   * fV - m_ptResult.y * fV);
//			memDC.MoveTo(ptNewBegin);
//			memDC.LineTo(ptNewEnd);
//
//			ptNewBegin.x = LONG(tempFigure.ptBegin.x * fH - m_ptResult.x * fH);
//			ptNewBegin.y = LONG(tempFigure.ptEnd.y * fV - m_ptResult.y * fV);
//			ptNewEnd.x = LONG(tempFigure.ptEnd.x   * fH - m_ptResult.x * fH);
//			ptNewEnd.y = LONG(tempFigure.ptEnd.y   * fV - m_ptResult.y * fV);
//			memDC.MoveTo(ptNewBegin);
//			memDC.LineTo(ptNewEnd);
//
//			ptNewBegin.x = LONG(tempFigure.ptEnd.x * fH - m_ptResult.x * fH);
//			ptNewBegin.y = LONG(tempFigure.ptBegin.y * fV - m_ptResult.y * fV);
//			ptNewEnd.x = LONG(tempFigure.ptEnd.x   * fH - m_ptResult.x * fH);
//			ptNewEnd.y = LONG(tempFigure.ptEnd.y   * fV - m_ptResult.y * fV);
//			memDC.MoveTo(ptNewBegin);
//			memDC.LineTo(ptNewEnd);
//		}
//		memDC.SelectObject(pOldPen);
//		DeleteObject(&FigurePen);
//	}
//
//	for (int i = 0; i < m_vtFigureText.size(); i++)
//	{
//		CFont font, *oldFont;
//		int nFontWidth = 10;
//		int nFontHeight = 10;
//		int nX = 0;
//		int nY = 0;
//
//		if (m_bDrawFitImage)
//		{
//			nFontWidth = m_vtFigureText[i].nFitSize;
//			nFontHeight = m_vtFigureText[i].nFitSize;
//
//			nX = m_vtFigureText[i].ptBeginFit.x * fH;
//			nY = m_vtFigureText[i].ptBeginFit.y * fV;
//		}
//		else
//		{
//			nFontWidth = m_vtFigureText[i].nOrgSize;
//			nFontHeight = m_vtFigureText[i].nOrgSize;
//
//			double ffH = (float)rect.Width() / m_nWidth;
//			double ffV = (float)rect.Height() / m_nHeight;
//
//			int nXX = m_vtFigureText[i].ptBeginFit.x * ffH;
//			int nYY = m_vtFigureText[i].ptBeginFit.y * ffV;
//
//			int dx = m_vtFigureText[i].ptBeginFit.x - m_vtFigureText[i].ptBegin.x;
//			int dy = m_vtFigureText[i].ptBeginFit.y - m_vtFigureText[i].ptBegin.y;
//
//			dx *= ffH;
//			dy *= ffV;
//
//			nX = m_vtFigureText[i].ptBegin.x * fH - m_ptResult.x * fH;
//			nY = m_vtFigureText[i].ptBegin.y * fV - m_ptResult.y * fV;
//
//			nX += dx;
//			nY += dy;
//		}
//
//		int nBkMode = NULL;
//
//		if (m_vtFigureText[i].is_show_bkg != true)
//		{
//			nBkMode = memDC.SetBkMode(TRANSPARENT);
//		}
//
//		memDC.SetTextColor(m_vtFigureText[i].clrColor);
//
//		font.CreateFont(nFontHeight, nFontWidth, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
//			OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY,
//			DEFAULT_PITCH | FF_DONTCARE, "Arial");
//		oldFont = memDC.SelectObject(&font);
//
//		memDC.TextOutA(nX, nY, m_vtFigureText[i].textString);
//		memDC.SelectObject(oldFont);
//		if (m_vtFigureText[i].is_show_bkg != true)
//		{
//			memDC.SetBkMode(nBkMode);
//		}
//
//		DeleteObject(&font);
//	}
//	
//	ReleaseSRWLockExclusive(&g_srwStrlock);
//}
//
//void CViewerEx::clearAllFigures()
//{
//	AcquireSRWLockExclusive(&g_srwStrlock);
//	m_vtFigure.clear();
//	m_vtFigureText.clear();
//	ReleaseSRWLockExclusive(&g_srwStrlock);
//}
//
//void CViewerEx::addFigureLine(struct stFigure figure, int nFitW, int nOrgW, COLORREF color)
//{
//	figure.isLine = true;
//	figure.isDraw = true;
//	figure.isCircle = false;
//	figure.isRectangle = false;
//	figure.nWidth = nOrgW;
//	figure.nFitWidth = nFitW;
//	figure.clrColor = color;
//	m_vtFigure.push_back(figure);
//}
//
//void CViewerEx::addFigureRect(struct stFigure figure, int nFitW, int nOrgW, COLORREF color)
//{
//	figure.isLine = false;
//	figure.isDraw = true;
//	figure.isCircle = false;
//	figure.isRectangle = true;
//	figure.nWidth = nOrgW;
//	figure.nFitWidth = nFitW;
//	figure.clrColor = color;
//	m_vtFigure.push_back(figure);
//}
//
//void CViewerEx::addFigureText(struct stFigureText figureText, int nFitS, int nOrgS, COLORREF color)
//{
//	figureText.nFitSize = nFitS;
//	figureText.nOrgSize = nOrgS;
//	figureText.clrColor = color;
//	m_vtFigureText.push_back(figureText);
//}
//
//void CViewerEx::drawReferencePos(CDC &memDC, int i)
//{
//	CRect rect;
//	GetClientRect(&rect);
//
//	if (m_bDrawFitImage)
//	{
//		int nSize = 20;
//		CPen refPen(PS_SOLID, 2, RGB(192, 64, 64));
//		CPen *olPen;
//		olPen = memDC.SelectObject(&refPen);
//
//		float fH = (float)rect.Width() / m_nWidth;
//		float fV = (float)rect.Height() / m_nHeight;
//
//		memDC.MoveTo(m_dbRefPosX[i] * fH - nSize, m_dbRefPosY[i] * fV);
//		memDC.LineTo(m_dbRefPosX[i] * fH + nSize, m_dbRefPosY[i] * fV);
//
//		memDC.MoveTo(m_dbRefPosX[i] * fH, m_dbRefPosY[i] * fV - nSize);
//		memDC.LineTo(m_dbRefPosX[i] * fH, m_dbRefPosY[i] * fV + nSize);
//
//		memDC.SelectObject(olPen);
//
//		DeleteObject(&refPen);
//	}
//	else
//	{
//		int nSize = 20;
//		CPen refPen(PS_SOLID, 2, RGB(192, 64, 64));
//		CPen *olPen;
//		olPen = memDC.SelectObject(&refPen);
//
//		double x = (m_dbRefPosX[i] - m_ptResult.x) * m_fMagnification;
//		double y = (m_dbRefPosY[i] - m_ptResult.y) * m_fMagnification;
//		memDC.MoveTo(x - nSize, y);
//		memDC.LineTo(x + nSize, y);
//
//		memDC.MoveTo(x, y - nSize);
//		memDC.LineTo(x, y + nSize);
//
//		memDC.SelectObject(olPen);
//		DeleteObject(&refPen);
//	}
//}
//
//void CViewerEx::saveScreenCapture(CString strPath)
//{
//	HDC h_screen_dc = ::GetDC(NULL);
//	CImage image;
//	// 현재 화면의 픽셀당 컬러 비트수 구하기
//	int color_depth = GetDeviceCaps(h_screen_dc, BITSPIXEL);
//	// 크기, 컬러비트 수를 이용한 이미지 생성
//	image.Create(GetWidth(), GetHeight(), color_depth, 0);
//	HDC hdc = image.GetDC();
//	// 이미지 클래스 객체 생성
//	// 이미지 dc에 화면 dc의 내용을 복사
//	BitBlt(hdc, 0, 0, GetWidth(), GetHeight(), m_ImgSrc.GetDC(), 0, 0, SRCCOPY);
//	//TransparentBlt(hdc, 0, 0, GetWidth(), GetHeight(), m_OverlayMemDC.m_hDC, 0, 0, GetWidth(), GetHeight(), RGB(0, 0, 0));
//	TransparentBlt(hdc, 0, 0, GetWidth(), GetHeight(), m_TotalOverlayMemDC.m_hDC, 0, 0, GetWidth(), GetHeight(), RGB(0, 0, 0));
//
//	// 이미지를 jpeg로 저장
//	 image.Save(strPath, Gdiplus::ImageFormatJPEG);
//
//	// 윈도우 dc 해제
//	::ReleaseDC(NULL, hdc);
//	::ReleaseDC(NULL, h_screen_dc);
//
//	image.ReleaseDC();
//	m_ImgSrc.ReleaseDC();
//}
////KJH 2022-07-12 View 복사기능 추가
//BOOL CViewerEx::MoveScreenCapture(CViewerEx *pViewer)
//{
//	HDC h_screen_dc = ::GetDC(NULL);
//	CImage image;
//	// 현재 화면의 픽셀당 컬러 비트수 구하기
//	int color_depth = GetDeviceCaps(h_screen_dc, BITSPIXEL);
//	// 크기, 컬러비트 수를 이용한 이미지 생성
//	image.Create(GetWidth(), GetHeight(), color_depth, 0);
//	HDC hdc = image.GetDC();
//	// 이미지 클래스 객체 생성
//	// 이미지 dc에 화면 dc의 내용을 복사
//	BitBlt(hdc, 0, 0, GetWidth(), GetHeight(), m_ImgSrc.GetDC(), 0, 0, SRCCOPY);
//	TransparentBlt(hdc, 0, 0, GetWidth(), GetHeight(), m_OverlayMemDC.m_hDC, 0, 0, GetWidth(), GetHeight(), RGB(0, 0, 0));
//	TransparentBlt(hdc, 0, 0, GetWidth(), GetHeight(), m_TotalOverlayMemDC.m_hDC, 0, 0, GetWidth(), GetHeight(), RGB(0, 0, 0));
//
//	pViewer->OnLoadImageFromPtr((BYTE*)image.GetBits());
//
//	// 윈도우 dc 해제
//	::ReleaseDC(NULL, hdc);
//	::ReleaseDC(NULL, h_screen_dc);
// 
//	image.ReleaseDC();
//	m_ImgSrc.ReleaseDC();
//	
//	return true;
//}