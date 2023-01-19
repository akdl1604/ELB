#pragma once

#define _CAM_MAX_GRAB_COUNT 10

#ifdef _SAPERA
#pragma comment(lib,"C:/Program Files/Teledyne DALSA/Sapera/Lib/Win64/SapClassBasic.lib")
#pragma comment(lib,"C:/Program Files/Teledyne DALSA/Sapera/Lib/Win64/VS2019/SapClassGuiD.lib")

#include "SapClassBasic.h"
#include "SapClassGui.h"

class SapMyProcessing;
class CSaperaCamera;

struct TransferContext {
	int cam_id=0;
	CDialog* pDlg=NULL;
	CSaperaCamera* pCamera = NULL;
	std::atomic_int frameGrabCount = 0, frameProcessingCount = 0;
	std::shared_ptr<SapMyProcessing> processing;
};

class SapMyProcessing : public SapProcessing
{
public:
	SapMyProcessing(SapBuffer* pBuffers, SapProCallback pCallback, void* pContext);
	virtual ~SapMyProcessing();

protected:
	virtual BOOL Run();
};
#endif

class CSaperaCamera
{
public:
	CSaperaCamera();
	virtual ~CSaperaCamera(void);

public:
	BOOL CreateObjects(const std::string& serialNumber, int index);
	BOOL DestroyObjects(void);
	BOOL IsOpend() { return m_IsOpen; }
	BOOL IsGrabing() { return m_bRecordOn; }
	BOOL IsTriggerMode() { return m_bTriggerMode; }
	void ReleaseSap();

	int	GetImageX() { return m_szImage.cx; }
	int	GetImageY() { return m_szImage.cy; }
	int CreateImageFullBuffer(CSize size, int nCount = 1);

	void grab(int maxFrameCount = 1);
	void SingleGrab();
	void SwTrigger(bool bline = true, int ncount = 15);
	void SwTriggerSource(bool bsw = true);
	void TriggerSelector(bool bFrame = true);
	void stop();
#ifdef _SAPERA 
	void Callback(SapXferCallbackInfo* pInfo);
#endif
	void setTriggerMode(bool bTriggerMode);
	void setExposure(double nExposure);
	void setGain(double nGain);
	void setLineRate(double nLinerate);
	void setOpenStatus(BOOL bopen) { m_IsOpen = bopen; }
	void setCamID(int id) {
#ifdef _SAPERA 
		m_Gcontext.cam_id = id;
#endif
	}
	void setMainPtr(CDialog* ptr) {
#ifdef _SAPERA 
		m_Gcontext.pDlg = ptr;
		m_Gcontext.pCamera = this;
#endif
	}
	void setImageSize(CSize size);
	void ClearFullBuffer();
	BOOL GetGrabEndFlag(int id) {	return m_bCamGrabEnd[id];	}
	void SetGrabEndFlag(int id, BOOL bFlag) {m_bCamGrabEnd[id] = bFlag;}
	cv::Mat* GetFullImage() { return &m_matFullImage; }

	int m_nGrabCnt;

	// dh.jung 2021-05-20 add video
	cv::VideoWriter   m_videoWriter;
	CString           m_videoName;
	INT frameCnt;
	INT fileCnt;
	bool m_bSaveMPEG;

	void    cvVideoOpen(CString fname, int w, int h);
	void    cvVideoClose();
	// end

protected:
#ifdef _SAPERA
	SapAcqDevice* m_AcqDevice;
	SapBuffer* m_Buffers;
	SapTransfer* m_Xfer;

	TransferContext m_Gcontext;
#endif
	CSize	m_szImage;

	BOOL m_IsOpen;
	BOOL m_bRecordOn;
	BOOL m_bTriggerMode;
	BOOL m_bCamGrabEnd[_CAM_MAX_GRAB_COUNT];

	cv::Mat m_matFullImage;
};
