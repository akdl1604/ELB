#pragma once


// DlgMarkCopy 대화 상자

class DlgMarkCopy : public CDialogEx
{
	DECLARE_DYNAMIC(DlgMarkCopy)

public:
	DlgMarkCopy(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~DlgMarkCopy();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_MARK_COPY };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	CLET_AlignClientDlg* m_pMain;

	afx_msg void OnBnClickedBtnMarkCopy();
	CComboBox m_cbSrcJob;
	CComboBox m_cbSrcCam;
	CComboBox m_cbSrcPos;
	CComboBox m_cbSrcIndex;
	CComboBox m_cbDstJob;
	CComboBox m_cbDstCam;
	CComboBox m_cbDstPos;
	CComboBox m_cbDstIndex;
	afx_msg void OnCbnSelchangeCbSrcJob();
	virtual BOOL OnInitDialog();
	afx_msg void OnCbnSelchangeCbSrcCam();
	afx_msg void OnCbnSelchangeCbDstJob();
	afx_msg void OnCbnSelchangeCbDstCam();
};
