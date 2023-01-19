#pragma once


// CSheetCaliperParam 대화 상자입니다.
enum _CALIPER_RESOUCE_ {
	RC_NUM_OF_CALIPER = 0,
	RC_PROJECTION_LENGTH,
	RC_SEARCH_LENGTH,
	RC_THRESHOLD,
	RC_HALF_FILTER_SIZE,
	RC_NUM_OF_IGNORE,
	RC_FIXTURE_X,
	RC_FIXTURE_Y,
	RC_FIXTURE_T,
	RC_ORG_START_X,
	RC_ORG_START_Y,
	RC_ORG_END_X,
	RC_ORG_END_Y,
	NUM_OF_CALIPER_RESOURCE,
};

class CCaliper;
class CDlgCaliper;
class CSheetCaliperParam: public CDialogEx
{
	DECLARE_DYNAMIC(CSheetCaliperParam)

public:
	CSheetCaliperParam(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CSheetCaliperParam();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_SHEET_CALIPER_PARAM };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	CCaliper *m_pCaliper;
	CDlgCaliper *m_pDlgCaliper;

	CComboBox m_cbSelectLine;
	CComboBox m_cbCopyJob;
	CComboBox m_cbCopyCam;
	CComboBox m_cbCopyPos;
	CComboBox m_cbCopyLine;
	CRect m_rcCaliperEditBox[NUM_OF_CALIPER_RESOURCE];
	int m_nCaliperResouceID[NUM_OF_CALIPER_RESOURCE];

	
	void updateCoordinates();
	void updateUIFromData();
	void updateDataFromUI();

	void copy_caliper_data(int src_obj, int src_pos, int src_line, int dst_job, int dst_cam, int dst_pos, int dst_line);
	
	afx_msg LRESULT OnViewControl(WPARAM wParam, LPARAM lParam);
	afx_msg void OnEnSetfocusEditSearchLength();
	afx_msg void OnBnClickedBtnCaliperDirectionSwap();
	afx_msg void OnBnClickedBtnCaliperFind();
	afx_msg void OnCbnSelchangeComboCaliperSearchType();
	afx_msg void OnBnClickedCheckCircleFitting();	
	afx_msg void OnCbnSelchangeComboFittingMethod();
	afx_msg void OnEnKillfocusEditOffsetX();
	afx_msg void OnEnKillfocusEditOffsetY();
	afx_msg void OnEnKillfocusEditOffsetT();
	virtual BOOL OnInitDialog();
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	afx_msg void OnCbnSelchangeComboSelectLine();
	afx_msg void OnBnClickedBtnCaliperInfoSave();
	afx_msg void OnCbnSelchangeComboEdgePolarity();
	afx_msg void OnCbnSelchangeComboSearchType();
	afx_msg void OnBnClickedBtnCaliperCopy();
	afx_msg void OnCbnSelchangeComboCopyJob();
	afx_msg void OnBnClickedBtnCaliperInfoClose();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
};
