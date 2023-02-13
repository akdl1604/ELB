#pragma once
class CLET_AlignClientDlg;
class CPrealign;
class CMachineSetting;
class CModel;
class CPatternMatching;

class CJob_1Cam1ShotAlign
{
public:
	CJob_1Cam1ShotAlign();
	~CJob_1Cam1ShotAlign();
private:
	int m_nNumPos;

	int m_calib_seq;
	int m_calib_timeout_cnt;
	int m_calib_cnt_x;
	int m_calib_cnt_y;

	CPrealign *pPrealign;
	CMachineSetting *pMachine;
	CModel *pModel;
	CPatternMatching *pMatching;
	CJobInfo *pJob;
	CJobResult *pResult;

	int m_nAutoSeq;
	int m_nJobID;
	BOOL is_seq_start;
public:
	CLET_AlignClientDlg *m_pMain;
	double m_dPanelLength;
	double m_dFilmLength;

	void init_job(int nJob);
	int do_auto_seq();
	int do_auto_seq_plc();
	int do_calibation_seq();
	
	int grab_start(int next_seq, BOOL is_auto);
	int grab_complete_check(int next_seq, BOOL is_auto);
	BOOL find_pattern(int next_seq, BOOL is_auto);
	int grab_retry_process(int return_seq, int manual_input_seq);
	int calculate_revision_data();
	int length_check_process(BOOL prealign_test);
	int included_angle_check_process();
	int send_revision_data();
	int manual_input_mark_process(int return_seq);

	int calibration_enable_check(int next_seq);
	int calibration_mode_on(int next_seq);
	int wait_calibration_mode_on_ack(int next_seq);
	int memory_calibration_origin(int next_seq);
	int reset_calibration_data(int next_seq);
	int calibration_move_request(int next_seq);
	int wait_calibration_move_ack(int next_seq);
	int add_calibration_data(int next_seq);
	BOOL calculate_calibration_data();

	void simulation();
	void draw_result(int nCam, int nPos, CViewerEx *pViewer, BOOL bErase);
	void draw_result_Matching(int nCam, int nPos, CViewerEx *pViewer, BOOL bErase);
	void draw_result_RANSAC(int nCam, int nPos, CViewerEx *pViewer, BOOL bErase);
	void draw_result_Caliper(int nCam, int nPos, CViewerEx *pViewer, BOOL bErase);


	void draw_mark(FakeDC *pDC, double posX, double posY, int size);
	void draw_roi(FakeDC*pDC, CRect rectROI);

	void save_image(_stSaveImageInfo *pInfo);
	void write_process_log(BOOL bJudge);
	void save_result_image(BOOL bJudge, BOOL bRetryEnd);

	friend UINT thread_find_pattern_1cam_1shot_align(void *pParam);		// 17.12.30 jmLee		Auto 모드일때 라이브 처리 하도록..
	int find_pattern_caliper(BYTE *pImage, int nCam, int nPos, int W, int H);
	friend UINT thread_1cam_1shot_align(void *pParam);
	void run_thread(CDialog *pDlg, int job_id);
};
