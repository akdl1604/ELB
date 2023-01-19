// LET_AlignClientDlg.h : 헤더 파일
//

#pragma once

#ifndef __LET_AlignClient_DLG_H___
#define __LET_AlignClient_DLG_H___

#include "Benaphore.h"
#include "IniFile.h"
#include "PaneHeader.h"
#include "PaneBottom.h"
#include "PaneManual.h"
#include "PaneCamera.h"
#include "PaneModel.h"
#include "PaneAuto.h"
#include "PaneMachine.h"
#include "PaneSimulation.h"
#include "PaneBottomSimulation.h"

#include "FormMainView.h"
#include "FormCameraView.h"
#include "FormMachineView.h"
#include "FormModelView.h"
#include "PanelExistInspection.h"
#include "FormPlcShareMemoryView.h"
#include "FormSystemOptionView.h"
#include "FormTrendView.h"

#include "Socket.h"
#include <direct.h>
#include "PatternMatching.h"
#include "MachineSetting.h"
#include "Prealign.h"
#include "OnscreenKeyboardDlg.h"
#include "AlignMeasureInspection.h"
#include "InspCounter.h"
#include "CSaperaCamera.h"
#include "ComPLC_RS.h"
#include "DlgManualInputMark.h"
#include "DlgManualInputMark1Cam.h"
#include "Caliper\DlgCaliper.h"
#include "ScanInspWorker.h"
#include "SratchInspWorker.h"
#include "CamLightControlDlg.h"

#include "DlgMsg.h"
#include "LightControl.h"
#include "CLLightSock.h"
#include "TabModelAlgoPage.h"
#include "UI_Control/CircleProgress.h"
#include "UI_Control/Stopwatch.h"
#include "TrendDataBaseClass.h"
#include "OffsetDlg.h"
#include "CPFScaleCalc.h"
#include "GridInspectionParamDlg.h"
#include "DlgList.h"
#include "DlgSelectJob.h"
#include "DlgAlarm.h"
#include "VAT_SimulatorDlg.h"
#include "An_engineering_calculator.h"

#include "CInspChartView.h"

#ifdef _SDV_VAT
#include "SamsungVATWraper.h"
#endif
#ifdef _DAHUHA
#include "Basler/DahuaCameraHelper.h"
#endif

//lhj add 220503 
struct stSameCenterItem
{
	BOOL bWetoutEnable; // UseDiffInsp
	BOOL bDustEnable; // UseInputInspection
	BOOL bLiquidDropsEnable; //UseDustInsp
	int inputThresh; //ExistDummyGray
	int iLiquidDropsThresh;
	int hthresh;
	int lthresh;
	double dCircleLength;
	double dCenterMetalDistanceSpec;
	BOOL bCircleShapeModeEnable; //22.06.02
};
//

// CLET_AlignClientDlg 대화 상자
class CLET_AlignClientDlg : public CDialogEx
{
// 생성입니다.
public:

	CLET_AlignClientDlg(CWnd* pParent = NULL);	// 표준 생성자입니다.
	~CLET_AlignClientDlg();

	//lhj add 220503
	struct stSameCenterItem sFileData;
	struct stSameCenterItem sGridData;
	struct stSameCenterItem sUIData;
	//

	char m_szWorkingPath[MAX_PATH];
	
	CString m_strDir;
	CString m_strSystemDir;
	CString m_strCalibrationDir;
	CString m_strConfigDir;
	CString m_strResultDir;
	CString m_strModelDir;
	CString m_strCurrModelDir;
	CString m_strHistoryDir;
	CString m_strMachineDir;
	CString m_strWorkDir;
	CString m_strLogDir;// 
	CString m_strImageDir;
	CString m_strVideoDir;
	CString m_strEquipName;
	CString m_strClientName;
	CString m_strComFolerName;
	CString m_strDopoValue;
	CString m_strSuckValue;
	CString m_strRDP_LogDir;

	CString m_strLastModel;
	CString m_strCurrModel;
	CString m_strCurrentModelName;
	CString m_strCurrentModelPath;
	CString m_strDispModelName;	// Model View 창에 선택 되어진 Model Name	
	CString m_strProcessGlassID;
	CString m_strResultDate[MAX_CAMERA];
	CString m_strResultTime[MAX_CAMERA];
	CString m_strLastInspectionFolder;
	CString m_strLastAlignFolder[4];

	CIniFile m_iniFile;
	CIniFile m_iniCameraFile;
	CIniFile m_iniViewerFile;
	CIniFile m_iniAlgoFile;
	CIniFile m_iniJobFile;

	stopwatch::Stopwatch my_watch;

	CTrendDataBaseClass m_dbResultDataBase;
	Benaphore m_csShareMem;
	COnscreenKeyboardDlg *m_pKeyboardDlg;
	CDlgManualInputMark *m_pManualInputMark[MAX_CAMERA];
	CDlgManualInputMark1Cam* m_pManualInputMark1Cam[MAX_JOB];
	CCircleProgress		 m_pCircleProgress;
	CCamLightControlDlg* m_pCamLightControlDlg;
	CPFScaleCalc*		 m_pPFScaleCalc;
	// YCS 2022-08-20 차트 다이얼로그 추가
	CInspChartView* m_pCInspChartDlg[2];
	CVAT_SimulatorDlg* m_pVAT_SimulatorDlg;
	An_engineering_calculator* m_pCalculator_Dlg;

	CDlgMsg *m_dlgMsg;
	CSaperaCamera m_pSaperaCam[MAX_CAMERA];
	CScanInspWorker m_pSaperaInspWorker[MAX_CAMERA];
	CSratchInspWorker m_pScratchInspWorker[MAX_CAMERA]; // Scratch Inspection 20221121 Tkyuha 멀티 쓰레드 대응
	

#ifdef _DAHUHA
	Dahua::Infra::TVector<ICameraPtr>	m_vCameraPtrList;
	CDahuaCameraHelper m_pDahuhaCam[MAX_CAMERA];
#else 
	CCamera** m_pBaslerCam;
#endif

	CDlgCaliper *m_pDlgCaliper;
	CInspCounter m_InspCounter;
	CCallbackThread	m_thdTime;
	CLightControl m_Light[MAX_LIGHT_CTRL];
	CLLightSock m_LightSock[MAX_LIGHT_CTRL];

	_stCameraInfo m_stCamInfo[MAX_CAMERA];
	_stInsp_DistanceResult m_stInsp_DistanceResult[MAX_CAMERA][MAX_PATTERN_INDEX];
	_stSaveImage_DistanceResult m_stSaveImage_DistanceResult[MAX_CAMERA][MAX_PATTERN_INDEX];

	double m_dDistanceInspDataResult[NUM_ALIGN_POINTS];
	BOOL m_dDistanceInspJudgeResult[NUM_ALIGN_POINTS];
	double m_dDistanceInspSubDataResult[NUM_ALIGN_POINTS];
	BOOL m_dDistanceInspSubJudgeResult[NUM_ALIGN_POINTS];

	CView *m_pForm[MAX_FORM];
	CView *m_pPane[MAX_PANE];
	CView *m_pPaneHeader;
	CView *m_pPaneBottom[MAX_B_PANE];

	std::vector<CJobResult> vt_result_info;

	std::vector<CJobInfo> vt_job_info;
	std::vector<CViewerInfo> vt_viewer_info;
	std::vector<CSystemOption> vt_system_option;
	_stSystemCommon m_system_option_common;

	//CDisplayViewInfo m_pViewDisplayInfo[MAX_CAMERA];
	//CAlgorithmInfo m_pAlgorithmInfo;
	CAlignMeasureInspection m_ManualAlignMeasure;
	CPanelExistInspection m_PanelExist;
#ifdef _SDV_VAT
	SamsungVATWraper      m_classSamsungVATWraper;
#endif
	CModel m_currModel;
	CModel m_NewModel;
	CMachineSetting m_MachineSetting[MAX_JOB];
	CPatternMatching m_PatternMatching[MAX_JOB];
	CPrealign m_Prealign[MAX_JOB];
	CInspSpecPara	m_InspSpecPara[MAX_JOB];

	HWND m_hHeaderBar;
	BOOL m_bGrabEnd[MAX_CAMERA];
	BOOL m_bProcessEnd[MAX_CAMERA];
	BOOL m_bLiveDisplay[MAX_CAMERA];

	BOOL m_bSimulGrabEnd[MAX_CAMERA][NUM_POS];

	BOOL m_bProgramEnd;
	BOOL m_bAutoStart;
	BOOL m_bSimulationStart;
	BOOL m_bPLCConnect;
	BOOL m_bPlcAlive;
	BOOL m_bInitCheck;
	BOOL m_bThreadPLCSignalCheckEnd;
	BOOL m_dMarkInputTime;

	int m_nPLCAlive;
	int m_nPLCAliveCount;	

	IplImage *m_pIplSrcImage;	// Panel 유무 검사 할때 사용

	BYTE *m_pCameraViewImage;
	BYTE **m_pSrcImage;
	BYTE ***m_pProcImage;
	BYTE ***m_pSaveImageBuf;
	cv::Mat m_matResultImage;
	cv::Mat m_matScanResultImage[MAX_CAMERA][MAX_CAMERA];
	BOOL m_bPrealignStartFlag[MAX_CAMERA];
	BOOL m_bGrabEndFlag[MAX_CAMERA];
	BOOL m_bCalibGrabEndFlag[MAX_CAMERA];

	BOOL m_bProcessResult[MAX_CAMERA];

	long *m_nBitData;
	long *m_nBitReadData;	// 19.10.26
	long *m_nBitWriteData;
	long *m_nBitPreData;
	long *m_nBitWritePreData;
	long *m_nWordData;
	double m_dbLCheck[MAX_CAMERA][4];
	double m_dbLCheckSpec[MAX_CAMERA][4];

	double m_dbRevisionData[MAX_CAMERA][4];
	double m_dbRevisionData_obejct[MAX_CAMERA][4][4];
	double m_dbLcheckLength[MAX_CAMERA];
	double m_dMarkRegistRatio;
	double m_dCalibrationMarkMinScore;	//211204 Tkyuha
	double m_dReeltoStageDistance[2];	//KJH 2022-03-11 Reel to Stage Distance 
	int	   m_dDisplayDir[MAX_JOB];		//KJH 2022-03-16 Time Display Dir
	clock_t m_clockStartTime[MAX_JOB];
	CString m_dActTime[MAX_JOB];		//KJH 2022-03-16 Act Time Display
	CString m_dSeqTime[MAX_JOB];		//KJH 2022-03-16 Seq Time Display

	BOOL m_bSaveReslutImage;
	BOOL m_bSaveRawImage;
	BOOL m_bSaveReslutImage_simulation[MAX_JOB];
	BOOL m_bSaveRawImage_simulation[MAX_JOB];
	int  m_nSaveTerm;
	int  m_nHDD_UsedRate;
	BOOL m_bUsePlcTime;

	int m_nClientID;
	int m_nNumCamera;
	int m_nViewX_Num;
	int m_nViewY_Num;

	int m_nCamWidth;
	int m_nCamHeight;

	int m_nCurForm;
	int m_nOldForm;
	int m_nCurPane;
	int m_nOldPane;
	int m_nCur_B_Pane;
	int m_nOld_B_Pane;

	int m_nStartReadBitAddr;
	int m_nStartReadBitAddr_Org;
	int m_nSizeReadBit;

	int m_nStartWriteBitAddr;
	int m_nStartWriteBitAddr_Org;
	int m_nSizeWriteBit;

	int m_nStartReadWordAddr;
	int m_nSizeReadWord;

	int m_nStartWriteWordAddr;
	int m_nSizeWriteWord;

	int m_nSizeReadBit_Org;
	int m_nSizeWriteBit_Org;

	int m_nCalibrationAck;
	int m_nCalibrationModeOnAck;
	int m_nCalibRevisionAck;

	int m_nSeqSecCalibData;
	int m_nSeqTotalInspGrabCount;
	int m_nSeqCurrentInspGrabCount;
	int m_nSeqNozzleGrabCount;
	int m_nNozzleAlignCount;
	BOOL m_binitNozzleCheck;

	int count_calib_time_out;
	int addr_rb_curr_calib_start;
	int addr_wb_curr_calib_start;
	int addr_wd_curr_calib_data;

	int m_nAlignRequestRead[MAX_JOB];
	int m_nAlignOkWrite[MAX_JOB];
	int m_nAlignNgWrite[MAX_JOB];
	int m_nAlignCompletWrite[MAX_JOB];
	int m_nAlignLcheckNgWrite[MAX_JOB];
	int m_nAlignManualMarkWrite[MAX_JOB];
	int m_nInspRequestRead[MAX_JOB];
	int m_nInspWordDataRead[MAX_JOB];
	int m_nInspAckWrite[MAX_JOB];
	int m_nInspOkWrite[MAX_JOB];
	int m_nInspNgWrite[MAX_JOB];
	int m_nInspResultWrite[MAX_JOB];
	int m_nCurrentXMotorPos[MAX_JOB];
	int m_nCurrentYMotorPos[MAX_JOB];
	int m_nCurrentTMotorPos[MAX_JOB];
	int m_nRevisionXMotorPos[MAX_JOB];
	int m_nRevisionYMotorPos[MAX_JOB];
	int m_nRevisionTMotorPos[MAX_JOB];

	int m_nSeqScanInspGrabCount[MAX_JOB];
	//BYTE m_cPanel_Read_ID[MAX_CAMERA][MAX_PATH];

	int m_nRetryCount[MAX_CAMERA];
	int m_nSeqProcess[MAX_CAMERA];
	int m_nGrabTimeOutCount[MAX_CAMERA];
	int m_nGrabWaitTimeOutCount[MAX_CAMERA];
	int m_nErrorType[MAX_CAMERA];
	
	BOOL m_bThreadEndFlag;
	BOOL m_bRetryOkNg[MAX_CAMERA];
	BOOL m_bInspResult[MAX_CAMERA];
	BOOL m_bLcheckNG[MAX_CAMERA];
	BOOL m_nManualInputMarkResult[MAX_CAMERA];
	BOOL m_nManualInputMarkCount[MAX_JOB];

	_stDrawInfo m_stDrawInfo;

	BOOL m_bFindPattern[MAX_CAMERA];
	BOOL m_bFindPatternEnd[MAX_CAMERA];
	BOOL m_bFind_object[2];

	BOOL m_bFindAlignPattern[MAX_CAMERA][MAX_INSP_COUNT];
	BOOL m_bFindAlignPatternEnd[MAX_CAMERA][MAX_INSP_COUNT];
	BOOL m_bFindInspPattern[MAX_CAMERA][MAX_INSP_COUNT];
	BOOL m_bFindInspPatternEnd[MAX_CAMERA][MAX_INSP_COUNT];

	CPoint m_dDummyStart_posXY;
	int m_dDummyInspRangeHeight;
	double m_dmatching_posX;
	double m_dmatching_posY;
	double m_dScanImageLength[MAX_CAMERA];

	int m_nSpecRowCount;
#ifndef JOB_INFO
	BOOL m_bMakeDefaultCoordinates[MAX_CAMERA];
#else
	BOOL m_bMakeDefaultCoordinates[MAX_CAMERA][NUM_POS];
#endif
	int m_nSaveImageRingBufIndex[MAX_JOB][MAX_SAVE_IMAGE_RING_BUF];			// 5개 이상의 이미지를 동시에 처리 못했을 경우에는 무시 ㅠ_ㅡ
	int m_nSaveImageRingBufIndex_Curr[MAX_JOB][MAX_SAVE_IMAGE_RING_BUF];
	char m_cSaveImageRingBufPath[MAX_JOB][MAX_CAMERA][MAX_SAVE_IMAGE_RING_BUF][MAX_PATH];
	char m_cSaveImageRingBufPath_Raw[MAX_JOB][MAX_CAMERA][MAX_SAVE_IMAGE_RING_BUF][MAX_PATH];
	_stSaveImageInfo m_stSaveImageInfo[MAX_JOB][MAX_CAMERA][MAX_SAVE_IMAGE_RING_BUF];

	_stFindPattern m_pPattern[MAX_CAMERA];
	stLIGHTCTRLINFO m_stLightCtrlInfo;
	CRITICAL_SECTION m_csScanWrite;
	CRITICAL_SECTION m_csScanBurInsp;
	HANDLE  m_hMetalSearchEvent;

	BOOL m_bFocusMeasure;
	BOOL m_bAutoSearch;
	BOOL m_bELB_LightControlMethod;
	
	double m_dbFocusEnergy;
	double m_dbAvgGray;
	double m_dbCirceErrorLossRate; // Tkyuha 221108 FDC 원 찾는 에러율 추가
	double m_dCurrentSearchPos[MAX_CAMERA];
	double m_dCurrentRobotSearchPos[MAX_CAMERA];
	//KJH 2021-07-23 명칭변경
	//double m_dCurrentRotateOffsetX;
	double m_dCurrentRotateOffsetX;
	double m_nCurrentCircleAccDec;
	double m_nCurrentNotchAcc;
	double m_nCurrentNotchDec;

	int m_nCurrentCircleCount;
	int m_nExistResult;//210110
	double m_nSecondCircleOffset_Y;
	double m_nThirdCircleOffset_Y;
	double m_dCircleThirdThetaPos;
	double m_nFourCircleOffset_Y;
	double m_dCircleFourThetaPos;
	double m_dCircleSecondThetaPos;
	double m_dExistResultData;
	double m_dNozzleCurrentAngle;
	double m_dNozzleTipSize;
	double m_dCircleRadius_PN;
	double m_dWetoutProperty[10];
	CString m_strExistResult;
	CString m_strGoldenDataFile;
	std::vector < CRect> m_RectExistBlob;

	CString m_strPlcIP;
	CString m_strServerIP;
	int m_nSockPlcPort;

	////// Socket ////////
	CRITICAL_SECTION m_csSendPacket;
	CRITICAL_SECTION m_cs;	
	SRWLOCK g_srwlock; // 메인 Thread와 Process Thread 동기화 객체 20211027 Tkyuha	
	SRWLOCK g_srScratchwlock; // 스크레치 검사 동기화 객체 20221121 Tkyuha	

	HANDLE mh_Thread;
	TCP::cSocket::cHash<SOCKET, DWORD> mi_SocketList;
	TCP::cSocket      mi_Socket;
	SOCKET m_hServerSockID;
	ToPacketData m_RecvPacket;
	ToPacketData m_SendPacket;
	BOOL m_bConnectServer;
	DWORD m_dwCurrentReceiveSize;

	int m_nServerIP;
	int m_nServerPort;
	cv::Point2d m_dCPos[8];
	
	int m_nLogInUserLevelType;
	BOOL m_bCleaningLive;
	/////////////////////

	_stInsp_ELB_Result m_ELB_TraceResult;
	_stInsp_ELB_Result m_ELB_DiffInspResult[3];
	_stInsp_ELB_ScanResult m_ELB_ScanInspResult_UT[2][4];
	std::vector<cv::Point> m_ELB_vtMaskContour;
	std::vector<cv::Point2f> m_ELB_vtMaskRawContour;
	std::vector<_stInsp_WetOut_Result> m_vInspWetOutResult;
	std::vector < _stRef_AirPresHighVolts_V> m_ELB_RefData;

	// YCS 2022-08-20 필름검사 데이터(LX, LY, RX, RY) 저장하는 벡터
	std::vector<_stInsp_FilmLength_Result> m_vInspFilmResult[2];


	IplImage* getIplSrcBuffer() {
		if (m_pIplSrcImage != NULL)
			return m_pIplSrcImage;
		else return NULL;
	}

	BYTE* getSrcBuffer(int nCam) {
		if (m_pSrcImage!= NULL && m_pSrcImage[nCam] != NULL)
		{
			return m_pSrcImage[nCam];
		}
		else return NULL;
	}

	BYTE* getProcBuffer(int nCam, int nPos) {
		if (m_pProcImage[nCam][nPos] != NULL)
		{
			if( m_pProcImage[nCam][nPos] != NULL )
				return m_pProcImage[nCam][nPos];
			else return NULL;
		}
		else return NULL;
	}

	BYTE* getCameraViewBuffer() {
		if (m_pCameraViewImage != NULL)
			return m_pCameraViewImage;
		else return NULL;
	}

	BYTE* getSaveImageBuffer(int nCam, int nIndex) {
		if (m_pSaveImageBuf[nCam][nIndex] != NULL)
			return m_pSaveImageBuf[nCam][nIndex];
		else return NULL;
	}
// 대화 상자 데이터입니다.
	enum { IDD = IDD_FPS_CLIENT_DIALOG };
	enum {
		LIGHT_OFF,
		LIGHT_ON,
	};

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.

// 구현입니다.
protected:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);

	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg LRESULT OnViewControl(WPARAM wParam, LPARAM lParam);
	afx_msg void OnClose();
	afx_msg LRESULT OnMessageFunc(WPARAM wParam, LPARAM lParam);
	DECLARE_MESSAGE_MAP()

	int (CLET_AlignClientDlg::*m_pFunc[MAX_CAMERA]) (int id);

private:
	void Loadini(void);
	void LoadViewerini(void);
	void LoadAlgoini(void);
	void LoadJobInfo(void);
	void LoadCameraini(void);
	void LoadPlcIni();
	void initBuffer();
	void initCaliper();
	void initPLC_CommunationAddress();
	void connectLightCtrl_NewType();
	void releaseBuffer();
	void releaseObject();
	void load_system_option();
public:
	friend UINT Thread_Process(void* pParam);
	friend UINT Thread_FindPattern(void* pParam);		// 17.12.30 jmLee		Auto 모드일때 라이브 처리 하도록..
	friend UINT Thread_FindNozzlePattern(void* pParam);
	friend UINT Thread_PcbDistanceInspection(void* pParam);
	friend UINT Thread_AssembleInspection(void* pParam);
	friend UINT Thread_EdgeIspection(void* pParam);
	friend UINT Thread_ImageGrab(void *pParam);
	friend UINT Thread_ImageGrabSapera(void* pParam);
	friend UINT Thread_DiffResultWrite(void* pParam);
	friend UINT Thread_Ink_45CamInspection(void* pParam);
	friend UINT Thread_Ink_UVInspection(void* pParam);
	friend UINT Thread_ScratchInsp(void* pParam);

	friend UINT Thread_ImageSave(void *pParam);
	friend UINT Thread_ImageSave_Capture(void* pParam);
	friend UINT Thread_ScanImageSave(void* pParam);
	friend UINT Thread_ScanOneImageSave(void* pParam);
	friend UINT Thread_InspImageSave(void *pParam);
	friend UINT Thread_assemble_Insp_ImageSave(void *pParam);
	friend UINT Thread_pcb_Insp_ImageSave(void *pParam);
	friend UINT Thread_CaliperImageSave(void *pParam);
	friend UINT Thread_CaliperImageSave2(void* pParam);
	friend UINT Thread_Film_Insp_ImageSave(void* pParam);
	friend UINT Thread_NozzleView_Insp_ImageSave(void* pParam);
	friend UINT Thread_CircleImageSave(void* pParam);

	friend UINT Thread_LogDelete(void *pParam);
	friend UINT Thread_LogDelete2(void* pParam);
	friend UINT Thread_Check_Model_Process_Able(void* pParam);
	

	int fnSetMessage(int nType, CString strText1, CString strText2 = _T(""));
	void copyMemory(void *pDst, void *pSrc, int size);	
	void hideForm();
	void hidePane();
	int changePane(int nID);
	int changeBottomPane(int nID);
	int changeForm(int nID);

	int seqSendCalibData();
	int seqSendCalibAckData();

	BOOL getPLCConnect() { return m_bPLCConnect; }
	void setPLCConnect() { m_bPLCConnect = TRUE; }
	void resetPLCConnect() { m_bPLCConnect = FALSE; }

	void setStartReadBitAddr(int nAddress) { m_nStartReadBitAddr = nAddress; }
	void setStartWriteBitAddr(int nAddress) { m_nStartWriteBitAddr = nAddress; }
	void setStartReadWordAddr(int nAddress) { m_nStartReadWordAddr = nAddress; }
	void setStartWriteWordAddr(int nAddress) { m_nStartWriteWordAddr = nAddress; }
	void setSizeReadBit(int nSize) { m_nSizeReadBit = nSize; }
	void setSizeWriteBit(int nSize) { m_nSizeWriteBit = nSize; }
	void setSizeReadWord(int nSize) { m_nSizeReadWord = nSize; }
	void setSizeWriteWord(int nSize) { m_nSizeWriteWord = nSize; }

	int getStartReadBitAddr() { return m_nStartReadBitAddr; }
	int getStartWriteBitAddr() { return m_nStartWriteBitAddr; }
	int getStartReadWordAddr() { return m_nStartReadWordAddr; }
	int getStartWriteWordAddr() { return m_nStartWriteWordAddr; }
	int getSizeReadBit() { return m_nSizeReadBit; }
	int getSizeWriteBit() { return m_nSizeWriteBit; }
	int getSizeReadWord() { return m_nSizeReadWord; }
	int getSizeWriteWord() { return m_nSizeWriteWord; }

	void CompareMemoryAddr();	
	void SendServer(int cmd, int nParam1 = 0, int nParam2 = 0, void *pData = NULL);

	void setDispModelName(CString strModel) { m_strDispModelName = strModel; }
	void setLastModel(CString strModel) { m_strLastModel = strModel; }
	void setCurrModelName(CString strModel) { m_strCurrentModelName = strModel; }
	void setCurrModelPath(CString strModel) { m_strCurrentModelPath.Format("%s%s\\", m_strModelDir, strModel); }

	CString getDispModelName() { return m_strDispModelName; }
	CString getCurrModelName() { return m_strCurrentModelName; }
	CString getLastModel() { return m_strLastModel; }
	CString getCurrModel() { return m_strCurrModel; }

	void writeLastModel(CString strLastModel);
	void readLastModel();

	BOOL process_model_copy(CString src_model_name, CString dst_model_name);

	BOOL copyModelFiles(CString strSrcDir, CString strDstDir, CString src_model_name, CString dst_model_name);
		//BOOL copyModelFiles(CString strSrcDir, CString strDstDir);
	BOOL saveModelData(int algo, CString strPath);
	BOOL saveModelData(int algo, CModel model);
	BOOL readAllModelData(int algo, CModel *model, CString model_path = "");
	BOOL readAllModelData(int algo, CString strPath);
	BOOL allModelChange(CString strModelName);
	BOOL saveAllModelData(int algo, CModel *model);
	BOOL saveIniFlag();

	void setProcessStartFlag(int index) { m_bPrealignStartFlag[index] = TRUE; }
	void resetProcessStartFlag(int index) { m_bPrealignStartFlag[index] = FALSE; }
	BOOL getProcessStartFlag(int index) { return m_bPrealignStartFlag[index]; }

	void setGrabEndFlag(int nCam) { m_bGrabEndFlag[nCam] = TRUE; }
	void resetGrabEndFlag(int nCam) { m_bGrabEndFlag[nCam] = FALSE; }

	void save_result_image(BOOL bJudge, int algo, BOOL bRetryEnd = FALSE);
	void save_result_image_nozzle_xy(BYTE *pImage, BOOL bJudge, int nJob, int nCam);
	//KJH 2021-12-20
	void save_result_image_nozzle_gap(BYTE *pImage, BOOL bJudge, int nJob, int nCam, double m_dCGThickness);
	void SaveResultImageINFRA_Align(BOOL bJudge, int algo,BOOL bRetryEnd);
	void SaveResultImageScan_Insp(BOOL bJudge, int algo, BOOL bRetryEnd);
	void SaveResultImageFilm_Insp(BOOL bJudge, int algo, BOOL bRetryEnd = FALSE);
	void SaveResultImageNozzleView_Insp(BOOL bJudge, int algo, BOOL bRetryEnd = FALSE);
	void save_result_image_1cam_2object(BOOL bJudge, int algo, BOOL bRetryEnd = FALSE);
	void save_result_image_1shot_align_infra(BOOL bJudge, int algo, BOOL bRetryEnd = FALSE);
	void save_result_image_ELB_center_align(BOOL bJudge, int algo, BOOL bRetryEnd = FALSE);
	void save_result_insp_image(BOOL bJudge, int algo, BOOL bRetryEnd);
	void SaveResultImageInspection(BOOL bJudge, int algo, BOOL bRetryEnd);
	void save_result_image_pcb_distance_insp(BOOL bJudge, int algo, BOOL bRetryEnd = FALSE);
	void save_result_image_2cam_2shot(BOOL bJudge, int algo, BOOL bRetryEnd);
	void SaveResultImageNozzle_XYZ_Insp(BOOL bJudge, int algo, BOOL bRetryEnd);

	void drawMarkPos(int algorithm);

	void draw_mark_pos_2cam_1shot(int algorithm);
	void draw_mark_pos_2cam_2shot(int algorithm);
	void draw_mark_pos_4cam_1shot(int algorithm);
    void draw_mark_pos_1cam_elb(int algorithm);
	void draw_mark_pos_1cam_nozzle(int algorithm);
	void draw_mark_pos_1cam_1shot(int algorithm);
	void draw_mark_pos_1cam_2object(int algorithm);
	void draw_align_revision(int algorithm, int nRet); // kbj 2022-02-05 Draw text of revision data

	void draw_inspection(int nJob);
	void drawInspectionDistancePos(int algorithm);
	void draw_pcb_distance_insp(int algorithm);
	void draw_film_distance_insp(int algorithm);
	
	void draw_calib_direction(int nJob, int nForm);

	void write_process_log(BOOL bJudge, int algo);
	void writeLengthData(CString strID, double length, int nType,int algo);
	BOOL write_result_align_data_1cam_2pos(CString serialNum, BOOL bJudge, int algo);
	BOOL write_result_align_data_2cam_1pos(CString serialNum, BOOL bJudge, int algo);
	BOOL write_result_align_data_4cam_1pos(CString serialNum, BOOL bJudge, int algo);
	BOOL write_result_align_data_2cam_2pos(CString serialNum, BOOL bJudge,int algo);
	BOOL write_result_align_data_1cam_2object(CString serialNum, BOOL bjudge, int algorithm);
	BOOL write_result_exist_insp(CString serialNum, BOOL bjudge, int algorithm);
	BOOL write_result_assemble_insp(CString serialNum, BOOL bjudge, int algorithm);
	BOOL write_result_pcb_distance_insp(CString serialNum, BOOL bjudge, int algorithm);
	BOOL write_result_diff_insp_ELB(CString serialNum, BOOL bjudge, int algorithm);
	BOOL write_result_align_data_1cam_1shot_ELB(CString serialNum, BOOL bJudge, int algo);
	BOOL write_result_Film_insp(CString serialNum, BOOL bjudge, int algorithm);
	BOOL write_result_Film_insp_new(CString serialNum, BOOL bjudge, int algorithm);
	BOOL write_result_NozzleView_insp(CString serialNum, BOOL bjudge, int algorithm);
	//KJH2 2022-08-20 Rotate Data
	BOOL write_result_rotate_data(CString strImageDirPath, BOOL bJudge, int algo, double xr, double xy);
	BOOL write_result_rotate_data_PC1(CString strImageDirPath, BOOL bJudge, int algo, double xr, double xy);
	// hsj 2022-02-14
	BOOL write_result_Reference(int nJob);
	BOOL write_result_Fixture(int nJob);
	void sendRevisionData(double x, double y, double t,int job_id = 0 );
	void sendRevisionSecondData(double x, double y, double t, int job_id=0);
	void sendRevisionNozzleData(double x, double y, double t, double z, int job_id = 0);
	void sendLCheckData4Cam4Align(int job_id = 0);
	void sendTraceProfileData(int startAddress, int centerAddress,double pitch);
	void sendTraceProfileCircleData(int startAddress, int centerAddress, double pitch,int lineCount =-1);
	void sendTraceProfileTwo_CircleData(int startAddress, int centerAddress, double pitch);
	void sendTraceProfileNotchData(int startAddress, int centerAddress, double pitch);
	void sendRevisionOffsetData(double x, double y, double t);
	void sendMatchingRateData(int job_id = 0);
	void sendFilmFeedingOffsetData(double sdata,double tdata, int job_id = 0,int id=0);
	void fnProcessCheckOK(int nJobID);
	void fnProcessCheckNG(int nJobID);

	void OnUpdateTime();
	void modifySpecData(struct _ST_PLC_MODEL_DATA_ *pData);
	void  setViewerReferencePos();
	void SetLightBright(int nCtrl, int nChannel, int nValue);

	CString GetNumberBox(CString text, int nLength, double minValue, double maxValue);

	int ProcessPrealignINFRA(int nJob);
	int process1Cam1ShotPrealign(int algorithm);
	int process1Cam2ShotPrealign(int algorithm);
	int process2Cam2ShotPrealign(int algorithm);
	int process1Cam1ShotPrealignWithScratchInsp(int algorithm);
	int ProcessPrealign_4Cam_4Align(int nJob);
	int processAlign1Cam2PosPreAlign(int nJob);
	int processAlign1Cam4PosRobotAlign(int nJob);
	int processAlign1Cam4PosRobotAlign_Conveyer(int nJob);
	int processAlignExist(int nJob);
	int processPcbDistanceInsp(int nJob);
	int  processPcbDistanceInsp2(int nJob);
	int processAssembleInsp(int nJob);
	BOOL processCenterAlignELB(int algorithm);
	BOOL processNozzleAlign(int nJob);
	BOOL processScanInsp(int nJob);
	BOOL processFilmInsp(int nJob);
	BOOL processNozzleStatusInsp(int nJob);
	BOOL processCenterNozzleAlign(int nJob);
	BOOL processCenterNozzleGapMeasure(int nJob);
	BOOL processViewerSubInspection(int nJob);


	int algorithm_AlignExist(BYTE *pImage, int nAlgo, int nCam, BOOL bManual);
	int algorithm_exist_insp_assemble(BYTE *pImage, int nAlgo, int nCam,BOOL bManual);		// 조립기 용 유무 검사
	BOOL algorithm_assemble_Insp(BYTE *pImage, _stFindPattern *pPattern, int nInspMethod = 0, BOOL bSubDir = FALSE);
	BOOL algorithm_pcb_Insp(BYTE *pImage, _stFindPattern *pPattern, int nInspMethod = 0, BOOL bSubDir = FALSE);
	BOOL algorithm_AssembleInsp_CALC(BYTE *pImage, _stFindPattern *pPattern, int nInspMethod, BOOL bSubDir = FALSE);		//함수로 변환 210210
	BOOL algorithm_WetOut_Insp(BYTE* pImage, int nJob, int nCam, BOOL bManual=FALSE);
	BOOL algorithm_Film_insp_assemble(BYTE* pImage, int nJob, int nCam, CViewerEx* mViewer = NULL);
	BOOL algorithm_PF_Film_insp(BYTE* pImage, int nJob, int nCam);
	BOOL display_PF_Film_insp(BYTE* pImage, int nJob, int nCam, CViewerEx* mViewer, BOOL bAgain = 0);
	BOOL display_PF_Film_insp_save(BYTE* pImage, int nJob, int nCam, CViewerEx* mViewer, _stSaveImageInfo* pInfo);
	BOOL display_PF_Film_insp_save_capture(BYTE* pImage, int nJob, int nCam, CViewerEx* mViewer, _stSaveImageInfo* pInfo);  // KBJ 2022-07-18 필름검사 저장 뷰어 캡처로 수정
	BOOL find_pf_film_pattern(BYTE* pImage, int nJob, int nCam);
	BOOL find_pf_film_edge_pattern(BYTE* pImage, int nJob, int nCam);
	BOOL find_pf_panel_edge(BYTE* pImage, int nJob, int nCam, BOOL bUseManulMark = FALSE);
	BOOL find_pf_film_edge(BYTE* pImage, int nJob, int nCam, BOOL bUseManulMark = FALSE);
	BOOL find_pf_film_pattern2(BYTE* pImage, int nJob, int nCam);
	BOOL find_pf_panel_pattern(BYTE* pImage, int nJob, int nCam);
	BOOL find_pf_film_stagedge(BYTE* pImage, int nJob, int nCam,double cx,double cy,double a,double b,double &distance);
	BOOL find_panel_mark_edge_line(BYTE* pImage, int nJob, int nCam, double *PosX, double *PosY);
	BOOL calculate_film_distance(BYTE* pImage, int nJob, int nCam);
	BOOL judge_film_inspection(int nJob, int nCam);
	BOOL find_pattern_caliper(BYTE* pImg, int w, int h, int job, int cam, int pos, CFindInfo* pInfo = NULL);

	int Panel_length_check_process(int nJob,BOOL prealign_test);
	int Film_length_check_process(int nJob,BOOL prealign_test);
	
	void readLightCtrlInfo(struct stLIGHTCTRLINFO *pInfo);

	BOOL readGlassInfoData(CGlassInfo *glassInfo, CString strModelPath);
	BOOL saveGlassInfoData(CGlassInfo *glassInfo, CString strModelPath);

	BOOL readLightInfoData(int algo, CLightInfo *lightInfo, CString strModelPath);
	BOOL saveLightInfoData(int algo, CLightInfo *lightInfo, CString strModelPath);

	BOOL readMachineInfoData(int algo, CMachineInfo *machineInfo, CString strModelPath);
	BOOL saveMachineInfoData(int algo, CMachineInfo *machineInfo, CString strModelPath);

	BOOL readAlignInfoData(int nJob, CAlignInfo *alignInfo, CString strModelPath);
	BOOL saveAlignInfoData(int nJob, CAlignInfo *alignInfo, CString strModelPath, BOOL bSave = TRUE);

	BOOL readPanelExistInfoData(CPanelExistInfo *panelExistInfo, CString strModelPath);
	BOOL savePanelExistInfoData(CPanelExistInfo *panelExistInfo, CString strModelPath);

	// dh.jung 2021-08-02 add 
	BOOL readInspSpecPara(int nJob, CInspSpecPara* inspSpecPara, CString strModelPath);
	BOOL saveInspSpecPara(int nJob, CInspSpecPara* inspSpecPara, CString strModelPath);
	// end

	BOOL readInspSpecParaSameCenterItem(int nJob, CString strModelPath);	//lhj add 220503
	BOOL readAlignInfoDataSameCenterItem(int nJob, CString strModelPath);	//lhj add 220503

	//CModel &getModel() { return m_currModel; }

	CMachineSetting &GetMachine(int job_id) { return m_MachineSetting[job_id]; }
	CPatternMatching &GetMatching(int job_id) { return m_PatternMatching[job_id]; }
	CPrealign &GetPrealign(int job_id) { return m_Prealign[job_id]; }
	CInspSpecPara &GetInspSpecPara(int job_id) { return m_InspSpecPara[job_id]; }
	CPanelExistInspection &GetPanelExist() { return m_PanelExist; }

	LRESULT OnDumpFileMessage(WPARAM wParam, LPARAM lParam);
	void saveImageFiles(int nJob, int nCam);

	void CloseSockets();
	void initSocket();
	void ProcessReceivedDataNormal(TCP::cSocket::cMemory* pi_RecvMem);
	void ProcessReceivedDataPrefix(TCP::cSocket::cMemory* pi_RecvMem);
	void ProcessEvents();
	void SendPacketToServer(ToPacketData packetData);
	void AnalysisCommand(pToPacketData packet);
	void convertIP(CString strIP, int *nIP);
	static ULONG WINAPI ProcessEventThread(void* p_Param);
	CString GetErrMsg(DWORD u32_Error);

	BOOL calc_lcheck_1cam_1shot(int algo, BOOL prealign_test = FALSE);	// kbj 2022-02-05  
	BOOL calc_lcheck_1cam_1shot_2object(int algo, BOOL prealign_test = FALSE);	// kbj 2022-02-05  
	BOOL calc_lcheck_1cam_2pos(int algo, BOOL prealign_test = FALSE);	// yeol 20210602
	BOOL calc_lcheck_2cam_1pos(int algo, BOOL prealign_test = FALSE);
	BOOL calc_lcheck_4cam_1pos(int algo, BOOL prealign_test = FALSE);
	BOOL calc_lcheck_2cam_2pos(int algo, BOOL prealign_test = FALSE);

	void calc_sub_mark_offset_1cam_2pos(int algo);
	void calc_sub_mark_offset_2cam_1pos(int algo, int cam_pos_type);
	void calc_sub_mark_offset_4cam_1pos(int algo);

	BOOL UsePassword();

	double m_dist;
	double m_dist_T, m_dist_B, m_dist_R, m_dist_L;
	BOOL m_bUseSubInspCamView;
	BOOL m_bUseRAlignFlag;	// kbj flag

	BOOL PCB_Distance_Insp(BYTE *pImage, _stFindPattern *pPattern);
	void init_calib_data();
	void init_gui();
	void init_camera();
	void init_plc_connection();
	void init_manual_input_mark();
	void init_algorithm();
	void initPLC_Time();
	void LightUse(int nLightuse, int algo);
	void draw_line(cv::Mat& Img, cv::Scalar& color, sLine lineInfo, int nWidth, int nHeight, int nPenThickness, BOOL bDraw=TRUE);
	void draw_mark(cv::Mat& Img, cv::Scalar& color, int nPosx, int nPosy, int nPenLength, int nPenThickness, BOOL bDraw = TRUE);
	void draw_line(FakeDC* pDC, sLine lineInfo, int nHeight, int nWidth, BOOL bDraw = TRUE);
	void draw_line(CViewerEx* pView, COLORREF color, sLine lineInfo, int nHeight, int nWidth, BOOL bDraw = TRUE);
	void draw_mark(FakeDC* pDC, CPen* pPen, int nPosx, int nPosy, int nPenLength, BOOL bDraw = TRUE);
	void draw_mark(CViewerEx* pView, COLORREF color, int nPosx, int nPosy, int nFitS, int nOrgS, int nPenLength, BOOL bDraw = TRUE);
	void draw_text(CViewerEx* pView, CString strText, COLORREF color, int nFitS, int nOrgS, int x = 40, int y = 40, bool is_show_bkg = false);
	void draw_arrow(CViewerEx* pView, int direction, CString strText, COLORREF color, int start_x, int strat_y, int end_x, int end_y, int nText_FitS = 5, int nText_OrgS = 5);
	void draw_direction(CViewerEx* pView, int direction, int line_length, CString strText, COLORREF color, int start_x, int strat_y, int nText_FitS = 5, int nText_OrgS = 5);
	double calcIncludedAngle(sLine line_hori, sLine line_vert);
	double make_included_angle_to_matching(double angle, int nJob);

	void set_viewer_judge_ok(int nJob);
	void set_viewer_judge_ng(int nJob);
	void set_viewer_judge_infra(int nJob, BOOL bJudge);
	void set_viewer_judge_2cam_2shot_align(int nJob, BOOL bJudge);
	void set_viewer_judge_pcb_distance_insp(int nJob, BOOL bJudge);
	void set_viewer_judge_assemble_insp(int nJob, BOOL bJudge);
	void set_viewer_judge_scan_insp(int nJob, BOOL bJudge);
	void set_viewer_judge_scratch_insp(int nJob, BOOL bJudge);		// YCS 2022-11-18 스크래치 뷰어도 OK, NG 갱신하도록 추가
	
	CDlgMessage *m_pDlgMessage;
	BOOL simulration_pcb_insp(int nJob);
	BOOL simulration_2cam_1shot_align(int nJob);
	BOOL simulration_2cam_2shot_align(int nJob);
	BOOL simulration_assemble_insp(int nJob);
	BOOL simulration_4cam_1shot_align(int nJob);
	BOOL simulration_1cam_1shot_align(int nJob);
	BOOL simulration_1cam_2pos_align(int nJob);
	BOOL simulration_ELB_center_align(int nJob);
	BOOL simulration_1cam_1shot_align_with_scratch_insp(int nJob);


	class NOTCH_LINE_DATA notch_line_data;

	//int wait_flag_over_time(int wait_time, BOOL *pArrFlag, int start_1d_index = 0, int num_of_1d = 1, int widthStep = 1, int start_2d_index = 0, int num_of_2d = 1);
	int wait_flag_over_time(int wait_time, BOOL* pArrFlag, BOOL bResult, int start_1d_index = 0, int num_of_1d = 1, int widthStep = 1, int start_2d_index = 0, int num_of_2d = 1, BOOL bVisual = FALSE);

	afx_msg BOOL OnCopyData(CWnd* pWnd, COPYDATASTRUCT* pCopyDataStruct);

	std::vector<cv::Point2f> m_vtTraceData;
	std::vector<cv::Point2f> m_vtTraceDataGold;
	std::vector<cv::Point2f> m_vtTraceData_Fitting;
	std::vector<cv::Point2f> m_vtTraceData_Metal;
	std::vector<cv::Point2f> m_vtTraceData_MetalGold;
	NOTCH_WETOUT_DATA notch_wetout_data;

	double notch_left_distance_from_center;
	double notch_right_distance_from_center;

	sEllipse notch_ellipse[2];	// [down / up]

	void EllipseRot(CDC *pDC, sEllipse &ellipse);


	void read_trace_data(CString str_path);
	void read_metal_trace_data(CString str_path);
	void read_trace_mask_data(CString str_path);

	//KJH 2021-10-30 Save End Check 변수 추가
	//Expose 저장중에 카메라 Live시 프로그램 죽음
	BOOL m_bSaveFinishCheck;

	void initialize_job();

	void add_process_history(CString strText);
	void add_process_history_machine(CString strText);
	void single_grab(int nCam);
	void set_grab_end(int nCam, BOOL bSet) { m_bGrabEnd[nCam] = bSet; }
	void set_live_display(int nCam, BOOL bSet) { m_bLiveDisplay[nCam] = bSet; }
	BOOL get_grab_end(int nCam) { return m_bGrabEnd[nCam]; }

	void set_revision_data(int nJob, int nAxis, double value) { m_dbRevisionData[nJob][nAxis] = value; }
	void increase_seq_process(int nJob) { m_nSeqProcess[nJob]++; }
	void set_seq_process(int nJob, int nSeq) { m_nSeqProcess[nJob] = nSeq; }
	int get_seq_process(int nJob) { return m_nSeqProcess[nJob]; }
	void reset_grab_timeout_count(int nJob) { m_nGrabTimeOutCount[nJob] = 0; }
	int get_grab_timeout_count(int nJob) { return m_nGrabTimeOutCount[nJob]; }
	void increase_grab_timeout_count(int nJob) { m_nGrabTimeOutCount[nJob]++; }

	int m_nPrealignGrabRetry[MAX_JOB];
	void increase_prealign_grab_retry(int nJob) { m_nPrealignGrabRetry[nJob]++; }
	int get_prealign_grab_retry(int nJob) { return m_nPrealignGrabRetry[nJob]; }
	void reset_prealign_grab_retry(int nJob) { m_nPrealignGrabRetry[nJob] = 0; }
	BOOL write_NGresult_SummaryFile(CString serialNum, CString Msg, BOOL bjudge, int algorithm);
	BOOL write_Rotate_SummaryFile(CString serialNum, CString Msg, BOOL bjudge, int algorithm);

	BOOL Process_Live[MAX_JOB];
	BOOL auto_seq_test[MAX_JOB];
	BOOL each_job_method;
	int  m_nNozzleSideCamMode;
	HANDLE m_hDummyEvent;
	HANDLE m_hInspEndEvent[MAX_CAMERA];
	bool m_bSubInspJudge[MAX_JOB];

	int copy_caliper_data(int src_cam, int src_pos, int src_line, int dst_job, int dst_cam, int dst_pos, int dst_line);
	void display_copy_caliper_info();
	void init_copy_caliper_info();

	BOOL LightControllerTurnOnOff(int nJob, int nCam, int nIndex = 0, BOOL bTotal = FALSE, BOOL OnOff = FALSE, BOOL SubLightOff = FALSE);
	BOOL InitProcessStart_Cam();
	void Save_Result_Image_Center_Nozzle_Align(BOOL bJudge, int algo, BOOL bRetryEnd);
	BOOL Write_Result_Align_Data_Center_Align_ELB(CString serialNum, BOOL bJudge, int algo);
	
	// hsj 2022-01-09 결과창 헤더 초기화 위치변경으로 인한..
	BOOL m_bResultInit[MAX_JOB];

	// hsj 2022-02-08 필름검사 L check 기능 추가
	int nFilmInspect[MAX_JOB][2];
	double m_dLcheckCurrentValue[MAX_JOB];

	//hsj 2022-02-14 reference 변경 유무
	BOOL m_bChangeReference[MAX_JOB][NUM_POS];
	BOOL m_bChangeFixture[MAX_JOB][NUM_POS];
	
	CString m_strRefDate_X[MAX_JOB][NUM_POS];
	CString m_strRefDate_Y[MAX_JOB][NUM_POS];
	CString m_strRefDate_T[MAX_JOB][NUM_POS];
	CString m_strFixtureDate_X[MAX_JOB][NUM_POS];
	CString m_strFixtureDate_Y[MAX_JOB][NUM_POS];
	CString m_strFixtureDate_T[MAX_JOB][NUM_POS];

	COffsetDlg* m_pOffsetDlg;
	CDlgList* m_pListDlg;
	CDlgSelectJob* m_pSelectDlg;

	//KJH 2022-03-12 PF INSP FDC 보고 추가
	void SendFDCValue_PFINSP(int nJob, int nCam);
	//KJH 2022-03-16 Act,Seq Time Display 추가
	void draw_Calc_Time(int nJob, int dir, CString strTime, int xoffset, int yoffset);
	void draw_NozzleY_Distance(int threadID);

	int process1Cam1ShotFilmalign(int algorithm);
	BOOL simulration_1cam_1shot_film_align(int nJob);
	double reCalculateMarkYpos(BYTE *src, int w, int h, double refx, double refy);
	double reCalculateMarkZGapYpos(BYTE* src, int w, int h, double refx, double refy);
	double reCalculateMarkZGapYpos2(BYTE* src, int w, int h, double refx, double refy); // KMB 2022-09-16 
	void reCalculateMarkZGapYpos3(int nJob, BYTE* src, int w, int h, double *refx, double *refy); // KMB 2022-09-16 

	//2022.06.18 ksm 전체 Reference 추가 시 Flag
	BOOL m_bAllReferflag;
	void AllReferSaveFlag(BOOL bSaveFlag = FALSE) { m_bAllReferflag = bSaveFlag; };
	BOOL getAllReferFlag() { return m_bAllReferflag; };

	BOOL Load_Align_PanelImage(int nJob, CViewerEx* pViewer, CString strPanelID, BOOL bJudge ,int nRealCam);

	//KJH
	CDlgAlarm* m_pDlgAlarm[MAX_ALARM_DLG];
	void replaceWindowPos(CDialog* pDlg, int i);
	void setAlarmMessage(int nUnit, BOOL bShow, CString strText = "");
	void Camera_Grab(int real_cam);
	BOOL Exception_Film_Insp_Judge(int nJob,CViewerEx* pViewer);
	int m_nExceptionJudgeCount;

	struct _ELB_MODEL_PROCESS_ADDRES_ model_process_addres;
	void init_model_process_addr();

	void ScratchInsp(int nJob);
	BOOL Film_Align_Compare_Angle(BYTE* pImage, int nJob, int nCam); // YCS 2022-11-09 필름얼라인용 필름 상부 라인 찾기 함수
};
#endif
