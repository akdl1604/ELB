#include "stdafx.h"
#include "CameraObject.h"
#include "BaslerCamera.h"

/*
 * - Lincoln Lee
 * - x.perfect.vn@gmail.com
 * - 2022-08-26
 */

#define WM_DISP						WM_USER+9800
#define WM_GRABEND					WM_USER+9700
#define GRAB_DISPLAY				1
#define MAX_GRABBUFFER              4

#ifdef __CAMERA_H__

using namespace Pylon;
using namespace Basler_GigECameraParams;

bool BaslerCamera::InitCamera(int camId, Pylon::CDeviceInfo& dev, CDialog* pDlg)
{
	m_nCam = camId;
	m_MainDlg = pDlg;

	auto& factory = CTlFactory::GetInstance();
	m_GigEConfiguration = new Pylon::CSoftwareTriggerConfiguration;	// ljm
	m_GigECamera.RegisterConfiguration(m_GigEConfiguration, RegistrationMode_Append, Cleanup_Delete); //Ownership_ExternalOwnership
	m_GigECamera.RegisterConfiguration(this, RegistrationMode_Append, Cleanup_Delete);	 //Ownership_ExternalOwnership
	m_GigECamera.RegisterImageEventHandler(this, RegistrationMode_Append, Cleanup_Delete); //Ownership_ExternalOwnership
	m_GigECamera.GrabCameraEvents = TRUE;
	m_GigECamera.Attach(factory.CreateDevice(dev));

	if (m_GigECamera.IsPylonDeviceAttached())
	{
		if (m_GigECamera.IsOpen())
			m_GigECamera.Close();

		m_GigECamera.Open();
		m_FPS = MAX(1, m_GigECamera.AcquisitionFrameRateAbs.GetValue());
		m_bInitialize = true;
	}

	return m_bInitialize;
}

bool BaslerCamera::IsGrabbing() const
{
	return (m_bInitialize && m_GigECamera.IsGrabbing());
}

bool BaslerCamera::DeInit()
{
	BOOL bSuccess = FALSE;

	try
	{
		if (IsOpen())
		{
			StopGrab();

			if (m_GigEConfiguration->DebugGetEventHandlerRegistrationCount() > 0)
				m_GigECamera.DeregisterConfiguration(m_GigEConfiguration);

			m_GigECamera.Close();
			m_GigECamera.DestroyDevice();

			bSuccess = TRUE;
		}
	}
	catch (...) {}

	m_bInitialize = false;

	return bSuccess;
}

BaslerCamera& BaslerCamera::SetTriggerMode(TriggerMode tm)
{
	if (IsOpen())
	{
		switch (tm)
		{
		case TriggerMode::SOFTWARE:
		{
			m_GigECamera.TriggerMode.SetValue(Basler_GigECamera::TriggerMode_Off);
			m_TriggerMode = TriggerMode::SOFTWARE;
		}
		break;
		case TriggerMode::HARDWARE:
		{
			m_GigECamera.TriggerMode.SetValue(Basler_GigECamera::TriggerMode_On);
			m_GigECamera.TriggerSource.SetValue(Basler_GigECamera::TriggerSource_Line1);
			m_TriggerMode = TriggerMode::HARDWARE;
		}
		break;
		}
		Delay(50);
	}

	return *this;
}

BaslerCamera& BaslerCamera::Grab(bool cont)
{
	if (IsOpen())
	{
		if (cont)
		{
			m_GigECamera.StopGrabbing();
			m_GigECamera.MaxNumBuffer = MAX_BUFFER;

			//SoftwareTrigger();
			m_GigECamera.StartGrabbing(GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
			
			m_bGrabContinuous = TRUE;
		}
		else
		{
			if (!m_GigECamera.IsGrabbing())
			{
				//SoftwareTrigger();
				m_GigECamera.StartGrabbing(1);
			}

			Pylon::CGrabResultPtr frame;
			m_GigECamera.RetrieveResult(5000, frame, TimeoutHandling_ThrowException);

			if (!m_bGrabContinuous) m_GigECamera.StopGrabbing();
		}
	}
	return *this;
}

BaslerCamera& BaslerCamera::SoftwareTrigger()
{
	if (IsOpen())
		m_GigECamera.ExecuteSoftwareTrigger();
	Delay(50);
	return *this;
}

BaslerCamera& BaslerCamera::GrabSingle()
{
	if (!m_GigECamera.IsOpen())	return *this;

	try {
		if (!m_GigECamera.IsGrabbing())
		{
			Pylon::CGrabResultPtr frame;

			m_bGrabContinuous = false;
			m_GigECamera.StartGrabbing(1, GrabStrategy_UpcomingImage, GrabLoop_ProvidedByUser);
			m_GigECamera.RetrieveResult(5000, frame, TimeoutHandling_ThrowException);

			if (frame->GrabSucceeded() != TRUE)
				theLog.logmsg(LOG_ERROR, "[Cam %d] Single Grab() - Grab TimeOut", m_nCam + 1);

			m_GigECamera.StopGrabbing();
		}
		else
		{
			theLog.logmsg(LOG_ERROR, "[Cam %d] Single Grab() - Failed because Already Grabbing", m_nCam + 1);
		}
	}
	catch (...)
	{
		theLog.logmsg(LOG_ERROR, "[Cam %d] Single Grab() - Failed  Exception Error", m_nCam + 1);
	}
	Delay(1);
	return *this;
}

BaslerCamera& BaslerCamera::SetGrabMode(bool bContious)
{
	Delay(1);
	return *this;
}

BaslerCamera& BaslerCamera::SetReverseX(bool v)
{
	if (IsOpen())
	{
		bool state = m_GigECamera.ReverseX.GetValue();
		if (state != v) m_GigECamera.ReverseX.SetValue(v);

		Delay(50);
	}

	return *this;
}

BaslerCamera& BaslerCamera::SetReverseY(bool v)
{
	if (IsOpen())
	{
		bool state = m_GigECamera.ReverseY.GetValue();
		if (state != v) m_GigECamera.ReverseY.SetValue(v);

		Delay(50);
	}

	return *this;
}

BaslerCamera& BaslerCamera::SetFrameRate(double frame_rate)
{
	if ((frame_rate > 0) && IsOpen())
	{
		bool enable = m_GigECamera.AcquisitionFrameRateEnable.GetValue();

		if (!enable)
			m_GigECamera.AcquisitionFrameRateEnable.SetValue(true);
		Delay(50);

		double rate = m_GigECamera.AcquisitionFrameRateAbs.GetValue();
		if (rate != frame_rate)
			m_GigECamera.AcquisitionFrameRateAbs.SetValue(frame_rate);
		Delay(50);

		m_FPS = MAX(1, m_GigECamera.AcquisitionFrameRateAbs.GetValue());
	}
	return *this;
}

BaslerCamera& BaslerCamera::SetPackageSize(int size)
{
	if ((size > 1500) && IsOpen())
	{
		int current = m_GigECamera.GevSCPSPacketSize.GetValue();

		if (current != size)
			m_GigECamera.GevSCPSPacketSize.SetValue(size);

		Delay(50);
	}
	return *this;
}

void BaslerCamera::OnImageGrabbed(CInstantCamera& camera, const CGrabResultPtr& frame)
{
	m_mtx.lock();
	if (frame->GrabSucceeded())
	{
		OnReceivedFrame(frame->GetBuffer(), ULONG64(m_Height * m_Width));
	}
	else
	{
		SetReceivedFrame(true);
		theLog.logmsg(LOG_ERROR, _T("Discription : %s"), frame->GetErrorDescription());
	}
	m_mtx.unlock();
}

bool BaslerCamera::IsConnected() const
{
	return (m_bInitialize && m_GigECamera.IsOpen());
}

bool BaslerCamera::IsOpen() const
{
	return m_GigECamera.IsOpen();// (m_bInitialize && m_GigECamera.IsOpen());
}

BaslerCamera& BaslerCamera::SetGain(INT nGain)
{
	if (IsOpen())
	{
		m_GigECamera.GainAuto.SetValue(GainAuto_Off);
		Delay(50);
		m_GigECamera.GainRaw.SetValue((int64_t)nGain);
		Delay(50);
	}
	return *this;
}

BaslerCamera& BaslerCamera::SetGainRaw(double dGainRaw)
{
	if (IsOpen())
	{
		m_GigECamera.GainAuto.SetValue(GainAuto_Off);
		Delay(50);
		m_GigECamera.GainRaw.SetValue((int64_t)dGainRaw);
		Delay(50);
	}
	return *this;
}

BaslerCamera& BaslerCamera::SetExposureTime(double dExposureTime, bool bAutoExposure)
{
	if (IsOpen())
	{
		m_GigECamera.ExposureAuto.SetValue(bAutoExposure ? Basler_GigECamera::ExposureAuto_Once : Basler_GigECamera::ExposureAuto_Off);
		Delay(50);
		m_GigECamera.ExposureTimeRaw.SetValue(int64_t(dExposureTime), false);
		Delay(50);
	}
	return *this;
}

BaslerCamera& BaslerCamera::SetExposure(INT dExposureTime)
{
	if (IsOpen())
	{
		m_GigECamera.ExposureTimeRaw.SetValue((int64_t)dExposureTime, false);
		Delay(50);
	}
	return *this;
}

BaslerCamera& BaslerCamera::SaveExposureTime(INT nExposure)
{
	if (IsOpen())
	{
		m_GigECamera.ExposureTimeRaw.SetValue((int64_t)nExposure, false);
		Delay(50);
		m_GigECamera.UserSetSelector.SetValue(UserSetSelector_UserSet1);
		Delay(50);
		m_GigECamera.UserSetSave.Execute();
	}

	return *this;
}

BaslerCamera& BaslerCamera::StopGrab()
{
	if(IsOpen())
		m_GigECamera.StopGrabbing();

	m_bGrabContinuous = false;
	return *this;
}

BaslerCamera::BaslerCamera() : CameraObject()
{
	m_GigEConfiguration = NULL;
	m_hDummy = CreateEvent(NULL, TRUE, FALSE, NULL);
}

BaslerCamera::~BaslerCamera()
{
	DeInit();
	if (m_hDummy)
	{
		CloseHandle(m_hDummy);
		m_hDummy = NULL;
	}
	if (m_GigEConfiguration != NULL)
	{
		delete m_GigEConfiguration;
		m_GigEConfiguration = NULL;
	}
}

BaslerCamera::BaslerCamera(const _stCameraInfo& info) : BaslerCamera()
{
	m_Width = info.w;
	m_Height = info.h;
	m_FlipCode = FlipCodes(info.flip_dir);
}

#endif
