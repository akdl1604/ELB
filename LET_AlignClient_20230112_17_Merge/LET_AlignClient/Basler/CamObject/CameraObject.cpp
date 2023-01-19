#include "stdafx.h"
#include "CameraObject.h"
#include "LET_AlignClient.h"
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

CameraObject::CameraObject()
{
	m_FPS = 10;
	m_Width = -1;
	m_Height = -1;

	m_nCam = 0;
	m_nInputFrame = 0;
	m_nGrabBufferNo = 0;

	m_SavingMPEG = false;
	m_bInitialize = false;
	m_bGrabContinuous = false;

	m_MainDlg = NULL;
	m_hDummy = NULL;

	InitializeSRWLock(&g_bufsrwlock);

	m_FlipCode = FlipCodes::FLIP_0;
	m_TriggerMode = TriggerMode::SOFTWARE;
}

CameraObject::~CameraObject()
{
}

bool CameraObject::IsRecordVideo() const
{
	return m_SavingMPEG;
}

CameraObject& CameraObject::cvVideoOpen(const CString& fname, int w, int h)
{
	cvVideoClose();

	m_videoWriter.open(fname.GetString(), CV_FOURCC('X', 'V', 'I', 'D'), m_FPS, cv::Size(w, h), false);
	m_SavingMPEG = true;
	return *this;
}

CameraObject& CameraObject::cvVideoClose()
{
	if (m_videoWriter.isOpened()) m_videoWriter.release();
	m_SavingMPEG = false;
	return *this;
}

void CameraObject::SetReceivedFrame(bool val)
{
	if (NULL != m_MainDlg)
		((CLET_AlignClientDlg*)m_MainDlg)->m_bGrabEnd[m_nCam] = val;
}

long CameraObject::OnReceivedFrame(const void* imgPTR, ULONG64 nImageSize)
{
	if ((NULL == m_MainDlg) || ((CLET_AlignClientDlg*)m_MainDlg)->m_bProgramEnd) return 0;

	auto pMain = (CLET_AlignClientDlg*)m_MainDlg;
	auto pMachine = (CPaneMachine*)pMain->m_pPane[PANE_MACHINE];
	auto job = pMachine->m_hSelectAlgoTab.GetCurSel();
	auto bManualPrealign = pMachine->c_TabMachinePage[job]->IsDlgButtonChecked(IDC_CHK_MANUAL_PREALIGN);

	if (bManualPrealign || imgPTR == NULL)
	{
		SetReceivedFrame(true);
		return 0;
	}

	AcquireSRWLockExclusive(&g_bufsrwlock);
	switch (m_FlipCode)
	{
	case FlipCodes::FLIP_90:
	{
		cv::Mat dstImg;
		cv::Mat srcImg(m_Width, m_Height, CV_8U, (uchar*)imgPTR);	// Width / Height를 미리 Swap

		cv::transpose(srcImg, dstImg);
		AcquireSRWLockExclusive(&pMain->g_srwlock); // 동기화 시킴 Tkyuha 20211027
		pMain->copyMemory(pMain->getSrcBuffer(m_nCam), dstImg.data, nImageSize);
		ReleaseSRWLockExclusive(&pMain->g_srwlock);

		srcImg.release();
		dstImg.release();
	}
	break;

	case FlipCodes::FLIP_180:
	{
		cv::Mat dstImg;
		cv::Mat srcImg(m_Height, m_Width, CV_8U, (uchar*)imgPTR);	// Width / Height를 미리 Swap

		cv::flip(srcImg, dstImg, 0);	// 상하 반전
		cv::flip(dstImg, dstImg, 1);	// 좌우 반전
		AcquireSRWLockExclusive(&pMain->g_srwlock); // 동기화 시킴 Tkyuha 20211027
		pMain->copyMemory(pMain->getSrcBuffer(m_nCam), dstImg.data, nImageSize);
		ReleaseSRWLockExclusive(&pMain->g_srwlock);

		srcImg.release();
		dstImg.release();
	}
	break;

	case FlipCodes::FLIP_270:
	{
		cv::Mat dstImg;
		cv::Mat srcImg(m_Width, m_Height, CV_8U, (uchar*)imgPTR);	// Width / Height를 미리 Swap

		cv::transpose(srcImg, dstImg);
		cv::flip(dstImg, dstImg, 0);	// 좌우 반전
		AcquireSRWLockExclusive(&pMain->g_srwlock); // 동기화 시킴 Tkyuha 20211027
		pMain->copyMemory(pMain->getSrcBuffer(m_nCam), dstImg.data, nImageSize);
		ReleaseSRWLockExclusive(&pMain->g_srwlock);

		srcImg.release();
		dstImg.release();
	}
	break;

	default:
	{
		AcquireSRWLockExclusive(&pMain->g_srwlock); // 동기화 시킴 Tkyuha 20211027
		pMain->copyMemory(pMain->getSrcBuffer(m_nCam), (void*)imgPTR, nImageSize);
		ReleaseSRWLockExclusive(&pMain->g_srwlock);
	}
	break;
	}

	if (!pMain->m_bGrabEnd[m_nCam] && (pMain->m_nCurPane == PANE_AUTO ||
		(pMain->m_nCurForm == FORM_MAIN || pMain->m_nCurForm == FORM_MACHINE || pMain->m_nCurForm == FORM_INTERFACE)))	//220503 Tkyuha 카메라 겹칩 확인용
	{
		SetReceivedFrame(true);
	}

	if (m_SavingMPEG && m_videoWriter.isOpened())
		m_videoWriter.write(cv::Mat(m_Height, m_Width, CV_8U, pMain->getSrcBuffer(m_nCam)));

	m_nGrabBufferNo = (m_nGrabBufferNo + 1) % MAX_GRABBUFFER;
	ReleaseSRWLockExclusive(&g_bufsrwlock);

	return 0;
}

#pragma region TRASH
//if (dir == 1)	// 90도
//{
//	cv::Mat dstImg;
//	cv::Mat srcImg(w, h, CV_8U, imgPTR);	// Width / Height를 미리 Swap
//	cv::transpose(srcImg, dstImg);
//	AcquireSRWLockExclusive(&pMain->g_srwlock); // 동기화 시킴 Tkyuha 20211027
//	pMain->copyMemory(pMain->getSrcBuffer(m_nCam), dstImg.data, nImageSize);
//	ReleaseSRWLockExclusive(&pMain->g_srwlock);
//	dstImg.release();
//}
//else if (dir == 2)	// 270도
//{
//	cv::Mat dstImg;
//	cv::Mat srcImg(w, h, CV_8U, imgPTR);	// Width / Height를 미리 Swap
//	cv::transpose(srcImg, dstImg);
//	cv::flip(dstImg, dstImg, 0);	// 좌우 반전
//	AcquireSRWLockExclusive(&pMain->g_srwlock); // 동기화 시킴 Tkyuha 20211027
//	pMain->copyMemory(pMain->getSrcBuffer(m_nCam), dstImg.data, nImageSize);
//	ReleaseSRWLockExclusive(&pMain->g_srwlock);
//	dstImg.release();
//}
//else if (dir == 3)	// 180도
//{
//	cv::Mat dstImg;
//	cv::Mat srcImg(w, h, CV_8U, imgPTR);	// Width / Height를 미리 Swap
//	cv::flip(srcImg, dstImg, 0);	// 상하 반전
//	cv::flip(dstImg, dstImg, 1);	// 좌우 반전
//	AcquireSRWLockExclusive(&pMain->g_srwlock); // 동기화 시킴 Tkyuha 20211027
//	pMain->copyMemory(pMain->getSrcBuffer(m_nCam), dstImg.data, nImageSize);
//	ReleaseSRWLockExclusive(&pMain->g_srwlock);
//	dstImg.release();
//}
//else
//{
//	AcquireSRWLockExclusive(&pMain->g_srwlock); // 동기화 시킴 Tkyuha 20211027
//	pMain->copyMemory(pMain->getSrcBuffer(m_nCam), imgPTR, nImageSize);
//	ReleaseSRWLockExclusive(&pMain->g_srwlock);
//}
#pragma endregion