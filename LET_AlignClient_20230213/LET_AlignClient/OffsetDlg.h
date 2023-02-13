#pragma once


// CCramerSRuleDlg 대화 상자입니다.

class COffsetDlg : public CDialogEx
{
	DECLARE_DYNAMIC(COffsetDlg)

public:
	COffsetDlg(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~COffsetDlg();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DIALOG_OFFSET };

	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedButtonSaveOffset();
	afx_msg void OnBnClickedButtonSetOffset();

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	DECLARE_MESSAGE_MAP()
public:
	CLET_AlignClientDlg* m_pMain;

	CEditEx m_edtOffsetX;
	CEditEx m_edtOffsetY;

	CString m_str_INI_path;
	CString m_str_section;

	int m_nJob;
	int m_nTarget;
	int m_nCam;
	int m_nPos;

	BOOL m_bApply_Offset;
	double m_dOrigin_x, m_dOrigin_y;
	double m_dTarget_x, m_dTarget_y;
	double m_dOffset_x, m_dOffset_y;
	double m_dResult_x, m_dResult_y;

public:
	void Set_Offset_Target(int nTarget, int nJob, int nCam, int nPos);
	void Set_Result_Offset();
	BOOL Check_ApplyData();
	void UpdateData();
	void Get_RotateCenter_Offset();
	void Save_RotateCenter_Offset();
	afx_msg void OnBnClickedButtonClear();
};
