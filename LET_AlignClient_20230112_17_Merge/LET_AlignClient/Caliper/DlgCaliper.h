#pragma once

//#include "Dlg_InputValue.h"
#include "Caliper.h"
#include "..\UI_Control\XListCtrl.h"
#include "SheetCaliperParam.h"
#include "SheetCaliperResult.h"

#ifndef MAX_CALIPER_LINES
#define MAX_CALIPER_LINES 6
#endif

#ifndef MAX_CALIPER_OBJECT
#define MAX_CALIPER_OBJECT 8
#endif

#ifndef NUM_POS
#define NUM_POS 4
#endif


class CLET_AlignClientDlg;
class INIReader; // Tkyuha 20221227 속도 올리기 위함

class CDlgCaliper: public CDialogEx
{
	DECLARE_DYNAMIC(CDlgCaliper)

public:
	CDlgCaliper(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgCaliper();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DLG_CALIPER };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
private:
	// Mouse Event 처리
	BOOL m_bHitStart;
	BOOL m_bHitMid;
	BOOL m_bHitEnd;	
	BOOL m_bMouseClicked;
	////////////

public:
	CSheetCaliperParam *m_pCaliperParam;
	CSheetCaliperResult *m_pCaliperResult;

	CCustomTabCtrl m_ctrlCaliperTab;
	CStatic        m_picCaliper;
	
	
	int m_nObject;
	int m_nPos;
	int m_nLine;

	BOOL m_bChanged[MAX_CALIPER_OBJECT][NUM_POS][MAX_CALIPER_LINES];
	CCaliper m_Caliper[MAX_CALIPER_OBJECT][NUM_POS][MAX_CALIPER_LINES];
	_st32fPoint m_ptCross[MAX_CALIPER_OBJECT][NUM_POS][MAX_CALIPER_LINES];
	char m_cCaliperName[MAX_CALIPER_OBJECT][NUM_POS][MAX_CALIPER_LINES][MAX_PATH];

	CLET_AlignClientDlg *m_pMain;
	CViewerEx *m_pViewer;
	
	int m_nWidth;
	int m_nHeight;

	virtual BOOL OnInitDialog();
	afx_msg LRESULT OnViewControl(WPARAM wParam, LPARAM lParam);
	afx_msg void OnSelchangeTab(  NMHDR* pNMHDR, LRESULT* pResult) ;

	// Mouse Event 처리
	BOOL getHitStart() { return m_bHitStart; }
	BOOL getHitMid() { return m_bHitMid; }
	BOOL getHitEnd() { return m_bHitEnd; }

	void setHitStart(BOOL bHit) { m_bHitStart = bHit; }
	void setHitMid(BOOL bHit) { m_bHitMid = bHit; }
	void setHitEnd(BOOL bHit) { m_bHitEnd = bHit; }

	void MouseMove(CPoint point, FakeDC* pDC = NULL);
	void LButtonDown(CPoint point, FakeDC* pDC = NULL);
	void LButtonUp(CPoint point, FakeDC* pDC = NULL);
	void LButtonDblClk(CPoint point, FakeDC* pDC = NULL);
	BOOL IsHitLine(CPoint point);
	//////////////////////////

	void saveCaliperInfo(CString strPath, CCaliper *pCaliper, int nObject, int nPos, int nLine);
	void readCaliperInfo(const INIReader* _ini, CCaliper *pCaliper, int nObject, int nPos, int nLine);// Tkyuha 20221227 속도 올리기 위함

	BOOL makeCrossPointFromLines(int bObj1, int nObj2, int nPos1, int nPos2, int nLine1, int nLine2, _st32fPoint *pt);
	void copy_caliper_data(int src_obj, int src_pos, int src_line, int dst_job, int dst_cam, int dst_pos, int dst_line);
	void display_copy_caliper_info();
	void init_copy_caliper_info();


	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};

