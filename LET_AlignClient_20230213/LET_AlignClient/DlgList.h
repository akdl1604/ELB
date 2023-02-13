#pragma once
// CDlgList 대화 상자입니다.
#include "UI_Control\ButtonEx.h"
#include "UI_Control\LabelEx.h"

#define MAX_LIST 100

struct CenterAlign_List_Item
{
	CString strTime;
	CString strPanelID;
	CString strCenterVideoPath;
	CString str45VideoPath;
	CString strImagePath;
	bool bJudge;
};

struct Align_List_Item
{
	CString strTime;
	CString strPanelID;
	CString strVideoPath;
	CString strImagePath;
	bool bJudge;
}; 

class CLET_AlignClientDlg;
class CDlgList : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgList)

public:
	CDlgList(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgList();

// 대화 상자 데이터입니다.
	enum { IDD = IDD_DIALOG_LIST };
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.
	DECLARE_MESSAGE_MAP()

private:
	void MainButtonInit(CButtonEx *pbutton,int size = 15);
	void InitTitle(CLabelEx *pTitle,CString str,float size,COLORREF color);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);

public:	
	HBRUSH m_hbrBkg;
	CLET_AlignClientDlg* m_pMain;
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	virtual BOOL OnInitDialog();

	int m_nJob;
	CGridCtrl m_grid_list;
	CLabelEx m_lbTitle;
	
	void get_ID_from_loadimage(int nJob, CString strFilePath, CString* strID);
	void get_file_create_time(CString strFilePath, CString* strTime);
	void get_file_create_Day(CString strFilePath, CString* strDay);

	void list_clear();

	// ========================= Center Align =====================================
	void update_list_centeralign(WORD year = 0, WORD month = 0, WORD day = 0);
	void init_grid_centeralign(CGridCtrl& pGrid);
	void get_image_directory_centeralign(CString *strImagePath, bool bJudge, int nJob, WORD year=0, WORD month=0, WORD day=0);
	void get_list_item_centeralign(std::vector < CenterAlign_List_Item > &vt_List, CString strPath, bool bJudge, int nJob);
	void get_list_image_path_centeralign(CString strImageDirPath, CString* strImagePath);
	void get_list_center_video_path(CString strPanelID, CString* strVideoPath);
	void get_list_45_video_path(CString strPanelID, CString* strVideoPath);
	void display_list_headline_cetneralign();
	void display_list_update_centeralign(std::vector < CenterAlign_List_Item >& vt_List);

	std::vector < CenterAlign_List_Item > m_vt_TOTAL_CA;
	std::vector < CenterAlign_List_Item > m_vt_OK_CA;
	std::vector < CenterAlign_List_Item > m_vt_NG_CA;

	void sort_centeralign_list(std::vector < CenterAlign_List_Item >& vt_List);
	// =============================================================================

	// ========================= JOB Align =====================================
	void update_list_align(int nJob, WORD year = 0, WORD month = 0, WORD day = 0);
	void get_image_directory_align(CString* strImagePath, bool bJudge, int nJob, WORD year = 0, WORD month = 0, WORD day = 0);
	void get_list_item_align(std::vector < Align_List_Item >& vt_List, CString strPath, bool bJudge, int nJob);
	void get_list_image_path_align(CString strImageDirPath, CString* strImagePath,int nJob);
	void get_list_video_path(CString strPanelID, CString* strVideoPath,int nJob);
	void display_list_headline_align();
	void display_list_update_align(std::vector < Align_List_Item >& vt_List);

	std::vector < Align_List_Item > m_vt_TOTAL;
	std::vector < Align_List_Item > m_vt_OK;
	std::vector < Align_List_Item > m_vt_NG;

	void sort_align_list(std::vector < Align_List_Item >& vt_List);
	// =============================================================================
	CDateTimeCtrl m_dateTimeCtrl;
	afx_msg void OnDatetimechangeDatetimepickerList(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnClose();

	CString m_strOldDate;

	int m_Year;
	int m_Month;
	int m_Day;
};

