#pragma once

#include "EditEx.h"

#include "UI_Control/tinyxml2.h"


// CImageProcessingSimulator 대화 상자
class CLET_AlignClientDlg;
class CImageProcessingSimulator : public CDialogEx
{
	DECLARE_DYNAMIC(CImageProcessingSimulator)

public:
	CImageProcessingSimulator(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~CImageProcessingSimulator();

	CLET_AlignClientDlg* m_pMain;
	cv::Mat* m_srcImg;
	CViewerEx* m_pView;

	HBRUSH m_hbrBkg;

	CLabelEx m_staticScriptList;
	CLabelEx m_staticScriptCommand;
	CLabelEx m_staticExplanation;
	CLabelEx m_staticHistory;
	CButtonEx m_btnProcessing;
	CButtonEx m_btnProcessingClear;
	CButtonEx m_btnProcessingGet;
	CButtonEx m_btnDeleteFile;
	CButtonEx m_btnImgProcAdd;
	CButtonEx m_btnProcessingAllClear;

	CListBox m_listCommandScriptList;
	CListBox m_listCommandHistorytList;
	CListBox m_ListScriptRun;
	CEditEx m_editCommand;
	//CEditEx m_editExplanation;
	CString m_listCmd;

	int m_nItemCount;

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_IMAGE_PROCESSING };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButtonProcessing();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedButtonProcessingClear();
	afx_msg void OnLbnDblclkListScriptList();
	afx_msg void OnBnClickedButtonProcessingGet();
	afx_msg void OnLbnDblclkListScriptHistory();

	void InitTitle(CLabelEx* pTitle, CString str, float size, COLORREF color);
	void MainButtonInit(CButtonEx* pbutton, int size = 15);
	void EditButtonInit(CEditEx* pbutton, int size, COLORREF color = RGB(255, 0, 0));

	void CreateHorizontalScroll();
	void init_Histroy();	
	void dumpToStdout(const tinyxml2::XMLNode* parent);	
	CString searchXMLData(const tinyxml2::XMLNode* parent, CString Key);
	afx_msg void OnBnClickedButtonXmlClear();
	afx_msg void OnLbnDblclkListScriptRun();
	afx_msg void OnBnClickedButtonProcessingAllClear();
	afx_msg void OnBnClickedButtonImgprocAdd();

	void init_command();
	std::vector <std::pair<CString, CString>> m_vt_list_command;
	CString m_strCommand;
	afx_msg void OnLbnSelchangeListScriptList();
};
