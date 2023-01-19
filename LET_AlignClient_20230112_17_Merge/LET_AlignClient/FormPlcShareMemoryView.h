#pragma once


#include "GridCtrl_src/GridCtrl.h"
#include "GroupEx.h"
#include "EditEx.h"
#include "afxwin.h"

typedef struct _CellData
{
	int col;
	int row;
	int status;
	CString data;
} CellData;

#define MAX_ADDR_UNIT 22
#define  PLC_UNIT_ADDRESS_SIZE 20
#import "xlsgen.dll" no_function_mapping

class xlsgenLoader
{
	typedef xlsgen::IXlsEngine* (*funcStart)();

	HINSTANCE m_hInst;
	funcStart m_funcstart;

public:
	xlsgenLoader() {

	m_hInst = LoadLibraryA("xlsgen.dll");


		if (!m_hInst)    return;

		m_funcstart = (funcStart) GetProcAddress(m_hInst, "Start");
		if (!m_funcstart) {
			FreeLibrary(m_hInst);
			m_hInst = NULL;
			return;
		}
	}

	~xlsgenLoader() {
		if (m_hInst)  {
			FreeLibrary(m_hInst);
			m_hInst = NULL;
		}
	}

	bool IsInitialized() {
		return (m_hInst != NULL);
	}

	xlsgen::IXlsEnginePtr Start() {
		return m_funcstart ? (m_funcstart)() : NULL;
	}
};

// CFormPlcShareMemoryView 폼 뷰입니다.
class CLET_AlignClientDlg;
class CFormPlcShareMemoryView : public CFormView
{
	DECLARE_DYNCREATE(CFormPlcShareMemoryView)

protected:
	CFormPlcShareMemoryView();           // 동적 만들기에 사용되는 protected 생성자입니다.
	virtual ~CFormPlcShareMemoryView();

public:
	enum { IDD = IDD_FORM_PLC_INTERFACE };
#ifdef _DEBUG
	virtual void AssertValid() const;
#ifndef _WIN32_WCE
	virtual void Dump(CDumpContext& dc) const;
#endif
#endif

public:
	xlsgenLoader xlsgen;

	CGridCtrl m_GridInterfacePcFromPlc;
	CGridCtrl m_GridInterfacePlcFromPc;

	CGroupEx m_grpUnitSelect1;
	CGroupEx m_grpUnitSelect2;
	CGroupEx m_grpPcToPlc;
	CGroupEx m_grpPlcToPC;

	CButtonEx m_btnMain;
	CButtonEx m_btnInputAlign1;
	CButtonEx m_btnInputAlign2;
	CButtonEx m_btnPrecisionAlign1;
	CButtonEx m_btnPrecisionAlign2;
	CButtonEx m_btnPrecisionAlign3;
	CButtonEx m_btnPrecisionAlign4;
	CButtonEx m_btnSamplingAlign;
	CButtonEx m_btnInspection;
	CButtonEx m_btnLoadTrayGlassInsp;
	CButtonEx m_btnStageDisplacementSensor1;
	CButtonEx m_btnStageDisplacementSensor2;
	CButtonEx m_btnStageDisplacementSensor3;
	CButtonEx m_btnStageDisplacementSensor4;
	CButtonEx m_btnUvMeasure1;
	CButtonEx m_btnUvMeasure2;
	CButtonEx m_btnBCR1;
	CButtonEx m_btnBCR2;
	CButtonEx m_btnLoadTrayBCR;
	CButtonEx m_btnSpare;
	CButtonEx m_btnSpare2;
	CButtonEx m_btnSpare3;

	CLabelEx m_LbPlcViewTitle;
	CLabelEx m_lblHistoryTitle;

	CLET_AlignClientDlg *m_pMain;

	CRITICAL_SECTION m_csView;
	CString m_strProcess;

	CArray<CellData> *m_RowPcFromPlc_Data;
	CArray<CellData> *m_RowPlcFromPc_Data;

	int m_nUnitSelectNum;
	long m_ldPcFromPlcWord[10];
	long m_ldPlcFromPcWord[10];

	BOOL m_bBitSelection;
	CString strIniFileName;
	CString m_strUnitName[20];

	int m_nPcFromPlcBit[MAX_ADDR_UNIT][PLC_UNIT_ADDRESS_SIZE];
	int m_nPlcFromPcBit[MAX_ADDR_UNIT][PLC_UNIT_ADDRESS_SIZE];

	int m_nPcFromPlcBitStatus[MAX_ADDR_UNIT][PLC_UNIT_ADDRESS_SIZE];
	int m_nPlcFromPcBitStatus[MAX_ADDR_UNIT][PLC_UNIT_ADDRESS_SIZE];

	_bstr_t strTempPlcFromPc[MAX_ADDR_UNIT][PLC_UNIT_ADDRESS_SIZE];
	_bstr_t strTempPcFromPlc[MAX_ADDR_UNIT][PLC_UNIT_ADDRESS_SIZE];

	CString m_strPcFromPlc[MAX_ADDR_UNIT][PLC_UNIT_ADDRESS_SIZE];
	CString m_strPlcFromPc[MAX_ADDR_UNIT][PLC_UNIT_ADDRESS_SIZE];
	CString m_strPcFromPlcWord[MAX_ADDR_UNIT][PLC_UNIT_ADDRESS_SIZE];
	CString m_strPlcFromPcWord[MAX_ADDR_UNIT][PLC_UNIT_ADDRESS_SIZE];

	void InitGuiControl();
	void addProcessHistory(CString str);
	void DrawGrid(CGridCtrl &grid, int col, int row);
	void DrawGridPcFromPlc(CGridCtrl &grid, int col, int row);
	void DrawGridPlcFromPc(CGridCtrl &grid, int col, int row);

	void GridDisplay_PcFromPlc();
	void GridDisplay_PlcFromPc();
	BOOL ReadExcelData();
	void changeButtonColor(int pre, int cur);
	void changeWordValue(int id);

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

	DECLARE_MESSAGE_MAP()	
private:
	
public:
	
	HBRUSH m_hbrBkg;
	void MainButtonInit(CButtonEx *pbutton,int size = 15);
	void InitTitle(CLabelEx *pTitle,CString str,float size,COLORREF color);
	afx_msg HBRUSH OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor);
	virtual void OnInitialUpdate();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	virtual BOOL OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult);
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg LRESULT OnViewControl(WPARAM wParam, LPARAM lParam);
	afx_msg void OnBnClickedButtonSelMain();
	afx_msg void OnBnClickedButtonSelInputAlign1();
	afx_msg void OnBnClickedButtonSelInputAlign2();
	afx_msg void OnBnClickedButtonSelPrecisionAlign1();
	afx_msg void OnBnClickedButtonSelPrecisionAlign2();
	afx_msg void OnBnClickedButtonSelPrecisionAlign3();
	afx_msg void OnBnClickedButtonSelPrecisionAlign4();
	afx_msg void OnBnClickedButtonSelSamplingAlign();
	afx_msg void OnBnClickedButtonSelInspection();
	afx_msg void OnBnClickedButtonSelLoadTrayGlassInsp();
	afx_msg void OnBnClickedButtonSelStageDisplacementSensor1();
	afx_msg void OnBnClickedButtonSelStageDisplacementSensor2();
	afx_msg void OnBnClickedButtonSelStageDisplacementSensor3();
	afx_msg void OnBnClickedButtonSelStageDisplacementSensor4();
	afx_msg void OnBnClickedButtonSelUvMeasure1();
	afx_msg void OnBnClickedButtonSelUvMeasure2();
	afx_msg void OnBnClickedButtonSelBcr1();
	afx_msg void OnBnClickedButtonSelBcr2();
	afx_msg void OnBnClickedButtonSelLoadTrayBcr();
	afx_msg void OnBnClickedButtonSelSpare();
	CEditEx m_EditProcess;
	afx_msg void OnBnClickedButtonSelSpare2();
	afx_msg void OnBnClickedButtonSelSpare3();
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
};