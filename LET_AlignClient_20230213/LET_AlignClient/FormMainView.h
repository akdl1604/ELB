#pragma once


#include "TabCtrlEx.h"

struct contourStats;

// CFormMainView 폼 뷰입니다.
class CLET_AlignClientDlg;
class CFormMainView : public CFormView
{
	DECLARE_DYNCREATE(CFormMainView)

protected:
	CFormMainView();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~CFormMainView();

public:
	enum { IDD = IDD_FORM_MAIN };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif
	CTabCtrlEx m_TabMainView;
	CWinThread* m_pThread;

	CLET_AlignClientDlg *m_pMain;
	CDlgViewer *m_pDlgViewerMain[MAX_VIEWER];
	CRect m_rcMainViewerPos[MAX_VIEWER];

	CLabelEx m_lbModelIdTitle;
	CLabelEx m_lbModelId;
	CLabelEx m_lbPanelIdTitle;
	CLabelEx m_lbPanelId;
	CLabelEx m_lbPanelJudge;
	CLabelEx m_lbLogIn;

	HBRUSH m_hbrBkg;

	BOOL m_bProcessStart[MAX_JOB];
	BOOL m_bSubProcessStart[MAX_JOB];
	BOOL m_bProcessEnd[MAX_JOB];

	BOOL m_bModelProcessStart[MAX_JOB];
	BOOL m_bPLCTimeStart;
	BOOL m_bThreadMainDisplayEndFlag;
	BOOL m_bCalcRotateCenter;
	BOOL m_bScratch[MAX_JOB];
	cv::Point2f m_leftRight, m_topBottom;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()	
private:
	CRect m_rcStaticViewerBaseMain;
	CRect m_rtModelIdTitle;
	CRect m_rtModelId;
	CRect m_rtLogIn;

	void replace_viewer_main(int nNumX, int nNumY);
	void createViewer_Main(int nNumX, int nNumY);

public:
	void replace_viewer_main_fix(int nNumX, int nNumY, int id);
	void setProcessStart(int nClient) { m_bProcessStart[nClient] = TRUE; }
	void resetProcessStart(int nClient) { m_bProcessStart[nClient] = FALSE; }
	BOOL getProcessStart(int nClient) { return m_bProcessStart[nClient]; }
	void setSubProcessStart(int nClient) { m_bSubProcessStart[nClient] = TRUE; }
	void resetSubProcessStart(int nClient) { m_bSubProcessStart[nClient] = FALSE; }
	BOOL getSubProcessStart(int nClient) { return m_bSubProcessStart[nClient]; }
	CString get_job_name(int nJob);
	void readGlassID(int nStartAddr, const int nSize, BYTE *pData);
	void readModelID_ZR(int nStartAddr, int nSize, BYTE *pData);
	void readModelID(int nStartAddr, int nSize, BYTE *pData);
	void sendGoldData(int nJob);
	void resultViewLoad(int cam, int id, int command);

	void resetPlcFlag();
	void resetPlcSignal();
	void resetCalibrationModeOn();

	void fnReceivePanelId(int nJob);
	void fnReceiveProcessStart(int id);

	void fnProcess_ProcessAlignAlgorithm(int nJob = 0);
	//KJH 2022-08-04 Robot Align 관련 분기 추가
	void fnProcess_ProcessLive(int nJob = 0);
	void fnProcess_ProcessRobotAlignAlgorithm(int nJob = 0);
	void fnProcess_ProcessInspAlgorithm(int nJob = 0);
	void fnProcess_ProcessBendingInsp(int nJob=0);
	//KJH 2021-07-23 이름좀 통일하자...
	//void fnProcess_ProcessAlignInspAlgorithm(int nJob=0);
	void fnProcess_ProcessCenterAlignInspAlgorithm(int nJob = 0);
	void fnProcess_ProcessRotateCenterPointCheck(int nJob = 0);
	//void fnProcess_ScanInspAlgorithm(int nJob = 0);
	void fnProcess_ProcessScanInspAlgorithm(int nJob = 0);
	void fnProcess_ProcessSideViewAlgorithm(int nJob = 0);
	void fnProcessCenterNozzleAlignAlgorithm(int nJob = 0);
	void fnProcessCenterNozzleGapMeasure(int nJob = 0);
	void fnProcessTeach_Gap_StatusMeasure(int nJob = 0); //20211209 Tkyuha 티칭 상태 점검용 노즐 과 스테이지 갭
	void fnProcessTeach_XYZ_StatusMeasure(int nJob = 0);  //20211209 Tkyuha 티칭 상태 점검용 노즐 X,Y 위치
	void fnProcess_ProcessFilmAlignAlgorithm(int nJob = 0);
	void fnProcess_ScratchInsp(int nJob = 0);

	friend UINT Thread_Load_Before_Image(void* pParam);
	friend UINT Thread_PLCSignalCheck(void *pParam);
	friend UINT Thread_MainViewerDisplay(void* pParam);
	friend UINT Thread_SubInspection(void* pParam);
	friend UINT Thread_SubBMInspection(void* pParam); //20220117 Tkyuha BM부 별도 처리
	friend UINT Thread_InspTraceImageSave(void* pParam);
	friend UINT Thread_RotateCenterCalc(void* pParam);
	friend UINT Thread_RotateCenterCalc2(void* pParam); // KBJ 2022-08-20 이미지 저장 부분 및 엑셀 파일 저장부분 수정

public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg LRESULT OnViewControl(WPARAM wParam, LPARAM lParam);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnClose();
	afx_msg void OnTcnSelchangeTabMainView(NMHDR* pNMHDR, LRESULT* pResult);

	virtual void OnInitialUpdate();

	void MainButtonInit(CButtonEx *pbutton, int size = 15);
	void InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color);
	void drawExistInspResult(int nCam, int nPosNum, int nViewer, int nJob, BOOL bErase);
	void drawAlignResult(int nCam, int nPosNum, int nViewer,int nJob, BOOL bErase);
	void draw_align_result_caliper(int nCam, int nPosNum, int nViewer, int nJob, BOOL bErase);
	void draw_align_result_line(int nCam, int nPos, int nViewer, int nJob, sLine lineInfo,BOOL bErase , BOOL bType = FALSE);
	void draw_pcb_insp_result2(int nCam, int nPos, int nViewer, int nJob, BOOL bErase);
	void draw_film_insp_result(int nCam, int nPos, int nViewer, int nJob, BOOL bErase);
	void draw_scan_insp_result(int nCam, int nPos, int nViewer, int nJob, BOOL bErase);
	void draw_assemble_insp_result2(int nCam, int nPosNum, int nViewer, int nJob, BOOL bErase);
	void draw_notch_line(int nCam, CViewerEx* pViewer, int nJob, BOOL bErase);
	void read_glass_id(int nJob);
	void reset_viewer(int nJob,int subview=0);
	void reset_viewerID(int nJob, int subview);
	void draw_align_result_2cam_2shot(int nCam, int nPosNum, int nViewer, int nJob, BOOL bErase);
	void draw_align_result(int nCam, int nPosNum, int nViewer, int nJob, BOOL bErase , BOOL bType = FALSE);
	void draw_align_result_circle(int nCam, int nPosNum, int nViewer, int nJob, BOOL bErase, BOOL bType = FALSE);
	void set_viewer_name(int nJob);
	void fnProcess_ModelProcess();
	void fnProcess_ModelProcess_2022();
	void fnProcess_BitResetProcess(int nJob);
	void fnProcess_AlignOffsetSendProcess(int nJob);
	void fnProcess_PLCTime(); 
	bool VerifyReCalc(void *rawData, double rX, double rY);
	void InspNotchEdgeDistanceDivide(bool bnot, bool bcir, int nJob, std::vector<cv::Point2f>* ptr, int endX, int W, int H, CViewerEx* mViewer = NULL);
	BOOL InspNotchEdgeDetection(bool bnot, bool bcir, int nJob, int nCam, CPoint ptStart, CPoint ptEnd, CViewerEx* mViewer = NULL, bool balign = false, bool clr=true);
	BOOL InspNotchEdgeDetection_jmLee(bool bnot, bool bcir,int nJob, int nCam, CPoint ptStart, CPoint ptEnd, CViewerEx* mViewer = NULL, bool balign = false);
	//HTK 2022-05-30 본사수정내용
	BOOL InspUT_EdgeDetection(bool bnot, bool bcir, int nJob, int nCam, CPoint ptStart, CPoint ptEnd, CViewerEx* mViewer = NULL, bool balign = false, bool clr = true);
	BOOL InspNotchLine(BYTE *pImage, int w, int h,int nJob, int nCam);
	BOOL InspNotchAlignSearchLine(BYTE* pImage, int w, int h, int nJob, int nCam, CViewerEx* mViewer = NULL);
	BOOL InspNozzleSearchInspection(BYTE* pImage, int w, int h, int nJob, int nCam, CViewerEx* mViewer = NULL, bool clr = true);
	BOOL InspNozzleSearchAngle_Bee(BYTE* pImage, int w, int h, int nJob, int nCam, CViewerEx* mViewer = NULL, BOOL Reverse = FALSE);
	BOOL InspNozzleSearchCaliper_Use(BYTE* pImage, int w, int h, int nJob, int nCam, cv::Point2f& ptResult,bool bGap = false,CViewerEx* mViewer = NULL, BOOL Reverse = FALSE);
	BOOL InspLineEdgeDetection(int nJob, const cv::Mat& src, CPoint ptStart, CPoint ptEnd, float fscanLength, CViewerEx* mViewer = NULL, bool btracking = false, bool bdust = false);
	BOOL InspCircleEdgeDetection(int nJob, const cv::Mat& src, CPoint ptStart, CPoint ptEnd, CViewerEx* mViewer = NULL);
	BOOL InspNotchMetalEdgeDetection(int nJob, int nCam, const cv::Mat& src, CPoint ptStart, CPoint ptEnd, CViewerEx* mViewer = NULL);
	BOOL InspCircleMetalEdgeDetection(int nJob, int nCam, const cv::Mat& src, CViewerEx* mViewer = NULL);
	BOOL InspCircleEdgeDetection_CC(int nJob, int nCam, const cv::Mat& src, CViewerEx* mViewer = NULL,BOOL reInsp=FALSE);
	BOOL InspCircleBMEdgeDetection(int nJob, int nCam, const cv::Mat& src, CViewerEx* mViewer = NULL, BOOL reInsp = FALSE);
    BOOL InspCaliperCircleBMEdgeDetection(int nJob, int nCam, const cv::Mat& src, CViewerEx* mViewer = NULL, BOOL reInsp = FALSE);
	BOOL InspCircleDummyDetection(int nJob, int nCam, const cv::Mat* src, CViewerEx* mViewer = NULL);
	BOOL InspNozzleDistance_align(int nJob, int nCam, BOOL Reverse, BOOL UseRef, const cv::Mat& src, CViewerEx* mViewer = NULL);
	BOOL InspLineDummyDetection(int nJob, int nCam, const cv::Mat* src, CViewerEx* mViewer = NULL);
	double InspLineCGDetection(int nJob, int nCam, const cv::Mat* src, CViewerEx* mViewer = NULL);
	int do_model_process(int mode=0);
	int do_model_process_2022(int mode = 0);
	int model_process_create(CString strCurrModel, CString strTargetModel);
	int model_process_change(CString strCurrModel, CString strTargetModel);
	int model_process_modify(CString strCurrModel, CString strTargetModel);
	int model_process_delete(CString strCurrModel, CString strTargetModel);
	int  InspDustBurrInspection(int nJob, const cv::Mat& gray, int binThresh, int xo,int yo , double ro,int minSize = 10, int threshSize = 150,bool bclr=true,bool preInsp=false);
	CRect findRawImageSize(cv::Mat srcGray, cv::Point2d pt, int width, int height);	

	bool fnGrabSingleImage(int nJob);
	bool fnExtractProfileData(int nJob, CViewerEx* mViewer = NULL,bool bclr=true, BOOL m_bManual = FALSE);
	bool fnExtractMetalProfileData(int nJob, CViewerEx* mViewer = NULL);
	bool fnDiffWetOutInspection(int nJob, int id = 1, CViewerEx* mViewer = NULL, BOOL m_bManual = FALSE);
	bool fnDiffDustInspection(int nJob, int id=1, CViewerEx* mViewer = NULL);
	bool fnEmptyInspection(int nJob, int id = 1, CViewerEx* mViewer = NULL);
	bool fnPreDustInspection(int nJob, int id, BYTE* ImgByte,CViewerEx* mViewer = NULL);
	bool fnCInk2_LackofInkInspection(int nJob, int id, BYTE* ImgByte, CViewerEx* mViewer = NULL); //2022.06.23 Tkyuha CInk2에서 검사 추가
	bool fnCInk2_UV_InkInspection(int nJob, int id, int &count,BYTE* ImgByte, CViewerEx* mViewer = NULL);
	bool fnLineDustInspection(BYTE* srcImg, int W, int H, int nJob,int nCam, int id, CRect roi, CViewerEx* mViewer = NULL);
	double fnCalcNozzleToPN_Distance(BYTE* pImage, int w, int h, int nJob, int nCam, int _thresh, CViewerEx* mViewer = NULL);
	double fnCalcNozzleToPN_Distance2(BYTE* pImage, int w, int h, int nJob, int nCam, int _thresh, CViewerEx* mViewer = NULL);
	double fnCalcYgapCINK2(BYTE* pImage, int w, int h, int nJob, int nCam, int _thresh, CViewerEx* mViewer = NULL);
	double fnCalcNozzleViewPos(BYTE* pImage, int w, int h, int nJob, int nCam, int _thresh, CViewerEx* mViewer = NULL);
	double fnCalcNozzleGap_Distance(BYTE* pImage, int w, int h, int nJob, int nCam, int _thresh, CViewerEx* mViewer = NULL); //20211007 Tkyuha 노즐과 스테이지 갭 측정
	double fnCalcNozzleClean_Insp(BYTE* pImage, int w, int h, int nJob, int nCam, int _thresh, CViewerEx* mViewer = NULL); //20211007 Tkyuha 노즐과 오염 검사
	double fnCalcNozzleDopo_Insp(BYTE* pImage, int w, int h, int nJob, int nCam, int _thresh, CViewerEx* mViewer = NULL); //20211007 Tkyuha 측면 도포 검사
	double fnSearchWideAngle(cv::Point2f _Mpt, cv::Point2f _Gpt); // 원중심을 이용해서 가장 넓은 영역을 찾아 각도를 계산
    //KJH 2021-10-21 FDC 보고함수 추가_Circle
	void SendFDCValue_Circle(int nJob, int real_cam);
	void ShowErrorMessageBlink(CString str);
	void HideErrorMessageBlink();
	void saveAVI_ImageType(int n);

	BOOL IsDoubleClickModelId(CPoint pt);
	BOOL IsClickModelIdTitle(CPoint pt);
	BOOL IsClickModelId(CPoint pt);
	BOOL IsClickLogIn(CPoint pt);

	BOOL CheckStatus(int nJob,int _w,int _h, CDC* pDC=NULL);
	void fnCalcManualCenterAlign(int nJob);
	void fnCalcNozzleGapInsp(int nJob);
	void fnSimulation_CalcNozzleGapInsp(int nJob, int nCam,const cv::Mat& src, CViewerEx* mViewer = NULL);

	int read_profile_data(CString str_path);
	void fnSearchMaxMinPos(int xo,int yo,std::vector< double > data, float th=10.f,CDC *pDC=NULL);
	void DrawArrow(CDC* pDC, COLORREF color, CPoint start, CPoint end, double T);

	//HSJ 검사 멤버변수 추가
	int    m_nNotchLeftStartCount;
	int    m_nNotchRightEndCount;
	double m_InspectionDistance[MAX_JOB];
	double m_dCircleRadius_MP;
	double m_dCircleRadius_CC;
	double m_dLeftNotchLength;
	double m_dRightNotchLength;
	double m_dLineRotateAngle;
	double m_dCircleInBright;
	double m_dCircleInBrightDir[4];

	// JSY 2022-11-05 검사 멤버변수 추가
	/// Comment : Add liquid inspection, dust inspection results item.
	/// Wetout결과파일에 항목 추가
	BOOL m_bDustInspJudge;		// Dust(이물) 검사 결과
	BOOL m_bLiquidInspJudge;	// Liquid(액튐) 검사 결과
	// END JSY 2022-11-05 검사 맴버변수 추가
	
	//KJH 2021-07-20 Line Center Align 인자 추가
	int m_dDopoType;
	int m_dLineNum;
	int m_dDummyLineNum;
	int m_iSaveImageTypeNum;
	double m_dInclination; 				// Adaptive Dispensing 사용 기울기
	double m_dADispesing_StartAngle; 	// Adaptive Dispensing 사용 기울기
	double m_dADispesing_EndAngle; 		// Adaptive Dispensing 사용 기울기
	double m_dAccLineLength;
	double m_dDecLineLength;
	double m_dCircleDistance_X_MP;
	double m_tDopoEdgeDentAngle[2];
	CPoint m_ptDopoEdgeDentPos[2];
	double m_tDopoEdgeDOentAngle[2];
	CPoint m_ptDopoEdgeDOentPos[2];
	CPoint m_ptCircleCenter_MP;
	CPoint m_ptCircleCenter_CC;
	CPoint m_ptCircleCenter_CC_Pre;
	std::vector<contourStats> m_vecDustResult;
	std::vector<cv::Point2f> m_vecInterSectionPoint; 
	_stInspectionPattern m_thInspParam;

	// 20210923 Tkyuha Circle에서만 사용 하는 파라미터 
	// 원을 찾을때 제외 영역을 메탈 찾는 경우에 적용 하기 위함
	int  m_Index_start;
	int  m_Index_end;
	bool m_Index_bsplit; // 0도 기준으로 양쪽 으로 영역이 나뉜경우 체크
	double m_IndexMaxWidth_Angle; //메탈과 Glass 사이거리 최대값 최대각도

	int m_posLineY_Move;
	double m_distLineY_Move;

	//HTK 2022-03-30 Tact Time Display 추가
	clock_t m_clockProcStartTime[MAX_JOB * 2];
	double  m_clockProc_elapsed_time[MAX_JOB *2];

	bool process_notch_dopo_inspection(BYTE* procImg, BYTE* viewImg, int nJob, int nCam, int w, int h, CViewerEx* pViewer);
	bool process_find_notch_metal(BYTE* procImg, int nJob, int nCam, int w, int h, CViewerEx *pViewer);
	bool judge_notch_metal(int nJob, int nCam);
	bool find_notch_metal_edge(BYTE* pImage, int nJob, int nCam, int w, int h);

	int find_notch_dopo_edge(BYTE* pImage, int nJob, int nCam, int w, int h);
	int find_notch_dopo_start_end(BYTE* pImage, int nJob, int nCam, int w, int h);
	int calculate_notch_dopo_data(int nJob, int nCam);
	bool judge_notch_dopo_insp(int nJob, int nCam);

	void display_notch_dopo_result(int nJob, int nCam, CViewerEx* pViewer);
	void display_notch_metal(int nJob, int nCam, CViewerEx *pViewer);

	void write_notch_dopo_insp_result(int nJob, int nCam, CString str_path);

	void trace_data_fitting(std::vector<cv::Point2f> &pts);
	BOOL get_2nd_order_regression(std::vector<double>* srcX, std::vector<double>* srcY, double *b0, double *b1, double *b2);

	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnMButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);

	BOOL m_bInspFlag[MAX_JOB];
	BOOL m_bResetBitFlag[MAX_JOB];
	BOOL m_bSendOffsetFlag[MAX_JOB];
	BOOL m_bResetBit2PosFlag[MAX_JOB];
	// hsj 2022-02-14 확인중
	BOOL m_bFindCaliper;
	BOOL GetCaliperFind() { return m_bFindCaliper; };
	void SetCaliperFind(BOOL bFindCaliper) { m_bFindCaliper = bFindCaliper; };

	BOOL m_bRecordCheck;
	int  m_nLackOfInk_CinkCount;
	bool m_bLackofInkCheckFlag[360];

	BOOL m_bCheckState;

	// KBJ 2022-07-13 3회 연속 Lack Of INK NG 일시 팝업 
	int  m_nCount_LackOfCink_NG;
	int  m_nCurrentSelMainView;

	void change_job_main_view(int nIndex);
	BOOL Inspection_ScratchHole(int nJob, int id, BYTE* ImgByte, CViewerEx* mViewer = NULL, bool bSimul = FALSE);
	friend UINT Inspection_ScratchHole_LoadProcImage(void* pParam);
	int m_nResetBitFlag_Count[MAX_JOB];
	
	BOOL m_bFirstTrace;
	BOOL m_bHandShake[MAX_JOB];
	BOOL m_bTraceSearchFindLine[C_CALIPER_POS_6];   //Tkyuha 20221123 Line검사 수정
	sLine m_TraceSearch_line_info[C_CALIPER_POS_6]; //Tkyuha 20221123 Line검사 수정

	std::vector<cv::Point2f> m_vtEdgePosition; // KBJ 2022-09-07 결과 이미지용 벡터

	int ELB_Error_Type(long Current_Error, int nErrorType);
	void ELB_RawImageBMinInspectionMode(cv::Mat& scrImg, int _nCam=0, int _nJob=0); // Tkyuha 221108 이미지 하나로 검사 기능 할수 있도록 추가
	void SetResCalibration();

	////SJB 2022-10-31 Insp Overflow Metal Line 도포 전 Trace 측정 시 Caliper 데이터로 측정	//SJB 2022-11-29
	//sLine m_Trace_Metal_Line_Info;
	//BOOL m_Trace_Metal_Find_Line;

	//SJB 2022-11-11 Overflow Min 판정 ErrorCode 추가
	int m_InspErrorCode;

	// YCS 2022-11-30 WetOut ASPC 판정 변수 추가
	std::vector<bool>m_vJudge_WetOut_ASPC;
	std::vector<bool>m_vWarning; // 경고창 팝업이 처리된 확인하기 위한 변수
	bool m_bWarningFlag_WetOut_ASPC;
	bool m_bLiquidDropResult;
};