#pragma once



class CLET_AlignClientDlg;
class CPaneSimulation : public CFormView
{
	DECLARE_DYNCREATE(CPaneSimulation)

protected:
	CPaneSimulation();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~CPaneSimulation();

public:
	enum { IDD = IDD_PANE_SIMULATION };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

	CLET_AlignClientDlg *m_pMain;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);
	virtual void OnInitialUpdate();
	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//afx_msg void OnBnClickedBtnSimulation();
	//afx_msg void OnBnClickedBtnSimulationStop();
	//afx_msg void OnBnClickedBtnExit();

	CRect m_rcForm;
	HBRUSH m_hbrBkg;
	BOOL m_bSimulStopFlag;
	BOOL m_bUseSimulMessage;
	std::vector < CString > m_vtDirPath;
	std::vector < BOOL > m_vtSelectList;
	int m_nCurrent_list;

	CGridCtrl m_grid_list;
	CButtonEx m_btnLive;
	CButtonEx m_btnAppendList;
	CButtonEx m_btnDeleteList;
	CButtonEx m_btnSimulationStart;
	CButtonEx m_btnReverseSelect;

	CLabelEx m_sttJob;
	CComboBox m_cmbSelectJob;

	int m_nJob;
	BOOL m_bUseLive;

	void MainButtonInit(CButtonEx *pbutton, int nID = -1, int size = 15);
	void InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color);
	void OnPaint();

	void set_simulation_start_stop(BOOL bSimulation);
	void update_frame_dialog();
	BOOL get_directory_path(std::vector < CString > &vtFilePath, int nJob);
	BOOL get_directory_path(std::vector < CString > &vtFilePath, CString strFilePath, int nJob);
	void get_ID_from_loadimage(int nJob, CString strFilePath, CString *strID);
	BOOL put_image_in_buffer(int nJob, int nCam, CString strFilePath, int nPos = 0);
	friend UINT Thread_put_image_in_buffer(void *pParam);

	void simulation_1cam_1shot_align(int nJob);
	void simulation_2cam_1shot_align(int nJob);
	void simulation_2cam_2shot_align(int nJob);
	void simulation_4cam_1shot_align(int nJob);
	void simulation_exist_isnp(int nJob);
	void simulation_cam_position_insp(int nJob);
	void simulation_Elb_centerAlign(int nJob);
	void simulation_nozzle_isnp(int nJob);
	void simulation_Scan_isnp(int nJob);
	void simulation_Center_NozzleAlign(int nJob);
	void simulation_zgap_insp(int nJob);
	void simulation_zgap_insp_Live(int nJob);

	BOOL get_Image_path_1cam_1shot(int nJob, CString strFilePath, std::vector < CString >& vtImagePath, int nExpectedCount = 1);
	BOOL get_Image_path_elb_center_align(int nJob, CString strFilePath, std::vector < CString >& vtImagePath, int nExpectedCount = 1);
	BOOL get_Image_path_2cam_1shot_align(int nJob, CString strFilePath, std::vector < CString > &vtImagePath, int nExpectedCount = 2);
	BOOL get_Image_path_2cam_2shot_align(int nJob, CString strFilePath, std::vector < CString > &vtImagePath, int nExpectedCount = 4);
	BOOL get_Image_path_4cam_1shot_align(int nJob, CString strFilePath, std::vector < CString > &vtImagePath, int nExpectedCount);
	BOOL get_Image_path_exist_insp(int nJob, CString strFilePath, std::vector < CString > &vtImagePath, int nExpectedCount);
	
	int SimulationMesaage(int nType, CString strText1, CString strText2);
	int get_job_from_img(CString strFilePath);

	afx_msg void OnBnClickedBtnLiveSimulation();
	afx_msg void OnBnClickedBtnAppendList();
	afx_msg void OnBnClickedBtnDeleteList();
	afx_msg void OnSelchangeComboJob();
	afx_msg void OnBnClickedBtnSimulationStart();
	afx_msg void OnBnClickedBtnReverseSelcet();

	void simulation_live(int nJob);
	void simulation_live_default(int nJob);
	void update_list();
	int current_list(int pos);
	void init_grid_list(CGridCtrl& grid, int col, int row);
};
