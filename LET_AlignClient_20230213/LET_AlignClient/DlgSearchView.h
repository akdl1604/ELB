#pragma once


// DlgSearchView 대화 상자

class DlgSearchView : public CDialogEx
{
	DECLARE_DYNAMIC(DlgSearchView)

public:
	DlgSearchView(CWnd* pParent = nullptr);   // 표준 생성자입니다.
	virtual ~DlgSearchView();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DLG_SEARCH_VIEW };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	CStatic m_pic_search_view;
	cv::Mat m_matSearchImg; // 이미지 정보를 담고 있는 객체.
	BITMAPINFO *m_pBitmapInfo; // Bitmap 정보를 담고 있는 구조체.

	void CreateBitmapInfo(int w, int h, int bpp); // Bitmap 정보를 생성하는 함수.
	void DrawImage(); // 그리는 작업을 수행하는 함수.
	void DrawImagePath(int nView); //선택한 이미지 불러오는 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnBnClickedBtnImgLoad();
	CScrollBar m_sc_load_v;
	CScrollBar m_sc_load_h;
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnVScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	double ratio;
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
	void ScrollInitRange();
	int WIDTH, HEIGHT;
	void DrawWhite();
	afx_msg void OnBnClickedBtnOriginal();
};
