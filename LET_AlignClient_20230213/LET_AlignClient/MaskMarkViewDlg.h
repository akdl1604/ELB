#pragma once


#ifndef __VIEWER_EX_H__
#define __VIEWER_EX_H__
#include "UI_Control\ViewerEx.h"
#endif

#ifndef __LABEL_EX_H__
#define __LABEL_EX_H__
#include "UI_Control\LabelEx.h"
#endif

#ifndef _MEMDC_H_
#include "UI_Control\MemDCEx.h"
#endif

#include "TabCtrlEx.h"

#include "MarkSearchingSheet.h"
#include "MarkMaskingSheet.h"
#include "MarkFindingSheet.h"
#include "MarkSearchTesterSheet.h"
#include "DOGU.h"
#include <vector>
#include <gdiplus.h>
#include <opencv2\opencv.hpp>
#include "UI_Control/Graphics/GraphicMaskDrawing.h"

// CMaskMarkViewDlg 대화 상자입니다.
#define UNCLICK_COLOR RGB( 64, 64, 64)
#define CLICK_COLOR RGB(128, 255, 128)


using namespace std;
class CLET_AlignClientDlg;
class CHalconProcessing;

class CMarkSearchingSheet;
class CMarkMaskingSheet;
class CMarkFindingSheet;
class CMarkSearchTesterSheet;


class CMaskMarkViewDlg: public CDialog
{
	DECLARE_DYNAMIC(CMaskMarkViewDlg)

public:
	CMaskMarkViewDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CMaskMarkViewDlg();

	// 대화 상자 데이터입니다.
	enum { IDD = IDD_DLG_MASK_MARKVIEW };

private: // Variable region
	GraphicMaskDrawing m_MaskDrawing;

private:
	void SetTabsPosition();
	bool IsExist(CWnd* pWnd) const;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

public:
	CHalconProcessing* m_pMatchingProc;

	CString m_strMaskingPath;

	HBRUSH m_hbrBkg;
	int m_nPattern;
	int m_nIndex;
	BYTE* m_pBuffer;

	BOOL m_bMaskPen;
	BOOL m_bMaskRect;
	BOOL m_bClickedEvent;
	double m_dSize;
	double m_dSize_OLD;
	int m_nWidth;
	int m_nHeight;
	double m_dbPatRefX;
	double m_dbPatRefY;

	CRect m_ViewerRECT;

	CRect m_rectDlg;
	CRect m_rectViewer;
	CRect m_rectCancel;
	BOOL m_bShape;
	BOOL m_bUseCalibModel; // Tkyuha 20211204 켈리브레이션 용으로 사용 하기 위함
	BOOL m_bMaskRoiUse[4];
	BOOL m_bMaskRoiShape[4];
	CRect m_crMaskRect[4];

	BYTE* m_pImage = NULL;
	BYTE* m_MaskIMG = NULL;
	BOOL m_bBtnClick;
	int m_nClickedBtn;
	int m_nClickCount;
	BOOL m_bIsBtnClick[4];
	CRect m_rcBtnPos[4];
	CRect m_rcMaskPos;
	CButtonEx m_btnSize[6];

	CButtonEx m_btnOk;
	CButtonEx m_btnCancel;
	CLabelEx m_stt_BtnPos[4];
	CViewerEx m_Viewer;

	CTabCtrlEx m_TabCtrl;
	CCheckEx m_btnShowMask;
	CCheckEx m_btnPreprocessing;
	CCheckEx m_btnTracker;
	CWnd* c_pWnd;
	CMarkSearchingSheet* m_pSearhingSheet;
	CMarkMaskingSheet* m_pMaskingSheet;
	CMarkFindingSheet* m_pFindingSheet;
	CMarkSearchTesterSheet* m_pSearhTesterSheet;

	// Lincoln Lee - 220219
	BOOL m_IsMouseDown = FALSE;

	int n_LastButtonID = -1;
	size_t lastSize = 0;

	int mark_nCam;
	int mark_nPos;
	int mark_nIndex;
	int mark_nAlgo;
	int m_nUnit = 1;

	GraphicMaskDrawing& GetMaskDrawer();

	void SetImage(int width, int height, double refX, double refY, BYTE* pImage);
	void SetParam(int com, int shape, int id = 0, CRect roi = CRect(0, 0, 0, 0));
	void SetParam(CHalconProcessing* pMatchingProc = NULL);
	void SetCalibMode(BOOL bEnable = FALSE) { m_bUseCalibModel = bEnable; }
	void SetParamInitial(int nCam, int nPos, int nIndex);
	void SetMarkId(int nCam, int nPos, int nIndex, int algo);
	void GetParameterData();
	void CreateTabGrid();

	void fnDispBtn();
	void SaveMaskImage(bool save = false);
	void ClearMaskImage();
	void LoadMaskData();
	void UpdateDisplay(int nSheetId = 0);
	void NotifyUpdate(int nSheetId = 0);

	// Lincoln Lee - 220219
	bool m_IsLoaded = false;
	void DoResize();

	// Lincoln Lee - 220222
	void UnClickColor(int id);
	void ClickColor(int id);
	BOOL IsInsideRegion(const CRect& rect, const CPoint& pt);
	BOOL CheckAndDoOriginBnClk(const CPoint& pt, int& btnId);

	// Lincoln Lee - 220210
	void(CMaskMarkViewDlg::* BnClickeds[4])(int);
	void OnBnClickedBtnLEFT(int nUnit);
	void OnBnClickedBtnTOP(int nUnit);
	void OnBnClickedBtnRIGHT(int nUnit);
	void OnBnClickedBtnBOTTOM(int nUnit);

	DECLARE_MESSAGE_MAP()
	virtual BOOL OnInitDialog();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnSelchangeTabList(NMHDR* pNMHDR, LRESULT* pResult);

	void InitTitle(CLabelEx* pTitle, CString str, float size, COLORREF color);
	void MainButtonInit(CButtonEx* pbutton, int size = 15);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedBtnOk();
	afx_msg void OnBnClickedBtnCancel();
	afx_msg void OnBnClickedBtnSizeX1();
	afx_msg void OnBnClickedBtnSizeX2();
	afx_msg void OnBnClickedBtnSizeX3();
	afx_msg void OnBnClickedBtnSizeX4();

	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedBtnSizeX5();
	afx_msg void OnBnClickedBtnSizeX6();

	afx_msg void OnPaint();
	afx_msg void OnBnClickedCheckShowMask();
	afx_msg void OnBnClickedCheckPreprocessing();
	afx_msg void OnBnClickedCheckTracker();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	virtual void PreSubclassWindow();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
