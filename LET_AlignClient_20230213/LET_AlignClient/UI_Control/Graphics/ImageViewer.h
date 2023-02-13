#pragma once
#include "stdafx.h"
#include "GraphicDisplayControl.h"

// ImageViewer 대화 상자

class ImageViewer: public CDialogEx
{
	DECLARE_DYNAMIC(ImageViewer)
public:
	ImageViewer(CWnd* pParent = nullptr, bool selfClose = false);   // 표준 생성자입니다.
	virtual ~ImageViewer();

	void SetImage(const cv::Mat& src);
	void SetTitle(CString strTitle);
	CString GetTitle() const;

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_IMAGE_VIEWER };
#endif

private:
	CString m_StrTitle = _T("");
	bool m_SelfDestroy = false;
	CViewerEx dsp;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()

	virtual void PostNcDestroy();
public:
	afx_msg void OnClose();

	std::function<void(CDialog*)> OnClosing;
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
};
