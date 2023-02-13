#pragma once

/*
*  enum ManualReason
    {
        Score = 0,
        LCheck = 1,
        Blur = 2,
        Angle = 3,
        DLFail = 4
    }
*/

// CVAT_SimulatorDlg 대화 상자

class CVAT_SimulatorDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CVAT_SimulatorDlg)

public:
	CVAT_SimulatorDlg(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CVAT_SimulatorDlg();

	CLET_AlignClientDlg* m_pMain;
	HBRUSH m_hbrBkg;

	CLabelEx m_LbViewTitle;
	CLabelEx m_LbConnectStatus;
	CLabelEx m_LbModelTitle;
	CLabelEx m_LbPosNamelTitle;
	CLabelEx m_LbEqpNameTitle;
	CLabelEx m_LbDllTitle;
	CLabelEx m_LbMcTitle;
	CLabelEx m_LbAttTitle;

	CButtonEx m_btnConnect;
	CButtonEx m_btnDisconnect;
	CButtonEx m_btnReqdlagent;
	CButtonEx m_btnReqmodeldataCopy;
	CButtonEx m_btnReportmanualRst;
	CButtonEx m_btnReportlogicRst;
	CButtonEx m_btnRequpdateAtt;
	CButtonEx m_btnReportAllrst;
	CButtonEx m_btnGeteqpList;
	CButtonEx m_btnGetposList;
	CButtonEx m_btnMonitorLog;
	CButtonEx m_btnImageLoad;
	CButtonEx m_btnClearLog;
	CButtonEx m_btnCancel;

	CEdit m_edtModel;
	CEdit m_edtDLTimeOut;
	CEdit m_edtMCTimeOut;
	CEdit m_edtATTTimeOut;

	CComboBox m_cmbSelectPosName;
	CComboBox m_cmbSelectEqpName;

	CWinThread* m_pThread;
	cv::Mat m_pImage;
	vector<CString>		m_vEQPList;		// 설비 이름
	vector<CString>		m_vPosNameList;		// POS 이름

	void InitEQPList();
	void InitPosList();

	static UINT ProgressThread(LPVOID lpParam);		// 프로그레스바 스레드
// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_SDV_VAT_SIMULATOR };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

	afx_msg void OnBnClickedButtonConnect();
	afx_msg void OnBnClickedButtonDisconnect();
	afx_msg void OnBnClickedButtonReqdlagent();
	afx_msg void OnBnClickedButtonReqmodeldataCopy();
	afx_msg void OnBnClickedButtonReportmanualRst();
	afx_msg void OnBnClickedButtonReportlogicRst();
	afx_msg void OnBnClickedButtonRequpdateAtt();
	afx_msg void OnBnClickedButtonReportAllrst();
	afx_msg void OnBnClickedButtonGeteqpList();
	afx_msg void OnBnClickedButtonGetposList();
	afx_msg void OnBnClickedButtonMonitorLog();
	afx_msg void OnBnClickedButtonImageLoad();
	afx_msg void OnBnClickedButtonClearLog();

	virtual BOOL OnInitDialog();
	
	void InitTitle(CLabelEx* pTitle, CString str, float size, COLORREF color);
	void MainButtonInit(CButtonEx* pbutton, int size = 15);
	afx_msg void OnEnSetfocusEditModel();
	afx_msg void OnEnSetfocusEditDltimeout();
	afx_msg void OnEnSetfocusEditMctimeout();
	afx_msg void OnEnSetfocusEditAtttimeout();
	CListBox m_listLog;
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	afx_msg void OnBnClickedButtonCopyCancle();
	afx_msg void OnClose();

	void fnStartVMS_log_first();
	void fnUpdate_mark_parameter_log(int camNo);
	void fnWrite_camera_info(int camNo);
	void fnWrite_pc_status();
	//void fnSend_process_image();
};
