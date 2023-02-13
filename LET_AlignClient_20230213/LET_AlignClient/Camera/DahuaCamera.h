#ifdef __DAHUHA
#pragma once
#include "CameraObject.h"
#include "GenICam/Defs.h"
#include "GenICam/System.h"
#include "GenICam/Camera.h"
#include "GenICam/Frame.h"
#include "Infra/Thread.h"
#include "Infra/Timer.h"
#include "Memory/SharedPtr.h"
#include "MessageQue.h"

/*
 * - Lincoln Lee
 * - x.perfect.vn@gmail.com
 * - 2022-08-26
 */

class DahuaCamera : public CameraObject
{
public:
	bool IsOpen() const override;
	bool DeInit() override;
	bool InitCamera(int camId, Dahua::GenICam::ICameraPtr& iCam, CDialog* pDlg = NULL) override;
	bool IsGrabbing() const override;
	bool IsConnected() const override;

	DahuaCamera& Grab(bool continous = true) override;
	DahuaCamera& GrabSingle() override;
	DahuaCamera& SoftwareTrigger() override;

	DahuaCamera& SetGain(INT nGain) override;
	DahuaCamera& SetFrameRate(double frame_rate) override;
	DahuaCamera& SetPackageSize(int size) override;
	DahuaCamera& SetGrabMode(bool bContious = true) override;
	DahuaCamera& SetReverseX(bool bSet = true) override;
	DahuaCamera& SetReverseY(bool bSet = true) override;
	DahuaCamera& SetGainRaw(double dGainRaw) override;
	DahuaCamera& SetExposure(INT nExposure) override;
	DahuaCamera& SetExposureTime(double dExposureTime, bool bAutoExposure = false) override;
	DahuaCamera& SetTriggerMode(TriggerMode tm = TriggerMode::SOFTWARE) override;
	DahuaCamera& StopGrab() override;
	DahuaCamera& changeParameters(int fps = 10) override;
	DahuaCamera& setGrabMode(bool cont = true) override;
	DahuaCamera& SaveExposureTime(INT nExposure) override { return SetExposure(nExposure); }

	DahuaCamera& SetContrast(int nBrightness) override;
	DahuaCamera& SetBrightness(int nBrightness) override;

	DahuaCamera();
	~DahuaCamera();

	DahuaCamera(const _stCameraInfo& info);
private:
	Dahua::GenICam::ICameraPtr m_CameraPTR;
	Dahua::GenICam::IStreamSourcePtr m_StreamPTR;
	Dahua::GenICam::IStreamSource::Proc m_Proc;

private:
	void onGetFrame(const Dahua::GenICam::CFrame& frame);
};
#endif