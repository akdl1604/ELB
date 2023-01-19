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
{										//			�ড�������������������ড��������������������
	bool isDraw;		// Draw ����.	//			��					  ��
	bool isLine;		// ��������.		//			��					  ��
	bool isRectangle;	// �簢������.	//			��					  ��
	bool isCircle;		// �� ����.		//	 Hit[7] ��					  �� Hit[3]
	int  nWidth;		// �β�.			//			��					  ��
	CPoint ptBegin;		// ���� ��.		//			��					  ��
	CPoint ptEnd;		// �� ��.		//			��					  ��
	CRect  rcHit[8];	// ��ġ ��.		//			�ড�������������������ড��������������������
	COLORREF clrColor;	// ���� ����.	//		  Hit[6]	  Hit[5]	  Hit[4]
	float fLength;		// ������ ����.
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
	void OnInitWithCamera(int nWidth, int nHeight, int nBpp);	//�̹��� �����͸� ����Ͽ� �ҷ��� ���, ���� ȣ��Ǿ�� ��.
	bool OnLoadImageFromPtr(BYTE* pBuffer);						//�̹��� �����ͷ� �ҷ��� ���.
	
	//---------- Figure Functions ----------//
	void SetEnableModifyFigure  (int iIndex, bool isEnable); // ������ �ε����� ������ ������ ������ ����.
	void SetCurrentFigureIndex	(int iIndex) { if (iIndex<0 || iIndex>=MAX_DRAW) return; m_nCurrFigure = iIndex; } // ���� ������ �ε��� ����.
	void SetModeDrawingFigure	(bool bEnable) { m_bDrawingFigure  = bEnable; } // ������ �׸��� ���� ������ ����.
	bool GetModeDrawingFigure() { return m_bDrawingFigure; }
	void SetEnableDrawAllFigures(bool bEnable) { m_bDrawAllFigures = bEnable; Invalidate();} // ��� ������ �׸� ������ ����.
	
	//---------- Figure Option Fuctions ----------//
	void SetColorFigure			 (int iIndex, COLORREF clrColor) { if (iIndex<0 || iIndex>=MAX_DRAW) return; m_stFigure[iIndex].clrColor = clrColor	; Invalidate(); } // ������ ������ ���� ����.
	void SetWidthFigure			 (int iIndex, int nWidth	   ) { if (iIndex<0 || iIndex>=MAX_DRAW) return; m_stFigure[iIndex].nWidth   = nWidth	; Invalidate(); } // ������ ������ �β� ����.
	void SetEnableDrawFigure	 (int iIndex, bool bEnable	   ) { if (iIndex<0 || iIndex>=MAX_DRAW) return; m_stFigure[iIndex].isDraw	 = bEnable	; Invalidate(); } // ������ ������ �׸������� ����.
	void SetEnableFigureLine	 (int iIndex); // ������ �ε����� ������ '����'���� ����.
	void SetEnableFigureCircle	 (int iIndex); // ������ �ε����� ������ '����'���� ����.
	void SetEnableFigureRectangle(int iIndex); // ������ �ε����� ������ '�簢��'���� ����.
	void SetSizeRectangle(int iIndex, CPoint pt1, CPoint pt2);
	void SetSizeRectangle(int iIndex, int nOrgX, int nOrgY, int nWidth, int nHeight);
	
	bool IsDrawFigure			(int iIndex) { if (iIndex<0 || iIndex>=MAX_DRAW) return false; return m_stFigure[iIndex].isDraw;	 } // ������ ������ Draw ���� Ȯ��.
	bool IsEnableFigureLine		(int iIndex) { if (iIndex<0 || iIndex>=MAX_DRAW) return false; return m_stFigure[iIndex].isLine;	 } // ������ ������ '����'���� Ȯ��.
	bool IsEnableFigureCircle	(int iIndex) { if (iIndex<0 || iIndex>=MAX_DRAW) return false; return m_stFigure[iIndex].isCircle;	 } // ������ ������ '����'���� Ȯ��.
	bool IsEnableFigureRectangle(int iIndex) { if (iIndex<0 || iIndex>=MAX_DRAW) return false; return m_stFigure[iIndex].isRectangle;} // ������ ������ '�簢��'���� Ȯ��.
	int  GetWidthFigure			(int iIndex) { if (iIndex<0 || iIndex>=MAX_DRAW) return 0;	   return m_stFigure[iIndex].nWidth;	 } // ������ ������ �β� Ȯ��.
	COLORREF GetColorFigure		(int iIndex) { if (iIndex<0 || iIndex>=MAX_DRAW) return 0;	   return m_stFigure[iIndex].clrColor;	 } // ������ ������ ���� Ȯ��.
	float GetLineLength			(int iIndex) { if (iIndex<0 || iIndex>=MAX_DRAW) return 0; if (m_stFigure[iIndex].isLine) return m_stFigure[iIndex].fLength; else return 0; } // ������ ������ ���� �Ÿ� ��ȯ.
	CPoint GetPointBegin		(int iIndex) { if (iIndex<0 || iIndex>=MAX_DRAW) return 0;	   return m_stFigure[iIndex].ptBegin;	 } // ������ ������ ���� ��ǥ Ȯ��.
	CPoint GetPointEnd			(int iIndex) { if (iIndex<0 || iIndex>=MAX_DRAW) return 0;	   return m_stFigure[iIndex].ptEnd;		 } // ������ ������ ���� ��ǥ Ȯ��.
	
	//---------- Draw Functions ----------//
	void SetEnableDrawCrossLine	(bool  bEnable) { m_bDrawCrossLine = bEnable; Invalidate(); }	// Cross Line�� �׸� ������ ����.
	void SetEnableDrawTargetLine (bool  bEnable) { m_bDrawAlignTargetLine = bEnable; Invalidate(); }	// Align Target Line.
	void SetClearTargetLine(){m_AlignTargetLine.clear();}
	//---------- Mouse Point Information ----------//
	CPoint GetMousePoint() { return m_ptMousePoint; }	// �̹������� ���� ��ǥ ��ȯ.

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
	CImage m_ImgSrc;				//�̹��� ��ü.
	CString m_strPath;				//�̹��� ���. (���Ϸ� �ҷ��� ���)
	//----- Figure Array -----//
	stFigure m_stFigure[MAX_DRAW];	//���� ����ü.
	std::vector<CPoint> m_AlignTargetLine;
	//----- Move Image -----//
	CPoint	m_ptLDown;				//���콺�� ������ ��ǥ.
	CPoint	m_ptLup;				//���콺�� �ö� ��ǥ.
	CPoint	m_ptResult;				//���� �̹����� ������ ��ǥ�� ���� ��.
	CPoint	m_ptMousePoint;			//���� ���콺�� �̹����� ��ǥ.
	CScrollBarEx m_wndSBH;			//���� ��ũ�ѹ�.
	CScrollBarEx m_wndSBV;			//���� ��ũ�ѹ�.
	
	//----- Image Information -----//
	//double  m_fCameraResolution;
	double m_fCameraResolutionX;	//resolution X
	double m_fCameraResolutionY;	//resolution Y
	float	m_fMagnification;		//�̹��� ����.
	int		m_nWidth;				//�̹��� ���� ũ��.
	int		m_nHeight;				//�̹��� ���� ũ��.
	int		m_nBpp;					//�̹��� �ȼ��� ��Ʈ ũ��.
	bool	m_bClear;				//�ʱ�ȭ.
	bool	m_bInit;				//ī�޶� ���� �� OnInitWithCamera()�� �� ȣ��Ǿ����� Ȯ�� ��.
	bool	m_bScroll;				//�̹��� �̵��� ��ũ�ѹٷ� �� ������.
	bool	m_bNavi;				//���̰��̼� ��� ��� �� ������.
	int		m_nY;					//�̹��� �ȼ� ��. (Gray-Level)
	COLORREF m_clrRGB;				//�̹��� �ȼ� ��. (Color-Level)

	//----- Menu -----//
	bool	m_bMenu;				//���콺 �� Ŭ�� �޴� ǥ�� ����.
	CMenu	*m_pMenu;				//���콺 �� Ŭ�� �޴�.
	CMenu	*m_pSubZoom;			//���콺 �� Ŭ�� ���� ���� �޴�.
	CMenu	*m_pSubMode;			//���콺 �� Ŭ�� ��� ���� �޴�.
	
	//----- Overlay Drawing -----//
	bool	m_bEnableDrawingFigure;		//���� ������ �׷����� �������.
	bool	m_bModifyFigure;		//�̹��� ���� �������.
	bool	m_bDrawFitImage;		//��ü ���� ��� ����.
	bool	m_bDrawingFigure;		//���� ������ �׷����� �������.
	bool	m_bDrawAllFigures;		//��� ������ �׸� ������.
	bool	m_bDrawCrossLine;		//������ �׸� ������.
	bool	m_bDrawAlignTargetLine;		//������ �׸� ������.
	int		m_nCurrFigure;			//���� Figure Index.
	bool    m_bDrawStart;

	//----- Mouse Moving -----//	//		NW			N			NE
	bool	m_isHitNW;				//		�ড�������������������ড��������������������
	bool	m_isHitN ;				//		��					  ��
	bool	m_isHitNE;				//		��					  ��
	bool	m_isHitE ;				//		��					  ��
	bool	m_isHitSE;				//	  W ��		Moving Area	  �� E
	bool	m_isHitS ;				//		��					  ��
	bool	m_isHitSW;				//		��					  ��
	bool	m_isHitW ;				//		��					  ��
	bool	m_isMoving;				//		�ড�������������������ড��������������������
	CRect	m_rcPreFigure;			//		SW			S			SE

	/*int GetPixelY(int nPosX, int nPosY);*/
	/*COLORREF GetPixelRGB(int nPosX, int nPosY);*/
	void CheckMenuItem(int nID, bool bMode);		//Mouse Right ��ư �޴�.
	void CreateMousePopupMenu();					//Popup menu ����.
	bool OnLoad(CString path);						//�̹��� �ҷ�����.
	void ResetScrollbarInfo();						//��ũ�ѹ� ���� �ʱ�ȭ.
	float CalculateLength(CPoint pt1, CPoint pt2);	//������ �Ÿ� ���ϴ� ����.

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


