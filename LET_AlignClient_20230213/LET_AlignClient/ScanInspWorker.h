#pragma once
class CScanInspWorker
{
public:
	CScanInspWorker(void);
	~CScanInspWorker(void);

	CView* m_pMainForm;
	CWnd* m_pM;

	CRITICAL_SECTION m_csScanWrite;

	BOOL write_result_scan_insp(CString serialNum, BOOL bjudge, int algorithm);
	BOOL algorithm_WetOut_OnesInsp(BYTE* pImage, int nJob, int nCam, int W, int H, CViewerEx* pView = NULL, BOOL bManual = FALSE);
};

