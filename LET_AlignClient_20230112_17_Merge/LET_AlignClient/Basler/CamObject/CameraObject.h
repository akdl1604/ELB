#pragma once
#include <opencv.hpp>
#include "../UI_Control/Graphics/FlipCodes.h"
#include "TriggerMode.h"
#include <GenICam/Camera.h>
#include <pylon/PylonIncludes.h>

/*
 * - Lincoln Lee
 * - x.perfect.vn@gmail.com
 * - 2022-08-26
 */

class CameraObject
{
public:
	SRWLOCK g_bufsrwlock;

public:
	CameraObject();
	virtual ~CameraObject();

	virtual bool    IsRecordVideo() const;
	virtual bool    IsConnected() const { return m_bInitialize; }
	virtual bool	IsOpen() const { return false; }
	virtual bool	IsGrabbing() const { return false; }
	virtual bool	IsGrabContinuous() const { return m_bGrabContinuous; }

	virtual LONG	GetImageX() const { return m_Width; }
	virtual LONG	GetImageY() const { return m_Height; }
	virtual INT		GetFrameCount() const { return m_nInputFrame; }

	virtual CameraObject& GrabSingle() { return *this; }
	virtual CameraObject& Grab(bool continuous = true) { return *this; }
	virtual CameraObject& SetTriggerMode(TriggerMode tm = TriggerMode::SOFTWARE) { return *this; }
	virtual CameraObject& SetFrameRate(double frame_rate) { return *this; }
	virtual CameraObject& SetPackageSize(int size) { return *this; }
	virtual CameraObject& SetExposure(INT nExposure) { return *this; };
	virtual CameraObject& SetGain(INT nGain) { return *this; };
	virtual CameraObject& SetContrast(int nBrightness) { return *this; }
	virtual CameraObject& SetBrightness(int nBrightness) { return *this; }
	virtual CameraObject& SaveExposureTime(INT nExposure) { return *this; }
	virtual CameraObject& SoftwareTrigger() { return *this; }
	virtual CameraObject& SetGrabMode(bool bContious = true) { return *this; }
	virtual CameraObject& SetReverseX(bool bSet = false) { return *this; }
	virtual CameraObject& SetReverseY(bool bSet = false) { return *this; }
	virtual CameraObject& SetGainRaw(double dGainRaw) { return *this; }
	virtual CameraObject& SetExposureTime(double dExposureTime, bool bAutoExposure = false) { return *this; }
	virtual CameraObject& SetSaveMPEG(bool save = true) { m_SavingMPEG = save;  return *this; }

	virtual CameraObject& setGrabMode(bool cont = true) { return *this; }
	virtual CameraObject& changeParameters(int fps = 10) { return *this; }
	virtual CameraObject& StopGrab() { return *this; }
	virtual CameraObject& cvVideoOpen(const CString& fname, int w, int h);
	virtual CameraObject& cvVideoClose();
	virtual bool DeInit() { return true; } // DeInit always OK with abstract class

	virtual bool InitCamera(int camId, Pylon::CDeviceInfo& dev, CDialog* pDlg = NULL) { return false; }
	virtual bool InitCamera(int camId, Dahua::GenICam::ICameraPtr& iCam, CDialog* pDlg = NULL) { return false; }
	virtual void SetReceivedFrame(bool val = true);

protected:
	std::mutex m_mtx;
	CDialog* m_MainDlg = NULL;

	HANDLE  m_hDummy;

	FlipCodes m_FlipCode;

	long m_FPS;
	long m_nCam;
	long m_Width;
	long m_Height;
	long m_nInputFrame;
	long m_nGrabBufferNo;

	bool m_bInitialize;
	bool m_SavingMPEG;
	bool m_bGrabContinuous;

	TriggerMode m_TriggerMode;
	cv::VideoWriter m_videoWriter;

	virtual long OnReceivedFrame(const void* data, ULONG64 nImageSize);
};
