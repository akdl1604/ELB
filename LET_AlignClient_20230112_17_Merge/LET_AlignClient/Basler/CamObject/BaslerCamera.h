#pragma once
#include "CameraObject.h"
#include <pylon/PylonIncludes.h>
#include <pylon/gige/BaslerGigEInstantCamera.h>

/*
 * - Lincoln Lee
 * - x.perfect.vn@gmail.com
 * - 2022-08-26
 */

class BaslerCamera : public Pylon::CConfigurationEventHandler, public Pylon::CImageEventHandler, public CameraObject
{
public:
	bool IsOpen() const override;
	bool DeInit() override;
	bool InitCamera(int camId, Pylon::CDeviceInfo& dev, CDialog* pDlg = NULL) override;
	bool IsGrabbing() const override;
	bool IsConnected() const override;

	BaslerCamera& Grab(bool continous = true) override;
	BaslerCamera& GrabSingle() override;
	BaslerCamera& SoftwareTrigger() override;

	BaslerCamera& SetGain(INT nGain) override;
	BaslerCamera& SetGainRaw(double dGainRaw) override;
	BaslerCamera& SetFrameRate(double frame_rate) override;
	BaslerCamera& SetPackageSize(int size) override;
	BaslerCamera& SetGrabMode(bool bContious = true) override;
	BaslerCamera& SetReverseX(bool bSet = true) override;
	BaslerCamera& SetReverseY(bool bSet = true) override;
	BaslerCamera& SetExposure(INT nExposure) override;
	BaslerCamera& SetExposureTime(double dExposureTime, bool bAutoExposure = false) override;
	BaslerCamera& SetTriggerMode(TriggerMode tm = TriggerMode::SOFTWARE) override;
	BaslerCamera& StopGrab() override;
	BaslerCamera& SaveExposureTime(INT nExposure) override;

	BaslerCamera();
	~BaslerCamera();

	BaslerCamera(const _stCameraInfo& info);
private:
	Pylon::CBaslerGigEInstantCamera		  m_GigECamera;
	Pylon::CSoftwareTriggerConfiguration* m_GigEConfiguration = NULL;

	void OnImageGrabbed(Pylon::CInstantCamera& camera, const Pylon::CGrabResultPtr& grabResult);
};

