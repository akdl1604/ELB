#pragma once
#include "afxwin.h"
#include "NMPNetworkChange.h"

const int GRID_ROW_JOB_LIST = 10;
const int GRID_ROW_PARAMETER = 20;

class CLET_AlignClientDlg;
class CFormSystemOptionView : public CFormView
{
	DECLARE_DYNCREATE(CFormSystemOptionView)

protected:
	CFormSystemOptionView();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~CFormSystemOptionView();

public:
	enum { IDD = IDD_FORM_SYSTEM_OPTION };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

	CLET_AlignClientDlg *m_pMain;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	int m_nCurrentSelectJob;	//0 : common, n : n번 Job ...
	CNMPNetworkChange _netWork;

	HBRUSH m_hbrBkg;

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	//afx_msg LRESULT OnViewControl(WPARAM wParam, LPARAM lParam);

	virtual void OnInitialUpdate();
	void UpdateFrameDialog();
	CLabelEx m_LbSelectedJobTitle;
	CLabelEx m_LbSelectedJob;
	CLabelEx m_stIpAddress;
	CLabelEx m_stSubnetMask;
	CLabelEx m_stGateWay;

	CButtonEx m_btnJobDataModify;
	CButtonEx m_btnJobDataSave;
	CButtonEx m_btnChangeIP;
	CGridCtrl m_GridJobList;
	CGridCtrl m_GridSystemOption;

	BOOL m_bUIEnable;

	void MainButtonInit(CButtonEx *pbutton, int size = 15);
	void InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color);

	void Init_grid(CGridCtrl &grid, int col, int row);
	void Init_grid_job_list(CGridCtrl &grid, int col, int row);
	void Init_grid_system_option(CGridCtrl &grid, int col, int row);
	void InitNetWorkCard();

	void get_job_list();
	void get_system_option();
	void get_system_option_common();
	void get_system_option_infra();
	void get_system_option_ELB();
	void get_system_option_assemble_insp();
	void get_system_option_pcb_bending_insp();

	void set_system_option(int nJob, int row);
	void set_system_option_common(int row);
	void set_system_option_infra(int nJob, int row);
	void set_system_option_ELB(int nJob, int row);
	void set_system_option_assemble_insp(int nJob, int row);
	void set_system_option_pcb_bending_insp(int nJob, int row);

	void save_system_option_job_ini(int nJob);
	void save_system_option_common_ini();

	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	void sign_selected_job();
	void setChangeValue(int n);
	void enableUI(BOOL bEanble);

	afx_msg void OnBnClickedBtnJobDataModify();
	afx_msg void OnBnClickedBtnJobDataSave();	
	afx_msg void OnBnClickedButtonChangeIp();
	afx_msg void OnCbnSelchangeComboAdaptList();
	afx_msg void OnCbnSelchangeComboAdaptName();
	afx_msg void OnEnSetfocusEditIpAddress();
	afx_msg void OnEnSetfocusEditSubnetMask();
	afx_msg void OnEnSetfocusEditGateWay();
};

