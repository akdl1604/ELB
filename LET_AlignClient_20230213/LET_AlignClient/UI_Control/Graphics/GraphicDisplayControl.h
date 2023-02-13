#pragma once
#include "stdafx.h"
#include <afxwin.h>
#include "Rigid.h"
#include <gdiplus.h>
#include "GraphicLabel.h"
#include "GraphicBase.h"
#include "GraphicInterativeCalipers.h"
#include "GraphicRegionalSelection.h"
#include "GraphicInterativeDimensional.h"
#include "GraphicRotateRectSelection.h"
#include <atlstr.h>
#include "GraphicPoint.h"
#include "FakeDC.h"
#include <mutex>
#include "EventHandlers.h"
/*
* - Lincoln Lee
* - x.perfect.vn@gmail.com
* - 2022-08-08
*/

#define VIEWER_WINCLASSNAME _T("VIEWEREX_CLASS")
#define SoftRefresh() if(m_Loaded){m_SoftDirty=true;Invalidate();}
#define HardRefresh() if(m_Loaded){m_HardDirty=true;Invalidate();}
#define MIN_ZOOM (0.05)
#define MAX_ZOOM (100.)
#define NAV_MAXSIZE (320.)

#define WM_DRAWING_FINISH_MESSAGE	(WM_USER +   1)
#define WM_LOAD_IMAGE_MESSAGE		(WM_USER +   2)
#define WM_DRAWING_LINE_MESSAGE		(WM_USER +   3)
#define WM_LINE_TRACKER_MESSAGE		(WM_USER +   5)
#define USER_ID		       (WM_USER + 100)

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


#define ID_MENU_GRABMOVE        USER_ID + 11
#define ID_MENU_MEASURE 	    USER_ID + 12
#define ID_MENU_RECT_SELECT 	USER_ID + 13
#define ID_MENU_ELLIPSE_SELECT 	USER_ID + 14
#define ID_MENU_LINE_CALIPER    USER_ID + 15
#define ID_MENU_CIRCLE_CALIPER  USER_ID + 16
#define ID_MENU_ROTATE_SELECT 	USER_ID + 17
#define ID_MENU_HIDE_INTGRAPHIC USER_ID + 18

#define ID_MENU_SAVE		    USER_ID + 19
#define ID_MENU_LOAD		    USER_ID + 20
#define ID_MENU_FIT			    USER_ID + 21
#define ID_MENU_NAVIGATION	    USER_ID + 22

class CViewerEx: public CWnd
{
	DECLARE_DYNAMIC(CViewerEx)
public: // Variable region
public: // Functions region
	CViewerEx();
	CViewerEx& ZoomIn();
	CViewerEx& ZoomOut();
	CViewerEx& SetInteraction(bool val = true);
	CViewerEx& SetImage(const cv::Mat& src);
	CViewerEx& SetImage(const void* data);
	CViewerEx& OnLoadImageFromPtr(const void* data);
	CViewerEx& SetImage(const void* data, cv::Size size, int nBpp = 8);
	CViewerEx& SetImageSize(cv::Size size, int nBpp = 8);
	CViewerEx& OnInitWithCamera(int width, int height, int nBpp);
	CViewerEx& SetParent(CDialog* parent);
	CViewerEx& SetSimpleMode(bool val = true);
	CViewerEx& SetZoomByMouse(bool val = true);
	CViewerEx& SetMoveByMouse(bool val = true);
	CViewerEx& SetInterativeItem(GraphicBase* item = NULL);
	CViewerEx& OnMouseMoveEvent(MouseEventCallback fnc);
	CViewerEx& OnMouseUpEvent(MouseEventCallback fnc);
	CViewerEx& OnMouseDownEvent(MouseEventCallback fnc);
	CViewerEx& OnMouseDbClkEvent(MouseEventCallback fnc);
	CViewerEx& SetEnableDrawGuidingLine(bool enable = true);
	CViewerEx& ToggleDrawGuidingLine();
	CViewerEx& SetEnableDrawGuidingCross(bool enable = true);
	CViewerEx& ToggleDrawGuidingCross();
	CViewerEx& SetEnableDrawGuidingGraphics(bool enable = true);
	CViewerEx& ToggleDrawGuidingGraphics();

	CViewerEx& SetFitMode(bool fit = true);
	CViewerEx& SetRoiSelectionMode();
	CViewerEx& SetDimensionalMode();
	CViewerEx& SetRotateRoiSelectionMode();
	CViewerEx& SetRoiSelectionEllipseMode();
	CViewerEx& GetCaliperBeginEnd(CPoint& st, CPoint& ed);
	CViewerEx& GetSelectorBeginEnd(CPoint& st, CPoint& ed);
	CViewerEx& SettingCalipers(const CPoint& st, const CPoint& ed, int nCount = 30, int projLEN = 5, int searchLEN = 120, bool reverse = false, bool lineMODE = true);
	CViewerEx& SettingSelector(const CPoint& st, const CPoint& ed, bool ellipseMODE = false);
	CViewerEx& GetCaliperSettings(CPoint& st, CPoint& ed, int& nCount, int& projLEN, int& searchLEN, bool& reverse, bool& lineMODE);
	CViewerEx& GetSelectorSettings(CPoint& st, CPoint& ed, bool& ellipseMODE);
	CViewerEx& SetCustomInteractiveGraphic(GraphicBase* item);
	CViewerEx& SetSelectorBeginEnd(const CPoint& st, const CPoint& ed);
	CViewerEx& GetBeginEnd(CPoint& st, CPoint& ed);
	CViewerEx& SetBeginEnd(const CPoint& st, const CPoint& ed);
	CViewerEx& SetMetricTransform(const Rigid& metric);
	CViewerEx& InactiveInteraction(bool inAct = true);
	CViewerEx& DirtyRefresh();
	CViewerEx& SoftDirtyRefresh();
	CViewerEx& HardDirtyRefresh();
	CViewerEx& SetCaliperProjectionLength(int val);
	CViewerEx& SetCaliperSearchLength(int val);
	CViewerEx& SetNumOfCalipers(int val);
	CViewerEx& SetCaliperCircleMode(bool circle = true);
	CViewerEx& SetCaliperLineMode(bool line = true);
	CViewerEx& SetCaliperBeginEnd(CPoint st, CPoint ed);
	CViewerEx& SetCaliperReverseMode(bool reverse = true);
	CViewerEx& ToggleCaliperDirection();
	CViewerEx& ToggleDrawOverlay();
	CViewerEx& SetNavMode(bool nav = true);

	bool IsInteraction() const;
	bool GetCaliperDirection() const;
	/* To make API compatible with old ViewerEx */
	bool InitControl(CWnd* pWnd, bool bMenu = true);

	// HardGraphic coordinate is on View
	// - Positive Position is as usual (from begin to end): X=10, Y=10 (view size 800x600) -> position is: X=10, Y=10
	// - Negative Position is from end to begin: X=-10, Y=-10 (view size 800x600) -> position is: X=800-10, Y=600-10
	// - Use positive position for coordinate from Top-Left
	// - Use negative postition for coordinate from Bottom-Right (If want to stick Graphic at bottom of control)
	CViewerEx& AddHardGraphic(GraphicBase* item);
	// SoftGraphic coordinate is by image
	CViewerEx& AddSoftGraphic(GraphicBase* item);

	CViewerEx& ClearGraphics();
	CViewerEx& ClearSoftGraphics();
	CViewerEx& ClearHardGraphics();

	Gdiplus::Color GetPixel(CPoint pt);
	Gdiplus::Color GetPixel(cv::Point2d pt);
	CViewerEx& OnLoadImage(const CString& strPath);
	CViewerEx& Screenshot(const CStringA& strFileName);

	int Width() const { return m_Width; } // Controls's Dimensions
	int Height() const { return m_Height; }
	int ImageWidth() const { return m_ImageSize.width; } // Controls's Dimensions
	int ImageHeight() const { return m_ImageSize.height; }
	int GetWidth() const { return m_ImageSize.width; } // Controls's Dimensions
	int GetHeight() const { return m_ImageSize.height; }

	void OnNavigation();
	void OnLoadImage();
	void OnFitImage();
	void OnSaveImage();
	void OnGrabMoveMode();
	void OnHideInteractGraphic();

	cv::Mat Screenshot();
	cv::Mat RenderToImage(cv::Size maxSize);

	auto& GetInteractiveItems() { return m_InteractItems; }
	auto& MouseEvents() { return m_MouseEvents; }

	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);

	~CViewerEx();

#pragma region Back Compatible APIs
	int GetBPP() const;
	int GetValueY() const;
	int GetValueY(int nPosX, int nPosY) const;
	COLORREF GetValueRGB() const;
	COLORREF GetValueRGB(int nPosX, int nPosY) const;
	CPoint GetMousePoint() const;

	void* GetImagePtr() const;
	CViewerEx& reset_to_gray_image();
	CViewerEx& clearAllFigures();
	CViewerEx& SetHwnd(HWND hwnd);
	CViewerEx& saveScreenCapture(const CString& strPath);
	CViewerEx& addFigureText(const stFigureText& figure, float p = 16, float q = 16, COLORREF color = 0xff00ff00);
	CViewerEx& addFigureLine(const stFigure& figure, float p = 16, float q = 16, COLORREF color = 0xff00ff00);
	CViewerEx& addFigureRect(const stFigure& figure, float p = 16, float q = 16, COLORREF color = 0xff00ff00);
	CViewerEx& SetUseMask(bool use);
	CViewerEx& SetEnableDrawAllFigures(bool val);
	CViewerEx& SetEnableDrawFigure(int Id, bool ena);
	FakeDC* getOverlayDC() const;
	FakeDC* getMaskOverlayDC() const;
	double GetMagnification() const;
	double getCameraResolutionX() const;
	double getCameraResolutionY() const;

	CPoint GetPointBegin(int idx) const;
	CPoint GetPointEnd(int idx) const;
	CViewerEx& ClearOverlayDC();
	CViewerEx& SetMaskPen(bool use = true);
	CViewerEx& SetMaskRect(bool use = true);
	CViewerEx& setOriginalImage();
	CViewerEx& ShowClipers(bool show = true);
	CViewerEx& setNavi(bool nav = true);
	CViewerEx& ShowOverlay(bool ena = true);
	CViewerEx& SetSemiTransparent(bool semi = true);
	CViewerEx& SetDrawOverlay(bool draw = true);
	CViewerEx& ClearCaliperOverlayDC(bool clear = true);
	CViewerEx& SetModeDrawingFigure(bool disp = true);
	CViewerEx& SetEnableModifyFigure(int Id, bool disp = true);
	CViewerEx& setUseCaliper(bool val = true);
	CViewerEx& setDrawCaliper(bool val = true);
	CViewerEx& SetEnableFigureRectangle(int val);

	bool IsDrawFigure(int Id) const;
	bool GetUseMask() const;
	bool IsFitImage() const;
	bool GetModeDrawingFigure() const;
	bool IsCalipersMode() const;
	bool GetEnableDrawCrossLine() const;
	bool GetDrawOverlay() const;
#pragma endregion


protected: // Variable Region
protected: // Functions Region
	DECLARE_MESSAGE_MAP()
	virtual void PreSubclassWindow();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnPaint();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnContextMenu(CWnd* /*pWnd*/, CPoint /*point*/);

	void ShowGuidingLine(Gdiplus::Graphics& graph, Rigid trans, cv::Size size, Gdiplus::Pen* pen);
	void ShowGuidingCross(Gdiplus::Graphics& graph, Rigid trans, cv::Size size, Gdiplus::Pen* pen);
	void ShowGuidingGraphic(Gdiplus::Graphics& graph, Rigid trans, cv::Size size, Gdiplus::Pen* penLINE, Gdiplus::Pen* penCROSS);

private: // Variable Region
	int m_nY = 0;
	int m_BPP = 8;
	int m_Width = 727;
	int m_Height = 401;
	int m_SrcDepth = CV_8U;
	double m_NavScale = 1.;

	HWND m_hWnd = NULL;

	COLORREF m_clrRGB;
	CPoint m_ptMousePoint;

	Rigid m_ImageTrans;
	Rigid m_SceneTrans;
	Rigid m_MetricTrans;

	cv::Mat m_SrcMAT;
	cv::Mat m_BaseMAT;
	cv::Mat m_SizedMAT;
	cv::Mat m_NavigateMAT;
	cv::Mat m_SrcSizedMAT;

	cv::Size m_ImageSize;
	CPoint m_LastMousePOS;
	CDialog* m_Parent = NULL;
	Gdiplus::RectF m_NavigationRect;

	Gdiplus::Bitmap* m_BaseIMG = NULL;
	Gdiplus::Bitmap* m_SoftIMG = NULL;
	Gdiplus::Bitmap* m_HardIMG = NULL;
	Gdiplus::Bitmap* m_SizedIMG = NULL;
	Gdiplus::Bitmap* m_NavigateIMG = NULL;

	Gdiplus::ImageAttributes att;
	Gdiplus::Color m_BackColor{ 0xff0a0a64 };
	Gdiplus::ColorPalette* m_Palete = NULL;
	Gdiplus::SolidBrush* m_PaddingBrush = NULL;
	Gdiplus::REAL m_DashStyle[2]{ 6.f, 6.f };

	Gdiplus::Pen* m_RedPen = NULL;
	Gdiplus::Pen* m_PenLINE = NULL;
	Gdiplus::Pen* m_PenCROSS = NULL;

	double m_LastScale = 1.;

	bool m_NeedFitImage = true;
	bool m_Closed = false;
	bool m_HardDirty = false;
	bool m_SoftDirty = false;
	bool m_MouseScroll = false;
	bool m_IsHasMouseDown = false;

	bool   m_bMenu = true;
	bool m_Loaded = false;
	bool m_Navigation = false;
	bool m_bDrawFitImage = true;
	bool m_SkipBaseImage = false;
	bool m_ForceBaseImage = false;

	bool m_Interative = false;
	bool m_ZoomByMouse = true;
	bool m_MoveByMouse = true;
	bool m_SimpleMode = false;
	bool m_FirstTimeImage = true;
	bool m_ShowGuidingLine = true;
	bool m_ShowGuidingCross = true;
	bool m_ShowInActiveInteractive = false;

	std::mutex m_Mutex;

	CMenu* m_pMenu = NULL;
	CMenu* m_pSubZoom = NULL;
	FakeDC* m_FakeCDC = new FakeDC(this);
	GraphicBase* m_InteractiveGraphic = NULL;

	std::vector<GraphicBase*> m_SoftGrahics;
	std::vector<GraphicBase*> m_HardGrahics;
	std::vector<float> m_ScaleLevels{ 0.125, 0.25, 0.5, 1, 2, 4, 8, 16, 32 };

	Gdiplus::ColorMatrix m_SemiTransparentColorMATRIX = {
		1.f, 0.f, 0.f, 0.f, 0.f,
		0.f, 1.f, 0.f, 0.f, 0.f,
		0.f, 0.f, 1.f, 0.f, 0.f,
		0.f, 0.f, 0.f, .8f, 0.f,
		0.f, 0.f, 0.f, 0.f, 1.f
	};

	struct
	{
		std::function<void(CViewerEx*, CPoint pt)> OnMouseMove;
		std::function<void(CViewerEx*, CPoint pt)> OnMouseDown;
		std::function<void(CViewerEx*, CPoint pt)> OnMouseUp;
		std::function<void(CViewerEx*, CPoint pt)> OnMouseDbClk;
	} m_MouseEvents;

	struct
	{
		GraphicInteractiveCalipers m_InteractCaliper{ Gdiplus::PointF(0, 0), Gdiplus::PointF(100, 0), 20, 3, 150 };
		GraphicRegionalSelection m_RoiSelector{ cv::Rect(100, 100, 200, 200) };
		GraphicInteractiveDimensional m_Dimensional{ Gdiplus::PointF(0, 0), Gdiplus::PointF(100, 0) };
		GraphicRotateRectSelection m_RoiRotateSelector{ cv::Rect(100, 100, 200, 200) };
	} m_InteractItems;

	std::vector<std::function<void()>> m_MouseModes{
		[&]() { m_Interative = false; }, /* Do GrabMove */
		[&]() { m_Interative = true; m_InteractItems.m_Dimensional.SetActive(true); m_InteractiveGraphic = &m_InteractItems.m_Dimensional; }, /* Do Measure */
		[&]() { m_Interative = true; m_InteractItems.m_RoiSelector.SetActive(true).SetShowEllipse(false); m_InteractiveGraphic = &m_InteractItems.m_RoiSelector; }, /* Do Rectangle Selection */
		[&]() { m_Interative = true; m_InteractItems.m_RoiSelector.SetActive(true).SetShowEllipse(true); m_InteractiveGraphic = &m_InteractItems.m_RoiSelector; }, /* Do Ellipse Selection */
		[&]() { m_Interative = true; m_InteractItems.m_InteractCaliper.SetActive(true).SetCircleMode(false); m_InteractiveGraphic = &m_InteractItems.m_InteractCaliper; }, /* Do Line Caliper */
		[&]() { m_Interative = true; m_InteractItems.m_InteractCaliper.SetActive(true).SetCircleMode(true); m_InteractiveGraphic = &m_InteractItems.m_InteractCaliper; },  /* Do Circle Caliper */
		[&]() { m_Interative = true; m_InteractItems.m_RoiRotateSelector.SetActive(true); m_InteractiveGraphic = &m_InteractItems.m_RoiRotateSelector; }  /* Do Rotate Rect Tracker */
	};
	
private: // Functions Regions
	void DoChangeImage();
	void RegisterWindowClass();
	void CreateMousePopupMenu();

	void OnSetZoom(UINT wParam);
	void OnMouseModes(UINT wParam);
	void CheckMenuItem(int nID, bool bMode);
	
	void RenderSoftGraphic(const Rigid& imgToView, cv::Size maxSize);
	void RenderHardGraphic(const Rigid& imgToView, cv::Size maxSize);
	void RenderSoftGraphic(Gdiplus::Graphics& graph, const Rigid& imgToView, cv::Size maxSize);
	void RenderHardGraphic(Gdiplus::Graphics& graph, const Rigid& imgToView, cv::Size maxSize);
	void DrawPaddingBorders(Gdiplus::Graphics& graph, cv::Rect padRECT, cv::Size maxSize);

	void SafeRemoveMenu(CMenu** mnu);
	void SafeReleaseBitmap(Gdiplus::Bitmap** bm);

	void Render(const Rigid& imgToView);
	void CreateSizedCacheImage(bool force = false);
	void OptimalGraphicSettings(Gdiplus::Graphics& graph, bool clear = false);

	inline Rigid GetImageToScreenTransform() const;
	inline Rigid GetScreenToImageTransform() const;

	Gdiplus::Bitmap* PaddingAndCreateBitmap(cv::Mat& src);
};