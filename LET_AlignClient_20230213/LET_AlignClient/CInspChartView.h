#pragma once

#include <afxdtctl.h>
#include "ChartPointsSerie.h"
#include "FilmInspDataBaseClass.h"

#define MAXIMUM_SIZE 4096
#define TEST_TICK_SIZE 0.002
#define GRAPH_SIZE_MIN 0.97
#define GRAPH_SIZE_MAX 1.03
#define CHART_MAX 4

// CInspChartView 대화 상자
class CInspChartView : public CDialogEx
{
	DECLARE_DYNAMIC(CInspChartView)

public:
	CInspChartView(int nJob, CWnd* pParent = nullptr);   
	virtual ~CInspChartView();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SCATTER };
#endif

	CFilmInspDataBaseClass m_dbFilmInspDataBase;
	CLET_AlignClientDlg* m_pMain;

	std::vector<_st_FILM_INSP_DATA> vt_Insp_Data;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	enum eChartPointIndex
	{
		// 스펙 디스플레이 포인트
		SIDE_BOTTOM			=	0,
		SIDE_LEFT			=	1,
		SIDE_TOP			=	2,
		SIDE_RIGHT			=	3,

		// 실제 데이터 디스플레이 포인트
		POINT_FIRST			=	4,
		POINT_RECENT_DATA	=	5,

		// 스펙 디스플레이 포인트
		LINE_HORI	=	6,
		LINE_VERT	=	7,
		POINT_MAX			=	10
	};

	enum eChartIndex
	{
		// 차트컨트롤 인덱스
		INSP_L = 0,
		DB_L,
		INSP_R,
		DB_R
	};

private:
	CChartPointsSerie*	m_pPointsSeries[CHART_MAX][POINT_MAX];
	CDateTimeCtrl		m_dateTimeSearch;
	CChartCtrl			m_arrChartView[CHART_MAX];
	CTabCtrl	m_tabChartView[2];

	double		m_dLX[MAXIMUM_SIZE] = { 0, };
	double		m_dLY[MAXIMUM_SIZE] = { 0, };

	double		m_dSpecMinLX[2],	m_dSpecMaxLX[2];
	double		m_dSpecMinLY[2],	m_dSpecMaxLY[2];
	double		m_dSpecLX[2],		m_dSpecLY[2];

	double		m_dSpecMinRX[2],	m_dSpecMaxRX[2];
	double		m_dSpecMinRY[2],	m_dSpecMaxRY[2];
	double		m_dSpecRX[2],		m_dSpecRY[2];
	
	double		m_dSpecMarginLX[2], m_dSpecMarginLY[2];
	double		m_dSpecMarginRX[2], m_dSpecMarginRY[2];

	float*		m_nChartData[15];
	
	int			m_nJobID; //job number

	CChartStandardAxis* m_pBottomAxis_L[2];
	CChartStandardAxis* m_pLeftAxis_L[2];
	CChartStandardAxis* m_pBottomAxis_R[2];
	CChartStandardAxis* m_pLeftAxis_R[2];

public:
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	virtual void UpdateScatterGraph(std::vector<_stInsp_FilmLength_Result> vtFilmResult, int resultSize); // 차트 업데이트 함수
	virtual void DBInsertInspData(double lx, double ly, double rx, double ry);

private:
	void CreateScatterGraph(int nJob);

	virtual void RenewTabView(int nIndex, int nCtrlNum);
	virtual void DrawSpecLine(int nChartNum, int nJob);
	virtual void InitSpec(int nJob);
	virtual void InitTabCtrl(int nMethod);

public:
	afx_msg void OnTcnSelchangeTabChartview(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTcnSelchangeTabChartview2(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnDtnCloseupDatetimepickerStart(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg LRESULT OnViewControl(WPARAM wParam, LPARAM lParam);
};
