#include "stdafx.h"
#include "CameraObject.h"
#include "DahuaCamera.h"
#include "LET_AlignClientDlg.h"

/*
 * - Lincoln Lee
 * - x.perfect.vn@gmail.com
 * - 2022-08-26
 */

#define WM_DISP						WM_USER+9800
#define WM_GRABEND					WM_USER+9700
#define GRAB_DISPLAY				1
#define MAX_GRABBUFFER              4

#ifdef __DAHUHA

#pragma comment(lib,"MVSDKmd.lib")

using namespace Dahua;
using namespace Dahua::GenICam;

DahuaCamera::DahuaCamera(const _stCameraInfo& info) : DahuaCamera()
{
	m_Width = info.w;
	m_Height = info.h;
	m_FPS = info.frame_rate;
	m_FlipCode = FlipCodes(info.flip_dir);
}

bool DahuaCamera::IsGrabbing() const
{
	return (m_StreamPTR.get() && m_StreamPTR->isGrabbing());
}

DahuaCamera::DahuaCamera() : CameraObject()
{
	m_hDummy = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_Proc = Dahua::GenICam::IStreamSource::Proc(&DahuaCamera::onGetFrame, this);
}

DahuaCamera::~DahuaCamera()
{
	DeInit();
}

bool DahuaCamera::IsConnected() const
{
	return (m_bInitialize && m_CameraPTR->isConnected());
}

bool DahuaCamera::IsOpen() const
{
	return (m_bInitialize && m_CameraPTR->isConnected());
}

bool DahuaCamera::DeInit()
{
	StopGrab();

	m_bInitialize = false;
	if (m_CameraPTR.get()) m_CameraPTR->disConnect();

	if (m_hDummy)
	{
		CloseHandle(m_hDummy);
		m_hDummy = NULL;
	}

	return true;
}

bool DahuaCamera::InitCamera(int camId, Dahua::GenICam::ICameraPtr& iCam, CDialog* pDlg)
{
	double val;
	m_nCam = camId;
	m_MainDlg = pDlg;
	m_CameraPTR = iCam;

	if (m_CameraPTR.get() && (m_CameraPTR->connect()))
	{
		auto& systemObj = CSystem::getInstance();
		auto acqCtrl = systemObj.createAcquisitionControl(m_CameraPTR);

		acqCtrl->acquisitionMode().setValueBySymbol("Continuous");
		acqCtrl->acquisitionFrameRate().getValue(val);

		m_FPS = MAX(1, lroundf(val));
		m_StreamPTR = systemObj.createStreamSource(m_CameraPTR);
		if (!m_StreamPTR) return false;

		m_StreamPTR->setBufferCount(1);
		if (m_StreamPTR->isGrabbing()) m_StreamPTR->stopGrabbing();
		m_bInitialize = m_StreamPTR->attachGrabbing(m_Proc);

		Delay(1);
	}
	return m_bInitialize;
}

DahuaCamera& DahuaCamera::Grab(bool continous)
{
	if (!continous)
		return GrabSingle();

	if (m_bInitialize && m_StreamPTR.get() && !m_StreamPTR->isGrabbing())
	{
		m_StreamPTR->attachGrabbing(m_Proc);
		Delay(1);
		m_StreamPTR->startGrabbing();
		m_bGrabContinuous = continous;
		Delay(1);
	}
	return *this;
}

DahuaCamera& DahuaCamera::GrabSingle()
{
	if (m_bInitialize && m_StreamPTR.get() && !m_StreamPTR->isGrabbing())
	{
		m_StreamPTR->attachGrabbing(m_Proc);
		Delay(1);
		m_StreamPTR->startGrabbing(1);
		Delay(1);

		int count = 0;
		while (((CLET_AlignClientDlg*)m_MainDlg)->m_bGrabEnd[m_nCam] == FALSE)
		{
			Delay(1);
			if (++count > 2000)
				break;
		}
		StopGrab();
	}
	return *this;
}

DahuaCamera& DahuaCamera::SoftwareTrigger()
{
	if (IsOpen())
		CSystem::getInstance().createAcquisitionControl(m_CameraPTR)->triggerSoftware().execute();
	Delay(1);
	return *this;
}

DahuaCamera& DahuaCamera::SetGain(INT nGain)
{
	if (IsOpen())
		CSystem::getInstance().createAnalogControl(m_CameraPTR)->gainRaw().setValue(nGain);
	Delay(1);
	return *this;
}

DahuaCamera& DahuaCamera::changeParameters(int fps)
{
	if (IsOpen())
	{
		auto acqCtrl = CSystem::getInstance().createAcquisitionControl(m_CameraPTR);

		acqCtrl->triggerSelector().setValueBySymbol("AcquisitionStart");
		acqCtrl->triggerMode().setValueBySymbol("Off");

		acqCtrl->acquisitionFrameRate().setValue(fps);
		acqCtrl->acquisitionFrameRateEnable().setValue(true);
	}
	Delay(1);
	return *this;
}

DahuaCamera& DahuaCamera::SetFrameRate(double fps)
{
	if (IsOpen())
	{
		auto acqCtrl = CSystem::getInstance().createAcquisitionControl(m_CameraPTR);
		acqCtrl->acquisitionFrameRate().setValue(fps);
		acqCtrl->acquisitionFrameRateEnable().setValue(true);
	}
	Delay(1);
	return *this;
}

DahuaCamera& DahuaCamera::SetPackageSize(int size)
{
	return *this;
}

DahuaCamera& DahuaCamera::SetGrabMode(bool bContious)
{
	return *this;
}

DahuaCamera& DahuaCamera::SetReverseX(bool bSet)
{
	if (IsOpen())
		CSystem::getInstance().createImageFormatControl(m_CameraPTR)->reverseX().setValue(bSet);
	Delay(1);
	return *this;
}

DahuaCamera& DahuaCamera::SetReverseY(bool bSet)
{
	if (IsOpen())
		CSystem::getInstance().createImageFormatControl(m_CameraPTR)->reverseY().setValue(bSet);
	Delay(1);
	return *this;
}

DahuaCamera& DahuaCamera::SetGainRaw(double dGainRaw)
{
	if (IsOpen())
		CSystem::getInstance().createAnalogControl(m_CameraPTR)->gainRaw().setValue(dGainRaw);
	Delay(1);
	return *this;
}

DahuaCamera& DahuaCamera::SetExposure(INT nExposure)
{
	if (IsOpen())
		CSystem::getInstance().createAcquisitionControl(m_CameraPTR)->exposureTime().setValue(nExposure);
	Delay(1);
	return *this;
}

DahuaCamera& DahuaCamera::SetContrast(int val)
{
	if (IsOpen())
		CSystem::getInstance().createISPControl(m_CameraPTR)->contrast().setValue(val);
	Delay(1);
	return *this;
}

DahuaCamera& DahuaCamera::SetBrightness(int val)
{
	if (IsOpen())
		CSystem::getInstance().createISPControl(m_CameraPTR)->brightness().setValue(val);
	Delay(1);
	return *this;
}

DahuaCamera& DahuaCamera::setGrabMode(bool cont)
{
	if (IsOpen())
	{
		auto acqCtrl = CSystem::getInstance().createAcquisitionControl(m_CameraPTR);
		if (cont)
		{
			acqCtrl->triggerSource().setValueBySymbol("Software");
			acqCtrl->triggerMode().setValueBySymbol("Off");
			acqCtrl->triggerSelector().setValueBySymbol("AcquisitionStart");
		}
		else
		{
			acqCtrl->triggerSource().setValueBySymbol("Software");
			acqCtrl->triggerMode().setValueBySymbol("On");
		}
	}
	Delay(1);
	return *this;
}

DahuaCamera& DahuaCamera::SetTriggerMode(TriggerMode tm)
{
	if (IsOpen())
	{
		auto acqCtrl = CSystem::getInstance().createAcquisitionControl(m_CameraPTR);
		if (tm == TriggerMode::SOFTWARE)
		{
			acqCtrl->triggerSource().setValueBySymbol("Software");
			acqCtrl->triggerMode().setValueBySymbol("On");
		}
		else
		{
			acqCtrl->triggerSource().setValueBySymbol("Hardware");
		}
	}
	Delay(1);
	return *this;
}

DahuaCamera& DahuaCamera::SetExposureTime(double dExposureTime, bool bAutoExposure)
{
	if (IsOpen())
	{
		auto acqCtrl = CSystem::getInstance().createAcquisitionControl(m_CameraPTR);
		if (bAutoExposure)
		{
			acqCtrl->exposureAuto().setValueBySymbol("Continuous");
		}
		else
		{
			acqCtrl->exposureAuto().setValueBySymbol("Off");
			acqCtrl->exposureTime().setValue(dExposureTime);
		}
	}
	Delay(1);
	return *this;
}

DahuaCamera& DahuaCamera::StopGrab()
{
	if (m_StreamPTR.get() && m_StreamPTR->isGrabbing())
	{
		m_bGrabContinuous = false;
		m_StreamPTR->detachGrabbing(m_Proc);
		Delay(1);
		m_StreamPTR->stopGrabbing();
		Delay(1);
	}

	m_bGrabContinuous = false;
	return *this;
}

void DahuaCamera::onGetFrame(const Dahua::GenICam::CFrame& frame)
{
	m_mtx.lock();
	if (frame.valid())
		OnReceivedFrame(frame.getImage(), ULONG64(m_Width * m_Height));
	else
		SetReceivedFrame(true);
	m_mtx.unlock();
}
#endif