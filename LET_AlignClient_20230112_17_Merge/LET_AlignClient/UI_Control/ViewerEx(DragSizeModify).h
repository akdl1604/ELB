#pragma once

//----------------------------------------------------------
// ViewerEx Control
//----------------------------------------------------------
// Programmed by William Kim
//----------------------------------------------------------

// CViewerEx

#include <atlimage.h>
#include "ScrollBarEx.h"
#include "Dib.h"

#define WM_DRAWING_FINISH_MESSAGE	(WM_USER +   1)
#define WM_LOAD_IMAGE_MESSAGE		(WM_USER +   2)
#define WM_DRAWING_LINE_MESSAGE		(WM_USER +   3)
#define USER_ID						(WM_USER + 100)

#define ID_MENU_ZOOM_X0125	USER_ID + 1
#define ID_MENU_ZOOM_X025	USER_ID + 2
#define ID_MENU_ZOOM_X05	USER_ID + 3
#define ID_MENU_ZOOM_X1		USER_ID + 4
#define ID_MENU_ZOOM_X2		USER_ID + 5
#define ID_MENU_ZOOM_X4		USER_ID + 6
#define ID_MENU_ZOOM_X8		USER_ID + 7
#define ID_MENU_ZOOM_X16	USER_ID + 8
#define ID_MENU_ZOOM_X32	USER_ID + 9
#define ID_MENU_ZOOM_MAX	USER_ID + 10

#define ID_MENU_SAVE		USER_ID + 11
#define ID_MENU_LOAD		USER_ID + 12
#define ID_MENU_FIT			USER_ID + 13
#define ID_MENU_NEVIGATION	USER_ID + 14

#define ID_MENU_MOUSE		USER_ID + 15
#define ID_MENU_SCROLL		USER_ID + 16
#define ID_MENU_MAX			USER_ID + 17


#define VERTICAL_SCROLL_ID			4010
#define HORIZONTAL_SCROLL_ID		4011

#define VIEWER_WINCLASSNAME _T("VIEWEREX_CLASS")

#define MAX_DRAW 20
#define SIZE_SCROLLBAR 20

using namespace Gdiplus;


struct stFigure							//		  Hit[0]	  Hit[1]	  Hit[2]
{										//			□──────────□──────────□
	bool isDraw;		// Draw 할지.	//			│					  │
	bool isLine;		// 직선인지.		//			│					  │
	bool isRectangle;	// 사각형인지.	//			│					  │
	bool isCircle;		// 원 인지.		//	 Hit[7] □					  □ Hit[3]
	int  nWidth;		// 두께.			//			│					  │
	CPoint ptBegin;		// 시작 점.		//			│					  │
	CPoint ptEnd;		// 끝 점.		//			│					  │
	CRect  rcHit[8];	// 터치 점.		//			□──────────□──────────□
	COLORREF clrColor;	// 도형 색상.	//		  Hit[6]	  Hit[5]	  Hit[4]
	float fLength;		// 직선의 길이.
};

struct stFigureText		
{	
	bool bDraw;
	CPoint ptBegin;		
	CString textString;
	COLORREF clrColor;			
};

class CViewerEx : public CWnd
{
	DECLARE_DYNAMIC(CViewerEx)

public:	
	CViewerEx();
	virtual ~CViewerEx();

public :
	void ClearOverlayDC();
	bool InitControl(CWnd* pWnd, bool bMenu=true);
	void OnInitWithCamera(int nWidth, int nHeight, int nBpp);	//이미지 포인터를 사용하여 불러올 경우, 먼저 호출되어야 함.
	bool OnLoadImageFromPtr(BYTE* pBuffer);						//이미지 포인터로 불러올 경우.
	
	//---------- Figure Functions ----------//
	void SetEnableModifyFigure  (int iIndex, bool isEnable); // 선택한 인덱스의 도형을 수정할 것인지 설정.
	void SetCurrentFigureIndex	(int iIndex) { if (iIndex<0 || iIndex>=MAX_DRAW) return; m_nCurrFigure = iIndex; } // 현재 도형의 인덱스 설정.
	void SetModeDrawingFigure	(bool bEnable) { m_bDrawingFigure  = bEnable; } // 도형을 그리는 동작 중인지 설정.
	bool GetModeDrawingFigure() { return m_bDrawingFigure; }
	void SetEnableDrawAllFigures(bool bEnable) { m_bDrawAllFigures = bEnable; Invalidate();} // 모든 도형을 그릴 것인지 설정.
	
	//---------- Figure Option Fuctions ----------//
	void SetColorFigure			 (int iIndex, COLORREF clrColor) { if (iIndex<0 || iIndex>=MAX_DRAW) return; m_stFigure[iIndex].clrColor = clrColor	; Invalidate(); } // 선택한 도형의 색상 선택.
	void SetWidthFigure			 (int iIndex, int nWidth	   ) { if (iIndex<0 || iIndex>=MAX_DRAW) return; m_stFigure[iIndex].nWidth   = nWidth	; Invalidate(); } // 선택한 도형의 두께 선택.
	void SetEnableDrawFigure	 (int iIndex, bool bEnable	   ) { if (iIndex<0 || iIndex>=MAX_DRAW) return; m_stFigure[iIndex].isDraw	 = bEnable	; Invalidate(); } // 선택한 도형을 그릴것인지 설정.
	void SetEnableFigureLine	 (int iIndex); // 선택한 인덱스의 도형을 '선형'으로 선택.
	void SetEnableFigureCircle	 (int iIndex); // 선택한 인덱스의 도형을 '원형'으로 선택.
	void SetEnableFigureRectangle(int iIndex); // 선택한 인덱스의 도형을 '사각형'으로 선택.
	void SetSizeRectangle(int iIndex, CPoint pt1, CPoint pt2);
	void SetSizeRectangle(int iIndex, int nOrgX, int nOrgY, int nWidth, int nHeight);
	
	bool IsDrawFigure			(int iIndex) { if (iIndex<0 || iIndex>=MAX_DRAW) return false; return m_stFigure[iIndex].isDraw;	 } // 선택한 도형의 Draw 여부 확인.
	bool IsEnableFigureLine		(int iIndex) { if (iIndex<0 || iIndex>=MAX_DRAW) return false; return m_stFigure[iIndex].isLine;	 } // 선택한 도형이 '선형'인지 확인.
	bool IsEnableFigureCircle	(int iIndex) { if (iIndex<0 || iIndex>=MAX_DRAW) return false; return m_stFigure[iIndex].isCircle;	 } // 선택한 도형이 '원형'인지 확인.
	bool IsEnableFigureRectangle(int iIndex) { if (iIndex<0 || iIndex>=MAX_DRAW) return false; return m_stFigure[iIndex].isRectangle;} // 선택한 도형이 '사각형'인지 확인.
	int  GetWidthFigure			(int iIndex) { if (iIndex<0 || iIndex>=MAX_DRAW) return 0;	   return m_stFigure[iIndex].nWidth;	 } // 선택한 도형의 두께 확인.
	COLORREF GetColorFigure		(int iIndex) { if (iIndex<0 || iIndex>=MAX_DRAW) return 0;	   return m_stFigure[iIndex].clrColor;	 } // 선택한 도형의 색상 확인.
	float GetLineLength			(int iIndex) { if (iIndex<0 || iIndex>=MAX_DRAW) return 0; if (m_stFigure[iIndex].isLine) return m_stFigure[iIndex].fLength; else return 0; } // 선택한 도형의 직선 거리 반환.
	CPoint GetPointBegin		(int iIndex) { if (iIndex<0 || iIndex>=MAX_DRAW) return 0;	   return m_stFigure[iIndex].ptBegin;	 } // 선택한 도형의 시작 좌표 확인.
	CPoint GetPointEnd			(int iIndex) { if (iIndex<0 || iIndex>=MAX_DRAW) return 0;	   return m_stFigure[iIndex].ptEnd;		 } // 선택한 도형의 종료 좌표 확인.
	
	//---------- Draw Functions ----------//
	void SetEnableDrawCrossLine	(bool  bEnable) { m_bDrawCrossLine = bEnable; Invalidate(); }	// Cross Line을 그릴 것인지 설정.
	void SetEnableDrawTargetLine (bool  bEnable) { m_bDrawAlignTargetLine = bEnable; Invalidate(); }	// Align Target Line.
	void SetClearTargetLine(){m_AlignTargetLine.clear();}
	//---------- Mouse Point Information ----------//
	CPoint GetMousePoint() { return m_ptMousePoint; }	// 이미지상의 현재 좌표 반환.

	void SetEnableMenuLoad(bool bEnable);
	void SetEnableMenuSave(bool bEnable);
	
	int GetWidth()  { return m_nWidth;  }
	int GetHeight() { return m_nHeight; }
	int GetBPP()	{ return m_nBpp;	}
	int GetValueY() { return m_nY;		}
	int GetValueY(int nPosX, int nPosY);
	COLORREF GetValueRGB() { return m_clrRGB;  }
	COLORREF GetValueRGB(int nPosX, int nPosY);
	CString GetImagePath() { return m_strPath; }
	bool IsFitImage() { return m_bDrawFitImage; }
	void setOriginalImage() { m_fMagnification = 1; m_bDrawFitImage = false; Invalidate(); }
	void setNavi(bool bEnable) { m_bNavi = bEnable; Invalidate(); }

	void SetFitMode() { OnFitImage(); }
	void SetHwnd(HWND hWnd) { m_hWnd = hWnd;}

	void EnablePatternLine(BOOL bEnable) { m_bPatternLine = bEnable; }
	void DrawPatternLine(CPoint pt) { m_ptPatternPos = pt; }
	CDC * getOverlayDC() { return &m_OverlayMemDC; }
	//void setCameraResolution(float resol){ m_fCameraResolution=resol;}
	void setCameraResolutionX(float resol){ m_fCameraResolutionX=resol;}
	void setCameraResolutionY(float resol){ m_fCameraResolutionY=resol;}

	/////////// Masking
	void SetUseMask( BOOL bSet ) { m_bUseMask = bSet; }
	void SetMaskPen( BOOL bSet ) { m_bMaskPen = bSet; }
	void SetMaskRect( BOOL bSet ) { m_bMaskRect = bSet; }
	BOOL GetUseMask() { return m_bUseMask; }
	BOOL GetMaskPen() { return m_bMaskPen; }
	BOOL GetMaskRect() { return m_bMaskRect; }
	void ClearMaskOverlayDC();
	void ClearRectMaskOverlayDC();
	CDC * getMaskOverlayDC(){ return &m_MaskOverlayMemDC;}	
	CDC * getRectMaskOverlayDC(){ return &m_RectOverlayMemDC;}	
	void SetMaskPos(CPoint point);
	BOOL DrawRect(CPoint pt1, CPoint pt2);
	void DrawRectMask(CPoint pt1, CPoint pt2);
	void SetAlignTargetLine(CPoint pt){m_AlignTargetLine.push_back(pt);}

	bool m_bDrawRectangleInside;
	stFigureText m_stFigureText[MAX_DRAW];
	///////////////////////////////
private :
	// Masking ////
	BOOL m_bMaskClick;
	BOOL m_bUseMask;
	BOOL m_bMaskPen;
	BOOL m_bMaskRect;
	CDC m_MaskOverlayMemDC;
	CDC m_RectOverlayMemDC;	
	////////////////

private :
	ULONG_PTR m_gdiplusToken;   // GDI+ Token.
	Bitmap* m_pBmp;             // Pointer to GDI+ bitmap.
	CDib m_pBmpInfo8BitGray;
	CDC m_OverlayMemDC;
	
	CPoint m_ptPatternPos;
	BOOL m_bPatternLine;
	void RegisterWindowClass(void);
	HWND m_hWnd;
	//----- Imagae -----//
	CImage m_ImgSrc;				//이미지 객체.
	CString m_strPath;				//이미지 경로. (파일로 불러온 경우)
	//----- Figure Array -----//
	stFigure m_stFigure[MAX_DRAW];	//도형 구조체.
	std::vector<CPoint> m_AlignTargetLine;
	//----- Move Image -----//
	CPoint	m_ptLDown;				//마우스가 내려간 좌표.
	CPoint	m_ptLup;				//마우스가 올라간 좌표.
	CPoint	m_ptResult;				//최종 이미지가 움직인 좌표의 시작 점.
	CPoint	m_ptMousePoint;			//현재 마우스의 이미지상 좌표.
	CScrollBarEx m_wndSBH;			//가로 스크롤바.
	CScrollBarEx m_wndSBV;			//세로 스크롤바.
	
	//----- Image Information -----//
	//double  m_fCameraResolution;
	double m_fCameraResolutionX;	//resolution X
	double m_fCameraResolutionY;	//resolution Y
	float	m_fMagnification;		//이미지 배율.
	int		m_nWidth;				//이미지 가로 크기.
	int		m_nHeight;				//이미지 세로 크기.
	int		m_nBpp;					//이미지 픽셀당 비트 크기.
	bool	m_bClear;				//초기화.
	bool	m_bInit;				//카메라 연결 시 OnInitWithCamera()가 선 호출되었는지 확인 용.
	bool	m_bScroll;				//이미지 이동을 스크롤바로 할 것인지.
	bool	m_bNavi;				//네이게이션 기능 사용 할 것인지.
	int		m_nY;					//이미지 픽셀 값. (Gray-Level)
	COLORREF m_clrRGB;				//이미지 픽셀 값. (Color-Level)

	//----- Menu -----//
	bool	m_bMenu;				//마우스 우 클릭 메뉴 표시 여부.
	CMenu	*m_pMenu;				//마우스 우 클릭 메뉴.
	CMenu	*m_pSubZoom;			//마우스 우 클릭 배율 세부 메뉴.
	CMenu	*m_pSubMode;			//마우스 우 클릭 모드 세부 메뉴.
	
	//----- Overlay Drawing -----//
	bool	m_bEnableDrawingFigure;		//현재 도형이 그려지는 모드인지.
	bool	m_bModifyFigure;		//이미지 수정 모드인지.
	bool	m_bDrawFitImage;		//전체 보기 모드 인지.
	bool	m_bDrawingFigure;		//현재 도형이 그려지는 모드인지.
	bool	m_bDrawAllFigures;		//모든 도형을 그릴 것인지.
	bool	m_bDrawCrossLine;		//눈금을 그릴 것인지.
	bool	m_bDrawAlignTargetLine;		//눈금을 그릴 것인지.
	int		m_nCurrFigure;			//현재 Figure Index.
	bool    m_bDrawStart;

	//----- Mouse Moving -----//	//		NW			N			NE
	bool	m_isHitNW;				//		□──────────□──────────□
	bool	m_isHitN ;				//		│					  │
	bool	m_isHitNE;				//		│					  │
	bool	m_isHitE ;				//		│					  │
	bool	m_isHitSE;				//	  W □		Moving Area	  □ E
	bool	m_isHitS ;				//		│					  │
	bool	m_isHitSW;				//		│					  │
	bool	m_isHitW ;				//		│					  │
	bool	m_isMoving;				//		□──────────□──────────□
	CRect	m_rcPreFigure;			//		SW			S			SE

	/*int GetPixelY(int nPosX, int nPosY);*/
	/*COLORREF GetPixelRGB(int nPosX, int nPosY);*/
	void CheckMenuItem(int nID, bool bMode);		//Mouse Right 버튼 메뉴.
	void CreateMousePopupMenu();					//Popup menu 생성.
	bool OnLoad(CString path);						//이미지 불러오기.
	void ResetScrollbarInfo();						//스크롤바 정보 초기화.
	float CalculateLength(CPoint pt1, CPoint pt2);	//직선의 거리 구하는 공식.

protected:
	DECLARE_MESSAGE_MAP()
	virtual void PreSubclassWindow();
public:
	afx_msg void OnSaveImage();
	afx_msg void OnLoadImage();
	afx_msg void OnFitImage();
	afx_msg void OnNavigation();
	afx_msg void OnSetZoom(UINT wParam);
	afx_msg void OnSetMode(UINT wParam);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnPaint();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnDropFiles(HDROP hDropInfo);

};


