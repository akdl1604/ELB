#pragma once

// CPaneModel �� ���Դϴ�.

class CPaneModel : public CFormView
{
	DECLARE_DYNCREATE(CPaneModel)

protected:
	CPaneModel();           // ���� ����⿡ ���Ǵ� protected �������Դϴ�.
	virtual ~CPaneModel();

public:
	enum { IDD = IDD_PANE_MODEL };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

	CLET_AlignClientDlg *m_pMain;

	CRect m_rcForm;
	CButtonEx m_btnModelClose;
	CButtonEx  m_btnCreateModel;
	CButtonEx  m_btnDeleteModel;
	CButtonEx  m_btnSystemOption;

	HBRUSH m_hbrBkg;
	
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:	
	virtual void OnInitialUpdate();

	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnBnClickedBtnModelClose();
	afx_msg void OnPaint();
	afx_msg void OnBnClickedBtnCreateModel();
	afx_msg void OnBnClickedBtnDeleteModel();
	afx_msg void OnBnClickedBtnSystemOption();
	
	void InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color);	
	void MainButtonInit(CButtonEx *pbutton, int size = 15);
};


