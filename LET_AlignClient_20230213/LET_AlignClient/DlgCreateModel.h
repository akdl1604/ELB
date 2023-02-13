#pragma once

#include "EditEx.h"
#include "ListCtrlStyled.h"
// CDlgCreateModel 대화 상자입니다.

class CLET_AlignClientDlg;

class CDlgCreateModel : public CDialog
{
	DECLARE_DYNAMIC(CDlgCreateModel)

public:
	CDlgCreateModel(CWnd* pParent = NULL);   // 표준 생성자입니다.
	virtual ~CDlgCreateModel();

	// 대화 상자 데이터입니다.
	enum { IDD = IDD_DLG_CREATE_MODEL };

	CListCtrlStyled m_ListModelID;
	CLET_AlignClientDlg *m_pMain;

	CLabelEx m_stt_ModelInfo;
	CLabelEx m_stt_Model_Name;
	CLabelEx m_stt_Inch_Name;
	CLabelEx m_stt_GlassWidth_Name;
	CLabelEx m_stt_GlassHeight_Name;

	CEditEx m_edt_ModelTitle;
	CEditEx m_edt_GlassInch;
	CEditEx m_edt_GlassWidth;
	CEditEx m_edt_GlassHeight;
	

	CButtonEx m_btnModelClose;
	CButtonEx  m_btnCreateModel;

	CString m_strEditModel;
	CString m_strEditInch;
	CString m_strEditGlassWidth;
	CString m_strEditGlassHeight;
	HBRUSH m_hbrBkg;
	BOOL nCopyExistModel;
	CString m_SelstrModel;

	int m_nSelPos;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()
public:
	virtual BOOL OnInitDialog();
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	afx_msg void OnNMClickListModel(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnBnClickedBtnCreate();
	afx_msg void OnBnClickedBtnClose();
	afx_msg void OnEnSetfocusEditInch();	
	afx_msg void OnEnSetfocusEditGlassWidth();
	afx_msg void OnEnSetfocusEditGlassHeight();	
	afx_msg void OnEnSetfocusEditCreateModel();

	CModel fnCreateModelData(int algo);
	void initListCtrl();
	BOOL getModelList();
	void InitTitle(CLabelEx *pTitle, CString str, float size, COLORREF color);
	void EditButtonInit(CEditEx *pbutton, int size, COLORREF color = RGB(255, 0, 0));
	void MainButtonInit(CButtonEx *pbutton, int size = 15);
};
	