#pragma once

// DlgINISetting 대화 상자
class CLET_AlignClientDlg;


/*class CJobLight {
public:
	CJobLight() {
		vJobLightCtrl.clear();
	}
	std::vector<int> vJobLightCtrl;
};

class CJob {
public:
	std::vector<CJobLight> Clight;
};*/

class DlgINISetting : public CDialogEx
{
	DECLARE_DYNAMIC(DlgINISetting)

public:
	DlgINISetting(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~DlgINISetting();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_INI_SETTING };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()


public:
	virtual BOOL OnInitDialog();

	

private:
	////////////////////////////Viewer ini////////////////////////////////
	int m_nViewCont;
	CString m_strName[MAX_VIEWER];
	int m_nWidth[MAX_VIEWER];
	int m_nHeight[MAX_VIEWER];
	int m_nPopup[MAX_VIEWER];
	double m_dResolutionX[MAX_VIEWER];
	double m_dResolutionY[MAX_VIEWER];
	int m_nNumOfViewer;
	int m_nViewerCntX;
	int m_nViewerCntY;
	int m_nViewerPos[MAX_VIEWER];
	int m_nCameraIdx[MAX_CAMERA];
	int m_nViewNumPos[MAX_CAMERA];
	////////////////////////////Job ini////////////////////////////////
	CString m_strJobName[MAX_CAMERA];
	CString m_stPosName[MAX_CAMERA][MAX_CAMERA][MAX_CAMERA];
	CString m_stPosIndex[MAX_CAMERA][MAX_CAMERA][MAX_CAMERA];
	int m_nJobCount;
	int m_nJobMethod[MAX_CAMERA];
	int m_nJobCamCount[MAX_CAMERA];
	int m_nJobViewerIndex[MAX_CAMERA][MAX_CAMERA];
	int m_nJobPosition[MAX_CAMERA];
	int m_nJobCamIndex[MAX_CAMERA][MAX_CAMERA];
	int m_nJobChangeViewer[MAX_CAMERA];
	int m_nJobViewerCount[MAX_CAMERA];
	int m_nJobReadBitStart[MAX_CAMERA];
	int m_nJobWriteBitStart[MAX_CAMERA];
	int m_nJobReadWordStart[MAX_CAMERA];
	int m_nJobWriteWordStart[MAX_CAMERA];
	int m_nJobUseMainObjectID[MAX_CAMERA];
	int m_nJobMainObjectIdStart[MAX_CAMERA];
	int m_nJobUseSubObjectID[MAX_CAMERA];
	int m_nJobSubObjectIdStart[MAX_CAMERA];

	CString m_stLightChannelName[MAX_CAMERA][MAX_CAMERA][MAX_CAMERA];
	CString m_stLightIndexName[MAX_CAMERA][MAX_CAMERA][MAX_LIGHT_INDEX];
	CString m_stCaliperName[MAX_CAMERA][MAX_CAMERA][MAX_CAMERA][MAX_CALIPER];
	int m_nNumOfUsingLightCount[MAX_CAMERA][MAX_CAMERA];
	int m_nLightControlId[MAX_CAMERA][MAX_CAMERA][MAX_CAMERA];
	int m_nLightChannelId[MAX_CAMERA][MAX_CAMERA][MAX_CAMERA];
	int m_nMainViewCountX[MAX_CAMERA];
	int m_nMainViewCountY[MAX_CAMERA];
	int m_nMainViewIndex[MAX_CAMERA][MAX_CAMERA];
	int m_nMachineViewCountX[MAX_CAMERA];
	int m_nMachineViewCountY[MAX_CAMERA];
	int m_nMachineViewIndex[MAX_CAMERA][MAX_CAMERA];
	////////////////////////////Camera ini////////////////////////////////
	CString m_strCamName[MAX_CAMERA];
	CString m_nCamSerial[MAX_CAMERA];
	int m_nCamWidth[MAX_CAMERA];
	int m_nCamHeight[MAX_CAMERA];
	int m_nCamDepth[MAX_CAMERA];
	int m_nCamSize[MAX_CAMERA];
	int m_nFlipDir[MAX_CAMERA];
	//////////////////////////LET_AlignClient////////////////////////////////
	int m_nNumOfLightControl;
	int m_nLightType[MAX_LIGHT_CTRL];
	int m_nLightPort[MAX_LIGHT_CTRL];
	int m_nLightBaudrate[MAX_LIGHT_CTRL];
	int m_nLightSockPort[MAX_LIGHT_CTRL];
	CString m_strLightSockIp[MAX_LIGHT_CTRL];
	CString m_stLightName[MAX_LIGHT_CTRL];
	CString m_stClientName;
	CString m_strPlcIp;
	CString m_stFolderName;
	int m_nDummyStartX;
	int m_nDummyStartY;
	int m_nDummyInspHeight;
	BOOL m_bUseElbNozzleToPaneldistCheck;
	int m_nClientId;
	int m_nPlcPort;
	////////////////////////////PLC Adress////////////////////////////////////
	int m_nStartReadBitAddress;
	int m_nStartWriteBitAddress;
	int m_nStartReadWordAddress;
	int m_nStartWriteWordAddress;
	int m_nSizeReadBitAddress;
	int m_nSizeReadWordAddress;
	int m_nSizeWriteBitAddress;
	int m_nSizeWriteWordAddress;
	//////////////////////////////////////////////////////////////////////////////////////

	int m_nNumOfView;
	void init_grid_view_setting(CGridCtrl &grid, int col, int row);

	void draw_grid_view_name(CGridCtrl &grid, int col, int row);
	void draw_grid_job_light_data(CGridCtrl &grid, int col, int row);
	void draw_grid_plc_data(CGridCtrl &grid, int col, int row);
	
	void draw_grid_job(CGridCtrl &grid, int col, int row);
	void draw_grid_job_cam(CGridCtrl &grid, int col, int row);
	void draw_grid_job_view_viewer(CGridCtrl &grid, int col, int row);
	void draw_grid_job_index_name_viewer(CGridCtrl &grid, int col, int row);
	void draw_grid_job_caliper_viewer(CGridCtrl &grid, int col, int row);
	void draw_grid_job_machine_viewer(CGridCtrl &grid, int col, int row);
	void draw_grid_job_main_viewer(CGridCtrl &grid, int col, int row);
	void draw_grid_position_viewer(CGridCtrl &grid, int col, int row);
	void draw_grid_camera_viewer(CGridCtrl &grid, int col, int row);
	void draw_grid_light_control_viewer(CGridCtrl &grid, int col, int row);
	void draw_grid_server_viewer(CGridCtrl &grid, int col, int row);
	void draw_grid_plc_address_viewer(CGridCtrl &grid, int col, int row);

	void save_view_config();
	void save_job_config();
	void save_camera_config();
	void save_client_config();
	void save_plc_address_config();
	
	void read_view_config();
	void read_job_config();
	void read_camera_config();
	void read_client_config();
	void read_plc_address_config();

	void InitINIData();
	void InitCameraData();

public:
	CLET_AlignClientDlg *m_pMain;
	
	CListBox m_listNetworkAdapter;
	CComboBox m_cbSelectViewer;
	vector<string> m_CameraSerial;
	vector<string> m_CameraName;

	afx_msg void OnBnClickedBtnIniSettingSave();
	afx_msg void OnCbnSelchangeCbSelectViewer();
	CGridCtrl m_grid_select_viewer;
	CGridCtrl m_grid_job_light_viewer;
	CGridCtrl m_grid_plc_data;
	CGridCtrl m_grid_job_viewer;
	CGridCtrl m_grid_job_cam_viewer;
	CGridCtrl m_grid_job_view_viewer;
	CGridCtrl m_grid_job_machine_viewer;
	CGridCtrl m_grid_job_main_viewer;
	CGridCtrl m_grid_camera_viewer;
	CGridCtrl m_grid_light_control_viewer;
	CGridCtrl m_grid_server_viewer;
	CGridCtrl m_grid_light_index_name_viewer;
	CGridCtrl m_grid_caliper_viewer;
	CGridCtrl m_grid_position_viewer;
	CGridCtrl m_grid_plc_address_viewer;

	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	afx_msg LRESULT OnViewControl(WPARAM wParam, LPARAM lParam);

	void fnMSG_INI_JOB_VIEWER(LPARAM lParam);
	void fnMSG_INI_JOB_POSITION(LPARAM lParam);
	void fnMSG_INI_JOB_VIEW_VIEWER(LPARAM lParam);
	void fnMSG_INI_JOB_MAIN_VIEWER(LPARAM lParam);
	void fnMSG_INI_JOB_MACHINE_VIEWER(LPARAM lParam);
	void fnMSG_INI_JOB_CAM_VIEWER(LPARAM lParam);
	void fnMSG_INI_JOB_CALIPER(LPARAM lParam);
	void fnMSG_INI_SETTING_LIGHT(LPARAM lParam);
	void fnMSG_INI_JOB_LIGHT_INDEX_NAME(LPARAM lParam);
	void fnMSG_INI_SETTING_PLC(LPARAM lParam);
	void fnMSG_INI_SETTING_VIEWER(LPARAM lParam);
	void fnMSG_INI_CAMERA_VIEWER(LPARAM lParam);
	void fnMSG_INI_SERVER_VIEWER(LPARAM lParam);
	void fnMSG_INI_LIGHT_CONTROL_VIEWER(LPARAM lParam);
	void fnMSG_INI_PLC_ADDRESS_VIEWER(LPARAM lParam);

	CEdit m_edt_jobcount;
	
	afx_msg void OnBnClickedBtnLightControlModify();
	afx_msg void OnBnClickedBtnViewerCountModifyOk();
	
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	CComboBox m_cb_select_job;
	
	afx_msg void OnBnClickedBtnJobCountModifyOk();
	afx_msg void OnCbnSelchangeCbSelectJob();

	CComboBox m_cbSelectLightCam;
	afx_msg void OnBnClickedBtnCameraCountModifyOk();

	int m_nNumOfCamera;
	afx_msg void OnCbnSelchangeCbSelectCamera();
	CComboBox m_cbSelectCamera;
	CComboBox m_cbSelectPos;
	afx_msg void OnCbnSelchangeCbSelectPos();
	afx_msg void OnBnClickedBtnPosModifyOk();
	afx_msg void OnBnClickedBtnJobCameraCountModifyOk();
	afx_msg void OnBnClickedBtnMachineCountModifyOk();

	void JobDataInit();
	afx_msg void OnBnClickedBtnMainCountModifyOk();

	void JobAll();
	void JobNotAll();
	afx_msg void OnCbnSelchangeCbSelectLightCamera();
	afx_msg void OnBnClickedBtnJobUseLightModifyOk();
	void ModifyJobCamCount(int nSel,int nCam);
	afx_msg void OnBnClickedBtnIniExit();

	std::vector<int> vJobCount;
	//std::vector<CJob> Cjob;
    int GetCameraList(vector<string> &serial, vector<string> &name);
	int GetIpConfigCameraList();
	void getAdapterList(); // 20211227
	bool runCmdWindow(unsigned long index, bool benable);

    afx_msg void OnBnClickedBtnIniCamMatchig();
	afx_msg void OnDblclkListNetwork();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnCbnSelchangeCbNumOfUsingCamera4();
};


