#pragma once

// CDlgProfile 대화 상자입니다.

class CDlgProfile : public CDialogEx
{
	DECLARE_DYNAMIC(CDlgProfile)

public:
	int m_window_width;
	int m_window_height;
	int m_hori_cnt;
	int m_vert_cnt;
	int m_origin_x;
	int m_origin_y;
	int m_window_h_st;
	int m_window_h_end;
	int m_window_w_st;
	int m_window_w_end;
	int m_active_width;
	int m_active_height;
	CvPoint m_Hori_Text;
	CvPoint m_Vert_Text;

	CPoint m_ptPFreal_st, m_ptPFreal_end;
	CvPoint m_ptROI_st, m_ptROI_end;

	cv::Mat m_bmpSrcImg;
	int  m_nProfileDirection;
public:
	CDlgProfile(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgProfile();

	void fnCalcRealCoordi(CPoint ptSRC_st, CPoint ptSRC_end);
// 대화 상자 데이터입니다.
	enum { IDD = IDD_DIALOG_PROFILE };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

public:
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	void fnDrawProfile(CPaintDC * pDC);
	void fnDrawGrid(CPaintDC * pDC);
};
