#pragma once
#ifndef __CAMERA_H__
#define __CAMERA_H__

#ifndef _DAHUHA

#include <pylon/PylonIncludes.h>
#include <pylon/gige/BaslerGigEInstantCamera.h>
#include <mutex>

#define WM_DISP						WM_USER+9800
#define WM_GRABEND					WM_USER+9700
#define GRAB_DISPLAY				0x01

static const INT CONTINUOUS			= 0;
static const INT HARDWARE_TRIGGER	= 1;
static const UINT MAX_GRABBUFFER	= 4;

using namespace Basler_GigECameraParams;
using namespace Pylon;

class CCamera : public CConfigurationEventHandler,  public CImageEventHandler
{
public:
	CTlFactory						*m_ptlFactory;
	CBaslerGigEInstantCamera		m_GigECamera;
	CSoftwareTriggerConfiguration	*m_GigEConfiguration;
	CGrabResultPtr					m_GrabResultPtr;

private:
	DeviceInfoList_t			m_DIList;
	
	BOOL						m_bInitialize;
	BOOL						m_bGrabContinuous;	
	CSize						m_szImage;

	INT							m_nGrabBufferNo;
	INT							m_nTriggerMode;
	INT							m_nInputFrame;

	cv::VideoWriter             m_videoWriter;
	CString                     m_videoName;
	INT frameCnt;
	INT fileCnt;
public:
	CCamera();
	virtual ~CCamera(void);

	BOOL	Initialize(INT nCam, CString strNum);
	BOOL	Uninitialize();
	
	BOOL	GrabContinuous(BOOL bContinuous);
	void    ExecuteSoftwareTrigger();
	BOOL	SetTriggerMode(INT nMode);
	void	SetExposure(INT nExposure);
	void    SetGain(INT nGain);
	void    SingleGrab();
	void    cvVideoOpen(CString fname, int w, int h);
	void    cvVideoClose();
	void	SaveExposureTime(INT nExposure);

	BOOL set_frame_rate(double frame_rate);
	BOOL set_packet_size(int size);

	BOOL set_reverse_x(BOOL v);
	BOOL set_reverse_y(BOOL v);


	BOOL	IsOpen()					{ return m_GigECamera.IsOpen(); }
	BOOL	IsGrabbing()				{ return m_GigECamera.IsGrabbing(); }
	BOOL	IsGrabContinuous()			{ return m_bGrabContinuous; }

	LONG	GetImageX()					{ return m_szImage.cx; }
	LONG	GetImageY()					{ return m_szImage.cy; }

	INT		GetFrameCount()				{ return m_nInputFrame; }

	HWND    m_hWndDlg;
	HANDLE  m_hDummy;
	int m_nCam;
	bool m_bSaveMPEG;
	CDialog *pDlg;

	std::mutex m_mtx;
	SRWLOCK g_bufsrwlock; // Display와 이미지 Grab Thread 동기화 객체 20211027 Tkyuha

private:
	BOOL OnProcessEnd(BYTE* pImageBuffer, int nImageSize);
	virtual void OnImageGrabbed(CInstantCamera& camera, const CGrabResultPtr& ptrGrabResult);
	virtual void OnGrabError(CInstantCamera& camera, const char* errorMessage);
};
#endif
#endif