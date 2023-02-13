#pragma once
class CSratchInspWorker
{
public:
	CSratchInspWorker(void);
	~CSratchInspWorker(void);

	CView* m_pMainForm;
	CLET_AlignClientDlg* m_pMain;

	CRITICAL_SECTION m_csScratchWrite;

	void InspectionEnhance(cv::Mat* src, int id);
	BOOL Inspection_ScratchHole(int nJob, int id, BYTE* ImgByte, CViewerEx* mViewer = NULL, bool bSimul = FALSE);
	BOOL Inspection_ScratchLine(int nJob, int id, BYTE* ImgByte, CViewerEx* mViewer = NULL, bool bSimul = FALSE);
	friend UINT Inspection_ScratchHole_ProcImage(void* pParam);
};

