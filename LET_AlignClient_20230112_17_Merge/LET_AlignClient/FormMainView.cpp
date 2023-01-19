// FormMainView.cpp : 구현 파일입니다.
//
#include "stdafx.h"
#include "LET_AlignClient.h"
#include "FormMainView.h"
#include "LET_AlignClientDlg.h"
#include "CommPLC.h"
#include "ComPLC_RS.h"
#include "Sequence.h"
#include "LoginDlg.h"
#include <tlhelp32.h>
#include "ImageProcessing/EdgesSubPix.h"
#include "ImageProcessing/MathUtil.h"
#include "ImageProcessing/CircleFit.h"
#include "ImageProcessing/persistence1d.hpp"
#include "ImageProcessing/SGSmooth.hpp"
#include <numeric>
#include "ImageProcessing/BetterBlobDetector.h"

//#define _CIRCLE
#define _NOTCH
#define _NOTCH_SHIFT
//#define _LINEFITTING
//#define _SUBPIXEL_MODE


using namespace p1d;
const static int BLUR_SIZE = 5;
static double CIRCLE_RADIUS = 2.75;

template<typename Out>
void split2(const string& s, char delim, Out result)
{
	stringstream ss(s);

	string item;
	while (getline(ss, item, delim))
		*(result++) = item;
}

vector<string> split2(const string& s, char delim)
{
	vector<string> elems;
	split2(s, delim, back_inserter(elems));
	return elems;
}

HWND m_hHidden = NULL;
BOOL CALLBACK EnumWindowsProc(HWND hwnd, DWORD lParam)
{
	DWORD dwID;
	CString name;
	TCHAR path[_MAX_PATH];

	GetWindowThreadProcessId(hwnd, &dwID);

	if (dwID == (DWORD)lParam)
	{
		GetWindowText(hwnd, path, _MAX_PATH);
		name = path;

		if (name == "Default IME")
		{
			m_hHidden = GetParent(hwnd);
			return FALSE;
		}
	}

	return TRUE;
}

bool ContourAreaSortPredicate(const std::vector<cv::Point> c1, const std::vector<cv::Point> c2)
{
	return cv::arcLength(c1, true) > cv::arcLength(c2, true);
}
//HTK 2022-07-04 RotateCenter Insp 기능 추가
UINT Thread_RotateCenterCalc(void* pParam)
{
	struct _stInspectionPattern* pPattern = (struct _stInspectionPattern*)pParam;
	CLET_AlignClientDlg* pMain = (CLET_AlignClientDlg*)pPattern->pDlg;
	CFormMainView* pView = (CFormMainView*)pMain->m_pForm[FORM_MAIN];

	int sequence = 0;
	int RotateCnt = 0;
	int sign = 1, nPos = 0;
	int nCam = pPattern->nCam;
	int nJob = pPattern->nJob;
	int real_cam = pMain->vt_job_info[nJob].camera_index[nCam];
	int num_of_camera = pMain->vt_job_info[nJob].num_of_camera;
	int viewer = pMain->vt_job_info[nJob].viewer_index[nCam];

	const int SEQ_ROTATE_MOVE_MOVE_REQ	= 0;
	const int SEQ_ROTATE_MOVE_MOVE_END	= 1;
	const int SEQ_ROTATE_MOVE_GRAB		= 2;
	const int SEQ_ROTATE_MOVE_INSP		= 3;
	const int SEQ_ROTATE_MOVE_CALC		= 10;

	BOOL bSuccess = FALSE;
	CPoint ptStart = pPattern->ptStart;
	CPoint ptEnd = pPattern->ptEnd;
	CViewerEx* mViewer = pPattern->mViewer;
	CString str = "Time Out";

	double dbData[3]		 = { 0, 0, 0 };
	double dbStepT			 = pMain->vt_job_info[nJob].model_info.getMachineInfo().getRotateRange();

	// 왼쪽 반시계 방향으로 회전 기준 //220908 Tkyuha 임의의 지점으로 회전 중심 계산 하기 위함
	// dbStartAngle 이 dbEndAngle 보다 작은 값 이어야함
	int  iCalibType          = pMain->vt_job_info[nJob].model_info.getMachineInfo().getCalibrationType();
	double dbStartAngle      = pMain->vt_job_info[nJob].model_info.getMachineInfo().getRotateStartPos();
	double dbEndAngle        = pMain->vt_job_info[nJob].model_info.getMachineInfo().getRotateEndPos();

	int address_movedata	 = pMain->vt_job_info[nJob].plc_addr_info.write_word_start;			// XYT영역
	int address_moverequest	 = pMain->vt_job_info[nJob].plc_addr_info.write_bit_start + 7;		// PC - > PLC Move Request
	int address_movecomplete = pMain->vt_job_info[nJob].plc_addr_info.read_bit_start + 7;		// PLC -> PC  Move Complete
	long lRevisionData[6]	 = { 0, };
	double xyr[2][3] = { {0,0,0},{0,0,0} };

	int nStick = GetTickCount();
	int nEtick = GetTickCount();

	lRevisionData[0] = LOWORD(dbData[AXIS_X] * MOTOR_SCALE);
	lRevisionData[1] = HIWORD(dbData[AXIS_X] * MOTOR_SCALE);
	lRevisionData[2] = LOWORD(dbData[AXIS_Y] * MOTOR_SCALE);
	lRevisionData[3] = HIWORD(dbData[AXIS_Y] * MOTOR_SCALE);
	
	pView->HideErrorMessageBlink();

	auto pDC = mViewer->getOverlayDC();
	CBrush brBase(RGB(255, 255, 0)), * pOldBr;
	CPen pen(PS_SOLID, 10, RGB(255, 0, 0)), * oldPen;

	oldPen = pDC->SelectObject(&pen);
	pOldBr = pDC->SelectObject(&brBase);
	pDC->SelectStockObject(NULL_BRUSH);

	int W = pMain->m_stCamInfo[real_cam].w;
	int H = pMain->m_stCamInfo[real_cam].h;

	while (sequence != -1)
	{
		switch (sequence)
		{
			// 1. 스테이지 -90도 위치 이동 
		case SEQ_ROTATE_MOVE_MOVE_REQ:
		{
			if(iCalibType==2) dbData[AXIS_T] = RotateCnt==0? dbStartAngle: -(360-dbEndAngle + dbStartAngle);  
			else dbData[AXIS_T] = dbStepT * sign;
			
			lRevisionData[4] = LOWORD(dbData[AXIS_T] * MOTOR_SCALE);
			lRevisionData[5] = HIWORD(dbData[AXIS_T] * MOTOR_SCALE);
			g_CommPLC.SetWord(address_movedata, 6, lRevisionData);
			g_CommPLC.SetBit(address_moverequest, TRUE);
			str.Format("PC->PLC MOVE Requst %3.2f", dbData[AXIS_T]);
			::SendMessage(pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			sequence = SEQ_ROTATE_MOVE_MOVE_END;
		}
		break;
		case SEQ_ROTATE_MOVE_MOVE_END: // 완료 체크
		{
			// YCS 2022-07-30 시퀀스 딜레이 추가
			Delay(100);
			BOOL bit = g_CommPLC.GetBit(address_movecomplete);
			if (bit == FALSE) break;
			g_CommPLC.SetBit(address_moverequest, FALSE);
			str.Format("PLC->PC MOVE Complete");
			::SendMessage(pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			::WaitForSingleObject(pMain->m_hDummyEvent, 2000);
			sequence = SEQ_ROTATE_MOVE_GRAB;
		}
		break;
		case SEQ_ROTATE_MOVE_GRAB: // 2. Grab 	
		{
#ifdef _DAHUHA
			AcquireSRWLockExclusive(&pMain->m_pDahuhaCam[real_cam].g_bufsrwlock); // 동기화 시킴 Tkyuha 20211027
#else
			AcquireSRWLockExclusive(&pMain->m_pBaslerCam[real_cam]->g_bufsrwlock); // 동기화 시킴 Tkyuha 20211027
#endif
			pMain->m_bGrabEnd[real_cam] = FALSE;
#ifdef _DAHUHA
			ReleaseSRWLockExclusive(&pMain->m_pDahuhaCam[real_cam].g_bufsrwlock);
#else
			ReleaseSRWLockExclusive(&pMain->m_pBaslerCam[real_cam]->g_bufsrwlock);
#endif
#ifdef _DAHUHA
			if (pMain->m_pDahuhaCam[real_cam].isconnected())
#else
			if (pMain->m_pBaslerCam[real_cam] || pMain->m_pSaperaCam[real_cam].IsOpend())
#endif
			pMain->SendMessage(WM_VIEW_CONTROL, MSG_MD_CAMERA_SINGLE_FRAME, real_cam);
			str.Format("GRAB Start");
			::SendMessage(pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			sequence = SEQ_ROTATE_MOVE_INSP;
		}
		break;
		case SEQ_ROTATE_MOVE_INSP:  // 3. 검사
		{
			// YCS 2022-07-30 시퀀스 딜레이 추가
			Delay(300);
			BOOL bGrabEnd = TRUE;
			if (pMain->m_bGrabEnd[real_cam] != TRUE)		bGrabEnd = FALSE;
			if (bGrabEnd != TRUE) break;

			BOOL bFind2 = TRUE;
			CFindInfo* pInfo;
			
			pInfo = &pMain->GetMatching(nJob).getFindInfo(nCam, nPos);
			pMain->copyMemory(pMain->getProcBuffer(real_cam, 0), pMain->getSrcBuffer(real_cam), W * H);

			// 그랩이미지 디스플레이
			mViewer->OnLoadImageFromPtr(pMain->getProcBuffer(real_cam, 0));
			mViewer->Invalidate();

			if (pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod() == METHOD_CIRCLE)
			{
				BYTE* pImg = pMain->getProcBuffer(real_cam, 0);
				pMain->find_pattern_caliper(pImg, W, H, nJob, nCam, nPos);

				double x = pMain->m_pDlgCaliper->m_Caliper[real_cam][0][0].m_circle_info.x;
				double y = pMain->m_pDlgCaliper->m_Caliper[real_cam][0][0].m_circle_info.y;
				double r = pMain->m_pDlgCaliper->m_Caliper[real_cam][0][0].m_circle_info.r;

				pDC->Ellipse(x - r, y - r, x + r, y + r);

				xyr[RotateCnt % 2][0] = x;
				xyr[RotateCnt % 2][1] = y;
				xyr[RotateCnt % 2][2] = r;
			}
			else if (pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod() ==  METHOD_LINE)
			{
				CRect realRoi = pMain->GetMatching(nJob).getSearchROI(nCam, nPos);
				bool bwhite = pMain->vt_job_info[nJob].model_info.getAlignInfo().getEnableAlignBlackWhiteSelect() ? true : false;
				pMain->GetMatching(nJob).findPattern(pMain->getProcBuffer(real_cam, nPos), nCam, nPos, W, H);

				double posX = pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetXPos();
				double posY = pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetYPos();
				double r = 20;
				pDC->Ellipse(posX - r, posY - r, posX + r, posY + r);

				xyr[RotateCnt % 2][0] = posX;
				xyr[RotateCnt % 2][1] = posY;
				xyr[RotateCnt % 2][2] = r;
			}

			if (pInfo->GetFound() == FIND_ERR)		bFind2 = FALSE;
			if (bFind2 != TRUE)
			{
				sequence = -1;
				str.Format("Mark search Error");
				::SendMessage(pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
				break;
			}

			pInfo = &pMain->GetMatching(nJob).getFindInfo(nCam, nPos);
			double posX = pInfo->GetXPos();
			double posY = pInfo->GetYPos();

			pMain->GetMachine(nJob).setImageOrgX(nCam, nPos, RotateCnt, posX);
			pMain->GetMachine(nJob).setImageOrgY(nCam, nPos, RotateCnt, posY);
			pMain->GetMachine(nJob).PixelToWorld(nCam, nPos, posX, posY, &posX, &posY);
			pMain->GetMachine(nJob).setImageX(nCam, nPos, RotateCnt, posX);
			pMain->GetMachine(nJob).setImageY(nCam, nPos, RotateCnt, posY);

			str.Format("Calc complete %d", RotateCnt);
			::SendMessage(pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			mViewer->Invalidate();
			
			RotateCnt++;
			if (RotateCnt >= 2) sequence = SEQ_ROTATE_MOVE_CALC; // 계산
			else
			{
				sign *= -2;
				sequence = SEQ_ROTATE_MOVE_MOVE_REQ;
				Delay(1000);
			}
		}
		break;
		case SEQ_ROTATE_MOVE_CALC: // 4. 회전 중심 계산 및 판정
		{
			// YCS 2022-07-30 시퀀스 딜레이 추가
			Delay(100);
			if (iCalibType == 2) pMain->GetMachine(nJob).setMotorT(dbEndAngle - dbStartAngle); 
			else pMain->GetMachine(nJob).setMotorT(dbStepT * 2);

			bSuccess = pMain->GetMachine(nJob).halcon_GetTwoPointTheta_Circle(nCam, nPos);

			if (bSuccess != TRUE)
			{
				sequence = -1;
				break;
			}

			double xr = pMain->GetMachine(nJob).getRotateX(nCam, nPos)*-1;
			double yr = pMain->GetMachine(nJob).getRotateY(nCam, nPos)*-1;
			CTime time = CTime::GetCurrentTime();
			CString strTime = _T("");
			strTime.Format(_T("%04d/%02d/%02d  %02d:%02d:%02d =>"), time.GetYear(), time.GetMonth(), time.GetDay(), time.GetHour(), time.GetMinute(), time.GetSecond());

			str.Format("[%s] dbRotateX = %.4f, dbRotateY = %.4f", pMain->vt_job_info[nJob].job_name.c_str(), xr, yr);
			theLog.logmsg(LOG_INSPECTOR, strTime + str);

			int address_okng = pMain->vt_job_info[nJob].plc_addr_info.write_bit_start + 15;
		
			lRevisionData[0] = LOWORD(xr * MOTOR_SCALE);
			lRevisionData[1] = HIWORD(xr * MOTOR_SCALE);
			lRevisionData[2] = LOWORD(yr * MOTOR_SCALE);
			lRevisionData[3] = HIWORD(yr * MOTOR_SCALE);

			g_CommPLC.SetWord(address_movedata + 14, 4, lRevisionData);

			//고객사 요청으로 수정
			//if (fabs(xr)<0.005 && fabs(yr)< 0.005)
			if (fabs(xr) < 0.01 && fabs(yr) < 0.01)
			{
				pView->m_pDlgViewerMain[viewer]->setViewerJudge(TRUE);
				g_CommPLC.SetBit(address_okng, TRUE);
				::SendMessage(pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			}
			else
			{
				pView->m_pDlgViewerMain[viewer]->setViewerJudge(FALSE);
				g_CommPLC.SetBit(address_okng +1, TRUE);
				pView->ShowErrorMessageBlink(str);
			}	

			str.Format("RC:(%.4f,%.4f)mm", xr, yr);

			CFont font, * oldFont;
			font.CreateFont(40, 40, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
				OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
			oldFont = pDC->SelectObject(&font);

			pDC->SetTextColor(COLOR_GREEN);
			pDC->TextOutA(W/2, H/2, str);
			mViewer->Invalidate();

			CString strDate, strImageDir, str_modelID, str_algo;
			strTime.Format(_T("%02d_%02d_%02d"), time.GetHour(), time.GetMinute(), time.GetSecond());
			strDate.Format("%04d%02d%02d", time.GetYear(), time.GetMonth(), time.GetDay());
			strImageDir.Format("%s%s", pMain->m_strImageDir, strDate);
			if (_access(strImageDir, 0) != 0)	CreateDirectory(strImageDir, NULL);

			//모델ID
			str_modelID.Format("%s", pMain->vt_job_info[nJob].model_info.getModelID());
			strImageDir.Format("%s\\%s", strImageDir, str_modelID);
			if (_access(strImageDir, 0) != 0)	CreateDirectory(strImageDir, NULL);

			//JOB이름
			str_algo.Format(_T("%s"), pMain->vt_job_info[nJob].job_name.c_str());
			strImageDir.Format("%s\\%s", strImageDir, str_algo);
			if (_access(strImageDir, 0) != 0)	CreateDirectory(strImageDir, NULL);

			//Rotate 폴더
			strImageDir.Format("%s\\Rotate", strImageDir);
			if (_access(strImageDir, 0) != 0)	CreateDirectory(strImageDir, NULL);

			//OK,NG,Simulation
			if (bSuccess)
			{
				strImageDir.Format("%s\\OK", strImageDir);
				if (_access(strImageDir, 0) != 0)	CreateDirectory(strImageDir, NULL);
			}
			else
			{
				strImageDir.Format("%s\\NG", strImageDir);
				if (_access(strImageDir, 0) != 0)	CreateDirectory(strImageDir, NULL);
			}

			str.Format("%s\\%s_%s_%d_ImgRaw.jpg", strImageDir, strTime, pMain->m_stCamInfo[real_cam].cName, RotateCnt);		// 카메라 이름

			cv::Mat img(H, W, CV_8UC1, pMain->getProcBuffer(real_cam, 0));
			cv::Mat color_img;
			cv::cvtColor(img, color_img, cv::COLOR_GRAY2BGR);

			str_algo.Format("RC:(%.4f,%.4f)mm", xr, yr);
			theLog.logmsg(LOG_CALIBRATION, str_algo);

			double posX, posY;
			pMain->GetMachine(nJob).WorldToPixel(nCam, nPos, xr, yr, &posX, &posY);

			cv::circle(color_img, cv::Point(xyr[0][0], xyr[0][1]), xyr[0][2], cv::Scalar(255, 216, 0), 2, 5);
			cv::circle(color_img, cv::Point(xyr[1][0], xyr[1][1]), xyr[1][2], cv::Scalar(255, 0, 0), 2, 5);
			cv::circle(color_img, cv::Point(posX, posY), xyr[1][2], cv::Scalar(255, 0, 0), 2, 5);
			cv::rectangle(color_img, cv::Point(posX - 4, posY - 4), cv::Point(posX + 4, posY + 4), cv::Scalar(255, 0, 0), 2);

			cv::putText(color_img, std::string(str_algo), cv::Point(color_img.cols / 2, color_img.rows / 2), cv::FONT_ITALIC, 2, cv::Scalar(0, 0, 255), 5);

			cv::imwrite(std::string(str), color_img);
			///////////////////////////
			//KJH2 2022-08-20 Rotata Data
			pMain->write_result_rotate_data(strImageDir, bSuccess, nJob, xr, yr);
			
			font.DeleteObject();

			bSuccess = TRUE;
			sequence = -1;
		}
		break;
		case 100:
			break;
		}

		nEtick = GetTickCount();

		if (nEtick - nStick > 30000)
		{
			// YCS 2022-07-29 회전중심 타임 아웃 메세지 수정 및 프로세스 히스토리 추가
			str.Format("[%s] Rotate Center Time Out", pMain->vt_job_info[nJob].job_name.c_str());
			::SendMessage(pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			pView->ShowErrorMessageBlink(str);
			break; // 30초 타임 아웃
		}

		Delay(50);
	}

	pDC->SelectObject(pOldBr);
	pDC->SelectObject(oldPen);

	pen.DeleteObject();
	brBase.DeleteObject();

	return 0;
}

UINT Thread_RotateCenterCalc2(void* pParam)
{
	struct _stInspectionPattern* pPattern = (struct _stInspectionPattern*)pParam;
	CLET_AlignClientDlg* pMain = (CLET_AlignClientDlg*)pPattern->pDlg;
	CFormMainView* pView = (CFormMainView*)pMain->m_pForm[FORM_MAIN];

	int sequence = 0;
	int RotateCnt = 0;
	int sign = 1, nPos = 0;
	int nCam = pPattern->nCam;
	int nJob = pPattern->nJob;
	int real_cam = pMain->vt_job_info[nJob].camera_index[nCam];
	int num_of_camera = pMain->vt_job_info[nJob].num_of_camera;
	int viewer = pMain->vt_job_info[nJob].viewer_index[nCam];

	const int SEQ_ROTATE_MOVE_MOVE_REQ = 0;
	const int SEQ_ROTATE_MOVE_MOVE_END = 1;
	const int SEQ_ROTATE_MOVE_GRAB = 2;
	const int SEQ_ROTATE_MOVE_INSP = 3;
	const int SEQ_ROTATE_MOVE_CALC = 10;

	BOOL bSuccess = FALSE;
	BOOL bJudge = FALSE;
	CPoint ptStart = pPattern->ptStart;
	CPoint ptEnd = pPattern->ptEnd;
	CViewerEx* mViewer = pPattern->mViewer;
	CString str = "Time Out";

	double dbData[3] = { 0, 0, 0 };
	double dbStepT = pMain->vt_job_info[nJob].model_info.getMachineInfo().getRotateRange();
	int address_movedata = pMain->vt_job_info[nJob].plc_addr_info.write_word_start;			// XYT영역
	int address_moverequest = pMain->vt_job_info[nJob].plc_addr_info.write_bit_start + 7;		// PC - > PLC Move Request
	int address_movecomplete = pMain->vt_job_info[nJob].plc_addr_info.read_bit_start + 7;		// PLC -> PC  Move Complete
	long lRevisionData[6] = { 0, };
	double xyr[2][3] = { {0,0,0},{0,0,0} };

	int nStick = GetTickCount();
	int nEtick = GetTickCount();

	lRevisionData[0] = LOWORD(dbData[AXIS_X] * MOTOR_SCALE);
	lRevisionData[1] = HIWORD(dbData[AXIS_X] * MOTOR_SCALE);
	lRevisionData[2] = LOWORD(dbData[AXIS_Y] * MOTOR_SCALE);
	lRevisionData[3] = HIWORD(dbData[AXIS_Y] * MOTOR_SCALE);

	pView->HideErrorMessageBlink();

	auto pDC = mViewer->getOverlayDC();
	CBrush brBase(RGB(255, 255, 0)), * pOldBr;
	CPen pen(PS_SOLID, 10, RGB(255, 0, 0)), thitpen(PS_SOLID, 2, RGB(0, 255, 0)), * oldPen;

	oldPen = pDC->SelectObject(&pen);
	pOldBr = pDC->SelectObject(&brBase);
	//pDC->SelectStockObject(NULL_BRUSH);

	int W = pMain->m_stCamInfo[real_cam].w;
	int H = pMain->m_stCamInfo[real_cam].h;

	double x1, x2, y1, y2, r1, r2;
	// 왼쪽 반시계 방향으로 회전 기준 //220908 Tkyuha 임의의 지점으로 회전 중심 계산 하기 위함
	// dbStartAngle 이 dbEndAngle 보다 작은 값 이어야함
	int  iCalibType = pMain->vt_job_info[nJob].model_info.getMachineInfo().getCalibrationType();
	double dbStartAngle = pMain->vt_job_info[nJob].model_info.getMachineInfo().getRotateStartPos();
	double dbEndAngle = pMain->vt_job_info[nJob].model_info.getMachineInfo().getRotateEndPos();

	while (sequence != -1)
	{
		switch (sequence)
		{
			// 1. 스테이지 -90도 위치 이동 
		case SEQ_ROTATE_MOVE_MOVE_REQ:
		{
			if (iCalibType == 2) dbData[AXIS_T] = RotateCnt == 0 ? dbStartAngle : -(360 - dbEndAngle + dbStartAngle);
			else dbData[AXIS_T] = dbStepT * sign;

			lRevisionData[4] = LOWORD(dbData[AXIS_T] * MOTOR_SCALE);
			lRevisionData[5] = HIWORD(dbData[AXIS_T] * MOTOR_SCALE);
			g_CommPLC.SetWord(address_movedata, 6, lRevisionData);
			Delay(50);
			g_CommPLC.SetBit(address_moverequest, TRUE);
			Delay(50);
			str.Format("PC->PLC MOVE Requst %3.2f", dbData[AXIS_T]);
			::SendMessage(pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			sequence = SEQ_ROTATE_MOVE_MOVE_END;
		}
		break;
		case SEQ_ROTATE_MOVE_MOVE_END: // 완료 체크
		{
			// YCS 2022-07-30 시퀀스 딜레이 추가
			Delay(100);
			BOOL bit = g_CommPLC.GetBit(address_movecomplete);
			if (bit == FALSE) break;
			Delay(50);
			g_CommPLC.SetBit(address_moverequest, FALSE);
			Delay(50);
			str.Format("PLC->PC MOVE Complete");
			::SendMessage(pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			::WaitForSingleObject(pMain->m_hDummyEvent, 2000);
			sequence = SEQ_ROTATE_MOVE_GRAB;
		}
		break;
		case SEQ_ROTATE_MOVE_GRAB: // 2. Grab 	
		{
#ifdef _DAHUHA
			AcquireSRWLockExclusive(&pMain->m_pDahuhaCam[real_cam].g_bufsrwlock); // 동기화 시킴 Tkyuha 20211027
#else
			AcquireSRWLockExclusive(&pMain->m_pBaslerCam[real_cam]->g_bufsrwlock); // 동기화 시킴 Tkyuha 20211027
#endif
			pMain->m_bGrabEnd[real_cam] = FALSE;
#ifdef _DAHUHA
			ReleaseSRWLockExclusive(&pMain->m_pDahuhaCam[real_cam].g_bufsrwlock);
#else
			ReleaseSRWLockExclusive(&pMain->m_pBaslerCam[real_cam]->g_bufsrwlock);
#endif

#ifdef _DAHUHA
			if (pMain->m_pDahuhaCam[real_cam].isconnected())
#elif _SAPERA
			if (pMain->m_pSaperaCam[real_cam].IsOpend())
#else
			if (pMain->m_pBaslerCam[real_cam])
#endif
				pMain->SendMessage(WM_VIEW_CONTROL, MSG_MD_CAMERA_SINGLE_FRAME, real_cam);
			str.Format("GRAB Start");
			::SendMessage(pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			sequence = SEQ_ROTATE_MOVE_INSP;
		}
		break;
		case SEQ_ROTATE_MOVE_INSP:  // 3. 검사
		{
			
			// YCS 2022-07-30 시퀀스 딜레이 추가
			Delay(300);
			BOOL bGrabEnd = TRUE;
			if (pMain->m_bGrabEnd[real_cam] != TRUE)		bGrabEnd = FALSE;
			if (bGrabEnd != TRUE) break;

			BOOL bFind2 = TRUE;
			CFindInfo* pInfo;

			pView->reset_viewer(nJob);	// KBJ 2022-11-11
			mViewer->ClearOverlayDC();
			mViewer->clearAllFigures();

			pInfo = &pMain->GetMatching(nJob).getFindInfo(nCam, nPos);
			pMain->copyMemory(pMain->getProcBuffer(real_cam, 0), pMain->getSrcBuffer(real_cam), W * H);

			// 그랩이미지 디스플레이
			mViewer->OnLoadImageFromPtr(pMain->getProcBuffer(real_cam, 0));
			mViewer->Invalidate();

			if (pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod() == METHOD_CIRCLE)
			{
				BYTE* pImg = pMain->getProcBuffer(real_cam, 0);

				// KBJ 2022-08-25 Rotate 이진화 추가
				//int lthresh = pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionLowThresh();
				// KBJ 2022-09-05 Rotate 이진화
				int lthresh = pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getRotateC_binary_value();

				cv::Mat img(H, W, CV_8UC1, pImg), bin2;
				cv::threshold(img, bin2, lthresh, 255, CV_THRESH_BINARY);

				if (lthresh != 0)
				{
					pImg = (BYTE*)bin2.data;
				}

				pMain->find_pattern_caliper(pImg, W, H, nJob, nCam, nPos);

				double  x, y, r;

				if (RotateCnt == 0)
				{
					x = x1 = pMain->m_pDlgCaliper->m_Caliper[real_cam][0][0].m_circle_info.x;
					y = y1 = pMain->m_pDlgCaliper->m_Caliper[real_cam][0][0].m_circle_info.y;
					r = r1 = pMain->m_pDlgCaliper->m_Caliper[real_cam][0][0].m_circle_info.r;
				}
				else
				{
					x = x2 = pMain->m_pDlgCaliper->m_Caliper[real_cam][0][0].m_circle_info.x;
					y = y2 = pMain->m_pDlgCaliper->m_Caliper[real_cam][0][0].m_circle_info.y;
					r = r2 = pMain->m_pDlgCaliper->m_Caliper[real_cam][0][0].m_circle_info.r;
				}
				pDC->Ellipse(x - r, y - r, x + r, y + r);
				pDC->SelectObject(&thitpen);

				pDC->MoveTo(x - 10, y);
				pDC->LineTo(x + 10, y);
				pDC->MoveTo(x, y - 10);
				pDC->LineTo(x, y + 10);

				pDC->SelectObject(&pen);

				img.release();
				bin2.release();
			}
			else if (pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod() == METHOD_LINE)
			{
				CRect realRoi = pMain->GetMatching(nJob).getSearchROI(nCam, nPos);
				bool bwhite = pMain->vt_job_info[nJob].model_info.getAlignInfo().getEnableAlignBlackWhiteSelect() ? true : false;
				pMain->GetMatching(nJob).findPattern(pMain->getProcBuffer(real_cam, nPos), nCam, nPos, W, H);

				double posX = pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetXPos();
				double posY = pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetYPos();
				double r = 20;		r1 = r2 = r;

				pDC->Ellipse(posX - r, posY - r, posX + r, posY + r);

				if (RotateCnt == 0)
				{
					x1 = posX;
					y1 = posY;
				}
				else
				{
					x2 = posX;
					y2 = posY;
				}
			}

			if (pInfo->GetFound() == FIND_ERR)		bFind2 = FALSE;
			if (bFind2 != TRUE) // KBJ 2022-11-11
			{
				lRevisionData[0] = LOWORD(0.0 * MOTOR_SCALE);
				lRevisionData[1] = HIWORD(0.0 * MOTOR_SCALE);
				lRevisionData[2] = LOWORD(0.0 * MOTOR_SCALE);
				lRevisionData[3] = HIWORD(0.0 * MOTOR_SCALE);
				address_movedata = pMain->vt_job_info[nJob].plc_addr_info.write_word_start;			// XYT영역
				g_CommPLC.SetWord(address_movedata + 14, 4, lRevisionData);

				// KBJ 2022-10-22 마크 못찾을시 NG 비트 살림
				int address_okng = pMain->vt_job_info[nJob].plc_addr_info.write_bit_start + 15;
				pView->m_pDlgViewerMain[viewer]->setViewerJudge(FALSE);
				g_CommPLC.SetBit(address_okng + 1, TRUE);

				sequence = -1;
				str.Format("Mark search Error");
				::SendMessage(pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
				pView->ShowErrorMessageBlink(str);
				break;
			}

			pInfo = &pMain->GetMatching(nJob).getFindInfo(nCam, nPos);
			double posX = pInfo->GetXPos();
			double posY = pInfo->GetYPos();

			pMain->GetMachine(nJob).setImageOrgX(nCam, nPos, RotateCnt, posX);
			pMain->GetMachine(nJob).setImageOrgY(nCam, nPos, RotateCnt, posY);
			pMain->GetMachine(nJob).PixelToWorld(nCam, nPos, posX, posY, &posX, &posY);
			pMain->GetMachine(nJob).setImageX(nCam, nPos, RotateCnt, posX);
			pMain->GetMachine(nJob).setImageY(nCam, nPos, RotateCnt, posY);

			str.Format("Calc Complete - %d", RotateCnt);
			::SendMessage(pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			mViewer->Invalidate();
			mViewer->SoftDirtyRefresh();

#pragma region 이미지로그 추가

			CString imgMsg, str_modelID, str_JobName, strMsg, imgMsgRaw, str_ImageType;

			CString strFileDir_Image, strFileDir_Image_Raw;
			CString strFileDir_Files;

			SYSTEMTIME time;
			GetLocalTime(&time);
			//KMB 220924 SYSTEM OPTION에 따라 저장이미지 확장자 변경
			if (pMain->vt_system_option[nJob].save_image_type == FALSE)
				str_ImageType = "jpg";
			else
				str_ImageType = "bmp";

			CString Time_str = pMain->m_strResultTime[nJob];
			CString Date_str = pMain->m_strResultDate[nJob];

			// KBJ 2022-11-27 change Rotate Image Name 
			CTime NowTime;
			NowTime = CTime::GetCurrentTime();
			CString Real_Time = "";
			Real_Time.Format("%02d_%02d_%02d", NowTime.GetHour(), NowTime.GetMinute(), NowTime.GetSecond());

			if (Time_str == "" || Date_str == "")
			{
				pMain->m_strResultDate[nJob].Format("%04d%02d%02d", NowTime.GetYear(), NowTime.GetMonth(), NowTime.GetDay());
				pMain->m_strResultTime[nJob].Format("%02d_%02d_%02d", NowTime.GetHour(), NowTime.GetMinute(), NowTime.GetSecond());

				Time_str = pMain->m_strResultTime[nJob];
				Date_str = pMain->m_strResultDate[nJob];
			}

			CString Cell_Name = pMain->vt_job_info[nJob].get_main_object_id();

			if (strlen(Cell_Name) < 10)
			{
				Cell_Name.Format("TEST_%s", Time_str);
			}

			///////////////////////////////////// 이미지 저장 폴더
			///  D:\\ Result \\ Image \\ 날짜 \\ 모델ID \\ JOB이름 \\ Trace \\ 

			//Image 폴더 확인 후 생성
			if (_access(pMain->m_strImageDir, 0) != 0)	CreateDirectory(pMain->m_strImageDir, NULL);

			//날짜
			strFileDir_Image.Format("%s%s", pMain->m_strImageDir, Date_str);
			if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);

			//모델ID
			str_modelID.Format("%s", pMain->vt_job_info[nJob].model_info.getModelID());
			strFileDir_Image.Format("%s\\%s", strFileDir_Image, str_modelID);
			if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);

			//JOB이름
			str_JobName.Format("%s", pMain->vt_job_info[nJob].job_name.c_str());
			strFileDir_Image.Format("%s\\%s", strFileDir_Image, str_JobName);
			if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);

			//Rotate 폴더 확인 후 생성
			strFileDir_Image.Format("%s\\Rotate\\", strFileDir_Image);
			if (_access(strFileDir_Image, 0) != 0)		CreateDirectory(strFileDir_Image, NULL);

			//PanelID
			strFileDir_Image.Format("%s\\%s", strFileDir_Image, Cell_Name);
			if (_access(strFileDir_Image, 0) != 0) CreateDirectory(strFileDir_Image, NULL);

			imgMsg.Format("%s\\Rotate_%s_%s_%s_Result%d_Img.%s", strFileDir_Image, Cell_Name, Time_str, Real_Time, RotateCnt + 1, str_ImageType);
			imgMsgRaw.Format("%s\\Rotate_%s_%s_%s_Result%d_ImgRaw.%s", strFileDir_Image, Cell_Name, Time_str, Real_Time, RotateCnt + 1, str_ImageType);

			if (RotateCnt == 0)
			{
				int real_cam = pMain->vt_job_info[nJob].camera_index[0];
				cv::Mat imgRaw(H, W, CV_8UC1, pMain->getProcBuffer(real_cam, 0));
				cv::imwrite(std::string(imgMsgRaw), imgRaw); // Kbj 2022-02-23 Trace RawImage save

				mViewer->saveScreenCapture(imgMsg);
			}
#pragma endregion

			RotateCnt++;
			if (RotateCnt >= 2) sequence = SEQ_ROTATE_MOVE_CALC; // 계산
			else
			{
				sign *= -2;
				sequence = SEQ_ROTATE_MOVE_MOVE_REQ;
				Delay(1000);
			}
		}
		break;
		case SEQ_ROTATE_MOVE_CALC: // 4. 회전 중심 계산 및 판정
		{
			// YCS 2022-07-30 시퀀스 딜레이 추가
			Delay(100);

			if (iCalibType == 2) pMain->GetMachine(nJob).setMotorT(dbEndAngle - dbStartAngle);
			else pMain->GetMachine(nJob).setMotorT(dbStepT * 2);
			bSuccess = pMain->GetMachine(nJob).halcon_GetTwoPointTheta_Circle(nCam, nPos);

			if (bSuccess != TRUE)
			{
				sequence = -1;
				break;
			}

			double xr = pMain->GetMachine(nJob).getRotateX(nCam, nPos) * -1;
			double yr = pMain->GetMachine(nJob).getRotateY(nCam, nPos) * -1;
			CTime time = CTime::GetCurrentTime();
			CString strTime = _T("");
			strTime.Format(_T("%04d/%02d/%02d  %02d:%02d:%02d =>"), time.GetYear(), time.GetMonth(), time.GetDay(), time.GetHour(), time.GetMinute(), time.GetSecond());

			str.Format("Cam %d  dbRotateX = %.4f, dbRotateY = %.4f", real_cam + 1, xr, yr);
			theLog.logmsg(LOG_INSPECTOR, strTime + str);

			int address_okng = pMain->vt_job_info[nJob].plc_addr_info.write_bit_start + 15;

			lRevisionData[0] = LOWORD(xr * MOTOR_SCALE);
			lRevisionData[1] = HIWORD(xr * MOTOR_SCALE);
			lRevisionData[2] = LOWORD(yr * MOTOR_SCALE);
			lRevisionData[3] = HIWORD(yr * MOTOR_SCALE);

			g_CommPLC.SetWord(address_movedata + 14, 4, lRevisionData);

			// KBJ 2022-09-05 Rotate 판정스펙 파라미터로 추가.
			double dbSpec = pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getRotateC_Spec();
			//고객사 요청으로 수정
			//if (fabs(xr)<0.005 && fabs(yr)< 0.005)
			//if (fabs(xr) < 0.01 && fabs(yr) < 0.01)
			if (fabs(xr) < dbSpec && fabs(yr) < dbSpec)
			{
				pView->m_pDlgViewerMain[viewer]->setViewerJudge(TRUE);
				g_CommPLC.SetBit(address_okng, TRUE);
				::SendMessage(pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
				bJudge = TRUE;
			}
			else
			{
				pView->m_pDlgViewerMain[viewer]->setViewerJudge(FALSE);
				g_CommPLC.SetBit(address_okng + 1, TRUE);
				pView->ShowErrorMessageBlink(str);
				bJudge = FALSE;

			}

			CFont font, * oldFont;
			font.CreateFont(40, 40, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
				OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
			oldFont = pDC->SelectObject(&font);
			if (bJudge == TRUE)	pDC->SetTextColor(COLOR_LIME);
			else				pDC->SetTextColor(COLOR_RED);
			str.Format("RX    :%.4fmm\nRY    :%.4fmm\nSpec  :%.4fmm", xr, yr, dbSpec);
			pDC->TextOutA(W / 2 - 200, H / 2 - 60, str);

			/*str.Format("RY    :%.4fmm", yr);
			pDC->TextOutA(W / 2 - 200, H / 2, str);

			str.Format("Spec :%.4fmm", dbSpec);
			pDC->TextOutA(W / 2 - 200, H / 2 + 60, str);*/

			mViewer->Invalidate();
			mViewer->DirtyRefresh();
#pragma region 이미지로그 추가

			CString imgMsg, str_modelID, str_JobName, strMsg, imgMsgRaw, str_ImageType;

			CString strFileDir_Image, strFileDir_Image_Raw;
			CString strFileDir_Files;

			SYSTEMTIME times;
			GetLocalTime(&times);
			//KMB 220924 SYSTEM OPTION에 따라 저장이미지 확장자 변경
			if (pMain->vt_system_option[nJob].save_image_type == FALSE)
				str_ImageType = "jpg";
			else
				str_ImageType = "bmp";

			CString Time_str = pMain->m_strResultTime[nJob];
			CString Date_str = pMain->m_strResultDate[nJob];

			// KBJ 2022-11-27 change Rotate Image Name 
			CTime NowTime;
			NowTime = CTime::GetCurrentTime();
			CString Real_Time = "";
			Real_Time.Format("%02d_%02d_%02d", NowTime.GetHour(), NowTime.GetMinute(), NowTime.GetSecond());

			if (Time_str == "" || Date_str == "")
			{
				pMain->m_strResultDate[nJob].Format("%04d%02d%02d", NowTime.GetYear(), NowTime.GetMonth(), NowTime.GetDay());
				pMain->m_strResultTime[nJob].Format("%02d_%02d_%02d", NowTime.GetHour(), NowTime.GetMinute(), NowTime.GetSecond());

				Time_str = pMain->m_strResultTime[nJob];
				Date_str = pMain->m_strResultDate[nJob];
			}

			CString Cell_Name = pMain->vt_job_info[nJob].get_main_object_id();

			if (strlen(Cell_Name) < 10)
			{
				Cell_Name.Format("TEST_%s", Time_str);
			}

			///////////////////////////////////// 이미지 저장 폴더
			///  D:\\ Result \\ Image \\ 날짜 \\ 모델ID \\ JOB이름 \\ Trace \\ 

			//Image 폴더 확인 후 생성
			if (_access(pMain->m_strImageDir, 0) != 0)	CreateDirectory(pMain->m_strImageDir, NULL);

			//날짜
			strFileDir_Image.Format("%s%s", pMain->m_strImageDir, Date_str);
			if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);

			//모델ID
			str_modelID.Format("%s", pMain->vt_job_info[nJob].model_info.getModelID());
			strFileDir_Image.Format("%s\\%s", strFileDir_Image, str_modelID);
			if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);

			//JOB이름
			str_JobName.Format("%s", pMain->vt_job_info[nJob].job_name.c_str());
			strFileDir_Image.Format("%s\\%s", strFileDir_Image, str_JobName);
			if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);

			//Rotate 폴더 확인 후 생성
			strFileDir_Image.Format("%s\\Rotate", strFileDir_Image);
			if (_access(strFileDir_Image, 0) != 0)		CreateDirectory(strFileDir_Image, NULL);

			//PanelID
			strFileDir_Image.Format("%s\\%s", strFileDir_Image, Cell_Name);
			if (_access(strFileDir_Image, 0) != 0) CreateDirectory(strFileDir_Image, NULL);

			imgMsg.Format("%s\\Rotate_%s_%s_%s_Result%d_Img.%s", strFileDir_Image, Cell_Name, Time_str, Real_Time, RotateCnt, str_ImageType);
			imgMsgRaw.Format("%s\\Rotate_%s_%s_%s_Result%d_ImgRaw.%s", strFileDir_Image, Cell_Name, Time_str, Real_Time, RotateCnt, str_ImageType);

			int real_cam = pMain->vt_job_info[nJob].camera_index[0];
			cv::Mat imgRaw(H, W, CV_8UC1, pMain->getProcBuffer(real_cam, 0));
			cv::imwrite(std::string(imgMsgRaw), imgRaw); // Kbj 2022-02-23 Trace RawImage save

			mViewer->saveScreenCapture(imgMsg);

#pragma endregion

			font.DeleteObject();
			//KJH2 2022-08-20 Rotata Data
			pMain->write_result_rotate_data(strFileDir_Image, bJudge, nJob, xr, yr);
			/*if(bJudge == FALSE) */pMain->write_result_rotate_data_PC1(strFileDir_Image, bJudge, nJob, xr, yr);

			bSuccess = TRUE;
			sequence = -1;
		}
		break;
		case 100:
			break;
		}

		nEtick = GetTickCount();

		if (nEtick - nStick > 30000)
		{
			sequence = -1;
			// YCS 2022-07-29 회전중심 타임 아웃 메세지 수정 및 프로세스 히스토리 추가
			str.Format("[%s] Rotate Center Time Out", pMain->vt_job_info[nJob].job_name.c_str());
			::SendMessage(pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
			pView->ShowErrorMessageBlink(str);
			break; // 30초 타임 아웃
		}

		Delay(50);
	}

	pDC->SelectObject(pOldBr);
	pDC->SelectObject(oldPen);

	pen.DeleteObject();
	thitpen.DeleteObject();
	brBase.DeleteObject();

	return 0;
}

UINT Thread_SubInspection(void* pParam)
{
	struct _stInspectionPattern* pPattern = (struct _stInspectionPattern*)pParam;
	CLET_AlignClientDlg* pMain = (CLET_AlignClientDlg*)pPattern->pDlg;
	CFormMainView* pView = (CFormMainView*)pMain->m_pForm[FORM_MAIN];

	int nCam = pPattern->nCam;
	int nJob = pPattern->nJob;
	int real_cam = pMain->vt_job_info[nJob].camera_index[nCam];
	CPoint ptStart = pPattern->ptStart;
	CPoint ptEnd = pPattern->ptEnd;
	CViewerEx* mViewer = pPattern->mViewer;

	/////////////////////////////  검사용으로 Steel 위치 계산 - 강제로 일정 위치 계산
	if (0)
	{
		double xres = MAX(0.00001, pMain->GetMachine(nJob).getCameraResolutionX(nCam, 0));
		double distanceT = 0.2 / xres;
		cv::Point2f rC;
		pView->m_vecInterSectionPoint.clear();
		for (int i = 0; i < pMain->m_vtTraceData.size() - 1; i++)
		{
			rC = Equation_of_Lines(pMain->m_vtTraceData[i], pMain->m_vtTraceData[i + 1], distanceT, false, true);
			pView->m_vecInterSectionPoint.push_back(rC);
		}
		rC = cv::Point2f(pMain->m_vtTraceData[pMain->m_vtTraceData.size() - 1].x, pMain->m_vtTraceData[pMain->m_vtTraceData.size() - 1].y + distanceT);
		pView->m_vecInterSectionPoint.push_back(rC);
	}
	else
	{
		int W = pMain->m_stCamInfo[nCam].w;
		int H = pMain->m_stCamInfo[nCam].h;
		//HTK 2022-03-21 MP,PN 간격이 가장 큰 각도 계산
		double Angle = 0;
		int method = pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod();
		//KJH 2022-08-04 Camera Buffer 변경
		cv::Mat img(H, W, CV_8UC1, pMain->getProcBuffer(nCam, 2));

		if (method == METHOD_NOTCH) pView->InspNotchMetalEdgeDetection(nJob, nCam, img, ptStart, ptEnd, mViewer);
		else if (method == METHOD_CIRCLE)
		{
			pView->InspCircleMetalEdgeDetection(nJob, nCam, img, mViewer);
			//pView->InspCircleBMEdgeDetection(nJob, nCam, img, mViewer);
			Angle = pView->fnSearchWideAngle(cv::Point2f(pView->m_ptCircleCenter_MP.x, pView->m_ptCircleCenter_MP.y), cv::Point2f(pMain->m_ELB_TraceResult.m_ELB_ResultXY.x, pMain->m_ELB_TraceResult.m_ELB_ResultXY.y));
		}			
		
		img.release();		
	}

	SetEvent(pMain->m_hMetalSearchEvent);

	return 0;
}
//UINT Thread_SubCCInspection(void* pParam)
UINT Thread_SubBMInspection(void* pParam)
{
	struct _stInspectionPattern* pPattern = (struct _stInspectionPattern*)pParam;
	CLET_AlignClientDlg* pMain = (CLET_AlignClientDlg*)pPattern->pDlg;
	CFormMainView* pView = (CFormMainView*)pMain->m_pForm[FORM_MAIN];

	int nCam = pPattern->nCam;
	int nJob = pPattern->nJob;
	int bevent = pPattern->bEvent;
	int real_cam = pMain->vt_job_info[nJob].camera_index[nCam];
	CPoint ptStart = pPattern->ptStart;
	CPoint ptEnd = pPattern->ptEnd;
	CViewerEx* mViewer = pPattern->mViewer;

	int W = pMain->m_stCamInfo[nCam].w;
	int H = pMain->m_stCamInfo[nCam].h;

	int method = pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod();
	//KJH 2022-08-04 Camera Buffer 변경
	cv::Mat img(H, W, CV_8UC1, pMain->getProcBuffer(nCam, 2));

	if (method == METHOD_CIRCLE)
	{
		if(pMain->m_pDlgCaliper->m_Caliper[nCam][0][C_CALIPER_POS_2].getCircleCaliper())
			pView->InspCaliperCircleBMEdgeDetection(nJob, nCam, img, mViewer);
		else
			pView->InspCircleBMEdgeDetection(nJob, nCam, img, mViewer);
	}

	img.release();
	
	if(bevent)	SetEvent(pMain->m_hMetalSearchEvent);
	return 0;
}

UINT Thread_MainViewerDisplay(void* pParam)
{
	CString str;
	int nRet = 0;
	int nCamNum = 0;
	CFormMainView* pView = (CFormMainView*)pParam;
	theLog.logmsg(LOG_PROCESS, "Thread_ViewerDisplay(), MainViewer, Thread Start");

	pView->m_bThreadMainDisplayEndFlag = TRUE;

	BOOL bDisplay = FALSE;
	int v = 0, cam = 1;
	int vcount = pView->m_pMain->m_nViewX_Num * pView->m_pMain->m_nViewY_Num;
	int job = pView->m_nCurrentSelMainView;

	if (job > 0)
	{
		vcount = pView->m_pMain->vt_job_info[job-1].main_view_count_x * pView->m_pMain->vt_job_info[job-1].main_view_count_y;
		cam = pView->m_pMain->vt_job_info[job-1].num_of_camera;
	}

	if (vcount <= 0) return -1;	

	int viewer = 0, viewID = 0;
	CViewerEx* pDispView=NULL;

	while (pView->m_bThreadMainDisplayEndFlag && pView->m_pMain->m_bProgramEnd != TRUE)
	{
		for (v = 0; v < vcount; v++) // View 수
		{

			if (job == 0)
			{
				pDispView = &(pView->m_pDlgViewerMain[v]->GetViewer());
				viewID = v;
			}
			else
			{
				viewer = pView->m_pMain->vt_job_info[job - 1].main_viewer_index[v];
				pDispView = &(pView->m_pDlgViewerMain[viewer]->GetViewer());
				viewID = pView->m_pDlgViewerMain[viewer]->getViewID();
			}

			nCamNum = pView->m_pMain->vt_viewer_info[viewID].camera_index;
			bDisplay = pView->m_pMain->vt_viewer_info[viewID].camera_status;
			int W = pView->m_pMain->m_stCamInfo[nCamNum].w;
			int H = pView->m_pMain->m_stCamInfo[nCamNum].h;
#ifdef _DAHUHA
			if (bDisplay && pView->m_pMain->m_pDahuhaCam[nCamNum].isconnected() && pView->m_pMain->m_pDahuhaCam[nCamNum].isgrabbing())
#else
			if (bDisplay && ((pView->m_pMain->m_pBaslerCam[nCamNum] && pView->m_pMain->m_pBaslerCam[nCamNum]->IsGrabContinuous()) ||
				(pView->m_pMain->m_pSaperaCam[nCamNum].IsOpend() && pView->m_pMain->m_pSaperaCam[nCamNum].IsGrabing())))
#endif
			{
				if (pView->m_pMain->getSrcBuffer(nCamNum) != NULL && pDispView->GetWidth() * pDispView->GetHeight() == W * H)
					pDispView->OnLoadImageFromPtr(pView->m_pMain->getSrcBuffer(nCamNum));
			}
			::WaitForSingleObject(pView->m_pMain->m_hDummyEvent, 10);
		}

		// KBJ 2022-11-19 WaitForSingleObject 100 -> 300
		//::WaitForSingleObject(pView->m_pMain->m_hDummyEvent, 100);
		::WaitForSingleObject(pView->m_pMain->m_hDummyEvent, 300);
	}

	pView->m_bThreadMainDisplayEndFlag = FALSE;


	return 0;
}

UINT Thread_InspTraceImageSave(void* pParam)
{
	struct _stInspectionPattern* pPattern = (struct _stInspectionPattern*)pParam;

	CLET_AlignClientDlg* pMain = (CLET_AlignClientDlg*)pPattern->pDlg;
	CFormMainView* pView = (CFormMainView*)pMain->m_pForm[FORM_MAIN];

	CString str_ImageType;
	int nCam = pPattern->nCam;
	int nJob = pPattern->nJob;
	int W = pMain->m_stCamInfo[nCam].w;
	int H = pMain->m_stCamInfo[nCam].h;

	//KJH 2021-09-02 ?????TRACE 이미지로그 없네??????
	cv::Mat img2(H, W, CV_8UC1, pMain->getSrcBuffer(nCam));

	SYSTEMTIME time;
	::GetLocalTime(&time);
	//KMB 220924 SYSTEM OPTION에 따라 저장이미지 확장자 변경
	if (pMain->vt_system_option[nJob].save_image_type == FALSE)
		str_ImageType = "jpg";
	else
		str_ImageType = "bmp";

	CString str_file_path;
	str_file_path.Format("%sTrace\\", pView->m_pMain->m_strResultDir);
	::CreateDirectory(str_file_path, 0);
	str_file_path.Format("%sTrace\\%04d%02d%02d\\", pView->m_pMain->m_strResultDir, time.wYear, time.wMonth, time.wDay);
	::CreateDirectory(str_file_path, 0);
	str_file_path.Format("%sTrace\\%04d%02d%02d\\TRACE_%02d%02d%02d.%s", pView->m_pMain->m_strResultDir, time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond,str_ImageType);

	cv::imwrite(std::string(str_file_path), img2);

	img2.release();

	return 0;
}

UINT Thread_PLCSignalCheck(void* pParam)
{
	CString str;

	CFormMainView* pMainFrame = (CFormMainView*)pParam;
	CLET_AlignClientDlg* pFrame = (CLET_AlignClientDlg*)theApp.m_pFrame;

	BOOL bAlive = FALSE;
	int nCount = 0;

	long pDataSize[4] = { 0, };

	Sleep(1000);

	try {
		while (!pFrame->m_bProgramEnd && pFrame->getPLCConnect())
		{
			g_CommPLC.GetTotalBitData(pFrame->getStartReadBitAddr(), pFrame->getSizeReadBit(), pFrame->m_nBitData);

			if (pFrame->m_bAutoStart)
			{
				for (int nJob = 0; nJob < pFrame->vt_job_info.size(); nJob++)  ///20210107임시 
				{
					switch (pFrame->vt_job_info[nJob].algo_method) 
					{
					case CLIENT_TYPE_ALIGN:
					case CLIENT_TYPE_1CAM_1SHOT_ALIGN:
					case CLIENT_TYPE_1CAM_1SHOT_ALIGN_WITH_SCRATCH_INSP:
					case CLIENT_TYPE_1CAM_2SHOT_ALIGN:
					case CLIENT_TYPE_2CAM_EACH_2SHOT_ALIGN:
					case CLIENT_TYPE_4CAM_1SHOT_ALIGN:
					case CLIENT_TYPE_1CAM_2POS_REFERENCE:
					case CLIENT_TYPE_NOZZLE_ALIGN:
						pMainFrame->fnProcess_ProcessAlignAlgorithm(nJob);
						break;
					case CLIENT_TYPE_PANEL_EXIST_INSP:
					case CLIENT_TYPE_ASSEMBLE_INSP:
					case CLIENT_TYPE_FILM_INSP:
						pMainFrame->fnProcess_ProcessInspAlgorithm(nJob);
						break;
					case CLIENT_TYPE_PCB_DISTANCE_INSP:
						pMainFrame->fnProcess_ProcessBendingInsp(nJob);
						break;
					case CLIENT_TYPE_ELB_CENTER_ALIGN:
						pMainFrame->fnProcess_ProcessCenterAlignInspAlgorithm(nJob);
						//HTK 2022-07-04 RotateCenter Insp 기능 추가
						pMainFrame->fnProcess_ProcessRotateCenterPointCheck(nJob);
						break;
					case CLIENT_TYPE_SCAN_INSP:
						pMainFrame->fnProcess_ProcessScanInspAlgorithm(nJob);
						break;
					case CLIENT_TYPE_NOZZLE_SIDE_VIEW:
						pMainFrame->fnProcess_ProcessSideViewAlgorithm(nJob);
						break;
					case CLIENT_TYPE_CENTER_NOZZLE_ALIGN: // 20211205 Tkyuha 신규 노즐 얼라인
						pMainFrame->fnProcessCenterNozzleAlignAlgorithm(nJob);
						pMainFrame->fnProcessTeach_XYZ_StatusMeasure(nJob);
						break;
					case CLIENT_TYPE_CENTER_SIDE_YGAP: // 20211205 Tkyuha 스테이지 와 노즐 높이 측정
						//pMainFrame->fnProcessCenterNozzleGapMeasure(nJob);
						pMainFrame->fnProcessTeach_Gap_StatusMeasure(nJob); // 티칭용 별도 계산
						break;
					case CLIENT_TYPE_1CAM_1SHOT_FILM:	//KJH 2022-05-11 Film Align Grab bit 관련 Process추가
						pMainFrame->fnProcess_ProcessFilmAlignAlgorithm(nJob);
						break;
					case CLIENT_TYPE_1CAM_4POS_ROBOT:
						pMainFrame->fnProcess_ProcessRobotAlignAlgorithm(nJob);
						break;
					case CLIENT_TYPE_ONLY_VIEW:
						pMainFrame->fnProcess_ProcessLive(nJob);
						break;
					default:
						pMainFrame->fnProcess_ProcessInspAlgorithm(nJob);
						break;
					}

					if(nJob == CLIENT_TYPE_ALIGN)
						pMainFrame->fnProcess_AlignOffsetSendProcess(nJob);
				}
			}
			
			// KBJ 2022-10-16 Auto모드가 아닐때에도 동작하도록 수정.
			for (int nJob = 0; nJob < pFrame->vt_job_info.size(); nJob++)  ///20210107임시 
			{
				pMainFrame->fnProcess_BitResetProcess(nJob); // Bit Reset 기능 추가 Tkyuha 20211124
			}

			// KBJ 2022-09-22 모델 프로세스 2022버젼 추가! 테스트 중
			//KJH 2021-09-13 Model 관련 협의 없다 우선 제거
			if( pFrame->m_system_option_common.use_plc_model_process == TRUE ) pMainFrame->fnProcess_ModelProcess_2022();

			pFrame->CompareMemoryAddr();

			::WaitForSingleObject(pFrame->m_hDummyEvent, 50);
		}
	}
	catch (...)
	{
		TRACE("pFrame->m_bThreadPLCSignalCheckEnd = TRUE;\n");
	}

	TRACE("pFrame->m_bThreadPLCSignalCheckEnd = TRUE;\n");
	pFrame->m_bThreadPLCSignalCheckEnd = TRUE;

	return 0;
}
// KBJ 2022-07-12 Conveyer Align 이전 이미지 옴기기 기능 추가
UINT Thread_Load_Before_Image(void* pParam)
{
	struct _stLoadBeforeImage* pPattern = (struct _stLoadBeforeImage*)pParam;
	CLET_AlignClientDlg* pMain = (CLET_AlignClientDlg*)pPattern->pDlg;
	int nJob = pPattern->nJob;
	int nIndex1 = pPattern->nViewer_index1;
	int nIndex2 = pPattern->nViewer_index2;

	// Viewer
	CFormMainView* pFormMain = (CFormMainView*)pMain->m_pForm[FORM_MAIN];
	int nViewer1 = pMain->vt_job_info[nJob].viewer_index[nIndex1];
	int nViewer2 = pMain->vt_job_info[nJob].viewer_index[nIndex2];
	CViewerEx* pViewer2 = &pFormMain->m_pDlgViewerMain[nViewer2]->GetViewer();

	// Viewer1 PanelName
	CString strPanelName;
	strPanelName.Format("%s", pFormMain->m_pDlgViewerMain[nViewer1]->m_GridInfo.GetItemText(0, 1));
	if (strPanelName == "") return 0;

	// Viewer1 bJudge
	BOOL bJudge = TRUE;
	if (pFormMain->m_pDlgViewerMain[nViewer1]->m_GridInfo.GetItemBkColour(0, 1) == COLOR_GREEN)	   bJudge = TRUE;
	else if (pFormMain->m_pDlgViewerMain[nViewer1]->m_GridInfo.GetItemBkColour(0, 1) == COLOR_RED) bJudge = FALSE;
	else return 0 ;

	// Viewer1 realcam
	int realCam = pMain->vt_viewer_info[nViewer1].camera_index;

	// Find Panel Image
	if (pMain->Load_Align_PanelImage(nJob, pViewer2, strPanelName, bJudge, realCam) == TRUE)
	{
		pFormMain->m_pDlgViewerMain[nViewer2]->m_GridInfo.SetItemText(0,1,strPanelName);
		if (bJudge == TRUE)	pFormMain->m_pDlgViewerMain[nViewer2]->m_GridInfo.SetItemBkColour(0, 1, COLOR_GREEN);
		else				pFormMain->m_pDlgViewerMain[nViewer2]->m_GridInfo.SetItemBkColour(0, 1, COLOR_RED);
		pFormMain->m_pDlgViewerMain[nViewer2]->m_GridInfo.RedrawRow(0);

		CString str;
		str.Format("Load_Align_PanelImage");
		pMain->add_process_history(str);
	}

	return 0;
}

// KBJ 2022-07-22 Conveyer 이전 화면 Load --> Caputre 변경
UINT Thread_Cature_Before_Image(void* pParam)
{
	struct _stLoadBeforeImage* pPattern = (struct _stLoadBeforeImage*)pParam;
	CLET_AlignClientDlg* pMain = (CLET_AlignClientDlg*)pPattern->pDlg;
	int nJob = pPattern->nJob;
	int nIndex1 = pPattern->nViewer_index1;
	int nIndex2 = pPattern->nViewer_index2;

	// Viewer
	CFormMainView* pFormMain = (CFormMainView*)pMain->m_pForm[FORM_MAIN];
	int nViewer1 = pMain->vt_job_info[nJob].viewer_index[nIndex1];
	int nViewer2 = pMain->vt_job_info[nJob].viewer_index[nIndex2];
	CViewerEx* pViewer1 = &pFormMain->m_pDlgViewerMain[nViewer1]->GetViewer();
	CViewerEx* pViewer2 = &pFormMain->m_pDlgViewerMain[nViewer2]->GetViewer();

	// Viewer1 PanelName
	CString strPanelName;
	strPanelName.Format("%s", pFormMain->m_pDlgViewerMain[nViewer1]->m_GridInfo.GetItemText(0, 1));
	if (strPanelName == "") return 0;

	// Viewer1 bJudge
	BOOL bJudge = TRUE;
	if (pFormMain->m_pDlgViewerMain[nViewer1]->m_GridInfo.GetItemBkColour(0, 1) == COLOR_GREEN)	   bJudge = TRUE;
	else if (pFormMain->m_pDlgViewerMain[nViewer1]->m_GridInfo.GetItemBkColour(0, 1) == COLOR_RED) bJudge = FALSE;
	else return 0;

	// Move Capture Image
	//pViewer1->MoveScreenCapture(pViewer2); // 221107 Tkyuha 디버깅 확인 해야함

	// Draw Before Information
	pFormMain->m_pDlgViewerMain[nViewer2]->m_GridInfo.SetItemText(0, 1, strPanelName);
	if (bJudge == TRUE)	pFormMain->m_pDlgViewerMain[nViewer2]->m_GridInfo.SetItemBkColour(0, 1, COLOR_GREEN);
	else				pFormMain->m_pDlgViewerMain[nViewer2]->m_GridInfo.SetItemBkColour(0, 1, COLOR_RED);
	pFormMain->m_pDlgViewerMain[nViewer2]->m_GridInfo.RedrawRow(0);

	return 0;
}

IMPLEMENT_DYNCREATE(CFormMainView, CFormView)

CFormMainView::CFormMainView() : CFormView(CFormMainView::IDD)
{
	m_pMain = NULL;

	for (int i = 0; i < MAX_VIEWER; i++)
	{
		m_pDlgViewerMain[i] = NULL;
	}

	for (int i = 0; i < MAX_JOB; i++)
	{
		m_bProcessStart[i] = FALSE;
		m_bSubProcessStart[i] = FALSE;
		m_bProcessEnd[i] = FALSE;
		m_bModelProcessStart[i] = FALSE;
		//HSJ 검사 길이 멤버변수 초기화 추가
		m_InspectionDistance[i] = 0.0;
		m_bResetBitFlag[i] = FALSE;
		m_bSendOffsetFlag[i] = FALSE;
		m_bResetBit2PosFlag[i] = FALSE;

		m_clockProc_elapsed_time[i] = 0;
		m_clockProc_elapsed_time[MAX_JOB + i] = 0;
		m_clockProcStartTime[i] = 0;
		m_clockProcStartTime[MAX_JOB + i] = 0;

		m_nResetBitFlag_Count[i] = 0;

		m_bScratch[i] = FALSE;
	}

	m_bThreadMainDisplayEndFlag = FALSE;
	m_bPLCTimeStart = FALSE;

	m_nNotchLeftStartCount = 0;
	m_nNotchRightEndCount = 0;
	m_dLeftNotchLength = 12.7;
	m_dRightNotchLength = 1.2;
	m_dDopoType = 0;
	m_dLineNum = 0;
	m_dDummyLineNum = 0;
	m_iSaveImageTypeNum = 0;
	m_dAccLineLength = 0;
	m_dDecLineLength = 0;
	m_dCircleRadius_MP = 0;
	m_dCircleDistance_X_MP = 0;
	m_dCircleRadius_CC = 0;
	m_dCircleInBright = 255;
	m_ptCircleCenter_MP = CPoint(0, 0);
	m_ptCircleCenter_CC = CPoint(0, 0);

	//20210923 Tkyuha Circle에서만 사용 하는 파라미터 
	//원을 찾을때 제외 영역을 메탈 찾는 경우에 적용 하기 위함 초기화
	m_Index_start = -1;
	m_Index_end = -1;
	m_Index_bsplit = false; 
	m_IndexMaxWidth_Angle = 0;
	m_dInclination = 0.0; 			// Adaptive Dispensing 사용 기울기 		20220103 Tkyuha
	m_dADispesing_StartAngle = 0; 	// Adaptive Dispensing 사용 시작 각도 	20220103 Tkyuha
	m_dADispesing_EndAngle = 360; 	// Adaptive Dispensing 사용 종료 각도 	20220103 Tkyuha	

	//HTK 2022-04-04 Line 도포시 Z Up Pos 이후 사선도포용 Offset
	m_posLineY_Move = 0;
	m_distLineY_Move = 0;
	m_nLackOfInk_CinkCount = 0;
	m_nCount_LackOfCink_NG = 0;
	memset(m_bLackofInkCheckFlag,0,sizeof(bool)*360);
	m_bRecordCheck = FALSE;
	m_bCheckState = FALSE;
	m_bCalcRotateCenter = FALSE;

	m_nCurrentSelMainView = 0;

	// JSY 2022-11-05 검사항목 추가
	m_bDustInspJudge = TRUE;		// 이물검사 초기화
	m_bLiquidInspJudge = TRUE;		// 액튐검사 초기화
	// END JSY 2022-11-05 검사항목 추가

	for (int i = 0; i < C_CALIPER_POS_6; i++)
	{
		memset(&m_TraceSearch_line_info[i],0,sizeof(sLine)); //Tkyuha 20221123 Line검사 수정
		m_bTraceSearchFindLine[i] = FALSE;                   //Tkyuha 20221123 Line검사 수정
	}
}

CFormMainView::~CFormMainView()
{
	for (int i = 0; i < MAX_VIEWER; i++)
	{
		if (m_pDlgViewerMain[i] != NULL)
		{
			delete m_pDlgViewerMain[i];
			m_pDlgViewerMain[i] = NULL;
		}
	}

	m_vtEdgePosition.clear();
}

void CFormMainView::DoDataExchange(CDataExchange* pDX)
{
	CFormView::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LB_MODEL_ID_TITLE, m_lbModelIdTitle);
	DDX_Control(pDX, IDC_LB_MODEL_ID, m_lbModelId);
	DDX_Control(pDX, IDC_LB_PANEL_ID_TITLE, m_lbPanelIdTitle);
	DDX_Control(pDX, IDC_LB_PANEL_ID, m_lbPanelId);
	DDX_Control(pDX, IDC_LB_PANEL_JUDGE, m_lbPanelJudge);
	DDX_Control(pDX, IDC_TAB_MAIN_VIEW, m_TabMainView);
	DDX_Control(pDX, IDC_LB_LOGIN, m_lbLogIn);
}

BEGIN_MESSAGE_MAP(CFormMainView, CFormView)
	ON_WM_CTLCOLOR()
	ON_MESSAGE(WM_VIEW_CONTROL, OnViewControl)
	ON_WM_LBUTTONUP()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB_MAIN_VIEW, &CFormMainView::OnTcnSelchangeTabMainView)
	ON_WM_CLOSE()
	ON_WM_MBUTTONDBLCLK()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CFormMainView 진단입니다.

#ifdef _DEBUG
void CFormMainView::AssertValid() const
{
	CFormView::AssertValid();
}

#ifndef _WIN32_WCE
void CFormMainView::Dump(CDumpContext& dc) const
{
	CFormView::Dump(dc);
}
#endif
#endif //_DEBUG

// CFormMainView 메시지 처리기입니다.
void CFormMainView::OnInitialUpdate()
{
	CFormView::OnInitialUpdate();

	m_hbrBkg = CreateSolidBrush(COLOR_UI_BODY);

	m_pMain = (CLET_AlignClientDlg*)AfxGetMainWnd();

	InitTitle(&m_lbModelIdTitle, "MODEL ID", 16.f, RGB(64, 64, 64));
	InitTitle(&m_lbModelId, "", 16.f, RGB(64, 64, 64));
	InitTitle(&m_lbPanelIdTitle, "PANEL ID", 16.f, RGB(64, 64, 64));
	InitTitle(&m_lbPanelId, "GLASS_ID", 20.f, RGB(64, 64, 64));
	InitTitle(&m_lbPanelJudge, "OK", 20.f, RGB(64, 64, 64));
	InitTitle(&m_lbLogIn, "LOG IN", 16.f, RGB(64, 64, 64));

	GetDlgItem(IDC_STATIC_VIEWER_BASE_MAIN)->GetWindowRect(&m_rcStaticViewerBaseMain);
	ScreenToClient(&m_rcStaticViewerBaseMain);
	GetDlgItem(IDC_STATIC_VIEWER_BASE_MAIN)->ShowWindow(SW_HIDE);

	if (m_pMain) createViewer_Main(m_pMain->m_nViewX_Num, m_pMain->m_nViewY_Num);

	CRect rect;
	int w, h;
	GetWindowRect(&rect);
	GetDlgItem(IDC_LB_MODEL_ID_TITLE)->GetWindowRect(&m_rtModelIdTitle);
	
	w = m_rtModelIdTitle.Width();
	h = m_rtModelIdTitle.Height();

	m_rtModelIdTitle.left	= m_rtModelIdTitle.left - rect.left;
	m_rtModelIdTitle.top	= m_rtModelIdTitle.top - rect.top;
	m_rtModelIdTitle.right	= m_rtModelIdTitle.left + w;
	m_rtModelIdTitle.bottom = m_rtModelIdTitle.top + h;

	GetDlgItem(IDC_LB_MODEL_ID)->GetWindowRect(&m_rtModelId);

	w = m_rtModelId.Width();
	h = m_rtModelId.Height();

	m_rtModelId.left		= m_rtModelId.left - rect.left;
	m_rtModelId.top			= m_rtModelId.top - rect.top;
	m_rtModelId.right		= m_rtModelId.left + w;
	m_rtModelId.bottom		= m_rtModelId.top + h;
	
	//KJH 2022-07-08 Login 기능 추가
	GetDlgItem(IDC_LB_LOGIN)->GetWindowRect(&m_rtLogIn);

	w = m_rtLogIn.Width();
	h = m_rtLogIn.Height();

	m_rtLogIn.left			= m_rtLogIn.left - rect.left;
	m_rtLogIn.top			= m_rtLogIn.top - rect.top;
	m_rtLogIn.right			= m_rtLogIn.left + w;
	m_rtLogIn.bottom		= m_rtLogIn.top + h;

	CString str_job;
	m_TabMainView.InsertItem(0, "     Total     ");

	for (int i = 0; i < m_pMain->vt_job_info.size(); i++)
	{
		str_job.Format("     %s     ", m_pMain->vt_job_info[i].get_job_name());
		m_TabMainView.InsertItem(i + 1, str_job);
	}
	m_TabMainView.SetCurSel(0);

	if (m_bThreadMainDisplayEndFlag == FALSE)
		m_pThread = AfxBeginThread(Thread_MainViewerDisplay, this);

	for(int i = 0; i < MAX_JOB; i++)
		m_bInspFlag[i] = FALSE;

	m_bFirstTrace = TRUE;

	for (int i = 0; i < MAX_JOB; i++) m_bHandShake[i] = FALSE;
}
HBRUSH CFormMainView::OnCtlColor(CDC* pDC, CWnd* pWnd, UINT nCtlColor)
{

	return m_hbrBkg;
}
void CFormMainView::replace_viewer_main(int nNumX, int nNumY)
{
	CString strTemp;
	int nStartX = 0;
	int nStartY = 0;
	int nWidth, nHeight;
	int gab = 5;

	nWidth = (m_rcStaticViewerBaseMain.Width() - ((nNumX - 1) * gab)) / nNumX;
	nHeight = (m_rcStaticViewerBaseMain.Height() - ((nNumY - 1) * gab)) / nNumY;

	int viewer_count = nNumX * nNumY;

	vector<CRect> vtViewerPos;

	for (int i = 0; i < viewer_count; i++)
	{
		int offX = m_rcStaticViewerBaseMain.left + (i % nNumX) * (gab + nWidth);
		int offY = m_rcStaticViewerBaseMain.top + (i / nNumX) * (gab + nHeight);

		CRect rect;

		rect.left = offX;
		rect.top = offY;
		rect.right = offX + nWidth;
		rect.bottom = offY + nHeight;
		vtViewerPos.push_back(rect);
	}

	for (int i = 0; i < m_pMain->vt_viewer_info.size(); i++)
	{
		if (i >= (nNumX * nNumY))
		{
			m_pDlgViewerMain[i]->GetViewer().ShowWindow(SW_HIDE);
			m_pDlgViewerMain[i]->ShowWindow(SW_HIDE);
			continue;
		}

		int offX = m_rcStaticViewerBaseMain.left + (i % nNumX) * (gab + nWidth);
		int offY = m_rcStaticViewerBaseMain.top + (i / nNumX) * (gab + nHeight);

		int pos = m_pMain->vt_viewer_info[i].viewer_position;


		//m_pDlgViewerMain[i]->MoveWindow(offX, offY, nWidth, nHeight);
		m_pDlgViewerMain[i]->MoveWindow(&vtViewerPos[pos]);
		m_pDlgViewerMain[i]->GetWindowRect(&vtViewerPos[pos]);
		ScreenToClient(&vtViewerPos[pos]);

		m_pDlgViewerMain[i]->setDlgChildPos(vtViewerPos[pos]);
		m_pDlgViewerMain[i]->MoveWindow(&vtViewerPos[pos]);
		m_pDlgViewerMain[i]->setViewID(i);
		m_pDlgViewerMain[i]->fitViewer();
		m_pDlgViewerMain[i]->Invalidate();

		m_pDlgViewerMain[i]->ShowWindow(SW_SHOWNOACTIVATE);
		m_pDlgViewerMain[i]->GetViewer().ShowWindow(SW_SHOWNOACTIVATE);
	}

	vtViewerPos.clear();
}
void CFormMainView::createViewer_Main(int nNumX, int nNumY)
{
	CString strTemp;
	int nStartX = 0;
	int nStartY = 0;
	int nWidth, nHeight;
	int gab = 5;

	nWidth = (m_rcStaticViewerBaseMain.Width() - ((nNumX - 1) * gab)) / nNumX;
	nHeight = (m_rcStaticViewerBaseMain.Height() - ((nNumY - 1) * gab)) / nNumY;

	int viewer_count = nNumX * nNumY;

	vector<CRect> vtViewerPos;

	for (int i = 0; i < viewer_count; i++)
	{
		int offX = m_rcStaticViewerBaseMain.left + (i % nNumX) * (gab + nWidth);
		int offY = m_rcStaticViewerBaseMain.top + (i / nNumX) * (gab + nHeight);

		CRect rect;

		rect.left = offX;
		rect.top = offY;
		rect.right = offX + nWidth;
		rect.bottom = offY + nHeight;
		vtViewerPos.push_back(rect);
	}


#ifndef JOB_INFO
	for (int i = 0; i < MAX_CAMERA; i++)
	{
		if (m_pDlgViewerMain[i] != NULL)
		{
			delete m_pDlgViewerMain[i];
			m_pDlgViewerMain[i] = NULL;
		}

		m_pDlgViewerMain[i] = new CDlgViewer;
		m_pDlgViewerMain[i]->Create(IDD_DLG_VIEWER, this);
		m_pDlgViewerMain[i]->ShowWindow(SW_SHOW);
		m_pDlgViewerMain[i]->setParent(this);
		m_pDlgViewerMain[i]->setViewerName(m_pMain->m_pViewDisplayInfo[i].getViewerName());
		m_pDlgViewerMain[i]->setPanelID_Name(m_pMain->m_pViewDisplayInfo[i].getPanelID());
		m_pDlgViewerMain[i]->setShowGridInfo(TRUE);
		m_pDlgViewerMain[i]->setShowTabCtrl(FALSE);
		m_pDlgViewerMain[i]->setUsePopupWindow(m_pMain->m_pViewDisplayInfo[i].geUseViewerPopup());
		m_pDlgViewerMain[i]->GetViewer().SetEnableDrawAllFigures(false);
		m_pDlgViewerMain[i]->GetViewer().SetEnableFigureRectangle(0);
		m_pDlgViewerMain[i]->GetViewer().SetEnableModifyFigure(0, true);
		m_pDlgViewerMain[i]->GetViewer().SetEnableDrawFigure(0, true);
		m_pDlgViewerMain[i]->GetViewer().setCameraResolutionX(m_pMain->m_pViewDisplayInfo[i].getResolutionX());
		m_pDlgViewerMain[i]->GetViewer().setCameraResolutionY(m_pMain->m_pViewDisplayInfo[i].getResolutionY());
		m_pDlgViewerMain[i]->GetViewer().OnInitWithCamera(m_pMain->m_pViewDisplayInfo[i].getViewImageWidth(), m_pMain->m_pViewDisplayInfo[i].getViewImageHeight(), 8);
		m_pDlgViewerMain[i]->GetViewer().SetEnableDrawCrossLine(true);
		m_pDlgViewerMain[i]->updateViewerFunc();

		m_pMain->m_pViewDisplayInfo[i].setViewer(&m_pDlgViewerMain[i]->GetViewer());

		if (i >= (nNumX * nNumY))
		{
			m_pDlgViewerMain[i]->GetViewer().ShowWindow(SW_HIDE);
			m_pDlgViewerMain[i]->ShowWindow(SW_HIDE);
			continue;
		}

		int offX = m_rcStaticViewerBaseMain.left + (i % nNumX) * (gab + nWidth);
		int offY = m_rcStaticViewerBaseMain.top + (i / nNumX) * (gab + nHeight);


		int pos = m_pMain->m_pViewDisplayInfo[i].get_viewer_pos();
		m_pDlgViewerMain[i]->setDlgChildPos(vtViewerPos[pos]);
		m_pDlgViewerMain[i]->MoveWindow(&vtViewerPos[pos]);
		m_pDlgViewerMain[i]->fitViewer();
	}
#else
	int nMetho = m_pMain->vt_job_info[0].algo_method;
	for (int i = 0; i < m_pMain->vt_viewer_info.size(); i++)
	{
		if (m_pDlgViewerMain[i] != NULL)
		{
			delete m_pDlgViewerMain[i];
			m_pDlgViewerMain[i] = NULL;
		}

		m_pDlgViewerMain[i] = new CDlgViewer;
		m_pDlgViewerMain[i]->Create(IDD_DLG_VIEWER, this);
		SetWindowLong(m_pDlgViewerMain[i]->GetSafeHwnd(), GWL_EXSTYLE, WS_EX_NOACTIVATE);
		m_pDlgViewerMain[i]->ShowWindow(SW_SHOWNOACTIVATE);
		m_pDlgViewerMain[i]->setParent(this);
		m_pDlgViewerMain[i]->setViewID(i);
		m_pDlgViewerMain[i]->setViewerName(m_pMain->vt_viewer_info[i].viewer_name.c_str());
		m_pDlgViewerMain[i]->setShowGridInfo(TRUE);
		m_pDlgViewerMain[i]->setShowTabCtrl(FALSE);
		m_pDlgViewerMain[i]->setUsePopupWindow(m_pMain->vt_viewer_info[i].use_viewer_popup);
		m_pDlgViewerMain[i]->GetViewer().SetEnableDrawAllFigures(false);
		m_pDlgViewerMain[i]->GetViewer().SetEnableFigureRectangle(0);
		m_pDlgViewerMain[i]->GetViewer().SetEnableModifyFigure(0, true);
		m_pDlgViewerMain[i]->GetViewer().SetEnableDrawFigure(0, true);

		if (nMetho == CLIENT_TYPE_SCAN_INSP)
		{
			//m_pDlgViewerMain[i]->GetViewer().setCameraResolutionX(m_pMain->vt_viewer_info[i].resolution_y);
			//m_pDlgViewerMain[i]->GetViewer().setCameraResolutionY(m_pMain->vt_viewer_info[i].resolution_x);
			m_pDlgViewerMain[i]->GetViewer().SetMetricTransform(Rigid(m_pMain->vt_viewer_info[i].resolution_x, 0, 0, 0, -m_pMain->vt_viewer_info[i].resolution_y, 0));
		}
		else
		{
			//m_pDlgViewerMain[i]->GetViewer().setCameraResolutionX(m_pMain->vt_viewer_info[i].resolution_x);
			//m_pDlgViewerMain[i]->GetViewer().setCameraResolutionY(m_pMain->vt_viewer_info[i].resolution_y);
			m_pDlgViewerMain[i]->GetViewer().SetMetricTransform(Rigid(m_pMain->vt_viewer_info[i].resolution_x, 0, 0, 0, -m_pMain->vt_viewer_info[i].resolution_y, 0));

			//KJH 2022-06-07 9View CenterAlign Live막기
			if (m_pMain->vt_job_info[0].num_of_viewer == 4)
			{
				if (i == 0 && nMetho == CLIENT_TYPE_ELB_CENTER_ALIGN)
				{
					m_pDlgViewerMain[i]->upDateControlUI(FALSE);
				}
				else if (i == 3 && nMetho == CLIENT_TYPE_ELB_CENTER_ALIGN)
				{
					m_pDlgViewerMain[i]->upDateControlUI(FALSE);
				}
				else if (i == 4 && nMetho == CLIENT_TYPE_ELB_CENTER_ALIGN)
				{
					m_pDlgViewerMain[i]->upDateControlUI(FALSE);
				}
				else if (i == 7 && nMetho == CLIENT_TYPE_ELB_CENTER_ALIGN)
				{
					m_pDlgViewerMain[i]->upDateControlUI(FALSE);
				}
			}
			else
			{
				if (i == 0 && nMetho == CLIENT_TYPE_ELB_CENTER_ALIGN)
				{
					m_pDlgViewerMain[i]->upDateControlUI(FALSE);
				}
			}

			// KBJ 2022-07-22 Conveyer 이전 화면 뷰어 Dlg 비활성화
			if (i == 5 && (m_pMain->vt_job_info[2].algo_method == CLIENT_TYPE_1CAM_2POS_REFERENCE ||
				m_pMain->vt_job_info[2].algo_method == CLIENT_TYPE_1CAM_4POS_ROBOT))
			{
				m_pDlgViewerMain[i]->upDateControlUI(FALSE);
			}

		}
		m_pDlgViewerMain[i]->GetViewer().OnInitWithCamera(m_pMain->vt_viewer_info[i].image_width, m_pMain->vt_viewer_info[i].image_height, 8);
		m_pDlgViewerMain[i]->GetViewer().SetEnableDrawGuidingGraphics(true);
		m_pDlgViewerMain[i]->updateViewerFunc();

#ifndef JOB_INFO
		m_pMain->m_pViewDisplayInfo[i].setViewer(&m_pDlgViewerMain[i]->GetViewer());
#endif
		if (i >= (nNumX * nNumY))
		{
			m_pDlgViewerMain[i]->GetViewer().ShowWindow(SW_HIDE);
			m_pDlgViewerMain[i]->ShowWindow(SW_HIDE);
			//KJH 2022-01-05 숨겨지는 Viewer ToolBar 추가작업
			m_pDlgViewerMain[i]->InitToolBar();
			m_pDlgViewerMain[i]->upDateControlUI(FALSE);
			continue;
		}

		int offX = m_rcStaticViewerBaseMain.left + (i % nNumX) * (gab + nWidth);
		int offY = m_rcStaticViewerBaseMain.top + (i / nNumX) * (gab + nHeight);

		int pos = m_pMain->vt_viewer_info[i].viewer_position;

		m_pDlgViewerMain[i]->InitToolBar();

		m_pDlgViewerMain[i]->childWindow();
		m_pDlgViewerMain[i]->setDlgChildPos(vtViewerPos[pos]);
		m_pDlgViewerMain[i]->MoveWindow(&vtViewerPos[pos]);
		m_pDlgViewerMain[i]->fitViewer();

		m_pDlgViewerMain[i]->GetViewer().reset_to_gray_image();
	}

#endif

	vtViewerPos.clear();
}
void CFormMainView::replace_viewer_main_fix(int nNumX, int nNumY, int id)
{
	std::vector<int> cam;

	int nStartX = 0;
	int nStartY = 0;
	int nWidth, nHeight;
	int gab = 5, nC = 0, nCam = 0;

	if (id != -1) cam = m_pMain->vt_job_info[id].camera_index;

	nWidth = (m_rcStaticViewerBaseMain.Width() - ((nNumX - 1) * gab)) / nNumX;
	nHeight = (m_rcStaticViewerBaseMain.Height() - ((nNumY - 1) * gab)) / nNumY;

	for (int i = 0; i < m_pMain->vt_viewer_info.size(); i++)
	{
		m_pDlgViewerMain[i]->GetViewer().ShowWindow(SW_HIDE);
		m_pDlgViewerMain[i]->ShowWindow(SW_HIDE);
	}

	vector<CRect> vtViewerPos;
	int viewer_count = nNumX * nNumY;
	for (int i = 0; i < viewer_count; i++)
	{
		int offX = m_rcStaticViewerBaseMain.left + (i % nNumX) * (gab + nWidth);
		int offY = m_rcStaticViewerBaseMain.top + (i / nNumX) * (gab + nHeight);

		CRect rect;

		rect.left = offX;
		rect.top = offY;
		rect.right = offX + nWidth;
		rect.bottom = offY + nHeight;
		vtViewerPos.push_back(rect);
	}

	bool _bcenterZoomChanbe = false;
	if (m_pMain->vt_job_info[id].algo_method == CLIENT_TYPE_ELB_CENTER_ALIGN ||
		m_pMain->vt_job_info[id].algo_method == CLIENT_TYPE_CENTER_NOZZLE_ALIGN) _bcenterZoomChanbe = true;

	for (int i = 0; i < m_pMain->vt_job_info[id].main_viewer_index.size(); i++)
	{
		if (i < vtViewerPos.size())
		{
			int viewer = m_pMain->vt_job_info[id].main_viewer_index[i];

			int offX = m_rcStaticViewerBaseMain.left + (i % nNumX) * (gab + nWidth);
			int offY = m_rcStaticViewerBaseMain.top + (i / nNumX) * (gab + nHeight);

			m_pDlgViewerMain[viewer]->MoveWindow(offX, offY, nWidth, nHeight);
			m_pDlgViewerMain[viewer]->GetWindowRect(&vtViewerPos[i]);
			ScreenToClient(&vtViewerPos[i]);

			m_pDlgViewerMain[viewer]->setDlgChildPos(vtViewerPos[i]);
			m_pDlgViewerMain[viewer]->MoveWindow(&vtViewerPos[i]);
			m_pDlgViewerMain[viewer]->fitViewer();

			if (_bcenterZoomChanbe && m_pDlgViewerMain[viewer]->GetViewer().IsFitImage())
			{
				m_pDlgViewerMain[viewer]->GetViewer().ZoomIn();
				m_pDlgViewerMain[viewer]->GetViewer().ZoomIn();
				m_pDlgViewerMain[viewer]->GetViewer().ZoomIn();
				m_pDlgViewerMain[viewer]->GetViewer().ZoomIn();
			}

			m_pDlgViewerMain[viewer]->Invalidate();

			m_pDlgViewerMain[viewer]->ShowWindow(SW_SHOWNOACTIVATE);
			m_pDlgViewerMain[viewer]->GetViewer().ShowWindow(SW_SHOWNOACTIVATE);
		}
		else
		{
			// KBJ 2022-07-12 Total에만 사용하고 Main Viewer에서 사용 안하는경우가 있어 주석처리
			/*
			CString str_temp;
			str_temp.Format("main viewer count Invalid!! - %d", vtViewerPos.size());
			AfxMessageBox(str_temp);
			break;
			*/
		}
	}

	vtViewerPos.clear();
}
void CFormMainView::InitTitle(CLabelEx* pTitle, CString str, float size, COLORREF color)
{
	pTitle->SetAlignTextCM();
	pTitle->SetSizeText(size);
	pTitle->SetColorBkg(255, color);
	pTitle->SetColorBorder(255, COLOR_DDARK_GRAY);
	pTitle->SetText(str);
	pTitle->ModifyStyleEx(WS_EX_CLIENTEDGE, 0, SWP_DRAWFRAME | SWP_FRAMECHANGED);
}
void CFormMainView::MainButtonInit(CButtonEx* pbutton, int size)
{
	pbutton->SetEnable(true);
	pbutton->SetColorBkg(255, RGB(64, 64, 64));
	pbutton->SetColorBorder(255, COLOR_DDARK_GRAY);
	pbutton->SetAlignTextCM();
	pbutton->SetSizeImage(5, 5, size, size);
	pbutton->LoadImageFromResource(IDB_PNG_BUTTON_CIRCLE, TRUE);
}

BOOL CFormMainView::IsClickModelIdTitle(CPoint pt)
{
	if (PtInRect(&m_rtModelIdTitle, pt)) return TRUE;
	else								 return FALSE;
}

BOOL CFormMainView::IsClickModelId(CPoint pt)
{
	CRect rect, ctrlRect;
	int w, h;

	GetWindowRect(&rect);
	GetDlgItem(IDC_LB_MODEL_ID)->GetWindowRect(&ctrlRect);

	w = ctrlRect.Width();
	h = ctrlRect.Height();

	ctrlRect.left	-= rect.left;
	ctrlRect.top	-= rect.top;
	ctrlRect.right	+= w;
	ctrlRect.bottom += h;

	if (pt.x >= ctrlRect.left && pt.x <= ctrlRect.right &&
		pt.y >= ctrlRect.top && pt.y <= ctrlRect.bottom)	return TRUE;
	else		return FALSE;
}

BOOL CFormMainView::IsClickLogIn(CPoint pt)
{
	if (PtInRect(&m_rtLogIn, pt))	return TRUE;
	else							return FALSE;
}

void CFormMainView::OnLButtonUp(UINT nFlags, CPoint point)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.

	CLET_AlignClientDlg* pMain = (CLET_AlignClientDlg*)GetParent();

	if (IsClickModelIdTitle(point) == TRUE)
	{
		pMain->ShowWindow(SW_MINIMIZE);
	}
	else if (IsClickLogIn(point) == TRUE)
	{
		if (m_pMain->m_bAutoStart) return;

		CLoginDlg dlg;
		dlg.DoModal();
	}

	CFormView::OnLButtonUp(nFlags, point);
}
void CFormMainView::fnReceivePanelId(int nJob)
{
#ifndef JOB_INFO
	CString strPanelID;
	strPanelID.Format("%s", m_pMain->m_cPanel_Read_ID[id]);

	if (strPanelID.GetLength() < 4)
	{
		SYSTEMTIME time;
		GetLocalTime(&time);
		strPanelID.Format("NONE_%02d%02d%02d", time.wHour, time.wMinute, time.wSecond);
	}

	m_lbPanelId.SetText(strPanelID);
#else
	m_lbPanelId.SetText(m_pMain->vt_job_info[nJob].main_object_id.c_str());

	set_viewer_name(nJob);

#endif
}
void CFormMainView::set_viewer_name(int nJob)
{
	int algo_method = m_pMain->vt_job_info[nJob].algo_method;

	switch (algo_method) {
	case CLIENT_TYPE_2CAM_EACH_2SHOT_ALIGN:
	{
		for (int nCam = 0; nCam < m_pMain->vt_job_info[nJob].num_of_camera; nCam++)
		{
			int viewer = m_pMain->vt_job_info[nJob].viewer_index[nCam * 2 + 0];
			m_pDlgViewerMain[viewer]->setPanelID_Name(m_pMain->vt_job_info[nJob].get_main_object_id());

			viewer = m_pMain->vt_job_info[nJob].viewer_index[nCam * 2 + 1];
			m_pDlgViewerMain[viewer]->setPanelID_Name(m_pMain->vt_job_info[nJob].get_main_object_id());
		}
	}
	break;
	case CLIENT_TYPE_SCAN_INSP:
	{
		for (int nCam = 0; nCam < m_pMain->vt_job_info[nJob].num_of_camera; nCam++)
		{
			for (int i = 0; i < m_pMain->vt_job_info[nJob].viewer_index.size(); i++)
			{
				int viewer = m_pMain->vt_job_info[nJob].viewer_index[i];
				m_pDlgViewerMain[viewer]->setPanelID_Name(m_pMain->vt_job_info[nJob].get_main_object_id());
			}
		}
	}
	break;
	case CLIENT_TYPE_ELB_CENTER_ALIGN:
	{
		for (int i = 0; i < m_pMain->vt_job_info[nJob].viewer_index.size(); i++)
		{
			int viewer = m_pMain->vt_job_info[nJob].viewer_index[i];
			m_pDlgViewerMain[viewer]->setPanelID_Name(m_pMain->vt_job_info[nJob].get_main_object_id());
		}

		// KBJ 2022-02-24 Nozzle View Set Panel Naming
		int viewer = m_pMain->vt_job_info[1].viewer_index[0];
		m_pDlgViewerMain[viewer]->setPanelID_Name(m_pMain->vt_job_info[nJob].get_main_object_id());
	}
	break;
	// YCS 2022-11-17 스크래치 뷰어에도 패널아이디 보이도록 표시 (배경색 Green or Red 어케하누)
	case CLIENT_TYPE_1CAM_1SHOT_ALIGN_WITH_SCRATCH_INSP:
	{
		for (int nCam = 0; nCam < m_pMain->vt_job_info[nJob].num_of_camera; nCam++)
		{
			int viewer = m_pMain->vt_job_info[nJob].viewer_index[nCam];
			int nScratchViewer = viewer + 1; // 스크래치 뷰어 (3번, 7번)

			m_pDlgViewerMain[viewer]->setPanelID_Name(m_pMain->vt_job_info[nJob].get_main_object_id());
			m_pDlgViewerMain[nScratchViewer]->setPanelID_Name(m_pMain->vt_job_info[nJob].get_main_object_id());
		}
	}
	break;
	default:
	{
		for (int nCam = 0; nCam < m_pMain->vt_job_info[nJob].num_of_camera; nCam++)
		{
			int viewer = m_pMain->vt_job_info[nJob].viewer_index[nCam];

			m_pDlgViewerMain[viewer]->setPanelID_Name(m_pMain->vt_job_info[nJob].get_main_object_id());
		}
	}
	break;
	}
}
void CFormMainView::fnReceiveProcessStart(int id)
{
#ifndef JOB_INFO
	CString strPanelID;
	strPanelID.Format("%s", m_pMain->m_cPanel_Read_ID[id]);
	std::vector<int> camBuf = m_pMain->m_pAlgorithmInfo.getAlgorithmCameraIndex(id);
	for (int i = 0; i < camBuf.size(); i++)	m_pMain->m_pViewDisplayInfo[camBuf.at(0)].setPanelID(strPanelID);
#endif

	// 이것 호출 되기 전에 이미 패널 아이디가 반영 됨.
	AcquireSRWLockExclusive(&m_pMain->g_srwlock); // 동기화 시킴 Tkyuha 20211027
	m_pMain->m_nSeqProcess[id] = 100;
	ReleaseSRWLockExclusive(&m_pMain->g_srwlock);
	m_pMain->setProcessStartFlag(id);
}

void CFormMainView::resetPlcSignal()
{
	//for (int threadID = 0; threadID < MAX_CAMERA; threadID++)
	for (int threadID = 0; threadID < m_pMain->vt_job_info.size(); threadID++)
	{
		g_CommPLC.SetBit(m_pMain->m_nAlignOkWrite[threadID], FALSE);
		g_CommPLC.SetBit(m_pMain->m_nAlignNgWrite[threadID], FALSE);
		g_CommPLC.SetBit(m_pMain->m_nAlignCompletWrite[threadID], FALSE);
		g_CommPLC.SetBit(m_pMain->m_nAlignManualMarkWrite[threadID], FALSE);
		g_CommPLC.SetBit(m_pMain->m_nAlignLcheckNgWrite[threadID], FALSE);

		g_CommPLC.SetBit(m_pMain->m_nInspAckWrite[threadID], FALSE);
		g_CommPLC.SetBit(m_pMain->m_nInspOkWrite[threadID], FALSE);
		g_CommPLC.SetBit(m_pMain->m_nInspNgWrite[threadID], FALSE);
	}
}
void CFormMainView::resetPlcFlag()
{
#ifndef JOB_INFO
	for (int threadID = 0; threadID < MAX_CAMERA; threadID++)
	{
		if (m_pMain->m_pAlgorithmInfo.getAlgorithmCount() <= threadID) continue;
		resetProcessStart(threadID);
	}
#else
	for (int threadID = 0; threadID < m_pMain->vt_job_info.size(); threadID++)
	{
		resetProcessStart(threadID);
	}

#endif
}
void CFormMainView::resetCalibrationModeOn()
{
#ifndef JOB_INFO
	int address;
	for (int threadID = 0; threadID < MAX_CAMERA; threadID++)
	{
		if (m_pMain->m_pAlgorithmInfo.getAlgorithmCount() <= threadID) continue;

		address = m_pMain->m_pAlgorithmInfo.getAlgorithmWriteBitStart(threadID);
		g_CommPLC.SetBit(address + 6, FALSE);
		g_CommPLC.SetBit(address + 7, FALSE);
	}
#else
	int address;
	for (int threadID = 0; threadID < m_pMain->vt_job_info.size(); threadID++)
	{
		address = m_pMain->vt_job_info[threadID].plc_addr_info.write_bit_start;
		g_CommPLC.SetBit(address + 6, FALSE);
		g_CommPLC.SetBit(address + 7, FALSE);
	}
#endif
}
void CFormMainView::reset_viewer(int nJob, int subview)
{
	for (int i = 0; i < m_pMain->vt_job_info[nJob].viewer_index.size(); i++)
	{
		if (subview != 0 && subview != i)continue;
		int viewer = m_pMain->vt_job_info[nJob].viewer_index[i];
		m_pDlgViewerMain[viewer]->GetViewer().reset_to_gray_image();
		m_pDlgViewerMain[viewer]->resetViewerJudge();
		m_pDlgViewerMain[viewer]->GetViewer().ClearOverlayDC();
		//KJH 2021-06-05 Grab시마다 화면에 불필요한거 다 지우기
		m_pDlgViewerMain[viewer]->GetViewer().clearAllFigures();
	}
}
void CFormMainView::reset_viewerID(int nJob, int subview)
{
	int viewer = m_pMain->vt_job_info[nJob].viewer_index[subview];
	m_pDlgViewerMain[viewer]->GetViewer().reset_to_gray_image();
	m_pDlgViewerMain[viewer]->GetViewer().ClearOverlayDC();
	m_pDlgViewerMain[viewer]->resetViewerJudge();
}

void CFormMainView::fnProcess_ProcessAlignAlgorithm(int nJob)
{
	CString str;
	int tm = STM_PN_ALIGN + nJob;

	BOOL bit = g_CommPLC.GetBit(m_pMain->m_nAlignRequestRead[nJob]);

	if ( (bit && getProcessStart(nJob) != TRUE) || m_pMain->auto_seq_test[nJob] )	// Align 시작 조건
	{
		m_pMain->auto_seq_test[nJob] = FALSE;
		str.Format("[%s] Process Start, ID : %s", get_job_name(nJob), m_pMain->vt_job_info[nJob].main_object_id.c_str());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		memset(m_pMain->vt_result_info[nJob].each_Judge, 0, sizeof(m_pMain->vt_result_info[nJob].each_Judge));

		if (m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_NOZZLE_ALIGN)
		{
			m_pMain->m_nSeqNozzleGrabCount++;
			if (m_pMain->m_nSeqNozzleGrabCount > 10) m_pMain->m_nSeqNozzleGrabCount = 10;

			////2021-05-19 KJH Process 시작 전 조명 On , OFF 제어 Start
			////2021-05-19 KJH 배열순서가 0번,1번,2번 카메라 순
			//KJH 2021-05-29 Grab전 조명 ON
			if (m_pMain->m_nSeqNozzleGrabCount == 1)
			{
				int camCount = m_pMain->vt_job_info[nJob].num_of_camera;
				for (int nCam = 0; nCam < camCount; nCam++)
				{
					m_pMain->LightControllerTurnOnOff(nJob, nCam);
				}
			}
		}

		// KBJ 2022-07-12 Conveyer Align 이전 이미지 옴기기 기능 추가
		if (m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_1CAM_2POS_REFERENCE ||
			m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_1CAM_4POS_ROBOT)
		{
			CString strJobName;
			strJobName.Format("%s", m_pMain->vt_job_info[nJob].job_name.c_str());
			if (strJobName.Find("CONV") > -1 && m_pMain->vt_job_info[nJob].num_of_viewer > 1)
			{
				_stLoadBeforeImage pParam;
				pParam.nJob = nJob;
				pParam.nViewer_index1 = 0;
				pParam.nViewer_index2 = 1;
				pParam.pDlg = m_pMain;
				AfxBeginThread(Thread_Load_Before_Image, &pParam);
			}
		}

		//2021-05-19 KJH 조명 On, Off 수정 End
		setProcessStart(nJob);	// 여기 함수 진입 flag
		read_glass_id(nJob);
		reset_viewer(nJob);
		SendMessageA(WM_VIEW_CONTROL, MSG_FMV_GLASSID_UPDATE, nJob);

		//KJH 2022-03-16 Act,Seq Time Display 추가
		m_pMain->m_clockStartTime[nJob] = clock();

		// 실제 Align Sequence 동작하는 시점.
		AcquireSRWLockExclusive(&m_pMain->g_srwlock); // 동기화 시킴 Tkyuha 20211027
		m_pMain->m_nSeqProcess[nJob] = 100;
		ReleaseSRWLockExclusive(&m_pMain->g_srwlock);
		m_pMain->setProcessStartFlag(nJob);

		
	}
	
	if (g_CommPLC.GetBit(m_pMain->m_nAlignRequestRead[nJob]) != TRUE && getProcessStart(nJob))	// Align 시작 조건
	{
		str.Format("[%s] Process End", get_job_name(nJob));
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		resetProcessStart(nJob);

		g_CommPLC.SetBit(m_pMain->m_nAlignOkWrite[nJob], FALSE);
		g_CommPLC.SetBit(m_pMain->m_nAlignOkWrite[nJob] + 2, FALSE);
		g_CommPLC.SetBit(m_pMain->m_nAlignNgWrite[nJob], FALSE);
		g_CommPLC.SetBit(m_pMain->m_nAlignNgWrite[nJob] + 2, FALSE);

		// KBJ 2022-10-20 Manual Mark Bit Reset 추가
		g_CommPLC.SetBit(m_pMain->m_nAlignOkWrite[nJob] + 8, FALSE);

		if (m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_1CAM_2POS_REFERENCE ||
			m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_1CAM_4POS_ROBOT)
		{   // 두번째 영역 초기화
			g_CommPLC.SetBit(m_pMain->m_nAlignOkWrite[nJob]	+ 20, FALSE);
			g_CommPLC.SetBit(m_pMain->m_nAlignOkWrite[nJob] + 22, FALSE);
			g_CommPLC.SetBit(m_pMain->m_nAlignNgWrite[nJob] + 20, FALSE);
			g_CommPLC.SetBit(m_pMain->m_nAlignNgWrite[nJob] + 22, FALSE);
		}
		else if(m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_1CAM_2POS_REFERENCE)
		{
			g_CommPLC.SetBit(m_pMain->m_nAlignOkWrite[nJob] + 2, FALSE); 
			g_CommPLC.SetBit(m_pMain->m_nAlignNgWrite[nJob] + 2, FALSE);
		}
	}
}
void CFormMainView::fnProcess_ProcessRobotAlignAlgorithm(int nJob)
{
	CString str;
	int tm = STM_PN_ALIGN + nJob;

	BOOL bit = g_CommPLC.GetBit(m_pMain->m_nAlignRequestRead[nJob]);

	if ((bit && getProcessStart(nJob) != TRUE) || m_pMain->auto_seq_test[nJob])	// Align 시작 조건
	{
		m_pMain->auto_seq_test[nJob] = FALSE;
		str.Format("[%s] Process Start, ID : %s", get_job_name(nJob), m_pMain->vt_job_info[nJob].main_object_id.c_str());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		memset(m_pMain->vt_result_info[nJob].each_Judge, 0, sizeof(m_pMain->vt_result_info[nJob].each_Judge));

		// KBJ 2022-07-12 Conveyer Align 이전 이미지 옴기기 기능 추가
		CString strJobName;
		strJobName.Format("%s", m_pMain->vt_job_info[nJob].job_name.c_str());
		if (strJobName.Find("CONV") > -1 && m_pMain->vt_job_info[nJob].num_of_viewer > 1)
		{
			_stLoadBeforeImage pParam;
			pParam.nJob = nJob;
			pParam.nViewer_index1 = 0;
			pParam.nViewer_index2 = 1;
			pParam.pDlg = m_pMain;

			// KBJ 2022-07-22 Conveyer 이전 화면 Load --> Caputre 변경
			// MoveCaputre 하다가 죽는데 뭣때문에 죽는지 잘 모르겠어요 ㅠ
			//Thread인데 밑에서 reset_viewer하니까 동시접근....... 단순 뷰어 정보읽어오는게 아니라 Move는 DC까지 선언되어 있어서 함수 끝날때까지 변동이 생기면 안됨.
			AfxBeginThread(Thread_Load_Before_Image, &pParam);
			//AfxBeginThread(Thread_Cature_Before_Image, &pParam);
		}

		//2021-05-19 KJH 조명 On, Off 수정 End
		setProcessStart(nJob);	// 여기 함수 진입 flag
		read_glass_id(nJob);
		reset_viewer(nJob);
		SendMessageA(WM_VIEW_CONTROL, MSG_FMV_GLASSID_UPDATE, nJob);

		//KJH 2022-03-16 Act,Seq Time Display 추가
		m_pMain->m_clockStartTime[nJob] = clock();

		// 실제 Align Sequence 동작하는 시점.
		AcquireSRWLockExclusive(&m_pMain->g_srwlock); // 동기화 시킴 Tkyuha 20211027
		m_pMain->m_nSeqProcess[nJob] = 100;
		ReleaseSRWLockExclusive(&m_pMain->g_srwlock);
		m_pMain->setProcessStartFlag(nJob);
	}

	if (g_CommPLC.GetBit(m_pMain->m_nAlignRequestRead[nJob]) != TRUE && getProcessStart(nJob))	// Align 시작 조건
	{
		str.Format("[%s] Process End", get_job_name(nJob));
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		resetProcessStart(nJob);

		g_CommPLC.SetBit(m_pMain->m_nAlignOkWrite[nJob]		, FALSE);
		g_CommPLC.SetBit(m_pMain->m_nAlignOkWrite[nJob] + 2 , FALSE);
		g_CommPLC.SetBit(m_pMain->m_nAlignNgWrite[nJob]		, FALSE);
		g_CommPLC.SetBit(m_pMain->m_nAlignNgWrite[nJob] + 2 , FALSE);
		
		// 두번째 영역 초기화
		g_CommPLC.SetBit(m_pMain->m_nAlignOkWrite[nJob] + 20, FALSE);
		g_CommPLC.SetBit(m_pMain->m_nAlignOkWrite[nJob] + 22, FALSE);
		g_CommPLC.SetBit(m_pMain->m_nAlignNgWrite[nJob] + 20, FALSE);
		g_CommPLC.SetBit(m_pMain->m_nAlignNgWrite[nJob] + 22, FALSE);
	}
}

// KBJ 2022-09-22 노즐 클리닝 카메라 라이브 추가
void CFormMainView::fnProcess_ProcessLive(int nJob)
{
	CString str;

	CString strJobName;
	strJobName.Format("%s", m_pMain->vt_job_info[nJob].job_name.c_str());
	if (strJobName.Find("CLEANING") < 0)
	{
		return;
	}
	int nviewer = m_pMain->vt_job_info[nJob].viewer_index[0];

	// A1 : 15138
	// A2 : 15238
	// B1 : 15438
	// B2 : 15638

	BOOL bit = g_CommPLC.GetBit(m_pMain->m_nAlignRequestRead[0] + 18);

	if (bit == TRUE && m_pMain->Process_Live[nJob] == FALSE)
	{
		m_pMain->Process_Live[nJob] = TRUE;
		m_pMain->LightControllerTurnOnOff(nJob, 0, 0);
		m_pDlgViewerMain[nviewer]->OnCommandLive();
	}

	if (bit == FALSE && m_pMain->Process_Live[nJob] == TRUE)
	{
		m_pMain->Process_Live[nJob] = FALSE;
		m_pMain->LightControllerTurnOnOff(nJob, 0, 0, false, true);
		m_pDlgViewerMain[nviewer]->OnCommandStop();
	}
}

//KJH 2022-05-11 Film Align Grab bit 관련 Process추가
void CFormMainView::fnProcess_ProcessFilmAlignAlgorithm(int nJob)
{
	CString str;
	int tm = STM_PN_ALIGN + nJob;

	BOOL bit = g_CommPLC.GetBit(m_pMain->m_nAlignRequestRead[nJob]);
	BOOL Grabbit = g_CommPLC.GetBit(m_pMain->m_nAlignRequestRead[nJob] + 1);

	if ((bit && getProcessStart(nJob) != TRUE) || m_pMain->auto_seq_test[nJob])	// Align 시작 조건
	{
		m_pMain->auto_seq_test[nJob] = FALSE;

		str.Format("[%s] Process Start, ID : %s", get_job_name(nJob), m_pMain->vt_job_info[nJob].main_object_id.c_str());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		memset(m_pMain->vt_result_info[nJob].each_Judge, 0, sizeof(m_pMain->vt_result_info[nJob].each_Judge));

		setProcessStart(nJob);	// 여기 함수 진입 flag
		setSubProcessStart(nJob);
		read_glass_id(nJob);
		reset_viewer(nJob);
		SendMessageA(WM_VIEW_CONTROL, MSG_FMV_GLASSID_UPDATE, nJob);

		//KJH 2022-03-16 Act,Seq Time Display 추가
		m_pMain->m_clockStartTime[nJob] = clock();

		// 실제 Align Sequence 동작하는 시점.
		AcquireSRWLockExclusive(&m_pMain->g_srwlock); // 동기화 시킴 Tkyuha 20211027
		m_pMain->m_nSeqProcess[nJob] = 100;
		ReleaseSRWLockExclusive(&m_pMain->g_srwlock);
		m_pMain->setProcessStartFlag(nJob);
	}

	if (bit != TRUE && getProcessStart(nJob))	// Align 시작 조건
	{
		str.Format("[%s] Process End", get_job_name(nJob));
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		resetProcessStart(nJob);
		resetSubProcessStart(nJob);

		g_CommPLC.SetBit(m_pMain->m_nAlignOkWrite[nJob], FALSE);
		g_CommPLC.SetBit(m_pMain->m_nAlignOkWrite[nJob] + 2, FALSE);
		g_CommPLC.SetBit(m_pMain->m_nAlignNgWrite[nJob], FALSE);
		g_CommPLC.SetBit(m_pMain->m_nAlignNgWrite[nJob] + 2, FALSE);
	}

	if (bit && Grabbit != TRUE && getSubProcessStart(nJob))	// Align 시작 조건
	{
		g_CommPLC.SetBit(m_pMain->m_nAlignOkWrite[nJob] + 2, FALSE);

		resetSubProcessStart(nJob);

		str.Format("[%s] Grab Bit Off", get_job_name(nJob));
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}
}
void CFormMainView::fnProcess_ProcessInspAlgorithm(int nJob)
{
	int tm = STM_INSP + nJob;

	BOOL request_bit = g_CommPLC.GetBit(m_pMain->m_nInspRequestRead[nJob]);

	// hsj 2022-01-13 initialize req,ack bit add
	// 매번 seq마다 request_bit 주기 전 비트살림 
	//BOOL initial_bit= g_CommPLC.GetBit(m_pMain->m_nInspRequestRead[nJob] + 19);

	////if (!bit&&g_CommPLC.GetBit(m_pMain->m_nInspRequestRead[nJob] + 19)&& m_bInspFlag !=TRUE)
	//if (request_bit!=TRUE && initial_bit && m_bInspFlag[nJob] != TRUE)
	//{
	//	g_CommPLC.SetBit(m_pMain->m_nInspOkWrite[nJob], FALSE);
	//	g_CommPLC.SetBit(m_pMain->m_nInspNgWrite[nJob], FALSE);

	//	g_CommPLC.SetBit(m_pMain->m_nInspAckWrite[nJob] + 19, TRUE);

	//	m_bInspFlag[nJob] = TRUE;
	//}

	//if(request_bit != TRUE && initial_bit!=TRUE && m_bInspFlag[nJob])
	//{
	//	g_CommPLC.SetBit(m_pMain->m_nInspAckWrite[nJob] + 19, FALSE);

	//	m_bInspFlag[nJob] = FALSE;
	//}

	if ((request_bit && getProcessStart(nJob) != TRUE) || m_pMain->auto_seq_test[nJob])	// Align 시작 조건
	{
		m_pMain->auto_seq_test[nJob] = FALSE;
		if (m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_FILM_INSP)
		{
			//HTK 2022-07-11 Film  Grab bit Reset추가
			g_CommPLC.SetBit(m_pMain->m_nInspOkWrite[nJob] + 8, FALSE);
		}

		memset(m_pMain->m_stInsp_DistanceResult, 0, sizeof(m_pMain->m_stInsp_DistanceResult));

		for (int i = 0; i < NUM_ALIGN_POINTS; i++)
		{
			m_pMain->m_dDistanceInspJudgeResult[i] = TRUE;
			m_pMain->m_dDistanceInspDataResult[i] = 0;
			m_pMain->m_dDistanceInspSubJudgeResult[i] = TRUE;
			m_pMain->m_dDistanceInspSubDataResult[i] = 0;
		}

		CString str, strTemp;
		long lRevisionData[6] = { 1, 0, };

		reset_viewer(nJob);
		setProcessStart(nJob);

#ifndef JOB_INFO
		CTime NowTime;
		NowTime = CTime::GetCurrentTime();
		m_pMain->m_strResultDate[id].Format("%04d%02d%02d", NowTime.GetYear(), NowTime.GetMonth(), NowTime.GetDay());
		m_pMain->m_strResultTime[id].Format("%02d_%02d_%02d", NowTime.GetHour(), NowTime.GetMinute(), NowTime.GetSecond());

		readGlassID(m_pMain->m_nPanel_ID_Read[id], 20, m_pMain->m_cPanel_Read_ID[id]);

		strTemp.Format("%s", m_pMain->m_cPanel_Read_ID[id]);
		if (strTemp.GetLength() < 10)
		{
			strTemp.Format("Test_%02d_%02d_%02d", NowTime.GetHour(), NowTime.GetMinute(), NowTime.GetSecond());
			sprintf((char*)m_pMain->m_cPanel_Read_ID[id], "%s", strTemp);
		}
#else

		read_glass_id(nJob);

#endif
		//seq_SetTimer(tm, 30000);
		SendMessageA(WM_VIEW_CONTROL, MSG_FMV_GLASSID_UPDATE, nJob);
		//fnReceiveProcessStart(id);

		str.Format("[%s] Process Start, ID : %s", get_job_name(nJob), m_pMain->vt_job_info[nJob].main_object_id.c_str());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		AcquireSRWLockExclusive(&m_pMain->g_srwlock); // 동기화 시킴 Tkyuha 20211027
		m_pMain->m_nSeqProcess[nJob] = 100;
		ReleaseSRWLockExclusive(&m_pMain->g_srwlock);
		m_pMain->setProcessStartFlag(nJob);
	}

	//BOOL seq_timeout = seq_ChkTimer(tm);
	//if (seq_timeout || g_CommPLC.GetBit(m_pMain->m_nInspRequestRead[nJob]) != TRUE && getProcessStart(nJob))	// Align 시작 조건
	if (request_bit != TRUE && getProcessStart(nJob))	// Align 시작 조건
	//KJH 2022-06-06 GrabTime 삭제요청에 따른 삭제 작업
	//if (seq_timeout || request_bit != TRUE && getProcessStart(nJob))	// Align 시작 조건
	{
		CString str;
		
		//if (seq_timeout)
		//{
		//	str.Format("[%s] Seq End Timeout!! forced reset", get_job_name(nJob));
		//	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		//}

		//seq_ResetChkTimer(tm);
		resetProcessStart(nJob);

		g_CommPLC.SetBit(m_pMain->m_nInspOkWrite[nJob], FALSE);
		g_CommPLC.SetBit(m_pMain->m_nInspNgWrite[nJob], FALSE);

		// YCS 2022-10-27 필름 3회 연속 틀어짐 비트 reset
		g_CommPLC.SetBit(m_pMain->m_nInspOkWrite[nJob] + 3, FALSE);

		// KBJ 2022-10-20 Manual Mark Bit Reset 추가
		if (m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_FILM_INSP)
		{
			g_CommPLC.SetBit(m_pMain->m_nAlignOkWrite[nJob] + 9, FALSE);
		}

		str.Format("[%s] Process End", get_job_name(nJob));
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}
}
void CFormMainView::fnProcess_ProcessBendingInsp(int nJob)
{
	if (g_CommPLC.GetBit(m_pMain->m_nInspRequestRead[nJob]) && getProcessStart(nJob) != TRUE)	// Align 시작 조건
	{
		memset(m_pMain->m_stInsp_DistanceResult, 0, sizeof(m_pMain->m_stInsp_DistanceResult));

		for (int i = 0; i < NUM_ALIGN_POINTS; i++)
		{
			m_pMain->m_dDistanceInspJudgeResult[i] = TRUE;
			m_pMain->m_dDistanceInspDataResult[i] = 0;
		}

		CString str;
		std::vector<int> camBuf = m_pMain->vt_job_info[nJob].camera_index;
		int nWordSize = 2;
		m_pMain->m_nSeqTotalInspGrabCount = 0;
		m_pMain->m_nSeqCurrentInspGrabCount = 0;

		long lData[20] = { 0, };
		int idx = 0;
		g_CommPLC.GetWord(m_pMain->m_nInspWordDataRead[nJob] + 20, 20, lData);

		// 총 Grab 횟수 저장
		idx = 0;
		m_pMain->m_nSeqTotalInspGrabCount = int(lData[idx]);

		/* 21.03.13 인터페이스 바꾸면서 없앰
		// 현재 포지션 저장
		idx = 2;
		m_pMain->m_nSeqCurrentInspGrabCount = int(lData[idx]);			// 현재 포지션 순서
		m_pMain->m_nSeqCurrentInspGrabCount -= 1;						// 1,2,3,4 -> 0,1,2,3
		*/

		// 카메라 Grab 위치 저장
		idx = 2;
		for (int GrabCount = 0; GrabCount < MAX_INSP_COUNT; GrabCount++)
		{
			double dCamera1Pos = MAKELONG(lData[idx + GrabCount * nWordSize], lData[idx + GrabCount * nWordSize + 1]) / MOTOR_SCALE;
			m_pMain->m_stInsp_DistanceResult[camBuf.at(0)][GrabCount].dCameraPos = dCamera1Pos;
			str.Format("[%s] GrabPos Cam 1 - %d : %.3f", get_job_name(nJob), get_job_name(nJob), m_pMain->m_nSeqCurrentInspGrabCount + 1, m_pMain->m_nSeqTotalInspGrabCount, dCamera1Pos);
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

			double dCamera2Pos = MAKELONG(lData[idx + GrabCount * nWordSize + 10], lData[idx + GrabCount * nWordSize + 11]) / MOTOR_SCALE;
			m_pMain->m_stInsp_DistanceResult[camBuf.at(1)][GrabCount].dCameraPos = dCamera2Pos;
			str.Format("[%s] GrabPos Cam 2 - %d : %.3f", get_job_name(nJob), get_job_name(nJob), m_pMain->m_nSeqCurrentInspGrabCount + 1, m_pMain->m_nSeqTotalInspGrabCount, dCamera1Pos);
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		}

		/*
		long lData[2] = { 0, };
		g_CommPLC.GetWord(m_pMain->m_nInspWordDataRead[nJob], 2, lData);
		m_pMain->m_nSeqTotalInspGrabCount = int(lData[0]);

		// 현재 포지션 저장
		g_CommPLC.GetWord(m_pMain->m_nInspWordDataRead[nJob] + 2, 2, lData);
		m_pMain->m_nSeqCurrentInspGrabCount = int(lData[0]);			// 현재 포지션 순서
		m_pMain->m_nSeqCurrentInspGrabCount -= 1;						// 1,2,3,4 -> 0,1,2,3

		// 카메라 Grab 위치 저장
		long lData2[20] = { 0, };
		g_CommPLC.GetWord(m_pMain->m_nInspWordDataRead[nJob] + 20, 20, lData2);	// 총 카메라 포지션 * 카메라 갯수 * 데이터크기
		for (int GrabCount = 0; GrabCount < MAX_INSP_COUNT; GrabCount++)
		{
			double dCamera1Pos = MAKELONG(lData2[GrabCount * nWordSize], lData2[GrabCount * nWordSize + 1]) / MOTOR_SCALE;
			m_pMain->m_stInsp_DistanceResult[camBuf.at(0)][GrabCount].dCameraPos = dCamera1Pos;
			str.Format("[%s] GrabPos Cam 1 - %d : %.3f", get_job_name(nJob), get_job_name(nJob), m_pMain->m_nSeqCurrentInspGrabCount + 1, m_pMain->m_nSeqTotalInspGrabCount, dCamera1Pos);
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

			double dCamera2Pos = MAKELONG(lData2[GrabCount * nWordSize + 10], lData2[GrabCount * nWordSize + 11]) / MOTOR_SCALE;
			m_pMain->m_stInsp_DistanceResult[camBuf.at(1)][GrabCount].dCameraPos = dCamera2Pos;
			str.Format("[%s] GrabPos Cam 2 - %d : %.3f", get_job_name(nJob), get_job_name(nJob), m_pMain->m_nSeqCurrentInspGrabCount + 1, m_pMain->m_nSeqTotalInspGrabCount, dCamera1Pos);
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		}
		*/

		read_glass_id(nJob);
		reset_viewer(nJob);
		
		SendMessageA(WM_VIEW_CONTROL, MSG_FMV_GLASSID_UPDATE, nJob);

		str.Format("[%s] Process Start, ID : %s", get_job_name(nJob), m_pMain->vt_job_info[nJob].main_object_id.c_str());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		setProcessStart(nJob);
		m_pMain->setProcessStartFlag(nJob);
		AcquireSRWLockExclusive(&m_pMain->g_srwlock); // 동기화 시킴 Tkyuha 20211027
		m_pMain->m_nSeqProcess[nJob] = 100;
		ReleaseSRWLockExclusive(&m_pMain->g_srwlock);
	}

	if (g_CommPLC.GetBit(m_pMain->m_nInspRequestRead[nJob]) != TRUE && getProcessStart(nJob))
	{
		resetProcessStart(nJob);

		// Bit array로 reset 시키는 부분 추가 필요.
		g_CommPLC.SetBit(m_pMain->m_nInspOkWrite[nJob], FALSE);
		g_CommPLC.SetBit(m_pMain->m_nInspNgWrite[nJob], FALSE);
		g_CommPLC.SetBit(m_pMain->m_nInspAckWrite[nJob] + 0, FALSE);
		g_CommPLC.SetBit(m_pMain->m_nInspAckWrite[nJob] + 1, FALSE);
		g_CommPLC.SetBit(m_pMain->m_nInspAckWrite[nJob] + 2, FALSE);
		g_CommPLC.SetBit(m_pMain->m_nInspAckWrite[nJob] + 3, FALSE);

		CString str;
		str.Format("[%s] Process End", get_job_name(nJob));
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}
}
//HTK 2022-07-04 RotateCenter Insp 기능 추가
void CFormMainView::fnProcess_ProcessRotateCenterPointCheck(int nJob)
{
	CString str;
	BOOL bit = g_CommPLC.GetBit(m_pMain->m_nAlignRequestRead[nJob] + 15);

	if (bit && m_bCalcRotateCenter != TRUE)
	{
		m_bCalcRotateCenter = TRUE;
		// KBJ 2022-11-11
		reset_viewer(nJob);
		HideErrorMessageBlink();

		str.Format("[%s] Rotate center Process Start, ID : %s", get_job_name(nJob), m_pMain->vt_job_info[nJob].main_object_id.c_str());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		// KBJ 2022-02-24 add
		str.Format("[%s] Light On", get_job_name(nJob));
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		
		for (int nCam = 0; nCam < m_pMain->vt_job_info[nJob].light_info.size(); nCam++)
		{
			m_pMain->LightControllerTurnOnOff(nJob, nCam, 0);
		}
		Delay(500);


		// KBJ 2022-09-05 Rotate Light 파라미터 추가.
		int nLight_Value = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getRotateC_light_value();
		if (nLight_Value != 0)
		{
			int ctrl = m_pMain->vt_job_info[nJob].light_info[0].controller_id[0] - 1;
			int ch = m_pMain->vt_job_info[nJob].light_info[0].channel_id[0];

			if (m_pMain->m_stLightCtrlInfo.nType[ctrl] == LTYPE_LLIGHT_LPC_COT_232)
			{
				m_pMain->SetLightBright(ctrl, ch, nLight_Value == 0 ? 0 : nLight_Value * 10);   // dh.jung 2021-07-12
			}
			else
			{
				m_pMain->SetLightBright(ctrl, ch, nLight_Value);
			}
			Delay(50);
		}

		std::vector<int> camBuf = m_pMain->vt_job_info[nJob].camera_index;

		CString str;
		int nCam = 0;
		int ncamera = camBuf.at(nCam);
		int W = m_pMain->m_stCamInfo[ncamera].w;
		int H = m_pMain->m_stCamInfo[ncamera].h;

		int viewer = m_pMain->vt_job_info[nJob].viewer_index[nCam];
		CViewerEx* pView = &m_pDlgViewerMain[viewer]->GetViewer();

		m_pDlgViewerMain[viewer]->GetViewer().ClearOverlayDC();
		m_pDlgViewerMain[viewer]->GetViewer().clearAllFigures();

		m_thInspParam.nCam = ncamera;
		m_thInspParam.nJob = nJob;
		m_thInspParam.pDlg = m_pMain;
		m_thInspParam.mViewer = pView;

		//AfxBeginThread(Thread_RotateCenterCalc, &m_thInspParam); //회전중시 체크 검사
		AfxBeginThread(Thread_RotateCenterCalc2, &m_thInspParam); // KBJ 2022-08-20 이미지 저장 부분 및 엑셀 파일 저장부분 수정
	}                                                  

	if ((g_CommPLC.GetBit(m_pMain->m_nAlignRequestRead[nJob] + 15) != TRUE) && m_bCalcRotateCenter==TRUE)	// Align 시작 조건
	{
		m_bCalcRotateCenter=FALSE;

		g_CommPLC.SetBit(m_pMain->m_nAlignOkWrite[nJob] + 15, FALSE);
		g_CommPLC.SetBit(m_pMain->m_nAlignOkWrite[nJob] + 16, FALSE);
		g_CommPLC.SetBit(m_pMain->m_nAlignOkWrite[nJob] + 7, FALSE);

		// KBJ 2022-09-05 Rotate Light 파라미터 추가.
		int nLight_Value = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getRotateC_light_value();
		if (nLight_Value != 0)
		{
			for (int nCam = 0; nCam < m_pMain->vt_job_info[nJob].light_info.size(); nCam++)
			{
				m_pMain->LightControllerTurnOnOff(nJob, nCam, 0);
			}
			Delay(500);
		}

		CString str;
		str.Format("[%s] Rotate Center Process End", get_job_name(nJob));
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}
}

void CFormMainView::fnProcess_ProcessCenterAlignInspAlgorithm(int nJob)
{
	CString str;

	bool bflag = false;
	static int nSeq[MAX_JOB] = { 0, };
	static bool bHandshake[MAX_JOB] = { false, };
	int tm = STM_PN_ALIGN + nJob;
	int method = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod();

	if (bHandshake[nJob] && !g_CommPLC.GetBit(m_pMain->m_nAlignRequestRead[nJob]) &&
		!g_CommPLC.GetBit(m_pMain->m_nAlignRequestRead[nJob] + 1) && !g_CommPLC.GetBit(m_pMain->m_nAlignRequestRead[nJob] + 2))
	{
		nSeq[nJob] = 0;
	}

#ifndef JOB_INFO
	int camCount = m_pAlgorithmInfo.getAlgorithmCameraNum(nJob);
	std::vector<int> camBuf = m_pAlgorithmInfo.getAlgorithmCameraIndex(nJob);
#else
	int camCount = m_pMain->vt_job_info[nJob].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[nJob].camera_index;
#endif

	int real_cam = camBuf.at(0);
	// Light index 0: Align
	//       index 1: Metal Trace
	//       index 2: Panel Trace
	//       index 3: Inspection

	int PLC_RB_CENTER_ALIGN_REQ 		= m_pMain->m_nAlignRequestRead[nJob];
	int PLC_RB_CENTER_TRACE_REQ 		= m_pMain->m_nAlignRequestRead[nJob] + 1;
	int PLC_RB_CENTER_INSP_REQ 			= m_pMain->m_nAlignRequestRead[nJob] + 2;
	int PLC_RB_CENTER_DOPO_START_REQ	= m_pMain->m_nAlignRequestRead[nJob] + 8;
	int PLC_RB_CENTER_DUMMMY_INSP_REQ 	= m_pMain->m_nAlignRequestRead[nJob] + 50;    //220304 변경 11에서 50로 변경
	int PLC_RB_CENTER_CG_GRAB_REQ       = m_pMain->m_nAlignRequestRead[nJob] + 11;    //220304 CG Grab모드로 변경
	int PLC_RB_NOZZLE_CHECK_GRAB_REQ 	= m_pMain->m_nAlignRequestRead[nJob] + 17;    //20211005 Tkyuha 노즐과 원외곽 거리 측정
	
	int PLC_WB_ALIGN_JUDGE_OK 			= m_pMain->m_nAlignOkWrite[nJob];				//20
	int PLC_WB_TRACE_JUDGE_OK 			= m_pMain->m_nAlignOkWrite[nJob] + 2;
	int PLC_WB_INSP_JUDGE_OK 			= m_pMain->m_nAlignOkWrite[nJob] + 4;
	int PLC_WB_CENTER_MANUAL_ALIGN_REQ  = m_pMain->m_nAlignOkWrite[nJob] + 8;
	int PLC_WB_CENTER_MANUAL_ALIGN_ACK  = m_pMain->m_nAlignOkWrite[nJob] + 9;
	int PLC_WB_TRACE_GRAB_COMPLETE_ACK	= m_pMain->m_nAlignOkWrite[nJob] + 10;
	int PLC_WB_GRAB_CG_OK               = m_pMain->m_nAlignOkWrite[nJob] + 11;
	int PLC_WB_DIFF_GRAB_COMPLETE_ACK   = m_pMain->m_nAlignOkWrite[nJob] + 13;
	int PLC_WB_NOZZLE_CHECK_OK_ACK 	    = m_pMain->m_nAlignOkWrite[nJob] + 17;
	
	int PLC_WB_ALIGN_JUDGE_NG 			= m_pMain->m_nAlignNgWrite[nJob];				//21
	int PLC_WB_TRACE_JUDGE_NG 			= m_pMain->m_nAlignNgWrite[nJob] + 2;
	int PLC_WB_INSP_JUDGE_NG 			= m_pMain->m_nAlignNgWrite[nJob] + 4;
	int PLC_WB_GRAB_CG_NG               = m_pMain->m_nAlignNgWrite[nJob] + 11;
	int PLC_WB_NOZZLE_CHECK_NG_ACK		= m_pMain->m_nAlignNgWrite[nJob] + 17;

	int PLC_RW_LEFT_DOPO_LENGTH 		= m_pMain->vt_job_info[nJob].plc_addr_info.read_word_start + 24;
	int PLC_RW_RIGHT_DOPO_LENGTH	 	= m_pMain->vt_job_info[nJob].plc_addr_info.read_word_start + 26;
	int PLC_RW_ROTATE_OFFSET_X 			= m_pMain->vt_job_info[nJob].plc_addr_info.read_word_start + 8;

	BOOL bManualJudge = FALSE;

	/// <summary>
	///	Image Buffer Index
	///  0 : Sub Insp
	///	 1 : Center Align
	///  2 : Trace
	///  3 : Diff Insp
	/// </summary>

	switch (nSeq[nJob])
	{
	case ALIGN_CENTER_RESET:
		str.Format("[%s] Process End", get_job_name(nJob));
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		
		HideErrorMessageBlink();

		resetProcessStart(nJob);

		g_CommPLC.SetBit(PLC_WB_ALIGN_JUDGE_OK			, FALSE);
		g_CommPLC.SetBit(PLC_WB_TRACE_JUDGE_OK			, FALSE);
		g_CommPLC.SetBit(PLC_WB_INSP_JUDGE_OK			, FALSE);
		g_CommPLC.SetBit(PLC_WB_ALIGN_JUDGE_NG			, FALSE);
		g_CommPLC.SetBit(PLC_WB_TRACE_JUDGE_NG			, FALSE);
		g_CommPLC.SetBit(PLC_WB_INSP_JUDGE_NG			, FALSE);
		g_CommPLC.SetBit(PLC_WB_GRAB_CG_OK				, FALSE);
		g_CommPLC.SetBit(PLC_WB_GRAB_CG_NG				, FALSE);
		g_CommPLC.SetBit(PLC_WB_ALIGN_JUDGE_OK + 10		, FALSE);
		g_CommPLC.SetBit(PLC_WB_ALIGN_JUDGE_NG + 10		, FALSE);
		//HTK 2022-07-11 Diff Insp Grab bit & Reset추가
		g_CommPLC.SetBit(PLC_WB_DIFF_GRAB_COMPLETE_ACK	, FALSE);
		//KJH 2022-07-14 Trace Grab bit & Reset추가
		g_CommPLC.SetBit(PLC_WB_TRACE_GRAB_COMPLETE_ACK	, FALSE);

		//KJH 2021-09-13 Nozzle Check 관련 Grab 인자 Reset 추가
		g_CommPLC.SetBit(PLC_WB_NOZZLE_CHECK_OK_ACK		, FALSE);
		g_CommPLC.SetBit(PLC_WB_NOZZLE_CHECK_NG_ACK		, FALSE);
		g_CommPLC.SetBit(PLC_WB_CENTER_MANUAL_ALIGN_REQ	, FALSE);
		g_CommPLC.SetBit(PLC_WB_CENTER_MANUAL_ALIGN_ACK	, FALSE);

		//KJH 2021-05-28 이미지 갱신관련 인자 추가
		m_pMain->m_ELB_TraceResult.bType = FALSE;

		bHandshake[nJob] = false;
		m_bHandShake[nJob] = false;

		bManualJudge = FALSE;

		nSeq[nJob] = ALIGN_CENTER_GRAB;
		break;
	case ALIGN_CENTER_GRAB:
	{
		if (g_CommPLC.GetBit(PLC_RB_CENTER_DOPO_START_REQ) == FALSE ) m_bRecordCheck = FALSE;

		//if (g_CommPLC.GetBit(m_pMain->m_nAlignRequestRead[nJob])) // 첫번째 얼라인 수행
		if (g_CommPLC.GetBit(PLC_RB_CENTER_ALIGN_REQ)) //얼라인 수행 준비 m_pMain->m_nSeqProcess[nJob] = 100;
		{
			//KJH 2022-01-05 Center Nozzle Align 시작하기 전에 ExposureTime 변경
			//Center Cam Index 0
			int nPos = m_pMain->vt_job_info[nJob].model_info.getMachineInfo().getExposureTime(real_cam);

#ifdef _DAHUHA
			if (m_pMain->m_pDahuhaCam[real_cam].isconnected())
				m_pMain->m_pDahuhaCam[real_cam].setExposureTime(nPos);
#else
			if (m_pMain->m_pBaslerCam[real_cam])
			{
				m_pMain->m_pBaslerCam[real_cam]->SetExposure(nPos);
			}
#endif

			m_pMain->LightControllerTurnOnOff(nJob, 0, 0, FALSE, FALSE, TRUE);

			str.Format("[%s] Set Exposure Time : %d", get_job_name(nJob), nPos);
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

			read_glass_id(nJob);
			reset_viewer(nJob);
			SendMessageA(WM_VIEW_CONTROL, MSG_FMV_GLASSID_UPDATE, nJob);

			str.Format("[%s] Process Start, ID : %s", get_job_name(nJob), m_pMain->vt_job_info[nJob].main_object_id.c_str());
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

			// =====================================================
			// KBJ 2022-07-22 Center Align Live 화면 빨간선 매번그리기
			int nViewer = m_pMain->vt_job_info[nJob].viewer_index[1];

			int W = m_pDlgViewerMain[nViewer]->GetViewer().GetWidth();
			int H = m_pDlgViewerMain[nViewer]->GetViewer().GetHeight();

			double xres = m_pDlgViewerMain[nViewer]->GetViewer().getCameraResolutionX();
			double yres = m_pDlgViewerMain[nViewer]->GetViewer().getCameraResolutionY();

			double centerPtX = W / 2, cetnerPtY = H / 2;

			// YCS 2022-11-21 Center Align Live YGAP 가이드라인 매번그리기
			double _offset = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getCenterLiveViewGuideLineOffset();
			double ylineoffset = _offset / yres;
			if (_offset != 0)
			{
				m_pDlgViewerMain[nViewer]->GetViewer().AddSoftGraphic(new GraphicLine(cv::Point2d(0, cetnerPtY - ylineoffset), cv::Point2d(W, cetnerPtY - ylineoffset), Gdiplus::Color::Blue));
			}

			ylineoffset = 0.03 / yres;
			stFigure tempFig;
			tempFig.ptBegin.x = 0;
			tempFig.ptBegin.y = cetnerPtY - ylineoffset;
			tempFig.ptEnd.x = W;
			tempFig.ptEnd.y = cetnerPtY - ylineoffset;
			m_pDlgViewerMain[nViewer]->GetViewer().addFigureLine(tempFig, 1, 1, COLOR_RED);
			// =====================================================

			if (method == METHOD_LINE || method == METHOD_NOTCH)
			{
				long lData[8] = { 0, };
				double totalLength = 0;
				
				g_CommPLC.GetWord(m_pMain->vt_job_info[nJob].plc_addr_info.read_word_start, 8, lData);
				
				m_dLineNum			= MAKELONG(lData[0], lData[1]);
				m_dDopoType			= MAKELONG(lData[2], lData[3]);
				m_dAccLineLength	= MAKELONG(lData[4], lData[5]) / MOTOR_SCALE;
				m_dDecLineLength	= MAKELONG(lData[6], lData[7]) / MOTOR_SCALE;

				totalLength = m_dAccLineLength + m_dDecLineLength;

				// Line 인 경우 위 0,1번 에서 제거하고 아래 값을 totalLength에 더해줘야함

				g_CommPLC.GetWord(PLC_RW_LEFT_DOPO_LENGTH, 2, lData);
				m_dLeftNotchLength = MAKELONG(lData[0], lData[1]) / MOTOR_SCALE;

				g_CommPLC.GetWord(PLC_RW_RIGHT_DOPO_LENGTH, 2, lData);
				m_dRightNotchLength = MAKELONG(lData[0], lData[1]) / MOTOR_SCALE;

				totalLength = m_dLeftNotchLength + m_dRightNotchLength + totalLength;

				CString sDopoType;
				if (m_dDopoType == 1)
				{
					sDopoType.Format("Line");
				}
				else if (m_dDopoType == 2)
				{
					sDopoType.Format("Hole");
				}
				else if (m_dDopoType == 3)
				{
					sDopoType.Format("Notch");
				}

				str.Format("[%s] 1st -> Dopo Type : %s, Pos : %d, Dopo Lenth : %.3f, Acc : %.3f, Dec : %.3f", get_job_name(nJob), sDopoType, m_dLineNum, totalLength, m_dAccLineLength, m_dDecLineLength);
				::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

				m_pMain->vt_job_info[nJob].model_info.getAlignInfo().setInspRangeLength(totalLength);
			}

			memset(m_pMain->vt_result_info[nJob].each_Judge, 0, sizeof(m_pMain->vt_result_info[nJob].each_Judge));

			setProcessStart(nJob);	// 여기 함수 진입 flag

			// 실제 Align Sequence 동작하는 시점.
			// ProcessPrealignINFRA함수로 Grab명령 보내기
			AcquireSRWLockExclusive(&m_pMain->g_srwlock); // 동기화 시킴 Tkyuha 20211027
			m_pMain->m_nSeqProcess[nJob] = 100;
			ReleaseSRWLockExclusive(&m_pMain->g_srwlock);
			m_pMain->setProcessStartFlag(nJob);

			// 자체 타이머 기능 온
			if (!m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getEnableAlignMeasureSpecOutJudge())		//seq_SetTimer(tm, 10000);
			{
				bHandshake[nJob] = true;
				m_bHandShake[nJob] = true;
			}
			nSeq[nJob] = ALIGN_WAIT_OFF_REQ;
		}
		else if (g_CommPLC.GetBit(PLC_RB_CENTER_TRACE_REQ)) // Trace 요청시 Trace 이미지 Grab 사전준비
		{
			int nPos = m_pMain->vt_job_info[nJob].model_info.getMachineInfo().getExposureTime(real_cam);

#ifdef _DAHUHA
			if (m_pMain->m_pDahuhaCam[real_cam].isconnected())
				m_pMain->m_pDahuhaCam[real_cam].setExposureTime(nPos);
#else
			if (m_pMain->m_pBaslerCam[real_cam])
			{
				m_pMain->m_pBaslerCam[real_cam]->SetExposure(nPos);
			}
#endif

			m_pMain->LightControllerTurnOnOff(nJob, 0, 0, FALSE, FALSE, TRUE);

			//KJH 2021-05-29 Trace시 패널 정보 다시 읽어오기
			read_glass_id(nJob);
			if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseRealTimeTrace())
			{
				if (m_pMain->vt_job_info[nJob].num_of_viewer == 2)
				{
					reset_viewer(nJob);
				}
			}
			nSeq[nJob] = ALIGN_TRACE_GRAB_START;
		}
		else if (g_CommPLC.GetBit(PLC_RB_CENTER_INSP_REQ)) // Insp 요청시 Insp Pass인지 판단
		{
			int nPos = m_pMain->vt_job_info[nJob].model_info.getMachineInfo().getExposureTime(real_cam);

#ifdef _DAHUHA
			if (m_pMain->m_pDahuhaCam[real_cam].isconnected())
				m_pMain->m_pDahuhaCam[real_cam].setExposureTime(nPos);
#else
			if (m_pMain->m_pBaslerCam[real_cam])
			{
				m_pMain->m_pBaslerCam[real_cam]->SetExposure(nPos);
			}
#endif

			m_pMain->LightControllerTurnOnOff(nJob, 0, 0, FALSE, FALSE, TRUE);

			str.Format("[%s] ELB Inspection Start", get_job_name(nJob));
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

			// KBJ 2022-11-30 결과 시간을 도포 시간과 동일하게 하기 위하여 껏다킨 경우가 아니면 안읽도록
			if(m_pMain->vt_job_info[nJob].main_object_id.size() <= 0) read_glass_id(nJob);
			bHandshake[nJob] = true;
			m_bHandShake[nJob] = true;

			if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getUseDiffInsp())
			{
				nSeq[nJob] = ALIGN_INSP_GRAB_START;
			}
			else
			{
				str.Format("[%s] Diff Insp Pass", get_job_name(nJob));
				::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

				g_CommPLC.SetBit(PLC_WB_INSP_JUDGE_OK, TRUE);  // 강제로 OK하고 끝냄				
				nSeq[nJob] = ALIGN_DIFF_INSP_END;
			}
		}
		else if (g_CommPLC.GetBit(PLC_RB_CENTER_DUMMMY_INSP_REQ)) // Dummy Glass Test Program
		{
			int nPos = m_pMain->vt_job_info[nJob].model_info.getMachineInfo().getExposureTime(real_cam);

#ifdef _DAHUHA
			if (m_pMain->m_pDahuhaCam[real_cam].isconnected())
				m_pMain->m_pDahuhaCam[real_cam].setExposureTime(nPos);
#else
			if (m_pMain->m_pBaslerCam[real_cam])
			{
				m_pMain->m_pBaslerCam[real_cam]->SetExposure(nPos);
			}
#endif

			m_pMain->LightControllerTurnOnOff(nJob, 0, 0, FALSE, FALSE, TRUE);

			long lData[2] = { 0, };
			g_CommPLC.GetWord(m_pMain->vt_job_info[nJob].plc_addr_info.read_word_start, 2, lData);
			m_dDummyLineNum = MAKELONG(lData[0], lData[1]);

#ifdef _DAHUHA
			if ( m_pMain->m_pDahuhaCam[real_cam].isconnected())
#else
			if (m_pMain->m_pBaslerCam[real_cam]->IsOpen())
#endif
			{
				str.Format("[%s] Image Grab Start", m_pMain->vt_job_info[nJob].job_name.c_str());
				::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

				
#ifdef _DAHUHA
				AcquireSRWLockExclusive(&m_pMain->m_pDahuhaCam[real_cam].g_bufsrwlock); // 동기화 시킴 Tkyuha 20211027
#else
				AcquireSRWLockExclusive(&m_pMain->m_pBaslerCam[real_cam]->g_bufsrwlock); // 동기화 시킴 Tkyuha 20211027
#endif
				m_pMain->m_bGrabEnd[real_cam] = FALSE;
#ifdef _DAHUHA
				ReleaseSRWLockExclusive(&m_pMain->m_pDahuhaCam[real_cam].g_bufsrwlock);
#else
				ReleaseSRWLockExclusive(&m_pMain->m_pBaslerCam[real_cam]->g_bufsrwlock);
#endif
				::SendMessage(m_pMain->m_hWnd, WM_VIEW_CONTROL, MSG_MD_CAMERA_SINGLE_FRAME, real_cam);
			}

			//seq_SetTimer(tm, 3000);
			nSeq[nJob] = ALIGN_DUMMY_WAIT_GRAB_END;
		}
		else if (g_CommPLC.GetBit(PLC_RB_CENTER_CG_GRAB_REQ)) // CG Grab
		{
			int method = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod();
			
			if (method == METHOD_LINE)
			{
				// PJH 22-10-05 EGL 요청 VIEW Cam Light On
				 m_pMain->LightControllerTurnOnOff(1, 0, 0, FALSE, FALSE, TRUE);
				// KBJ 2022-11-30 CG 사이드 조명 킬시 빛이 반사되어 끄는게 낫다고 판단 Light Off
				// m_pMain->LightControllerTurnOnOff(1, 0, 0, FALSE, TRUE, TRUE);
			}

			//SJB 2022-11-19 CG Grab Exposure Time 파라미터 값으로 변경
			int nPos = (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getCGGrabExposure() * 100);

#ifdef _DAHUHA
			if (m_pMain->m_pDahuhaCam[real_cam].isconnected())
#else
			if (m_pMain->m_pBaslerCam[real_cam]->IsOpen())
#endif
			{
				//SJB 2022-11-19 CG Grab Exposure Time 파라미터 값으로 변경
#ifdef _DAHUHA
				if (nPos > 0)
					m_pMain->m_pDahuhaCam[real_cam].setExposureTime(nPos);
#else
				if (nPos > 0)
					m_pMain->m_pBaslerCam[real_cam]->SetExposure(nPos);
#endif

				str.Format("[%s] CG Image Grab Start", m_pMain->vt_job_info[nJob].job_name.c_str());
				::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

#ifdef _DAHUHA
				AcquireSRWLockExclusive(&m_pMain->m_pDahuhaCam[real_cam].g_bufsrwlock); // 동기화 시킴 Tkyuha 20211027
#else
				AcquireSRWLockExclusive(&m_pMain->m_pBaslerCam[real_cam]->g_bufsrwlock); // 동기화 시킴 Tkyuha 20211027
#endif
				m_pMain->m_bGrabEnd[real_cam] = FALSE;
#ifdef _DAHUHA
				ReleaseSRWLockExclusive(&m_pMain->m_pDahuhaCam[real_cam].g_bufsrwlock);
#else
				ReleaseSRWLockExclusive(&m_pMain->m_pBaslerCam[real_cam]->g_bufsrwlock);
#endif
				::SendMessage(m_pMain->m_hWnd, WM_VIEW_CONTROL, MSG_MD_CAMERA_SINGLE_FRAME, real_cam);
			}

			//seq_SetTimer(tm, 3000);
			nSeq[nJob] = ALIGN_CG_WAIT_GRAB_END;

			str.Format("[%s] CG Image Grab End", m_pMain->vt_job_info[nJob].job_name.c_str());
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		}		
		else if (g_CommPLC.GetBit(PLC_RB_NOZZLE_CHECK_GRAB_REQ)) // Nozzle Check Grab Image
		{
			int nPos = m_pMain->vt_job_info[nJob].model_info.getMachineInfo().getExposureTime(real_cam);

#ifdef _DAHUHA
			if (m_pMain->m_pDahuhaCam[real_cam].isconnected())
				m_pMain->m_pDahuhaCam[real_cam].setExposureTime(nPos);
#else
			if (m_pMain->m_pBaslerCam[real_cam])
			{
				m_pMain->m_pBaslerCam[real_cam]->SetExposure(nPos);
			}
#endif

			m_pMain->LightControllerTurnOnOff(nJob, 0, 0, FALSE, FALSE, TRUE);
			Delay(300);

			int viewer = m_pMain->vt_job_info[nJob].viewer_index[0];
			m_pDlgViewerMain[viewer]->setViewerJudge(FALSE);

			// ksm 2022-08-10 Y Gap 시 노즐 Side Lamp On
			//=======================================================================================//
			int ctrl_side = m_pMain->vt_job_info[2].light_info[1].controller_id[0] - 1;
			int ch_side = m_pMain->vt_job_info[2].light_info[1].channel_id[0];
			int value_side = m_pMain->vt_job_info[2].model_info.getLightInfo().getLightBright(1, 0, 0);
			int ch_center = m_pMain->vt_job_info[0].light_info[0].channel_id[0];
			int value_center = m_pMain->vt_job_info[0].model_info.getLightInfo().getLightBright(0, 0, 0);

			//if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseMetalTraceFirst() == TRUE)
			// KJH2 2022-08-17 Metal_Trace_First -> Y Light Enable/Disalbe 으로 판단  
			if(m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getUseCenterNozzleYLight()== TRUE)
			{
				value_side = 255;
				value_center = 0;

				if (m_pMain->m_stLightCtrlInfo.nType[ctrl_side] == LTYPE_LLIGHT_LPC_COT_232)
				{
					m_pMain->SetLightBright(ctrl_side, ch_side, value_side == 0 ? 0 : value_side * 10);   // dh.jung 2021-07-12
					Delay(50);
					m_pMain->SetLightBright(ctrl_side, ch_center, value_center == 0 ? 0 : value_center * 10);   // dh.jung 2021-07-12
					Delay(50);
				}
				else
				{
					m_pMain->SetLightBright(ctrl_side, ch_side, value_side);
					Delay(50);
					m_pMain->SetLightBright(ctrl_side, ch_center, value_center);
					Delay(50);
				}
				Delay(300);
			}
			//=======================================================================================//

#ifdef _DAHUHA
		  if (m_pMain->m_pDahuhaCam[real_cam].isconnected())
#else
			if (m_pMain->m_pBaslerCam[real_cam]->IsOpen())
#endif
			{
				str.Format("[%s] Nozzle Check Image Grab Start", m_pMain->vt_job_info[nJob].job_name.c_str());
				::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

#ifdef _DAHUHA
				AcquireSRWLockExclusive(&m_pMain->m_pDahuhaCam[real_cam].g_bufsrwlock); // 동기화 시킴 Tkyuha 20211027
#else
				AcquireSRWLockExclusive(&m_pMain->m_pBaslerCam[real_cam]->g_bufsrwlock); // 동기화 시킴 Tkyuha 20211027
#endif
				m_pMain->m_bGrabEnd[real_cam] = FALSE;
#ifdef _DAHUHA
				ReleaseSRWLockExclusive(&m_pMain->m_pDahuhaCam[real_cam].g_bufsrwlock);
#else
				ReleaseSRWLockExclusive(&m_pMain->m_pBaslerCam[real_cam]->g_bufsrwlock);
#endif
				::SendMessage(m_pMain->m_hWnd, WM_VIEW_CONTROL, MSG_MD_CAMERA_SINGLE_FRAME, real_cam);
			}

			//seq_SetTimer(tm, 3000);
			nSeq[nJob] = ALIGN_NOZZLE_CHECK_GRAB_END;
		}
		else if (g_CommPLC.GetBit(PLC_RB_CENTER_DOPO_START_REQ) && !m_bRecordCheck)
		{
			m_bRecordCheck = TRUE;

			int method = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod();
			if (method == METHOD_LINE)
			{
				// PJH 22-10-05 EGL 요청 VIEW Cam Light On
				m_pMain->LightControllerTurnOnOff(1, 0, 0, FALSE, FALSE, TRUE);
			}

			read_glass_id(nJob);
			SendMessageA(WM_VIEW_CONTROL, MSG_FMV_GLASSID_UPDATE, nJob);

			//KJH 2022-04-29 Dopo Start시 동영상 녹화 기능
			int viewer = m_pMain->vt_job_info[nJob].viewer_index[0];
			int W = m_pMain->m_stCamInfo[real_cam].w;
			int H = m_pMain->m_stCamInfo[real_cam].h;

			if (m_pMain->vt_job_info[0].model_info.getAlignInfo().getUseAutoSaveAVI())
			{
				if (m_pDlgViewerMain[viewer + 1]->m_bEnableGrab != TRUE) m_pDlgViewerMain[viewer + 1]->m_bLiveCheck = TRUE;
				if (m_pDlgViewerMain[viewer + 2]->m_bEnableGrab != TRUE) m_pDlgViewerMain[viewer + 2]->m_bLiveCheck = TRUE;

				if (!m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getAVI_Save_ImageTypeEnable())
				{ // 동영상 저장시 죽는 문제 있다고 해서 강제로 종료 시킴
					::SendMessageA(m_pDlgViewerMain[viewer + 1]->GetSafeHwnd(), WM_VIEW_CONTROL, 300, 0);
					int _nSView = m_pMain->vt_job_info[nJob + 1].viewer_index[0];
					::SendMessageA(m_pDlgViewerMain[_nSView]->GetSafeHwnd(), WM_VIEW_CONTROL, 300, 0);
				}

				if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getAVI_Save_ImageTypeEnable())
				{
					m_iSaveImageTypeNum = 0;
					SetTimer(10000, 500, NULL);
				}
				else
				{
					::SendMessageA(m_pDlgViewerMain[viewer + 1]->GetSafeHwnd(), WM_VIEW_CONTROL, 200, 0);
					//  사이드뷰 자동 저장 기능 활성화 20220110 Tkyuha
					int _nSView = m_pMain->vt_job_info[nJob + 1].viewer_index[0];
					::SendMessageA(m_pDlgViewerMain[_nSView]->GetSafeHwnd(), WM_VIEW_CONTROL, 200, 0);

					if (method == METHOD_LINE)
					{
						if (m_dLineNum == 1)		SetTimer(viewer + 1, 12000, NULL); // 자동으로 녹화 중지
						else						SetTimer(viewer + 1, 5500, NULL);
					}
					else
					{
						setProcessStart(nJob + 1);
						AcquireSRWLockExclusive(&m_pMain->g_srwlock); // 45도 카메라 검사 시작 Bit
						m_pMain->m_nSeqProcess[nJob + 1] = 100;
						ReleaseSRWLockExclusive(&m_pMain->g_srwlock);
						
						m_pMain->setProcessStartFlag(nJob+1);
						SetTimer(viewer + 1, m_pMain->vt_system_option[nJob].stELB.Auto_Save_Video_Time * 1000, NULL);  // 홀 ,노치 인경우 4초 대기시간
					}

					SetTimer(_nSView, m_pMain->vt_system_option[nJob].stELB.Auto_Save_Video_Time * 1000, NULL);  // 45도 카메라 동영상 저장 종료 타이머
				}
			}
		}
	}
	break;
	case ALIGN_DUMMY_WAIT_GRAB_END:
	{
		BOOL bGrabComplete = TRUE;
		if (m_pMain->m_bGrabEnd[real_cam] != TRUE)			bGrabComplete = FALSE;
		//KJH 2022-06-06 GrabTime 삭제요청에 따른 삭제 작업
		//if (seq_ChkTimer(tm) || bGrabComplete || !g_CommPLC.GetBit(PLC_RB_CENTER_DUMMMY_INSP_REQ))
		if (bGrabComplete || !g_CommPLC.GetBit(PLC_RB_CENTER_DUMMMY_INSP_REQ))
		{
			//seq_ResetChkTimer(tm);
			int viewer = m_pMain->vt_job_info[nJob].viewer_index[0];
			//BOOL b=InspCircleDummyDetection(nJob, real_cam, NULL, &m_pDlgViewerMain[viewer]->GetViewer());
			m_pDlgViewerMain[viewer]->GetViewer().OnLoadImageFromPtr(m_pMain->getSrcBuffer(real_cam));

			BOOL b = InspLineDummyDetection(nJob, real_cam, NULL, &m_pDlgViewerMain[viewer]->GetViewer());
	
			if (!b) 
			{
				g_CommPLC.SetBit(PLC_WB_ALIGN_JUDGE_NG + 50, TRUE);  // 나중에 PLC와 인터페이스 정해야함 //강제로 NG하고 끝냄
				m_pDlgViewerMain[viewer]->setViewerJudge(FALSE);
				str.Format("[%s] Dummy Line Check NG", m_pMain->vt_job_info[nJob].job_name.c_str());
				ShowErrorMessageBlink(str);
				m_pMain->write_NGresult_SummaryFile(m_pMain->vt_job_info[nJob].main_object_id.c_str(), str, FALSE, nJob);
			}
			else {
				g_CommPLC.SetBit(PLC_WB_ALIGN_JUDGE_OK + 50, TRUE);  // 나중에 PLC와 인터페이스 정해야함 //강제로 OK하고 끝냄		
				m_pDlgViewerMain[viewer]->setViewerJudge(TRUE);
			}
						
			m_pDlgViewerMain[viewer]->GetViewer().Invalidate();

			nSeq[nJob] = ALIGN_DUMMY_INSP_END;
		}
	}
	break;
	case ALIGN_CG_WAIT_GRAB_END:
	{
		BOOL bGrabComplete = TRUE;
		if (m_pMain->m_bGrabEnd[real_cam] != TRUE)			bGrabComplete = FALSE;
		//KJH 2022-06-06 GrabTime 삭제요청에 따른 삭제 작업
		//if (seq_ChkTimer(tm) || bGrabComplete || !g_CommPLC.GetBit(PLC_RB_CENTER_CG_GRAB_REQ))
		if (bGrabComplete || !g_CommPLC.GetBit(PLC_RB_CENTER_CG_GRAB_REQ))
		{
			str.Format("[%s] CG Inspection Start", m_pMain->vt_job_info[nJob].main_object_id.c_str());
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

			//seq_ResetChkTimer(tm);
			int viewer = m_pMain->vt_job_info[nJob].viewer_index[0];
			int W = m_pMain->m_stCamInfo[real_cam].w;
			int H = m_pMain->m_stCamInfo[real_cam].h;
			
			m_pMain->copyMemory(m_pMain->getProcBuffer(real_cam, 0), m_pMain->getSrcBuffer(real_cam), W* H);
			cv::Mat img(H, W, CV_8UC1, m_pMain->getProcBuffer(real_cam, 0));
			double b = InspLineCGDetection(nJob, real_cam, &img, &m_pDlgViewerMain[viewer]->GetViewer());

			if (b==0)
			{
				g_CommPLC.SetBit(PLC_WB_GRAB_CG_NG, TRUE);  // 나중에 PLC와 인터페이스 정해야함 //강제로 NG하고 끝냄
				m_pDlgViewerMain[viewer]->setViewerJudge(FALSE);
				str.Format("[%s] CG Grab Check NG", m_pMain->vt_job_info[nJob].job_name.c_str());
				ShowErrorMessageBlink(str);
				m_pMain->write_NGresult_SummaryFile(m_pMain->vt_job_info[nJob].main_object_id.c_str(), str, FALSE, nJob);
			}
			else {
				g_CommPLC.SetBit(PLC_WB_GRAB_CG_OK, TRUE);  // 나중에 PLC와 인터페이스 정해야함 //강제로 OK하고 끝냄		
				m_pDlgViewerMain[viewer]->setViewerJudge(TRUE);
			}

			m_pDlgViewerMain[viewer]->GetViewer().OnLoadImageFromPtr(m_pMain->getProcBuffer(real_cam, 0));			
			m_pDlgViewerMain[viewer]->GetViewer().Invalidate();

			nSeq[nJob] = ALIGN_CG_GRAB_RESET;

			str.Format("[%s] CG Inspection End", m_pMain->vt_job_info[nJob].main_object_id.c_str());
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

			//SJB 2022-11-23 CG Grab 후 Exposure Time 변경 시점 수정
			int nExposure_value = m_pMain->vt_job_info[nJob].model_info.getMachineInfo().getExposureTime(real_cam);
#ifdef _DAHUHA
			m_pMain->m_pDahuhaCam[real_cam].setExposureTime(nExposure_value);
#else
			m_pMain->m_pBaslerCam[real_cam]->SetExposure(nExposure_value);
#endif
		}
	}
	break;
	//KJH 2021-09-13 Nozzle Check 관련 Grab 인자 Reset 추가
	case ALIGN_NOZZLE_CHECK_GRAB_END:
	{
		BOOL bGrabComplete = TRUE;
		if (m_pMain->m_bGrabEnd[real_cam] != TRUE)			bGrabComplete = FALSE;
		//KJH 2022-06-06 GrabTime 삭제요청에 따른 삭제 작업
		//if (seq_ChkTimer(tm) || bGrabComplete || !g_CommPLC.GetBit(PLC_RB_NOZZLE_CHECK_GRAB_REQ))
		if (bGrabComplete || !g_CommPLC.GetBit(PLC_RB_NOZZLE_CHECK_GRAB_REQ))
		{
			//seq_ResetChkTimer(tm);
			
			int viewer = m_pMain->vt_job_info[nJob].viewer_index[0];
			//KJH2 2022-08-17 Panel Gray -> Nozzle YGapThreshold 값으로 측정
			int _thresh = m_pMain->vt_job_info[0].model_info.getInspSpecParaInfo().getNozzleYGapThreshold();
			int W = m_pMain->m_stCamInfo[real_cam].w;
			int H = m_pMain->m_stCamInfo[real_cam].h;
			double _dist = 9999.0;
			//if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseMetalTraceFirst() == FALSE)
			// KJH2 2022-08-17 Metal_Trace_First -> Y Light Enable/Disalbe 으로 판단
			m_pMain->copyMemory(m_pMain->getProcBuffer(real_cam, 0), m_pMain->getSrcBuffer(real_cam), W * H);
			_dist = fnCalcNozzleToPN_Distance2(m_pMain->getProcBuffer(real_cam, 0), W, H, nJob, real_cam, _thresh, &m_pDlgViewerMain[viewer]->GetViewer());

			// ksm 2022-08-10 Y Gap 시 노즐 Side Lamp OFF
			// KJH2 2022-08-17 Metal_Trace_First -> Y Light Enable/Disalbe 으로 판단 
			if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getUseCenterNozzleYLight() == TRUE)
			{
				m_pMain->LightControllerTurnOnOff(nJob, 0, 0, FALSE, FALSE, TRUE);
				Delay(300);
			}
			//KJH 2022-07-14 Nozzle Y Gap Insp Spec Judge 추가
			double Min_spec = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(8);
			double Max_spec = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(8);
			bool bNozzlePanelHoleCheck = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getNozzlePanelHoleCheck();

			if(bNozzlePanelHoleCheck && (_dist < Min_spec || _dist > Max_spec))
			{
				 g_CommPLC.SetBit(PLC_WB_NOZZLE_CHECK_NG_ACK, TRUE);
				 m_pDlgViewerMain[viewer]->setViewerJudge(FALSE);
				 str.Format("[%s] Nozzle Distance %3.4f NG", m_pMain->vt_job_info[nJob].job_name.c_str(), _dist);
				 ShowErrorMessageBlink(str);
				 m_pMain->write_NGresult_SummaryFile(m_pMain->vt_job_info[nJob].main_object_id.c_str(), str, FALSE, nJob);
            }
    		else 
    		{
				 g_CommPLC.SetBit(PLC_WB_NOZZLE_CHECK_OK_ACK, TRUE);
				 m_pDlgViewerMain[viewer]->setViewerJudge(TRUE);
            }

			nSeq[nJob] = ALIGN_NOZZLE_CHECK_END;
		}
	}
	break;
	case ALIGN_WAIT_OFF_REQ:
	{
		//KJH 2021-07-23 Align Time Out 기능 삭제(별도 함수에서 진행하기 때문에(그 함수에도 타임아웃있음...) 무한 Grab위험성있음))
		//if (seq_ChkTimer(tm) || !g_CommPLC.GetBit(m_pMain->m_nAlignRequestRead[nJob]))
		//KJH 2022-06-06 GrabTime 삭제요청에 따른 삭제 작업
		if (!g_CommPLC.GetBit(PLC_RB_CENTER_ALIGN_REQ))	//Align의 경우 별도의 쓰레드에서 마크 찾기 진행 ProcessPrealignINFRA -> m_pMain->m_nSeqProcess[nJob] = 100;
		{
			nSeq[nJob] = ALIGN_RESET_FLAG;
			//seq_ResetChkTimer(tm);			
		}
	}
	break;
	case ALIGN_TRACE_GRAB_START:
	{
		if (g_CommPLC.GetBit(PLC_RB_CENTER_TRACE_REQ)) // Trace Req시 사전 준비작업 시작
		{
			long lData[8] = { 0, };
			//HTK 2022-04-04 Trace TactTime 추가
			m_clockProcStartTime[7] = clock();
			//Pre Data Read
			if (method == METHOD_LINE || method == METHOD_NOTCH)
			{
				double totalLength = 0;
				g_CommPLC.GetWord(m_pMain->vt_job_info[nJob].plc_addr_info.read_word_start, 8, lData);

				m_dLineNum					= MAKELONG(lData[0], lData[1]);
				m_dDopoType					= MAKELONG(lData[2], lData[3]);
				m_dAccLineLength			= MAKELONG(lData[4], lData[5]) / MOTOR_SCALE;
				m_dDecLineLength			= MAKELONG(lData[6], lData[7]) / MOTOR_SCALE;

				m_pMain->m_nCurrentNotchAcc = MAKELONG(lData[4], lData[5]) / MOTOR_SCALE;
				m_pMain->m_nCurrentNotchDec = MAKELONG(lData[6], lData[7]) / MOTOR_SCALE;

				totalLength = m_dAccLineLength + m_dDecLineLength;

				g_CommPLC.GetWord(PLC_RW_LEFT_DOPO_LENGTH, 2, lData);
				m_dLeftNotchLength = MAKELONG(lData[0], lData[1]) / MOTOR_SCALE;

				g_CommPLC.GetWord(PLC_RW_RIGHT_DOPO_LENGTH, 2, lData);
				m_dRightNotchLength = MAKELONG(lData[0], lData[1]) / MOTOR_SCALE;

				totalLength = m_dLeftNotchLength + m_dRightNotchLength + totalLength;
				m_pMain->vt_job_info[nJob].model_info.getAlignInfo().setInspRangeLength(totalLength);

				g_CommPLC.GetWord(PLC_RW_ROTATE_OFFSET_X, 2, lData);
				m_pMain->m_dCurrentRotateOffsetX = MAKELONG(lData[0], lData[1]) / MOTOR_SCALE;
				//Trace Rotate OffsetX값이 +-8보다 크면 오류로 판단한다..(이미지상 총량이 8보다 클수가 없다))
				if (fabs(m_pMain->m_dCurrentRotateOffsetX) > 8)
				{
					str.Format("[%s] Rotate Offset X Range NG : %d -> Change : 0 (Limit : 8)", m_pMain->m_dCurrentRotateOffsetX);
					::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

					m_pMain->m_dCurrentRotateOffsetX = 0.0;
				}

				// 20220407 Tkyuha PLC와 협의후 적용할것 //직선 도포에서 Y를 움직이기 위함
				//g_CommPLC.GetWord(m_pMain->vt_job_info[nJob].plc_addr_info.read_word_start+12, 4, lData);
				//m_posLineY_Move = MAKELONG(lData[0], lData[1]) / MOTOR_SCALE;
				//m_distLineY_Move = MAKELONG(lData[2], lData[3]) / MOTOR_SCALE;

				str.Format("NULL");

				CString sDopoType;
				if (m_dDopoType == 1)
				{
					sDopoType.Format("Line");
					str.Format("[%s] Dopo Type : %s, Pos : %d, Dopo Lenth : %.3f, Acc : %.3f, Dec : %.3f", get_job_name(nJob), sDopoType, m_dLineNum, totalLength, m_dAccLineLength, m_dDecLineLength);
				}
				else if (m_dDopoType == 3)
				{
					sDopoType.Format("Notch");
					str.Format("[%s] Dopo Type : %s, Dopo Lenth : %.3f, Acc : %.3f, Dec : %.3f", get_job_name(nJob), sDopoType, totalLength, m_dAccLineLength, m_dDecLineLength);
				}

				::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
			}
			else if (method == METHOD_CIRCLE)
			{
				//Trace X Offset Shift 기능 사용시 이용
				g_CommPLC.GetWord(PLC_RW_ROTATE_OFFSET_X, 2, lData);
				m_pMain->m_dCurrentRotateOffsetX = MAKELONG(lData[0], lData[1]) / MOTOR_SCALE;

				//Trace Rotate OffsetX값이 +-8보다 크면 오류로 판단한다..(이미지상 총량이 8보다 클수가 없다))
				if (fabs(m_pMain->m_dCurrentRotateOffsetX) > 8)
				{
					str.Format("[%s] Rotate Offset X Range NG : %d -> Change : 0 (Limit : 8)", m_pMain->m_dCurrentRotateOffsetX);
					::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

					m_pMain->m_dCurrentRotateOffsetX = 0.0;
				}

				//총 Trace 회전수
				g_CommPLC.GetWord(m_pMain->vt_job_info[nJob].plc_addr_info.read_word_start + 20, 2, lData);
				m_pMain->m_nCurrentCircleCount = MAKELONG(lData[0], lData[1]);

				//2차 회전시 Y축 변동량
				g_CommPLC.GetWord(m_pMain->vt_job_info[nJob].plc_addr_info.read_word_start + 40, 2, lData);
				m_pMain->m_nSecondCircleOffset_Y = MAKELONG(lData[0], lData[1]) / MOTOR_SCALE;

				//2차 회전시 T Position 값
				g_CommPLC.GetWord(m_pMain->vt_job_info[nJob].plc_addr_info.read_word_start + 42, 2, lData);
				m_pMain->m_dCircleSecondThetaPos = MAKELONG(lData[0], lData[1]) / MOTOR_SCALE;
				if (m_pMain->m_dCircleSecondThetaPos == 0)	m_pMain->m_dCircleSecondThetaPos = 411.0;

				//3차 회전시 Y축 변동량
				g_CommPLC.GetWord(m_pMain->vt_job_info[nJob].plc_addr_info.read_word_start + 44, 2, lData);
				m_pMain->m_nThirdCircleOffset_Y = MAKELONG(lData[0], lData[1]) / MOTOR_SCALE;

				//3차 회전시 T Position 값
				g_CommPLC.GetWord(m_pMain->vt_job_info[nJob].plc_addr_info.read_word_start + 46, 2, lData);
				m_pMain->m_dCircleThirdThetaPos = MAKELONG(lData[0], lData[1]) / MOTOR_SCALE;

				//4차 회전시 Y축 변동량
				g_CommPLC.GetWord(m_pMain->vt_job_info[nJob].plc_addr_info.read_word_start + 48, 2, lData);
				m_pMain->m_nFourCircleOffset_Y = MAKELONG(lData[0], lData[1]) / MOTOR_SCALE;

				//4차 회전시 T Position 값
				g_CommPLC.GetWord(m_pMain->vt_job_info[nJob].plc_addr_info.read_word_start + 50, 2, lData);
				m_pMain->m_dCircleFourThetaPos = MAKELONG(lData[0], lData[1]) / MOTOR_SCALE;

				//가감속 거리 계산
				m_pMain->m_nCurrentCircleAccDec = 0;
				g_CommPLC.GetWord(m_pMain->vt_job_info[nJob].plc_addr_info.read_word_start + 4, 4, lData);
				m_pMain->m_nCurrentCircleAccDec += MAKELONG(lData[0], lData[1]);
				m_pMain->m_nCurrentCircleAccDec += MAKELONG(lData[2], lData[3]);
				m_pMain->m_nCurrentCircleAccDec = m_pMain->m_nCurrentCircleAccDec / MOTOR_SCALE;

				str.Format("[%s] Trace X Offset : %.3f, Circle Count : %d, AccDec : %.3f", m_pMain->vt_job_info[nJob].job_name.c_str(), m_pMain->m_dCurrentRotateOffsetX,
					m_pMain->m_nCurrentCircleCount, m_pMain->m_nCurrentCircleAccDec);
				::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

				str.Format("[%s] Second Rot Y Offset : %.3f , Second Rot Start T Pos : %.3f", m_pMain->vt_job_info[nJob].job_name.c_str(),
					m_pMain->m_nSecondCircleOffset_Y, m_pMain->m_dCircleSecondThetaPos);
				::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
			}

			//KJH 2021-05-28 이미지 갱신관련 인자 추가
			m_pMain->m_ELB_TraceResult.bType = TRUE;
#ifdef _DAHUHA
			AcquireSRWLockExclusive(&m_pMain->m_pDahuhaCam[real_cam].g_bufsrwlock); // 동기화 시킴 Tkyuha 20211027
#else
			AcquireSRWLockExclusive(&m_pMain->m_pBaslerCam[real_cam]->g_bufsrwlock); // 동기화 시킴 Tkyuha 20211027
#endif
			m_pMain->m_bGrabEnd[real_cam] = FALSE;
#ifdef _DAHUHA
			ReleaseSRWLockExclusive(&m_pMain->m_pDahuhaCam[real_cam].g_bufsrwlock);
#else
			ReleaseSRWLockExclusive(&m_pMain->m_pBaslerCam[real_cam]->g_bufsrwlock);
#endif

			//KJH 2021-07-22 Trace Grab전 조명 켜기
			if (method == METHOD_LINE || method == METHOD_NOTCH || method == METHOD_CIRCLE)
			{
				int index = 0;

				if (m_pMain->m_bELB_LightControlMethod == 1)
				{
					index = 0; // index 확인 필요 [Center Grab 동일한 조명사용시]
				}
				else
				{
					index = 1; // index 확인 필요 [Trace Grab 다른 조명사용시]
				}
				std::vector<int> camBuf = theApp.m_pFrame->vt_job_info[nJob].camera_index;
				int nCam = camBuf.at(0);	//Center Cam Index 0

				str.Format("[%s] Trace 1st Light index - %d ON", m_pMain->vt_job_info[nJob].job_name.c_str(), index);
				::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
				if(index == 1)m_pMain->LightControllerTurnOnOff(nJob, nCam, index);
			}

			//KJH 2021-07-22 Sapera가 왜 있어???
#ifdef _DAHUHA
			if (m_pMain->m_pDahuhaCam[real_cam].isconnected())
#else
			if (m_pMain->m_pBaslerCam[real_cam]->IsOpen())
#endif
			{
				str.Format("[%s] Trace Grab Start", m_pMain->vt_job_info[nJob].job_name.c_str());
				::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

				//Tkyuha 2021-12-24 Delay 설정
				Delay(m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getDustInspGrabDelay());

				//KJH 2021-05-28 카메라 밀리는 현상 수정
				::SendMessage(m_pMain->m_hWnd, WM_VIEW_CONTROL, MSG_MD_CAMERA_SINGLE_FRAME, real_cam);
				//seq_SetTimer(tm, 2000);
			}

			bHandshake[nJob] = true;
			m_bHandShake[nJob] = true;

			if (method == METHOD_LINE || m_pMain->m_bELB_LightControlMethod < 2) nSeq[nJob] = ALIGN_TRACE_GRAB_END;
			else  nSeq[nJob] = ALIGN_TRACE_2ND_GRAB; // Metal Search 안할 경우는 nSeq[nJob] = ALIGN_TRACE_GRAB_END;
			}
	}
	break;
	case ALIGN_TRACE_2ND_GRAB: // Trace 2nd Image Grab
	{
		BOOL bGrabComplete = TRUE;
		if (m_pMain->m_bGrabEnd[real_cam] != TRUE)			bGrabComplete = FALSE;
		//KJH 2022-06-06 GrabTime 삭제요청에 따른 삭제 작업
		//if (bGrabComplete || seq_ChkTimer(tm) || !g_CommPLC.GetBit(PLC_RB_CENTER_TRACE_REQ))
		if (bGrabComplete || !g_CommPLC.GetBit(PLC_RB_CENTER_TRACE_REQ))
		{
			//seq_ResetChkTimer(tm);
			int W = m_pMain->m_stCamInfo[real_cam].w;
			int H = m_pMain->m_stCamInfo[real_cam].h;

			//SrcBuffer = 방금 찍은거
			//ProcBuffer = 저장된거

			m_pMain->copyMemory(m_pMain->getProcBuffer(real_cam, 1), m_pMain->getSrcBuffer(real_cam), W * H);

			//2차 조명값으로 조명을 키자
			if (method == METHOD_NOTCH || method == METHOD_CIRCLE)
			{
				int index = 2; // index 확인 필요
				std::vector<int> camBuf = theApp.m_pFrame->vt_job_info[nJob].camera_index;
				int nCam = camBuf.at(0);	//Center Cam Index 0

				str.Format("[%s] Trace 2nd Light index - %d ON", m_pMain->vt_job_info[nJob].job_name.c_str(), index);
				::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

				m_pMain->LightControllerTurnOnOff(nJob, nCam, index);
			}

#ifdef _DAHUHA
			AcquireSRWLockExclusive(&m_pMain->m_pDahuhaCam[real_cam].g_bufsrwlock); // 동기화 시킴 Tkyuha 20211027
#else
			AcquireSRWLockExclusive(&m_pMain->m_pBaslerCam[real_cam]->g_bufsrwlock); // 동기화 시킴 Tkyuha 20211027
#endif
			m_pMain->m_bGrabEnd[real_cam] = FALSE;
#ifdef _DAHUHA
			ReleaseSRWLockExclusive(&m_pMain->m_pDahuhaCam[real_cam].g_bufsrwlock);
#else
			ReleaseSRWLockExclusive(&m_pMain->m_pBaslerCam[real_cam]->g_bufsrwlock);
#endif
#ifdef _DAHUHA
			if (m_pMain->m_pDahuhaCam[real_cam].isconnected())
#else
			if (m_pMain->m_pBaslerCam[real_cam]->IsOpen())
#endif
			{
				str.Format("[%s] Trace 2nd Grab Start", m_pMain->vt_job_info[nJob].job_name.c_str());
				::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

				//KJH 2021-05-28 카메라 밀리는 현상 수정
				::SendMessage(m_pMain->m_hWnd, WM_VIEW_CONTROL, MSG_MD_CAMERA_SINGLE_FRAME, real_cam);
			}

			nSeq[nJob] = ALIGN_TRACE_GRAB_END;
		}
	}
	break;
	case ALIGN_TRACE_GRAB_END:
	{
		BOOL bGrabComplete = TRUE;
		
		if (m_pMain->m_bGrabEnd[real_cam] != TRUE)			bGrabComplete = FALSE;
		//KJH 2022-06-06 GrabTime 삭제요청에 따른 삭제 작업
		//if (bGrabComplete || seq_ChkTimer(tm) || !g_CommPLC.GetBit(PLC_RB_CENTER_TRACE_REQ))
		if (bGrabComplete || !g_CommPLC.GetBit(PLC_RB_CENTER_TRACE_REQ))
		{
			//seq_ResetChkTimer(tm);
			m_bCheckState = TRUE;
			BOOL bReturn = TRUE;

			int viewer = 0;
			//KJH 2022-05-26 6View -> 9 View 분기작업
			//viewer_index[0] = Align
			//viewer_index[1] = Live
			//viewer_index[2] = Trace
			//viewer_index[3] = Insp
			if (m_pMain->vt_job_info[nJob].num_of_viewer == 2)
			{
				viewer = m_pMain->vt_job_info[nJob].viewer_index[0];
			}
			else
			{
				viewer = m_pMain->vt_job_info[nJob].viewer_index[2];
			}
			int method = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod();
			int W = m_pMain->m_stCamInfo[real_cam].w;
			int H = m_pMain->m_stCamInfo[real_cam].h;

			if (m_pMain->m_bELB_LightControlMethod < 2)
			{
				//SrcBuffer = 방금 찍은거
				//ProcBuffer = 저장된거
				m_pMain->copyMemory(m_pMain->getProcBuffer(real_cam, 2), m_pMain->getSrcBuffer(real_cam), W * H);
			}

			m_pDlgViewerMain[viewer]->resetViewerJudge();

			if (bGrabComplete)
			{
				str.Format("[%s] Trace Grab Complete - Success", get_job_name(nJob));
			}
			else
			{
				str.Format("[%s] Trace Grab Complete - Fail", get_job_name(nJob));
			}
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

			//KJH 2022-07-14 Trace Grab bit & Reset추가
			g_CommPLC.SetBit(PLC_WB_TRACE_GRAB_COMPLETE_ACK, TRUE);

			//KJH 2022-04-29 Dopo Start시 동영상 녹화 기능
			//BOOL bLive1 = FALSE;
			//BOOL bLive2 = FALSE;
			//if (m_pDlgViewerMain[viewer + 1]->m_bEnableGrab != TRUE) bLive1 = true;
			//if (m_pDlgViewerMain[viewer + 2]->m_bEnableGrab != TRUE) bLive2 = true;

			//if (!m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getAVI_Save_ImageTypeEnable() &&
			//	m_pMain->vt_job_info[0].model_info.getAlignInfo().getUseAutoSaveAVI())
			//{ // 동영상 저장시 죽는 문제 있다고 해서 강제로 종료 시킴
			//	::SendMessageA(m_pDlgViewerMain[viewer + 1]->GetSafeHwnd(), WM_VIEW_CONTROL, 300, 0);
			//	int _nSView = m_pMain->vt_job_info[nJob + 1].viewer_index[0];
			//	::SendMessageA(m_pDlgViewerMain[_nSView]->GetSafeHwnd(), WM_VIEW_CONTROL, 300, 0);
			//}

			//KJH Trace Data 전송 위치
			if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseRealTimeTrace())
			{
				bReturn = fnExtractProfileData(nJob, &m_pDlgViewerMain[viewer]->GetViewer());

				str.Format("[%s] Real Time Trace Mode", get_job_name(nJob));
				::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
			}
			else
			{
				sendGoldData(nJob);
				str.Format("[%s] Send Gold Data Mode", m_pMain->vt_job_info[nJob].job_name.c_str());
				::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
			}

			if (bReturn != TRUE)
			{
				//Trace 실패시
				m_pDlgViewerMain[viewer]->setViewerJudge(FALSE);				
				// Manual Mark Input 진행 위치 20220307
				///// 
				//KJH 2022-03-10 L-Check Error일 경우에도 Manual Mark 가능하게 변경
				//JSH 2022-03-16 진입 조건에서 L-Check 삭제
				//KJH 2022-07-01 Trace NG PopUp 분기 추가
				if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getEnableAlignMeasureSpecOutJudge() && m_bCheckState)
				{
					m_pMain->GetMatching(nJob).getFindInfo(0, 0).SetFound(TRUE);
					m_pMain->GetMatching(nJob).getFindInfo(0, 1).SetFound(FALSE);

					str.Format("Manual Mark Input");
					m_pMain->add_process_history(str);
					// KBJ 2022-07-05 Manual Mark 1Cam 기능 추가
					//SendMessageA(WM_VIEW_CONTROL, MSG_FMV_SHOW_MANUAL_INPUT_MARK, MAKELPARAM(nJob, TRUE));
					SendMessageA(WM_VIEW_CONTROL, MSG_FMV_SHOW_MANUAL_INPUT_MARK_1CAM, MAKELPARAM(nJob, TRUE));
					if (method == METHOD_LINE)			m_pMain->m_pManualInputMark1Cam[nJob]->initManualInputTracker(1); // 직선 선택
					else if (method == METHOD_CIRCLE)	m_pMain->m_pManualInputMark1Cam[nJob]->initManualInputTracker(2); // 원 선택
					g_CommPLC.SetBit(PLC_WB_CENTER_MANUAL_ALIGN_REQ, TRUE);
					bManualJudge = TRUE;
					nSeq[nJob] = ALIGN_TRACE_MANUAL_MARK;
				}
				else
				{
					str.Format("[%s] Trace Measure NG", get_job_name(nJob));
					::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
					g_CommPLC.SetBit(PLC_WB_TRACE_JUDGE_NG, TRUE);
					nSeq[nJob] = ALIGN_TRACE_MEASURE_END;
				}
				
				break;
			}
			else
			{
				//KJH 2022-04-29 Dopo Start시 동영상 녹화 기능
				//if (m_pMain->vt_job_info[0].model_info.getAlignInfo().getUseAutoSaveAVI())
				//{
				//	if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getAVI_Save_ImageTypeEnable())
				//	{
				//		m_iSaveImageTypeNum = 0;
				//		SetTimer(10000, 500, NULL);
				//	}
				//	else
				//	{
				//		::SendMessageA(m_pDlgViewerMain[viewer + 1]->GetSafeHwnd(), WM_VIEW_CONTROL, 200, 0);
				//		//  사이드뷰 자동 저장 기능 활성화 20220110 Tkyuha
				//		int _nSView = m_pMain->vt_job_info[nJob + 1].viewer_index[0];
				//		::SendMessageA(m_pDlgViewerMain[_nSView]->GetSafeHwnd(), WM_VIEW_CONTROL, 200, 0);
				//		SetTimer(_nSView, m_pMain->vt_system_option[nJob].stELB.Auto_Save_Video_Time * 1000, NULL);

				//		if (method == METHOD_LINE)
				//		{
				//			if (m_dLineNum == 1)		SetTimer(viewer + 1, 12000, NULL); // 자동으로 녹화 중지
				//			else SetTimer(viewer + 1, 2500, NULL);
				//		}
				//		else SetTimer(viewer + 1, m_pMain->vt_system_option[nJob].stELB.Auto_Save_Video_Time * 1000, NULL);  // 홀 ,노치 인경우 4초 대기시간
				//		// kbj 2022-02-10 auto video 이후 라이브 중지되어서 Live 되도록 추가
				//		if (bLive1 == TRUE) SetTimer(9501, m_pMain->vt_system_option[nJob].stELB.Auto_Save_Video_Time * 1000 + 50, NULL); //s -> ms
				//		if (bLive2 == TRUE) SetTimer(9502, m_pMain->vt_system_option[nJob].stELB.Auto_Save_Video_Time * 1000 + 50, NULL); //s -> ms
				//	}
				//}
				
				// KBJ 2022-08-25 프로그램 재시작 후 트레이스 치면 Lack of ink 오검출 하는 현상 있어서 한번더 하도록 추가.
				if (m_bFirstTrace == TRUE && m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseMetalTraceFirst())
				{
					m_bFirstTrace = FALSE;
					bReturn = fnExtractProfileData(nJob, &m_pDlgViewerMain[viewer]->GetViewer());
					break;
				}
jumpManualMark:	
				g_CommPLC.SetBit(PLC_WB_TRACE_JUDGE_OK, TRUE);
				if(bManualJudge) m_pMain->copyMemory(m_pMain->getProcBuffer(real_cam, 2), m_pMain->getSrcBuffer(real_cam), m_pMain->m_stCamInfo[real_cam].w * m_pMain->m_stCamInfo[real_cam].h);
				nSeq[nJob] = ALIGN_TRACE_MEASURE_END;
			}
			//HTK 2022-04-04 Trace TactTime 추가
			m_clockProc_elapsed_time[7] = (clock() - m_clockProcStartTime[7]) / (float)CLOCKS_PER_SEC;
			((CPaneAuto*)m_pMain->m_pPane[PANE_AUTO])->update_grid_processing_time(1, m_clockProc_elapsed_time[7], 8, bReturn);
			
			//KJH 2022-07-27 Tact Time Log 추가
			theLog.logmsg(LOG_PLC_SEQ, "Trace Seq Tact Time = %.4f", m_clockProc_elapsed_time[7]);

			//KJH 2022-07-14 Trace Grab bit & Reset추가
			g_CommPLC.SetBit(PLC_WB_TRACE_GRAB_COMPLETE_ACK, FALSE);
		}
	}
	break;
	case ALIGN_INSP_PRE_GRAB:
	{
		BOOL bGrabComplete = TRUE;
		if (m_pMain->m_bGrabEnd[real_cam] != TRUE)			bGrabComplete = FALSE;
		//KJH 2022-06-06 GrabTime 삭제요청에 따른 삭제 작업
		//if (bGrabComplete || seq_ChkTimer(tm) || !g_CommPLC.GetBit(PLC_RB_CENTER_TRACE_REQ))
		if (bGrabComplete || !g_CommPLC.GetBit(PLC_RB_CENTER_TRACE_REQ))
		{
			//seq_ResetChkTimer(tm);
			g_CommPLC.SetBit(PLC_WB_TRACE_JUDGE_OK, TRUE);

			int viewer = m_pMain->vt_job_info[nJob].viewer_index[0];
			m_pDlgViewerMain[viewer]->setViewerJudge(TRUE);

			//str.Format("[%s] Cam %d - 3rd Grab Complete", m_pMain->vt_job_info[nJob].job_name.c_str(), 1);
			str.Format("[%s] Diff Insp Pre Grab Complete", m_pMain->vt_job_info[nJob].job_name.c_str());
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

			if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getUseDiffInsp())
			{
				int W = m_pMain->m_stCamInfo[real_cam].w;
				int H = m_pMain->m_stCamInfo[real_cam].h;
				m_pMain->copyMemory(m_pMain->getProcBuffer(real_cam, 2), m_pMain->getSrcBuffer(real_cam), W * H);
			}

			nSeq[nJob] = ALIGN_TRACE_MEASURE_END;
		}
	}
	break;
	case ALIGN_INSP_GRAB_START:
		if (g_CommPLC.GetBit(PLC_RB_CENTER_INSP_REQ)) // 세번째 검사 이미지 확보 
		{
			//HTK 2022-07-27 Total Insp Seq Time 추가
			m_clockProcStartTime[8] = clock();

			bool bLine_InspWithTraceResultMode = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLine_InspWithTraceResultMode();
			//if (method == METHOD_LINE && bLine_InspWithTraceResultMode)  //Tkyuha 20221123 라인 검사시 조명 변경해서 촬상 하기 위함
			if(0)
			{
				int nCam = theApp.m_pFrame->vt_job_info[nJob].camera_index.at(0);	//Center Cam Index 0

				str.Format("[%s] Insp 1st Light index - %d ON", m_pMain->vt_job_info[nJob].job_name.c_str(), 1);				
				m_pMain->LightControllerTurnOnOff(nJob, nCam, 1);
				::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)& str);
			}

#ifdef _DAHUHA
			AcquireSRWLockExclusive(&m_pMain->m_pDahuhaCam[real_cam].g_bufsrwlock); // 동기화 시킴 Tkyuha 20211027
#else
			AcquireSRWLockExclusive(&m_pMain->m_pBaslerCam[real_cam]->g_bufsrwlock); // 동기화 시킴 Tkyuha 20211027
#endif
			m_pMain->m_bGrabEnd[real_cam] = FALSE;
#ifdef _DAHUHA
			ReleaseSRWLockExclusive(&m_pMain->m_pDahuhaCam[real_cam].g_bufsrwlock);
#else
			ReleaseSRWLockExclusive(&m_pMain->m_pBaslerCam[real_cam]->g_bufsrwlock);
#endif

			//Tkyuha 2021-12-24 Delay 설정
			Delay(m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getDropInspGrabDelay());

#ifdef _DAHUHA
			if (m_pMain->m_pDahuhaCam[real_cam].isconnected())
#else
			if (m_pMain->m_pBaslerCam[real_cam]->IsOpen())
#endif
			{
				//str.Format("[%s] Cam %d - 4th Grab Start", m_pMain->vt_job_info[nJob].job_name.c_str(), 1);
				str.Format("[%s] Diff Insp Grab Start", get_job_name(nJob));
				::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

				::SendMessage(m_pMain->m_hWnd, WM_VIEW_CONTROL, MSG_MD_CAMERA_SINGLE_FRAME, real_cam);
			}
			seq_SetTimer(tm, 2000);
			bHandshake[nJob] = true;
			m_bHandShake[nJob] = true;

			nSeq[nJob] = ALIGN_INSP_GRAB_END;
		}
		break;
	case ALIGN_INSP_GRAB_END:
	{
		BOOL bGrabComplete = TRUE;
		if (m_pMain->m_bGrabEnd[real_cam] != TRUE)			bGrabComplete = FALSE;

		//KJH 2022-07-30 Wetout Grab Time Out 추가
		if (!bGrabComplete && seq_ChkTimer(tm))
		{
			seq_ResetChkTimer(tm);

			str.Format("[%s] Diff Insp Grab - Time Out Fail", m_pMain->vt_job_info[nJob].job_name.c_str());
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

			theLog.logmsg(LOG_ERROR, "%s", str);

			nSeq[nJob] = ALIGN_INSP_GRAB_START;

			break;
		}
		
		//KJH 2022-06-06 GrabTime 삭제요청에 따른 삭제 작업
		//if (bGrabComplete || seq_ChkTimer(tm) || !g_CommPLC.GetBit(PLC_RB_CENTER_INSP_REQ))
		if (bGrabComplete || !g_CommPLC.GetBit(PLC_RB_CENTER_INSP_REQ))
		{
			//seq_ResetChkTimer(tm);

			int _nLiveviewer = m_pMain->vt_job_info[nJob].viewer_index[1];
			int _nSideView	 = m_pMain->vt_job_info[nJob + 1].viewer_index[0];

			if (!m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getAVI_Save_ImageTypeEnable() &&
				m_pMain->vt_job_info[0].model_info.getAlignInfo().getUseAutoSaveAVI())
			{
				KillTimer(_nLiveviewer); // 미리 타이머를 종료 시킨다.
				::SendMessageA(m_pDlgViewerMain[_nLiveviewer]->GetSafeHwnd(), WM_VIEW_CONTROL, 300, 0);
				//  사이드뷰 자동 저장 기능 활성화 20220110 Tkyuha
				KillTimer(_nSideView);
				::SendMessageA(m_pDlgViewerMain[_nSideView]->GetSafeHwnd(), WM_VIEW_CONTROL, 300, 0);
			}

			int W = m_pMain->m_stCamInfo[real_cam].w;
			int H = m_pMain->m_stCamInfo[real_cam].h;

			//m_pMain->copyMemory(m_pMain->getProcBuffer(real_cam, 0), m_pMain->getSrcBuffer(real_cam), W* H);
			m_pMain->copyMemory(m_pMain->getProcBuffer(real_cam, 3), m_pMain->getSrcBuffer(real_cam), W * H);

			if (bGrabComplete)
			{
				str.Format("[%s] Diff Insp Grab Complete - Success", m_pMain->vt_job_info[nJob].job_name.c_str());
			}
			else
			{
				str.Format("[%s] Diff Insp Grab Complete - Fail", m_pMain->vt_job_info[nJob].job_name.c_str());
			}
			//str.Format("[%s] 4th Grab Complete - %d", m_pMain->vt_job_info[nJob].job_name.c_str(), bGrabComplete);
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

			//HTK 2022-07-11 Diff Insp Grab bit & Reset추가
			g_CommPLC.SetBit(PLC_WB_DIFF_GRAB_COMPLETE_ACK, TRUE);

			bool bReturn = false;

			int method = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod();
			int datasheet = 0;
			
			int viewer = 0;
			//KJH 2022-05-26 6View -> 9 View 분기작업
			//viewer_index[0] = Align
			//viewer_index[1] = Live
			//viewer_index[2] = Trace
			//viewer_index[3] = Insp
			if (m_pMain->vt_job_info[nJob].num_of_viewer == 2)
			{
				viewer = m_pMain->vt_job_info[nJob].viewer_index[0];
			}
			else
			{
				viewer = m_pMain->vt_job_info[nJob].viewer_index[3];
			}

			m_pDlgViewerMain[viewer]->GetViewer().OnLoadImageFromPtr(m_pMain->getProcBuffer(real_cam, 3));
			m_pDlgViewerMain[viewer]->resetViewerJudge();

			// Circle Diff Insp
			bReturn = fnDiffWetOutInspection(nJob);

			if (m_pMain->m_ELB_DiffInspResult[datasheet].m_bResultSummary == FALSE)
			{
				// KBJ 2022-10-10 검사 에러타입추가.	======================================
				m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType += ELB_Error_Type(m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType, ELB_INSP_ERROR_OVER_FLOW);
				//	==========================================================================
				str.Format("[%s] Overflow NG", m_pMain->vt_job_info[nJob].job_name.c_str());
				ShowErrorMessageBlink(str);
				bReturn = FALSE;
			}
			if (m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_bResultSummary == FALSE)
			{
				// KBJ 2022-10-10 검사 에러타입추가.	======================================
				m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType += ELB_Error_Type(m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType, ELB_INSP_ERROR_UNDER_FLOW);
				//	==========================================================================
				str.Format("[%s] Underflow NG", m_pMain->vt_job_info[nJob].job_name.c_str());
				ShowErrorMessageBlink(str);
				bReturn = FALSE;
			}
			if (m_pMain->m_ELB_DiffInspResult[datasheet].bType == FALSE)
			{
				str.Format("[%s] Not Dispensed NG", m_pMain->vt_job_info[nJob].job_name.c_str());
				ShowErrorMessageBlink(str);
				bReturn = FALSE;
			}
			if (m_pMain->m_ELB_DiffInspResult[datasheet].bRJudge == FALSE)
			{
				str.Format("[%s] R Spec NG", m_pMain->vt_job_info[nJob].job_name.c_str());
				ShowErrorMessageBlink(str);
				bReturn = FALSE;
			}
			if (m_pMain->m_ELB_DiffInspResult[datasheet].bBJudge == FALSE)
			{
				str.Format("[%s] B Spec NG", m_pMain->vt_job_info[nJob].job_name.c_str());
				ShowErrorMessageBlink(str);
				bReturn = FALSE;
			}

			if (bReturn)
			{
				str.Format("[%s] fnDiffWetOutInspection - Success", m_pMain->vt_job_info[nJob].job_name.c_str());
				::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
			}
			else
			{
				str.Format("[%s] fnDiffWetOutInspection - Fail", m_pMain->vt_job_info[nJob].job_name.c_str());
				::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
			}

			//KJH 2021-11-30 DustInsp 판정이 최상위 판정이던거 수정함
			BOOL bLiquidReturn = TRUE;

			//?????? 검사가 NG여도 Dust가 OK면 OK로 나가겠다??????
			// 2022-01-12 DUST가 아니라 LIQUID 검사임. 변수명 변경
			if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getUseDustInsp())
			{
				bLiquidReturn = fnDiffDustInspection(nJob);

				if (bLiquidReturn == FALSE)
				{
					bReturn = FALSE;
				}

				if (bLiquidReturn)
				{
					m_bLiquidInspJudge = TRUE;

					str.Format("[%s] LiquidInspection - Success", m_pMain->vt_job_info[nJob].job_name.c_str());
					::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
				}
				else
				{
					m_bLiquidInspJudge = FALSE;

					str.Format("[%s] LiquidInspection - Fail", m_pMain->vt_job_info[nJob].job_name.c_str());
					::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
					::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->GetSafeHwnd(), WM_VIEW_CONTROL, MSG_PA_SPEC_VIEW, 2);
				}

			}
			if (theApp.m_bEnforceOkFlag) bReturn = true;

			//KJH 2021-10-21 FDC 보고함수 추가_Circle
			SendFDCValue_Circle(nJob, real_cam);

			if (bReturn)
			{
				g_CommPLC.SetBit(PLC_WB_INSP_JUDGE_OK, TRUE);
				m_pDlgViewerMain[viewer]->setViewerJudge(TRUE);
			}
			else
			{
				g_CommPLC.SetBit(PLC_WB_INSP_JUDGE_NG, TRUE);
				m_pDlgViewerMain[viewer]->setViewerJudge(FALSE);
			}

			// kbj 2022-02-10 auto video 이후 라이브 중지되어서 Live 되도록 추가
			if (!m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getAVI_Save_ImageTypeEnable() &&
				m_pMain->vt_job_info[0].model_info.getAlignInfo().getUseAutoSaveAVI())
			{
				if (m_pDlgViewerMain[_nLiveviewer]->m_bLiveCheck == TRUE)
				{
					m_pDlgViewerMain[_nLiveviewer]->m_bLiveCheck = FALSE;
					m_pDlgViewerMain[_nLiveviewer]->OnCommandLive();
				}
				if (m_pDlgViewerMain[_nSideView]->m_bLiveCheck == TRUE)
				{
					m_pDlgViewerMain[_nSideView]->m_bLiveCheck = FALSE;
					m_pDlgViewerMain[_nSideView]->OnCommandLive();
				}
			}
			//HTK 2022-07-11 Diff Insp Grab bit & Reset추가
			g_CommPLC.SetBit(PLC_WB_DIFF_GRAB_COMPLETE_ACK, FALSE);

			m_clockProc_elapsed_time[8] = (clock() - m_clockProcStartTime[8]) / (float)CLOCKS_PER_SEC;
			((CPaneAuto*)m_pMain->m_pPane[PANE_AUTO])->update_grid_processing_time(1, m_clockProc_elapsed_time[8], 9, true);

			//KJH 2022-07-27 Tact Time Log 추가
			theLog.logmsg(LOG_PLC_SEQ, "Diff Wetout Insp Seq Tact Time = %.4f", m_clockProc_elapsed_time[8]);

			nSeq[nJob] = ALIGN_DIFF_INSP_END;
		}
	}
	break;
	case ALIGN_TRACE_MEASURE_END:
		if (!g_CommPLC.GetBit(PLC_RB_CENTER_TRACE_REQ))
			nSeq[nJob] = ALIGN_RESET_FLAG;
		break;
	case ALIGN_TRACE_MANUAL_MARK:
	{
		if (m_pMain->m_nManualInputMarkResult[nJob] == 1)
		{
			// KBJ 2022-07-05 Manual Mark 1Cam 기능 추가
			//m_pMain->m_pManualInputMarkCam[nJob]->KillTimer(TIMER_MANUAL_MARK);
			m_pMain->m_pManualInputMark1Cam[nJob]->KillTimer(TIMER_MANUAL_MARK);
			// 얼라인 및 Trace 동작 시킬것
			fnCalcManualCenterAlign(nJob);
			g_CommPLC.SetBit(PLC_WB_CENTER_MANUAL_ALIGN_ACK, TRUE);
			goto jumpManualMark;
		}
		else if (m_pMain->m_nManualInputMarkResult[nJob] == 2)
		{
			g_CommPLC.SetBit(PLC_WB_TRACE_JUDGE_NG, TRUE);
			nSeq[nJob] = ALIGN_TRACE_MEASURE_END;
		}
	}
	break;
	case ALIGN_DIFF_INSP_END:
		if (!g_CommPLC.GetBit(PLC_RB_CENTER_INSP_REQ))
			nSeq[nJob] = ALIGN_RESET_FLAG;
		break;
	case ALIGN_DUMMY_INSP_END:
		if (!g_CommPLC.GetBit(PLC_RB_CENTER_DUMMMY_INSP_REQ))
		{
			g_CommPLC.SetBit(PLC_WB_ALIGN_JUDGE_NG + 50, FALSE);
			g_CommPLC.SetBit(PLC_WB_ALIGN_JUDGE_OK + 50, FALSE);
			nSeq[nJob] = ALIGN_RESET_FLAG;
		}
		break;
	case ALIGN_CG_GRAB_RESET:
	{
		g_CommPLC.SetBit(PLC_WB_GRAB_CG_OK, FALSE);
		g_CommPLC.SetBit(PLC_WB_GRAB_CG_NG, FALSE);
		nSeq[nJob] = ALIGN_RESET_FLAG;
	}
	break;
	case ALIGN_NOZZLE_CHECK_END:
		if (!g_CommPLC.GetBit(PLC_RB_NOZZLE_CHECK_GRAB_REQ))
			nSeq[nJob] = ALIGN_RESET_FLAG;
		break;
	case ALIGN_RESET_FLAG:
		bHandshake[nJob] = false;
		m_bHandShake[nJob] = false;

		nSeq[nJob] = ALIGN_CENTER_RESET;
		break;
	}
}
void CFormMainView::fnProcess_ProcessScanInspAlgorithm(int nJob)
{
	CString str;

	static int nSeq[MAX_JOB] = { 0, };
	static bool bHandshake[MAX_JOB] = { false, };
	int tm = STM_INSP + nJob;

	if (bHandshake[nJob] && !g_CommPLC.GetBit(m_pMain->m_nInspRequestRead[nJob]))
	{
		nSeq[nJob] = 0;
	}

#ifndef JOB_INFO
	int camCount = m_pAlgorithmInfo.getAlgorithmCameraNum(nJob);
	std::vector<int> camBuf = m_pAlgorithmInfo.getAlgorithmCameraIndex(nJob);
#else
	int camCount = m_pMain->vt_job_info[nJob].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[nJob].camera_index;
	int nStartAddr = m_pMain->vt_job_info[nJob].plc_addr_info.read_word_start;
#endif

	int real_cam = camBuf.at(0);
	switch (nSeq[nJob])
	{
	case 0:
		str.Format("[%s] Scan Process End", get_job_name(nJob));
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		resetProcessStart(nJob);

		g_CommPLC.SetBit(m_pMain->m_nInspOkWrite[nJob], FALSE);
		g_CommPLC.SetBit(m_pMain->m_nInspNgWrite[nJob], FALSE);
		g_CommPLC.SetBit(m_pMain->m_nInspAckWrite[nJob], FALSE);

		bHandshake[nJob] = false;
		m_bHandShake[nJob] = false;

		nSeq[nJob] = 10;
		break;
	case 10:
		if (!getProcessStart(nJob) && g_CommPLC.GetBit(m_pMain->m_nInspRequestRead[nJob])) // 첫번째 검사 수행  
		{
			long lData[10] = { 0, };
			g_CommPLC.GetWord(nStartAddr, 1, lData); // Grab count
			m_pMain->m_nSeqScanInspGrabCount[nJob] = MIN(3, MAX(1, lData[0])); //최소 Grab은 1이고 최대 4번 Grab
			g_CommPLC.GetWord(nStartAddr + 2, 8, lData); // Grab 길이결정

			m_pMain->m_dScanImageLength[0] = MAKELONG(lData[0], lData[1]) / MOTOR_SCALE;
			m_pMain->m_dScanImageLength[1] = MAKELONG(lData[2], lData[3]) / MOTOR_SCALE;
			m_pMain->m_dScanImageLength[2] = MAKELONG(lData[4], lData[5]) / MOTOR_SCALE;
			m_pMain->m_dScanImageLength[3] = MAKELONG(lData[6], lData[7]) / MOTOR_SCALE;

			m_pMain->vt_job_info[nJob].model_info.getAlignInfo().setInspRangeLength(fabs(m_pMain->m_dScanImageLength[1] - m_pMain->m_dScanImageLength[0]));

			str.Format("[%s] S : %4.2f, E : %4.2f, C : %4.2f , ID : %s", get_job_name(nJob), m_pMain->m_dScanImageLength[0], m_pMain->m_dScanImageLength[1], m_pMain->m_dScanImageLength[2], m_pMain->vt_job_info[nJob].main_object_id.c_str());
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
			str.Format("[%s] Insp Num : %d, Total Length : %4.2f", get_job_name(nJob), m_pMain->m_nSeqScanInspGrabCount[nJob], fabs(m_pMain->m_dScanImageLength[1] - m_pMain->m_dScanImageLength[0]));

			/*str.Format("[%s][GC: %d][S(%4.2f,%4.2f)][E(%4.2f,%4.2f)] -L: %4.2fmm Process Start, ID : %s", get_job_name(nJob), m_pMain->m_nSeqScanInspGrabCount[nJob],
				m_pMain->m_dScanImageLength[0], m_pMain->m_dScanImageLength[1], m_pMain->m_dScanImageLength[2], m_pMain->m_dScanImageLength[3],
				fabs(m_pMain->m_dScanImageLength[1] - m_pMain->m_dScanImageLength[0]), m_pMain->vt_job_info[nJob].main_object_id.c_str());*/

			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

			memset(m_pMain->vt_result_info[nJob].each_Judge, 0, sizeof(m_pMain->vt_result_info[nJob].each_Judge));

			setProcessStart(nJob);	// 여기 함수 진입 flag

			read_glass_id(nJob);
			reset_viewerID(nJob, m_pMain->m_nSeqScanInspGrabCount[nJob] - 1);
			SendMessageA(WM_VIEW_CONTROL, MSG_FMV_GLASSID_UPDATE, nJob);

			// 실제 검사 Sequence 동작하는 시점.
			AcquireSRWLockExclusive(&m_pMain->g_srwlock); // 동기화 시킴 Tkyuha 20211027
			m_pMain->m_nSeqProcess[nJob] = 100;
			ReleaseSRWLockExclusive(&m_pMain->g_srwlock);
			m_pMain->setProcessStartFlag(nJob);

			//seq_SetTimer(tm, 10000);
			bHandshake[nJob] = true;
			m_bHandShake[nJob] = true;

			nSeq[nJob] = 19;
		}
		break;
	case 19:
		//if (seq_ChkTimer(tm) || !g_CommPLC.GetBit(m_pMain->m_nInspRequestRead[nJob]))
		//if (g_CommPLC.GetBit(m_pMain->m_nInspRequestRead[nJob]) != TRUE && getProcessStart(nJob))
		if (!g_CommPLC.GetBit(m_pMain->m_nInspRequestRead[nJob]))
		{
			seq_ResetChkTimer(tm);
			nSeq[nJob] = 120;
		}

		break;
	case 120:
		bHandshake[nJob] = false;
		m_bHandShake[nJob] = false;

		nSeq[nJob] = 0;
		break;
	}
}
void CFormMainView::fnProcess_ProcessSideViewAlgorithm(int nJob )
{
	CString str;
	int tm = STM_INSP + nJob;

	BOOL bit = g_CommPLC.GetBit(m_pMain->m_nInspRequestRead[nJob]);			// 티칭시 노즐 높이 측정용	: Height Measure
	BOOL bit2 = g_CommPLC.GetBit(m_pMain->m_nInspRequestRead[nJob] + 1);	// 노즐 클리닝 상태 검사		: Nozzle Insp
	BOOL bit3 = g_CommPLC.GetBit(m_pMain->m_nInspRequestRead[nJob] + 2);	// 측면 도포 상태 검사		: Dopo Insp

	bit = bit | bit2 | bit3; // 3개중 하나라도 bit 활성화시 검사 진행

	if ((bit && getProcessStart(nJob) != TRUE) || m_pMain->auto_seq_test[nJob])	// Align 시작 조건
	{
		m_pMain->auto_seq_test[nJob] = FALSE;
		if (bit3) m_pMain->m_nNozzleSideCamMode = 2;
		else if (bit2) m_pMain->m_nNozzleSideCamMode = 1;
		else m_pMain->m_nNozzleSideCamMode = 0;

		reset_viewer(nJob);
		setProcessStart(nJob);
		read_glass_id(nJob);
		//seq_SetTimer(tm, 3000);
		SendMessageA(WM_VIEW_CONTROL, MSG_FMV_GLASSID_UPDATE, nJob);
		
		str.Format("[%s] Process Start, ID : %s", get_job_name(nJob), m_pMain->vt_job_info[nJob].main_object_id.c_str());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		AcquireSRWLockExclusive(&m_pMain->g_srwlock); // 동기화 시킴 Tkyuha 20211027
		m_pMain->m_nSeqProcess[nJob] = 100;
		ReleaseSRWLockExclusive(&m_pMain->g_srwlock);
		m_pMain->setProcessStartFlag(nJob);

		//seq_SetTimer(tm, 3000);
	}

	//KJH 2022-06-06 GrabTime 삭제요청에 따른 삭제 작업
	//if (seq_ChkTimer(tm) || g_CommPLC.GetBit(m_pMain->m_nInspRequestRead[nJob]) != TRUE && getProcessStart(nJob))	// Align 시작 조건
	if (g_CommPLC.GetBit(m_pMain->m_nInspRequestRead[nJob]) != TRUE && getProcessStart(nJob))	// Align 시작 조건
	{
		//seq_ResetChkTimer(tm);
		resetProcessStart(nJob);

		g_CommPLC.SetBit(m_pMain->m_nInspOkWrite[nJob], FALSE);
		g_CommPLC.SetBit(m_pMain->m_nInspNgWrite[nJob], FALSE);

		CString str;
		str.Format("[%s] Process End", get_job_name(nJob));
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}
}
void CFormMainView::fnProcessCenterNozzleAlignAlgorithm(int nJob)
{
	CString str;
	int tm = STM_INSP + nJob;

	BOOL bit = g_CommPLC.GetBit(m_pMain->m_nAlignRequestRead[nJob]);  //센터카메라를 이용한 얼라인 측정 요청

	if ((bit && getProcessStart(nJob) != TRUE) || m_pMain->auto_seq_test[nJob])	// Align 시작 조건
	{
		m_pMain->auto_seq_test[nJob] = FALSE;
	
		m_pMain->m_nSeqNozzleGrabCount++;
		//KJH 2022-01-19 Nozzle Align 4회 1Cycle 반복가능하게 변경
		if (m_pMain->m_nSeqNozzleGrabCount > 4 && m_pMain->m_nSeqNozzleGrabCount != 100) m_pMain->m_nSeqNozzleGrabCount = m_pMain->m_nSeqNozzleGrabCount - 4;

		reset_viewer(nJob);
		setProcessStart(nJob);
		read_glass_id(nJob);
		
		SendMessageA(WM_VIEW_CONTROL, MSG_FMV_GLASSID_UPDATE, nJob);

		str.Format("[%s] Process Start, ID : %s", get_job_name(nJob), m_pMain->vt_job_info[nJob].main_object_id.c_str());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		
		AcquireSRWLockExclusive(&m_pMain->g_srwlock); 
		m_pMain->m_nSeqProcess[nJob] = 100;
		ReleaseSRWLockExclusive(&m_pMain->g_srwlock);
		m_pMain->setProcessStartFlag(nJob);

		//seq_SetTimer(tm, 30000);
	}

	//KJH 2022-06-06 GrabTime 삭제요청에 따른 삭제 작업
	if ((g_CommPLC.GetBit(m_pMain->m_nAlignRequestRead[nJob]) != TRUE) && getProcessStart(nJob))	// Align 시작 조건
	//if ((seq_ChkTimer(tm) || g_CommPLC.GetBit(m_pMain->m_nAlignRequestRead[nJob]) != TRUE) && getProcessStart(nJob))	// Align 시작 조건
	{
		//seq_ResetChkTimer(tm);
		resetProcessStart(nJob);

		g_CommPLC.SetBit(m_pMain->m_nAlignOkWrite[nJob], FALSE);
		g_CommPLC.SetBit(m_pMain->m_nAlignNgWrite[nJob], FALSE);

		CString str;
		str.Format("[%s] Process End", get_job_name(nJob));
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}
}
void CFormMainView::fnProcessCenterNozzleGapMeasure(int nJob)
{
	CString str;
	int tm = STM_INSP + nJob;

	BOOL bit = g_CommPLC.GetBit(m_pMain->m_nAlignRequestRead[nJob] + 2);  //노즐과 스테이지 높이 측정용: Height Measure

	if ((bit && getProcessStart(nJob) != TRUE) || m_pMain->auto_seq_test[nJob])	
	{
		m_pMain->auto_seq_test[nJob] = FALSE;

		reset_viewer(nJob);
		setProcessStart(nJob);
		read_glass_id(nJob);

		SendMessageA(WM_VIEW_CONTROL, MSG_FMV_GLASSID_UPDATE, nJob);

		str.Format("[%s] Process Start, ID : %s", get_job_name(nJob), m_pMain->vt_job_info[nJob].main_object_id.c_str());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		AcquireSRWLockExclusive(&m_pMain->g_srwlock); 
		m_pMain->m_nSeqProcess[nJob] = 100;
		ReleaseSRWLockExclusive(&m_pMain->g_srwlock);
		m_pMain->setProcessStartFlag(nJob);

		//seq_SetTimer(tm, 3000);
	}
	//KJH 2022-06-06 GrabTime 삭제요청에 따른 삭제 작업
	//if ((seq_ChkTimer(tm) || g_CommPLC.GetBit(m_pMain->m_nAlignRequestRead[nJob] + 2) != TRUE) && getProcessStart(nJob))	// Align 시작 조건
	if ((g_CommPLC.GetBit(m_pMain->m_nAlignRequestRead[nJob] + 2) != TRUE) && getProcessStart(nJob))	// Align 시작 조건
	{
		//seq_ResetChkTimer(tm);
		resetProcessStart(nJob);

		g_CommPLC.SetBit(m_pMain->m_nAlignOkWrite[nJob] + 4, FALSE);
		g_CommPLC.SetBit(m_pMain->m_nAlignNgWrite[nJob] + 4, FALSE);

		CString str;
		str.Format("[%s] Process End", get_job_name(nJob));
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}
}

//20211209 Tkyuha 티칭 상태 점검용 노즐 과 스테이지 갭
void CFormMainView::fnProcessTeach_Gap_StatusMeasure(int nJob) 
{
	CString str;
	int tm = STM_INSP + nJob;

	BOOL bit = g_CommPLC.GetBit(m_pMain->m_nAlignRequestRead[nJob] + 2);  

	if (bit && getProcessStart(nJob) != TRUE || m_pMain->auto_seq_test[nJob])
	{			
		setProcessStart(nJob);
		reset_viewer(nJob);
		//seq_SetTimer(tm, 10000);
		HideErrorMessageBlink();
		
		str.Format("[%s] Process Start, ID : %s", get_job_name(nJob), m_pMain->vt_job_info[nJob].main_object_id.c_str());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		//KBJ 2022-06-17 센터 노즐 얼라인 조명 켜지고 난후 조명 변경 하기 위함 Delay 추가
		int	nNozzleAlignGrabDelay = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getNozzleAlignGrabDelay();
		Delay(500+nNozzleAlignGrabDelay);

		// KBJ 2022-02-24 add
		str.Format("[%s] Light On", get_job_name(nJob));
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		for (int nCam = 0; nCam < m_pMain->vt_job_info[nJob].light_info.size(); nCam++)
		{
			m_pMain->LightControllerTurnOnOff(nJob, nCam, 0);
		}
		
		//KJH 2022-06-01 Nozzle Gap 조명 켜지기 기다리기 위해 Delay 추가
		Delay(500);

		fnCalcNozzleGapInsp(nJob);

		::SendMessageA(theApp.m_pFrame->m_pPane[PANE_AUTO]->GetSafeHwnd(), WM_VIEW_CONTROL, MSG_PA_UPDATE_ALIGN_RESULT, int(nJob));

	}

	//KJH 2022-06-06 GrabTime 삭제요청에 따른 삭제 작업
	//if ((seq_ChkTimer(tm) || g_CommPLC.GetBit(m_pMain->m_nAlignRequestRead[nJob] + 2) != TRUE) && getProcessStart(nJob))	// Align 시작 조건
	if ((g_CommPLC.GetBit(m_pMain->m_nAlignRequestRead[nJob]+2) != TRUE) && getProcessStart(nJob))	// Align 시작 조건
	{
		resetProcessStart(nJob);
		//seq_ResetChkTimer(tm);

		g_CommPLC.SetBit(m_pMain->m_nAlignOkWrite[nJob]+4, FALSE);
		g_CommPLC.SetBit(m_pMain->m_nAlignNgWrite[nJob]+4, FALSE);

		m_pMain->m_nSeqNozzleGrabCount = 0;

		CString str;
		str.Format("[%s] Process End", get_job_name(nJob));
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}
}


// hsj 2022-10-31 scratch process add
void CFormMainView::fnProcess_ScratchInsp(int nJob)
{
	CString str;
	BOOL bit = g_CommPLC.GetBit(m_pMain->m_nAlignRequestRead[nJob] + 4);

	int ncamera = 0;
	int camCount = m_pMain->vt_job_info[nJob].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[nJob].camera_index;
	
	if (bit && m_bScratch[nJob] != TRUE&& m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseCrackInsp())
	{
		m_bScratch[nJob] = TRUE;
		reset_viewer(nJob);
		HideErrorMessageBlink();

		str.Format("[%s] Process Start, ID: %s", get_job_name(nJob), m_pMain->vt_job_info[nJob].main_object_id.c_str());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		for (int nCam = 0; nCam < camCount; nCam++)
		{
			ncamera = camBuf.at(nCam);
			m_pMain->m_pPattern[ncamera].pDlg = m_pMain;
			m_pMain->m_pPattern[ncamera].nPos = 0;
			m_pMain->m_pPattern[ncamera].nCam = nCam;
			m_pMain->m_pPattern[ncamera].nSearchCount = 2;  //2point search
			m_pMain->m_pPattern[ncamera].nJob = nJob;

			m_pMain->m_bFindPatternEnd[ncamera] = FALSE;
			m_pMain->m_bFindPattern[ncamera] = TRUE;

			AfxBeginThread(Thread_ScratchInsp, &m_pMain->m_pPattern[ncamera]);
		}
	}

	if ((g_CommPLC.GetBit(m_pMain->m_nAlignRequestRead[nJob] + 4) != TRUE) && m_bScratch[nJob] ==TRUE)	// Align 시작 조건
	{
		m_bScratch[nJob] = FALSE;

		g_CommPLC.SetBit(m_pMain->m_nAlignOkWrite[nJob] + 4, FALSE);
		g_CommPLC.SetBit(m_pMain->m_nAlignNgWrite[nJob] + 4, FALSE);

		CString str;
		str.Format("[%s] Process End", get_job_name(nJob));
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}
}

//20211209 Tkyuha 티칭 상태 점검용 노즐 X,Y 위치
void CFormMainView::fnProcessTeach_XYZ_StatusMeasure(int nJob)
{
	CString str;
	int tm = STM_INSP + nJob;

	BOOL bit = g_CommPLC.GetBit(m_pMain->m_nAlignRequestRead[nJob] + 1);

	if ((bit && getProcessStart(MAX_JOB-1) != TRUE))
	{
		reset_viewer(nJob);
		setProcessStart(MAX_JOB-1);
		read_glass_id(nJob);

		SendMessageA(WM_VIEW_CONTROL, MSG_FMV_GLASSID_UPDATE, nJob);

		str.Format("[%s] Process Start", get_job_name(nJob));
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		//KJH 2022-01-05 Center Nozzle Align 시작하기 전에 ExposureTime 변경
		int camCount = theApp.m_pFrame->vt_job_info[nJob].num_of_camera;
		std::vector<int> camBuf = theApp.m_pFrame->vt_job_info[nJob].camera_index;
		int nCam = camBuf.at(1);	//Center Cam Index 1
		int nExposure = m_pMain->vt_job_info[nJob].model_info.getMachineInfo().getExposureTime(nCam);
#ifdef _DAHUHA
		if (m_pMain->m_pDahuhaCam[nCam].isconnected())
			m_pMain->m_pDahuhaCam[nCam].setExposureTime(nExposure);
#else
		if (m_pMain->m_pBaslerCam[nCam])
		{
			m_pMain->m_pBaslerCam[nCam]->SetExposure(nExposure);
		}
		Delay(50);
#endif
		m_pMain->m_nSeqNozzleGrabCount = 100;

		AcquireSRWLockExclusive(&m_pMain->g_srwlock);
		m_pMain->m_nSeqProcess[nJob] = 100;
		ReleaseSRWLockExclusive(&m_pMain->g_srwlock);
		m_pMain->setProcessStartFlag(nJob);

		//seq_SetTimer(tm, 30000);

	}
	//KJH 2022-06-06 GrabTime 삭제요청에 따른 삭제 작업
	if ((g_CommPLC.GetBit(m_pMain->m_nAlignRequestRead[nJob] + 1) != TRUE) && getProcessStart(MAX_JOB - 1))	// Align 시작 조건
	//if ((seq_ChkTimer(tm) || g_CommPLC.GetBit(m_pMain->m_nAlignRequestRead[nJob] + 1) != TRUE) && getProcessStart(MAX_JOB - 1))	// Align 시작 조건
	{
		resetProcessStart(MAX_JOB - 1);
		//seq_ResetChkTimer(tm);

		g_CommPLC.SetBit(m_pMain->m_nAlignOkWrite[nJob] + 2, FALSE);
		g_CommPLC.SetBit(m_pMain->m_nAlignNgWrite[nJob] + 2, FALSE);

		CString str;
		str.Format("[%s] Process End", get_job_name(nJob));
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}
}

void CFormMainView::read_glass_id(int nJob)
{
	CString strTemp, beforeID;
	BYTE main_object_id[MAX_PATH] = { 0, };
	BYTE sub_object_id[MAX_PATH] = { 0, };
	CTime NowTime;
	NowTime = CTime::GetCurrentTime();
	m_pMain->m_strResultDate[nJob].Format("%04d%02d%02d", NowTime.GetYear(), NowTime.GetMonth(), NowTime.GetDay());
	m_pMain->m_strResultTime[nJob].Format("%02d_%02d_%02d", NowTime.GetHour(), NowTime.GetMinute(), NowTime.GetSecond());

	//KJH 2021-08-15 제어쪽에서 Cell ID 10Word(20자)로 변경

	if (m_pMain->vt_job_info[nJob].plc_addr_info.use_main_object_id)
	{
		readGlassID(m_pMain->vt_job_info[nJob].plc_addr_info.read_main_object_id_start, 20, main_object_id);
	}

	if (m_pMain->vt_job_info[nJob].plc_addr_info.use_sub_object_id)
	{
		readGlassID(m_pMain->vt_job_info[nJob].plc_addr_info.read_sub_object_id_start, 20, sub_object_id);
	}

	beforeID.Format("%s", m_pMain->vt_job_info[nJob].main_object_id);
	strTemp.Format("%s", main_object_id);
	if (strTemp.GetLength() < 10)	strTemp.Format("TEST_%02d_%02d_%02d", NowTime.GetHour(), NowTime.GetMinute(), NowTime.GetSecond());
	if (strcmp(beforeID, strTemp) == 0)
		strTemp.Format("%s_%02d_%02d_%02d", main_object_id, NowTime.GetHour(), NowTime.GetMinute(), NowTime.GetSecond());
	m_pMain->vt_job_info[nJob].main_object_id = strTemp;

	strTemp.Format("%s", sub_object_id);
	if (strTemp.GetLength() < 10)	strTemp.Format("SUB_%02d_%02d_%02d", NowTime.GetHour(), NowTime.GetMinute(), NowTime.GetSecond());
	if (strcmp(beforeID, strTemp) == 0)
		strTemp.Format("%s_%02d_%02d_%02d", main_object_id, NowTime.GetHour(), NowTime.GetMinute(), NowTime.GetSecond());
	m_pMain->vt_job_info[nJob].sub_object_id = strTemp;
}
void CFormMainView::readGlassID(int nStartAddr, const int nSize, BYTE* pData)
{
	//long *lData = new long[nSize];
	long lData[MAX_PATH] = { 0, };

	try 
	{
		g_CommPLC.GetWord(nStartAddr, nSize, lData);

		for (int i = 0; i < nSize; i++)
		{
			pData[i * 2 + 0] = (lData[i] >> 0) & 0xFF;
			pData[i * 2 + 1] = (lData[i] >> 8) & 0xFF;
		}

		for (int i = 0; i < nSize * 2; i++)
		{
			if (pData[i] == 32)			pData[i] = '0';
			else if (pData[i] == 10)	pData[i] = '_';
			else if (pData[i] == 0)		break;
			else if (pData[i] < 32)	pData[i] = '_';
		}
	}
	catch (...)
	{

	}
}
void CFormMainView::readModelID(int nStartAddr, int nSize, BYTE* pData)
{
	long* lData = new long[nSize];

	g_CommPLC.GetWord(nStartAddr, nSize, lData);

	for (int i = 0; i < nSize; i++)
	{
		pData[i * 2 + 0] = (lData[i] >> 0) & 0xFF;
		pData[i * 2 + 1] = (lData[i] >> 8) & 0xFF;
	}

	for (int i = 0; i < nSize * 2; i++)
	{
		if (pData[i] == 32)
			pData[i] = '0';
	}

	delete lData;
}
void CFormMainView::readModelID_ZR(int nStartAddr, int nSize, BYTE* pData)
{
	long* lData = new long[nSize];

	g_CommPLC.GetWordZR(nStartAddr, nSize, lData);

	for (int i = 0; i < nSize; i++)
	{
		pData[i * 2 + 0] = (lData[i] >> 0) & 0xFF;
		pData[i * 2 + 1] = (lData[i] >> 8) & 0xFF;
	}

	for (int i = 0; i < nSize * 2; i++)
	{
		if (pData[i] == 32)
			pData[i] = '0';
	}

	delete lData;
}

CString CFormMainView::get_job_name(int nJob)
{
	CString strHeader;

	if (m_pMain->vt_job_info.size() < nJob)		strHeader.Format("Unknown-%d", nJob);
	else										strHeader.Format("%s", m_pMain->vt_job_info[nJob].job_name.c_str());

	return strHeader;
}

void CFormMainView::resultViewLoad(int cam, int id,int command)
{
	// KBJ 2022-09-22 예외처리 수정
	if (m_pMain->vt_job_info[0].algo_method != CLIENT_TYPE_ELB_CENTER_ALIGN) return;
	
	if (m_pMain->vt_job_info[0].num_of_viewer == 4 && id == 4) {}
	else if (m_pMain->vt_job_info[0].num_of_viewer == 2 && id == 0) {}
	else
	{
		return;
	}

	if (m_pMain->m_strLastInspectionFolder.GetLength() > 10)
	{
		CString pathName;

		switch (command)
		{
		case 1:     pathName.Format("%s_ImgRaw.jpg", m_pMain->m_strLastInspectionFolder);  break;
		case 2: 	pathName.Format("%s_ImgRaw2.jpg", m_pMain->m_strLastInspectionFolder); break;
		case 3: 	pathName.Format("%s_ImgRaw3.jpg", m_pMain->m_strLastInspectionFolder); break;
		case 4: 	pathName.Format("%s_result.jpg", m_pMain->m_strLastInspectionFolder);  break;
		}

		cv::Mat loadImg = cv::imread(std::string(pathName), 0);
		int W = m_pMain->m_stCamInfo[cam].w;
		int H = m_pMain->m_stCamInfo[cam].h;

		if (loadImg.cols != W || loadImg.rows != H)			loadImg.release();
		else
		{
			m_pDlgViewerMain[id]->GetViewer().OnLoadImageFromPtr(loadImg.data);
			m_pDlgViewerMain[id]->GetViewer().Invalidate();
			loadImg.release();
		}
	}
}

LRESULT CFormMainView::OnViewControl(WPARAM wParam, LPARAM lParam)
{
	switch (wParam) {
	case MSG_FMV_THREAD_CREATE:
		AfxBeginThread(Thread_PLCSignalCheck, this);
		break;
	case MSG_FMV_JUDGE:
	{
		int nJudge = (int)lParam;

		if (nJudge == 1)
		{
			m_lbPanelJudge.SetSizeText(30.f);
			m_lbPanelJudge.SetColorBkg(255, COLOR_GREEN);
			m_lbPanelJudge.SetText("OK");
		}
		else if (nJudge == 0)
		{
			m_lbPanelJudge.SetSizeText(30.f);
			m_lbPanelJudge.SetColorBkg(255, COLOR_RED);
			m_lbPanelJudge.SetText("NG");
		}
		break;
	}
	case MSG_FMV_DRAW_ALIGN_RESULT:
	{
		_stAlignResult* info = (_stAlignResult*)lParam;

		if (m_pMain->vt_job_info[info->nAlgorithm].algo_method == CLIENT_TYPE_2CAM_EACH_2SHOT_ALIGN)
		{
			draw_align_result(info->nCam, info->nPos, info->nViewer, info->nAlgorithm, info->bErase, info->bType);
		}
		else
		{
			int find_method = m_pMain->vt_job_info[info->nAlgorithm].model_info.getAlignInfo().getMarkFindMethod();
			if (find_method == METHOD_CALIPER)		draw_align_result_caliper(info->nCam, info->nPos, info->nViewer, info->nAlgorithm, info->bErase);
			else if(find_method == METHOD_CIRCLE)	draw_align_result_circle(info->nCam, info->nPos, info->nViewer, info->nAlgorithm, info->bErase, info->bType);
			else if(find_method == METHOD_MARK_EDGE) draw_align_result_mark_edge(info->nCam, info->nPos, info->nViewer, info->nAlgorithm, info->bErase, info->bType);
			else									draw_align_result(info->nCam, info->nPos, info->nViewer, info->nAlgorithm, info->bErase, info->bType);
		}
	}
	break;
	case MSG_FMV_DISPLAY_PANEL_EXIST:
	{
		_stAlignResult* info = (_stAlignResult*)lParam;
		drawExistInspResult(info->nCam, info->nPos, info->nViewer, info->nAlgorithm, info->bErase);
		//m_pMain->dispAlignExistResult(m_pMain->getProcBuffer(0), &m_pDlgViewerMain[0]->GetViewer());
	}
	break;
	case MSG_FMV_DISPLAY_PCB_DISTANCE:
	{
		_stAlignResult* info = (_stAlignResult*)lParam;
		draw_pcb_insp_result2(info->nCam, info->nPos, info->nViewer, info->nAlgorithm, info->bErase);
	}
	break;
	case MSG_FMV_DISPLAY_PANEL_ASSEMBLE:
	{
		_stAlignResult* info = (_stAlignResult*)lParam;
		draw_assemble_insp_result2(info->nCam, info->nPos, info->nViewer, info->nAlgorithm, info->bErase);
	}
	break;
	case MSG_FMV_SET_MODELNAME:
		m_lbModelId.SetText(m_pMain->m_strCurrentModelName);
		break;
	case  MSG_FMV_GLASSID_UPDATE:
		fnReceivePanelId(lParam);
		break;
	case MSG_FMV_DISPLAY_TARGET_DRAW_VIEW:
		break;
	case MSG_FMV_SHOW_MANUAL_INPUT_MARK:
	{
		int nID = LOWORD(lParam);
		BOOL bShow = HIWORD(lParam);

		m_pMain->m_pManualInputMark[nID]->initializeParameter();
		m_pMain->m_pManualInputMark[nID]->ShowWindow(bShow);
		m_pMain->m_pManualInputMark[nID]->SetTimer(TIMER_MANUAL_MARK, m_pMain->m_dMarkInputTime * 1000, NULL);
	}
	break;
	// KBJ 2022-07-05 Manual Mark 1Cam 기능 추가
	case MSG_FMV_SHOW_MANUAL_INPUT_MARK_1CAM:
	{
		int nID = LOWORD(lParam);
		BOOL bShow = HIWORD(lParam);

		m_pMain->m_pManualInputMark1Cam[nID]->initializeParameter();
		m_pMain->m_pManualInputMark1Cam[nID]->ShowWindow(bShow);
		m_pMain->m_pManualInputMark1Cam[nID]->SetTimer(TIMER_MANUAL_MARK, m_pMain->m_dMarkInputTime * 1000, NULL);
	}
	break;
	case MSG_FMV_INIT_INTERFACE:
	{
		resetPlcSignal();
		resetPlcFlag();
		resetCalibrationModeOn();
	}
	break;
	case MSG_FMV_TOOL_COMMAND:
	{
		int ncom = lParam & 0xFFFF;
		int id = lParam >> 16 & 0xFFFF;
		int cam = m_pMain->vt_viewer_info[id].camera_index;

		switch (ncom)
		{
		case MSG_DLGVIEW_TOOL_COMMAND_FIRST:
		{
			m_pMain->vt_viewer_info[id].image_index = 0;
			//if (m_pMain->vt_job_info[0].algo_method == CLIENT_TYPE_ELB_CENTER_ALIGN && id == 1)
			//{
			//	int _thresh = MAX(10,m_pMain->vt_job_info[0].model_info.getAlignInfo().getExistPanelGray());
			//	int w = m_pMain->m_stCamInfo[cam].w;
			//	int h = m_pMain->m_stCamInfo[cam].h;
			//	//20210927 Tkyuha 노즐 높이 측정 테스트용
			//	fnCalcNozzleToPN_Distance(m_pMain->getProcBuffer(cam, 0), w, h, 0, cam, _thresh, &m_pDlgViewerMain[id]->GetViewer());
			//}
			resultViewLoad(cam, id, 1);
		}
		break;
		case MSG_DLGVIEW_TOOL_COMMAND_SECOND: m_pMain->vt_viewer_info[id].image_index = 1;  resultViewLoad(cam, id, 2); break;
		case MSG_DLGVIEW_TOOL_COMMAND_THIRD:  m_pMain->vt_viewer_info[id].image_index = 2;  resultViewLoad(cam, id, 3); break;
		case MSG_DLGVIEW_TOOL_COMMAND_FOUR:   m_pMain->vt_viewer_info[id].image_index = 3;  resultViewLoad(cam, id, 4); break;
		case MSG_DLGVIEW_TOOL_COMMAND_LIVE:
		{
			m_pMain->SendMessage(WM_VIEW_CONTROL, MSG_MD_CAMERA_LIVE, cam);
			m_pMain->vt_viewer_info[id].camera_status = 1;
		}
		break;
		case MSG_DLGVIEW_TOOL_COMMAND_RECORD:
		{
			m_pMain->vt_viewer_info[id].camera_status = 1;

			SYSTEMTIME time;
			::GetLocalTime(&time);
			CString strDate;

			//날짜파일 생성
			CString strImageDir;

			strDate.Format("%04d%02d%02d", time.wYear, time.wMonth, time.wDay);
			strImageDir.Format("%s%s\\", m_pMain->m_strVideoDir, strDate);
			if (_access(strImageDir, 0) != 0)	CreateDirectory(strImageDir, NULL);

			//시간
			CString strTime, strTime2;
			strTime2.Format("%02d_%02d_%02d", time.wHour, time.wMinute, time.wSecond);
			strTime.Format("%02d_%02d", time.wHour, time.wMinute);

			// KBJ 2022-08-25 
			/*if (m_pDlgViewerMain[id]->m_strPanelName.GetLength() < 10)
			{
				strImageDir.Format("%sNoPanelName_%s\\", strImageDir, strTime);
				if (_access(strImageDir, 0) != 0)	CreateDirectory(strImageDir, NULL);
			}
			else*/
			{
				strImageDir.Format("%s%s\\", strImageDir, m_pDlgViewerMain[id]->m_strPanelName);
				if (_access(strImageDir, 0) != 0)	CreateDirectory(strImageDir, NULL);
				else
				{
					// KBJ 2022-11-30 비디오 폴더 분배
					CFileFind Finder;
					BOOL bFind = FALSE;

					bFind = Finder.FindFile(strImageDir + _T("\\*.*"));
					while (bFind)
					{
						bFind = Finder.FindNextFile();

						if (Finder.IsDots() || Finder.IsDirectory())	// '.', '..', '파일' 인 경우 continue;
							continue;

						CString File_name;
						File_name = Finder.GetFilePath();	
						if (File_name.Find(m_pMain->vt_viewer_info[id].viewer_name.c_str()) > 0)
						{
							strImageDir.Format("%s%s\\", m_pMain->m_strVideoDir, strDate);
							strImageDir.Format("%s%s", strImageDir, m_pDlgViewerMain[id]->m_strPanelName);
							strImageDir.Format("%s_%s\\", strImageDir, m_pMain->m_strResultTime[0]);
							CreateDirectory(strImageDir, NULL);
							break;
						}
					}
				}
			}

			//파일명
			CString fname;
			fname.Format("%s%s_%s_%s.avi", strImageDir, strTime2,								//날짜													//result 폴더안
				m_pMain->vt_viewer_info[id].viewer_name.c_str(),								//잡 이름												//시간
				//m_pMain->m_stCamInfo[cam].cName,												//모델 이름
				m_pDlgViewerMain[id]->m_strPanelName
			);

			int w = m_pMain->m_stCamInfo[cam].w;
			int h = m_pMain->m_stCamInfo[cam].h;

#ifdef _SAPERA 
			m_pMain->m_pSaperaCam[cam].cvVideoOpen(fname, w, h);    // dh.jung 2021-05-20 add video
#else
#ifdef _DAHUHA
			if (m_pMain->m_pDahuhaCam[cam].isconnected())  // 20210927 Tkyuha 예외처리
				m_pMain->m_pDahuhaCam[cam].cvVideoOpen(fname, w, h);
#else
			if(m_pMain->m_pBaslerCam!=NULL && m_pMain->m_pBaslerCam[cam])  // 20210927 Tkyuha 예외처리
				m_pMain->m_pBaslerCam[cam]->cvVideoOpen(fname, w, h);
#endif
#endif

			m_pMain->PostMessage(WM_VIEW_CONTROL, MSG_MD_CAMERA_LIVE, cam);

		}
		break;
		case MSG_DLGVIEW_TOOL_COMMAND_STOP:
		{
			m_pMain->vt_viewer_info[id].camera_status = 0;

#ifdef _SAPERA 
			if (m_pMain->m_pSaperaCam[cam].m_bSaveMPEG)
			{
				m_pMain->m_pSaperaCam[cam].cvVideoClose();
			}
#else	
#ifndef _DAHUHA
			//HSJ Video stop
			if (m_pMain->m_pBaslerCam[cam] && m_pMain->m_pBaslerCam[cam]->m_bSaveMPEG)
			{
				Sleep(300);
				m_pMain->m_pBaslerCam[cam]->cvVideoClose();
			}
#else
			if (m_pMain->m_pDahuhaCam[cam].isconnected() && m_pMain->m_pDahuhaCam[cam].m_bSaveMPEG)
			{
				Sleep(300);
				AcquireSRWLockExclusive(&m_pMain->m_pDahuhaCam[cam].g_bufsrwlock); // 동기화 시킴 Tkyuha 20211027
				m_pMain->m_pDahuhaCam[cam].cvVideoClose();
				ReleaseSRWLockExclusive(&m_pMain->m_pDahuhaCam[cam].g_bufsrwlock);
			}
#endif
			else//live stop
			{
				m_pMain->PostMessage(WM_VIEW_CONTROL, MSG_MD_CAMERA_STOP, cam);
			}
#endif
		}
		break;
		}
	}
	break;
	case MSG_FMV_DRAW_LINE_RESULT:
	{
		_stAlignResult* info = (_stAlignResult*)lParam;
		sLine lineinfo = m_pMain->m_ELB_TraceResult.m_ELB_ResultLine[info->nPos];
		draw_align_result_line(info->nCam, info->nPos, info->nViewer, info->nAlgorithm, lineinfo, info->bErase, m_pMain->m_ELB_TraceResult.bType);
	}
	break;
	case MSG_FMV_DISPLAY_SCAN_INSP:
	{
		_stAlignResult* info = (_stAlignResult*)lParam;
		draw_scan_insp_result(info->nCam, info->nPos, info->nViewer, info->nAlgorithm, info->bErase);
	}
	break;
	case MSG_FMV_DISPLAY_FILM_DISTANCE:
	{
		_stAlignResult* info = (_stAlignResult*)lParam;
		draw_film_insp_result(info->nCam, info->nPos, info->nViewer, info->nAlgorithm, info->bErase);
	}
	break;
	case MSG_FMV_AUTO_SEQ_TEST:
	{
		int job = (int)lParam - 1;
		if (job < 0 || job > m_pMain->vt_job_info.size()) return 0;

		CString str_temp;
		str_temp.Format("%s - auto seq test?", m_pMain->vt_job_info[job].get_job_name());

		if (AfxMessageBox(str_temp, MB_YESNO) != IDYES) return 0;

		m_pMain->auto_seq_test[job] = TRUE;
	}
	break;
	//KJH 2022-07-08 Login 기능 추가
	case MSG_FMV_AUTO_LOGOFF_ADMIN_MODE:
	{
		SetTimer(TIMER_AUTO_LOGOFF_ADMIN_MODE, 600000, NULL);		//10분뒤에 자동 로그오프되도록 변경
	}
	break;
	case MSG_FMV_LOGIN_STATUS_DISPLAY:
	{
		if (m_pMain->m_nLogInUserLevelType == MASTER)
		{
			m_lbLogIn.SetText("MASTER LEVEL");
			m_lbLogIn.SetColorBkg(255, COLOR_RED);
		}
		else if (m_pMain->m_nLogInUserLevelType == SUPER)
		{
			m_lbLogIn.SetText("SUPER LEVEL");
			m_lbLogIn.SetColorBkg(255, COLOR_GREEN);
		}
		else if (m_pMain->m_nLogInUserLevelType == USER)
		{
			m_lbLogIn.SetText("LOG IN");
			m_lbLogIn.SetColorBkg(255, RGB(64, 64, 64));
		}
	}
	break;
	}
	return 0;
}

void CFormMainView::drawExistInspResult(int nCam, int nPosNum, int nViewer, int nJob, BOOL bErase)
{
	if (bErase)
	{
		m_pDlgViewerMain[nViewer]->GetViewer().ClearOverlayDC();
		m_pDlgViewerMain[nViewer]->GetViewer().clearAllFigures();
	}
	int nPenThickness = MAIN_VIEW_PEN_THICKNESS;
	int nPenLength = MAIN_VIEW_PEN_LENGTH;
	int nFontWidth = MAIN_VIEW_FONT_WIDTH;
	int nFontHeight = MAIN_VIEW_FONT_HEIGHT;

	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

	COLORREF color = COLOR_GREEN;

	CString strText, strDataText;
	CFindInfo find = m_pMain->GetMatching(nJob).getFindInfo(nCam, 0);
	////////////////////////////////////////////////////////////////////////
	////////////////////////Draw Exist//////////////////////////////////////

	//ROI
	color = COLOR_WHITE;
	stFigure tempFig;
	CRect rcROI;
	rcROI = m_pMain->GetMatching(nJob).getSearchROI(nCam, 0);
	tempFig.ptBegin.x = rcROI.left;
	tempFig.ptBegin.y = rcROI.top;
	tempFig.ptEnd.x = rcROI.right;
	tempFig.ptEnd.y = rcROI.bottom;
	m_pDlgViewerMain[nViewer]->GetViewer().addFigureRect(tempFig, 1, 1, color);

	//찾은 Blob 표시
	if (m_pMain->m_nExistResult == 2)
	{
		long x = 0, y = 0;
		x = rcROI.left;
		y = rcROI.top;
		color = COLOR_GREEN;
		stFigure tempFig;
		for (int i = 0; i < m_pMain->m_RectExistBlob.size(); i++)
		{
			tempFig.ptBegin.x = x + m_pMain->m_RectExistBlob[i].left;
			tempFig.ptBegin.y = y + m_pMain->m_RectExistBlob[i].top;
			tempFig.ptEnd.x = x + m_pMain->m_RectExistBlob[i].right;
			tempFig.ptEnd.y = y + m_pMain->m_RectExistBlob[i].bottom;
			m_pDlgViewerMain[nViewer]->GetViewer().addFigureRect(tempFig, 1, 1, color);
		}
	}

	//Text
	if (m_pMain->m_nExistResult > 0)		color = COLOR_GREEN;
	else 									color = COLOR_RED;
	if (m_pMain->m_nExistResult == 3)		strText.Format("Result: %s ", "Empty");
	else if (m_pMain->m_nExistResult == 1)		strText.Format("Result: %s ", "GLASS");
	else if (m_pMain->m_nExistResult == 2)		strText.Format("Result: %s ", "Separator");
	else if (m_pMain->m_nExistResult == -1)	strText.Format("Result: %s ", "Light Error");
	//m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, 15, 15, rcROI.left + 30, rcROI.top + 30);
	//strDataText.Format("Avg Brigtness: %.0f", m_pMain->m_dExistResultData);
	//m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strDataText, color, 15, 15, rcROI.left + 30, rcROI.top + 100);

	m_pDlgViewerMain[nViewer]->GetViewer().AddHardGraphic(new GraphicLabel(rcROI.left, rcROI.top, fmt("%s\nAvg Brigtness: %.1f", strText, m_pMain->m_dExistResultData)));

	m_pDlgViewerMain[nViewer]->GetViewer().OnLoadImageFromPtr(m_pMain->getProcBuffer(real_cam, 0));
	m_pDlgViewerMain[nViewer]->GetViewer().DirtyRefresh();
}
void CFormMainView::drawAlignResult(int nCam, int nPosNum, int nViewer, int nJob, BOOL bErase)
{
	auto pDC = m_pDlgViewerMain[nViewer]->GetViewer().getOverlayDC();
	if (bErase)
	{
		m_pDlgViewerMain[nViewer]->GetViewer().ClearOverlayDC();
		//m_pDlgViewerMain[nViewer]->GetViewer().clearAllFigures();
	}
	int nPenThickness = MAIN_VIEW_PEN_THICKNESS;
	int nPenLength = MAIN_VIEW_PEN_LENGTH;
	int nFontWidth = MAIN_VIEW_FONT_WIDTH;
	int nFontHeight = MAIN_VIEW_FONT_HEIGHT;

	CPen penOK(PS_SOLID, nPenThickness, RGB(0, 255, 0)), * pOldPen;
	CPen penNG(PS_SOLID, nPenThickness, RGB(255, 0, 0)), penMatch(PS_SOLID, nPenThickness, RGB(255, 255, 0));
	CPen penROI(PS_SOLID, nPenThickness, RGB(255, 255, 255));

	CFont font, * oldFont;

	font.CreateFont(nFontHeight, nFontWidth, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
		OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, "Arial");
	oldFont = pDC->SelectObject(&font);

	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

	if (bErase)	m_pDlgViewerMain[nViewer]->GetViewer().ClearOverlayDC();
	int W = m_pDlgViewerMain[nViewer]->GetViewer().GetWidth();
	int H = m_pDlgViewerMain[nViewer]->GetViewer().GetHeight();

	for (int nPos = 0; nPos < nPosNum; nPos++)
	{
		CFindInfo info = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos);
		pOldPen = pDC->SelectObject(&penOK);

		double posX = info.GetXPos();
		double posY = info.GetYPos();
		double score = info.getScore();
		double angle = info.GetAngle();

		int index = info.GetFoundPatternNum();

		double mark_offset_x = m_pMain->GetMatching(nJob).getFinalMarkOffsetX(nCam, nPos, index);
		double mark_offset_y = m_pMain->GetMatching(nJob).getFinalMarkOffsetY(nCam, nPos, index);

		posX += mark_offset_x;
		posY += mark_offset_y;

		CRect rcROI;
		rcROI = m_pMain->GetMatching(nJob).getSearchROI(nCam, nPos);

		COLORREF color = COLOR_GREEN;
		if (info.GetFound() == FIND_ERR) { color = COLOR_RED; }
		else if (info.GetFound() == FIND_MATCH) { color = COLOR_YELLOW; }

		int fatW = m_pMain->GetMatching(nJob).getHalcon(nCam, nPos, index).getFatWidth();
		int fatH = m_pMain->GetMatching(nJob).getHalcon(nCam, nPos, index).getFatHeight();
		double offsetX = m_pMain->GetMatching(nJob).getPosOffsetX(nCam, nPos, index);
		double offsetY = m_pMain->GetMatching(nJob).getPosOffsetY(nCam, nPos, index);

		/*
		stFigure tempFig;

		int oriX = (int)(posX - offsetX);
		int oriY = (int)(posY - offsetY);

		tempFig.ptBegin.x = oriX - fatW / 2;
		tempFig.ptBegin.y = oriY + fatH / 2;
		tempFig.ptEnd.x = oriX + fatW / 2;
		tempFig.ptEnd.y = oriY + fatH / 2;
		m_pDlgViewerMain[nViewer]->GetViewer().addFigureLine(tempFig, 2, 1, color);

		tempFig.ptBegin.x = oriX - fatW / 2;
		tempFig.ptBegin.y = oriY + fatH / 2;
		tempFig.ptEnd.x = oriX - fatW / 2;
		tempFig.ptEnd.y = oriY - fatH / 2;
		m_pDlgViewerMain[nViewer]->GetViewer().addFigureLine(tempFig, 2, 1, color);

		tempFig.ptBegin.x = oriX + fatW / 2;
		tempFig.ptBegin.y = oriY - fatH / 2;
		tempFig.ptEnd.x = oriX + fatW / 2;
		tempFig.ptEnd.y = oriY + fatH / 2;
		m_pDlgViewerMain[nViewer]->GetViewer().addFigureLine(tempFig, 2, 1, color);

		tempFig.ptBegin.x = oriX + fatW / 2;
		tempFig.ptBegin.y = oriY - fatH / 2;
		tempFig.ptEnd.x = oriX - fatW / 2;
		tempFig.ptEnd.y = oriY - fatH / 2;
		m_pDlgViewerMain[nViewer]->GetViewer().addFigureLine(tempFig, 2, 1, color);

		m_pMain->draw_mark(&m_pDlgViewerMain[nViewer]->GetViewer(), color, posX, posY, nPenThickness, nPenThickness / 2, nPenLength);
		*/

		pDC->SetBkMode(TRANSPARENT);
		CString strText;
		strText.Format("%d, [%.1f , %.1f] %.1f%% - [%d] %.2f˚", nPos, posX, posY, score, index + 1, angle);

		pDC->SelectObject(&penROI);
		pDC->MoveTo(rcROI.left, rcROI.top);
		pDC->LineTo(rcROI.right, rcROI.top);
		pDC->LineTo(rcROI.right, rcROI.bottom);
		pDC->LineTo(rcROI.left, rcROI.bottom);
		pDC->LineTo(rcROI.left, rcROI.top);

		if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod() == METHOD_RANSAC)
		{
			CPen penRealROI(PS_SOLID, nPenThickness, RGB(255, 255, 0));
			pDC->SelectObject(&penRealROI);

			CRect rcRealROI = m_pMain->GetMatching(nJob).m_rcRealROI[nCam][nPos][0];
			pDC->MoveTo(rcRealROI.left, rcRealROI.top);
			pDC->LineTo(rcRealROI.left, rcRealROI.bottom);
			pDC->LineTo(rcRealROI.right, rcRealROI.bottom);
			pDC->LineTo(rcRealROI.right, rcRealROI.top);
			pDC->LineTo(rcRealROI.left, rcRealROI.top);

			rcRealROI = m_pMain->GetMatching(nJob).m_rcRealROI[nCam][nPos][1];
			pDC->MoveTo(rcRealROI.left, rcRealROI.top);
			pDC->LineTo(rcRealROI.left, rcRealROI.bottom);
			pDC->LineTo(rcRealROI.right, rcRealROI.bottom);
			pDC->LineTo(rcRealROI.right, rcRealROI.top);
			pDC->LineTo(rcRealROI.left, rcRealROI.top);
		}

		pDC->SelectObject(pOldPen);
	}

#ifndef JOB_INFO
	if (m_pMain->getModel().getAlignInfo().getEnableLCheck(nCam))
#else
	if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getEnableLCheck())
#endif
	{
		if (m_pMain->m_bLcheckNG[nJob])
		{
			CPen penCrack(PS_SOLID, nPenThickness, COLOR_RED), * pOld;
			pOld = pDC->SelectObject(&penCrack);

			CFont font, * oldFont;
			font.CreateFont(nFontHeight, nFontWidth, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
				OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
			oldFont = pDC->SelectObject(&font);

			COLORREF oldColor = pDC->SetTextColor(COLOR_RED);
			pDC->TextOutA(W - nFontWidth * 20, H - nFontHeight * 4, "L-CHECK ERROR");

			CString strText;
#ifndef JOB_INFO
			strText.Format("lmt : %.3f", m_pMain->getModel().getAlignInfo().getLCheckLimit(nCam));
#else
			strText.Format("lmt : %.3f", m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckTor());
#endif
			pDC->TextOutA(W - nFontWidth * 20, H - nFontHeight * 2, strText);

			strText.Format("L : %.3f", m_pMain->m_dbLcheckLength[nJob]);
			pDC->TextOutA(W - nFontWidth * 20, H - nFontHeight * 3, strText);

			pDC->SetTextColor(oldColor);
			pDC->SelectObject(oldFont);
		}
	}


	m_pDlgViewerMain[nViewer]->GetViewer().OnLoadImageFromPtr(m_pMain->getProcBuffer(real_cam, 0));
	m_pDlgViewerMain[nViewer]->GetViewer().Invalidate();

	font.DeleteObject();
	penOK.DeleteObject();
	penNG.DeleteObject();
	penMatch.DeleteObject();
	penROI.DeleteObject();
	//	auto pDC = m_pDlgViewerMain[nViewer]->GetViewer().getOverlayDC();
	//	if (bErase)
	//	{
	//		m_pDlgViewerMain[nViewer]->GetViewer().ClearOverlayDC();
	//		//m_pDlgViewerMain[nViewer]->GetViewer().clearAllFigures();
	//	}
	//	int nPenThickness = MAIN_VIEW_PEN_THICKNESS;
	//	int nPenLength = MAIN_VIEW_PEN_LENGTH;
	//	int nFontWidth = MAIN_VIEW_FONT_WIDTH;
	//	int nFontHeight = MAIN_VIEW_FONT_HEIGHT;
	//
	//	CPen penOK(PS_SOLID, nPenThickness, RGB(0, 255, 0)), * pOldPen;
	//	CPen penNG(PS_SOLID, nPenThickness, RGB(255, 0, 0)), penMatch(PS_SOLID, nPenThickness, RGB(255, 255, 0));
	//	CPen penROI(PS_SOLID, nPenThickness, RGB(255, 255, 255));
	//
	//	CFont font, * oldFont;
	//	int OLD_BKMODE = 0;
	//
	//	font.CreateFont(nFontHeight, nFontWidth, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
	//		OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY,
	//		DEFAULT_PITCH | FF_DONTCARE, "Arial");
	//	oldFont = pDC->SelectObject(&font);
	//
	//	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
	//
	//	if (bErase)	m_pDlgViewerMain[nViewer]->GetViewer().ClearOverlayDC();
	//	int W = m_pDlgViewerMain[nViewer]->GetViewer().GetWidth();
	//	int H = m_pDlgViewerMain[nViewer]->GetViewer().GetHeight();
	//
	//	for (int nPos = 0; nPos < nPosNum; nPos++)
	//	{
	//		CFindInfo info = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos);
	//		pOldPen = pDC->SelectObject(&penOK);
	//
	//		double posX = info.GetXPos();
	//		double posY = info.GetYPos();
	//		double score = info.getScore();
	//		double angle = info.GetAngle();
	//
	//		int index = info.GetFoundPatternNum();
	//
	//		double mark_offset_x = m_pMain->GetMatching(nJob).getFinalMarkOffsetX(nCam, nPos, index);
	//		double mark_offset_y = m_pMain->GetMatching(nJob).getFinalMarkOffsetY(nCam, nPos, index);
	//
	//		posX += mark_offset_x;
	//		posY += mark_offset_y;
	//
	//		CRect rcROI;
	//		rcROI = m_pMain->GetMatching(nJob).getSearchROI(nCam, nPos);
	//
	//		COLORREF color = COLOR_GREEN;
	//		if (info.GetFound() == FIND_ERR) { color = COLOR_RED; }
	//		else if (info.GetFound() == FIND_MATCH) { color = COLOR_YELLOW; }
	//
	//		int fatW = m_pMain->GetMatching(nJob).getHalcon(nCam, nPos, index).getFatWidth();
	//		int fatH = m_pMain->GetMatching(nJob).getHalcon(nCam, nPos, index).getFatHeight();
	//		double offsetX = m_pMain->GetMatching(nJob).getPosOffsetX(nCam, nPos, index);
	//		double offsetY = m_pMain->GetMatching(nJob).getPosOffsetY(nCam, nPos, index);
	//
	//		stFigure tempFig;
	//
	//		int oriX = (int)(posX - offsetX);
	//		int oriY = (int)(posY - offsetY);
	//
	//		tempFig.ptBegin.x = oriX - fatW / 2;
	//		tempFig.ptBegin.y = oriY + fatH / 2;
	//		tempFig.ptEnd.x = oriX + fatW / 2;
	//		tempFig.ptEnd.y = oriY + fatH / 2;
	//		m_pDlgViewerMain[nViewer]->GetViewer().addFigureLine(tempFig, 2, 1, color);
	//
	//		tempFig.ptBegin.x = oriX - fatW / 2;
	//		tempFig.ptBegin.y = oriY + fatH / 2;
	//		tempFig.ptEnd.x = oriX - fatW / 2;
	//		tempFig.ptEnd.y = oriY - fatH / 2;
	//		m_pDlgViewerMain[nViewer]->GetViewer().addFigureLine(tempFig, 2, 1, color);
	//
	//		tempFig.ptBegin.x = oriX + fatW / 2;
	//		tempFig.ptBegin.y = oriY - fatH / 2;
	//		tempFig.ptEnd.x = oriX + fatW / 2;
	//		tempFig.ptEnd.y = oriY + fatH / 2;
	//		m_pDlgViewerMain[nViewer]->GetViewer().addFigureLine(tempFig, 2, 1, color);
	//
	//		tempFig.ptBegin.x = oriX + fatW / 2;
	//		tempFig.ptBegin.y = oriY - fatH / 2;
	//		tempFig.ptEnd.x = oriX - fatW / 2;
	//		tempFig.ptEnd.y = oriY - fatH / 2;
	//		m_pDlgViewerMain[nViewer]->GetViewer().addFigureLine(tempFig, 2, 1, color);
	//
	//		m_pMain->draw_mark(&m_pDlgViewerMain[nViewer]->GetViewer(), color, posX, posY, nPenThickness, nPenThickness / 2, nPenLength);
	//
	//		OLD_BKMODE = pDC->SetBkMode(TRANSPARENT);
	//		CString strText;
	//		strText.Format("%d, [%.1f, %.1f] %.1f%% - [%d] %.2f°", nPos, posX, posY, score, index + 1, angle);
	//
	//		pDC->SelectObject(&penROI);
	//		pDC->MoveTo(rcROI.left, rcROI.top);
	//		pDC->LineTo(rcROI.right, rcROI.top);
	//		pDC->LineTo(rcROI.right, rcROI.bottom);
	//		pDC->LineTo(rcROI.left, rcROI.bottom);
	//		pDC->LineTo(rcROI.left, rcROI.top);
	//
	//		if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod() == METHOD_RANSAC)
	//		{
	//			CPen penRealROI(PS_SOLID, nPenThickness, RGB(255, 255, 0));
	//			pDC->SelectObject(&penRealROI);
	//
	//			CRect rcRealROI = m_pMain->GetMatching(nJob).m_rcRealROI[nCam][nPos][0];
	//			pDC->MoveTo(rcRealROI.left, rcRealROI.top);
	//			pDC->LineTo(rcRealROI.left, rcRealROI.bottom);
	//			pDC->LineTo(rcRealROI.right, rcRealROI.bottom);
	//			pDC->LineTo(rcRealROI.right, rcRealROI.top);
	//			pDC->LineTo(rcRealROI.left, rcRealROI.top);
	//
	//			rcRealROI = m_pMain->GetMatching(nJob).m_rcRealROI[nCam][nPos][1];
	//			pDC->MoveTo(rcRealROI.left, rcRealROI.top);
	//			pDC->LineTo(rcRealROI.left, rcRealROI.bottom);
	//			pDC->LineTo(rcRealROI.right, rcRealROI.bottom);
	//			pDC->LineTo(rcRealROI.right, rcRealROI.top);
	//			pDC->LineTo(rcRealROI.left, rcRealROI.top);
	//		}
	//
	//		pDC->SelectObject(pOldPen);
	//		pDC->SetBkMode(OLD_BKMODE);
	//	}
	//
	//#ifndef JOB_INFO
	//	if (m_pMain->getModel().getAlignInfo().getEnableLCheck(nCam))
	//#else
	//	if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getEnableLCheck())
	//#endif
	//	{
	//		if (m_pMain->m_bLcheckNG[nJob])
	//		{
	//			CPen penCrack(PS_SOLID, nPenThickness, COLOR_RED), * pOld;
	//			pOld = pDC->SelectObject(&penCrack);
	//
	//			CFont font, * oldFont;
	//			font.CreateFont(nFontHeight, nFontWidth, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
	//				OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
	//			oldFont = pDC->SelectObject(&font);
	//
	//			COLORREF oldColor = pDC->SetTextColor(COLOR_RED);
	//			pDC->TextOutA(W - nFontWidth * 20, H - nFontHeight * 4, "L-CHECK ERROR");
	//
	//			CString strText;
	//#ifndef JOB_INFO
	//			strText.Format("lmt: %.3f", m_pMain->getModel().getAlignInfo().getLCheckLimit(nCam));
	//#else
	//			strText.Format("lmt: %.3f", m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckTor());
	//#endif
	//			pDC->TextOutA(W - nFontWidth * 20, H - nFontHeight * 2, strText);
	//
	//			strText.Format("L: %.3f", m_pMain->m_dbLcheckLength[nJob]);
	//			pDC->TextOutA(W - nFontWidth * 20, H - nFontHeight * 3, strText);
	//
	//			pDC->SetTextColor(oldColor);
	//			pDC->SelectObject(oldFont);
	//		}
	//	}
	//
	//
	//	m_pDlgViewerMain[nViewer]->GetViewer().OnLoadImageFromPtr(m_pMain->getProcBuffer(real_cam, 0));
	//	m_pDlgViewerMain[nViewer]->GetViewer().Invalidate();
	//
	//	font.DeleteObject();
	//	penOK.DeleteObject();
	//	penNG.DeleteObject();
	//	penMatch.DeleteObject();
	//	penROI.DeleteObject();
}
void CFormMainView::draw_align_result_line(int nCam, int nPos, int nViewer, int nJob, sLine lineInfo, BOOL bErase, BOOL bType)
{
	auto pDC = m_pDlgViewerMain[nViewer]->GetViewer().getOverlayDC();

	if (bErase)
	{
		m_pDlgViewerMain[nViewer]->GetViewer().ClearOverlayDC();
		//m_pDlgViewerMain[nViewer]->GetViewer().clearAllFigures();
	}

	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

	int nWidth = m_pMain->m_stCamInfo[real_cam].w;
	int nHeight = m_pMain->m_stCamInfo[real_cam].h;

	COLORREF color = COLOR_RED;
	CPoint ptStart, ptEnd;

	if (bType)
	{
	}
	//KJH 2021-05-29 Trace 아닐때 캘리퍼 라인그리기
	else
	{
		//KJH 2021-06-05 Align시 Display 영역
		//KJH 2021-05-30 Trace NOTCH 모드일때 Align 캘리퍼 선 2개 표시
		int method = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod();
		if (method == METHOD_LINE)	m_pMain->draw_line(&m_pDlgViewerMain[nViewer]->GetViewer(), color, lineInfo, nHeight, nWidth);
		if (method == METHOD_NOTCH)
		{
			int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
			sLine line_info = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][0].m_lineInfo;
			m_pMain->draw_line(&m_pDlgViewerMain[nViewer]->GetViewer(), color, line_info, nHeight, nWidth);
			line_info = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][1].m_lineInfo;
			m_pMain->draw_line(&m_pDlgViewerMain[nViewer]->GetViewer(), color, line_info, nHeight, nWidth);
		}
		if (method == METHOD_CIRCLE)
		{
			CString strText ="";
			CString strText1 = "";

			COLORREF color = COLOR_GREEN;

			int nPenThickness = 2;
			double posX = m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x;
			double posY = m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y;

			m_pMain->draw_mark(&m_pDlgViewerMain[nViewer]->GetViewer(), color, posX, posY, nPenThickness / 2, nPenThickness / 2, 15);

			//stFigure tempFig;
			//tempFig.ptBegin.x = posX - CIRCLE_RADIUS;
			//tempFig.ptBegin.y = posY - CIRCLE_RADIUS;
			//tempFig.ptEnd.x = posX + CIRCLE_RADIUS;
			//tempFig.ptEnd.y = posY + CIRCLE_RADIUS;
			//m_pDlgViewerMain[nViewer]->GetViewer().addFigureRect(tempFig, 1, 1, color);


			////////////////////////////////////// 2022-05-07 KBJ gray average
			// area
			int size_pixcel = 10;
			double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(0, 0);
			double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);

			CRect rect_area;
			rect_area.left = posX - size_pixcel;
			rect_area.right = posX + size_pixcel;
			rect_area.top = posY - size_pixcel;
			rect_area.bottom = posY + size_pixcel;

			// gray
			cv::Mat img(nHeight, nWidth, CV_8UC1, m_pMain->getProcBuffer(real_cam, 0));
			cv::Mat InspImg = img(cv::Rect(rect_area.left, rect_area.top, rect_area.Width(), rect_area.Height()));
			cv::Scalar scalar = cv::mean(InspImg);

			if (m_pMain->m_nErrorType[nJob] == ERR_TYPE_CIRCLE_ERROR)
			{
				color = COLOR_RED;

				double PNSizespec = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getPNSizeSpec() ;
				double PNSizespectorr = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getPNSizeSpecTorr() ;

				double PNSizeSpecMin = PNSizespec - PNSizespectorr;
				double PNSizeSpecMax = PNSizespec + PNSizespectorr;

				strText.Format("Find Circle Error");
				m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, 18, 18, posX - CIRCLE_RADIUS, posY - CIRCLE_RADIUS - 450, TRUE);
				strText.Format("Circle : %.2f", m_pMain->m_ELB_TraceResult.m_nRadius * xres);
				m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, 15, 15, posX - CIRCLE_RADIUS, posY - CIRCLE_RADIUS - 300, TRUE);
				strText.Format("Spec(%.2f) : (%.2f ~ %.2f)", PNSizespec, PNSizeSpecMin, PNSizeSpecMax);
				m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, 15, 15, posX - CIRCLE_RADIUS, posY - CIRCLE_RADIUS - 150, TRUE);
			}

			m_pMain->draw_mark(&m_pDlgViewerMain[nViewer]->GetViewer(), color, posX, posY, nPenThickness / 2, nPenThickness / 2, 15);

			//stFigure tempFig;
			//tempFig.isDot = false;
			//tempFig.ptBegin.x = posX - CIRCLE_RADIUS;
			//tempFig.ptBegin.y = posY - CIRCLE_RADIUS;
			//tempFig.ptEnd.x = posX + CIRCLE_RADIUS;
			//tempFig.ptEnd.y = posY + CIRCLE_RADIUS;
			//m_pDlgViewerMain[nViewer]->GetViewer().addFigureRect(tempFig, 2, 2, color);
			CRect rcROI = CRect(posX - CIRCLE_RADIUS, posY - CIRCLE_RADIUS, posX + CIRCLE_RADIUS, posY + CIRCLE_RADIUS);
			m_pDlgViewerMain[nViewer]->GetViewer().AddSoftGraphic(new GraphicRectangle(rcROI, color | 0xff000000, 2.0));

			// draw
			/*tempFig.isDot = false;
			tempFig.ptBegin.x = posX - size_pixcel;
			tempFig.ptBegin.y = posY - size_pixcel;
			tempFig.ptEnd.x = posX + size_pixcel;
			tempFig.ptEnd.y = posY + size_pixcel;
			m_pDlgViewerMain[nViewer]->GetViewer().addFigureRect(tempFig, 1, 1, color);*/
			rcROI = CRect(posX - size_pixcel, posY - size_pixcel, posX + size_pixcel, posY + size_pixcel);
			m_pDlgViewerMain[nViewer]->GetViewer().AddSoftGraphic(new GraphicRectangle(rcROI, color | 0xff000000, 1.0));

			strText.Format("[Gray]	 : %.1f", scalar.val[0]);
			//m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, 5, 13, rect_area.left, rect_area.bottom, TRUE);

			strText1.Format("[Radius] : %.3fmm", m_pMain->m_ELB_TraceResult.m_nRadius * xres);
			//m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, 5, 13, rect_area.left, rect_area.bottom + 120, TRUE);

			m_pDlgViewerMain[nViewer]->GetViewer().AddSoftGraphic(new GraphicLabel(m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x, m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y, fmt("%s\n%s", strText, strText1), Gdiplus::Color(0xff00ff00)));
			/////////////////////////////////////
		}
	}

	m_pDlgViewerMain[nViewer]->GetViewer().SoftDirtyRefresh();
}
void CFormMainView::draw_align_result_caliper(int nCam, int nPos, int nViewer, int nJob, BOOL bErase)
{
	//KJH 2021-08-14
	//ELB 1Cam Caliper 2Point / Pos 1 -> Line 1(좌),2(상) / Pos 2 -> Line 1(우),2(상)
	auto pDC = m_pDlgViewerMain[nViewer]->GetViewer().getOverlayDC();
	if (bErase)
	{
		if (m_pMain->vt_job_info[nJob].algo_method != CLIENT_TYPE_1CAM_1SHOT_ALIGN_WITH_SCRATCH_INSP
			&& m_pMain->vt_job_info[nJob].algo_method != CLIENT_TYPE_1CAM_1SHOT_FILM)
		{
			m_pDlgViewerMain[nViewer]->GetViewer().ClearOverlayDC();
			m_pDlgViewerMain[nViewer]->GetViewer().clearAllFigures();
		}
	}

	if (m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_1CAM_1SHOT_ALIGN ||
		m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_1CAM_1SHOT_FILM || 
		m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_1CAM_1SHOT_ALIGN_WITH_SCRATCH_INSP)		nCam = 0;

	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

	int nWidth = m_pMain->m_stCamInfo[real_cam].w;
	int nHeight = m_pMain->m_stCamInfo[real_cam].h;
	int nViewWidth = m_pDlgViewerMain[nViewer]->GetViewer().GetWidth();
	int nViewHeight = m_pDlgViewerMain[nViewer]->GetViewer().GetHeight();
	int nRate = double(nHeight / nViewHeight) + double(nWidth / nViewWidth) - 1;

	int nPenThickness = MAIN_VIEW_PEN_THICKNESS * nRate;
	int nPenLength = MAIN_VIEW_PEN_LENGTH * nRate;
	int nFontWidth = MAIN_VIEW_FONT_WIDTH * nRate;
	int nFontHeight = MAIN_VIEW_FONT_HEIGHT * nRate;

	int W = m_pDlgViewerMain[nViewer]->GetViewer().GetWidth();
	int H = m_pDlgViewerMain[nViewer]->GetViewer().GetHeight();

	COLORREF color = COLOR_GREEN;
	CPoint ptStart, ptEnd;

	BOOL bFindLine[2];
	sLine line_info[2];

	double posx[2], posy[2];

	//////////////////// Fixture //////////////////////
	BOOL bFixtureUse = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseFixtureMark();
	if (bFixtureUse)
	{
		double fixtureX = m_pMain->GetMatching(nJob).getFixtureX(nCam, nPos);
		double fixtureY = m_pMain->GetMatching(nJob).getFixtureY(nCam, nPos);

		// Fixture 마크 표시
		// kbj 2022-01-10 Fixture Mark Color = ORANGE, Referace Mark Color = BULE
		//color = COLOR_ORANGE;
		//m_pMain->draw_mark(&m_pDlgViewerMain[nViewer]->GetViewer(), color, fixtureX, fixtureY, nPenThickness, nPenThickness / 2, nPenLength);

		////찾은 마크 표시
		if (m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetFound() == FIND_OK)					color = COLOR_ORANGE;
		else if (m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetFound() == FIND_MATCH)			color = COLOR_YELLOW;
		else			color = COLOR_RED;

		double posX = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetMatchingXPos();
		double posY = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetMatchingYPos();
		m_pMain->draw_mark(&m_pDlgViewerMain[nViewer]->GetViewer(), color, posX, posY, nPenThickness * 2, nPenThickness, nPenLength);

		//ROI
		color = COLOR_WHITE;
		stFigure tempFig;
		CRect rcROI;
		rcROI = m_pMain->GetMatching(nJob).getSearchROI(nCam, nPos);
		tempFig.ptBegin.x = rcROI.left;
		tempFig.ptBegin.y = rcROI.top;
		tempFig.ptEnd.x = rcROI.right;
		tempFig.ptEnd.y = rcROI.bottom;
		m_pDlgViewerMain[nViewer]->GetViewer().addFigureRect(tempFig, 1, 1, color);
	}

	color = COLOR_GREEN;
	////////////////////////////////////////////////////////
	////////////////////////Draw Caliper////////////////////
	if (nPos == 0)
	{
		line_info[0] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_1].m_lineInfo;
		line_info[1] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_2].m_lineInfo;

		bFindLine[0] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_1].getIsMakeLine();
		bFindLine[1] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_2].getIsMakeLine();
	}
	else
	{
		if (m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_1CAM_1SHOT_ALIGN ||
			m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_1CAM_2POS_REFERENCE ||
			m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_1CAM_4POS_ROBOT ||
			m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_1CAM_1SHOT_FILM ||
			m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_1CAM_1SHOT_ALIGN_WITH_SCRATCH_INSP)
		{
			line_info[0] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_1].m_lineInfo;
			line_info[1] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_2].m_lineInfo;
			bFindLine[0] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_1].getIsMakeLine();
			bFindLine[1] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_2].getIsMakeLine();
		}
		else
		{
			line_info[0] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_4].m_lineInfo;
			line_info[1] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_5].m_lineInfo;
			bFindLine[0] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_4].getIsMakeLine();
			bFindLine[1] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_5].getIsMakeLine();
		}
	}

	//////////////////// 라인 그리기 //////////////////////
	if (bFindLine[0] && bFindLine[1])	color = COLOR_GREEN;
	else								color = COLOR_RED;

	// 수직 그리기
	if (bFindLine[0])
		m_pMain->draw_line(&m_pDlgViewerMain[nViewer]->GetViewer(), color, line_info[0], nHeight, nWidth);

	// 수평 그리기
	if (bFindLine[1])
		m_pMain->draw_line(&m_pDlgViewerMain[nViewer]->GetViewer(), color, line_info[1], nHeight, nWidth);

	//////////////////// 후보군 그리기 //////////////////////
	// kbj 2022-01-03 modify draw caliper_result 
	//m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][0].draw_final_result(pDC);
	//m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][1].draw_final_result(pDC);
	m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][0].draw_final_result(pDC);
	m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][1].draw_final_result(pDC);

	//////////////////// 매칭율 //////////////////////
	double dAngle = 0;
	double dRate = 0;
	if (bFindLine[0] && bFindLine[1])
	{
		sLine lineHori = line_info[0];
		sLine lineVert = line_info[1];

		CCaliper::cramersRules(-lineHori.a, 1, -lineVert.a, 1, lineHori.b, lineVert.b, &posx[0], &posy[0]);

		dAngle = m_pMain->calcIncludedAngle(lineHori, lineVert);
		dRate = m_pMain->make_included_angle_to_matching(dAngle, nJob);
	}

	//////////////////// Text 표시 //////////////////////
	CString strTemp;
	if (bFindLine[0] && bFindLine[1])
	{
		color = COLOR_GREEN;
		//HTK 2022-06-29 Caliper Method시 Angle 판정 추가
		if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseIncludedAngle() &&
			m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod() == METHOD_CALIPER)
		{
			double angle_spec = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getIncludedAngle();
			double angle_limit = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getIncludedAngleLimit();

			if (fabs(dAngle - angle_spec) > angle_limit)
			{
				color = COLOR_RED;
			}
		}

		//strTemp.Format("(%.1f, %.1f) - %.2f%% [%.2f°]", posx[0], posy[0], dRate, dAngle);
		//m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strTemp, color, 12, 12, 40, 40 + nPos * 100, TRUE);
		CStringW str;
		str.Format(L"(%.1f, %.1f) - %.2f%% [%.2f°]", posx[0], posy[0], dRate, dAngle);
		m_pDlgViewerMain[nViewer]->GetViewer().AddHardGraphic(new GraphicLabel(cv::Point2f(10, 10 + (nPos * 20)), str, ColorRefToColor(color)));
	}
	else
	{
		CString strText;
		if (bFindLine[0] != TRUE)	strText.Format("%s", m_pMain->m_pDlgCaliper->m_cCaliperName[real_cam][nPos][0]);
		else						strText.Format("%s", m_pMain->m_pDlgCaliper->m_cCaliperName[real_cam][nPos][1]);

		color = COLOR_RED;
		strTemp.Format("Fail to find Line: [%s]", strText);
		m_pDlgViewerMain[nViewer]->GetViewer().AddHardGraphic(new GraphicLabel(10, 10 + (nPos * 20), strTemp, ColorRefToColor(color)));
	}

	//////////////////// Reference //////////////////////
	double RefX = m_pMain->GetMatching(nJob).getRefX(nCam, nPos);
	double RefY = m_pMain->GetMatching(nJob).getRefY(nCam, nPos);

	//if (RefX > 0 || RefY > 0)
	if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseReferenceMark())
	{
		// Reference 마크 표시
		// kbj 2022-01-10 Fixture Mark Color = ORANGE, Referace Mark Color = BULE
		color = COLOR_BLUE;
		m_pMain->draw_mark(&m_pDlgViewerMain[nViewer]->GetViewer(), color, RefX, RefY, nPenThickness, nPenThickness / 2, nPenLength);
	}

	//////////////////// L-check 표시 //////////////////////
	if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getEnableLCheck())
	{
		if (m_pMain->m_bLcheckNG[nJob])
		{
			color = COLOR_RED;
			CString strText;
			int nMethod = m_pMain->vt_job_info[nJob].algo_method;

			switch (nMethod)
			{
			case CLIENT_TYPE_2CAM_EACH_2SHOT_ALIGN:
			case CLIENT_TYPE_4CAM_1SHOT_ALIGN:
			{
				m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), "L-CHECK ERROR", color, 12, 12, 40, H - 600);

				strText.Format("TOP : %.3f BOTTOM : %.3f", m_pMain->m_dbLCheck[nJob][0], m_pMain->m_dbLCheck[nJob][3]);
				m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, 12, 12, 40, H - 500);

				strText.Format("SpecX : %.3f ~ %.3f", m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckSpecX() - m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckTor(),
					m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckSpecX() + m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckTor());
				m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, 12, 12, 40, H - 400);

				strText.Format("LEFT: %.3f RIGHT : %.3f", m_pMain->m_dbLCheck[nJob][1], m_pMain->m_dbLCheck[nJob][2]);
				m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, 12, 12, 40, H - 300);

				strText.Format("SpecY : %.3f ~ %.3f", m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckSpecY() - m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckTor(),
					m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckSpecY() + m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckTor());
				m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, 12, 12, 40, H - 200);
			}
			break;
			default:
			{
				double dist_spec = 0;
				int target_dir = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getAlignmentTargetDir();	// vertical or horizontal
				if (target_dir == 0)	dist_spec = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckSpecX();// horizontal
				else					dist_spec = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckSpecY();// vertical

				strText.Format("Length: %.3f", m_pMain->m_dbLCheck[nJob][0]);
				m_pDlgViewerMain[nViewer]->GetViewer().AddHardGraphic(new GraphicLabel(10, -30, strText, Gdiplus::Color(0xffff0000), Anchor::TopRight));

				strText.Format("Spec : %.3f ~ %.3f", dist_spec - m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckTor(),
					dist_spec + m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckTor());
				m_pDlgViewerMain[nViewer]->GetViewer().AddHardGraphic(new GraphicLabel(10, -50, strText, Gdiplus::Color(0xffff0000), Anchor::TopRight));
			}
			break;
			}
		}
	}

	m_pDlgViewerMain[nViewer]->GetViewer().OnLoadImageFromPtr(m_pMain->getProcBuffer(real_cam, 0));
	m_pDlgViewerMain[nViewer]->GetViewer().Invalidate();
}
void CFormMainView::draw_pcb_insp_result2(int nCam, int nPos, int nViewer, int nJob, BOOL bErase)
{
	auto pDC = m_pDlgViewerMain[nViewer]->GetViewer().getOverlayDC();
	if (bErase)
	{
		m_pDlgViewerMain[nViewer]->GetViewer().ClearOverlayDC();
		m_pDlgViewerMain[nViewer]->GetViewer().clearAllFigures();
	}

	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

	int nWidth = m_pMain->m_stCamInfo[real_cam].w;
	int nHeight = m_pMain->m_stCamInfo[real_cam].h;
	int nViewWidth = m_pDlgViewerMain[nViewer]->GetViewer().GetWidth();
	int nViewHeight = m_pDlgViewerMain[nViewer]->GetViewer().GetHeight();
	int nRate = double(nHeight / nViewHeight) + double(nWidth / nViewWidth) - 1;

	int nPenThickness = CAM_VIEW_PEN_THICKNESS * nRate;
	int nPenLength = CAM_VIEW_PEN_LENGTH * nRate;
	int nFontWidth = CAM_VIEW_FONT_WIDTH * nRate;
	int nFontHeight = CAM_VIEW_FONT_HEIGHT * nRate;

	int dX = 0, dY = 0, dX1 = 0, dY1 = 0;

	int nJobnMethod = m_pMain->vt_job_info[nJob].algo_method;
	int nMethod, nMethodDir = 0;
	nMethod = m_pMain->m_stInsp_DistanceResult[real_cam][nPos].nInspMethodType;
	nMethodDir = m_pMain->m_stInsp_DistanceResult[real_cam][nPos].nInspMethodType;

	////////////////////////////////////////////////////////////////////////
	////////////////////////Draw PCBInspection//////////////////////////////
	CString strText;
	COLORREF color;
	// 판정 여부 확인
	color = COLOR_GREEN;

	/////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////첫번째 위치 그리기
	switch (nMethod)
	{
	case DISTANCE_INSP_METHOD_M_TO_M: // Mark
	case DISTANCE_INSP_METHOD_M_TO_E: // Mark
	{
		// Draw Mark
		if (m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindPattern[FIND_PANEL])
		{
			m_pMain->draw_mark(&m_pDlgViewerMain[nViewer]->GetViewer(), color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].xPos[POINT_PANEL], m_pMain->m_stInsp_DistanceResult[real_cam][nPos].yPos[POINT_PANEL], nPenThickness, nPenThickness / 2, nPenLength);
		}
		else strText.Format("Fail to find Panel Mark");
	}
	break;
	case DISTANCE_INSP_METHOD_L_TO_M: // Line
	case DISTANCE_INSP_METHOD_L_TO_E: // Line
	{
		BOOL bFixtureUse = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseFixtureMark();
		if (bFixtureUse)
		{
			double fixtureX = m_pMain->GetMatching(nJob).getFixtureX(nCam, nPos);
			double fixtureY = m_pMain->GetMatching(nJob).getFixtureY(nCam, nPos);

			//Reference 마크 표시
			color = COLOR_ORANGE;
			m_pMain->draw_mark(&m_pDlgViewerMain[nViewer]->GetViewer(), color, fixtureX, fixtureY, nPenThickness / 2, nPenThickness / 2, nPenLength);

			//찾은 마크 표시
			color = COLOR_GREEN;
			double posX = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetXPos();
			double posY = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetYPos();
			m_pMain->draw_mark(&m_pDlgViewerMain[nViewer]->GetViewer(), color, posX, posY, nPenThickness / 2, nPenThickness / 2, nPenLength);

			//ROI
			color = COLOR_WHITE;
			stFigure tempFig;
			CRect rcROI;
			rcROI = m_pMain->GetMatching(nJob).getSearchROI(nCam, nPos);
			tempFig.ptBegin.x = rcROI.left;
			tempFig.ptBegin.y = rcROI.top;
			tempFig.ptEnd.x = rcROI.right;
			tempFig.ptEnd.y = rcROI.bottom;
			m_pDlgViewerMain[nViewer]->GetViewer().addFigureRect(tempFig, 0.5, 0.5, color);
		}

		color = COLOR_BLUE;
		// Position 1 Edge 
		if (m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindPattern[FIND_PANEL])
		{
			CPoint ptStart, ptEnd;

			if (nMethodDir == TRUE)		//방향이 vertical 일때
			{
				// Line Veritical
				m_pMain->draw_line(&m_pDlgViewerMain[nViewer]->GetViewer(), color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].line[FIND_CALIPER_PANEL_VERT], nHeight, nWidth);

				// 후보군 그리기
				m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][CALIPER_PANEL_VERT].draw_final_result(pDC);

			}
			else //방향이 Horizontal 일때
			{
				// Line Horizontal
				m_pMain->draw_line(&m_pDlgViewerMain[nViewer]->GetViewer(), color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].line[FIND_CALIPER_PANEL_HORI], nHeight, nWidth);

				// 후보군 그리기
				m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][CALIPER_PANEL_HORI].draw_final_result(pDC);
			}
		}
		else strText.Format("Fail to find Panel Line");

		color = COLOR_GREEN;
	}
	break;
	case DISTANCE_INSP_METHOD_E_TO_E: // Edge
	{
		if (m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindPattern[FIND_PANEL])
		{
			// Line Veritical
			m_pMain->draw_line(&m_pDlgViewerMain[nViewer]->GetViewer(), color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].line[FIND_CALIPER_PANEL_VERT], nHeight, nWidth);

			// 후보군 그리기
			m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][CALIPER_PANEL_VERT].draw_final_result(pDC);

			// Line Horizontal
			m_pMain->draw_line(&m_pDlgViewerMain[nViewer]->GetViewer(), color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].line[FIND_CALIPER_PANEL_HORI], nHeight, nWidth);

			// 후보군 그리기
			m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][CALIPER_PANEL_HORI].draw_final_result(pDC);
		}
		else strText.Format("Fail to find Panel Edge");
	}
	break;
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////두번째 위치 그리기
	switch (nMethod)
	{
	case DISTANCE_INSP_METHOD_M_TO_M: // Mark
	case DISTANCE_INSP_METHOD_L_TO_M: // Mark
	{
		// Draw Mark
		if (m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindPattern[POINT_PCB_BOTTOM])
		{
			m_pMain->draw_mark(&m_pDlgViewerMain[nViewer]->GetViewer(), color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].xPos[POINT_PCB_BOTTOM], m_pMain->m_stInsp_DistanceResult[real_cam][nPos].yPos[POINT_PCB_BOTTOM], nPenThickness, nPenThickness / 2, nPenLength);
		}
		else
		{
			if (nJobnMethod == CLIENT_TYPE_PCB_DISTANCE_INSP)	strText.Format("Fail to find PCB Edge");
			else												strText.Format("Fail to find BOTTOM Edge");
		}
	}
	break;
	case DISTANCE_INSP_METHOD_M_TO_E: // Edge
	case DISTANCE_INSP_METHOD_E_TO_E: // Edge
	case DISTANCE_INSP_METHOD_L_TO_E: // Edge
	{
		BOOL bFixtureUse = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseFixtureMark();
		if (bFixtureUse)
		{
			double fixtureX = m_pMain->GetMatching(nJob).getFixtureX(nCam, nPos);
			double fixtureY = m_pMain->GetMatching(nJob).getFixtureY(nCam, nPos);

			//Reference 마크 표시
			color = COLOR_ORANGE;
			m_pMain->draw_mark(&m_pDlgViewerMain[nViewer]->GetViewer(), color, fixtureX, fixtureY, nPenThickness / 2, nPenThickness / 2, nPenLength);

			//찾은 마크 표시
			color = COLOR_GREEN;
			double posX = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetXPos();
			double posY = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetYPos();
			m_pMain->draw_mark(&m_pDlgViewerMain[nViewer]->GetViewer(), color, posX, posY, nPenThickness / 2, nPenThickness / 2, nPenLength);

			//ROI
			color = COLOR_WHITE;
			CRect rcROI = m_pMain->GetMatching(nJob).getSearchROI(nCam, nPos);

			//stFigure tempFig;
			//tempFig.ptBegin.x = rcROI.left;
			//tempFig.ptBegin.y = rcROI.top;
			//tempFig.ptEnd.x = rcROI.right;
			//tempFig.ptEnd.y = rcROI.bottom;
			//m_pDlgViewerMain[nViewer]->GetViewer().addFigureRect(tempFig, 0.5, 0.5, color);

			m_pDlgViewerMain[nViewer]->GetViewer().AddSoftGraphic(new GraphicRectangle(rcROI, Gdiplus::Color(0xff00ff00)));
		}

		color = COLOR_GREEN;

		if (m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindPattern[POINT_PCB_BOTTOM])
		{
			// Line Veritical
			m_pMain->draw_line(&m_pDlgViewerMain[nViewer]->GetViewer(), color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].line[FIND_CALIPER_PCB_VERT], nHeight, nWidth);

			// 후보군 그리기
			m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][CALIPER_PCB_VERT].draw_final_result(pDC);

			// Line Horizontal
			m_pMain->draw_line(&m_pDlgViewerMain[nViewer]->GetViewer(), color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].line[FIND_CALIPER_PCB_HORI], nHeight, nWidth);

			// 후보군 그리기
			m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][CALIPER_PCB_HORI].draw_final_result(pDC);

		}
		else
		{
			if (nJobnMethod == CLIENT_TYPE_PCB_DISTANCE_INSP)	strText.Format("Fail to find PCB Edge");
			else												strText.Format("Fail to find BOTTOM Edge");
		}
	}
	break;
	}

	//교차점 그리기
	color = RGB(255, 125, 0);
	switch (nMethod)
	{
	case DISTANCE_INSP_METHOD_E_TO_E: // Edge 교차점
	{
		if (m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindPattern[FIND_PANEL] && m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindPattern[FIND_PCB_BOTTOM])
		{
			m_pMain->draw_mark(&m_pDlgViewerMain[nViewer]->GetViewer(), color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].xPos[POINT_CROSS_1], m_pMain->m_stInsp_DistanceResult[real_cam][nPos].yPos[POINT_CROSS_1], nPenThickness, nPenThickness / 2, nPenLength);
			m_pMain->draw_mark(&m_pDlgViewerMain[nViewer]->GetViewer(), color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].xPos[POINT_CROSS_2], m_pMain->m_stInsp_DistanceResult[real_cam][nPos].yPos[POINT_CROSS_2], nPenThickness, nPenThickness / 2, nPenLength);
		}
	}
	break;

	case DISTANCE_INSP_METHOD_L_TO_E: // Line 교차점
	{
		if (m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindLine[FIND_CALIPER_PANEL_VERT])
		{
			m_pMain->draw_mark(&m_pDlgViewerMain[nViewer]->GetViewer(), color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].xPos[POINT_CROSS_1], m_pMain->m_stInsp_DistanceResult[real_cam][nPos].yPos[POINT_CROSS_1], nPenThickness, nPenThickness / 2, nPenLength);

		}
		else if (m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindPattern[FIND_CALIPER_PANEL_HORI])
		{
			m_pMain->draw_mark(&m_pDlgViewerMain[nViewer]->GetViewer(), color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].xPos[POINT_CROSS_1], m_pMain->m_stInsp_DistanceResult[real_cam][nPos].yPos[POINT_CROSS_1], nPenThickness, nPenThickness / 2, nPenLength);
		}
	}
	}


	// Distance 검사 포인트 추출
	int nWidthPoint = 0, nHeightPoint = 0;
	if (m_pMain->m_nSeqTotalInspGrabCount == 2)
	{
		if (nCam == 0) {
			switch (nPos)
			{
			case 0:	nWidthPoint = 0;  nHeightPoint = 1;		break;
			case 1:	nWidthPoint = -1; nHeightPoint = 2;		break;
			}
		}
		else {
			switch (nPos)
			{
			case 0:	nWidthPoint = -1; nHeightPoint = 3;		break;
			case 1:	nWidthPoint = 5;  nHeightPoint = 4;		break;
			}
		}
	}
	else if (m_pMain->m_nSeqTotalInspGrabCount == 4)
	{
		if (nCam == 0) {
			switch (nPos)
			{
			case 0:	nWidthPoint = 0;  nHeightPoint = 1;		break;
			case 1:	nWidthPoint = -1; nHeightPoint = 2;		break;
			case 2:	nWidthPoint = -1; nHeightPoint = 3;		break;
			case 3:	nWidthPoint = -1; nHeightPoint = 4;		break;
			}
		}
		else {
			switch (nPos)
			{
			case 0:	nWidthPoint = -1; nHeightPoint = 5;		break;
			case 1:	nWidthPoint = -1; nHeightPoint = 6;		break;
			case 2:	nWidthPoint = -1; nHeightPoint = 7;		break;
			case 3:	nWidthPoint = 9;  nHeightPoint = 8;		break;
			}
		}
	}
	double Spec_distance_LSL_W = 0;
	double Spec_distance_USL_W = 0;
	double Spec_distance_LSL_H = 0;
	double Spec_distance_USL_H = 0;

	if (nWidthPoint >= 0) {
		Spec_distance_LSL_W = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(nWidthPoint);
		Spec_distance_USL_W = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(nWidthPoint);
	}
	Spec_distance_LSL_H = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(nHeightPoint);
	Spec_distance_USL_H = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(nHeightPoint);

	// Text
	const int orisize = 12;
	const int fitsize = 12;

	pDC->SetBkColor(COLOR_WHITE);
	stFigureText tempFigText;

	if (m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindPattern[FIND_PANEL] &&
		m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindPattern[FIND_PCB_BOTTOM]) {

		switch (nMethod)
		{
		case DISTANCE_INSP_METHOD_M_TO_M:
		case DISTANCE_INSP_METHOD_M_TO_E:
		case DISTANCE_INSP_METHOD_E_TO_E:
		{
			if (m_pMain->m_dDistanceInspJudgeResult[nWidthPoint])	color = COLOR_DARK_LIME;
			else													color = COLOR_RED;
			strText.Format("X(P%d) = %.3f( %.3f ~ %.3f )", nWidthPoint + 1,
				m_pMain->m_stInsp_DistanceResult[real_cam][nPos].dWidthDistance,
				Spec_distance_LSL_W,
				Spec_distance_USL_W);
			m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, orisize, fitsize, 40, 40, true);

			if (m_pMain->m_dDistanceInspJudgeResult[nHeightPoint])	color = COLOR_DARK_LIME;
			else													color = COLOR_RED;
			strText.Format("Y(P%d) = %.3f( %.3f ~ %.3f )", nHeightPoint + 1,
				m_pMain->m_stInsp_DistanceResult[real_cam][nPos].dHeightDistance,
				Spec_distance_LSL_H,
				Spec_distance_USL_H);
			m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, orisize, fitsize, 40, 200, true);
		}
		break;
		case DISTANCE_INSP_METHOD_L_TO_M:
		case DISTANCE_INSP_METHOD_L_TO_E:
		{
			if (m_pMain->m_dDistanceInspJudgeResult[nHeightPoint])	color = COLOR_DARK_LIME;
			else													color = COLOR_RED;
			strText.Format("Y(P%d) = %.3f( %.3f ~ %.3f )", nHeightPoint + 1,
				m_pMain->m_stInsp_DistanceResult[real_cam][nPos].dHeightDistance,
				Spec_distance_LSL_H,
				Spec_distance_USL_H);
			m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, orisize, fitsize, 40, 40, true);
		}
		}
	}
	else
	{
		color = COLOR_RED;
		m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, orisize, fitsize, 40, 40, true);
	}

	m_pDlgViewerMain[nViewer]->GetViewer().OnLoadImageFromPtr(m_pMain->getProcBuffer(real_cam, nPos));

	m_pDlgViewerMain[nViewer]->GetViewer().Invalidate();
}
void CFormMainView::draw_film_insp_result(int nCam, int nPos, int nViewer, int nJob, BOOL bErase)
{
	auto pDC = m_pDlgViewerMain[nViewer]->GetViewer().getOverlayDC();
	if (bErase)
	{
		m_pDlgViewerMain[nViewer]->GetViewer().ClearOverlayDC();
		m_pDlgViewerMain[nViewer]->GetViewer().clearAllFigures();
	}

	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

	int nWidth = m_pMain->m_stCamInfo[real_cam].w;
	int nHeight = m_pMain->m_stCamInfo[real_cam].h;
	int nViewWidth = m_pDlgViewerMain[nViewer]->GetViewer().GetWidth();
	int nViewHeight = m_pDlgViewerMain[nViewer]->GetViewer().GetHeight();
	int nRate = double(nHeight / nViewHeight) + double(nWidth / nViewWidth) - 1;

	int nPenThickness = CAM_VIEW_PEN_THICKNESS * nRate;
	int nPenLength = CAM_VIEW_PEN_LENGTH * nRate;
	int nFontWidth = CAM_VIEW_FONT_WIDTH * nRate;
	int nFontHeight = CAM_VIEW_FONT_HEIGHT * nRate;
	const int orisize = 12;
	const int fitsize = 12;
	////////////////////////////////////////////////////////////////////////
	////////////////////////Draw Film Inspection//////////////////////////////
	// 아래 다시 그려 줘야 함 임시로 표시
	CString strText = "Result";
	COLORREF color = COLOR_WHITE;

	//ROI
	CRect rcROI = m_pMain->GetMatching(nJob).getSearchROI(nCam, nPos);

	//stFigure tempFig;
	//tempFig.ptBegin.x = rcROI.left;
	//tempFig.ptBegin.y = rcROI.top;
	//tempFig.ptEnd.x = rcROI.right;
	//tempFig.ptEnd.y = rcROI.bottom;
	//m_pDlgViewerMain[nViewer]->GetViewer().addFigureRect(tempFig, 0.5, 0.5, color);

	m_pDlgViewerMain[nViewer]->GetViewer().AddSoftGraphic(new GraphicRectangle(rcROI, 0xff00ff00));
	// 판정 여부 확인
	color = COLOR_GREEN;
	m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, orisize, fitsize, 40, 40, true);

	m_pDlgViewerMain[nViewer]->GetViewer().SoftDirtyRefresh();
}
void CFormMainView::draw_scan_insp_result(int nCam, int nPos, int nViewer, int nJob, BOOL bErase)
{
	nViewer = nViewer + (m_pMain->m_nSeqScanInspGrabCount[nJob] - 1);

	int real_cam = m_pMain->vt_job_info[nJob].camera_index[0];
	int nWidth = m_pMain->m_stCamInfo[real_cam].w;
	int nHeight = m_pMain->m_stCamInfo[real_cam].h;
	int nViewWidth = m_pDlgViewerMain[nViewer]->GetViewer().GetWidth();
	int nViewHeight = m_pDlgViewerMain[nViewer]->GetViewer().GetHeight();

	double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(real_cam, 0);
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(real_cam, 0);

	cv::Mat srcPtr = cv::Mat::zeros(nViewWidth, nViewHeight, CV_8UC1);

	CString strText;
	CViewerEx* pView = &m_pDlgViewerMain[nViewer]->GetViewer();
	auto pDC = pView->getOverlayDC();
	CPen pen(PS_SOLID, 3, COLOR_REALGREEN);
	CPen penOK(PS_SOLID, 3, RGB(255, 255, 0));
	CFont font, * oldFont, resultfont;
	COLORREF color = COLOR_RED;
	COLORREF color_BLUE = COLOR_BLUE;
	stFigure tempFig, tempFig_Insp;
	int OLD_BKMODE = 0;

	int CamGrabCount = m_pMain->m_nSeqScanInspGrabCount[nJob] == 1 ? _SCAN_GRAB_MAXCOUNT : 1;
	int SeqGrabCount = m_pMain->m_nSeqScanInspGrabCount[nJob];

	font.CreateFont(40, 40, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
		OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");

	resultfont.CreateFont(20, 20, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
		OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");

	if (pView->GetSafeHwnd() == NULL) return;

	oldFont = pDC->SelectObject(&font);

	pView->ClearOverlayDC();
	pView->clearAllFigures();

	pDC->SetTextColor(COLOR_RED);
	OLD_BKMODE = pDC->SetBkMode(TRANSPARENT);
	pDC->SelectObject(&pen);
	pDC->SelectObject(GetStockObject(NULL_BRUSH));

	double xa, ya;
	double xa2, ya2;
	double ya3;

	double ax = m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_ResultLine[nPos].a;
	double bc = m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_ResultLine[nPos].b;
	double x1 = m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vELB_SearchLine.x;
	double x2 = m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vELB_SearchLine.y;
	CRect roi = m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_InspRoi[nPos];

	double ax2 = m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_ResultLine_Insp[nPos].a;
	double bc2 = m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_ResultLine_Insp[nPos].b;
	double x3 = m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vELB_SearchLine_Insp.x;
	double x4 = m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vELB_SearchLine_Insp.y;

	//KJH 2021-11-08 UT INSP 조건 추가
	BOOL method_select = m_pMain->vt_job_info[0].model_info.getAlignInfo().getUseSubInspAlgorithm();

	//KJH 2021-07-27 검사 파라미터로 뺄거 미리 작업하자
	int LeftResultTextGap = 150;
	int RightResultTextGap = 30;
	int StartRangeJudgeValue = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getStartSerchPoint(SeqGrabCount - 1);
	int StartSearchValueOffset = 20;
	int Limit_Line_Insp_Offset = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getLimitLineInspOffset(SeqGrabCount - 1);

	CPen penRight(PS_SOLID, 3, COLOR_GREEN);
	CPen penLeft(PS_SOLID, 3, RGB(255, 0, 255));
	CPen penData(PS_SOLID, 3, COLOR_YELLOW);

	if (m_pMain->m_pSaperaCam[nCam].IsOpend())
	{
		int xw = nWidth;
		int yh = nHeight * (SeqGrabCount == 1 ? _SCAN_GRAB_MAXCOUNT : 1);

		BYTE* data = SeqGrabCount == 1 ? m_pMain->m_pSaperaCam[nCam].GetFullImage()->data : m_pMain->getProcBuffer(real_cam, 0);

		cv::Mat matImage_View(yh, xw, CV_8UC1, data);
		if (SeqGrabCount == 1)
		{
			//모션 변경으로 인한 이미지 변환
			rot90(matImage_View, 2); //1. 반시계 방향 후 상하 반전
			flip(matImage_View, matImage_View, 0);
		}
		else
		{
			rot90(matImage_View, 2); //반시계 방향 회전
		}
		memcpy(srcPtr.data, matImage_View.data, nViewWidth * nViewHeight);
	}

	//Long 이미지(1번 이미지 그리기))
	if (SeqGrabCount == 1)
	{
		//캘리퍼 그리기
		tempFig.ptBegin.x = int(x1 * ax + bc);
		tempFig.ptBegin.y = int(x1);
		tempFig.ptEnd.x = int(x2 * ax + bc);
		tempFig.ptEnd.y = int(x2);

		if (method_select)
		{
			//캘리퍼 그리기
			tempFig_Insp.ptBegin.x = int(x3 * ax2 + bc2);
			tempFig_Insp.ptBegin.y = int(x3);
			tempFig_Insp.ptEnd.x = int(x4 * ax2 + bc2);
			tempFig_Insp.ptEnd.y = int(x4);

			pView->addFigureLine(tempFig_Insp, 2, 1, color_BLUE);
		}

		pView->addFigureLine(tempFig, 2, 1, color);
		//cv::line(m_pMain->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(tempFig.ptBegin.x), int(tempFig.ptBegin.y)),cv::Point(int(tempFig.ptEnd.x),int(tempFig.ptEnd.y)),cv::Scalar(0,0,255),3);

		//캘리퍼 시작점 원형표시
		pDC->Ellipse((tempFig.ptBegin.x - 4), int(tempFig.ptBegin.y - 4), (tempFig.ptBegin.x + 4), int(tempFig.ptBegin.y + 4));
		//cv::circle(m_pMain->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(tempFig.ptBegin.x), int(tempFig.ptBegin.y)), 2, cv::Scalar(0, 255, 255), 2, 5);

		//캘리퍼 종료점 원형표시
		pDC->Ellipse((tempFig.ptEnd.x - 4), int(tempFig.ptEnd.y - 4), (tempFig.ptEnd.x + 4), int(tempFig.ptEnd.y + 4));
		//cv::circle(m_pMain->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(tempFig.ptEnd.x), int(tempFig.ptEnd.y)), 2, cv::Scalar(0, 255, 255), 2, 5);

		//Roi 디스플레이

		pDC->Rectangle(roi.top, roi.left, roi.bottom, roi.right);
		//cv::rectangle(m_pMain->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(roi.top, roi.left), cv::Point(roi.bottom, roi.right), CV_RGB(0, 255, 0), 5);

		pDC->SelectObject(&penOK);

		//KJH 2021-07-28 이미지 방향 맞추기
		/*if (m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].size() > 0)
			pDC->MoveTo(nViewWidth - m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].at(0), m_pMain->m_ELB_DiffInspResult[nCam].m_vTraceProfile[1].at(0));*/
		if (m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].size() > 0)
			pDC->MoveTo(m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].at(0), m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[1].at(0));
		int nsize = int(MIN(m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].size(), m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[1].size()));

		//pDC->TextOutA(100, 100, strText);
		//cv::putText(m_pMain->m_matResultImage, std::string(strText), cv::Point(100, 100), cv::FONT_ITALIC, 2, cv::Scalar(0, 0, 255), 5);

		//In Result Overlay Display Start
		// 
		//In 방향 Display 설정
		pDC->SelectObject(&penRight);			//RGB( 64,255, 64)
		pDC->SetTextColor(RGB(255, 18, 18));

		for (int i = 1; i < nsize; i++)
		{
			ya = m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[1].at(i);
			xa = m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].at(i);
			ya3 = m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[5].at(i);

			if (i > 1)
			{
				ya2 = m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[1].at(i - 1);
				xa2 = m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].at(i - 1);

			}
			else
			{
				ya2 = ya;
				xa2 = xa;
			}

			//KJH 2021-07-28 이미지 방향 맞추기
			//KJH 2021-07-28 In,Out Display 수정
			if (xa < m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x || xa > m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y)
			{
				pDC->MoveTo(xa, ya);
			}
			else
			{
				if (method_select)
				{
					if (ya > ya3 + Limit_Line_Insp_Offset)
					{
						pDC->SelectObject(&penLeft);
					}
					else
					{
						pDC->SelectObject(&penRight);
					}
				}
				pDC->LineTo(xa, ya);
				//cv::line(m_pMain->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(xa2), int(ya2)), cv::Point(int(xa), int(ya)), cv::Scalar(64, 255, 64), 3);

				//if (!pView->IsFitImage() && i % 200 == 0)
				//{
				//	if ((m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_FindPoint.x < i && m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_FindPoint.x != -1) ||
				//		(m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_FindPoint.y != -1 && m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_FindPoint.y > i))
				//	{
				//		strText.Format("[IN] %.4f mm", m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[2].at(i));
				//		pDC->TextOutA(xa, ya, strText);
				//	}
				//}
			}

		}
		//In Result Overlay Display End

		//KJH 2021-07-28 이미지 방향 맞추기
		/*if (m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].size() > 0)
			pDC->MoveTo(nViewWidth - m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].at(0), m_pMain->m_ELB_DiffInspResult[nCam].m_vTraceProfile[3].at(0));*/
		if (m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].size() > 0)
			pDC->MoveTo(m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].at(0), m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[3].at(0));
		nsize = int(MIN(m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].size(), m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[3].size()));

		//Out Result Overlay Display Start
		//보라색(OUT) 외각선 표시
		pDC->SelectObject(&penLeft);
		pDC->SetTextColor(RGB(255, 18, 18));
		for (int i = 1; i < nsize; i++)
		{
			ya = m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[3].at(i);
			xa = m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].at(i);

			if (i > 1)
			{
				ya2 = m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[3].at(i - 1);
				xa2 = m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].at(i - 1);

			}
			else
			{
				ya2 = ya;
				xa2 = xa;
			}

			//KJH 2021-07-27	Start 찾은점과 End 찾은점으로 영역 변경
			if (xa < m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x || xa > m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y)
			{
				pDC->MoveTo(xa, ya);
			}
			else
			{
				pDC->LineTo(xa, ya);
				//cv::line(m_pMain->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(xa2), int(ya2)), cv::Point(int(xa), int(ya)), cv::Scalar(255, 0, 255), 3);

				//파란색(L) 왼쪽선 표시
				//왼쪽 Data 표시
				//if (!pView->IsFitImage() && i % 200 == 0)
				//{
				//	if ((m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_FindPoint.x < i && m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_FindPoint.x != -1) ||
				//		(m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_FindPoint.y != -1 && m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_FindPoint.y > i))
				//	{
				//		strText.Format("[OUT] %.4f mm", m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[4].at(i));
				//		pDC->TextOutA(xa, ya, strText);
				//	}
				//}
			}
		}
		//Out Result Overlay Display End

		pDC->SelectObject(&penOK);

		pDC->MoveTo(m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x, 0);
		pDC->LineTo(m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x, nHeight);
		//cv::line(m_pMain->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x), int(0)), cv::Point(int(m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x), int(nHeight)), cv::Scalar(0, 255, 255), 3);

		pDC->MoveTo(m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y, 0);
		pDC->LineTo(m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y, nHeight);
		//cv::line(m_pMain->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y), int(0)), cv::Point(int(m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y), int(nHeight)), cv::Scalar(0, 255, 255), 3);
	}
	else
	{
		//캘리퍼 그리기
		tempFig.ptBegin.x = int(x1 * ax + bc); // 반전 사각형
		tempFig.ptBegin.y = int(nWidth - x1);
		tempFig.ptEnd.x = int(x2 * ax + bc);
		tempFig.ptEnd.y = int(nWidth - x2);
		pView->addFigureLine(tempFig, 2, 1, color);
		//cv::line(m_pMain->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(tempFig.ptBegin.x), int(tempFig.ptBegin.y)), cv::Point(int(tempFig.ptEnd.x), int(tempFig.ptEnd.y)), cv::Scalar(0, 0, 255), 3);

		//캘리퍼 시작점 원형표시
		pDC->Ellipse((tempFig.ptBegin.x - 4), int(tempFig.ptBegin.y - 4), (tempFig.ptBegin.x + 4), int(tempFig.ptBegin.y + 4));
		//cv::circle(m_pMain->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(tempFig.ptBegin.x), int(tempFig.ptBegin.y)), 2, cv::Scalar(0, 255, 255), 2, 5);

		//캘리퍼 종료점 원형표시
		pDC->Ellipse((tempFig.ptEnd.x - 4), int(tempFig.ptEnd.y - 4), (tempFig.ptEnd.x + 4), int(tempFig.ptEnd.y + 4));
		//cv::circle(m_pMain->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(tempFig.ptEnd.x), int(tempFig.ptEnd.y)), 2, cv::Scalar(0, 255, 255), 2, 5);

		pDC->Rectangle(roi.top, nWidth - roi.right, roi.bottom, nWidth - roi.left);
		//cv::rectangle(m_pMain->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(roi.top, nWidth - roi.right), cv::Point(roi.bottom, nWidth - roi.left), CV_RGB(0, 255, 0), 3);

		pDC->SelectObject(&penOK);
		if (m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].size() > 0)
			pDC->MoveTo(m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].at(0), nWidth - m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[1].at(0));

		int nsize = int(MIN(m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].size(), m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[1].size()));

		//In 방향 Display 설정
		pDC->SelectObject(&penRight);			//RGB( 64,255, 64)
		pDC->SetTextColor(RGB(255, 18, 18));

		for (int i = 1; i < nsize; i++)
		{
			ya = m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[1].at(i);
			xa = m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].at(i);

			if (i > 1)
			{
				ya2 = m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[1].at(i - 1);
				xa2 = m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].at(i - 1);

			}
			else
			{
				ya2 = ya;
				xa2 = xa;
			}

			//KJH 2021-07-27	Start 찾은점과 End 찾은점으로 영역 변경
			if (xa < m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x || xa > m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y)
			{
				pDC->MoveTo(xa, ya);
			}
			else
			{
				pDC->LineTo(xa, nWidth - ya);
				//cv::line(m_pMain->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(xa2), int(nWidth - ya2)), cv::Point(int(xa), int(nWidth - ya)), cv::Scalar(64, 255, 64), 3);
			}

			//if (!pView->IsFitImage() && i % 200 == 0)
			//{
			//	strText.Format("[%.4f, %.4f] %.4f mm", xa, nWidth - ya, m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[2].at(i));
			//	pDC->TextOutA(xa, ya, strText);
			//}
		}

		if (m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].size() > 0)
			pDC->MoveTo(m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].at(0), nWidth - m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[3].at(0));

		nsize = int(MIN(m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].size(), m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[3].size()));

		//Out Result Overlay Display Start
		//보라색(OUT) 외각선 표시
		pDC->SelectObject(&penLeft);		//RGB(255, 0, 255)
		pDC->SetTextColor(RGB(255, 18, 18));

		for (int i = 1; i < nsize; i++)
		{
			ya = m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[3].at(i);
			xa = m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].at(i);

			if (i > 1)
			{
				ya2 = m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[3].at(i - 1);
				xa2 = m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].at(i - 1);

			}
			else
			{
				ya2 = ya;
				xa2 = xa;
			}

			//KJH 2021-07-27	Start 찾은점과 End 찾은점으로 영역 변경
			if (xa < m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x || xa > m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y)
			{
				pDC->MoveTo(xa, ya);
			}
			else
			{
				pDC->LineTo(xa, nWidth - ya);
				//cv::line(m_pMain->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(xa2), int(nWidth - ya2)), cv::Point(int(xa), int(nWidth - ya)), cv::Scalar(255, 0, 255), 3);

				//파란색(L) 왼쪽선 표시
				//왼쪽 Data 표시
				//if (!pView->IsFitImage() && i % 200 == 0)
				//{
				//	if ((m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_FindPoint.x < i && m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_FindPoint.x != -1) ||
				//		(m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_FindPoint.y != -1 && m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_FindPoint.y > i))
				//	{
				//		strText.Format("[OUT] %.4f mm", m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[4].at(i));
				//		pDC->TextOutA(xa, ya, strText);
				//	}
				//}
			}
		}

		pDC->SelectObject(&penOK);

		pDC->MoveTo(m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x, 0);
		pDC->LineTo(m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x, nHeight);
		//cv::line(m_pMain->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x), int(0)), cv::Point(int(m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x), int(nHeight)), cv::Scalar(0, 255, 255), 3);

		pDC->MoveTo(m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y, 0);
		pDC->LineTo(m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y, nHeight);
		//cv::line(m_pMain->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y), int(0)), cv::Point(int(m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y), int(nHeight)), cv::Scalar(0, 255, 255), 3);
	}

	double distance = (m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y - m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x) * yres;

	double	MinDistance_in	= m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vELB_InMinMaxValue.x;
	double	MaxDistance_in	= m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vELB_InMinMaxValue.y;
	double	MinDistance_out	= m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vELB_OutMinMaxValue.x;
	double	MaxDistance_out	= m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vELB_OutMinMaxValue.y;
	double	MinDistance		= m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vELB_TotalMinMaxValue.x;
	double	MaxDistance		= m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vELB_TotalMinMaxValue.y;

	pDC->SelectObject(&resultfont);

	//pDC->TextOutA(100, 100, strText);
	//cv::putText(m_pMain->m_matResultImage, std::string(strText), cv::Point(100, 100), cv::FONT_ITALIC, 2, cv::Scalar(0, 0, 255), 5);

	pDC->SetTextColor(COLOR_RED);			//RGB(255, 64, 64)
	strText.Format("[S: %.4f , E: %.4f] Distance = %.4f mm", m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x * yres, m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y * yres, distance);
	pDC->TextOutA(50, 20, strText);
	//cv::putText(m_pMain->m_matScanResultImage[nCam][SeqGrabCount], std::string(strText), cv::Point(20, 50), cv::FONT_ITALIC, 1, cv::Scalar(64, 64, 255), 3);

	pDC->SetTextColor(COLOR_GREEN);			//RGB( 64,255, 64)
	strText.Format("[Min(in): %.4f , Max(in): %.4f]mm", MinDistance_in, MaxDistance_in);
	pDC->TextOutA(50, 40, strText);
	//cv::putText(m_pMain->m_matScanResultImage[nCam][SeqGrabCount], std::string(strText), cv::Point(20, 100), cv::FONT_ITALIC, 1, cv::Scalar(64, 255, 64), 3);

	pDC->SetTextColor(RGB(255, 0, 255));
	strText.Format("[Min(out): %.4f , Max(out): %.4f]mm", MinDistance_out, MaxDistance_out);
	pDC->TextOutA(50, 60, strText);
	//cv::putText(m_pMain->m_matScanResultImage[nCam][SeqGrabCount], std::string(strText), cv::Point(20, 150), cv::FONT_ITALIC, 1, cv::Scalar(255, 0, 255), 3);

	pDC->SetTextColor(COLOR_YELLOW);			//RGB(255,255, 64)
	strText.Format("[Min(total): %.4f , Max(total): %.4f]mm", MinDistance, MaxDistance);
	pDC->TextOutA(50, 80, strText);
	//cv::putText(m_pMain->m_matScanResultImage[nCam][SeqGrabCount], std::string(strText), cv::Point(20, 200), cv::FONT_ITALIC, 1, cv::Scalar(64, 255, 255), 3);

	pDC->SetTextColor(COLOR_RED);
	strText.Format("[In_Center : %.4f , Out_Center : %.4f]mm", m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_Result_Center_XWidth_in.y, m_pMain->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_Result_Center_XWidth_out.y);
	pDC->TextOutA(50, 100, strText);
	//cv::putText(m_pMain->m_matScanResultImage[nCam][SeqGrabCount], std::string(strText), cv::Point(20, 250), cv::FONT_ITALIC, 1, cv::Scalar(64, 64, 255), 3);

	//HSJ Scan Inspection 길이 멤버변수 추가
	m_InspectionDistance[nCam] = distance;

	pDC->SelectObject(oldFont);
	font.DeleteObject();
	penOK.DeleteObject();
	pen.DeleteObject();
	penLeft.DeleteObject();
	penRight.DeleteObject();
	penData.DeleteObject();

	//실제 영상 띄우기
	m_pDlgViewerMain[nViewer]->GetViewer().OnLoadImageFromPtr(srcPtr.data);
	m_pDlgViewerMain[nViewer]->GetViewer().Invalidate();

	//CString strfile;
	//strfile.Format("D:\\m_matScanResultImage_overlay_%d_%d.jpg", nCam, SeqGrabCount);
	//imwrite(std::string(strfile), m_pMain->m_matScanResultImage[nCam][SeqGrabCount]);
}
void CFormMainView::draw_notch_line(int nCam, CViewerEx* pViewer, int nJob, BOOL bErase)
{
	auto pDC = pViewer->getOverlayDC();
	if (bErase)
	{
		pViewer->ClearOverlayDC();
		pViewer->clearAllFigures();
	}
	// 보류
	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
	int w = m_pMain->m_stCamInfo[real_cam].w;
	int h = m_pMain->m_stCamInfo[real_cam].h;

	CPen penOK(PS_SOLID, 3, COLOR_LIME);
	CPen* oldPen;
	oldPen = pDC->SelectObject(&penOK);
	int pos_x, pos_y;


	pos_x = m_pMain->notch_line_data.pt_up.x;
	pos_y = m_pMain->notch_line_data.pt_up.y;
	pDC->MoveTo(0, pos_y);
	pDC->LineTo(w, pos_y);

	pos_x = m_pMain->notch_line_data.pt_down.x;
	pos_y = m_pMain->notch_line_data.pt_down.y;
	pDC->MoveTo(0, pos_y);
	pDC->LineTo(w, pos_y);

	pos_x = m_pMain->notch_line_data.pt_mid.x;
	pos_y = m_pMain->notch_line_data.pt_mid.y;

	//CString str_temp;
	//str_temp.Format("%.1f %.1f", pos_x, pos_y);
	//AfxMessageBox(str_temp);

	pDC->MoveTo(pos_x - 10, pos_y);
	pDC->LineTo(pos_x + 10, pos_y);
	pDC->MoveTo(pos_x, pos_y - 10);
	pDC->LineTo(pos_x, pos_y + 10);

	pDC->SelectObject(oldPen);

	pViewer->Invalidate();
}
void CFormMainView::draw_assemble_insp_result2(int nCam, int nPos, int nViewer, int nJob, BOOL bErase)
{
	auto pDC = m_pDlgViewerMain[nViewer]->GetViewer().getOverlayDC();
	if (bErase)
	{
		m_pDlgViewerMain[nViewer]->GetViewer().ClearOverlayDC();
		//m_pDlgViewerMain[nViewer]->GetViewer().clearAllFigures();
	}
	// 보류
	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

	int nWidth = m_pMain->m_stCamInfo[real_cam].w;
	int nHeight = m_pMain->m_stCamInfo[real_cam].h;
	int nViewWidth = m_pDlgViewerMain[nViewer]->GetViewer().GetWidth();
	int nViewHeight = m_pDlgViewerMain[nViewer]->GetViewer().GetHeight();
	int nRate = double(nHeight / nViewHeight) + double(nWidth / nViewWidth) - 1;

	int nPenThickness = CAM_VIEW_PEN_THICKNESS * nRate;
	int nPenLength = CAM_VIEW_PEN_LENGTH * nRate;
	int nFontWidth = CAM_VIEW_FONT_WIDTH * nRate;
	int nFontHeight = CAM_VIEW_FONT_HEIGHT * nRate;

	int dX = 0, dY = 0, dX1 = 0, dY1 = 0;

	int nJobnMethod = m_pMain->vt_job_info[nJob].algo_method;
	int nMethod, nMethodDir = 0;
	nMethod = m_pMain->m_stInsp_DistanceResult[real_cam][nPos].nInspMethodType;
	nMethodDir = m_pMain->m_stInsp_DistanceResult[real_cam][nPos].nInspMethodType;

	////////////////////////////////////////////////////////////////////////
	////////////////////////Draw PCBInspection//////////////////////////////
	CString strText;
	COLORREF color;

	// 라인의 경우 초록색.
	color = COLOR_GREEN;
	/////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////첫번째 위치 그리기
	switch (nMethod)
	{
	case DISTANCE_INSP_METHOD_M_TO_M: // Mark
	case DISTANCE_INSP_METHOD_M_TO_E: // Mark
	{
		// Draw Mark
		if (m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindPattern[FIND_PANEL])
		{
			m_pMain->draw_mark(&m_pDlgViewerMain[nViewer]->GetViewer(), color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].xPos[POINT_PANEL], m_pMain->m_stInsp_DistanceResult[real_cam][nPos].yPos[POINT_PANEL], nPenThickness, nPenThickness / 2, nPenLength);
		}
		else strText.Format("Fail to find Panel Mark");
	}
	break;
	case DISTANCE_INSP_METHOD_L_TO_M: // Line
	case DISTANCE_INSP_METHOD_L_TO_E: // Line
	{
		BOOL bFixtureUse = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseFixtureMark();
		if (bFixtureUse)
		{
			double fixtureX = m_pMain->GetMatching(nJob).getFixtureX(nCam, nPos);
			double fixtureY = m_pMain->GetMatching(nJob).getFixtureY(nCam, nPos);

			//Reference 마크 표시
			color = COLOR_ORANGE;
			m_pMain->draw_mark(&m_pDlgViewerMain[nViewer]->GetViewer(), color, fixtureX, fixtureY, nPenThickness / 2, nPenThickness / 2, nPenLength);

			//찾은 마크 표시
			color = COLOR_GREEN;
			double posX = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetXPos();
			double posY = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetYPos();
			m_pMain->draw_mark(&m_pDlgViewerMain[nViewer]->GetViewer(), color, posX, posY, nPenThickness / 2, nPenThickness / 2, nPenLength);

			//ROI
			color = COLOR_WHITE;
			CRect rcROI = m_pMain->GetMatching(nJob).getSearchROI(nCam, nPos);

			//stFigure tempFig;
			//tempFig.ptBegin.x = rcROI.left;
			//tempFig.ptBegin.y = rcROI.top;
			//tempFig.ptEnd.x = rcROI.right;
			//tempFig.ptEnd.y = rcROI.bottom;
			//m_pDlgViewerMain[nViewer]->GetViewer().addFigureRect(tempFig, 0.5, 0.5, color);
			m_pDlgViewerMain[nViewer]->GetViewer().AddSoftGraphic(new GraphicRectangle(rcROI, Gdiplus::Color(color | 0xff000000)));
		}

		color = COLOR_BLUE;

		// Position 1 Edge 
		if (m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindPattern[FIND_PANEL])
		{
			CPoint ptStart, ptEnd;

			if (nMethodDir == TRUE)		//방향이 vertical 일때
			{
				// Line Veritical
				m_pMain->draw_line(&m_pDlgViewerMain[nViewer]->GetViewer(), color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].line[FIND_CALIPER_PANEL_VERT], nHeight, nWidth);

				// 후보군 그리기
				m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][CALIPER_PANEL_VERT].draw_final_result(pDC);

			}
			else //방향이 Horizontal 일때
			{
				// Line Horizontal
				m_pMain->draw_line(&m_pDlgViewerMain[nViewer]->GetViewer(), color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].line[FIND_CALIPER_PANEL_HORI], nHeight, nWidth);

				// 후보군 그리기
				m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][CALIPER_PANEL_HORI].draw_final_result(pDC);
			}
		}
		else strText.Format("Fail to find Panel Line");

		color = COLOR_GREEN;
	}
	break;
	case DISTANCE_INSP_METHOD_E_TO_E: // Edge
	{
		if (m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindPattern[FIND_PANEL])
		{
			// Line Veritical
			m_pMain->draw_line(&m_pDlgViewerMain[nViewer]->GetViewer(), color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].line[FIND_CALIPER_PANEL_VERT], nHeight, nWidth);

			// 후보군 그리기
			m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][CALIPER_PANEL_VERT].draw_final_result(pDC);

			// Line Horizontal
			m_pMain->draw_line(&m_pDlgViewerMain[nViewer]->GetViewer(), color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].line[FIND_CALIPER_PANEL_HORI], nHeight, nWidth);

			// 후보군 그리기
			m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][CALIPER_PANEL_HORI].draw_final_result(pDC);
		}
		else strText.Format("Fail to find Panel Edge");
	}
	break;
	}

	/////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////두번째 위치 그리기


	if (m_pMain->vt_job_info[nJob].model_info.getGlassInfo().getModelPanelType() == _CORE_MODEL_CALC)
	{
		/////////////////////////////////////////////////////////////////////////////////////
		//////////////////////////계산된 위치 그리기
		if (m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindPattern[FIND_PCB_BOTTOM] == TRUE)
		{
			// 돌출부분 찾은데 표시
			m_pMain->draw_mark(&m_pDlgViewerMain[nViewer]->GetViewer(), color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].xPos[POINT_PROTRUDING], m_pMain->m_stInsp_DistanceResult[real_cam][nPos].yPos[POINT_PCB_BOTTOM], nPenThickness, nPenThickness / 2, nPenLength);

			// 계산된 PCB 위치 표시
			m_pMain->draw_mark(&m_pDlgViewerMain[nViewer]->GetViewer(), color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].xPos[POINT_PCB_BOTTOM], m_pMain->m_stInsp_DistanceResult[real_cam][nPos].yPos[POINT_PCB_BOTTOM], nPenThickness, nPenThickness / 2, nPenLength);
		}
		else
		{
			strText.Format("Fail to find Data");
		}
	}
	else
	{
		switch (nMethod)
		{
		case DISTANCE_INSP_METHOD_M_TO_M: // Mark
		case DISTANCE_INSP_METHOD_L_TO_M: // Mark
		{
			// Draw Mark
			if (m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindPattern[POINT_PCB_BOTTOM])
			{
				m_pMain->draw_mark(&m_pDlgViewerMain[nViewer]->GetViewer(), color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].xPos[POINT_PCB_BOTTOM], m_pMain->m_stInsp_DistanceResult[real_cam][nPos].yPos[POINT_PCB_BOTTOM], nPenThickness, nPenThickness / 2, nPenLength);
			}
			else
			{
				if (nJobnMethod == CLIENT_TYPE_PCB_DISTANCE_INSP)	strText.Format("Fail to find PCB Edge");
				else												strText.Format("Fail to find BOTTOM Edge");
			}
		}
		break;
		case DISTANCE_INSP_METHOD_M_TO_E: // Edge
		case DISTANCE_INSP_METHOD_E_TO_E: // Edge
		case DISTANCE_INSP_METHOD_L_TO_E: // Edge
		{
			BOOL bFixtureUse = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseFixtureMark();
			if (bFixtureUse)
			{
				double fixtureX = m_pMain->GetMatching(nJob).getFixtureX(nCam, nPos);
				double fixtureY = m_pMain->GetMatching(nJob).getFixtureY(nCam, nPos);

				//Reference 마크 표시
				color = COLOR_ORANGE;
				m_pMain->draw_mark(&m_pDlgViewerMain[nViewer]->GetViewer(), color, fixtureX, fixtureY, nPenThickness / 2, nPenThickness / 2, nPenLength);

				//찾은 마크 표시
				color = COLOR_GREEN;
				double posX = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetXPos();
				double posY = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetYPos();
				m_pMain->draw_mark(&m_pDlgViewerMain[nViewer]->GetViewer(), color, posX, posY, nPenThickness / 2, nPenThickness / 2, nPenLength);

				//ROI
				color = COLOR_WHITE;
				CRect rcROI = m_pMain->GetMatching(nJob).getSearchROI(nCam, nPos);

				//stFigure tempFig;
				//tempFig.ptBegin.x = rcROI.left;
				//tempFig.ptBegin.y = rcROI.top;
				//tempFig.ptEnd.x = rcROI.right;
				//tempFig.ptEnd.y = rcROI.bottom;
				//m_pDlgViewerMain[nViewer]->GetViewer().addFigureRect(tempFig, 0.5, 0.5, color);
				m_pDlgViewerMain[nViewer]->GetViewer().AddSoftGraphic(new GraphicRectangle(rcROI, color | 0xff000000));
			}

			color = COLOR_BLUE;

			if (m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindPattern[POINT_PCB_BOTTOM])
			{
				// Line Veritical
				m_pMain->draw_line(&m_pDlgViewerMain[nViewer]->GetViewer(), color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].line[FIND_CALIPER_PCB_VERT], nHeight, nWidth);

				// 후보군 그리기
				m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][CALIPER_PCB_VERT].draw_final_result(pDC);

				// Line Horizontal
				m_pMain->draw_line(&m_pDlgViewerMain[nViewer]->GetViewer(), color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].line[FIND_CALIPER_PCB_HORI], nHeight, nWidth);

				// 후보군 그리기
				m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][CALIPER_PCB_HORI].draw_final_result(pDC);

			}
			else
			{
				if (nJobnMethod == CLIENT_TYPE_PCB_DISTANCE_INSP)	strText.Format("Fail to find PCB Edge");
				else												strText.Format("Fail to find BOTTOM Edge");
			}
		}
		break;
		}

		//교차점 그리기
		color = RGB(255, 125, 0);
		switch (nMethod)
		{
		case DISTANCE_INSP_METHOD_E_TO_E: // Edge 교차점
		{
			if (m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindPattern[FIND_PANEL] && m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindPattern[FIND_PCB_BOTTOM])
			{
				m_pMain->draw_mark(&m_pDlgViewerMain[nViewer]->GetViewer(), color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].xPos[POINT_CROSS_1], m_pMain->m_stInsp_DistanceResult[real_cam][nPos].yPos[POINT_CROSS_1], nPenThickness, nPenThickness / 2, nPenLength);
				m_pMain->draw_mark(&m_pDlgViewerMain[nViewer]->GetViewer(), color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].xPos[POINT_CROSS_2], m_pMain->m_stInsp_DistanceResult[real_cam][nPos].yPos[POINT_CROSS_2], nPenThickness, nPenThickness / 2, nPenLength);
			}
		}
		break;

		case DISTANCE_INSP_METHOD_L_TO_E: // Line 교차점
		{
			if (m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindLine[FIND_CALIPER_PANEL_VERT])
			{
				m_pMain->draw_mark(&m_pDlgViewerMain[nViewer]->GetViewer(), color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].xPos[POINT_CROSS_1], m_pMain->m_stInsp_DistanceResult[real_cam][nPos].yPos[POINT_CROSS_1], nPenThickness, nPenThickness / 2, nPenLength);

			}
			else if (m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindPattern[FIND_CALIPER_PANEL_HORI])
			{
				m_pMain->draw_mark(&m_pDlgViewerMain[nViewer]->GetViewer(), color, m_pMain->m_stInsp_DistanceResult[real_cam][nPos].xPos[POINT_CROSS_1], m_pMain->m_stInsp_DistanceResult[real_cam][nPos].yPos[POINT_CROSS_1], nPenThickness, nPenThickness / 2, nPenLength);
			}
		}
		}
	}

	// Distance 검사 포인트 추출
	int nWidthPoint = 0, nHeightPoint = 0;
	switch (nCam)
	{
	case 0:	nWidthPoint = 5; nHeightPoint = 6; break;
	case 1:	nWidthPoint = 0; nHeightPoint = 7; break;
	case 2:	nWidthPoint = 4; nHeightPoint = 3; break;
	case 3:	nWidthPoint = 1; nHeightPoint = 2; break;
	}
	double Spec_distance_LSL_W = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(nWidthPoint);
	double Spec_distance_USL_W = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(nWidthPoint);
	double Spec_distance_LSL_H = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(nHeightPoint);
	double Spec_distance_USL_H = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(nHeightPoint);

	// Tilt 검사 포인트 추출
	int nTilt_Point_W = 0, nTilt_Point_H = 0;
	switch (nCam)
	{
	case 0:	nTilt_Point_W = 3; nTilt_Point_H = 2; break;
	case 1:	nTilt_Point_W = 3; nTilt_Point_H = 0; break;
	case 2:	nTilt_Point_W = 1; nTilt_Point_H = 2; break;
	case 3:	nTilt_Point_W = 1; nTilt_Point_H = 0; break;
	}
	double Spec_Tilt_W = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSubSpec(nTilt_Point_W);
	double Spec_Tilt_H = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSubSpec(nTilt_Point_H);
	double Spec_Tilt_Tolerance = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSubSpecTolerance(0);

	pDC->SetBkColor(COLOR_WHITE);
	if (m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bNG != TRUE) {
		color = COLOR_DARK_LIME;
	}
	else { color = COLOR_RED; }

	const int orisize = 12;
	const int fitsize = 12;
	// Text
	if (m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindPattern[FIND_PANEL] &&
		m_pMain->m_stInsp_DistanceResult[real_cam][nPos].bFindPattern[FIND_PCB_BOTTOM])
	{
		switch (nMethod)
		{
		case DISTANCE_INSP_METHOD_M_TO_M:
		case DISTANCE_INSP_METHOD_M_TO_E:
		case DISTANCE_INSP_METHOD_E_TO_E:
		{
			int offset = 50;
			if (m_pMain->m_dDistanceInspJudgeResult[nWidthPoint])	color = COLOR_DARK_LIME;
			else													color = COLOR_RED;
			strText.Format("X(P%d) = %.3f( %.3f ~ %.3f )", nWidthPoint + 1,
				m_pMain->m_stInsp_DistanceResult[real_cam][nPos].dWidthDistance,
				Spec_distance_LSL_W,
				Spec_distance_USL_W);
			m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, orisize, fitsize, 40, 50, true);

			if (m_pMain->m_dDistanceInspJudgeResult[nHeightPoint])	color = COLOR_DARK_LIME;
			else													color = COLOR_RED;
			strText.Format("Y(P%d) = %.3f( %.3f ~ %.3f )", nHeightPoint + 1,
				m_pMain->m_stInsp_DistanceResult[real_cam][nPos].dHeightDistance,
				Spec_distance_LSL_H,
				Spec_distance_USL_H);
			m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, orisize, fitsize, 40, 50 + offset, true);

			if (m_pMain->m_dDistanceInspSubJudgeResult[nTilt_Point_W])	color = COLOR_DARK_LIME;
			else														color = COLOR_RED;
			strText.Format("X(Tilt%d) = %.3f( %d ~ %.3f )", nTilt_Point_W + 1,
				m_pMain->m_dDistanceInspSubDataResult[nTilt_Point_W],
				0,
				Spec_Tilt_W + Spec_Tilt_Tolerance);
			m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, orisize, fitsize, 40, 50 + (offset * 2), true);

			if (m_pMain->m_dDistanceInspSubJudgeResult[nTilt_Point_H])	color = COLOR_DARK_LIME;
			else														color = COLOR_RED;
			strText.Format("Y(Tilt%d) = %.3f( %d ~ %.3f )", nTilt_Point_H + 1,
				m_pMain->m_dDistanceInspSubDataResult[nTilt_Point_H],
				0,
				Spec_Tilt_H + Spec_Tilt_Tolerance);
			m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, orisize, fitsize, 40, 50 + (offset * 3), true);
		}
		break;
		case DISTANCE_INSP_METHOD_L_TO_M:
		case DISTANCE_INSP_METHOD_L_TO_E:
		{
			strText.Format("Distance=%.3f", m_pMain->m_stInsp_DistanceResult[real_cam][nPos].dDistance);
			m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, orisize, fitsize, 40, 50, true);
		}
		break;
		}
	}
	else
	{
		color = COLOR_RED;
		m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, orisize, fitsize, 40, 50, true);
	}

	m_pDlgViewerMain[nViewer]->GetViewer().OnLoadImageFromPtr(m_pMain->getProcBuffer(real_cam, nPos));

	m_pDlgViewerMain[nViewer]->GetViewer().Invalidate();
}
void CFormMainView::draw_align_result_2cam_2shot(int nCam, int nPos, int nViewer, int nJob, BOOL bErase)
{
	auto& viewer = m_pDlgViewerMain[nViewer]->GetViewer();
	auto pDC = m_pDlgViewerMain[nViewer]->GetViewer().getOverlayDC();
	if (bErase)
	{
		m_pDlgViewerMain[nViewer]->GetViewer().ClearOverlayDC();
		m_pDlgViewerMain[nViewer]->GetViewer().clearAllFigures();
	}

	int nPenThickness = MAIN_VIEW_PEN_THICKNESS;
	int nPenLength = MAIN_VIEW_PEN_LENGTH;
	int nFontWidth = MAIN_VIEW_FONT_WIDTH;
	int nFontHeight = MAIN_VIEW_FONT_HEIGHT;

	CPen penOK(PS_SOLID, nPenThickness, RGB(0, 255, 0)), * pOldPen;
	CPen penNG(PS_SOLID, nPenThickness, RGB(255, 0, 0)), penMatch(PS_SOLID, nPenThickness, RGB(255, 255, 0));
	CPen penROI(PS_SOLID, nPenThickness, RGB(255, 255, 255));

	CFont font, * oldFont;
	int OLD_BKMODE = 0;

	font.CreateFont(nFontHeight, nFontWidth, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
		OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, "Arial");
	oldFont = pDC->SelectObject(&font);

	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

	int W = m_pDlgViewerMain[nViewer]->GetViewer().GetWidth();
	int H = m_pDlgViewerMain[nViewer]->GetViewer().GetHeight();

	{
		CFindInfo info = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos);
		pOldPen = pDC->SelectObject(&penOK);

		double posX = info.GetXPos();
		double posY = info.GetYPos();
		double score = info.getScore();
		double angle = info.GetAngle();

		int index = info.GetFoundPatternNum();
		int nTextX, nTextY;

		double mark_offset_x = m_pMain->GetMatching(nJob).getFinalMarkOffsetX(nCam, nPos, index);
		double mark_offset_y = m_pMain->GetMatching(nJob).getFinalMarkOffsetY(nCam, nPos, index);

		posX += mark_offset_x;
		posY += mark_offset_y;


		//20.03.29 MultFlexRoi로 Patter 찾을 시 rcROI 변경.
		CRect rcROI = m_pMain->GetMatching(nJob).getSearchROI(nCam, nPos);

		if (info.GetFound() == FIND_ERR)			pDC->SelectObject(&penNG);
		else if (info.GetFound() == FIND_MATCH)		pDC->SelectObject(&penMatch);

		int fatW = m_pMain->GetMatching(nJob).getHalcon(nCam, nPos, index).getFatWidth();
		int fatH = m_pMain->GetMatching(nJob).getHalcon(nCam, nPos, index).getFatHeight();
		double offsetX = m_pMain->GetMatching(nJob).getPosOffsetX(nCam, nPos, index);
		double offsetY = m_pMain->GetMatching(nJob).getPosOffsetY(nCam, nPos, index);


		auto oriX = (posX - offsetX);
		auto oriY = (posY - offsetY);


		viewer.AddSoftGraphic(new GraphicRectangle(Gdiplus::PointF(oriX, oriY), fatW, fatH, pDC->GetCurrentColor()));
		viewer.AddSoftGraphic(new GraphicPoint(Gdiplus::PointF(oriX, oriY), pDC->GetCurrentColor(), 5, 2));

		//pDC->MoveTo((int)(oriX - fatW / 2), (int)(oriY - fatH / 2));
		//pDC->LineTo((int)(oriX + fatW / 2), (int)(oriY - fatH / 2));
		//pDC->LineTo((int)(oriX + fatW / 2), (int)(oriY + fatH / 2));
		//pDC->LineTo((int)(oriX - fatW / 2), (int)(oriY + fatH / 2));
		//pDC->LineTo((int)(oriX - fatW / 2), (int)(oriY - fatH / 2));

		//pDC->MoveTo((int)(oriX - fatW / 2), (int)(posY));
		//pDC->LineTo((int)(oriX + fatW / 2), (int)(posY));
		//pDC->MoveTo((int)(posX), (int)(oriY - fatH / 2));
		//pDC->LineTo((int)(posX), (int)(oriY + fatH / 2));

		nTextX = 100;
		nTextY = 100 + 100 * nPos;

		if (info.GetFound() == FIND_OK)			pDC->SetTextColor(COLOR_GREEN);
		else if (info.GetFound() == FIND_MATCH)			pDC->SetTextColor(COLOR_YELLOW);
		else			pDC->SetTextColor(COLOR_RED);

		pDC->SetBkMode(TRANSPARENT);
		CString strText;

		strText.Format("%d, [%.1f , %.1f] %.1f%% - [%d] %.2f˚", nPos, posX, posY, score, index + 1, angle);
		pDC->TextOutA(nTextX, nTextY, strText);

		viewer.AddSoftGraphic(new GraphicRectangle(rcROI, pDC->GetCurrentColor()));

		//pDC->SelectObject(&penROI);
		//pDC->SetColor(Gdiplus::Color(0xff00ff00));
		//pDC->MoveTo(rcROI.left, rcROI.top);
		//pDC->LineTo(rcROI.right, rcROI.top);
		//pDC->LineTo(rcROI.right, rcROI.bottom);
		//pDC->LineTo(rcROI.left, rcROI.bottom);
		//pDC->LineTo(rcROI.left, rcROI.top);


		pDC->SelectObject(pOldPen);
	}

#ifndef JOB_INFO
	if (m_pMain->getModel().getAlignInfo().getEnableLCheck(nCam))
#else
	if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getEnableLCheck())
#endif
	{
		if (m_pMain->m_bLcheckNG[nJob])
		{
			CPen penCrack(PS_SOLID, nPenThickness, COLOR_RED), * pOld;
			pOld = pDC->SelectObject(&penCrack);

			CFont font, * oldFont;
			font.CreateFont(nFontHeight, nFontWidth, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
				OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
			oldFont = pDC->SelectObject(&font);

			COLORREF oldColor = pDC->SetTextColor(COLOR_RED);
			pDC->TextOutA(W - nFontWidth * 20, H - nFontHeight * 4, "L-CHECK ERROR");

			CString strText;
#ifndef JOB_INFO
			strText.Format("lmt : %.3f", m_pMain->getModel().getAlignInfo().getLCheckLimit(nCam));
#else
			strText.Format("lmt : %.3f", m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckTor());
#endif
			pDC->TextOutA(W - nFontWidth * 20, H - nFontHeight * 2, strText);

			strText.Format("L : %.3f", m_pMain->m_dbLcheckLength[nJob]);
			pDC->TextOutA(W - nFontWidth * 20, H - nFontHeight * 3, strText);

			pDC->SetTextColor(oldColor);
			pDC->SelectObject(oldFont);
		}
	}

	m_pDlgViewerMain[nViewer]->GetViewer().OnLoadImageFromPtr(m_pMain->getProcBuffer(real_cam, nPos));
	m_pDlgViewerMain[nViewer]->GetViewer().Invalidate();

	font.DeleteObject();
	penOK.DeleteObject();
	penNG.DeleteObject();
	penMatch.DeleteObject();
	penROI.DeleteObject();
}
void CFormMainView::draw_align_result_mark_edge(int nCam, int nPos, int nViewer, int nJob, BOOL bErase, BOOL bType) {
	auto pDC = m_pDlgViewerMain[nViewer]->GetViewer().getOverlayDC();

	int nPenThickness = MAIN_VIEW_PEN_THICKNESS;
	int nPenLength = MAIN_VIEW_PEN_LENGTH;
	int nFontWidth = MAIN_VIEW_FONT_WIDTH;
	int nFontHeight = MAIN_VIEW_FONT_HEIGHT;

	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

	int W = m_pDlgViewerMain[nViewer]->GetViewer().GetWidth();
	int H = m_pDlgViewerMain[nViewer]->GetViewer().GetHeight();

	BOOL bFindLine;
	sLine line_info;

#pragma region 마크 정보
	CFindInfo info = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos);

	double posX = info.GetXPos();
	double posY = info.GetYPos();
	double score = info.getScore();
	double angle = info.GetAngle();

	int index = info.GetFoundPatternNum();

	double mark_offset_x = m_pMain->GetMatching(nJob).getFinalMarkOffsetX(nCam, nPos, index);
	double mark_offset_y = m_pMain->GetMatching(nJob).getFinalMarkOffsetY(nCam, nPos, index);

	posX += mark_offset_x;
	posY += mark_offset_y;

	CRect rcROI = m_pMain->GetMatching(nJob).getSearchROI(nCam, nPos);

	int fatW = m_pMain->GetMatching(nJob).getHalcon(nCam, nPos, index).getFatWidth();
	int fatH = m_pMain->GetMatching(nJob).getHalcon(nCam, nPos, index).getFatHeight();
	double offsetX = m_pMain->GetMatching(nJob).getPosOffsetX(nCam, nPos, index);
	double offsetY = m_pMain->GetMatching(nJob).getPosOffsetY(nCam, nPos, index);
	int oriX = (int)(posX - offsetX);
	int oriY = (int)(posY - offsetY);
#pragma endregion
	COLORREF color;
#pragma region 레퍼런스등록
	//////////////////// Reference //////////////////////
	double RefX = m_pMain->GetMatching(nJob).getRefX(nCam, nPos);
	double RefY = m_pMain->GetMatching(nJob).getRefY(nCam, nPos);

	//if (RefX > 0 || RefY > 0)
	if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseReferenceMark())
	{
		// Reference 마크 표시
		// kbj 2022-01-10 Fixture Mark Color = ORANGE, Referace Mark Color = BULE
		color = COLOR_BLUE;
		m_pMain->draw_mark(&m_pDlgViewerMain[nViewer]->GetViewer(), color, RefX, RefY, nPenThickness, nPenThickness / 2, nPenLength);
	}
#pragma endregion

#pragma regioin Fixture 등록 / Roi
	BOOL bFixtureUse = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseFixtureMark();

	if (bFixtureUse)
	{
		double fixtureX = m_pMain->GetMatching(nJob).getFixtureX(nCam, nPos);
		double fixtureY = m_pMain->GetMatching(nJob).getFixtureY(nCam, nPos);

		////찾은 마크 표시
		if (m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetFound() == FIND_OK)					color = COLOR_ORANGE;
		else if (m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetFound() == FIND_MATCH)			color = COLOR_YELLOW;
		else			color = COLOR_RED;

		double posX = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetMatchingXPos();
		double posY = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetMatchingYPos();
		m_pMain->draw_mark(&m_pDlgViewerMain[nViewer]->GetViewer(), color, posX, posY, nPenThickness * 2, nPenThickness, nPenLength);

		//ROI
		color = COLOR_WHITE;
		stFigure tempFig;
		CRect rcROI;
		rcROI = m_pMain->GetMatching(nJob).getSearchROI(nCam, nPos);
		tempFig.ptBegin.x = rcROI.left;
		tempFig.ptBegin.y = rcROI.top;
		tempFig.ptEnd.x = rcROI.right;
		tempFig.ptEnd.y = rcROI.bottom;
		m_pDlgViewerMain[nViewer]->GetViewer().addFigureRect(tempFig, 1, 1, color);
	}
#pragma endregion
	color = COLOR_GREEN;
#pragma region 켈리퍼 정보
	if (nPos == 0)
	{
		line_info = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_1].m_lineInfo;

		bFindLine = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_1].getIsMakeLine();
	}
	else
	{
		line_info= m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_1].m_lineInfo;

		bFindLine = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][C_CALIPER_POS_1].getIsMakeLine();
	}

	if (info.GetFound() == FIND_OK)			color = COLOR_GREEN;
	else if (info.GetFound() == FIND_MATCH)			color = COLOR_YELLOW;
	else			color = COLOR_RED;
#pragma endregion

#pragma region 라인 그리기
	if (bFindLine)	color = COLOR_GREEN;
	else								color = COLOR_RED;

	// 수직 그리기
	if (bFindLine)
		m_pMain->draw_line(&m_pDlgViewerMain[nViewer]->GetViewer(), color, line_info, H, W);

	// 수평 그리기
	if (bFindLine)
		m_pMain->draw_line(&m_pDlgViewerMain[nViewer]->GetViewer(), color, line_info, H, W);

	//////////////////// 후보군 그리기 //////////////////////
	// kbj 2022-01-03 modify draw caliper_result 
	m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][0].draw_final_result(pDC);

#pragma endregion
	cv::Point2f st, ed;

	//KJH 2021-05-28 Trace일때 이미지 제거
	if (!bType)
	{
		m_pDlgViewerMain[nViewer]->GetViewer().AddSoftGraphic(new GraphicRectangle(Gdiplus::PointF(oriX, oriY), float(fatW), float(fatH), ColorRefToColor(color), 1));
		m_pDlgViewerMain[nViewer]->GetViewer().AddSoftGraphic(new GraphicPoint(posX, posY, ColorRefToColor(color), 5, 2));
	}

	// 텍스트
	pDC->SetBkMode(TRANSPARENT);
	CString strText;

	//strText.Format("%d, [%.1f, %.1f] %.1f%% - [%d] %.2f°", nPos, posX, posY, score, index + 1, angle);
	int offset = nPos * 20;
	//m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, 12, 12, 40, 40 + offset, TRUE);

	CStringW str;
	str.Format(L"%d, [%.1f, %.1f] %.1f%% - [%d] %.2f°", nPos, posX, posY, score, index + 1, angle);
	m_pDlgViewerMain[nViewer]->GetViewer().AddHardGraphic(new GraphicLabel(cv::Point2f(10, 10 + offset), str, Gdiplus::Color(0xff00ff00)));
	//m_pDlgViewerMain[nViewer]->GetViewer().AddHardGraphic(new GraphicLabel(10, 10 + offset, strText));

	// ROI
	color = COLOR_WHITE;

	st.x = rcROI.left;
	st.y = rcROI.top;
	ed.x = rcROI.right;
	ed.y = rcROI.bottom;

	if (!bType)
	{
		m_pDlgViewerMain[nViewer]->GetViewer().AddSoftGraphic(new GraphicRectangle(st, ed, ColorRefToColor(color)));
	}

#pragma region L Check
	int nMethod = m_pMain->vt_job_info[nJob].algo_method;

	if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getEnableLCheck())
	{
		if (m_pMain->m_bLcheckNG[nJob])
		{
			color = COLOR_RED;
			CString strText;


			switch (nMethod)
			{
			case CLIENT_TYPE_2CAM_EACH_2SHOT_ALIGN:
			case CLIENT_TYPE_4CAM_1SHOT_ALIGN:
			{
				m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), "L-CHECK ERROR", color, 12, 12, 40, H - 600);

				strText.Format("TOP : %.3f ,BOTTOM : %.3f", m_pMain->m_dbLCheck[nJob][0], m_pMain->m_dbLCheck[nJob][3]);
				m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, 12, 12, 40, H - 500);

				strText.Format("SpecX : %.3f ~ %.3f", m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckSpecX() - m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckTor(),
					m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckSpecX() + m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckTor());
				m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, 12, 12, 40, H - 400);

				strText.Format("LEFT: %.3f ,RIGHT : %.3f", m_pMain->m_dbLCheck[nJob][1], m_pMain->m_dbLCheck[nJob][2]);
				m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, 12, 12, 40, H - 300);

				strText.Format("SpecY : %.3f ~ %.3f", m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckSpecY() - m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckTor(),
					m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckSpecY() + m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckTor());
				m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, 12, 12, 40, H - 200);
			}
			break;
			default:
			{
				double dist_spec = 0;
				int target_dir = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getAlignmentTargetDir();	// vertical or horizontal
				if (target_dir == 0)	dist_spec = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckSpecX();// horizontal
				else					dist_spec = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckSpecY();// vertical


				strText.Format("Length: %.3f", m_pMain->m_dbLCheck[nJob][0]);
				m_pDlgViewerMain[nViewer]->GetViewer().AddHardGraphic(new GraphicLabel(10, -30, strText, Gdiplus::Color(0xffff0000), Anchor::TopRight));

				strText.Format("Spec: %.3f ~ %.3f", dist_spec - m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckTor(),
					dist_spec + m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckTor());
				m_pDlgViewerMain[nViewer]->GetViewer().AddHardGraphic(new GraphicLabel(10, -50, strText, Gdiplus::Color(0xffff0000), Anchor::TopRight));
			}
			break;
			}
		}
	}
#pragma endregion
	if (nMethod == CLIENT_TYPE_1CAM_2POS_REFERENCE ||
		nMethod == CLIENT_TYPE_1CAM_4POS_ROBOT)
	{
		// KBJ 2022-07-07 1Cam 2Pos Reference nPos 0일때만 이미지 한번 그리도록 수정
		if (nPos == 0)
		{
			m_pDlgViewerMain[nViewer]->GetViewer().OnLoadImageFromPtr(m_pMain->getProcBuffer(real_cam, 0));  //KJH 2022-03-10 PF Matching 사용시 Image Index 0 이미지 그리기
		}
	}
	else
	{
		m_pDlgViewerMain[nViewer]->GetViewer().OnLoadImageFromPtr(m_pMain->getProcBuffer(real_cam, nPos));
	}
	m_pDlgViewerMain[nViewer]->GetViewer().DirtyRefresh();
}
void CFormMainView::draw_align_result(int nCam, int nPos, int nViewer, int nJob, BOOL bErase, BOOL bType)
{
	auto pDC = m_pDlgViewerMain[nViewer]->GetViewer().getOverlayDC();
	if (bErase)
	{
		if (m_pMain->vt_job_info[nJob].algo_method != CLIENT_TYPE_1CAM_1SHOT_ALIGN_WITH_SCRATCH_INSP
			&& m_pMain->vt_job_info[nJob].algo_method != CLIENT_TYPE_1CAM_1SHOT_FILM)
		{
			m_pDlgViewerMain[nViewer]->GetViewer().ClearOverlayDC();
			m_pDlgViewerMain[nViewer]->GetViewer().clearAllFigures();
		}
	}

	int nPenThickness = MAIN_VIEW_PEN_THICKNESS;
	int nPenLength = MAIN_VIEW_PEN_LENGTH;
	int nFontWidth = MAIN_VIEW_FONT_WIDTH;
	int nFontHeight = MAIN_VIEW_FONT_HEIGHT;

	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

	int W = m_pDlgViewerMain[nViewer]->GetViewer().GetWidth();
	int H = m_pDlgViewerMain[nViewer]->GetViewer().GetHeight();

	CFindInfo info = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos);

	double posX = info.GetXPos();
	double posY = info.GetYPos();
	double score = info.getScore();
	double angle = info.GetAngle();

	int index = info.GetFoundPatternNum();

	double mark_offset_x = m_pMain->GetMatching(nJob).getFinalMarkOffsetX(nCam, nPos, index);
	double mark_offset_y = m_pMain->GetMatching(nJob).getFinalMarkOffsetY(nCam, nPos, index);

	posX += mark_offset_x;
	posY += mark_offset_y;

	CRect rcROI = m_pMain->GetMatching(nJob).getSearchROI(nCam, nPos);

	int fatW = m_pMain->GetMatching(nJob).getHalcon(nCam, nPos, index).getFatWidth();
	int fatH = m_pMain->GetMatching(nJob).getHalcon(nCam, nPos, index).getFatHeight();
	double offsetX = m_pMain->GetMatching(nJob).getPosOffsetX(nCam, nPos, index);
	double offsetY = m_pMain->GetMatching(nJob).getPosOffsetY(nCam, nPos, index);
	int oriX = (int)(posX - offsetX);
	int oriY = (int)(posY - offsetY);

	COLORREF color;
	//////////////////// Reference //////////////////////
	double RefX = m_pMain->GetMatching(nJob).getRefX(nCam, nPos);
	double RefY = m_pMain->GetMatching(nJob).getRefY(nCam, nPos);

	//if (RefX > 0 || RefY > 0)
	if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseReferenceMark())
	{
		// Reference 마크 표시
		// kbj 2022-01-10 Fixture Mark Color = ORANGE, Referace Mark Color = BULE
		color = COLOR_BLUE;
		m_pMain->draw_mark(&m_pDlgViewerMain[nViewer]->GetViewer(), color, RefX, RefY, nPenThickness, nPenThickness / 2, nPenLength);
	}

	if (info.GetFound() == FIND_OK)			color = COLOR_GREEN;
	else if (info.GetFound() == FIND_MATCH)			color = COLOR_YELLOW;
	else			color = COLOR_RED;

	cv::Point2f st, ed;

	//// 마크 그리기
	////stFigure tempFig;
	////tempFig.ptBegin.x = oriX - fatW / 2;
	////tempFig.ptBegin.y = oriY + fatH / 2;
	////tempFig.ptEnd.x = oriX + fatW / 2;
	////tempFig.ptEnd.y = oriY + fatH / 2;
	//st.x = oriX - fatW / 2;
	//st.y = oriY + fatH / 2;
	//ed.x = oriX + fatW / 2;
	//ed.y = oriY + fatH / 2;

	////KJH 2021-05-28 Trace일때 이미지 제거
	//if (!bType)
	//{
	//	//m_pDlgViewerMain[nViewer]->GetViewer().addFigureLine(tempFig, 2, 1, color);
	//	m_pDlgViewerMain[nViewer]->GetViewer().AddSoftGraphic(new GraphicLine(st, ed, color | 0xff000000));
	//}

	////tempFig.ptBegin.x = oriX - fatW / 2;
	////tempFig.ptBegin.y = oriY + fatH / 2;
	////tempFig.ptEnd.x = oriX - fatW / 2;
	////tempFig.ptEnd.y = oriY - fatH / 2;
	//st.x = oriX - fatW / 2;
	//st.y = oriY + fatH / 2;
	//ed.x = oriX - fatW / 2;
	//ed.y = oriY - fatH / 2;

	////KJH 2021-05-28 Trace일때 이미지 제거
	//if (!bType)
	//{
	//	//m_pDlgViewerMain[nViewer]->GetViewer().addFigureLine(tempFig, 2, 1, color);
	//	m_pDlgViewerMain[nViewer]->GetViewer().AddSoftGraphic(new GraphicLine(st, ed, color | 0xff000000));
	//}

	////tempFig.ptBegin.x = oriX + fatW / 2;
	////tempFig.ptBegin.y = oriY - fatH / 2;
	////tempFig.ptEnd.x = oriX + fatW / 2;
	////tempFig.ptEnd.y = oriY + fatH / 2;
	//st.x = oriX + fatW / 2;
	//st.y = oriY - fatH / 2;
	//ed.x = oriX + fatW / 2;
	//ed.y = oriY + fatH / 2;

	////KJH 2021-05-28 Trace일때 이미지 제거
	//if (!bType)
	//{
	//	//m_pDlgViewerMain[nViewer]->GetViewer().addFigureLine(tempFig, 2, 1, color);
	//	m_pDlgViewerMain[nViewer]->GetViewer().AddSoftGraphic(new GraphicLine(st, ed, color | 0xff000000));
	//}

	////tempFig.ptBegin.x = oriX + fatW / 2;
	////tempFig.ptBegin.y = oriY - fatH / 2;
	////tempFig.ptEnd.x = oriX - fatW / 2;
	////tempFig.ptEnd.y = oriY - fatH / 2;
	//st.x = oriX + fatW / 2;
	//st.y = oriY - fatH / 2;
	//ed.x = oriX - fatW / 2;
	//ed.y = oriY - fatH / 2;

	//KJH 2021-05-28 Trace일때 이미지 제거
	if (!bType)
	{
		m_pDlgViewerMain[nViewer]->GetViewer().AddSoftGraphic(new GraphicRectangle(Gdiplus::PointF(oriX, oriY), float(fatW), float(fatH), ColorRefToColor(color), 1));
		//m_pDlgViewerMain[nViewer]->GetViewer().addFigureLine(tempFig, 2, 1, color);
		//m_pDlgViewerMain[nViewer]->GetViewer().AddSoftGraphic(new GraphicLine(st, ed, color | 0xff000000));
		//m_pMain->draw_mark(&m_pDlgViewerMain[nViewer]->GetViewer(), color, posX, posY, nPenThickness, nPenThickness / 2, nPenLength);
		m_pDlgViewerMain[nViewer]->GetViewer().AddSoftGraphic(new GraphicPoint(posX, posY, ColorRefToColor(color), 5, 2));
	}

	// 텍스트
	pDC->SetBkMode(TRANSPARENT);
	CString strText;

	//strText.Format("%d, [%.1f, %.1f] %.1f%% - [%d] %.2f°", nPos, posX, posY, score, index + 1, angle);
	int offset = nPos * 20;
	//m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, 12, 12, 40, 40 + offset, TRUE);

	CStringW str;
	str.Format(L"%d, [%.1f, %.1f] %.1f%% - [%d] %.2f°", nPos, posX, posY, score, index + 1, angle);
	m_pDlgViewerMain[nViewer]->GetViewer().AddHardGraphic(new GraphicLabel(cv::Point2f(10, 10 + offset), str, Gdiplus::Color(0xff00ff00)));
	//m_pDlgViewerMain[nViewer]->GetViewer().AddHardGraphic(new GraphicLabel(10, 10 + offset, strText));

	// ROI
	color = COLOR_WHITE;
	//tempFig.ptBegin.x = rcROI.left;
	//tempFig.ptBegin.y = rcROI.top;
	//tempFig.ptEnd.x = rcROI.right;
	//tempFig.ptEnd.y = rcROI.bottom;
	//KJH 2021-05-28 Trace일때 이미지 제거

	st.x = rcROI.left;
	st.y = rcROI.top;
	ed.x = rcROI.right;
	ed.y = rcROI.bottom;

	if (!bType)
	{
		//m_pDlgViewerMain[nViewer]->GetViewer().addFigureRect(tempFig, 1, 1, color);
		//m_pDlgViewerMain[nViewer]->GetViewer().AddSoftGraphic(new GraphicLine(st, ed, color | 0xff000000));
		m_pDlgViewerMain[nViewer]->GetViewer().AddSoftGraphic(new GraphicRectangle(st, ed, ColorRefToColor(color)));
	}

	//////////////////// L-check 표시 //////////////////////
	int nMethod = m_pMain->vt_job_info[nJob].algo_method;

	if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getEnableLCheck())
	{
		if (m_pMain->m_bLcheckNG[nJob])
		{
			color = COLOR_RED;
			CString strText;


			switch (nMethod)
			{
			case CLIENT_TYPE_2CAM_EACH_2SHOT_ALIGN:
			case CLIENT_TYPE_4CAM_1SHOT_ALIGN:
			{
				m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), "L-CHECK ERROR", color, 12, 12, 40, H - 600);

				strText.Format("TOP : %.3f ,BOTTOM : %.3f", m_pMain->m_dbLCheck[nJob][0], m_pMain->m_dbLCheck[nJob][3]);
				m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, 12, 12, 40, H - 500);

				strText.Format("SpecX : %.3f ~ %.3f", m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckSpecX() - m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckTor(),
					m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckSpecX() + m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckTor());
				m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, 12, 12, 40, H - 400);

				strText.Format("LEFT: %.3f ,RIGHT : %.3f", m_pMain->m_dbLCheck[nJob][1], m_pMain->m_dbLCheck[nJob][2]);
				m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, 12, 12, 40, H - 300);

				strText.Format("SpecY : %.3f ~ %.3f", m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckSpecY() - m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckTor(),
					m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckSpecY() + m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckTor());
				m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, 12, 12, 40, H - 200);
			}
			break;
			default:
			{
				double dist_spec = 0;
				int target_dir = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getAlignmentTargetDir();	// vertical or horizontal
				if (target_dir == 0)	dist_spec = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckSpecX();// horizontal
				else					dist_spec = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckSpecY();// vertical

				//strText.Format("Length: %.3f", m_pMain->m_dbLCheck[nJob][0]);
				//m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, 12, 12, 40, H - 700, TRUE);

				//strText.Format("Spec: %.3f ~ %.3f", dist_spec - m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckTor(),
				//	dist_spec + m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckTor());
				//m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, 12, 12, 40, H - 500);

				strText.Format("Length: %.3f", m_pMain->m_dbLCheck[nJob][0]);
				m_pDlgViewerMain[nViewer]->GetViewer().AddHardGraphic(new GraphicLabel(10, -30, strText, Gdiplus::Color(0xffff0000), Anchor::TopRight));

				strText.Format("Spec: %.3f ~ %.3f", dist_spec - m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckTor(),
					dist_spec + m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLCheckTor());
				m_pDlgViewerMain[nViewer]->GetViewer().AddHardGraphic(new GraphicLabel(10, -50, strText, Gdiplus::Color(0xffff0000), Anchor::TopRight));
			}
			break;
			}
		}
	}
	if (nMethod == CLIENT_TYPE_1CAM_2POS_REFERENCE ||
		nMethod == CLIENT_TYPE_1CAM_4POS_ROBOT)
	{
		// KBJ 2022-07-07 1Cam 2Pos Reference nPos 0일때만 이미지 한번 그리도록 수정
		if (nPos == 0)
		{
			m_pDlgViewerMain[nViewer]->GetViewer().OnLoadImageFromPtr(m_pMain->getProcBuffer(real_cam, 0));  //KJH 2022-03-10 PF Matching 사용시 Image Index 0 이미지 그리기
		}
	}
	else
	{
		m_pDlgViewerMain[nViewer]->GetViewer().OnLoadImageFromPtr(m_pMain->getProcBuffer(real_cam, nPos));
	}
	m_pDlgViewerMain[nViewer]->GetViewer().DirtyRefresh();
}

void CFormMainView::draw_align_result_circle(int nCam, int nPos, int nViewer, int nJob, BOOL bErase, BOOL bType)
{
	auto pDC = m_pDlgViewerMain[nViewer]->GetViewer().getOverlayDC();
	int OLD_BKMODE = 0;

	if (bErase)
	{
		m_pDlgViewerMain[nViewer]->GetViewer().ClearOverlayDC();
		m_pDlgViewerMain[nViewer]->GetViewer().clearAllFigures();
	}

	int nPenThickness = MAIN_VIEW_PEN_THICKNESS;
	int nPenLength = MAIN_VIEW_PEN_LENGTH;
	int nFontWidth = MAIN_VIEW_FONT_WIDTH;
	int nFontHeight = MAIN_VIEW_FONT_HEIGHT;

	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];

	int W = m_pDlgViewerMain[nViewer]->GetViewer().GetWidth();
	int H = m_pDlgViewerMain[nViewer]->GetViewer().GetHeight();

	CString strText;
	COLORREF color = COLOR_GREEN;

	double posX = m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x;
	double posY = m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y;

	if (m_pMain->m_nErrorType[nJob] == ERR_TYPE_CIRCLE_ERROR)
	{
		color = COLOR_RED;

		double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(0, 0);
		double camHole = 0.0;
		if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseMetalTraceFirst()) // 메탈 얼라인
			camHole = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspSteelHoleSize();
		else  camHole = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspCamHoleSize();

		strText.Format("Find Circle Error");
		m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, 18, 18, posX - CIRCLE_RADIUS, posY - CIRCLE_RADIUS - 450 , TRUE);
		strText.Format("Circle : %.2f", CIRCLE_RADIUS * xres);
		m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, 15, 15, posX - CIRCLE_RADIUS, posY - CIRCLE_RADIUS - 300, TRUE);
		strText.Format("Spec(%.2f) : (%.2f ~ %.2f)", camHole, camHole - xres * 30, camHole + xres * 30);
		m_pMain->draw_text(&m_pDlgViewerMain[nViewer]->GetViewer(), strText, color, 15, 15, posX - CIRCLE_RADIUS, posY - CIRCLE_RADIUS - 150, TRUE);
	}

	//m_pMain->draw_mark(&m_pDlgViewerMain[nViewer]->GetViewer(), color, posX, posY, 3, 3, nPenLength);

	/*stFigure tempFig;
	tempFig.isDot = false;
	tempFig.ptBegin.x = posX - CIRCLE_RADIUS;
	tempFig.ptBegin.y = posY - CIRCLE_RADIUS;
	tempFig.ptEnd.x = posX + CIRCLE_RADIUS;
	tempFig.ptEnd.y = posY + CIRCLE_RADIUS;
	m_pDlgViewerMain[nViewer]->GetViewer().addFigureRect(tempFig, 3, 3, color);*/

	m_pDlgViewerMain[nViewer]->GetViewer().OnLoadImageFromPtr(m_pMain->getProcBuffer(real_cam, nPos));
	m_pDlgViewerMain[nViewer]->GetViewer().Invalidate();
}

void CFormMainView::OnTcnSelchangeTabMainView(NMHDR* pNMHDR, LRESULT* pResult)
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	*pResult = 0;

	int sel = m_TabMainView.GetCurSel();
	int job = sel - 1;

	m_nCurrentSelMainView = sel;

	if (sel == 0)
	{
		replace_viewer_main(m_pMain->m_nViewX_Num, m_pMain->m_nViewY_Num);
		//HTK 2022-07-11 Main View 변경시 화면 재 그리도록 변경
		//kmb 20221020 Toal View 변경시 모든뷰어 Fit
		for (int nViewer = 0; nViewer < m_pMain->vt_viewer_info.size(); nViewer++) {
			m_pDlgViewerMain[nViewer]->GetViewer().SetFitMode();
		}
		RedrawWindow();
	}
	else
	{
		//KJH 2021-09-13 Total View말고 다 Return
		//m_TabMainView.SetCurSel(0);
		//return;
		//KJH 2021-09-18 이게 문제가 아닌거 확인함
		int x_cnt = m_pMain->vt_job_info[job].main_view_count_x;
		int y_cnt = m_pMain->vt_job_info[job].main_view_count_y;
		replace_viewer_main_fix(x_cnt, y_cnt, job);
		//HTK 2022-07-11 Main View 변경시 화면 재 그리도록 변경
		RedrawWindow();
	}

	//HSJ 2022-01-05 Main View Tab이동시 해당 결과창도 같이 바뀌게
	//HSJ 2022-01-13 Main View Tab이동시 해당 스펙창도 같이 바뀌게

	CPaneAuto* pPaneAuto = (CPaneAuto*)m_pMain->m_pPane[PANE_AUTO];

	if (sel == 0) return;
	else
	{
		int index = sel - 1;
		pPaneAuto->m_TabResultView.SetCurSel(index);
		pPaneAuto->RenewResultChart(index);

		int method = m_pMain->vt_job_info[index].algo_method;
		if (method == CLIENT_TYPE_SCAN_INSP)
		{
			pPaneAuto->m_TabSpecView.SetCurSel(1);
			pPaneAuto->RenewSpecChart(1);
		}
		else if (method == CLIENT_TYPE_FILM_INSP)
		{
			if (index == 0)
			{
				pPaneAuto->m_TabSpecView.SetCurSel(2);
				pPaneAuto->RenewSpecChart(2);
			}
			if (index == 1)
			{
				pPaneAuto->m_TabSpecView.SetCurSel(3);
				pPaneAuto->RenewSpecChart(3);
			}
		}
		else 
		{
			pPaneAuto->m_TabSpecView.SetCurSel(0);
			pPaneAuto->RenewSpecChart(0);
		}
	}
}

const int ERR_NOTHING = 0;                            //(정상동작)
const int ERR_MODEL_TYPE_ERROR = 1;                   //(Process Type 에러)
const int ERR_CREATE_CURR_MODEL_NOT_EXIST = 2;        //(생성 시 현재모델 없음)
const int ERR_CREATE_TARGET_MODEL_EXIST = 3;          //(생성 시 타겟모델 있음)
const int ERR_CHANGE_TARGET_MODEL_NOT_EXIST = 4;       //(변동 시 타겟모델 없음)
const int ERR_MODIFY_CURR_MODEL_NOT_EXIST = 5;        //(수정 시 현재모델 없음)
const int ERR_MODIFY_CURR_MODEL_NOT_MATCHING = 6;     //(수정 시 현재 PC모델과 PLC모델 일치 안됨)
const int ERR_DELETE_TARGET_MODEL_NOT_EXIST = 7;      //(삭제 시 타겟모델 없음)
const int ERR_DELETE_CURR_TARGET_MODEL_MATCHING = 8;   //(삭제 시 현재모델과 타겟모델과 일치)

void CFormMainView::fnProcess_ModelProcess()
{
	CString strTemp;

	for (int m = 0; m < 6; m++)
	{
		if (g_CommPLC.GetBit(m_pMain->m_nStartReadBitAddr_Org + 2 + m) && m_bModelProcessStart[m] != TRUE)
		{
			m_bModelProcessStart[m] = TRUE;

			//m_nStartWriteBitAddr   m_nStartReadBitAddr

			strTemp.Format("[PLC MODEL] Process Start");
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);

			long nRet = do_model_process(m);
			if (nRet == 0)	g_CommPLC.SetBit(m_pMain->m_nStartWriteBitAddr_Org + 2 + m, TRUE);
			else
			{
				//g_CommPLC.SetWord(PLC_WD_MODEL_ERROR_CODE, 1, &nRet);
				g_CommPLC.SetBit(m_pMain->m_nStartWriteBitAddr_Org + 9, TRUE);
			}
		}

		if (g_CommPLC.GetBit(m_pMain->m_nStartReadBitAddr_Org + 2 + m) != TRUE && m_bModelProcessStart[m])
		{
			m_bModelProcessStart[m] = FALSE;

			strTemp.Format("[PLC MODEL] Process End");
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);

			g_CommPLC.SetBit(m_pMain->m_nStartWriteBitAddr_Org + 2 + m, FALSE);
			g_CommPLC.SetBit(m_pMain->m_nStartWriteBitAddr_Org + 9, FALSE);
		}
	}
}

// KBJ 2022-09-22 모델 프로세스 2022버젼 추가
void CFormMainView::fnProcess_ModelProcess_2022()
{
	CString strTemp;
	BOOL bEnable = TRUE;

	for (int m = 0; m < 4; m++)
	{
		if (m_bModelProcessStart[m] == TRUE)
		{
			bEnable = FALSE;
		}
	}

	if (bEnable == FALSE)
	{
		for (int m = 0; m < 4; m++)
		{
			if (g_CommPLC.GetBit(m_pMain->model_process_addres.PLC_RB_PLC_MODEL_CREAT + m) == TRUE)
			{
				strTemp.Format("[PLC MODEL] No Able !!! Process Mode : %d", m);
				::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);
			}
		}
		return;
	}

	for (int m = 0; m < 4; m++)
	{
		for (int check = 0; check < 4; check++)
		{
			if (check == m) continue;

			if (m_bModelProcessStart[m] == TRUE)
			{
				bEnable = FALSE;
			}
		}

		if (g_CommPLC.GetBit(m_pMain->model_process_addres.PLC_RB_PLC_MODEL_CREAT + m) && m_bModelProcessStart[m] != TRUE && bEnable == TRUE)
		{
			g_CommPLC.SetBit(m_pMain->model_process_addres.PLC_WB_PLC_MODEL_PROCESS_ABLE, FALSE);

			m_bModelProcessStart[m] = TRUE;

			strTemp.Format("[PLC MODEL] Process Start");
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);

			long nRet = do_model_process_2022(m);
			if (nRet == 0)	g_CommPLC.SetBit(m_pMain->model_process_addres.PLC_WB_PLC_MODEL_CREAT_ACK + m, TRUE);
			else
			{
				g_CommPLC.SetBit(m_pMain->model_process_addres.PLC_WB_PLC_MODEL_NACK, TRUE);
			}

			g_CommPLC.SetWord(m_pMain->model_process_addres.PLC_WD_PLC_MODEL_NACK_CODE, 1, &nRet);
		}

		if (g_CommPLC.GetBit(m_pMain->model_process_addres.PLC_WB_PLC_MODEL_CREAT_ACK + m) != TRUE && m_bModelProcessStart[m])
		{
			m_bModelProcessStart[m] = FALSE;

			strTemp.Format("[PLC MODEL] Process End");
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);

			g_CommPLC.SetBit(m_pMain->model_process_addres.PLC_WB_PLC_MODEL_CREAT_ACK + m, FALSE);
			g_CommPLC.SetBit(m_pMain->model_process_addres.PLC_WB_PLC_MODEL_NACK, FALSE);
		}
	}
}
void CFormMainView::fnProcess_BitResetProcess(int nJob)
{
	//if (m_pMain->m_bAutoStart)  // KBJ 2022-10-16 Auto모드가 아닐때에도 동작하도록 수정.
	{
		int address, address3;

		address = m_pMain->vt_job_info[nJob].plc_addr_info.read_bit_start;
		address3 = m_pMain->vt_job_info[nJob].plc_addr_info.write_bit_start;
				
		//KJH 2022-04-01 Reel Align Init bit관련 예외처리 추가
		if (m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_1CAM_2POS_REFERENCE ||
			m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_1CAM_4POS_ROBOT)
		{
			if (g_CommPLC.GetBit(address + 39) && m_bResetBit2PosFlag[nJob] == FALSE)
			{
				m_nResetBitFlag_Count[nJob] = 0; // KBJ 2022-11-15 Reset Bit TimeOut 관련 추가

				for (int j = 0; j < 19; j++)
					g_CommPLC.SetBit(address3 + 20 + j, FALSE);

				g_CommPLC.SetBit(address3 + 39, TRUE);
				m_bResetBit2PosFlag[nJob] = TRUE;
			}
			else
			{
				if (g_CommPLC.GetBit(address + 39) == FALSE && m_bResetBit2PosFlag[nJob])
				{
					m_nResetBitFlag_Count[nJob] = 0; // KBJ 2022-11-15 Reset Bit TimeOut 관련 추가

					g_CommPLC.SetBit(address3 + 39, FALSE);
					m_bResetBit2PosFlag[nJob] = FALSE;
				}
			}
		}

		if (g_CommPLC.GetBit(address + 19) && m_bResetBitFlag[nJob] == FALSE)
		{
			m_nResetBitFlag_Count[nJob] = 0; // KBJ 2022-11-15 Reset Bit TimeOut 관련 추가

			for (int j = 0; j < 19; j++)
				g_CommPLC.SetBit(address3 + j, FALSE);

			if (m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_CENTER_NOZZLE_ALIGN)
			{
				//KJH 2022-01-05 Center Nozzle Align 시작하기 전에 ExposureTime 변경
				int real_cam = m_pMain->vt_job_info[nJob].camera_index[1];			//Center Cam
				int nPos = m_pMain->vt_job_info[nJob].model_info.getMachineInfo().getExposureTime(real_cam);
#ifdef _DAHUHA
				if (m_pMain->m_pDahuhaCam[real_cam].isconnected())		m_pMain->m_pDahuhaCam[real_cam].setExposureTime(nPos);
#else
				if (m_pMain->m_pBaslerCam[real_cam])		m_pMain->m_pBaslerCam[real_cam]->SetExposure(nPos);
#endif

				// KBJ 2022-09-01 add Delay
				Delay(300);

				m_pMain->m_nSeqNozzleGrabCount = 0;
				m_pMain->m_nNozzleAlignCount = 0;
			}
			else if (m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_ELB_CENTER_ALIGN)
			{
				//KJH 2022-05-16 ELB Center Align 시작하기 전에 ExposureTIme 변경
				int real_cam = m_pMain->vt_job_info[nJob].camera_index[0];			//Center Cam
				int nPos = m_pMain->vt_job_info[nJob].model_info.getMachineInfo().getExposureTime(real_cam);
#ifdef _DAHUHA
				if (m_pMain->m_pDahuhaCam[real_cam].isconnected())		m_pMain->m_pDahuhaCam[real_cam].setExposureTime(nPos);
#else
				if (m_pMain->m_pBaslerCam[real_cam])		m_pMain->m_pBaslerCam[real_cam]->SetExposure(nPos);
#endif
			}
			
			g_CommPLC.SetBit(address3 + 19, TRUE);
			m_bResetBitFlag[nJob] = TRUE;
		}
		//else
		{
			if (g_CommPLC.GetBit(address + 19) == FALSE && m_bResetBitFlag[nJob])
			{
				m_nResetBitFlag_Count[nJob] = 0; // KBJ 2022-11-15 Reset Bit TimeOut 관련 추가

				g_CommPLC.SetBit(address3 + 19, FALSE);
				m_bResetBitFlag[nJob] = FALSE;
				m_nResetBitFlag_Count[nJob] = 0;
			}
		}

		if (m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_CENTER_SIDE_YGAP)
		{
			if (g_CommPLC.GetBit(address + 8) && m_bResetBit2PosFlag[nJob] == FALSE)
			{
				m_nResetBitFlag_Count[nJob] = 0; // KBJ 2022-11-15 Reset Bit TimeOut 관련 추가

				g_CommPLC.SetBit(address3 + 4, FALSE);
				g_CommPLC.SetBit(address3 + 5, FALSE);
				g_CommPLC.SetBit(address3 + 8, TRUE);

				m_bResetBit2PosFlag[nJob] = TRUE;
			}
			else
			{
				if (g_CommPLC.GetBit(address + 8) == FALSE && m_bResetBit2PosFlag[nJob])
				{
					m_nResetBitFlag_Count[nJob] = 0; // KBJ 2022-11-15 Reset Bit TimeOut 관련 추가

					g_CommPLC.SetBit(address3 + 8, FALSE);

					m_bResetBit2PosFlag[nJob] = FALSE;
				}
			}
		}

		// TimeOut
		// KBJ 2022-11-15 Reset Bit TimeOut 수정
		//if (g_CommPLC.GetBit(address + 19) == TRUE && (m_bResetBitFlag[nJob] == TRUE || m_bResetBit2PosFlag[nJob] == TRUE))
		if (m_bResetBitFlag[nJob] == TRUE || m_bResetBit2PosFlag[nJob] == TRUE)
		{
			m_nResetBitFlag_Count[nJob]++;
			// 50ms * 200 = 10s
			// KBJ 2022-10-17
			if (m_nResetBitFlag_Count[nJob] == 200)
			{
				g_CommPLC.SetBit(address3 + 19, FALSE);
				m_bResetBitFlag[nJob] = FALSE;
				m_nResetBitFlag_Count[nJob] = 0;
				m_bResetBit2PosFlag[nJob] = FALSE;
			}
		}
	}
}
void CFormMainView::fnProcess_AlignOffsetSendProcess(int nJob)
{
	if (m_pMain->m_bAutoStart)
	{
		int address, address3, address2, address4;

		address = m_pMain->vt_job_info[nJob].plc_addr_info.read_bit_start + 74;
		address3 = m_pMain->vt_job_info[nJob].plc_addr_info.write_bit_start + 74;
		address2 = m_pMain->vt_job_info[nJob].plc_addr_info.read_word_start + 60;	
		address4 = m_pMain->vt_job_info[nJob].plc_addr_info.write_word_start + 60;	

		if (g_CommPLC.GetBit(address) && m_bSendOffsetFlag[nJob] == FALSE)
		{
			long lData[8] = { 0, };
			long calibData[16] = { 0, };
			double revisionData[4][2],x,y,t;
			double rT[4] = { 0,90,180,270 };

			g_CommPLC.GetWord(address2, 6, lData);
			x = MAKELONG(lData[0], lData[1]) / MOTOR_SCALE;
			y = MAKELONG(lData[2], lData[3]) / MOTOR_SCALE;
			t = MAKELONG(lData[4], lData[5]) / MOTOR_SCALE;
			t = t* CV_PI / 180.0;

			double tmpx, tmpy;
			double rotx = m_pMain->GetMachine(nJob).getRotateX(0, 0);
			double roty = m_pMain->GetMachine(nJob).getRotateY(0, 0);

			tmpx = x - rotx;
			tmpy = y - roty;

			x = (tmpx * cos(t) - tmpy * sin(t) + rotx);
			y = (tmpx * sin(t) + tmpy * cos(t) + roty);

			tmpx = x - rotx;
			tmpy = y - roty;

			for (int i = 0; i < 4; i++)
			{
				rT[i] = rT[i] * CV_PI / 180.0;
				revisionData[i][0] = (tmpx * cos(rT[i]) - tmpy * sin(rT[i]) + rotx);
				revisionData[i][1] = (tmpx * sin(rT[i]) + tmpy * cos(rT[i]) + roty);

				calibData[4 * i + 0] = LOWORD((long)(revisionData[i][0] * MOTOR_SCALE));
				calibData[4 * i + 1] = HIWORD((long)(revisionData[i][0] * MOTOR_SCALE));
				calibData[4 * i + 2] = LOWORD((long)(revisionData[i][1] * MOTOR_SCALE));
				calibData[4 * i + 3] = HIWORD((long)(revisionData[i][1] * MOTOR_SCALE));
			}

			g_CommPLC.SetWord(address4, 16, calibData);
			g_CommPLC.SetBit(address3, TRUE);
			m_bSendOffsetFlag[nJob] = TRUE;
		}
		else
		{
			if (g_CommPLC.GetBit(address) == FALSE && m_bSendOffsetFlag[nJob])
			{
				g_CommPLC.SetBit(address3, FALSE);
				m_bSendOffsetFlag[nJob] = FALSE;
			}
		}
	}
}

int CFormMainView::do_model_process(int mode)
{
	CString strTemp;
	int nRet = 0;

	// 1 : create
	// 2 : change
	// 3 : modify
	// 4 : delete
	// 6 : PLC Parameter Change
	char cCurrModel[MAX_PATH] = { 0, };
	char cTargetModel[MAX_PATH] = { 0, };

	long model_proc_type = 0;
	long lData[60] = { 0, };
	g_CommPLC.GetWord(m_pMain->m_nStartReadWordAddr, 60, lData);

	model_proc_type = mode + 2;

	//KJH 2021-07-20 6이머냐.....
	if (model_proc_type == 6)
	{
		strTemp.Format("[PLC MODEL] Process Type : PLC Parameter Change", model_proc_type);
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);
		//nRet = ERR_MODEL_TYPE_ERROR;
		return nRet;
	}
	
	if (model_proc_type < 1 || model_proc_type > 4)
	{
		strTemp.Format("[PLC MODEL] process type : %d", model_proc_type);
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);
		//nRet = ERR_MODEL_TYPE_ERROR;
		return nRet;
	}

	if (model_proc_type == 1)	strTemp = "[PLC MODEL] Process Type : Create";
	if (model_proc_type == 2)	strTemp = "[PLC MODEL] Process Type : Change";
	if (model_proc_type == 3)	strTemp = "[PLC MODEL] Process Type : Modify";
	if (model_proc_type == 4)	strTemp = "[PLC MODEL] Process Type : Delete";
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);


	int off = 20;
	// Current Model
	for (int i = 0; i < 10; i++)
	{
		cCurrModel[i * 2 + 0] = (lData[off + i] >> 0) & 0xFF;
		cCurrModel[i * 2 + 1] = (lData[off + i] >> 8) & 0xFF;
	}

	for (int i = 0; i < 20; i++)
	{
		if (cCurrModel[i] == 32)		cCurrModel[i] = '0';
		else if (cCurrModel[i] == 10)	cCurrModel[i] = '_';
	}

	// Target Model
	off = 40;
	for (int i = 0; i < 10; i++)
	{
		cTargetModel[i * 2 + 0] = (lData[off + i] >> 0) & 0xFF;
		cTargetModel[i * 2 + 1] = (lData[off + i] >> 8) & 0xFF;
	}

	for (int i = 0; i < 20; i++)
	{
		if (cTargetModel[i] == 32)			cTargetModel[i] = '0';
		else if (cTargetModel[i] == 10)		cTargetModel[i] = '_';
	}

	strTemp.Format("[PLC MODEL] Curr Model : %s", cCurrModel);
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);

	strTemp.Format("[PLC MODEL] Target Model : %s", cTargetModel);
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);

	CString strCurrModel, strTargetModel;
	strCurrModel.Format("%s", cCurrModel);
	strTargetModel.Format("%s", cTargetModel);

	switch (model_proc_type) {
	case 1:	nRet = model_process_create(strCurrModel, strTargetModel);	break;
	case 2:	nRet = model_process_change(strCurrModel, strTargetModel);	break;
	case 3:	nRet = model_process_modify(strCurrModel, strTargetModel);	break;
	case 4:	nRet = model_process_delete(strCurrModel, strTargetModel);	break;
	}
	return nRet;
}

// KBJ 2022-09-22 모델 프로세스 2022버젼 추가
int CFormMainView::do_model_process_2022(int mode)
{
	CString strTemp;
	int nRet = 0;

	const int MODEL_CREATE = 0;
	const int MODEL_CHANGE = 1;
	const int MODEL_MODIFY = 2;
	const int MODEL_DELETE = 3;
	
	char cCurrModel[MAX_PATH] = { 0, };
	char cTargetModel[MAX_PATH] = { 0, };

	int nRead_ID_Word_Size = 10;
	long lData[40] = { 0, };
	g_CommPLC.GetWord(m_pMain->model_process_addres.PLC_WD_PLC_MODEL_CURRENT_ID, 40, lData);

	if (mode == MODEL_CREATE)	strTemp = "[PLC MODEL] Process Type : Create";
	if (mode == MODEL_CHANGE)	strTemp = "[PLC MODEL] Process Type : Change";
	if (mode == MODEL_MODIFY)	strTemp = "[PLC MODEL] Process Type : Modify";
	if (mode == MODEL_DELETE)	strTemp = "[PLC MODEL] Process Type : Delete";
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);


	int off = 0;
	// Current Model
	for (int i = 0; i < nRead_ID_Word_Size; i++)
	{
		cCurrModel[i * 2 + 0] = (lData[off + i] >> 0) & 0xFF;
		cCurrModel[i * 2 + 1] = (lData[off + i] >> 8) & 0xFF;
	}

	for (int i = 0; i < nRead_ID_Word_Size*2; i++)
	{
		if (cCurrModel[i] == 32)		cCurrModel[i] = '0';
		else if (cCurrModel[i] == 10)	cCurrModel[i] = '_';
	}

	// Target Model
	off = 10;
	for (int i = 0; i < nRead_ID_Word_Size; i++)
	{
		cTargetModel[i * 2 + 0] = (lData[off + i] >> 0) & 0xFF;
		cTargetModel[i * 2 + 1] = (lData[off + i] >> 8) & 0xFF;
	}

	for (int i = 0; i < nRead_ID_Word_Size*2; i++)
	{
		if (cTargetModel[i] == 32)			cTargetModel[i] = '0';
		else if (cTargetModel[i] == 10)		cTargetModel[i] = '_';
	}

	strTemp.Format("[PLC MODEL] Curr Model : %s", cCurrModel);
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);

	strTemp.Format("[PLC MODEL] Target Model : %s", cTargetModel);
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);

	CString strCurrModel, strTargetModel;
	strCurrModel.Format("%s", cCurrModel);
	strTargetModel.Format("%s", cTargetModel);

	switch (mode) {
	case 0:	nRet = model_process_create(strCurrModel, strTargetModel);	break;
	case 1:	nRet = model_process_change(strCurrModel, strTargetModel);	break;
	case 2:	nRet = model_process_modify(strCurrModel, strTargetModel);	break;
	case 3:	nRet = model_process_delete(strCurrModel, strTargetModel);	break;
	}

	m_pMain->changeForm(FORM_MAIN);
	m_pMain->changePane(PANE_AUTO);

	return nRet;
}

int CFormMainView::model_process_create(CString strCurrModel, CString strTargetModel)
{
	int nRet = ERR_NOTHING;
	CString strTemp;
	CString currPath;
	CString targetPath;

	currPath.Format("%s%s", m_pMain->m_strModelDir, strCurrModel);
	targetPath.Format("%s%s", m_pMain->m_strModelDir, strTargetModel);

	if (_access(currPath, 0) != 0)
	{
		strTemp.Format("[PLC MODEL] %s Current Model is not Exist!!!", strCurrModel);
		::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);
		nRet = ERR_CREATE_CURR_MODEL_NOT_EXIST;
		return nRet;
	}

	/*
	if (strCurrModel != m_pMain->getModel().getModelID())
	{
		strTemp.Format("[PLC MODEL] %s Model is not Match!!!", strCurrModel);
		::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);
		nRet = ERR_CURRENT_MODEL_NOT_MATCH;
		return nRet;
	}
	*/

	if (_access(targetPath, 0) == 0)
	{
		//strTemp.Format("[PLC MODEL] %s Target Model is Already Exist!!!", strTargetModel);
		//::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);
		//	nRet = ERR_CREATE_TARGET_MODEL_EXIST;		// Target 모델 있어도 ERROR 없음 진행
		return nRet;
	}

	CString dstModel = m_pMain->m_strModelDir + strTargetModel;
	if (_access(dstModel, 0) != 0)			CreateDirectory(dstModel, 0);

	CString strModel = strCurrModel;
	CString strCopyModelPath, str;

	strCopyModelPath.Format("%s%s", m_pMain->m_strModelDir, strModel);

	if (m_pMain->process_model_copy(strModel, strTargetModel) == FALSE)
	{
		strTemp.Format("[PLC MODEL] %s Model Copy NG", strTargetModel);
		::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);
	}
	else
	{
		strTemp.Format("[PLC MODEL] %s Model Copy Complete", strTargetModel);
		::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);
	}
	
	::SendMessageA(m_pMain->m_pForm[FORM_MODEL]->m_hWnd, WM_VIEW_CONTROL, MSG_MV_DISP_MODEL_LIST, 0);

	return nRet;
}
int CFormMainView::model_process_change(CString strCurrModel, CString strTargetModel)
{
	int nRet = 0;
	CString strTemp;
	CString currPath;
	CString targetPath;

	currPath.Format("%s%s", m_pMain->m_strModelDir, strCurrModel);
	targetPath.Format("%s%s", m_pMain->m_strModelDir, strTargetModel);

	if (_access(targetPath, 0) != 0)
	{
		strTemp.Format("[PLC MODEL] %s Model is not Exist!!!", strTargetModel);
		::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);
		nRet = ERR_CHANGE_TARGET_MODEL_NOT_EXIST;
		return nRet;
	}

	CFormModelView* pModelView = (CFormModelView*)m_pMain->m_pForm[FORM_MODEL];
	if (m_pMain->allModelChange(strTargetModel) != TRUE)
	{
		m_pMain->fnSetMessage(1, "Model Change Failed");
		strTemp.Format("[PLC MODEL] %s allModelChange Failed!!!", strTargetModel);
		::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);
		return nRet;
	}
	else
	{
		strTemp.Format("[PLC MODEL] %s allModelChange Complete", strTargetModel);
		::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);
	}
	::SendMessageA(theApp.m_pFrame->m_pForm[FORM_MODEL]->m_hWnd, WM_VIEW_CONTROL, MSG_MV_DISP_MODEL_INFO, (LPARAM)&strTargetModel);


	return nRet;
}
int CFormMainView::model_process_modify(CString strCurrModel, CString strTargetModel)
{
	int nRet = 0;

	CString strTemp;
	CString currPath;
	CString targetPath;

	currPath.Format("%s%s", m_pMain->m_strModelDir, strCurrModel);
	targetPath.Format("%s%s", m_pMain->m_strModelDir, strTargetModel);

	if (_access(currPath, 0) != 0)
	{
		strTemp.Format("[PLC MODEL] %s Model is not Exist!!!", strCurrModel);
		::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);
		nRet = ERR_MODIFY_CURR_MODEL_NOT_EXIST;
		return nRet;
	}

	if (strCurrModel != m_pMain->getCurrModelName())
	{
		strTemp.Format("[PLC MODEL] PLC Model : %s ", strCurrModel);
		::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);
		strTemp.Format("[PLC MODEL] PLC Model : %s ", m_pMain->getCurrModelName());
		::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);
		strTemp.Format("[PLC MODEL] PLC Model != PC Model Not Match ", strCurrModel, m_pMain->getCurrModelName());
		::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);
		nRet = ERR_MODIFY_CURR_MODEL_NOT_MATCHING;
		return nRet;
	}

	//double panel_x = 0.0;
	//double panel_y = 0.0;
	//double p_film_x = 0.0;
	//double p_film_y = 0.0;
	//double dic_x = 0.0;
	//double dic_y = 0.0;
	//double p_film_to_mark_distance = 0.0;
	//double mark_pitch = 0.0;

	//long lData[100] = { 0, };
	//g_CommPLC.GetWord(PLC_RD_MODEL_DATA, 100, lData);

	//panel_x = MAKELPARAM(lData[0], lData[1]) / 10000.0;			// DIC에서는 A/A		BEND CGS
	//panel_y = MAKELPARAM(lData[2], lData[3]) / 10000.0;
	//p_film_x = MAKELPARAM(lData[4], lData[5]) / 10000.0;
	//p_film_y = MAKELPARAM(lData[6], lData[7]) / 10000.0;
	//dic_x = MAKELPARAM(lData[8], lData[9]) / 10000.0;
	//dic_y = MAKELPARAM(lData[10], lData[11]) / 10000.0;
	//p_film_to_mark_distance = MAKELPARAM(lData[12], lData[13]) / 10000.0;
	//mark_pitch = MAKELPARAM(lData[14], lData[15]) / 10000.0;

	//strTemp.Format("[PLC MODEL] panel_x : %.3f", panel_x);
	//::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);

	//strTemp.Format("[PLC MODEL] panel_y : %.3f", panel_y);
	//::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);

	//strTemp.Format("[PLC MODEL] p_film_x : %.3f", p_film_x);
	//::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);

	//strTemp.Format("[PLC MODEL] p_film_y : %.3f", p_film_y);
	//::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);

	//strTemp.Format("[PLC MODEL] dic_x : %.3f", dic_x);
	//::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);

	//strTemp.Format("[PLC MODEL] dic_y : %.3f", dic_y);
	//::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);

	//strTemp.Format("[PLC MODEL] p_film_to_mark_distance : %.3f", p_film_to_mark_distance);
	//::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);

	//strTemp.Format("[PLC MODEL] mark_pitch : %.3f", mark_pitch);
	//::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);

	//strTemp.Format("[PLC MODEL] %s Model Modify Complete", strTargetModel);
	//::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);



	return nRet;
}
int CFormMainView::model_process_delete(CString strCurrModel, CString strTargetModel)
{
	int nRet = ERR_NOTHING;

	CString strTemp;
	CString currPath;
	CString targetPath;

	currPath.Format("%s%s", m_pMain->m_strModelDir, strCurrModel);
	targetPath.Format("%s%s", m_pMain->m_strModelDir, strTargetModel);

	if (_access(targetPath, 0) != 0)
	{
		strTemp.Format("[PLC MODEL] %s Model is Not Exist!!!", strTargetModel);
		::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);
		//	nRet = ERR_DELETE_TARGET_MODEL_NOT_EXIST;
		// 삭제 하려는 모델이 PC에 없을 경우 그냥 ACK로 협의 20.08.27
		return nRet;
	}

	if (strCurrModel == strTargetModel)
	{
		strTemp.Format("[PLC MODEL] Current Model and Target Model is Same!!!");
		::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);
		nRet = ERR_DELETE_CURR_TARGET_MODEL_MATCHING;
		return nRet;
	}

	::SendMessageA(m_pMain->m_pForm[FORM_MODEL]->m_hWnd, WM_VIEW_CONTROL, MSG_MV_DELETE_MODEL, (LPARAM)&strTargetModel);		// 서버의 모델을 삭제

	strTemp.Format("[PLC MODEL] %s Model Delete Complete", strTargetModel);
	::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);

	::SendMessageA(m_pMain->m_pForm[FORM_MODEL]->m_hWnd, WM_VIEW_CONTROL, MSG_MV_DISP_MODEL_LIST, 0);

	return nRet;
}

void CFormMainView::fnProcess_PLCTime()
{
	CString strTemp;

	if (g_CommPLC.GetBit(PLC_RB_PLC_TIME_REQ) && m_bPLCTimeStart != TRUE)
	{
		m_bPLCTimeStart = TRUE;

		strTemp.Format("[PLC TIME] Process Start");
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);

		m_pMain->initPLC_Time();
		g_CommPLC.SetBit(PLC_WB_PLC_TIME_ACK, TRUE);
	}

	if (g_CommPLC.GetBit(PLC_RB_PLC_TIME_REQ) != TRUE && m_bPLCTimeStart)
	{
		m_bPLCTimeStart = FALSE;

		strTemp.Format("[PLC TIME] Process End");
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);

		g_CommPLC.SetBit(PLC_WB_PLC_TIME_ACK, FALSE);
	}
}
bool CFormMainView::fnGrabSingleImage(int nJob)
{
#ifndef JOB_INFO
	int camCount = m_pAlgorithmInfo.getAlgorithmCameraNum(nJob);
	std::vector<int> camBuf = m_pAlgorithmInfo.getAlgorithmCameraIndex(nJob);
#else
	int camCount = m_pMain->vt_job_info[nJob].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[nJob].camera_index;
#endif

	CString str;
	int ncamera = 0;
	bool bReturn = true;

	ncamera = camBuf.at(0);
#ifdef _DAHUHA
	AcquireSRWLockExclusive(&m_pMain->m_pDahuhaCam[ncamera].g_bufsrwlock); // 동기화 시킴 Tkyuha 20211027
#else
	AcquireSRWLockExclusive(&m_pMain->m_pBaslerCam[ncamera]->g_bufsrwlock); // 동기화 시킴 Tkyuha 20211027
#endif
	m_pMain->m_bGrabEnd[ncamera] = FALSE;
#ifdef _DAHUHA
	ReleaseSRWLockExclusive(&m_pMain->m_pDahuhaCam[ncamera].g_bufsrwlock);
#else
	ReleaseSRWLockExclusive(&m_pMain->m_pBaslerCam[ncamera]->g_bufsrwlock);
#endif
#ifdef _DAHUHA
	if (m_pMain->m_pDahuhaCam[ncamera].isconnected())
#else
	if (m_pMain->m_pBaslerCam[ncamera] || m_pMain->m_pSaperaCam[ncamera].IsOpend())
#endif
	{
		str.Format("Cam %d - Grab Start", ncamera + 1);
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		m_pMain->SendMessage(WM_VIEW_CONTROL, MSG_MD_CAMERA_SINGLE_FRAME, ncamera);
		seq_SetTimer(2000);

		BOOL bGrabComplete = TRUE;

		do
		{
			bGrabComplete = TRUE;
			if (m_pMain->m_bGrabEnd[ncamera] != TRUE)
				bGrabComplete = FALSE;
			Sleep(0);
		} while (!seq_ChkTimer() && bGrabComplete == FALSE);

		if (bGrabComplete)		str = "Grab Complete";
		else
		{
			str = "Grab TimeOut";
			bReturn = false;
		}

		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}
	else bReturn = false;

	return bReturn;
}
bool CFormMainView::VerifyReCalc(void* rawData, double rX, double rY)
{
	bool r_value = true;

	TPointList5D rawData5D = *(TPointList5D*)rawData;

	int rawDataSize = int(rawData5D.size());
	int nSize = rawDataSize;
	int nc = 0, maxpos = -1, ns = -1, ne = -1, c = 0;
	int lc = 0, rc = 0, totalc = 0, _offset = 0;
	double maxv = -999999, astep = 1.0, cangle = 90;
	double dbAlignX, dbAlignY;
	// 최대 각도 지점 하고 각이 0도가 아닌 지점을 찾아서 포인트 갯수를 확인
	// ns는 0이 아닌 각도 시작 지점
	// ne는 0이 아닌 각도 end 지점
	for (int i = 0; i < nSize; i++)
	{
		if (rawData5D[i].at(2) != 0)
		{
			if (ns == -1) ns = i;
			ne = i;
			nc++;
		}
		if (maxv < fabs(rawData5D[i].at(2)))
		{
			maxv = fabs(rawData5D[i].at(2));
			maxpos = i;
		}
	}
	if (ne == nSize - 1 || ns == 0 || ne == -1) return false; //20210915 Tkyuha 예외 처리
	if (ns == 1) _offset = 1;

	if (maxpos != -1)
	{
		lc = maxpos - ns;
		rc = ne - maxpos;

		totalc = MAX(1, lc);
		astep = 90.0 / totalc;

		for (int i = 0; i < 3; i++)			m_pMain->m_ELB_TraceResult.m_vTraceProfile[i].clear();

		//for (int i = 0; i < ns; i++) // Left 시작 위치
		//{
		//	if (i < 0 || i >= rawDataSize) continue;
		//	m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].push_back(-rawData5D[i].at(3));
		//	m_pMain->m_ELB_TraceResult.m_vTraceProfile[1].push_back(rawData5D[i].at(4));
		//	m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].push_back(0);
		//}

		//for (int i = maxpos - totalc; i < maxpos; i++) // 중심에서 왼쪽 채우기 7개
		//{
		//	if (i < 0 || i >= rawDataSize) continue;
		//	cangle = (sin(ToRadian(-90.- rawData5D[ns].at(2) + (180. * double(c+ _offset)) / totalc)) + 1.) / 2. * rawData5D[maxpos].at(2);

		//	calcRotate(rX, rY, rawData5D[i].at(0), rawData5D[i].at(1), (-cangle) * CV_PI / 180.0, &dbAlignX, &dbAlignY);
		//	m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].push_back(-dbAlignX);
		//	m_pMain->m_ELB_TraceResult.m_vTraceProfile[1].push_back(dbAlignY);
		//	m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].push_back(cangle);
		//	c++;
		//}

		//m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].push_back(-rawData5D[maxpos].at(3));
		//m_pMain->m_ELB_TraceResult.m_vTraceProfile[1].push_back(rawData5D[maxpos].at(4));
		//m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].push_back(rawData5D[maxpos].at(2));

		for (int i = 0; i <= maxpos; i++) // Left 시작 위치  각도가 0인 경우는 이전값 그래도 사용
		{
			if (i < 0 || i >= rawDataSize) continue;
			m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].push_back(-rawData5D[i].at(3));
			m_pMain->m_ELB_TraceResult.m_vTraceProfile[1].push_back(rawData5D[i].at(4));
			m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].push_back(rawData5D[i].at(2));
		}
		c = rc - 1;
		totalc = MAX(1, rc);

		for (int i = maxpos + 1; i < maxpos + totalc + 1; i++) // 중심에서 오른쪽 채우기 Sin 곡선으로 완만 하게 데이타 생성
		{
			if (i < 0 || i >= rawDataSize) continue;
			cangle = (sin(ToRadian(-90. - rawData5D[ne].at(2) + (180. * double(c)) / totalc)) + 1.) / 2. * rawData5D[maxpos].at(2);

			calcRotate(rX, rY, rawData5D[i].at(0), rawData5D[i].at(1), (-cangle) * CV_PI / 180.0, &dbAlignX, &dbAlignY);
			m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].push_back(-dbAlignX);
			m_pMain->m_ELB_TraceResult.m_vTraceProfile[1].push_back(dbAlignY);
			m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].push_back(cangle);
			c--;
		}

		for (int i = maxpos + totalc + 1; i < nSize; i++) // Right 시작 위치  각도가 0인경우 그대로 이전값 사용
		{
			if (i < 0 || i >= rawDataSize) continue;
			m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].push_back(-rawData5D[i].at(3));
			m_pMain->m_ELB_TraceResult.m_vTraceProfile[1].push_back(rawData5D[i].at(4));
			m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].push_back(0);
		}
	}

	return r_value;
}

bool CFormMainView::fnExtractMetalProfileData(int nJob, CViewerEx* mViewer)
{
	bool bReturn = true;
	int camCount = m_pMain->vt_job_info[nJob].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[nJob].camera_index;

	CString str;
	int ncamera = camBuf.at(0);
	int W = m_pMain->m_stCamInfo[ncamera].w;
	int H = m_pMain->m_stCamInfo[ncamera].h;

	str.Format("[%s] Extract Metal Profile Data Start", m_pMain->vt_job_info[nJob].job_name.c_str());
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

	m_pMain->copyMemory(m_pMain->getProcBuffer(ncamera, 0), m_pMain->getSrcBuffer(ncamera), W * H);

	double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(0, 0);
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);
	int x = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeX() / xres / 2; //  중심점 기준 좌우 분리
	int y = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeY() / yres / 2;  //  중심점 기준 상하 분리
	double l = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeLength(); //도포 길이
	int cx = W / 2, cy = H / 2;
	double lMax = MIN(14.0, l);
	int offset = lMax / xres / 2;

	m_thInspParam.nCam = ncamera;
	m_thInspParam.nJob = nJob;
	m_thInspParam.ptStart = CPoint(MAX(0, cx - offset), cy - y);
	m_thInspParam.ptEnd = CPoint(MIN(W - 1, cx + offset), cy + y);
	m_thInspParam.pDlg = m_pMain;
	m_thInspParam.mViewer = mViewer;
	AfxBeginThread(Thread_SubInspection, &m_thInspParam); //Metal Edge 검사
	AfxBeginThread(Thread_SubBMInspection, &m_thInspParam); //BM Edge 검사
	
	cv::Mat img(H, W, CV_8UC1, m_pMain->getProcBuffer(ncamera, 0));
	if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getUseInputInspection()) // 20220110 이물검사를 Metal Edge 검사와 동시에 수행
		fnPreDustInspection(nJob, 0, img.data, mViewer);

	return bReturn;
}

bool CFormMainView::fnExtractProfileData(int nJob, CViewerEx* mViewer, bool bclr, BOOL m_bManual)
{
#ifndef JOB_INFO
	int camCount = m_pAlgorithmInfo.getAlgorithmCameraNum(nJob);
	std::vector<int> camBuf = m_pAlgorithmInfo.getAlgorithmCameraIndex(nJob);
#else
	int camCount = m_pMain->vt_job_info[nJob].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[nJob].camera_index;
#endif

	CString str;
	int ncamera = camBuf.at(0);
	bool bReturn = true;
	int _TextGap = 0;
	int W = m_pMain->m_stCamInfo[ncamera].w;
	int H = m_pMain->m_stCamInfo[ncamera].h;
	double posx = 0.0, posy = 0.0;
	double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(0, 0);
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);
	int x = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeX() / xres / 2; //  중심점 기준 좌우 분리
	int y = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeY() / yres / 2;  //  중심점 기준 상하 분리
	double l = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeLength(); //도포 길이
	int cx = W / 2, cy = H / 2;

	str.Format("[%s] Extract Profile Data Start", m_pMain->vt_job_info[nJob].job_name.c_str());
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	
	if (m_bManual)
	{	
		m_pMain->copyMemory(m_pMain->getProcBuffer(ncamera, 0), m_pMain->getSrcBuffer(ncamera), W * H);
		m_pMain->copyMemory(m_pMain->getProcBuffer(ncamera, 2), m_pMain->getSrcBuffer(ncamera), W * H);
	}
	
	//method = Line , Circle, Notch
	int method = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod();

	//KJH 2022-08-04 Camera Buffer 변경
	cv::Mat img(H, W, CV_8UC1, m_pMain->getProcBuffer(ncamera, 2));
	cv::Mat img2(H, W, CV_8UC1, m_pMain->getSrcBuffer(ncamera));

	FakeDC* pDC = NULL;
	int OLD_BKMODE = 0;

	//KJH 2021-05-28 두번째 Grab 화면에 표시
	if (mViewer != NULL)
	{
		//KJH 2021-06-05 두번째 Grab시 화면 지우고 Trace만 Display
		if (bclr)
		{
			mViewer->ClearOverlayDC();
			mViewer->clearAllFigures();
		}
		
		//KJH 2022-08-04 Camera Buffer 변경
		mViewer->OnLoadImageFromPtr(m_pMain->getProcBuffer(ncamera, 2));
		pDC = mViewer->getOverlayDC();
		pDC->SetTextColor(COLOR_RED);
		OLD_BKMODE = pDC->SetBkMode(TRANSPARENT);
	}

	switch (method)
	{
	case  METHOD_LINE:
	{
		InspLineEdgeDetection(nJob, img, CPoint(cx - x, cy - y), CPoint(cx + x, cy + y), l, mViewer, true);
		
		//KJH 2021-12-07 Trace 이후 자재 검사 ExtractProfileData에 옴기기 작업 시작
		double spec = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(0);
		double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);

		spec = spec / yres;
		//KBJ 2022-11-30 Trace Y 스펙 아웃 알림창 및 오버레이 추가
		bool bCenterDistanceEnable = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getCenterDistanceEnable();
		if (bCenterDistanceEnable && fabs(H / 2 - m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y) > spec)
		{
			bReturn = FALSE;
			str.Format("Line Center Error - %.3fmm \nspec : %.3fmm", fabs(H / 2 - m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y) * yres, spec * yres);
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

			if (pDC != NULL) pDC->TextOutA(1000, 1000, str);
			ShowErrorMessageBlink(str);
		}

		int cBright = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInkCheckThresh(0);
		bool bDispensingStatusEnable = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getDispensingStatusEnable();

		if (bDispensingStatusEnable && m_dCircleInBright < cBright)
		{
			bReturn = FALSE;
			str.Format("Line Brightness Error - %.3f gray( %.3f)", m_pMain->vt_job_info[nJob].job_name.c_str(), m_dCircleInBright, cBright);
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

			if (pDC != NULL) pDC->TextOutA(1000, 1100, str);
			ShowErrorMessageBlink(str);
		}

		break;
	}
	case  METHOD_CIRCLE:
	{
		//Trace 데이타 추출
		bool _btraceusingAlign = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getTraceUsingAlignFlag(); // 얼라인 사용 해서 Trace 계산
		if(_btraceusingAlign) InspNotchEdgeDetection_jmLee(false, true, nJob, ncamera, CPoint(cx - x, cy - y), CPoint(cx + x, cy + y), mViewer, false);
		else InspNotchEdgeDetection(false, true, nJob, ncamera, CPoint(cx - x, cy - y), CPoint(cx + x, cy + y), mViewer, false, bclr);
		
		//KJH 2021-09-13 Trace Image LOG 만들지도 않았다..
		//m_thInspParam.nCam = ncamera;
		//m_thInspParam.nJob = nJob;
		//m_thInspParam.pDlg = m_pMain;
		//AfxBeginThread(Thread_InspTraceImageSave, &m_thInspParam); //SAVE
		
		//KJH 2022-07-01 Trace NG PopUp 분기 추가
		m_bCheckState = CheckStatus(nJob, W, H, pDC) ? true : false;
		bReturn = m_bCheckState;
	}
	break;
	case  METHOD_NOTCH:
	{
		double lMax = MIN(18.0, l);
		int offset_l = (m_dLeftNotchLength + m_pMain->m_nCurrentNotchDec) / xres;
		int offset_r = (m_dRightNotchLength + m_pMain->m_nCurrentNotchAcc) / xres;
		if (offset_l == 0) offset_l = l / xres / 2;
		if (offset_r == 0) offset_r = l / xres / 2;

		InspNotchEdgeDetection(true, false, nJob, ncamera, CPoint(MAX(0, cx - offset_l), cy - y), CPoint(MIN(W - 1, cx + offset_r), cy + y), mViewer);
		if (m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].size() > 2)
		{
			if (m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].at(m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].size() - 1) -
				m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].at(m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].size() - 2) != 0)

			{
				bReturn = false;
				str.Format("[%s] Extract Profile NG", m_pMain->vt_job_info[nJob].job_name.c_str());
				::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
			}

		}
		double spec = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(0);
		double ydist = fabs(m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.y + m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.y);
		double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(0, 0);

		if (ydist > spec)
		{
			bReturn = FALSE;
			str.Format("[%s] Notch Center Error - %.3fmm , %.3fmm(Spec)", m_pMain->vt_job_info[nJob].job_name.c_str(), ydist, spec);
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
			_TextGap = _TextGap + 100;
			pDC->TextOutA(50, _TextGap, str, str.GetLength());
		}

		spec = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(0);

		if (m_pMain->m_vtTraceData.size() > 0)
		{
			int W = m_pMain->m_stCamInfo[ncamera].w;
			int H = m_pMain->m_stCamInfo[ncamera].h;
			double minsum = 99999, s_1 = 0;

			CPointF<int> p10(W / 2, H / 2);

			for (int i = 0; i < m_pMain->m_vtTraceData.size(); i++)
			{
				CPointF<int> p11(m_pMain->m_vtTraceData.at(i).x, m_pMain->m_vtTraceData.at(i).y);
				s_1 = GetDistance(p10, p11);
				if (s_1 < minsum) minsum = s_1;
			}

			if (minsum * xres > spec)
			{
				bReturn = FALSE;
				str.Format("[%s] Notch Center X Error - %.3fmm , %.3fmm(Spec)", m_pMain->vt_job_info[nJob].job_name.c_str(), minsum * xres, spec);
				::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
				_TextGap = _TextGap + 100;
				pDC->TextOutA(50, _TextGap, str, str.GetLength());
			}
		}

		if (m_dCircleInBright < 60.)
		{
			bReturn = FALSE;
			str.Format("[%s] Notch Brightness Error - %.3f gray", m_pMain->vt_job_info[nJob].job_name.c_str(), m_dCircleInBright);
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
			_TextGap = _TextGap + 100;
			pDC->TextOutA(50, _TextGap, str, str.GetLength());
		}
		//	InspNotchEdgeDetection_jmLee(true, false, nJob, ncamera, CPoint(MAX(0, cx - offset), cy - y), CPoint(MIN(W - 1, cx + offset), cy + y), mViewer);
	}
	break;
	case  METHOD_ELLIPSE: break;
	}

	str.Format("[%s] Extract Profile Data End", m_pMain->vt_job_info[nJob].job_name.c_str());
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

	if (mViewer != NULL)
	{
		mViewer->Invalidate();
		mViewer->SoftDirtyRefresh();
	}

	//KJH 2021-12-07 Trace Image & 자재 검사 관련 이미지 로그 추가 (Tact 문제 생기면 쓰레드 처리 필요)
#pragma region Image Log 저장
	//////////////////////////////////////////////
	//20210923 Tkyuha Trace 이미지 오늘 날짜에 저장
	//////////////////////////////////////////////
	system_clock::time_point start = system_clock::now();

	CString imgMsg, str_modelID, str_JobName, strMsg, imgMsgRaw,str_ImageType;

	CString strFileDir_Image, strFileDir_Image_Raw;
	CString strFileDir_Files;

	SYSTEMTIME time;
	GetLocalTime(&time);
	//KMB 220924 SYSTEM OPTION에 따라 저장이미지 확장자 변경
	if (m_pMain->vt_system_option[nJob].save_image_type == FALSE)
		str_ImageType = "jpg";
	else
		str_ImageType = "bmp";

	CString Time_str = m_pMain->m_strResultTime[nJob];
	CString Date_str = m_pMain->m_strResultDate[nJob];

	if (Time_str == "" || Date_str == "")
	{
		CTime NowTime;
		NowTime = CTime::GetCurrentTime();
		m_pMain->m_strResultDate[nJob].Format("%04d%02d%02d", NowTime.GetYear(), NowTime.GetMonth(), NowTime.GetDay());
		m_pMain->m_strResultTime[nJob].Format("%02d_%02d_%02d", NowTime.GetHour(), NowTime.GetMinute(), NowTime.GetSecond());

		Time_str = m_pMain->m_strResultTime[nJob];
		Date_str = m_pMain->m_strResultDate[nJob];
	}

	CString Cell_Name = m_pMain->vt_job_info[nJob].get_main_object_id();

	if (strlen(Cell_Name) < 10)
	{
		Cell_Name.Format("TEST_%s", Time_str);
	}

	///////////////////////////////////// 이미지 저장 폴더
	///  D:\\ Result \\ Image \\ 날짜 \\ 모델ID \\ JOB이름 \\ Trace \\ 

	//Image 폴더 확인 후 생성
	if (_access(m_pMain->m_strImageDir, 0) != 0)	CreateDirectory(m_pMain->m_strImageDir, NULL);

	//날짜
	strFileDir_Image.Format("%s%s", m_pMain->m_strImageDir, Date_str);
	if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);

	//모델ID
	str_modelID.Format("%s", m_pMain->vt_job_info[nJob].model_info.getModelID());
	strFileDir_Image.Format("%s\\%s", strFileDir_Image, str_modelID);
	if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);

	//JOB이름
	str_JobName.Format("%s", m_pMain->vt_job_info[nJob].job_name.c_str());
	strFileDir_Image.Format("%s\\%s", strFileDir_Image, str_JobName);
	if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);

	//Trace 폴더 확인 후 생성
	strFileDir_Image.Format("%s\\Trace", strFileDir_Image);  // KBJ 2022-11-02
	if (_access(strFileDir_Image, 0) != 0)		CreateDirectory(strFileDir_Image, NULL);

	//OK,NG,Simulation
	if (m_pMain->m_bSimulationStart)
	{
		strFileDir_Image.Format("%s\\Simulation", strFileDir_Image);
		if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);
	}
	else
	{
		if (bReturn)
		{
			strFileDir_Image.Format("%s\\OK", strFileDir_Image);
			if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);
		}
		else
		{
			strFileDir_Image.Format("%s\\NG", strFileDir_Image);
			if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);
		}
	}

	//PanelID
	strFileDir_Image.Format("%s\\%s", strFileDir_Image, Cell_Name);
	if (_access(strFileDir_Image, 0) != 0) CreateDirectory(strFileDir_Image, NULL);
	//PanelID가 있는경우
	else
	{
		strFileDir_Image.Format("%s_%s", strFileDir_Image, Time_str);
		if (_access(strFileDir_Image, 0) != 0) CreateDirectory(strFileDir_Image, NULL);
	}

	imgMsg.Format("%s\\Trace_%s_%s_Img.%s", strFileDir_Image, Cell_Name, Time_str,str_ImageType);
	imgMsgRaw.Format("%s\\Trace_%s_%s_ImgRaw.%s", strFileDir_Image, Cell_Name, Time_str, str_ImageType);

	int real_cam = m_pMain->vt_job_info[nJob].camera_index[0];
	cv::Mat imgRaw(H, W, CV_8UC1, m_pMain->getProcBuffer(real_cam, 2));
	cv::imwrite(std::string(imgMsgRaw), imgRaw); // Kbj 2022-02-23 Trace RawImage save
	
	mViewer->saveScreenCapture(imgMsg);

	system_clock::time_point end = system_clock::now();
	nanoseconds nano = end - start;
	strMsg.Format("Trace Image Save Time : %f", nano.count() / 1000000.);
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strMsg);
	//20210923 Tkyuha Trace 이미지 오늘 날짜에 저장

	//m_thInspParam.nCam = ncamera;
	//m_thInspParam.nJob = nJob;
	//m_thInspParam.pDlg = m_pMain;
	//m_thInspParam.mViewer = mViewer;
	//AfxBeginThread(Thread_InspTraceImageSave, &m_thInspParam); //SAVE

	if (mViewer != NULL) mViewer->Invalidate();

#pragma endregion

	if(pDC != NULL) pDC->SetBkMode(OLD_BKMODE);

	return bReturn;
}

bool CFormMainView::fnLineDustInspection(BYTE* srcImg, int W, int H, int nJob, int nCam, int id, CRect roi, CViewerEx* mViewer)
{
	EnterCriticalSection(&m_pMain->m_csScanBurInsp);

	bool bReturn = true;
	int nDefect = 0;
	CString str;

	str.Format("Cam %d _%d- Dust Inspection Start", nCam, id);
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

	cv::Rect inspRoi = cv::Rect(roi.left, roi.top, roi.Width(), roi.Height());
	int method = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod();
	double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(0, 0);
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);
	int dThresh = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspDustThresh();

	cv::Mat imgSrc(H, W, CV_8UC1, srcImg);
	cv::Mat img, mask = GetRectmask(imgSrc, inspRoi);

	bitwise_not(mask, mask);

	inspRoi = cv::Rect(20, 20, W - 40, H - 40);
	imgSrc(inspRoi).copyTo(img, mask);

	cv::Mat element15(20, 20, CV_8U, cv::Scalar::all(1));
	cv::Mat s_down, close; // 축소 해서 검사 진행
	cv::resize(img, s_down, cv::Size(), 0.25, 0.25, CV_INTER_CUBIC);

	cv::threshold(s_down, s_down, dThresh, 255, CV_THRESH_BINARY_INV);

	morphologyEx(s_down, close, cv::MORPH_CLOSE, element15);
	morphologyEx(close, close, cv::MORPH_OPEN, element15);

	int spec_S = MAX(10, int(m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(1) / xres) / 4); //최소 크기
	int spec_A = MAX(250, int(m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(1) / xres) / 4); //최소 면적

	nDefect = InspDustBurrInspection(nJob, close, dThresh, inspRoi.x, inspRoi.y, 4, spec_S, spec_A, true); //10,25

	if (nDefect > 0)
	{
		for (int i = 0; i< int(m_vecDustResult.size()); i++)
		{
			m_vecDustResult.at(i).center.x = m_vecDustResult.at(i).center.x * 4 + inspRoi.x;
			m_vecDustResult.at(i).center.y = m_vecDustResult.at(i).center.y * 4 + inspRoi.y;
			m_vecDustResult.at(i).rr.center.x = m_vecDustResult.at(i).rr.center.x * 4 + inspRoi.x;
			m_vecDustResult.at(i).rr.center.y = m_vecDustResult.at(i).rr.center.y * 4 + inspRoi.y;
		}
	}
	img.release();

	if (nDefect > 0)
	{
		FakeDC* pDC = NULL;
		CString strText;
		int nview = m_pMain->vt_job_info[nJob].viewer_index[id];
		if (mViewer != NULL)		pDC = mViewer->getOverlayDC();
		else		pDC = m_pDlgViewerMain[nview]->GetViewer().getOverlayDC();

		CPen penNG(PS_SOLID, 5, COLOR_RED);
		CPen* pOldPen;
		CFont font, * oldFont;
		font.CreateFont(50, 50, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
			OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
		oldFont = pDC->SelectObject(&font);
		pDC->SelectObject(GetStockObject(NULL_BRUSH));
		pDC->SetTextColor(COLOR_RED);
		pOldPen = pDC->SelectObject(&penNG);

		for (int i = 0; i < nDefect; i++)
		{
			cv::RotatedRect rr = m_vecDustResult.at(i).rr;
			cv::Rect brect = rr.boundingRect();
			cv::Point2f vertices2f[4];
			rr.points(vertices2f);

			cv::Point vertices[4];
			for (int j = 0; j < 4; ++j) {
				vertices[j] = vertices2f[j];
			}

			pDC->MoveTo(vertices[0].x, vertices[0].y);
			pDC->LineTo(vertices[1].x, vertices[1].y);
			pDC->LineTo(vertices[2].x, vertices[2].y);
			pDC->LineTo(vertices[3].x, vertices[3].y);
			pDC->LineTo(vertices[0].x, vertices[0].y);

			strText.Format("[S: %.3f,%.3fmm, A: %.2f]", m_vecDustResult.at(i).xw * xres, m_vecDustResult.at(i).yw * yres, m_vecDustResult.at(i).area * 4);
			pDC->TextOutA(m_vecDustResult.at(i).center.x, m_vecDustResult.at(i).center.y, strText);
		}

		if (mViewer != NULL)				mViewer->Invalidate();
		else	m_pDlgViewerMain[nview]->GetViewer().Invalidate();

		penNG.DeleteObject();
		font.DeleteObject();

		bReturn = false;
	}

	if (theApp.m_bEnforceOkFlag) bReturn = true;

	LeaveCriticalSection(&m_pMain->m_csScanBurInsp);

	return bReturn;
}
bool CFormMainView::fnPreDustInspection(int nJob, int id, BYTE* ImgByte, CViewerEx* mViewer)
{
	bool bReturn = true;

	int camCount = m_pMain->vt_job_info[nJob].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[nJob].camera_index;

	CString str;
	int ncamera = camBuf.at(0);
	int W = m_pMain->m_stCamInfo[ncamera].w;
	int H = m_pMain->m_stCamInfo[ncamera].h;

	str.Format("[%s] Pre Dust Inspection Start", m_pMain->vt_job_info[nJob].job_name.c_str());
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

	cv::Rect inspRoi = cv::Rect(0, 0, W, H);

	int method		= m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod();
	double xres		= m_pMain->GetMachine(nJob).getCameraResolutionX(0, 0);
	double yres		= m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);
	int dThresh		= m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getExistDummyGray();

	cv::Mat imgSrc(H, W, CV_8UC1, ImgByte);
	cv::Mat img, mask;

	int nDefect = 0;

	if (method == METHOD_CIRCLE)
	{
		cv::Mat mask2 = GetCirclemask(imgSrc, m_ptCircleCenter_CC.x, m_ptCircleCenter_CC.y, m_dCircleRadius_CC);
		cv::Mat mask3 = GetCirclemask(imgSrc, m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x, m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y, CIRCLE_RADIUS);

		cv::subtract(mask3, mask2, mask);
		inspRoi = cv::Rect(m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x - CIRCLE_RADIUS, m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y - CIRCLE_RADIUS,
			CIRCLE_RADIUS * 2, CIRCLE_RADIUS * 2);

		mask2.release();
		mask3.release();
	}
	else if (method == METHOD_NOTCH)
	{
		mask = cv::Mat::zeros(H, W, CV_8UC1);

		double distanceT = -0.3 / xres;
		cv::Point2f rC;
		if (m_pMain->m_vtTraceData.size() > 0)
		{
			vector<cv::Point> vtContour;

			for (int i = 0; i < m_pMain->m_vtTraceData.size() - 1; i++)
			{
				rC = Equation_of_Lines(m_pMain->m_vtTraceData[i], m_pMain->m_vtTraceData[i + 1], distanceT);
				vtContour.insert(vtContour.begin(), cv::Point(int(rC.x), int(rC.y)));
			}
			distanceT = -0.1 / xres;
			for (int i = 0; i < m_pMain->m_vtTraceData.size() - 1; i++)
			{
				rC = Equation_of_Lines(m_pMain->m_vtTraceData[i], m_pMain->m_vtTraceData[i + 1], distanceT);
				vtContour.push_back(cv::Point(int(rC.x), int(rC.y)));
			}

			int num = (int)vtContour.size();
			const cv::Point* pt4 = &(vtContour[0]);
			cv::fillPoly(mask, &pt4, &num, 1, cv::Scalar(255, 255, 255), 8);

			inspRoi = cv::Rect(vtContour[num / 2 - 1].x, vtContour[0].y, vtContour[0].x - vtContour[num / 2 - 1].x, vtContour[num / 2].y - vtContour[0].y);

			inspRoi.x = MIN(W - 1, MAX(0, inspRoi.x));
			inspRoi.y = MIN(H - 1, MAX(0, inspRoi.y));
			inspRoi.width = MIN(W - inspRoi.x, MAX(0, inspRoi.width));
			inspRoi.height = MIN(H - inspRoi.y, MAX(0, inspRoi.height));
			vtContour.clear();
		}
	}
	else if (method == METHOD_LINE)
	{
		mask = cv::Mat::zeros(H, W, CV_8UC1);
	}

	//bitwise_not(mask, mask);

	imgSrc.copyTo(img, mask);

	//2022.10.04 ksm Inspection 이미지 사이즈 안맞으면 죽는현상 예외처리
	if (inspRoi.area() <= 0)
		return false;

	cv::Mat s_down; // 축소 해서 검사 진행
	cv::resize(img(inspRoi), s_down, cv::Size(), 0.5, 0.5, CV_INTER_CUBIC);

	int spec_S = MAX(10, int(m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(2) / xres) / 2); //최소 크기
	int spec_A = MAX(250, int(m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(2) / xres) / 2); //최소 면적

    m_vecDustResult.clear();
	nDefect = InspDustBurrInspection(nJob, s_down, dThresh, inspRoi.x, inspRoi.y, 2, spec_S, spec_A, true, true); //10,25

	if (nDefect > 0)
	{
		for (int i = 0; i< int(m_vecDustResult.size()); i++)
		{
			m_vecDustResult.at(i).center.x			= m_vecDustResult.at(i).center.x * 2 + inspRoi.x;
			m_vecDustResult.at(i).center.y			= m_vecDustResult.at(i).center.y * 2 + inspRoi.y;
			m_vecDustResult.at(i).rr.center.x		= m_vecDustResult.at(i).rr.center.x * 2 + inspRoi.x;
			m_vecDustResult.at(i).rr.center.y		= m_vecDustResult.at(i).rr.center.y * 2 + inspRoi.y;
			
			m_vecDustResult.at(i).rr.size.width		= m_vecDustResult.at(i).rr.size.width * 2;
			m_vecDustResult.at(i).rr.size.height	= m_vecDustResult.at(i).rr.size.height * 2;
			m_vecDustResult.at(i).xw				= m_vecDustResult.at(i).xw * 2;
			m_vecDustResult.at(i).yw				= m_vecDustResult.at(i).yw * 2;
		}
	}
	
	img.release();
	mask.release();
	s_down.release();
    imgSrc.release();

	if (nDefect > 0)
	{
		FakeDC* pDC = NULL;
		CString strText;
		int nview = m_pMain->vt_job_info[nJob].viewer_index[id];
		if (mViewer != NULL)		pDC = mViewer->getOverlayDC();
		else						pDC = m_pDlgViewerMain[nview]->GetViewer().getOverlayDC();

		CPen penNG(PS_SOLID, 5, COLOR_RED);
		CPen* pOldPen;
		CFont font, * oldFont;
		font.CreateFont(50, 50, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
			OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
		oldFont = pDC->SelectObject(&font);
		pDC->SelectObject(GetStockObject(NULL_BRUSH));
		pDC->SetTextColor(COLOR_RED);
		pOldPen = pDC->SelectObject(&penNG);

		for (int i = 0; i < nDefect; i++)
		{
			cv::RotatedRect rr = m_vecDustResult.at(i).rr;
			cv::Rect brect = rr.boundingRect();
			cv::Point2f vertices2f[4];
			rr.points(vertices2f);

			cv::Point vertices[4];
			for (int j = 0; j < 4; ++j) {
				vertices[j] = vertices2f[j];
			}

			pDC->MoveTo(vertices[0].x, vertices[0].y);
			pDC->LineTo(vertices[1].x, vertices[1].y);
			pDC->LineTo(vertices[2].x, vertices[2].y);
			pDC->LineTo(vertices[3].x, vertices[3].y);
			pDC->LineTo(vertices[0].x, vertices[0].y);

			if (fabs(m_vecDustResult.at(i).rr.angle) > 45)
			{
				strText.Format("[W: %.3f, H: %.3fmm, A: %.2f]", m_vecDustResult.at(i).yw * yres, m_vecDustResult.at(i).xw * xres, m_vecDustResult.at(i).area * 2);

			}
			else
			{
				strText.Format("[W: %.3f, H: %.3fmm, A: %.2f]", m_vecDustResult.at(i).xw * xres, m_vecDustResult.at(i).yw * yres, m_vecDustResult.at(i).area * 2);
			}
			pDC->TextOutA(m_vecDustResult.at(i).center.x, m_vecDustResult.at(i).center.y, strText);
		}

		if (mViewer != NULL)				mViewer->Invalidate();
		else								m_pDlgViewerMain[nview]->GetViewer().Invalidate();

		penNG.DeleteObject();
		font.DeleteObject();

		bReturn = false;
	}

	if (theApp.m_bEnforceOkFlag) bReturn = true;

	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->GetSafeHwnd(), WM_VIEW_CONTROL, MSG_PA_JUDGE_DISPLAY, MAKELPARAM(1, bReturn?TRUE:FALSE));

	return bReturn;
}

bool CFormMainView::fnCInk2_LackofInkInspection(int nJob, int id, BYTE* ImgByte, CViewerEx* mViewer)
{
	bool bReturn = true;

	int camCount = m_pMain->vt_job_info[nJob].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[nJob].camera_index;

	CString str;
	int ncamera = camBuf.at(0);
	int W = m_pMain->m_stCamInfo[ncamera].w;
	int H = m_pMain->m_stCamInfo[ncamera].h;

	str.Format("[%s] Pre Lack of Ink Inspection Start", m_pMain->vt_job_info[nJob].job_name.c_str());
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

	cv::Rect inspRoi = cv::Rect(0, 0, W, H);

	int method = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod();
	double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(0, 0);
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);
	int dThresh = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getCCFindFilter();

	inspRoi = cv::Rect(m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x - CIRCLE_RADIUS, m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y - CIRCLE_RADIUS,
		CIRCLE_RADIUS * 2, CIRCLE_RADIUS * 2);

	inspRoi.x = MIN(W - 1, MAX(0, inspRoi.x));
	inspRoi.y = MIN(H - 1, MAX(0, inspRoi.y));
	inspRoi.width = MIN(W - inspRoi.x, MAX(0, inspRoi.width));
	inspRoi.height = MIN(H - inspRoi.y, MAX(0, inspRoi.height));

	inspRoi.width = inspRoi.width / 8 * 8;
	inspRoi.height = inspRoi.height / 8 * 8;

	if (inspRoi.width <= 0 || inspRoi.height <= 0) return false;


	cv::Mat imgSrc(H, W, CV_8UC1, ImgByte);
	cv::Mat img, s_down;

	s_down = imgSrc(inspRoi).clone();

	cv::threshold(s_down, img, dThresh, 255, CV_THRESH_BINARY);
	cv::Mat maskerode = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3), cv::Point(1, 1));

	erode(img, img, maskerode, cv::Point(-1, -1), 2);
	dilate(img, img, maskerode, cv::Point(-1, -1), 2);

	// 디버깅 모드에서 이미지 보기
	if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
	{
		cvShowImage("LackofInk Org Image", s_down);
		cvShowImage("LackofInk bin Image", img);
	}

	uchar* OrgRaw = img.data;
	int ncol = inspRoi.width;
	int nrow = inspRoi.height;
	int __searchCount = 0;
	double __startDistance = 0;
	double _ptDistance = 0;
	bool _bfind = false;

	m_nLackOfInk_CinkCount = 0;

	CPointF<int> p_center(m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x - inspRoi.x, m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y - inspRoi.y);
	//double	CInkMinWidthSpec = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getCInkMinWidthSpec();
	double CInkMinWidthSpec = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInkUnderflowSpec();

	for (int _angle = 0; _angle < 360; _angle++)
	{
		__searchCount = 0;
		__startDistance = 0;

		m_bLackofInkCheckFlag[_angle] = false;
		_bfind = false;

		for (int _dist = 15; _dist < 300; _dist++)
		{
			int __bmX = (cos(ToRadian(_angle)) * (m_dCircleRadius_CC + _dist) + m_ptCircleCenter_CC.x - inspRoi.x); // CC Edge point
			int __bmY = (sin(ToRadian(_angle)) * (m_dCircleRadius_CC + _dist) + m_ptCircleCenter_CC.y - inspRoi.y); // CC Edge point

			if (__bmX > 0 && __bmX < ncol && __bmY>0 && __bmY < nrow)
			{
				if (_bfind==false && OrgRaw[__bmY * ncol + __bmX] < 100)
				{
					CPointF<int> __p2(__bmX, __bmY);
					_ptDistance = GetDistance(p_center, __p2);

					if (__searchCount > 20 && _ptDistance < CIRCLE_RADIUS) // 찾은 Edge Point 
					{
						_bfind = true;
					}
					__searchCount++;	
					if (__searchCount == 1)
					{
						__startDistance = _ptDistance;
					}
				}
				else if (_bfind==true && OrgRaw[__bmY * ncol + __bmX] > 100)
				{
					CPointF<int> __p3(__bmX, __bmY);
					double _ptLastDistance = GetDistance(p_center, __p3);
					_ptDistance = fabs(_ptLastDistance - __startDistance) * ((xres + yres) / 2.);

					if (_ptDistance < CInkMinWidthSpec)
					{
						m_bLackofInkCheckFlag[_angle] = true;
						m_nLackOfInk_CinkCount++;
					}
					break;	
				}
				else __searchCount = 0;
			}
		}

		if (_bfind == false)
		{
			m_bLackofInkCheckFlag[_angle] = true;
			m_nLackOfInk_CinkCount++;
		}
	}

	img.release();
	s_down.release();
	imgSrc.release();
	maskerode.release();

	if (m_nLackOfInk_CinkCount > 0)
	{
		FakeDC* pDC = NULL;
		int nview = m_pMain->vt_job_info[nJob].viewer_index[id];
		if (mViewer != NULL)		pDC = mViewer->getOverlayDC();
		else						pDC = m_pDlgViewerMain[nview]->GetViewer().getOverlayDC();

		CPen penNG(PS_SOLID, 5, COLOR_GREEN);
		CPen* pOldPen;

		pDC->SelectObject(GetStockObject(NULL_BRUSH));
		pOldPen = pDC->SelectObject(&penNG);

		for (int i = 0; i < 360; i++)
		{
			if (!m_bLackofInkCheckFlag[i]) continue;

			int __x = (cos(ToRadian(i)) * (CIRCLE_RADIUS)+m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x); // Glass Edge point
			int __y = (sin(ToRadian(i)) * (CIRCLE_RADIUS)+m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y); // Glass Edge point	

			pDC->Ellipse(__x - 10, __y - 10, __x + 10, __y + 10);
		}

		if (mViewer != NULL)				mViewer->Invalidate();
		else								m_pDlgViewerMain[nview]->GetViewer().Invalidate();

		penNG.DeleteObject();
	}

	str.Format("[%s] Pre Lack of Ink Inspection End", m_pMain->vt_job_info[nJob].job_name.c_str());
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

	return bReturn;
}
//액튐검사
bool CFormMainView::fnDiffDustInspection(int nJob, int id, CViewerEx* mViewer)
{
	// m_ptBMCircleCenter
	bool bReturn = true;
	
	//HTK 2022-03-30 Tact Time Display 추가
	m_clockProcStartTime[6] = clock();

	int camCount = m_pMain->vt_job_info[nJob].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[nJob].camera_index;

	CString str;
	int ncamera = camBuf.at(0);
	int W = m_pMain->m_stCamInfo[ncamera].w;
	int H = m_pMain->m_stCamInfo[ncamera].h;

	str.Format("[%s] Dust Inspection Start", m_pMain->vt_job_info[nJob].job_name.c_str());
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

	//KJH 2022-07-25 GrabBit추가로 이미지 버퍼 갱신 삭제
	//AcquireSRWLockExclusive(&m_pMain->g_bufsrwlock); // 동기화 시킴 Tkyuha 20211027				
	//m_pMain->copyMemory(m_pMain->getProcBuffer(ncamera, 0), m_pMain->getSrcBuffer(ncamera), W * H);
	//ReleaseSRWLockExclusive(&m_pMain->g_bufsrwlock);

	cv::Rect inspRoi = cv::Rect(0, 0, W, H);
	int method = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod();
	double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(0, 0);
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);
	int dThresh = 255 - m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspDustThresh();

	cv::Mat imgSrc(H, W, CV_8UC1, m_pMain->getProcBuffer(ncamera, 3));		//도포 후 이미지
	cv::Mat orgimgSrc(H, W, CV_8UC1, m_pMain->getProcBuffer(ncamera, 2));	//도포 전 이미지
	cv::Mat img, mask, diff_im;

	BOOL _m_nShapeMaskType = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getDiffInspROIShape();
	int _m_nMarkSize = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getDiffInspROISize() / xres;

	int nDefect = 0;
	inspRoi = cv::Rect(W / 2 - _m_nMarkSize, H / 2 - _m_nMarkSize, _m_nMarkSize*2, _m_nMarkSize*2);

	if (method == METHOD_CIRCLE)
	{
		double radius2 = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspSteelHoleSize() / xres + 10;

		cv::Mat mask2 = GetCirclemask(imgSrc, m_ptCircleCenter_CC.x, m_ptCircleCenter_CC.y, m_dCircleRadius_CC);
		cv::Mat mask3 = GetCirclemask(imgSrc, W / 2, H / 2, radius2);
        //KJH 2021-12-11 액튐 검사 예외영역 Master MP 영역에서 실제 MP영역으로 변경 + Margin 3
		//cv::Mat mask3 = GetCirclemask(imgSrc, m_ptCircleCenter_MP.x, m_ptCircleCenter_MP.y, m_dCircleRadius_MP + 3);

		cv::subtract(mask3, mask2, mask);
		mask2.release();
		mask3.release();		
		
		// Shift 된경우에 처리 // 20220325 Tkyuha 수정// 흔들림에 의해 발생하는 과검 제거 목적
		if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getELBInsp_ShiftSearchEnable())
		{
			//KJH 2021-12-26 합본 후 Diff Insp 인자 매칭 작업

			int xoffset = m_ptCircleCenter_CC_Pre.x - m_ptCircleCenter_CC.x;
			int yoffset = m_ptCircleCenter_CC_Pre.y - m_ptCircleCenter_CC.y;

			if (m_ptCircleCenter_CC_Pre.x != 0 && m_ptCircleCenter_CC_Pre.y != 0 && (abs(xoffset) > 20 || abs(yoffset) > 20))
			{
				cv::Mat transImg = translateImg(imgSrc, xoffset, yoffset);
				cv::absdiff(transImg, orgimgSrc, diff_im);
				transImg.release();

				m_ptCircleCenter_CC = m_ptCircleCenter_CC_Pre;

				if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
					cvShowImage("Dust Shift Image", diff_im);
			}
			else cv::absdiff(imgSrc, orgimgSrc, diff_im);
		}
		else	cv::absdiff(imgSrc, orgimgSrc, diff_im);
	}
	else if (method == METHOD_NOTCH)
	{
		mask = cv::Mat::zeros(H, W, CV_8UC1);

		double distanceT = -0.3 / xres;
		cv::Point2f rC;
		if (m_pMain->m_vtTraceData.size() > 0)
		{
			vector<cv::Point> vtContour;

			for (int i = 0; i < m_pMain->m_vtTraceData.size() - 1; i++)
			{
				rC = Equation_of_Lines(m_pMain->m_vtTraceData[i], m_pMain->m_vtTraceData[i + 1], distanceT);
				vtContour.insert(vtContour.begin(), cv::Point(int(rC.x), int(rC.y)));
			}
			distanceT = -0.1 / xres;
			for (int i = 0; i < m_pMain->m_vtTraceData.size() - 1; i++)
			{
				rC = Equation_of_Lines(m_pMain->m_vtTraceData[i], m_pMain->m_vtTraceData[i + 1], distanceT);
				vtContour.push_back(cv::Point(int(rC.x), int(rC.y)));
			}

			int num = (int)vtContour.size();
			const cv::Point* pt4 = &(vtContour[0]);
			cv::fillPoly(mask, &pt4, &num, 1, cv::Scalar(255, 255, 255), 8);

			vtContour.clear();
		}

		cv::absdiff(imgSrc, orgimgSrc, diff_im);
	}
	else if (method == METHOD_LINE)
	{
		double _wx = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeX() / xres/2; //  중심점 기준 좌우 분리
		double _wy = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeY() / yres/2;  //  중심점 기준 상하 분리

		// PJH 22-10-12 WETOUT 검사에서 무시하는 영역 액튐 검사에서도 무시

		BOOL find_line[2] =
		{
			m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][C_CALIPER_POS_2].getIsMakeLine(),		// 얼라인 라인
			m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][C_CALIPER_POS_3].getIsMakeLine()		// 메탈 라인
		};
		sLine line_info[2] =
		{
			m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][C_CALIPER_POS_2].m_lineInfo,			// 얼라인 라인
			m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][C_CALIPER_POS_3].m_lineInfo			// 메탈 라인
		};

		double l = MAX(0, MIN(W /2, (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeLength() + 0.5) / xres / 2));
		
		cv::Rect roi = cv::Rect(int(W / 2 - _wx), int(H / 2 - _wy), int(_wx * 2), int(_wy * 2));

		mask = GetRectmask(imgSrc, roi);
		cv::absdiff(imgSrc, orgimgSrc, diff_im);

		if (find_line[0] && find_line[1]) // 캘리퍼 둘 다 찾았을 경우 그 사이 영역 마스킹
		{
			cv::Rect roi_line = cv::Rect(0, int(line_info[0].sy), W, int(line_info[1].sy - line_info[0].sy));

			cv::Mat mask2 = GetRectmask(diff_im, roi_line);

			bitwise_not(mask2, mask2);

			cv::Mat temp_img;

			diff_im.copyTo(temp_img, mask2);
			temp_img.copyTo(diff_im);
			temp_img.release();
			mask2.release();
		}

		inspRoi = cv::Rect(W / 2 - _wx*2, H / 2 - _wy*2, _wx * 4, _wy * 4);		
	}

	inspRoi.x = MAX(0, inspRoi.x);
	inspRoi.y = MAX(0, inspRoi.y);
	inspRoi.width = MIN(W - inspRoi.x, inspRoi.width);
	inspRoi.height = MIN(H - inspRoi.y, inspRoi.height);

	bitwise_not(mask, mask);
	diff_im.copyTo(img, mask);	

	cv::Mat s_down; // 축소 해서 검사 진행
	cv::resize(img(inspRoi), s_down, cv::Size(), 0.25, 0.25, CV_INTER_CUBIC);

	if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
		cvShowImage("InspDropletOrg Image", s_down);

	//Tkyuha 2021-12-26 액튐 과검 관련 이미지 처리 추가
	int  iDropsAfterImageRemove = int(m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(6));
	int niters = 1;//반복 횟수   20211224 Tkyuha 잔상 제거 루틴
	erode(s_down, s_down, Mat(), cv::Point(-1, -1), niters);//팽창, 밝은 영역 확장   
	dilate(s_down, s_down, Mat(), cv::Point(-1, -1), niters * 2);//침식, 노이즈 제거   
	erode(s_down, s_down, Mat(), cv::Point(-1, -1), niters);//다시 팽창
	threshold(s_down, s_down, iDropsAfterImageRemove, 256, CV_THRESH_TOZERO);//회색(그림자) 지우기

	if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
	{
		cvShowImage("InspOrg Image", s_down);
		cvShowImage("Mask Image", mask);
		cvShowImage("Gray Image", img);
	}

	int spec_S = MAX(10, int(m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(1) / xres) / 4); //최소 크기
	int spec_A = MAX(250, int(m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(1) / xres) / 4); //최소 면적

	nDefect = InspDustBurrInspection(nJob, s_down, dThresh, inspRoi.x, inspRoi.y, 4, spec_S, spec_A, true); //10,25

	if (nDefect > 0)
	{
		for (int i = 0; i< int(m_vecDustResult.size()); i++)
		{
			m_vecDustResult.at(i).center.x = m_vecDustResult.at(i).center.x * 4 + inspRoi.x;
			m_vecDustResult.at(i).center.y = m_vecDustResult.at(i).center.y * 4 + inspRoi.y;
			m_vecDustResult.at(i).rr.center.x = m_vecDustResult.at(i).rr.center.x * 4 + inspRoi.x;
			m_vecDustResult.at(i).rr.center.y = m_vecDustResult.at(i).rr.center.y * 4 + inspRoi.y;
		}
	}

	img.release();
	s_down.release();
	imgSrc.release();;
	orgimgSrc.release();; //도포전 이미지
	mask.release();
	diff_im.release();

	if (nDefect > 0)
	{
		FakeDC* pDC = NULL; 
		CString strText;
		int nview = m_pMain->vt_job_info[nJob].viewer_index[id];
		if (mViewer != NULL)			pDC = mViewer->getOverlayDC();
		else	pDC = m_pDlgViewerMain[nview]->GetViewer().getOverlayDC();
	
		CPen penNG(PS_SOLID, 5, COLOR_RED);
		CPen penROI(PS_SOLID, 5, COLOR_WHITE);

		pDC->SetTextColor(COLOR_RED);

		//KJH 2021-12-02 Dust Insp로 검출시 ROI 표시
		pDC->SelectObject(&penROI);

		if (_m_nShapeMaskType == TRUE)
		{
			pDC->Ellipse(W / 2 - _m_nMarkSize, H / 2 - _m_nMarkSize, W / 2 + _m_nMarkSize, H / 2 + _m_nMarkSize);
		}
		else
		{
			pDC->MoveTo(W / 2 - _m_nMarkSize, H / 2 - _m_nMarkSize);		//좌상으로 이동

			pDC->LineTo(W / 2 + _m_nMarkSize, H / 2 - _m_nMarkSize);		//우상
			pDC->LineTo(W / 2 + _m_nMarkSize, H / 2 + _m_nMarkSize);		//우하
			pDC->LineTo(W / 2 - _m_nMarkSize, H / 2 + _m_nMarkSize);		//좌하
			pDC->LineTo(W / 2 - _m_nMarkSize, H / 2 - _m_nMarkSize);		//좌상
		}

		pDC->SelectObject(&penNG);

		for (int i = 0; i < nDefect; i++)
		{
			cv::RotatedRect rr = m_vecDustResult.at(i).rr;
			cv::Rect brect = rr.boundingRect();
			cv::Point2f vertices2f[4];
			rr.points(vertices2f);

			cv::Point vertices[4];
			for (int j = 0; j < 4; ++j) {
				vertices[j] = vertices2f[j];
			}

			pDC->MoveTo(vertices[0].x, vertices[0].y);
			pDC->LineTo(vertices[1].x, vertices[1].y);
			pDC->LineTo(vertices[2].x, vertices[2].y);
			pDC->LineTo(vertices[3].x, vertices[3].y);
			pDC->LineTo(vertices[0].x, vertices[0].y);

			strText.Format("[S: %.3f,%.3fmm, A: %.2f]", m_vecDustResult.at(i).xw * xres, m_vecDustResult.at(i).yw * yres, m_vecDustResult.at(i).area * 4);
			pDC->TextOutA(m_vecDustResult.at(i).center.x, m_vecDustResult.at(i).center.y, strText);
		}

		if (mViewer != NULL)			mViewer->DirtyRefresh();
		else			m_pDlgViewerMain[nview]->GetViewer().DirtyRefresh();

		penNG.DeleteObject();
		penROI.DeleteObject();

		bReturn = false;
	}
	if (nDefect > 0)
	{
		str.Format("[%s] InspDustBurrInspection - find!", m_pMain->vt_job_info[nJob].job_name.c_str());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}

	if (theApp.m_bEnforceOkFlag) bReturn = true;

	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->GetSafeHwnd(), WM_VIEW_CONTROL, MSG_PA_JUDGE_DISPLAY, MAKELPARAM(1, bReturn?TRUE:FALSE));

	//HTK 2022-03-30 Tact Time Display 추가
	m_clockProc_elapsed_time[6] = (clock() - m_clockProcStartTime[6]) / (float)CLOCKS_PER_SEC;
	((CPaneAuto*)m_pMain->m_pPane[PANE_AUTO])->update_grid_processing_time(1, m_clockProc_elapsed_time[6], 6,bReturn);

	//KJH 2022-07-27 Tact Time Log 추가
	theLog.logmsg(LOG_PLC_SEQ, "Drop Insp Tact Time = %.4f", m_clockProc_elapsed_time[6]);

	return bReturn;
}
//도포검사
bool CFormMainView::fnDiffWetOutInspection(int nJob, int id, CViewerEx* mViewer, BOOL m_bManual)
{
	CString strLog;
	int nview = m_pMain->vt_job_info[nJob].viewer_index[id];

	//KJH 2022-05-26 6View -> 9 View 분기작업
	//viewer_index[0] = Align
	//viewer_index[1] = Live
	//viewer_index[2] = Trace
	//viewer_index[3] = Insp
	if (m_pMain->vt_job_info[nJob].num_of_viewer == 2)
	{
		nview = m_pMain->vt_job_info[nJob].viewer_index[0];
	}
	else
	{
		nview = m_pMain->vt_job_info[nJob].viewer_index[3];
	}

	FakeDC* pDC = NULL;

	//HTK 2022-03-30 Tact Time Display 추가
	m_clockProcStartTime[5] = clock();

	if (mViewer != NULL)
	{
		pDC = mViewer->getOverlayDC();
		mViewer->ClearOverlayDC();
		mViewer->clearAllFigures();
	}
	else
	{
		mViewer = &m_pDlgViewerMain[nview]->GetViewer();
		pDC = m_pDlgViewerMain[nview]->GetViewer().getOverlayDC();
		m_pDlgViewerMain[nview]->GetViewer().ClearOverlayDC();
		m_pDlgViewerMain[nview]->GetViewer().clearAllFigures();
	}
	//KJH 2021-09-17 합본(이거 왜 현장소스는 주석되있었을까???)
	//KJH 2021-09-27 종료 안시킬꺼면 쓰지맙시다.....
	//m_pMain->m_pCircleProgress.StartProgress();

	CPen penOK(PS_SOLID, 5, COLOR_GREEN);
	CPen penNG(PS_SOLID, 5, COLOR_RED);
	CPen penSum(PS_SOLID, 5, COLOR_BLUE);
	CPen penCut(PS_SOLID, 1, COLOR_YELLOW);
	CPen penBM(PS_SOLID, 5, COLOR_SKY_BLUE);
	CPen penDebug(PS_SOLID, 5, COLOR_PURPLE);
	CPen penDebug2(PS_SOLID, 5, COLOR_DARKPURPLE);
	CPen penDB(PS_SOLID, 5, COLOR_DARK_BLUE);
	CPen penOrange(PS_SOLID, 5, COLOR_ORANGE);	
	CPen penRed(PS_SOLID, 5, COLOR_RED);
	CPen penMin(PS_SOLID, 5, COLOR_ORANGE);
	CPen penGreenYellow(PS_SOLID, 5, RGB(173, 255, 47));
	CPen penSkyblue(PS_SOLID, 5, RGB(128, 128, 255));
	CPen penLime(PS_SOLID, 1, COLOR_LIME);


	pDC->SetTextColor(COLOR_RED);

#ifndef JOB_INFO
	int camCount = m_pAlgorithmInfo.getAlgorithmCameraNum(nJob);
	std::vector<int> camBuf = m_pAlgorithmInfo.getAlgorithmCameraIndex(nJob);
#else
	int camCount = m_pMain->vt_job_info[nJob].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[nJob].camera_index;
#endif

	CString str;
	int ncamera = camBuf.at(0);
	int datasheet = 0;
	bool bReturn = true;
	int W = m_pMain->m_stCamInfo[ncamera].w;
	int H = m_pMain->m_stCamInfo[ncamera].h;

	double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(0, 0);
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);
	int x = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeX() / xres / 2; //  중심점 기준 좌우 분리
	int y = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeY() / yres / 2;  //  중심점 기준 상하 분리
	double l = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeLength(); //도포 길이
	int dThresh = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspDustThresh();
	// KBJ 2022-09-01 change parameter
	//bool bSearchElbEndPosFlag = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getSearchElbEndPosFlag();
	bool bSearchElbEndPosFlag = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseMetalTraceFirst(); 
	m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType = 0;
	
	int cx = W / 2, cy = H / 2, _siftX = 0, _siftY = 0;
	int _lineOverflowLine = H / 2;
	CPointF<double> _searchPocket(0, 0);

	/*str.Format("Cam %d - Extract Profile Data Start", ncamera + 1);
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);*/

	// 2022-10-27 image draw
	m_pDlgViewerMain[nview]->GetViewer().OnLoadImageFromPtr(m_pMain->getProcBuffer(ncamera, 3));

	int m_nBufferIndex = 3;
	//KJH 2022-07-15 DiffWetOut OrgImg ProcBuffer 변경
	if (m_bManual)
	{
		m_pMain->copyMemory(m_pMain->getProcBuffer(ncamera, 0), m_pMain->getSrcBuffer(ncamera), W * H);
		m_pMain->copyMemory(m_pMain->getProcBuffer(ncamera, 3), m_pMain->getSrcBuffer(ncamera), W * H);
		m_nBufferIndex = 0;
	}
	cv::Rect inspRoi = cv::Rect(0, 0, W, H);

	int method = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod();

	m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY = cv::Point2d(0, 0);

	if (method == METHOD_CIRCLE)
	{
		if (!m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseRealTimeTrace())
		{
			CIRCLE_RADIUS = fabs(m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.y);
			if (CIRCLE_RADIUS < 1 || CIRCLE_RADIUS > 5)		CIRCLE_RADIUS = 2.75;
			CIRCLE_RADIUS = CIRCLE_RADIUS / xres;
		}

		if (m_dCircleRadius_MP == 0)
		{
			double steelhole = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspSteelHoleSize() / xres + 20;
			m_dCircleRadius_MP = steelhole - 20;
			m_ptCircleCenter_MP = CPoint(W / 2, H / 2);
		}

		//HTK 2022-05-30 본사수정내용
		if (CIRCLE_RADIUS == 0)
		{
			CIRCLE_RADIUS = m_dCircleRadius_MP;
			m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x = m_ptCircleCenter_MP.x;
			m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y = m_ptCircleCenter_MP.y;
		}

		int aOffset = 200 + (CIRCLE_RADIUS);
		inspRoi = cv::Rect(MAX(0, m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x - aOffset), MAX(0, m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y - aOffset), aOffset * 2, aOffset * 2);  // 20211227 수정중 W/2, H/2 변경
		inspRoi.width = MIN(W - inspRoi.x, inspRoi.width);
		inspRoi.height = MIN(H - inspRoi.y, inspRoi.height);
		inspRoi.width = inspRoi.width / 16 * 16;
		inspRoi.height = inspRoi.height / 16 * 16;

		m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY = cv::Point2d(0, 2. * CV_PI * CIRCLE_RADIUS);
	}
	else if (method == METHOD_LINE)
	{
		double pocket_x = 0.0;

		if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getELBCG_Edge_CalcModeEnable() == FALSE)
		{
			// KBJ 2022-12-05 포켓 중앙을 검사 Roi 중심으로 설정.
			cv::Mat imgSrcFindCenter(H, W, CV_8UC1, m_pMain->getProcBuffer(ncamera, m_nBufferIndex)); // 도포된 현재 이미지
			m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][C_CALIPER_POS_5].processCaliper(imgSrcFindCenter.data, W, H, 0, 0, 0); // Cover Line 아래 라인

			BOOL find_line;
			sLine line_info;
			
			find_line = m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][C_CALIPER_POS_5].getIsMakeLine();
			line_info = m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][C_CALIPER_POS_5].m_lineInfo;
			if (find_line == TRUE)
			{
				double pocket_size = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getELB_Line_Pocket_Size() / xres;

				pocket_x = (line_info.my - line_info.b) / line_info.a; // Pocket x 픽셀 위치
				pocket_x = pocket_x + (pocket_size / 2) - W / 2;       // 포켓이 중앙과 얼마나 벗어나 있는지

				if (fabs(pocket_x) > 200)
				{
					pocket_x = 0.0;
				}
				else
				{
					m_pMain->draw_line(mViewer, COLOR_GREEN, line_info, W, H);
				}
			}
		}

		int aOffset = int(l/xres+50)/2;
		int bOffset = 400;

		inspRoi = cv::Rect(MAX(0, m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x + pocket_x - aOffset), MAX(0, m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y - bOffset), aOffset * 2, bOffset * 2);  // 20211227 수정중 W/2, H/2 변경
		inspRoi.width = MIN(W - inspRoi.x, inspRoi.width);
		inspRoi.height = MIN(H - inspRoi.y, inspRoi.height);
		inspRoi.width = inspRoi.width / 16 * 16;
		inspRoi.height = inspRoi.height / 16 * 16;
	}
	else
	{
		int aX = x + 100;
		int aY = y + 100;
		int nvt = int(m_pMain->m_ELB_TraceResult.m_vTraceProfile[4].size() - 1);
		if (nvt > 0)
		{
			inspRoi.x = MAX(0, m_pMain->m_ELB_TraceResult.m_vTraceProfile[3].at(0));
			inspRoi.y = MAX(0, m_pMain->m_ELB_TraceResult.m_vTraceProfile[4].at(nvt) - 200);
			inspRoi.width = abs(m_pMain->m_ELB_TraceResult.m_vTraceProfile[3].at(nvt) - inspRoi.x + 10);
			inspRoi.height = abs(m_pMain->m_ELB_TraceResult.m_vTraceProfile[4].at(0) - inspRoi.y) + 400;
		}
		else	inspRoi = cv::Rect(MAX(0, W / 2 - aX), MAX(0, H / 2 - aY), aX * 2, aY * 2);

		inspRoi.width = MIN(W - inspRoi.x, inspRoi.width);
		inspRoi.height = MIN(H - inspRoi.y, inspRoi.height);
		inspRoi.width = inspRoi.width / 4 * 4;
		inspRoi.height = inspRoi.height / 4 * 4;

		double s = 0;
		double e = m_pMain->m_ELB_TraceResult.m_vTraceProfile[3].size();
		double interval = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspTraceInterval();
		m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY = cv::Point2d(s, e * interval / xres);
	}

	cv::Mat element15(3, 3, CV_8U, cv::Scalar::all(1));
	cv::Mat bin, bin2;
	cv::Mat diff_im, diff_im2, diff_im_org, skeleton, skeleton2;

	if (!m_bManual)
	{
		m_nBufferIndex = 3;
	}
	cv::Mat imgSrc(H, W, CV_8UC1, m_pMain->getProcBuffer(ncamera, m_nBufferIndex)); // 도포된 현재 이미지
	cv::Mat orgimgSrc(H, W, CV_8UC1, m_pMain->getProcBuffer(ncamera, 2)); //도포전 이미지
	cv::Mat img, orgimg, binOrg;

	if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getELBInsp_ShiftSearchEnable())
	{
		if (method == METHOD_CIRCLE)
		{  
			m_ptCircleCenter_CC_Pre = m_ptCircleCenter_CC;
			// Shift 된경우에 처리 // 20220325 Tkyuha 수정// 흔들림에 의해 발생하는 과검 제거 목적		
			if (m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][C_CALIPER_POS_2].getCircleCaliper())
			{
				if (InspCaliperCircleBMEdgeDetection(nJob, ncamera, imgSrc)) // // 켈리퍼를 이용하여 BM 찾기 방식
				{
					int xoffset = m_ptCircleCenter_CC_Pre.x - m_ptCircleCenter_CC.x; // 
					int yoffset = m_ptCircleCenter_CC_Pre.y - m_ptCircleCenter_CC.y;

					if (m_ptCircleCenter_CC_Pre.x != 0 && m_ptCircleCenter_CC_Pre.y != 0 && (abs(xoffset) > 20 || abs(yoffset) > 20))
					{
						//KJH 2022-05-06 이미지 Shift 기능 사용시 getProcBuffer 영향주는거 수정
						cv::Mat transImg = translateImg(imgSrc.clone(), xoffset, yoffset);
						transImg.copyTo(imgSrc);
						transImg.release();
						m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x -= xoffset;
						m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y -= yoffset;
						m_ptCircleCenter_MP.x -= xoffset;
						m_ptCircleCenter_MP.y -= yoffset;
						_siftX = xoffset; _siftY = yoffset;
						str.Format("[%s] Shift Image (%d ,%d)", m_pMain->vt_job_info[nJob].job_name.c_str(), xoffset, yoffset);
						::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

						if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
							cvShowImage("Shift Image", imgSrc);
					}
				}
			}
		}
		else if(method == METHOD_LINE)
		{
			BOOL _find_line;
			sLine _line_info;
			m_ptCircleCenter_CC_Pre.x = m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x;
			m_ptCircleCenter_CC_Pre.y = m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y;
			m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][C_CALIPER_POS_2].processCaliper(imgSrc.data, W, H, 0, 0, 0); // Glass Line 기준라인
			_find_line = m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][C_CALIPER_POS_2].getIsMakeLine();
			_line_info = m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][C_CALIPER_POS_2].m_lineInfo;

			if (_find_line) 
			{
				int xoffset = 0; // 
				int yoffset = m_ptCircleCenter_CC_Pre.y - (_line_info.a *(W/2) + _line_info.b);

				if (m_ptCircleCenter_CC_Pre.y != 0 && (abs(xoffset) > 20 || abs(yoffset) >= 5))
				{
					cv::Mat transImg = translateImg(imgSrc, xoffset, yoffset);
					transImg.copyTo(imgSrc);
					transImg.release();
					m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x -= xoffset;
					m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y -= yoffset;

					_siftX = xoffset; _siftY = yoffset;
					str.Format("[%s] Shift Image (%d ,%d)", m_pMain->vt_job_info[nJob].job_name.c_str(), xoffset, yoffset);
					::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

					if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
						cvShowImage("Shift Image(WetoutInsp)", imgSrc);
				}
			}
		}
	}

	imgSrc(inspRoi).copyTo(img);
	orgimgSrc(inspRoi).copyTo(orgimg);

	// 디버깅 모드에서 이미지 보기
	if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
		cvShowImage("Resize img Image", img);

	double minVal, maxVal;
	cv::Point minLoc, maxLoc;
	cv::Mat mask = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(15, 15), cv::Point(7, 7));
	cv::Mat maskerode = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3), cv::Point(1, 1));

	cv::Mat s_down, s_downorg, s_thinMat, s_CINK2; // 축소 해서 검사 진행
	cv::resize(img, s_down, cv::Size(), 0.5, 0.5, CV_INTER_CUBIC);
	cv::resize(orgimg, s_downorg, cv::Size(), 0.5, 0.5, CV_INTER_CUBIC);

	//HTK 2022-05-19 DiffWetout검사시 Noise 제거 목적으로 Median Blur 수행
	cv::Mat image1, image2; 
	image1 = s_down.clone();
	image2 = s_downorg.clone();

	medianBlur(s_down, image1, 15);
	medianBlur(s_downorg, image2, 15);

	cv::absdiff(image1, image2, diff_im);
	cv::absdiff(s_down, s_downorg, diff_im2);

	//HTK 2022-05-30 본사수정내용
	cv::Scalar data = cv::mean(diff_im); // 2022-05-23 Tkyuha Align 없이 검사만 진행 하는 경우
	if (data[0] == 0)
	{
		diff_im = s_down;
	}

	image1.release();
	image2.release();

	//KJH 2021-12-25 긴급 대응용 추후 검사 파라미터로 분리예정
	//KJH 2022-01-12 CC Find Filter Insp Para로 변경
	int filter = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getCCFindFilter();

	//KJH 2022-03-24 Dopo length 오인식 예외처리 추가
	diff_im_org = diff_im.clone();
	cv::resize(diff_im_org, diff_im_org, cv::Size(), 2, 2, CV_INTER_CUBIC);

	int  iDropsAfterImageRemove = int(m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(6));

	// KBJ 2022-09-16 CINK2 외각도포
	// KBJ 2022-09-12 CINK2 외각도포 찾는 이미지 추가 및 수정.
	//int nThresh = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionLowThresh();
	
	
	GaussianBlur(diff_im2, diff_im2, cv::Size(0, 0), 1.);
	//cv::normalize(diff_im2, s_CINK2, 255, 0, NORM_MINMAX, CV_8UC1);
	
	// KBJ 2022-10-10 ThreshHold 파라미터 수정. CINK2에서는 사용 안함.
	int nThresh = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspDustThresh();
	cv::threshold(diff_im2, s_CINK2, nThresh, 255, CV_THRESH_BINARY); //150 // KBJ 2022-09-12 CINK2 외각도포 찾는 이미지 추가 및 수정.
	//erode(s_CINK2, s_CINK2, maskerode, cv::Point(-1, -1), 2);

	//HTK 2022-05-30 본사수정내용 [Manual Insp 추가]
	if (data[0] == 0) threshold(diff_im, diff_im, filter, 256, CV_THRESH_BINARY_INV);				//회색(그림자) 지우기
	else			  threshold(diff_im, diff_im, iDropsAfterImageRemove, 256, CV_THRESH_TOZERO);	//회색(그림자) 지우기

	cv::morphologyEx(diff_im, diff_im, cv::MORPH_CLOSE, element15);
	//KJH 2021-12-25 Wetout Insp 과검 개선
	GaussianBlur(diff_im, diff_im, cv::Size(0, 0), 3.);
	cv::threshold(diff_im, bin, 30, 255, CV_THRESH_BINARY); //CV_THRESH_OTSU

	// KBJ 2022-09-16 CINK2 외각도포
	// KBJ 2022-09-12 CINK2 외각도포 찾는 이미지 추가 및 수정.
	//int nThresh = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspDustThresh();
	//cv::normalize(diff_im, s_CINK2, 255, 0, NORM_MINMAX, CV_8UC1);
	//cv::threshold(s_CINK2, s_CINK2, nThresh, 255, CV_THRESH_BINARY); //150 // KBJ 2022-09-12 CINK2 외각도포 찾는 이미지 추가 및 수정.
	//erode(s_CINK2, s_CINK2, maskerode, cv::Point(-1, -1), 2);

	// 디버깅 모드에서 이미지 보기
	if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
		cvShowImage("Insp Binary Image(WetoutInsp_0)", bin);

	////KJH 2021-12-25 긴급 대응용 추후 검사 파라미터로 분리예정
	////KJH 2022-01-12 CC Find Filter Insp Para로 변경
	//int filter = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getCCFindFilter();
	if (method == METHOD_LINE)
	{   // 액튐 이미지 전처리 실제 도포액과 액튐 이미지 분리 작성 // Tkyuha 20221125 전처리 필터 추가
		//cv::medianBlur(img, binOrg,15);
		cv::cvtColor(img, img, COLOR_GRAY2BGR);
		cv::pyrMeanShiftFiltering(img, binOrg, 5, 20, 2);
		cv::cvtColor(binOrg, binOrg, COLOR_BGR2GRAY);
		
		// KBJ 2022-12-05 Filter 관련 파라미터 공용으로 사용되어서 분리
		int LinePreFilter = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getExistPanelGray();
		cv::threshold(binOrg, binOrg, LinePreFilter, 255, CV_THRESH_BINARY);
	}
	else cv::threshold(img, binOrg, filter, 255, CV_THRESH_BINARY);

	if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
		cvShowImage("Insp Noise Remove Image", binOrg);

	erode(binOrg, binOrg, maskerode, cv::Point(-1, -1), 2);	
	dilate(binOrg, binOrg, maskerode, cv::Point(-1, -1), 2);

	erode(bin, bin, maskerode, cv::Point(-1, -1), 2);
	dilate(bin, bin, maskerode, cv::Point(-1, -1), 2);

	//fnRemoveNoise(bin, 150);

	if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
		cvShowImage("Orignal Binary Insp Image(WetoutInsp_1)", binOrg);
	
	// 디버깅 모드에서 이미지 보기
	if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
		cvShowImage("After Noise Remove Insp Image(WetoutInsp_2)", bin);

	if (method == METHOD_NOTCH)
	{
		cv::blur(bin, bin, cv::Size(BLUR_SIZE, BLUR_SIZE));
		cv::morphologyEx(bin, bin, cv::MORPH_CLOSE, mask);
	}

	cv::distanceTransform(bin, bin2, CV_DIST_L2, CV_DIST_MASK_PRECISE);
	cv::normalize(bin2, s_thinMat, 255, 0, NORM_MINMAX, CV_8UC1);
	cv::threshold(s_thinMat, s_thinMat, 5, 255, CV_THRESH_BINARY); //5
	erode(s_thinMat, s_thinMat, maskerode, cv::Point(-1, -1), 2);

	//thinImage(skeleton, skeleton2);
	if (method == METHOD_NOTCH)
	{
		cv::blur(s_thinMat, s_thinMat, cv::Size(BLUR_SIZE, BLUR_SIZE));
	}
	else
	{
		dilate(binOrg, binOrg, Mat::ones(7, 7, CV_8U));
		erode(binOrg, binOrg, Mat::ones(5, 5, CV_8U));	
	//	fnRemoveNoiseBlack(binOrg, 500);
	}
	
	cv::resize(s_thinMat, skeleton, cv::Size(), 2, 2, CV_INTER_CUBIC);
	cv::resize(s_CINK2, skeleton2, cv::Size(), 2, 2, CV_INTER_CUBIC);

	// 디버깅 모드에서 이미지 보기
	if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
	{
		cvShowImage("Orinal Insp Image(WetoutInsp_3)", binOrg);
		cvShowImage("Fixed Insp Image(WetoutInsp_4)", skeleton);
	}

	m_pMain->m_matResultImage.release();
	imgSrc(cv::Rect(0, 0, W, H)).copyTo(m_pMain->m_matResultImage);
	cv::cvtColor(imgSrc, m_pMain->m_matResultImage, COLOR_GRAY2BGR);

	for (int i = 0; i < 5; i++)
	{
		m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[i].clear();
		m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[i].clear();
		m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[i].clear();
		m_pMain->m_ELB_DiffInspResult[datasheet].m_RCheckangle[i] = 0.0;
	}

	for (int i = 0; i < 8; i++)
	{
		m_pMain->m_ELB_DiffInspResult[datasheet].m_BCheckangle[i] = 0.0;
	}

	m_pMain->m_ELB_DiffInspResult[datasheet].m_bResultSummary					= TRUE;
	m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_bResultSummary				= TRUE;
	m_pMain->m_ELB_DiffInspResult[datasheet].bType								= TRUE;
	m_pMain->m_ELB_DiffInspResult[datasheet].bRJudge							= TRUE;
	m_pMain->m_ELB_DiffInspResult[datasheet].bBJudge							= TRUE;
	m_pMain->m_ELB_DiffInspResult[datasheet].m_RminResult						= 0;
	m_pMain->m_ELB_DiffInspResult[datasheet].m_RminAngle						= 0;
	m_pMain->m_ELB_DiffInspResult[datasheet].m_RmaxResult						= 0;
	m_pMain->m_ELB_DiffInspResult[datasheet].m_RmaxAngle						= 0;
	m_pMain->m_ELB_DiffInspResult[datasheet].m_BminResult						= 0;
	m_pMain->m_ELB_DiffInspResult[datasheet].m_BminAngle						= 0;
	m_pMain->m_ELB_DiffInspResult[datasheet].m_BmaxResult						= 0;
	m_pMain->m_ELB_DiffInspResult[datasheet].m_BmaxAngle						= 0;
	m_pMain->m_ELB_DiffInspResult[datasheet].m_nLackOfInkAngleCount_Total		= 0;
	m_pMain->m_ELB_DiffInspResult[datasheet].m_nLackOfInkAngleCount_Judge		= 0;
	m_pMain->m_ELB_DiffInspResult[datasheet].m_nLackOfInkAngleCount_Continue	= 0;

	int minIndex = -1, maxIndex = -1;
	int RminIndex = -1, RmaxIndex = -1;
	int BminIndex = -1, BmaxIndex = -1;

	double _checkArray[4] = { cx - 4.002 / xres, cx - 1.014 / xres, double(cx), cx + 1.335 / xres };
	double _checkArrayValue[4][6] = { {0,}, }, _ax, _bc;

	// KBJ 2022-08-16 CINK1 찾은 B값에서 부터 150 픽셀 연속적인지 확인
	BOOL _bOverflowDefect_Flag[360] = { FALSE, };

	//HTK 2022-05-30 본사수정내용
	if (data[0] == 0)
	{
		m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_bResultSummary = FALSE; //Tkyuha 2022-05-25 미도포 검출
	}

	if (method == METHOD_CIRCLE)
	{
		m_pMain->m_dCircleRadius_PN = CIRCLE_RADIUS;

		BOOL _bFirstMetal = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseMetalTraceFirst();
		int radius = int(CIRCLE_RADIUS);
		int colC = skeleton.cols / 2, rowC = skeleton.rows / 2;
		int rposy = 0, rposx = 0, _limitX, _limitY;
		int nInX = 0, nInY = 0,_bmBaseX = 0,_bmBaseY = 0;
		bool bfind = false;
		double first, two, minS = radius - 200, s, D_FindStart = 0, _cinkw = 0,_bmBaseW = 0;
		//KJH 2022-05-25 BM In <-> Cink In Insp 추가
		double  D_BM_In = 0;
		double  D_BM_In_CC = 0;
		int		nBM_InX = 0, nBM_InY = 0;
		uchar* uchar_Cink2  = skeleton2.data; // KBJ 2022-09-12 CINK2 외각도포 찾는 이미지 추가 및 수정.
		uchar* Raw		= skeleton.data;	// diff image
		uchar* OrgRaw	= binOrg.data;		// 원본 이미지

		// KBJ 2022-09-12 CINK2 외각도포 찾는 이미지 추가 및 수정.
		//imwrite("D:\\Raw.jpg", skeleton);
		//imwrite("D:\\OrgRaw.jpg", binOrg);
		//imwrite("D:\\uchar_Cink2.jpg", skeleton2);
		// 디버깅 모드에서 이미지 보기

		if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
		{
			imwrite("D:\\OrgRaw.jpg", binOrg);
			imwrite("D:\\cink1.jpg", skeleton);
			imwrite("D:\\cink2.jpg", skeleton2);
		}

		////////////////////////////////////////////////////
		//0 = 블랙 , 255 = 화이트						  //
		//OrgRaw	= 원본 2진화 영상	(도포 = 블랙)		  //
		//Raw		= 차영상 2진화 영상 (도포 = 화이트)     //
		////////////////////////////////////////////////////
		// 변수명 정리
		// D = GetDistance
		// A = for문으로 Angle 처리한 변수들
		// p = Point2F로 좌표값
		// _ = for문 안에서만 사용하는 변수들
		// __ = for문 안에 for문에서 사용하는 변수들

		CPointF<double> p_ImgCenter(colC, rowC);
		float xy[2][2] = { 0, };
		cv::Point2f _ptFirstPos = cv::Point2f(m_ptCircleCenter_CC.x - inspRoi.x, m_ptCircleCenter_CC.y - inspRoi.y);
		
		//Metal Search 시 차 영상을 우선시 해서 검사 진행 추가 20220124 Tkyuha
		if (_bFirstMetal) 
		{
			_ptFirstPos.x = m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x - inspRoi.x;
			_ptFirstPos.y = m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y - inspRoi.y;
		}

		cv::Point2f p_CCCenter	= cv::Point2f(m_ptCircleCenter_CC.x - inspRoi.x, m_ptCircleCenter_CC.y - inspRoi.y);
		double rejectspec		= m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSubSpecTolerance(0) / xres;		

		for (int second = 0; second < 360; second++)
		{
			// 원위의 점 찾기
			bfind = false;
			first = -1, two = -1;
			//KJH 2022-05-25 BM In <-> Cink In Insp 추가
			D_BM_In = -1;
			//HTK 2022-04-09 Clink 도포 유무 검사 초기값 변경
			_cinkw = CIRCLE_RADIUS;		

			//_limitX = (cos(ToRadian(second)) * m_dCircleRadius_CC + (m_ptCircleCenter_CC.x - inspRoi.x));
			//_limitY = (sin(ToRadian(second)) * m_dCircleRadius_CC + (m_ptCircleCenter_CC.y - inspRoi.y));
			x = (cos(ToRadian(second)) * radius + colC);
			y = (sin(ToRadian(second)) * radius + rowC);

			cv::Point2f p_A_radius = cv::Point2f(x, y);

			if (_bFirstMetal) //Metal Search 시 차 영상을 우선시 해서 검사 진행 추가 20220124 Tkyuha
				FindLineCircleIntersection(_ptFirstPos.x, _ptFirstPos.y, radius - 100, _ptFirstPos, p_A_radius, xy);
			else
				FindLineCircleIntersection(_ptFirstPos.x, _ptFirstPos.y, m_dCircleRadius_CC, _ptFirstPos, p_A_radius, xy);

			CPointF<int> p10(xy[0][0], xy[0][1]);
			CPointF<int> p11(xy[1][0], xy[1][1]);
			double s_1 = GetDistance(p10, p_A_radius);
			double s_2 = GetDistance(p11, p_A_radius);
			
			if (s_1 > s_2)
			{
				_limitX = xy[1][0];
				_limitY = xy[1][1];
			}
			else
			{
				_limitX = xy[0][0];
				_limitY = xy[0][1];
			}

			CPointF<int> p_FindStart(_limitX, _limitY);	//도포 찾기 Start 점 [CInk1 = CC, CInk2 = PN]
			//if (m_pMain->vt_job_info[0].model_info.getAlignInfo().getUsePNBase())
			
			//KJH 2021-12-25 검사 Start Pos 수정
			//KJH 2022-05-16 도포 검사 Start 위치 +15 -> +10 으로 변경
			D_FindStart = GetDistance(p_ImgCenter, p_FindStart) - radius + 10;
			if (_bFirstMetal && bSearchElbEndPosFlag)
			{
				//D_FindStart -= 60;
				D_FindStart -= 80; // KBJ 2022-09-12 -60 -> -80
			}
			//else _S = -100;
			int _searchCount = 0;
			int _bmX = 0, _bmY = 0;
			
			int _nfirstPtrCount_cink1 = 0;
			int _nfirstPtrCount_cink2_Org = 0;			// KBJ 2022-10-10 (2)
			int _nfirstPtrCount_cink2_Skelton2 = 0;		// KBJ 2022-10-10 (2)

			int _nfirstPtrX = 0, _nfirstPtrY = 0;
			double _dfirstPtr = 0;
			double _dfirstPtr_cink2 = 0;				// KBJ 2022-10-10 (2)

			_bmBaseX = -1, _bmBaseY = -1;

			//KJH 2022-05-02 Cink Wetout Insp 검사시 CC까지 Interlock
			
			int A_ccX		= (cos(ToRadian(second)) * m_dCircleRadius_CC			+ m_ptCircleCenter_CC.x - inspRoi.x);
			int A_ccY		= (sin(ToRadian(second)) * m_dCircleRadius_CC			+ m_ptCircleCenter_CC.y - inspRoi.y);
			int A_ccX_3		= (cos(ToRadian(second)) * (m_dCircleRadius_CC + 3)		+ m_ptCircleCenter_CC.x - inspRoi.x); //2022.06.14 Tkyuha 과검 제거 목적
			int A_ccY_3		= (sin(ToRadian(second)) * (m_dCircleRadius_CC + 3)		+ m_ptCircleCenter_CC.y - inspRoi.y);
			int A_mpX_50	= (cos(ToRadian(second)) * (m_dCircleRadius_MP + 50)	+ m_ptCircleCenter_MP.x - inspRoi.x); //KJH 2022-06-29 Find two Limit 생성용
			int A_mpY_50	= (sin(ToRadian(second)) * (m_dCircleRadius_MP + 50)	+ m_ptCircleCenter_MP.y - inspRoi.y);

			double D_A_cc_5 = GetDistance(cv::Point2f(A_ccX, A_ccY), p_ImgCenter) + 5;

			//TKyuha 2022-06-03 BM In <-> Cink In Insp BM에서 카메라 중심방향으로  //2022-06-08 추가 업데이트
			//KJH 2022-06-08 Search Start 15->30으로 변경
			for (int _i = 30; _i < 200; _i++)
			{
				int A_ccX_i = (cos(ToRadian(second)) * (m_dCircleRadius_CC - _i) + m_ptCircleCenter_CC.x - inspRoi.x);
				int A_ccY_i = (sin(ToRadian(second)) * (m_dCircleRadius_CC - _i) + m_ptCircleCenter_CC.y - inspRoi.y);

				if (A_ccX_i > 0 && A_ccX_i < colC * 2 && A_ccY_i>0 && A_ccY_i < rowC * 2)
				{
					if (OrgRaw[A_ccY_i * skeleton.cols + A_ccX_i] > 100)
					{
						if (_searchCount > 20)
						{
							nBM_InX = _bmX;
							nBM_InY = _bmY;
							CPointF<int> p__bm(_bmX, _bmY);
							D_BM_In		= GetDistance(p_ImgCenter,	p__bm);
							D_BM_In_CC	= GetDistance(p_CCCenter,	p__bm);
							break;
						}
						_searchCount++; // 2022-06-08 추가 업데이트						
					}
					else 
					{
						_bmX = A_ccX_i;
						_bmY = A_ccY_i;
					}
				}
			}

			for (int distanceT = int(D_FindStart); distanceT < 200; distanceT++)
			{
				int _rad = radius + distanceT;

				rposx = x = (cos(ToRadian(second)) * _rad + colC);
				rposy = y = (sin(ToRadian(second)) * _rad + rowC);

				if (x < 0 || x > colC * 2) continue;
				if (y < 0 || y > rowC * 2) continue;

				if (_bFirstMetal && bSearchElbEndPosFlag && D_A_cc_5 > _rad) continue;

				//OrgRaw = binOrg
				//Raw = skeleton
				if (OrgRaw[y * skeleton.cols + x] < 100)
				{
					CPointF<int> p_A_rad(x, y);

					if (first == -1)
					{
						double D_s = GetDistance(p_A_radius, p_A_rad);
						if (D_s > rejectspec)
						{
							for (int _i = distanceT + 5; _i < distanceT + 30; _i++) //for (int _i = -30; _i > distanceT; _i--)
							{
								int A__radiusX_i = (cos(ToRadian(second)) * (radius + _i) + colC);
								int A__radiusY_i = (sin(ToRadian(second)) * (radius + _i) + rowC);

								if (A__radiusX_i > 0 && A__radiusX_i < colC * 2 && A__radiusY_i > 0 && A__radiusY_i < rowC * 2)
								{
									//if (OrgRaw[__y * skeleton.cols + __x] > 100)
									// Metal Search 시 차 영상을 우선시 해서 검사 진행 추가 20220528 Tkyuha // 확인 필요 OrgRaw cpzm x,y로 되어 있었음
									if ((_bFirstMetal && bSearchElbEndPosFlag && OrgRaw[A__radiusY_i * skeleton.cols + A__radiusX_i] < 100) || (!bSearchElbEndPosFlag && Raw[A__radiusY_i * skeleton.cols + A__radiusX_i] > 100))
									{
										rposx = x = p_A_rad.x = A__radiusX_i;
										rposy = y = p_A_rad.y = A__radiusY_i;
										distanceT = _i - 1;
										break;
									}
								}
							}
						}

						if (m_pMain->vt_job_info[0].model_info.getAlignInfo().getUsePNBase() && _bmBaseX == -1 && _bmBaseY == -1)
						{
							_bmBaseX = x; _bmBaseY = y; _bmBaseW = GetDistance(p_ImgCenter, p_A_rad);  //임시 저장 용

							for (int _i = 15; _i < 200; _i++)
							{
								int A__ccX_i = (cos(ToRadian(second)) * (m_dCircleRadius_CC - _i) + m_ptCircleCenter_CC.x - inspRoi.x);
								int A__ccY_i = (sin(ToRadian(second)) * (m_dCircleRadius_CC - _i) + m_ptCircleCenter_CC.y - inspRoi.y);

								if (A__ccX_i > 0 && A__ccX_i < colC * 2 && A__ccY_i > 0 && A__ccY_i < rowC * 2)
								{
									if (OrgRaw[A__ccY_i * skeleton.cols + A__ccX_i] > 100)
									{
										rposx = A__ccX_i;
										rposy = A__ccY_i;
										CPointF<int> p_A_cc_i(A__ccX_i, A__ccY_i);
										s = GetDistance(p_ImgCenter, p_A_cc_i);
										break;
									}
								}
							}
						}
						else
						{
							s = GetDistance(p_ImgCenter, p_A_rad);
							// KBJ 2022-09-19 연속성으로 판단하도록 수정
							//if (minS > s || (!_bFirstMetal && Raw[p_A_rad.y * skeleton.cols + p_A_rad.x] < 100)) continue;
							if (minS > s || (!_bFirstMetal && Raw[p_A_rad.y * skeleton.cols + p_A_rad.x] < 100))
							{
								// KBJ 2022-09-19 CINK1은 차형상 이미지
								_nfirstPtrCount_cink1 = 0;
								continue;
							}

							if (minS > s || (_bFirstMetal && OrgRaw[p_A_rad.y * skeleton.cols + p_A_rad.x] > 100))
							{
								// KBJ 2022-09-19 CINK2은 원본 이미지
								_nfirstPtrCount_cink2_Org = 0;
								continue;
							}

							CPointF<int> p_A_cc_3(A_ccX_3, A_ccY_3);
							double D_A_cc_3 = GetDistance(p_ImgCenter, p_A_cc_3);
							if (D_A_cc_3 > s) continue;

							if (!_bFirstMetal)	_nfirstPtrCount_cink1++;
							if (_bFirstMetal)	_nfirstPtrCount_cink2_Org++;

							if (_nfirstPtrCount_cink1 == 1)
							{
								_dfirstPtr = s;
								_nfirstPtrX = rposx;
								_nfirstPtrY = rposy;
							}

							if (_nfirstPtrCount_cink2_Org == 1)
							{
								_dfirstPtr = s;
								_nfirstPtrX = rposx;
								_nfirstPtrY = rposy;
							}

							if (!_bFirstMetal && _nfirstPtrCount_cink1 < 15) continue;
							if (_bFirstMetal && _nfirstPtrCount_cink2_Org < 30) continue; // KBJ 2022-09-19 CINK2는 이전에 도포되어있어서 픽셀 크기 증가

							s = _dfirstPtr;
							rposx = _nfirstPtrX;
							rposy = _nfirstPtrY;
						}
						first = s;
						nInX = rposx, nInY = rposy;

						if (_bFirstMetal && bSearchElbEndPosFlag) //C-Ink + ELB 검사 모드에서 추가로 C-Ink 영역 별도 계산
						{
							for (int _i = distanceT; _i < distanceT + 160; _i++)
							{
								int A__radiusX_i = (cos(ToRadian(second)) * (radius + _i) + colC);
								int A__radiusY_i = (sin(ToRadian(second)) * (radius + _i) + rowC);

								if (A__radiusX_i > 0 && A__radiusX_i < colC * 2 && A__radiusY_i>0 && A__radiusY_i < rowC * 2)
								{
									if (Raw[A__radiusY_i * skeleton.cols + A__radiusX_i] > 100)
									{
										CPointF<int> p_A_radius_i(A__radiusX_i, A__radiusY_i);
										_cinkw = GetDistance(p_ImgCenter, p_A_radius_i);
										break;
									}
								}
							}
						}
					}
					else // 외곽찾기
					{
						double D_s  = GetDistance(p_ImgCenter, p_A_rad);		//화면 중심과 two 후보 점까지의 거리
						double D_s2 = GetDistance(p_A_radius,  p_A_rad);		//radius점과 two 후보 점까지의 거리

						if (D_s2 > 20 && D_s - first < 20)
						{
							continue;
						}
						if (m_pMain->vt_job_info[0].model_info.getAlignInfo().getUsePNBase())
						{
							x = _bmBaseX; y = _bmBaseY; s = _bmBaseW;
						}
						else
						{
							s = GetDistance(p_ImgCenter, p_A_rad);
						}
						two = s;
					}
				}
				else if (first != -1 && two == -1)
				{
					CPointF<int> p_A_rad(x, y);

					//KJH 2022-06-29 Wetout_out Limit 제한 추가
					CPointF<int> p_Limit_MP(A_mpX_50, A_mpY_50);

					double _s		= GetDistance(p_ImgCenter, p_A_rad);		//화면 중심과 two 후보 점까지의 거리
					double _Limit	= GetDistance(p_ImgCenter, p_Limit_MP);		//화면 중심에서 MP + 50 까지의 거리

					if (_s > _Limit)
					{
						two = -1;
						break;
					}
				}
				else if (two != -1 && ((!_bFirstMetal && Raw[y * skeleton.cols + x] < 200) || (_bFirstMetal && OrgRaw[y * skeleton.cols + x] > 200)))
				{
					//HTK 2022-03-29 Metal HIAA Mode에서 차영상 , 원본영상 둘 다 비교하도록 변경
					if (_bFirstMetal) // 테스트중 20220329 Tkyuha
					{
						// KBJ 2022-09-19 distanceT 100 -> 150
						for (int _i = distanceT - 30; _i < distanceT + 150; _i++)
						{
							int A__radiusX_i = (cos(ToRadian(second)) * (radius + _i) + colC);
							int A__radiusY_i = (sin(ToRadian(second)) * (radius + _i) + rowC);

							if (A__radiusX_i > 0 && A__radiusX_i < colC * 2 && A__radiusY_i > 0 && A__radiusY_i < rowC * 2)
							{
								// KBJ 2022-09-12 CINK2 외각도포 찾는 이미지 추가 및 수정.
								if (uchar_Cink2[A__radiusY_i * skeleton.cols + A__radiusX_i] > 200)
								//if (OrgRaw[A__radiusY_i * skeleton.cols + A__radiusX_i] < 100 && Raw[A__radiusY_i * skeleton.cols + A__radiusX_i] > 100)
								{
									int nContinue = 3;
									int nCount = 0;
									for (int filter = 0; filter < nContinue; filter++)
									{
										int _A__radiusX_i = (cos(ToRadian(second)) * (radius +_i - filter) + colC);
										int _A__radiusY_i = (sin(ToRadian(second)) * (radius +_i - filter) + rowC);
										if (uchar_Cink2[_A__radiusY_i * skeleton.cols + _A__radiusX_i] > 200) nCount++;
										else break;
									}

									if (nCount == nContinue)
									{
										CPointF<int> p_A_radius_i(A__radiusX_i, A__radiusY_i);
										s = GetDistance(p_ImgCenter, p_A_radius_i);
										x = A__radiusX_i;
										y = A__radiusY_i;
										two = s;
									}
									
								}
							}
						}
					}
					// KBJ 2022-09-19 CINK2 에서도 검사
					//else //HTK 2022-08-16 미검 검사 추가, 안쪽 거리에서 얼라인 거리 까지 침범 한경우 Overflow로 처리 하기 위함
					{
						_searchCount = 0;

						for (int _i = 15; _i < 200; _i++)
						{
							int A_ccX_i = (cos(ToRadian(second)) * (_i) + nBM_InX);
							int A_ccY_i = (sin(ToRadian(second)) * (_i) + nBM_InY);

							if (A_ccX_i > 0 && A_ccX_i < colC * 2 && A_ccY_i>0 && A_ccY_i < rowC * 2)
							{
								if (OrgRaw[A_ccY_i * skeleton.cols + A_ccX_i] < 100)
								{
									_searchCount++; // 2022-06-08 추가 업데이트						
								}
								else break;
							}
						}

						if (_searchCount > 150)
						{
							int pBx = nBM_InX + inspRoi.x;
							int pBy = nBM_InY + inspRoi.y;
							pDC->SelectObject(&penNG);
							// KBJ 2022-08-16 CINK1 찾은 B값에서 부터 150 픽셀 연속적인지 확인
							pDC->Ellipse(int(pBx - 5), int(pBy - 5), int(pBx + 5), int(pBy + 5));
							_bOverflowDefect_Flag[second] = TRUE;
						}

					} //  미검 검사 추가
					if (m_pMain->vt_job_info[0].model_info.getAlignInfo().getUsePNBase())
					{
						x = _bmBaseX; y = _bmBaseY; two = _bmBaseW;
					}

					// HTK 2022-08-31 Cink1+Cink2 모드시 검사 overflow 기준을 GlassEdge에서 MetalEdge로 변경	// blue 2022.09.02
					if (m_pMain->vt_job_info[0].model_info.getInspSpecParaInfo().getCalcWetoutMetalEdge())
					{
						int __CCx = (cos(ToRadian(second)) * CIRCLE_RADIUS + colC);
						int __CCy = (sin(ToRadian(second)) * CIRCLE_RADIUS + rowC);
						int __MPx = (cos(ToRadian(second)) * m_dCircleRadius_MP + m_ptCircleCenter_MP.x - inspRoi.x);
						int __MPy = (sin(ToRadian(second)) * m_dCircleRadius_MP + m_ptCircleCenter_MP.y - inspRoi.y);

						double _CCMP_Distance = GetDistance(cv::Point2f(__MPx, __MPy), cv::Point2f(__CCx, __CCy));
						two = two - _CCMP_Distance;
					}

					m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].push_back(first);					// 중심에서 wetout
					m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[1].push_back(two);						// 중심에서 metal
					m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[2].push_back((second + 90) % 360);		// 화면 기준 Angle
					m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[3].push_back(x - _siftX);				// OUT X좌표
					m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[4].push_back(y - _siftY);				// OUT Y좌표
					m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[3].push_back(nInX - _siftX);		// IN X좌표
					m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[4].push_back(nInY - _siftY);		// IN Y좌표
					m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[2].push_back(_cinkw);				// CINK DISTANCE
					m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[0].push_back(D_BM_In);				// BM_In 거리
					m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[1].push_back(first - D_BM_In);		// BM_In <-> Wetout 거리
					m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[2].push_back(D_BM_In_CC);			// BM_IN 거리(CC중앙기준)
					m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[3].push_back(nBM_InX);				// BM_In X 좌표
					m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[4].push_back(nBM_InY);				// BM_In Y 좌표

					two = first = -1;
					D_BM_In = -1;
					bfind = true;
					break;
				}
				else
				{
					_nfirstPtrCount_cink1 = 0;
					_nfirstPtrCount_cink2_Org = 0;
				}
			}

			// KBJ 2022-10-10 CINK2에서만 바른 도포두께 찾기 (2)
			if (_bFirstMetal)
			{
				for (int distanceT = int(D_FindStart); distanceT < 200; distanceT++)
				{
					int _rad = radius + distanceT;

					rposx = x = (cos(ToRadian(second)) * _rad + colC);
					rposy = y = (sin(ToRadian(second)) * _rad + rowC);

					if (x < 0 || x > colC * 2) continue;
					if (y < 0 || y > rowC * 2) continue;

					if (_bFirstMetal && bSearchElbEndPosFlag && D_A_cc_5 > _rad) continue;

					if (_bFirstMetal)
					{
						CPointF<int> p_A_rad(x, y);
						double ss = GetDistance(p_ImgCenter, p_A_rad);
						if (minS > ss || (_bFirstMetal && uchar_Cink2[p_A_rad.y * skeleton.cols + p_A_rad.x] < 100))
						{
							_nfirstPtrCount_cink2_Skelton2 = 0;
							continue;
						}

						if (_bFirstMetal)	_nfirstPtrCount_cink2_Skelton2++;

						if (_nfirstPtrCount_cink2_Skelton2 == 1)
						{
							_dfirstPtr_cink2 = ss;
						}

						if (_bFirstMetal && _nfirstPtrCount_cink2_Skelton2 < 5) continue;
						if (m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[1].size() > second)
						{
							double two_pos = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[1][second];
							m_pMain->m_ELB_DiffInspResult[datasheet]._dbCink2DopoWidth[second] = fabs(two_pos - _dfirstPtr_cink2) * xres;
						}
						else
						{
							m_pMain->m_ELB_DiffInspResult[datasheet]._dbCink2DopoWidth[second] = fabs(CIRCLE_RADIUS - _dfirstPtr_cink2) * xres;
						}
						break;
					}
				}
			}
			

			if (bfind == false)
			{
				x = cos(second * CV_PI / 180.) * CIRCLE_RADIUS + colC;
				y = sin(second * CV_PI / 180.) * CIRCLE_RADIUS + rowC;
				
				//KJH 2022-06-30 Find Dopo Edge 관련 예외처리 변경 first, two = 기존 0 -> 변경 CIRCLE_RADIUS
				m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].push_back(first == -1 ? CIRCLE_RADIUS : first);							// 중심에서 wetout
				m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[1].push_back(two	== -1 ? CIRCLE_RADIUS : two);							// 중심에서 metal
				m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[2].push_back((second + 90) % 360);											// 화면 기준 Angle
				m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[3].push_back(two == -1 ? x : rposx);										// OUT X좌표
				m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[4].push_back(two == -1 ? y : rposy);										// OUT Y좌표
				m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[3].push_back(first == -1 ? x : nInX);									// IN X좌표
				m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[4].push_back(first == -1 ? y : nInY);									// IN Y좌표
				m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[2].push_back(_cinkw);													// CINK DISTANCE
				m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[0].push_back(D_BM_In);													// BM_In 거리
				m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[1].push_back((first == -1 ? CIRCLE_RADIUS : first) - D_BM_In);			// BM_In <-> Wetout 거리
				m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[2].push_back(D_BM_In_CC);												// BM_IN 거리(CC중앙기준)
				m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[3].push_back(nBM_InX);													// BM_In X 좌표
				m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[4].push_back(nBM_InY);													// BM_In Y 좌표
				
				m_pMain->m_ELB_DiffInspResult[datasheet]._dbCink2DopoWidth[second] = fabs((two == -1 ? CIRCLE_RADIUS : two) - _dfirstPtr_cink2) * xres;

				two = first = -1;
			}
		}

		if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getWetoutInspectionSimulation())                        //Tkyuha 221109                       //Wetout 검사 이미지 한장으로 검사
		{
			ELB_RawImageBMinInspectionMode(imgSrc, ncamera, nJob);  // 검사 이미지 한장으로 BMIN 값 계산하기 위한 목적
			inspRoi = cv::Rect(0, 0, W, H);
		}

		// Tkyuha 2022.06.27 과검 제거 검은점 제거 
		int _nvectorCount  = int(m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].size());
		double _nvectorSum = 0, _nvectorAvg = 0, _ncmin, _ncmax;
		bool _bok = false;
		std::vector<double> _tmpVector;

		for (int i = 1; i < _nvectorCount; i++)
		{
			CPointF<double> _tp1(m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[3].at(i-1), m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[4].at(i-1));
			CPointF<double> _tp2(m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[3].at(i), m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[4].at(i));
			s = GetDistance(_tp1, _tp2);
			_nvectorSum += s;
			_tmpVector.push_back(s);
		}

		if (_nvectorCount > 1)
		{
			_nvectorAvg = _nvectorSum / (_nvectorCount - 1);
			_ncmin = _nvectorAvg - (_nvectorAvg*0.6);
			_ncmax = _nvectorAvg + (_nvectorAvg*0.6);

			for (int i = 0; i < _nvectorCount-1; i++)
			{
				double tv = _tmpVector.at(i);
				if(tv< _ncmin || tv>_ncmax) // 재계산 수행
				{
					if (_bok && i > 0)
					{
						double __dist = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].at(i);
						int __x = (cos(ToRadian(i+1)) * __dist) + colC;
						int __y = (sin(ToRadian(i+1)) * __dist) + rowC;

						m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].at(i+1) = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].at(i);		// 중심에서 wetout
						m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[3].at(i+1) = __x;	// IN X좌표
						m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[4].at(i+1) = __y;	// IN Y좌표
						m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[1].at(i+1) = m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[1].at(i); // BM_In <-> Wetout 거리
					}
				}
				else _bok = true;
			}
		}

		_tmpVector.clear();
	}
	else if(method == METHOD_LINE) /// Line 검사 진행
	{     
		//  1. Metal Line search
		//  2. Glass Line search
		// 	3. 도포 길이 측정
		//  4. Inspection (WetIn / WetOut)	
		cv::Mat iproc;

		uchar* Raw = skeleton.data; // diff image
		uchar* OrgRaw = binOrg.data; // 원본 이미지
		
		
		GaussianBlur(skeleton, iproc, cv::Size(0, 0), 3.);
		m_pMain->m_ManualAlignMeasure.InspectionEnhance(&iproc, 1, false);
		cv::threshold(iproc, iproc, 130, 255, CV_THRESH_BINARY); //5
		uchar* iprocEnh = iproc.data; // 원본 이미지

		// 디버깅 모드에서 이미지 보기
		if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
		{
			cvShowImage("Enhance Image", iproc);
		}

		double rejectspec = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSubSpecTolerance(0) / xres;
		double _wx = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeX() / xres / 2; //  중심점 기준 좌우 분리
		double _wy = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeY() / yres / 2;  //  중심점 기준 상하 분리

		int colC = skeleton.cols / 2, rowC = skeleton.rows / 2;
		int rposy = 0, rposx = 0;
		int nInX = 0, nInY = 0;
		double first, two, _overflow;
		double dx = 0, dy = 0, dt = 0;
		cv::Rect _searchRoi = cv::Rect(0, 0, colC * 2, rowC * 2);
		CPointF<double> p_center(colC, rowC);
		CPointF<int> p2(0, 0);

		m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.x = -1;
		m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.y = -1;

		m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][C_CALIPER_POS_3].processCaliper(imgSrc.data, W, H, dx, dy, dt); // Metal Line 윗라인
		//m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][C_CALIPER_POS_4].processCaliper(imgSrc.data, W, H, dx, dy, dt); // Cover Line 아래 라인
		m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][C_CALIPER_POS_5].processCaliper(imgSrc.data, W, H, dx, dy, dt); // Cover Line 아래 라인

		BOOL find_line[3];
		sLine line_info[3];

		find_line[0] = m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][C_CALIPER_POS_3].getIsMakeLine();
		//find_line[1] = m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][C_CALIPER_POS_4].getIsMakeLine();
		find_line[2] = m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][C_CALIPER_POS_5].getIsMakeLine();
		line_info[0] = m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][C_CALIPER_POS_3].m_lineInfo;
		//line_info[1] = m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][C_CALIPER_POS_4].m_lineInfo;
		line_info[2] = m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][C_CALIPER_POS_5].m_lineInfo;

		////SJB 2022-10-31 Insp Overflow Metal Line 도포 전 Trace 측정 시 Caliper 데이터로 측정	//SJB 2022-11-29
		//find_line[1] = m_Trace_Metal_Find_Line;
		//line_info[1] = m_Trace_Metal_Line_Info;

		if (find_line[0] == FALSE)
		{ // CC 상단 라인 못 찾을 경우 다시 한번 시도 해보는 걸로
			_st32fPoint spt = m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][C_CALIPER_POS_3].getStartPt();
			_st32fPoint ept = m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][C_CALIPER_POS_3].getEndPt();

			dy = MIN(50, MAX(-50, (m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y - (spt.y + ept.y) / 2) - 3));
			m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][C_CALIPER_POS_3].processCaliper(imgSrc.data, W, H, dx, dy, dt); // Metal Line 윗라인
			find_line[0] = m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][C_CALIPER_POS_3].getIsMakeLine();
			line_info[0] = m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][C_CALIPER_POS_3].m_lineInfo;

			if (find_line[0] == FALSE)
			{
				line_info[0].a = 0;
				line_info[0].b = m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y;
				find_line[0] = TRUE;
			}
		}

		//SJB 2022-10-31 Insp Overflow Metal Line 도포 전 Trace 측정 시 Caliper 데이터로 측정(미사용시 주석 해제)
		if (find_line[1] == FALSE)
		{ // Metal 하단 라인 못 찾을 경우 다시 한번 시도 해보는 걸로
			_st32fPoint spt = m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][C_CALIPER_POS_4].getStartPt();
			_st32fPoint ept = m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][C_CALIPER_POS_4].getEndPt();

			dy = MAX(-50, MIN(50, (m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y - (spt.y + ept.y) / 2) + 3));
			m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][C_CALIPER_POS_4].processCaliper(imgSrc.data, W, H, dx, dy, dt); // Metal Line 윗라인
			find_line[1] = m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][C_CALIPER_POS_4].getIsMakeLine();
			line_info[1] = m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][C_CALIPER_POS_4].m_lineInfo;
		}

		bool bLine_InspWithTraceResultMode = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getLine_InspWithTraceResultMode();  
		if (bLine_InspWithTraceResultMode)
		{
			// Tkyuha 20221123 메탈 라인을 Trace 에서 찾은 것으로 대체 , 메탈 아래 라인도 동일 하게 대체
			memcpy(&line_info[0], &m_TraceSearch_line_info[C_CALIPER_POS_3], sizeof(sLine));   //Tkyuha 20221123 Line검사 수정
			find_line[0] = m_bTraceSearchFindLine[C_CALIPER_POS_3];                             //Tkyuha 20221123 Line검사 수정
			memcpy(&line_info[1], &m_TraceSearch_line_info[C_CALIPER_POS_3], sizeof(sLine));    //Tkyuha 20221123 Line검사 수정
			find_line[1] = m_bTraceSearchFindLine[C_CALIPER_POS_3];                            //Tkyuha 20221123 Line검사 수정
		}
		if (find_line[0] && find_line[1])
		{
			vector<vector<cv::Point>> contours;
			vector<cv::Vec4i> hierarchy;
			cv::Mat binContour;
			cv::resize(s_thinMat, binContour, cv::Size(), 2, 2, CV_INTER_CUBIC);

			//KJH 2022-03-24 Dopo length 오인식 예외처리 추가
			BOOL m_bPreimageMode = FALSE;
			double _linespec = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getLineLengthSpec();

			findContours(binContour, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
			cv::Rect bounding_rect;
			int ID = getMaxAreaContourId(contours);
			if (ID != -1)
			{
				bounding_rect = boundingRect(contours[ID]);

				double lw = bounding_rect.width * xres; //l/ xres;//
				double lh = bounding_rect.height * yres; //l / xres;// 
				_searchRoi = bounding_rect;
				_searchRoi.y = MAX(_searchRoi.y - 20, 10);
				_searchRoi.height = MIN(rowC * 2 - 20, _searchRoi.height + 40);

				_searchRoi.width = _searchRoi.width / 8 * 8;
				_searchRoi.height = _searchRoi.height / 8 * 8;

				//만약 찾은 Contour가 Spec 보다 크면 오인식이라고 가정함                                                                     
				if (bounding_rect.width * xres > _linespec)		m_bPreimageMode = TRUE;

				m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.x = inspRoi.x + bounding_rect.x;							//Dopo Start
				m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.y = inspRoi.x + bounding_rect.x + bounding_rect.width;		//DOpo End

			}
			binContour.release();
			contours.clear();
			hierarchy.clear();

			//skeleton 축소 후 이미지 (데이터 처리 영상)
			//길이검사 NG시 추가 영상 처리 -> GaussianBlur -> threshold
			GaussianBlur(diff_im_org, binContour, cv::Size(0, 0), 8.);
			cv::threshold(binContour, binContour, iDropsAfterImageRemove, 255, CV_THRESH_BINARY);

			//KJH 2022-03-24 Dopo length 오인식 예외처리 추가
			if (m_bPreimageMode)
			{
				cv::Mat binContourPre;
				binContour.copyTo(skeleton);
				binContour.copyTo(binContourPre);

				findContours(binContourPre, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
				cv::Rect bounding_rect;
				int ID = getMaxAreaContourId(contours);
				if (ID != -1)
				{
					bounding_rect = boundingRect(contours[ID]);

					double lw = bounding_rect.width * xres; //l/ xres;//
					double lh = bounding_rect.height * yres; //l / xres;// 
					_searchRoi = bounding_rect;
					_searchRoi.y = MAX(_searchRoi.y - 20, 10);
					_searchRoi.height = MIN(rowC * 2 - 20, _searchRoi.height + 40);

					_searchRoi.width = _searchRoi.width / 8 * 8;
					_searchRoi.height = _searchRoi.height / 8 * 8;

					m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.x = inspRoi.x + bounding_rect.x;
					m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.y = inspRoi.x + bounding_rect.x + bounding_rect.width;

				}

				contours.clear();
				hierarchy.clear();
				binContourPre.release();
			}

			if (_searchRoi.height < 100 || _searchRoi.y< rowC-100 ||
				_wx > _searchRoi.width || _searchRoi.x < colC - 100)
			{
				_searchRoi.y = MAX(10,rowC-200);
				_searchRoi.height = 400;
			}
			// 디버깅 모드에서 이미지 보기
			if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
			{
				cvShowImage("binContour Org Image", diff_im_org);
				cvShowImage("binContour Image", binContour);				
			}

			pDC->SelectObject(&penNG);
			p2.x = inspRoi.x, p2.y = line_info[0].a * inspRoi.x + line_info[0].b;
			//pDC->MoveTo(p2.x, p2.y);
			p2.x = inspRoi.x + inspRoi.width, p2.y = line_info[0].a * p2.x + line_info[0].b;
			//pDC->LineTo(p2.x, p2.y); // MP 라인(상단) 그리기

			//SJB 2022-12-02 이미지 저장 Panel, Metal 라인 추가
			p2.x = inspRoi.x, p2.y = line_info[1].a * inspRoi.x + line_info[1].b;
			pDC->MoveTo(p2.x, p2.y); _lineOverflowLine = p2.y;
			cv::Point pt_Metal1(p2.x, p2.y);
			p2.x = inspRoi.x + inspRoi.width, p2.y = line_info[1].a * p2.x + line_info[1].b;
			pDC->LineTo(p2.x, p2.y); // Cover Edge(하단) 그리기
			cv::Point pt_Metal2(p2.x, p2.y);
			_lineOverflowLine = (_lineOverflowLine + p2.y) / 2;
			cv::line(m_pMain->m_matResultImage, pt_Metal1, pt_Metal2, cv::Scalar(0, 255, 0), 1);

			pDC->SelectObject(&penOK);

			//SJB 2022-12-02 이미지 저장 Panel, Metal 라인 추가
			p2.x = inspRoi.x, p2.y = m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y;
			pDC->MoveTo(p2.x, p2.y);
			cv::Point pt_Panel1(p2.x, p2.y);
			p2.x = inspRoi.x + inspRoi.width, p2.y = m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y;
			pDC->LineTo(p2.x, p2.y); // Glass Edge(도포 얼라인 기준) 그리기 
			cv::Point pt_Panel2(p2.x, p2.y);
			cv::line(m_pMain->m_matResultImage, pt_Panel1, pt_Panel2, cv::Scalar(64, 255, 64), 1);

			double _off = p_center.y + inspRoi.y - _lineOverflowLine;

			if (find_line[2])
			{
				if (line_info[2].a == 0)
				{
					p2.y = 0;     				p2.x = line_info[2].b;
					//pDC->MoveTo(p2.x, p2.y);    
					_searchPocket.x = p2.x;
					p2.y = imgSrc.rows - 1; 	p2.x = line_info[2].b;
					//pDC->LineTo(p2.x, p2.y);    
					_searchPocket.y = p2.x;// Pocket 시작점

				}
				else
				{
					p2.y = 0;			        p2.x = (p2.y - line_info[2].b) / line_info[2].a;
					//pDC->MoveTo(p2.x, p2.y);   
					_searchPocket.x = p2.x;
					p2.y = imgSrc.rows - 1;		p2.x = (p2.y - line_info[2].b) / line_info[2].a;
					//pDC->LineTo(p2.x, p2.y);     
					_searchPocket.y = p2.x;// Pocket 시작점
				}
			}

			int _tPreData = 0; // Tkyuha 20230102 나비모양 과검 제거 목적
			for (int _x = _searchRoi.x; _x < _searchRoi.x + _searchRoi.width; _x += 1)
			{
				first = -1, two = -1;
				rposy = rowC, rposx = 0;
				nInX = 0, nInY = rowC;
				x = _x, _overflow = 0;

				for (int _y = rowC; _y > _searchRoi.y; _y -= 1) //wetout search
				{  // Tkyuha 20230102 나비모양 과검 제거 목적 //아래 수정
					bool  _tfind = false;
					bool  _tfindRaw = false;

					if (_x < 0 || _y < 0 || _x >= binOrg.cols || _y >= binOrg.rows) continue;
					if (OrgRaw[_y * binOrg.cols + _x] < 100 && Raw[_y * binOrg.cols + _x] > 100)					_tfind = true;						
					else if (OrgRaw[_y * binOrg.cols + _x] < 100)		_tfindRaw = true;

					if (_tfind || _tfindRaw)
					{
						nInX = _x, nInY = _tfindRaw? _tPreData :_y;
						p_center.x = p2.x = _x, p2.y = _tfindRaw ? _tPreData : _y;
						first = GetDistance(p_center, p2);
					} //////////////////////////////////////
				}

				for (int _y = rowC; _y < rowC + _searchRoi.height; _y += 1) //wetin search
				{
					if (_x < 0 || _y < 0 || _x >= iproc.cols || _y >= iproc.rows) continue;
					if (iprocEnh[_y * iproc.cols + _x] > 100) //OrgRaw[_y * binOrg.cols + _x] < 100 && Raw[_y * binOrg.cols + _x]
					{
						rposx = _x, rposy = _y;
						p_center.x = p2.x = _x, p2.y = _y;
						two = GetDistance(p_center, p2);
					}
				}

				dx = line_info[0].a * _x + line_info[0].b - inspRoi.y;
				dy = line_info[1].a * _x + line_info[1].b - inspRoi.y;
				if (two != -1 && dy < rposy) _overflow = fabs(dy - rposy);
				if (first != -1 && dx > nInY)	_overflow = fabs(dx - nInY);

				if (first != -1 && rejectspec <= first)
				{
					bool _balarm = false;
					int _bcount = 0;
					for (int _y = 1; _y < 30; _y += 1) //과검 제거 목적 아래로 찾아보고 연속되어 있으면 그위치로 선정 아니면 과검으로 판정
					{
						if (_x < 0 || (nInY + _y) < 0 || _x >= binOrg.cols || (nInY + _y) >= binOrg.rows) continue;
						if (binContour.data[(nInY + _y) * binOrg.cols + nInX] < 100) _bcount++;
						if (_bcount > 5) _balarm = true;
					}

					if (_balarm)
					{
						for (int _y = rowC - 10; _y > _searchRoi.y; _y -= 1) //wetout search
						{
							//HTK 2022-05-30 본사수정내용
							if (_x < 0 || _y < 0 || _x >= binOrg.cols || _y >= binOrg.rows) continue;
							if (binContour.data[_y * binOrg.cols + _x] > 100)
							{
								nInX = _x, nInY = _y;
								p_center.x = p2.x = _x, p2.y = _y;
								first = GetDistance(p_center, p2);
								break;
							}
						}
					}
				}

				_tPreData = first == -1 ? rowC : nInY; // Tkyuha 20230102 나비모양 과검 제거 목적
				// Bluring 된 이미지 보정값 적용 (5 pixexl) 20230106 Tkyuha
				int _wetoutOffset = 4, _ovreflowOffset = 2;
				first += _wetoutOffset, nInY -= _wetoutOffset;// Wetout 보정
				two += _ovreflowOffset, rposy += _ovreflowOffset; // Overflow 보정

				m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].push_back(first == -1 ? 0 : first);					// 중심에서 wetout
				m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[1].push_back(two == -1 ? 0 : ((two + _off)<0?0: (two + _off)));						// 중심에서 metal
				m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[2].push_back(_overflow);								// 각도
				m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[3].push_back(two == -1 ? x : rposx - _siftX);
				m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[4].push_back(two == -1 ? rowC : rposy - _siftY);
				m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[3].push_back(first == -1 ? x : nInX - _siftX);		// IN X좌표
				m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[4].push_back(first == -1 ? rowC : nInY- _siftY);	// IN Y좌표
			}
			int step = int((m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].size()-1)/8);
			int step_c = int(m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].size() / 2);
			double _line_step = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getLineLengthFDC_Distance()/xres;

			if (step > 0)
			{
				for (int s = 0; s < 8; s++)
				{
					first	= m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].at(step * (s + 1));
					two		= m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[1].at(step * (s + 1));
					m_pMain->m_dCPos[s] = cv::Point2d(first * yres, two * yres);
				}
				// 20221109 Tkyuha 라인 정보 수집 데이타 중앙 과 좌우 1mm 지점 결과로 변경
				for (int s = -1; s <= 1; s++)
				{
					int _pos = step_c + _line_step*s;

					if (_pos < 0 || _pos > m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].size()) continue;

					first = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].at(_pos);
					two = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[1].at(_pos);
					//m_pMain->m_dCPos[2+s] = cv::Point2d(first * yres, two * yres);
					m_pMain->m_dCPos[1 + s] = cv::Point2d(first * yres, two * yres); // 20221109 Tkyuha 0,1,2 번째에 Left, Center, Right 값 저장
				}
			}

			binContour.release();
		}

		iproc.release();
	}
	else
	{  /// Notch 검사 진행
		uchar* Raw = skeleton.data;
		uchar* OrgRaw = binOrg.data;
		double first, two;
		m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.x = -1;
		m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.y = -1;

		if (m_pMain->m_ELB_TraceResult.m_vTraceProfile[3].size() > 1 + m_nNotchLeftStartCount)
		{
			int nsizeT = int(m_pMain->m_ELB_TraceResult.m_vTraceProfile[3].size() - 1 - m_nNotchLeftStartCount);
			double vecX = m_pMain->m_ELB_TraceResult.m_vTraceProfile[3].at(m_nNotchLeftStartCount < 0 ? 0 : m_nNotchLeftStartCount);
			double vecY = m_pMain->m_ELB_TraceResult.m_vTraceProfile[4].at(m_nNotchLeftStartCount < 0 ? 0 : m_nNotchLeftStartCount);
			int nInX = 0, nInY = 0;
			bool bfind;
			double vecX_p = 0, vecY_p = 0;
			cv::Point2f rC;

			pDC->MoveTo(int(vecX), int(vecY));

			//double _checkArray[4] = { cx - 4.002 / xres, cx - 1.014 / xres, double(cx), cx + 1.335 / xres };
			//double _checkArrayValue[4][6] = { {0,}, }, _ax, _bc;
			bool _bsim = false;
			int _startsim = 0;

			for (int i = 1; i < nsizeT; i++)
			{
				_bsim = false;
				if (i + m_nNotchLeftStartCount - 1 < 0) continue;
				vecX = m_pMain->m_ELB_TraceResult.m_vTraceProfile[3].at(i + m_nNotchLeftStartCount);
				vecY = m_pMain->m_ELB_TraceResult.m_vTraceProfile[4].at(i + m_nNotchLeftStartCount);
				vecX_p = m_pMain->m_ELB_TraceResult.m_vTraceProfile[3].at(i + m_nNotchLeftStartCount - 1);
				vecY_p = m_pMain->m_ELB_TraceResult.m_vTraceProfile[4].at(i + m_nNotchLeftStartCount - 1);

				pDC->LineTo(int(vecX), int(vecY));
				cv::line(m_pMain->m_matResultImage, cv::Point(int(m_pMain->m_ELB_TraceResult.m_vTraceProfile[3].at(i - 1)), int(m_pMain->m_ELB_TraceResult.m_vTraceProfile[4].at(i - 1))),
					cv::Point(int(m_pMain->m_ELB_TraceResult.m_vTraceProfile[3].at(i)), int(m_pMain->m_ELB_TraceResult.m_vTraceProfile[4].at(i))), cv::Scalar(0, 255, 0), 2);

_loopsimulation:
				CPointF<int> p2(vecX - inspRoi.x, vecY - inspRoi.y);
				p2.x -= (vecX - vecX_p);
				p2.y -= (vecY - vecY_p);

				if (p2.x<0 || p2.y<0 || p2.x>binOrg.cols || p2.y>binOrg.rows) continue;
				first = two = -1;
				bfind = false;

				for (double j = 0; j < 150; j += 1)
				{
					rC = Equation_of_Lines(cv::Point2f(vecX_p - inspRoi.x, vecY_p - inspRoi.y), cv::Point2f(vecX - inspRoi.x, vecY - inspRoi.y), -j);
					if (rC.x < 0)  rC.x = 0;
					if (int(rC.x) < 0 || int(rC.x) >= binOrg.cols) continue;
					if (int(rC.y) < 0 || int(rC.y) >= binOrg.rows) continue;

					if (OrgRaw[int(rC.y) * binOrg.cols + int(rC.x)] < 100)
						//if (Raw[int(rC.y) * skeleton.cols + int(rC.x)] != 0 )
					{
						CPointF<int> p_center(int(rC.x), int(rC.y));
						first = GetDistance(p_center, p2);
					}
					else if (first != -1)
					{
						nInX = int(rC.x), nInY = int(rC.y);
						break;
					}
				}
				for (double j = 0; j < 150; j += 1)
				{
					rC = Equation_of_Lines(cv::Point2f(vecX_p - inspRoi.x, vecY_p - inspRoi.y), cv::Point2f(vecX - inspRoi.x, vecY - inspRoi.y), j);
					if (rC.x < 0)  rC.x = 0;
					if (int(rC.x) < 0 || int(rC.x) >= binOrg.cols) continue;
					if (int(rC.y) < 0 || int(rC.y) >= binOrg.rows) continue;

					if (OrgRaw[int(rC.y) * binOrg.cols + int(rC.x)] < 100)
						//	if (Raw[int(rC.y) * skeleton.cols + int(rC.x)] != 0)
					{
						CPointF<int> p_center(int(rC.x), int(rC.y));
						two = GetDistance(p_center, p2);
					}
					else if (first != -1 && two != -1)
					{
						if (_bsim)
						{
							_checkArrayValue[_startsim][0] = first;
							_checkArrayValue[_startsim][1] = two;
							_checkArrayValue[_startsim][2] = nInX;
							_checkArrayValue[_startsim][3] = nInY;
							_checkArrayValue[_startsim][4] = rC.x;
							_checkArrayValue[_startsim][5] = rC.y;
							m_pMain->m_dCPos[_startsim] = cv::Point2d(first, two);
						}
						else
						{
							m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].push_back(first);		//wetout
							m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[1].push_back(two);			//metal
							m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[2].push_back(0);
							m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[3].push_back(rC.x);
							m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[4].push_back(rC.y);
							m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[3].push_back(nInX);	// IN X좌표
							m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[4].push_back(nInY);	// IN Y좌표
						}
						two = first = -1;
						bfind = true;
						break;
					}
					else break;
				}
				if (bfind == false)
				{
					if (_bsim)
					{
						_checkArrayValue[_startsim][0] = first == -1 ? 0 : first;
						_checkArrayValue[_startsim][1] = two == -1 ? 0 : two;
						_checkArrayValue[_startsim][2] = two == -1 ? vecX - inspRoi.x : rC.x;
						_checkArrayValue[_startsim][3] = two == -1 ? vecY - inspRoi.y : rC.y;
						_checkArrayValue[_startsim][4] = first == -1 ? vecX - inspRoi.x : nInX;
						_checkArrayValue[_startsim][5] = first == -1 ? vecY - inspRoi.y : nInY;
						m_pMain->m_dCPos[_startsim] = cv::Point2d(first, two);
					}
					else
					{
						m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].push_back(first == -1 ? 0 : first);						//wetout
						m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[1].push_back(two == -1 ? 0 : two);							//metal
						m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[2].push_back(0);
						m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[3].push_back(two == -1 ? vecX - inspRoi.x : rC.x);
						m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[4].push_back(two == -1 ? vecY - inspRoi.y : rC.y);
						m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[3].push_back(first == -1 ? vecX - inspRoi.x : nInX);	// IN X좌표
						m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[4].push_back(first == -1 ? vecY - inspRoi.y : nInY);	// IN Y좌표
					}
					two = first = -1;
				}
				if (_bsim) continue;

				for (int _k = 0; _k < 4; _k++)
				{
					if ((_k == 2 && (vecY < cy && vecY_p > cy)) ||
						(_k != 2 && vecX > _checkArray[_k] && vecX_p < _checkArray[_k]))
					{
						_bsim = true;
						GetLineCoef(vecX, vecY, vecX_p, vecY_p, _ax, _bc);
						vecX = _checkArray[_k];
						vecY = _ax * vecX + _bc;
						_startsim = _k;
						break;
					}
				}

				if (_bsim) goto _loopsimulation;
			}
			///////////////////////////
			bool _sfind = false;
			int _i = 0;
			vecX = 4;// m_pMain->m_ELB_TraceResult.m_vTraceProfile[3].at(0) - inspRoi.x;
			vecY = m_pMain->m_ELB_TraceResult.m_vTraceProfile[4].at(0) - inspRoi.y;
			m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.x = vecX + inspRoi.x;

			for (int j = 0; j < 500; j++)//x축
			{
				for (_i = 6; _i < 50; _i++) //y축
				{
					if (OrgRaw[int(vecY - _i) * binOrg.cols + int(vecX + j)] > 100)
					{
						if (fabs(_i * yres) > 0.05)
						{
							m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.x = vecX + inspRoi.x + j;
							_sfind = true;
						}
						break;
					}
				}
				if (_sfind) break;
				else if (_i == 50)
				{
					m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.x = vecX + inspRoi.x + j;
					break;
				}
			}

			_sfind = false;
			vecX = binOrg.cols - 4;
			vecY = m_pMain->m_ELB_TraceResult.m_vTraceProfile[4].at(nsizeT) - inspRoi.y;
			m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.y = vecX + inspRoi.x;

			for (int j = 0; j < 500; j++)//x축
			{
				for (_i = 4; _i < 50; _i++) //y축
				{
					if (OrgRaw[int(vecY - _i) * binOrg.cols + int(vecX - j)] > 100)
					{
						if (fabs(_i * yres) > 0.05)
						{
							m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.y = vecX + inspRoi.x - j;
							_sfind = true;
						}
						break;
					}
				}

				if (_sfind) break;
				else if (_i == 50)
				{
					m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.y = vecX + inspRoi.x - j;
					break;
				}
			}
			///////////////////////////
			pDC->SelectObject(&penNG);
			for (int _k = 0; _k < 4; _k++)
			{
				int _nxxx = _checkArrayValue[_k][2] + inspRoi.x;
				int _nyyy = _checkArrayValue[_k][3] + inspRoi.y;
				pDC->Ellipse(_nxxx - 10, _nyyy - 10, _nxxx + 10, _nyyy + 10);
				_nxxx = _checkArrayValue[_k][4] + inspRoi.x;
				_nyyy = _checkArrayValue[_k][5] + inspRoi.y;
				pDC->Ellipse(_nxxx - 10, _nyyy - 10, _nxxx + 10, _nyyy + 10);
			}
			pDC->SelectObject(&penOK);
		}
	}

	// 거리 최대값, 최소값 찾기
	double minmaxPosX[4] = { 0, };
	double minmaxPosY[4] = { 0, };
	double minmax_MaxValue[2] = { -9999, -99999 };
	double minmax_MinValue[2] = { 99999, 99999 };

	double minDent	= 999999,	maxDent		= -999999;
	double minDOent = 999999,	maxDOent	= -999999;
	minVal = 999999;
	maxVal = -999999;
	int Diff = 0;

	//KJH 2022-05-03 Spec 비교 추가 , Min Spec과 별개로 사이 검사 추가
	//KJH 2022-04-29 InkLack Insp 기능추가(연속성 검사, 총량 검사)
	//int _nLackOfInkAngleCount = 0, _nLackOfInkContinueCount = 0, _nLackOfInkContinueCount_Judge = 0;

	////KJH 2022-05-09 Mincount 상시 사용으로 변경 HTK 2022-05-19 CInkLack Insp 중복으로 제거
	//bool MinCheckMode				= true;
	////bool MinCheckMode			= m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getMinCountCheckInspModeEnable();
	//int	 MinCheckContinueSpec	= m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getMinCountCheckInspContinueCount();
	//int	 MinCheckTotalSpec		= m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getMinCountCheckInspTotalCount();
	//double	CInkMinWidthSpec	= m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getCInkMinWidthSpec();

	for (int i = 0; i < m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].size(); i++)
	{
		double sx = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].at(i);
		double ex = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[1].at(i);
		Diff = fabs(ex - sx);

		if (method != METHOD_CIRCLE)
		{
			Diff = fabs(ex + sx);
			double xxx = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[3].at(i) + inspRoi.x;
			if (i == 0 || m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.x > xxx ||
				m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.y < xxx)
			{
				m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[0].push_back(sx * xres); //  글라스 끝단에서 안쪽 도포 거리 WETOUT
				m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[1].push_back(ex * xres);  //  글라스 끝단에서 바깥쪽 도포 거리 METAL
				continue;
			}
		}
		else
		{
			if (i > 0)
			{
				double n_diff = fabs(m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[1].at(i) - m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[1].at(i - 1));

				if (n_diff > 20 || m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[3].at(i) == 0)
				{
					//double __angle = m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[2].at(i);
					//int __x = (cos(ToRadian(__angle)) * __dist) + colC;
					//int __y = (sin(ToRadian(__angle)) * __dist) + rowC;
					double __angle = i;
					double __dist = m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[2].at(i - 1);

					int __x = (cos(ToRadian(__angle)) * __dist) + m_ptCircleCenter_CC.x - inspRoi.x;
					int __y = (sin(ToRadian(__angle)) * __dist) + m_ptCircleCenter_CC.y - inspRoi.y;
					
					m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[0].at(i) = m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[0].at(i - 1);
					m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[1].at(i) = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].at(i) - m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[0].at(i);
					m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[2].at(i) = m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[2].at(i - 1);
					m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[3].at(i) = __x;
					m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[4].at(i) = __y;
				}
			}
				if (sx == 0)	sx = CIRCLE_RADIUS;
				if (ex == 0) 	ex = CIRCLE_RADIUS;
				Diff = fabs(ex - sx);
		}

		if (Diff > maxVal)
		{
			maxVal = Diff;
			maxIndex = i;
			maxLoc.x = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[3].at(i) + inspRoi.x;
			maxLoc.y = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[4].at(i) + inspRoi.y;
			
			
			// blue 2022.09.11 CINK1+CINK2(EGL) 에서 GlassEdge 대신 MetalEdge 사용할때, 결과 display도 MetalEdge로 변경
			if (method == METHOD_CIRCLE && m_pMain->vt_job_info[0].model_info.getInspSpecParaInfo().getCalcWetoutMetalEdge())
			{
				int colC = skeleton.cols / 2, rowC = skeleton.rows / 2;
				maxLoc.x = (cos(ToRadian(i)) * CIRCLE_RADIUS + colC) + inspRoi.x;
				maxLoc.y = (sin(ToRadian(i)) * CIRCLE_RADIUS + rowC) + inspRoi.y;
			}
			

			m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_ELB_ResultAngle[5] = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[2].at(i);
		}
		if (Diff < minVal)
		{
			minVal = Diff;
			minIndex = i;
			minLoc.x = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[3].at(i) + inspRoi.x;
			minLoc.y = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[4].at(i) + inspRoi.y;

			// blue 2022.09.11 CINK1+CINK2(EGL) 에서 GlassEdge 대신 MetalEdge 사용할때, 결과 display도 MetalEdge로 변경
			if (method == METHOD_CIRCLE && m_pMain->vt_job_info[0].model_info.getInspSpecParaInfo().getCalcWetoutMetalEdge())
			{
				int colC = skeleton.cols / 2, rowC = skeleton.rows / 2;
				minLoc.x = (cos(ToRadian(i)) * CIRCLE_RADIUS + colC) + inspRoi.x;
				minLoc.y = (sin(ToRadian(i)) * CIRCLE_RADIUS + rowC) + inspRoi.y;
			}
			
			m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_ELB_ResultAngle[4] = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[2].at(i);
		}

		if (method == METHOD_CIRCLE)
		{
			if (sx < minDent)
			{
				minDent = sx;
				m_tDopoEdgeDentAngle[0] = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[2].at(i);
				m_ptDopoEdgeDentPos[0]	= CPoint(m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[3].at(i) + inspRoi.x,
				m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[4].at(i) + inspRoi.y);
			}
			if (sx > maxDent)
			{
				maxDent = sx;
				m_tDopoEdgeDentAngle[1] = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[2].at(i);
				m_ptDopoEdgeDentPos[1] = CPoint(m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[3].at(i) + inspRoi.x,
				m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[4].at(i) + inspRoi.y);
			}

			//KJH 2022-05-26 Wetout_Out MIN,MAX 좌표 표시 추가
			if (ex < minDOent)
			{
				minDOent = ex;
				m_tDopoEdgeDOentAngle[0] = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[2].at(i);
				m_ptDopoEdgeDOentPos[0] = CPoint(m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[3].at(i) + inspRoi.x,
					m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[4].at(i) + inspRoi.y);
			}
			if (ex > maxDOent)
			{
				maxDOent = ex;
				m_tDopoEdgeDOentAngle[1] = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[2].at(i);
				m_ptDopoEdgeDOentPos[1] = CPoint(m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[3].at(i) + inspRoi.x,
					m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[4].at(i) + inspRoi.y);
			}

			if (m_pMain->vt_job_info[0].model_info.getAlignInfo().getUsePNBase())
			{
				m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[0].push_back((CIRCLE_RADIUS - sx) * xres); //  글라스 끝단에서 안쪽 도포 거리 WETOUT
				m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[1].push_back((ex - sx) * xres);			//  글라스 끝단에서 바깥쪽 도포 거리 METAL
			}
			else
			{
				if (sx == 0) sx = CIRCLE_RADIUS;
				if (ex == 0) ex = CIRCLE_RADIUS;
				m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[0].push_back((CIRCLE_RADIUS - sx) * xres); //  글라스 끝단에서 안쪽 도포 거리 WETOUT
				m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[1].push_back((ex - CIRCLE_RADIUS) * xres); //  글라스 끝단에서 바깥쪽 도포 거리 METAL
			}
		}
		else
		{
			m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[0].push_back(sx * xres);						//  글라스 끝단에서 안쪽 도포 거리 WETOUT
			m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[1].push_back(ex * xres);						//  글라스 끝단에서 바깥쪽 도포 거리 METAL
		}

		sx = m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[0].at(i);
		ex = m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[1].at(i);

		if (sx < minmax_MinValue[0])
		{
			minmax_MinValue[0] = sx;
			minmaxPosX[0] = m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[3].at(i) + inspRoi.x;
			minmaxPosY[0] = m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[4].at(i) + inspRoi.y;
			m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_ELB_ResultAngle[0] = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[2].at(i);
		}

		if (sx > minmax_MaxValue[0])
		{
			minmax_MaxValue[0] = sx;
			minmaxPosX[1] = m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[3].at(i) + inspRoi.x;
			minmaxPosY[1] = m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[4].at(i) + inspRoi.y;
			m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_ELB_ResultAngle[1] = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[2].at(i);
		}

		if (ex < minmax_MinValue[1])
		{
			minmax_MinValue[1] = ex;
			minmaxPosX[2] = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[3].at(i) + inspRoi.x;
			minmaxPosY[2] = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[4].at(i) + inspRoi.y;
			m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_ELB_ResultAngle[2] = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[2].at(i);
		}

		if (ex > minmax_MaxValue[1])
		{
			minmax_MaxValue[1] = ex;
			minmaxPosX[3] = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[3].at(i) + inspRoi.x;
			minmaxPosY[3] = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[4].at(i) + inspRoi.y;
			m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_ELB_ResultAngle[3] = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[2].at(i);
		}

	}


	if (method == METHOD_CIRCLE)
	{
		//Tkyuha 2022-03-10 도포 반대편 1/2 평균값 계산 추가
		int _minVal			 = 999999;
		int _maxVal			 = -999999;
		int _halfcount		 = int(m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].size() / 2);
		int _Totalcount		 = int(m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].size());
		int _Checkangle		 = 0;
		int _CountCheckangle = 0;
		bool   bWetoutRJudgeModeEnable	= m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getWetoutRJudgeModeEnable();
		double RminSpec					= m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getWetoutRMinSpec();
		double RmaxSpec					= m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getWetoutRMaxSpec();
		bool   bWetoutBJudgeModeEnable	= m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getWetoutBJudgeModeEnable();
		double BminSpec					= m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getWetoutBMinSpec();
		double BmaxSpec                 = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getWetoutBMaxSpec(); //22.06.02

		for (int i = 0; i < _halfcount; i++)
		{
			double sx = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].at(i);
			double ex = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[1].at(i);
			double sx2 = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].at(i + _halfcount);
			double ex2 = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[1].at(i + _halfcount);

			if (sx == 0)	sx  = CIRCLE_RADIUS;
			if (ex == 0) 	ex  = CIRCLE_RADIUS;
			if (sx2 == 0)	sx2 = CIRCLE_RADIUS;
			if (ex2 == 0) 	ex2 = CIRCLE_RADIUS;
			// Tkyuha 220318 중간값에서 4.7 고정 값에서 빼서 계산으로 변경
			//KJH 2022-05-05 R Diff 계산방식 변경 마주보는 사이 거리 측정
			Diff = (fabs(sx) + fabs(sx2)); 
			
			double _dist_l = fabs(sx + sx2);
			//Diff = (4.7 / xres - _dist_l) / 2.;

			if (bWetoutRJudgeModeEnable)
			{
				if (RminSpec > Diff * xres || RmaxSpec < Diff * xres)
				{
					m_pMain->m_ELB_DiffInspResult[datasheet].bRJudge = FALSE;
				}
			}

			if (_Checkangle == i && _Checkangle < 179)
			{
				m_pMain->m_ELB_DiffInspResult[datasheet].m_RCheckangle[_CountCheckangle] = Diff * xres;
				_Checkangle = _Checkangle + 45;
				_CountCheckangle++;
			}

			if (Diff > _maxVal)
			{
				_maxVal = Diff;
				RmaxIndex = i;
			}

			if (Diff < _minVal)
			{
				_minVal = Diff;
				RminIndex = i;
			}
		}
		
		m_pMain->m_ELB_DiffInspResult[datasheet].m_RminResult	= _minVal * xres;
		m_pMain->m_ELB_DiffInspResult[datasheet].m_RminAngle	= RminIndex;
		m_pMain->m_ELB_DiffInspResult[datasheet].m_RmaxResult	= _maxVal * xres;
		m_pMain->m_ELB_DiffInspResult[datasheet].m_RmaxAngle	= RmaxIndex;

		if (!m_pMain->m_ELB_DiffInspResult[datasheet].bRJudge)
		{
			str.Format("[%s] R Spec NG : R Min - %.3f, R Max - %.3f", m_pMain->vt_job_info[nJob].job_name.c_str(), _minVal* xres, _maxVal* xres);
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		}

		//KJH 2022-05-25 BM In <-> Cink In Insp 추가
		_minVal					= 999999;
		double _minVal_Original = 99999;
		_maxVal					= -99999;
		_Checkangle				= 0;
		_CountCheckangle		= 0;

		for (int i = 0; i < _Totalcount; i++)
		{
			Diff = m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[1].at(i);
			
			if (Diff > _maxVal)
			{
				_maxVal = Diff;
				BmaxIndex = i;
			}

			if (Diff < _minVal_Original)
			{
				//이전값(두번째값)
				_minVal = _minVal_Original;
				//최신값
				_minVal_Original = Diff;
				BminIndex = i;
			}
			else if (_minVal_Original < Diff && Diff < _minVal)
			{
				//Min_Old < Diff < Min
				//이전값(두번째값) 갱신
				_minVal = Diff;
				BminIndex = i;
			}

			//KJH 2022-06-06 B 특수각 로그 및 보고관련 추가
			if (_Checkangle == i && _Checkangle < 359)
			{
				m_pMain->m_ELB_DiffInspResult[datasheet].m_BCheckangle[_CountCheckangle] = Diff * xres;
				_Checkangle = _Checkangle + 45;
				_CountCheckangle++;
			}
		}

		if (bWetoutBJudgeModeEnable) // 22.06.02
		{
			if (BminSpec > _minVal * xres || BmaxSpec < _maxVal * xres)
			{
				m_pMain->m_ELB_DiffInspResult[datasheet].bBJudge = FALSE;				
			}
		}

		m_pMain->m_ELB_DiffInspResult[datasheet].m_BminResult	= _minVal * xres;
		m_pMain->m_ELB_DiffInspResult[datasheet].m_BminAngle	= BminIndex;
		m_pMain->m_ELB_DiffInspResult[datasheet].m_BmaxResult	= _maxVal * xres;
		m_pMain->m_ELB_DiffInspResult[datasheet].m_BmaxAngle	= BmaxIndex;

		if (!m_pMain->m_ELB_DiffInspResult[datasheet].bBJudge)
		{
			if (BminSpec > _minVal * xres)
			{
				// KBJ 2022-10-10 검사 에러타입추가.	======================================
				m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType += ELB_Error_Type(m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType, ELB_INSP_ERROR_B_MIN);
				//	==========================================================================
			}
			if (BmaxSpec < _maxVal * xres)
			{
				// KBJ 2022-10-10 검사 에러타입추가.	======================================
				m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType += ELB_Error_Type(m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType, ELB_INSP_ERROR_B_MAX);
				//	==========================================================================
			}
			bReturn = false; //22.06.02
			str.Format("[%s] B Spec NG : B Min - %.4f", m_pMain->vt_job_info[nJob].job_name.c_str(), _minVal * xres);
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		}

		memset(m_pMain->m_dWetoutProperty, 0, sizeof(double) * 10);

		int _nCircleSize = int(m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[0].size());
		
		if (_nCircleSize > 10)
		{
			double* XiData = new double[_nCircleSize];
			double* YiData = new double[_nCircleSize];
			double totalSumWetOut = 0; // Wetout 평균 구하기 위함
			double varianceWetOut = 0; // Wetout  표준편차 구하기 위함

			for (int i = 0; i < _nCircleSize; i++)
			{
				XiData[i] = m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[3].at(i) + inspRoi.x;
				YiData[i] = m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[4].at(i) + inspRoi.y;
				totalSumWetOut+=m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[0].at(i);
			}

			CircleData data1(_nCircleSize, XiData, YiData);
			CircleFit FitCircle = CircleFitByHyper(data1);
			if (FitCircle.r > 0)
			{
				m_pMain->m_dWetoutProperty[0] = FitCircle.a; // 중심x
				m_pMain->m_dWetoutProperty[1] = FitCircle.b; // 중심y
				m_pMain->m_dWetoutProperty[2] = FitCircle.r; // 반지름
				m_pMain->m_dWetoutProperty[3] = FitCircle.s; // 시그마
				m_pMain->m_dWetoutProperty[4] = totalSumWetOut / _nCircleSize; // 평균	

				for (int i = 0; i < _nCircleSize; i++)
				{
					varianceWetOut += pow(m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[0].at(i) - m_pMain->m_dWetoutProperty[4], 2);
				}
				m_pMain->m_dWetoutProperty[5] = sqrt(varianceWetOut / (_nCircleSize - 1)); // 표준편차

				std::vector<double> destVector;
				destVector.assign(m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[0].begin(), m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[0].end());
				m_pMain->m_dWetoutProperty[6] = CalcMHWScore(destVector);  //벡터 중간값 출력
				int _q1 = int(destVector.size() * 0.25);
				int _q3 = int(destVector.size() * 0.75);
				if (_q1 >= 0 && _q1 < destVector.size() - 1) m_pMain->m_dWetoutProperty[7] = (destVector[_q1] + destVector[_q1 + 1]) / 2.;  //1사분위수 출력
				if (_q3 >= 0 && _q3 < destVector.size() - 1) m_pMain->m_dWetoutProperty[8] = (destVector[_q3] + destVector[_q3 + 1]) / 2.;  //3사분위수 출력
				destVector.clear();

				pDC->AddGraphic(new GraphicPoint(FitCircle.a, FitCircle.b, pDC->GetCurrentColor()));
				//pDC->Ellipse(int(FitCircle.a - 1), int(FitCircle.b - 1), int(FitCircle.a + 1), int(FitCircle.b + 1));
				cv::circle(m_pMain->m_matResultImage, cv::Point(FitCircle.a, FitCircle.b), 1, cv::Scalar(0, 255, 0), 2, 5);

				pDC->Ellipse(int(FitCircle.a - FitCircle.r), int(FitCircle.b - FitCircle.r), int(FitCircle.a + FitCircle.r), int(FitCircle.b + FitCircle.r));
				cv::circle(m_pMain->m_matResultImage, cv::Point(FitCircle.a, FitCircle.b), FitCircle.r, cv::Scalar(64, 255, 255), 2, 5);
			}
			delete [] XiData;
			delete [] YiData;
		}
	}

	if (m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].size() > 0)
	{
		int lm = int(m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].size() - 1);
		if (m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.x != -1 && m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.y == -1)
			m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.y = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[3].at(lm) + inspRoi.x;
	}

	if (method == METHOD_CIRCLE)
	{
		int xX = int(m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x);// W / 2;// m_pMain->m_dCurrentSearchPos[2];
		int yY = int(m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y);// H / 2;// m_pMain->m_dCurrentSearchPos[2];

		pDC->SelectObject(&penNG);
		// Target Circle 그리기
		pDC->Ellipse(int(xX - CIRCLE_RADIUS), int(yY - CIRCLE_RADIUS), int(xX + CIRCLE_RADIUS), int(yY + CIRCLE_RADIUS));
		cv::circle(m_pMain->m_matResultImage, cv::Point(xX, yY), CIRCLE_RADIUS, cv::Scalar(0, 0, 255), 2, 5);


		pDC->SelectObject(&penOrange);
		xX = m_ptCircleCenter_MP.x;
		yY = m_ptCircleCenter_MP.y;
		pDC->Ellipse(int(xX - m_dCircleRadius_MP), int(yY - m_dCircleRadius_MP), int(xX + m_dCircleRadius_MP), int(yY + m_dCircleRadius_MP));
		cv::circle(m_pMain->m_matResultImage, cv::Point(xX, yY), m_dCircleRadius_MP, cv::Scalar(0, 125, 255), 2, 5);
	}

	/// 외곽선 그리기
	if (method == METHOD_LINE)	pDC->SelectObject(&penCut);
	else						pDC->SelectObject(&penOK);

	auto cl = pDC->GetCurrentColor();
	for (int kk = 0; kk < m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[3].size(); kk++)
	{
		int LocX = m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[3].at(kk) + inspRoi.x;
		int LocY = m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[4].at(kk) + inspRoi.y;
		pDC->Ellipse(int(LocX - 1), int(LocY - 1), int(LocX + 1), int(LocY + 1));
		//pDC->AddGraphic(new GraphicPoint(LocX, LocY, cl));
		//SJB 2022-12-02 Dopo 외곽선 저장 이미지 색상 통일
		cv::circle(m_pMain->m_matResultImage, cv::Point(LocX, LocY), 1, CV_RGB(128, 255, 128), 2, 5);
		LocX = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[3].at(kk) + inspRoi.x;
		LocY = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[4].at(kk) + inspRoi.y;
		pDC->Ellipse(int(LocX - 1), int(LocY - 1), int(LocX + 1), int(LocY + 1));
		//pDC->AddGraphic(new GraphicPoint(LocX, LocY, cl));
		//SJB 2022-12-02 Dopo 외곽선 저장 이미지 색상 통일
		cv::circle(m_pMain->m_matResultImage, cv::Point(LocX, LocY), 1, CV_RGB(255, 255, 64), 2, 5);
	}
	for (int kk = 0; kk < m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[3].size(); kk++)
	{
		pDC->SelectObject(&penBM);
		int LocX = m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[3].at(kk) + inspRoi.x;
		int LocY = m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[4].at(kk) + inspRoi.y;
		pDC->Ellipse(int(LocX - 1), int(LocY - 1), int(LocX + 1), int(LocY + 1));

		cl = pDC->GetCurrentColor();
		//pDC->AddGraphic(new GraphicPoint(LocX, LocY, cl));
		if (_bOverflowDefect_Flag[kk] == TRUE) cv::circle(m_pMain->m_matResultImage, cv::Point(LocX, LocY), 3, cv::Scalar(0, 0, 255), 2, 5);
		cv::circle(m_pMain->m_matResultImage, cv::Point(LocX, LocY), 1, cv::Scalar(255, 216, 0), 2, 5);
	}

	//SJB 2022-11-11 Overflow Min 판정 추가
	if (method == METHOD_LINE)
	{
		int center_X = int(m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[3].size() / 2);
		int left_X = MAX(0,int(center_X - (1 / xres)));
		int right_X = MIN(int(m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[1].size()),int(center_X + (1 / xres)));
		int maxMinValue = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[1].at(left_X);
		int minValue = 0;

		//SJB 2022-11-14 Insp Y1, Y2 범위 표시
		pDC->SelectObject(&penBM);
		int centerX = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[3].at(center_X) + inspRoi.x;
		int leftX = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[3].at(left_X) + inspRoi.x;
		int rightX = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[3].at(right_X) + inspRoi.x;

		pDC->MoveTo(centerX, 0);
		pDC->LineTo(centerX, H - 1);
		cv::line(m_pMain->m_matResultImage, cv::Point(centerX, 0), cv::Point(centerX, H - 1), CV_RGB(0, 216, 255), 3);
		pDC->MoveTo(leftX, 0);
		pDC->LineTo(leftX, H - 1);
		cv::line(m_pMain->m_matResultImage, cv::Point(leftX, 0), cv::Point(leftX, H - 1), CV_RGB(0, 216, 255), 3);
		pDC->MoveTo(rightX, 0);
		pDC->LineTo(rightX, H - 1);
		cv::line(m_pMain->m_matResultImage, cv::Point(rightX, 0), cv::Point(rightX, H - 1), CV_RGB(0, 216, 255), 3);


		pDC->SelectObject(&penMin);
		for (int i = left_X; i <= right_X; i++)
		{
			minValue = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[1].at(i);

			if (maxMinValue > minValue)
				maxMinValue = minValue;

			if (minValue * yres < m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getLineOverflowMinSpec())
			{
				int LocX = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[3].at(i) + inspRoi.x;
				int LocY = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[4].at(i) + inspRoi.y;
				pDC->Ellipse(int(LocX - 1), int(LocY - 1), int(LocX + 1), int(LocY + 1));
				cv::circle(m_pMain->m_matResultImage, cv::Point(LocX, LocY), 1, cv::Scalar(255, 125, 0), 2, 5);
			}
		}

		if (maxMinValue * yres < m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getLineOverflowMinSpec())
		{
			m_InspErrorCode = EGL_INSP_ERROR_OVERFLOW_MIN;
			m_pMain->m_ELB_DiffInspResult[datasheet].m_bResultSummary = FALSE;
		}
		minmax_MinValue[1] = maxMinValue * yres;
	}

	pDC->SelectObject(&penNG);

	//SJB 2022-12-02 not use
	//if (minIndex != -1)
	if(0)
	{   // 도포 폭이 최소 최대값 표시
		int LocX = m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[3].at(minIndex) + inspRoi.x;
		int LocY = m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[4].at(minIndex) + inspRoi.y;
		//KJH 2022-05-26 도포 최소,최대 폭 디스플레이 간소화
		//pDC->MoveTo(minLoc.x, minLoc.y);
		//pDC->LineTo(LocX, LocY);

		//mViewer->AddSoftGraphic(new GraphicArrow(Gdiplus::PointF(minLoc.x, minLoc.y), Gdiplus::PointF(LocX, LocY), 11, Gdiplus::Color::Red, true, 2));

		//pDC->Rectangle(minLoc.x - 2, minLoc.y - 2, LocX + 2, LocY + 2);
		//cv::rectangle(m_pMain->m_matResultImage, cv::Point(minLoc.x - 2, minLoc.y - 2), cv::Point(LocX + 2, LocY + 2), CV_RGB(255, 0, 0), 3);
		//cv::line(m_pMain->m_matResultImage, cv::Point(minLoc.x, minLoc.y), cv::Point(LocX, LocY), CV_RGB(255, 0, 0), 3);
		if (maxIndex != -1)
		{
			LocX = m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[3].at(maxIndex) + inspRoi.x;
			LocY = m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[4].at(maxIndex) + inspRoi.y;
			//KJH 2022-05-26 도포 최소,최대 폭 디스플레이 간소화
			//pDC->MoveTo(maxLoc.x, maxLoc.y);
			//pDC->LineTo(LocX, LocY);

			//mViewer->AddSoftGraphic(new GraphicArrow(Gdiplus::PointF(maxLoc.x, maxLoc.y), Gdiplus::PointF(LocX, LocY), 11, Gdiplus::Color::Red, true, 2));

			//pDC->Rectangle(maxLoc.x - 2, maxLoc.y - 2, LocX + 2, LocY + 2);
			//cv::rectangle(m_pMain->m_matResultImage, cv::Point(maxLoc.x - 2, maxLoc.y - 2), cv::Point(LocX + 2, LocY + 2), CV_RGB(255, 0, 0), 3);
			//cv::line(m_pMain->m_matResultImage, cv::Point(maxLoc.x, maxLoc.y), cv::Point(LocX, LocY), CV_RGB(255, 0, 0), 3);
		}
	}

	CString strText; // 결과값 표시 (최대, 최소 거리)
	if (method == METHOD_CIRCLE)
	{
		// Tkyuha 20220405 화면 정리 용으로 삭제 SDC 요청 사항
		//if (minIndex != -1)
		//{
		//	double sx = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].at(minIndex);
		//	double ex = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[1].at(minIndex);
		//	double angle = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[2].at(minIndex);
		//	if (sx == 0) sx = CIRCLE_RADIUS;
		//	if (ex == 0) ex = CIRCLE_RADIUS;

		//	double c = fabs(ex - sx);
		//	strText.Format("Min :(%d,%d_%.1fDeg) %.3fmm", minLoc.x, minLoc.y, angle, c * yres);
		//	pDC->TextOutA(minLoc.x, minLoc.y, strText);
		//	cv::putText(m_pMain->m_matResultImage, std::string(strText), cv::Point(minLoc.x, minLoc.y), cv::FONT_ITALIC, 2, cv::Scalar(0, 0, 255), 5);
		//}
		//if (maxIndex != -1)
		//{
		//	double sx = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].at(maxIndex);
		//	double ex = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[1].at(maxIndex);
		//	double angle = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[2].at(maxIndex);
		//	if (sx == 0) sx = CIRCLE_RADIUS;
		//	if (ex == 0) ex = CIRCLE_RADIUS;
		//	double c = fabs(ex - sx);
		//	strText.Format("Max :(%d,%d_%.1fDeg) %.3fmm", maxLoc.x, maxLoc.y, angle, c * yres);
		//	pDC->TextOutA(maxLoc.x, maxLoc.y, strText);
		//	cv::putText(m_pMain->m_matResultImage, std::string(strText), cv::Point(maxLoc.x, maxLoc.y), cv::FONT_ITALIC, 2, cv::Scalar(0, 0, 255), 5);

		//}
		////  4방향 결과 그리기
		//int step = int(m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].size() / 4);
		//if (step > 0)
		//{
		//	pDC->SetTextColor(COLOR_GREEN);
		//	for (int i = 0; i < 4; i++)
		//	{
		//		double sx = m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[0].at(step * i);
		//		double ex = m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[1].at(step * i);
		//		double angle = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[2].at(step * i);
		//		int xpos = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[3].at(step * i);
		//		int ypos = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[4].at(step * i);
		//		//sx.... ->IN 방향 값, ex.....-> OUT 방향 값
		//		strText.Format("L:(%.3f,%.3f)mm_%.1fDeg", sx, ex, angle);
		//		pDC->TextOutA(xpos + inspRoi.x, ypos + inspRoi.y + 100, strText);
		//		cv::putText(m_pMain->m_matResultImage, std::string(strText), cv::Point(xpos + inspRoi.x, ypos + inspRoi.y), cv::FONT_ITALIC, 2, cv::Scalar(0, 255, 0), 5);
		//		cv::circle(m_pMain->m_matResultImage, cv::Point(xpos + inspRoi.x, ypos + inspRoi.y), 5, cv::Scalar(0, 0, 255), 2, 5);

		//	}
		//}

		auto xX = (m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x);// W / 2;// m_pMain->m_dCurrentSearchPos[2];
		auto yY = (m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y);// H / 2;// m_pMain->m_dCurrentSearchPos[2];

		//  0 도 방향 표시
		double x_ = (cos(ToRadian(90)) * (CIRCLE_RADIUS + 50) + xX);
		double y_ = (sin(ToRadian(90)) * (CIRCLE_RADIUS + 50) + yY);
		double x2_ = (cos(ToRadian(90)) * (CIRCLE_RADIUS + 100) + xX);
		double y2_ = (sin(ToRadian(90)) * (CIRCLE_RADIUS + 100) + yY);
		DrawArrow(pDC, RGB(192, 128, 64), CPoint(x2_, y2_), CPoint(x_, y_), ToRadian(90));

		//  -40 도 방향 표시
		x_ = (cos(ToRadian(90 - 40)) * (CIRCLE_RADIUS + 50) + xX);
		y_ = (sin(ToRadian(90 - 40)) * (CIRCLE_RADIUS + 50) + yY);
		x2_ = (cos(ToRadian(90 - 40)) * (CIRCLE_RADIUS + 100) + xX);
		y2_ = (sin(ToRadian(90 - 40)) * (CIRCLE_RADIUS + 100) + yY);
		DrawArrow(pDC, RGB(255, 0, 0), CPoint(x2_, y2_), CPoint(x_, y_), ToRadian(90 - 40));

		//  40 도 방향 표시
		x_ = (cos(ToRadian(90 + 40)) * (CIRCLE_RADIUS + 50) + xX);
		y_ = (sin(ToRadian(90 + 40)) * (CIRCLE_RADIUS + 50) + yY);
		x2_ = (cos(ToRadian(90 + 40)) * (CIRCLE_RADIUS + 100) + xX);
		y2_ = (sin(ToRadian(90 + 40)) * (CIRCLE_RADIUS + 100) + yY);
		DrawArrow(pDC, RGB(0, 255, 0), CPoint(x2_, y2_), CPoint(x_, y_), ToRadian(90 + 40));

		pDC->SelectObject(&penSum);
		// Wetout Min, Max Circle 그리기
		pDC->Ellipse(int(m_ptDopoEdgeDentPos[0].x - 10), int(m_ptDopoEdgeDentPos[0].y - 10),
			int(m_ptDopoEdgeDentPos[0].x + 10), int(m_ptDopoEdgeDentPos[0].y + 10));
		cv::circle(m_pMain->m_matResultImage, cv::Point(m_ptDopoEdgeDentPos[0].x, m_ptDopoEdgeDentPos[0].y), 5, cv::Scalar(255, 0, 0), 2, 5);
		pDC->Ellipse(int(m_ptDopoEdgeDentPos[1].x - 10), int(m_ptDopoEdgeDentPos[1].y - 10),
			int(m_ptDopoEdgeDentPos[1].x + 10), int(m_ptDopoEdgeDentPos[1].y + 10));
		cv::circle(m_pMain->m_matResultImage, cv::Point(m_ptDopoEdgeDentPos[1].x, m_ptDopoEdgeDentPos[1].y), 5, cv::Scalar(255, 0, 0), 2, 5);

		pDC->SelectObject(&penDB);
		//KJH 2022-05-26 Wetout_Out MIN,MAX 좌표 표시 추가
		pDC->Ellipse(int(m_ptDopoEdgeDOentPos[0].x - 10), int(m_ptDopoEdgeDOentPos[0].y - 10),
			int(m_ptDopoEdgeDOentPos[0].x + 10), int(m_ptDopoEdgeDOentPos[0].y + 10));
		cv::circle(m_pMain->m_matResultImage, cv::Point(m_ptDopoEdgeDOentPos[0].x, m_ptDopoEdgeDOentPos[0].y), 5, cv::Scalar(255, 128, 128), 2, 5);
		pDC->Ellipse(int(m_ptDopoEdgeDOentPos[1].x - 10), int(m_ptDopoEdgeDOentPos[1].y - 10),
			int(m_ptDopoEdgeDOentPos[1].x + 10), int(m_ptDopoEdgeDOentPos[1].y + 10));
		cv::circle(m_pMain->m_matResultImage, cv::Point(m_ptDopoEdgeDOentPos[1].x, m_ptDopoEdgeDOentPos[1].y), 5, cv::Scalar(255, 128, 128), 2, 5);

		strText.Format("T:%.1fDeg", m_tDopoEdgeDentAngle[0]);
		//pDC->TextOutA(m_ptDopoEdgeDentPos[0].x, m_ptDopoEdgeDentPos[0].y, strText);
		//cv::putText(m_pMain->m_matResultImage, std::string(strText), cv::Point(m_ptDopoEdgeDentPos[0].x, m_ptDopoEdgeDentPos[0].y), cv::FONT_ITALIC, 2, cv::Scalar(0, 255, 0), 5);
		strText.Format("T:%.1fDeg", m_tDopoEdgeDentAngle[1]);
		//pDC->TextOutA(m_ptDopoEdgeDentPos[1].x, m_ptDopoEdgeDentPos[1].y, strText);
		//cv::putText(m_pMain->m_matResultImage, std::string(strText), cv::Point(m_ptDopoEdgeDentPos[1].x, m_ptDopoEdgeDentPos[1].y), cv::FONT_ITALIC, 2, cv::Scalar(0, 255, 0), 5);

		pDC->SetTextColor(COLOR_PURPLE); // 20220612 Tkyuha color 변경
		pDC->SelectObject(&penDebug);

		x_ = m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[3].at(m_pMain->m_ELB_DiffInspResult[datasheet].m_BminAngle) + inspRoi.x;				// BM_In X 좌표
		y_ = m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[4].at(m_pMain->m_ELB_DiffInspResult[datasheet].m_BminAngle) + inspRoi.y;
		strText.Format("BMin : %.3fmm", m_pMain->m_ELB_DiffInspResult[datasheet].m_BminResult); //2022-06-08 Tkyuha Bmin Bmax 그리기
		pDC->TextOutA(x_, y_, strText);
		cv::putText(m_pMain->m_matResultImage, std::string(strText), cv::Point(x_, y_ - 20), cv::FONT_ITALIC, 2, cv::Scalar(255, 64, 255), 5);
		pDC->Ellipse(int(x_ - 10), int(y_ - 10),	int(x_ + 10), int(y_ + 10));
		cv::circle(m_pMain->m_matResultImage, cv::Point(x_, y_), 5, cv::Scalar(255, 64, 255), 2, 5);

		x_ = m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[3].at(m_pMain->m_ELB_DiffInspResult[datasheet].m_BmaxAngle) + inspRoi.x;				// BM_In X 좌표
		y_ = m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[4].at(m_pMain->m_ELB_DiffInspResult[datasheet].m_BmaxAngle) + inspRoi.y;
		strText.Format("BMax : %.3fmm", m_pMain->m_ELB_DiffInspResult[datasheet].m_BmaxResult);
		pDC->TextOutA(x_, y_, strText);
		cv::putText(m_pMain->m_matResultImage, std::string(strText), cv::Point(x_, y_ - 20), cv::FONT_ITALIC, 2, cv::Scalar(255, 64, 255), 5);
		pDC->Ellipse(int(x_ - 10), int(y_ - 10), int(x_ + 10), int(y_ + 10));
		cv::circle(m_pMain->m_matResultImage, cv::Point(x_, y_), 5, cv::Scalar(255, 64, 255), 2, 5);
	}
	else if (method == METHOD_NOTCH)
	{
		if (minIndex != -1)
		{
			double sx = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].at(minIndex);
			double ex = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[1].at(minIndex);
			double c = fabs(ex + sx);
			strText.Format("Min(%d,%d) %.3fmm", minLoc.x, minLoc.y, c * yres);
			pDC->TextOutA(minLoc.x, minLoc.y, strText);
			cv::putText(m_pMain->m_matResultImage, std::string(strText), cv::Point(minLoc.x, minLoc.y), cv::FONT_ITALIC, 2, cv::Scalar(0, 0, 255), 5);
		}
		if (maxIndex != -1)
		{
			double sx = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].at(maxIndex);
			double ex = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[1].at(maxIndex);
			double c = fabs(ex + sx);
			strText.Format("Max(%d,%d) %.3fmm", maxLoc.x, maxLoc.y, c * yres);
			pDC->TextOutA(maxLoc.x, maxLoc.y, strText);
			cv::putText(m_pMain->m_matResultImage, std::string(strText), cv::Point(maxLoc.x, maxLoc.y), cv::FONT_ITALIC, 2, cv::Scalar(0, 0, 255), 5);
		}

		strText.Format("Min(%d,%d) %.3fmm", minLoc.x, minLoc.y, minVal * yres);
		pDC->TextOutA(minLoc.x, minLoc.y, strText);
		cv::putText(m_pMain->m_matResultImage, std::string(strText), cv::Point(minLoc.x, minLoc.y), cv::FONT_ITALIC, 2, cv::Scalar(0, 0, 255), 5);
		strText.Format("Max(%d,%d) %.3fmm", maxLoc.x, maxLoc.y, maxVal * yres);
		pDC->TextOutA(maxLoc.x, maxLoc.y, strText);
		cv::putText(m_pMain->m_matResultImage, std::string(strText), cv::Point(maxLoc.x, maxLoc.y), cv::FONT_ITALIC, 2, cv::Scalar(0, 0, 255), 5);
		if (m_vecInterSectionPoint.size() > 1)  // Metal Line 그리기
		{
			pDC->SelectObject(&penCut);
			pDC->MoveTo(int(m_vecInterSectionPoint.at(0).x), int(m_vecInterSectionPoint.at(0).y));

			for (int i = 1; i < m_vecInterSectionPoint.size(); i++)
			{
				pDC->LineTo(int(m_vecInterSectionPoint.at(i).x), int(m_vecInterSectionPoint.at(i).y));
				cv::line(m_pMain->m_matResultImage, cv::Point(int(m_vecInterSectionPoint.at(i - 1).x), int(m_vecInterSectionPoint.at(i - 1).y)), cv::Point(int(m_vecInterSectionPoint.at(i).x), int(m_vecInterSectionPoint.at(i).y)), cv::Scalar(0, 0, 255), 3);
			}
		}

		for (int i = 0; i < 4; i++)
		{
			double sx_ = _checkArrayValue[i][0];
			double ex_ = _checkArrayValue[i][1];
			int p__x = _checkArrayValue[i][2] + inspRoi.x;
			int p__y = _checkArrayValue[i][3] + inspRoi.y;
			pDC->MoveTo(p__x, p__y - 20);
			pDC->LineTo(p__x, p__y + 20);
			strText.Format("C%d (%d,%d)  -  %.3fmm, %.3fmm", 4 - i, p__x, p__y, sx_ * xres, ex_ * xres);
			pDC->TextOutA(p__x, p__y, strText);
			cv::line(m_pMain->m_matResultImage, cv::Point(p__x, p__y - 20), cv::Point(p__x, p__y + 20), cv::Scalar(0, 0, 255), 3);
			cv::putText(m_pMain->m_matResultImage, std::string(strText), cv::Point(p__x, p__y), cv::FONT_ITALIC, 2, cv::Scalar(0, 0, 255), 5);
		}
	}

	pDC->SetTextColor(COLOR_BLUE);

	if (method == METHOD_LINE)
	{
		//SJB 2022-11-07 Wetout, Overflow Min Max Draw 시점 변경
		////Min
		//pDC->SelectObject(&penSum);
		//pDC->Rectangle(int(minmaxPosX[0] - 4), int(minmaxPosY[0] - 4), int(minmaxPosX[0] + 4), int(minmaxPosY[0] + 4));
		//pDC->Rectangle(int(minmaxPosX[2] - 4), int(minmaxPosY[2] - 4), int(minmaxPosX[2] + 4), int(minmaxPosY[2] + 4));

		////Max
		//pDC->SelectObject(&penRed);
		//pDC->Rectangle(int(minmaxPosX[1] - 4), int(minmaxPosY[1] - 4), int(minmaxPosX[1] + 4), int(minmaxPosY[1] + 4));
		//pDC->Rectangle(int(minmaxPosX[3] - 4), int(minmaxPosY[3] - 4), int(minmaxPosX[3] + 4), int(minmaxPosY[3] + 4));

		//SJB 2022-12-02 Min, Max 저장 이미지 색상 통일
		pDC->SelectObject(&penSum);
		cv::rectangle(m_pMain->m_matResultImage, cv::Point(int(minmaxPosX[0] - 4), int(minmaxPosY[0] - 4)), cv::Point(int(minmaxPosX[0] + 4), int(minmaxPosY[0] + 4)), CV_RGB(173, 255, 47), 3);
		cv::rectangle(m_pMain->m_matResultImage, cv::Point(int(minmaxPosX[1] - 4), int(minmaxPosY[1] - 4)), cv::Point(int(minmaxPosX[1] + 4), int(minmaxPosY[1] + 4)), CV_RGB(173, 255, 47), 3);
		cv::rectangle(m_pMain->m_matResultImage, cv::Point(int(minmaxPosX[2] - 4), int(minmaxPosY[2] - 4)), cv::Point(int(minmaxPosX[2] + 4), int(minmaxPosY[2] + 4)), CV_RGB(128, 128, 255), 3);
		cv::rectangle(m_pMain->m_matResultImage, cv::Point(int(minmaxPosX[3] - 4), int(minmaxPosY[3] - 4)), cv::Point(int(minmaxPosX[3] + 4), int(minmaxPosY[3] + 4)), CV_RGB(128, 128, 255), 3);
	}
	else
	{
		//KJH 2022-05-26 Display UI 개선 작업 진행
		//strText.Format("InMin :(%d,%d) %.3fmm", int(minmaxPosX[0]), int(minmaxPosY[0]), minmax_MinValue[0]);
		//pDC->TextOutA(minmaxPosX[0], minmaxPosY[0], strText);
		//cv::putText(m_pMain->m_matResultImage, std::string(strText), cv::Point(minmaxPosX[0], minmaxPosY[0] + 20), cv::FONT_ITALIC, 2, cv::Scalar(255, 0, 0), 5);
		pDC->SelectObject(&penSum);
		pDC->SetTextColor(COLOR_BLUE);

		strText.Format("InMin : %.3fmm", minmax_MinValue[0]);
		pDC->TextOutA(minmaxPosX[0], minmaxPosY[0], strText);
		cv::putText(m_pMain->m_matResultImage, std::string(strText), cv::Point(minmaxPosX[0], minmaxPosY[0] + 20), cv::FONT_ITALIC, 2, cv::Scalar(255, 0, 0), 5);
		pDC->Rectangle(int(minmaxPosX[0] - 50), int(minmaxPosY[0] - 50), int(minmaxPosX[0] + 50), int(minmaxPosY[0] + 50));
		cv::rectangle(m_pMain->m_matResultImage, cv::Point(int(minmaxPosX[0] - 50), int(minmaxPosY[0] - 50)), cv::Point(int(minmaxPosX[0] + 50), int(minmaxPosY[0] + 50)), cv::Scalar(255, 64, 64), 3);
		//strText.Format("InMax :(%d,%d) %.3fmm", int(minmaxPosX[1]), int(minmaxPosY[1]), minmax_MaxValue[0]);
		//pDC->TextOutA(minmaxPosX[1], minmaxPosY[1], strText);
		//cv::putText(m_pMain->m_matResultImage, std::string(strText), cv::Point(minmaxPosX[1], minmaxPosY[1] + 20), cv::FONT_ITALIC, 2, cv::Scalar(255, 0, 0), 5);

		strText.Format("InMax : %.3fmm", minmax_MaxValue[0]);
		pDC->TextOutA(minmaxPosX[1], minmaxPosY[1], strText);
		cv::putText(m_pMain->m_matResultImage, std::string(strText), cv::Point(minmaxPosX[1], minmaxPosY[1] + 20), cv::FONT_ITALIC, 2, cv::Scalar(255, 0, 0), 5);
		pDC->Rectangle(int(minmaxPosX[1] - 50), int(minmaxPosY[1] - 50), int(minmaxPosX[1] + 50), int(minmaxPosY[1] + 50));
		cv::rectangle(m_pMain->m_matResultImage, cv::Point(int(minmaxPosX[1] - 50), int(minmaxPosY[1] - 50)), cv::Point(int(minmaxPosX[1] + 50), int(minmaxPosY[1] + 50)), cv::Scalar(255, 64, 64), 3);
		//strText.Format("OutMin :(%d,%d) %.3fmm", int(minmaxPosX[2]), int(minmaxPosY[2]), minmax_MinValue[1]);
		//pDC->TextOutA(minmaxPosX[2], minmaxPosY[2], strText);
		//cv::putText(m_pMain->m_matResultImage, std::string(strText), cv::Point(minmaxPosX[2], minmaxPosY[2] - 20), cv::FONT_ITALIC, 2, cv::Scalar(255, 0, 0), 5);

		pDC->SelectObject(&penDB);
		pDC->SetTextColor(COLOR_DARK_BLUE);

		strText.Format("OutMin : %.3fmm", minmax_MinValue[1]);
		pDC->TextOutA(minmaxPosX[2], minmaxPosY[2], strText);
		cv::putText(m_pMain->m_matResultImage, std::string(strText), cv::Point(minmaxPosX[2], minmaxPosY[2] - 20), cv::FONT_ITALIC, 2, cv::Scalar(255, 128, 128), 5);
		pDC->Rectangle(int(minmaxPosX[2] - 50), int(minmaxPosY[2] - 50), int(minmaxPosX[2] + 50), int(minmaxPosY[2] + 50));
		cv::rectangle(m_pMain->m_matResultImage, cv::Point(int(minmaxPosX[2] - 50), int(minmaxPosY[2] - 50)), cv::Point(int(minmaxPosX[2] + 50), int(minmaxPosY[2] + 50)), cv::Scalar(255, 128, 128), 3);

		//strText.Format("OutMax :(%d,%d) %.3fmm", int(minmaxPosX[3]), int(minmaxPosY[3]), minmax_MaxValue[1]);
		//pDC->TextOutA(minmaxPosX[3], minmaxPosY[3], strText);
		//cv::putText(m_pMain->m_matResultImage, std::string(strText), cv::Point(minmaxPosX[3], minmaxPosY[3] - 20), cv::FONT_ITALIC, 2, cv::Scalar(255, 0, 0), 5);

		strText.Format("OutMax : %.3fmm", minmax_MaxValue[1]);
		pDC->TextOutA(minmaxPosX[3], minmaxPosY[3], strText);
		cv::putText(m_pMain->m_matResultImage, std::string(strText), cv::Point(minmaxPosX[3], minmaxPosY[3] - 20), cv::FONT_ITALIC, 2, cv::Scalar(255, 128, 128), 5);
		pDC->Rectangle(int(minmaxPosX[3] - 50), int(minmaxPosY[3] - 50), int(minmaxPosX[3] + 50), int(minmaxPosY[3] + 50));
		cv::rectangle(m_pMain->m_matResultImage, cv::Point(int(minmaxPosX[3] - 50), int(minmaxPosY[3] - 50)), cv::Point(int(minmaxPosX[3] + 50), int(minmaxPosY[3] + 50)), cv::Scalar(255, 128, 128), 3);

		pDC->SelectObject(&penSum);
	}

	if (method == METHOD_CIRCLE)  // 홀 안쪽 영역 변곡점 찾기
	{
		fnSearchMaxMinPos(inspRoi.x, inspRoi.y, m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0], 20.f, pDC);
	}
	
	//KJH 2022-06-29 WetoutSpec관련 변수명 변경
	double Spec_In_Min			= m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSubSpec(0);
	double Spec_In_Max			= m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSubSpecTolerance(0);
	double Spec_Out_Min			= m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSubSpec(1);
	double Spec_Out_MAX			= m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSubSpecTolerance(1);

	//SJB 2022-11-16 Overflow Min Spec 저장 수정
	if (method == METHOD_LINE)	Spec_Out_Min = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getLineOverflowMinSpec();

	//KJH 2022-04-08 Min Spec 0보다 작을때 검사 판정안하도록 변경
	if (Spec_In_Min <= 0)	Spec_In_Min  = -9999;
	if (Spec_Out_Min <= 0)	Spec_Out_Min = -9999;
	
	double maxDistance_in	= -1, minDistance_in	= 99999;
	double maxDistance_out	= -1, minDistance_out	= 99999;
	double maxDistance		= -1, minDistance		= 99999;

	if (method == METHOD_CIRCLE)  // Metal 영역 침범했는지 검사 -> 미도포 검사 항목으로 통합관리
	{
		uchar* Raw					= skeleton.data;
		uchar* OrgRaw				= binOrg.data;

		////////////////////////////////////////////////////
		//0 = 블랙 , 255 = 화이트						  //
		//OrgRaw	= 원본 2진화 영상	(도포 = 블랙)     //
		//Raw		= 차영상 2진화 영상 (도포 = 화이트)   //
		////////////////////////////////////////////////////
		
		//KJH 2022-04-29 InkLack Insp 기능추가(연속성 검사, 총량 검사)
		int _nLackOfInkAngleCount	= 0, _nLackOfInkContinueCount = 0, _nLackOfInkContinueCount_Judge = 0;
		BOOL _bFirstMetal			= m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseMetalTraceFirst();

		//KJH 2022-05-09 Mincount 상시 사용으로 변경
		bool MinCheckMode			= true;
		//bool MinCheckMode = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getMinCountCheckInspModeEnable();
		int	 MinCheckContinueSpec	= m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getMinCountCheckInspContinueCount();
		int	 MinCheckTotalSpec		= m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getMinCountCheckInspTotalCount();
		int  m_nMetalOverflowCount = 0, _nMetalLackofInkMincount=0;

		bool _bLackOfInkLog[360] = { 0, };

		for (int i = 0; i < m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[0].size(); i++)
		{
			//KJH 2022-05-26 미도포 검사시 수학좌표계와 화면좌표계의 0도 차이 반영
			double angle	= m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[2].at(i) - 90;
			double D_two	= m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[1].at(i);

			if (!_bFirstMetal)
			{
				//HTK 2022-06-29 PN Edge 정보 Trace 좌표기준으로 계산하도록 변경
				x = cos(angle * CV_PI / 180.) * CIRCLE_RADIUS + (m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x - inspRoi.x); //m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.x
				y = sin(angle * CV_PI / 180.) * CIRCLE_RADIUS + (m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y - inspRoi.y); //m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.y
			}
			else
			{
				x = cos(angle * CV_PI / 180.) * m_dCircleRadius_MP + (m_ptCircleCenter_MP.x - inspRoi.x);
				y = sin(angle * CV_PI / 180.) * m_dCircleRadius_MP + (m_ptCircleCenter_MP.y - inspRoi.y);
			}
			CPointF<int> p_radius(x, y);
			CPointF<double> p_ImgCenter(skeleton.cols / 2, skeleton.rows / 2);
			double D_radius_imgcenter = GetDistance(p_radius, p_ImgCenter);
			
			// HTK 2022-08-31 Cink1+Cink2 모드시 검사 overflow 기준을 GlassEdge에서 MetalEdge로 변경	// blue 2022.09.02
			double _CCMP_Distance = 0.0;
			if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getCalcWetoutMetalEdge())
			{
				int __CCx = (cos(ToRadian(angle + 90)) * CIRCLE_RADIUS + (skeleton.cols / 2));
				int __CCy = (sin(ToRadian(angle + 90)) * CIRCLE_RADIUS + (skeleton.rows / 2));

				_CCMP_Distance = GetDistance(cv::Point2f(x, y), cv::Point2f(__CCx, __CCy));
				D_radius_imgcenter = D_radius_imgcenter - _CCMP_Distance;
			}
			
			if (!_bFirstMetal || !bSearchElbEndPosFlag)
			{
				//HTK 2022-05-19 CInk1 모드일때 미도포 관련 예외처리
				if (D_two == 0) D_two = m_dCircleRadius_MP;
				m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[2].at(i) = (D_radius_imgcenter - D_two) * xres;
			}
			
			//KJH 2022-02-05 Metal Over flow Margin
			double m_nMetalOverflowMargin = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getMetalOverflowMargin() / yres;

			//KJH 2022-05-14 CInk2 모드일때 Overflow는 Spec으로 판정하는걸로 변경
			//if (!_bFirstMetal && (D_radius_imgcenter + m_nMetalOverflowMargin) < D_two) // 20픽셀은 넘어야 올라탄걸로 간주 // 입력값으로 변경
			
			// KBJ 2022-09-16 EGL Merge시 변경되어 있었던 부분
			if (!_bFirstMetal && (D_radius_imgcenter + _CCMP_Distance + m_nMetalOverflowMargin) < D_two) // 20픽셀은 넘어야 올라탄걸로 간주 // 입력값으로 변경 // blue 2022.09.03 metalEdge일 경우 뺀 값 다시 더해줌
			{
				if (m_nMetalOverflowCount == 0)
				{
					m_nMetalOverflowCount++;
				}
				else
				{	
					//KJH 2022-06-29 Metal Over Flow 2번째 포인트가 있을때만 검사하기
					m_pMain->m_ELB_DiffInspResult[datasheet].m_bResultSummary = FALSE;
					pDC->Ellipse(int(x + inspRoi.x - 20), int(y + inspRoi.y - 20), int(x + inspRoi.x + 20), int(y + inspRoi.y + 20));
					cv::circle(m_pMain->m_matResultImage, cv::Point(int(x + inspRoi.x), int(y + inspRoi.y)), 4, cv::Scalar(0, 0, 255), 2, 5);
				}
			}

	//		if (_bFirstMetal && bSearchElbEndPosFlag) // C-Ink 인경우 미도포 체크  //HTK 2022-05-14 CINK1, CINK2 대응 해서 삭제
			{
				//KJH 2022-06-30 CInk2에서 Metal 기준으로 Lack 검사하는걸로 변경

				bool _metalLackofInk = false;  //Metal에 약액이 Contact되거나 넘침되었는지 판단
				double _dInkUnderflowSpec = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInkUnderflowSpec();
				double _spec	= m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getCInkMinWidthSpec();
				double cinw		= fabs(CIRCLE_RADIUS - m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[2].at(i)) * xres;
				int _findoffset = -3;

				if (!_bFirstMetal || !bSearchElbEndPosFlag)
				{
					cinw = m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[0].at(i);
				}
				else
				{
					// CINK2 에서만 적용하기 위함 // 두번째 찾은 포인트 하고 메탈 까지 거리
					//중심에서 MP까지의 거리 D_radius_imgcenter , 중심에서 Two 까지의 거리 D_two 를 비교한다.
					
					// KBJ 2022-10-05 Trace할때 Lack Of Ink 검사와, Cink2 도포후 검사 Lack Of Ink가 같은 스펙을 쓰고 있어서 변경
					//if (D_two >= (D_radius_imgcenter - _dInkUnderflowSpec /xres))
					if (D_two >= (D_radius_imgcenter - _spec /xres))
					{
						//MP보다 Two가 큰경우 : 넘침 or Contact
						_metalLackofInk = false;
						_nMetalLackofInkMincount = 0;
					}
					else 
					{
						_nMetalLackofInkMincount++;

						if (_nMetalLackofInkMincount > 3)
						{
							//MP보다 Two가 작은경우 : 줄빠짐
							_metalLackofInk = true;
							//진성불량 판정은 _spec을 포함한 위치에서 다시 확인한다.
							_findoffset = -5 - int(_spec / xres);
						}
					}
				}
				
				if (cinw < _spec || _metalLackofInk)
				{   // 도포전 이미지가 Black인경우는 양품처리 Tkyuha 2022 - 04 - 13

					//KJH 2022-05-14 미도포 검출 안되던거 수정...
					int _dhalfx = int(cos(angle * CV_PI / 180.) * (_findoffset) + x);
					int _dhalfy = int(sin(angle * CV_PI / 180.) * (_findoffset) + y);

					if (_metalLackofInk)
					{
						pDC->SelectObject(&penDebug2);
					}
					else
					{
						pDC->SelectObject(&penDebug);
					}

					if (_bFirstMetal && bSearchElbEndPosFlag)
					{		
						if (!_metalLackofInk) continue;				// KBJ 2022-10-05 추가
						// KBJ 2022-10-05 8월 11일 이후 고객사 변경사항으로 이전에 발생한 카운트 합산하지 않으므로 주석처리 
						//if (m_bLackofInkCheckFlag[i])	continue;	// cink1에서 미도포 발생으로 카운트 하지 않음 Tkyuha 2022.06.24

						if (_dhalfx > 0 && _dhalfx < skeleton.cols && _dhalfy>0 && _dhalfy < skeleton.rows)
						{
							pDC->SelectObject(&penDebug2);

							//0 = 블랙 , 255 = 화이트
							//OrgRaw	= 원본 2진화 영상	(도포 = 블랙)
							//Raw		= 차영상 2진화 영상 (도포 = 화이트)
							//들어올때 NG 인데 다시 확인
							if (OrgRaw[_dhalfy * skeleton.cols + _dhalfx] > 100)
							{
								//KJH 2022-04-29 InkLack Insp 기능추가(연속성 검사, 총량 검사)
								_nLackOfInkAngleCount++;		//Total count
								_nLackOfInkContinueCount++;		//continue count
								pDC->Ellipse(int(_dhalfx + inspRoi.x - 20), int(_dhalfy + inspRoi.y - 20), int(_dhalfx + inspRoi.x + 20), int(_dhalfy + inspRoi.y + 20));
								cv::circle(m_pMain->m_matResultImage, cv::Point(int(_dhalfx + inspRoi.x), int(_dhalfy + inspRoi.y)), 20, cv::Scalar(255, 64, 255), 2, 5);

								_bLackOfInkLog[i] = TRUE;
							}
							//KJH 2022-05-09 원본, 차영상 둘다 검정일때 미도포인 경계선으로 판정 추가
							else if (OrgRaw[_dhalfy * skeleton.cols + _dhalfx] > 100 && Raw[_dhalfy * skeleton.cols + _dhalfx] < 100)
							{
								//KJH 2022-04-29 InkLack Insp 기능추가(연속성 검사, 총량 검사)
								_nLackOfInkAngleCount++;		//Total count
								_nLackOfInkContinueCount++;		//continue count
								pDC->Ellipse(int(_dhalfx + inspRoi.x - 20), int(_dhalfy + inspRoi.y - 20), int(_dhalfx + inspRoi.x + 20), int(_dhalfy + inspRoi.y + 20));
								cv::circle(m_pMain->m_matResultImage, cv::Point(int(_dhalfx + inspRoi.x), int(_dhalfy + inspRoi.y)), 20, cv::Scalar(255, 64, 255), 2, 5);

								_bLackOfInkLog[i] = TRUE;
							}
						}						
					}
					else
					{
						//Tkyuh 2022-06-17 cink1 InkLack Insp 기능추가(연속성 검사, 총량 검사)a
						_nLackOfInkAngleCount++;		//Total count
						_nLackOfInkContinueCount++;		//continue count
						pDC->Ellipse(int(_dhalfx + inspRoi.x - 20), int(_dhalfy + inspRoi.y - 20), int(_dhalfx + inspRoi.x + 20), int(_dhalfy + inspRoi.y + 20));
						cv::circle(m_pMain->m_matResultImage, cv::Point(int(_dhalfx + inspRoi.x), int(_dhalfy + inspRoi.y)), 20, cv::Scalar(255, 64, 255), 2, 5);
					}

					pDC->SelectObject(&penSum);
				}
				else
				{	//스펙 참일떄
					if (_nLackOfInkContinueCount_Judge < _nLackOfInkContinueCount)
					{
						//최대값 저장
						_nLackOfInkContinueCount_Judge = _nLackOfInkContinueCount;
					}
					_nLackOfInkContinueCount = 0;
				}
			}
		}	//360
		
		// KBJ 2022-10-10 LackOfInk 엑셀로그파일용으로 추가. (1)
		for (int i = 0; i < m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].size(); i++)
		{
			if (_bLackOfInkLog[i] || m_bLackofInkCheckFlag[i])
			{
				m_pMain->m_ELB_DiffInspResult[datasheet]._bLackOfInkAlgleJudge[i] = true;
			}
			else m_pMain->m_ELB_DiffInspResult[datasheet]._bLackOfInkAlgleJudge[i] = false;

			// KBJ 2022-09-27 Trace 에서 찾은 것 다시 표시
			if (_bFirstMetal && bSearchElbEndPosFlag && (m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType & (1 << ELB_INSP_ERROR_LACK_CONTINUE)) != 0)
			{
				if (m_bLackofInkCheckFlag[i])
				{
					int __x = (cos(ToRadian(i)) * (CIRCLE_RADIUS)+m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x); // Glass Edge point
					int __y = (sin(ToRadian(i)) * (CIRCLE_RADIUS)+m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y); // Glass Edge point	

					pDC->SelectObject(&penOK);
					pDC->Ellipse(__x - 20, __y - 20, __x + 20, __y + 20);
					cv::circle(m_pMain->m_matResultImage, cv::Point(int(__x), int(__y)), 10, cv::Scalar(64, 255, 64), 2, 5);
				}
				pDC->SelectObject(&penSum);
			}
		}

		//HTK 2022-05-19 미도포검사시 전체 도포가 안된 경우 하고 마지막 확인
		if (_nLackOfInkContinueCount_Judge < _nLackOfInkContinueCount) 
			_nLackOfInkContinueCount_Judge = _nLackOfInkContinueCount;

		//KJH 2022-07-01 Lack Of Cink Total Count 로그 추가
		m_pMain->m_ELB_DiffInspResult[datasheet].m_nLackOfInkAngleCount_Total = _nLackOfInkAngleCount;

		//HTK 2022-06-24 CInk2에서 CInk1에서 발생한 Lack of Ink 사전 검사 후 최종 합산
		if (_bFirstMetal && bSearchElbEndPosFlag) 
		{
			//_nLackOfInkAngleCount += m_nLackOfInk_CinkCount; // 기존 검사방식  220811 Tkyuha
			_nLackOfInkContinueCount_Judge += m_nLackOfInk_CinkCount; // 고객요청방식검사방식 220811 Tkyuha
		}

		m_pMain->m_ELB_DiffInspResult[datasheet].m_nLackOfInkAngleCount_Judge = _nLackOfInkAngleCount;
		//KJH 2022-04-29 CInkLack Insp 기능추가(연속성 검사, 총량 검사)
		if (MinCheckMode)
		{
			if (_nLackOfInkAngleCount >= MinCheckTotalSpec)
			{
				// KBJ 2022-10-10 검사 에러타입추가.	======================================
				m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType += ELB_Error_Type(m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType, ELB_INSP_ERROR_LACK_TOTAL);
				//	==========================================================================
				bReturn = false;
				str.Format("[%s] Min Spec Total Count Insp NG : Find - %d", m_pMain->vt_job_info[nJob].job_name.c_str(), _nLackOfInkAngleCount);
				::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
				m_pMain->m_ELB_DiffInspResult[datasheet].bType = FALSE;
			}

			if (_nLackOfInkContinueCount_Judge >= MinCheckContinueSpec)
			{
				// KBJ 2022-10-10 검사 에러타입추가.	======================================
				m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType += ELB_Error_Type(m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType, ELB_INSP_ERROR_LACK_CONTINUE);
				//	==========================================================================
				bReturn = false;
				str.Format("[%s] Min Spec Continue Count Insp NG : Find - %d", m_pMain->vt_job_info[nJob].job_name.c_str(), _nLackOfInkContinueCount_Judge);
				::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
				m_pMain->m_ELB_DiffInspResult[datasheet].bType = FALSE;
			}

			// KBJ 2022-07-13 3회 연속 Lack Of INK NG 일시 팝업
			if (m_pMain->m_ELB_DiffInspResult[datasheet].bType == FALSE)
			{
				m_nCount_LackOfCink_NG++;
				if (m_nCount_LackOfCink_NG > 4)
				{
					//HideErrorMessageBlink();
					CString str;
					str.Format("[Lack of Ink NG InterLock !!] - Count : %d", m_nCount_LackOfCink_NG);
					m_pMain->setAlarmMessage(ALARM_DIFF_LACK_INTERLOCK, TRUE, str);
					//ShowErrorMessageBlink(str);
				}
				else if (m_nCount_LackOfCink_NG > 2)
				{
					//HideErrorMessageBlink();
					CString str;
					str.Format("[Lack of Ink NG Warning !!] - Count : %d", m_nCount_LackOfCink_NG);
					m_pMain->setAlarmMessage(ALARM_DIFF_LACK_WARNING, TRUE, str);
					//ShowErrorMessageBlink(str);
				}
			}
			else
			{
				m_nCount_LackOfCink_NG = 0;
			}
		}
	}
	else if (method == METHOD_LINE)
	{
		double v2 = 0,vMax=0,x=-1,y=-1, wo,v3,wi;
		int vex, vey;
		int nsize = int(m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[2].size());
		double _spec = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getLineLengthMinThick();
		double _linespec = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getLineLengthSpec();
		double _linecenterspec = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getCamCenterToCG();
		int _x_start_soffset = 0, _x_end_soffset = 0;

		pDC->SelectObject(&penLime);
		for (int i = 0; i < nsize; i++)
		{
			wo = m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[0].at(i);
			wi = m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[0].at(nsize - i - 1);
			if (_spec > 0 ) // 최소 도포 높이가 되는 부분만 길이 검출 하기 위함
			{
				if (_x_start_soffset == 0 && wo > _spec)
					_x_start_soffset = i;
				if (_x_end_soffset == 0 && wi > _spec)
					_x_end_soffset = nsize - i - 1;
			}
			v3 = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[4].at(i)- (_lineOverflowLine - inspRoi.y);
			v2 = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[2].at(i);
			m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[2].push_back(v2* yres);
			vex = m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[3].at(i);
			vey = m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[4].at(i);
			if (v3 > vMax)
			{
				vMax = v3;
				x = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[3].at(i);
				y = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[4].at(i);
			}
			//SJB 2022-11-29 Wetout, Overflow 오버레이 수정(색 통일), 사이즈 변경
			if(v3>0) pDC->Ellipse(int(vex + inspRoi.x - 1), int(vey + inspRoi.y - 1), int(vex + inspRoi.x + 1), int(vey + inspRoi.y + 1));
		}
		
		//SJB 2022-11-07 Wetout, Overflow Min Max Draw 시점 변경
		//SJB 2022-11-29 Wetout, Overflow 오버레이 수정(색 통일), 사이즈 변경
		pDC->SelectObject(&penGreenYellow);
		pDC->Rectangle(int(minmaxPosX[0] - 6), int(minmaxPosY[0] - 6), int(minmaxPosX[0] + 6), int(minmaxPosY[0] + 6), 3);
		pDC->Rectangle(int(minmaxPosX[1] - 6), int(minmaxPosY[1] - 6), int(minmaxPosX[1] + 6), int(minmaxPosY[1] + 6), 3);

		pDC->SelectObject(&penSkyblue);
		pDC->Rectangle(int(minmaxPosX[2] - 6), int(minmaxPosY[2] - 6), int(minmaxPosX[2] + 6), int(minmaxPosY[2] + 6), 3);
		pDC->Rectangle(int(minmaxPosX[3] - 6), int(minmaxPosY[3] - 6), int(minmaxPosX[3] + 6), int(minmaxPosY[3] + 6), 3);

		//SJB 2022-11-29 Wetout, Overflow Min Max 오버레이 추가
		//SJB 2022-12-03 not use
		//Gdiplus::Color color_GreenYellow = Gdiplus::Color::GreenYellow;
		//mViewer->AddSoftGraphic(new GraphicLabel(int(minmaxPosX[0]), int(minmaxPosY[0]) - 30, fmt("[Wetout Min : %.4f]mm", minmax_MinValue[0]), color_GreenYellow));
		//mViewer->AddSoftGraphic(new GraphicLabel(int(minmaxPosX[1]), int(minmaxPosY[1]) - 30, fmt("[Wetout Max : %.4f]mm", minmax_MaxValue[0]), color_GreenYellow));

		//Gdiplus::Color color_skyBlue = Gdiplus::Color(128, 128, 255);
		//mViewer->AddSoftGraphic(new GraphicLabel(int(minmaxPosX[2]), int(minmaxPosY[2]) + 20, fmt("[Overflow Min : %.4f]mm", minmax_MinValue[1]), color_skyBlue));
		//mViewer->AddSoftGraphic(new GraphicLabel(int(minmaxPosX[3]), int(minmaxPosY[3]) + 20, fmt("[Overflow Max : %.4f]mm", minmax_MaxValue[1]), color_skyBlue));


		double m_nMetalOverflowMargin = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getMetalOverflowMargin()/yres;
		double _dInkUnderflowSpec = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInkUnderflowSpec();

		if (vMax > m_nMetalOverflowMargin) // 20픽셀은 넘어야 올라탄걸로 간주 // 입력값으로 변경
		{
			m_pMain->m_ELB_DiffInspResult[datasheet].m_bResultSummary = FALSE;
			pDC->Ellipse(int(x + inspRoi.x - 20), int(y + inspRoi.y - 20), int(x + inspRoi.x + 20), int(y + inspRoi.y + 20));
			cv::circle(m_pMain->m_matResultImage, cv::Point(int(x + inspRoi.x), int(y + inspRoi.y)), 4, cv::Scalar(0, 0, 255), 2, 5);
		}
		
		if(minmax_MaxValue[0]< _dInkUnderflowSpec) // 미충진
			m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_bResultSummary = FALSE;

		// 도포 전체 길이
		m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultAngle[0] = m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.x;
		m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultAngle[1] = m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.y;
		
		if (_x_start_soffset > 0 && _x_end_soffset >0) // 도포 스펙 이상인경우 길이
		{
			m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.y = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[3].at(_x_end_soffset) + inspRoi.x;
			m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.x = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[3].at(_x_start_soffset) + inspRoi.x;
		}
		// CG 기준 도포 시작점 계산
		if(!m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getELBCG_Edge_CalcModeEnable())							// Cover도포모드
			m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultAngle[2] = _linecenterspec - (W / 2 - m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.x) * xres;
		else
		{
			m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultAngle[2] = _linecenterspec + (W / 2 - m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultAngle[2]) * xres;
			m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultAngle[2] -= (W / 2 - m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.x) * xres;
		}

		CPointF<int> p_center;
		p_center.x = int(m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.y );
		p_center.y = 0;
		//pDC->MoveTo(p_center.x, p_center.y);
		//pDC->LineTo(p_center.x, H - 1);
		p_center.x = int(m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.x );
		p_center.y = 0;
		//pDC->MoveTo(p_center.x, p_center.y);
		//pDC->LineTo(p_center.x, H - 1);
		
		// KBJ 2022-12-05 Blob 시작과 끝점이 아닌 검사 영역으로 수정.
		int centerX = inspRoi.width/2 + inspRoi.x;
		int leftX = inspRoi.x;
		int rightX = inspRoi.width + inspRoi.x;

		pDC->MoveTo(leftX, 0);
		pDC->LineTo(leftX, H - 1);
		cv::line(m_pMain->m_matResultImage, cv::Point(leftX, 0), cv::Point(leftX, H - 1), CV_RGB(64, 64, 255), 3);
		pDC->MoveTo(rightX, 0);
		pDC->LineTo(rightX, H - 1);
		cv::line(m_pMain->m_matResultImage, cv::Point(rightX, 0), cv::Point(rightX, H - 1), CV_RGB(64, 64, 255), 3);
		
		//HTK 2022-03-10 CC Edge Calc Display
		if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getELBCG_Edge_CalcModeEnable())
		{
			p_center.x = int(m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultAngle[2]);
			p_center.y = 0;
			pDC->MoveTo(p_center.x, p_center.y);
			pDC->LineTo(p_center.x, H - 1);
		}

		// 길이 판정 사용
		if (_linespec < fabs(m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.y - m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.x) * xres)
		{
			// KBJ 2022-10-10 검사 에러타입추가.	======================================
			m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType += ELB_Error_Type(m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType, ELB_INSP_ERROR_LINE_WIDTH);
			//	==========================================================================
			bReturn = false;
		}
		m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultT = ((_searchPocket.y + _searchPocket.x) / 2 - m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.x) * xres;

		//SJB 2022-11-29 Dist, CG_Start 텍스트 오버레이 수정
		Gdiplus::Color color_Lime = Gdiplus::Color::Lime;

		//mViewer->AddHardGraphic(new GraphicLabel(10, 10 + (20 * 0), fmt("[Dist: %.4f]mm", m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultT), color_Green, Anchor::XCenterMiddleBottom));
		mViewer->AddHardGraphic(new GraphicLabel(10, 10 + (20 * 1), fmt("[CG-Start: %.4f]mm", m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultAngle[2]), color_Lime, Anchor::XCenterMiddleBottom));

		//pDC->SetTextColor(COLOR_GREEN);
		//strText.Format("[Dist: %.4f]mm", m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultT);
		////pDC->TextOutA(_searchPocket.x, H/2+200, strText);
		//pDC->TextOutA(W / 2 - 300, H / 2 + 200, strText);
		//strText.Format("[CG-Start: %.4f]mm", m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultAngle[2]);
		////pDC->TextOutA(_searchPocket.x, H / 2 + 300, strText);
		//pDC->TextOutA(W / 2 - 300, H / 2 + 300, strText);

		//if (m_pMain->m_ELB_DiffInspResult[datasheet].m_bResultSummary == FALSE)
		//{
		//	// KBJ 2022-10-10 검사 에러타입추가.	======================================
		//	m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType += ELB_Error_Type(m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType, ELB_INSP_ERROR_OVER_FLOW);
		//	//	==========================================================================
		//	strText.Format("[OverFlow: %.4f]mm", vMax*yres);
		//	//pDC->TextOutA(_searchPocket.x, H / 2 + 400, strText);
		//	//pDC->TextOutA(W / 2 - 300, H / 2 + 400, strText);
		//	//SJB 2022-11-29 Overflow Min Max 텍스트 위치 변경
		//	pDC->TextOutA(int(minmaxPosX[3]), int(minmaxPosY[3]) + 20, strText);
		//}

		//if (m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_bResultSummary == FALSE)
		//{
		//	// KBJ 2022-10-10 검사 에러타입추가.	======================================
		//	m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType += ELB_Error_Type(m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType, ELB_INSP_ERROR_UNDER_FLOW);
		//	//	==========================================================================
		//	strText.Format("[UnderFlow(%.4f): %.4f]mm", _dInkUnderflowSpec,minmax_MaxValue[0]);
		//	//pDC->TextOutA(_searchPocket.x, H / 2 + 500, strText);
		//	pDC->TextOutA(W / 2 - 300, H / 2 + 500, strText);
		//}

		m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultAngle[9] = vMax * yres;  //overflow 
	}
	else
	{
		CPointF<int> p_center;
		double vecX, vecY;
		int vecS = int(m_vecInterSectionPoint.size()) - 1;
		int vecoFF = 0;
		if (m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[3].size() > 0)
		{
			for (int i = 0; i < m_vecInterSectionPoint.size(); i++)
			{
				if (m_vecInterSectionPoint.at(i).x >= m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[3].at(0) + inspRoi.x)
				{
					vecoFF = MAX(0, i - 1);
					break;
				}
			}
		}
		for (int i = 0; i < m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].size(); i++)
		{
			vecX = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[3].at(i);
			vecY = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[4].at(i);

			CPointF<int> p2(vecX + inspRoi.x, vecY + inspRoi.y);
			if (vecS > vecoFF + i)
			{
				p_center.x = m_vecInterSectionPoint.at(vecoFF + i).x;
				p_center.y = m_vecInterSectionPoint.at(vecoFF + i).y;
			}
			else
			{
				p_center.x = 0;
				p_center.y = 0;
			}
			double  v = m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[1].at(i);
			double v2 = GetDistance(p_center, p2);

			m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[2].push_back(v2 * xres);
			if ((v2 + 20) < v) // 20픽셀은 넘어야 올라탄걸로 간주
			{
				m_pMain->m_ELB_DiffInspResult[datasheet].m_bResultSummary = FALSE;
				pDC->Ellipse(int(p2.x - 20), int(p2.y - 20), int(p2.x + 20), int(p2.y + 20));
				cv::circle(m_pMain->m_matResultImage, cv::Point(int(p2.x), int(p2.y)), 4, cv::Scalar(0, 0, 255), 2, 5);
			}
		}

		p_center.x = int(m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.y);
		p_center.y = 0;
		pDC->MoveTo(p_center.x, p_center.y);
		pDC->LineTo(p_center.x, H - 1);
		p_center.x = int(m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.x);
		p_center.y = 0;
		pDC->MoveTo(p_center.x, p_center.y);
		pDC->LineTo(p_center.x, H - 1);
	}

	for (int i = 0; i < m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[0].size(); i++) // wetout 최대 최소 계산
	{
		double v = fabs(m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[0].at(i));
		double v2 = (m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[1].at(i));
		
		//KJH 2021-12-11 Total Value 이상현상 수정
		double v3 = fabs(v) + fabs(v2);

		if (v >= maxDistance_in)	maxDistance_in	= v;
		if (v <= minDistance_in)	minDistance_in	= v;
		if (v2 >= maxDistance_out)	maxDistance_out = v2;
		//SJB 2022-11-15 Min Value 저장 수정
		if (method == METHOD_LINE)
		{
			minDistance_out = minmax_MinValue[1];
		}
		else
		{
		if (v2 <= minDistance_out)	minDistance_out = v2;
		}
		if (v3 >= maxDistance)		maxDistance		= v3;
		if (v3 <= minDistance)		minDistance		= v3;
	}

	//Tkyuha 2022-06-18 FDC -값 보고 수정 
	if (maxDistance_out < 0) maxDistance_out = 0.01;
	if (maxDistance_in < 0 ) maxDistance_in  = 0.01;

	m_pMain->m_ELB_DiffInspResult[datasheet].m_vELB_ReverseRotateCenter.x = minDistance_in;
	m_pMain->m_ELB_DiffInspResult[datasheet].m_vELB_ReverseRotateCenter.y = maxDistance_in;
	m_pMain->m_ELB_DiffInspResult[datasheet].m_vELB_RotateCenter.x = minDistance_out;
	m_pMain->m_ELB_DiffInspResult[datasheet].m_vELB_RotateCenter.y = maxDistance_out;
	m_pMain->m_ELB_DiffInspResult[datasheet].m_vELB_DistMinMax.x = minDistance;
	m_pMain->m_ELB_DiffInspResult[datasheet].m_vELB_DistMinMax.y = maxDistance;

	if (m_pMain->m_vInspWetOutResult.size() > 1000)
		m_pMain->m_vInspWetOutResult.clear();

	_stInsp_WetOut_Result temp;
	temp.dMin_In	= minDistance_in;
	temp.dMax_In	= maxDistance_in;
	temp.dMin_Out	= minDistance_out;
	temp.dMax_Out	= maxDistance_out;
	temp.dMin_Total = minDistance;
	temp.dMax_Total = maxDistance;
	m_pMain->m_vInspWetOutResult.push_back(temp);

	//HTK 2022-06-29 Wetout MIn,Max Judge 관련 Enable Mode 추가
	bool bWetoutJudgeEnable = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getWetoutMinMaxJudgeModeEnable();
	if (bWetoutJudgeEnable && (minDistance_in < Spec_In_Min || maxDistance_in > Spec_In_Max))
	{
		if(minDistance_in < Spec_In_Min)
		{
			// KBJ 2022-10-10 검사 에러타입추가.	======================================
			m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType += ELB_Error_Type(m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType, ELB_INSP_ERROR_WET_IN_MIN);
			//	==========================================================================
		}
		if(maxDistance_in > Spec_In_Max)
		{
			// KBJ 2022-10-10 검사 에러타입추가.	======================================
			m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType += ELB_Error_Type(m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType, ELB_INSP_ERROR_WET_IN_MAX);
			//	==========================================================================
		}
		bReturn = false;
		str.Format("[%s] Wet In Spec - NG", m_pMain->vt_job_info[nJob].job_name.c_str());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}

	// YCS 2022-11-30 WetOut 데이터와 WetOut_ASPC Spec 비교 - 이준재 프로 요청사항
#pragma region WetOut 데이터와 WetOut_ASPC Spec 비교
	// 최근 10매 중 2매이상에서 Wetout 데이터 0.1이상 발생 시 경고창 팝업 
	if (method == METHOD_CIRCLE)
	{
		const double WETOUT_ASPC_SPEC = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getWetoutASPCSpec(); // ASPC 스펙
		const bool bWETOUT_ASPC_SPECOUT = FALSE;
		const bool bWETOUT_ASPC_SPECIN = TRUE;

		const int nWARNING_COUNT_SPEC = 2;	// 경고창 띄우는 최소 카운트 개수
		const bool bWARNING_NO_CLEAR = FALSE;
		const bool bWARNING_CLEAR = TRUE;

		int nWarningCount = 0;
		int nSpecOutCount = 0;
		int nFirstWarningPosition = 0;
		bool bFlag = FALSE;
		bool bWarningFlag = FALSE;
		CString strASPC;

		// 데이터 10개 초과시 오래된 순서대로 삭제
		if (m_vJudge_WetOut_ASPC.size() > 9)
		{
			m_vJudge_WetOut_ASPC.erase(m_vJudge_WetOut_ASPC.begin());
			m_vWarning.erase(m_vWarning.begin());
		}

		// ASPC 스펙과 비교
		if (maxDistance_out >= WETOUT_ASPC_SPEC)
		{
			m_vJudge_WetOut_ASPC.push_back(bWETOUT_ASPC_SPECOUT); // Spec Out 개수 체크용 변수
			m_vWarning.push_back(bWETOUT_ASPC_SPECOUT); // 경고 처리 상태 체크용 변수
		}
		else
		{
			m_vJudge_WetOut_ASPC.push_back(bWETOUT_ASPC_SPECIN);
			m_vWarning.push_back(bWETOUT_ASPC_SPECIN);
		}

		// 최근 10매 중 경고 처리 안 된 첫 번째 SpecOut 데이터의 인덱스를 구함
		for (int i = 0; i < m_vWarning.size(); i++)
		{
			if (m_vWarning.at(i) == bWETOUT_ASPC_SPECOUT)
			{
				if (bFlag == FALSE)
				{
					// 처리 안된 경고의 위치가 어딘지 확인
					nFirstWarningPosition = i;
					bFlag = TRUE;
				}

				// 2매 이상 경고 있으면 무조건 NG
				nWarningCount++;
				if (nWarningCount == nWARNING_COUNT_SPEC)
				{
					bWarningFlag = TRUE;
					break;
				}
			}
		}

		// 최근 10매 중 총 몇 매에서 Spec Out 발생했는지 카운팅
		for (int i = 0; i < m_vJudge_WetOut_ASPC.size(); i++)
		{
			if (m_vJudge_WetOut_ASPC.at(i) == bWETOUT_ASPC_SPECOUT)	 nSpecOutCount++;
		}

		// 카운트 2회 이상이면 경고창 팝업
		if (bWarningFlag)
		{
			CString str;
			str.Format("[WetOut_ASPC Spec Out Warning!!] - SpecOut Count : %d", nSpecOutCount);
			m_pMain->setAlarmMessage(ALARM_WETOUT_WARNING, TRUE, str);

			strASPC.Format("[%s] WetOut_ASPC Spec Out Warning!!", m_pMain->vt_job_info[nJob].job_name.c_str());
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

			m_vWarning.at(nFirstWarningPosition) = bWARNING_CLEAR; // 경고처리 되었음 확인
		}
	}
#pragma endregion

	// WET OUT 검사 
	if (method == METHOD_CIRCLE)
	{
		// YCS 2022-12-01 CINK2일 때, Wetout Judge Disable이라도 무조건 WetOut NG 발생시키도록 조건 수정 (김진용 프로 요청: Spec Out이 OK로 나가면 유출이다.)
		BOOL bMetalTraceFirst = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseMetalTraceFirst();
		if ((bWetoutJudgeEnable || bMetalTraceFirst) && (minDistance_out < Spec_Out_Min || maxDistance_out > Spec_Out_MAX))
		{
			if (minDistance_out < Spec_Out_Min)
			{
				// KBJ 2022-10-10 검사 에러타입추가.	======================================
				m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType += ELB_Error_Type(m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType, ELB_INSP_ERROR_WET_OUT_MIN);
				//	==========================================================================
			}
			if (maxDistance_out > Spec_Out_MAX)
			{
				// KBJ 2022-10-10 검사 에러타입추가.	======================================
				m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType += ELB_Error_Type(m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType, ELB_INSP_ERROR_WET_OUT_MAX);
				//	==========================================================================
			}
			bReturn = false;
			str.Format("[%s] Wet Out Spec - NG", m_pMain->vt_job_info[nJob].job_name.c_str());
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		}
	}
	else
	{
		if (bWetoutJudgeEnable && (minDistance_out < Spec_Out_Min || maxDistance_out > Spec_Out_MAX))
		{
			if (minDistance_out < Spec_Out_Min)
			{
				// KBJ 2022-10-10 검사 에러타입추가.	======================================
				m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType += ELB_Error_Type(m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType, ELB_INSP_ERROR_WET_OUT_MIN);
				//	==========================================================================
			}
			if (maxDistance_out > Spec_Out_MAX)
			{
				// KBJ 2022-10-10 검사 에러타입추가.	======================================
				m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType += ELB_Error_Type(m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType, ELB_INSP_ERROR_WET_OUT_MAX);
				//	==========================================================================
			}
			bReturn = false;
			str.Format("[%s] Wet Out Spec - NG", m_pMain->vt_job_info[nJob].job_name.c_str());
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		}
	}

	// KBJ 2022-09-19 CINK2 에서도 검사
	// KBJ 2022-08-16 CINK1 찾은 B값에서 부터 150 픽셀 연속적인지 확인
	//if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseMetalTraceFirst() != TRUE)
	{
		for (int i = 0; i < 360; i++)
		{
			// 연속성 찾기
			if (_bOverflowDefect_Flag[i] == TRUE)
			{
				//근처 10도 중에서 5도나오면 연속성으로 판단
				int nCount = 0;
				for (int j = 0; j < 10; j++)
				{
					int nDegree = i + j;
					if (nDegree >= 360) nDegree -= 360;

					if (_bOverflowDefect_Flag[nDegree] == TRUE)
					{
						nCount++;
					}
				}

				if (nCount >= 5)
				{
					m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_bResultSummary = FALSE;
					break;
				}

				if (1) // KBJ 2022-10-04 연속 3도 나오면 NG추가
				{
					// KBJ 2022-09-24 연속 3도 나오면 NG추가 아직 정하지 않음
					nCount = 0;
					for (int j = 0; j < 3; j++)
					{
						int nDegree = i + j;
						if (nDegree >= 360) nDegree -= 360;

						if (_bOverflowDefect_Flag[nDegree] == TRUE)
						{
							nCount++;
						}
					}

					if (nCount >= 3)
					{
						m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_bResultSummary = FALSE;
						break;
					}
				}
			}
		}
	}

	CString str_temp;
	if (method == METHOD_LINE)
	{
	str_temp.Format("%.3f, %.3f, %.3f, %.3f, %.3f, %.3f - %d", minDistance_in, maxDistance_in, minDistance_out, maxDistance_out, minDistance,
		maxDistance, m_pMain->m_ELB_DiffInspResult[datasheet].m_bResultSummary);
	::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str_temp);
	}
	else
	{
		str_temp.Format("%.3f, %.3f, %.3f, %.3f, %.3f, %.3f - %d", minDistance_in, maxDistance_in, minDistance_out, maxDistance_out, minDistance,
			maxDistance, m_pMain->m_ELB_DiffInspResult[datasheet].m_bResultSummary);
		::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str_temp);
	}

	if (m_pMain->m_ELB_DiffInspResult[datasheet].m_bResultSummary == FALSE ||
		m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_bResultSummary == FALSE) bReturn = false;

	if (m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_bResultSummary == FALSE)
	{
		// KBJ 2022-10-10 검사 에러타입추가.	======================================
		m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType += ELB_Error_Type(m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType, ELB_INSP_ERROR_UNDER_FLOW);
		//	==========================================================================
		str.Format("[%s] Resin UnderFlow - NG", m_pMain->vt_job_info[nJob].job_name.c_str());
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}
	if (theApp.m_bEnforceOkFlag) bReturn = true;

	cl = bReturn ? 0xff00ff00 : Gdiplus::Color::Red;
	//mViewer->AddHardGraphic(new GraphicLabel(10.f, 10.f, bReturn ? "OK" : "NG", cl));
	//mViewer->AddHardGraphic(new GraphicLabel(10.f, 30.f, fmt("[Dopo Width Min: %.4f, Max: %.4f]mm", minVal * yres, maxVal * yres), cl));
	//mViewer->AddHardGraphic(new GraphicLabel(10.f, 50.f,
	//	fmt("[Length: %.4f, L: %.4f, R: %.4f]mm", (m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.y - m_pMain->m_ELB_DiffInspResult[ncamera].m_ELB_ResultXY.x) * xres,
	//		(W / 2 - m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.x) * xres, (m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.y - W / 2) * xres), cl));

	//cl = (bWetoutJudgeEnable && (minDistance_in < Spec_In_Min || maxDistance_in > Spec_In_Max)) ? Gdiplus::Color::Red : 0xff00ff00;
	//mViewer->AddHardGraphic(new GraphicLabel(10.f, 70.f, fmt("[Wetout_In Min: %.4f, Max: %.4f]mm", minmax_MinValue[0], minmax_MaxValue[0]), cl));

	//cl = (bWetoutJudgeEnable && (minDistance_out < Spec_Out_Min || maxDistance_out > Spec_Out_MAX)) ? Gdiplus::Color::Red : 0xff00ff00;
	//mViewer->AddHardGraphic(new GraphicLabel(10.f, 90.f, fmt("[Wetout_Out Min: %.4f, Max: %.4f]mm", minmax_MinValue[1], minmax_MaxValue[1]), cl));

	cv::Scalar color;
	CPaneAuto* pPaneAuto = (CPaneAuto*)m_pMain->m_pPane[PANE_AUTO];
	if (bReturn)
	{
		pDC->SetTextColor(COLOR_GREEN);
		color = cv::Scalar(64, 255, 64);
		strText.Format("OK");
		//pDC->TextOutA(100, 100, strText);
		mViewer->AddHardGraphic(new GraphicLabel(10.f, 10.f, bReturn ? "OK" : "NG", cl));
		cv::putText(m_pMain->m_matResultImage, std::string(strText), cv::Point(100, 100), cv::FONT_ITALIC, 2, color, 5);
		pDC->SetTextColor(COLOR_GREEN);
	}
	else
	{
		pDC->SetTextColor(COLOR_RED);
		color = cv::Scalar(0, 0, 255);
		strText.Format("NG");
		//pDC->TextOutA(100, 100, strText);
		mViewer->AddHardGraphic(new GraphicLabel(10.f, 10.f, bReturn ? "OK" : "NG", cl));
		cv::putText(m_pMain->m_matResultImage, std::string(strText), cv::Point(100, 100), cv::FONT_ITALIC, 2, color, 5);
		pDC->SetTextColor(COLOR_GREEN);
	}

	strText.Format("[Dopo Width Min: %.4f , Max: %.4f]mm", minVal * yres, maxVal * yres);
	//pDC->TextOutA(100, 200, strText);
	mViewer->AddHardGraphic(new GraphicLabel(10.f, 30.f, fmt(strText), cl));
	cv::putText(m_pMain->m_matResultImage, std::string(strText), cv::Point(100, 200), cv::FONT_ITALIC, 2, Scalar(64, 255, 64), 5);

	strText.Format("[Length: %.4f , L: %.4f , R: %.4f]mm", (m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.y - m_pMain->m_ELB_DiffInspResult[ncamera].m_ELB_ResultXY.x) * xres,
		(W / 2 - m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.x) * xres, (m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultXY.y - W / 2) * xres);
	//pDC->TextOutA(100, 300, strText);
	mViewer->AddHardGraphic(new GraphicLabel(10.f, 50.f,fmt(strText), cl));
	cv::putText(m_pMain->m_matResultImage, std::string(strText), cv::Point(100, 300), cv::FONT_ITALIC, 2, Scalar(64, 255, 64), 5);

	//SJB 2022-11-29 Wetout, Overflow 오버레이 수정(색 통일), 라인도포 구분
	if(method == METHOD_LINE)
	{
		if (bWetoutJudgeEnable && (minDistance_in < Spec_In_Min || maxDistance_in > Spec_In_Max)) { cl = Gdiplus::Color::Red; pDC->SetTextColor(COLOR_RED);		color = cv::Scalar(0, 0, 255); }
		else { cl = Gdiplus::Color::GreenYellow; pDC->SetTextColor(COLOR_BLUE);	color = cv::Scalar(255, 64, 64); }
		strText.Format("[Wetout Min: %.4f , Max: %.4f]mm", minmax_MinValue[0], minmax_MaxValue[0]);
		//pDC->TextOutA(100, 400, strText);
		mViewer->AddHardGraphic(new GraphicLabel(10.f, 70.f, fmt(strText), cl));
		cv::putText(m_pMain->m_matResultImage, std::string(strText), cv::Point(100, 400), cv::FONT_ITALIC, 2, color, 5);

		if (bWetoutJudgeEnable && (minDistance_out < Spec_Out_Min || maxDistance_out > Spec_Out_MAX)) { cl = Gdiplus::Color::Red; pDC->SetTextColor(COLOR_RED);			color = cv::Scalar(0, 0, 255); }
		else { cl = Gdiplus::Color(128, 128, 255); pDC->SetTextColor(COLOR_DARK_BLUE);		color = cv::Scalar(255, 128, 128); }
		strText.Format("[Overflow Min: %.4f , Max: %.4f]mm", minmax_MinValue[1], minmax_MaxValue[1]);
		//pDC->TextOutA(100, 500, strText);
		mViewer->AddHardGraphic(new GraphicLabel(10.f, 90.f, fmt(strText), cl));
		cv::putText(m_pMain->m_matResultImage, std::string(strText), cv::Point(100, 500), cv::FONT_ITALIC, 2, color, 5);
	}
	else {
		if (bWetoutJudgeEnable && (minDistance_in < Spec_In_Min || maxDistance_in > Spec_In_Max)) { cl = Gdiplus::Color::Red; pDC->SetTextColor(COLOR_RED);		color = cv::Scalar(0, 0, 255); }
		else { cl = Gdiplus::Color::GreenYellow; pDC->SetTextColor(COLOR_BLUE);	color = cv::Scalar(255, 64, 64); }
		strText.Format("[Wetout_In Min: %.4f , Max: %.4f]mm", minmax_MinValue[0], minmax_MaxValue[0]);
		//pDC->TextOutA(100, 400, strText);
		mViewer->AddHardGraphic(new GraphicLabel(10.f, 70.f, fmt(strText), cl));
		cv::putText(m_pMain->m_matResultImage, std::string(strText), cv::Point(100, 400), cv::FONT_ITALIC, 2, color, 5);

		if (bWetoutJudgeEnable && (minDistance_out < Spec_Out_Min || maxDistance_out > Spec_Out_MAX)) { cl = Gdiplus::Color::Red; pDC->SetTextColor(COLOR_RED);			color = cv::Scalar(0, 0, 255); }
		else { cl = Gdiplus::Color(128, 128, 255); pDC->SetTextColor(COLOR_DARK_BLUE);		color = cv::Scalar(255, 128, 128); }
		strText.Format("[Wetout_Out Min: %.4f , Max: %.4f]mm", minmax_MinValue[1], minmax_MaxValue[1]);
		//pDC->TextOutA(100, 500, strText);
		mViewer->AddHardGraphic(new GraphicLabel(10.f, 90.f, fmt(strText), cl));
		cv::putText(m_pMain->m_matResultImage, std::string(strText), cv::Point(100, 500), cv::FONT_ITALIC, 2, color, 5);
	}

	bool   bWetoutBJudgeModeEnable = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getWetoutBJudgeModeEnable();
	double BminSpec = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getWetoutBMinSpec();
	double BmaxSpec = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getWetoutBMaxSpec(); //22.06.02

	//SJB 2022-11-29 Pocket Length 사용 안함
	/*if (method == METHOD_LINE)
	{
		pDC->SetTextColor(COLOR_GREEN);
		strText.Format("[Pocket Length: %.4f]mm", m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultT);
		cv::putText(m_pMain->m_matResultImage, std::string(strText), cv::Point(100, 700), cv::FONT_ITALIC, 2, cv::Scalar(0, 0, 255), 5);

		mViewer->AddHardGraphic(new GraphicLabel(10.f, 110.f, fmt("[Pocket Length: %.4f]mm", m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultT), 0xff00ff00));
	}*/
	if (method == METHOD_CIRCLE)
	{
		if (bWetoutBJudgeModeEnable && (BminSpec > m_pMain->m_ELB_DiffInspResult[datasheet].m_BminResult
			|| BmaxSpec < m_pMain->m_ELB_DiffInspResult[datasheet].m_BmaxResult)) {
			cl = Gdiplus::Color::Red; pDC->SetTextColor(COLOR_RED);			color = cv::Scalar(0, 0, 255);
		}
		else { cl = Gdiplus::Color(255, 64, 255);  pDC->SetTextColor(COLOR_PURPLE);		color = cv::Scalar(255, 64, 255); }
		strText.Format("[B Min: %.4f , Max: %.4f]mm", m_pMain->m_ELB_DiffInspResult[datasheet].m_BminResult, m_pMain->m_ELB_DiffInspResult[datasheet].m_BmaxResult);
		//pDC->TextOutA(100, 600, strText);
		mViewer->AddHardGraphic(new GraphicLabel(10.f, 110.f, fmt(strText), cl));
		cv::putText(m_pMain->m_matResultImage, std::string(strText), cv::Point(100, 600), cv::FONT_ITALIC, 2, color, 5);
	}

	// KBJ 2022-10-10 ErrorCode 전송
	if (bReturn != TRUE)
	{
		g_CommPLC.SetWord(m_pMain->vt_job_info[nJob].plc_addr_info.write_word_start + 18, 1, &(long)m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType);

		pDC->SetTextColor(COLOR_RED);
		strText.Format("Error");
		//pDC->TextOutA(W / 7 * 4, 100, strText);
		mViewer->AddHardGraphic(new GraphicLabel(1000.f, 10.f, fmt(strText), Gdiplus::Color::Red));

		int offset = 20.f;
		for (int i = 0; i < ELB_INSP_MAX_ERROR_TYPE; i++)
		{
			if ((m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType & (1 << i)) != 0)
			{
				switch (i)
				{
				case ELB_INSP_ERROR_WET_IN_MIN:		str_temp.Format("[ELB_INSP_ERROR_WET_IN_MIN]");		break;
				case ELB_INSP_ERROR_WET_IN_MAX:		str_temp.Format("[ELB_INSP_ERROR_WET_IN_MAX]");		break;
				case ELB_INSP_ERROR_WET_OUT_MIN:	str_temp.Format("[ELB_INSP_ERROR_WET_OUT_MIN]");	break;
				case ELB_INSP_ERROR_WET_OUT_MAX:	str_temp.Format("[ELB_INSP_ERROR_WET_OUT_MAX]");	break;
				case ELB_INSP_ERROR_B_MIN:			str_temp.Format("[ELB_INSP_ERROR_B_MIN]");			break;
				case ELB_INSP_ERROR_B_MAX:			str_temp.Format("[ELB_INSP_ERROR_B_MAX]");			break;
				case ELB_INSP_ERROR_R_MIN:			str_temp.Format("[ELB_INSP_ERROR_R_MIN]");			break;
				case ELB_INSP_ERROR_R_MAX:			str_temp.Format("[ELB_INSP_ERROR_R_MAX]");			break;
				case ELB_INSP_ERROR_LACK_TOTAL:		str_temp.Format("[ELB_INSP_ERROR_LACK_TOTAL]");		break;
				case ELB_INSP_ERROR_LACK_CONTINUE:	str_temp.Format("[ELB_INSP_ERROR_LACK_CONTINUE]");	break;
				case ELB_INSP_ERROR_OVER_FLOW:		str_temp.Format("[ELB_INSP_ERROR_OVER_FLOW]");		break;
				case ELB_INSP_ERROR_UNDER_FLOW:		str_temp.Format("[ELB_INSP_ERROR_UNDER_FLOW]");		break;
				case ELB_INSP_ERROR_LINE_WIDTH:		str_temp.Format("[ELB_INSP_ERROR_LINE_WIDTH]");		break;
				}

				//pDC->TextOutA(W/7*4 , 100 + offset, str_temp);
				mViewer->AddHardGraphic(new GraphicLabel(1000.f, 10.f + offset, fmt(str_temp), Gdiplus::Color::Red));
				cv::putText(m_pMain->m_matResultImage, std::string(str_temp), cv::Point(W / 7 * 4, 100 + offset), cv::FONT_ITALIC, 2, Scalar(0, 0, 255), 5);
				offset += 20.f;

				strText.Format("[%s] %s", m_pMain->vt_job_info[nJob].get_main_object_id(), str_temp);
				::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strText);
			}
		}

		str_temp.Format("[%s] Error Code : %ld", m_pMain->vt_job_info[nJob].get_main_object_id(), m_pMain->m_ELB_DiffInspResult[datasheet]._nErrorType);
		::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str_temp);
	}

	if (mViewer != NULL)
	{
		//mViewer->Invalidate();
		mViewer->DirtyRefresh();
	}
	else
	{
		m_pDlgViewerMain[nview]->GetViewer().OnLoadImageFromPtr(imgSrc.data);
		//m_pDlgViewerMain[nview]->GetViewer().Invalidate();
		m_pDlgViewerMain[nview]->GetViewer().DirtyRefresh();
	}

	penOK.DeleteObject();
	penNG.DeleteObject();
	penSum.DeleteObject();
	penCut.DeleteObject();
	penBM.DeleteObject();
	penDebug.DeleteObject();
	penDebug2.DeleteObject();
	penDB.DeleteObject();
	penRed.DeleteObject();
	penMin.DeleteObject();
	penGreenYellow.DeleteObject();
	penSkyblue.DeleteObject();
	penLime.DeleteObject();

	element15.release();
	bin.release();
	bin2.release();
	diff_im.release();
	diff_im2.release();
	skeleton.release();
	skeleton2.release();
	imgSrc.release(); // 도포된 현재 이미지
	orgimgSrc.release(); //도포전 이미지
	img.release();
	orgimg.release();
	binOrg.release();
	mask.release();
	maskerode.release();
	s_down.release();
	s_downorg.release();
	s_thinMat.release(); // 축소 해서 검사 진행
	diff_im_org.release();
	s_CINK2.release();

	//	m_pMain->m_pCircleProgress.SetPercent(100);

	::SendMessage(m_pMain->m_hWnd, WM_VIEW_CONTROL, MSG_MD_DIFF_RESULT_WRITE, MAKELPARAM(nJob, bReturn ? TRUE : FALSE));
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->GetSafeHwnd(), WM_VIEW_CONTROL, MSG_PA_JUDGE_DISPLAY, MAKELPARAM(nJob, bReturn ? TRUE : FALSE));
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->GetSafeHwnd(), WM_VIEW_CONTROL, MSG_PA_SPEC_VIEW, NULL);
	m_pMain->m_nSpecRowCount++;
	if (m_pMain->m_nSpecRowCount > df_MAX_CNT + 1)
		m_pMain->m_nSpecRowCount = 2;

	//m_pMain->m_pCircleProgress.SetPercent(100);  //20210910 Tkyuha 로그 파일 작성 완료 시점 위치 변경

#ifdef _DATA_BASE
	// DataBase에 저장 Tkyuha 20211221
	_st_SUM_RESULT_DATA _result; // DataBase에 저장 Tkyuha 20211221
	memset(&_result, 0, sizeof(_st_SUM_RESULT_DATA));
	//HTK 2022-06-29 Database에 Job이름 저장
	_result._bok = bReturn; _result._bng = !bReturn;  _result._jobID = nJob;
	if (method == METHOD_CIRCLE)
	{
		int n = 4;
		int step = 360 / n;
		for (int i = 0; i < n; i++)
		{
			int curDeg = (270 + (step * i)) % 360;
			double m_nWetOut = m_pMain->m_ELB_DiffInspResult[1].m_vTraceProfile[0].at(curDeg);
			switch (i)
			{
			case 0:	_result._r1 = m_nWetOut; break;
			case 1:	_result._r2 = m_nWetOut; break;
			case 2:	_result._r3 = m_nWetOut; break;
			case 3:	_result._r4 = m_nWetOut; break;
			case 4:	_result._r5 = m_nWetOut; break;
			}
		}
	}

	m_pMain->m_dbResultDataBase.insertResultDataBase(_result); // DataBase에 저장 Tkyuha 20211221
#endif

	//HTK 2022-03-30 Tact Time Display 추가
	m_clockProc_elapsed_time[5] = (clock() - m_clockProcStartTime[5]) / (float)CLOCKS_PER_SEC;
	((CPaneAuto*)m_pMain->m_pPane[PANE_AUTO])->update_grid_processing_time(1, m_clockProc_elapsed_time[5], 5, bReturn);

	//KJH 2022-07-27 Tact Time Log 추가
	theLog.logmsg(LOG_PLC_SEQ, "Diff Wetout Insp Tact Time = %.4f", m_clockProc_elapsed_time[5]);

	if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getWetoutInspectionSimulation())                        //Tkyuha 221109                       //Wetout 검사 이미지 한장으로 검사
	{
		m_pMain->m_ELB_DiffInspResult[datasheet].m_bResultSummary = TRUE;
		m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_bResultSummary = TRUE;
		m_pMain->m_ELB_DiffInspResult[datasheet].bType = TRUE;
		m_pMain->m_ELB_DiffInspResult[datasheet].bRJudge = TRUE;
		m_pMain->m_ELB_DiffInspResult[datasheet].bBJudge = TRUE;
		bReturn = TRUE;
	}
	return bReturn;
}
//줄빠짐검사
bool CFormMainView::fnEmptyInspection(int nJob, int id, CViewerEx* mViewer)
{
	int nview = m_pMain->vt_job_info[nJob].viewer_index[id];

	bool bReturn = true;

	CDC* pDC = NULL;
	
	//if (mViewer != NULL)
	//{
	//	pDC = mViewer->getOverlayDC();
	//	mViewer->ClearOverlayDC();
	//	mViewer->clearAllFigures();
	//}
	//else
	//{
	//	pDC = m_pDlgViewerMain[nview]->GetViewer().getOverlayDC();
	//	m_pDlgViewerMain[nview]->GetViewer().ClearOverlayDC();
	//	m_pDlgViewerMain[nview]->GetViewer().clearAllFigures();
	//}

	int camCount = m_pMain->vt_job_info[nJob].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[nJob].camera_index;

	int nCam = 0;
	CString str;
	int real_cam = camBuf.at(nCam);
	int W = m_pMain->m_stCamInfo[real_cam].w;
	int H = m_pMain->m_stCamInfo[real_cam].h;

	str.Format("[Cam %d] - Empty Inspection Start", real_cam + 1);
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

	m_pMain->copyMemory(m_pMain->getProcBuffer(real_cam, 0), m_pMain->getSrcBuffer(real_cam), W * H);

	cv::Rect inspRoi = cv::Rect(0, 0, W, H);
	int method = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod();
	double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(nCam, 0);
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(nCam, 0);
	int dThresh = 255 - m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspDustThresh();

	//KJH 2022-07-15 DiffWetOut OrgImg ProcBuffer 변경
	cv::Mat imgSrc(H, W, CV_8UC1, m_pMain->getProcBuffer(real_cam, 3));
	cv::Mat orgimgSrc(H, W, CV_8UC1, m_pMain->getProcBuffer(real_cam, 2)); //도포전 이미지
	cv::Mat img, mask, diff_im, diff_im_org;

	int nDefect = 0;

	bool testmode = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getTestImageSave();

	double radius2 = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspSteelHoleSize() / xres + 10;

	cv::Mat mask2 = GetCirclemask(imgSrc, m_ptCircleCenter_CC.x, m_ptCircleCenter_CC.y, m_dCircleRadius_CC);
	cv::Mat mask3 = GetCirclemask(imgSrc, m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x, m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y, m_pMain->m_dCircleRadius_PN - 3);

	cv::subtract(mask3, mask2, mask);

	cv::absdiff(imgSrc, orgimgSrc, diff_im_org);

	inspRoi = cv::Rect(m_ptCircleCenter_MP.x - m_dCircleRadius_MP, m_ptCircleCenter_MP.y - m_dCircleRadius_MP, m_dCircleRadius_MP * 2, m_dCircleRadius_MP * 2);

	threshold(diff_im_org, diff_im_org, 20, 256, CV_THRESH_BINARY | CV_THRESH_OTSU);

	diff_im_org.copyTo(img, mask);

	cvShowImage("EMPTY_Diff", diff_im_org);

	cv::Mat s_down; // 축소 해서 검사 진행
	cv::resize(img(inspRoi), s_down, cv::Size(), 0.25, 0.25, CV_INTER_CUBIC);
	threshold(s_down, s_down, 20, 256, CV_THRESH_BINARY);

	cvShowImage("InspOrg Image", s_down);

	if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
	{
		cvShowImage("InspOrg Image", s_down);
		cvShowImage("Mask Image", mask);
		cvShowImage("Gray Image", img);
	}

	int spec_S = MAX(10, int(m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(1) / xres) / 4); //최소 크기
	int spec_A = MAX(250, int(m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(1) / xres) / 4); //최소 면적

	nDefect = InspDustBurrInspection(nJob, s_down, dThresh, inspRoi.x, inspRoi.y, 4, spec_S, spec_A, true); //10,25

	if (nDefect > 0)
	{
		for (int i = 0; i< int(m_vecDustResult.size()); i++)
		{
			m_vecDustResult.at(i).center.x = m_vecDustResult.at(i).center.x * 4 + inspRoi.x;
			m_vecDustResult.at(i).center.y = m_vecDustResult.at(i).center.y * 4 + inspRoi.y;
			m_vecDustResult.at(i).rr.center.x = m_vecDustResult.at(i).rr.center.x * 4 + inspRoi.x;
			m_vecDustResult.at(i).rr.center.y = m_vecDustResult.at(i).rr.center.y * 4 + inspRoi.y;
		}
	}
	img.release();
	
	if (nDefect > 0)
	{
		FakeDC* pDC = NULL;
		CString strText;
		int nview = m_pMain->vt_job_info[nJob].viewer_index[id];
		if (mViewer != NULL)		pDC = mViewer->getOverlayDC();
		else		pDC = m_pDlgViewerMain[nview]->GetViewer().getOverlayDC();

		CPen penNG(PS_SOLID, 5, COLOR_RED);
		CPen penROI(PS_SOLID, 5, COLOR_WHITE);
		CPen* pOldPen;
		CFont font, * oldFont;
		font.CreateFont(50, 50, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
			OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
		oldFont = pDC->SelectObject(&font);
		pDC->SelectObject(GetStockObject(NULL_BRUSH));
		pDC->SetTextColor(COLOR_RED);

		//KJH 2021-12-02 Dust Insp로 검출시 ROI 표시
		pDC->SelectObject(&penROI);

		pOldPen = pDC->SelectObject(&penNG);

		for (int i = 0; i < nDefect; i++)
		{
			cv::RotatedRect rr = m_vecDustResult.at(i).rr;
			cv::Rect brect = rr.boundingRect();
			cv::Point2f vertices2f[4];
			rr.points(vertices2f);

			cv::Point vertices[4];
			for (int j = 0; j < 4; ++j) {
				vertices[j] = vertices2f[j];
			}

			pDC->MoveTo(vertices[0].x, vertices[0].y);
			pDC->LineTo(vertices[1].x, vertices[1].y);
			pDC->LineTo(vertices[2].x, vertices[2].y);
			pDC->LineTo(vertices[3].x, vertices[3].y);
			pDC->LineTo(vertices[0].x, vertices[0].y);

			strText.Format("[S: %.3f,%.3fmm, A: %.2f]", m_vecDustResult.at(i).xw * xres, m_vecDustResult.at(i).yw * yres, m_vecDustResult.at(i).area * 4);
			pDC->TextOutA(m_vecDustResult.at(i).center.x, m_vecDustResult.at(i).center.y, strText);
		}

		if (mViewer != NULL)				mViewer->Invalidate();
		else	m_pDlgViewerMain[nview]->GetViewer().Invalidate();

		penNG.DeleteObject();
		font.DeleteObject();
		penROI.DeleteObject();

		bReturn = false;
	}

	return bReturn;
}
//HTK 2022-03-21 두 원의 중점사이 각도 구하기
double CFormMainView::fnSearchWideAngle(cv::Point2f _Mpt, cv::Point2f _Gpt)
{
	double dValue = 0;

	dValue = atan2(_Mpt.y - _Gpt.y, _Mpt.x - _Gpt.x);
	dValue = ToDegree(dValue);

	return dValue;
}

bool compareContourAreas(std::vector<cv::Point> contour1, std::vector<cv::Point> contour2) {
	double i = fabs(contourArea(cv::Mat(contour1)));
	double j = fabs(contourArea(cv::Mat(contour2)));
	return (i < j);
}

BOOL CFormMainView::InspNotchLine(BYTE* pImage, int w, int h, int nJob, int nCam)
{
	BOOL breturn = TRUE;
	cv::Mat img(h, w, CV_8UC1, pImage);
	cv::Mat median, dstImage, dstImage2;

	img.copyTo(median);

	if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseIncludedAngle())
	{
		sobelDirection(3, SOBEL_UP, median, dstImage);
		sobelDirection(3, SOBEL_LEFT, median, dstImage2);
	}
	else
	{
		sobelDirection(3, SOBEL_DN, median, dstImage);
		sobelDirection(3, SOBEL_RIGHT, median, dstImage2);
	}
	////////////////////////
	// 형상 매칭 후 켈리퍼 사용
	//////////////////////////
	BOOL bFixtureUse = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseFixtureMark();
	double dx = 0, dy = 0, dt = 0;
	if (bFixtureUse)
	{
		CPatternMatching* pPattern = &m_pMain->GetMatching(nJob);
		double fixtureX = pPattern->getFixtureX(nCam, 0);
		double fixtureY = pPattern->getFixtureY(nCam, 0);

		pPattern->findPattern(median.data, nCam, 0, w, h);

		if (m_pMain->GetMatching(nJob).getFindInfo(nCam, 0).GetFound() == FIND_OK)
		{
			double posX = m_pMain->GetMatching(nJob).getFindInfo(nCam, 0).GetXPos();
			double posY = m_pMain->GetMatching(nJob).getFindInfo(nCam, 0).GetYPos();

			if (fixtureX != 0.0 && fixtureY != 0.0)
			{
				dx = posX - fixtureX;
				dy = posY - fixtureY;
				dt = 0.0;
			}
		}
	}
	/////////////////////////

	cv::Rect rect_roi;

	rect_roi.x = w / 4;
	rect_roi.y = h / 4;
	rect_roi.width = w / 2;
	rect_roi.height = h / 2;

	cv::medianBlur(median(rect_roi), median(rect_roi), 11);

	if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseCrackInsp())
	{
		m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][0].processCaliper(dstImage.data, w, h, dx, dy, dt);
		m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][1].processCaliper(dstImage.data, w, h, dx, dy, dt);
	}
	else
	{
		m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][0].processCaliper(median.data, w, h, dx, dy, dt);
		m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][1].processCaliper(median.data, w, h, dx, dy, dt);
	}

	BOOL find_line[3];
	sLine line_info[3];

	find_line[0] = m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][0].getIsMakeLine();
	find_line[1] = m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][1].getIsMakeLine();
	line_info[0] = m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][0].m_lineInfo;
	line_info[1] = m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][1].m_lineInfo;

	if (find_line[0] && find_line[1])
	{
		double y1 = line_info[0].sy;
		double y2 = line_info[1].sy;
		double target_y = (y1 + y2) / 2.0;

		_st32fPoint pt_start_b = m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][2].getOrgStartPt();
		_st32fPoint pt_end_b = m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][2].getOrgEndPt();
		_st32fPoint pt_start = pt_start_b, pt_end = pt_end_b;

		double cy = (pt_start.y + pt_end.y) / 2.0;
		double dy = fabs(target_y - cy);

		//pt_start.y += dy;
		//pt_end.y += dy;
		if (pt_start.y < pt_end.y)
		{
			pt_start.y = target_y - 1;
			pt_end.y = target_y + 1;
		}
		else
		{
			pt_start.y = target_y + 1;
			pt_end.y = target_y - 1;
		}

		//m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][2].setProjectionLength(3);
		m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][2].setOrgStartPt(pt_start);
		m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][2].setOrgEndPt(pt_end);

		//m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][2].processCaliper(median.data, w, h, 0.0, 0.0, 0.0);
		m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][2].processCaliper(dstImage2.data, w, h, 0.0, 0.0, 0.0);

		find_line[2] = m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][2].getIsMakeLine();

		if (find_line[2])
		{
			line_info[2] = m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][2].m_lineInfo;
			m_pMain->notch_line_data.pt_mid.x = line_info[2].sx;
			m_pMain->notch_line_data.pt_mid.y = target_y;
		}

		m_pMain->notch_line_data.pt_up.x = line_info[0].sx;
		m_pMain->notch_line_data.pt_up.y = line_info[0].sy;
		m_pMain->notch_line_data.pt_down.x = line_info[1].sx;
		m_pMain->notch_line_data.pt_down.y = line_info[1].sy;

		//위아래 켈리퍼 기울기 지정한 값
		if (fabs(line_info[0].a) > m_pMain->vt_job_info[nJob].model_info.getMachineInfo().getRevisionLimit(AXIS_T) ||
			fabs(line_info[1].a) > m_pMain->vt_job_info[nJob].model_info.getMachineInfo().getRevisionLimit(AXIS_T))
		{
			CString str_temp;
			str_temp.Format("Angle limit, up angle : %.3f, down angle : %.3f", line_info[0].a, line_info[1].a);
			::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str_temp);

			breturn = FALSE;
		}
		else breturn = TRUE;

		////HSJ 센터 켈리퍼 일정 기울기 이상일 시 NG
		//if (fabs(line_info[2].a) < 15.00) 
		//{
		//	CString str_temp;
		//	str_temp.Format("line_info[2].a : %.3f", line_info[2].a);
		//	::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str_temp);

		//	breturn = FALSE;
		//}
		//else breturn = TRUE;

		m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][2].setOrgStartPt(pt_start_b);
		m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][2].setOrgEndPt(pt_end_b);
		m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][2].calcCaliperRect();
	}
	else
	{
		m_pMain->notch_line_data.pt_mid.x = 0.0;
		m_pMain->notch_line_data.pt_mid.y = 0.0;
		m_pMain->notch_line_data.pt_up.x = 0.0;
		m_pMain->notch_line_data.pt_up.y = 0.0;
		m_pMain->notch_line_data.pt_down.x = 0.0;
		m_pMain->notch_line_data.pt_down.y = 0.0;

		breturn = FALSE;
	}

	median.release();

	return breturn;
}
BOOL CFormMainView::InspNozzleSearchInspection(BYTE* pImage, int w, int h, int nJob, int nCam, CViewerEx* mViewer, bool clr)
{
	BOOL breturn = TRUE;

	FakeDC* pDC = NULL;
	CPen penOrg(PS_SOLID, 2, RGB(0, 0, 255));
	CPen penSelect(PS_SOLID, 3, RGB(255, 0, 0));
	CPen penAlign(PS_SOLID, 3, RGB(255, 255, 0));

	if (mViewer != NULL)
	{
		if (clr)		mViewer->ClearOverlayDC();

		pDC = mViewer->getOverlayDC();
		pDC->SelectObject(&penOrg);
		pDC->SelectStockObject(NULL_BRUSH);
	}

	cv::Mat img(h, w, CV_8UC1, pImage), bin, bin2;
	cv::Rect r = cv::Rect(w / 2 - 700, h / 2 - 700, 1400, 1400);

	cv::threshold(img, bin2, -1, 255, CV_THRESH_BINARY_INV | THRESH_OTSU);//
	dilate(bin2, bin2, Mat::ones(cv::Size(3, 3), CV_8UC1), cv::Point(-1, -1), 1);

	vector<vector<cv::Point>> contours;
	vector<cv::Vec4i> hierarchy;

	findContours(bin2, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

	CString str;
	CRect InspRoi = CRect(0, 0, w - 1, h - 1);
	cv::Rect bounding_rect;
	int defect = 0;

	int ID = getMaxAreaContourId(contours);
	if (ID != -1)
	{
		bounding_rect = boundingRect(contours[ID]);

		int lw = bounding_rect.width; //l/ xres;//
		int lh = bounding_rect.height; //l / xres;// 

		InspRoi.left = bounding_rect.x;
		InspRoi.top = bounding_rect.y;
		InspRoi.right = MIN(w - 1, MAX(0, InspRoi.left + lw));
		InspRoi.bottom = MIN(h - 1, MAX(0, InspRoi.right + lh));
	}

#ifdef _DAHUHA
	AcquireSRWLockExclusive(&m_pMain->m_pDahuhaCam[nCam].g_bufsrwlock); // 동기화 시킴 Tkyuha 20211027
#else
	AcquireSRWLockExclusive(&m_pMain->m_pBaslerCam[nCam]->g_bufsrwlock); // 동기화 시킴 Tkyuha 20211027
#endif
	m_pMain->m_bGrabEnd[nCam] = FALSE;
#ifdef _DAHUHA
	ReleaseSRWLockExclusive(&m_pMain->m_pDahuhaCam[nCam].g_bufsrwlock);
#else
	ReleaseSRWLockExclusive(&m_pMain->m_pBaslerCam[nCam]->g_bufsrwlock);
#endif

	if (m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_ELB_CENTER_ALIGN) // Light ON + MAX
	{
		m_pMain->LightControllerTurnOnOff(nJob, 0, 1);

		//int camCount = m_pMain->vt_job_info[nJob].num_of_camera;
		//for (int nLight = 0; nLight < m_pMain->vt_job_info[nJob].light_info[0].num_of_using_light; nLight++)
		//{
		//	int ctrl = m_pMain->vt_job_info[nJob].light_info[0].controller_id[nLight] - 1;
		//	int ch = m_pMain->vt_job_info[nJob].light_info[0].channel_id[nLight];
		//	int value = m_pMain->vt_job_info[nJob].model_info.getLightInfo().getLightBright(0, nLight, 1);

		//	if (m_pMain->m_stLightCtrlInfo.nType[0] == LTYPE_LLIGHT_LPC_COT_232)	m_pMain->SetLightBright(ctrl, ch, value == 0 ? 0 : value * 10);   // dh.jung 2021-07-12
		//	else																	m_pMain->SetLightBright(ctrl, ch, value);
		//	Delay(50);
		//}
	}
#ifdef _DAHUHA
	if (m_pMain->m_pDahuhaCam[nCam].isconnected())
#else
	if (m_pMain->m_pBaslerCam[nCam])
#endif
	{
		str.Format("[%s] Cam %d - Grab Start", m_pMain->vt_job_info[nJob].job_name.c_str(), nCam + 1);
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		m_pMain->SendMessage(WM_VIEW_CONTROL, MSG_MD_GRAB_THREAD, nCam);

		MSG msg;
		BOOL bGrabComplete = TRUE;
		seq_SetTimer(nCam, 3000);
		do
		{
			bGrabComplete = TRUE;
			if (m_pMain->m_bGrabEnd[nCam] != TRUE)		bGrabComplete = FALSE;

			while (::PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE))	::DispatchMessage(&msg);
			Sleep(10);

		} while (bGrabComplete && seq_ChkTimer(nCam) == false);

		seq_ResetChkTimer(nCam);

		cv::Mat diff_im, s_RoiImage, s_Enh; // 축소 해서 검사 진행
		cv::Mat s_downorg(h, w, CV_8UC1, m_pMain->getSrcBuffer(nCam));

		cv::absdiff(img, s_downorg, diff_im);

		diff_im(bounding_rect).copyTo(s_RoiImage);

		m_pMain->m_ManualAlignMeasure.InspectionEnhance(&s_RoiImage, 0, false);
		threshold(s_RoiImage, s_Enh, 80, 255, CV_THRESH_BINARY_INV);

		contours.clear();
		hierarchy.clear();

		findContours(s_Enh, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
		cv::Mat drawing = cv::Mat::zeros(s_Enh.size(), CV_8U);

		for (int i = 0; i < contours.size(); i++)
		{
			cv::Moments moms = moments(cv::Mat(contours[i]));
			double area = moms.m00;

			if (area > 10 && area < 500)
			{
				cv::Rect brect = boundingRect(contours[i]);
				drawContours(drawing, contours, i, cv::Scalar(255), CV_FILLED, 8, hierarchy, 0, cv::Point());
				pDC->Ellipse(int(brect.x + bounding_rect.x), int(brect.y + bounding_rect.y),
					int(brect.x + bounding_rect.x + brect.width), int(brect.y + bounding_rect.y + brect.height));
				defect++;
			}
		}
	}

	if (m_pMain->vt_job_info[nJob].algo_method == CLIENT_TYPE_ELB_CENTER_ALIGN)
	{
		m_pMain->LightControllerTurnOnOff(nJob, 0, 0);

		//for (int nLight = 0; nLight < m_pMain->vt_job_info[nJob].light_info[0].num_of_using_light; nLight++)
		//{
		//	int ctrl = m_pMain->vt_job_info[nJob].light_info[0].controller_id[nLight] - 1;
		//	int ch = m_pMain->vt_job_info[nJob].light_info[0].channel_id[nLight];
		//	int value = m_pMain->vt_job_info[nJob].model_info.getLightInfo().getLightBright(0, nLight, 0);

		//	if (m_pMain->m_stLightCtrlInfo.nType[0] == LTYPE_LLIGHT_LPC_COT_232)	m_pMain->SetLightBright(ctrl, ch, value == 0 ? 0 : value * 10);   // dh.jung 2021-07-12
		//	else																	m_pMain->SetLightBright(ctrl, ch, value);
		//	Delay(50);
		//}
	}
	if(mViewer)
		mViewer->Invalidate();
	penOrg.DeleteObject();
	penSelect.DeleteObject();
	penAlign.DeleteObject();

	if (defect > 0) breturn = FALSE;

	return breturn;
}
BOOL CFormMainView::InspNozzleSearchAngle_Bee(BYTE* pImage, int w, int h, int nJob, int nCam, CViewerEx* mViewer, BOOL Reverse)
{
	BOOL breturn = FALSE;

	FakeDC* pDC = NULL;
	CPen penOrg(PS_SOLID, 2, RGB(0, 0, 255));
	CPen penSelect(PS_SOLID, 3, RGB(255, 0, 0));
	CPen penAlign(PS_SOLID, 3, RGB(255, 255, 0));

	if (mViewer != NULL)
	{
		mViewer->ClearOverlayDC();
		pDC = mViewer->getOverlayDC();
		pDC->SelectObject(&penOrg);
	}

	cv::Mat img(h, w, CV_8UC1, pImage), bin, bin2, sobelUp, sobelDn, sobelImg;
	cv::Rect r = cv::Rect(700, 350, w - 1400, h - 700);
	double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(0, 0);
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);

	if(m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getNozzleAngleSearchMode())
	{ // Enhance 를 이용하여 노즐 각도 찾는 방법 추가 / Edge가 선명 하지 않는 경우에 처리
		cv::Mat iproc;
		cv::resize(img, iproc, cv::Size(), 0.5, 0.5, CV_INTER_CUBIC);
		m_pMain->m_ManualAlignMeasure.InspectionEnhance(&iproc, 0, false);
		cv::resize(iproc, iproc, cv::Size(), 2, 2, CV_INTER_CUBIC);

		int  _th = 80;
		if (Reverse)	cv::threshold(iproc(r), bin2, _th, 255, CV_THRESH_BINARY);//
		else			cv::threshold(iproc(r), bin2, _th, 255, CV_THRESH_BINARY_INV);//

		iproc.release();
	}
	else
	{
		if (Reverse)	cv::threshold(img(r), bin2, -1, 255, CV_THRESH_BINARY | THRESH_OTSU);//
		else			cv::threshold(img(r), bin2, -1, 255, CV_THRESH_BINARY_INV | THRESH_OTSU);//
	}

	cv::Canny(bin2, bin, 20, 80);

	if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
	{
		cvShowImage("Nozzle_Threshold_Image", bin2);
		cvShowImage("Nozzle_Canny_Image", bin);
	}

	std::vector<cv::Vec4i> lines;
	cv::HoughLinesP(bin, lines, 1, CV_PI / 180, 100, 150, 100);

	if (lines.size() > 0)
	{
		vector<Vec3d> ids0;
		vector<Vec3d> ids;

		CString strText;
		double last = DBL_MAX, a, b;

		// Calculate all lines equation
		for (int i = 0; i < lines.size(); i++)
		{
			GetLineCoef(lines[i][0], lines[i][1], lines[i][2], lines[i][3], a, b);
			ids0.push_back({ a, (double)i, b });
		}

		//Increased sorting
		std::sort(ids0.begin(), ids0.end(), [](const Vec3d& a, const Vec3d& b) { return b[0] > a[0]; });

		if (ids0.size() > 1)
		{
			ids.push_back(ids0[0]);
			ids.push_back(ids0[ids0.size() - 1]);
		}

		cv::Point2f _sizePt = cv::Point2f(-1, -1);
		cv::Point2f pt = cv::Point(-1, -1);
		cv::Point2f p_result, p_result2;

		if (ids.size() == 2)
		{
			auto a0 = ids[0][0];
			auto a1 = ids[1][0];
			auto i0 = (int)ids[0][1];
			auto i1 = (int)ids[1][1];
			auto b0 = ids[0][2];
			auto b1 = ids[1][2];

			pt = IntersectionPoint2(a0, b0, a1, b1);

			pt.x = pt.x + r.x;
			pt.y = pt.y + r.y;

			double radius = 800;
			double r0 = atan(a0);
			double r1 = atan(a1);

			if (r0 > 0)r0 += CV_PI;
			if (r1 > 0)r1 += CV_PI;

			double x0 = cos(r0) * radius + pt.x;
			double y0 = sin(r0) * radius + pt.y;
			double x1 = cos(r1) * radius + pt.x;
			double y1 = sin(r1) * radius + pt.y;

			double x2 = (x0 + x1) / 2;
			double y2 = (y0 + y1) / 2;

			double dx = x2 - pt.x;
			double dy = y2 - pt.y;

			double th = atan2(dy, dx);
			double dth = 180 + (th * 180. / CV_PI);

			//// 노즐 크기 측정 하기
			// 1. 중앙직선식 만들기
			int _noiseCount = 0, _ifindX = 0, rposx, rposy;
			double _a, _b, _v;
			GetLineCoef(double(pt.x), double(pt.y), x2, y2, _a, _b);
			// 2. 교차점에서 중앙점 까지 수직 직선을 그려서 이진화 영상이 0인경우 까지 찾는다
			int lastP = int(fabs(x2 - pt.x));
			int _sign = (x2 - pt.x) > 0 ? 1 : -1;

			for (int i = 10; i < lastP; i++)
			{
				rposx = int((cos(atan(_a)) * i * _sign + pt.x));
				rposy = int((sin(atan(_a)) * i * _sign + pt.y));

				if (rposx > 0 && rposx < (bin2.cols+r.x) && rposy>0 && rposy < (bin2.rows+r.y))
				{
					if (bin2.data[(rposy-r.y) * bin2.cols + (rposx-r.x)] > 100)
					{
						_noiseCount++;
						_ifindX = rposx;
					}
				}

				if (_noiseCount > 10) break;
			}

			if (_noiseCount > 10)
			{
				_v = _a * _ifindX + _b;
				_sizePt.x = _ifindX;
				_sizePt.y = _v;

				double _newA = _a != 0 ? -1 / _a : 0, _newB;
				_newB = _sizePt.y - (_newA * _sizePt.x);

				double _fA, _fB, _eA, _eB;
				GetLineCoef(double(pt.x), double(pt.y), x0, y0, _fA, _fB);
				p_result = IntersectionPoint2(_fA, _fB, _newA, _newB);
				GetLineCoef(double(pt.x), double(pt.y), x1, y1, _eA, _eB);
				p_result2 = IntersectionPoint2(_eA, _eB, _newA, _newB);

				m_pMain->m_dNozzleTipSize = GetDistance(p_result, p_result2) * (xres + yres) / 2;
				strText.Format(" => (%.3fmm Nozzle)", m_pMain->m_dNozzleTipSize * 0.9);
				int nview = m_pMain->vt_job_info[nJob].viewer_index[0];
				m_pDlgViewerMain[nview]->setViewerName(m_pMain->vt_viewer_info[nview].viewer_name.c_str() + strText);

				::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strText);
			}


			if (mViewer != NULL)
			{
				CPoint ptS, ptE;

				pDC->SelectObject(&penAlign);
				ptS = CPoint(pt.x, pt.y);
				ptE = CPoint(x0, y0);
				pDC->MoveTo(ptS.x, ptS.y);
				pDC->LineTo(ptE.x, ptE.y);

				ptE = CPoint(x1, y1);
				pDC->MoveTo(ptS.x, ptS.y);
				pDC->LineTo(ptE.x, ptE.y);

				ptE = CPoint(x2, y2);
				pDC->MoveTo(ptS.x, ptS.y);
				pDC->LineTo(ptE.x, ptE.y);

				ptE = CPoint(p_result.x, p_result.y);
				pDC->MoveTo(p_result2.x, p_result2.y);
				pDC->LineTo(ptE.x, ptE.y);

				pDC->SelectObject(&penSelect);
				pDC->SetTextColor(RGB(255, 0, 0));

				CStringW str;
				CFont font, * oldFont;
				font.CreateFont(40, 40, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
					OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY,
					DEFAULT_PITCH | FF_DONTCARE, "Arial");
				oldFont = pDC->SelectObject(&font);

				for (int i = 0; i < ids.size(); i++)
				{
					int idx = ids[i][1];
					double d = atan(ids[i][0]);
					double Th = ToDegree(d > 0 ? d : d + CV_PI);

					ptS = CPoint(lines[idx][0] + r.x, lines[idx][1] + r.y);
					ptE = CPoint(lines[idx][2] + r.x, lines[idx][3] + r.y);
					
					str.Format(L"T: %.1f°", Th > 90 ? 180 - Th : Th);
					mViewer->AddSoftGraphic(new GraphicLabel(cv::Point2f((ptS.x + ptE.x) / 2.f, (ptS.y + ptE.y) / 2.f), str, Gdiplus::Color(0xff00ff00)));
				}

				pDC->Ellipse(int(pt.x - 2), int(pt.y - 2), int(pt.x + 2), int(pt.y + 2));
				pDC->Ellipse(int(x0 - 2), int(y0 - 2), int(x0 + 2), int(y0 + 2));
				pDC->Ellipse(int(x1 - 2), int(y1 - 2), int(x1 + 2), int(y1 + 2));
				pDC->Ellipse(int(x2 - 2), int(y2 - 2), int(x2 + 2), int(y2 + 2));
				pDC->Ellipse(int(_sizePt.x - 4), int(_sizePt.y - 4), int(_sizePt.x + 4), int(_sizePt.y + 4));
				pDC->Ellipse(int(p_result.x - 4), int(p_result.y - 4), int(p_result.x + 4), int(p_result.y + 4));
				pDC->Ellipse(int(p_result2.x - 4), int(p_result2.y - 4), int(p_result2.x + 4), int(p_result2.y + 4));

				pDC->SetTextColor(RGB(0, 255, 0));

				str.Format(L"T: %.1f° W: %.3fmm", dth > 90 ? 180 - dth : dth, m_pMain->m_dNozzleTipSize);
				mViewer->AddSoftGraphic(new GraphicLabel(cv::Point2f(pt.x, pt.y + 40), str, Gdiplus::Color(0xff00ff00)));

				m_pMain->m_dNozzleCurrentAngle = dth > 90 ? 180 - dth : dth;

				mViewer->Invalidate();
				font.DeleteObject();
			}
		}

		ids0.clear();
		ids.clear();
		lines.clear();

		breturn = TRUE;
	}

	penOrg.DeleteObject();
	penSelect.DeleteObject();
	penAlign.DeleteObject();

	img.release();
	bin.release();
	bin2.release();
	sobelUp.release();
	sobelDn.release();
	sobelImg.release();

	return breturn;
}

BOOL CFormMainView::InspNozzleSearchCaliper_Use(BYTE* pImage, int w, int h, int nJob, int nCam, cv::Point2f &ptResult,bool bGap,CViewerEx* mViewer, BOOL Reverse)
{
	BOOL breturn = FALSE;

	FakeDC* pDC = NULL;
	CPen penOrg(PS_SOLID, 2, RGB(0, 0, 255));
	CPen penSelect(PS_SOLID, 3, RGB(255, 0, 0));
	CPen penAlign(PS_SOLID, 3, RGB(255, 255, 0));

	if (mViewer != NULL)
	{
		mViewer->ClearOverlayDC();
		pDC = mViewer->getOverlayDC();
		pDC->SelectObject(&penOrg);
	}

	cv::Mat img(h, w, CV_8UC1, pImage);

	int nPos = 1; 
	_st32fPoint pt_start, pt_end;

	if (m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_6].getSearchLength() != 1000) //설정이 안되어 있는 경우 강제로 설정
	{		
		m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_6].setNumOfCalipers(30);
		m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_6].setNumOfIgnore(5);
		m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_6].setSearchType(SEARCH_TYPE_PEAK);
		m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_6].setThreshold(15);
		m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_6].setCircleCaliper(FALSE);
		m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_6].setHalfFilterSize(3);
		m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_6].setFittingMethod(2); // MLESAC			
	}

	if (nCam==0) //센터 카메라
	{ 
		if (m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_6].getSearchLength() != 1000) //설정이 안되어 있는 경우 강제로 설정
		{//  중앙 기준
			pt_start.x = 2736, pt_start.y = 2191;
			pt_end.x = 2755, pt_end.y = 1526;

			m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_6].setOrgStartPt(pt_start);
			m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_6].setOrgEndPt(pt_end);
			m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_6].setSearchLength(1000);
			m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_6].init_caliper_info();
		}
		else
		{
			pt_start = m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_6].getOrgStartPt();
			pt_end = m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_6].getOrgEndPt();
		}
		m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_6].setContrastDirection(0); // Dark To Light		
	}
	else // 측면 카메라
	{
		if (m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_6].getSearchLength() != 1000) //설정이 안되어 있는 경우 강제로 설정
		{
			if (bGap) // ZGAP 측정시 영역 설정
			{
				if (Reverse) // 오른쪽 상단
				{
					pt_start.x = 2300, pt_start.y = 426;
					pt_end.x = 1500, pt_end.y = 888;
				}
				else
				{  // 왼쪽 상단
					pt_start.x = 919, pt_start.y = 426;
					pt_end.x = 1633, pt_end.y = 888;
				}

				m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_6].setSearchLength(500);
			}
			else
			{
				if (Reverse) // 오른쪽 상단
				{
					pt_start.x = 1164, pt_start.y = 1223;
					pt_end.x = 2385, pt_end.y = 510;
				}
				else
				{  // 왼쪽 상단
					pt_start.x = 748, pt_start.y = 494;
					pt_end.x = 2003, pt_end.y = 1288;
				}

				m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_6].setSearchLength(1000);
			}

			m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_6].setOrgStartPt(pt_start);
			m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_6].setOrgEndPt(pt_end);
			

			m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_6].init_caliper_info();
		}
		else
		{
			pt_start = m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_6].getOrgStartPt();
			pt_end = m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_6].getOrgEndPt();
		}

		if (bGap) m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_6].setContrastDirection(0); // Dark To Light
		else m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_6].setContrastDirection(1); // Light To Dark
	}

	BOOL bFindLine[2];
	sLine line_info[2];

	cv::Point2f pt =cv::Point(0,0), p_result, p_result2, p_final= cv::Point2f(0, 0);
	cv::Point2f _sizePt = cv::Point2f(-1, -1);

	m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_6].setDirectionSwap(FALSE);
	m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_6].processCaliper(pImage, w, h, 0, 0, 0, TRUE);
	line_info[0] = m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_6].m_lineInfo;
	bFindLine[0] = m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_6].getIsMakeLine();

	m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_6].setDirectionSwap(TRUE);
	m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_6].processCaliper(pImage, w, h, 0, 0, 0, TRUE);
	line_info[1] = m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_6].m_lineInfo;
	bFindLine[1] = m_pMain->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_6].getIsMakeLine();

	if(bFindLine[0] && bFindLine[1])	pt = IntersectionPoint2(line_info[0].a, line_info[0].b, line_info[1].a, line_info[1].b);

	if (pt.x!=0 && pt.y!=0)
	{
		// 교차점 찾기
		double radius = 800;
		double r0 = atan(line_info[0].a);
		double r1 = atan(line_info[1].a);

		if (r0 > 0)r0 += CV_PI;
		if (r1 > 0)r1 += CV_PI;

		double x0 = cos(r0) * radius + pt.x;
		double y0 = sin(r0) * radius + pt.y;
		double x1 = cos(r1) * radius + pt.x;
		double y1 = sin(r1) * radius + pt.y;

		double x2 = (x0 + x1) / 2;
		double y2 = (y0 + y1) / 2;

		double _a, _b;
		GetLineCoef(double(pt.x), double(pt.y), x2, y2, _a, _b);

		cv::Mat sobelX, sobelMagnitude, iproc;
		cv::Mat sobelY, sobelOrientation, iprocResize;

		cv::resize(img, iprocResize, cv::Size(), 0.5, 0.5, CV_INTER_CUBIC);
		cv::Sobel(iprocResize, sobelX, CV_32F, 1, 0, 3);
		cv::Sobel(iprocResize, sobelY, CV_32F, 0, 1, 3);
		cv::cartToPolar(sobelX, sobelY, sobelMagnitude, sobelOrientation);
		normalize(sobelMagnitude, iproc, 0, 255, NORM_MINMAX, CV_8UC1);
		cv::resize(iproc, iproc, cv::Size(), 2, 2, CV_INTER_CUBIC);

		if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
		{
			cvShowImage("Nozzle_cartToPolar_Image", iproc);
		}

		int _noiseCount = 0, _ifindX = 0, _ifindY = 0, rposx, rposy;
		// 2. 교차점에서 중앙점 까지 수직 직선을 그려서 이진화 영상이 0인경우 까지 찾는다
		int lastP = int(fabs(x2 - pt.x));
		int _sign = (x2 - pt.x) > 0 ? 1 : -1;

		if (nCam == 0) //센터 카메라
		{
			lastP = int(fabs(y2 - pt.y));
	//		_sign = -1;
		}

		for (int i = 10; i < lastP; i++)
		{
			rposx = int((cos(atan(_a)) * i * _sign + pt.x));
			rposy = int((sin(atan(_a)) * i * _sign + pt.y));

			if (rposx > 0 && rposx < iproc.cols && rposy>0 && rposy < iproc.rows)
			{
				if (iproc.data[rposy * iproc.cols + rposx] > 40)
				{
					_noiseCount++;
					 _sizePt.y = rposy;
					_sizePt.x = rposx;
				}
				else _noiseCount = 0;
			}

			if (_noiseCount > 2)
			{
				_sizePt.x = int((cos(atan(_a)) * (i+1) * _sign + pt.x)); // 전처리로 이동된 이미지 2픽셀 보상 하기 위함
				_sizePt.y = int((sin(atan(_a)) * (i+1) * _sign + pt.y));
				break;
			}
		}

		if (_noiseCount > 2)
		{
			double _newA = _a != 0 ? -1 / _a : 0, _newB;
			_newB = _sizePt.y - (_newA * _sizePt.x);

			p_result = IntersectionPoint2(line_info[0].a, line_info[0].b, _newA, _newB); 
			p_result2 = IntersectionPoint2(line_info[1].a, line_info[1].b, _newA, _newB);

			if (nCam == 0) // Align Point 지점 선택 //센터 카메라
			{
				p_final.x = (p_result.x + p_result2.x) / 2;
				p_final.y = (p_result.y + p_result2.y) / 2;				
			}
			else
			{				
				if (p_result.y > p_result2.y)  p_final = p_result;
				else p_final = p_result2;
			}
		}

		if (mViewer != NULL)
		{
			CPoint ptS, ptE;

			pDC->SelectObject(&penAlign);

			if (nCam == 0) //센터카메라
			{
				ptS.y = pt_start.y;
				ptE.y = pt_end.y;

				if (line_info[0].a == 0) { ptS.x = line_info[0].b; ptE.x = line_info[0].b; }
				else {
					ptS.x = (ptS.y - line_info[0].b) / line_info[0].a;
					ptE.x = (ptE.y - line_info[0].b) / line_info[0].a;
				}
				pDC->MoveTo(ptS.x, ptS.y);
				pDC->LineTo(ptE.x, ptE.y);	

				
				if (line_info[1].a == 0) { ptS.x = line_info[1].b; ptE.x = line_info[1].b; }
				else {
					ptS.x = (ptS.y - line_info[1].b) / line_info[1].a;
					ptE.x = (ptE.y - line_info[1].b) / line_info[1].a;
				}					
				pDC->MoveTo(ptS.x, ptS.y);
				pDC->LineTo(ptE.x, ptE.y);
			}
			else
			{
				ptS.x = pt_start.x;				
				ptE.x = pt_end.x;				

				ptS.y = line_info[0].a * ptS.x + line_info[0].b;
				ptE.y = line_info[0].a * ptE.x + line_info[0].b;
				pDC->MoveTo(ptS.x, ptS.y);
				pDC->LineTo(ptE.x, ptE.y);

				ptS.y = line_info[1].a * ptS.x + line_info[1].b;
				ptE.y = line_info[1].a * ptE.x + line_info[1].b;
				pDC->MoveTo(ptS.x, ptS.y);
				pDC->LineTo(ptE.x, ptE.y);
			}

			pDC->Ellipse(int(pt.x - 2), int(pt.y - 2), int(pt.x + 2), int(pt.y + 2));			
			pDC->Ellipse(int(p_result.x - 2), int(p_result.y - 2), int(p_result.x + 2), int(p_result.y + 2));
			pDC->Ellipse(int(p_result2.x - 2), int(p_result2.y - 2), int(p_result2.x + 2), int(p_result2.y + 2));

			pDC->SelectObject(&penSelect);

			pDC->Ellipse(int(_sizePt.x - 2), int(_sizePt.y - 2), int(_sizePt.x + 2), int(_sizePt.y + 2));
			pDC->Ellipse(int(p_final.x - 2), int(p_final.y - 2), int(p_final.x + 2), int(p_final.y + 2));

			CStringW str;				
			pDC->SetTextColor(RGB(0, 255, 0));

			// hsj 2022-10-21 header display값 안떠서 추가
			m_pMain->GetMatching(nJob).getFindInfo(nCam, 0).SetXPos(p_final.x);
			m_pMain->GetMatching(nJob).getFindInfo(nCam, 0).SetYPos(p_final.y);

			// hsj 2022-10-21 결과값 이상하게 나와서 수정
			str.Format(L"P : %.2f , %.2f", p_final.x, p_final.y);
			mViewer->AddSoftGraphic(new GraphicLabel(cv::Point2f(p_final.x, p_final.y + 40), str, Gdiplus::Color(0xff00ff00)));

			mViewer->Invalidate();
		}
	

		iproc.release();
		iprocResize.release();
		sobelX.release();
		sobelMagnitude.release();
		sobelY.release();
		sobelOrientation.release();

		breturn = TRUE;
	}

	penOrg.DeleteObject();
	penSelect.DeleteObject();
	penAlign.DeleteObject();

	img.release();

	ptResult = p_final;

	return breturn;
}

BOOL CFormMainView::InspNotchAlignSearchLine(BYTE* pImage, int w, int h, int nJob, int nCam, CViewerEx* mViewer)
{
	BOOL breturn = FALSE;

	FakeDC* pDC = NULL;
	CPen penOrg(PS_SOLID, 3, RGB(0, 0, 255));

	if (mViewer != NULL)
	{
		mViewer->ClearOverlayDC();
		pDC = mViewer->getOverlayDC();
		pDC->SelectObject(&penOrg);
	}

	int range = 100, mergeD = 20;
	float l_midy = 0.f, r_midy = 0.f;
	cv::Mat img(h, w, CV_8UC1, pImage);
	cv::Mat median, roiImg, pOrgImage;

	std::vector<cv::Vec4i> lines;
	std::vector<cv::Vec4i> _s_lines;

	img.copyTo(median);
	cv::Rect rect_roi, insp_roi;
	bool bleft = false, bright = false;

	rect_roi.x = w / 4;
	rect_roi.y = h / 4;
	rect_roi.width = w / 2;
	rect_roi.height = h / 2;

	cv::medianBlur(median(rect_roi), median(rect_roi), 11);

	_st32fPoint lt = m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][0].getOrgStartPt();
	_st32fPoint rb = m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][0].getOrgEndPt();

	insp_roi.x = MIN(MAX(0, lt.x), w);
	insp_roi.y = MIN(MAX(lt.y - range, 0), w);
	insp_roi.width = MAX(0, MIN(rb.x - lt.x, w - insp_roi.x - 1));
	insp_roi.height = MAX(0, MIN(range * 2, w - insp_roi.y - 1));

	median(insp_roi).copyTo(roiImg);

	sobelDirection(3, SOBEL_DN, roiImg, pOrgImage);
	cv::HoughLinesP(pOrgImage, lines, 1, CV_PI / 2, 250, 800, 400);

	if (lines.size() > 0)
	{
		if (mViewer != NULL)
		{
			CPoint ptS, ptE;
			for (int i = 0; i < lines.size(); i++)
			{
				ptS = CPoint(lines[i][0] + insp_roi.x, lines[i][1] + insp_roi.y);
				ptE = CPoint(lines[i][2] + insp_roi.x, lines[i][3] + insp_roi.y);
				pDC->MoveTo(ptS.x, ptS.y);
				pDC->LineTo(ptE.x, ptE.y);
			}
		}

		for (int i = 1; i < lines.size(); i++)
		{
			if (lines[i][1] - lines[i - 1][1] < mergeD)
			{
				if (lines[i][1] > lines[i - 1][1])	_s_lines.push_back(lines[i - 1]);
				else _s_lines.push_back(lines[i - 1]);
			}
		}

		switch (lines.size())
		{
		case 1:	l_midy = (lines[0][1] + lines[0][3]) / 2.f;		break;
		case 2:
		{
			if (lines[0][1] > lines[1][3])		l_midy = (lines[0][1] + lines[0][3]) / 2.f;
			else l_midy = (lines[1][1] + lines[1][3]) / 2.f;
		}
		break;
		case 3:
		{
			l_midy = (lines[1][1] + lines[1][3]) / 2.f;
		}
		break;
		}

		m_pMain->notch_line_data.pt_up.y = l_midy;

		bleft = true;
	}


	roiImg.release();
	pOrgImage.release();
	lines.clear();

	lt = m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][1].getStartPt();
	rb = m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][1].getEndPt();

	insp_roi.x = MIN(MAX(0, lt.x), w);
	insp_roi.y = MIN(MAX(lt.y - range, 0), w);
	insp_roi.width = MAX(0, MIN(rb.x - lt.x, w - insp_roi.x - 1));
	insp_roi.height = MAX(0, MIN(range * 2, w - insp_roi.y - 1));
	median(insp_roi).copyTo(roiImg);

	sobelDirection(3, SOBEL_DN, roiImg, pOrgImage);
	thinImage(pOrgImage, pOrgImage);
	cv::HoughLinesP(pOrgImage, lines, 1, CV_PI / 2, 250, 200, 200);

	if (lines.size() > 0)
	{
		if (mViewer != NULL)
		{
			CPoint ptS, ptE;
			for (int i = 0; i < lines.size(); i++)
			{
				ptS = CPoint(lines[i][0] + insp_roi.x, lines[i][1] + insp_roi.y);
				ptE = CPoint(lines[i][2] + insp_roi.x, lines[i][3] + insp_roi.y);
				pDC->MoveTo(ptS.x, ptS.y);
				pDC->LineTo(ptE.x, ptE.y);
			}
		}

		switch (lines.size())
		{
		case 1:	r_midy = (lines[0][1] + lines[0][3]) / 2.f;		break;
		case 2:
		{
			if (lines[0][1] > lines[1][3])		r_midy = (lines[0][1] + lines[0][3]) / 2.f;
			else r_midy = (lines[1][1] + lines[1][3]) / 2.f;
		}
		break;
		case 3:
		{
			r_midy = (lines[1][1] + lines[1][3]) / 2.f;
		}
		break;
		}
		m_pMain->notch_line_data.pt_down.y = r_midy;

		bright = true;
	}

	if (bleft && bright)
	{
		char* _mag = pOrgImage.ptr<char>();
		int ypos = (r_midy + l_midy) / 2.f;
		int xpos = w / 2;
		for (int i = xpos + 20; i > xpos - 20; i--)
		{
			if (_mag[ypos * roiImg.cols + i] > 0)
			{
				xpos = i;
				break;
			}
		}

		m_pMain->notch_line_data.pt_up.x = xpos;
		m_pMain->notch_line_data.pt_down.x = xpos;

		m_pMain->notch_line_data.pt_mid.x = xpos;
		m_pMain->notch_line_data.pt_mid.y = (r_midy + l_midy) / 2.f;

		//위아래 켈리퍼 기울기 지정한 값
		breturn = TRUE;
	}
	else
	{
		m_pMain->notch_line_data.pt_mid.x = 0.0;
		m_pMain->notch_line_data.pt_mid.y = 0.0;
		m_pMain->notch_line_data.pt_up.x = 0.0;
		m_pMain->notch_line_data.pt_up.y = 0.0;
		m_pMain->notch_line_data.pt_down.x = 0.0;
		m_pMain->notch_line_data.pt_down.y = 0.0;

		breturn = FALSE;
	}


	median.release();
	penOrg.DeleteObject();
	return breturn;
}
BOOL CFormMainView::InspNotchEdgeDetection(bool bnot, bool bcir, int nJob, int nCam, CPoint ptStart, CPoint ptEnd, CViewerEx* mViewer, bool balign,bool clr)
{
	CString str5;
	str5.Format("[Trace] Find ROI : %d, %d, %d, %d", ptStart.x, ptStart.y, ptEnd.x, ptEnd.y);
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str5);

	system_clock::time_point start = system_clock::now();
	
	//HTK 2022-03-30 Tact Time Display 추가
	m_clockProcStartTime[3] = clock();

	FakeDC* pDC = NULL;
	CPen penROI(PS_SOLID, 3, RGB(255, 0, 0));
	CPen penFit(PS_SOLID, 3, RGB(0, 255, 255));
	CPen penOrg(PS_SOLID, 3, RGB(0, 0, 255));

	if (mViewer != NULL)
	{
		if (clr)
			mViewer->ClearOverlayDC();

		pDC = mViewer->getOverlayDC();
		pDC->SelectObject(&penROI);
	}

	int W = m_pMain->m_stCamInfo[nCam].w;
	int H = m_pMain->m_stCamInfo[nCam].h;

	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;

	cv::Mat bin, bin2, bin3;
	//KJH 2022-08-04 Trace 버퍼 변경
	cv::Mat img(H, W, CV_8UC1, m_pMain->getProcBuffer(nCam, 2));
	BOOL breturn = TRUE;

	if (balign)
	{
		BOOL b = InspNotchLine(img.data, W, H, nJob, nCam);
		//BOOL b = InspNotchAlignSearchLine(img.data, W, H, nJob, nCam,mViewer);
		if (b)
		{
			m_pMain->GetMatching(nJob).getFindInfo(nCam, 0).SetXPos(m_pMain->notch_line_data.pt_mid.x);
			m_pMain->GetMatching(nJob).getFindInfo(nCam, 0).SetYPos(m_pMain->notch_line_data.pt_mid.y);
			m_pMain->m_ELB_TraceResult.m_ELB_ResultXY = cv::Point2d(m_pMain->notch_line_data.pt_mid.x, m_pMain->notch_line_data.pt_mid.y);
			m_pMain->m_ELB_TraceResult.m_ELB_ResultT = 0.0;

			if (mViewer != NULL)
			{
				CPoint ptS, ptE;

				if (1)
				{
					sLine line_info[2];
					line_info[0] = m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][0].m_lineInfo;
					line_info[1] = m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][1].m_lineInfo;
					ptS.x = 0, ptE.x = W;
					ptS.y = line_info[0].a * ptS.x + line_info[0].b;
					ptE.y = line_info[0].a * ptE.x + line_info[0].b;

					pDC->MoveTo(ptS.x, ptS.y);
					pDC->LineTo(ptE.x, ptE.y);

					ptS.y = line_info[1].a * ptS.x + line_info[1].b;
					ptE.y = line_info[1].a * ptE.x + line_info[1].b;
					pDC->MoveTo(ptS.x, ptS.y);
					pDC->LineTo(ptE.x, ptE.y);
				}
				else
				{
					ptS.x = 0, ptE.x = W;
					ptS.y = m_pMain->notch_line_data.pt_down.y;
					ptE.y = m_pMain->notch_line_data.pt_down.y;

					pDC->MoveTo(ptS.x, ptS.y);
					pDC->LineTo(ptE.x, ptE.y);

					ptS.y = m_pMain->notch_line_data.pt_up.y;
					ptE.y = m_pMain->notch_line_data.pt_up.y;
					pDC->MoveTo(ptS.x, ptS.y);
					pDC->LineTo(ptE.x, ptE.y);
				}

				int pos_x = m_pMain->notch_line_data.pt_mid.x;
				int pos_y = m_pMain->notch_line_data.pt_mid.y;
				pDC->MoveTo(pos_x - 20, pos_y);
				pDC->LineTo(pos_x + 20, pos_y);
				pDC->MoveTo(pos_x, pos_y - 20);
				pDC->LineTo(pos_x, pos_y + 20);

				double rotx = m_pMain->GetMachine(nJob).getRotateX(nCam, 0);
				double roty = m_pMain->GetMachine(nJob).getRotateY(nCam, 0);
				double worldX, worldY;
				m_pMain->GetMachine(nJob).WorldToPixel(nCam, nCam, rotx, roty, &worldX, &worldY);
				pDC->SelectObject(&penOrg);
				//std::vector<cv::Point2f> calibPt = m_pMain->GetMachine(nJob).getPtImage(nCam, 0);
				auto calibPt = m_pMain->GetMachine(nJob).getPtImage(nCam, 0);
				auto _x = W / 2 - calibPt.at(calibPt.size() / 2).x;
				auto _y = H / 2 - calibPt.at(calibPt.size() / 2).y;

				pos_x = int(worldX + _x);	pos_y = int(worldY + _y);

				pDC->MoveTo(pos_x - 20, pos_y);
				pDC->LineTo(pos_x + 20, pos_y);
				pDC->MoveTo(pos_x, pos_y - 20);
				pDC->LineTo(pos_x, pos_y + 20);
			}

			breturn = TRUE;
		}
		else
		{
			CPatternMatching* pPattern = &m_pMain->GetMatching(nJob);
			pPattern->findPattern(img.data, nCam, 0, W, H);
			CFindInfo find = m_pMain->GetMatching(nJob).getFindInfo(nCam, 0);
			double pos_x = 0, pos_y = 0, angle = 0;

			if (find.GetFound() == FIND_OK || find.GetFound() == FIND_MATCH)
			{
				int find_index = find.GetFoundPatternNum();
				double mark_offset_x = m_pMain->GetMatching(nJob).getMarkOffsetX(nCam, 0, find_index);
				double mark_offset_y = m_pMain->GetMatching(nJob).getMarkOffsetY(nCam, 0, find_index);

				pos_x = find.GetXPos() + mark_offset_x;
				pos_y = find.GetYPos() + mark_offset_y;
				angle = find.GetAngle();

				m_pMain->m_ELB_TraceResult.m_ELB_ResultXY = cv::Point2d(pos_x, pos_y);
				m_pMain->m_ELB_TraceResult.m_ELB_ResultT = angle;

				if (mViewer != NULL)
				{
					pDC->MoveTo(pos_x - 20, pos_y);
					pDC->LineTo(pos_x + 20, pos_y);
					pDC->MoveTo(pos_x, pos_y - 20);
					pDC->LineTo(pos_x, pos_y + 20);
				}
			}
			else breturn = FALSE;
		}

		if (mViewer != NULL) mViewer->Invalidate();

		return breturn;
	}

	bool bNotch = bnot;
	bool bCircle = bcir;

	int polarity = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getSearchEdgePolarity();
	int searchdir = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getSearchSearchDir();

	int hthresh = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionHighThresh();
	int lthresh = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionLowThresh();

	cv::Mat s_down, maskbin; // 축소 해서 검사 진행
	cv::resize(img, s_down, cv::Size(), 0.5, 0.5, CV_INTER_CUBIC);

	int _cMaxId = -1, _cMaxV = -99999;

	if (bNotch)
	{
		if (polarity == 0)
		{
			cv::threshold(s_down, bin3, hthresh, 255, CV_THRESH_BINARY);
			cv::GaussianBlur(bin3, bin3, cv::Size(0, 0), 3.);
			cv::threshold(bin3, bin3, lthresh, 255, CV_THRESH_BINARY);
		}
		else
		{
			cv::threshold(s_down, bin3, hthresh, 255, CV_THRESH_BINARY_INV);
			erode(bin3, bin3, Mat::ones(cv::Size(3, 3), CV_8UC1), cv::Point(-1, -1), 1);
			dilate(bin3, bin3, Mat::ones(cv::Size(3, 3), CV_8UC1), cv::Point(-1, -1), 1);
			bitwise_not(bin3, bin3);

			//cv::GaussianBlur(bin3, bin3, cv::Size(0, 0), 3.);
			//cv::threshold(bin3, bin3, lthresh, 255, CV_THRESH_BINARY_INV);
		}
	}
	else
	{
		if (polarity == 0)		cv::threshold(s_down, bin3, hthresh, 255, CV_THRESH_BINARY);
		else 					cv::threshold(s_down, bin3, hthresh, 255, CV_THRESH_BINARY_INV);
	}

	cv::Mat mask = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3), cv::Point(1, 1));
	bool _bremoveNoise = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getMetalSearchRemoveNoiseFlag();
	if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseMetalTraceFirst() == FALSE) _bremoveNoise = true;

	if (bcir && _bremoveNoise)
	{		
		dilate(bin3, bin3, mask, cv::Point(-1, -1), 1);			
		erode(bin3, bin3, mask, cv::Point(-1, -1), 2);			
        //fnRemoveNoiseBlack(bin3, 300);	
	}
	
	cv::Mat maskRect = cv::Mat::zeros(bin3.rows, bin3.cols, CV_8U);

	if (m_pMain->m_ELB_vtMaskContour.size() > 0)
	{
		int num = (int)m_pMain->m_ELB_vtMaskContour.size();
		const cv::Point* pt4 = &(m_pMain->m_ELB_vtMaskContour[0]);
		cv::fillPoly(maskRect, &pt4, &num, 1, cv::Scalar(255, 255, 255), 8);
	}
	else
	{
		maskRect(cv::Rect(ptStart.x / 2, ptStart.y / 2, abs(ptEnd.x - ptStart.x) / 2, abs(ptEnd.y - ptStart.y) / 2)) = 255;
		//maskRect(cv::Rect(bin3.cols / 2 + 250, ptStart.y / 2 + 30, bin3.cols / 2 - 250, bin3.rows - ptStart.y / 2 - 30)) = 0;
	}

	bin3.copyTo(maskbin, maskRect);

	if (bNotch)
	{  // 노이즈 제거용
		cv::bitwise_not(maskbin, bin2, maskRect);
		dilate(bin2, bin2, Mat::ones(cv::Size(3, 3), CV_8UC1)); //20210914 Tkyuha 끊어진것 이어 주기

		cv::findContours(bin2, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
		std::vector<std::vector<cv::Point>>::const_iterator itc = contours.begin();
		//cv::GaussianBlur(maskRect, maskRect, cv::Size(0, 0), 3.);
		erode(maskRect, maskRect, Mat::ones(cv::Size(7, 7), CV_8UC1), cv::Point(-1, -1), 2);

		//큰 블랍 중에서 맨 아래쪽에 있는 노치 라인을 찾음		
		//20210915 Tkyuha 알고리즘 노이즈 제거 추가
		double _xres = MAX(0.00001, m_pMain->GetMachine(nJob).getCameraResolutionX(0, 0));
		double _totalLength = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeLength();
		_totalLength = _totalLength / _xres * 0.75 / 2;

		itc = contours.begin();
		for (int i = 0; i < contours.size(); i++)
		{
			cv::Rect box = boundingRect(*itc);
			if (box.width > _totalLength && box.height > _totalLength / 2)// 컨투어가 도포길이 가로 75% 와 세로 35%이상 되는 후보점 대상
			{
				if (box.y + box.height > _cMaxV)
				{
					_cMaxId = i;
					_cMaxV = box.y + box.height;
				}
			}
			++itc;
		}

		//20210915 Tkyuha 선택 영역만 남기고 나머지는 제거
		itc = contours.begin();
#pragma omp parallel for
		for (int i = 0; i < contours.size(); i++)
		{
			cv::Rect box = boundingRect(*itc);
			if (box.y < ptStart.y / 2 || box.y > ptEnd.y / 2) continue;

			// 잘못 찾을 경우 arcLength 비교 3500=>2000 값을 줄여서 확인 해보세요
			// // 노이즈 제거 목적 
			if (_cMaxId != -1)
			{
				if (i != _cMaxId)		drawContours(maskbin, contours, i, RGB(255, 255, 255), CV_FILLED, 8, hierarchy);
			}
			else if (arcLength(*itc, true) < 2000 || itc->size() < 250 || box.width < 1000)  //itc->size() < 3500
			{
				drawContours(maskbin, contours, i, RGB(255, 255, 255), CV_FILLED, 8, hierarchy);
			}
			++itc;
		}
	}
	else cv::GaussianBlur(maskRect, maskRect, cv::Size(0, 0), 3.);

	cv::resize(maskbin, bin, cv::Size(), 2, 2, CV_INTER_CUBIC);

	std::vector<double> x_series;
	std::vector<double> y_series;
	std::vector<double> destX;
	std::vector<double> destY;

	std::vector<cv::Point2f> ptrPoint;
	BYTE* pBuffer = bin.data;
	BYTE* pMaskBuffer = maskRect.data;
	//imwrite("d:\\NotchProc.jpg", bin);
	if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
	{
		cvShowImage("TraceInsp_Image", bin);
	}

	if (bNotch)
	{
		bool bInterpolation = false;
		double sumavg = 0;
		cv::Mat scharr;
		double mindiff_pre = 0, mindiff_v = 0, line_ax, line_bc;
		int bcstart_y = -1, bcend_y = -1, bcstart_x = -1, bcend_x = -1;
		int _nptPreEnd = ptEnd.y;

		BOOL b = InspNotchLine(img.data, W, H, nJob, nCam);

		sLine line_info[2];
		line_info[0] = m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][0].m_lineInfo;
		line_info[1] = m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][1].m_lineInfo;

		double chcekTopY = line_info[1].a * ptEnd.x + line_info[1].b;
		double chcekTopY2 = line_info[1].a * (W / 2) + line_info[1].b;		//20210915 Tkyuha 오른쪽으로 기울어진 경우 왼쪽 Top 이 벗어남
		double chcekBottomY = line_info[0].a * ptStart.x + line_info[0].b;
		double chcekBottomY2 = line_info[0].a * (W / 2) + line_info[0].b;	//20210915 Tkyuha 오른쪽으로 기울어진 경우 왼쪽 Top 이 벗어남
		BOOL bfind_1 = m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][0].getIsMakeLine();
		BOOL bfind_2 = m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][1].getIsMakeLine();

		if (b && (bfind_1 || bfind_2)) //20210915 Tkyuha 예외 상황처리
		{
			ptStart.y = MAX(0, MIN(chcekTopY2, chcekTopY) - 24); //20210914 Tkyuha 위쪽 영역은 확장 해도 상관 없음 => 틀어짐에 대한 대응
			if (_cMaxId != -1) ptEnd.y = MIN(H - 1, MAX(chcekBottomY2, chcekBottomY) + 24); // 20210915 Tkyuha 모든게 제거 되어서 확장해도 됨
			else ptEnd.y = MIN(H - 1, chcekBottomY + 4);
		}

		for (int x = ptStart.x; x < ptEnd.x; x++) //정방향 사용
		{
			if (searchdir == 0)
			{
				for (int y = ptStart.y; y < ptEnd.y; y++) //정방향 사용
				{
					if (x < 0 || x >= W || y < 0 || y >= H) continue;
					if (pMaskBuffer[y / 2 * (W / 2) + (x / 2)] != 255) continue;
					if (pBuffer[y * W + x] != 255)
					{
						cv::Point2f pt = cv::Point2f(x, y);
						float xsc = x, ysc = y;
						int xi = 0, yi = 0;

						if (mindiff_pre > 0)
						{
							if (fabs(mindiff_pre - y) > 50.)
							{
								if (bcstart_x == -1)
								{
									bcstart_y = mindiff_pre;		bcstart_x = x - 1;
								}
								else
								{
									if (bcend_y != -1 && y > bcend_y) bInterpolation = true;
									bcend_y = y;	bcend_x = x;
								}
							}
							else
							{
								mindiff_v += fabs(mindiff_pre - y);
								mindiff_v /= 2.;
							}
						}

						mindiff_pre = y;
#ifdef _SUBPIXEL_MODE
						float img3x3[3][3];
						for (int n = y - 1; n < y + 1; n++)
						{
							for (int m = x - 1; m < x + 1; m++)
							{
								img3x3[xi++][yi] = img.data[n * W + m];
							}
							yi++;
							xi = 0;
						}
						calcSubPixelCenter(img3x3, &xsc, &ysc, 10);
#else
						x_series.clear();
						y_series.clear();
						destX.clear();
						destY.clear();

						if (y > 3)
						{
							for (int j = y - 3; j < y + 3; j++)
							{
								x_series.push_back(j);
								y_series.push_back(img.data[j * W + x]);
							}

							monotonic_cubic_Hermite_spline(x_series, y_series, &destX, &destY);

							ysc = 0;

							cv::Scharr(destY, scharr, CV_64F, 1, 0);

							double* matData = (double*)scharr.data;

							for (int m = 0; m < destY.size(); m++)
							{
								if (matData[m] > ysc)
								{
									ysc = matData[m];
									yi = m;
								}
							}

							ysc = destX[yi];
						}
#endif
						pt = cv::Point2f(xsc, ysc);
						ptrPoint.push_back(pt);
						break;
					}
				}

				if ((bInterpolation || ptEnd.x - 1 == x) && bcstart_y != -1 && bcend_y != -1)
				{
					GetLineCoef(bcstart_x, bcstart_y, bcend_x, bcend_y, line_ax, line_bc);

					for (int ai = bcstart_x; ai <= bcend_x; ai++)
					{
						if (ptrPoint.size() <= (ai - ptStart.x)) continue;
						ptrPoint.at(ai - ptStart.x).y = line_ax * ai + line_bc;
					}

					bcstart_y = -1;
					bcend_y = -1;
					bInterpolation = false;
				}
			}
			else  // 아래에서 위쪽으로 찾는 방식 => 현재 사용 중
			{
				bool bsearch_s = false;
				bool bsearch_e = false;
				int bsearch_count = 0;
				int _nptEnd = ptEnd.y;
				int _nInd = int(ptrPoint.size());  //2021 Tkyuha 임시변수

				// 초기 20포인트 직선의 평균을 이용 해서 다음 포인트를 미리 예측 하기 위해 20개 평균을 계산
				if (_cMaxId == -1 && b && _nInd >= 1) //20210915 Tkyuha 센터라인을 찾은 경우만 하도록 예외처리
				{
					if (_nInd > 20)
					{
						sumavg += ptrPoint.at(_nInd - 1).y;
						sumavg /= 2.;
						if (_nptPreEnd > sumavg + 1) sumavg -= (fabs(_nptPreEnd - sumavg) + 2);

						//20210914 Tkyuha 아래 내용 변경
						_nptEnd = int(sumavg + 1); //_nptEnd =  int(sumavg + 2);
						_nptPreEnd = _nptEnd;
					}
					else if (_nInd == 20)
					{
						sumavg = sumavg / 19.;
						_nptPreEnd = int(sumavg);
					}
					else sumavg += ptrPoint.at(_nInd - 1).y;
				}

				for (int y = _nptEnd; y > ptStart.y; y--) //역방향 사용
				{
					if (x < 0 || x >= W || y < 0 || y >= H)continue;

					if (pMaskBuffer[y / 2 * (W / 2) + (x / 2)] == 0) continue;  // 마스크 영역 에서만 찾기 위함
					if (searchdir == 2) // 찾는 위치가 흰색 일경우 처리 현재는 검은색을 찾는 방식임
					{
						if (!bsearch_s && pBuffer[y * W + x] != 255)
						{
							if (bsearch_count++ > 3)		bsearch_s = true;
						}
						else if (bsearch_s && pBuffer[y * W + x] == 255)
							bsearch_e = true;
					}
					else if (pBuffer[y * W + x] != 255)  bsearch_e = true; //여기서 항상 체크

					if (bsearch_e)
					{
						cv::Point2f pt = cv::Point2f(x, y);
						float xsc = x, ysc = y;
						int xi = 0, yi = 0;

						if (mindiff_pre > 0)
						{
							// 중간에 끊기는 경우 직선 회귀 분석을 하기 위해서 30 픽셀 이상 이면 못 찾은 걸로 간주 하고
							// 다음 포인트에서 찾으면 직선을 구해서 중간을 채워줌
							if (fabs(mindiff_pre - y) > 30.)
							{
								if (bcstart_x == -1)
								{
									bcstart_y = mindiff_pre;
									bcstart_x = x - 1;
								}
								else
								{
									if (y > bcend_y) bInterpolation = true;
									bcend_y = y;
									bcend_x = x;
								}
							}
							else
							{
								mindiff_v += fabs(mindiff_pre - y);
								mindiff_v /= 2.;
							}
						}

						mindiff_pre = y;

						x_series.clear();
						y_series.clear();
						destX.clear();
						destY.clear();

						/*if (y > 3)
						{
							for (int j = y - 3; j < y + 3; j++)
							{
								x_series.push_back(j);
								y_series.push_back(img.data[j * W + x]);
							}
							monotonic_cubic_Hermite_spline(x_series, y_series, &destX, &destY);

							ysc = 0;
							cv::Scharr(destY, scharr, CV_64F, 1, 0);

							double* matData = (double*)scharr.data;
							for (int m = 0; m < destY.size(); m++)
							{
								if (matData[m] > ysc)
								{
									ysc = matData[m];
									yi = m;
								}
							}

							ysc = destX[yi];
						}*/
						pt = cv::Point2f(xsc, ysc);

						ptrPoint.push_back(pt);
						break;
					}
				}

				// 중간에 끊기는 경우 직선 회귀 분석을 하기 위해서 30 픽셀 이상 이면 못 찾은 걸로 간주 하고
				// 다음 포인트에서 찾으면 직선을 구해서 중간을 채워줌
				if ((bInterpolation || ptEnd.x - 1 == x) && bcstart_y != -1 && bcend_y != -1)
				{
					GetLineCoef(bcstart_x, bcstart_y, bcend_x, bcend_y, line_ax, line_bc);
					for (int ai = bcstart_x; ai <= bcend_x; ai++)
					{
						if (ptrPoint.size() <= (ai - ptStart.x)) continue;
						ptrPoint.at(ai - ptStart.x).y = line_ax * ai + line_bc;
					}
					bcstart_y = -1;
					bcend_y = -1;
					bInterpolation = false;
				}
			}
		}

		//////////////////// 도포 된것 찾기
		m_dCircleInBright = 255;
		if (ptrPoint.size() > 20)
		{
			double cx, cy, rad = 30;
			double t_mean = 0;

			cx = ptrPoint.at(20).x;
			cy = ptrPoint.at(20).y - rad;

			t_mean += (cv::mean(img(cv::Rect(cx - rad, cy - rad, 30, 30)))).val[0]; //left				
			t_mean += (cv::mean(img(cv::Rect(cx, cy - rad, 30, 30)))).val[0];      //right

			m_dCircleInBright = t_mean / 2.;
			if (mViewer != NULL)
			{
				pDC->Rectangle(CRect(cx - rad, cy - rad, cx - rad + 30, cy - rad + 30));
				pDC->Rectangle(CRect(cx, cy - rad, cx + 30, cy - rad + 30));
			}
		}
	}
	else
	{
		uchar* Raw = bin.data;
		int colC = bin.cols / 2, rowC = bin.rows / 2;
		int rposx, rposy,_noiseR = 0;
		std::vector<cv::Point2f> edgePositions;
		CPointF<double> p_center(colC, rowC);
		double x, y;
		bool _bcircleEdgeFind = false, _bFirstFind = false, _bMetalSearch = false;

		double xres = MAX(0.00001, m_pMain->GetMachine(nJob).getCameraResolutionX(nCam, 0));
		double radius = MAX(6., m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeLength()) / xres; //도포 길이
		double cirsize = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspInHoleSize() / xres;

		if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseMetalTraceFirst()) // Tkyuha 20211216 메탈로 얼라인 하기 위함
			_bMetalSearch = true;
		//KJH 2022-01-25 ELB Cover CInk mode
		bool bELBCoverMode = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getELBCoverCInkMode();
		// 원을 360 등분 해서 찾기
		for (int second = 0; second < 360; second++)
		{
			_bcircleEdgeFind = true;
			_bFirstFind = false;
			_noiseR = 0;
			// 원위의 점 찾기
			for (int distanceT = 0; distanceT < 500; distanceT++)
			{
				int _rad = cirsize + distanceT;

				rposx = (cos(ToRadian(second)) * _rad + colC);
				rposy = (sin(ToRadian(second)) * _rad + rowC);

				if (rposx < 0 || rposx >= colC * 2) continue;
				if (rposy < 0 || rposy >= rowC * 2) continue;

				CPointF<int> p2(rposx, rposy);
				if (pMaskBuffer[rposy / 2 * (W / 2) + (rposx / 2)] != 255) continue;

				if (_bMetalSearch) // Tkyuha 20211216 메탈을 이용해서 도포하기 위함
				{
					if (!bELBCoverMode)
					{
						if (_bFirstFind && Raw[rposy * bin.cols + rposx] == 255) 
						{
							edgePositions.push_back(cv::Point2f(rposx, rposy));
							_bcircleEdgeFind = false;
							break;
						}
						if (!_bFirstFind && Raw[rposy * bin.cols + rposx] != 255)
						{
							_noiseR++;
							if(_noiseR > 3)		_bFirstFind = true; //_noiseR>3 톱니같이 생긴 엣지 예외처리
						}
					}
					else
					{
						//CoverMode
						if (Raw[rposy * bin.cols + rposx] != 255)
						{
							edgePositions.push_back(cv::Point2f(rposx, rposy));
							_bcircleEdgeFind = false;
							break;
						}
					}
				}
				else
				{		
					if (_bFirstFind && Raw[rposy * bin.cols + rposx] != 255)
					{
						edgePositions.push_back(cv::Point2f(rposx, rposy));
						_bcircleEdgeFind = false;
						break;
					}
					if (!_bFirstFind && Raw[rposy * bin.cols + rposx] == 255) _bFirstFind = true;
				}				
			}
			if (_bcircleEdgeFind)  // Edge를 못찾는 경우 가상의 데이타 추가
			{
				edgePositions.push_back(cv::Point2f(-1, -1)); // 20211101 Tkyuha 원의 Edge를 못찾는 경우 추가
			}
		}

		
	// 20210909 Tkyuha 포인트 선택 영역 계산 8등분 해서 면적이 넓은 영역 6등분 선택
	// 메탈과 글라스 사이 검은 영역 면적 계산

		int _start = 0;
		int _end = int(edgePositions.size());
		bool _bsplit = false; // 0도 기준으로 양쪽 으로 영역이 나뉜경우 체크

	   //20210923 Tkyuha Circle에서만 사용 하는 파라미터 
	   //원을 찾을때 제외 영역을 메탈 찾는 경우에 적용 하기 위함 초기화
		m_Index_end = m_Index_start = -1;
		m_Index_bsplit = false;
		m_IndexMaxWidth_Angle = 0;

		if (edgePositions.size() > 10)
		{
			int _divideCheckCount = 8;
			
			//HTK 2022-03-21 MP,PN 간격이 가장 큰 각도 계산하여 일부분만 도포하는 모드를 위한 계산
			if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getELBPartial_DispensingModeEnable())
				_divideCheckCount = 36;

			int _step = int(edgePositions.size() / _divideCheckCount);

			std::vector<pair<int, int>> _v;
			bool _bfind = false;

			for (int i = 0; i < edgePositions.size(); i += _step)
			{
				if (edgePositions[i].x == -1 && edgePositions[i].y == -1)
				{
					_v.push_back(pair<int, int>(0, i));
					continue;
				}

				CPointF<int> _p(edgePositions[i].x, edgePositions[i].y);				
				int _Srad = (int)GetDistance(p_center, _p);
				int _sign = _bMetalSearch?-1:1;
				int _dcount = 0;
				if (m_pMain->m_ELB_TraceResult.m_nRadius > 0)
				{	
					_Srad = m_pMain->m_ELB_TraceResult.m_nRadius;
					_p.x = cos(ToRadian(i)) * _Srad + m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x;
					_p.y = sin(ToRadian(i)) * _Srad + m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y;
				}
				_bfind = false;

				for (int distanceT = 5; distanceT < 150; distanceT++) // 반지름 크기가 5픽셀 크기 부터 150 픽셀만 검사 
				{
					int _rad = _Srad + distanceT* _sign;  //반지름 계산

					rposx = (cos(ToRadian(i)) * _rad + colC);  // 회전 중심에 의한 좌표
					rposy = (sin(ToRadian(i)) * _rad + rowC);

					if (rposx < 0 || rposx >= colC * 2) continue;
					if (rposy < 0 || rposy >= rowC * 2) continue;
					CPointF<int> p2(rposx, rposy);
					if (Raw[rposy * bin.cols + rposx] == 255)
					{
						_dcount++;
						int _S = (int)GetDistance(_p, p2);
						if (_dcount > 5)
						{
							_v.push_back(pair<int, int>(_S, i));
							_bfind = true;
							break;
						}
					}
				}

				if (_bfind == false) _v.push_back(pair<int, int>(0, i));
			}

			sort(_v.begin(), _v.end());

			if (_v.size() > _divideCheckCount-1)  // Tkyuha 범위 설정 수정 20220103
			{
				int max_index = _v[_v.size() - 1].second;
				int _revID = (max_index + 180) % 360;
				
				_end = (_revID+45)%360;
				_start = (_revID-45+360)%360;

				if(abs(_end-_start)>180 && _start!=0 && _end!=0)
				{
					_start = 0;
					_end = 45;
					_bsplit = true;
				}

				m_IndexMaxWidth_Angle = _revID;
			}
			else  _end = 0;

			_v.clear();
		}
		
		///////// <>
		if (edgePositions.size() > 10)  // 10개 이상 포인트를 찾은 경우만 검사 진행
		{
			int sCount = 0, margin = 5, _realCount = 0;
			double cx = 0, cy = 0, rad = 0;
			double  avgdistance = 0;
			double* XiData = new double[edgePositions.size()];
			double* YiData = new double[edgePositions.size()];
			double* DistData = new double[edgePositions.size()];

			//outlier 제거
			for (int i = 0; i < edgePositions.size() - 1; i++) // 마지막 하나는 빼고 수행 ,예외처리함
			{
				CPointF<int> p2(edgePositions[i].x, edgePositions[i].y);
				CPointF<int> p1(edgePositions[i + 1].x, edgePositions[i + 1].y);

				if ((p2.x == -1 && p2.y == -1) || p1.x == -1 && p1.y == -1) continue;

				XiData[_realCount] = GetDistance(p1, p2);
				DistData[_realCount] = XiData[_realCount];
				_realCount++;
			}

			std::sort(XiData, XiData + _realCount);
			avgdistance = XiData[_realCount / 2];  // 중앙값을 선택

			int _continueC = 0;
			for (int i = 0; i < _realCount; i++)
			{
				if ((_start<i && _end>i) || (_bsplit && i > 315)) continue;

				if (DistData[i] > (avgdistance - margin) && DistData[i] < (avgdistance + margin))
				{
					_continueC++;

					if (_continueC >= 5 && edgePositions[i].x!=-1 && edgePositions[i].y!=-1)
					{
						XiData[sCount] = edgePositions[i].x;
						YiData[sCount] = edgePositions[i].y;
						sCount++;
					}
				}
				else _continueC = 0;
			}

			if (sCount > 10)
			{
				if (mViewer != NULL && m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())//디버깅용으로 
				{
					auto pts = new GraphicPoints(0xff00ffff, 5, 2);
					for (int i = 0; i < sCount; i++)
						pts->AddPoint(Gdiplus::PointF(XiData[i], YiData[i]));
					mViewer->AddSoftGraphic(pts);
				}

				std::vector<cv::Point> pts;
				for (int i = 0; i < sCount; i++) {
					cv::Point pt = cv::Point(XiData[i], YiData[i]);
					pts.push_back(pt);
				}
				MLSAC_CircleFitting(pts, cx, cy, rad);
				pts.clear();

				//CircleFit_LS(sCount, XiData, YiData, &cx, &cy, &rad);
				m_Index_end = _end;
				m_Index_start = _start;
				m_Index_bsplit = _bsplit;

				if (_start < edgePositions.size() && _end < edgePositions.size())
				{
					pDC->SelectObject(&penROI);

					if (edgePositions[_start].x == -1 && edgePositions[_start].y == -1)
					{
						x = cos(ToRadian(_start)) * rad + cx;
						y = sin(ToRadian(_start)) * rad + cy;
						pDC->MoveTo(int(x), int(y));
					}
					else pDC->MoveTo(int(edgePositions[_start].x), int(edgePositions[_start].y));

					if (edgePositions[_end].x == -1 && edgePositions[_end].y == -1)
					{
						x = cos(ToRadian(_end)) * rad + cx;
						y = sin(ToRadian(_end)) * rad + cy;
						pDC->LineTo(int(x), int(y));
					}
					else pDC->LineTo(int(edgePositions[_end].x), int(edgePositions[_end].y));
				}
			}

			delete [] XiData;
			delete [] YiData;
			delete [] DistData;

			//20211007 Tkyuha 원의 최외곽을 따라서 도포 하는 옵션 살리기 위해
			if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseDummyCornerInsp() ) 
			{
				uchar* iprocRaw = img.data;
				int _id = 0;

				for (int i = 0; i < edgePositions.size(); i++)
				{
					cv::Point2f pt = cv::Point2f(-1,-1);
					x = cos(ToRadian(i)) * rad + cx;
					y = sin(ToRadian(i)) * rad + cy;
					_id = 0;

					for (int _j = -20; _j < 20; _j++)
					{
						rposx = lround(cos(ToRadian(i)) * _j + x);  // 회전 중심에 의한 좌표
						rposy = lround(sin(ToRadian(i)) * _j + y);

						if (rposx < 0 || rposx >= img.cols) continue;
						if (rposy < 0 || rposy >= img.rows) continue;

						if (iprocRaw[rposy * img.cols + rposx] > hthresh) _id++;
						if(_id>3)
						{

							pt.x = lround(cos(ToRadian(i)) *( _j-6) + x);  // 회전 중심에 의한 좌표
							pt.y = lround(sin(ToRadian(i)) *( _j-6) + y);
							break;
						}
					}

//#pragma	 region 	예외영역 추가	//20211007 Tkyuha 
					if (pt.x == -1 || pt.y == -1)
					{
						pt = cv::Point2f(x, y);
					}
//#pragma endregion
					ptrPoint.push_back(pt);
				}

				std::vector<cv::Point> pts;
				for (int i = 0; i < ptrPoint.size(); i++) {
					cv::Point pt = cv::Point(ptrPoint[i].x, ptrPoint[i].y);
					pts.push_back(pt);
				}
				MLSAC_CircleFitting(pts, cx, cy, rad);
				pts.clear();
			}
			else
			{
				for (int i = 0; i < 360; i++)
				{
					x = cos(ToRadian(i)) * rad + cx;
					y = sin(ToRadian(i)) * rad + cy;
					cv::Point2f pt = cv::Point2f(x, y);
					ptrPoint.push_back(pt);
				}
			}

			if (mViewer != NULL)
			{
				pDC->Ellipse(int(cx - 2), int(cy - 2), int(cx + 2), int(cy + 2));

				pDC->SelectObject(&penFit);
				pDC->SelectObject(GetStockObject(NULL_BRUSH));

				//pDC->Ellipse(int(cx - rad), int(cy - rad), int(cx + rad), int(cy + rad));
				//mViewer->AddSoftGraphic(new GraphicCircle(int(cx - rad), int(cy - rad), int(cx + rad), int(cy + rad), COLOR_RED, 5.0));
			}

			CIRCLE_RADIUS = rad;
			//KJH 2022-04-28 PN 반지름 계산 누락 추가
			m_pMain->m_ELB_TraceResult.m_nRadius = rad;
			m_pMain->m_ELB_TraceResult.m_ELB_ResultXY = cv::Point2d(cx, cy);

			if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseMetalTraceFirst())
			{
				m_dCircleRadius_MP = m_pMain->m_ELB_TraceResult.m_nRadius;
				m_ptCircleCenter_MP.x = (int)m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x;
				m_ptCircleCenter_MP.y = (int)m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y;
			}

			m_dCircleInBright = 255;
			for (int i = 0; i < 4; i++)		m_dCircleInBrightDir[i] = 255;

			//  도포 되서 들어오는 패널 찾기 위한 예외 처리 루틴 
			// 4 방향,3시,6시,9시,12시 방향 gray 값 비교
			// KBJ 2022-08-16 Lami Error 판단하는 범위 수정.
			int InspRect_Width = 40;
			int InspRect_WW = int(InspRect_Width/2);
			int InspRect_Height = 20;
			int InspRect_HH = int(InspRect_Height/2);

			cv::Rect rect[4];
			if (cx - rad > 0 && cx + rad - InspRect_Width > 0 && cx - InspRect_HH >0 &&
				cy - InspRect_HH >0 && cy - rad>0 && cy + rad - InspRect_Width >0)
			{
				double t_mean = 0;
				rect[0] = cv::Rect(cx - rad + InspRect_WW, cy, InspRect_Width, InspRect_Height);	//9
				rect[1] = cv::Rect(cx + rad - InspRect_WW, cy, InspRect_Width, InspRect_Height);	//3
				rect[2] = cv::Rect(cx, cy - rad + InspRect_WW, InspRect_Height, InspRect_Width);	//12
				rect[3] = cv::Rect(cx, cy + rad - InspRect_WW, InspRect_Height, InspRect_Width);	//6

				t_mean += (cv::mean(img(rect[0]))).val[0];		//left
				m_dCircleInBrightDir[0] = t_mean;
				t_mean += (cv::mean(img(rect[1]))).val[0];      //right
				m_dCircleInBrightDir[1] = t_mean - m_dCircleInBrightDir[0];
				t_mean += (cv::mean(img(rect[2]))).val[0];		//top
				m_dCircleInBrightDir[2] = t_mean - m_dCircleInBrightDir[1];
				t_mean += (cv::mean(img(rect[3]))).val[0];		//bottom
				m_dCircleInBrightDir[3] = t_mean - m_dCircleInBrightDir[2];

				m_dCircleInBright = t_mean / 4.;
				if (mViewer != NULL)
				{
					pDC->Rectangle(CRect(rect[0].x - (int)(rect[0].width  / 2),	rect[0].y - (int)(rect[0].height / 2), rect[0].x + (int)(rect[0].width / 2),	rect[0].y + (int)(rect[0].height / 2)));		// 3
					pDC->Rectangle(CRect(rect[1].x - (int)(rect[1].width  / 2), rect[1].y - (int)(rect[1].height / 2), rect[1].x + (int)(rect[1].width / 2),	rect[1].y + (int)(rect[1].height / 2)));		// 3
					pDC->Rectangle(CRect(rect[2].x - (int)(rect[2].width  / 2),	rect[2].y - (int)(rect[2].height / 2), rect[2].x + (int)(rect[2].width / 2),	rect[2].y + (int)(rect[2].height / 2)));		// 9
					pDC->Rectangle(CRect(rect[3].x - (int)(rect[3].width  / 2), rect[3].y - (int)(rect[3].height / 2), rect[3].x + (int)(rect[3].width / 2),	rect[3].y + (int)(rect[3].height / 2)));		// 3
				}
			}
		}

		edgePositions.clear();
	}

	if (ptrPoint.size() > 10)
	{
		x_series.clear();
		y_series.clear();
		destX.clear();
		destY.clear();

		for (int j = 0; j < ptrPoint.size(); j++)
		{
			x_series.push_back(j);
			y_series.push_back(ptrPoint[j].y);
		}

		if (bNotch) destY = sg_smooth(y_series, 3, 1);// i'm only saw smoothng for Y not x is it correct??
		else Savitzky_Golay_smoothing(&x_series, &y_series, &destX, &destY);

		for (int i = 0; i < destY.size(); i++)
			ptrPoint[i].y = lround(destY[i]);

		x_series.clear();
		y_series.clear();
		destX.clear();
		destY.clear();

		if (mViewer != NULL)
		{
			pDC->SelectObject(&penROI);

			if (ptrPoint.size() > 0)
			{
				pDC->MoveTo(int(ptrPoint[0].x), int(ptrPoint[0].y));

				for (int i = 1; i < ptrPoint.size(); i++)
				{
					pDC->LineTo(int(ptrPoint[i].x), int(ptrPoint[i].y));
				}
			}
		}

		if (ptrPoint.size() > 0)
		{
			m_pMain->m_ELB_vtMaskRawContour.clear();
			m_pMain->m_ELB_vtMaskRawContour.assign(ptrPoint.begin(), ptrPoint.end());
		}

		/*
		if (bCircle)
		{
			double* XiData = new double[ptrPoint.size()];
			double* YiData = new double[ptrPoint.size()];

			for (int i = 0; i < ptrPoint.size(); i++)
			{
				XiData[i] = ptrPoint[i].x;
				YiData[i] = ptrPoint[i].y;
			}

			CircleData data1(int(ptrPoint.size()), XiData, YiData);
			CircleFit FitCircle = CircleFitByHyper(data1);

			if (mViewer != NULL)
			{
				pDC->Ellipse(int(FitCircle.a - 2), int(FitCircle.b - 2), int(FitCircle.a + 2), int(FitCircle.b + 2));

				pDC->SelectObject(&penFit);
				pDC->SelectObject(GetStockObject(NULL_BRUSH));

				pDC->Ellipse(int(FitCircle.a - FitCircle.r), int(FitCircle.b - FitCircle.r), int(FitCircle.a + FitCircle.r), int(FitCircle.b + FitCircle.r));
			}
			delete XiData;
			delete YiData;
		}		
		*/
		// 디버깅 모드에서 이미지 보기
		if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
		{
			cvShowImage("Inspection Image", bin);
		}

		ResetEvent(m_pMain->m_hMetalSearchEvent);

		InspNotchEdgeDistanceDivide(bNotch, bCircle, nJob, &ptrPoint, ptEnd.x, W, H, mViewer);				

		//HTK 2022-03-30 Tact Time Display 추가
		m_clockProcStartTime[1] = clock();
		
		bool bDustReturn = true;

		if (bCircle)
		{
			m_thInspParam.nCam = nCam;
			m_thInspParam.nJob = nJob;
			m_thInspParam.bEvent = false;
			m_thInspParam.ptStart = ptStart;
			m_thInspParam.ptEnd = ptEnd;
			m_thInspParam.pDlg = m_pMain;
			m_thInspParam.mViewer = mViewer;
			
			if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseMetalTraceFirst() == FALSE) // Tkyuha 20211216 메탈로 얼라인 하기 위함
			{				
				AfxBeginThread(Thread_SubInspection, &m_thInspParam, THREAD_PRIORITY_HIGHEST); //Metal Edge 검사
				AfxBeginThread(Thread_SubBMInspection, &m_thInspParam); //BM Edge 검사

				if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getUseInputInspection()) // 20220110 이물검사를 Metal Edge 검사와 동시에 수행	
					bDustReturn = fnPreDustInspection(nJob, 0, img.data, mViewer);
			}
			else
			{
				m_thInspParam.bEvent = true;
				AfxBeginThread(Thread_SubBMInspection, &m_thInspParam); //BM Edge 검사
				
				//HTK 2022-06-24 CInk2에서 CInk1에서 발생한 Lack of Ink 사전 검사 후 최종 합산
				fnCInk2_LackofInkInspection(nJob, 0, img.data, mViewer); 

				if (mViewer != NULL)  //20220413 Tkyuha MP 결과 표시
				{
					CString str_data, str_data1;
					//CFont font, * oldFont;
					//font.CreateFont(40, 40, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
					//	OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
					//oldFont = pDC->SelectObject(&font);

					double _xres = m_pMain->GetMachine(nJob).getCameraResolutionX(0, 0);
					CPointF<double> p_center2(W / 2, H / 2);
					CPointF<double> _center(m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x, m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y);
					double m_dCircleDistance_X_CC = GetDistance(_center, p_center2) * _xres;

					//pDC->SetTextColor(COLOR_BLUE);
					str_data.Format("[Center GAP] : %.3fmm", m_dCircleDistance_X_CC);
					//pDC->TextOutA(int(m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x + 10), int(m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y + 100), str_data);
					str_data1.Format("[MP Radius]  : %.3fmm", CIRCLE_RADIUS * _xres);
					//pDC->TextOutA(int(m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x + 10), int(m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y + 000), str_data);
					
					// KBJ 2022-11-25 Bule -> Green
					//mViewer->AddSoftGraphic(new GraphicLabel(m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x, m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y, fmt("%s\n%s", str_data, str_data1), Gdiplus::Color::Blue));
					mViewer->AddSoftGraphic(new GraphicLabel(m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x, m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y, fmt("%s\n%s", str_data, str_data1), Gdiplus::Color(0xff00ff00)));

					//font.DeleteObject();
				}
			}

			::WaitForSingleObject(m_pMain->m_hMetalSearchEvent, 1000);
		}

		//HTK 2022-03-30 Tact Time Display 추가
		m_clockProc_elapsed_time[1] = (clock() - m_clockProcStartTime[1]) / (float)CLOCKS_PER_SEC;
		((CPaneAuto*)m_pMain->m_pPane[PANE_AUTO])->update_grid_processing_time(1, m_clockProc_elapsed_time[1], 1, bDustReturn);

		//KJH 2022-07-27 Tact Time Log 추가
		theLog.logmsg(LOG_PLC_SEQ, "Pre Insp Tact Time = %.4f", m_clockProc_elapsed_time[1]);
	}

	if (mViewer != NULL) mViewer->Invalidate();

	penROI.DeleteObject();
	penFit.DeleteObject();
	penOrg.DeleteObject();

	CString str;

	if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getUseInputInspection())
	{
		if (m_vecDustResult.size() > 0 && theApp.m_bEnforceOkFlag == FALSE)
		{
			breturn = FALSE;

			// JSY 2022-11-05 이물검사 결과 추가 Dust Insp
			m_bDustInspJudge = FALSE;

			str.Format("Input Inspection Dust Count = %d NG", m_vecDustResult.size());
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
			ShowErrorMessageBlink(str);
		}
		else m_bDustInspJudge = TRUE;
	}

	bin.release();
	bin2.release();
	bin3.release();
	img.release();
	s_down.release();
	maskbin.release();
	maskRect.release();

	contours.clear();
	hierarchy.clear();

	system_clock::time_point end = system_clock::now();
	nanoseconds nano = end - start;

	str.Format("Divide time: %f", nano.count() / 1000000.);
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_OPEN_IMAGE_FILENAME, (LPARAM)&str);

	return breturn;
}
BOOL CFormMainView::InspNotchEdgeDetection_jmLee(bool bnot, bool bcir, int nJob, int nCam, CPoint ptStart, CPoint ptEnd, CViewerEx* mViewer, bool balign)
{
	CString str5;
	str5.Format("[Trace] Find ROI : %d, %d, %d, %d", ptStart.x, ptStart.y, ptEnd.x, ptEnd.y);
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str5);

	system_clock::time_point start = system_clock::now();

	FakeDC* pDC = NULL;
	CPen penROI(PS_SOLID, 3, RGB(255, 0, 0));
	CPen penFit(PS_SOLID, 3, RGB(0, 255, 255));
	CPen penOrg(PS_SOLID, 3, RGB(0, 0, 255));

	if (mViewer != NULL)
	{
		if (bcir)	mViewer->ClearOverlayDC();

		pDC = mViewer->getOverlayDC();
		pDC->SelectObject(&penROI);
	}

	int W = m_pMain->m_stCamInfo[nCam].w;
	int H = m_pMain->m_stCamInfo[nCam].h;
	int cx = W / 2;
	int cy = H / 2;

	//KJH 2022-08-04 Trace 버퍼 변경
	cv::Mat img(H, W, CV_8UC1, m_pMain->getProcBuffer(nCam, 2));
	std::vector<cv::Point2f> ptrPoint;

	BOOL breturn = TRUE;
	double x, y, rad = CIRCLE_RADIUS;;

	{
		m_Index_end = m_Index_start = -1;
		m_Index_bsplit = false;
		m_IndexMaxWidth_Angle = 0;

		for (int i = 0; i < 360; i++)
		{
			x = cos(ToRadian(i)) * rad + cx;
			y = sin(ToRadian(i)) * rad + cy;
			cv::Point2f pt = cv::Point2f(x, y);
			ptrPoint.push_back(pt);
		}

		if (mViewer != NULL)
		{
			pDC->Ellipse(int(cx - 2), int(cy - 2), int(cx + 2), int(cy + 2));

			pDC->SelectObject(&penFit);
			pDC->SelectObject(GetStockObject(NULL_BRUSH));

			pDC->Ellipse(int(cx - rad), int(cy - rad), int(cx + rad), int(cy + rad));
		}

		m_dCircleInBright = 255;
		for (int i = 0; i < 4; i++)		m_dCircleInBrightDir[i] = 255;

		//  도포 되서 들어오는 패널 찾기 위한 예외 처리 루틴 
		// 4 방향,3시,6시,9시,12시 방향 gray 값 비교
		if (cx - rad > 0 && cx + rad - 20 > 0 && cx - 2 > 0 &&
			cy - 2 > 0 && cy - rad > 0 && cy + rad - 20 > 0)
		{
			double t_mean = 0;
			t_mean += (cv::mean(img(cv::Rect(cx - rad, cy - 2, 20, 4)))).val[0];		//left	
			m_dCircleInBrightDir[0] = t_mean;
			t_mean += (cv::mean(img(cv::Rect(cx + rad - 20, cy - 2, 20, 4)))).val[0];     //right
			m_dCircleInBrightDir[1] = t_mean - m_dCircleInBrightDir[0];
			t_mean += (cv::mean(img(cv::Rect(cx - 2, cy - rad, 4, 20)))).val[0];		//top
			m_dCircleInBrightDir[2] = t_mean - m_dCircleInBrightDir[1];
			t_mean += (cv::mean(img(cv::Rect(cx - 2, cy + rad - 20, 4, 20)))).val[0];		//bottom
			m_dCircleInBrightDir[3] = t_mean - m_dCircleInBrightDir[2];

			m_dCircleInBright = t_mean / 4.;
			if (mViewer != NULL)
			{
				pDC->Rectangle(CRect(cx - rad, cy - 2, cx - rad + 20, cy + 2));
				pDC->Rectangle(CRect(cx + rad - 20, cy - 2, cx + rad, cy + 2));
				pDC->Rectangle(CRect(cx - 2, cy - rad, cx + 2, cy - rad + 20));
				pDC->Rectangle(CRect(cx - 2, cy + rad - 20, cx + 2, cy + rad));
			}
		}

	}

	if (ptrPoint.size() > 10)
	{
		if (mViewer != NULL)
		{
			pDC->SelectObject(&penROI);

			if (ptrPoint.size() > 0)
			{
				pDC->MoveTo(int(ptrPoint[0].x), int(ptrPoint[0].y));

				for (int i = 1; i < ptrPoint.size(); i++)
				{
					pDC->LineTo(int(ptrPoint[i].x), int(ptrPoint[i].y));
				}
			}
		}

		if (ptrPoint.size() > 0)
		{
			m_pMain->m_ELB_vtMaskRawContour.clear();
			m_pMain->m_ELB_vtMaskRawContour.assign(ptrPoint.begin(), ptrPoint.end());
		}

		ResetEvent(m_pMain->m_hMetalSearchEvent);

		m_thInspParam.nCam = nCam;
		m_thInspParam.nJob = nJob;
		m_thInspParam.ptStart = ptStart;
		m_thInspParam.ptEnd = ptEnd;
		m_thInspParam.pDlg = m_pMain;
		m_thInspParam.mViewer = mViewer;
		AfxBeginThread(Thread_SubInspection, &m_thInspParam, THREAD_PRIORITY_HIGHEST); //Metal,CC Edge 검사

		InspNotchEdgeDistanceDivide(bnot, bcir, nJob, &ptrPoint, ptEnd.x, W, H, mViewer);

		if (bcir)	::WaitForSingleObject(m_pMain->m_hMetalSearchEvent, 1000);
	}

	if (mViewer != NULL) mViewer->Invalidate();

	penROI.DeleteObject();
	penFit.DeleteObject();
	penOrg.DeleteObject();

	ptrPoint.clear();

	CString str;

	if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getUseInputInspection())
	{
		if (m_vecDustResult.size() > 0 && theApp.m_bEnforceOkFlag == FALSE)
		{
			breturn = FALSE;
			str.Format("Input Inspection Dust Count = %d NG", m_vecDustResult.size());
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		}
	}

	system_clock::time_point end = system_clock::now();
	nanoseconds nano = end - start;

	str.Format("Divide time: %f", nano.count() / 1000000.);
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

	return breturn;
}
void CFormMainView::InspNotchEdgeDistanceDivide(bool bnot, bool bcir, int nJob, std::vector<cv::Point2f>* ptr, int endX, int W, int H, CViewerEx* mViewer)
{
	system_clock::time_point start = system_clock::now();

	float x, y, r = 32;
	float rh = r / 2;
	// x,y : 원의 중심좌표, r: 반경
	// (a,b),(c,d) : 직선을 이루는 두점의 좌표
	// m,n : 두점을 지나는 직선의 기울기와 절편
	float a, b, c, d;

	int n, cp = 0, cp2 = 0;
	bool bFlag = true;
	float xy[2][2];// 교점의 좌표 두개
	bool bNotch = bnot;
	bool bCircle = bcir;
	bool bNotchShift = false;

	double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(0, 0); //0.004965;// m_pMain->m_pViewDisplayInfo[m_nSelViewerNum].getResolutionX();
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);// 0.004965;// m_pMain->m_pViewDisplayInfo[m_nSelViewerNum].getResolutionY();

	BOOL _bAdaptiveDispensing = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getAdaptiveDispensingFlag(); // Tkyuha 20211216 메탈로 얼라인 하기 위함
	
	r = int(m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspTraceInterval() / xres);
	rh = r / 2;
	double pitch = r * xres;

	FakeDC* pDC = NULL;
	CPen penROI(PS_SOLID, 3, RGB(0, 255, 0));
	CPen penS(PS_SOLID, 3, RGB(255, 255, 0));
	CPen penE(PS_SOLID, 3, RGB(0, 0, 255));
	CPen penRed(PS_SOLID, 3, RGB(255, 0, 0));
	CPen penX(PS_SOLID, 3, RGB(0, 255, 255));

	if (mViewer != NULL)
	{
		pDC = mViewer->getOverlayDC();
	}

	std::vector<cv::Point2f> ptrInterSectionPoint;
	std::vector<float> ptrInterSectionTheta;

	if (!bcir) // Notch 모델 모션 위치 계산 - 설정된  Pitch 간격으로  포인트 생성 루틴
	{
		x = ptr->at(0).x;		y = ptr->at(0).y;
		a = ptr->at(0).x;		b = ptr->at(0).y;
		c = ptr->at(r).x;		d = ptr->at(r).y;

		ptrInterSectionPoint.push_back(cv::Point2f(x, y));

		while (bFlag) // 정상 x>0 && (cp< ptr->size() - 1 || ptr->at(ptr->size()-1).x>x)
		{
			//n = CircleLineIntersection(x, y, r, a, b, c, d, xy);
			// 현재 위치에서 r 만큼 벗어난 지점과의 직선방정식을 만들고,,
			// 원과 만나는 두 점을 구한다.
			n = FindLineCircleIntersection(x, y, r, cv::Point2f(a, b), cv::Point2f(c, d), xy);

			if (n == 0)		break;
			else if (n == 1)
			{
				x = xy[0][0];			y = xy[0][1];
				ptrInterSectionPoint.push_back(cv::Point2f(x, y));
			}
			else if (n == 2)
			{
				float minDiff = 99999;
				int mpos = -1;

				for (int j = cp; j < ptr->size(); j++)
				{
					// 두점과 이후 점들 중에서 가장 가까운 위치를 찾는다..
					float diff1 = fabs(xy[0][0] - ptr->at(j).x) + fabs(xy[0][1] - ptr->at(j).y);
					float diff2 = fabs(xy[1][0] - ptr->at(j).x) + fabs(xy[1][1] - ptr->at(j).y);

					if (diff1 < minDiff)
					{
						mpos = j;
						minDiff = diff1;
						x = xy[0][0];
						y = xy[0][1];
						cp = j;
					}

					if (diff2 < minDiff)
					{
						mpos = j;
						minDiff = diff2;
						x = xy[1][0];
						y = xy[1][1];
						cp = j;
					}
				}

				if (mpos == -1)
				{
					ptrInterSectionPoint.push_back(cv::Point2f(x, y));
				}
				else ptrInterSectionPoint.push_back(cv::Point2f(ptr->at(mpos).x, ptr->at(mpos).y));
			}

			float minDiff = 99999;
			for (int j = cp2; j < ptr->size() - rh; j++)
			{
				float diffSum = fabs(x - ptr->at(j).x) + fabs(y - ptr->at(j).y);
				if (diffSum < minDiff)
				{
					a = ptr->at(j).x;			b = ptr->at(j).y;
					c = ptr->at(j + rh).x;		d = ptr->at(j + rh).y;
					minDiff = diffSum;
					cp2 = j + 1;
				}
			}

			float diffSum2 = fabs(x - ptrInterSectionPoint[0].x) + fabs(y - ptrInterSectionPoint[0].y);
			if (minDiff > 50 || diffSum2 < 10) 			bFlag = false;
			if (xy[0][0] > endX || xy[1][0] > endX)		bFlag = false;
		}
		//////////// 마지막 포인트 하고 이전 포인트가 겹치는 경우 삭제 => 반복 계산된 경우 예외 처리
		if (ptrInterSectionPoint[ptrInterSectionPoint.size() - 1].x == ptrInterSectionPoint[ptrInterSectionPoint.size() - 2].x &&
			ptrInterSectionPoint[ptrInterSectionPoint.size() - 1].y == ptrInterSectionPoint[ptrInterSectionPoint.size() - 2].y)
			ptrInterSectionPoint.erase(ptrInterSectionPoint.end() - 1);
		//////////// 마지막 포인트가 끝점이 아닌 경우 끝점에 한포인트 추가
		if (bNotch && fabs(endX - ptrInterSectionPoint[ptrInterSectionPoint.size() - 1].x) > rh && fabs(endX - ptrInterSectionPoint[ptrInterSectionPoint.size() - 1].x) < rh + 5)
			ptrInterSectionPoint.push_back(cv::Point2f(endX, ptrInterSectionPoint[ptrInterSectionPoint.size() - 1].y));
	}
	else  // Circle 모델 모션 위치 계산 - 설정된  Pitch 간격으로  포인트 생성 루틴
	{
		// 픽셀 단위의 반지름을 mm 단위로 변환
		double circleLength = 2. * CV_PI * CIRCLE_RADIUS * xres;
		int circleCount = MAX(1, int(circleLength / pitch));
		double anglestep = 360. / circleCount;
		bool _bsign = m_dADispesing_StartAngle - m_dADispesing_EndAngle > 0 ? true : false;

		// 360도를 Pitch 간격으로 나누어 포인트 갯수 계산
		if (_bAdaptiveDispensing) //20220103 Tkyuha Adaptive Dispensing 사용
		{
			int _dec = 0,ni= int(m_dADispesing_StartAngle- anglestep+0.5);
			double _tmpPitch= anglestep;

			for (int i = 0; i < circleCount; i++) // 절반의 각을 구한다.
			{
				_tmpPitch = anglestep - (m_dInclination * _dec);
				ni = int(ni + _tmpPitch + 0.5) % 360;

				if (ni > ptr->size()) continue;
				if ((_bsign && ni> m_dADispesing_EndAngle && ni< m_dADispesing_StartAngle)||
					(!_bsign && ni > m_dADispesing_EndAngle))
				{// 원복하고 빠진다.
					ni = int(ni - _tmpPitch+360 + 0.5) % 360;
					break;
				}

				ptrInterSectionPoint.push_back(cv::Point2f(ptr->at(ni).x, ptr->at(ni).y));
				ptrInterSectionTheta.push_back(ni);				
				_dec++;
			} 

			for (int i = _dec; _dec>0; i++) // 절반의 각을 구한다.
			{
				_tmpPitch = anglestep - (m_dInclination * _dec);
				ni = int(ni + _tmpPitch + 0.5) % 360;

				if (ni > ptr->size()) continue;
				if ((!_bsign && ni < m_dADispesing_EndAngle && ni > m_dADispesing_StartAngle) ||
					(_bsign && (ni < m_dADispesing_EndAngle || ni > m_dADispesing_StartAngle)))
					break;

				ptrInterSectionPoint.push_back(cv::Point2f(ptr->at(ni).x, ptr->at(ni).y));
				ptrInterSectionTheta.push_back(ni);
				_dec--;
			}

			if (ptrInterSectionTheta.size() > 10)
			{
				int _ni = int(ptrInterSectionTheta[0] - 1 + 360) % 360;
				if (ptrInterSectionTheta[ptrInterSectionTheta.size() - 1] != _ni && ptrInterSectionTheta[ptrInterSectionTheta.size() - 1]< _ni)
				{					
					// 도포 시작 위치로 보내기 위해 마지막에 시작 위치 90도 포인트 추가(초기 시작 위치 90도)
					ptrInterSectionPoint.push_back(cv::Point2f(ptr->at(_ni).x, ptr->at(_ni).y));
					ptrInterSectionTheta.push_back(_ni);
				}
				// 시작 위치를 동일 하기 위해서 90도 기준으로 변경					
				double _baseT = ptrInterSectionTheta[0]-360;
				int _tmpV = 0;
				for (int i = 0; i < ptrInterSectionTheta.size(); i++)
				{
					_tmpV = int(ptrInterSectionTheta[i]- _baseT + 90);
					ptrInterSectionTheta[i] = float(_tmpV%360) ;
				}				
			}
		}
		else
		{
			for (int i = 0; i < circleCount; i++)
			{
				int ni = int(90 + (anglestep * i));
				if (ni >= 360) ni = ni - 360;
				if (ni > ptr->size()) continue;
				ptrInterSectionPoint.push_back(cv::Point2f(ptr->at(ni).x, ptr->at(ni).y));
				ptrInterSectionTheta.push_back(ni);
			}

			if (ptrInterSectionTheta[ptrInterSectionTheta.size() - 1] < 89.f)
			{
				// 도포 시작 위치로 보내기 위해 마지막에 시작 위치 90도 포인트 추가(초기 시작 위치 90도)
				ptrInterSectionPoint.push_back(cv::Point2f(ptr->at(89).x, ptr->at(89).y));
				ptrInterSectionTheta.push_back(89.);
			}
		}
	}
	////  여기까지 도포 위치 포인트 좌표 생성 완료

	double x_centerPos = 0, y_centerPos = 0;

	if (ptrInterSectionPoint.size() > 3) // 정상적인 경우에만 진행 하기 위해 3포인트 이상 인경우만 진행
	{
		double xmin = 9999, ymin = 9999;
		int xmin_pos = 1, count = int(ptrInterSectionPoint.size());
		double _tsum = 0;
		std::vector<double> x_series1;
		std::vector<double> y_series1;
		std::vector<double> destX1;
		std::vector<double> destY1;

		for (int j = 0; j < ptrInterSectionPoint.size(); j++)
		{
			x_series1.push_back(j);
			y_series1.push_back(ptrInterSectionPoint[j].y);
		}

		// 노이즈 제거 하기위해 Smoothing 필터 적용
		//destY1 = sg_smooth(y_series1, 3, 1);
		Savitzky_Golay_smoothing(&x_series1, &y_series1, &destX1, &destY1);
		// 아래 for문 사용 하지 않음=>중심점이 포인트에 추가 된 경우만 사용됨
		for (int j = 0; j < ptrInterSectionPoint.size(); j++)
		{
			ptrInterSectionPoint[j].y = destY1.at(j);
			if (j > 0 && xmin > fabs(ptrInterSectionPoint[j].x - ptrInterSectionPoint[j - 1].x))
			{
				xmin_pos = j;
				xmin = fabs(ptrInterSectionPoint[j].x - ptrInterSectionPoint[j - 1].x);
			}
			if (j > 0) _tsum += fabs(destY1.at(j) - destY1.at(j - 1));
		}
		// 포인트 중심점 계산을 위해서 
		// 얼라인이 정상적으로 된경우 카메라 센터에 위치 하므로 이미지 중심점으로 변경
		x_centerPos = W / 2.;// (ptrInterSectionPoint[0].x + endX) / 2.;// +(ptrInterSectionPoint[xmin_pos].x + ptrInterSectionPoint[xmin_pos - 1].x) / 2.;
		y_centerPos = H / 2.;/*((ptrInterSectionPoint[0].y + ptrInterSectionPoint[1].y + ptrInterSectionPoint[2].y) +
			(ptrInterSectionPoint[count - 1].y + ptrInterSectionPoint[count - 2].y + ptrInterSectionPoint[count - 3].y)) / 6.;*/

		x_series1.clear();
		y_series1.clear();
		destX1.clear();
		destY1.clear();
	}

	int step = 8;
	int nLoopCount = 0;
	double distanceT = 0.5 / xres;
	cv::Point2f rC;
	std::vector<cv::Point2f> ptrInterSectionShiftPoint;
	std::vector<cv::Point2f> ptrInterNewSectionPoint;

	std::vector<double> x_src;
	std::vector<double> y_src;
	std::vector<double> destX;
	std::vector<double> destY;

	std::vector<cv::Point2f> ptrInterSectionTmpShiftPoint;
	std::vector<cv::Point2f> ptrInterNewTmpSectionPoint;

	if (!bNotchShift && !bCircle)
	{
			// Notch 인경우 각 포인트에 대한 각도 계산 
			// 각 점들 간의 각도를 계산
			for (int j = 1; j < ptrInterSectionPoint.size(); j++)
			{
				// 4도 이하인 경우는 노이즈 인걸로 간주해서 0으로 강제 계산 => calcAngleFromPoints 참조
				// 이전 포인트와 현재 포인트 두점을 이용 해서 계산
				ptrInterSectionTheta.push_back(calcAngleFromPoints(ptrInterSectionPoint[j - 1], ptrInterSectionPoint[j]));  // 원데이타로 계산
			}
			// j 인덱스가 1부처 시작 해서 포이트 갯수를 맞추기 위해 마지막에 한포인트 이전 데이타로 추가
			// 중요 :: 포인트 좌표 갯수와 각도 갯수는 일치 해야 함
			ptrInterSectionTheta.push_back(calcAngleFromPoints(
				ptrInterSectionPoint[ptrInterSectionPoint.size() - 2], ptrInterSectionPoint[ptrInterSectionPoint.size()-1]));  // 원데이타로 계산

	}

	if (ptrInterSectionPoint.size() > 10)
	{
		// 계산된 각도 노이즈 제거를 위한 smoothing 필터 적용
		if (bNotch)
		{
			std::vector<double> x_series;
			std::vector<double> y_series;
			std::vector<double> destX;
			std::vector<double> destY;

			ptrInterSectionTheta[0] = 0;
			ptrInterSectionTheta[ptrInterSectionTheta.size() - 1] = 0;
			for (int j = 0; j < ptrInterSectionTheta.size(); j++)
			{
				x_series.push_back(j);
				y_series.push_back(ptrInterSectionTheta[j]);
			}

			//destY = sg_smooth(y_series, 3, 1);
			Savitzky_Golay_smoothing(&x_series, &y_series, &destX, &destY);

			m_pMain->m_ELB_TraceResult.m_vTraceProfile[3].clear();
			m_pMain->m_ELB_TraceResult.m_vTraceProfile[4].clear();

			for (int j = 0; j < ptrInterSectionTheta.size(); j++)
			{
				//if (j < 3 && fabs(destY.at(j))<6) destY.at(j) = 0;
				if (ptrInterSectionTheta[j] != 0)		ptrInterSectionTheta[j] = destY.at(j);
				m_pMain->m_ELB_TraceResult.m_vTraceProfile[3].push_back(ptrInterSectionPoint[j].x);
				m_pMain->m_ELB_TraceResult.m_vTraceProfile[4].push_back(ptrInterSectionPoint[j].y);
			}

			x_series.clear();
			y_series.clear();
			destX.clear();
			destY.clear();
		}

		if (bCircle)  // 회귀 분석을 통해서 노이즈 제거 기능
		{
			if (_bAdaptiveDispensing) //20220103 Tkyuha Adaptive Dispensing 사용
			{
				bool _baddflag = false;
				int _removecount = 0;
				for (int j = 2; j < ptrInterSectionTheta.size(); j++)
				{
					if (!_baddflag && ptrInterSectionTheta[j] <= 90) _baddflag = true;
					if (_baddflag)
					{						
						if(ptrInterSectionTheta[j]>90)			_removecount++;
						ptrInterSectionTheta[j] += 360;
					}
				}

				for (int j = 0; j < _removecount; j++)
				{
					ptrInterSectionPoint.pop_back();
					ptrInterSectionTheta.pop_back();
					m_pMain->m_ELB_TraceResult.m_vTraceProfile[3].pop_back();
					m_pMain->m_ELB_TraceResult.m_vTraceProfile[4].pop_back();
				}
			}
			else
			{
				double x[1000] = { 0, };
				double y[1000] = { 0, };
				double aa, bb;
				int ncount = 0;

				for (int j = 0; j < ptrInterSectionTheta.size(); j++)
				{
					if (ptrInterSectionTheta[j] < 90) continue;
					y[ncount] = ptrInterSectionTheta[j];
					x[ncount] = j;
					ncount++;
				}

				LeastSqrRegression(x, y, ncount, aa, bb); // 20211102 Tkyuha 2에서 ncount로 변경됨

				for (int j = 0; j < ptrInterSectionTheta.size(); j++)
					ptrInterSectionTheta[j] = aa * j + bb;
			}
		}

		//  화면에 그려 주는 역활
		if (pDC != NULL)
		{
			pDC->SelectObject(&penROI);

			for (int i = 0; i < ptrInterSectionPoint.size(); i++)
			{
				Gdiplus::Color color;
				if (i < 5) color = 0xffffff00;
				else if ((i / 5) % 2 == 0)color = 0xff0000ff;
				else color = 0xff00ff00;

				if (mViewer != NULL) mViewer->AddSoftGraphic(new GraphicPoint(cv::Point2f(ptrInterSectionPoint[i].x, ptrInterSectionPoint[i].y), color, 3));
				if (bNotchShift)
				{
					if (mViewer != NULL) mViewer->AddSoftGraphic(new GraphicPoint(cv::Point2f(ptrInterSectionShiftPoint[i].x, ptrInterSectionShiftPoint[i].y), 0xff00ffff, 3));
				}
			}
			if (bNotchShift)
			{
				if (bNotch)
					if (mViewer != NULL) mViewer->AddSoftGraphic(new GraphicPoint(cv::Point2f(x_centerPos, y_centerPos), 0xff00ff00, 3));

				for (int i = 0; i < ptrInterNewSectionPoint.size(); i++)
					if (mViewer != NULL) mViewer->AddSoftGraphic(new GraphicPoint(cv::Point2f(ptrInterNewSectionPoint[i].x, ptrInterNewSectionPoint[i].y), 0xffff0000, 3));
			}
		}
		//// 여기까지 화면 Display 
		// 
		//// 아래는 노즐 시작 위치 변경 하기 위함
		double rotateX = m_pMain->m_ELB_TraceResult.m_vELB_ImageRotateCenter.x; //0.049
		double rotateY = m_pMain->m_ELB_TraceResult.m_vELB_ImageRotateCenter.y;  //1.35

		CString str_temp;
		str_temp.Format("[Trace] Rot_X : %.3f, Rot_Y : %.3f", rotateX, rotateY);
		::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str_temp);

		if (!bNotchShift) // 시작 위치 전달
		{
			if (bCircle)  /// 원 인경우는 반지름으로 시작 위치 변경
			{
				rotateX = 0;
				rotateY = CIRCLE_RADIUS * xres;
			}
			else
			{
				int org_pos = m_pMain->vt_job_info[nJob].model_info.getMachineInfo().getAlignOrginPos(0);
				int real_cam = m_pMain->vt_job_info[nJob].camera_index[0];
				int w = m_pMain->m_stCamInfo[real_cam].w;
				int h = m_pMain->m_stCamInfo[real_cam].h;
				//  Notch인경우 한쪽으로 치우친 경우에 사용 현재는 사용 하지 않고 있음
				if (org_pos == _NOTCH_X85_Y50) // 얼라인 중심점이 오른쪽 에 있는 경우
				{
					double xr = 0.85;
					double yr = 0.5;
					double robot_org_x = 0.0;
					double robot_org_y = 0.0;
					double robot_trace_x = 0.0;
					double robot_trace_y = 0.0;

					double avg_y = (ptrInterSectionPoint.at(ptrInterSectionPoint.size() - 1).y + ptrInterSectionPoint.at(0).y) / 2.0;

					m_pMain->GetMachine(nJob).PixelToWorld(0, 0, w * xr, avg_y, &robot_org_x, &robot_org_y);
					m_pMain->GetMachine(nJob).PixelToWorld(0, 0, ptrInterSectionPoint.at(ptrInterSectionPoint.size() - 1).x,
						ptrInterSectionPoint.at(ptrInterSectionPoint.size() - 1).y, &robot_trace_x, &robot_trace_y);

					rotateX = robot_org_x - robot_trace_x;
					rotateY = robot_trace_y - robot_org_y;
					rotateY = (avg_y - ptrInterSectionPoint.at(ptrInterSectionPoint.size() - 1).y) * yres;
				}
				else
				{
					// 이미지 중심점(센터 얼라인 중심점) 에서 노즐의 시작 위치를 계산 => 오른쪽 마지막 포인트 에서 계산
					/*rotateX = (x_centerPos - ptrInterSectionPoint.at(ptrInterSectionPoint.size() - 1).x) * xres;
					rotateY = (y_centerPos - ptrInterSectionPoint.at(ptrInterSectionPoint.size() - 1).y) * yres;*/
					rotateX = (W / 2. - ptrInterSectionPoint.at(ptrInterSectionPoint.size() - 1).x) * xres + m_pMain->m_dCurrentRotateOffsetX;
					rotateY = (H / 2. - ptrInterSectionPoint.at(ptrInterSectionPoint.size() - 1).y) * yres;
				}
			}
		}
		else
		{
			//  여기는 사용 하지 않음
			rotateX = (x_centerPos - ptrInterNewSectionPoint.at(ptrInterNewSectionPoint.size() - 1).x) * xres;
			rotateY = (y_centerPos - ptrInterNewSectionPoint.at(ptrInterNewSectionPoint.size() - 1).y) * yres;
		}
		// PLC에 전달 하기 위해 m_vELB_RotateCenter 변수에 시작 위치 전달
		m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter = cv::Point2d(rotateX, rotateY);
		str_temp.Format("[Trace_Recal] Rot_X : %.3f, Rot_Y : %.3f", rotateX, rotateY);
		::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str_temp);

		int lastPtr = 0;
		float offsetx = 0;
		float offsety = 0;
		float a = 0, b = 0, c = 0;

		if (!bNotchShift)
		{
			//PLC에 전달 하기 위해서는 시작위치를 0으로 만들고 상대 위치 로 계산 해야됨
			// 초기 변수 생성
			lastPtr = int(ptrInterSectionPoint.size() - 1);   //역방향  ptrInterSectionPoint.size() - 1 Notch인경우 , 타원인 경우 0  //변경점
			offsetx = ptrInterSectionPoint.at(lastPtr).x;	// 상대 위치 변수 초기화 값 (오른쪽 마지막 값)
			offsety = ptrInterSectionPoint.at(lastPtr).y;	// 상대 위치 변수 초기화 값 (오른쪽 마지막 값)
			c = ptrInterSectionTheta.at(lastPtr - 1); 		// 상대 위치 변수 초기화 값 (오른쪽 마지막 값)
		}
		else
		{
			// 여기는 사용 되지 않음
			lastPtr = int(ptrInterNewSectionPoint.size() - 1);   //역방향  ptrInterSectionPoint.size() - 1 Notch인경우 , 타원인 경우 0  //변경점
			offsetx = ptrInterNewSectionPoint.at(lastPtr).x;
			offsety = ptrInterNewSectionPoint.at(lastPtr).y;
			c = ptrInterSectionTheta.at(lastPtr - 2);
		}
		double dbAlignX, dbAlignY;
		TPointList5D arrayList;
		//  Notch 오른쪽 기준으로 상대 위치 계산 (현재 값에서  마지막 초기값을 빼서 계산)
		if (bNotch)
		{

			for (int k = lastPtr; k >= 0; k--) // 역방향 Notch
			{
				float tc = (k == 0) ? ptrInterSectionTheta.at(0) - c : ptrInterSectionTheta.at(k - 1) - c;
				///// 원영상 위치에서 계산 할경우
				float xc = 0;
				float yc = 0;
				if (!bNotchShift)
				{
					// 로봇 좌표계로 계산
					xc = (ptrInterSectionPoint.at(k).x - offsetx) * xres;
					yc = (ptrInterSectionPoint.at(k).y - offsety) * yres;
				}
				else
				{
					// 사용 하지 않음
					///// Shift 영상 위치에서 계산 할경우
					xc = (ptrInterNewSectionPoint.at(k).x - offsetx) * xres;
					yc = (ptrInterNewSectionPoint.at(k).y - offsety) * yres;
				}

				if (!bNotch)
				{
					// Circle 인경우 360도 넘어 가는 경우 예외 처리
					//if (tc > 0) tc = tc - 360.f; // 반시계방향
					if (tc < 0) tc = tc + 360.f; // 시계방향
				}
				// 각도에 따른 회전 변환 으로 이한 좌표 계산
				calcRotate(rotateX, rotateY, xc, yc, (-tc) * CV_PI / 180.0, &dbAlignX, &dbAlignY);

				if (k == lastPtr)
				{
					// 시작 위치는 무조건 0으로 만든다
					a = dbAlignX;
					b = dbAlignY;
					arrayList.push_back({ xc, yc, 0.f, 0.f, 0.f }); //(3, 4, 5번째 데이타 사용) 1,2번은 참고용
				}
				else
				{
					dbAlignX = dbAlignX - a;
					dbAlignY = dbAlignY - b;
					arrayList.push_back({ xc, yc, tc, float(dbAlignX), float(dbAlignY) });
					//변수 참고        //{ 회전이전 x,회전 이전 y,각도, 회전 이후 x, 회전 이후 y}
				}

			}

			int nS = int(arrayList.size());

			if (!bNotch)
			{
				// 여기는 사용 되지 않음
				// 루프가 형성된 경우 마지막 데이타 생성
				int lastn = int(ptrInterSectionPoint.size() - 1);
				double lastAngle = calcAngleFromPoints(ptrInterSectionPoint[lastn], ptrInterSectionPoint[0]) - c;
				//if (tc > 0) tc = tc - 360.f; // 반시계방향
				if (lastAngle < 0) lastAngle = lastAngle + 360.f; // 시계방향

				calcRotate(rotateX, rotateY, 0, 0, (-lastAngle) * CV_PI / 180.0, &dbAlignX, &dbAlignY);

				dbAlignX = dbAlignX - a;
				dbAlignY = dbAlignY - b;
				arrayList.push_back({ 0.f, 0.f, float(lastAngle), float(dbAlignX), float(dbAlignY) });

				m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.x = arrayList[0].at(2) * xres;
				m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.y = (arrayList[0].at(3) - arrayList[nS / 2].at(3)) * yres;
				m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.x = m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.x;
				m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.y = m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.y;
			}
			else
			{
				/////
				// 
				// 중요 지점
				// m_vELB_RotateCenter 변수는 노즐의 시작 위치 
				// m_vELB_ReverseRotateCenter  변수는 노즐의 역방향 시작 위치 
				// 
				/////
				/*m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.x = arrayList[0].at(2) * xres;
				m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.y = (arrayList[0].at(3) - arrayList[nS / 2].at(3)) * yres;*/
				// 모션 위치 방향으로 인해 - 계산
				m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.x *= -1;  // 회전 중심은 그냥 중앙에 위치 시킴
				m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.y *= -1;
				//m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.x -= m_pMain->m_dCurrentRotateStatus; // 시작 위치 옮기기

				int org_pos = m_pMain->vt_job_info[nJob].model_info.getMachineInfo().getAlignOrginPos(0);
				if (org_pos != _NOTCH_X85_Y50)
				{
					// 역방향 노즐 시작 위치 계산
					// 정방향은 오른쪽에서 시작 했기 때문에 역방향은 처음 포인트 에서 계산
					// m_dCurrentRotateOffsetX : Plc에서 시작 위치 offset을 사용 한경우
					double shiftX = (m_pMain->m_dCurrentRotateOffsetX != 0) ? m_pMain->m_dCurrentRotateOffsetX * 2 - rotateX :
						(W / 2. - ptrInterSectionPoint.at(0).x) * xres;

					m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.x = -shiftX;// -m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.x;
					m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.y = -(H / 2. - ptrInterSectionPoint.at(0).y) * yres; //-m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.y;
				}
				else
				{
					// 현재 사용 하지 않음
					// 한쪽으로 치우친 경우에만 계산
					int real_cam = m_pMain->vt_job_info[nJob].camera_index[0];
					int w = m_pMain->m_stCamInfo[real_cam].w;
					int h = m_pMain->m_stCamInfo[real_cam].h;

					double xr = 0.85;
					double yr = 0.5;
					double robot_org_x = 0.0;
					double robot_org_y = 0.0;
					double robot_trace_x = 0.0;
					double robot_trace_y = 0.0;

					double avg_y = (ptrInterSectionPoint.at(ptrInterSectionPoint.size() - 1).y + ptrInterSectionPoint.at(0).y) / 2.0;

					m_pMain->GetMachine(nJob).PixelToWorld(0, 0, w * xr, avg_y, &robot_org_x, &robot_org_y);
					m_pMain->GetMachine(nJob).PixelToWorld(0, 0, ptrInterSectionPoint.at(0).x, ptrInterSectionPoint.at(0).y, &robot_trace_x, &robot_trace_y);


					m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.x = robot_org_x - robot_trace_x;
					m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.y = robot_trace_y - robot_org_y;
					//m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.y = (ptrInterSectionPoint.at(0).y - h / 2.0) * yres;
					m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.y = (avg_y - ptrInterSectionPoint.at(0).y) * yres;

					m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.x *= -1;
					m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.y *= -1;
				}
			}
		}
		else  //  Circle 0도 기준으로 상대 위치 계산 (현재 값에서  마지막 초기값을 빼서 계산)
		{
			lastPtr = int(ptrInterSectionPoint.size());
			offsetx = ptrInterSectionPoint.at(0).x;
			offsety = ptrInterSectionPoint.at(0).y;
			c = ptrInterSectionTheta.at(0);

			CPointF<double> p_center(m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x, m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y);
			std::vector<double> angleseries;
			std::vector<double> destangleY;
			double _Init_yV = 0;

			for (int k = 0; k < lastPtr; k++)
			{
				float tc = ptrInterSectionTheta.at(k) - c;
				///// 원영상 위치에서 계산 할경우
				float xc = 0;
				float yc = 0;
				// 차이를 이용할때 사용
				xc = (ptrInterSectionPoint.at(k).x - offsetx) * xres;
				yc = (ptrInterSectionPoint.at(k).y - offsety) * yres;

				calcRotate(rotateX, rotateY, xc, yc, (-tc) * CV_PI / 180.0, &dbAlignX, &dbAlignY);

				CPointF<double> p2(ptrInterSectionPoint.at(k).x, ptrInterSectionPoint.at(k).y);
				double distance = (GetDistance(p_center, p2) - CIRCLE_RADIUS) * yres;
				if (k == 0) { _Init_yV = distance;		distance = 0; }//시작은 무조건 0도로 시작함
				else distance -= _Init_yV;

				if (!m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseDummyCornerInsp())		distance = 0; //  Y보정은 하지 않음 , Y보정 사용할경우 삭제 필요
				angleseries.push_back(distance);

				arrayList.push_back({ xc, yc, tc, float(0), float(distance) });
			}
			/// Circle Y 보정을 사용 할경우  노이즈 제거를 하기 위함
			if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseDummyCornerInsp())
			{
				std::vector<double> _tmpY; //22.06.02  smoothing 한번더
				_tmpY = sg_smooth(angleseries, 13, 1);
				destangleY = sg_smooth(_tmpY, 4, 1);
				for (int k = 1; k < lastPtr; k++)		arrayList[k].at(4) = destangleY.at(k);
				_tmpY.clear();
			}
			destangleY.clear();
			angleseries.clear();

			/// Circle 은 정방향 역방향이 같으니까 같은 값으로 대체
			m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.x = m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.x;
			m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.y = m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.y;
		}

		// Notch 인경우에 PLC모션 탈선 방지를 위해 완만한 각도로 변경 하기 위해 모든 포인트 재계산
		// Sin 곡선을 이용해서 
		bool brcalc = false;

		if (bNotch)
		{
			brcalc = VerifyReCalc(&arrayList, rotateX, rotateY); // 보정 사용시 ,사용 않할 경우 주석 처리 
		}

		// 로그 파일 남기기 위함

#pragma region File Log 저장

		CString strFileDir, msg;
		SYSTEMTIME	csTime;
		::GetLocalTime(&csTime);

		CString	Time_str = m_pMain->m_strResultTime[nJob];
		CString	Date_str = m_pMain->m_strResultDate[nJob];

		if (Time_str == "" || Date_str == "")
		{
			CTime NowTime;
			NowTime = CTime::GetCurrentTime();
			m_pMain->m_strResultDate[nJob].Format("%04d%02d%02d", NowTime.GetYear(), NowTime.GetMonth(), NowTime.GetDay());
			m_pMain->m_strResultTime[nJob].Format("%02d_%02d_%02d", NowTime.GetHour(), NowTime.GetMinute(), NowTime.GetSecond());

			Time_str = m_pMain->m_strResultTime[nJob];
			Date_str = m_pMain->m_strResultDate[nJob];
		}

		///  D:\\ Result \\ Files \\ 날짜 \\ 모델ID  \\ TraceResult \\

		strFileDir.Format("%sFiles\\", m_pMain->m_strResultDir);
		if (_access(strFileDir, 0) != 0)	CreateDirectory(strFileDir, NULL);
		//날짜
		strFileDir.Format("%s%s\\", strFileDir, Date_str);
		if (_access(strFileDir, 0) != 0)	CreateDirectory(strFileDir, NULL);
		//모델ID
		strFileDir.Format("%s%s\\", strFileDir, m_pMain->vt_job_info[nJob].model_info.getModelID());
		if (_access(strFileDir, 0) != 0)	CreateDirectory(strFileDir, NULL);
		//구분폴더
		strFileDir.Format("%sTRACE\\", strFileDir);
		if (_access(strFileDir, 0) != 0)	CreateDirectory(strFileDir, NULL);

		//PanelID
		//KJH 2021-08-14 PANEL ID 없는거 따로 별도의 폴더로 관리
		if (strlen(m_pMain->vt_job_info[nJob].main_object_id.c_str()) <= 13)
		{
			strFileDir.Format("%sNot Exist Panel ID\\", strFileDir);
			if (_access(strFileDir, 0) != 0)	CreateDirectory(strFileDir, NULL);
		}

		msg.Format("%sTraceProfileResult_%s_%s.csv", strFileDir, m_pMain->vt_job_info[nJob].main_object_id.c_str(), Time_str);

		ofstream  writeFile(msg);
		CString str;
		str.Format("write - %s", msg);
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		if (writeFile.is_open())
		{
			msg.Format("%d , %4.3f, %4.3f, %4.3f, %4.3f\n", arrayList.size(), rotateX, rotateY, -m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.x, -m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.y);		writeFile << msg;

			if (!brcalc)
			{
				// 재계산 하지 않은 원본 데이타 로그 생성
				for (int i = 0; i < 3; i++) m_pMain->m_ELB_TraceResult.m_vTraceProfile[i].clear();
				for (int i = 0; i < arrayList.size(); i++) // 부호 반전해서 사용하고 있음 확인 필요 // 2021022 TKYUHA
				{
					msg.Format("%4.3f, %4.3f, %4.3f, %4.3f, %4.3f\n", -arrayList[i].at(0), -arrayList[i].at(1), -arrayList[i].at(3), -arrayList[i].at(4), arrayList[i].at(2));
					writeFile << msg;
					m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].push_back(-arrayList[i].at(3));
					m_pMain->m_ELB_TraceResult.m_vTraceProfile[1].push_back(arrayList[i].at(4));
					m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].push_back(arrayList[i].at(2));
				}
			}
			else
			{
				// 재계산한 원본 데이타 로그 생성

				for (int i = 0; i < arrayList.size(); i++) // 부호 반전해서 사용하고 있음 확인 필요 // 2021022 TKYUHA
				{
					msg.Format("%4.3f, %4.3f, %4.3f,  %4.3f, %4.3f\n", -arrayList[i].at(0), -arrayList[i].at(1),
						m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].at(i), -m_pMain->m_ELB_TraceResult.m_vTraceProfile[1].at(i),
						m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].at(i));
					writeFile << msg;
				}
			}

			writeFile.close();

			m_nNotchLeftStartCount = 0;
			m_nNotchRightEndCount = 0;

			int org_pos = m_pMain->vt_job_info[nJob].model_info.getMachineInfo().getAlignOrginPos(0);
			int method = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod();
			//KJH 2021-11-01 1회전 정방향 후 1회전 역방향 알고리즘 추가
			BOOL Use_Cycle_Trace = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseCycleTrace();

			//PLC에 전송 하기 위한 데이타 생성
			// OFFSET 길이 회전수 에 따른 위치 변경
			if (method == METHOD_CIRCLE)
			{
				double tx, ty, tt;
				int ncc = MIN(4, MAX(1, m_pMain->m_nCurrentCircleCount)); //회전수

				//KJH 2021-11-01 1회전 정방향 후 1회전 역방향 알고리즘 추가
				if (Use_Cycle_Trace)
				{
					ncc = 1;
				}

				if (ncc != 1)
				{
					for (int j = 1; j < ncc; j++)  // 2회전 반복해서 생성
					{
						for (int i = 1; i < arrayList.size(); i++)
						{
							tx = m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].at(i);
							ty = m_pMain->m_ELB_TraceResult.m_vTraceProfile[1].at(i); //+ m_pMain->m_nSecondCircleOffset_Y
							tt = m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].at(i);

							/*/
							if (tt + 360 * j > dTargetAngle)	// blue 2022.09.01
								break;
							//*/

							m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].push_back(tx);
							m_pMain->m_ELB_TraceResult.m_vTraceProfile[1].push_back(ty);
							m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].push_back(tt + (360 * j));
						}
					}
				}
				//Tkyuha 2021-11-22 Concept 변경 호의 길이를 반영

				double tLength = 2 * CV_PI * fabs(rotateY);
				double oStep = MAX(0.01, tLength / 360.); //1도당 도는 길이
				double oCount = oStep / m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspTraceInterval();// 인터발 배율
				int nICount = int(m_pMain->m_nCurrentCircleAccDec / oStep * oCount);
				
				// 가감속에 따른 데이타 포인트 추가 생성
				for (int j = 1; j < nICount; j++)
				{
					tx = m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].at(j);
					ty = m_pMain->m_ELB_TraceResult.m_vTraceProfile[1].at(j); //+ m_pMain->m_nSecondCircleOffset_Y
					tt = m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].at(j);
					m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].push_back(tx);
					m_pMain->m_ELB_TraceResult.m_vTraceProfile[1].push_back(ty);
					m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].push_back(tt + (360 * ncc));
				}

				// 메탈하고 글라스 사이 거리 미리 계산 함
				// 예외 처리에서 사용 
				//m_dCircleRadius_MP,		m_ptCircleCenter_MP ,CIRCLE_RADIUS,m_pMain->m_ELB_TraceResult.m_ELB_ResultXY
				double diffSteelAlignX = (m_ptCircleCenter_MP.x - m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x) * xres;
				double diffSteelAlignY = (m_ptCircleCenter_MP.y - m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y) * yres;
				// PLC에서 z축 위치를 변경하는 시점에
				// y축을 이동하는 걸로.. 변경 가감속 거리 만큼은 더 돌리자.
				for (int i = 0; i < m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].size(); i++)
				{
					if (ncc >= 4 && m_pMain->m_ELB_TraceResult.m_vTraceProfile[2][i] > abs(m_pMain->m_dCircleFourThetaPos))
					{
						m_pMain->m_ELB_TraceResult.m_vTraceProfile[1][i] += m_pMain->m_nFourCircleOffset_Y;
						//m_pMain->m_ELB_TraceResult.m_vTraceProfile[0][i] += diffSteelAlignX; // Steel 보상시 사용
						//m_pMain->m_ELB_TraceResult.m_vTraceProfile[1][i] += diffSteelAlignY;
					}
					else if (ncc >= 3 && m_pMain->m_ELB_TraceResult.m_vTraceProfile[2][i] > abs(m_pMain->m_dCircleThirdThetaPos))
					{
						m_pMain->m_ELB_TraceResult.m_vTraceProfile[1][i] += m_pMain->m_nThirdCircleOffset_Y;
						//m_pMain->m_ELB_TraceResult.m_vTraceProfile[0][i] += diffSteelAlignX; // Steel 보상시 사용
						//m_pMain->m_ELB_TraceResult.m_vTraceProfile[1][i] += diffSteelAlignY;
					}
					else if (ncc >= 2 && m_pMain->m_ELB_TraceResult.m_vTraceProfile[2][i] > abs(m_pMain->m_dCircleSecondThetaPos))
					{
						m_pMain->m_ELB_TraceResult.m_vTraceProfile[1][i] += m_pMain->m_nSecondCircleOffset_Y;
						//m_pMain->m_ELB_TraceResult.m_vTraceProfile[0][i] += diffSteelAlignX; // Steel 보상시 사용
						//m_pMain->m_ELB_TraceResult.m_vTraceProfile[1][i] += diffSteelAlignY;
					}
				}
				// 수정 Trace 로그 저장
				int vs = int(m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].size());
				msg.Format("%sTraceProfile_Recalc_%s_%s.csv", strFileDir, m_pMain->vt_job_info[nJob].main_object_id.c_str(), Time_str);
				ofstream  writeFile(msg);

				if (writeFile.is_open())
				{
					msg.Format("%d , %4.3f, %4.3f, %4.3f, %4.3f\n", vs, m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.x, m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.y,
						m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.x, m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.y);
					writeFile << msg;
					for (int i = 0; i < vs; i++)
					{
						msg.Format("%4.3f, %4.3f, %4.3f,  %4.3f, %4.3f\n", 0, 0,
							m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].at(i), 
							m_pMain->m_ELB_TraceResult.m_vTraceProfile[1].at(i),
							m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].at(i));
						writeFile << msg;
					}
				}
				writeFile.close();

				//HTK 2022-03-30 Tact Time Display 추가
				 m_clockProc_elapsed_time[3] = (clock() - m_clockProcStartTime[3]) / (float)CLOCKS_PER_SEC;
				((CPaneAuto*)m_pMain->m_pPane[PANE_AUTO])->update_grid_processing_time(1, m_clockProc_elapsed_time[3], 3, true);

				//KJH 2022-07-27 Tact Time Log 추가
				theLog.logmsg(LOG_PLC_SEQ, "Trace Calc Tact Time = %.4f", m_clockProc_elapsed_time[3]);

				if (m_pMain->m_bAutoStart)
				{
					//HTK 2022-03-30 Tact Time Display 추가
					m_clockProcStartTime[4] = clock();
					// PLC에 모션 위치 및 초기 위치 전달
					if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getELBPartial_DispensingModeEnable())
					{
						double _range = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getAdaptiveDispensingRange();
						int _partialCount = MIN(int(m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].size()),int(_range * oCount + nICount)); // 가감속 추가 해서
	
						long calibData[2] = { 0, };
						calibData[0] = LOWORD((long)(m_IndexMaxWidth_Angle * MOTOR_SCALE));
						calibData[1] = HIWORD((long)(m_IndexMaxWidth_Angle * MOTOR_SCALE));
						g_CommPLC.SetWord(m_pMain->vt_job_info[nJob+1].plc_addr_info.write_word_start+8, 2, calibData); // 노즐잡에 추가 했음 Tkyuha 20220327 조심해서 사용 할것

						m_pMain->sendTraceProfileCircleData(m_pMain->vt_job_info[nJob].plc_addr_info.write_word_sub_start,
							m_pMain->vt_job_info[nJob].plc_addr_info.write_word_start, pitch, _partialCount);
					}
					else
					{
						m_pMain->sendTraceProfileCircleData(m_pMain->vt_job_info[nJob].plc_addr_info.write_word_sub_start,
							m_pMain->vt_job_info[nJob].plc_addr_info.write_word_start, pitch);
					}
					
					//HTK 2022-03-30 Tact Time Display 추가
					m_clockProc_elapsed_time[4] = (clock() - m_clockProcStartTime[4]) / (float)CLOCKS_PER_SEC;
					((CPaneAuto*)m_pMain->m_pPane[PANE_AUTO])->update_grid_processing_time(1, m_clockProc_elapsed_time[4], 4, true);

					//KJH 2022-07-27 Tact Time Log 추가
					theLog.logmsg(LOG_PLC_SEQ, "Trace Data PLC Send Tact Time = %.4f", m_clockProc_elapsed_time[4]);
				}
			}
			else
			{
				// Notch 인경우  왼쪽 길이와 오른쪽 길이 감안해서 생성 삭제 

				if (org_pos != _NOTCH_X85_Y50 && m_pMain->m_dCurrentRotateOffsetX != 0)
				{
					// 왼쪽 연장
					double tx = 0, ty = 0, tt = 0;
					int ncc;
					int vs = int(m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].size() - 1);
					double interval = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspTraceInterval();
					double lCurrent = fabs(arrayList[vs].at(0)) / 2 + m_pMain->m_dCurrentRotateOffsetX;
					double lLength = (m_dLeftNotchLength + m_pMain->m_nCurrentNotchDec) - lCurrent;
					double rLength = lCurrent - (m_pMain->m_dCurrentRotateOffsetX * 2) - (m_dRightNotchLength + m_pMain->m_nCurrentNotchAcc);
					ncc = int(lLength / interval);

					if (vs > 0)
					{
						if (ncc < 0)
						{
							// 왼쪽 길이가 짧아진 경우  삭제
							for (int j = ncc; j < 0; j++)
							{
								m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].pop_back();
								m_pMain->m_ELB_TraceResult.m_vTraceProfile[1].pop_back();
								m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].pop_back();
							}
						}
						else
						{
							tx = m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].at(vs);
							ty = m_pMain->m_ELB_TraceResult.m_vTraceProfile[1].at(vs);
							tt = m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].at(vs);

							// 왼쪽 길이가 길어진 경우 마지막 데이타로 추가
							for (int j = 0; j < ncc; j++)
							{
								tx = tx + interval;
								m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].push_back(tx);
								m_pMain->m_ELB_TraceResult.m_vTraceProfile[1].push_back(ty);
								m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].push_back(tt);
							}
						}
						str.Format("Notch Left add - %d, %f", ncc, tx);
						::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

						m_nNotchLeftStartCount = ncc;
					}

					///////////// 오른쪽 삭제
					ncc = int(rLength / interval);
					if (ncc<0 && vs > abs(ncc))
					{
						// 길이가 짧은 경우 삭제
						m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].erase(m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].begin(), m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].begin() - ncc);
						m_pMain->m_ELB_TraceResult.m_vTraceProfile[1].erase(m_pMain->m_ELB_TraceResult.m_vTraceProfile[1].begin(), m_pMain->m_ELB_TraceResult.m_vTraceProfile[1].begin() - ncc);
						m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].erase(m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].begin(), m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].begin() - ncc);
					}
					else
					{ // 길이가 늘어난 경우 마지막 데이타로 추가 연장
						tx = m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].at(0);
						ty = m_pMain->m_ELB_TraceResult.m_vTraceProfile[1].at(0);
						tt = m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].at(0);
						for (int j = ncc; j < 0; j++)
						{
							tx = tx - interval;
							m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].insert(m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].begin(), tx);
							m_pMain->m_ELB_TraceResult.m_vTraceProfile[1].insert(m_pMain->m_ELB_TraceResult.m_vTraceProfile[1].begin(), ty);
							m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].insert(m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].begin(), tt);
						}
					}
					m_nNotchRightEndCount = ncc;
					vs = int(m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].size());
					tx = m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].at(0);
					ty = m_pMain->m_ELB_TraceResult.m_vTraceProfile[1].at(0);
					tt = m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].at(0);

					for (int i = 0; i < vs; i++)
					{
						m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].at(i) = m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].at(i) - tx;
						m_pMain->m_ELB_TraceResult.m_vTraceProfile[1].at(i) = m_pMain->m_ELB_TraceResult.m_vTraceProfile[1].at(i) - ty;
						m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].at(i) = m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].at(i) - tt;
					}

					// 시작 위치 변경에 따른 노즐 시작 위치 변경 
					// 추가 삭제 포인트에 따른 변경점
					ncc = int(rLength / interval);
					m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.x += (ncc * interval);
					m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.x = -(m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].at(vs - 1) - m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.x);

					// 수정 Trace
					//msg.Format("%sTraceProfile_Recalc_%s.csv", strFileDir, Time_str);
					//ofstream  writeFile(msg);
					//if (writeFile.is_open()) {
					//	msg.Format("%d , %4.3f, %4.3f, %4.3f, %4.3f\n", vs, -m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.x, -m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.y,
					//		-m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.x, -m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.y);
					//	writeFile << msg;
					//	for (int i = 0; i < vs; i++)
					//	{
					//		msg.Format("%4.3f, %4.3f, %4.3f,  %4.3f, %4.3f\n", 0, 0,
					//			-m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].at(i), -m_pMain->m_ELB_TraceResult.m_vTraceProfile[1].at(i),
					//			m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].at(i));
					//		writeFile << msg;
					//	}
					//}
					//writeFile.close();

					str.Format("Notch Right remove - %d", ncc);
					::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

				}
				// 최종 PLC에 전달 하기 위한 데이타 값 저장
				msg.Format("%sTraceProfile_Recalc_%s.csv", strFileDir, Time_str);
				ofstream  writeFile(msg);
				CString msg;
				int vs = int(m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].size());
				if (writeFile.is_open()) 
				{
					msg.Format("%d , %4.3f, %4.3f, %4.3f, %4.3f\n", vs, m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.x, m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.y,
						m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.x, m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.y);
					writeFile << msg;
					for (int i = 0; i < vs; i++)
					{
						msg.Format("%4.3f, %4.3f, %4.3f,  %4.3f, %4.3f\n", 0, 0,
							m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].at(i), m_pMain->m_ELB_TraceResult.m_vTraceProfile[1].at(i),
							m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].at(i));
						writeFile << msg;
					}
				}
				writeFile.close();

				if (m_pMain->m_bAutoStart)
				{
					m_pMain->sendTraceProfileNotchData(m_pMain->vt_job_info[nJob].plc_addr_info.write_word_sub_start,
						m_pMain->vt_job_info[nJob].plc_addr_info.write_word_start, pitch);
				}

			}
		}
		arrayList.clear();
#pragma endregion
	}

	if (bNotchShift)
	{
		cv::Mat mask = cv::Mat::zeros(H, W, CV_8UC1);
		cv::Mat image = cv::Mat::zeros(H, W, CV_8UC1);

		std::vector<cv::Point> vecpt;

		for (int i = 0; i < ptrInterSectionPoint.size(); i++)
			vecpt.push_back(cv::Point(ptrInterSectionPoint.at(i).x, ptrInterSectionPoint.at(i).y));

		for (int i = int(ptrInterNewTmpSectionPoint.size() - 1); i >= 0; i--)
			vecpt.push_back(cv::Point(ptrInterNewTmpSectionPoint.at(i).x, ptrInterNewTmpSectionPoint.at(i).y));

		int num = (int)vecpt.size();
		const cv::Point* pt4 = &(vecpt[0]); // *@#! const
		cv::fillPoly(mask, &pt4, &num, 1, cv::Scalar(255, 255, 255), 8);

		cv::Mat src(H, W, CV_8UC1, m_pMain->getCameraViewBuffer());
		src.copyTo(image, mask);

		//InspDustBurrInspection(nJob, image, 120);

		//cv::inRange(image, cv::Scalar::all(20), cv::Scalar::all(30), image);
		//cv::erode(image, image, cv::Mat::ones(cv::Size(3, 3), CV_8UC1), cv::Point(-1, -1), 2);
		///Subpixel Edge Search  // 회전 중심 찾기 얼라인
		CPatternMatching* pPattern = &m_pMain->GetMatching(nJob);
		pPattern->findPattern(src.data, 0, 0, W, H);
		CFindInfo find = m_pMain->GetMatching(nJob).getFindInfo(0, 0);

		int find_index = find.GetFoundPatternNum();
		double mark_offset_x = m_pMain->GetMatching(nJob).getMarkOffsetX(0, 0, find_index);
		double mark_offset_y = m_pMain->GetMatching(nJob).getMarkOffsetY(0, 0, find_index);

		double pos_x = find.GetXPos() + mark_offset_x;
		double pos_y = find.GetYPos() + mark_offset_y;

		if (pDC != NULL)
		{
			pDC->MoveTo(int(pos_x - 20), int(pos_y));
			pDC->LineTo(int(pos_x + 20), int(pos_y));
			pDC->MoveTo(int(pos_x), int(pos_y - 20));
			pDC->LineTo(int(pos_x), int(pos_y + 20));
		}

		std::vector<Contour> contours;
		std::vector<cv::Vec4i> hierarchy;

		IplImage* pOrgImage = cvCreateImage(cvSize(W, H), 8, 1);
		cv::Mat srcMask;
		srcMask.create(cv::Size(W, H), CV_8U);

		memcpy(pOrgImage->imageData, src.data, W * H);
		m_pMain->m_ManualAlignMeasure.sobelDirection(3, SOBEL_RIGHT, pOrgImage, pOrgImage);
		memcpy(srcMask.data, pOrgImage->imageData, W * H);
		cvReleaseImage(&pOrgImage);

		if (pos_x > 0 && pos_y > 3 && pos_x < W - 56 && pos_y < H - 6)
		{
			cv::Rect inspRoi = cv::Rect(int(pos_x), int(pos_y - 5), 56, 6);

			EdgesSubPix(srcMask(inspRoi), 1.0, 10, 20, contours, hierarchy, 2);

			vector<float> ocvContours;

			for (size_t i = 0; i < contours.size(); ++i)
			{
				for (int j = 0; j < contours[i].points.size(); j++)
				{
					ocvContours.push_back(contours[i].points[j].x + pos_x);
				}
			}

			if (ocvContours.size() > 0)
			{
				sort(ocvContours.begin(), ocvContours.end());

				pos_x = ocvContours.back();
				if (pDC != NULL)
				{
					pDC->MoveTo(int(pos_x - 20), int(pos_y));
					pDC->LineTo(int(pos_x + 20), int(pos_y));
					pDC->MoveTo(int(pos_x), int(pos_y - 20));
					pDC->LineTo(int(pos_x), int(pos_y + 20));
				}
			}
		}

	}

	CString str;
	str.Format("[Trace] Intersection Find Size - %d", ptrInterSectionPoint.size());
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

	//trace_data_fitting(ptrInterSectionPoint);

	if (pDC != NULL)
	{
		pDC->SelectObject(&penROI);


		m_pMain->EllipseRot(pDC, m_pMain->notch_ellipse[0]);
		m_pMain->EllipseRot(pDC, m_pMain->notch_ellipse[1]);

		for (int i = 0; i < m_pMain->m_vtTraceData_Fitting.size(); i++)
		{
			if (i < 5)			pDC->SelectObject(&penS);
			else if (i < 10)	pDC->SelectObject(&penE);
			else				pDC->SelectObject(&penROI);

			//	pDC->Ellipse(int(m_vtTraceData_Fitting[i].x - 2), int(m_vtTraceData_Fitting[i].y - 2), int(m_vtTraceData_Fitting[i].x + 2), int(m_vtTraceData_Fitting[i].y + 2));
			pDC->MoveTo(m_pMain->m_vtTraceData_Fitting[i].x - 5, m_pMain->m_vtTraceData_Fitting[i].y);
			pDC->LineTo(m_pMain->m_vtTraceData_Fitting[i].x + 5, m_pMain->m_vtTraceData_Fitting[i].y);
			pDC->MoveTo(m_pMain->m_vtTraceData_Fitting[i].x, m_pMain->m_vtTraceData_Fitting[i].y - 5);
			pDC->LineTo(m_pMain->m_vtTraceData_Fitting[i].x, m_pMain->m_vtTraceData_Fitting[i].y + 5);
		}
	}

	m_pMain->m_vtTraceData.clear();
	for (int i = 0; i < ptrInterSectionPoint.size(); i++)
	{
		cv::Point2f temp_pt = ptrInterSectionPoint[i];
		m_pMain->m_vtTraceData.push_back(temp_pt);
	}

	if (mViewer != NULL)	mViewer->Invalidate();

	penROI.DeleteObject();
	penE.DeleteObject();
	penS.DeleteObject();
	penX.DeleteObject();
	penRed.DeleteObject();
}
int  CFormMainView::InspDustBurrInspection(int nJob, const cv::Mat& gray, int binThresh, int xo, int yo, double ro, int minSize, int threshSize, bool bclr, bool preInsp)
{
	if (bclr)	m_vecDustResult.clear();

	int rV = 0;
	vector<vector<cv::Point>> contours;
	vector<cv::Vec4i> hierarchy;

	cv::Mat imagemean, Mask3, Mask, Mask2;
	cv::Mat resultImg, iproc, EnLarger, overFlowImg;

	int method 			= m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod();
	double spec_S 		= m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(1); //최소 크기
	double spec_A 		= m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(1); //최소 면적
	double lineratio 	= m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSubSpec(3); // 선형 선분 과검 제거 용

	if (method == METHOD_CIRCLE)		Mask2 = GetCirclemask(gray, gray.cols / 2, gray.rows / 2, gray.cols / 2 - 10);
	else
	{
		Mask2 = cv::Mat::ones(gray.size(), CV_8UC1);
		Mask2 = 255;
	}

	if (preInsp)
	{
		//threshold(gray, Mask3, hthresh, 255, CV_THRESH_BINARY);
		Mask3 = GetCirclemask(gray, gray.cols / 2, gray.rows / 2, gray.cols / 2 - 40, false); //250um 영역만 검사
		//bitwise_not(Mask3, Mask3);
		bitwise_and(Mask2, Mask3, Mask2);

		//Pos3
		spec_S 		= m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(2); //최소 크기
		spec_A 		= m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(2); //최소 면적
		lineratio 	= m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSubSpec(2);  // 선형 선분 과검 제거용 4
	}

	//threshold(gray, imagemean, 1, 255, CV_THRESH_BINARY);
	//blur(imagemean, imagemean, cv::Size(13, 13));
	//threshold(imagemean, Mask2, 250, 255, CV_THRESH_BINARY);

	GaussianBlur(gray, iproc, cv::Size(0, 0), 3.);
	m_pMain->m_ManualAlignMeasure.InspectionEnhance(&iproc, 0, false);

	if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
	{
		cvShowImage("Enhance Image", iproc);
	}

	if (binThresh > 128)		threshold(iproc, iproc, binThresh, 255, CV_THRESH_BINARY);
	else						threshold(iproc, iproc, binThresh, 255, CV_THRESH_BINARY_INV);

	bitwise_and(Mask2, iproc, Mask);
	cvtColor(gray, resultImg, CV_GRAY2RGB);	
	
	if (method == METHOD_LINE) 	fnRemoveNoise(Mask, 20);
	else 						fnRemoveNoise(Mask, 40);

	if (preInsp)				cv::erode(Mask, Mask, cv::Mat::ones(cv::Size(3, 3), CV_8UC1), cv::Point(-1, -1), 1);

	////////// Overflow검사시 사용
	//cv::dilate(Mask2, EnLarger, cv::Mat::ones(cv::Size(3, 3), CV_8UC1), cv::Point(-1, -1), 40);
	//bitwise_xor(EnLarger, Mask2, EnLarger);
	//gray.copyTo(overFlowImg, EnLarger);
	//////////	Overflow검사시 사용

	if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
	{
		cvShowImage("Mask Image", Mask);
		cvShowImage("Mask2 Image", Mask2);
		cvShowImage("Proc Image", iproc);
		cvShowImage("Gray Image", gray);
	}
	contours.clear();
	hierarchy.clear();

	findContours(Mask, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
	cv::Mat drawing = cv::Mat::zeros(Mask.size(), CV_8U);

	int j = 0;

	for (int i = 0; i < contours.size(); i++)
	{
		cv::Moments moms = moments(cv::Mat(contours[i]));
		double area = moms.m00;

		if (area > minSize && area < threshSize * 2)
		{
			drawContours(drawing, contours, i, cv::Scalar(255), CV_FILLED, 8, hierarchy, 0, cv::Point());
			j = j + 1;
		}
	}

	//cvShowImage("drawing", drawing);  // kjh test

	cv::Mat element15(3, 3, CV_8U, cv::Scalar::all(1));
	cv::Mat close;
	morphologyEx(drawing, close, cv::MORPH_CLOSE, element15);

	//cvShowImage("close_D", close);  // kjh test

	vector<vector<cv::Point> > contours1;
	vector<cv::Vec4i> hierarchy1;
	findContours(close, contours1, hierarchy1, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

	vector<contourStats> crackStats(contours1.size());
	double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(0, 0);
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);

	j = 0;
	int m = 0;
	
	for (int i = 0; i < contours1.size(); i++)
	{
		cv::Moments moms = moments(cv::Mat(contours1[i]));
		double area = moms.m00;
		double area1 = contourArea(contours1[i]);

		crackStats[i].calculateStats(contours1[i]);

		double ratio = crackStats[i].rr.boundingRect().width > 0 ? double(crackStats[i].rr.boundingRect().height) / crackStats[i].rr.boundingRect().width : 1;
		
		if(crackStats[i].rr.boundingRect().width > crackStats[i].rr.boundingRect().height)
			ratio = crackStats[i].rr.boundingRect().height > 0 ? double(crackStats[i].rr.boundingRect().width) / crackStats[i].rr.boundingRect().height : 1;
		
		crackStats[i].bdefect = false;

		if (!preInsp) // 액튐 검사시 홀과 메탈 영역 스펙 판정을 나누어서 하도록 수정
		{
			double R = m_dCircleRadius_CC /ro;
			double X = (m_ptCircleCenter_CC.x-xo) /ro;
			double Y = (m_ptCircleCenter_CC.y-yo) /ro;
			bool bcircelIn = (pow(R, 2) >= (pow(X - crackStats[i].center.x, 2) + pow(Y - crackStats[i].center.y, 2))) ? true : false;

			if (bcircelIn)
			{
				minSize = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(1) / xres/ ro;// 0.15;
				threshSize = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(1) / xres / ro; //1;
				lineratio = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSubSpec(3); //2.5
			}
			else
			{
				minSize = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(4) / xres / ro;// 0.5;
				threshSize = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(4) / xres / ro; //1;
				lineratio = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSubSpecTolerance(3); //2.0
			}
		}

		if (area > minSize && area < threshSize && ratio < lineratio)
		{
			drawContours(resultImg, contours1, i, cv::Scalar(0, 0, 255), CV_FILLED, 8, hierarchy1, 0, cv::Point());
			j = j + 1;
			crackStats[i].bdefect = true;
		}
		else
		{
			drawContours(resultImg, contours1, i, cv::Scalar(255, 0, 0), CV_FILLED, 8, hierarchy1, 0, cv::Point());
			m = m + 1;
			crackStats[i].bdefect = false;
		}
	}

	if (crackStats.size() > 0)
	{
		CRect d_Rect;
		cv::RotatedRect cvR;

		if (!preInsp)
		{
			spec_S = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(5); //최소 크기
			spec_A = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(5); //최소 면적
		}
		else
		{
			spec_S = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(2); //최소 크기
			spec_A = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(2); //최소 면적
		}

		for (int i = 0; i< int(crackStats.size()); i++)
		{
			//KJH 2022-02-01 검사 기준 변경 boundingRect Size -> crackStats 
			double x = crackStats[i].rr.boundingRect().width	* ro * xres;
			double y = crackStats[i].rr.boundingRect().height	* ro * yres;

			if (crackStats[i].bdefect && ((spec_S < x && x < spec_A) && (spec_S < y && y < spec_A)))
			{
				d_Rect = findRawImageSize(gray, crackStats[i].center, crackStats[i].rr.boundingRect().width, crackStats[i].rr.boundingRect().height);

				cvR = RotatedRect(crackStats[i].center, cv::Size2f(d_Rect.Width() * ro, d_Rect.Height() * ro), crackStats[i].rr.angle+(fabs(crackStats[i].rr.angle)>1?90:0));
				crackStats[i].rr = cvR;
				crackStats[i].xw = d_Rect.Width() * ro;
				crackStats[i].yw = d_Rect.Height() * ro;

				m_vecDustResult.push_back(crackStats.at(i));
				rV++;
			}
		}
	}

	vector<contourStats>::iterator itor = crackStats.begin();

	CString msg, strFileDir, imgMsg, strImgDir, str_ImageType;
	SYSTEMTIME time;
	GetLocalTime(&time);
	//KMB 220924 SYSTEM OPTION에 따라 저장이미지 확장자 변경
	if (m_pMain->vt_system_option[nJob].save_image_type == FALSE)
		str_ImageType = "jpg";
	else
		str_ImageType = "bmp";

	strFileDir = m_pMain->m_strResultDir + "Files\\";
	if (_access(strFileDir, 0) != 0)	CreateDirectory(strFileDir, NULL);
	if (_access(m_pMain->m_strImageDir, 0) != 0)	CreateDirectory(m_pMain->m_strImageDir, NULL);

	if (m_pMain->m_strResultDate[0] == "")
	{
		CTime NowTime;
		NowTime = CTime::GetCurrentTime();
		m_pMain->m_strResultDate[nJob].Format("%04d%02d%02d", NowTime.GetYear(), NowTime.GetMonth(), NowTime.GetDay());
	}

	//PanelID
	if (m_pMain->vt_job_info[nJob].main_object_id.size() <= 0) m_pMain->vt_job_info[nJob].main_object_id = "TestImg";

	strFileDir = strFileDir + m_pMain->m_strResultDate[nJob];
	strImgDir  = m_pMain->m_strImageDir + m_pMain->m_strResultDate[nJob];
	if (_access(strFileDir, 0) != 0)	CreateDirectory(strFileDir, NULL);
	if (_access(strImgDir, 0) != 0)		CreateDirectory(strImgDir, NULL);

	if (preInsp)
	{
		imgMsg.Format("%s\\DustBurrInspResult_%s_%02d%02d%02d.%s", strImgDir, m_pMain->vt_job_info[nJob].main_object_id.c_str(), time.wHour, time.wMinute, time.wSecond, str_ImageType);
		msg.Format("%s\\DustBurrInspResult_%s_%02d%02d%02d.csv", strFileDir, m_pMain->vt_job_info[nJob].main_object_id.c_str(), time.wHour, time.wMinute, time.wSecond);
	}
	else
	{
		imgMsg.Format("%s\\DispensingInspResult_%s_%02d%02d%02d.%s", strImgDir, m_pMain->vt_job_info[nJob].main_object_id.c_str(), time.wHour, time.wMinute, time.wSecond, str_ImageType);
		msg.Format("%s\\DispensingInspResult_%s_%02d%02d%02d.csv", strFileDir, m_pMain->vt_job_info[nJob].main_object_id.c_str(), time.wHour, time.wMinute, time.wSecond);
	}

	ofstream  writeFile(msg);

	//KJH 2022-02-03 DustInsp관련 불필요 로그 저장 개선
	if (rV > 0)
	{
		if (writeFile.is_open())
		{
			BOOL maketitle = false;

			j = 0;
			for (; itor != crackStats.end(); itor++)
			{
				if (itor->bdefect)
				{
					if (!maketitle)
					{
						msg.Format("Index, X, Y, Angle, Area,Width,Height, Circularity, Perimeter\n");		writeFile << msg;
						maketitle = TRUE;
					}
					msg.Format("%4d, %4.3f, %4.3f, %4.3f, %4.3f, %4.3f, %4.3f,  %4.3f, %4.3f\n", ++j, itor->center.x * ro + xo, itor->center.y * ro + yo, itor->area * ro, itor->rr.angle,
						itor->xw * ro * xres, itor->yw * ro * yres, itor->circularity, itor->perimeter * ro);
					writeFile << msg;
					msg.Format("[%4d] xw : %4.3f, yw : %4.3f, area : %4.3f", j - 1, itor->xw * ro * xres, itor->yw * ro * yres, itor->area * ro);

					cv::putText(resultImg, std::string(msg), cv::Point(itor->center.x, itor->center.y), cv::FONT_ITALIC, 1, cv::Scalar(0, 255, 0), 1);
				}
			}

			writeFile.close();
		}
		
		cv::imwrite(std::string(imgMsg), resultImg);
	}

	crackStats.clear();

	imagemean.release();
	Mask.release();
	Mask2.release();
	Mask3.release();
	resultImg.release();
	iproc.release();
	EnLarger.release();
	overFlowImg.release();

	drawing.release();
	element15.release();
	close.release();

	return rV;
}

CRect CFormMainView::findRawImageSize(cv::Mat srcGray, cv::Point2d pt, int width, int height)
{
	cv::Mat bin2;
	cv::Rect imgRoi = cv::Rect(pt.x - width / 2, pt.y - height / 2, width, height);

	imgRoi.x = MAX(0, imgRoi.x);
	imgRoi.y = MAX(0, imgRoi.y);
	imgRoi.width = MIN(srcGray.cols - imgRoi.x, width);
	imgRoi.height = MIN(srcGray.rows - imgRoi.y, height);

	cv::threshold(srcGray(imgRoi), bin2, -1, 255, CV_THRESH_BINARY_INV | CV_THRESH_OTSU);//

	CRect r(imgRoi.x, imgRoi.y, imgRoi.x + imgRoi.width, imgRoi.y + imgRoi.height);

	vector<vector<cv::Point>> contours;
	vector<cv::Vec4i> hierarchy;

	findContours(bin2, contours, hierarchy, CV_RETR_TREE, CV_CHAIN_APPROX_SIMPLE, cv::Point(0, 0));
	int ID = getMaxAreaContourId(contours);
	if (ID != -1)
	{
		cv::Rect bounding_rect = boundingRect(contours[ID]);
		r.left = bounding_rect.x + imgRoi.x;
		r.top = bounding_rect.y + imgRoi.y;
		r.right = bounding_rect.x + bounding_rect.width + imgRoi.x;
		r.bottom = bounding_rect.y + bounding_rect.height + imgRoi.y;
	}

	contours.clear();
	hierarchy.clear();

	return r;
}

BOOL CFormMainView::InspCircleEdgeDetection(int nJob, const cv::Mat& src, CPoint ptStart, CPoint ptEnd, CViewerEx* mViewer)
{
	//Center Align용 Circle 찾기(중심점 필요)
	
	//HTK 2022-03-30 Tact Time Display 추가
	m_clockProcStartTime[2] = clock();

	FakeDC* pDC = NULL;
	CPen penS(PS_SOLID, 3, RGB(255, 255, 0));
	CPen penE(PS_SOLID, 3, RGB(255, 0, 0));
	BOOL breturn = TRUE;

	if (mViewer != NULL)
	{
		mViewer->ClearOverlayDC();
		pDC = mViewer->getOverlayDC();
		//pDC->SelectObject(&penE);
		pDC->SetColor(Gdiplus::Color::Red);
	}

	std::vector<cv::Vec3f> circles;
	cv::Mat color, gray;
	double xres = MAX(0.00001, m_pMain->GetMachine(nJob).getCameraResolutionX(0, 0));
	double l = MIN(5., m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspCamHoleSize()); //도포 길이
	double camHole = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspCamHoleSize();

	int method = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspectionMethod();
	int hthresh = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionHighThresh();
	int minCircle = int(m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspInHoleSize() / xres);

	if (method == DISTANCE_INSP_METHOD_AUTO)
	{
		std::vector<std::vector<cv::Point>> contours;
		std::vector<cv::Vec4i> hierarchy;

		cv::Mat bin;
		cv::Rect bounding_rect = cv::Rect(ptStart.x, ptStart.y, ptEnd.x - ptStart.x, ptEnd.y - ptStart.y);
		cv::threshold(src(bounding_rect), bin, -1, 255, CV_THRESH_BINARY_INV | THRESH_OTSU);
		cv::findContours(bin, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

		int ID = getMaxAreaContourId(contours);
		if (ID != -1)
		{
			bounding_rect = boundingRect(contours[ID]);

			int lw = bounding_rect.width; //l/ xres;//
			int lh = bounding_rect.height; //l / xres;// 

			ptStart.x = MIN(src.cols - 1, MAX(0, bounding_rect.x + ptStart.x));
			ptStart.y = MIN(src.rows - 1, MAX(0, bounding_rect.y + ptStart.y));
			ptEnd.x = MIN(src.cols - 1, MAX(0, ptStart.x + lw));
			ptEnd.y = MIN(src.rows - 1, MAX(0, ptStart.y + lh));
		}

		contours.clear();
		hierarchy.clear();
		bin.release();
	}

	ptStart.x = MIN(src.cols - 1, MAX(0, ptStart.x));
	ptStart.y = MIN(src.rows - 1, MAX(0, ptStart.y));
	ptEnd.x = MIN(src.cols - 1, MAX(0, ptEnd.x));
	ptEnd.y = MIN(src.rows - 1, MAX(0, ptEnd.y));

	gray = src(cv::Rect(ptStart.x, ptStart.y, ptEnd.x - ptStart.x, ptEnd.y - ptStart.y));
	cv::cvtColor(gray, color, CV_GRAY2BGR);

	m_dInclination = 0.0; 			// Adaptive Dispensing 사용 기울기 20220103 Tkyuha
	m_dADispesing_StartAngle = 0; 	// Adaptive Dispensing 사용 시작 각도 20220103 Tkyuha
	m_dADispesing_EndAngle = 359; 	// Adaptive Dispensing 사용 종료 각도 20220103 Tkyuha

	bool bauto = method == DISTANCE_INSP_METHOD_AUTO ? true : false;
	if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseMetalTraceFirst()) // Tkyuha 20211216 메탈로 얼라인 하기 위함
	{
		bool _bremoveNoise = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getMetalSearchRemoveNoiseFlag();
		camHole = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspSteelHoleSize();
		l = MIN(5., camHole);
		bool bELBCoverMode = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getELBCoverCInkMode();
		//FindCircleAlign_MP(gray, ptStart.x, ptStart.y, int(l / xres), minCircle, circles, bauto, hthresh, _bremoveNoise,pDC, bELBCoverMode);
		m_pMain->m_dbCirceErrorLossRate = FindCircleAlign_MP(gray, ptStart.x, ptStart.y, int(l / xres), minCircle, circles, &m_vtEdgePosition, bauto, hthresh,_bremoveNoise,pDC, bELBCoverMode);
		
	    if(m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getAdaptiveDispensingFlag())
			m_dInclination = FindMetalToInkDistance(gray, ptStart.x, ptStart.y, circles,m_dADispesing_StartAngle, m_dADispesing_EndAngle, pDC); // 메탈과 잉크 사이 거리 측정후 기울기 계산
	}
	else
		m_pMain->m_dbCirceErrorLossRate = FindCircleAlign_PN(gray, ptStart.x, ptStart.y, int(l / xres), minCircle, circles, &m_vtEdgePosition, bauto, hthresh, pDC);
		//FindCircleAlign_PN(gray, ptStart.x, ptStart.y, int(l / xres), minCircle, circles, bauto, hthresh, pDC);

	if (circles.size() >= 1) breturn = TRUE;
	else breturn = FALSE;
	BYTE* sdata = src.data;

	if (pDC != NULL)
	{
		for (size_t i = 0; i < circles.size(); i++)
		{
			cv::Point center(cvRound(circles[i][0] + ptStart.x), cvRound(circles[i][1] + ptStart.y));
			int radius = cvRound(circles[i][2]);

			mViewer->AddSoftGraphic(new GraphicPoint(center.x, center.y, Gdiplus::Color::Red, 5, 3));
			pDC->SetColor(Gdiplus::Color::Yellow);
			//pDC->Ellipse(int(center.x - radius), int(center.y - radius), int(center.x + radius), int(center.y + radius));

			// 얼라인 값 계산
			if (i == 0)
			{
				m_pMain->m_ELB_TraceResult.m_ELB_ResultXY = cv::Point2d(center.x, center.y);
				CIRCLE_RADIUS = radius;
				m_pMain->m_ELB_TraceResult.m_nRadius = radius;
			}
		}

		double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(0, 0);
		double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);

		//CString str_data;
		//pDC->SetTextColor(COLOR_WHITE);

		//str_data.Format("[CIRCLE ALIGN] Find Circle R: %.3fmm , DiffRate = %.5f", CIRCLE_RADIUS * xres, m_pMain->m_dbCirceErrorLossRate); // Tkyuha 221108 에러율 표기
		//pDC->TextOutA(int(m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x + 10), int(m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y + 200), str_data);

		double posX = m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x;
		double posY = m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y;

		//if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseMetalTraceFirst())
		//{
		//	m_dCircleRadius_MP = m_pMain->m_ELB_TraceResult.m_nRadius;
		//	m_ptCircleCenter_MP.x = (int)m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x;
		//	m_ptCircleCenter_MP.y = (int)m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y;
		//}

		//mViewer->AddSoftGraphic(new GraphicRectangle(CRect(posX - CIRCLE_RADIUS, posY - CIRCLE_RADIUS, posX + CIRCLE_RADIUS, posY + CIRCLE_RADIUS), Gdiplus::Color(0xffff0000), 2));

		////////////////////////////////////// 2022-05-07 KBJ gray average
		// area
		int size_pixcel = 10;

		CRect rect_area;
		rect_area.left = posX - size_pixcel;
		rect_area.right = posX + size_pixcel;
		rect_area.top = posY - size_pixcel;
		rect_area.bottom = posY + size_pixcel;
				
		//2022.10.04 ksm Inspection 이미지 사이즈 안맞으면 죽는현상 예외처리
		if (rect_area.left < 0)
			rect_area.left = 0;

		if (rect_area.right > src.rows)
			rect_area.right = src.rows - 1;

		if (rect_area.top < 0)
			rect_area.top = 0;

		if (rect_area.bottom > src.cols)
			rect_area.bottom = src.cols -1;

		//// gray
		//cv::Mat InspImg = src(cv::Rect(rect_area.left, rect_area.top, rect_area.Width(), rect_area.Height()));
		//cv::Scalar scalar = cv::mean(InspImg);

		//// draw
		//pDC->Rectangle(rect_area);

		//str_data.Format("Gray: %.1f", scalar.val[0]);
		//pDC->TextOutA(rect_area.left, rect_area.top - 5, str_data);
		///////////////////////////////////////

		/*
		bool bflag = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getCircleFindCenterCheck();

		if (bflag && (CIRCLE_RADIUS <camHole / xres - 30 || CIRCLE_RADIUS > camHole / xres + 30))
		{
			CString str;
			str.Format("[CIRCLE ALIGN] Find Circle Error - (30 pixel) %f", CIRCLE_RADIUS - (camHole / xres));
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
			breturn = false;

			m_pMain->m_nErrorType[nJob] = ERR_TYPE_CIRCLE_ERROR;
		}*/
		//Tkyuha 2022-04-13 PN 반지름 비교 검사 추가 , 30pixel 검사 대신 반지름 크기로 대체함
		bool bPNSizeInspEnable = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getPNSizeInspEnable();
		double PNSizespec = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getPNSizeSpec() / xres;
		double PNSizespectorr = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getPNSizeSpecTorr() / xres;

		double PNSizeSpecMin = PNSizespec - PNSizespectorr;
		double PNSizeSpecMax = PNSizespec + PNSizespectorr;;

		if (bPNSizeInspEnable && (m_pMain->m_ELB_TraceResult.m_nRadius > PNSizeSpecMax || m_pMain->m_ELB_TraceResult.m_nRadius < PNSizeSpecMin))
		{
			CString str;
			str.Format("[%s] PN Size Error - %.3fmm / [Spec] - %.3fmm", m_pMain->vt_job_info[nJob].job_name.c_str(), m_pMain->m_ELB_TraceResult.m_nRadius* xres, PNSizespec* xres);
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
			breturn = false;
			ShowErrorMessageBlink(str);
			m_pMain->m_nErrorType[nJob] = ERR_TYPE_CIRCLE_ERROR;
		}

		////////////////////////////////////////////////////////////////////////////////////////////////
		m_pMain->m_dCurrentSearchPos[0] = m_pMain->m_dCurrentSearchPos[2];
		m_pMain->m_dCurrentSearchPos[1] = m_pMain->m_dCurrentSearchPos[3];
		m_pMain->m_dCurrentRobotSearchPos[0] = m_pMain->m_dCurrentRobotSearchPos[2];
		m_pMain->m_dCurrentRobotSearchPos[1] = m_pMain->m_dCurrentRobotSearchPos[3];
		m_pMain->m_dCurrentSearchPos[2] = m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x;
		m_pMain->m_dCurrentSearchPos[3] = m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y;

		::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_SET_SEARCH_POS, NULL);
	}

	if (pDC != NULL)	mViewer->Invalidate();

	penS.DeleteObject();
	penE.DeleteObject();

	circles.clear();
	color.release();
	gray.release();

	//HTK 2022-03-30 Tact Time Display 추가
	m_clockProc_elapsed_time[2] = (clock() - m_clockProcStartTime[2]) / (float)CLOCKS_PER_SEC;
	((CPaneAuto*)m_pMain->m_pPane[PANE_AUTO])->update_grid_processing_time(1, m_clockProc_elapsed_time[2], 2, true);

	//KJH 2022-07-27 Tact Time Log 추가
	theLog.logmsg(LOG_PLC_SEQ, "Center Align Tact Time = %.4f", m_clockProc_elapsed_time[2]);
	theLog.logmsg(LOG_INSPECTOR, "Center Align Search Rate = %.4f", m_pMain->m_dbCirceErrorLossRate); //20221122 Tkyuha Circle 에러율 로그저장

	return breturn;
}
BOOL CFormMainView::InspCircleMetalEdgeDetection(int nJob, int nCam, const cv::Mat& src, CViewerEx* mViewer)
{
	BOOL breturn = TRUE;
	CString str_data ="";
	CString str_data1 = "";
	
	int method = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspectionMethod();

	int W = m_pMain->m_stCamInfo[nCam].w;
	int H = m_pMain->m_stCamInfo[nCam].h;

	double posx = 0.0, posy = 0.0;
	double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(0, 0);
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);
	int x = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeX() / xres / 2; //  중심점 기준 좌우 분리
	int y = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeY() / yres / 2;  //  중심점 기준 상하 분리
	double l = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeLength(); //도포 길이
	//int cx = W / 2, cy = H / 2;
	int cx = m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x;
	int cy = m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y;

	//검사 홀 영역 Search
	double camhole = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspCamHoleSize() / xres - 20;
	double steelhole = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspSteelHoleSize() / xres + 20;

	std::vector<cv::Vec3f> circles;
	bool bauto = method == DISTANCE_INSP_METHOD_AUTO ? true : false;
	int hthresh = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionHighThresh();

	x = steelhole, y = steelhole;

	cv::Rect InspRect = cv::Rect(MAX(0, cx - x), MAX(0, cy - y), 2 * x, 2 * y);
	if (InspRect.width + InspRect.x > W - 1) InspRect.width = W - 1 - InspRect.x;
	if (InspRect.height + InspRect.y > H - 1) InspRect.height = H - 1 - InspRect.y;

	cv::Mat imgGray = src(InspRect);
	std::vector<cv::Point> vtMaskContour;

	if (m_pMain->m_vtTraceData.size() > 0)
	{
		cv::Mat maskRect = cv::Mat::zeros(imgGray.rows, imgGray.cols, CV_8U);
		cv::Point2f rC;

		double rad = CIRCLE_RADIUS;// fabs(m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y - m_pMain->m_vtTraceData[0].y) + 20;

		for (int i = 0; i < 360; i++)
		{
			rC.x = (cos(ToRadian(i)) * rad + m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x) - InspRect.x;
			rC.y = (sin(ToRadian(i)) * rad + m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y) - InspRect.y;

			vtMaskContour.push_back(rC);
		}
		vtMaskContour.push_back(vtMaskContour[0]);

		rad = CIRCLE_RADIUS + 120;// fabs(m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y - m_pMain->m_vtTraceData[0].y) + 120;
	
		for (int i = 0; i < 360; i++)
		{
			rC.x = (cos(ToRadian(i)) * rad + m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x) - InspRect.x;
			rC.y = (sin(ToRadian(i)) * rad + m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y) - InspRect.y;

			vtMaskContour.push_back(rC);
		}
		vtMaskContour.push_back(vtMaskContour[361]);

		int num = (int)vtMaskContour.size();
		const cv::Point* pt4 = &(vtMaskContour[0]);
		cv::fillPoly(maskRect, &pt4, &num, 1, cv::Scalar(255, 255, 255), 8);
		cv::GaussianBlur(maskRect, maskRect, cv::Size(0, 0), 3.);
		// 20210923 제외 영역 사용 하기 위함
		findMultipleCircle(imgGray, circles, camhole, steelhole, CIRCLE_RADIUS+30, hthresh, bauto, &maskRect, m_Index_start, m_Index_end, m_Index_bsplit);
		vtMaskContour.clear();
	}
	else 	findMultipleCircle(imgGray, circles, camhole, steelhole, CIRCLE_RADIUS+30, hthresh, bauto, NULL, m_Index_start, m_Index_end, m_Index_bsplit);
	// 20210923 제외 영역 사용 하기 위함 다음 사용을 위해 초기화
	m_Index_start = m_Index_end = -1;
	m_Index_bsplit = false;
	m_IndexMaxWidth_Angle = 0;

	m_dCircleDistance_X_MP = 0;
	if (circles.size() >= 1)
	{
		m_dCircleRadius_MP = circles[0][2];
		m_ptCircleCenter_MP = CPoint(cvRound(circles[0][0] + InspRect.x), cvRound(circles[0][1] + InspRect.y));

		CPointF<double> p_center(circles[0][0] + InspRect.x, circles[0][1] + InspRect.y);
		CPointF<double> p_center2(W / 2, H / 2);
		m_dCircleDistance_X_MP = GetDistance(p_center, p_center2) * xres;

		if (mViewer != NULL)
		{
			FakeDC* pDC = NULL;
			CPen penROI(PS_SOLID, 3, RGB(255, 125, 0));
			

			pDC = mViewer->getOverlayDC();			
			pDC->SelectObject(&penROI);
			
			pDC->Ellipse(int(m_ptCircleCenter_MP.x - m_dCircleRadius_MP), int(m_ptCircleCenter_MP.y - m_dCircleRadius_MP),
				int(m_ptCircleCenter_MP.x + m_dCircleRadius_MP), int(m_ptCircleCenter_MP.y + m_dCircleRadius_MP));
			pDC->Ellipse(int(p_center.x - 4), int(p_center.y - 4), int(p_center.x + 4), int(p_center.y + 4));

			CPointF<double> _center(m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x, m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y);
			double m_dCircleDistance_X_PN = GetDistance(_center, p_center2) * xres;

			/*pDC->SetTextColor(COLOR_BLUE);
			str_data.Format("[Center GAP] MP : %.3fmm , PN : %.3fmm", m_dCircleDistance_X_MP, m_dCircleDistance_X_PN);
			pDC->TextOutA(int(p_center.x), int(p_center.y), str_data);
			str_data.Format("[PN Radius] : %.3fmm", CIRCLE_RADIUS * xres);
			pDC->TextOutA(int(m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x + 10), int(m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y + 100), str_data);*/

			str_data.Format("[Center GAP] : %.3fmm", m_dCircleDistance_X_PN);
			//pDC->TextOutA(int(m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x + 10), int(m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y + 100), str_data);
			str_data1.Format("[PN Radius]  : %.3fmm", CIRCLE_RADIUS * xres);
			//pDC->TextOutA(int(m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x + 10), int(m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y + 000), str_data);

			// KBJ 2022-11-25 Bule -> Green
			//mViewer->AddSoftGraphic(new GraphicLabel(m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x, m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y, fmt("%s\n%s", str_data, str_data1), Gdiplus::Color::Blue));
			mViewer->AddSoftGraphic(new GraphicLabel(m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x, m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y, fmt("%s\n%s", str_data, str_data1), Gdiplus::Color(0xff00ff00)));

			penROI.DeleteObject();

			mViewer->Invalidate();
			mViewer->DirtyRefresh();
		}
	}
	else
	{
		m_dCircleRadius_MP = steelhole - 20;
		m_ptCircleCenter_MP = CPoint(W / 2, H / 2);
	}

	CPointF<int> p_camcenter(m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x, m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y);

	for (int angle = 0; angle < 360; angle += 45)
	{
		int _x = int(cos(angle * CV_PI / 180.) * m_dCircleRadius_MP + (m_ptCircleCenter_MP.x));
		int _y = int(sin(angle * CV_PI / 180.) * m_dCircleRadius_MP + (m_ptCircleCenter_MP.y));

		CPointF<int> p_scenter(_x, _y);
		double _vs = GetDistance(p_camcenter, p_scenter);

		if (_vs < CIRCLE_RADIUS)
		{
			str_data.Format("[%s] Circle Trace Search Error - %.3fmm - %.3fmm, ", m_pMain->vt_job_info[nJob].job_name.c_str(), _vs, CIRCLE_RADIUS);
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str_data);
			break;
		}
	}

	circles.clear();
	imgGray.release();

	return breturn;
}
BOOL CFormMainView::InspCircleEdgeDetection_CC(int nJob, int nCam, const cv::Mat& src, CViewerEx* mViewer, BOOL reInsp)
{
	BOOL breturn = TRUE;

	CString str_data;

	int W = m_pMain->m_stCamInfo[nCam].w;
	int H = m_pMain->m_stCamInfo[nCam].h;

	double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(0, 0);
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);
	double BMhole = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspInHoleSize() / xres + 20;

	cv::Mat imgGray = src;

	m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][C_CALIPER_POS_1].processCaliper_circle(imgGray.ptr(), W, H, 0.0, 0.0, 0.0, TRUE);

	BOOL bFindLine = m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][C_CALIPER_POS_1].getIsMakeLine();
	sCircle _circle = m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][C_CALIPER_POS_1].m_circle_info;

	// 디버깅 모드에서 이미지 보기 //20220117 Tkyuha
	if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
	{
		cvShowImage("BMInspection_Image", imgGray);
	}

	if (bFindLine)
	{
		m_dCircleRadius_CC = _circle.r;
		m_ptCircleCenter_CC = CPoint(cvRound(_circle.x), cvRound(_circle.y));

		if (mViewer != NULL)
		{
			FakeDC* pDC = NULL;
			CPen penROI(PS_SOLID, 3, RGB(255, 255, 0));
			CPen penGreen(PS_SOLID, 3, RGB(0, 255, 0));
			CFont font, * oldFont;
			CString str_data;
			int OLD_BKMODE = 0;

			pDC = mViewer->getOverlayDC();
			font.CreateFont(40, 40, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
				OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
			oldFont = pDC->SelectObject(&font);
			pDC->SelectObject(&penGreen);
			OLD_BKMODE = pDC->SetBkMode(TRANSPARENT);
			pDC->SelectObject(GetStockObject(NULL_BRUSH));
			pDC->Ellipse(int(m_ptCircleCenter_CC.x - m_dCircleRadius_CC), int(m_ptCircleCenter_CC.y - m_dCircleRadius_CC),
				int(m_ptCircleCenter_CC.x + m_dCircleRadius_CC), int(m_ptCircleCenter_CC.y + m_dCircleRadius_CC));

			if (m_leftRight.x != 0 && m_leftRight.y != 0)
			{
				int cx = (m_leftRight.x + m_leftRight.y) / 2;
				int cy = (m_topBottom.x + m_topBottom.y) / 2;
				pDC->SelectObject(&penGreen);
				pDC->Ellipse(int(cx - 4), int(cy - 4), int(cx + 4), int(cy + 4));

				double left		= (m_leftRight.x - (W / 2 - CIRCLE_RADIUS)) * xres;
				double right	= ((W / 2 + CIRCLE_RADIUS) - m_leftRight.y) * yres;
				double top		= (m_topBottom.x - (H / 2 - CIRCLE_RADIUS)) * xres;
				double bottom	= ((H / 2 + CIRCLE_RADIUS) - m_topBottom.y) * yres;

				//KJH 2022-05-06 CC to PN 검사 Display 삭제
				//pDC->SetTextColor(COLOR_RED);
				//str_data.Format("[CC <-> PN] L : %.3fmm, R : %.3fmm, T : %.3fmm, B : %.3fmm", left, right, top, bottom);
				//pDC->TextOutA(int(W / 2), int(H / 2 - 100), str_data);
			}

			penROI.DeleteObject();
			penGreen.DeleteObject();
			font.DeleteObject();
			pDC->SetBkMode(OLD_BKMODE);

			mViewer->Invalidate();
		}
	}
	else
	{
		if (mViewer != NULL)
		{
			FakeDC* pDC = NULL;
			CPen penROI(PS_SOLID, 3, RGB(255, 255, 0));
			CPen penGreen(PS_SOLID, 3, RGB(0, 255, 0));
			CFont font, * oldFont;
			int OLD_BKMODE = 0;

			pDC = mViewer->getOverlayDC();
			font.CreateFont(40, 40, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
				OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
			oldFont = pDC->SelectObject(&font);
			pDC->SelectObject(&penGreen);
			OLD_BKMODE = pDC->SetBkMode(TRANSPARENT);
			pDC->SelectObject(GetStockObject(NULL_BRUSH));

			str_data.Format("[CC <-> PN] Do Not Find CC Circle");
			pDC->TextOutA(int(W / 2 - 200), int(H / 2 - 100), str_data);

			penROI.DeleteObject();
			penGreen.DeleteObject();
			font.DeleteObject();
			pDC->SetBkMode(OLD_BKMODE);

			mViewer->Invalidate();

		}
		m_dCircleRadius_CC = BMhole - 20;
		m_ptCircleCenter_CC = CPoint(W / 2, H / 2);
	}

	return breturn;
}
BOOL CFormMainView::InspCaliperCircleBMEdgeDetection(int nJob, int nCam, const cv::Mat& src, CViewerEx* mViewer, BOOL reInsp)
{
	BOOL breturn = TRUE;

	CString str_data;

	int W = m_pMain->m_stCamInfo[nCam].w;
	int H = m_pMain->m_stCamInfo[nCam].h;

	double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(0, 0);
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);
	double BMhole = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspInHoleSize() / xres + 20;

	cv::Mat imgGray = src;

	m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][C_CALIPER_POS_2].processCaliper_circle(imgGray.ptr(), W, H, 0.0, 0.0, 0.0, TRUE);

	BOOL bFindLine = m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][C_CALIPER_POS_2].getIsMakeLine();
	sCircle _circle = m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][C_CALIPER_POS_2].m_circle_info;

	// 디버깅 모드에서 이미지 보기 //20220117 Tkyuha
	if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
	{
		cvShowImage("BMInspection_Image", imgGray);
	}

	if (bFindLine)
	{
		m_dCircleRadius_CC = _circle.r;
		m_ptCircleCenter_CC = CPoint(cvRound(_circle.x), cvRound(_circle.y));

		if (mViewer != NULL)
		{
			FakeDC* pDC = NULL;
			CPen penROI(PS_SOLID, 3, RGB(255, 255, 0));
			CPen penGreen(PS_SOLID, 3, RGB(0, 255, 0));

			CString str_data;

			pDC = mViewer->getOverlayDC();		
			pDC->SelectObject(&penGreen);

			pDC->Ellipse(int(m_ptCircleCenter_CC.x - m_dCircleRadius_CC), int(m_ptCircleCenter_CC.y - m_dCircleRadius_CC),
				int(m_ptCircleCenter_CC.x + m_dCircleRadius_CC), int(m_ptCircleCenter_CC.y + m_dCircleRadius_CC));

			if (m_leftRight.x != 0 && m_leftRight.y != 0)
			{
				int cx = (m_leftRight.x + m_leftRight.y) / 2;
				int cy = (m_topBottom.x + m_topBottom.y) / 2;
				pDC->SelectObject(&penGreen);
				pDC->Ellipse(int(cx - 4), int(cy - 4), int(cx + 4), int(cy + 4));

				double left		= (m_leftRight.x - (W / 2 - CIRCLE_RADIUS)) * xres;
				double right	= ((W / 2 + CIRCLE_RADIUS) - m_leftRight.y) * yres;
				double top		= (m_topBottom.x - (H / 2 - CIRCLE_RADIUS)) * xres;
				double bottom	= ((H / 2 + CIRCLE_RADIUS) - m_topBottom.y) * yres;

				//KJH 2022-05-06 CC to PN 검사 Display 삭제
				//pDC->SetTextColor(COLOR_RED);
				//str_data.Format("[CC <-> PN] L : %.3fmm, R : %.3fmm, T : %.3fmm, B : %.3fmm", left, right, top, bottom);
				//pDC->TextOutA(int(W / 2), int(H / 2 - 100), str_data);
			}

			penROI.DeleteObject();
			penGreen.DeleteObject();

			mViewer->Invalidate();
		}
	}
	else
	{
		if (mViewer != NULL)
		{
			FakeDC* pDC = NULL;
			CPen penROI(PS_SOLID, 3, RGB(255, 255, 0));
			CPen penGreen(PS_SOLID, 3, RGB(0, 255, 0));

			pDC = mViewer->getOverlayDC();			
			pDC->SelectObject(&penGreen);

			str_data.Format("[CC <-> PN] Do Not Find CC Circle");
			pDC->TextOutA(int(W / 2 - 200), int(H / 2 - 100), str_data);

			penROI.DeleteObject();
			penGreen.DeleteObject();

			mViewer->Invalidate();

		}
		m_dCircleRadius_CC = BMhole - 20;
		m_ptCircleCenter_CC = CPoint(W / 2, H / 2);

		breturn = FALSE;
	}

	return breturn;
}
BOOL CFormMainView::InspCircleBMEdgeDetection(int nJob, int nCam, const cv::Mat& src, CViewerEx* mViewer,BOOL reInsp)
{
	BOOL breturn = TRUE;
	CString str_data;
	int method = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspectionMethod();

	int W = m_pMain->m_stCamInfo[nCam].w;
	int H = m_pMain->m_stCamInfo[nCam].h;

	double posx = 0.0, posy = 0.0;
	double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(0, 0);
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);
	int x = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeX() / xres / 2; //  중심점 기준 좌우 분리
	int y = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeY() / yres / 2;  //  중심점 기준 상하 분리
	double l = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeLength(); //도포 길이
	//int cx = W / 2, cy = H / 2;
	int cx = m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x;
	int cy = m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y;

	//검사 홀 영역 Search
	double camhole = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspCamHoleSize() / xres - 20;
	double BMhole = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspInHoleSize() / xres + 20;

	if (reInsp)
	{
		cx 		= m_ptCircleCenter_CC.x;
		cy	 	= m_ptCircleCenter_CC.y;
		BMhole 	= m_dCircleRadius_CC - 80; 
	}

	std::vector<cv::Vec3f> circles;
	bool bauto = method == DISTANCE_INSP_METHOD_AUTO ? true : false;
	int hthresh = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionLowThresh();

	x = BMhole + 100, y = BMhole + 100;
	cv::Rect InspRect = cv::Rect(MAX(0, cx - x), MAX(0, cy - y), 2 * x, 2 * y);
	if (InspRect.width + InspRect.x > W - 1) InspRect.width = W - 1 - InspRect.x;
	if (InspRect.height + InspRect.y > H - 1) InspRect.height = H - 1 - InspRect.y;

	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;

	//Master CC Size보다 100pixel 큰 Rect를 생성하고 이미지를 자른다.
	cv::Mat imgGray = src(InspRect), bin, bin2;
	cv::threshold(imgGray, bin, hthresh, 255, CV_THRESH_BINARY_INV);
	
	bin2 = bin.clone();

	//이미지를 2진화 시켜서 Contour를 적용시켜 Edge만 검출한다. 검출된 이미지는 bin2로 넣는다.
	cv::findContours(bin2, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

	m_leftRight = cv::Point2f(0, 0);
	m_topBottom = cv::Point2f(0, 0);
	
	//contour중 제일 큰 Blob index를 받아온다.
	int ID = getMaxAreaContourId(contours);
	if (ID != -1)
	{
		cv::Rect bounding_rect = boundingRect(contours[ID]);
		m_leftRight = cv::Point2f(bounding_rect.x + InspRect.x, bounding_rect.x + bounding_rect.width + InspRect.x);
		m_topBottom = cv::Point2f(bounding_rect.y + InspRect.y, bounding_rect.y + bounding_rect.height + InspRect.y);
		cv::circle(bin, cv::Point(bounding_rect.x + bounding_rect.width / 2, bounding_rect.y + bounding_rect.height / 2), BMhole, cv::Scalar(255, 255, 255), -1, 8, 0);
	}
	else	cv::circle(bin, cv::Point(imgGray.cols / 2, imgGray.rows / 2), BMhole, cv::Scalar(255, 255, 255), -1, 8, 0);

	cv::Canny(bin, bin, 20, 150);	
	dilate(bin, bin, Mat::ones(5, 5, CV_8U));
	
	uchar* Raw = bin.data;
	uchar* RawSrc = bin2.data;
	std::vector<cv::Point3i> outlierVec;
	CPointF<int> p_center(bin.cols/2, bin.rows/2);
	int  margin = 100,endPoint= reInsp ? 10: -10;
	double sumdistance = 0, avgdistance = 0;
	double rad_std = reInsp ? m_dCircleRadius_CC : CIRCLE_RADIUS;

	bin2 = 0;

	for (int second = 0; second < 360; second++)
	{
		// 원위의 점 찾기
		for (int distanceT = -300; distanceT < endPoint; distanceT++)
		{
			int _rad = rad_std + distanceT;
			int rposx = (cos(ToRadian(second)) * _rad + cx - InspRect.x);
			int rposy = (sin(ToRadian(second)) * _rad + cy - InspRect.y);

			if (rposx < 0 || rposx >= bin.cols) continue;
			if (rposy < 0 || rposy >= bin.rows) continue;

			if (Raw[rposy * bin.cols + rposx] == 255)
			{
				//이진화 처리 이미지(bin)에서 찾은 Edge를 Contour 이미지(bin2)에서 White로 변경
				RawSrc[rposy * bin.cols + rposx] = 255;

				CPointF<int> p2(rposx, rposy);
				int _S = GetDistance(p_center, p2);
				cv::Point3i ptr = cv::Point3i(rposx, rposy, _S);
				outlierVec.push_back(ptr);
				sumdistance += _S;
				break;
			}
		}
	}

	if (outlierVec.size() > 10)
	{
		avgdistance = sumdistance / outlierVec.size();
		//outlier 제거
		for (int i = 0; i < outlierVec.size(); i++)
		{
			if (outlierVec[i].z < avgdistance - margin || outlierVec[i].z > avgdistance + margin)
			{
				//CC Edge를 제외한 나머지 이미지를 검정으로 변경한다.
				RawSrc[outlierVec[i].y * bin.cols + outlierVec[i].x] = 0;
			}
		}
	}

	outlierVec.clear();

	dilate(bin2, bin2, Mat::ones(5, 5, CV_8U));

	//인자 3,4번 의미가 없네???
//	findOrgImageCircle(bin2, circles, BMhole, BMhole);
	findOrgImageLMERanscCircle(bin2, circles, BMhole, BMhole);  //20220117 Tkyuha 새로운 알고리즘 테스트 진행중

	// 디버깅 모드에서 이미지 보기 //20220117 Tkyuha
	if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
	{
		cvShowImage("BMInspection_Image", bin2);
	}

	if (circles.size() >= 1)
	{
		m_dCircleRadius_CC = circles[0][2];
		m_ptCircleCenter_CC = CPoint(cvRound(circles[0][0] + InspRect.x), cvRound(circles[0][1] + InspRect.y));

		if (mViewer != NULL)
		{
			FakeDC* pDC = NULL;
			CPen penROI(PS_SOLID, 3, RGB(255, 255, 0));
			CPen penGreen(PS_SOLID, 3, RGB(0, 255, 0));

			pDC = mViewer->getOverlayDC();			
			pDC->SelectObject(&penGreen);

			pDC->Ellipse(int(m_ptCircleCenter_CC.x - m_dCircleRadius_CC), int(m_ptCircleCenter_CC.y - m_dCircleRadius_CC),
				int(m_ptCircleCenter_CC.x + m_dCircleRadius_CC), int(m_ptCircleCenter_CC.y + m_dCircleRadius_CC));

			if (m_leftRight.x != 0 && m_leftRight.y != 0)
			{
				int cx = (m_leftRight.x + m_leftRight.y) / 2;
				int cy = (m_topBottom.x + m_topBottom.y) / 2;
				pDC->SelectObject(&penGreen);
				pDC->Ellipse(int(cx - 4), int(cy - 4), int(cx + 4), int(cy + 4));
				
				double left		= (m_leftRight.x - (W / 2 - CIRCLE_RADIUS)) * xres;
				double right	= ((W / 2 + CIRCLE_RADIUS) - m_leftRight.y) * yres;
				double top		= (m_topBottom.x - (H / 2 - CIRCLE_RADIUS)) * xres;
				double bottom	= ((H / 2 + CIRCLE_RADIUS) - m_topBottom.y) * yres;

				//KJH 2022-05-06 CC to PN 검사 Display 삭제
				//pDC->SetTextColor(COLOR_RED);
				//str_data.Format("[CC <-> PN] L : %.3fmm, R : %.3fmm, T : %.3fmm, B : %.3fmm", left, right, top, bottom);
				//pDC->TextOutA(int(W / 2), int(H / 2 - 100), str_data);

			}

			penROI.DeleteObject();
			penGreen.DeleteObject();

			mViewer->Invalidate();
		}
	}
	else
	{
		if (mViewer != NULL)
		{
			FakeDC* pDC = NULL;
			CPen penROI(PS_SOLID, 3, RGB(255, 255, 0));
			CPen penGreen(PS_SOLID, 3, RGB(0, 255, 0));

			pDC = mViewer->getOverlayDC();
			
			pDC->SelectObject(&penGreen);

			str_data.Format("[CC <-> PN] Do Not Find CC Circle");
			pDC->TextOutA(int(W / 2 - 200), int(H / 2 - 100), str_data);

			penROI.DeleteObject();
			penGreen.DeleteObject();

			mViewer->Invalidate();

		}
		m_dCircleRadius_CC = BMhole - 20;
		m_ptCircleCenter_CC = CPoint(W / 2, H / 2);
	}

	imgGray.release();
	bin.release();
	bin2.release();

	circles.clear();
	contours.clear();
	hierarchy.clear();

	return breturn;
}
BOOL CFormMainView::InspCircleDummyDetection(int nJob, int nCam, const cv::Mat* src, CViewerEx* mViewer)
{
	int W = m_pMain->m_stCamInfo[nCam].w;
	int H = m_pMain->m_stCamInfo[nCam].h;

	if (src == NULL || src->empty())
	{
		cv::Mat img(H, W, CV_8UC1, m_pMain->getProcBuffer(nCam, nJob));
		img.copyTo(*src);
	}

	BOOL breturn = TRUE;
	CString str_data;
	int method = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspectionMethod();

	double posx = 0.0, posy = 0.0;
	double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(0, 0);
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);

	std::vector<cv::Vec3f> circles;
	int hthresh = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionLowThresh();
	cv::Rect InspRect = cv::Rect(0, 0, W, H);
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;

	cv::Mat imgGray = *src, bin, bin2;
	cv::threshold(imgGray, bin, hthresh, 255, CV_THRESH_BINARY_INV);

	bin2 = bin.clone();
	cv::findContours(bin2, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

	int ID = getMaxAreaContourId(contours);
	if (ID != -1)
	{
		InspRect = boundingRect(contours[ID]);
	}
	cv::Mat element15(20, 20, CV_8U, cv::Scalar::all(1));
	cv::Mat close;
	morphologyEx(bin, close, cv::MORPH_CLOSE, element15);
	fnRemoveNoise(close, 1000);
	morphologyEx(close, close, cv::MORPH_OPEN, element15);

	uchar* Raw = close.data;
	bool bfirst = false;
	double sumWidth = 0;
	double minV = 99999, maxV = -9999, minA = 0, maxA = 0;

	cv::Point2f minPtrPos = cv::Point2f(0, 0);
	cv::Point2f maxPtrPos = cv::Point2f(0, 0);

	std::vector<cv::Point2f> ptrFirstPoint;
	std::vector<cv::Point2f> ptrSecondPoint;

	for (int second = 0; second < 360; second++)
	{
		bfirst = false;
		// 원위의 점 찾기
		for (int distanceT = 50; distanceT > -700; distanceT--)
		{
			int _rad = InspRect.width / 2 + distanceT;
			int rposx = (cos(ToRadian(second)) * _rad + InspRect.x + InspRect.width / 2);
			int rposy = (sin(ToRadian(second)) * _rad + InspRect.y + InspRect.height / 2);

			if (rposx < 0 || rposx >= close.cols) continue;
			if (rposy < 0 || rposy >= close.rows) continue;

			if (!bfirst && Raw[rposy * close.cols + rposx] > 128)
			{
				ptrFirstPoint.push_back(cv::Point2f(rposx, rposy));
				bfirst = true;
			}
			else if (bfirst && Raw[rposy * close.cols + rposx] < 128)
			{
				CPointF<int> p(ptrFirstPoint.at(ptrFirstPoint.size() - 1).x, ptrFirstPoint.at(ptrFirstPoint.size() - 1).y);
				CPointF<int> p2(rposx, rposy);
				double d = GetDistance(p, p2) * xres;
				if (d > 0.1)
				{
					ptrSecondPoint.push_back(cv::Point2f(rposx, rposy));

					sumWidth += d;
					if (minV > d)
					{
						minA = second;
						minV = d;
						minPtrPos = cv::Point2f(rposx, rposy);
					}
					if (maxV < d)
					{
						maxA = second;
						maxV = d;
						maxPtrPos = cv::Point2f(rposx, rposy);
					}

					break;
				}
			}
		}
	}

	if (ptrSecondPoint.size() > 0)
	{
		sumWidth /= ptrSecondPoint.size();

		if (m_pMain->m_ELB_RefData.size() > 0)
		{
			double* lx = new double[m_pMain->m_ELB_RefData.size()];
			double* ly = new double[m_pMain->m_ELB_RefData.size()];
			double ax_Out = 0, bc_Out = 0;
			double _rval = 0, _rRef = 0;

			for (int x = 0; x < m_pMain->m_ELB_RefData.size(); x++)
			{
				lx[x] = m_pMain->m_ELB_RefData[x].dAir;
				ly[x] = m_pMain->m_ELB_RefData[x].dWet_Out;
			}

			LeastSqrRegression(lx, ly, int(m_pMain->m_ELB_RefData.size()), ax_Out, bc_Out);

			if (ax_Out > 0)
			{
				_rval = (sumWidth - bc_Out) / ax_Out;
				_rRef = (0.2 - bc_Out) / ax_Out;
			}

			delete lx;
			delete ly;

			//KJH 2021-09-02 mBar->kPa
			//str_data.Format("Current(%.3f)mm -Air Press(%.3f)mBar\r\n Recommand (0.2)mm -Air Press(%.3f)mBar ", sumWidth, _rval, _rRef);
			str_data.Format("Current(%.3f)mm - Air Press(%.3f)kPa\r\n Recommand (0.2)mm - Air Press(%.3f)kPa ", sumWidth, _rval, _rRef);
			m_pMain->fnSetMessage(2, str_data);

		}
	}

	if (mViewer != NULL)
	{
		FakeDC* pDC = NULL;
		CPen penROI(PS_SOLID, 3, RGB(255, 255, 0));
		CPen penGreen(PS_SOLID, 3, RGB(0, 255, 0));
		CFont font, * oldFont;
		int OLD_BKMODE = 0;

		pDC = mViewer->getOverlayDC();
		font.CreateFont(30, 30, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
			OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
		oldFont = pDC->SelectObject(&font);
		pDC->SelectObject(&penROI);
		OLD_BKMODE = pDC->SetBkMode(TRANSPARENT);
		pDC->SelectObject(GetStockObject(NULL_BRUSH));

		pDC->Rectangle(CRect(InspRect.x, InspRect.y, InspRect.x + InspRect.width, InspRect.y + InspRect.height));

		pDC->SelectObject(&penGreen);

		int cx, cy;

		for (int j = 0; j < ptrFirstPoint.size(); j++)
		{
			cx = ptrFirstPoint.at(j).x;
			cy = ptrFirstPoint.at(j).y;
			pDC->Ellipse(int(cx - 4), int(cy - 4), int(cx + 4), int(cy + 4));
		}
		for (int j = 0; j < ptrSecondPoint.size(); j++)
		{
			cx = ptrSecondPoint.at(j).x;
			cy = ptrSecondPoint.at(j).y;
			pDC->Ellipse(int(cx - 4), int(cy - 4), int(cx + 4), int(cy + 4));
		}

		pDC->SetTextColor(COLOR_RED);
		str_data.Format("%.3fmm", sumWidth);
		pDC->TextOutA(int(W / 2), int(H / 2 - 100), str_data);


		str_data.Format("Min:(%.1f,%.1f)-(%.1f)Angle - %.3fmm", minPtrPos.x, minPtrPos.y, minA, minV);
		pDC->TextOutA(int(minPtrPos.x), int(minPtrPos.y), str_data);

		str_data.Format("Min:(%.1f,%.1f)-(%.1f)Angle - %.3fmm", maxPtrPos.x, maxPtrPos.y, maxA, maxV);
		pDC->TextOutA(int(maxPtrPos.x), int(maxPtrPos.y), str_data);


		penROI.DeleteObject();
		penGreen.DeleteObject();
		font.DeleteObject();
		pDC->SetBkMode(OLD_BKMODE);

		mViewer->Invalidate();
	}

	ptrFirstPoint.clear();
	ptrSecondPoint.clear();


	return breturn;
}
BOOL CFormMainView::InspLineDummyDetection(int nJob, int nCam, const cv::Mat* src, CViewerEx* mViewer)
{
	int W = m_pMain->m_stCamInfo[nCam].w;
	int H = m_pMain->m_stCamInfo[nCam].h;

	cv::Mat src_img;

	if (src == NULL || src->empty())
	{
		cv::Mat img(H, W, CV_8UC1, m_pMain->getSrcBuffer(nCam));
		img.copyTo(src_img);
	}
	else
	{
		src->copyTo(src_img);
	}

	BOOL breturn = TRUE;
	CString str_data;
	int inspH = m_pMain->m_dDummyInspRangeHeight;	// Tkyuha 20211124 검사 폭을 추후 확인 해서 입력 하도록
	int iBaseY = m_pMain->m_dDummyStart_posXY.y;	// Tkyuha 20211124 검사 첫번째 시작위치 추후 확인 해서 입력 하도록
	int iBaseX = W / 2 - 150;						// m_pMain->m_dDummyStart_posXY.x; // Tkyuha 20211124 검사 첫번째 시작위치 추후 확인 해서 입력 하도록
	int method = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspectionMethod();

	m_dDummyLineNum = 0; // 20220305 무조건 중앙에 위치 하도록 PLC에서 보내줄것

	double posx = 0.0, posy = 0.0;
	double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(0, 0);
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);

	std::vector<cv::Vec3f> circles;
	int hthresh = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionLowThresh();
	//HTK 2022-03-16 DummyInspSpec 추가
	double _dDummyMinSizeSpec = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getDummyMinSizeSpec();

	cv::Rect InspRect = cv::Rect(0, 0, W, H);
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;

	cv::Mat imgGray = src_img, bin, bin2;
	//cv::threshold(imgGray, bin, hthresh, 255, CV_THRESH_BINARY_INV);
	cv::threshold(imgGray, bin, -1, 255, CV_THRESH_BINARY_INV | THRESH_OTSU);//

	bin2 = bin.clone();
	cv::findContours(bin2, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);

	int ID = getMaxAreaContourId(contours);
	if (ID != -1)
	{
		InspRect = boundingRect(contours[ID]);
	}

	InspRect.x = iBaseX;
	InspRect.y = MAX(0, MIN(iBaseY + (m_dDummyLineNum * inspH), H - inspH - 1)); // 영역을 변경
	InspRect.height = inspH;

	cv::Mat element15(20, 20, CV_8U, cv::Scalar::all(1));
	cv::Mat close;
	morphologyEx(bin, close, cv::MORPH_CLOSE, element15);
	fnRemoveNoise(close, 1000);
	morphologyEx(close, close, cv::MORPH_OPEN, element15);

	if(m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())//디버깅용으로 
		cvShowImage("InspOrg Image", close);

	uchar* Raw = close.data;
	bool bfirst = false;
	double sumWidth = 0,dopoLength=0;
	double minV = 99999, maxV = -9999, minA = 0, maxA = 0;

	cv::Point2f minPtrPos = cv::Point2f(0, 0);
	cv::Point2f maxPtrPos = cv::Point2f(0, 0);

	std::vector<cv::Point2f> ptrFirstPoint;
	std::vector<cv::Point2f> ptrSecondPoint;
	std::vector<double> ptrDistancePoint;

	int xrange = InspRect.x + InspRect.width+300;
	int yrange = InspRect.y + InspRect.height;
	int nstart_Y=-1, nend_Y=-1;

	for (int j = InspRect.y; j < yrange; j++)
	{
		bfirst = false;
		// 원위의 점 찾기
		for (int i = InspRect.x; i < xrange; i++)
		{
			int rposx = i;
			int rposy = j;

			if (rposx < 0 || rposx >= close.cols) continue;
			if (rposy < 0 || rposy >= close.rows) continue;

			if (!bfirst && Raw[rposy * close.cols + rposx] > 128)
			{
				ptrFirstPoint.push_back(cv::Point2f(rposx, rposy));
				bfirst = true;
			}
			else if (bfirst && Raw[rposy * close.cols + rposx] < 128)
			{
				CPointF<int> p(ptrFirstPoint.at(ptrFirstPoint.size() - 1).x, ptrFirstPoint.at(ptrFirstPoint.size() - 1).y);
				CPointF<int> p2(rposx, rposy);
				double d = GetDistance(p, p2) * xres;
				if (d > 0.1)
				{
					ptrSecondPoint.push_back(cv::Point2f(rposx, rposy));
					ptrDistancePoint.push_back(d);

					sumWidth += d;
					if (minV > d)
					{
						minA = i;
						minV = d;
						minPtrPos = cv::Point2f(rposx, rposy);
					}
					if (maxV < d)
					{
						maxA = i;
						maxV = d;
						maxPtrPos = cv::Point2f(rposx, rposy);
					}

					if (nstart_Y == -1) nstart_Y = j;
					nend_Y = j;

					break;
				}				
			}
		}
	}

	if (ptrSecondPoint.size() > 0)
	{
		//HTK 2022-03-16 DummyInspSpec 추가
		if (_dDummyMinSizeSpec > 0)
		{
			sumWidth = 0;
			minV = 99999, maxV = -9999;

			int _findex = _dDummyMinSizeSpec / yres;
			for (int _ax = _findex; _ax < ptrSecondPoint.size(); _ax++)
			{
				double d = ptrDistancePoint.at(_ax);
				sumWidth += d;
				if (minV > d)
				{
					minA = _ax;
					minV = d;
					minPtrPos = ptrSecondPoint.at(_ax);
				}
				if (maxV < d)
				{
					maxA = _ax;
					maxV = d;
					maxPtrPos = ptrSecondPoint.at(_ax);
				}
			}

			if((ptrSecondPoint.size()-_findex+1)>0) sumWidth /= (ptrSecondPoint.size() - _findex + 1);
		}
		else	sumWidth /= ptrSecondPoint.size();

		if (m_pMain->m_ELB_RefData.size() > 0)
		{
			double* lx = new double[m_pMain->m_ELB_RefData.size()];
			double* ly = new double[m_pMain->m_ELB_RefData.size()];
			double ax_Out = 0, bc_Out = 0;
			double _rval = 0, _rRef = 0;

			for (int x = 0; x < m_pMain->m_ELB_RefData.size(); x++)
			{
				lx[x] = m_pMain->m_ELB_RefData[x].dAir;
				ly[x] = m_pMain->m_ELB_RefData[x].dWet_Out;
			}

			LeastSqrRegression(lx, ly, int(m_pMain->m_ELB_RefData.size()), ax_Out, bc_Out);

			if (ax_Out > 0)
			{
				_rval = (sumWidth - bc_Out) / ax_Out;
				_rRef = (0.2 - bc_Out) / ax_Out;
			}

			delete lx;
			delete ly;

			//KJH 2021-09-02 mBar->kPa
			//str_data.Format("Current(%.3f)mm -Air Press(%.3f)mBar\r\n Recommand (0.2)mm -Air Press(%.3f)mBar ", sumWidth, _rval, _rRef);
			str_data.Format("Current(%.3f)mm -Air Press(%.3f)kPa\r\n Recommand (0.2)mm -Air Press(%.3f)kPa ", sumWidth, _rval, _rRef);
			//m_pMain->fnSetMessage(2, str_data);

		}

		dopoLength = (nend_Y - nstart_Y)* yres;
	}

	if (mViewer != NULL)
	{
		FakeDC* pDC = NULL;
		CPen penROI(PS_SOLID, 3, RGB(255, 255, 0));
		CPen penGreen(PS_SOLID, 3, RGB(0, 255, 0));
		CFont font, * oldFont;
		int OLD_BKMODE = 0;

		pDC = mViewer->getOverlayDC();
		mViewer->ClearOverlayDC();

		font.CreateFont(30, 30, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
			OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
		oldFont = pDC->SelectObject(&font);
		pDC->SelectObject(&penROI);
		OLD_BKMODE = pDC->SetBkMode(TRANSPARENT);
		pDC->SelectObject(GetStockObject(NULL_BRUSH));

		pDC->Rectangle(CRect(InspRect.x, InspRect.y, InspRect.x + InspRect.width, InspRect.y + InspRect.height));

		pDC->SelectObject(&penGreen);

		int cx, cy;

		for (int j = 0; j < ptrFirstPoint.size(); j++)
		{
			cx = ptrFirstPoint.at(j).x;
			cy = ptrFirstPoint.at(j).y;
			pDC->Ellipse(int(cx - 4), int(cy - 4), int(cx + 4), int(cy + 4));
		}
		for (int j = 0; j < ptrSecondPoint.size(); j++)
		{
			cx = ptrSecondPoint.at(j).x;
			cy = ptrSecondPoint.at(j).y;
			pDC->Ellipse(int(cx - 4), int(cy - 4), int(cx + 4), int(cy + 4));
		}

		pDC->SetTextColor(COLOR_RED);
		str_data.Format("Average Width: %.3fmm , Length :%.3fmm ", sumWidth, dopoLength);
		pDC->TextOutA(int(W / 2), int(H / 2 - 100), str_data);		

		str_data.Format("Min:(%.1f,%.1f)-(%.1f) - %.3fmm", minPtrPos.x, minPtrPos.y, minA, minV);
		pDC->TextOutA(int(minPtrPos.x), int(minPtrPos.y), str_data);

		str_data.Format("Max:(%.1f,%.1f)-(%.1f) - %.3fmm", maxPtrPos.x, maxPtrPos.y, maxA, maxV);
		pDC->TextOutA(int(maxPtrPos.x), int(maxPtrPos.y), str_data);

		penROI.DeleteObject();
		penGreen.DeleteObject();
		font.DeleteObject();
		pDC->SetBkMode(OLD_BKMODE);

		system_clock::time_point start = system_clock::now();

		CString imgMsg, strImgDir,logStr,str_ImageType;
		SYSTEMTIME time;
		GetLocalTime(&time);
		//KMB 220924 SYSTEM OPTION에 따라 저장이미지 확장자 변경
		if (m_pMain->vt_system_option[nJob].save_image_type == FALSE)
			str_ImageType = "jpg";
		else
			str_ImageType = "bmp";

		if (_access(m_pMain->m_strImageDir, 0) != 0)	CreateDirectory(m_pMain->m_strImageDir, NULL);
		if (m_pMain->m_strResultDate[0] == "")	m_pMain->m_strResultDate[nJob].Format("%04d%02d%02d", time.wYear, time.wMonth, time.wDay);
		if (m_pMain->vt_job_info[nJob].main_object_id.size() <= 0) m_pMain->vt_job_info[nJob].main_object_id = "TestImg";

		strImgDir = m_pMain->m_strImageDir + m_pMain->m_strResultDate[nJob];
		if (_access(strImgDir, 0) != 0)		CreateDirectory(strImgDir, NULL);
		imgMsg.Format("%s\\dummyGlass_overlay_%s_%02d%02d%02d.%s", strImgDir, m_pMain->vt_job_info[nJob].main_object_id.c_str(), time.wHour, time.wMinute, time.wSecond, str_ImageType);
		mViewer->saveScreenCapture(imgMsg);

		imgMsg.Format("%s\\dummyGlass_%s_%02d%02d%02d.%s", strImgDir, m_pMain->vt_job_info[nJob].main_object_id.c_str(), time.wHour, time.wMinute, time.wSecond, str_ImageType);
		cv::imwrite(std::string(imgMsg), imgGray);

		mViewer->Invalidate();

		strImgDir.Format("%s%s", m_pMain->m_strHistoryDir, "Dummy_Log");
		if (_access(strImgDir, 0) != 0)	CreateDirectory(strImgDir, NULL);
		imgMsg.Format("%s\\DummyInspection.Log", strImgDir);

		CStdioFile WriteFile;
		if (WriteFile.Open(imgMsg, CFile::modeNoTruncate | CFile::modeCreate | CFile::modeWrite | CFile::shareDenyNone | CFile::typeText))
		{
			WriteFile.SeekToEnd();
			logStr.Format("%04d%02d%02d_%02d%02d%02d = W: %.3f L: %.3f MIN: %.3f MAX: %.3f\n", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond, sumWidth, dopoLength, minV, maxV);
			WriteFile.WriteString(logStr);
			
			WriteFile.Close();
		}
	}

	ptrFirstPoint.clear();
	ptrSecondPoint.clear();
	ptrDistancePoint.clear();

	src_img.release();

	return breturn;
}
double CFormMainView::InspLineCGDetection(int nJob, int nCam, const cv::Mat* src, CViewerEx* mViewer)
{
	int W = m_pMain->m_stCamInfo[nCam].w;
	int H = m_pMain->m_stCamInfo[nCam].h;	

	cv::Mat src_img, src_img_draw;
	CPointF<int> p2(0, 0);
	CPointF<double> _searchPocket(0, 0);

	if (src == NULL || src->empty())
	{
		cv::Mat img(H, W, CV_8UC1, m_pMain->getSrcBuffer(nCam));
		img.copyTo(src_img);
		img.copyTo(src_img_draw);
	}
	else
	{
		src->copyTo(src_img);
		src->copyTo(src_img_draw);
	}

	double dx = 0, dy = 0, dt = 0;
	int camCount = m_pMain->vt_job_info[nJob].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[nJob].camera_index;

	CString str;
	int ncamera = camBuf.at(0);
	int datasheet = 0;
	double _searchCG_X = W / 2.;
	double _linecenterspec = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getCamCenterToCG();

	m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][C_CALIPER_POS_6].processCaliper(src_img.data, W, H, dx, dy, dt); // Metal Line 윗라인
	
	BOOL find_line[1];
	sLine line_info[1];

	find_line[0] = m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][C_CALIPER_POS_6].getIsMakeLine();
	line_info[0] = m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][C_CALIPER_POS_6].m_lineInfo;

	if (find_line[0] == TRUE)
	{
	if (mViewer != NULL)
	{
		FakeDC* pDC = NULL;
		CPen penROI(PS_SOLID, 3, RGB(255, 255, 0));
		CPen penGreen(PS_SOLID, 3, RGB(0, 255, 0));		

		pDC = mViewer->getOverlayDC();
		mViewer->ClearOverlayDC();
		
		pDC->SelectObject(&penROI);
		pDC->SelectObject(&penGreen);

			if (line_info[0].a == 0)
			{
				p2.y = 0;     				p2.x = line_info[0].b;
				pDC->MoveTo(p2.x, p2.y);   _searchPocket.x = p2.x;
				cv::Point pt1(p2.x, p2.y);
				p2.y = src_img.rows - 1; 	p2.x = line_info[0].b;
				pDC->LineTo(p2.x, p2.y);    _searchPocket.y = p2.x;// Pocket 시작점
				cv::Point pt2(p2.x, p2.y);

				//SJB 2022-10-31 CGDection 이미지 FindLine 그리기
				cv::line(src_img_draw, pt1, pt2, cv::Scalar(255, 0, 0), 4);
			}
			else
			{
				p2.y = 0;			        p2.x = (p2.y - line_info[0].b) / line_info[0].a;
				pDC->MoveTo(p2.x, p2.y);    _searchPocket.x = p2.x;
				cv::Point pt1(p2.x, p2.y);
				p2.y = src_img.rows - 1;		p2.x = (p2.y - line_info[0].b) / line_info[0].a;
				pDC->LineTo(p2.x, p2.y);     _searchPocket.y = p2.x;// Pocket 시작점
				cv::Point pt2(p2.x, p2.y);

				//SJB 2022-10-31 CGDection 이미지 FindLine 그리기
				cv::line(src_img_draw, pt1, pt2, cv::Scalar(255, 0, 0), 4);
		}
		if (find_line[0] != FALSE)
		{
			CRect realRoi = m_pMain->GetMatching(nJob).getSearchROI(0, 4);
			bool bwhite = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getEnableAlignBlackWhiteSelect() ? true : false;
			m_pMain->GetMatching(nJob).findPattern(src_img.data, nCam, 4, W, H);
			CFindInfo pFindInfo = m_pMain->GetMatching(nJob).getFindInfo(nCam, 4);

			if (pFindInfo.GetFound() == FIND_OK)
					_searchCG_X = m_pMain->GetMatching(nJob).getFindInfo(nCam, 4).GetXPos();

			pDC->MoveTo(_searchCG_X, 0);
			pDC->LineTo(_searchCG_X, H);
		}

		m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultAngle[2] = _searchCG_X;

		penROI.DeleteObject();
		penGreen.DeleteObject();
		}
	}
	else
	{
		_searchCG_X = 0.0;
		m_pMain->m_ELB_DiffInspResult[datasheet].m_ELB_ResultAngle[2] = _searchCG_X;
	}

	CString imgMsg, strImgDir, str_ImageType;
	SYSTEMTIME time;
	GetLocalTime(&time);
	//KMB 220924 SYSTEM OPTION에 따라 저장이미지 확장자 변경
	if (m_pMain->vt_system_option[nJob].save_image_type == FALSE)
		str_ImageType = "jpg";
	else
		str_ImageType = "bmp";

	if (_access(m_pMain->m_strImageDir, 0) != 0)	CreateDirectory(m_pMain->m_strImageDir, NULL);
	if (m_pMain->m_strResultDate[0] == "")	m_pMain->m_strResultDate[nJob].Format("%04d%02d%02d", time.wYear, time.wMonth, time.wDay);
	if (m_pMain->vt_job_info[nJob].main_object_id.size() <= 0) m_pMain->vt_job_info[nJob].main_object_id = "TestImg";

	//SJB 2022-10-31 CGDection 이미지 경로 수정

	strImgDir.Format("%s%04d%02d%02d", m_pMain->m_strImageDir, time.wYear, time.wMonth, time.wDay);
	if (_access(strImgDir, 0) != 0) ::CreateDirectory(strImgDir, 0);

	strImgDir.Format("%s\\%s", strImgDir, m_pMain->m_strCurrModel);
	if (_access(strImgDir, 0) != 0) ::CreateDirectory(strImgDir, 0);

	strImgDir.Format("%s\\CENTER_ALIGN", strImgDir);
	if (_access(strImgDir, 0) != 0) ::CreateDirectory(strImgDir, 0);

	strImgDir.Format("%s\\CGDetection", strImgDir);
	if (_access(strImgDir, 0) != 0) ::CreateDirectory(strImgDir, 0);

	if (strlen(m_pMain->vt_job_info[nJob].main_object_id.c_str()) <= 13)
	{
		strImgDir.Format("%s\\Not Exist Panel ID", strImgDir);
		if (_access(strImgDir, 0) != 0)	CreateDirectory(strImgDir, NULL);
	}
	strImgDir.Format("%s\\%s", strImgDir, m_pMain->vt_job_info[nJob].main_object_id.c_str());
	if (_access(strImgDir, 0) != 0)		CreateDirectory(strImgDir, NULL);

	imgMsg.Format("%s\\CG_EdgeGlass_%s_%02d%02d%02d.jpg", strImgDir, m_pMain->vt_job_info[nJob].main_object_id.c_str(), time.wHour, time.wMinute, time.wSecond);
	cv::imwrite(std::string(imgMsg), src_img_draw);

	imgMsg.Format("%s\\CG_EdgeGlass_%s_%02d%02d%02d_ImgRaw.jpg", strImgDir, m_pMain->vt_job_info[nJob].main_object_id.c_str(), time.wHour, time.wMinute, time.wSecond);
	cv::imwrite(std::string(imgMsg), src_img);

	src_img_draw.release();
	src_img.release();

	return _searchCG_X;
}

BOOL CFormMainView::InspNozzleDistance_align(int nJob, int nCam, BOOL Reverse, BOOL UseRef, const cv::Mat& src, CViewerEx* mViewer)
{
	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
	int W = m_pMain->m_stCamInfo[real_cam].w;
	int H = m_pMain->m_stCamInfo[real_cam].h;
	int Center_W = m_pMain->m_stCamInfo[real_cam].w / 2;
	int Center_H = m_pMain->m_stCamInfo[real_cam].h / 2;

	//KJH 2021-09-13 getSrcBuffer(카메라 Grab 이미지)
	//KJH 2021-09-13 getProcBuffer(별도 작업 이미지)
	cv::Mat img = src.clone();
	cv::Mat Porcimg = src.clone();
	//cvShowImage("img", img);

	FakeDC* pDC = NULL;
	CPen penE(PS_SOLID, 3, RGB(255, 0, 0));
	CPen penS(PS_SOLID, 3, RGB(0, 255, 0));
	CPen penB(PS_SOLID, 3, RGB(0, 0, 255));
	CPen penB2(PS_DOT, 2, RGB(0, 0, 255));
	CFont font, * oldFont;
	int OLD_BKMODE = 0;
	BOOL breturn = TRUE;
	CString str;

	if (mViewer != NULL)
	{
		pDC = mViewer->getOverlayDC();
		mViewer->ClearOverlayDC();
		pDC->SelectObject(&penE);

		font.CreateFont(50, 50, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
			OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
		oldFont = pDC->SelectObject(&font);

		OLD_BKMODE = pDC->SetBkMode(TRANSPARENT);
		pDC->SelectObject(GetStockObject(NULL_BRUSH));
	}

	//화면 갱신이 필요할까??
	mViewer->OnLoadImageFromPtr(m_pMain->getSrcBuffer(real_cam));

#pragma region 영상처리

	cv::cvtColor(img, img, COLOR_GRAY2BGR);

	double posx = 0.0, posy = 0.0;
	double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(nCam, 0);
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(nCam, 0);
	double refX = m_pMain->GetMatching(nJob).getRefX(nCam, 0);
	double refY = m_pMain->GetMatching(nJob).getRefY(nCam, 0);
	
	if (!UseRef)
	{
		refX = Center_W;
		refY = Center_H;
	}

	int Find_X, Find_Y = 0;
	double distance_MtoN = 0.0;

	int _imgThresholdvalue = 60;
	int _thresh = 80;

	if(Reverse)
	{
		cv::threshold(Porcimg, Porcimg, _imgThresholdvalue, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
	}
	else
	{
		cv::threshold(Porcimg, Porcimg, _imgThresholdvalue, 255, CV_THRESH_BINARY_INV | CV_THRESH_OTSU);
	}

	const uchar* dataptr = Porcimg.data;

	bool testmode = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getTestImageSave();

	if (testmode)
	{
		imwrite("D:\\InspNozzleDistance_align_Final.jpg", Porcimg);
	}

#pragma endregion

#pragma region 위치 찾기

	BOOL bFind = FALSE;

	//검색 영역 

	for (int j = Center_H + 500; j > Center_H / 2; j--)
	{ 
		for (int i = Center_W - 250; i < Center_W + 250; i++)
		{
			if (j < 0 || H <= j) continue;

			//Nozzle Find(Black Find))
			if (dataptr[j * W + i] < 30) // 중심에서 5Pixel 아래서부터 0까지 검색 0~255 //Black이 0 / White가 255 
			{
				//처음 Black->White인 점 찾기 [Panel Edge 찾기]]
				if (Find_Y == 0)
				{
					Find_Y = j;
					Find_X = i;
					distance_MtoN = fabs(Find_Y - refY) * yres;
					
					// KBJ 2022-07-19 노즐 메뉴얼로 거리검사 그리는 직선이 카메라 중앙이 아닌 카메라 중앙선과 일직선으로 보이게끔 수정
					if (!UseRef) refX = Find_X;
					break;
				}
			}
		}

		// KBJ 2022-07-19 노즐 메뉴얼로 거리검사 x 찾는 위치 보완(왼쪽과 오른쪽 둘다 찾아 중간값으로 계산)
		if (Find_Y != 0)
		{
			for (int i = Center_W + 250; i > Find_X; i--)
			{
				if (dataptr[j * W + i] < 30) // 중심에서 5Pixel 아래서부터 0까지 검색 0~255 //Black이 0 / White가 255 
				{
					//처음 Black->White인 점 찾기 [Panel Edge 찾기]]
					Find_X = int((Find_X + i) / 2);
					distance_MtoN = fabs(Find_Y - refY) * yres;

					// KBJ 2022-07-19 노즐 메뉴얼로 거리검사 그리는 직선이 카메라 중앙이 아닌 카메라 중앙선과 일직선으로 보이게끔 수정
					if (!UseRef) refX = Find_X;
					bFind = TRUE;
					break;
				}
			}
		}

		if (bFind == TRUE) break;
	}

#pragma endregion

	CString strText;
	int		TextSize_Thickness = 10;
	int		TextSize = 5;
	int		TextPosition_Shift = 200;

#pragma region 화면 Display 및 이미지 로그 작업

	if (pDC != NULL)
	{
		pDC->Ellipse(int(refX - 2), int(refY - 2), int(refX + 2), int(refY + 2));
		pDC->SelectObject(&penS);
		pDC->Ellipse(int(Find_X - 2), int(Find_Y - 2), int(Find_X + 2), int(Find_Y + 2));

		// KBJ 2022-07-19 노즐 메뉴얼로 거리검사 그리는 직선이 카메라 중앙이 아닌 카메라 중앙선과 일직선으로 보이게끔 수정
		if (!UseRef)
		{
			pDC->SelectObject(&penB);
			pDC->MoveTo(Find_X, Find_Y);
			pDC->LineTo(refX, refY);
		}
		else
		{
			pDC->SelectObject(&penB);
			pDC->MoveTo(Find_X, Find_Y);
			pDC->LineTo(Find_X, refY);

			pDC->SelectObject(&penB2);
			pDC->MoveTo(refX, refY);
			pDC->LineTo(Find_X, refY);
		}

		strText.Format("Gap : %.4fmm", distance_MtoN);
		pDC->SetTextColor(COLOR_RED);
		pDC->TextOutA(Find_X, Find_Y, strText);
		
		pDC->SetBkMode(OLD_BKMODE);

		cv::putText(img, std::string(strText), cv::Point(Find_X + TextPosition_Shift, Find_Y), cv::FONT_ITALIC, TextSize, cv::Scalar(0, 0, 255), TextSize_Thickness);
		cv::line(img, cv::Point(int(Find_X), int(Find_Y)), cv::Point(int(refX), int(refY)), cv::Scalar(64, 255, 64), 2);
		cv::circle(img, cv::Point(int(Find_X), int(Find_Y)), 2, cv::Scalar(0, 255, 255), 2, 3);
		cv::circle(img, cv::Point(int(refX), int(refY)), 2, cv::Scalar(255, 255, 0), 2, 3);
	}
	else
	{
		strText.Format("Gap : %.4fmm", distance_MtoN);
		cv::putText(img, std::string(strText), cv::Point(Find_X + TextPosition_Shift, Find_Y), cv::FONT_ITALIC, TextSize, cv::Scalar(0, 0, 255), TextSize_Thickness);
		cv::line(img, cv::Point(int(Find_X), int(Find_Y)), cv::Point(int(refX), int(refY)), cv::Scalar(64, 255, 64), 2);
		cv::circle(img, cv::Point(int(Find_X), int(Find_Y)), 2, cv::Scalar(0, 255, 255), 2, 3);
		cv::circle(img, cv::Point(int(refX), int(refY)), 2, cv::Scalar(255, 255, 0), 2, 3);
	}

#pragma endregion

	if (testmode)
	{
		imwrite("D:\\InspNozzleDistance_align_Overlay.jpg", img);
	}

	img.release();
	Porcimg.release();

#pragma endregion

	if (pDC != NULL)	mViewer->Invalidate();

	penS.DeleteObject();
	penE.DeleteObject();
	penB.DeleteObject();
	penB2.DeleteObject();
	font.DeleteObject();
	//oldFont->DeleteObject();

	return breturn;
}
BOOL CFormMainView::InspNotchMetalEdgeDetection(int nJob, int nCam, const cv::Mat& src, CPoint ptStart, CPoint ptEnd, CViewerEx* mViewer)
{
	FakeDC* pDC = NULL;
	CPen penE(PS_SOLID, 3, RGB(255, 0, 0));
	CPen penS(PS_SOLID, 3, RGB(255, 255, 0));
	int OLD_BKMODE = 0;
	BOOL breturn = TRUE;

	if (mViewer != NULL)
	{
		pDC = mViewer->getOverlayDC();
		pDC->SelectObject(&penE);
	}

	int W = m_pMain->m_stCamInfo[nCam].w;
	int H = m_pMain->m_stCamInfo[nCam].h;
	int hthresh = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionHighThresh();
	int lthresh = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionLowThresh();

	cv::Mat bin3, bin;
	cv::Mat s_down, maskbin; // 축소 해서 검사 진행
	cv::resize(src, s_down, cv::Size(), 0.5, 0.5, CV_INTER_CUBIC);

	cv::threshold(s_down, bin3, hthresh, 255, CV_THRESH_BINARY_INV);
	cv::GaussianBlur(bin3, bin3, cv::Size(0, 0), 3.);
	cv::threshold(bin3, bin3, lthresh, 255, CV_THRESH_BINARY_INV);

	cv::Mat mask = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3), cv::Point(1, 1));
	dilate(bin3, bin3, mask, cv::Point(-1, -1), 1);
	erode(bin3, bin3, mask, cv::Point(-1, -1), 1);

	cv::Point2f rC;
	std::vector<cv::Point> vtMaskContour;
	cv::Mat maskRect = cv::Mat::zeros(bin3.rows, bin3.cols, CV_8U);
	double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(0, 0);
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);
	double distanceT = 0.2 / xres;

	if (m_pMain->m_vtTraceData.size() > 0)
	{
		for (int i = 0; i < m_pMain->m_vtTraceData.size(); i++)
		{
			vtMaskContour.push_back(cv::Point(int(m_pMain->m_vtTraceData[i].x / 2.), int(m_pMain->m_vtTraceData[i].y / 2.)));
		}
		for (int i = 0; i < m_pMain->m_vtTraceData.size(); i++)
		{
			if (i == m_pMain->m_vtTraceData.size() - 1)
				rC = Equation_of_Lines(m_pMain->m_vtTraceData[i], m_pMain->m_vtTraceData[i - 1], distanceT);
			else rC = Equation_of_Lines(m_pMain->m_vtTraceData[i], m_pMain->m_vtTraceData[i + 1], distanceT);

			vtMaskContour.insert(vtMaskContour.begin(), cv::Point(int(rC.x / 2.), int(rC.y / 2.)));
		}
		int num = (int)vtMaskContour.size();
		const cv::Point* pt4 = &(vtMaskContour[0]);
		cv::fillPoly(maskRect, &pt4, &num, 1, cv::Scalar(255, 255, 255), 8);
	}
	else
	{
		maskRect(cv::Rect(ptStart.x / 2, ptStart.y / 2, abs(ptEnd.x - ptStart.x) / 2, abs(ptEnd.y - ptStart.y) / 2)) = 1;
		maskRect(cv::Rect(bin3.cols / 2 + 250, ptStart.y / 2 + 30, bin3.cols / 2 - 250, bin3.rows - ptStart.y / 2 - 30)) = 0;
	}

	//cv::GaussianBlur(bin3, bin3, cv::Size(0, 0), 3.);
	bin3.copyTo(maskbin, maskRect);
	cv::resize(maskbin, bin, cv::Size(), 2, 2, CV_INTER_CUBIC);

	BYTE* pBuffer = bin.data;
	double j, prePos = 0;
	m_vecInterSectionPoint.clear();
	for (int i = 0; i < m_pMain->m_vtTraceData.size() - 1; i++)
	{
		for (j = 20; j < 100; j += 1)
		{
			rC = Equation_of_Lines(m_pMain->m_vtTraceData[i], m_pMain->m_vtTraceData[i + 1], j);
			if (pBuffer[int(rC.y) * W + int(rC.x)] < 20)
			{
				if (j < distanceT) prePos = j;
				break;
			}
		}
		if (j >= distanceT && prePos > 0) rC = Equation_of_Lines(m_pMain->m_vtTraceData[i], m_pMain->m_vtTraceData[i + 1], prePos);
		m_vecInterSectionPoint.push_back(rC);
	}
	m_vecInterSectionPoint.push_back(cv::Point2f(m_pMain->m_vtTraceData[m_pMain->m_vtTraceData.size() - 1].x,
		m_vecInterSectionPoint[m_vecInterSectionPoint.size() - 1].y));
	
	if (pDC != NULL)
	{
		if (m_vecInterSectionPoint.size() > 0)
		{
			pDC->MoveTo(m_vecInterSectionPoint[0].x, m_vecInterSectionPoint[0].y);
			for (int i = 1; i < m_vecInterSectionPoint.size(); i++)
			{
				pDC->LineTo(m_vecInterSectionPoint[i].x, m_vecInterSectionPoint[i].y);
			}
		}

		if (vtMaskContour.size() > 0)
		{
			pDC->SelectObject(&penS);
			pDC->MoveTo(vtMaskContour[0].x * 2, vtMaskContour[0].y * 2);
			for (int i = 1; i < vtMaskContour.size(); i++)
			{
				pDC->LineTo(vtMaskContour[i].x * 2, vtMaskContour[i].y * 2);
			}
		}
	}
	
	// 각 점들 간의 각도를 계산
	std::vector<double>  ptrTheta;
	for (int j = 1; j < m_vecInterSectionPoint.size(); j++)
	{
		ptrTheta.push_back(calcAngleFromPoints(m_vecInterSectionPoint[j - 1], m_vecInterSectionPoint[j]));  // 원데이타로 계산
	}
	double rotateX, rotateY, dbAlignX, dbAlignY, a, b, c;
	TPointList5D arrayList;
	double offsetx = m_vecInterSectionPoint.at(m_vecInterSectionPoint.size() - 1).x;
	double offsety = m_vecInterSectionPoint.at(m_vecInterSectionPoint.size() - 1).y;
	c = ptrTheta.at(m_vecInterSectionPoint.size() - 2);
	rotateX = m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.x * -1;
	rotateY = m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.y * -1;

	for (int k = int(m_vecInterSectionPoint.size() - 1); k >= 0; k--) // 역방향 Notch
	{
		float tc = (k == 0) ? ptrTheta.at(0) - c : ptrTheta.at(k - 1) - c;
		///// 원영상 위치에서 계산 할경우
		float xc = (m_vecInterSectionPoint.at(k).x - offsetx) * xres;
		float yc = (m_vecInterSectionPoint.at(k).y - offsety) * yres;

		calcRotate(rotateX, rotateY, xc, yc, (-tc) * CV_PI / 180.0, &dbAlignX, &dbAlignY);

		if (k == m_vecInterSectionPoint.size() - 1)
		{
			a = dbAlignX;
			b = dbAlignY;
			arrayList.push_back({ xc, yc, 0.f, 0.f, 0.f });
		}
		else
		{
			dbAlignX = dbAlignX - a;
			dbAlignY = dbAlignY - b;
			arrayList.push_back({ xc, yc, tc, float(dbAlignX), float(dbAlignY) });
		}
	}

	ptrTheta.clear();
	vtMaskContour.clear();
	arrayList.clear();

	if (pDC != NULL)	mViewer->Invalidate();

	penS.DeleteObject();
	penE.DeleteObject();

	return breturn;
}
BOOL CFormMainView::InspLineEdgeDetection(int nJob, const cv::Mat& src, CPoint ptStart, CPoint ptEnd, float fscanLength, CViewerEx* mViewer, bool btracking, bool bdust)
{
	int CALIPER_NO_USE_0 = C_CALIPER_POS_1;
	int CALIPER_PANEL = C_CALIPER_POS_2;
	int CALIPER_METAL = C_CALIPER_POS_3;
	int CALIPER_NO_USE_4 = C_CALIPER_POS_4;
	int CALIPER_NO_USE_5 = C_CALIPER_POS_5;
	int CALIPER_CG = C_CALIPER_POS_6;

	//HTK 2022-03-30 Tact Time Display 추가
	m_clockProcStartTime[2] = clock();
	m_clockProcStartTime[3] = clock();

	FakeDC* pDC = NULL;
	CPen penROI(PS_SOLID, 3, RGB(255, 0, 0));
	CPen penFit(PS_SOLID, 3, RGB(0, 255, 255));
	CPen penDot(PS_DOT, 3, COLOR_YELLOW);
	CPen penS(PS_SOLID, 3, RGB(255, 255, 0));
	CPen penE(PS_SOLID, 3, RGB(0, 0, 255));
	CPen penMetalLine(PS_SOLID, 5, RGB(0, 255, 0));
	CPen penPanelLine(PS_SOLID, 5, RGB(255, 0, 0));

	BOOL breturn = TRUE;

	if (mViewer != NULL)
	{
		mViewer->clearAllFigures();
		mViewer->ClearOverlayDC();
		pDC = mViewer->getOverlayDC();
	}

	cv::Mat bin, bin2;
	std::vector<Contour> contours;
	std::vector<cv::Vec4i> hierarchy;

	double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(0, 0);
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);

	if (fscanLength == 0) fscanLength = 10;
	float fFullScanLength = fscanLength / xres;
	double diffAcc = m_dAccLineLength / xres;
	double sumAccDec = (m_dAccLineLength + m_dDecLineLength) / xres;
	cv::Rect inspRoi = cv::Rect(int(ptStart.x), int(ptStart.y), ptEnd.x - ptStart.x, ptEnd.y - ptStart.y);

	if (inspRoi.x < 0) inspRoi.x = 0;
	if (inspRoi.y < 0) inspRoi.y = 0;
	if ((inspRoi.width + inspRoi.x) > src.cols) inspRoi.width = src.cols - inspRoi.x - 1;
	if ((inspRoi.height + inspRoi.y) > src.rows) inspRoi.height = src.rows - inspRoi.y - 1;

	if (inspRoi.width <= 0 || inspRoi.height <= 0) return FALSE;

	/*cv::threshold(src(inspRoi), bin, -1, 255, CV_THRESH_BINARY | THRESH_OTSU);
	cv::GaussianBlur(bin, bin, cv::Size(0, 0), 3.);

	if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getSearchEdgePolarity() == 0)
		cv::threshold(bin, bin2, -1, 255, CV_THRESH_BINARY | THRESH_OTSU);
	else cv::threshold(bin, bin2, -1, 255, CV_THRESH_BINARY_INV | THRESH_OTSU);*/

	double _dFindThresh = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getFeriAlignFindThresh();
	GaussianBlur(src(inspRoi), bin, cv::Size(0, 0), 3.);
	sobelDirection(3, SOBEL_UP, bin, bin);
	cv::threshold(bin, bin2, _dFindThresh, 255, CV_THRESH_BINARY);

	cv::Mat mask = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3), cv::Point(1, 1));
	/*dilate(bin2, bin2, mask, cv::Point(-1, -1), 1);
	erode(bin2, bin2, mask, cv::Point(-1, -1), 1);*/

	TPointList2D points;
	std::array<float, 2> arr;
	int nCount = 0;

	uchar* buf = bin2.data;
	std::vector<double> x_series;
	std::vector<double> y_series;
	std::vector<double> destX;
	std::vector<double> destY;
	std::vector<cv::Point2f> ptrPoint;

	for (int i = 0; i < C_CALIPER_POS_6; i++)
	{
		memset(&m_TraceSearch_line_info[i], 0, sizeof(sLine)); //Tkyuha 20221123 Line검사 수정
		m_bTraceSearchFindLine[i] = FALSE;                   //Tkyuha 20221123 Line검사 수정
	}

	if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getSearchSearchDir() == 0)
	{
		for (int i = 0; i < bin2.cols; i++)
		{
			for (int j = 0; j < bin2.rows - 1; j++)
			{
				if (buf[j * bin2.cols + i] != 0)
				{
					arr.at(0) = i + ptStart.x;
					arr.at(1) = j + ptStart.y;
					points.push_back(arr);
					break;
				}
			}
		}
	}
	else
	{
		for (int i = 0; i < bin2.cols; i++)
		{
			for (int j = bin2.rows - 1; j > 0; j--)
			{
				if (buf[j * bin2.cols + i] != 0)
				{
					arr.at(0) = i + ptStart.x;
					arr.at(1) = j + ptStart.y;
					points.push_back(arr);
					break;
				}
			}
		}
	}

	CString str;
	str.Format("find size - %d", points.size());
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

	if (points.size() > 10)
	{
		if (pDC != NULL)
		{
			/////////// Raw Search Display
			pDC->SelectObject(&penDot);
			pDC->MoveTo(int(points[0].at(0)), int(points[0].at(1)));

			for (int i = 1; i < points.size(); i++)
				pDC->LineTo(int(points[i].at(0)), int(points[i].at(1)));
			////////// Raw Search Display
		}

		nCount = int(points.size());

		double* lx = new double[MAX(int(fFullScanLength), nCount)];
		double* ly = new double[MAX(int(fFullScanLength), nCount)];
		double ax_Out = 0, bc_Out = 0, cx =  (ptStart.x + ptEnd.x) / 2.;
		int start = 0;

		BOOL bFindLine = FALSE, bFindLine2=FALSE, bFindLine3 = FALSE;
		sLine line_info, line_info2, line_info3;

		for (int x = 0; x < nCount; x++)
		{
			lx[x] = points[x].at(0) - cx;
			ly[x] = points[x].at(1);
		}

		std::vector<cv::Point> pts;
		double c = 0;	// 직선의 중심점

		for (int i = 0; i < nCount; i++) {
			cv::Point pt = cv::Point(lx[i], ly[i]);
			pts.push_back(pt);
		}

		MLSAC_LineFitting(pts, ax_Out, bc_Out, c); 
		//LeastSqrRegression(lx, ly, nCount, ax_Out, bc_Out); //MLSAC_LineFitting으로 변경
		pts.clear();

		int real_cam = m_pMain->vt_job_info[nJob].camera_index[0];
		double dx = 0.0, dy = 0.0, dt = 0.0;
		//KJH 2021-05-30 카메라 Grab Data 변경(기존 Camera Buffer에서 함수 외부에서 보내온 Camera Grab data로 수정)
		
		BYTE* pImage = src.data;

		BOOL bFixtureUse = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseFixtureMark();

		if (bFixtureUse)
		{
			int nPos = 0, nIndex = 3;
			double fixtureX = m_pMain->GetMatching(nJob).getFixtureX(0, nPos);
			double fixtureY = m_pMain->GetMatching(nJob).getFixtureY(0, nPos);

			m_pMain->GetMatching(nJob).findPatternIndex(pImage, 0, nPos, src.cols, src.rows, nIndex);

			if (m_pMain->GetMatching(nJob).getFindInfo(0, nPos).GetFound() == FIND_OK)
			{
				double posX = m_pMain->GetMatching(nJob).getFindInfo(0, nPos).GetXPos();
				double posY = m_pMain->GetMatching(nJob).getFindInfo(0, nPos).GetYPos();

				if (fixtureX != 0.0 && fixtureY != 0.0)
				{
					dx = posX - fixtureX;
					dy = posY - fixtureY;
					dt = 0.0;
				}
			}
		}
		else  // Metal 라인을 Reference로 활용 하기 위함
		{
			_st32fPoint spt = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][CALIPER_METAL].getStartPt();
			_st32fPoint ept = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][CALIPER_METAL].getEndPt();

			//m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][CALIPER_METAL].processCaliper(pImage, src.cols, src.rows, dx, dy, dt, TRUE);
			m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][CALIPER_METAL].processCaliper(pImage, src.cols, src.rows, dx, dy, dt, FALSE);
			line_info = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][CALIPER_METAL].m_lineInfo;
			bFindLine = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][CALIPER_METAL].getIsMakeLine();

			memcpy(&m_TraceSearch_line_info[C_CALIPER_POS_3], &line_info, sizeof(sLine)); //Tkyuha 20221123 Line검사 수정
			m_bTraceSearchFindLine[C_CALIPER_POS_3] = bFindLine;                          //Tkyuha 20221123 Line검사 수정

			////SJB 2022-10-31 Insp Overflow Metal Line 도포 전 Trace 측정 시 Caliper 데이터로 측정	//SJB 2022-11-29 
			//m_Trace_Metal_Line_Info = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][CALIPER_METAL].m_lineInfo;
			//m_Trace_Metal_Find_Line = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][CALIPER_METAL].getIsMakeLine();

			if (bFindLine)
			{
				//SJB 2022-11-01 Trace 측정 시 Metal Line 선 표시
				CPoint ptMetal;
				pDC->SelectObject(&penMetalLine);
				ptMetal.x = inspRoi.x, ptMetal.y = line_info.a * inspRoi.x + line_info.b;
				pDC->MoveTo(ptMetal.x, ptMetal.y);
				ptMetal.x = inspRoi.x + inspRoi.width, ptMetal.y = line_info.a * ptMetal.x + line_info.b;
				pDC->LineTo(ptMetal.x, ptMetal.y);

				double p1= spt.y, p2=ept.y;
				p1 = line_info.a * spt.x + line_info.b;
				p2 = line_info.a * ept.x + line_info.b;

				dy = ((p1+p2) - (spt.y + ept.y) )/ 2;
			}
		}

		//m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][C_CALIPER_POS_2].processCaliper(pImage, src.cols, src.rows, dx, dy, dt, TRUE);
		m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][C_CALIPER_POS_2].processCaliper(pImage, src.cols, src.rows, dx, dy, dt, FALSE);
		line_info = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][C_CALIPER_POS_2].m_lineInfo;
		bFindLine = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][C_CALIPER_POS_2].getIsMakeLine();

		memcpy(&m_TraceSearch_line_info[C_CALIPER_POS_2], &line_info, sizeof(sLine)); //Tkyuha 20221123 Line검사 수정
		m_bTraceSearchFindLine[C_CALIPER_POS_2] = bFindLine;                          //Tkyuha 20221123 Line검사 수정

		if (bFindLine)
		{
			//SJB 2022-11-01 Trace 측정 시 Panel Line 선 표시
			CPoint ptPanel;
			pDC->SelectObject(&penPanelLine);
			ptPanel.x = inspRoi.x, ptPanel.y = line_info.a * inspRoi.x + line_info.b;
			pDC->MoveTo(ptPanel.x, ptPanel.y);
			ptPanel.x = inspRoi.x + inspRoi.width, ptPanel.y = line_info.a * ptPanel.x + line_info.b;
			pDC->LineTo(ptPanel.x, ptPanel.y);
		}

		BOOL bLineShift_DoubleDispensingModeEnable=m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseDoubleDispensingMode();  // false : 

		if(bLineShift_DoubleDispensingModeEnable)
		{
			m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][C_CALIPER_POS_6].processCaliper(pImage, src.cols, src.rows, dx, dy, dt, TRUE);
			line_info2 = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][C_CALIPER_POS_6].m_lineInfo;
			bFindLine2 = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][C_CALIPER_POS_6].getIsMakeLine();

			m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][C_CALIPER_POS_5].processCaliper(pImage, src.cols, src.rows, dx, dy, dt, TRUE);   //b 
			line_info3 = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][C_CALIPER_POS_5].m_lineInfo;
			bFindLine3 = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][C_CALIPER_POS_5].getIsMakeLine();
		}

		if (bFindLine)
		{
			double _sobelYline = fabs(src.rows/2 - (ax_Out* (src.cols/2)+ bc_Out));
			double _caliperYline = fabs(src.rows / 2 - (line_info.a * (src.cols / 2) + line_info.b));

			//if (fabs(line_info.a) < fabs(ax_Out) || _sobelYline > _caliperYline)
			{
				ax_Out = line_info.a;
				bc_Out = line_info.b;
			}
		}
		else breturn = FALSE;

		//KBJ 2022-11-30 Center Align 각도 스펙 아웃 알림창 및 오버레이 추가
		double dAngle = fabs(ax_Out) * 180.0 / CV_PI;
		if (dAngle > m_pMain->vt_job_info[nJob].model_info.getMachineInfo().getRevisionLimit(AXIS_T))
		{
			str.Format("Line Theta NG : %.4f \nspec : %.4f", dAngle, m_pMain->vt_job_info[nJob].model_info.getMachineInfo().getRevisionLimit(AXIS_T));
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

			pDC->SetTextColor(COLOR_RED);
			if (pDC != NULL) pDC->TextOutA(m_pMain->m_stCamInfo[0].w /2, 1000, str);
			ShowErrorMessageBlink(str);

			m_pMain->m_nErrorType[nJob] = ERR_TYPE_REVISION_LIMIT;
			breturn = FALSE;
		}

		if (bLineShift_DoubleDispensingModeEnable && bFindLine2 && bFindLine3)
		{
			sPoint ptrCross;
			ptrCross.x = src.cols / 2.;
			ptrCross.y = fabs((ax_Out *ptrCross.x + bc_Out));
			double _dist=m_pMain->GetMatching(nJob).compute_distance(line_info2, ptrCross)*yres;
			double _dist2 = m_pMain->GetMatching(nJob).compute_distance(line_info3, ptrCross)*yres;

			long lRevisionData[2] = { 0, };
			lRevisionData[0] = LOWORD((_dist- _dist2 )* MOTOR_SCALE);
			lRevisionData[1] = HIWORD((_dist - _dist2) * MOTOR_SCALE);

			int address = m_pMain->vt_job_info[nJob].plc_addr_info.write_word_start + 36;

			g_CommPLC.SetWord(address, 2, lRevisionData);
			str.Format("dist1 - %4.3f", (_dist - _dist2));
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

			
			lRevisionData[0] = LOWORD(_dist2 * MOTOR_SCALE);
			lRevisionData[1] = HIWORD(_dist2 * MOTOR_SCALE);
			g_CommPLC.SetWord(address+2, 2, lRevisionData);

			str.Format("dist2 - %4.3f", _dist2);
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		}

		//if (!btracking && ax_Out* src.cols /2+ bc_Out> src.rows/2-50) // 얼라인 예외처리 테스트중
		//{
		//	m_pMain->GetMatching(nJob).findPattern(pImage, 0, 0, src.cols, src.rows);

		//	if (m_pMain->GetMatching(nJob).getFindInfo(0, 0).GetFound() == FIND_OK)
		//	{
		//		double posX = m_pMain->GetMatching(nJob).getFindInfo(0, 0).GetXPos();
		//		double posY = m_pMain->GetMatching(nJob).getFindInfo(0, 0).GetYPos();

		//		if(ax_Out* src.cols / 2 + bc_Out< posY) breturn = FALSE;
		//	}
		//}

		nCount = int(fFullScanLength);		
	//	start = int(-fFullScanLength / 2); // 가감속 고려 안한 경우
		start = int(-(fFullScanLength- sumAccDec) / 2 - diffAcc); 
		// 가감속 고려한 경우 왼쪽 오른쪽으로 해야함
		// 시작 순서가 아닌 왼쪽을 가속 , 오른쪽을 감속으로 설정 해서 사용 PLC터치창
		start = start + (src.cols / 2);
		for (int j = 0; j < nCount; j++)
		{
			x_series.push_back(start);
			y_series.push_back(ax_Out * start + bc_Out);
			start++;
		}

		for (int j = 0; j < nCount; j++)
			ptrPoint.push_back(cv::Point2f(x_series.at(j)- (src.cols / 2), y_series.at(j)));

		// KBJ 2022-11-30 쓰지않아서 안그리도록
		//if (pDC != NULL)
		if(0)
		{
			///////////// 회귀분석 Search Display
			pDC->SelectObject(&penROI);
			pDC->MoveTo(int(x_series.at(0) ), int(y_series.at(0)));

			for (int i = 1; i < nCount; i++)
				pDC->LineTo(int(x_series.at(i)), int(y_series.at(i)));
			///////////// 회귀분석 Search Display
		}

		delete lx;
		delete ly;

		// 얼라인 값 계산
		double alignTheata = atan((ptrPoint[nCount - 1].y - ptrPoint[0].y) * yres / ((ptrPoint[nCount - 1].x - ptrPoint[0].x) * xres)) * 180 / CV_PI;
		double alignX = (ptStart.x + ptEnd.x) / 2.;
		double alignY = ptrPoint[nCount / 2].y;

		m_pMain->m_ELB_TraceResult.m_ELB_ResultXY = cv::Point2d(alignX, alignY);
		m_pMain->m_ELB_TraceResult.m_ELB_ResultT = alignTheata;
		m_pMain->m_ELB_TraceResult.m_ELB_ResultLine[0].a = ax_Out;
		m_pMain->m_ELB_TraceResult.m_ELB_ResultLine[0].b = bc_Out;
		m_pMain->m_ELB_TraceResult.m_ELB_ResultLine[0].sx = cx;
		//

		if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getTraceUsingAlignFlag())
		{
			int _tmpCy = (src.rows / 2) - alignY;
			for (int i = 0; i < nCount; i++)
			{
				ptrPoint.at(i).y = y_series.at(i)+ _tmpCy;
			}
			btracking = true;
		}

		if (btracking)
		{
			////////// 등간격 분류
			float x, y, r = 32;
			float rh = r / 2;// x,y: 원의 중심좌표, r: 반경
			float a, b, c, d;
			int n, cp = 0, endX = start - (src.cols / 2);// fFullScanLength / 2;

			r = int(m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspTraceInterval() / xres);
			rh = r / 2;
			double pitch = r * xres;

			bool bFlag = true;
			float xy[2][2];// 교점의 좌표 두개
			std::vector<cv::Point2f> ptrInterSectionPoint;

			x = ptrPoint.at(0).x;		y = ptrPoint.at(0).y;
			a = ptrPoint.at(0).x;		b = ptrPoint.at(0).y;
			c = ptrPoint.at(r).x;		d = ptrPoint.at(r).y;

			ptrInterSectionPoint.push_back(cv::Point2f(x, y));

			if (1)
			{
				int _soffset = x;
				int _si = 0;
				double  _lastpointx = ptrPoint[ptrPoint.size() - 1].x;
				for (int i = x; i < _lastpointx; i += r)
				{
					_soffset += r;
					_si += r;
					if (_soffset <= _lastpointx)
					{
						float _nx = ptrPoint.at(_si).x;
						float _ny = ptrPoint.at(_si).y;

						ptrInterSectionPoint.push_back(cv::Point2f(_nx, _ny));
					}
					else break;
				}
			}
			else
			{
				while (bFlag) // 정상 x>0 && (cp< ptr->size() - 1 || ptr->at(ptr->size()-1).x>x)
				{
					//n = CircleLineIntersection(x, y, r, a, b, c, d, xy);
					n = FindLineCircleIntersection(x, y, r, cv::Point2f(a, b), cv::Point2f(c, d), xy);

					if (n == 0)		break;
					else if (n == 1) { x = xy[0][0];			y = xy[0][1]; }
					else if (n == 2)
					{
						if (a < c) { x = xy[1][0];			y = xy[1][1]; }
						else if (a == c)
						{
							if (b < d) { x = xy[0][0];			y = xy[0][1]; }
							else { x = xy[1][0];			y = xy[1][1]; }
						}
						else { x = xy[0][0];			y = xy[0][1]; }

					}

					float minDiff = 99999;
					int mpos = -1;

					if (endX > x)
					{
						for (int j = cp; j < ptrPoint.size(); j++)
						{
							float diffSum = fabs(x - ptrPoint.at(j).x) + fabs(y - ptrPoint.at(j).y);
							if (diffSum < minDiff)
							{
								mpos = j;
								minDiff = diffSum;
							}
						}
						if (mpos == -1)	ptrInterSectionPoint.push_back(cv::Point2f(x, y));
						else ptrInterSectionPoint.push_back(cv::Point2f(ptrPoint.at(mpos).x, ptrPoint.at(mpos).y));
					}

					minDiff = 99999;
					for (int j = cp; j < ptrPoint.size() - rh; j++)
					{
						float diffSum = fabs(x - ptrPoint.at(j).x) + fabs(y - ptrPoint.at(j).y);
						if (diffSum < minDiff)
						{
							a = ptrPoint.at(j).x;		b = ptrPoint.at(j).y;
							c = ptrPoint.at(j + rh).x;		d = ptrPoint.at(j + rh).y;
							minDiff = diffSum;
							cp = j;
						}
					}

					float diffSum2 = fabs(x - ptrInterSectionPoint[0].x) + fabs(y - ptrInterSectionPoint[0].y);
					if (minDiff > 20 || diffSum2 < 20) bFlag = false;
				}
			}
			if (fabs(endX - ptrInterSectionPoint[ptrInterSectionPoint.size() - 1].x) > rh)
				ptrInterSectionPoint.push_back(cv::Point2f(endX, ptrInterSectionPoint[ptrInterSectionPoint.size() - 1].y));

			////////// 등간격 분류
			if (pDC != NULL)
			{
				pDC->SelectObject(&penROI);

				for (int i = 0; i < ptrInterSectionPoint.size(); i++)
				{
					if (i < 5) pDC->SelectObject(&penS);
					else if (i < 10) pDC->SelectObject(&penE);
					else pDC->SelectObject(&penROI);

					n = ptrInterSectionPoint[i].x + src.cols / 2.; //
					pDC->Ellipse(int(n - 2), int(ptrInterSectionPoint[i].y - 2), int(n + 2), int(ptrInterSectionPoint[i].y + 2));
				}

				if (ptrInterSectionPoint.size() > 2)
				{
					int _rad = 25;
					int _cx = src.cols / 2;
					int _cy = MIN(src.rows - 1, MAX(0, int(ptrInterSectionPoint[ptrInterSectionPoint.size() / 2].y)));

					pDC->Rectangle(CRect(cx - _rad, _cy - 2 * _rad, cx, _cy - _rad));
					pDC->Rectangle(CRect(cx - _rad, _cy + _rad, cx, _cy + 2 * _rad));
					pDC->Rectangle(CRect(cx, _cy - 2 * _rad, cx + _rad, _cy - _rad));
					pDC->Rectangle(CRect(cx, _cy + _rad, cx + _rad, _cy + 2 * _rad));
				}
			}


			m_dCircleInBright = 255;
			if (ptrInterSectionPoint.size() > 2)
			{
				int _rad = 30;
				int _cx = src.cols / 2;
				int _cy = MIN(src.rows - 1, MAX(0, int(ptrInterSectionPoint[ptrInterSectionPoint.size() / 2].y)));

				if (_cy == 0)
				{
					_cy = src.rows / 2;
				}

				double t_mean = 0;
				t_mean += (cv::mean(src(cv::Rect(cx - _rad, _cy - 2 * _rad, _rad, _rad)))).val[0]; //left				
				t_mean += (cv::mean(src(cv::Rect(cx - _rad, _cy + _rad, _rad, _rad)))).val[0];      //right
				t_mean += (cv::mean(src(cv::Rect(cx, _cy - 2 * _rad, _rad, _rad)))).val[0]; //top
				t_mean += (cv::mean(src(cv::Rect(cx, _cy + _rad, _rad, _rad)))).val[0]; //bottom
				m_dCircleInBright = t_mean / 4.;
			}

			breturn = TRUE;

			CString strFileDir, msg;
			SYSTEMTIME	csTime;
			::GetLocalTime(&csTime);

			CString	Time_str = m_pMain->m_strResultTime[nJob];
			CString	Date_str = m_pMain->m_strResultDate[nJob];

			int nS = int(ptrInterSectionPoint.size());  
			
			m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.x = ptrInterSectionPoint[0].x * xres;
			m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.y = (ptrInterSectionPoint[0].y - ptrInterSectionPoint[nS / 2].y) * yres;

			m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.x = ptrInterSectionPoint[nS - 1].x * xres;
			m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.y = (ptrInterSectionPoint[nS - 1].y - ptrInterSectionPoint[nS / 2].y) * yres;


			for (int i = 0; i < 5; i++) m_pMain->m_ELB_TraceResult.m_vTraceProfile[i].clear();

			bool bCircle_ShapeDispensingModeEnable = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseDummyCornerInsp();
			double  dCenterDistanceSpec = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(0);

			for (int i = 0; i < ptrInterSectionPoint.size(); i++)
			{
				c = (ptrInterSectionPoint[i].x - ptrInterSectionPoint[0].x) * xres;
				d = (ptrInterSectionPoint[i].y - ptrInterSectionPoint[0].y) * yres;
				if (!bCircle_ShapeDispensingModeEnable) d = 0;

				m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].push_back(c);
				m_pMain->m_ELB_TraceResult.m_vTraceProfile[1].push_back(d);
				m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].push_back(0);

				if (fabs(src.rows / 2 - ptrInterSectionPoint[i].y)*yres> dCenterDistanceSpec) breturn = FALSE;;
			}

			if (Time_str == "" || Date_str == "")
			{
				CTime NowTime;
				NowTime = CTime::GetCurrentTime();
				m_pMain->m_strResultDate[nJob].Format("%04d%02d%02d", NowTime.GetYear(), NowTime.GetMonth(), NowTime.GetDay());
				m_pMain->m_strResultTime[nJob].Format("%02d_%02d_%02d", NowTime.GetHour(), NowTime.GetMinute(), NowTime.GetSecond());

				Time_str = m_pMain->m_strResultTime[nJob];
				Date_str = m_pMain->m_strResultDate[nJob];
			}

			//KJH 2021-08-15 Line Trace Log Cell ID로 변경
			CString Algo_Name = m_pMain->vt_job_info[nJob].job_name.c_str();
			CString Cell_Name = m_pMain->vt_job_info[nJob].get_main_object_id();

			if (strlen(Cell_Name) < 10)
			{
				Cell_Name.Format("TEST_%s", Time_str);
			}

			strFileDir.Format("%sFiles\\", m_pMain->m_strResultDir);
			if (_access(strFileDir, 0) != 0)	CreateDirectory(strFileDir, NULL);
			strFileDir.Format("%s%s\\", strFileDir, Date_str);
			if (_access(strFileDir, 0) != 0)	CreateDirectory(strFileDir, NULL);
			strFileDir.Format("%s%s\\", strFileDir, m_pMain->m_strCurrentModelName);
			if (_access(strFileDir, 0) != 0)	CreateDirectory(strFileDir, NULL);
			strFileDir.Format("%s%s\\", strFileDir, Algo_Name);
			if (_access(strFileDir, 0) != 0)	CreateDirectory(strFileDir, NULL);
			strFileDir.Format("%sTrace_RawData\\", strFileDir);
			if (_access(strFileDir, 0) != 0)	CreateDirectory(strFileDir, NULL);

			msg.Format("%s%s_LineTraceProfileResult_%d.csv", strFileDir, Cell_Name, m_dLineNum);

			//system_clock::time_point start_t = system_clock::now();

			if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getLineShift_DispensingModeEnable())
			{
				// 20220407 Tkyuha PLC와 협의후 적용할것 //직선 도포에서 Y를 움직이기 위함
				// m_posLineY_Move , m_distLineY_Move 로 바꿔줘야함 ,  아래는 임시 적용중
				//int _nPos = int(m_posLineY_Move/m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspTraceInterval()+0.5);
				//double _nHeight = m_distLineY_Move;
				int _nPos = atoi(((CPaneAuto*)m_pMain->m_pPane[PANE_AUTO])->m_grid_ref_data_viewer.GetItemText(1, 1));  
				double _nHeight = atof(((CPaneAuto*)m_pMain->m_pPane[PANE_AUTO])->m_grid_ref_data_viewer.GetItemText(1, 2)); 

				int _nC = MAX(1,int(ptrInterSectionPoint.size() - _nPos+1)),_inc=0;
				double _a = _nHeight / _nC;
				for (int i = _nC; i < ptrInterSectionPoint.size(); i++)
				{
					m_pMain->m_ELB_TraceResult.m_vTraceProfile[1].at(i) += (_inc *_a);
					_inc++;
				}				
			}

			ofstream  writeFile(msg);

			if (writeFile.is_open()) {
				msg.Format("%d, %f, %f, %f, %f\n", ptrInterSectionPoint.size(), m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.x, m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.y, m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.x, m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.y);
				writeFile << msg;

				for (int i = 0; i < ptrInterSectionPoint.size(); i++)
				{
					c = m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].at(i);
					d = m_pMain->m_ELB_TraceResult.m_vTraceProfile[1].at(i);
					msg.Format("%f, %f, %f,  %f, %f\n", ptrInterSectionPoint[i].x, ptrInterSectionPoint[i].y, c, d, 0);
					writeFile << msg;
				}

				writeFile.close();
			}

			//HTK 2022-03-30 Tact Time Display 추가
			m_clockProc_elapsed_time[3] = (clock() - m_clockProcStartTime[3]) / (float)CLOCKS_PER_SEC;
			((CPaneAuto*)m_pMain->m_pPane[PANE_AUTO])->update_grid_processing_time(1, m_clockProc_elapsed_time[3], 3, true);

			//KJH 2022-07-27 Tact Time Log 추가
			theLog.logmsg(LOG_PLC_SEQ, "Trace Calc Tact Time = %.4f", m_clockProc_elapsed_time[3]);

			m_clockProc_elapsed_time[4] = clock();

			if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseRealTimeTrace())
			{
				CString str;
				str.Format("sendTraceProfileData - Start");
				::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

				m_pMain->sendTraceProfileData(m_pMain->vt_job_info[nJob].plc_addr_info.write_word_sub_start,
					m_pMain->vt_job_info[nJob].plc_addr_info.write_word_start, pitch);

				str.Format("sendTraceProfileData - OK");
				::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
			}
			else
			{
				CString str;
				str.Format("sendTraceProfileData - SKIP");
				::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
			}
			
			//HTK 2022-03-30 Tact Time Display 추가
			m_clockProc_elapsed_time[4] = (clock() - m_clockProcStartTime[4]) / (float)CLOCKS_PER_SEC;
			((CPaneAuto*)m_pMain->m_pPane[PANE_AUTO])->update_grid_processing_time(1, m_clockProc_elapsed_time[4], 4, true);

			//KJH 2022-07-27 Tact Time Log 추가
			theLog.logmsg(LOG_PLC_SEQ, "Trace Data PLC Send Tact Time = %.4f", m_clockProc_elapsed_time[4]);

			////system_clock::time_point end_t = system_clock::now();
			////nanoseconds nano = end_t - start_t;

			////str.Format("%f", nano.count() / 1000000.);
			////::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

#ifdef _SAPERA
#pragma region 도포 유무 검사
			if (bdust && m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getUseInputInspection())
			{
				cv::Mat dust;
				cv::Point2f pointsRotate[4];
				float shiftOffset = -130.;

				start = ptrInterSectionPoint[0].x + cx;
				pointsRotate[0].x = start;
				pointsRotate[0].y = ax_Out * start + bc_Out;
				pointsRotate[3].x = start;
				pointsRotate[3].y = ax_Out * start + bc_Out + shiftOffset;

				start = ptrInterSectionPoint[ptrInterSectionPoint.size() - 1].x + cx;
				pointsRotate[1].x = start;
				pointsRotate[1].y = ax_Out * start + bc_Out;
				pointsRotate[2].x = start;
				pointsRotate[2].y = ax_Out * start + bc_Out + shiftOffset;

				cv::Mat maskImg = cv::Mat::zeros(src.rows, src.cols, CV_8UC1);

				cv::Point vertices[4];
				for (int i = 0; i < 4; ++i)
				{
					vertices[i].x = MIN(src.cols - 1, MAX(0, pointsRotate[i].x));
					vertices[i].y = MIN(src.rows - 1, MAX(0, pointsRotate[i].y));
				}

				cv::fillConvexPoly(maskImg, vertices, 4, cv::Scalar(255, 255, 255));
				src.copyTo(dust, maskImg);

				inspRoi.x = MIN(dust.cols - 1, MAX(0, vertices[0].x));
				inspRoi.y = MIN(dust.rows - 1, MAX(0, vertices[2].y));
				inspRoi.width = MIN(dust.cols - inspRoi.x, MAX(0, vertices[2].x - inspRoi.x));
				inspRoi.height = MIN(dust.rows - inspRoi.y, MAX(0, vertices[1].y - inspRoi.y));

				cv::Mat s_down; // 축소 해서 검사 진행
				cv::resize(dust(inspRoi), s_down, cv::Size(), 0.5, 0.5, CV_INTER_CUBIC);

				int dThresh	= m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getExistDummyGray();
				int spec_S	= MAX(10, int(m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(2) / xres) / 2); //최소 크기
				int spec_A	= MAX(250, int(m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(2) / xres) / 2); //최소 면적

				int nDefect = InspDustBurrInspection(nJob, s_down, dThresh, inspRoi.x, inspRoi.y, 2, spec_S, spec_A, true, true); //10,25

				if (nDefect > 0)
				{
					for (int i = 0; i< int(m_vecDustResult.size()); i++)
					{
						m_vecDustResult.at(i).center.x = m_vecDustResult.at(i).center.x * 2 + inspRoi.x;
						m_vecDustResult.at(i).center.y = m_vecDustResult.at(i).center.y * 2 + inspRoi.y;
						m_vecDustResult.at(i).rr.center.x = m_vecDustResult.at(i).rr.center.x * 2 + inspRoi.x;
						m_vecDustResult.at(i).rr.center.y = m_vecDustResult.at(i).rr.center.y * 2 + inspRoi.y;
					}
				}

				if (nDefect > 0)
				{
					CString strText;
					CFont font, * oldFont;
					font.CreateFont(50, 50, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
						OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
					oldFont = pDC->SelectObject(&font);
					pDC->SelectObject(GetStockObject(NULL_BRUSH));
					pDC->SetTextColor(COLOR_RED);
					pDC->SelectObject(&penE);

					for (int i = 0; i < nDefect; i++)
					{
						cv::RotatedRect rr = m_vecDustResult.at(i).rr;
						cv::Rect brect = rr.boundingRect();
						cv::Point2f vertices2f[4];
						rr.points(vertices2f);

						cv::Point vertices[4];
						for (int j = 0; j < 4; ++j) {
							vertices[j] = vertices2f[j];
						}

						pDC->MoveTo(vertices[0].x, vertices[0].y);
						pDC->LineTo(vertices[1].x, vertices[1].y);
						pDC->LineTo(vertices[2].x, vertices[2].y);
						pDC->LineTo(vertices[3].x, vertices[3].y);
						pDC->LineTo(vertices[0].x, vertices[0].y);

						strText.Format("[S: %.4f,%.4fmm, A: %.2f]", m_vecDustResult.at(i).xw * xres, m_vecDustResult.at(i).yw * yres, m_vecDustResult.at(i).area * 2);
						pDC->TextOutA(m_vecDustResult.at(i).center.x, m_vecDustResult.at(i).center.y, strText);
					}

					font.DeleteObject();
				}

				if (m_vecDustResult.size() > 0 && theApp.m_bEnforceOkFlag == FALSE)
				{
					breturn = FALSE;
					str.Format("Input Inspection Dust Count = %d NG", m_vecDustResult.size());
					::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
				}

				maskImg.release();
				dust.release();
			}
#pragma endregion
#endif
			ptrInterSectionPoint.clear();
		}
		else
		{
			//HTK 2022-03-30 Tact Time Display 추가
			m_clockProc_elapsed_time[2] = (clock() - m_clockProcStartTime[2]) / (float)CLOCKS_PER_SEC;
			((CPaneAuto*)m_pMain->m_pPane[PANE_AUTO])->update_grid_processing_time(1, m_clockProc_elapsed_time[2], 2, true);

			//KJH 2022-07-27 Tact Time Log 추가
			theLog.logmsg(LOG_PLC_SEQ, "Center Align Tact Time = %.4f", m_clockProc_elapsed_time[2]);
		}

		if (pDC != NULL)	mViewer->Invalidate();

		penROI.DeleteObject();
		penFit.DeleteObject();
		penDot.DeleteObject();
		penS.DeleteObject();
		penE.DeleteObject();
		penPanelLine.DeleteObject();
		penMetalLine.DeleteObject();

		x_series.clear();
		y_series.clear();
		destX.clear();
		destY.clear();
		ptrPoint.clear();
		contours.clear();
		hierarchy.clear();

		mask.release();
		bin.release();
		bin2.release();
	}
	else breturn = FALSE;

	return breturn;
}

void CFormMainView::OnClose()
{
	if (m_bThreadMainDisplayEndFlag == TRUE)
	{
		m_bThreadMainDisplayEndFlag = FALSE;
		WaitForSingleObject(m_pThread->m_hThread, 3000);
	}

	CFormView::OnClose();
}

int CFormMainView::read_profile_data(CString str_path)
{
	FILE* fp = fopen(str_path, "r");
	char cData[MAX_PATH] = { 0, };

	if (fp == NULL) return -1;
	int num_of_data = 0;
	double rot_x = 0.0, rot_y = 0.0;
	double rev_rot_x = 0.0, rev_rot_y = 0.0;

	int ret = 0;
	ret = fscanf(fp, "%d, %lf, %lf, %lf, %lf", &num_of_data, &rot_x, &rot_y, &rev_rot_x, &rev_rot_y);

	//int lineCount = int(m_ELB_TraceResult.m_vTraceProfile[0].size());
	const int DATA_SET = 5;
	double data[DATA_SET] = { 0, };
	m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.x = rot_x;
	m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.y = rot_y;
	m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.x = rev_rot_x;
	m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.y = rev_rot_y;

	for (int i = 0; i < DATA_SET; i++)
	{
		m_pMain->m_ELB_TraceResult.m_vTraceProfile[i].clear();
	}

	for (int i = 0; i < num_of_data; i++)
	{
		ret = fscanf(fp, "%lf, %lf, %lf, %lf, %lf\n", &data[0], &data[1], &data[2], &data[3], &data[4]);
		m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].push_back(data[0]);
		m_pMain->m_ELB_TraceResult.m_vTraceProfile[1].push_back(data[1]);
		m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].push_back(data[2]);
		m_pMain->m_ELB_TraceResult.m_vTraceProfile[3].push_back(data[3]);
		m_pMain->m_ELB_TraceResult.m_vTraceProfile[4].push_back(data[4]);
	}

	fclose(fp);
	return 0;
}
void CFormMainView::sendGoldData(int nJob)
{
	if (!m_pMain->m_strGoldenDataFile.IsEmpty())
	{
		CString fileName = m_pMain->m_strGoldenDataFile, tmpFileName;
		int method = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod();

		if (method == METHOD_LINE)
		{
			if (m_dLineNum != 1)
			{
				tmpFileName.Format("%s_%d.csv", fileName.Left(m_pMain->m_strGoldenDataFile.GetLength() - 4), m_dLineNum);
				fileName = tmpFileName;
			}
		}

		std::ifstream openFile(fileName);
		double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(0, 0);
		double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);

		if (openFile.is_open())
		{
			for (int i = 0; i < 5; i++) m_pMain->m_ELB_TraceResult.m_vTraceProfile[i].clear();

			std::string line;
			getline(openFile, line);
			vector<string> xX = split2(line, ',');

			int lineCount = atoi(xX.at(0).c_str());
			double rotateX = atof(xX.at(1).c_str());
			double rotateY = atof(xX.at(2).c_str());
			int cam = m_pMain->vt_job_info[nJob].camera_index[0];
			double xw = m_pMain->m_stCamInfo[cam].w / 2;
			double yh = m_pMain->m_stCamInfo[cam].h / 2;
			double xof = 0, yof = 0;
			vector<double> x_vector;
			vector<double> y_vector;

			if (method == METHOD_CIRCLE || method == METHOD_LINE)
			{
				m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.x = -rotateX;
				m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.y = -rotateY;
			}
			else
			{
				//rotateX *= -1;
				//rotateY *= -1;
				if (xX.size() >= 5)
				{
					m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.x = atof(xX.at(3).c_str());
					m_pMain->m_ELB_TraceResult.m_vELB_ReverseRotateCenter.y = atof(xX.at(4).c_str());
				}
			}

			m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.x = rotateX;
			m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.y = rotateY;

			xof = MAX(0, MIN(xw * 2 - 1, xw + rotateX / xres)), yof = MAX(0, MIN(yh * 2 - 1, yh + rotateY / yres));

			while (getline(openFile, line))
			{
				vector<string> x = split2(line, ',');
				double rx = atof(x.at(2).c_str());  //3
				double ry = (atof(x.at(3).c_str())); //4
				double rt = atof(x.at(4).c_str()); // 임시

				if (method == METHOD_CIRCLE || method == METHOD_LINE)
				{
				}
				else
				{
					//rx *= -1;
					//ry *= -1;
				}

				m_pMain->m_ELB_TraceResult.m_vTraceProfile[0].push_back(rx);
				m_pMain->m_ELB_TraceResult.m_vTraceProfile[1].push_back(ry);
				m_pMain->m_ELB_TraceResult.m_vTraceProfile[2].push_back(rt);
				m_pMain->m_ELB_TraceResult.m_vTraceProfile[3].push_back(atof(x.at(0).c_str()) / xres + xof);
				m_pMain->m_ELB_TraceResult.m_vTraceProfile[4].push_back(atof(x.at(1).c_str()) / yres + yof);

				x_vector.push_back(xof - (atof(x.at(0).c_str()) / xres));
				y_vector.push_back(yof - (atof(x.at(1).c_str()) / yres));

				x.clear();
			}

			if (method == METHOD_CIRCLE || method == METHOD_LINE)
			{
			}
			else
			{
				for (int i = int(x_vector.size() - 1); i >= 0; i--)
				{
					m_pMain->m_ELB_TraceResult.m_vTraceProfile[3].push_back(x_vector.at(i));
					m_pMain->m_ELB_TraceResult.m_vTraceProfile[4].push_back(y_vector.at(i));
				}
			}

			x_vector.clear();
			y_vector.clear();
		}
		openFile.close();

		double r = int(m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspTraceInterval() / xres);
		double pitch = r * xres;

		if (method == METHOD_CIRCLE)
		{
			m_pMain->sendTraceProfileCircleData(m_pMain->vt_job_info[nJob].plc_addr_info.write_word_sub_start,
				m_pMain->vt_job_info[nJob].plc_addr_info.write_word_start, pitch);
		}
		else if (method == METHOD_NOTCH)
			m_pMain->sendTraceProfileNotchData(m_pMain->vt_job_info[nJob].plc_addr_info.write_word_sub_start,
				m_pMain->vt_job_info[nJob].plc_addr_info.write_word_start, pitch);
		else
			m_pMain->sendTraceProfileData(m_pMain->vt_job_info[nJob].plc_addr_info.write_word_sub_start,
				m_pMain->vt_job_info[nJob].plc_addr_info.write_word_start, pitch);
	}
}

bool CFormMainView::process_find_notch_metal(BYTE* procImg, int nJob, int nCam, int w, int h, CViewerEx* pViewer)
{
	bool is_ok = false;
	if (m_pMain->m_vtTraceData.size() < 50) return is_ok;

	cv::Mat img(h, w, CV_8UC1, procImg);
	cv::Mat filter;
	cv::Mat binary;

	cv::medianBlur(img, filter, 3);

	//cv::threshold(filter, binary, 0, 255, cv::THRESH_OTSU);

	is_ok = find_notch_metal_edge(filter.data, nJob, nCam, w, h);				// Trace 데이터 법선 방향으로 도포 폭을 찾자.
	if (is_ok != true)
	{
		CString str_temp;
		str_temp.Format("find_notch_metal_edge - NG");
		::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str_temp);
	}

	if (is_ok)	is_ok = judge_notch_metal(nJob, nCam);

	display_notch_metal(nJob, nCam, pViewer);


	filter.release();
	binary.release();


	pViewer->OnLoadImageFromPtr(procImg);
	pViewer->Invalidate();
	is_ok = true;

	return is_ok;
}
bool CFormMainView::judge_notch_dopo_insp(int nJob, int nCam)
{
	bool is_ok = true;
	double spec_top = 0.35;
	double spec_metal_t = 0.03;
	double spec_metal_b = 0.1;

	//if (m_pMain->notch_wetout_data.dist_up_max > spec_top)		is_ok = false;

	double res_x = m_pMain->GetMachine(nJob).getCameraResolutionX(nCam, 0);
	double res_y = m_pMain->GetMachine(nJob).getCameraResolutionY(nCam, 0);

	CString str_temp;

	int ng_count = 0;
	for (int i = 0; i < m_pMain->notch_wetout_data.pt_up.size(); i++)
	{
		if (m_pMain->notch_wetout_data.pt_up[i].x == 0 || m_pMain->notch_wetout_data.pt_up[i].y == 0) continue;

		double dx = (m_pMain->notch_wetout_data.pt_center[i].x - m_pMain->notch_wetout_data.pt_up[i].x) * res_x;
		double dy = (m_pMain->notch_wetout_data.pt_center[i].y - m_pMain->notch_wetout_data.pt_up[i].y) * res_y;
		double dist = sqrt(dx * dx + dy * dy);

		if (dist > spec_top)
		{
			ng_count++;
			str_temp.Format("up%d - %.3f - NG", i, dist);
			::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str_temp);
			m_pMain->notch_wetout_data.judge_up[i] = false;
		}
		else
			m_pMain->notch_wetout_data.judge_up[i] = true;
	}

	if (ng_count > 2)	is_ok = false;


	// 양끝 2포인트는 일단 pass

	for (int i = 2; i < m_pMain->notch_wetout_data.pt_metal.size() - 2; i++)
	{
		if (m_pMain->notch_wetout_data.pt_down[i].x == 0 || m_pMain->notch_wetout_data.pt_down[i].y == 0) continue;

		double dx = (m_pMain->notch_wetout_data.pt_metal[i].x - m_pMain->notch_wetout_data.pt_down[i].x) * res_x;
		double dy = (m_pMain->notch_wetout_data.pt_metal[i].y - m_pMain->notch_wetout_data.pt_down[i].y) * res_y;
		double dist = sqrt(dx * dx + dy * dy);

		double dx_metal = m_pMain->notch_wetout_data.pt_metal[i].x - m_pMain->notch_wetout_data.pt_center[i].x;
		double dy_metal = m_pMain->notch_wetout_data.pt_metal[i].y - m_pMain->notch_wetout_data.pt_center[i].y;
		double dist_metal = sqrt(dx_metal * dx_metal + dy_metal * dy_metal);

		double dx_down = m_pMain->notch_wetout_data.pt_down[i].x - m_pMain->notch_wetout_data.pt_center[i].x;
		double dy_down = m_pMain->notch_wetout_data.pt_down[i].y - m_pMain->notch_wetout_data.pt_center[i].y;
		double dist_down = sqrt(dx_down * dx_down + dy_down * dy_down);

		// 거리는 방향이 없으니 center에서 down까지의 거리와 metal 까지의 거리를 비교하여
		// 미충진 인지 오버플로우 인지 판단..

		if (dist_down < dx_metal)	// 미충진 조건
		{
			if (dist > spec_metal_t)
			{
				str_temp.Format("metal t %d - %.3f - NG", i, dist);
				::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str_temp);

				is_ok = false;
				m_pMain->notch_wetout_data.judge_down[i] = false;
			}
			else
				m_pMain->notch_wetout_data.judge_down[i] = true;
		}
		else
		{
			if (dist > spec_metal_b)
			{
				str_temp.Format("metal b %d - %.3f - NG", i, dist);
				::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str_temp);

				is_ok = false;
				m_pMain->notch_wetout_data.judge_down[i] = false;
			}
			else
				m_pMain->notch_wetout_data.judge_down[i] = true;
		}
	}

	return is_ok;
}
bool CFormMainView::judge_notch_metal(int nJob, int nCam)
{
	bool is_ok = true;
	double spec = 0.05;
	char szValue[MAX_PATH] = { 0, };
	m_pMain->m_iniFile.GetProfileStringA("SERVER_INFO", "NOTCH_METAL_NG_SPEC", "0.05", szValue, MAX_PATH);
	spec = atof(szValue);

	double res_x = m_pMain->GetMachine(nJob).getCameraResolutionX(nCam, 0);
	double res_y = m_pMain->GetMachine(nJob).getCameraResolutionY(nCam, 0);

	int size = int(m_pMain->notch_wetout_data.pt_metal.size());

	m_pMain->notch_wetout_data.judge_metal.clear();
	m_pMain->notch_wetout_data.dist_metal.clear();

	m_pMain->notch_wetout_data.judge_metal.assign(size, false);
	m_pMain->notch_wetout_data.dist_metal.assign(size, 0.0);


	CString str_temp;

	int continuous_count = 0;
	for (int i = 0; i < m_pMain->notch_wetout_data.pt_metal.size(); i++)
	{
		if (m_pMain->notch_wetout_data.pt_metal[i].x == 0 || m_pMain->notch_wetout_data.pt_metal[i].y == 0 ||
			m_pMain->notch_wetout_data.pt_center.size() <= i) continue;

		double dx = (m_pMain->notch_wetout_data.pt_center[i].x - m_pMain->notch_wetout_data.pt_metal[i].x) * res_x;
		double dy = (m_pMain->notch_wetout_data.pt_center[i].y - m_pMain->notch_wetout_data.pt_metal[i].y) * res_y;
		double dist = sqrt(dx * dx + dy * dy);

		m_pMain->notch_wetout_data.dist_metal[i] = dist;

		if (dist < spec)
		{
			continuous_count++;
			m_pMain->notch_wetout_data.judge_metal[i] = false;
		}
		else
		{
			continuous_count = 0;
			m_pMain->notch_wetout_data.judge_metal[i] = true;
		}

		if (continuous_count > 3)
		{
			str_temp.Format("metal dist %.3f - %.3f", dist, spec);
			::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str_temp);
			is_ok = false;
		}
	}

	return is_ok;
}
bool CFormMainView::process_notch_dopo_inspection(BYTE* procImg, BYTE* viewImg, int nJob, int nCam, int w, int h, CViewerEx* pViewer)
{
	bool is_ok = false;
	m_pMain->notch_wetout_data.pt_up.clear();
	m_pMain->notch_wetout_data.pt_down.clear();
	m_pMain->notch_wetout_data.pt_center.clear();
	m_pMain->notch_wetout_data.dist_up.clear();
	m_pMain->notch_wetout_data.dist_down.clear();
	m_pMain->notch_wetout_data.width_up_down.clear();


	if (m_pMain->m_vtTraceData.size() < 50) return is_ok;

	cv::Mat img(h, w, CV_8UC1, procImg);
	cv::Mat filter;
	cv::Mat binary;

	cv::medianBlur(img, filter, 5);

	cv::threshold(filter, binary, 0, 255, cv::THRESH_OTSU);

	find_notch_dopo_start_end(binary.data, nJob, nCam, w, h);		// 도포 시작 / 끝점을 찾자.		(시작 끝 점은 오츠로)
	find_notch_dopo_edge(filter.data, nJob, nCam, w, h);			// Trace 데이터 법선 방향으로 도포 폭을 찾자.

	calculate_notch_dopo_data(nJob, nCam);

	is_ok = judge_notch_dopo_insp(nJob, nCam);

	display_notch_dopo_result(nJob, nCam, pViewer);

	if (m_pMain->m_strResultDate[nJob].GetLength() == 0)
	{
		SYSTEMTIME time;
		::GetLocalTime(&time);
		m_pMain->m_strResultDate[nJob].Format("%04d%02d%02d", time.wYear, time.wMonth, time.wDay);
	}
	if (m_pMain->m_strResultTime[nJob].GetLength() == 0)
	{
		SYSTEMTIME time;
		::GetLocalTime(&time);
		m_pMain->m_strResultTime[nJob].Format("%02d_%02d_%02d", time.wHour, time.wMinute, time.wSecond);
	}

	CString str_file_path;
	str_file_path = m_pMain->m_strResultDir + "Files\\" + m_pMain->m_strResultDate[nJob];
	if (_access(str_file_path, 0) != 0)	CreateDirectory(str_file_path, NULL);
	str_file_path.Format("%sFiles\\%s\\%s", m_pMain->m_strResultDir, m_pMain->m_strResultDate[nJob], m_pMain->vt_job_info[nJob].get_job_name());
	if (_access(str_file_path, 0) != 0)	CreateDirectory(str_file_path, NULL);
	str_file_path.Format("%sFiles\\%s\\%s\\insp_result_%s.csv", m_pMain->m_strResultDir, m_pMain->m_strResultDate[nJob], m_pMain->vt_job_info[nJob].get_job_name(), m_pMain->m_strResultDate[nJob]);

	write_notch_dopo_insp_result(nJob, nCam, str_file_path);

	filter.release();
	binary.release();

	pViewer->OnLoadImageFromPtr(viewImg);
	pViewer->Invalidate();

	return is_ok;
}
int CFormMainView::calculate_notch_dopo_data(int nJob, int nCam)
{
	NOTCH_WETOUT_DATA* pData = &m_pMain->notch_wetout_data;

	double res_x = m_pMain->GetMachine(nJob).getCameraResolutionX(nCam, 0);
	double res_y = m_pMain->GetMachine(nJob).getCameraResolutionY(nCam, 0);
	double dx, dy, dist;

	int count = 0;
	double total_up = 0.0;
	double total_down = 0.0;
	double total_width = 0.0;
	double total_dist = 0.0;

	pData->dist_up_min = 99999999999.0;
	pData->dist_down_min = 99999999999.0;
	pData->dist_width_min = 99999999999.0;
	pData->dist_up_max = 0.0;
	pData->dist_down_max = 0.0;
	pData->dist_width_max = 0.0;

	for (int i = 0; i < pData->pt_up.size(); i++)
	{
		dx = (pData->pt_up[i].x - pData->pt_center[i].x) * res_x;
		dy = (pData->pt_up[i].y - pData->pt_center[i].y) * res_y;
		dist = sqrt(dx * dx + dy * dy);
		pData->dist_up[i] = dist;
		total_up += dist;

		if (pData->dist_up_min > dist)
		{
			pData->dist_up_min = dist;
			pData->pt_up_min.x = pData->pt_up[i].x;
			pData->pt_up_min.y = pData->pt_up[i].y;
		}

		if (pData->dist_up_max < dist)
		{
			pData->dist_up_max = dist;
			pData->pt_up_max.x = pData->pt_up[i].x;
			pData->pt_up_max.y = pData->pt_up[i].y;
		}


		dx = (pData->pt_down[i].x - pData->pt_metal[i].x) * res_x;
		dy = (pData->pt_down[i].y - pData->pt_metal[i].y) * res_y;
		dist = sqrt(dx * dx + dy * dy);
		pData->dist_down[i] = dist;
		total_down += dist;

		if (pData->dist_down_min > dist)
		{
			pData->dist_down_min = dist;
			pData->pt_down_min.x = pData->pt_down[i].x;
			pData->pt_down_min.y = pData->pt_down[i].y;
		}

		if (pData->dist_down_max < dist)
		{
			pData->dist_down_max = dist;
			pData->pt_down_max.x = pData->pt_down[i].x;
			pData->pt_down_max.y = pData->pt_down[i].y;
		}


		pData->width_up_down[i] = pData->dist_up[i] + pData->dist_down[i];
		dist = pData->dist_up[i] + pData->dist_down[i];
		total_width += dist;

		if (pData->dist_width_min > dist)
		{
			pData->dist_width_min = dist;
			pData->pt_width_min.x = pData->pt_center[i].x;
			pData->pt_width_min.y = pData->pt_center[i].y;
		}

		if (pData->dist_width_max < dist)
		{
			pData->dist_width_max = dist;
			pData->pt_width_max.x = pData->pt_center[i].x;
			pData->pt_width_max.y = pData->pt_center[i].y;
		}

		count++;
	}

	if (count > 0)
	{
		pData->dist_up_avg = total_up / (double)count;
		pData->dist_down_avg = total_down / (double)count;
		pData->dist_width_avg = total_width / (double)count;
	}

	double robot_x, robot_y;
	m_pMain->GetMachine(nJob).PixelToWorld(nCam, 0, pData->pt_left.x, pData->pt_left.y, &robot_x, &robot_y);
	pData->dist_left = abs(robot_x);
	m_pMain->GetMachine(nJob).PixelToWorld(nCam, 0, pData->pt_right.x, pData->pt_right.y, &robot_x, &robot_y);
	pData->dist_right = abs(robot_x);


	return 0;
}
int CFormMainView::find_notch_dopo_start_end(BYTE* pImage, int nJob, int nCam, int w, int h)
{
	int left_x = 9999;
	int left_y = 0;
	int right_x = 0;
	int right_y = 0;

	int search_off = 100;
	int ignore = 9;
	int size = int(m_pMain->m_vtTraceData.size() - ignore);
	int left_end_pos_x = m_pMain->m_vtTraceData[0].x;
	int left_end_pos_y = m_pMain->m_vtTraceData[0].y;
	int right_end_pos_x = m_pMain->m_vtTraceData[size - 1].x;
	int right_end_pos_y = m_pMain->m_vtTraceData[size - 1].y;

	for (int y = left_end_pos_y - search_off; y < left_end_pos_y + search_off; y++)
	{
		if (y < 0 || y > h - 1) continue;
		for (int x = left_end_pos_x + search_off / 2; x > left_end_pos_x - search_off; x--)
		{
			if (x < 0 || x > w - 1) continue;

			if (pImage[y * w + x] == 255)
			{
				if (x < left_x)
				{
					left_x = x;
					left_y = y;
				}
			}
		}
	}

	for (int y = right_end_pos_y - search_off; y < right_end_pos_y + search_off; y++)
	{
		if (y < 0 || y > h - 1) continue;
		for (int x = right_end_pos_x - search_off / 2; x < right_end_pos_x + search_off; x++)
		{
			if (x < 0 || x > w - 1) continue;

			if (pImage[y * w + x] == 255)
			{
				if (x > right_x)
				{
					right_x = x;
					right_y = y;
				}
			}
		}
	}



	m_pMain->notch_wetout_data.pt_left.x = left_x;
	m_pMain->notch_wetout_data.pt_left.y = left_y;
	m_pMain->notch_wetout_data.pt_right.x = right_x;
	m_pMain->notch_wetout_data.pt_right.y = right_y;

	return 0;
}
bool CFormMainView::find_notch_metal_edge(BYTE* pImage, int nJob, int nCam, int w, int h)
{
	double dx, dy, a;
	struct _st32fPoint pt;
	int search_range_t = 0.4 / m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);
	int search_range_b = 0.13 / m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);

	CCaliper* pCaliper = &m_pMain->m_pDlgCaliper->m_Caliper[0][0][3];

	const int num_of_caliper = 1;

	int ignore = 9;
	int size = int(m_pMain->m_vtTraceData.size() - ignore);

	BOOL is_regist_metal_gold = FALSE;
	if (m_pMain->m_vtTraceData_MetalGold.size() > 0)	is_regist_metal_gold = TRUE;




	m_pMain->notch_wetout_data.pt_metal.clear();
	m_pMain->notch_wetout_data.pt_metal.assign(size * num_of_caliper, cv::Point2f(0, 0));



	pCaliper->setProjectionLength(3);
	pCaliper->setHalfFilterSize(1);
	pCaliper->setSearchType(SEARCH_TYPE_PEAK);
	pCaliper->setEachCaliperSearchType(1);	// median
	pCaliper->setThreshold(10);
	pCaliper->setNumOfCalipers(num_of_caliper);
	pCaliper->setNumOfIgnore(0);
	pCaliper->setFittingMethod(1);		// pca
	pCaliper->setContrastDirection(1);	// light to dark


	cv::Point2f pt_start, pt_end;

	double search_off = 2;
	for (int i = 0; i < size; i++)
	{
		// y = ax + b
		if (i == size - 1)
		{
			pt_start = m_pMain->m_vtTraceData[size - 2];
			pt_end = m_pMain->m_vtTraceData[size - 1];

			double dx = pt_end.x - pt_start.x;
			double dy = pt_end.y - pt_start.y;
			pt_start = m_pMain->m_vtTraceData[size - 1];
			pt_end.x = pt_start.x + dx;
			pt_end.y = pt_start.y + dy;
		}
		else
		{
			pt_start = m_pMain->m_vtTraceData[i];
			pt_end = m_pMain->m_vtTraceData[i + 1];
		}

		dx = (pt_end.x - pt_start.x);
		dy = (pt_end.y - pt_start.y);
		if (dx == 0.0) dx = 0.000000001;
		a = dy / dx;



		double radian = atan(a) - 3.141592 / 2.0;
		double src_x = pt_start.x;
		double src_y = pt_start.y;
		double dst_x = pt_start.x + search_range_t / 2.0 + search_off;
		double dst_y = pt_start.y;

		double tmpx, tmpy;
		tmpx = dst_x - src_x;
		tmpy = dst_y - src_y;

		double final_x = (tmpx * cos(radian) - tmpy * sin(radian) + src_x);
		double final_y = (tmpx * sin(radian) + tmpy * cos(radian) + src_y);

		double dx2 = final_x - src_x;
		double dy2 = final_y - src_y;

		radian = atan(a) + 3.141592 / 2.0;

		src_x = pt_start.x;
		src_y = pt_start.y;
		dst_x = pt_start.x + search_range_b / 2.0 + search_off;
		dst_y = pt_start.y;

		tmpx, tmpy;
		tmpx = dst_x - src_x;
		tmpy = dst_y - src_y;

		final_x = (tmpx * cos(radian) - tmpy * sin(radian) + src_x);
		final_y = (tmpx * sin(radian) + tmpy * cos(radian) + src_y);

		dx2 = final_x - src_x;
		dy2 = final_y - src_y;

		pt.x = final_x;
		pt.y = final_y;
		pCaliper->setOrgStartPt(pt);
		pCaliper->setStartPt(pt);

		pt.x = pt_end.x + dx2;
		pt.y = pt_end.y + dy2;
		pCaliper->setOrgEndPt(pt);
		pCaliper->setEndPt(pt);


		pCaliper->setSearchLength(search_range_b);
		pCaliper->setDirectionSwap(TRUE);
		pCaliper->processCaliper(pImage, w, h, 0.0, 0.0, 0.0);

		for (int x = 0; x < num_of_caliper; x++)
		{
			if (pCaliper->get_find_info(x).find)
			{
				m_pMain->notch_wetout_data.pt_metal[i * num_of_caliper + x].x = pCaliper->get_line_x(x, 0);
				m_pMain->notch_wetout_data.pt_metal[i * num_of_caliper + x].y = pCaliper->get_line_y(x, 0);
			}
			else
			{
				m_pMain->notch_wetout_data.pt_metal[i * num_of_caliper + x].x = 0.0;
				m_pMain->notch_wetout_data.pt_metal[i * num_of_caliper + x].y = 0.0;
			}
		}
	}

	int count = 0;
	sPoint pts[100];
	vector<int> vtFindIndex;

	int interest_count = 55;

	// 시작부터 55개까지가 직선 구간임..

	vtFindIndex.assign(100, 0);
	for (int i = 0; i < interest_count; i++)
	{
		if (m_pMain->notch_wetout_data.pt_metal[i].x == 0 || m_pMain->notch_wetout_data.pt_metal[i].y == 0) continue;
		vtFindIndex[i] = i;

		pts[count].x = m_pMain->notch_wetout_data.pt_metal[i].x;
		pts[count].y = m_pMain->notch_wetout_data.pt_metal[i].y;
		count++;
	}


	int num_of_ignore = interest_count / 4;
	if (count < interest_count / 2) return false;

	sLine line_info;
	vector<double> vtDistance;
	vector<int> vtIgnore;

	vtIgnore.assign(100, 0);

	double temp;

	m_pMain->m_pDlgCaliper->m_Caliper[0][0][0].compute_model_parameter(pts, count, line_info);


	for (int i = 0; i < count; i++)
		vtDistance.push_back(m_pMain->m_pDlgCaliper->m_Caliper[0][0][0].compute_distance(line_info, pts[i]));

	for (int i = 0; i < count - 1; i++)
	{
		for (int j = i + 1; j < count; j++)
		{
			if (vtDistance[i] < vtDistance[j])
			{
				temp = vtDistance[i];
				vtDistance[i] = vtDistance[j];
				vtDistance[j] = temp;

				temp = vtFindIndex[i];
				vtFindIndex[i] = vtFindIndex[j];
				vtFindIndex[j] = temp;
			}
		}
	}

	for (int i = 0; i < num_of_ignore; i++)
	{
		if (i >= count) continue;
		vtIgnore[vtFindIndex[i]] = TRUE;
	}


	count = 0;
	for (int i = 0; i < interest_count; i++)
	{
		if (vtIgnore[i]) continue;
		if (m_pMain->notch_wetout_data.pt_metal[i].x == 0 || m_pMain->notch_wetout_data.pt_metal[i].y == 0) continue;

		pts[count].x = m_pMain->notch_wetout_data.pt_metal[i].x;
		pts[count].y = m_pMain->notch_wetout_data.pt_metal[i].y;
		count++;
	}

	if (count < 3) return false;
	m_pMain->m_pDlgCaliper->m_Caliper[0][0][0].compute_model_parameter(pts, count, line_info);

	for (int i = 0; i < interest_count; i++)
	{
		if (m_pMain->notch_wetout_data.pt_metal[i].x == 0 || m_pMain->notch_wetout_data.pt_metal[i].y == 0) continue;
		m_pMain->notch_wetout_data.pt_metal[i].y = line_info.a * m_pMain->notch_wetout_data.pt_metal[i].x + line_info.b;
	}


	if (is_regist_metal_gold && m_pMain->notch_wetout_data.pt_metal.size() && m_pMain->m_vtTraceData_MetalGold.size())
	{
		double dx = 0.0, dy = 0.0, avg_dx = 0.0, avg_dy = 0.0;
		int count_x = 0;
		int count_y = 0;

		// 골드 데이터 대비 전체 시프트량 계산
		for (int i = 0; i < m_pMain->m_vtTraceData_MetalGold.size(); i++)
		{
			if (m_pMain->notch_wetout_data.pt_metal[i].x == 0 || m_pMain->notch_wetout_data.pt_metal[i].y == 0) continue;

			// 가로 직선 구간에서 x 쉬프트량은 파악이 되지 않음..
			// 전체 쉬프트량 계산에 역효과가 발생..
			// y는 전체 다 봐도 됨.
			if (i > interest_count)	// 곡선 구간만 계산하자.
			{
				dx += (m_pMain->notch_wetout_data.pt_metal[i].x - m_pMain->m_vtTraceData_MetalGold[i].x);
				count_x++;
			}

			dy += (m_pMain->notch_wetout_data.pt_metal[i].y - m_pMain->m_vtTraceData_MetalGold[i].y);
			count_y++;
		}

		if (count_x > 0)	avg_dx = dx / count_x;
		if (count_y > 0)	avg_dy = dy / count_y;

		for (int i = 0; i < m_pMain->m_vtTraceData_MetalGold.size(); i++)
		{
			// 못 찾은 곳은 골드데이터롤 채우자..
			if (m_pMain->notch_wetout_data.pt_metal[i].x == 0 || m_pMain->notch_wetout_data.pt_metal[i].y == 0)
			{
				m_pMain->notch_wetout_data.pt_metal[i].x = m_pMain->m_vtTraceData_MetalGold[i].x + avg_dx;
				m_pMain->notch_wetout_data.pt_metal[i].y = m_pMain->m_vtTraceData_MetalGold[i].y + avg_dy;
			}
			else
			{
				dx = m_pMain->m_vtTraceData_MetalGold[i].x - m_pMain->notch_wetout_data.pt_metal[i].x;
				dy = m_pMain->m_vtTraceData_MetalGold[i].y - m_pMain->notch_wetout_data.pt_metal[i].y;

				// 일정 이상 벗어나면 그냥 gold 데이터 + 쉬프트량 사용
				if (abs(dx) > abs(avg_dx * 1.2))
				{
					m_pMain->notch_wetout_data.pt_metal[i].x = m_pMain->m_vtTraceData_MetalGold[i].x + avg_dx;
					m_pMain->notch_wetout_data.pt_metal[i].y = m_pMain->m_vtTraceData_MetalGold[i].y + avg_dy;
				}
			}
		}
	}


	m_pMain->m_pDlgCaliper->m_pCaliperParam->updateUIFromData();
	m_pMain->m_pDlgCaliper->m_pCaliperParam->updateCoordinates();

	return true;
}
int CFormMainView::find_notch_dopo_edge(BYTE* pImage, int nJob, int nCam, int w, int h)
{
	double dx, dy, a;
	struct _st32fPoint pt;
	int search_range_t = 0.4 / m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);
	int search_range_b = 0.25 / m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);
	double up_spec = 0.35 / m_pMain->GetMachine(nJob).getCameraResolutionX(0, 0);

	CCaliper* pCaliper = &m_pMain->m_pDlgCaliper->m_Caliper[0][0][3];

	const int num_of_caliper = 1;

	int ignore = 9;
	int size = int(m_pMain->m_vtTraceData.size() - ignore);

	m_pMain->notch_wetout_data.pt_up.clear();
	m_pMain->notch_wetout_data.pt_up_limit.clear();
	m_pMain->notch_wetout_data.pt_down.clear();
	m_pMain->notch_wetout_data.pt_center.clear();

	m_pMain->notch_wetout_data.judge_up.clear();
	m_pMain->notch_wetout_data.judge_down.clear();

	m_pMain->notch_wetout_data.dist_up.clear();
	m_pMain->notch_wetout_data.dist_down.clear();
	m_pMain->notch_wetout_data.width_up_down.clear();


	m_pMain->notch_wetout_data.pt_up.assign(size * num_of_caliper, cv::Point2f(0, 0));
	m_pMain->notch_wetout_data.pt_up_limit.assign(size * num_of_caliper, cv::Point2f(0, 0));
	m_pMain->notch_wetout_data.pt_down.assign(size * num_of_caliper, cv::Point2f(0, 0));
	m_pMain->notch_wetout_data.pt_center.assign(size * num_of_caliper, cv::Point2f(0, 0));

	m_pMain->notch_wetout_data.judge_up.assign(size * num_of_caliper, true);
	m_pMain->notch_wetout_data.judge_down.assign(size * num_of_caliper, true);


	m_pMain->notch_wetout_data.dist_up.assign(size * num_of_caliper, 0);
	m_pMain->notch_wetout_data.dist_down.assign(size * num_of_caliper, 0);
	m_pMain->notch_wetout_data.width_up_down.assign(size * num_of_caliper, 0);

	pCaliper->setProjectionLength(3);
	pCaliper->setHalfFilterSize(1);
	pCaliper->setSearchType(SEARCH_TYPE_END);
	pCaliper->setEachCaliperSearchType(1);	// median
	pCaliper->setThreshold(20);
	pCaliper->setNumOfCalipers(num_of_caliper);
	pCaliper->setNumOfIgnore(0);
	pCaliper->setFittingMethod(1);		// pca
	pCaliper->setContrastDirection(1);	// light to dark


	cv::Point2f pt_start, pt_end;

	double search_off = 10;
	for (int i = 0; i < size; i++)
	{
		// y = ax + b
		if (i == size - 1)
		{
			pt_start = m_pMain->m_vtTraceData[size - 2];
			pt_end = m_pMain->m_vtTraceData[size - 1];

			double dx = pt_end.x - pt_start.x;
			double dy = pt_end.y - pt_start.y;
			pt_start = m_pMain->m_vtTraceData[size - 1];
			pt_end.x = pt_start.x + dx;
			pt_end.y = pt_start.y + dy;
		}
		else
		{
			pt_start = m_pMain->m_vtTraceData[i];
			pt_end = m_pMain->m_vtTraceData[i + 1];
		}

		dx = (pt_end.x - pt_start.x);
		dy = (pt_end.y - pt_start.y);
		if (dx == 0.0) dx = 0.000000001;
		a = dy / dx;



		double radian = atan(a) - 3.141592 / 2.0;
		double src_x = pt_start.x;
		double src_y = pt_start.y;
		double dst_x = pt_start.x + search_range_t / 2.0 + search_off;
		double dst_y = pt_start.y;

		double spec_x = pt_start.x + up_spec;
		double spec_y = pt_start.y;

		double tmpx, tmpy;
		tmpx = dst_x - src_x;
		tmpy = dst_y - src_y;

		double final_x = (tmpx * cos(radian) - tmpy * sin(radian) + src_x);
		double final_y = (tmpx * sin(radian) + tmpy * cos(radian) + src_y);

		double final_lmt_x = ((spec_x - src_x) * cos(radian) - (spec_y - src_y) * sin(radian) + src_x);
		double final_lmt_y = ((spec_x - src_x) * sin(radian) + (spec_y - src_y) * cos(radian) + src_y);

		m_pMain->notch_wetout_data.pt_up_limit[i].x = final_lmt_x;
		m_pMain->notch_wetout_data.pt_up_limit[i].y = final_lmt_y;

		double dx2 = final_x - src_x;
		double dy2 = final_y - src_y;

		pt.x = final_x;
		pt.y = final_y;
		pCaliper->setOrgStartPt(pt);
		pCaliper->setStartPt(pt);

		pt.x = pt_end.x + dx2;
		pt.y = pt_end.y + dy2;
		pCaliper->setOrgEndPt(pt);
		pCaliper->setEndPt(pt);


		pCaliper->setSearchLength(search_range_t);
		pCaliper->setDirectionSwap(FALSE);
		pCaliper->calcCaliperRect();
		pCaliper->processCaliper(pImage, w, h, 0.0, 0.0, 0.0);

		for (int x = 0; x < num_of_caliper; x++)
		{
			m_pMain->notch_wetout_data.pt_center[i * num_of_caliper + x].x = pt_start.x;
			m_pMain->notch_wetout_data.pt_center[i * num_of_caliper + x].y = pt_start.y;

			if (pCaliper->get_find_info(x).find)
			{
				m_pMain->notch_wetout_data.pt_up[i * num_of_caliper + x].x = pCaliper->get_line_x(x, 0);
				m_pMain->notch_wetout_data.pt_up[i * num_of_caliper + x].y = pCaliper->get_line_y(x, 0);
			}
			else
			{
				m_pMain->notch_wetout_data.pt_up[i * num_of_caliper + x].x = 0.0;
				m_pMain->notch_wetout_data.pt_up[i * num_of_caliper + x].y = 0.0;
			}
		}

		radian = atan(a) + 3.141592 / 2.0;

		src_x = pt_start.x;
		src_y = pt_start.y;
		dst_x = pt_start.x + search_range_b / 2.0 + search_off;
		dst_y = pt_start.y;

		tmpx, tmpy;
		tmpx = dst_x - src_x;
		tmpy = dst_y - src_y;

		final_x = (tmpx * cos(radian) - tmpy * sin(radian) + src_x);
		final_y = (tmpx * sin(radian) + tmpy * cos(radian) + src_y);

		dx2 = final_x - src_x;
		dy2 = final_y - src_y;

		pt.x = final_x;
		pt.y = final_y;
		pCaliper->setOrgStartPt(pt);
		pCaliper->setStartPt(pt);

		pt.x = pt_end.x + dx2;
		pt.y = pt_end.y + dy2;
		pCaliper->setOrgEndPt(pt);
		pCaliper->setEndPt(pt);


		pCaliper->setSearchLength(search_range_b);
		pCaliper->setDirectionSwap(TRUE);
		pCaliper->processCaliper(pImage, w, h, 0.0, 0.0, 0.0);

		for (int x = 0; x < num_of_caliper; x++)
		{
			if (pCaliper->get_find_info(x).find)
			{
				m_pMain->notch_wetout_data.pt_down[i * num_of_caliper + x].x = pCaliper->get_line_x(x, 0);
				m_pMain->notch_wetout_data.pt_down[i * num_of_caliper + x].y = pCaliper->get_line_y(x, 0);
			}
			else
			{
				m_pMain->notch_wetout_data.pt_down[i * num_of_caliper + x].x = 0.0;
				m_pMain->notch_wetout_data.pt_down[i * num_of_caliper + x].y = 0.0;
			}
		}
	}

	m_pMain->m_pDlgCaliper->m_pCaliperParam->updateUIFromData();
	m_pMain->m_pDlgCaliper->m_pCaliperParam->updateCoordinates();

	return 0;
}
void CFormMainView::write_notch_dopo_insp_result(int nJob, int nCam, CString str_path)
{
	char cData[MAX_PATH] = { 0, };

	BOOL write_header = FALSE;
	if (_access(str_path, 0) != 0)	write_header = TRUE;


	FILE* fp = fopen(str_path, "a");
	if (fp == NULL) return;

	CString str_body, str_header, str_temp;


	str_header = "PANEL_ID";
	str_temp.Format("%s", m_pMain->vt_job_info[nJob].get_main_object_id());
	str_body += str_temp;

	str_header += ",TIME";
	str_temp.Format(",%s", m_pMain->m_strResultTime[nJob]);
	str_body += str_temp;

	str_header += ",JUDGE";
	str_temp.Format(",%s", m_pMain->notch_wetout_data.judge ? "OK" : "NG");
	str_body += str_temp;

	str_header += ",DOPO_LEFT_DIST";
	str_temp.Format(",%.3f", m_pMain->notch_wetout_data.dist_left);
	str_body += str_temp;

	str_header += ",DOPO_RIGHT_DIST";
	str_temp.Format(",%.3f", m_pMain->notch_wetout_data.dist_right);
	str_body += str_temp;

	str_header += ",DOPO_WIDTH_MIN";
	str_temp.Format(",%.3f", m_pMain->notch_wetout_data.dist_width_min);
	str_body += str_temp;

	str_header += ",DOPO_WIDTH_MAX";
	str_temp.Format(",%.3f", m_pMain->notch_wetout_data.dist_width_max);
	str_body += str_temp;

	str_header += ",DOPO_WIDTH_AVG";
	str_temp.Format(",%.3f", m_pMain->notch_wetout_data.dist_width_avg);
	str_body += str_temp;

	str_header += ",DOPO_UP_MIN";
	str_temp.Format(",%.3f", m_pMain->notch_wetout_data.dist_up_min);
	str_body += str_temp;

	str_header += ",DOPO_UP_MAX";
	str_temp.Format(",%.3f", m_pMain->notch_wetout_data.dist_up_max);
	str_body += str_temp;

	str_header += ",DOPO_UP_AVG";
	str_temp.Format(",%.3f", m_pMain->notch_wetout_data.dist_up_avg);
	str_body += str_temp;

	str_header += ",DOPO_DOWN_MIN";
	str_temp.Format(",%.3f", m_pMain->notch_wetout_data.dist_down_min);
	str_body += str_temp;

	str_header += ",DOPO_DOWN_MAX";
	str_temp.Format(",%.3f", m_pMain->notch_wetout_data.dist_down_max);
	str_body += str_temp;

	str_header += ",DOPO_DOWN_AVG";
	str_temp.Format(",%.3f", m_pMain->notch_wetout_data.dist_down_avg);
	str_body += str_temp;


	double interval = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspTraceInterval();

	for (int i = 0; i < m_pMain->notch_wetout_data.dist_up.size(); i++)
	{
		str_temp.Format(",UP_%.3f", i * interval);
		str_header += str_temp;

		str_temp.Format(",%.3f", m_pMain->notch_wetout_data.dist_up[i]);
		str_body += str_temp;
	}

	for (int i = 0; i < m_pMain->notch_wetout_data.dist_down.size(); i++)
	{
		str_temp.Format(",DOWN_%.3f", i * interval);
		str_header += str_temp;

		str_temp.Format(",%.3f", m_pMain->notch_wetout_data.dist_down[i]);
		str_body += str_temp;
	}

	for (int i = 0; i < m_pMain->notch_wetout_data.dist_up.size(); i++)
	{
		str_temp.Format(",WIDTH_%.3f", i * interval);
		str_header += str_temp;

		str_temp.Format(",%.3f", m_pMain->notch_wetout_data.dist_down[i] + m_pMain->notch_wetout_data.dist_up[i]);
		str_body += str_temp;
	}

	str_header += "\n";
	str_body += "\n";

	if (write_header)		fwrite(str_header, str_header.GetLength(), 1, fp);

	fwrite(str_body, str_body.GetLength(), 1, fp);



	fclose(fp);

}
void CFormMainView::display_notch_metal(int nJob, int nCam, CViewerEx* pViewer)
{
	pViewer->ClearOverlayDC();

	auto pDC = pViewer->getOverlayDC();
	CPen pen_up(PS_SOLID, 2, COLOR_LIME);
	CPen pen_down(PS_SOLID, 2, COLOR_LIME);
	CPen pen_center(PS_SOLID, 2, COLOR_YELLOW);
	CPen pen_lr(PS_SOLID, 2, COLOR_BLUE);
	CPen pen_ng(PS_SOLID, 2, COLOR_RED);

	cv::Point2f pt;
	int size = 2;


	pDC->SelectObject(&pen_down);

	pt = m_pMain->notch_wetout_data.pt_metal[0];

	pDC->MoveTo(pt.x, pt.y);

	for (int i = 1; i < m_pMain->notch_wetout_data.pt_metal.size(); i++)
	{
		pt = m_pMain->notch_wetout_data.pt_metal[i];
		pDC->LineTo(pt.x, pt.y);
	}

	size = 3;
	for (int i = 0; i < m_pMain->notch_wetout_data.pt_metal.size(); i++)
	{
		if (m_pMain->notch_wetout_data.judge_metal[i])	pDC->SelectObject(&pen_down);
		else											pDC->SelectObject(&pen_ng);

		pt = m_pMain->notch_wetout_data.pt_metal[i];
		pDC->Ellipse(pt.x - size, pt.y - size, pt.x + size, pt.y + size);
	}

	pDC->SelectObject(&pen_center);
	size = 3;
	for (int i = 0; i < m_pMain->m_vtTraceData.size(); i++)
	{
		pt = m_pMain->m_vtTraceData[i];
		pDC->Ellipse(pt.x - size, pt.y - size, pt.x + size, pt.y + size);
	}

	pViewer->Invalidate();
}
void CFormMainView::display_notch_dopo_result(int nJob, int nCam, CViewerEx* pViewer)
{
	pViewer->ClearOverlayDC();

	auto pDC = pViewer->getOverlayDC();
	CPen pen_up(PS_SOLID, 2, COLOR_LIME);
	CPen pen_down(PS_SOLID, 2, COLOR_LIME);
	CPen pen_center(PS_SOLID, 2, COLOR_YELLOW);
	CPen pen_metal(PS_SOLID, 1, COLOR_DDARK_GRAY);
	CPen pen_lr(PS_SOLID, 2, COLOR_BLUE);
	CPen pen_ng(PS_SOLID, 2, COLOR_RED);
	CPen pen_limit(PS_DOT, 1, COLOR_RED);

	cv::Point2f pt;
	int size = 2;

	CString str_data;

	pDC->SetTextColor(COLOR_DARK_LIME);

	for (int i = 0; i < m_pMain->notch_wetout_data.pt_up.size(); i++)
	{
		pt = m_pMain->notch_wetout_data.pt_up[i];

		if (m_pMain->notch_wetout_data.judge_up[i])		pDC->SelectObject(&pen_up);
		else											pDC->SelectObject(&pen_ng);

		pDC->MoveTo(pt.x - 5, pt.y);
		pDC->LineTo(pt.x + 5, pt.y);
		pDC->MoveTo(pt.x, pt.y - 5);
		pDC->LineTo(pt.x, pt.y + 5);

		if (i % 2 == 0)
		{
			str_data.Format("%.3f", m_pMain->notch_wetout_data.dist_up[i]);
			pDC->TextOutA(pt.x, pt.y - 30, str_data);
		}
	}

	pDC->SelectObject(&pen_down);
	for (int i = 0; i < m_pMain->notch_wetout_data.pt_down.size(); i++)
	{
		if (m_pMain->notch_wetout_data.judge_down[i])	pDC->SelectObject(&pen_down);
		else											pDC->SelectObject(&pen_ng);

		pt = m_pMain->notch_wetout_data.pt_down[i];
		pDC->MoveTo(pt.x - 5, pt.y);
		pDC->LineTo(pt.x + 5, pt.y);
		pDC->MoveTo(pt.x, pt.y - 5);
		pDC->LineTo(pt.x, pt.y + 5);

		if (i % 2 == 0)
		{
			str_data.Format("%.3f", m_pMain->notch_wetout_data.dist_down[i]);
			pDC->TextOutA(pt.x, pt.y + 10, str_data);
		}
	}

	pDC->SelectObject(&pen_limit);
	pt = m_pMain->notch_wetout_data.pt_up_limit[0];

	pDC->MoveTo(pt.x, pt.y);

	for (int i = 1; i < m_pMain->notch_wetout_data.pt_up_limit.size(); i++)
	{
		pt = m_pMain->notch_wetout_data.pt_up_limit[i];
		pDC->LineTo(pt.x, pt.y);
	}

	pDC->SelectObject(&pen_center);

	pt = m_pMain->notch_wetout_data.pt_center[0];

	pDC->MoveTo(pt.x, pt.y);

	for (int i = 1; i < m_pMain->notch_wetout_data.pt_center.size(); i++)
	{
		pt = m_pMain->notch_wetout_data.pt_center[i];
		pDC->LineTo(pt.x, pt.y);
	}

	size = 3;
	for (int i = 0; i < m_pMain->notch_wetout_data.pt_center.size(); i++)
	{
		pt = m_pMain->notch_wetout_data.pt_center[i];
		pDC->Ellipse(pt.x - size, pt.y - size, pt.x + size, pt.y + size);
	}

	pDC->SelectObject(&pen_metal);
	pt = m_pMain->notch_wetout_data.pt_metal[0];

	pDC->MoveTo(pt.x, pt.y);

	for (int i = 1; i < m_pMain->notch_wetout_data.pt_metal.size(); i++)
	{
		pt = m_pMain->notch_wetout_data.pt_metal[i];
		pDC->LineTo(pt.x, pt.y);
	}

	size = 3;
	for (int i = 0; i < m_pMain->notch_wetout_data.pt_metal.size(); i++)
	{
		pt = m_pMain->notch_wetout_data.pt_metal[i];
		pDC->Ellipse(pt.x - size, pt.y - size, pt.x + size, pt.y + size);
	}

	pDC->SelectObject(&pen_lr);

	pt = m_pMain->notch_wetout_data.pt_left;
	pDC->MoveTo(pt.x - 10, pt.y);
	pDC->LineTo(pt.x + 10, pt.y);
	pDC->MoveTo(pt.x, pt.y - 10);
	pDC->LineTo(pt.x, pt.y + 10);

	size = 5;
	pt = m_pMain->notch_wetout_data.pt_up_min;
	pDC->Ellipse(pt.x - size, pt.y - size, pt.x + size, pt.y + size);
	pt = m_pMain->notch_wetout_data.pt_up_max;
	pDC->Ellipse(pt.x - size, pt.y - size, pt.x + size, pt.y + size);
	pt = m_pMain->notch_wetout_data.pt_down_min;
	pDC->Ellipse(pt.x - size, pt.y - size, pt.x + size, pt.y + size);
	pt = m_pMain->notch_wetout_data.pt_down_max;
	pDC->Ellipse(pt.x - size, pt.y - size, pt.x + size, pt.y + size);

	pt = m_pMain->notch_wetout_data.pt_width_min;
	pDC->Ellipse(pt.x - size, pt.y - size, pt.x + size, pt.y + size);
	pt = m_pMain->notch_wetout_data.pt_width_max;
	pDC->Ellipse(pt.x - size, pt.y - size, pt.x + size, pt.y + size);

	int y_off = 100;
	CString str_temp;

	//struct stFigureText figure;
	//figure.nOrgSize = 3;
	//figure.nFitSize = 5;

	//figure.ptBegin.x = pt.x;
	//figure.ptBegin.y = pt.y + 50;
	//figure.ptBeginFit = figure.ptBegin;
	//str_temp.Format("L: %.3f", m_pMain->notch_wetout_data.dist_left);
	//pViewer->addFigureText(figure, 20, 20, COLOR_BLUE);
	pViewer->AddSoftGraphic(new GraphicLabel(pt.x, pt.y + 50, fmt("L: %.3f", m_pMain->notch_wetout_data.dist_left), COLOR_BLUE | 0xff000000));

	pt = m_pMain->notch_wetout_data.pt_right;

	pDC->MoveTo(pt.x - 10, pt.y);
	pDC->LineTo(pt.x + 10, pt.y);
	pDC->MoveTo(pt.x, pt.y - 10);
	pDC->LineTo(pt.x, pt.y + 10);

	//figure.ptBegin.x = pt.x;
	//figure.ptBegin.y = pt.y + 50;
	//figure.ptBeginFit = figure.ptBegin;
	//str_temp.Format("R: %.3f", m_pMain->notch_wetout_data.dist_right);
	//pViewer->addFigureText(figure, 20, 20, COLOR_BLUE);
	pViewer->AddSoftGraphic(new GraphicLabel(pt.x, pt.y + 50, fmt("R: %.3f", m_pMain->notch_wetout_data.dist_right), COLOR_BLUE | 0xff000000));

	//figure.ptBegin.x = 100;
	//figure.ptBegin.y = 100 + y_off * 0;
	//figure.ptBeginFit = figure.ptBegin;
	//str_temp.Format("L ↔ C Dist: %.3f", m_pMain->notch_wetout_data.dist_left);
	//pViewer->addFigureText(figure, 20, 20, COLOR_BLUE);

	pViewer->AddSoftGraphic(new GraphicLabel(100, 100 + y_off * 0, fmt("L ↔ C Dist: %.3f", m_pMain->notch_wetout_data.dist_left), COLOR_BLUE | 0xff000000));
	pViewer->AddSoftGraphic(new GraphicLabel(100, 100 + y_off * 1, fmt("R ↔ C Dist: %.3f", m_pMain->notch_wetout_data.dist_right), COLOR_BLUE | 0xff000000));
	pViewer->AddSoftGraphic(new GraphicLabel(100, 100 + y_off * 2, fmt("L ↔ R Dist: %.3f", m_pMain->notch_wetout_data.dist_left + m_pMain->notch_wetout_data.dist_right), COLOR_BLUE | 0xff000000));
	pViewer->AddSoftGraphic(new GraphicLabel(100, 100 + y_off * 3, fmt("Up min _ max: %.3f _ %.3f", m_pMain->notch_wetout_data.dist_up_min, m_pMain->notch_wetout_data.dist_up_max), COLOR_BLUE | 0xff000000));
	pViewer->AddSoftGraphic(new GraphicLabel(100, 100 + y_off * 4, fmt("Down min _ max: %.3f _ %.3f", m_pMain->notch_wetout_data.dist_down_min, m_pMain->notch_wetout_data.dist_down_max), COLOR_BLUE | 0xff000000));
	pViewer->AddSoftGraphic(new GraphicLabel(100, 100 + y_off * 5, fmt("Width min _ max: %.3f _ %.3f", m_pMain->notch_wetout_data.dist_width_min, m_pMain->notch_wetout_data.dist_width_max), COLOR_BLUE | 0xff000000));


	//figure.ptBegin.x = 100;
	//figure.ptBegin.y = 100 + y_off * 1;
	//figure.ptBeginFit = figure.ptBegin;
	//str_temp.Format("R ↔ C Dist: %.3f", m_pMain->notch_wetout_data.dist_right);
	//pViewer->addFigureText(figure, 20, 20, COLOR_BLUE);

	//figure.ptBegin.x = 100;
	//figure.ptBegin.y = 100 + y_off * 2;
	//figure.ptBeginFit = figure.ptBegin;
	//str_temp.Format("L ↔ R Dist: %.3f", m_pMain->notch_wetout_data.dist_left + m_pMain->notch_wetout_data.dist_right);
	//pViewer->addFigureText(figure, 20, 20, COLOR_BLUE);

	//figure.ptBegin.x = 100;
	//figure.ptBegin.y = 100 + y_off * 3;
	//figure.ptBeginFit = figure.ptBegin;
	//str_temp.Format("Up min _ max: %.3f _ %.3f", m_pMain->notch_wetout_data.dist_up_min, m_pMain->notch_wetout_data.dist_up_max);
	//pViewer->addFigureText(figure, 20, 20, COLOR_BLUE);

	//figure.ptBegin.x = 100;
	//figure.ptBegin.y = 100 + y_off * 4;
	//figure.ptBeginFit = figure.ptBegin;
	//str_temp.Format("Down min _ max: %.3f _ %.3f", m_pMain->notch_wetout_data.dist_down_min, m_pMain->notch_wetout_data.dist_down_max);
	//pViewer->addFigureText(figure, 20, 20, COLOR_BLUE);

	//figure.ptBegin.x = 100;
	//figure.ptBegin.y = 100 + y_off * 5;
	//figure.ptBeginFit = figure.ptBegin;
	//str_temp.Format("Width min _ max: %.3f _ %.3f", m_pMain->notch_wetout_data.dist_width_min, m_pMain->notch_wetout_data.dist_width_max);
	//pViewer->addFigureText(figure, 20, 20, COLOR_BLUE);

	pViewer->Invalidate();
}
void CFormMainView::trace_data_fitting(std::vector<cv::Point2f>& pts)
{
	BOOL find_bottom_line = FALSE;

	double base_y = pts[0].y;
	double dy;
	double thresh = 5;
	int index = 0;
	sPoint temp_pt;
	sPoint* points = new sPoint[pts.size()];
	cv::Point2f temp;

	int area_pos[6] = { 0, }; // 각 구간별 구분 인덱스

	m_pMain->m_vtTraceData_Fitting.clear();

	int dx_min_index = 0;

	// 총 6구간으로 나누자.
	//											(6)
	//                     -----------------------------------------
	//                   /
	//                  /	(5)
	//                 |   
	//                ＼
	//                  ＼    (4)
	//                    ＼
	//					   |
	//					   |  (3)
	//		               |
	//					  /
	//					 /   (2)
	//					/
	//	--------------
	//         (1)

	int i = 0;
	area_pos[0] = 0;
	// 2구간 - 시작 지점을 찾자.	시작 좌표 기준 y변화가 5픽셀 이상 되기 시작 하는 점
	{
		for (i = 0; pts.size(); i++)
		{
			dy = base_y - pts[i].y;

			temp_pt.x = pts[i].x;
			temp_pt.y = pts[i].y;
			points[i] = temp_pt;

			if (abs(dy) > thresh)
			{
				index = MAX(0, i);
				break;
			}
		}
		area_pos[1] = index;
	}

	// 6구간 - 시작 지점을 찾자.	끝 좌표 기준 y변화가 5픽셀 이상 되기 시작 하는 점
	{
		int count = 0;
		base_y = pts[pts.size() - 1].y;
		for (i = int(pts.size() - 1); i > 0; i--)
		{
			dy = base_y - pts[i].y;

			temp_pt.x = pts[i].x;
			temp_pt.y = pts[i].y;
			points[count++] = temp_pt;

			if (abs(dy) > thresh)
			{
				index = MAX(0, count - 2);
				break;
			}
		}

		area_pos[5] = int(pts.size() - index - 1);
	}

	// 3구간 - 시작 지점을 찾자. x변화가 20이내로 되는 지점
	{
		for (int i = 0; i < pts.size() - 1; i++)
		{
			double dx = abs(pts[i].x - pts[i + 1].x);

			if (dx < 20)
			{
				area_pos[2] = i;
				break;
			}
		}
	}

	// 5구간 - 시작 지점을 찾자. x변화가 20이내로 되는 지점
	{
		for (int i = int(pts.size() - 2); i > 0; i--)
		{
			double dx = abs(pts[i].x - pts[i + 1].x);

			if (dx < 25)
			{
				area_pos[4] = i;
				break;
			}
		}
	}

	// 4구간 - 시작 지점을 찾자. x변화가 가장 작은 지점(변곡점)
	{
		double dx_min = 999990.0;
		for (int i = 0; i < pts.size() - 1; i++)
		{
			double dx = abs(pts[i].x - pts[i + 1].x);

			if (dx < dx_min)
			{
				dx_min = dx;
				dx_min_index = i;
				area_pos[3] = i;
			}
		}
	}


	// 1구간 - line fitting
	{
		sLine line_info;
		sPoint points[500];
		int count = 0;
		for (int i = 0; i < area_pos[1]; i++)
		{
			points[count].x = pts[i].x;
			points[count].y = pts[i].y;
			count++;
		}

		if (count > 1)
		{
			cv::Point2f temp_pt;
			m_pMain->m_pDlgCaliper->m_Caliper[0][0][0].compute_model_parameter(points, count, line_info);

			// y = ax + b;
			for (int i = 0; i < area_pos[1]; i++)
			{
				temp_pt.x = pts[i].x;
				temp_pt.y = line_info.a * pts[i].x + line_info.b;
				m_pMain->m_vtTraceData_Fitting.push_back(temp_pt);
			}
		}
	}

	// 2구간 -  ellipse fitting
	if (0)
	{
		sPoint data[500] = { 0, };
		sEllipse ellipse;
		int count = 0;
		for (int i = area_pos[1]; i < area_pos[2]; i++)
		{
			data[count].x = pts[i].x;
			data[count].y = pts[i].y;
			count++;
		}


		m_pMain->m_pDlgCaliper->m_Caliper[0][0][0].compute_ellipse_model_parameter(data, count, m_pMain->notch_ellipse[0]);
		m_pMain->notch_ellipse[0].convert_std_form();
		ellipse = m_pMain->notch_ellipse[0];


		double ct = cos(ellipse.theta);
		double st = sin(ellipse.theta);

		for (int i = area_pos[1]; i < area_pos[2]; i++)
		{
			double temp_x = pts[i].x;
			double temp_y = pts[i].y;

			double min_dist = 999999;
			double min_t = 0.0;
			cv::Point2f temp_pt;
			for (double t = 0; t < 360; t += 0.2)
			{
				double x = ellipse.w * cos(t / 180.0 * 3.141592);	// 270
				double y = ellipse.h * sin(t / 180.0 * 3.141592);

				double rx = x * ct - y * st;
				double ry = x * st + y * ct;
				double px = ellipse.cx + rx;
				double py = ellipse.cy + ry;

				double dx = temp_x - px;
				double dy = temp_y - py;
				double dist = sqrt(dx * dx + dy * dy);

				if (min_dist > dist)
				{
					min_dist = dist;
					min_t = t;

					temp_pt.x = px;
					temp_pt.y = py;
				}
			}

			if (i == area_pos[1])
			{
				temp_pt.x = pts[i].x;
				temp_pt.y = pts[i].y;
			}
			m_pMain->m_vtTraceData_Fitting.push_back(temp_pt);
		}
	}
	else
	{
		cv::Point2f temp_pt;
		for (int i = area_pos[1]; i < area_pos[2]; i++)
		{
			temp_pt.x = pts[i].x;
			temp_pt.y = pts[i].y;
			m_pMain->m_vtTraceData_Fitting.push_back(temp_pt);
		}

	}

	// 3구간 - 2차 회귀 분석
	if (0)
	{
		vector<double> vtx;
		vector<double> vty;
		cv::Point2f temp_pt;

		int count = 0;
		for (int i = area_pos[2]; i < area_pos[3]; i++)
		{
			vtx.push_back(pts[i].x);
			vty.push_back(pts[i].y);
			count++;
		}

		// y = b2 * x^2 + b1 * x + b0
		double b0, b1, b2;
		get_2nd_order_regression(&vtx, &vty, &b0, &b1, &b2);


		// ax^2 + bx + c = 0
		// x = (-b += sqrt(b^2 - 4ac)) / 2a
		for (int i = area_pos[2]; i < area_pos[3]; i++)
		{
			double a = b2;
			double b = b1;
			double c = b0 - pts[i].y;

			double x1 = ((-b) + sqrt(b * b - 4 * a * c)) / (2 * a);
			double x2 = ((-b) - sqrt(b * b - 4 * a * c)) / (2 * a);


			if (abs(pts[i].x - x1) < abs(pts[i].x - x2))
			{
				temp_pt.x = x1;
			}
			else
			{
				temp_pt.x = x2;
			}
			temp_pt.y = pts[i].y;
			m_pMain->m_vtTraceData_Fitting.push_back(temp_pt);
		}
	}
	else
	{
		cv::Point2f temp_pt;
		for (int i = area_pos[2]; i < area_pos[3]; i++)
		{
			temp_pt.x = pts[i].x;
			temp_pt.y = pts[i].y;
			m_pMain->m_vtTraceData_Fitting.push_back(temp_pt);
		}
	}

	// 4구간 - 2차 회귀 분석
	if (0)
	{
		vector<double> vtx;
		vector<double> vty;
		cv::Point2f temp_pt;

		int count = 0;
		for (int i = area_pos[3]; i < area_pos[4]; i++)
		{
			vtx.push_back(pts[i].x);
			vty.push_back(pts[i].y);
			count++;
		}

		// y = b2 * x^2 + b1 * x + b0
		double b0, b1, b2;
		get_2nd_order_regression(&vtx, &vty, &b0, &b1, &b2);


		// ax^2 + bx + c = 0
		// x = (-b += sqrt(b^2 - 4ac)) / 2a
		for (int i = area_pos[3]; i < area_pos[4]; i++)
		{
			double a = b2;
			double b = b1;
			double c = b0 - pts[i].y;

			double x1 = ((-b) + sqrt(b * b - 4 * a * c)) / (2 * a);
			double x2 = ((-b) - sqrt(b * b - 4 * a * c)) / (2 * a);


			if (abs(pts[i].x - x1) < abs(pts[i].x - x2))
			{
				temp_pt.x = x1;
			}
			else
			{
				temp_pt.x = x2;
			}
			temp_pt.y = pts[i].y;
			m_pMain->m_vtTraceData_Fitting.push_back(temp_pt);
		}
	}
	else
	{
		cv::Point2f temp_pt;
		for (int i = area_pos[3]; i < area_pos[4]; i++)
		{
			temp_pt.x = pts[i].x;
			temp_pt.y = pts[i].y;
			m_pMain->m_vtTraceData_Fitting.push_back(temp_pt);
		}
	}

	// 5구간 -  ellipse fitting
	if (1)
	{
		sPoint data[1000] = { 0, };
		sEllipse ellipse;
		int count = 0;
		for (int i = area_pos[4]; i < area_pos[5]; i++)
		{
			data[count].x = pts[i].x;
			data[count].y = pts[i].y;
			count++;
		}

		m_pMain->m_pDlgCaliper->m_Caliper[0][0][0].compute_ellipse_model_parameter(data, count, m_pMain->notch_ellipse[1]);
		m_pMain->notch_ellipse[1].convert_std_form();

		ellipse = m_pMain->notch_ellipse[1];

		double ct = cos(ellipse.theta);
		double st = sin(ellipse.theta);

		for (int i = area_pos[4]; i < area_pos[5]; i++)
		{
			double temp_x = pts[i].x;
			double temp_y = pts[i].y;

			double min_dist = 999999;
			double min_t = 0.0;
			cv::Point2f temp_pt;
			for (double t = 0; t < 360; t += 0.2)
			{
				double x = ellipse.w * cos(t / 180.0 * 3.141592);	// 270
				double y = ellipse.h * sin(t / 180.0 * 3.141592);

				double rx = x * ct - y * st;
				double ry = x * st + y * ct;
				double px = ellipse.cx + rx;
				double py = ellipse.cy + ry;

				double dx = temp_x - px;
				double dy = temp_y - py;
				double dist = sqrt(dx * dx + dy * dy);

				if (min_dist > dist)
				{
					min_dist = dist;
					min_t = t;

					temp_pt.x = px;
					temp_pt.y = py;
				}
			}
			m_pMain->m_vtTraceData_Fitting.push_back(temp_pt);
		}
	}
	else
	{
		cv::Point2f temp_pt;
		for (int i = area_pos[4]; i < area_pos[5]; i++)
		{
			temp_pt.x = pts[i].x;
			temp_pt.y = pts[i].y;
			m_pMain->m_vtTraceData_Fitting.push_back(temp_pt);
		}
	}

	// 6구간 - line fitting
	{
		sLine line_info;
		sPoint points[500];
		int count = 0;
		for (int i = area_pos[5]; i < pts.size(); i++)
		{
			points[count].x = pts[i].x;
			points[count].y = pts[i].y;
			count++;
		}

		if (count > 1)
		{
			cv::Point2f temp_pt;
			//m_pMain->m_pDlgCaliper->m_Caliper[0][0][0].compute_model_parameter(points, count, line_info);
			line_info = m_pMain->m_pDlgCaliper->m_Caliper[0][0][1].m_lineInfo;

			// y = ax + b;
			for (int i = area_pos[5]; i < pts.size(); i++)
			{
				temp_pt.x = pts[i].x;
				temp_pt.y = line_info.a * pts[i].x + line_info.b;
				m_pMain->m_vtTraceData_Fitting.push_back(temp_pt);
			}
		}
		else
		{
			cv::Point2f temp_pt;
			for (int i = area_pos[5]; i < pts.size(); i++)
			{
				temp_pt.x = pts[i].x;
				temp_pt.y = pts[i].y;
				m_pMain->m_vtTraceData_Fitting.push_back(temp_pt);
			}
		}
	}

	CString str_temp;
	str_temp.Format("%d - %d - %d - %d - %d", area_pos[1], area_pos[2], area_pos[3], area_pos[4], area_pos[5]);
	::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str_temp);
	/*

		FILE *fp = fopen("D:\\tract_pts.csv", "w");
		char cData[MAX_PATH] = { 0, };
		if (fp != NULL)
		{
			for (int i = 0; i < pts.size(); i++)
			{
				sprintf(cData, "%f, %f\n", pts[i].x, pts[i].y);
				fwrite(cData, strlen(cData), 1, fp);
			}
			fclose(fp);
		}*/


	if (m_pMain->m_vtTraceData_Fitting.size() == pts.size())
	{
		for (int i = 0; i < m_pMain->m_vtTraceData_Fitting.size(); i++)
		{
			pts[i].x = m_pMain->m_vtTraceData_Fitting[i].x;
			pts[i].y = m_pMain->m_vtTraceData_Fitting[i].y;
		}

	}


	delete[]points;
}

double square(double init, double x)
{
	return init + x * x;
}
double cubic(double init, double x)
{
	return init + x * x * x;
}
double forth_power(double init, double x)
{
	return init + x * x * x * x;
}

BOOL CFormMainView::get_2nd_order_regression(std::vector<double>* srcX, std::vector<double>* srcY, double* b0, double* b1, double* b2)
{
	if (srcX->size() <= 3 || srcY->size() <= 3)	return FALSE;
	if (srcX->size() != srcY->size())			return FALSE;

	double Y = std::accumulate(srcY->begin(), srcY->end(), 0.0);
	double X = std::accumulate(srcX->begin(), srcX->end(), 0.0);
	double X2 = std::accumulate(srcX->begin(), srcX->end(), 0.0, square);
	double X3 = std::accumulate(srcX->begin(), srcX->end(), 0.0, cubic);
	double X4 = std::accumulate(srcX->begin(), srcX->end(), 0.0, forth_power);

	double K = 0.0;
	double L = 0.0;
	int i = 0;

	int n = (int)srcX->size();

	for (i = 0; i < n; i++) {
		K += ((*srcY)[i] * (*srcX)[i] * (*srcX)[i]);
		L += ((*srcY)[i] * (*srcX)[i]);
	}

	double denominator = -n * X4 * X2 + X4 * X * X + X2 * X2 * X2 + X3 * X3 * n - 2 * X3 * X * X2;
	double b0p = -(Y * X4 * X2 - Y * X3 * X3 - X * L * X4 + X * X3 * K - X2 * X2 * K + X2 * X3 * L);
	double b1p = X * Y * X4 - X * K * X2 - L * n * X4 + X3 * n * K - Y * X2 * X3 + X2 * X2 * L;
	double b2p = -(K * n * X2 - K * X * X - X2 * X2 * Y - X3 * n * L + X3 * X * Y + X * X2 * L);


	*b0 = b0p / denominator;
	*b1 = b1p / denominator;
	*b2 = b2p / denominator;

	return TRUE;
}
void CFormMainView::fnSearchMaxMinPos(int xo, int yo, std::vector< double > data, float th, CDC* pDC)
{
	std::vector< float > fdata;

	for (int i = 0; i<int(data.size()); i++)
	{
		fdata.push_back(float(data.at(i)));
	}

	Persistence1D p;
	p.RunPersistence(fdata);

	vector< TPairedExtrema > Extrema;
	p.GetPairedExtrema(Extrema, th);

	CString str;
	int xmin, xmax, ymin, ymax;
	for (vector< TPairedExtrema >::iterator it = Extrema.begin(); it != Extrema.end(); it++)
	{
		ymin = m_pMain->m_ELB_DiffInspResult[1].m_vTraceProfile[4].at((*it).MinIndex) + yo;
		ymax = m_pMain->m_ELB_DiffInspResult[1].m_vTraceProfile[4].at((*it).MaxIndex) + yo;
		xmin = m_pMain->m_ELB_DiffInspResult[1].m_vTraceProfile[3].at((*it).MinIndex) + xo;
		xmax = m_pMain->m_ELB_DiffInspResult[1].m_vTraceProfile[3].at((*it).MaxIndex) + xo;
		if (pDC != NULL)
		{
			pDC->Ellipse(int(xmin - 40), int(ymin - 40), int(xmin + 40), int(ymin + 40));
			pDC->Ellipse(int(xmax - 40), int(ymax - 40), int(xmax + 40), int(ymax + 40));
		}
	}

	str.Format("Global minimum (%d,%f)", p.GetGlobalMinimumIndex() + yo, p.GetGlobalMinimumValue());
	::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	//AfxMessageBox(str);

	fdata.clear();
}
void CFormMainView::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	if (nIDEvent < 8)
	{
		KillTimer(nIDEvent);
		if (!m_pMain->vt_job_info[0].model_info.getInspSpecParaInfo().getAVI_Save_ImageTypeEnable() &&
			m_pMain->vt_job_info[0].model_info.getAlignInfo().getUseAutoSaveAVI())
		{
			::SendMessageA(m_pDlgViewerMain[nIDEvent]->GetSafeHwnd(), WM_VIEW_CONTROL, 300, 0);
		}
	}
	else if (nIDEvent == 10000) // 자동 저장 기능 ,동영상 대신 저장
	{
		m_iSaveImageTypeNum++;
		saveAVI_ImageType(m_iSaveImageTypeNum);

		if (m_iSaveImageTypeNum > 20)
		{
			KillTimer(nIDEvent);			
			m_iSaveImageTypeNum = 0;
		}
	}
	//KJH 2022-07-08 Login 기능 추가
	else if(TIMER_AUTO_LOGOFF_ADMIN_MODE)
	{
		KillTimer(TIMER_AUTO_LOGOFF_ADMIN_MODE);

		m_pMain->m_nLogInUserLevelType = USER;
		OnViewControl(MSG_FMV_LOGIN_STATUS_DISPLAY, 0);

		theLog.logmsg(LOG_LOGIN, "[LogOut] Auto LogOff");
	}

	CFormView::OnTimer(nIDEvent);
}

void CFormMainView::saveAVI_ImageType(int n)
{
	SYSTEMTIME time;
	::GetLocalTime(&time);
	CString strDate;

	//날짜파일 생성
	CString strImageDir;

	strDate.Format("%04d%02d%02d", time.wYear, time.wMonth, time.wDay);
	strImageDir.Format("%s%s\\", m_pMain->m_strVideoDir, strDate);
	if (_access(strImageDir, 0) != 0)	CreateDirectory(strImageDir, NULL);

	//시간
	CString strFileDir;

	if (strlen(m_pMain->vt_job_info[0].main_object_id.c_str()) <= 13)
	{
		strFileDir.Format("%sNot Exist Panel ID\\", strImageDir);
		if (_access(strFileDir, 0) != 0)	CreateDirectory(strFileDir, NULL);
	}
	else
	{
		strFileDir.Format("%s%s\\", strImageDir,m_pMain->vt_job_info[0].main_object_id.c_str());
		if (_access(strFileDir, 0) != 0)	CreateDirectory(strFileDir, NULL);
	}

	//파일명
	CString fname;
	fname.Format("%sImage_%d.jpg", strFileDir, n);
	int W = m_pMain->m_stCamInfo[0].w;
	int H = m_pMain->m_stCamInfo[0].h;
	vector<int> params_jpg; // Tkyuha 20211214 영상 압축해서 저장하기
	params_jpg.push_back(IMWRITE_JPEG_QUALITY);
	params_jpg.push_back(m_pMain->vt_system_option[0].jpg_compress_rate_Result);
	cv::Mat videoFrame(H, W, CV_8UC1, m_pMain->getSrcBuffer(0));

	cv::imwrite(std::string(fname), videoFrame, params_jpg);
	videoFrame.release();
	params_jpg.clear();
}

bool compareNtoPN(TPairedExtrema a, TPairedExtrema b)
{
	return a.MaxIndex < b.MaxIndex;
}

double CFormMainView::fnCalcNozzleToPN_Distance(BYTE* pImage, int w, int h, int nJob, int nCam, int _thresh, CViewerEx* mViewer)
{
	FakeDC* pDC = NULL;
	CPen penLine(PS_SOLID, 4, RGB(255, 255, 0));

	if (mViewer != NULL)
	{
		mViewer->ClearOverlayDC();
		pDC = mViewer->getOverlayDC();
		mViewer->clearAllFigures();
		pDC->SelectObject(&penLine);
	}

	mViewer->OnLoadImageFromPtr(m_pMain->getSrcBuffer(nCam));

	int _NozzleX = 0;
	int _NozzleY = 0;
	int _PanelX = 0;
	int _PanelY = 0;

	double _DistanceGap = 9999.0;

	CString strText;
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);

	cv::Mat img(h, w, CV_8UC1, pImage), sobelDn, cropSobel, projectV;
	cv::Mat Rawimg = img.clone();
	cv::Rect r = cv::Rect(w / 2 - 2, h / 2 - 50, 4, 100);

	cv::cvtColor(img, img, COLOR_GRAY2BGR);

	sobelDirection(3, SOBEL_DN, Rawimg, sobelDn);

	cropSobel = sobelDn(r).clone();
	projectV = cvProjectionVertical(cropSobel, 1);

	// 20210928 Tkyuha Peak를 찾아서 표시
	Persistence1D _pPeak;
	std::vector<float> _pData;
	std::vector<TPairedExtrema> _pExtremaData;
	std::vector<TPairedExtrema> _pExtremaData_Max;

	for (int i = 0; i < projectV.rows; i++)
	{
		float gray = projectV.at<float>(i, 0);
		_pData.push_back(gray);
	}

	_pPeak.RunPersistence(_pData);
	_pPeak.GetPairedExtrema(_pExtremaData, _thresh); // 노즐 위치 찾기
	_pPeak.GetPairedExtrema(_pExtremaData_Max, 200); // Glass Edge 찾기

	sort(_pExtremaData.begin(), _pExtremaData.end(), compareNtoPN);
	sort(_pExtremaData_Max.begin(), _pExtremaData_Max.end(), compareNtoPN);

	if (_pExtremaData.size() >= 1 && _pExtremaData_Max.size() >= 1)
	{
		for (std::vector<TPairedExtrema>::iterator it = _pExtremaData.begin(); it != _pExtremaData.end(); it++)
		{
			int _MaxY = (*it).MaxIndex;

			_NozzleX = w / 2 - 20;
			_NozzleY = _MaxY + h / 2 - 50;
			break;
		}

		//for (std::vector<TPairedExtrema>::iterator it = _pExtremaData_Max.begin(); it != _pExtremaData_Max.end(); it++)
		//{
		//	int _MaxY = (*it).MaxIndex;
		//  // Glass Edge
		//	_PanelX = w / 2 - 20;
		//	_PanelY = _MaxY + h / 2 - 50;
		//	if(_NozzleY!= _PanelY)	break;
		//}

		//_DistanceGap = (_PanelY - _NozzleY) * yres;
		_DistanceGap = (h / 2 - _NozzleY) * yres;

	}
	else if (_pExtremaData.size() <= 1)
	{
		pDC->SetTextColor(RGB(255, 0, 0));
		strText.Format("Find NG");
		pDC->TextOutA(w / 2 + 200, h / 2, strText);
		cv::putText(img, std::string(strText), cv::Point(w / 2, h), cv::FONT_ITALIC, 2, cv::Scalar(0, 0, 255), 5);
	}

	if (mViewer != NULL)
	{
		CString strText;
		int		TextSize_Thickness = 5;
		int		TextSize = 5;
		int		TextPosition_Shift = 200;

		CFont font, * oldFont;
		font.CreateFont(50, 50, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
			OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
		oldFont = pDC->SelectObject(&font);

		pDC->MoveTo(_NozzleX, _NozzleY);
		pDC->LineTo(_NozzleX + 40, _NozzleY);

		//pDC->MoveTo(_PanelX, _PanelY);
		//pDC->LineTo(_PanelX + 40, _PanelY);

		pDC->SetTextColor(RGB(0, 0, 255));
		strText.Format("Distance: %.4fmm", _DistanceGap);
		pDC->TextOutA(_NozzleX + 20, _NozzleY, strText);

		strText.Format("Distance: %.4fmm", _DistanceGap);
		cv::putText(img, std::string(strText), cv::Point(_NozzleX + TextPosition_Shift, _NozzleY), cv::FONT_ITALIC, TextSize, cv::Scalar(0, 0, 255), TextSize_Thickness);
		cv::line(img, cv::Point(int(0), int(h / 2)), cv::Point(int(w), int(h / 2)), cv::Scalar(64, 255, 0), 1);
		cv::line(img, cv::Point(int(_NozzleX), int(_NozzleY)), cv::Point(int(_NozzleX + 40), int(_NozzleY)), cv::Scalar(64, 255, 64), 2);
		cv::circle(img, cv::Point(int(w / 2), int(h / 2)), 2, cv::Scalar(0, 255, 255), 2, 3);

		font.DeleteObject();

		mViewer->Invalidate();
	}

#pragma region Log 저장 경로 생성

	CString	Time_str = m_pMain->m_strResultTime[nJob];
	CString	Date_str = m_pMain->m_strResultDate[nJob];
	CTime NowTime;
	CString real_Time_str;
	NowTime = CTime::GetCurrentTime();
	real_Time_str.Format("%04d%02d%02d", NowTime.GetYear(), NowTime.GetMonth(), NowTime.GetDay());

	if (Time_str == "" || Date_str == "")
	{
		m_pMain->m_strResultDate[nJob].Format("%04d%02d%02d", NowTime.GetYear(), NowTime.GetMonth(), NowTime.GetDay());
		m_pMain->m_strResultTime[nJob].Format("%02d_%02d_%02d", NowTime.GetHour(), NowTime.GetMinute(), NowTime.GetSecond());

		Time_str = m_pMain->m_strResultTime[nJob];
		Date_str = m_pMain->m_strResultDate[nJob];
	}

	CString Cell_Name = m_pMain->vt_job_info[nJob].get_main_object_id();

	if (strlen(Cell_Name) < 14)
	{
		Cell_Name.Format("TEST_%s", Time_str);
	}

	CString strFileDir_Image, strFileDir_Image_Proc, strFileDir_Image_Raw;
	CString strFileDir_Files;

	strFileDir_Files.Format("%sFiles\\", m_pMain->m_strResultDir);
	if (_access(strFileDir_Files, 0) != 0)	CreateDirectory(strFileDir_Files, NULL);
	strFileDir_Files.Format("%s%s\\", strFileDir_Files, Date_str);
	if (_access(strFileDir_Files, 0) != 0)	CreateDirectory(strFileDir_Files, NULL);
	strFileDir_Files.Format("%s%s\\", strFileDir_Files, m_pMain->m_strCurrentModelName);
	if (_access(strFileDir_Files, 0) != 0)	CreateDirectory(strFileDir_Files, NULL);
	//KJH 2022-08-03 Nozzle Log 경로 변경(요청자 김경철수석)
	strFileDir_Files.Format("%sNOZZLE_YGAP\\", strFileDir_Files);
	if (_access(strFileDir_Files, 0) != 0)	CreateDirectory(strFileDir_Files, NULL);

	strFileDir_Image = m_pMain->m_strImageDir + Date_str;
	if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);
	strFileDir_Image.Format("%s\\%s", strFileDir_Image, m_pMain->m_strCurrentModelName);
	if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);
	//KJH 2022-08-03 Nozzle Log 경로 변경(요청자 김경철수석)
	/*strFileDir_Image.Format("%s\\%s", strFileDir_Image, m_pMain->vt_job_info[nJob].get_job_name());
	if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);*/
	strFileDir_Image.Format("%s\\NOZZLE_YGAP\\", strFileDir_Image);
	if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);

	strFileDir_Image.Format("%s%s\\", strFileDir_Image, Cell_Name);
	if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);

	strFileDir_Files.Format("%sNOZZLE_YGAP_Summary_%s.csv", strFileDir_Files, Date_str);

	strFileDir_Image_Raw.Format("%s%s_%s_Raw.jpg", strFileDir_Image, Cell_Name, Time_str);
	strFileDir_Image_Proc.Format("%s%s_%s.jpg", strFileDir_Image, Cell_Name, Time_str);

	if (_access(strFileDir_Image_Proc, 0) != 0) strFileDir_Image_Proc.Format("%s%s_%s.jpg", strFileDir_Image, Cell_Name, real_Time_str);
	if (_access(strFileDir_Image_Raw, 0) != 0)	strFileDir_Image_Raw.Format("%s%s_%s_Raw.jpg", strFileDir_Image, Cell_Name, real_Time_str);

	cv::imwrite(std::string(strFileDir_Image_Proc), img);
	//cv::imwrite(std::string(strFileDir_Image_Proc), sobelDn);
	cv::imwrite(std::string(strFileDir_Image_Raw), Rawimg);

#pragma endregion

#pragma region PLC Data 전송

	long pData[2] = { 0, };

	if (_DistanceGap == 9999.0)
	{
		pData[0] = LOWORD(_DistanceGap);
		pData[1] = HIWORD(_DistanceGap);
	}
	else
	{
		pData[0] = LOWORD(_DistanceGap * 10000);
		pData[1] = HIWORD(_DistanceGap * 10000);
	}

	//// 거리 전송 //// 15160, 15360, 15560, 15760
	g_CommPLC.SetWord(m_pMain->vt_job_info[nJob].plc_addr_info.write_word_start + 40, 2, pData);

	CString str;
	str.Format("[%s] Nozzle Check - %.4fmm", m_pMain->vt_job_info[nJob].job_name.c_str(), _DistanceGap);
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

	//KJH 2021-09-17 Insp Summary에 Log 추가
	m_pMain->m_ELB_TraceResult.m_nNozzleCheckDistance = _DistanceGap;

#pragma endregion

#pragma	region Data Log 저장

	CString strTemp;

	BOOL bNew = FALSE;
	if (_access(strFileDir_Files, 0) != 0) bNew = TRUE;

	CFile file;
	if (file.Open(strFileDir_Files, CFile::modeCreate | CFile::modeReadWrite | CFile::modeNoTruncate) != TRUE)
	{

	}
	else
	{
		file.SeekToEnd();

		if (bNew)
		{
			strTemp = "DATE,TIME,PANEL ID,Distance\r\n";
			file.Write(strTemp, strTemp.GetLength());
		}

		strTemp.Format("%s,%s,%s,%.3f\r\n", Date_str, Time_str, Cell_Name, _DistanceGap);

		file.Write(strTemp, strTemp.GetLength());
		file.Close();
	}

#pragma endregion

	penLine.DeleteObject();

	img.release();
	Rawimg.release();
	sobelDn.release();
	cropSobel.release();
	projectV.release();

	_pData.clear();
	_pExtremaData.clear();
	_pExtremaData_Max.clear();

	return _DistanceGap;
}

double CFormMainView::fnCalcNozzleToPN_Distance2(BYTE* pImage, int w, int h, int nJob, int nCam, int _thresh, CViewerEx* mViewer)
{
	FakeDC* pDC = NULL;
	CPen penE(PS_SOLID, 1, RGB(255, 0, 0));
	CPen penG(PS_SOLID, 3, RGB(0, 255, 0));
	CPen penR(PS_SOLID, 3, RGB(255, 0, 0));


	if (mViewer != NULL)
	{
		pDC = mViewer->getOverlayDC();
		mViewer->ClearOverlayDC();
		mViewer->clearAllFigures();
		pDC->SelectObject(&penE);
	}

	mViewer->OnLoadImageFromPtr(m_pMain->getSrcBuffer(nCam));

	int _NozzleX = 0;
	int _NozzleY = 0;
	int _PanelX = 0;
	int _PanelY = 0;

	double _DistanceGap = 9999.0;

	CString strText;
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);

	cv::Mat img(h, w, CV_8UC1, pImage), sobelDn, sobelUp, cropSobel, cropSobelUp, projectV;
	cv::Mat Rawimg = img.clone();
	cv::Rect r = cv::Rect(w / 2 - 2, h / 2 - 50, 4, 100);

	cv::cvtColor(img, img, COLOR_GRAY2BGR);

	sobelDirection(3, SOBEL_DN, Rawimg, sobelDn);
	sobelDirection(3, SOBEL_UP, Rawimg, sobelUp);

	cropSobel = sobelDn(r).clone();
	projectV = cvProjectionVertical(cropSobel, 1);

	// 20210928 Tkyuha Peak를 찾아서 표시
	Persistence1D _pPeak;
	std::vector<float> _pData;
	std::vector<TPairedExtrema> _pExtremaData;
	std::vector<TPairedExtrema> _pExtremaData_Max;

	CFont font, * oldFont;
	font.CreateFont(70, 70, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
		OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
	oldFont = pDC->SelectObject(&font);
	int OLD_BKMODE = 0;
	OLD_BKMODE = pDC->SetBkMode(TRANSPARENT);
	pDC->SelectObject(GetStockObject(NULL_BRUSH));

	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
	int W = m_pMain->m_stCamInfo[real_cam].w;
	int H = m_pMain->m_stCamInfo[real_cam].h;
	int Center_W = m_pMain->m_stCamInfo[real_cam].w / 2;
	int Center_H = m_pMain->m_stCamInfo[real_cam].h / 2;

	// =========================================================================================== 
	cv::Mat Porcimg = img.clone();

	int _imgThresholdvalue = 10;

	// 노즐쪽 평균 Gray
	int size_pixcel = 20;
	CRect rect_area_N, rect_area_P;
	rect_area_N.left = Center_W - size_pixcel;
	rect_area_N.right = Center_W + size_pixcel;
	rect_area_N.top = Center_H - 60 - size_pixcel;
	rect_area_N.bottom = Center_H - 60 + size_pixcel;

	cv::Mat InspImg1 = Porcimg(cv::Rect(rect_area_N.left, rect_area_N.top, rect_area_N.Width(), rect_area_N.Height()));
	cv::Scalar scalar1 = cv::mean(InspImg1);
	double N_Gray = scalar1.val[0];

	// 패널쪽 평균 Gray
	size_pixcel = 100;
	rect_area_P.left = Center_W - size_pixcel;
	rect_area_P.right = Center_W + size_pixcel;
	rect_area_P.top = Center_H + 110 - size_pixcel;
	rect_area_P.bottom = Center_H + 110 + size_pixcel;

	cv::Mat InspImg2 = Porcimg(cv::Rect(rect_area_P.left, rect_area_P.top, rect_area_P.Width(), rect_area_P.Height()));
	cv::Scalar scalar2 = cv::mean(InspImg2);
	double P_Gray = scalar2.val[0];

	// 두개의 평균 Gray 값 
	_imgThresholdvalue = int((N_Gray + P_Gray) / 2);
	if (N_Gray < P_Gray)
	{
		_imgThresholdvalue = MIN(40, _imgThresholdvalue);
	}
	Porcimg.release();
	InspImg1.release();
	InspImg2.release();

	CPen penW(PS_SOLID, 3, RGB(255, 255, 255));
	CPen penB(PS_SOLID, 3, COLOR_DDARK_GRAY);

	// Draw
	if (N_Gray >= P_Gray)	pDC->SelectObject(&penW);
	else					pDC->SelectObject(&penB);
	pDC->Rectangle(rect_area_N);
	pDC->Rectangle(rect_area_P);

	if (N_Gray >= P_Gray)	pDC->SetTextColor(COLOR_WHITE);
	else					pDC->SetTextColor(COLOR_WHITE);
	//strText.Format("nozzle gray = %.1f", N_Gray);
	//pDC->TextOutA(100, 300, strText);
	//strText.Format("panel gray = %.1f", P_Gray);
	//pDC->TextOutA(100, 500, strText);

	mViewer->AddHardGraphic(new GraphicLabel(10, 30, fmt("Nozzle histogram: %.1f", N_Gray), pDC->GetCurrentTextColor()));
	mViewer->AddHardGraphic(new GraphicLabel(10, 50, fmt("Panel histogram: %.1f", P_Gray), pDC->GetCurrentTextColor()));

	if (_thresh == 0)
	{
		pDC->SetTextColor(RGB(0, 0, 255));

		//strText.Format("Auto Mode On");
		//pDC->TextOutA(100, 100, strText);
		mViewer->AddHardGraphic(new GraphicLabel(10, 10, "Auto Mode On", pDC->GetCurrentTextColor()));

		_thresh = _imgThresholdvalue;

	}
	else
	{
		pDC->SetTextColor(RGB(0, 0, 255));

		//strText.Format("Auto Mode Off");
		mViewer->AddHardGraphic(new GraphicLabel(10, 10, "Auto Mode Off", pDC->GetCurrentTextColor()));
		//pDC->TextOutA(100, 100, strText);
	}
	// =========================================================================================== 

	for (int i = 0; i < projectV.rows; i++)
	{
		float gray = projectV.at<float>(i, 0);
		_pData.push_back(gray);
	}

	_pPeak.RunPersistence(_pData);
	_pPeak.GetPairedExtrema(_pExtremaData, _thresh); // 노즐 위치 찾기
	_pPeak.GetPairedExtrema(_pExtremaData_Max, 200); // Glass Edge 찾기

	sort(_pExtremaData.begin(), _pExtremaData.end(), compareNtoPN);
	sort(_pExtremaData_Max.begin(), _pExtremaData_Max.end(), compareNtoPN);

	//if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseMetalTraceFirst()) // Tkyuha 20220811 메탈로 얼라인 하기 위함
	// KJH2 2022-08-17 Metal_Trace_First -> Y Light Enable/Disalbe 으로 판단 
	//if(m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getUseCenterNozzleYLight())
	if (N_Gray >= P_Gray)
	{
		cv::Mat projectVup;

		cropSobelUp = sobelUp(r).clone();
		projectVup = cvProjectionVertical(cropSobelUp, 1);

		std::vector<float> _pDataUp;
		std::vector<TPairedExtrema> _pExtremaDataUp;


		for (int i = 0; i < projectVup.rows; i++)
		{
			float gray = projectVup.at<float>(i, 0);
			_pDataUp.push_back(gray);
		}

		_pPeak.RunPersistence(_pDataUp);
		_pPeak.GetPairedExtrema(_pExtremaDataUp, _thresh); // 노즐 위치 찾기
		sort(_pExtremaDataUp.begin(), _pExtremaDataUp.end(), compareNtoPN);

		//2022.09.16 ksm 먼저 못찾으면 thresh 값 낮추면서 찾아보기 
		//파라미터 0인경우-> (Edge gray + Nozzle gray) / 2 -> 값이 너무 높아 못찾는 경우 발생
		if (_pExtremaDataUp.size() < 1)
		{
			for (int i = 0; i < _thresh; i++)
			{
				_thresh /= 2;

				_pPeak.GetPairedExtrema(_pExtremaDataUp, _thresh);

				if (_thresh < 10 || _pExtremaDataUp.size() >= 1) break;
			}
		}
		if (_pExtremaDataUp.size() >= 1)
		{
			for (std::vector<TPairedExtrema>::iterator it = _pExtremaDataUp.begin(); it != _pExtremaDataUp.end(); it++)
			{
				int _MaxY = (*it).MaxIndex;

				_NozzleX = w / 2 - 20;
				_NozzleY = _MaxY + h / 2 - 50;
				break;
			}

			_DistanceGap = (h / 2 - _NozzleY) * yres;

		}
		else if (_pExtremaDataUp.size() <= 1)
		{
			pDC->SetTextColor(RGB(255, 0, 0));
			strText.Format("Find NG");
			pDC->TextOutA(w / 2 + 200, h / 2, strText);
			cv::putText(img, std::string(strText), cv::Point(w / 2, h), cv::FONT_ITALIC, 2, cv::Scalar(0, 0, 255), 5);
		}

		sobelUp.release();
		cropSobelUp.release();
		projectVup.release();
	}
	else
	{
		if (_pExtremaData.size() >= 1 && _pExtremaData_Max.size() >= 1)
		{
			for (std::vector<TPairedExtrema>::iterator it = _pExtremaData.begin(); it != _pExtremaData.end(); it++)
			{
				int _MaxY = (*it).MaxIndex;

				_NozzleX = w / 2 - 20;
				_NozzleY = _MaxY + h / 2 - 50;
				break;
			}

			//for (std::vector<TPairedExtrema>::iterator it = _pExtremaData_Max.begin(); it != _pExtremaData_Max.end(); it++)
			//{
			//	int _MaxY = (*it).MaxIndex;
			//  // Glass Edge
			//	_PanelX = w / 2 - 20;
			//	_PanelY = _MaxY + h / 2 - 50;
			//	if(_NozzleY!= _PanelY)	break;
			//}

			//_DistanceGap = (_PanelY - _NozzleY) * yres;
			_DistanceGap = (h / 2 - _NozzleY) * yres;

		}
		else if (_pExtremaData.size() <= 1)
		{
			pDC->SetTextColor(RGB(255, 0, 0));
			strText.Format("Find NG");
			pDC->TextOutA(w / 2 + 200, h / 2, strText);
			cv::putText(img, std::string(strText), cv::Point(w / 2, h), cv::FONT_ITALIC, 2, cv::Scalar(0, 0, 255), 5);
		}

		sobelDn.release();
		cropSobel.release();
		projectV.release();
	}

	double Min_spec = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(8);
	double Max_spec = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(8);
	bool bNozzlePanelHoleCheck = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getNozzlePanelHoleCheck();

	BOOL bJudge = TRUE;
	if (bNozzlePanelHoleCheck && (_DistanceGap < Min_spec || _DistanceGap > Max_spec))
	{
		bJudge = FALSE;
	}

	if (mViewer != NULL)
	{
		pDC->SelectObject(&penE);
		pDC->MoveTo(0, Center_H);
		pDC->LineTo(W, Center_H);

		if (bJudge)		pDC->SelectObject(&penG);
		else			pDC->SelectObject(&penR);

		CString strText;
		int		TextSize_Thickness = 5;
		int		TextSize = 5;
		int		TextPosition_Shift = 200;

		pDC->MoveTo(_NozzleX, _NozzleY);
		pDC->LineTo(_NozzleX + 40, _NozzleY);

		//pDC->MoveTo(_PanelX, _PanelY);
		//pDC->LineTo(_PanelX + 40, _PanelY);

		if (bJudge)  pDC->SetTextColor(COLOR_GREEN);
		else		 pDC->SetTextColor(COLOR_RED);
		strText.Format("Threshold: %d", _thresh);
		//pDC->TextOutA(100, 700, strText);

		mViewer->AddHardGraphic(new GraphicLabel(10, 70, fmt("Threshold: %d", _thresh), pDC->GetCurrentTextColor()));

		if (bJudge) cv::putText(img, std::string(strText), cv::Point(100, 700), cv::FONT_ITALIC, TextSize, cv::Scalar(0, 255, 0), TextSize_Thickness);
		else		cv::putText(img, std::string(strText), cv::Point(100, 700), cv::FONT_ITALIC, TextSize, cv::Scalar(0, 0, 255), TextSize_Thickness);

		strText.Format("Y Gap: %.4fmm", _DistanceGap);
		pDC->TextOutA(_NozzleX + 80, H / 2 - 70, strText);


		if (bJudge) cv::putText(img, std::string(strText), cv::Point(_NozzleX + TextPosition_Shift, _NozzleY), cv::FONT_ITALIC, TextSize, cv::Scalar(0, 255, 0), TextSize_Thickness);
		else		cv::putText(img, std::string(strText), cv::Point(_NozzleX + TextPosition_Shift, _NozzleY), cv::FONT_ITALIC, TextSize, cv::Scalar(0, 0, 255), TextSize_Thickness);
		cv::line(img, cv::Point(int(0), int(h / 2)), cv::Point(int(w), int(h / 2)), cv::Scalar(0, 0, 255), 1);
		cv::line(img, cv::Point(int(_NozzleX), int(_NozzleY)), cv::Point(int(_NozzleX + 40), int(_NozzleY)), cv::Scalar(64, 255, 64), 2);
		cv::circle(img, cv::Point(int(w / 2), int(h / 2)), 2, cv::Scalar(0, 255, 255), 2, 3);

		font.DeleteObject();

		//mViewer->Invalidate();
		mViewer->DirtyRefresh();
	}

#pragma region Log 저장 경로 생성

	CString	Time_str = m_pMain->m_strResultTime[nJob];
	CString	Date_str = m_pMain->m_strResultDate[nJob];
	CTime NowTime;
	CString real_Time_str;
	NowTime = CTime::GetCurrentTime();
	real_Time_str.Format("%04d%02d%02d", NowTime.GetYear(), NowTime.GetMonth(), NowTime.GetDay());

	if (Time_str == "" || Date_str == "")
	{
		m_pMain->m_strResultDate[nJob].Format("%04d%02d%02d", NowTime.GetYear(), NowTime.GetMonth(), NowTime.GetDay());
		m_pMain->m_strResultTime[nJob].Format("%02d_%02d_%02d", NowTime.GetHour(), NowTime.GetMinute(), NowTime.GetSecond());

		Time_str = m_pMain->m_strResultTime[nJob];
		Date_str = m_pMain->m_strResultDate[nJob];
	}

	CString Cell_Name = m_pMain->vt_job_info[nJob].get_main_object_id();

	if (strlen(Cell_Name) < 10)
	{
		Cell_Name.Format("Test_%s", Time_str);
	}

	CString strFileDir_Image, strFileDir_Image_Proc, strFileDir_Image_Raw;
	CString strFileDir_Files;

	strFileDir_Files.Format("%sFiles\\", m_pMain->m_strResultDir);
	if (_access(strFileDir_Files, 0) != 0)	CreateDirectory(strFileDir_Files, NULL);
	strFileDir_Files.Format("%s%s\\", strFileDir_Files, Date_str);
	if (_access(strFileDir_Files, 0) != 0)	CreateDirectory(strFileDir_Files, NULL);
	strFileDir_Files.Format("%s%s\\", strFileDir_Files, m_pMain->m_strCurrentModelName);
	if (_access(strFileDir_Files, 0) != 0)	CreateDirectory(strFileDir_Files, NULL);
	//KJH 2022-08-03 Nozzle Log 경로 변경(요청자 김경철수석)
	strFileDir_Files.Format("%sNOZZLE_YGAP\\", strFileDir_Files);
	if (_access(strFileDir_Files, 0) != 0)	CreateDirectory(strFileDir_Files, NULL);

	strFileDir_Image = m_pMain->m_strImageDir + Date_str;
	if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);
	strFileDir_Image.Format("%s\\%s", strFileDir_Image, m_pMain->m_strCurrentModelName);
	if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);
	//KJH 2022-08-03 Nozzle Log 경로 변경(요청자 김경철수석)
	/*strFileDir_Image.Format("%s\\%s", strFileDir_Image, m_pMain->vt_job_info[nJob].get_job_name());
	if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);*/
	strFileDir_Image.Format("%s\\NOZZLE_YGAP\\", strFileDir_Image);
	if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);

	strFileDir_Image.Format("%s%s\\", strFileDir_Image, Cell_Name);
	if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);

	strFileDir_Files.Format("%sNOZZLE_YGAP_Summary_%s.csv", strFileDir_Files, Date_str);

	strFileDir_Image_Raw.Format("%s%s_%s_Raw.jpg", strFileDir_Image, Cell_Name, Time_str);
	strFileDir_Image_Proc.Format("%s%s_%s.jpg", strFileDir_Image, Cell_Name, Time_str);

	if (_access(strFileDir_Image_Proc, 0) == 0) strFileDir_Image_Proc.Format("%s%s_%s_%s.jpg", strFileDir_Image, Cell_Name, Time_str, real_Time_str);
	if (_access(strFileDir_Image_Raw, 0) == 0)	strFileDir_Image_Raw.Format("%s%s_%s_%s_Raw.jpg", strFileDir_Image, Cell_Name, Time_str, real_Time_str);

	cv::imwrite(std::string(strFileDir_Image_Proc), img);
	//cv::imwrite(std::string(strFileDir_Image_Proc), sobelDn);
	cv::imwrite(std::string(strFileDir_Image_Raw), Rawimg);

#pragma endregion

#pragma region PLC Data 전송

	long pData[2] = { 0, };

	if (_DistanceGap == 9999.0)
	{
		pData[0] = LOWORD(_DistanceGap);
		pData[1] = HIWORD(_DistanceGap);
	}
	else
	{
		pData[0] = LOWORD(_DistanceGap * 10000);
		pData[1] = HIWORD(_DistanceGap * 10000);
	}

	//// 거리 전송 //// 15160, 15360, 15560, 15760
	g_CommPLC.SetWord(m_pMain->vt_job_info[nJob].plc_addr_info.write_word_start + 40, 2, pData);

	CString str;
	str.Format("[%s] Nozzle Check - %.4fmm", m_pMain->vt_job_info[nJob].job_name.c_str(), _DistanceGap);
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

	//KJH 2021-09-17 Insp Summary에 Log 추가
	m_pMain->m_ELB_TraceResult.m_nNozzleCheckDistance = _DistanceGap;

#pragma endregion

#pragma	region Data Log 저장

	CString strTemp;

	BOOL bNew = FALSE;
	if (_access(strFileDir_Files, 0) != 0) bNew = TRUE;

	CFile file;
	if (file.Open(strFileDir_Files, CFile::modeCreate | CFile::modeReadWrite | CFile::modeNoTruncate) != TRUE)
	{

	}
	else
	{
		file.SeekToEnd();

		if (bNew)
		{
			strTemp = "DATE,TIME,PANEL ID,Distance\r\n";
			file.Write(strTemp, strTemp.GetLength());
		}

		strTemp.Format("%s,%s,%s,%.3f\r\n", Date_str, Time_str, Cell_Name, _DistanceGap);

		file.Write(strTemp, strTemp.GetLength());
		file.Close();
	}

#pragma endregion
	penW.DeleteObject();
	penE.DeleteObject();
	penB.DeleteObject();
	penG.DeleteObject();
	penR.DeleteObject();

	img.release();
	Rawimg.release();
	sobelDn.release();
	cropSobel.release();
	projectV.release();

	_pData.clear();
	_pExtremaData.clear();
	_pExtremaData_Max.clear();

	return _DistanceGap;
}

double CFormMainView::fnCalcYgapCINK2(BYTE* pImage, int w, int h, int nJob, int nCam, int _thresh, CViewerEx* mViewer)
{
	int real_cam = m_pMain->vt_job_info[nJob].camera_index[nCam];
	int W = m_pMain->m_stCamInfo[real_cam].w;
	int H = m_pMain->m_stCamInfo[real_cam].h;
	int Center_W = m_pMain->m_stCamInfo[real_cam].w / 2;
	int Center_H = m_pMain->m_stCamInfo[real_cam].h / 2;

	//KJH 2021-09-13 getSrcBuffer(카메라 Grab 이미지)
	//KJH 2021-09-13 getProcBuffer(별도 작업 이미지)
	cv::Mat Rawimg(h, w, CV_8UC1, pImage);
	cv::Mat img = Rawimg.clone();
	cv::Mat Porcimg = Rawimg.clone();
	//cv::Rect r = cv::Rect(w / 2 - 2, h / 2 - 50, 4, 100);

	FakeDC* pDC = NULL;
	CPen penW(PS_SOLID, 1, RGB(255, 255, 255));
	CPen penE(PS_SOLID, 1, RGB(255, 0, 0));
	CPen penS(PS_SOLID, 3, RGB(0, 255, 0));
	CPen penB(PS_SOLID, 2, RGB(0, 0, 255));
	CPen penB2(PS_DOT, 2, RGB(0, 0, 255));
	CFont font, * oldFont;
	int OLD_BKMODE = 0;
	BOOL breturn = TRUE;
	CString str;

	if (mViewer != NULL)
	{
		pDC = mViewer->getOverlayDC();
		mViewer->ClearOverlayDC();
		mViewer->clearAllFigures();
		pDC->SelectObject(&penE);

		font.CreateFont(50, 50, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
			OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
		oldFont = pDC->SelectObject(&font);

		OLD_BKMODE = pDC->SetBkMode(TRANSPARENT);
		pDC->SelectObject(GetStockObject(NULL_BRUSH));
	}
	cv::cvtColor(img, img, COLOR_GRAY2BGR);

	double posx = 0.0, posy = 0.0;
	double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(nCam, 0);
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(nCam, 0);

	int Find_X, Find_Y = 0;
	double distance_MtoN = 9999.0;

	// =========================================================================================== 영상처리
	int _imgThresholdvalue = 10;
	CString strText;

	// 노즐쪽 평균 Gray
	int size_pixcel = 20;
	CRect rect_area_N, rect_area_P;
	rect_area_N.left = Center_W - size_pixcel;
	rect_area_N.right = Center_W + size_pixcel;
	rect_area_N.top = Center_H - 60 - size_pixcel;
	rect_area_N.bottom = Center_H - 60 + size_pixcel;

	cv::Mat InspImg1 = Porcimg(cv::Rect(rect_area_N.left, rect_area_N.top, rect_area_N.Width(), rect_area_N.Height()));
	cv::Scalar scalar1 = cv::mean(InspImg1);
	double N_Gray = scalar1.val[0];

	// 패널쪽 평균 Gray
	size_pixcel = 100;
	rect_area_P.left = Center_W - size_pixcel;
	rect_area_P.right = Center_W + size_pixcel;
	rect_area_P.top = Center_H + 110 - size_pixcel;
	rect_area_P.bottom = Center_H + 110 + size_pixcel;

	cv::Mat InspImg2 = Porcimg(cv::Rect(rect_area_P.left, rect_area_P.top, rect_area_P.Width(), rect_area_P.Height()));
	cv::Scalar scalar2 = cv::mean(InspImg2);
	double P_Gray = scalar2.val[0];

	// 두개의 평균 Gray 값 
	_imgThresholdvalue = int((N_Gray + P_Gray) / 2);

	if (_imgThresholdvalue >= 50)
	{
		// Draw
		pDC->SelectObject(&penW);
		pDC->Rectangle(rect_area_N);
		pDC->SetTextColor(COLOR_WHITE);
		strText.Format("nozzle gray = %.1f", N_Gray);
		pDC->TextOutA(100, 100, strText);

		pDC->Rectangle(rect_area_P);
		pDC->SelectObject(&penW);
		strText.Format("panel gray = %.1f", P_Gray);
		pDC->TextOutA(100, 300, strText);

		pDC->SetTextColor(COLOR_GREEN);
		strText.Format("average gray= %d -> 30", _imgThresholdvalue);
		pDC->TextOutA(100, 500, strText);

		_imgThresholdvalue = 30;
	}
	else
	{
		_imgThresholdvalue = 10;
	}


	cv::threshold(Porcimg, Porcimg, _imgThresholdvalue, 255, CV_THRESH_BINARY_INV);
	const uchar* dataptr = Porcimg.data;


	if (0)
	{
		imwrite("D:\\InspNozzleDistance_align_Final.jpg", Porcimg);
	}
	// ===========================================================================================

	BOOL bFind = FALSE;
	//검색 영역 

	for (int j = Center_H; j > Center_H / 2; j--)
	{
		// 무조건 가운데
		if (j < 0 || H <= j) continue;

		//Nozzle Find(Black Find))
		if (dataptr[j * W + Center_W] < 30) // 중심에서 5Pixel 아래서부터 0까지 검색 0~255 //Black이 0 / White가 255 
		{
			if (Find_Y == 0)
			{
				BOOL bContinue = FALSE;
				for (int k = 0; k < 20; k++)
				{
					if (dataptr[(j - k) * W + Center_W] < 30)
					{
						bContinue = TRUE;
					}
					else
					{
						bContinue = FALSE;
						break;
					}
				}
				if (bContinue == TRUE)
				{
					Find_Y = j;
					Find_X = Center_W;
					distance_MtoN = fabs(Find_Y - Center_H) * yres;
					bFind = TRUE;
				}
			}
		}
		if (bFind == TRUE) break;
	}

	double Min_spec = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(8);
	double Max_spec = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(8);
	bool bNozzlePanelHoleCheck = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getNozzlePanelHoleCheck();

	BOOL bJudge = TRUE;
	if (bNozzlePanelHoleCheck && (distance_MtoN < Min_spec || distance_MtoN > Max_spec))
	{
		bJudge = FALSE;
	}

	int		TextSize_Thickness = 10;
	int		TextSize = 5;
	int		TextPosition_Shift = 200;

	if (pDC != NULL)
	{
		pDC->SelectObject(&penB);
		pDC->MoveTo(Find_X, Find_Y);
		pDC->LineTo(Find_X, Center_H);

		pDC->SelectObject(&penE);
		pDC->MoveTo(0, Center_H);
		pDC->LineTo(W, Center_H);

		strText.Format("Y Gap: %.4fmm", distance_MtoN);
		if (bJudge) pDC->SetTextColor(COLOR_GREEN);
		else		 pDC->SetTextColor(COLOR_RED);

		pDC->SelectObject(&penS);
		pDC->Ellipse(int(Find_X - 2), int(Find_Y - 2), int(Find_X + 2), int(Find_Y + 2));

		pDC->TextOutA(Find_X + 50, Find_Y + 50, strText);
		pDC->SetBkMode(OLD_BKMODE);

		cv::putText(img, std::string(strText), cv::Point(Find_X + TextPosition_Shift, Find_Y), cv::FONT_ITALIC, TextSize, cv::Scalar(0, 0, 255), TextSize_Thickness);
		cv::line(img, cv::Point(int(Find_X), int(Find_Y)), cv::Point(int(Center_W), int(Center_H)), cv::Scalar(64, 255, 64), 2);
		cv::circle(img, cv::Point(int(Find_X), int(Find_Y)), 2, cv::Scalar(0, 255, 255), 2, 3);
	}
	else
	{
		cv::Scalar color;
		strText.Format("Y Gap: %.4fmm", distance_MtoN);
		if (bJudge) color = cv::Scalar(0, 0, 255);
		else		color = cv::Scalar(255, 0, 0);
		cv::putText(img, std::string(strText), cv::Point(Find_X + TextPosition_Shift, Find_Y), cv::FONT_ITALIC, TextSize, color, TextSize_Thickness);
		cv::line(img, cv::Point(int(Find_X), int(Find_Y)), cv::Point(int(Center_W), int(Center_H)), cv::Scalar(64, 255, 64), 2);
		cv::circle(img, cv::Point(int(Find_X), int(Find_Y)), 2, cv::Scalar(0, 255, 255), 2, 3);
	}

#pragma region Log 저장 경로 생성

	CString	Time_str = m_pMain->m_strResultTime[nJob];
	CString	Date_str = m_pMain->m_strResultDate[nJob];

	CString	real_Time_str;

	CTime NowTime;
	NowTime = CTime::GetCurrentTime();
	real_Time_str.Format("%04d%02d%02d", NowTime.GetYear(), NowTime.GetMonth(), NowTime.GetDay());

	if (Time_str == "" || Date_str == "")
	{
		m_pMain->m_strResultDate[nJob].Format("%04d%02d%02d", NowTime.GetYear(), NowTime.GetMonth(), NowTime.GetDay());
		m_pMain->m_strResultTime[nJob].Format("%02d_%02d_%02d", NowTime.GetHour(), NowTime.GetMinute(), NowTime.GetSecond());

		Time_str = m_pMain->m_strResultTime[nJob];
		Date_str = m_pMain->m_strResultDate[nJob];
	}

	CString Cell_Name = m_pMain->vt_job_info[nJob].get_main_object_id();

	if (strlen(Cell_Name) < 14)
	{
		Cell_Name.Format("Test_%s", Time_str);
	}

	CString strFileDir_Image, strFileDir_Image_Proc, strFileDir_Image_Raw;
	CString strFileDir_Files;

	strFileDir_Files.Format("%sFiles\\", m_pMain->m_strResultDir);
	if (_access(strFileDir_Files, 0) != 0)	CreateDirectory(strFileDir_Files, NULL);
	strFileDir_Files.Format("%s%s\\", strFileDir_Files, Date_str);
	if (_access(strFileDir_Files, 0) != 0)	CreateDirectory(strFileDir_Files, NULL);
	strFileDir_Files.Format("%s%s\\", strFileDir_Files, m_pMain->m_strCurrentModelName);
	if (_access(strFileDir_Files, 0) != 0)	CreateDirectory(strFileDir_Files, NULL);
	//KJH 2022-08-03 Nozzle Log 경로 변경(요청자 김경철수석)
	strFileDir_Files.Format("%sNOZZLE_YGAP\\", strFileDir_Files);
	if (_access(strFileDir_Files, 0) != 0)	CreateDirectory(strFileDir_Files, NULL);

	strFileDir_Image = m_pMain->m_strImageDir + Date_str;
	if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);
	strFileDir_Image.Format("%s\\%s", strFileDir_Image, m_pMain->m_strCurrentModelName);
	if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);
	//KJH 2022-08-03 Nozzle Log 경로 변경(요청자 김경철수석)
	/*strFileDir_Image.Format("%s\\%s", strFileDir_Image, m_pMain->vt_job_info[nJob].get_job_name());
	if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);*/
	strFileDir_Image.Format("%s\\NOZZLE_YGAP\\", strFileDir_Image);
	if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);

	strFileDir_Image.Format("%s%s\\", strFileDir_Image, Cell_Name);
	if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);

	strFileDir_Files.Format("%sNOZZLE_YGAP_Summary_%s.csv", strFileDir_Files, Date_str);

	strFileDir_Image_Raw.Format("%s%s_%s_Raw.jpg", strFileDir_Image, Cell_Name, Time_str);
	strFileDir_Image_Proc.Format("%s%s_%s.jpg", strFileDir_Image, Cell_Name, Time_str);

	if (_access(strFileDir_Image_Proc, 0) != 0) strFileDir_Image_Proc.Format("%s%s_%s.jpg", strFileDir_Image, Cell_Name, real_Time_str);
	if (_access(strFileDir_Image_Raw, 0) != 0)	strFileDir_Image_Raw.Format("%s%s_%s_Raw.jpg", strFileDir_Image, Cell_Name, real_Time_str);

	cv::imwrite(std::string(strFileDir_Image_Proc), img);
	//cv::imwrite(std::string(strFileDir_Image_Proc), sobelDn);
	cv::imwrite(std::string(strFileDir_Image_Raw), Rawimg);

#pragma endregion

#pragma region PLC Data 전송

	long pData[2] = { 0, };

	if (distance_MtoN == 9999.0)
	{
		pData[0] = LOWORD(distance_MtoN);
		pData[1] = HIWORD(distance_MtoN);
	}
	else
	{
		pData[0] = LOWORD(distance_MtoN * 10000);
		pData[1] = HIWORD(distance_MtoN * 10000);
	}

	//// 거리 전송 //// 15160, 15360, 15560, 15760
	g_CommPLC.SetWord(m_pMain->vt_job_info[nJob].plc_addr_info.write_word_start + 40, 2, pData);

	str.Format("[%s] Nozzle Check - %.4fmm", m_pMain->vt_job_info[nJob].job_name.c_str(), distance_MtoN);
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

	//KJH 2021-09-17 Insp Summary에 Log 추가
	m_pMain->m_ELB_TraceResult.m_nNozzleCheckDistance = distance_MtoN;

#pragma endregion

#pragma	region Data Log 저장

	CString strTemp;

	BOOL bNew = FALSE;
	if (_access(strFileDir_Files, 0) != 0) bNew = TRUE;

	CFile file;
	if (file.Open(strFileDir_Files, CFile::modeCreate | CFile::modeReadWrite | CFile::modeNoTruncate) != TRUE)
	{

	}
	else
	{
		file.SeekToEnd();

		if (bNew)
		{
			strTemp = "DATE,TIME,PANEL ID,Distance\r\n";
			file.Write(strTemp, strTemp.GetLength());
		}

		strTemp.Format("%s,%s,%s,%.3f\r\n", Date_str, Time_str, Cell_Name, distance_MtoN);

		file.Write(strTemp, strTemp.GetLength());
		file.Close();
	}

#pragma endregion
	if (pDC != NULL)	mViewer->Invalidate();
	mViewer->OnLoadImageFromPtr(pImage);

	penW.DeleteObject();
	penS.DeleteObject();
	penE.DeleteObject();
	penB.DeleteObject();
	penB2.DeleteObject();
	font.DeleteObject();

	img.release();
	Rawimg.release();
	Porcimg.release();

	return distance_MtoN;
}
//KJH 2021-11-23 Nozzle Align 확인용 Nozzle View 검사 알고리즘
double CFormMainView::fnCalcNozzleViewPos(BYTE* pImage, int w, int h, int nJob, int nCam, int _thresh, CViewerEx* mViewer)
{
	int W = m_pMain->m_stCamInfo[nCam].w;
	int H = m_pMain->m_stCamInfo[nCam].h;
	int Center_W = m_pMain->m_stCamInfo[nCam].w / 2;
	int Center_H = m_pMain->m_stCamInfo[nCam].h / 2;
	int _NozzlePosition = 0;
	BOOL bfind = false;
	CPoint FindPos;

	cv::Mat src(h, w, CV_8UC1, pImage);

	//KJH 2021-09-13 getSrcBuffer(카메라 Grab 이미지)
	//KJH 2021-09-13 getProcBuffer(별도 작업 이미지)
	cv::Mat img = src.clone();
	cv::Mat Porcimg = src.clone();
	cv::Mat Rawimg = src.clone();

	FakeDC* pDC = NULL;
	CPen penE(PS_SOLID, 3, RGB(255, 0, 0));
	CPen penS(PS_SOLID, 3, RGB(255, 255, 0));
	CPen penB(PS_SOLID, 3, RGB(0, 0, 255));
	CFont font, * oldFont;
	BOOL breturn = TRUE;
	CString str;
	int OLD_BKMODE = 0;

	if (mViewer != NULL)
	{
		pDC = mViewer->getOverlayDC();
		mViewer->ClearOverlayDC();
		pDC->SelectObject(&penE);

		font.CreateFont(50, 50, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
			OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
		oldFont = pDC->SelectObject(&font);

		OLD_BKMODE = pDC->SetBkMode(TRANSPARENT);
		pDC->SelectObject(GetStockObject(NULL_BRUSH));
	}

	//화면 갱신이 필요할까??
	mViewer->OnLoadImageFromPtr(m_pMain->getSrcBuffer(nCam));

#pragma region 영상처리

	//KJH 2021-10-26 알고리즘 분리( 0 : KJH 방식 / 1 : Tkyuha 방식)
	cv::Mat sobelUp, cropSobel, projectV;
	cv::Rect r = cv::Rect(W / 2 - 20, H / 2 - 100, 40, 200);
	sobelDirection(3, SOBEL_UP, img, sobelUp);
	cropSobel = sobelUp(r).clone();

	cv::cvtColor(img, img, COLOR_GRAY2BGR);

	int _imgThresholdvalue, _m_bShowImage = 1;

	double posx = 0.0, posy = 0.0;
	double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(0, 0);
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);

	//CKeyPadDlg dlg;
	//CString strNumber;
	//strNumber.Format("200");
	//dlg.SetValueString(false, strNumber);

	//if (dlg.DoModal() != IDOK)	return 0;

	//dlg.GetValue(strNumber);

	//_imgThresholdvalue = atoi(strNumber);

	_imgThresholdvalue = 250;

	if (_imgThresholdvalue <= 0 || _imgThresholdvalue > 255)
	{
		_imgThresholdvalue = 60;
	}
	if (_thresh <= 0 || _thresh > 255)
	{
		_thresh = 80;
	}

	cv::threshold(Porcimg, Porcimg, _imgThresholdvalue, 255, CV_THRESH_BINARY);

	const uchar* dataptr = Porcimg.data;

	if (_m_bShowImage == 0)
	{
		cvShowImage("Porcimg_0", Porcimg);
	}
	//sobelDirection(3, SOBEL_DN, Porcimg, Porcimg);
	//if (m_bShowImage == 0)
	//{
	//	cvShowImage("Porcimg_1", Porcimg);
	//}
	//cv::threshold(Porcimg, Porcimg, 0, 255, CV_THRESH_OTSU);
	//
	//if (m_bShowImage == 0)
	//{
	//	cvShowImage("Porcimg_2", Porcimg);
	//}	

#pragma endregion

#pragma region 위치 찾기

	//검색 영역 
	// CX - 100 ~ CX + 100
	// CY - 100 ~ CY + 100
	for (int i = Center_H + 0; i > 0; i--)
	{
		if (i < 0 || H <= i) continue;
		for (int j = Center_W + 300; j > Center_W - 300; j--)
		{
			if (j < 0 || W <= j) continue;
			if (dataptr[i * W + j] > _thresh) // 중심에서 5Pixel 아래서부터 0까지 검색 0~255 //Black이 0 / White가 255
			{
				_NozzlePosition++;
				//처음 Black->White인 점 찾기 [Panel Edge 찾기]]
				if (!bfind && _NozzlePosition == 1)
				{
					_NozzlePosition = j;
					bfind = true;
					FindPos.x = j;
					FindPos.y = i;
					break;
				}
			}
		}
	}

#pragma endregion

	CString strText;
	int		TextSize_Thickness = 10;
	int		TextSize = 5;
	int		TextPosition_Shift = 200;

#pragma region 화면 Display 및 이미지 로그 작업

	if (pDC != NULL)
	{
		if (bfind)	//OK일때
		{
			pDC->Ellipse(int(FindPos.x - 1), int(Center_H - 1), int(FindPos.x + 1), int(Center_H + 1));
			pDC->SelectObject(&penS);
			pDC->Ellipse(int(FindPos.x - 1), int(FindPos.y - 1), int(FindPos.x + 1), int(FindPos.y + 1));
			pDC->SelectObject(&penB);
			pDC->MoveTo(FindPos.x, Center_H);
			pDC->LineTo(FindPos.x, FindPos.y);

			strText.Format("X : %d, Y : %d", FindPos.x, FindPos.y);
			pDC->SetTextColor(COLOR_RED);
			pDC->TextOutA(1, Center_H, strText);
		}
		else					//NG일때
		{
			strText.Format("Find NG");
			pDC->SetTextColor(COLOR_RED);
			pDC->TextOutA(Center_W, Center_H, strText);
		}

		pDC->SetBkMode(OLD_BKMODE);
	}

#pragma endregion

	if (pDC != NULL)	mViewer->Invalidate();

	penS.DeleteObject();
	penE.DeleteObject();
	penB.DeleteObject();
	font.DeleteObject();
	//oldFont->DeleteObject();

	return breturn;
}
//KJH 2021-10-21 FDC 보고함수 추가_Circle
void CFormMainView::SendFDCValue_Circle(int nJob, int real_cam)
{
	//추가 될 수도 있으니 우선 200Word 기준으로 잡아둠
	const int m_nFDCCount = 100;
	const int PLC_DATA_REVISION = 10000;
	int m_nFDCCount_Current = 0;
	long pData[2 * m_nFDCCount] = { 0, };
	double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(0, 0);
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);
	int Center_W = m_pMain->m_stCamInfo[real_cam].w / 2;
	int Center_H = m_pMain->m_stCamInfo[real_cam].h / 2;
	int datasheet = 0;

	// KJH 2021-09-25 FDC 보고 준비
	// 
	// MP_RADIUS	(m_dCircleRadius_MP * xres),														[0]

	pData[m_nFDCCount_Current] = LOWORD(int(m_dCircleRadius_MP * xres * PLC_DATA_REVISION));
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = HIWORD(int(m_dCircleRadius_MP * xres * PLC_DATA_REVISION));

	// MP_X_POS		(m_ptCircleCenter_MP.x * xres),														[1]

	int nMP_x_Pos = m_ptCircleCenter_MP.x - Center_W;

	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = LOWORD(int(nMP_x_Pos * xres * PLC_DATA_REVISION));
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = HIWORD(int(nMP_x_Pos * xres * PLC_DATA_REVISION));

	// MP_Y_POS		(m_ptCircleCenter_MP.y * yres),														[2]

	int nMP_y_Pos = Center_H - m_ptCircleCenter_MP.y;

	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = LOWORD(int(nMP_y_Pos * yres * PLC_DATA_REVISION));
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = HIWORD(int(nMP_y_Pos * yres * PLC_DATA_REVISION));

	// PN_RADIUS	(m_pMain->m_dCircleRadius_PN * xres),												[3]

	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = LOWORD(int(m_pMain->m_dCircleRadius_PN * xres * PLC_DATA_REVISION));
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = HIWORD(int(m_pMain->m_dCircleRadius_PN * xres * PLC_DATA_REVISION));

	// PN_X_POS		(m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x * xres),								[4]

	int nPN_x_Pos = m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x - Center_W;

	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = LOWORD(int(nPN_x_Pos * xres * PLC_DATA_REVISION));
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = HIWORD(int(nPN_x_Pos * xres * PLC_DATA_REVISION));

	// PN_Y_POS		(m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y * yres),								[5]

	int nPN_y_Pos = Center_H - m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y;

	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = LOWORD(int(nPN_y_Pos * yres * PLC_DATA_REVISION));
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = HIWORD(int(nPN_y_Pos * yres * PLC_DATA_REVISION));

	// CC_RADIUS	(m_dCircleRadius_CC * xres),														[6]

	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = LOWORD(int(m_dCircleRadius_CC * xres * PLC_DATA_REVISION));
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = HIWORD(int(m_dCircleRadius_CC * xres * PLC_DATA_REVISION));

	// CC_X_POS		(m_ptCircleCenter_CC.x * xres),														[7]

	int nCC_x_Pos = m_ptCircleCenter_CC.x - Center_W;

	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = LOWORD(int(nCC_x_Pos * xres * PLC_DATA_REVISION));
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = HIWORD(int(nCC_x_Pos * xres * PLC_DATA_REVISION));

	// CC_Y_POS		(m_ptCircleCenter_CC.y * yres),														[8]

	int nCC_y_Pos = Center_H - m_ptCircleCenter_CC.y;

	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = LOWORD(int(nCC_y_Pos * yres * PLC_DATA_REVISION));
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = HIWORD(int(nCC_y_Pos * yres * PLC_DATA_REVISION));

	//순서 -> 0, 72, 144, 216, 288 Angle의 WetOut값 보고
	// 
	int method = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod();
	if (method == METHOD_CIRCLE)
	{
		//HTK 2022-06-14 Wetout FDC 보고 관련 5개에서 8개로 변경
		int n = 8; 
		//KJH 2022-05-05 72도 단위에서 90도로 변경 0, 90 , 180, 270 , 0으로 변경[마지막 0 data는 미사용]
		int step = 360 / (n);
		for (int i = 0; i < n; i++)
		{
			int curDeg = (270 + (step * i)) % 360;

			double m_nWetOut_In		= m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[0].at(curDeg);		//[9]		[12]	[15]	[18]	[21]	[24]   	[27]   [30]
			double m_nWetOut_Out	= m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[1].at(curDeg);		//[10]		[13]	[16]	[19]	[22]	[25]   	[28]   [31]
			double angle 			= m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[2].at(curDeg);			//[11]		[14]	[17]	[20]	[23]	[26]   	[29]   [32]

			m_nFDCCount_Current++;
			pData[m_nFDCCount_Current] = LOWORD(m_nWetOut_In * PLC_DATA_REVISION);
			m_nFDCCount_Current++;
			pData[m_nFDCCount_Current] = HIWORD(m_nWetOut_In * PLC_DATA_REVISION);
			m_nFDCCount_Current++;
			pData[m_nFDCCount_Current] = LOWORD(m_nWetOut_Out * PLC_DATA_REVISION);
			m_nFDCCount_Current++;
			pData[m_nFDCCount_Current] = HIWORD(m_nWetOut_Out * PLC_DATA_REVISION);
			m_nFDCCount_Current++;
			pData[m_nFDCCount_Current] = LOWORD(angle * PLC_DATA_REVISION);
			m_nFDCCount_Current++;
			pData[m_nFDCCount_Current] = HIWORD(angle * PLC_DATA_REVISION);
		}
	}
	else if (method == METHOD_LINE)
	{
		for (int i = 0; i < 8; i++)
		{
			double m_nWetOut	= m_pMain->m_dCPos[i].y;												//[9]		[12]	[15]	[18]	[21]
			double m_nWetOut_In	= m_pMain->m_dCPos[i].x;												//[10]		[13]	[16]	[19]	[22]
			double angle = i;																			//[11]		[14]	[17]	[20]	[23]

			m_nFDCCount_Current++;
			pData[m_nFDCCount_Current] = LOWORD(m_nWetOut * PLC_DATA_REVISION);
			m_nFDCCount_Current++;
			pData[m_nFDCCount_Current] = HIWORD(m_nWetOut * PLC_DATA_REVISION);
			m_nFDCCount_Current++;
			pData[m_nFDCCount_Current] = LOWORD(m_nWetOut_In * PLC_DATA_REVISION);
			m_nFDCCount_Current++;
			pData[m_nFDCCount_Current] = HIWORD(m_nWetOut_In * PLC_DATA_REVISION);
			m_nFDCCount_Current++;
			pData[m_nFDCCount_Current] = LOWORD(angle * PLC_DATA_REVISION);
			m_nFDCCount_Current++;
			pData[m_nFDCCount_Current] = HIWORD(angle * PLC_DATA_REVISION);
		}
	}
	// 
	// 순서 -> Wetout Max , Min , Max Pos , Min Pos
	// 
	double minDistance_Wetout_in = m_pMain->m_ELB_DiffInspResult[datasheet].m_vELB_ReverseRotateCenter.x;		//[33]
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = LOWORD(minDistance_Wetout_in * PLC_DATA_REVISION);
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = HIWORD(minDistance_Wetout_in * PLC_DATA_REVISION);
	double maxDistance_Wetout_in = m_pMain->m_ELB_DiffInspResult[datasheet].m_vELB_ReverseRotateCenter.y;		//[34]
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = LOWORD(maxDistance_Wetout_in * PLC_DATA_REVISION);
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = HIWORD(maxDistance_Wetout_in * PLC_DATA_REVISION);
	double minDistance_Wetout = m_pMain->m_ELB_DiffInspResult[datasheet].m_vELB_RotateCenter.x;					//[35]
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = LOWORD(minDistance_Wetout * PLC_DATA_REVISION);
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = HIWORD(minDistance_Wetout * PLC_DATA_REVISION);
	double maxDistance_Wetout = m_pMain->m_ELB_DiffInspResult[datasheet].m_vELB_RotateCenter.y;					//[36]
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = LOWORD(maxDistance_Wetout * PLC_DATA_REVISION);
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = HIWORD(maxDistance_Wetout * PLC_DATA_REVISION);
	double minDistance_Total = m_pMain->m_ELB_DiffInspResult[datasheet].m_vELB_DistMinMax.x;					//[37]
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = LOWORD(minDistance_Total * PLC_DATA_REVISION);
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = HIWORD(minDistance_Total * PLC_DATA_REVISION);
	double maxDistance_Total = m_pMain->m_ELB_DiffInspResult[datasheet].m_vELB_DistMinMax.y;					//[38]
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = LOWORD(maxDistance_Total * PLC_DATA_REVISION);
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = HIWORD(maxDistance_Total * PLC_DATA_REVISION);

	double minmaxangle[10] = { 0, };																			//[39~44]
	memcpy(minmaxangle, m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_ELB_ResultAngle, sizeof(m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_ELB_ResultAngle));

	// [39]~[44]
	// [39] Min_in , [40] Max_in, [41] Min_Out, [42] Max_Out, [43] Min, [44] Max

	for (int i = 0; i < 6; i++)
	{
		m_nFDCCount_Current++;
		pData[m_nFDCCount_Current] = LOWORD(minmaxangle[i] * PLC_DATA_REVISION);
		m_nFDCCount_Current++;
		pData[m_nFDCCount_Current] = HIWORD(minmaxangle[i] * PLC_DATA_REVISION);
	}

	// [45]~[48]
	// [45] RminResult , [46] RmaxResult , [47] RminAngle , [48] RmaxAngle
	//m_pMain->m_ELB_DiffInspResult[datasheet].m_RminResult = _minVal * xres;
	double RminResult	= m_pMain->m_ELB_DiffInspResult[datasheet].m_RminResult;
	double RminAngle	= m_pMain->m_ELB_DiffInspResult[datasheet].m_RminAngle;
	double RmaxResult	= m_pMain->m_ELB_DiffInspResult[datasheet].m_RmaxResult;
	double RmaxAngle	= m_pMain->m_ELB_DiffInspResult[datasheet].m_RmaxAngle;

	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = LOWORD(RminResult * PLC_DATA_REVISION);
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = HIWORD(RminResult * PLC_DATA_REVISION);
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = LOWORD(RmaxResult * PLC_DATA_REVISION);
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = HIWORD(RmaxResult * PLC_DATA_REVISION);
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = LOWORD(RminAngle * PLC_DATA_REVISION);
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = HIWORD(RminAngle * PLC_DATA_REVISION);
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = LOWORD(RmaxAngle * PLC_DATA_REVISION);
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = HIWORD(RmaxAngle * PLC_DATA_REVISION);

	// [49]~[56]
	//순서 -> 0, 45, 90, 135 Angle의 R Value 값 보고
	//m_pMain->m_ELB_DiffInspResult[datasheet].m_RCheckangle[i] = Diff * xres;

	int n = 4;
	for (int i = 0; i < n; i++)
	{

		double m_dRValue = m_pMain->m_ELB_DiffInspResult[datasheet].m_RCheckangle[i];					//[49]	[51]	[53]	[55]
		double angle = i * 45.0;																		//[50]	[52]	[54]	[56]

		m_nFDCCount_Current++;
		pData[m_nFDCCount_Current] = LOWORD(m_dRValue * PLC_DATA_REVISION);
		m_nFDCCount_Current++;
		pData[m_nFDCCount_Current] = HIWORD(m_dRValue * PLC_DATA_REVISION);
		m_nFDCCount_Current++;
		pData[m_nFDCCount_Current] = LOWORD(angle * PLC_DATA_REVISION);
		m_nFDCCount_Current++;
		pData[m_nFDCCount_Current] = HIWORD(angle * PLC_DATA_REVISION);
	}

	// [57]~[60]
	// [57] BminResult , [58] BmaxResult , [59] BminAngle , [60] BmaxAngle
	//m_pMain->m_ELB_DiffInspResult[datasheet].m_BminResult = _minVal * xres;
	double BminResult	= m_pMain->m_ELB_DiffInspResult[datasheet].m_BminResult;
	double BminAngle	= m_pMain->m_ELB_DiffInspResult[datasheet].m_BminAngle;
	double BmaxResult	= m_pMain->m_ELB_DiffInspResult[datasheet].m_BmaxResult;
	double BmaxAngle	= m_pMain->m_ELB_DiffInspResult[datasheet].m_BmaxAngle;

	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = LOWORD(BminResult * PLC_DATA_REVISION);
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = HIWORD(BminResult * PLC_DATA_REVISION);
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = LOWORD(BmaxResult * PLC_DATA_REVISION);
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = HIWORD(BmaxResult * PLC_DATA_REVISION);
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = LOWORD(BminAngle * PLC_DATA_REVISION);
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = HIWORD(BminAngle * PLC_DATA_REVISION);
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = LOWORD(BmaxAngle * PLC_DATA_REVISION);
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = HIWORD(BmaxAngle * PLC_DATA_REVISION);

	// [61]~[76]
	//순서 -> 0, 45, 90, 135....315 Angle의 B Value 값 보고
	//m_pMain->m_ELB_DiffInspResult[datasheet].m_BCheckangle[i] = Diff * xres;

	if (method == METHOD_CIRCLE)
	{
	n = 8;
	for (int i = 0; i < n; i++)
	{

		double m_dRValue = m_pMain->m_ELB_DiffInspResult[datasheet].m_BCheckangle[i];					//[61]	[63]	[65]	[67]	[69]	[71]	[73]	[75]
		double angle = i * 45.0;																		//[62]	[64]	[66]	[68]	[70]	[72]	[74]	[76]

		m_nFDCCount_Current++;
		pData[m_nFDCCount_Current] = LOWORD(m_dRValue * PLC_DATA_REVISION);
		m_nFDCCount_Current++;
		pData[m_nFDCCount_Current] = HIWORD(m_dRValue * PLC_DATA_REVISION);
		m_nFDCCount_Current++;
		pData[m_nFDCCount_Current] = LOWORD(angle * PLC_DATA_REVISION);
		m_nFDCCount_Current++;
		pData[m_nFDCCount_Current] = HIWORD(angle * PLC_DATA_REVISION);
	}
	}
	else if (method == METHOD_LINE)
	{
		for (int i = 0; i < 8; i++)
		{
			double m_nWetOut = m_pMain->m_dCPos[i].y;												//[9]		[12]	[15]	[18]	[21]
			double m_nWetOut_In = m_pMain->m_dCPos[i].x;												//[10]		[13]	[16]	[19]	[22]
			double angle = i;																			//[11]		[14]	[17]	[20]	[23]

			m_nFDCCount_Current++;
			pData[m_nFDCCount_Current] = LOWORD(m_nWetOut_In * PLC_DATA_REVISION);
			m_nFDCCount_Current++;
			pData[m_nFDCCount_Current] = HIWORD(m_nWetOut_In * PLC_DATA_REVISION);
			m_nFDCCount_Current++;
			pData[m_nFDCCount_Current] = LOWORD(angle * PLC_DATA_REVISION);
			m_nFDCCount_Current++;
			pData[m_nFDCCount_Current] = HIWORD(angle * PLC_DATA_REVISION);
		}
	}

	//KJH 2022-06-28 Lack of CInk Total Count FDC보고 추가
	int m_nLackofCInkTotalAngleCount = m_pMain->m_ELB_DiffInspResult[datasheet].m_nLackOfInkAngleCount_Judge;		//[77]

	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = LOWORD(m_nLackofCInkTotalAngleCount * PLC_DATA_REVISION);
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = HIWORD(m_nLackofCInkTotalAngleCount * PLC_DATA_REVISION);


	// TKYUHA 2022-11-21 Circle Search Error율	FDC보고 추가														[78]
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = LOWORD(int(m_pMain->m_dbCirceErrorLossRate * PLC_DATA_REVISION));
	m_nFDCCount_Current++;
	pData[m_nFDCCount_Current] = HIWORD(int(m_pMain->m_dbCirceErrorLossRate * PLC_DATA_REVISION));
	
	//FDC 정보 전송 Word
	//Stage A1 : 18000 , A2 : 18200, B1 : 18400, B2 : 18600

	//FDC Data 전송
	g_CommPLC.SetWord(m_pMain->vt_job_info[nJob].plc_addr_info.write_word_start + 2880, m_nFDCCount_Current + 1 , pData);

	CString str;

	str.Format("[%s]Send FDC Value - %dWord ", m_pMain->vt_job_info[nJob].job_name.c_str(), m_nFDCCount_Current + 1);
	::SendMessage(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

	return ;
}

#pragma region //20211007 Tkyuha 노즐과 스테이지 갭 측정

bool _ContourAreaSort(const std::vector<cv::Point> c1,	const std::vector<cv::Point> c2)
{
	return cv::contourArea(c1) > cv::contourArea(c2);
}

double CFormMainView::fnCalcNozzleGap_Distance(BYTE* pImage, int w, int h, int nJob, int nCam, int _thresh, CViewerEx* mViewer)
{
	double _rV = 0;

	FakeDC* pDC = NULL;
	CPen penAlign(PS_SOLID, 4, RGB(255, 255, 0));
	CPen penPre(PS_SOLID, 4, RGB(255, 0, 0));

	if (mViewer != NULL)
	{
		mViewer->ClearOverlayDC();
		pDC = mViewer->getOverlayDC();
		pDC->SelectObject(&penPre);
		pDC->SelectObject(GetStockObject(NULL_BRUSH));
	}

	int _sigmaColor = 10;
	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;

	CString strText;
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);

	cv::Mat img(h, w, CV_8UC1, pImage), blurImg, binImg, cropBinImg,inspImg, inspImgCanny;
	cv::Rect r = cv::Rect(w / 2 - 200, h / 2 - 200, 400, 400);

	cv::bilateralFilter(img, blurImg, -1, _sigmaColor, _sigmaColor / 2);
	cv::Canny(blurImg, binImg, 0, 50);
	cv::GaussianBlur(binImg, binImg, cv::Size(0, 0), 3.);
	cv::threshold(binImg, binImg, -1, 255, CV_THRESH_BINARY_INV | THRESH_OTSU);
	cv::erode(binImg, binImg, Mat::ones(cv::Size(3, 3), CV_8UC1), cv::Point(-1, -1), 1);
	cv::dilate(binImg, binImg, Mat::ones(cv::Size(3, 3), CV_8UC1), cv::Point(-1, -1), 1);
	cropBinImg = binImg(r).clone();
	cropBinImg.copyTo(inspImg);

	cv::findContours(cropBinImg, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	std::vector<std::vector<cv::Point>>::const_iterator itc = contours.begin();

	std::sort(contours.begin(), contours.end(), _ContourAreaSort);

#pragma omp parallel for
	for (int i = 0; i < contours.size(); i++)
	{
		cv::Rect box = boundingRect(*itc);
		double ar = contourArea(*itc);
		// // 노이즈 제거 목적 
		if (i >=2|| contourArea(*itc)<10000)
		{
			drawContours(inspImg, contours, i, RGB(0, 0, 0), CV_FILLED, 8, hierarchy);
		}
		++itc;
	}

	// 20211007 Tkyuha 노즐 끝단을 찾아서 표시	
	contours.clear();
	hierarchy.clear();

	cv::Canny(inspImg, inspImgCanny, 100, 200);

	std::map<double, std::vector<cv::Point>> _mLine;
	map<double, std::vector<cv::Point>>::iterator iter;

	vector<Vec4i> linesP;
	cv::HoughLinesP(inspImgCanny, linesP, 1, CV_PI / 180, 30, 0, 10);
	// 유효한 라인을 선택함
	for (size_t i = 0; i < linesP.size(); i++)
	{
		bool bfind = true;
		Vec4i l = linesP[i];
		double angle = atan2(l[3] - l[1], l[2] - l[0]) * 180.0 / CV_PI;

		if ((angle > 5 && angle < 60) || (angle > -60 && angle < -5))
		{
			for (iter = _mLine.begin(); iter != _mLine.end(); iter++) 
			{
				if (iter->first - 5 < angle && iter->first + 5 > angle)
				{
					(*iter).second.push_back(cv::Point(r.x + l[0], r.y + l[1]));
					(*iter).second.push_back(cv::Point(r.x + l[2], r.y + l[3]));
					bfind = false;
					break;
				}
			}

			if (bfind)
			{
				std::vector<cv::Point> _LineList;
				_LineList.push_back(cv::Point(r.x + l[0], r.y + l[1]));
				_LineList.push_back(cv::Point(r.x + l[2], r.y + l[3]));

				_mLine[angle] = _LineList;
			}
		}		
	}
	//if (mViewer != NULL) // 전체 라인을 표시, 디버깅 목적임
	//{
	//	for (iter = _mLine.begin(); iter != _mLine.end(); iter++)
	//	{
	//		std::vector<cv::Point> inVect = (*iter).second;

	//		pDC->MoveTo(CPoint(inVect.at(0).x, inVect.at(0).y));
	//		pDC->LineTo(CPoint(inVect.at(1).x, inVect.at(1).y));
	//	}
	//}

	std::map<double, std::vector<cv::Point>> _mLineSelect;
	map<double, std::vector<cv::Point>>::iterator iterSelect;
	// 여러개의 라인을 한개의 라인으로 Merge 해야함
	for (iter = _mLine.begin(); iter != _mLine.end(); iter++)
	{
		bool bfind = true;
		cv::Vec4f line;
		std::vector<cv::Point> inVect = (*iter).second;
		cv::fitLine(inVect, line, CV_DIST_L2, 0, 0.01, 0.01);

		int x0 = line[2]; // 선에 놓은 한 점
		int y0 = line[3];
		int x1 = x0 - 200 * line[0]; // 200 길이를 갖는 벡터 추가
		int y1 = y0 - 200 * line[1]; // 단위 벡터 사용

		double angle = atan2(y0 - y1, x0 - x1) * 180.0 / CV_PI,_cangle=0;

		_cangle = angle; // 두개가 이루는 각을 선택 하기 위함 45도와 19도가 한쌍(노즐), -7도와 7도가 한쌍(그림자)
		if (angle > 5 && angle < 10) _cangle = -7;
		else if (angle > 35 && angle < 50) _cangle = 19;
		else if (angle > -10 && angle < -5) _cangle = 7;
		else if (angle > 15 && angle < 24) _cangle = 42;

		for (iterSelect = _mLineSelect.begin(); iterSelect != _mLineSelect.end(); iterSelect++)
		{
			if (iterSelect->first - 5 < _cangle && iterSelect->first + 5 > _cangle)
			{
				(*iterSelect).second.push_back(cv::Point(x0, y0));
				(*iterSelect).second.push_back(cv::Point(x1, y1));
				bfind = false;
				break;
			}
		}

		if (bfind)
		{
			std::vector<cv::Point> _LineList;
			_LineList.push_back(cv::Point(x0, y0));
			_LineList.push_back(cv::Point(x1, y1));

			_mLineSelect[angle] = _LineList;
		}
	}

	if (mViewer != NULL)
	{
		double a0, a1, b0, b1;
		double radius = 400;

		std::vector<cv::Vec2f> _finalLine;

		for (iter = _mLineSelect.begin(); iter != _mLineSelect.end(); iter++)
		{
			cv::Vec4f line;
			std::vector<cv::Point> inVect = (*iter).second;
			
			if (inVect.size() >= 4)
			{
				pDC->SelectObject(&penAlign); // 선택된 직선 그리기
				pDC->MoveTo(CPoint(inVect.at(0).x, inVect.at(0).y));
				pDC->LineTo(CPoint(inVect.at(1).x, inVect.at(1).y));
				pDC->MoveTo(CPoint(inVect.at(2).x, inVect.at(2).y));
				pDC->LineTo(CPoint(inVect.at(3).x, inVect.at(3).y));

				cv::Point _inPt = IntersectionPoint1(&inVect.at(0), &inVect.at(1),&inVect.at(2), &inVect.at(3));
				pDC->Ellipse(int(_inPt.x - 4), int(_inPt.y - 4), int(_inPt.x + 4), int(_inPt.y + 4));

				GetLineCoef(inVect.at(0).x, inVect.at(0).y, inVect.at(1).x, inVect.at(1).y, a0, b0);
				GetLineCoef(inVect.at(2).x, inVect.at(2).y, inVect.at(3).x, inVect.at(3).y, a1, b1);
				
				double r0 = atan(a0);
				double r1 = atan(a1);

				r0 += CV_PI;
				r1 += CV_PI;

				double x0 = cos(r0) * radius + _inPt.x;
				double y0 = sin(r0) * radius + _inPt.y;
				double x1 = cos(r1) * radius + _inPt.x;
				double y1 = sin(r1) * radius + _inPt.y;

				int x2 = (x0 + x1) / 2;
				int y2 = (y0 + y1) / 2;

				pDC->SelectObject(&penPre);// 중앙선 그리기
				pDC->MoveTo(CPoint(_inPt.x, _inPt.y));
				pDC->LineTo(CPoint(x2, y2));

				// 중앙에서 그림자와 노즐 사이 거리 측정 하기
				GetLineCoef(_inPt.x, _inPt.y, x2, y2, a0, b0);
				_finalLine.push_back(cv::Vec2f(a0, b0));
			}
		}

		if (_finalLine.size() >= 2)
		{
			a1 = a0 = w / 2.;
			b0 = _finalLine.at(0)[0] * w / 2. + _finalLine.at(0)[1];
			b1 = _finalLine.at(1)[0] * w / 2. + _finalLine.at(1)[1];

			pDC->SetTextColor(RGB(255, 0, 0));
			strText.Format("H: %.4fmm", (b0 - b1) * yres);
			pDC->TextOutA(a0 - 20, h/2-50, strText);
		}

		_finalLine.clear();
		mViewer->Invalidate();
	}

	_mLine.clear();
	_mLineSelect.clear();
	linesP.clear();	

	penAlign.DeleteObject();
	penPre.DeleteObject();

	img.release();
	blurImg.release();
	binImg.release();
	cropBinImg.release();
	inspImg.release();
	inspImgCanny.release();

	return _rV;
}
#pragma endregion

#pragma region //20211124 Tkyuha 노즐과 클리닝 상태 검사
double CFormMainView::fnCalcNozzleClean_Insp(BYTE* pImage, int w, int h, int nJob, int nCam, int _thresh, CViewerEx* mViewer)
{
	double _rV = 0;

	FakeDC* pDC = NULL;
	CPen penOk(PS_SOLID, 4, RGB(255, 255, 0));
	CPen penNg(PS_SOLID, 4, RGB(255, 0, 0));

	if (mViewer != NULL)
	{
		mViewer->ClearOverlayDC();
		pDC = mViewer->getOverlayDC();
		pDC->SelectObject(&penOk);
		pDC->SelectObject(GetStockObject(NULL_BRUSH));
	}

	CString strText;
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);

	cv::Mat img(h, w, CV_8UC1, pImage), binImg, cropBinImg;
	cv::Rect r = cv::Rect(w / 2 - 200, h / 2 - 200, 400, 400);

	cv::GaussianBlur(img, binImg, cv::Size(0, 0), 3.);
	cv::threshold(binImg, binImg, 60, 255, CV_THRESH_BINARY_INV);  // 어두운게 있으면 무조건 검사 진행
	cv::erode(binImg, binImg, Mat::ones(cv::Size(3, 3), CV_8UC1), cv::Point(-1, -1), 1);
	cv::dilate(binImg, binImg, Mat::ones(cv::Size(3, 3), CV_8UC1), cv::Point(-1, -1), 1);
	cropBinImg = binImg(r).clone();

	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;

	cv::findContours(cropBinImg, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	std::vector<std::vector<cv::Point>>::const_iterator itc = contours.begin();

	for (int i = 0; i < contours.size(); i++)
	{
		cv::Rect box = boundingRect(*itc);

		if (contourArea(*itc) > 100)
		{
			pDC->SelectObject(&penNg);
			_rV++;
		}
		else pDC->SelectObject(&penOk);

		if (mViewer != NULL)
		{
			pDC->Rectangle(int(box.x), int(box.y), int(box.x + box.width), int(box.y + box.height));
		}
		++itc;
	}

	contours.clear();
	hierarchy.clear();

	img.release();
	binImg.release();
	cropBinImg.release();

	penOk.DeleteObject();
	penNg.DeleteObject();

	return _rV;
}
#pragma endregion

#pragma region //20211124 Tkyuha 측면 카메라를 이용한 도포 검사
double CFormMainView::fnCalcNozzleDopo_Insp(BYTE * pImage, int w, int h, int nJob, int nCam, int _thresh, CViewerEx * mViewer)
{
	double _rV = 0;

	FakeDC* pDC = NULL;
	CPen penOk(PS_SOLID, 4, RGB(255, 255, 0));
	CPen penNg(PS_SOLID, 4, RGB(255, 0, 0));

	if (mViewer != NULL)
	{
		mViewer->ClearOverlayDC();
		pDC = mViewer->getOverlayDC();
		pDC->SelectObject(&penOk);
		pDC->SelectObject(GetStockObject(NULL_BRUSH));
	}

	CString strText;
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);

	cv::Mat img(h, w, CV_8UC1, pImage), binImg, cropBinImg;
	cv::Rect r = cv::Rect(w / 2 - 200, h / 2 - 200, 400, 400);

	cv::GaussianBlur(img, binImg, cv::Size(0, 0), 3.);
	cv::threshold(binImg, binImg, 150, 255, CV_THRESH_BINARY);  // 밝은게 있으면 무조건 NG
	cv::erode(binImg, binImg, Mat::ones(cv::Size(3, 3), CV_8UC1), cv::Point(-1, -1), 1);
	cv::dilate(binImg, binImg, Mat::ones(cv::Size(3, 3), CV_8UC1), cv::Point(-1, -1), 1);
	cropBinImg = binImg(r).clone();

	std::vector<std::vector<cv::Point>> contours;
	std::vector<cv::Vec4i> hierarchy;

	cv::findContours(cropBinImg, contours, hierarchy, CV_RETR_CCOMP, CV_CHAIN_APPROX_SIMPLE);
	std::vector<std::vector<cv::Point>>::const_iterator itc = contours.begin();

	for (int i = 0; i < contours.size(); i++)
	{
		cv::Rect box = boundingRect(*itc);

		if (contourArea(*itc) > 100)
		{
			pDC->SelectObject(&penNg);
			_rV++;
		}
		else pDC->SelectObject(&penOk);

		if (mViewer != NULL)
		{
			pDC->Rectangle(int(box.x), int(box.y), int(box.x + box.width), int(box.y + box.height));
		}
		++itc;
	}

	contours.clear();
	hierarchy.clear();

	img.release();
	binImg.release();
	cropBinImg.release();

	penOk.DeleteObject();
	penNg.DeleteObject();
	return _rV;
}
#pragma endregion
// Tkyuha 211112 에러 메시지 표시
void CFormMainView::ShowErrorMessageBlink(CString str)
{
	CRect rect;

	m_pMain->m_dlgMsg->GetClientRect(&rect);

	int width	= GetSystemMetrics(SM_CXSCREEN);
	int height	= GetSystemMetrics(SM_CYSCREEN);
	int left	= width / 2 - rect.Width() / 2;
	int top		= height / 2 - rect.Height() / 2;

	m_pMain->m_dlgMsg->KillTimer(100);
	m_pMain->m_dlgMsg->m_sMessage = str;
	m_pMain->m_dlgMsg->SetTimer(100, 500, NULL);
	m_pMain->m_dlgMsg->SetWindowPos(NULL, left, top, rect.Width(), rect.Height(), SWP_HIDEWINDOW);
	m_pMain->m_dlgMsg->ShowWindow(SW_SHOW);
}

void CFormMainView::HideErrorMessageBlink()
{
	m_pMain->m_dlgMsg->KillTimer(100);
	m_pMain->m_dlgMsg->ShowWindow(SW_HIDE);
}

void CFormMainView::OnMButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	CFormView::OnMButtonDblClk(nFlags, point);
}

BOOL CFormMainView::IsDoubleClickModelId(CPoint pt)
{
	if (PtInRect(&m_rtModelId, pt))	return TRUE;
	else							return FALSE;
}

void CFormMainView::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	// TODO: Add your message handler code here and/or call default

	// auto 상태에서 더블클릭 할 경우 auto sequence 진입
	if (IsDoubleClickModelId(point) && m_pMain->m_bAutoStart)
	{
		int sel = m_TabMainView.GetCurSel();
		PostMessage(WM_VIEW_CONTROL, MSG_FMV_AUTO_SEQ_TEST, sel);
	}

	CFormView::OnLButtonDblClk(nFlags, point);
}

BOOL CFormMainView::CheckStatus(int nJob,int _w,int _h,CDC * pDC)
{
	BOOL _check = TRUE;
	CString str;

	int _TextGap = 0;
	//Metal 검사 기능
	//카메라 중심에서 MP Circle Center Pos 비교(x축비교) 
	//MP 중점비교검사
	double spec = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspHoleMetal();
	bool bCircleMetalCenterCheck = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getCircleMetalCenterCheck();
	if (bCircleMetalCenterCheck && m_dCircleDistance_X_MP > spec)
	{
		_check = FALSE;
		str.Format("[%s] Metal Circle Center Gap Dist NG (%.3f) - %.3fmm", get_job_name(nJob), spec, m_dCircleDistance_X_MP);
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		_TextGap = _TextGap + 100;
		if(pDC!=NULL) pDC->TextOutA(50, _TextGap, str, str.GetLength());
		ShowErrorMessageBlink(str);
		m_pMain->write_NGresult_SummaryFile(m_pMain->vt_job_info[nJob].main_object_id.c_str(), str, FALSE, nJob);
	}

	//PN Circle Center랑 MP Circle Center
	bool bCircleMetalRadiusCheck = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getCircleMetalRadiusCheck();
	CPointF<int> p_circle_center_PN(m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x, m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y);

	for (int angle = 0; angle < 360; angle += 45)
	{
		int _x = int(cos(angle * CV_PI / 180.) * m_dCircleRadius_MP + (m_ptCircleCenter_MP.x));
		int _y = int(sin(angle * CV_PI / 180.) * m_dCircleRadius_MP + (m_ptCircleCenter_MP.y));

		CPointF<int> p_circle_MP(_x, _y);
		double _vs = GetDistance(p_circle_center_PN, p_circle_MP);

		if (bCircleMetalRadiusCheck && _vs < CIRCLE_RADIUS)
		{
			_check = FALSE;
			str.Format("[%s] Circle Trace PN <-> MP Gap NG - Gap : %.3fmm(T : %d) , R(PN) : %.3fmm, ", get_job_name(nJob), _vs, angle, CIRCLE_RADIUS);
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
			_TextGap = _TextGap + 100;
			if (pDC != NULL) pDC->TextOutA(50, _TextGap, str, str.GetLength());
			ShowErrorMessageBlink(str);
			m_pMain->write_NGresult_SummaryFile(m_pMain->vt_job_info[nJob].main_object_id.c_str(), str, FALSE, nJob);
			break;
		}
	}

	double brigthness = 50;
	double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(0, 0);
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);
	spec = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(0) / xres;
	bool bCenterDistanceEnable = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getCenterDistanceEnable();
	if (bCenterDistanceEnable && (fabs(_w / 2 - m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x) > spec || fabs(_h / 2 - m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y) > spec))
	{
		_check = FALSE;
		str.Format("[%s] PN Center Error - %.3fmm , %.3fmm", m_pMain->vt_job_info[nJob].job_name.c_str(), fabs(_w / 2 - m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x) * xres,
			fabs(_h / 2 - m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y) * xres);
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		_TextGap = _TextGap + 100;
		if (pDC != NULL) pDC->TextOutA(50, _TextGap, str, str.GetLength());
		ShowErrorMessageBlink(str);
		m_pMain->write_NGresult_SummaryFile(m_pMain->vt_job_info[nJob].main_object_id.c_str(), str, FALSE, nJob);
	}

	//KJH 2022-04-09 PN 반지름 비교 검사 추가
	bool bPNSizeInspEnable	= m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getPNSizeInspEnable();
	double PNSizespec		= m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getPNSizeSpec() / xres;
	double PNSizespectorr	= m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getPNSizeSpecTorr() / xres;

	double PNSizeSpecMin = PNSizespec - PNSizespectorr;
	double PNSizeSpecMax = PNSizespec + PNSizespectorr;;

	if (bPNSizeInspEnable && (m_pMain->m_ELB_TraceResult.m_nRadius > PNSizeSpecMax || m_pMain->m_ELB_TraceResult.m_nRadius < PNSizeSpecMin))
	{
		_check = FALSE;
		str.Format("[%s] PN Size Error - %.3fmm / [Spec] - %.3fmm", m_pMain->vt_job_info[nJob].job_name.c_str(), m_pMain->m_ELB_TraceResult.m_nRadius * xres, PNSizespec * xres);
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		_TextGap = _TextGap + 100;
		if (pDC != NULL) pDC->TextOutA(50, _TextGap, str, str.GetLength());
		ShowErrorMessageBlink(str);
		m_pMain->write_NGresult_SummaryFile(m_pMain->vt_job_info[nJob].main_object_id.c_str(), str, FALSE, nJob);
	}

	///////////////////////////////////
	// 20210924 Tkyuha 예외 처리 추가
	// 원의 반지름 하고 실제 노즐의 위치를 비교
	// Trace에서 찾은 위치하고 얼라인 중심점 합한 거리가 반지름 보다 큰 경우 알람 발생
	//모니터링후 적용 할것 
	double _dist = fabs((_h / 2 - m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y) * yres + fabs(m_pMain->m_ELB_TraceResult.m_vELB_RotateCenter.y)
		- m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspCamHoleSize());
	bool bNozzleToHoleEdgeEnable = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getNozzleToHoleEdgeEnable();

	if (bNozzleToHoleEdgeEnable && _dist > spec)
	{
		_check = FALSE;
		str.Format("[%s] Circle Center Pos Gap NG (Spec : %.3f) - X : %.3fmm , Y : %.3fmm", get_job_name(nJob), spec, fabs(_w / 2 - m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.x) * xres,
			fabs(_h / 2 - m_pMain->m_ELB_TraceResult.m_ELB_ResultXY.y) * yres);
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		_TextGap = _TextGap + 100;
		if (pDC != NULL) pDC->TextOutA(50, _TextGap, str, str.GetLength());
		m_pMain->write_NGresult_SummaryFile(m_pMain->vt_job_info[nJob].main_object_id.c_str(), str, FALSE, nJob);
	}

	//도포 유무 판별
	int cBright = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInkCheckThresh(0);
	bool bDispensingStatusEnable = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getDispensingStatusEnable();

	if (bDispensingStatusEnable && m_dCircleInBright < cBright)
	{
		_check = FALSE;
		str.Format("[%s] Circle Brightness NG(Spec : 60) - %.2f gray", get_job_name(nJob), m_dCircleInBright);
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		_TextGap = _TextGap + 100;
		if (pDC != NULL) pDC->TextOutA(50, _TextGap, str, str.GetLength());
		ShowErrorMessageBlink(str);
		m_pMain->write_NGresult_SummaryFile(m_pMain->vt_job_info[nJob].main_object_id.c_str(), str, FALSE, nJob);
	}

	cBright = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getLamiCheckThresh(0);
	bool bLamiStatusEnable = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getLamiStatusEnable();
	for (int a = 0; a < 4; a++)
	{
		if (bLamiStatusEnable && m_dCircleInBrightDir[a] < cBright)
		{
			_check = FALSE;
			str.Format("[%s] Miss Lami Error - %.3f gray", m_pMain->vt_job_info[nJob].job_name.c_str(), m_dCircleInBrightDir[a]);
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
			_TextGap = _TextGap + 100;
			if (pDC != NULL) pDC->TextOutA(50, _TextGap, str, str.GetLength());
			ShowErrorMessageBlink(str);
			m_pMain->write_NGresult_SummaryFile(m_pMain->vt_job_info[nJob].main_object_id.c_str(), str, FALSE, nJob);
			break;
		}
	}

	// 좌 , 우, 상, 하 길이 비교
	//2021-12-25 -_-??
	bool bCenterCCDistanceEnable = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getCenterCCDistanceEnable();
	if (bCenterCCDistanceEnable && m_leftRight.x != 0 && m_leftRight.y != 0)
	{
		spec = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(0);

		double ShiftLeft = (m_leftRight.x - (_w / 2 - CIRCLE_RADIUS / xres)) * xres;
		double ShiftRight = ((_w / 2 + CIRCLE_RADIUS / xres) - m_leftRight.y) * yres;
		double ShiftTop = (m_topBottom.x - (_h / 2 - CIRCLE_RADIUS / xres)) * xres;
		double ShiftBottom = ((_h / 2 + CIRCLE_RADIUS / xres) - m_topBottom.y) * yres;

		if (ShiftLeft < spec || ShiftRight < spec || ShiftTop < spec || ShiftBottom < spec)
		{
			_check = FALSE;
			str.Format("[%s] CC Circle Shift Insp NG(Spec : %.3f) - L : %.3fmm , R : %.3fmm, T : %.3fmm , B : %.3fmm", get_job_name(nJob), spec,
				ShiftLeft, ShiftRight, ShiftTop, ShiftBottom);
			::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
			_TextGap = _TextGap + 100;
			if (pDC != NULL) pDC->TextOutA(50, _TextGap, str, str.GetLength());
			ShowErrorMessageBlink(str);
			m_pMain->write_NGresult_SummaryFile(m_pMain->vt_job_info[nJob].main_object_id.c_str(), str, FALSE, nJob);
		}
	}
	return _check;
}

void CFormMainView::fnCalcManualCenterAlign(int nJob)
{
	int method = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod();
	int nCam = 0, nPos = 0;
	double worldX, worldY;
	double posX = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetXPos();
	double posY = m_pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetYPos();

	// 일단은 같은 좌표계에서 불러 오도록
	m_pMain->GetMachine(nJob).PixelToWorld(nCam, nCam, posX, posY, &worldX, &worldY);
	m_pMain->GetPrealign(nJob).setPosX(nPos, worldX);
	m_pMain->GetPrealign(nJob).setPosY(nPos, worldY);

	if (method == METHOD_LINE)
	{
		m_pMain->m_dbRevisionData[nJob][AXIS_X] = worldX;
		m_pMain->m_dbRevisionData[nJob][AXIS_Y] = worldY;
	}
	else
	{
		m_pMain->m_dbRevisionData[nJob][AXIS_X] = worldX;
		m_pMain->m_dbRevisionData[nJob][AXIS_Y] = worldY;
	}

	switch (method)
	{
	case METHOD_LINE:			// Line
	{
		m_pMain->m_dbRevisionData[nJob][AXIS_X] = 0;
		m_pMain->m_dbRevisionData[nJob][AXIS_T] = 0;
		if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getRevisionReverseY())
			m_pMain->m_dbRevisionData[nJob][AXIS_Y] *= -1; // Y축 부호 반전					
	}
	break;
	case METHOD_CIRCLE:			// Circle
	{
		m_pMain->m_dbRevisionData[nJob][AXIS_T] = 0;
		if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getRevisionReverseY())
			m_pMain->m_dbRevisionData[nJob][AXIS_Y] *= -1; // Y축 부호 반전			
		m_pMain->m_dbRevisionData[nJob][AXIS_Y] += m_pMain->GetPrealign(nJob).getRevisionOffset(1);
		if (m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getRevisionReverseX())
			m_pMain->m_dbRevisionData[nJob][AXIS_X] *= -1; // Y축 부호 반전	
		m_pMain->m_dbRevisionData[nJob][AXIS_X] += m_pMain->GetPrealign(nJob).getRevisionOffset(0);
	}
	break;
	}

	m_pMain->sendRevisionData(m_pMain->m_dbRevisionData[nJob][AXIS_X], m_pMain->m_dbRevisionData[nJob][AXIS_Y],
		m_pMain->m_dbRevisionData[nJob][AXIS_T], nJob);
}

void CFormMainView::fnCalcNozzleGapInsp(int nJob)
{
	CString str;

	bool bfind = true;
	std::vector<int> camBuf = m_pMain->vt_job_info[nJob].camera_index;
	int camCount = m_pMain->vt_job_info[nJob].num_of_camera;
	int nCam = 0;
	int real_cam = camBuf.at(nCam);
	int viewer = m_pMain->vt_job_info[nJob].viewer_index[nCam];
	double m_dCGThickness = 0.0;

#pragma region Camera Grab & Matching
	//  0번 카메라 Grab
#ifdef _DAHUHA
	AcquireSRWLockExclusive(&m_pMain->m_pDahuhaCam[real_cam].g_bufsrwlock); // 동기화 시킴 Tkyuha 20211027
#else
	AcquireSRWLockExclusive(&m_pMain->m_pBaslerCam[real_cam]->g_bufsrwlock); // 동기화 시킴 Tkyuha 20211027
#endif
	m_pMain->m_bGrabEnd[real_cam] = FALSE;
#ifdef _DAHUHA
	ReleaseSRWLockExclusive(&m_pMain->m_pDahuhaCam[real_cam].g_bufsrwlock);
#else
	ReleaseSRWLockExclusive(&m_pMain->m_pBaslerCam[real_cam]->g_bufsrwlock);
#endif
	::SendMessage(m_pMain->m_hWnd, WM_VIEW_CONTROL, MSG_MD_CAMERA_SINGLE_FRAME, real_cam);

	//  0번 카메라 Matching
	int W = m_pMain->m_stCamInfo[real_cam].w;
	int H = m_pMain->m_stCamInfo[real_cam].h;

	memcpy(m_pMain->getProcBuffer(real_cam, 0), m_pMain->getSrcBuffer(real_cam), W * H);

	bool _bCenterNozzleAlignMethod = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getCenterNozzleAlignMethod();
	if (_bCenterNozzleAlignMethod) // 테스트중
	{
		cv::Point2f matchPt = cv::Point2f(-1, -1);
		bfind = InspNozzleSearchCaliper_Use(m_pMain->getProcBuffer(real_cam, 0), W, H, nJob, real_cam, matchPt,true);
		if (bfind)
		{
			m_pMain->GetMatching(nJob).getFindInfo(nCam, 0).SetXPos(matchPt.x);
			m_pMain->GetMatching(nJob).getFindInfo(nCam, 0).SetYPos(matchPt.y);
		}
	}
	else
	{
		m_pMain->GetMatching(nJob).findPattern(m_pMain->getProcBuffer(real_cam, 0), nCam, 0, W, H); // 4번 위치에 기준 마크 등록
		m_pDlgViewerMain[viewer]->GetViewer().OnLoadImageFromPtr(m_pMain->getProcBuffer(real_cam, 0));

		if (m_pMain->GetMatching(nJob).getFindInfo(nCam, 0).GetFound() != FIND_OK)  bfind = false;
	}

	//Tkyuha 2022-06-17 Nozzle XY 검사중 다시 한번 Y만 정밀 Search , 검증후 적용 예정

	bool _recalc = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getCenterNozzleYAlignRecalc();
	if (_recalc)
	{
		double posX = m_pMain->GetMatching(nJob).getFindInfo(nCam, 0).GetXPos();
		double posY = m_pMain->GetMatching(nJob).getFindInfo(nCam, 0).GetYPos();
		// KMB 2022-09-16
		//double rv = m_pMain->reCalculateMarkZGapYpos2(m_pMain->getProcBuffer(real_cam, 0), W, H, posX, posY);
		//if (rv > 0 && fabs(rv - posY) > 2.0)  m_pMain->GetMatching(nJob).getFindInfo(nCam, 0).SetYPos(rv);		
		
		// KBJ 2022-12-06 Z-Gap Recalc 개선 
		m_pMain->reCalculateMarkZGapYpos3(nJob, m_pMain->getProcBuffer(real_cam, 0), W, H, &posX, &posY);
		m_pMain->GetMatching(nJob).getFindInfo(nCam, 0).SetXPos(posX);
		m_pMain->GetMatching(nJob).getFindInfo(nCam, 0).SetYPos(posY);
	}

#pragma endregion

	double FindMark_X = m_pMain->GetMatching(nJob).getFindInfo(nCam, 0).GetXPos();
	double FindMark_Y = m_pMain->GetMatching(nJob).getFindInfo(nCam, 0).GetYPos();
	double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(nCam, 0);
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(nCam, 0);
	double _offset = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getNozzleToStageGapOffset();

	BOOL bFindLine = FALSE;
	sLine line_info;
	double dx = 0.0, dy = 0.0, dt = 0.0;
	double centerPtX = W / 2, cetnerPtY = H / 2;

#pragma region  Find Stage Edge & Calc Distance
	m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][C_CALIPER_POS_1].processCaliper(m_pMain->getProcBuffer(real_cam, 0), W, H, dx, dy, dt, TRUE);
	line_info = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][C_CALIPER_POS_1].m_lineInfo;
	bFindLine = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][C_CALIPER_POS_1].getIsMakeLine();

	// KBJ 2022-08-18 ZGap Caliper 미사용
	//if (bFindLine)
	if (0)
	{
		sPoint _point, point_on_line;
		_point.x = FindMark_X;
		_point.y = FindMark_Y;
		m_pMain->GetMatching(nJob).compute_orthogonal_point_on_line(line_info, _point, point_on_line);
		centerPtX = point_on_line.x, cetnerPtY = point_on_line.y;
	}

	m_pMain->vt_result_info[nJob].revision_data[nCam][0] = 0.0;
	m_pMain->vt_result_info[nJob].revision_data[nCam][1] = 0.0;
	m_pMain->vt_result_info[nJob].revision_data[nCam][2] = 0.0;

	m_pMain->m_dbRevisionData[nJob][AXIS_X] = 0;
	m_pMain->m_dbRevisionData[nJob][AXIS_Y] = 0;
	m_pMain->m_dbRevisionData[nJob][AXIS_T] = 0;
	m_pMain->m_dbRevisionData[nJob][AXIS_Z] = fabs(cetnerPtY - FindMark_Y) * yres + _offset;

	m_pMain->GetPrealign(nJob).setRevisionX(m_pMain->m_dbRevisionData[nJob][AXIS_X]);
	m_pMain->GetPrealign(nJob).setRevisionY(m_pMain->m_dbRevisionData[nJob][AXIS_Y]);
	m_pMain->GetPrealign(nJob).setRevisionT(m_pMain->m_dbRevisionData[nJob][AXIS_T]);
	m_pMain->GetPrealign(nJob).setRevisionZ(m_pMain->m_dbRevisionData[nJob][AXIS_Z]);

	m_pMain->sendRevisionNozzleData(m_pMain->m_dbRevisionData[nJob][AXIS_X], m_pMain->m_dbRevisionData[nJob][AXIS_Y], m_pMain->m_dbRevisionData[nJob][AXIS_T], m_pMain->m_dbRevisionData[nJob][AXIS_Z], nJob);

	double xdist = fabs(centerPtX - FindMark_X) * xres;
	double zdist = fabs(cetnerPtY - FindMark_Y) * yres + _offset;
#pragma endregion

#pragma region Judge & Display
	
	//  Result Judge
	// YCS 2022-11-30 Z GAP 판정 변수 위치 변경
	BOOL m_bJudge = FALSE;
	if (!bfind)
	{
		g_CommPLC.SetBit(m_pMain->m_nAlignNgWrite[nJob] + 4, TRUE);
		m_pDlgViewerMain[viewer]->setViewerJudge(FALSE);
	}
	else
	{
		//KJH 2022-05-19 CG Thickness 받아서 Z Gap 판정하기
		long lData[2] = { 0, };

		g_CommPLC.GetWord(m_pMain->vt_job_info[nJob].plc_addr_info.read_word_start + 10, 2, lData);

		m_dCGThickness = MAKELONG(lData[0], lData[1]);

		m_dCGThickness /= 10000.0;

		double Z_maxspec = m_dCGThickness + m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(7);
		double Z_minspec = m_dCGThickness + m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(7);

		if (zdist <= Z_maxspec && zdist >= Z_minspec)
		{
			g_CommPLC.SetBit(m_pMain->m_nAlignOkWrite[nJob] + 4, TRUE);
			m_pDlgViewerMain[viewer]->setViewerJudge(TRUE);

			m_bJudge = TRUE;
		}
		else
		{
			g_CommPLC.SetBit(m_pMain->m_nAlignNgWrite[nJob] + 4, TRUE);
			m_pDlgViewerMain[viewer]->setViewerJudge(FALSE);
			//str.Format("[%s] Nozzle Gap X,Z(%3.4f,%3.4f)um NG", m_pMain->vt_job_info[nJob].job_name.c_str(), xdist, zdist);
			str.Format("[%s] Nozzle Gap Z : %3.4f(%3.4f - %3.4f)um NG", m_pMain->vt_job_info[nJob].job_name.c_str(), zdist, Z_minspec, Z_maxspec);
			ShowErrorMessageBlink(str);

			m_bJudge = FALSE;
		}
		// PJH 2022-09-05 JUDGE 판정 추가
		::SendMessageA(theApp.m_pFrame->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_JUDGE_JOB, MAKELPARAM(m_bJudge, nJob));

		FakeDC* pDC = m_pDlgViewerMain[viewer]->GetViewer().getOverlayDC();
		m_pDlgViewerMain[viewer]->GetViewer().ClearOverlayDC();
		m_pDlgViewerMain[viewer]->GetViewer().clearAllFigures();

		CPen penROI(PS_SOLID, 10, RGB(0, 255, 0));
		CPen penBase(PS_SOLID, 3, RGB(255, 0, 0));
		CPen penStage(PS_SOLID, 3, RGB(255, 255, 0));

		CFont font, * oldFont;
		int OLD_BKMODE = 0;

		font.CreateFont(30, 30, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
			OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
		oldFont = pDC->SelectObject(&font);
		OLD_BKMODE = pDC->SetBkMode(TRANSPARENT);
		pDC->SelectObject(&penROI);
		(CBrush*)pDC->SelectStockObject(NULL_BRUSH);

		pDC->SetTextColor(RGB(255, 0, 0));
		
		// KBJ 2022-10-13 Z-Gap 글자 표시 수정
		struct stFigureText figure;
		CString str_temp;

		str.Format("Target : %3.4fum", m_dCGThickness);
		figure.ptBegin.x = centerPtX;
		figure.ptBegin.y = 50;
		figure.ptBeginFit = figure.ptBegin;
		figure.textString = str;
		figure.is_show_bkg = TRUE;

		if (m_bJudge)	m_pDlgViewerMain[viewer]->GetViewer().addFigureText(figure, 12, 20, COLOR_GREEN);
		else			m_pDlgViewerMain[viewer]->GetViewer().addFigureText(figure, 12, 20, COLOR_RED);
		//pDC->TextOutA(50, 50, str);

		str_temp.Format("Z : %.4fum( Orignal: %.4fum )", zdist, zdist - _offset);
		figure.ptBegin.x = centerPtX;
		figure.ptBegin.y = 150;
		figure.ptBeginFit = figure.ptBegin;
		figure.textString = str_temp;
		figure.is_show_bkg = TRUE;

		if (m_bJudge)	m_pDlgViewerMain[viewer]->GetViewer().addFigureText(figure, 12, 20, COLOR_GREEN);
		else			m_pDlgViewerMain[viewer]->GetViewer().addFigureText(figure, 12, 20, COLOR_RED);

		m_pMain->vt_result_info[nJob].revision_data[nCam][0] = FindMark_X;
		m_pMain->vt_result_info[nJob].revision_data[nCam][1] = FindMark_Y;
		m_pMain->vt_result_info[nJob].revision_data[nCam][2] = zdist;

		// KBJ 2022-07-12 Gap Line 너무 두꺼운거 같아서 PDC->Viewer로 그리기 변경
		stFigure tempFig;

		/////// Draw Pattern Find Pos
		int fatW = m_pMain->GetMatching(nJob).getHalcon(0, 0, 0).getFatWidth();
		int fatH = m_pMain->GetMatching(nJob).getHalcon(0, 0, 0).getFatHeight();
		double offsetX = m_pMain->GetMatching(nJob).getPosOffsetX(0, 0, 0);
		double offsetY = m_pMain->GetMatching(nJob).getPosOffsetY(0, 0, 0);
		int oriX = (int)(FindMark_X - offsetX);
		int oriY = (int)(FindMark_Y - offsetY);

		// 마크 그리기
		tempFig.ptBegin.x = oriX - fatW / 2;
		tempFig.ptBegin.y = oriY + fatH / 2;
		tempFig.ptEnd.x = oriX + fatW / 2;
		tempFig.ptEnd.y = oriY + fatH / 2;
		m_pDlgViewerMain[viewer]->GetViewer().addFigureLine(tempFig, 2, 1, COLOR_GREEN);

		tempFig.ptBegin.x = oriX - fatW / 2;
		tempFig.ptBegin.y = oriY + fatH / 2;
		tempFig.ptEnd.x = oriX - fatW / 2;
		tempFig.ptEnd.y = oriY - fatH / 2;
		m_pDlgViewerMain[viewer]->GetViewer().addFigureLine(tempFig, 2, 1, COLOR_GREEN);

		tempFig.ptBegin.x = oriX + fatW / 2;
		tempFig.ptBegin.y = oriY - fatH / 2;
		tempFig.ptEnd.x = oriX + fatW / 2;
		tempFig.ptEnd.y = oriY + fatH / 2;
		m_pDlgViewerMain[viewer]->GetViewer().addFigureLine(tempFig, 2, 1, COLOR_GREEN);

		tempFig.ptBegin.x = oriX + fatW / 2;
		tempFig.ptBegin.y = oriY - fatH / 2;
		tempFig.ptEnd.x = oriX - fatW / 2;
		tempFig.ptEnd.y = oriY - fatH / 2;
		m_pDlgViewerMain[viewer]->GetViewer().addFigureLine(tempFig, 2, 1, COLOR_GREEN);
		m_pMain->draw_mark(&m_pDlgViewerMain[viewer]->GetViewer(), COLOR_GREEN, FindMark_X, FindMark_Y, 2, 1, 20);

		/////// Draw Base Line
		double ylineoffset = _offset / yres;
		tempFig.ptBegin.x = 0;
		tempFig.ptBegin.y = cetnerPtY + ylineoffset;
		tempFig.ptEnd.x = W;
		tempFig.ptEnd.y = cetnerPtY + ylineoffset;
		m_pDlgViewerMain[viewer]->GetViewer().addFigureLine(tempFig, 2, 2, COLOR_RED);

		/////// Draw Caliper Line
		// KBJ 2022-08-18 ZGap Caliper 미사용
		//if (bFindLine)
		if(0)
		{
			m_pMain->draw_line(&m_pDlgViewerMain[viewer]->GetViewer(), COLOR_YELLOW, line_info, H, W);
		}
			 
		//pDC->SelectObject(&penBase);
		//pDC->MoveTo((int)(0), (int)(cetnerPtY + ylineoffset));
		//pDC->LineTo((int)(W - 1), (int)(cetnerPtY + ylineoffset));
		
		/////// Draw Pattern Find Pos
		//pDC->MoveTo((int)(FindMark_X - 20), (int)(FindMark_Y));
		//pDC->MoveTo((int)(FindMark_X), (int)(FindMark_Y - 20));
		//pDC->LineTo((int)(FindMark_X), (int)(FindMark_Y + 20));

		//double ylineoffset = _offset / yres;
		//pDC->SelectObject(&penBase);
		//pDC->MoveTo((int)(0), (int)(cetnerPtY + ylineoffset));
		//pDC->LineTo((int)(W - 1), (int)(cetnerPtY + ylineoffset));

		//if (bFindLine)
		//{
		//	pDC->SelectObject(&penStage);
		//	pDC->MoveTo((int)(0), int(line_info.b));
		//	pDC->LineTo((int)(W - 1), int(line_info.a * (W - 1) + line_info.b));
		//}

		str.Format("[GAP INSP] [X:%4.1f, Z:%4.1f] => [X:%4.1f, Z:%4.1f]", centerPtX, cetnerPtY, FindMark_X, FindMark_Y);
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		m_pDlgViewerMain[viewer]->GetViewer().SetEnableDrawAllFigures(true);

		pDC->SetBkMode(OLD_BKMODE);
		font.DeleteObject();
		penROI.DeleteObject();
		penBase.DeleteObject();
		penStage.DeleteObject();
	}

	SYSTEMTIME time;
	::GetLocalTime(&time);
	CString str_temp;
	str_temp.Format("NOZZLE_GAP_%04d%02d%02d_%02d%02d%02d", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
	m_pMain->vt_job_info[nJob].main_object_id = str_temp;

	str_temp.Format("%02d_%02d_%02d", time.wHour, time.wMinute, time.wSecond);
	m_pMain->m_strResultTime[nJob] = str_temp;

	double Z_maxspec = m_dCGThickness + m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(7);
	double Z_minspec = m_dCGThickness + m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(7);

	// YCS 2022-11-30 Z Gap 프로세스 로그 수정 - OK NG 상관없이 모두 NG라고 로그 남김
	if (m_bJudge) 	str.Format("[%s] Nozzle Gap Z : %3.4f(%3.4f - %3.4f)um - Spec OK", m_pMain->vt_job_info[nJob].job_name.c_str(), zdist, Z_minspec, Z_maxspec);
	else str.Format("[%s] Nozzle Gap Z : %3.4f(%3.4f - %3.4f)um - Spec NG", m_pMain->vt_job_info[nJob].job_name.c_str(), zdist, Z_minspec, Z_maxspec);
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
#pragma endregion

	// YCS 2022-11-30 Z Gap 결과저장 파라미터 변경. 패턴 판정 -> Z GAP스펙 판정
	m_pMain->save_result_image_nozzle_gap(m_pMain->getProcBuffer(real_cam, 0), m_bJudge, nJob, nCam, m_dCGThickness);
}

void CFormMainView::fnSimulation_CalcNozzleGapInsp(int nJob, int nCam, const cv::Mat& src, CViewerEx* mViewer)
{
	CString str;

	bool bfind = true;
	std::vector<int> camBuf = m_pMain->vt_job_info[nJob].camera_index;
	int camCount = m_pMain->vt_job_info[nJob].num_of_camera;
	int cam = 0;
	int real_cam = camBuf.at(cam);
	double m_dCGThickness = 0.0;
	int W = m_pMain->m_stCamInfo[nCam].w;
	int H = m_pMain->m_stCamInfo[nCam].h;

#pragma region Camera Grab & Matching
	
	bool _bCenterNozzleAlignMethod = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getCenterNozzleAlignMethod();

	if (_bCenterNozzleAlignMethod) // 테스트중
	{
		cv::Point2f matchPt = cv::Point2f(-1, -1);

		bfind = InspNozzleSearchCaliper_Use(src.data, W, H, nJob, real_cam, matchPt,true, mViewer);

		if (bfind)
		{
			m_pMain->GetMatching(nJob).getFindInfo(cam, 0).SetXPos(matchPt.x);
			m_pMain->GetMatching(nJob).getFindInfo(cam, 0).SetYPos(matchPt.y);
		}
	}
	else
	{
		m_pMain->GetMatching(nJob).findPattern(src.data, cam, 0, W, H); // 4번 위치에 기준 마크 등록
		if (m_pMain->GetMatching(nJob).getFindInfo(cam, 0).GetFound() != FIND_OK)  bfind = false;
	}

	//Tkyuha 2022-06-17 Nozzle XY 검사중 다시 한번 Y만 정밀 Search , 검증후 적용 예정

	bool _recalc = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getCenterNozzleYAlignRecalc();
	if (_recalc)
	{
		double posX = m_pMain->GetMatching(nJob).getFindInfo(cam, 0).GetXPos();
		double posY = m_pMain->GetMatching(nJob).getFindInfo(cam, 0).GetYPos();
		// KMB 2022-09-16
		//double rv = m_pMain->reCalculateMarkZGapYpos2(m_pMain->getProcBuffer(real_cam, 0), W, H, posX, posY);
		//if (rv > 0 && fabs(rv - posY) > 2.0)  m_pMain->GetMatching(nJob).getFindInfo(cam, 0).SetYPos(rv);
		
		// KBJ 2022-12-06 Z-Gap Recalc 개선
		m_pMain->reCalculateMarkZGapYpos3(nJob, m_pMain->getProcBuffer(real_cam, 0), W, H, &posX, &posY);
		m_pMain->GetMatching(nJob).getFindInfo(cam, 0).SetXPos(posX);
		m_pMain->GetMatching(nJob).getFindInfo(cam, 0).SetYPos(posY);
	}
#pragma endregion

	double FindMark_X = m_pMain->GetMatching(nJob).getFindInfo(cam, 0).GetXPos();
	double FindMark_Y = m_pMain->GetMatching(nJob).getFindInfo(cam, 0).GetYPos();
	double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(cam, 0);
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(cam, 0);
	double _offset = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getNozzleToStageGapOffset();

	BOOL bFindLine = FALSE;
	sLine line_info;
	double dx = 0.0, dy = 0.0, dt = 0.0;
	double centerPtX = W / 2, cetnerPtY = H / 2;

#pragma region  Find Stage Edge & Calc Distance
	m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][C_CALIPER_POS_1].processCaliper(m_pMain->getProcBuffer(real_cam, 0), W, H, dx, dy, dt, TRUE);
	line_info = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][C_CALIPER_POS_1].m_lineInfo;
	bFindLine = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][0][C_CALIPER_POS_1].getIsMakeLine();

	// KBJ 2022-07-13 Z-Gap 캘리퍼 무시하도록 설정.
	//if (bFindLine)
	if (0)
	{
		sPoint _point, point_on_line;
		_point.x = FindMark_X;
		_point.y = FindMark_Y;
		m_pMain->GetMatching(nJob).compute_orthogonal_point_on_line(line_info, _point, point_on_line);
		centerPtX = point_on_line.x, cetnerPtY = point_on_line.y;
	}

	double xdist = fabs(centerPtX - FindMark_X) * xres;
	double zdist = fabs(cetnerPtY - FindMark_Y) * yres + _offset;
#pragma endregion

#pragma region Judge & Display

	//  Result Judge
	// YCS 2022-11-30 Z GAP 판정 변수 위치 변경
	BOOL m_bJudge = FALSE;
	if (bfind)
	{
		//KJH 2022-05-19 CG Thickness 받아서 Z Gap 판정하기
		long lData[2] = { 0, };

		g_CommPLC.GetWord(m_pMain->vt_job_info[nJob].plc_addr_info.read_word_start + 10, 2, lData);

		m_dCGThickness = MAKELONG(lData[0], lData[1]);

		m_dCGThickness /= 10000.0;

		double Z_maxspec = m_dCGThickness + m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(7);
		double Z_minspec = m_dCGThickness + m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(7);

		if (zdist <= Z_maxspec && zdist >= Z_minspec)
		{
			m_bJudge = TRUE;
		}
		else
		{
			m_bJudge = FALSE;
		}
		
		auto pDC = mViewer->getOverlayDC();

		if (!_bCenterNozzleAlignMethod)
		{
			mViewer->ClearOverlayDC();
			mViewer->clearAllFigures();
		}

		CPen penROI(PS_SOLID, 7, RGB(0, 255, 0));
		CPen penBase(PS_SOLID, 5, RGB(255, 0, 0));
		CPen penStage(PS_SOLID, 5, RGB(255, 255, 0));

		CFont font, * oldFont;
		int OLD_BKMODE = 0;

		font.CreateFont(30, 30, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
			OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
		oldFont = pDC->SelectObject(&font);
		OLD_BKMODE = pDC->SetBkMode(TRANSPARENT);
		pDC->SelectObject(&penROI);
		(CBrush*)pDC->SelectStockObject(NULL_BRUSH);

		// KBJ 2022-10-13 Z-Gap 글자 표시 수정
		CString str_temp;
		struct stFigureText figure;

		str.Format("Target : %3.4fum", m_dCGThickness);
		figure.ptBegin.x = centerPtX;
		figure.ptBegin.y = 50;
		figure.ptBeginFit = figure.ptBegin;
		figure.textString = str;
		figure.is_show_bkg = TRUE;

		if (m_bJudge)	mViewer->addFigureText(figure, 12, 20, COLOR_GREEN);
		else			mViewer->addFigureText(figure, 12, 20, COLOR_RED);
		//pDC->TextOutA(50, 50, str);

		str_temp.Format("Z : %.4fum( Orignal: %.4fum )", zdist, zdist - _offset);
		figure.ptBegin.x = centerPtX;
		figure.ptBegin.y = 150;
		figure.ptBeginFit = figure.ptBegin;
		figure.textString = str_temp;
		figure.is_show_bkg = TRUE;
		
		if (m_bJudge)	mViewer->addFigureText(figure, 12, 20, COLOR_GREEN);
		else			mViewer->addFigureText(figure, 12, 20, COLOR_RED);

		m_pMain->vt_result_info[nJob].revision_data[nCam][0] = FindMark_X;
		m_pMain->vt_result_info[nJob].revision_data[nCam][1] = FindMark_Y;
		m_pMain->vt_result_info[nJob].revision_data[nCam][2] = zdist;

		// KBJ 2022-07-12 Gap Line 너무 두꺼운거 같아서 PDC->Viewer로 그리기 변경
		stFigure tempFig;

		/////// Draw Pattern Find Pos
		int fatW = m_pMain->GetMatching(nJob).getHalcon(0, 0, 0).getFatWidth();
		int fatH = m_pMain->GetMatching(nJob).getHalcon(0, 0, 0).getFatHeight();
		double offsetX = m_pMain->GetMatching(nJob).getPosOffsetX(0, 0, 0);
		double offsetY = m_pMain->GetMatching(nJob).getPosOffsetY(0, 0, 0);
		//int oriX = (int)(FindMark_X - offsetX);
		//int oriY = (int)(FindMark_Y - offsetY);

		//// 마크 그리기
		//tempFig.ptBegin.x = oriX - fatW / 2;
		//tempFig.ptBegin.y = oriY + fatH / 2;
		//tempFig.ptEnd.x = oriX + fatW / 2;
		//tempFig.ptEnd.y = oriY + fatH / 2;
		//mViewer->addFigureLine(tempFig, 2, 1, COLOR_GREEN);

		//tempFig.ptBegin.x = oriX - fatW / 2;
		//tempFig.ptBegin.y = oriY + fatH / 2;
		//tempFig.ptEnd.x = oriX - fatW / 2;
		//tempFig.ptEnd.y = oriY - fatH / 2;
		//mViewer->addFigureLine(tempFig, 2, 1, COLOR_GREEN);

		//tempFig.ptBegin.x = oriX + fatW / 2;
		//tempFig.ptBegin.y = oriY - fatH / 2;
		//tempFig.ptEnd.x = oriX + fatW / 2;
		//tempFig.ptEnd.y = oriY + fatH / 2;
		//mViewer->addFigureLine(tempFig, 2, 1, COLOR_GREEN);

		//tempFig.ptBegin.x = oriX + fatW / 2;
		//tempFig.ptBegin.y = oriY - fatH / 2;
		//tempFig.ptEnd.x = oriX - fatW / 2;
		//tempFig.ptEnd.y = oriY - fatH / 2;
		//mViewer->addFigureLine(tempFig, 2, 1, COLOR_GREEN);

		if(mViewer!=NULL) mViewer->AddSoftGraphic(new GraphicRectangle(Gdiplus::PointF(FindMark_X - offsetX, FindMark_Y - offsetY), fatW, fatH, 0xff00ff00));
		if (mViewer != NULL) mViewer->AddSoftGraphic(new GraphicPoint(cv::Point2f(FindMark_X, FindMark_Y), 0xff00ff00, 7, 2));
		//m_pMain->draw_mark(mViewer, COLOR_GREEN, FindMark_X, FindMark_Y, 2, 1, 20);

		/////// Draw Base Line
		double ylineoffset = _offset / yres;
		//tempFig.ptBegin.x = 0;
		//tempFig.ptBegin.y = cetnerPtY + ylineoffset;
		//tempFig.ptEnd.x = W;
		//tempFig.ptEnd.y = cetnerPtY + ylineoffset;
		//mViewer->addFigureLine(tempFig, 2, 2, COLOR_RED);

		if (mViewer != NULL) mViewer->AddSoftGraphic(new GraphicLine(Gdiplus::PointF(0, cetnerPtY + ylineoffset), Gdiplus::PointF(W, cetnerPtY + ylineoffset), 0xffff0000, 1, Gdiplus::DashStyle::DashStyleSolid, Gdiplus::SmoothingMode::SmoothingModeNone));

		/////// Draw Caliper Line
		// KBJ 2022-07-13 Z-Gap 캘리퍼 무시하도록 설정.
		//if (bFindLine)
		if (0)
		{
			m_pMain->draw_line(mViewer, COLOR_YELLOW, line_info, H, W);
		}

		//pDC->SelectObject(&penBase);
		//pDC->MoveTo((int)(0), (int)(cetnerPtY + ylineoffset));
		//pDC->LineTo((int)(W - 1), (int)(cetnerPtY + ylineoffset));

		/////// Draw Pattern Find Pos
		//pDC->MoveTo((int)(FindMark_X - 20), (int)(FindMark_Y));
		//pDC->LineTo((int)(FindMark_X + 20), (int)(FindMark_Y));
		//pDC->MoveTo((int)(FindMark_X), (int)(FindMark_Y - 20));
		//pDC->LineTo((int)(FindMark_X), (int)(FindMark_Y + 20));

		//double ylineoffset = _offset / yres;
		//pDC->SelectObject(&penBase);
		//pDC->MoveTo((int)(0), (int)(cetnerPtY + ylineoffset));
		//pDC->LineTo((int)(W - 1), (int)(cetnerPtY + ylineoffset));

		//if (bFindLine)
		//
		//	pDC->SelectObject(&penStage);
		//	pDC->MoveTo((int)(0), int(line_info.b));
		//	pDC->LineTo((int)(W - 1), int(line_info.a * (W - 1) + line_info.b));
		//}

		str.Format("[GAP INSP] [X:%4.3f, Z:%4.3f] => [X:%4.3f, Z:%4.3f ]", W / 2., cetnerPtY, FindMark_X, FindMark_Y);
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

		mViewer->SetEnableDrawAllFigures(true);

		pDC->SetBkMode(OLD_BKMODE);
		font.DeleteObject();
		penROI.DeleteObject();
		penBase.DeleteObject();
		penStage.DeleteObject();
	}

	SYSTEMTIME time;
	::GetLocalTime(&time);
	CString str_temp;
	str_temp.Format("NOZZLE_GAP_%04d%02d%02d_%02d%02d%02d", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute, time.wSecond);
	m_pMain->vt_job_info[nJob].main_object_id = str_temp;

	double Z_maxspec = m_dCGThickness + m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(7);
	double Z_minspec = m_dCGThickness + m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(7);

	//str.Format("[%s] Nozzle Gap X,Z(%3.4f,%3.4f)um", m_pMain->vt_job_info[nJob].job_name.c_str(), xdist, zdist);
	str.Format("[%s] Nozzle Gap Z : %3.4f(%3.4f - %3.4f)um NG", m_pMain->vt_job_info[nJob].job_name.c_str(), zdist, Z_minspec, Z_maxspec);
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
#pragma endregion
}

void CFormMainView::DrawArrow(CDC* pDC,  COLORREF color,CPoint start, CPoint end,double T)
{
	// Draw Arrow
	CPen penArrow(PS_SOLID, 10, color),  * pOldPen;
	pOldPen = pDC->SelectObject(&penArrow);

	POINT pt1, pt2;
	double cX, cY;
	double arrowStartX, arrowStartY;
	double arrowEndX, arrowEndY;
	double arrowLY, arrowLX, arrowRX, arrowRY;

	cX = (start.x + end.x) / 2.0;
	cY = (start.y + end.y) / 2.0;

	double lineT = T;

	if (start.y <= end.y)	lineT = lineT + PI;
	if (lineT > PI)			lineT -= (PI * 2);
	if (lineT < -PI)			lineT += (PI * 2);

	double arrowLength = 100.0;
	double arrowT = lineT + 45.0 / 180.0 * PI;
	arrowStartX = cX + arrowLength * cos(lineT);
	arrowStartY = cY + arrowLength * sin(lineT);
	arrowEndX = cX + arrowLength * cos(lineT + PI);
	arrowEndY = cY + arrowLength * sin(lineT + PI);

	pt1.x = (int)(cX + arrowLength * cos(lineT));
	pt1.y = (int)(cY + arrowLength * sin(lineT));

	pt2.x = (int)cX;
	pt2.y = (int)cY;
	pDC->MoveTo(pt1);
	pDC->LineTo(pt2);

	pt1.x = (int)(cX + arrowLength * cos(lineT + PI));
	pt1.y = (int)(cY + arrowLength * sin(lineT + PI));
	pDC->MoveTo(pt1);
	pDC->LineTo(pt2);

	// 꺽쇠 좌표 계산
	arrowLX = arrowEndX + 50 * cos(arrowT);
	arrowLY = arrowEndY + 50 * sin(arrowT);
	arrowRX = arrowEndX + 50 * cos(arrowT + 270.0 / 180.0 * PI);
	arrowRY = arrowEndY + 50 * sin(arrowT + 270.0 / 180.0 * PI);

	pt1.x = (int)arrowLX;
	pt1.y = (int)arrowLY;
	pt2.x = (int)arrowEndX;
	pt2.y = (int)arrowEndY;
	pDC->MoveTo(pt1);
	pDC->LineTo(pt2);

	pt1.x = (int)arrowRX;
	pt1.y = (int)arrowRY;
	pDC->MoveTo(pt1);
	pDC->LineTo(pt2);

	pDC->SelectObject(pOldPen);
}

//HTK 2022-05-30 본사수정내용
BOOL CFormMainView::InspUT_EdgeDetection(bool bnot, bool bcir, int nJob, int nCam, CPoint ptStart, CPoint ptEnd, CViewerEx* mViewer, bool balign, bool clr)
{
	CString str;
	str.Format("[Trace] Find ROI : %d, %d, %d, %d", ptStart.x, ptStart.y, ptEnd.x, ptEnd.y);
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

	system_clock::time_point start = system_clock::now();

	FakeDC* pDC = NULL;
	CPen penROI(PS_SOLID, 3, RGB(255, 0, 0));
	CPen penFit(PS_SOLID, 3, RGB(0, 255, 255));
	CPen penOrg(PS_SOLID, 3, RGB(0, 0, 255));

	if (mViewer != NULL)
	{
		if (clr)
			mViewer->ClearOverlayDC();

		pDC = mViewer->getOverlayDC();
		pDC->SelectObject(&penROI);
	}

	int W = m_pMain->m_stCamInfo[nCam].w;
	int H = m_pMain->m_stCamInfo[nCam].h;

	double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(nCam, 0);
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(nCam, 0);

	cv::Mat bin, bin2, bin3;
	cv::Mat img(H, W, CV_8UC1, m_pMain->getProcBuffer(nCam, 0));
	BOOL breturn = TRUE;

	cv::Mat median, dstImage, dstImage2;
	img.copyTo(median);

	cv::threshold(median, dstImage, -1, 255, CV_THRESH_BINARY | THRESH_OTSU);
	////////////////////////
	// 형상 매칭 후 켈리퍼 사용
	//////////////////////////
	BOOL bFixtureUse = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getUseFixtureMark();
	double dx = 0, dy = 0, dt = 0;
	if (bFixtureUse)
	{
		CPatternMatching* pPattern = &m_pMain->GetMatching(nJob);
		double fixtureX = pPattern->getFixtureX(nCam, 0);
		double fixtureY = pPattern->getFixtureY(nCam, 0);

		pPattern->findPattern(median.data, nCam, 0, W, H);

		if (m_pMain->GetMatching(nJob).getFindInfo(nCam, 0).GetFound() == FIND_OK)
		{
			double posX = m_pMain->GetMatching(nJob).getFindInfo(nCam, 0).GetXPos();
			double posY = m_pMain->GetMatching(nJob).getFindInfo(nCam, 0).GetYPos();

			if (fixtureX != 0.0 && fixtureY != 0.0)
			{
				dx = posX - fixtureX;
				dy = posY - fixtureY;
				dt = 0.0;
			}
		}
	}
	/////////////////////////

	cv::Rect rect_roi;

	rect_roi.x = ptStart.x;
	rect_roi.y = ptStart.y;
	rect_roi.width = abs(ptEnd.y - ptStart.x);
	rect_roi.height = abs(ptEnd.y - ptStart.y);

	cv::medianBlur(dstImage(rect_roi), median(rect_roi), 11);

	m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][3].processCaliper(median.data, W, H, dx, dy, dt);
	m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][4].processCaliper(median.data, W, H, dx, dy, dt);
	m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][5].processCaliper_circle(median.data, W, H, dx, dy, dt);

	BOOL find_line[3];
	sLine line_info[3];
	sCircle _circle;

	find_line[0] = m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][3].getIsMakeLine();
	find_line[1] = m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][4].getIsMakeLine();
	find_line[2] = m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][5].getIsMakeLine();
	line_info[0] = m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][3].m_lineInfo;
	line_info[1] = m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][4].m_lineInfo;
	_circle = m_pMain->m_pDlgCaliper->m_Caliper[nCam][0][5].m_circle_info;

	if (find_line[0] && find_line[1] && find_line[2])
	{
		// 가로 선과 세로선 교차점
		cv::Point2f pt_cross = IntersectionPoint2(line_info[0].a, line_info[0].b, line_info[1].a, line_info[1].b);

		// 원 중심에서 양쪽 직선에 법선을 구해야 함 
		sPoint _point, pt_left_corss,pt_top_cross;
		_point.x = _circle.x;
		_point.y = _circle.y;			

		//m_pMain->GetMatching(nJob).compute_orthogonal_point_on_line(line_info[1], _point, pt_left_corss);
		//m_pMain->GetMatching(nJob).compute_orthogonal_point_on_line(line_info[0], _point, pt_top_cross);

		GetTangentPointsAtCircle(_circle.x, _circle.y, _circle.r, pt_cross.x, pt_cross.y,
								 pt_top_cross.x, pt_top_cross.y, pt_left_corss.x, pt_left_corss.y);
	
		// Trace 데이타 생성
		
		std::vector<cv::Point2f> imgPtr;
		std::vector<cv::Point2f> _tmpimgPtr;
		cv::Point2f _tmpPt;
		// Top 교차점에서 오른쪽 10mm 지점 계산
		double _angle = atan(line_info[0].a);
		double pitch = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspTraceInterval() / xres;
		double distance = 10./xres;

		while (distance>0)
		{
			_tmpPt.x = distance * cos(_angle) + pt_top_cross.x;
			_tmpPt.y = distance * sin(_angle) + pt_top_cross.y;
			distance -= pitch;
			if (_tmpPt.x < pt_top_cross.x) break;
			
			imgPtr.push_back(_tmpPt);
		}
		//Round 부분 계산
		double _totalLength = 2. * CV_PI * _circle.r / 4.; // 90도에대한 길이
		int _ncount = MAX(1,int(cvRound(_totalLength / pitch)));
		double _divangle = 90. / _ncount;

		for (int i = 0; i < _ncount+1; i++)
		{
			double _ta = 270 - (i * _divangle);
			if (_ta >= 180)
			{
				_tmpPt.x = (cos(ToRadian(_ta)) * _circle.r + _circle.x);
				_tmpPt.y = (sin(ToRadian(_ta)) * _circle.r + _circle.y);

				imgPtr.push_back(_tmpPt);
			}
		}

		// Left 교차점에서 우선 아래 60mm 지점 계산
		_angle = atan(line_info[1].a);
		pitch  = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getInspTraceInterval() / yres;
		distance = 60. / yres;

		while (distance > 0)
		{
			_tmpPt.x = distance * cos(_angle) + pt_left_corss.x;
			_tmpPt.y = distance * sin(_angle) + pt_left_corss.y;
			distance -= pitch;
			if (_tmpPt.y < pt_left_corss.y) break;

			_tmpimgPtr.push_back(_tmpPt);
		}
		// 순서를 바꾸기 위해 수행
		for (int i= int(_tmpimgPtr.size()-1); i>=0; i--)
			imgPtr.push_back(_tmpimgPtr.at(i));
				
		_tmpimgPtr.clear();
		_tmpimgPtr.push_back(cv::Point2f(imgPtr[0].x- _circle.x, imgPtr[0].y- _circle.y));
		// 포인트별 각도 계산
		std::vector<float> ptrInterSectionTheta;
		for (int j = 1; j < imgPtr.size(); j++)
		{
			_tmpimgPtr.push_back(cv::Point2f(imgPtr[j].x - _circle.x, imgPtr[j].y - _circle.y));
			ptrInterSectionTheta.push_back(calcAngleFromPoints(imgPtr[j - 1], imgPtr[j]));  // 원데이타로 계산
		}

		ptrInterSectionTheta.push_back(calcAngleFromPoints(imgPtr[imgPtr.size() - 2], imgPtr[imgPtr.size() - 1]));  // 원데이타로 계산

		double startX=_circle.x, startY= _circle.y;
		double startRevX = _circle.x, startRevY = _circle.y;
		double dbAlignX, dbAlignY, rotateX = _circle.x, rotateY = _circle.y;
		float a = 0, b = 0;
		int lastPtr = int(_tmpimgPtr.size());
		double offsetx = _tmpimgPtr.at(0).x * xres;
		double offsety = _tmpimgPtr.at(0).y * yres;
		double c = ptrInterSectionTheta.at(0);

		TPointList5D arrayList;

		//// 아래는 노즐 시작 위치 변경 하기 위함
		startX = _tmpimgPtr[0].x * xres;
		startY = _tmpimgPtr[0].y * yres;

		startRevX = (_tmpimgPtr[lastPtr-1].x) * xres;
		startRevY = (_tmpimgPtr[lastPtr-1].y) * yres;

		rotateX = 0;
		rotateY = startY;

		for (int k = 0; k < lastPtr; k++)
		{
			float tc = (k == 0) ? ptrInterSectionTheta.at(0) - c : ptrInterSectionTheta.at(k - 1) - c;
			///// 원영상 위치에서 계산 할경우
			float xc = _tmpimgPtr.at(k).x*xres;
			float yc = _tmpimgPtr.at(k).y*yres;
	
			// 각도에 따른 회전 변환 으로 이한 좌표 계산
			calcRotate(rotateX, rotateY, xc, yc, (-tc) * CV_PI / 180.0, &dbAlignX, &dbAlignY);

			dbAlignX = (dbAlignX ) - (offsetx);
			dbAlignY = (dbAlignY ) - (offsety);

			if (k == lastPtr)
			{
				// 시작 위치는 무조건 0으로 만든다
				a = dbAlignX;
				b = dbAlignY;
				arrayList.push_back({ xc, yc, 0.f, 0.f, 0.f }); //(3, 4, 5번째 데이타 사용) 1,2번은 참고용
			}
			else
			{
				dbAlignX = dbAlignX - a;
				dbAlignY = dbAlignY - b;
				arrayList.push_back({ xc, yc, tc, float(dbAlignX), float(dbAlignY) });
				//변수 참고        //{ 회전이전 x,회전 이전 y,각도, 회전 이후 x, 회전 이후 y}
			}
		}

		if (mViewer != NULL)
		{
			CFont font, * oldFont;
			int OLD_BKMODE = 0;
			CString str_data;
			CPoint p2;
			int ptSize = 5;

			font.CreateFont(40, 40, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
				OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
			oldFont = pDC->SelectObject(&font);
			pDC->SelectObject(&penFit);
			OLD_BKMODE = pDC->SetBkMode(TRANSPARENT);
			pDC->SelectObject(GetStockObject(NULL_BRUSH));
			// 원중심
			pDC->Ellipse(int(_circle.x - _circle.r), int(_circle.y - _circle.r),
				int(_circle.x + _circle.r), int(_circle.y + _circle.r));
			// 왼쪽, 오른쪽 교차점
			pDC->Ellipse(int(pt_left_corss.x - ptSize), int(pt_left_corss.y - ptSize),
				int(pt_left_corss.x + ptSize), int(pt_left_corss.y + ptSize));
			pDC->Ellipse(int(pt_top_cross.x - ptSize), int(pt_top_cross.y - ptSize),
				int(pt_top_cross.x + ptSize), int(pt_top_cross.y + ptSize));
			//직선 그리기
			p2.x = 0;			        p2.y = line_info[0].a*p2.x + line_info[0].b;
			pDC->MoveTo(p2.x, p2.y);
			p2.x = W - 1;		p2.y = line_info[0].a * p2.x + line_info[0].b;
			pDC->LineTo(p2.x, p2.y);

			p2.y = 0;			        p2.x = (p2.y - line_info[1].b) / line_info[1].a;
			pDC->MoveTo(p2.x, p2.y);   
			p2.y = H-1;		p2.x = (p2.y - line_info[1].b) / line_info[1].a;
			pDC->LineTo(p2.x, p2.y);  

			// 원중심에서 직선과 교차점
			pDC->MoveTo(int(pt_left_corss.x), int(pt_left_corss.y));
			pDC->LineTo(int(_circle.x), int(_circle.y));
			pDC->LineTo(int(pt_top_cross.x), int(pt_top_cross.y));

			pDC->SelectObject(&penROI);

			for (auto iter : imgPtr) {
				pDC->Ellipse(int(iter.x - ptSize), int(iter.y - ptSize),
					int(iter.x + ptSize), int(iter.y + ptSize));
			}
		
			imgPtr.clear();
			_tmpimgPtr.clear();

			pDC->SetBkMode(OLD_BKMODE);
			penROI.DeleteObject();
			penFit.DeleteObject();
			font.DeleteObject();
		}

		// 수정 Trace 로그 저장
		CString msg;
		
		msg.Format("D:\\TraceProfile_Recalc_%s_%s.csv", m_pMain->vt_job_info[nJob].main_object_id.c_str(), "UT");
		ofstream  writeFile(msg);

		if (writeFile.is_open())
		{
			msg.Format("%d , %4.3f, %4.3f, %4.3f, %4.3f\n", lastPtr, startX, startY, startRevX, startRevY);
			writeFile << msg;
			for (int i = 0; i < lastPtr; i++)
			{
				msg.Format("%4.3f, %4.3f, %4.3f,  %4.3f, %4.3f\n", arrayList[i].at(0), arrayList[i].at(1),
					arrayList[i].at(2),
					arrayList[i].at(3),
					arrayList[i].at(4));
				writeFile << msg;
			}
		}
		writeFile.close();

		arrayList.clear();
	}
	else
	{
		m_pMain->notch_line_data.pt_mid.x = 0.0;
		m_pMain->notch_line_data.pt_mid.y = 0.0;
		m_pMain->notch_line_data.pt_up.x = 0.0;
		m_pMain->notch_line_data.pt_up.y = 0.0;
		m_pMain->notch_line_data.pt_down.x = 0.0;
		m_pMain->notch_line_data.pt_down.y = 0.0;

		breturn = FALSE;
	}

	median.release();

	if (mViewer != NULL) mViewer->Invalidate();

	

	system_clock::time_point end = system_clock::now();
	nanoseconds nano = end - start;

	str.Format("Divide time: %f", nano.count() / 1000000.);
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	::SendMessageA(m_pMain->m_pForm[FORM_CAMERA]->m_hWnd, WM_VIEW_CONTROL, MSG_FCV_OPEN_IMAGE_FILENAME, (LPARAM)&str);

	return breturn;
}

bool CFormMainView::fnCInk2_UV_InkInspection(int nJob, int id, int &count, BYTE* ImgByte, CViewerEx* mViewer)
{
	bool bReturn = true;

	int camCount = m_pMain->vt_job_info[nJob].num_of_camera;
	std::vector<int> camBuf = m_pMain->vt_job_info[nJob].camera_index;

	CString str;
	int ncamera = camBuf.at(0);
	int W = m_pMain->m_stCamInfo[ncamera].w;
	int H = m_pMain->m_stCamInfo[ncamera].h;

	str.Format("Cam %d - Lack of Ink Inspection Start", ncamera + 1);
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

	cv::Rect inspRoi = cv::Rect(0, 0, W-1, H-1);

	int method = m_pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod();
	double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(0, 0);
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);
	double rcx=0, rcy=0, rcr=0;

	bool bFilmUVCheck = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getFilmUVCheckEnable();
	int dThresh = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getFilmUVCheckThresh();
	int bFilmUVCheckSpec = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getFilmUVCheckSpec();

	count = 0;

	if (bFilmUVCheck && m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][5].getCircleCaliper())
	{

		m_pMain->find_pattern_caliper(ImgByte, W, H, nJob, 0, 0);

		rcx = m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][5].m_circle_info.x;
		rcy = m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][5].m_circle_info.y;
		rcr = m_pMain->m_pDlgCaliper->m_Caliper[ncamera][0][5].m_circle_info.r;

		inspRoi = cv::Rect(rcx - rcr-100, rcy - rcr-100, rcr * 2+200, rcr * 2+200);
	}
	else return true;

	inspRoi.x = MIN(W - 1, MAX(0, inspRoi.x));
	inspRoi.y = MIN(H - 1, MAX(0, inspRoi.y));
	inspRoi.width = MIN(W - inspRoi.x, MAX(0, inspRoi.width));
	inspRoi.height = MIN(H - inspRoi.y, MAX(0, inspRoi.height));

	inspRoi.width = inspRoi.width / 8 * 8;
	inspRoi.height = inspRoi.height / 8 * 8;

	cv::Mat imgSrc(H, W, CV_8UC1, ImgByte);
	cv::Mat img, s_down;

	s_down = imgSrc(inspRoi).clone();

	cv::threshold(s_down, img, dThresh, 255, CV_THRESH_BINARY);
	cv::Mat maskerode = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3), cv::Point(1, 1));

	erode(img, img, maskerode, cv::Point(-1, -1), 2);
	dilate(img, img, maskerode, cv::Point(-1, -1), 2);

	// 디버깅 모드에서 이미지 보기
	if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
	{
		cvShowImage("LackofInk Org Image", s_down);
		cvShowImage("LackofInk bin Image", img);
	}

	uchar* OrgRaw = img.data;
	int ncol = inspRoi.width;
	int nrow = inspRoi.height;
	int __searchCount = 0;
	bool _bfind = false;

	int nLackOfInk_CinkCount = 0;
	bool bLackofInkCheckFlag[360] = { false, };


	for (int _angle = 0; _angle < 360; _angle++)
	{
		__searchCount = 0;

		bLackofInkCheckFlag[_angle] = false;
		_bfind = false;

		for (int _dist = 15; _dist < 60; _dist++)
		{
			int __bmX = (cos(ToRadian(_angle)) * (rcr + _dist) + rcx - inspRoi.x); // CC Edge point
			int __bmY = (sin(ToRadian(_angle)) * (rcr + _dist) + rcy - inspRoi.y); // CC Edge point

			if (__bmX > 0 && __bmX < ncol && __bmY>0 && __bmY < nrow)
			{
				if (OrgRaw[__bmY * ncol + __bmX] > 100)
				{
					if (__searchCount > 5 ) // 찾은 Edge Point 
					{
						_bfind = true;
						break;
					}
					__searchCount++;

				}
				else __searchCount = 0;
			}
		}

		if (_bfind == false)
		{
			bLackofInkCheckFlag[_angle] = true;
			nLackOfInk_CinkCount++;
		}
	}

	count = nLackOfInk_CinkCount;

	img.release();
	s_down.release();
	imgSrc.release();
	maskerode.release();

	if (nLackOfInk_CinkCount > 0)
	{
		FakeDC* pDC = NULL;
		int nview = m_pMain->vt_job_info[nJob].viewer_index[id];
		if (mViewer != NULL)		pDC = mViewer->getOverlayDC();
		else						pDC = m_pDlgViewerMain[nview]->GetViewer().getOverlayDC();

		CPen penNG(PS_SOLID, 5, COLOR_GREEN);
		CPen* pOldPen;

		pDC->SelectObject(GetStockObject(NULL_BRUSH));
		pOldPen = pDC->SelectObject(&penNG);

		for (int i = 0; i < 360; i++)
		{
			if (!bLackofInkCheckFlag[i]) continue;

			int __x = (cos(ToRadian(i)) * (rcr)+rcx); // Glass Edge point
			int __y = (sin(ToRadian(i)) * (rcr)+rcy); // Glass Edge point	

			pDC->Ellipse(__x - 10, __y - 10, __x + 10, __y + 10);
		}

		if (mViewer != NULL)				mViewer->Invalidate();
		else								m_pDlgViewerMain[nview]->GetViewer().Invalidate();

		penNG.DeleteObject();
	}

	if (bFilmUVCheckSpec < nLackOfInk_CinkCount) bReturn = false;

	str.Format("Cam %d - UV Ink Inspection End", ncamera + 1);
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

	return bReturn;
}

void CFormMainView::change_job_main_view(int nIndex)
{
	m_TabMainView.SetCurSel(nIndex);

	int sel = m_TabMainView.GetCurSel();
	int job = sel - 1;

	if (sel == 0)
	{
		replace_viewer_main(m_pMain->m_nViewX_Num, m_pMain->m_nViewY_Num);
		//HTK 2022-07-11 Main View 변경시 화면 재 그리도록 변경
		RedrawWindow();
	}
	else
	{
		int x_cnt = m_pMain->vt_job_info[job].main_view_count_x;
		int y_cnt = m_pMain->vt_job_info[job].main_view_count_y;
		replace_viewer_main_fix(x_cnt, y_cnt, job);
		//HTK 2022-07-11 Main View 변경시 화면 재 그리도록 변경
		RedrawWindow();
	}

	//HSJ 2022-01-05 Main View Tab이동시 해당 결과창도 같이 바뀌게
	//HSJ 2022-01-13 Main View Tab이동시 해당 스펙창도 같이 바뀌게

	CPaneAuto* pPaneAuto = (CPaneAuto*)m_pMain->m_pPane[PANE_AUTO];

	if (sel == 0) return;
	else
	{
		int index = sel - 1;
		pPaneAuto->m_TabResultView.SetCurSel(index);
		pPaneAuto->RenewResultChart(index);

		int method = m_pMain->vt_job_info[index].algo_method;
		if (method == CLIENT_TYPE_SCAN_INSP || method == CLIENT_TYPE_FILM_INSP)
		{
			pPaneAuto->m_TabSpecView.SetCurSel(1);
			pPaneAuto->RenewSpecChart(1);
		}
		else
		{
			pPaneAuto->m_TabSpecView.SetCurSel(0);
			pPaneAuto->RenewSpecChart(0);
		}
	}

	return;
}

// 20220928 스크레치 검사 추가
BOOL CFormMainView::Inspection_ScratchHole(int nJob, int id, BYTE* ImgByte, CViewerEx* mViewer, bool bSimul)
{
	BOOL bReturn = TRUE;

	system_clock::time_point start = system_clock::now();

	std::vector<int> camBuf = m_pMain->vt_job_info[nJob].camera_index;

	if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
	{
		cvHideImage("Enhance Image");
	}

	CString str="NULL";
	int ncamera = camBuf.at(0);
	int W = m_pMain->m_stCamInfo[ncamera].w;
	int H = m_pMain->m_stCamInfo[ncamera].h;
	double xres = m_pMain->GetMachine(nJob).getCameraResolutionX(0, 0);
	double yres = m_pMain->GetMachine(nJob).getCameraResolutionY(0, 0);
	double cir_size = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getScratchMaskingRadius();
	int radius = int(cir_size / xres );
	int inspAreaSpec = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getScratchInspAreaSpec();
	int roiOffset = (radius + inspAreaSpec) / 8 * 8; // 반지름

	cv::Mat gray;
	cv::Mat imgSrc(H, W, CV_8UC1, ImgByte);

	CRect InspROI = CRect(W / 2 - 1000, H / 2 - 1000, W / 2 + 1000, H / 2 + 1000);

	cv::Mat tmpimg(200, 200, CV_8UC1, cv::Scalar(0));
	cv::circle(tmpimg, cv::Point(100, 100), radius, Scalar(255, 255, 255), -1, 8);
	
    FakeDC* pDC = NULL;
	int nview = m_pMain->vt_job_info[nJob].viewer_index[id];

    if (mViewer != NULL)
	{
		pDC = mViewer->getOverlayDC();
		mViewer->ClearOverlayDC();
		mViewer->clearAllFigures();
	}
	else
	{
		pDC = m_pDlgViewerMain[nview]->GetViewer().getOverlayDC();
		m_pDlgViewerMain[nview]->GetViewer().ClearOverlayDC();
		m_pDlgViewerMain[nview]->GetViewer().clearAllFigures();
	}

			// hsj 2022-10-31 image draw
	//m_pDlgViewerMain[nview]->GetViewer().OnLoadImageFromPtr(m_pMain->getProcBuffer(ncamera, 2));

	CPen penNG(PS_SOLID, 5, COLOR_RED);
	CPen penOK(PS_SOLID, 5, COLOR_GREEN);
	CPen penROI(PS_SOLID, 3, COLOR_WHITE);
	
	pDC->SelectObject(&penROI);
	
	pDC->Rectangle(CRect(InspROI.left, InspROI.top, InspROI.right, InspROI.bottom));
	// 디버깅 모드에서 이미지 보기
	if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
	{
		cvShowImage("Scratch Insp Org Image", tmpimg);
	}

	BYTE* pImage;
	if( bSimul == TRUE) pImage = m_pMain->getCameraViewBuffer();
	else pImage = m_pMain->getProcBuffer(ncamera, 0);

	m_pMain->GetMatching(nJob).setSearchROI(0, 2, InspROI);
	m_pMain->GetMatching(nJob).findPattern(pImage, 0, 2, W, H);
	BOOL bFind = m_pMain->GetMatching(nJob).getFindInfo(0, 2).GetFound() == FIND_OK ? TRUE : FALSE;

	//m_pMain->GetMatching(nJob).getHalcon(0, 2, 0).m_ReadSmallScale = 0.3;
	//m_pMain->GetMatching(nJob).getHalcon(0, 2, 0).m_SearchSubPixel = "least_squares";
	//m_pMain->GetMatching(nJob).getHalcon(0, 2, 0).halcon_ReadScaleModel(tmpimg.data, NULL, tmpimg.cols, tmpimg.rows);
	//bFind = m_pMain->GetMatching(nJob).getHalcon(0, 2, 0).halcon_SearchScaleMark(imgSrc.data, W, H, InspROI, 80.);

	double max_blob_width = 0.0;
	double max_blob_height = 0.0;

	if (bFind)
	{
		int dThresh = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getScratchThresh();
		//double dbPosX = m_pMain->GetMatching(nJob).getHalcon(0, 2, 0).halcon_GetResultPos().x + InspROI.left;
		//double dbPosY = m_pMain->GetMatching(nJob).getHalcon(0, 2, 0).halcon_GetResultPos().y + InspROI.top;

		double dbPosX = m_pMain->GetMatching(nJob).getFindInfo(0, 2).GetXPos();
		double dbPosY = m_pMain->GetMatching(nJob).getFindInfo(0, 2).GetYPos();

		cv::Mat Mask2, Mask, iproc;
		cv::Rect _Roi = cv::Rect(dbPosX - roiOffset, dbPosY - roiOffset, roiOffset * 2, roiOffset * 2);

		_Roi.x = MIN(W - 1, MAX(0, _Roi.x));
		_Roi.y = MIN(H - 1, MAX(0, _Roi.y));
		_Roi.width = MIN(W - _Roi.x, MAX(0, _Roi.width));
		_Roi.height = MIN(H - _Roi.y, MAX(0, _Roi.height));


		if (_Roi.width > 0 && _Roi.height > 0)
		{
			gray = imgSrc(_Roi).clone();
			Mask2 = GetCirclemask(gray, gray.cols / 2, gray.rows / 2, radius + 12);
			bitwise_not(Mask2, Mask2);

			GaussianBlur(gray, gray, cv::Size(0, 0), 3.);
			m_pMain->m_ManualAlignMeasure.InspectionEnhance(&gray, 0, false);

			if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
			{
				cvShowImage("Enhance Image", gray);
			}

			threshold(gray, iproc, dThresh, 255, CV_THRESH_BINARY);
			bitwise_and(Mask2, iproc, Mask);

			if (m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
			{
				cvShowImage("Enhance Bin Image", Mask);
			}
			/// <summary>

			CMBlob m_Blob;
			m_Blob.Create(1000, 1500, 1500);

			m_Blob.InitValue();
			m_Blob.SetImgInfo(gray.cols, gray.rows, 1, gray.cols);
			m_Blob.SetRoi(CRect(4, 4, gray.cols - 4, gray.rows - 4));

			m_Blob.BlobScan((LPBYTE)Mask.data);
			m_Blob.ExtractBlob();

			int nPtCnt = m_Blob.GetBlobNum();
			int nRealDefectCount = 0;
			/// <param name="mViewer"></param>
			
			pDC->SelectObject(&penOK);

			pDC->Ellipse(dbPosX - radius, dbPosY - radius, dbPosX + radius, dbPosY + radius);

			Gdiplus::Color color = 0xffff0000;

			pDC->SelectObject(&penNG);

			PSTBLOB lpCurBlob;
			std::vector<PSTBLOB> vtBlob;
			CRect roi;
			int pxlSizeSpec = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getScratchInspSpec();
			double min_size = m_pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspMinSize();

			for (int i = 1; i < nPtCnt; i++)
			{
				lpCurBlob = m_Blob.GetBlob(i);
				if (-1 == lpCurBlob->state) continue;

				roi.left = lpCurBlob->left + _Roi.x;
				roi.top = (gray.rows - lpCurBlob->bottom) + _Roi.y;
				roi.right = lpCurBlob->right + _Roi.x;
				roi.bottom = (gray.rows - lpCurBlob->top) + _Roi.y;

				pDC->Rectangle(CRect(roi.left, roi.top, roi.right, roi.bottom));

				vtBlob.push_back(m_Blob.GetBlob(i));

				if (int(lpCurBlob->pxlSize) > pxlSizeSpec&& (min_size < (xres * roi.Width()) || min_size < (yres * roi.Height())))
				{	
					nRealDefectCount++;
					if (mViewer != NULL)	mViewer->AddSoftGraphic(new GraphicRectangle(roi.left, roi.top, roi.right, roi.bottom, Gdiplus::Color::Red));
					else pDC->Rectangle(CRect(roi.left, roi.top, roi.right, roi.bottom));
				}
				else										continue;

				max_blob_width = MAX(max_blob_width, fabs(lpCurBlob->right - lpCurBlob->left));
				max_blob_height = MAX(max_blob_height, fabs(lpCurBlob->bottom - lpCurBlob->top));
			}

			/*if (mViewer != NULL)				mViewer->Invalidate();
			else								m_pDlgViewerMain[nview]->GetViewer().Invalidate();

			if (mViewer != NULL)				mViewer->DirtyRefresh();
			else								m_pDlgViewerMain[nview]->GetViewer().DirtyRefresh();*/


			if (m_pMain->vt_job_info[nJob].num_of_viewer > 1)
			{
				int nview2 = m_pMain->vt_job_info[nJob].viewer_index[1];
				
				_stLoadScratchImage pParam;
				pParam.pDlg = m_pMain;
				pParam.nJob = nJob;
				pParam.ImgByte = pImage;
				pParam.viewerDlg = m_pDlgViewerMain[nview2];
				for (int i = 0; i < 10; i++)
				{
					if (i < vtBlob.size())	pParam.vtBlob[i] = vtBlob.at(i);
					else					pParam.vtBlob[i] = 0;
				}

				AfxBeginThread(Inspection_ScratchHole_LoadProcImage, &pParam);
			}

			if (nRealDefectCount >= 1)
			{
				//cvShowImage("Enhance Image", gray);

#pragma region Defect 저장 

				CString	Time_str = m_pMain->m_strResultTime[nJob];
				CString	Date_str = m_pMain->m_strResultDate[nJob];
				CString	real_Time_str, str_JobName;

				CTime NowTime;
				NowTime = CTime::GetCurrentTime();
				real_Time_str.Format("%04d%02d%02d", NowTime.GetYear(), NowTime.GetMonth(), NowTime.GetDay());

				if (Time_str == "" || Date_str == "")
				{
					m_pMain->m_strResultDate[nJob].Format("%04d%02d%02d", NowTime.GetYear(), NowTime.GetMonth(), NowTime.GetDay());
					m_pMain->m_strResultTime[nJob].Format("%02d_%02d_%02d", NowTime.GetHour(), NowTime.GetMinute(), NowTime.GetSecond());

					Time_str = m_pMain->m_strResultTime[nJob];
					Date_str = m_pMain->m_strResultDate[nJob];
				}

				CString Cell_Name = m_pMain->vt_job_info[nJob].get_main_object_id();
				if (strlen(Cell_Name) < 10)					Cell_Name.Format("Test_%s", Time_str);


				CString strFileDir_Image, strFileDir_Image_Proc, strFileDir_Image_Raw;

				strFileDir_Image = m_pMain->m_strImageDir + Date_str;
				if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);
				strFileDir_Image.Format("%s\\%s", strFileDir_Image, m_pMain->m_strCurrentModelName);
				if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);

				//JOB이름
				str_JobName.Format("%s",m_pMain->vt_job_info[nJob].job_name.c_str());
				strFileDir_Image.Format("%s\\%s", strFileDir_Image, str_JobName);
				if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);

				strFileDir_Image.Format("%s\\SCRATCH_INSP\\", strFileDir_Image);
				if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);

				if (m_pMain->m_bSimulationStart)
				{
					strFileDir_Image.Format("%ssimulation\\", strFileDir_Image);
					if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);
				}

				strFileDir_Image.Format("%s%s\\", strFileDir_Image, Cell_Name);
				if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);

				strFileDir_Image_Raw.Format("%s%s_%s_Raw.jpg", strFileDir_Image, Cell_Name, Time_str);
				strFileDir_Image_Raw.Format("%s%s_%s_%sImgRaw.jpg", strFileDir_Image, Cell_Name, Time_str, m_pMain->m_stCamInfo[ncamera].cName);

				if (_access(strFileDir_Image_Proc, 0) != 0) strFileDir_Image_Proc.Format("%s%s_%s_Proc_Img.jpg", strFileDir_Image, Cell_Name, real_Time_str);
				if (_access(strFileDir_Image_Raw, 0) != 0)	strFileDir_Image_Raw.Format("%s%s_%s_%s_ImgRaw.jpg", strFileDir_Image, Cell_Name, real_Time_str, m_pMain->m_stCamInfo[ncamera].cName);

				if (m_pMain->vt_job_info[nJob].num_of_viewer == 0) 
				cv::imwrite(std::string(strFileDir_Image_Proc), gray);
				cv::imwrite(std::string(strFileDir_Image_Raw), imgSrc);

				CString strFileDir, strTemp;

				//strFileDir.Format("%sFiles\\", m_pMain->m_strResultDir);
				//if (_access(strFileDir, 0) != 0)	CreateDirectory(strFileDir, NULL);
				//strFileDir.Format("%s%s\\", strFileDir, Date_str);
				//if (_access(strFileDir, 0) != 0)	CreateDirectory(strFileDir, NULL);
				//strFileDir.Format("%s%s\\", strFileDir, m_pMain->m_strCurrentModelName);
				//if (_access(strFileDir, 0) != 0)	CreateDirectory(strFileDir, NULL);
				//strFileDir.Format("%s%s\\", strFileDir, str_JobName);
				//if (_access(strFileDir, 0) != 0)	CreateDirectory(strFileDir, NULL);

				//if (m_pMain->m_bSimulationStart)	strFileDir.Format("%ssimulation_", strFileDir);
				//strTemp.Format("%s%s_Result_%s.csv", strFileDir, str_JobName, Date_str);
				strTemp.Format("%s%s_%s_%s.csv", strFileDir_Image, Cell_Name, real_Time_str, m_pMain->m_stCamInfo[ncamera].cName);

				BOOL bWriteHeader = FALSE;
				if ((_access(strTemp, 0)) == -1)
				{
					bWriteHeader = TRUE;
				}

				CStdioFile WriteFile;
				if (WriteFile.Open(strTemp, CFile::modeNoTruncate | CFile::modeCreate | CFile::modeWrite | CFile::shareDenyNone | CFile::typeText) != TRUE)
				{
					CString str;
					str.Format("Fail to Open Logfile !! : %s", strTemp.Mid(strTemp.ReverseFind('\\') + 1));
					::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
					return 0;
				}

				CString strHeader, strBody;

				strTemp.Format("TIME");																					strHeader += strTemp;
				strTemp.Format("%s", Time_str);																			strBody += strTemp;

				strTemp.Format(",MODEL ID");																			strHeader += strTemp;
				strTemp.Format(",%s", m_pMain->m_strCurrentModelName);													strBody += strTemp;

				strTemp.Format(",PANEL ID");																			strHeader += strTemp;
				strTemp.Format(",%s", Cell_Name);																		strBody += strTemp;

				strTemp.Format(",Scratch Count");																		strHeader += strTemp;
				strTemp.Format(",%d", vtBlob.size());																	strBody += strTemp;

				strTemp.Format(",Max Size Width");																		strHeader += strTemp;
				strTemp.Format(",%.3f", max_blob_width * xres);															strBody += strTemp;
				
				strTemp.Format(",Max Size Height");																		strHeader += strTemp;
				strTemp.Format(",%.3f", max_blob_height * yres);														strBody += strTemp;

				strHeader += "\n";
				strBody += "\n";

				if (bWriteHeader)
				{
					WriteFile.WriteString(strHeader);
				}

				WriteFile.SeekToEnd();
				WriteFile.WriteString(strBody);
				WriteFile.Close();
#pragma endregion
				vtBlob.clear();

				bReturn = FALSE;
			}
			else bReturn = TRUE;
		}
	}
	else bReturn = FALSE;


	tmpimg.release();
	imgSrc.release();
	gray.release();

	penROI.DeleteObject();
	penNG.DeleteObject();
	penOK.DeleteObject();

	if (bSimul != TRUE && m_pMain->m_bSimulationStart != TRUE)
	{
		if (bReturn == TRUE) g_CommPLC.SetBit(m_pMain->m_nAlignRequestRead[nJob] + 2, TRUE);
		else				 g_CommPLC.SetBit(m_pMain->m_nAlignRequestRead[nJob] + 3, TRUE);
	}

	max_blob_width = max_blob_width * xres;
	max_blob_height = max_blob_height * yres;

	long MaxSizeData[4] = { 0, };
	MaxSizeData[0] = LOWORD((long)(max_blob_width * MOTOR_SCALE));
	MaxSizeData[1] = HIWORD((long)(max_blob_width * MOTOR_SCALE));
	MaxSizeData[2] = LOWORD((long)(max_blob_height * MOTOR_SCALE));
	MaxSizeData[3] = HIWORD((long)(max_blob_height * MOTOR_SCALE));

	if (bSimul != TRUE && m_pMain->m_bSimulationStart != TRUE)
	{
		int address = m_pMain->vt_job_info[nJob].plc_addr_info.write_word_start + 14;
		g_CommPLC.SetWord(address, 4, MaxSizeData);

		str.Format("Send Data Adress : %d", address);
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		str.Format("max scratch width : %.3fmm", max_blob_width);
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		str.Format("max scratch height : %.3fmm", max_blob_height);
		::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
	}

	system_clock::time_point end = system_clock::now();
	nanoseconds nano = end - start;
	str.Format("Scratch Inspection Time: %f", nano.count() / 1000000.);
	::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

	return bReturn;
}

UINT Inspection_ScratchHole_LoadProcImage(void* pParam)
{
	_stLoadScratchImage* pScratch_Info = (_stLoadScratchImage*)pParam;
	CLET_AlignClientDlg* pMain = (CLET_AlignClientDlg*)pScratch_Info->pDlg;
	int nJob = (int)pScratch_Info->nJob;
	BYTE* ImgByte = (BYTE*)pScratch_Info->ImgByte;
	CDlgViewer* pView = (CDlgViewer*)pScratch_Info->viewerDlg;
	vector< PSTBLOB > vtBlob;
	for (int i = 0; i < 10; i++)
	{
		if (pScratch_Info->vtBlob[i] != 0) vtBlob.push_back(pScratch_Info->vtBlob[i]);
	}

	std::vector<int> camBuf = pMain->vt_job_info[nJob].camera_index;

	int ncamera = camBuf.at(0);
	int W = pMain->m_stCamInfo[ncamera].w;
	int H = pMain->m_stCamInfo[ncamera].h;

	cv::Mat gray, mask, result_img;
	cv::Mat imgSrc(H, W, CV_8UC1, ImgByte);

	double xres = pMain->GetMachine(nJob).getCameraResolutionX(0, 0);
	double yres = pMain->GetMachine(nJob).getCameraResolutionY(0, 0);
	double cir_size = pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getScratchMaskingRadius();
	int radius = int(cir_size / xres);
	int inspAreaSpec = pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getScratchInspAreaSpec();
	int roiOffset = (radius + inspAreaSpec) / 8 * 8; // 반지름

	double dbPosX = pMain->GetMatching(nJob).getFindInfo(0, 2).GetXPos();
	double dbPosY = pMain->GetMatching(nJob).getFindInfo(0, 2).GetYPos();

	// ================================================== Viewer 사이즈로 이미지 자르기 =================================

	int viewer_width = pView->GetViewer().GetWidth();
	int viewer_height = pView->GetViewer().GetHeight();

	cv::Rect _Roi = cv::Rect(dbPosX - (viewer_width / 2), dbPosY - (viewer_height / 2), viewer_width, viewer_height);
	_Roi.x = MIN(W - 1, MAX(0, _Roi.x));
	_Roi.y = MIN(H - 1, MAX(0, _Roi.y));
	_Roi.width = MIN(W - _Roi.x, MAX(0, _Roi.width));
	_Roi.height = MIN(H - _Roi.y, MAX(0, _Roi.height));

	gray = imgSrc(_Roi).clone();
	mask = GetCirclemask(gray, gray.cols / 2, gray.rows / 2, radius);
	bitwise_not(mask, mask);

	GaussianBlur(gray, gray, cv::Size(0, 0), 3.);
	pMain->m_ManualAlignMeasure.InspectionEnhance(&gray, 0, false);

	FakeDC* pDC = pView->GetViewer().getOverlayDC();
	pView->GetViewer().ClearOverlayDC();
	pView->GetViewer().clearAllFigures();
	pView->GetViewer().OnLoadImageFromPtr(gray.data);

	//2022-10-20 KJH2 Scratch 결과 이미지 저장
	result_img = gray.clone();
	cv::cvtColor(result_img, result_img, COLOR_GRAY2BGR);

	// =================================================================================================================


	// ================================================== Draw =========================================================
	CString str;
	CPen penOK(PS_SOLID, 2, COLOR_GREEN);
	CPen penNG(PS_SOLID, 2, COLOR_RED);
	CPen penROI(PS_SOLID, 2, COLOR_WHITE);

	CFont fontBolb, font, * oldFont;
	int OLD_BKMODE = 0;

	fontBolb.CreateFont(12, 12, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
		OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
	font.CreateFont(15, 15, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET,
		OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
	OLD_BKMODE = pDC->SetBkMode(TRANSPARENT);
	(CBrush*)pDC->SelectStockObject(NULL_BRUSH);

	//Mask
	pDC->SelectObject(&penOK);
	pDC->Ellipse(viewer_width / 2 - radius, viewer_height / 2 - radius, viewer_width / 2 + radius, viewer_height / 2 + radius);

	//ROI
	pDC->SelectObject(&penROI);
	CRect InspROI = CRect(viewer_width / 2 - roiOffset, viewer_height / 2 - roiOffset, viewer_width / 2 + roiOffset, viewer_width / 2 + roiOffset);
	pDC->Rectangle(CRect(InspROI.left, InspROI.top, InspROI.right, InspROI.bottom));

	//Found Blob
	oldFont = pDC->SelectObject(&fontBolb);
	pDC->SelectObject(&penNG);
	pDC->SetTextColor(COLOR_RED);

	//2022-10-20 KJH2 Scratch 결과 이미지 저장
	cv::circle(result_img, cv::Point(viewer_width / 2, viewer_height / 2), radius, cv::Scalar(0, 255, 0), 2);
	cv::rectangle(result_img, cv::Point(viewer_width / 2 - (radius * 2), viewer_height / 2 - (radius * 2)),
		cv::Point(viewer_width / 2 + (radius * 2), viewer_height / 2 + (radius * 2)),
		cv::Scalar(255, 255, 255), 2);

	for (int i = 0; i < vtBlob.size(); i++)
	{
		CRect roi;
		roi.left = vtBlob[i]->left + InspROI.left;
		roi.top = gray.rows - (vtBlob[i]->top + InspROI.top);
		roi.right = vtBlob[i]->right + InspROI.left;
		roi.bottom = gray.rows - (vtBlob[i]->bottom + InspROI.top);

		pDC->Rectangle(CRect(roi.left, roi.bottom, roi.right, roi.top));
		//2022-10-20 KJH2 Scratch 결과 이미지 저장
		cv::rectangle(result_img, cv::Point(roi.left, roi.top), cv::Point(roi.right, roi.bottom), cv::Scalar(0, 0, 255), 2);
		str.Format("x:%.3fmm, y:%.3fmm", fabs(roi.Width()) * xres, fabs(roi.Height()) * yres);
		pDC->TextOutA(roi.right, roi.bottom, str);
		cv::putText(result_img, std::string(str), cv::Point(roi.right, roi.bottom), cv::FONT_ITALIC, 0.7,
			cv::Scalar(0, 0, 255), 2);
	}

	//spec
	oldFont = pDC->SelectObject(&font);
	if (vtBlob.size() == 0)	pDC->SetTextColor(COLOR_GREEN);
	else					pDC->SetTextColor(COLOR_RED);
	double SpecSize = pMain->vt_job_info[nJob].model_info.getInspSpecParaInfo().getScratchInspSpec() * xres * xres;
	str.Format("spec size : %.4fmm^2", SpecSize);
	pDC->TextOutA(100, 20, str);

	// YCS 2022-11-19
	CString strViewName;
	int nViewer = pMain->vt_job_info[nJob].viewer_index[1];
	strViewName.Format("%s", pMain->vt_viewer_info[nViewer].viewer_name.c_str());
	pView->GetViewer().AddHardGraphic(new GraphicLabel(10, 10, strViewName, Gdiplus::Color(0xff00ff00), Anchor::XCenterMiddleBottom));
	cv::putText(result_img, std::string(str), cv::Point(100, 35), cv::FONT_ITALIC, 1, cv::Scalar(0, 0, 255), 3);
	//pView->GetViewer().Invalidate();
	pView->GetViewer().DirtyRefresh();
	// =====================================================================================================================

	CString	Time_str = pMain->m_strResultTime[nJob];
	CString	Date_str = pMain->m_strResultDate[nJob];
	CString	real_Time_str, str_JobName;

	CTime NowTime;
	NowTime = CTime::GetCurrentTime();
	real_Time_str.Format("%04d%02d%02d", NowTime.GetYear(), NowTime.GetMonth(), NowTime.GetDay());

	if (Time_str == "" || Date_str == "")
	{
		pMain->m_strResultDate[nJob].Format("%04d%02d%02d", NowTime.GetYear(), NowTime.GetMonth(), NowTime.GetDay());
		pMain->m_strResultTime[nJob].Format("%02d_%02d_%02d", NowTime.GetHour(), NowTime.GetMinute(), NowTime.GetSecond());

		Time_str = pMain->m_strResultTime[nJob];
		Date_str = pMain->m_strResultDate[nJob];
	}

	CString Cell_Name = pMain->vt_job_info[nJob].get_main_object_id();
	if (strlen(Cell_Name) < 10)					Cell_Name.Format("Test_%s", Time_str);

	CString strFileDir_Image, strFileDir_Image_Proc, strFileDir_Image_Proc_Raw;

	if (vtBlob.size() != 0)
	{
		strFileDir_Image = pMain->m_strImageDir + Date_str;
		if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);
		strFileDir_Image.Format("%s\\%s", strFileDir_Image, pMain->m_strCurrentModelName);
		if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);

		//JOB이름
		str_JobName.Format("%s", pMain->vt_job_info[nJob].job_name.c_str());
		strFileDir_Image.Format("%s\\%s", strFileDir_Image, str_JobName);
		if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);

		strFileDir_Image.Format("%s\\SCRATCH_INSP\\", strFileDir_Image);
		if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);

		if (pMain->m_bSimulationStart)
		{
			strFileDir_Image.Format("%s\\Simulation\\", strFileDir_Image);
			if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);
		}
		strFileDir_Image.Format("%s%s\\", strFileDir_Image, Cell_Name);
		if (_access(strFileDir_Image, 0) != 0)	CreateDirectory(strFileDir_Image, NULL);

		strFileDir_Image_Proc_Raw.Format("%s%s_%s_%s_proc_imgRaw.jpg", strFileDir_Image, Cell_Name, Time_str, pMain->m_stCamInfo[ncamera].cName);
		strFileDir_Image_Proc.Format("%s%s_%s_%s_proc_img.jpg", strFileDir_Image, Cell_Name, Time_str, pMain->m_stCamInfo[ncamera].cName);

		if (_access(strFileDir_Image_Proc_Raw, 0) != 0) strFileDir_Image_Proc_Raw.Format("%s%s_%s_%s_proc_ImgRaw.jpg", strFileDir_Image, Cell_Name, real_Time_str, pMain->m_stCamInfo[ncamera].cName);
		if (_access(strFileDir_Image_Proc, 0) != 0) strFileDir_Image_Proc.Format("%s%s_%s_%s_proc_Img.jpg", strFileDir_Image, Cell_Name, real_Time_str, pMain->m_stCamInfo[ncamera].cName);
		cv::imwrite(std::string(strFileDir_Image_Proc_Raw), gray);
		//pView->GetViewer().saveScreenCapture(strFileDir_Image_Proc);

		//2022-10-20 KJH2 Scratch 결과 이미지 저장
		cv::imwrite(std::string(strFileDir_Image_Proc), result_img);

	}
	result_img.release();
	gray.release();
	imgSrc.release();

	return 0;
}
int CFormMainView::ELB_Error_Type(long Current_Error,int nErrorType)
{
	if ((Current_Error & (1 << nErrorType)) != 0 ) return 0;

	int nReturn = 1;
	for (int i = 0; i < nErrorType; i++)
	{
		nReturn = nReturn * 2;
	}
	return nReturn;
}

void CFormMainView::ELB_RawImageBMinInspectionMode(cv::Mat& scrImg,int _nCam, int _nJob)
{
	if (m_pMain->vt_job_info[_nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
	{
		cvShowImage("Orinal Insp Image(WetoutInsp)", scrImg);
	}

	int W = scrImg.cols;
	int H = scrImg.rows;
	int method = m_pMain->vt_job_info[_nJob].model_info.getAlignInfo().getMarkFindMethod();

	if (method == METHOD_CIRCLE)
	{
		cv::Mat img, binOrg, bin;
		cv::Mat maskerode = cv::getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(3, 3), cv::Point(1, 1));
		cv::Rect inspRoi = cv::Rect(0, 0, W, H);

		m_pMain->m_pDlgCaliper->m_Caliper[_nCam][0][C_CALIPER_POS_2].processCaliper_circle(scrImg.ptr(), W, H, 0.0, 0.0, 0.0, TRUE);

		BOOL bFindLine = m_pMain->m_pDlgCaliper->m_Caliper[_nCam][0][C_CALIPER_POS_2].getIsMakeLine();
		sCircle _circle = m_pMain->m_pDlgCaliper->m_Caliper[_nCam][0][C_CALIPER_POS_2].m_circle_info;

		int filter = m_pMain->vt_job_info[_nJob].model_info.getInspSpecParaInfo().getCCFindFilter();
		int aOffset = 1024, colC, rowC, datasheet=0;
		int nInX=0, nInY=0;
		double x, y;

		if (bFindLine)
		{
			m_dCircleRadius_CC = _circle.r;
			m_ptCircleCenter_CC = CPoint(cvRound(_circle.x), cvRound(_circle.y));		

			inspRoi = cv::Rect(MAX(0, m_ptCircleCenter_CC.x - aOffset), MAX(0, m_ptCircleCenter_CC.y - aOffset), aOffset * 2, aOffset * 2);  
			inspRoi.width = MIN(W - inspRoi.x, inspRoi.width);
			inspRoi.height = MIN(H - inspRoi.y, inspRoi.height);
			inspRoi.width = inspRoi.width / 16 * 16;
			inspRoi.height = inspRoi.height / 16 * 16;

			scrImg(inspRoi).copyTo(img);

			if (m_pMain->vt_job_info[_nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
			{
				cvShowImage("BMInspection_Image", img);
			}

			cv::threshold(img, binOrg, filter, 255, CV_THRESH_BINARY);

			erode(binOrg, binOrg, maskerode, cv::Point(-1, -1), 2);
			dilate(binOrg, bin, maskerode, cv::Point(-1, -1), 6);
			erode(bin, bin, maskerode, cv::Point(-1, -1), 4);

			if (m_pMain->vt_job_info[_nJob].model_info.getInspSpecParaInfo().getInspectionDebugCheck())
			{
				cvShowImage("BMInspection_BinImage", bin);
			}

			colC = aOffset;
			rowC = aOffset;

			BYTE* Raw = bin.data;
			double D_BM_In=0, D_BM_In_CC=0, BM_InWetout=0;
			cv::Point2f p_CCCenter = cv::Point2f(m_ptCircleCenter_CC.x - inspRoi.x, m_ptCircleCenter_CC.y - inspRoi.y);
			cv::Point2f p_ImgCenter = cv::Point2f(colC, rowC);

			for (int angle = 0; angle < 360; angle++)
			{
				x = (cos(ToRadian(angle)) * m_dCircleRadius_CC + colC);
				y = (sin(ToRadian(angle)) * m_dCircleRadius_CC + rowC);

				for (int inc = 10; inc < colC; inc++)
				{
					nInX = (cos(ToRadian(angle)) * inc + x);
					nInY = (sin(ToRadian(angle)) * inc + y);

					if (nInX > 0 && nInX < aOffset*2 && nInY>0 && nInY < aOffset*2)
					{
						if (Raw[nInY * bin.cols + nInX] < 100)
						{
							CPointF<int> p__bm(nInX, nInY);
							CPointF<double> p__raw(x, y);
							D_BM_In = GetDistance(p_ImgCenter, p__bm);
							D_BM_In_CC = GetDistance(p_CCCenter, p__bm);
							BM_InWetout = GetDistance(p__raw, p__bm);
							break;
						}
					}
				}

				m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[0].at(angle) = (CIRCLE_RADIUS);							// 중심에서 wetout
				m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[1].at(angle) = (CIRCLE_RADIUS);							// 중심에서 metal
				m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[2].at(angle) = ((angle + 90) % 360);				// 화면 기준 Angle
				m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[3].at(angle) = (inspRoi.x + nInX);							// OUT X좌표
				m_pMain->m_ELB_DiffInspResult[datasheet].m_vTraceProfile[4].at(angle) = (inspRoi.y + nInY);							// OUT Y좌표
				m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[3].at(angle) = (inspRoi.x + nInX);						// IN X좌표
				m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[4].at(angle) = (inspRoi.y + nInY);						// IN Y좌표
				m_pMain->m_ELB_DiffInspResult[datasheet + 1].m_vTraceProfile[2].at(angle) = (CIRCLE_RADIUS);				// CINK DISTANCE
				m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[0].at(angle) = (D_BM_In);						// BM_In 거리
				m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[1].at(angle) = (BM_InWetout);			    // BM_In <-> Wetout 거리
				m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[2].at(angle) = (D_BM_In_CC);					// BM_IN 거리(CC중앙기준)
				m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[3].at(angle) = (inspRoi.x + x);							// BM_In X 좌표
				m_pMain->m_ELB_DiffInspResult[datasheet + 2].m_vTraceProfile[4].at(angle) = (inspRoi.y + y);							// BM_In Y 좌표
			}
		}

		img.release();
		binOrg.release();
		bin.release();
		maskerode.release();
	}
}

//SJB 2022-10-24 캘리브레이션 뷰어 Resolution 즉시 적용
void CFormMainView::SetResCalibration()
{
	for (int i = 0; i < m_pMain->vt_viewer_info.size(); i++)
	{
		//m_pDlgViewerMain[i]->GetViewer().setCameraResolutionX(m_pMain->vt_viewer_info[i].resolution_y);
		//m_pDlgViewerMain[i]->GetViewer().setCameraResolutionY(m_pMain->vt_viewer_info[i].resolution_x);
	}
}