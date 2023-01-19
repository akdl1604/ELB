#include "stdafx.h"
#include "ScanInspWorker.h"
#include "LET_AlignClient.h"
#include "FormMainView.h"
#include "LET_AlignClientDlg.h"
#include "ImageProcessing/MathUtil.h"

CScanInspWorker::CScanInspWorker(void)
{
	m_pMainForm = NULL;
	m_pM = NULL;
	InitializeCriticalSection(&m_csScanWrite);
}

CScanInspWorker::~CScanInspWorker(void)
{
	DeleteCriticalSection(&m_csScanWrite);
}

BOOL CScanInspWorker::algorithm_WetOut_OnesInsp(BYTE* pImage, int nJob, int nCam, int W, int H, CViewerEx* pView, BOOL bManual)
{
	if (m_pM == NULL) return FALSE;

	FakeDC* pDC = NULL;
	BOOL bNG = TRUE;
	CString strText;

	CFormMainView* pFormMain = (CFormMainView*)m_pMainForm;
	CLET_AlignClientDlg* pDlg = (CLET_AlignClientDlg*)m_pM;

	int SeqGrabCount = pDlg->m_nSeqScanInspGrabCount[nJob];

	if (pView != NULL)
	{
		pDC = pView->getOverlayDC();
		pView->ClearOverlayDC();
		//pView->clearAllFigures();
		pDC->SetTextColor(COLOR_RED);
		pDC->SetBkMode(TRANSPARENT);
	}

	int nPos = 0;
	double xres = pDlg->GetMachine(nJob).getCameraResolutionX(nCam, 0);
	double yres = pDlg->GetMachine(nJob).getCameraResolutionY(nCam, 0);
	int x = pDlg->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeX() / xres / 2; //  중심점 기준 좌우 분리
	int y = pDlg->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeY() / yres / 2;  //  중심점 기준 상하 분리
	double l = pDlg->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeLength() / yres / 2; //도포 길이

	cv::Mat* srcPtr;
	cv::Mat img(H, W, CV_8UC1, pImage);
	cv::Mat dstImg = img.clone();
	cv::Mat First_dstImg;
	cv::Mat Second_dstImg;
	cv::Mat Sobel_dstlmg;

	srcPtr = &dstImg;

	int camW = pDlg->m_stCamInfo[nCam].w;
	int camH = pDlg->m_stCamInfo[nCam].h;

	//KJH 2021-07-27 검사 파라미터로 뺄거 미리 작업하자

	int LeftResultTextGap		= 150;
	int RightResultTextGap		= 30;
	double StartRangeJudgeValue = pDlg->vt_job_info[nJob].model_info.getInspSpecParaInfo().getStartSerchPoint(SeqGrabCount - 1) / xres;
	double EndRangeJudgeValue	= pDlg->vt_job_info[nJob].model_info.getInspSpecParaInfo().getEndSerchPoint(SeqGrabCount - 1) / xres;
	int SearchValueOffset_In	= pDlg->vt_job_info[nJob].model_info.getInspSpecParaInfo().getSerchMinMaxJudgeOffsetIn(SeqGrabCount - 1);
	int SearchValueOffset_Out	= pDlg->vt_job_info[nJob].model_info.getInspSpecParaInfo().getSerchMinMaxJudgeOffsetOut(SeqGrabCount - 1);
	int Image_Thresh_Vaule		= pDlg->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionHighThresh();
	int Limit_Line_Insp_Offset	= pDlg->vt_job_info[nJob].model_info.getInspSpecParaInfo().getLimitLineInspOffset(SeqGrabCount - 1);

	//KJH 2021-11-08 UT INSP 조건 추가
	BOOL method_select = pDlg->vt_job_info[0].model_info.getAlignInfo().getUseSubInspAlgorithm();

	//KJH 2021-08-11 Panel ID 미리 빼두자
	pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_strPanelID = pDlg->vt_job_info[nCam].main_object_id.c_str();

	// 거리 측정 클리어
	for (int i = 0; i < 6; i++) pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[i].clear();

	int cx = W / 2, cy = H / 2;
	double ax	= 1, bc		= 0,	dist	= 0,	dist2	= 0;
	double ax2	= 1, bc2	= 0;
	bool bsplit = (camH < H) ? true: false;

	if (bsplit)
	{
		if (pDlg->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeLength() < 56)
			l = 60.0 / yres / 2;
	}
	else
	{
		if (pDlg->vt_job_info[nJob].model_info.getAlignInfo().getInspRangeLength() < 6)
			l = 10.0 / yres / 2;
	}

	CPoint ptStart	= CPoint(MAX(4, cx - y), MAX(4, cy - l));
	CPoint ptEnd	= CPoint(MIN(W - 4, cx + y), MIN(H - 4, cy + l));

#pragma region 영상 방향 결정
	if (pImage != NULL)
	{
		//KJH 2021-07-29 오버레이용 디스플레이 추가
		pDlg->m_matScanResultImage[nCam][SeqGrabCount].release();

		int xw = camW;
		int yh = camH * (SeqGrabCount == 1 ? _SCAN_GRAB_MAXCOUNT: 1);

		//BYTE* data = SeqGrabCount == 1 ? m_pSaperaCam[nCam].GetFullImage()->data: getProcBuffer(nCam, 0);

		cv::Mat matImage_View(yh, xw, CV_8UC1, pImage);
		if (bsplit)
		{
			//모션 변경으로 인한 이미지 변환
			rot90(matImage_View, 2); //1. 반시계 방향 후 상하 반전
			flip(matImage_View, matImage_View, 0);
		}
		else
		{
			rot90(matImage_View, 2); //반시계 방향 회전
		}
		matImage_View(cv::Rect(0, 0, yh, xw)).copyTo(pDlg->m_matScanResultImage[nCam][SeqGrabCount]);
		cv::cvtColor(matImage_View, pDlg->m_matScanResultImage[nCam][SeqGrabCount], COLOR_GRAY2BGR);
		
		if (pDlg->vt_job_info[nJob].model_info.getAlignInfo().getUseRealTimeTrace())
		{
			CString strfile;
			strfile.Format("D:\\m_matScanResultImage_%d_%d.jpg", nCam, SeqGrabCount);
			imwrite(std::string(strfile), pDlg->m_matScanResultImage[nCam][SeqGrabCount]);
		}
		//나중에는 ResultImage로 Viewer에 쓰자
	}
#pragma endregion

#pragma region (캘리퍼를 이용한 Panel Edge 찾기 및 Roi 설정)

	pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_InspRoi[nPos] = CRect(ptStart, ptEnd);
	pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x = -1;
	pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y = -1;
	pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint_Index.x = 0;
	pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint_Index.y = 0;
	pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_bResultSummary = TRUE;

	if (ptEnd.x - ptStart.x > 0 && ptEnd.y - ptStart.y > 0)
	{
		BOOL bFindLine[4];
		sLine line_info[4];
		double dx = 0.0, dy = 0.0, dt = 0.0, posx = 0.0, posy = 0.0;
		int bstart = MAX(0, 0 + (pDlg->m_nSeqScanInspGrabCount[nJob] - 1) * 2);

		if (bsplit)
		{
			cv::Mat srcImg(H, W, CV_8UC1, pImage);

			cv::Mat imgCut_NUM1_POS = srcImg(cv::Rect(0, 0, camW, camH));
			//cv::threshold(imgCut_NUM1_POS, Sobel_dstlmg, Image_Thresh_Vaule, 255, CV_THRESH_BINARY);
			sobelDirection(3, SOBEL_RIGHT, imgCut_NUM1_POS, Sobel_dstlmg);

			if (pDlg->vt_job_info[nJob].model_info.getAlignInfo().getUseRealTimeTrace())
			{
				CString strTemp;
				strTemp.Format("D:\\Caliper_%d_%d_0.jpg", nCam, pDlg->m_nSeqScanInspGrabCount[nJob]);
				cv::imwrite(std::string(strTemp), Sobel_dstlmg);
			}
			pDlg->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_1 + bstart].processCaliper(Sobel_dstlmg.data, W, H, dx, dy, dt, TRUE);

			//KJH 2021-11-08 UT INSP 조건 추가
			if (method_select)
			{
				pDlg->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_3 + bstart].processCaliper(imgCut_NUM1_POS.data, W, H, dx, dy, dt, TRUE);
			}

			cv::Mat imgCut_MAX_POS = srcImg(cv::Rect(0, (_SCAN_GRAB_MAXCOUNT - 1) * camH, camW, camH));
			//cv::threshold(imgCut_MAX_POS, Sobel_dstlmg, Image_Thresh_Vaule, 255, CV_THRESH_BINARY);
			sobelDirection(3, SOBEL_RIGHT, imgCut_MAX_POS, Sobel_dstlmg);

			if (pDlg->vt_job_info[nJob].model_info.getAlignInfo().getUseRealTimeTrace())
			{
				CString strTemp;
				strTemp.Format("D:\\Caliper_%d_%d_1.jpg", nCam, pDlg->m_nSeqScanInspGrabCount[nJob]);
				cv::imwrite(std::string(strTemp), Sobel_dstlmg);
			}
			pDlg->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_2 + bstart].processCaliper(Sobel_dstlmg.data, camW, camH, dx, dy, dt, TRUE);

			//KJH 2021-11-08 UT INSP 조건 추가
			if (method_select)
			{
				pDlg->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_4 + bstart].processCaliper(imgCut_MAX_POS.data, W, H, dx, dy, dt, TRUE);
			}
		}
		else
		{
			cv::Mat srcImg(H, W, CV_8UC1, pImage);
			//cv::threshold(srcImg, Sobel_dstlmg, Image_Thresh_Vaule, 255, CV_THRESH_BINARY);
			sobelDirection(3, SOBEL_LEFT, srcImg, Sobel_dstlmg);

			if (pDlg->vt_job_info[nJob].model_info.getAlignInfo().getUseRealTimeTrace())
			{
				CString strTemp;
				strTemp.Format("D:\\Caliper_%d_%d_1.jpg", nCam, pDlg->m_nSeqScanInspGrabCount[nJob]);
				cv::imwrite(std::string(strTemp), Sobel_dstlmg);
			}

			pDlg->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_1 + bstart].processCaliper(Sobel_dstlmg.data, W, H, dx, dy, dt, TRUE);
			pDlg->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_2 + bstart].processCaliper(Sobel_dstlmg.data, W, H, dx, dy, dt, TRUE);
		}

		line_info[0] = pDlg->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_1 + bstart].m_lineInfo;
		bFindLine[0] = pDlg->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_1 + bstart].getIsMakeLine();

		//KJH 2021-11-08 UT INSP 조건 추가
		if (method_select)
		{
			line_info[1] = pDlg->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_2 + bstart].m_lineInfo;
			bFindLine[1] = pDlg->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_2 + bstart].getIsMakeLine();
			line_info[2] = pDlg->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_3 + bstart].m_lineInfo;
			bFindLine[2] = pDlg->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_3 + bstart].getIsMakeLine();
			line_info[3] = pDlg->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_4 + bstart].m_lineInfo;
			bFindLine[3] = pDlg->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_4 + bstart].getIsMakeLine();
		}
		else
		{
			line_info[1] = pDlg->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_2 + bstart].m_lineInfo;
			bFindLine[1] = pDlg->m_pDlgCaliper->m_Caliper[nCam][nPos][C_CALIPER_POS_2 + bstart].getIsMakeLine();
		}

		if (bFindLine[0] || bFindLine[1])
		{
			sLine lineLeft, lineRight, lineLeftInsp, lineRightInsp;
			
			//KJH 2021-11-08 UT INSP 조건 추가
			if (bFindLine[2] || bFindLine[3])
			{
				lineLeftInsp = line_info[2];
				lineRightInsp = line_info[3];
			}
			else
			{
				lineLeftInsp = line_info[0];
				lineRightInsp = line_info[1];
			}

			lineLeft = line_info[0];
			lineRight = line_info[1];

			if (bFindLine[0] && !bFindLine[1])
			{
				lineLeft = line_info[0];
				lineRight = line_info[0];
			}
			else if (!bFindLine[0] && bFindLine[1])
			{
				lineLeft = line_info[1];
				lineRight = line_info[1];
			}
			
			//KJH 2021-11-08 UT INSP 조건 추가
			if (bFindLine[2] && !bFindLine[3])
			{
				lineLeftInsp	= line_info[2];
				lineRightInsp	= line_info[2];
			}
			else if (!bFindLine[2] && bFindLine[3])
			{
				lineLeftInsp	= line_info[3];
				lineRightInsp	= line_info[3];
			}

			cv::Point2f pt = IntersectionPoint2(0, ptStart.y, lineLeft.a, lineLeft.b);
			cv::Point2f pt2 = IntersectionPoint2(0, ptEnd.y, lineRight.a, lineRight.b);

			cv::Point2f pt_insp_start = IntersectionPoint2(0, ptStart.y, lineLeftInsp.a, lineLeftInsp.b);
			cv::Point2f pt_insp_end	= IntersectionPoint2(0, ptEnd.y, lineRightInsp.a, lineRightInsp.b);

			if (bsplit)
			{
				pt2 = IntersectionPoint2(0, ptEnd.y - (camH * (_SCAN_GRAB_MAXCOUNT - 1)), lineRight.a, lineRight.b);
				pt2.y += (camH * (_SCAN_GRAB_MAXCOUNT - 1));

				if (method_select)
				{
					pt_insp_end = IntersectionPoint2(0, ptEnd.y - (camH * (_SCAN_GRAB_MAXCOUNT - 1)), lineRightInsp.a, lineRightInsp.b);
					pt_insp_end.y += (camH * (_SCAN_GRAB_MAXCOUNT - 1));
				}
			}

			double x1 = pt.x;				// y1 = lineLeft.a* ptStart.x + lineLeft.b;
			double x2 = pt2.x;				// y2 = lineRight.a* ptEnd.x + lineRight.b;
			double x3 = pt_insp_start.x;	// y1 = lineLeftInsp.a* ptStart.x + lineLeftInsp.b;
			double x4 = pt_insp_end.x;		// y2 = lineRightInsp.a* ptEnd.x + lineRightInsp.b;
			
			GetLineCoef(x1, ptStart.y, x2, ptEnd.y, ax, bc);

			pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_ResultLine[nPos].a		= ax;
			pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_ResultLine[nPos].b		= bc;
			pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vELB_SearchLine.x			= x1;
			pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vELB_SearchLine.y			= x2;
			
			if (method_select)
			{
				GetLineCoef(x3, ptStart.y, x4, ptEnd.y, ax2, bc2);

				pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_ResultLine_Insp[nPos].a = ax2;
				pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_ResultLine_Insp[nPos].b = bc2;
				pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vELB_SearchLine_Insp.x = x3;
				pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vELB_SearchLine_Insp.y = x4;
			}

			if (pView != NULL)
			{
				CPen pen(PS_SOLID, 3, RGB(0, 255, 0));
				pDC->SelectObject(&pen);
				COLORREF color		= COLOR_RED;
				COLORREF color_BLUE = COLOR_BLUE;
				stFigure tempFig, tempFig_Insp;

				tempFig.ptBegin.x	= int(x1);
				tempFig.ptBegin.y	= int(x1 * ax + bc);
				tempFig.ptEnd.x		= int(x2);
				tempFig.ptEnd.y		= int(x2 * ax + bc);

				if (int(x1 * ax + bc) == int(x2 * ax + bc))
				{
					tempFig.ptBegin.y = ptStart.y;
					tempFig.ptEnd.y = ptEnd.y;
				}
				//캘리퍼 그리기
				pView->addFigureLine(tempFig, 6, 4, color);
				pDC->Ellipse(int(pt.x - 4), int(pt.y - 4), int(pt.x + 4), int(pt.y + 4));
				pDC->Ellipse(int(pt2.x - 4), int(pt2.y - 4), int(pt2.x + 4), int(pt2.y + 4));
				
				if (method_select)
				{
					tempFig_Insp.ptBegin.x	= int(x3);
					tempFig_Insp.ptBegin.y	= int(x3 * ax2 + bc2);
					tempFig_Insp.ptEnd.x	= int(x4);
					tempFig_Insp.ptEnd.y	= int(x4 * ax2 + bc2);

					if (int(x3 * ax2 + bc2) == int(x4 * ax2 + bc2))
					{
						tempFig_Insp.ptBegin.y	= ptStart.y;
						tempFig_Insp.ptEnd.y	= ptEnd.y;
					}

					pView->addFigureLine(tempFig_Insp, 6, 4, color_BLUE);
					pDC->Ellipse(int(pt_insp_start.x - 4), int(pt_insp_start.y - 4), int(pt_insp_start.x + 4), int(pt_insp_start.y + 4));
					pDC->Ellipse(int(pt_insp_end.x - 4), int(pt_insp_end.y - 4), int(pt_insp_end.x + 4), int(pt_insp_end.y + 4));
				}

				if (bsplit)
				{
					cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(tempFig.ptBegin.y), int(tempFig.ptBegin.x)), cv::Point(int(tempFig.ptEnd.y), int(tempFig.ptEnd.x)), cv::Scalar(64, 64, 255), 3);
					cv::circle(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(tempFig.ptBegin.y), int(tempFig.ptBegin.x)), 2, cv::Scalar(64, 255, 255), 2, 5);
					cv::circle(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(tempFig.ptEnd.y), int(tempFig.ptEnd.x)), 2, cv::Scalar(64, 255, 255), 2, 5);
					
					if (method_select)
					{
						cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(tempFig_Insp.ptBegin.y), int(tempFig_Insp.ptBegin.x)), cv::Point(int(tempFig_Insp.ptEnd.y), int(tempFig_Insp.ptEnd.x)), cv::Scalar(255, 64, 64), 3);
						cv::circle(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(tempFig_Insp.ptBegin.y), int(tempFig_Insp.ptBegin.x)), 2, cv::Scalar(64, 255, 255), 2, 5);
						cv::circle(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(tempFig_Insp.ptEnd.y), int(tempFig_Insp.ptEnd.x)), 2, cv::Scalar(64, 255, 255), 2, 5);
					}
				}
				else
				{
					cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(tempFig.ptBegin.y), int(W - tempFig.ptBegin.x)), cv::Point(int(tempFig.ptEnd.y), int(W - tempFig.ptEnd.x)), cv::Scalar(64, 64, 255), 3);
					cv::circle(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(tempFig.ptBegin.y), int(W - tempFig.ptBegin.x)), 2, cv::Scalar(64, 255, 255), 2, 5);
					cv::circle(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(tempFig.ptEnd.y), int(W - tempFig.ptEnd.x)), 2, cv::Scalar(64, 255, 255), 2, 5);

					if (method_select)
					{
						cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(tempFig_Insp.ptBegin.y), int(W - tempFig_Insp.ptBegin.x)), cv::Point(int(tempFig_Insp.ptEnd.y), int(W - tempFig_Insp.ptEnd.x)), cv::Scalar(255, 64, 64), 3);
						cv::circle(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(tempFig_Insp.ptBegin.y), int(W - tempFig_Insp.ptBegin.x)), 2, cv::Scalar(64, 255, 255), 2, 5);
						cv::circle(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(tempFig_Insp.ptEnd.y), int(W - tempFig_Insp.ptEnd.x)), 2, cv::Scalar(64, 255, 255), 2, 5);
					}
				}
				
				pen.DeleteObject();
			}
			else
			{
				stFigure tempFig, tempFig_Insp;

				tempFig.ptBegin.x	= int(x1);
				tempFig.ptBegin.y	= int(x1 * ax + bc);
				tempFig.ptEnd.x		= int(x2);
				tempFig.ptEnd.y		= int(x2 * ax + bc);

				if (int(x1 * ax + bc) == int(x2 * ax + bc))
				{
					tempFig.ptBegin.y = ptStart.y;
					tempFig.ptEnd.y = ptEnd.y;
				}

				if (method_select)
				{
					tempFig_Insp.ptBegin.x = int(x3);
					tempFig_Insp.ptBegin.y = int(x3 * ax2 + bc2);
					tempFig_Insp.ptEnd.x = int(x4);
					tempFig_Insp.ptEnd.y = int(x4 * ax2 + bc2);

					if (int(x3 * ax2 + bc2) == int(x4 * ax2 + bc2))
					{
						tempFig_Insp.ptBegin.y = ptStart.y;
						tempFig_Insp.ptEnd.y = ptEnd.y;
					}
				}

				if (bsplit)
				{
					cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(tempFig.ptBegin.y), int(tempFig.ptBegin.x)), cv::Point(int(tempFig.ptEnd.y), int(tempFig.ptEnd.x)), cv::Scalar(64, 64, 255), 3);
					cv::circle(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(tempFig.ptBegin.y), int(tempFig.ptBegin.x)), 2, cv::Scalar(64, 255, 255), 2, 5);
					cv::circle(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(tempFig.ptEnd.y), int(tempFig.ptEnd.x)), 2, cv::Scalar(64, 255, 255), 2, 5);
					
					if (method_select)
					{
						cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(tempFig_Insp.ptBegin.y), int(tempFig_Insp.ptBegin.x)), cv::Point(int(tempFig_Insp.ptEnd.y), int(tempFig_Insp.ptEnd.x)), cv::Scalar(64, 64, 255), 3);
						cv::circle(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(tempFig_Insp.ptBegin.y), int(tempFig_Insp.ptBegin.x)), 2, cv::Scalar(64, 255, 255), 2, 5);
						cv::circle(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(tempFig_Insp.ptEnd.y), int(tempFig_Insp.ptEnd.x)), 2, cv::Scalar(64, 255, 255), 2, 5);
					}
				}
				else
				{
					cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(tempFig.ptBegin.y), int(W - tempFig.ptBegin.x)), cv::Point(int(tempFig.ptEnd.y), int(W - tempFig.ptEnd.x)), cv::Scalar(64, 64, 255), 3);
					cv::circle(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(tempFig.ptBegin.y), int(W - tempFig.ptBegin.x)), 2, cv::Scalar(64, 255, 255), 2, 5);
					cv::circle(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(tempFig.ptEnd.y), int(W - tempFig.ptEnd.x)), 2, cv::Scalar(64, 255, 255), 2, 5);
					
					if (method_select)
					{
						cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(tempFig_Insp.ptBegin.y), int(W - tempFig_Insp.ptBegin.x)), cv::Point(int(tempFig_Insp.ptEnd.y), int(W - tempFig_Insp.ptEnd.x)), cv::Scalar(64, 64, 255), 3);
						cv::circle(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(tempFig_Insp.ptBegin.y), int(W - tempFig_Insp.ptBegin.x)), 2, cv::Scalar(64, 255, 255), 2, 5);
						cv::circle(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(tempFig_Insp.ptEnd.y), int(W - tempFig_Insp.ptEnd.x)), 2, cv::Scalar(64, 255, 255), 2, 5);
					}
				}
			}

			ptStart.x	= int(MAX(0, MIN(x1, x2) - 150));
			ptEnd.x		= int(MIN(W - 1, MAX(x1, x2) + 150));

			COLORREF color = COLOR_GREEN;
			stFigure tempFig;

			pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_InspRoi[nPos] = CRect(ptStart, ptEnd);
			
			tempFig.ptBegin.x	= int(ptStart.x);
			tempFig.ptBegin.y	= int(ptStart.y);
			tempFig.ptEnd.x		= int(ptEnd.x);
			tempFig.ptEnd.y		= int(ptEnd.y);

			//Overlay만 ROI표시하기
			CRect roi = pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_InspRoi[nPos];
			if (pView != NULL)
			{
				pView->addFigureRect(tempFig, 6, 4, color);
			}

			if (bsplit)
			{
				cv::rectangle(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(roi.top, roi.left), cv::Point(roi.bottom, roi.right), CV_RGB(0, 255, 0), 5);
			}
			else
			{
				cv::rectangle(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(roi.top, W - roi.left), cv::Point(roi.bottom, W - roi.right), CV_RGB(0, 255, 0), 5);
			}
		}
		else // 에러 발생
		{
			pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_ResultLine[nPos].a = 0;
			pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_ResultLine[nPos].b = 0;
			bNG = FALSE;

			if (theApp.m_bEnforceOkFlag) bNG = TRUE;

			return bNG;
		}
#pragma endregion

#pragma region 영상처리
		//if (bsplit)
		//{
		//	sobelDirection(3, SOBEL_LEFT, dstImg, second_dstImg);
		//	sobelDirection(3, SOBEL_RIGHT, dstImg, dstImg);
		//}
		//else
		//{
		//	sobelDirection(3, SOBEL_RIGHT, dstImg, second_dstImg);
		//	sobelDirection(3, SOBEL_LEFT, dstImg, dstImg);
		//}
		Image_Thresh_Vaule = pDlg->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionHighThresh();
		cv::threshold(dstImg, dstImg, Image_Thresh_Vaule, 255, CV_THRESH_BINARY);
		//cv::threshold(dstImg, dstImg, 80, 255, CV_THRESH_BINARY | CV_THRESH_OTSU);
		//cvShowImage("Test1", dstImg, W, H);
		First_dstImg = dstImg;

		if (pDlg->vt_job_info[nJob].model_info.getAlignInfo().getUseRealTimeTrace())
		{
			CString strTemp;
			strTemp.Format("D:\\test1_%d_%d.jpg", nCam, pDlg->m_nSeqScanInspGrabCount[nJob]);
			cv::imwrite(std::string(strTemp), dstImg);
		}

		cv::GaussianBlur(dstImg, dstImg, cv::Size(0, 0), 3.);

		if (pDlg->vt_job_info[nJob].model_info.getAlignInfo().getUseRealTimeTrace())
		{
			CString strTemp;
			strTemp.Format("D:\\test2_%d_%d.jpg", nCam, pDlg->m_nSeqScanInspGrabCount[nJob]);
			cv::imwrite(std::string(strTemp), dstImg);
		}
		Second_dstImg = dstImg;
#pragma endregion

#pragma region Dopo 영역 찾기(캘리퍼를 이용한 각종 거리 측정)
		const uchar* dataptr = srcPtr->data;
		const uchar* second_dataptr = Second_dstImg.data;
		int thresh = pDlg->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspectionLowThresh();;
		bool blfind, brfind;

		for (int j = ptStart.y; j < ptEnd.y; j++)
		{
			if (j < 0 || H <= j) continue;
			
			cv::Point2f pt		= IntersectionPoint2(0, j, ax, bc);
			cv::Point2f pt_Insp	= IntersectionPoint2(0, j, ax2, bc2);

			if (ax == 0 && bc == 0)
			{
				pt.x = pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vELB_SearchLine.x;
			}
			
			if (ax2 == 0 && bc2 == 0)
			{
				pt_Insp.x = pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vELB_SearchLine_Insp.x;
			}
		
			dist2 = dist = x = pt.x;
			blfind = false, brfind = false;
			
			if (bsplit)
			{
				for (int i = x + 4; i < ptEnd.x; i++)
				{
					if (i < 0 || W <= i) continue;
					if (dataptr[j * W + i] > thresh) // 기준 Thresh 배경이 white 인경우//  임시 사용
					{
						dist = i;
						brfind = true;
						break;
					}
				}
				for (int i = x - 4; i > ptStart.x; i--)
				{
					if (i < 0 || W <= i) continue;
					if (second_dataptr[j * W + i] > thresh) // 기준 Thresh 배경이 white 인경우//  임시 사용
					{
						dist2 = i;
						blfind = true;
						break;
					}
				}
				if (!brfind) dist = ptEnd.x;
				if (!blfind) dist2 = ptStart.x;
				if (!brfind || !blfind)  pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_bResultSummary = FALSE;
			}
			else
			{
				for (int i = x - 4; i > ptStart.x; i--)
				{
					if (i < 0 || W <= i) continue;
					if (dataptr[j * W + i] > thresh) // 기준 Thresh 배경이 white 인경우//  임시 사용
					{
						dist = i;
						blfind = true;
						break;
					}
				}
				for (int i = x + 4; i < ptEnd.x; i++)
				{
					if (i < 0 || W <= i) continue;
					if (second_dataptr[j * W + i] > thresh) // 기준 Thresh 배경이 white 인경우//  임시 사용
					{
						dist2 = i;
						brfind = true;
						break;
					}
				}

				if (!brfind) dist2 = ptEnd.x;
				if (!blfind) dist = ptStart.x;
				if (!brfind || !blfind)  pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_bResultSummary = FALSE;
			}

			//도포 Start, End 찾기 - 주석처리(2021-08-11)
			//if (pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x == -1)
			//{
			//	//만큼 떨어지면 그점을 검사 Start로 본다..
			//	if (fabs(dist - pt.x) > StartRangeJudgeValue)
			//	{
			//		pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x = j;
			//	}
			//}
			//else
			//{
			//	if (pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y == -1)
			//	{
			//		//StartRangeJudgeValue = 1;
			//		//보다 작아지면 그점을 검사 End로 본다..
			//		if (fabs(dist - pt.x) < EndRangeJudgeValue)
			//		{
			//			pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y = j;
			//		}
			//	}
			//}

			//계산값 기록하기 (IN)

			pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].push_back(j);
			pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[1].push_back(dist);
			pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[2].push_back(fabs(dist - x) * xres);

			//계산값 기록하기 (OUT)

			pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[3].push_back(dist2);
			pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[4].push_back(fabs(dist2 - x) * xres);

			//계산값 기록하기 (Insp Limit Line)

			pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[5].push_back(pt_Insp.x);
		}

#pragma region Dopo Start,End찾기
		//양끝선 다시 판단하기
		//Start 찾기
		int _conCount = 0; // 연속 갯수 찾기
		int _JudgeCount = 30;
		for (int j = 0; j <int(pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[2].size() - 1); j++)
		{
			if (pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[2].at(j) / xres > StartRangeJudgeValue)
			{
				_conCount++;
			}
			else _conCount = 0;

			if (_conCount > _JudgeCount)
			{
				//Dopo Start Pixel 값 저장
				pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x = pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].at(j - _JudgeCount);
				pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint_Index.x = j - _JudgeCount;
				break;
			}
		}
		
		//End 찾기
		_conCount = 0;
		for (int j = int(pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[2].size() - 1); j >= 0; j--)
		{
			if (pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[2].at(j) / xres > EndRangeJudgeValue)
			{
				_conCount++;
			}
			else _conCount = 0;

			if (_conCount > _JudgeCount)
			{
				//Dopo End Pixel 값 저장
				pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y = pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].at(j + _JudgeCount);
				pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint_Index.y = j + _JudgeCount;
				break;
			}
		}
#pragma endregion

		int nDopo_Start	=	pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x;
		int nDopo_End	=	pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y;
		int Center_Index = 0;

		if (nDopo_Start < 0)
		{
			nDopo_Start = 0;
		}		
		if (nDopo_End < 0)
		{
			nDopo_End = 0;
		}
#pragma region Dopo Center 찾기
		//도포 중심의 도포 길이 판단하기
		//도포 중심의 Index 값 저장
		//Center Index 저장
		pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_Result_Center_XWidth_in.x	= (int)(nDopo_Start + nDopo_End) / 2;	// 도포의 중심점 Index
		pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_Result_Center_XWidth_out.x	= (int)(nDopo_Start + nDopo_End) / 2;	// 도포의 중심점 Index
		
		Center_Index = pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_Result_Center_XWidth_in.x;
		if (Center_Index < 0)
		{
			Center_Index = 0;
		}
		pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_Result_Center_XWidth_in.y	= pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[2].at(Center_Index);
		pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_Result_Center_XWidth_out.y	= pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[4].at(Center_Index);

#pragma endregion
		
		int Point1_Index = 0;
		int Point2_Index = 0;

#pragma region Dopo Point1 찾기
		//Point 1 = Dopo Start + 70 pixel 위치값
		//예외처리  Dopo Start + 70 이 Center Index보다 클경우(길이에 문제가 있는거지만 우선 Dopo Start의 좌표값으로 계산)
		
		Point1_Index = pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint_Index.x + 70;

		if (Point1_Index > Center_Index)
		{
			Point1_Index = nDopo_Start;
		}
		pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_Result_Point1_XWidth_in.x	= Point1_Index;
		pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_Result_Point1_XWidth_out.x	= Point1_Index;
		pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_Result_Point1_XWidth_in.y	= pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[2].at(Point1_Index);
		pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_Result_Point1_XWidth_out.y	= pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[4].at(Point1_Index);;

#pragma endregion
#pragma region Dopo Point2 찾기
		//Point 2 = Dopo End - 70 pixel 위치값
		//예외처리  Dopo End - 70 이 Center Index보다 클경우(검사 실패로 판단)

		Point2_Index = pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint_Index.y - 70;

		if (Point2_Index < Center_Index)
		{
			Point2_Index = nDopo_End;
		}
		pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_Result_Point2_XWidth_in.x		= Point2_Index;
		pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_Result_Point2_XWidth_out.x		= Point2_Index;
		pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_Result_Point2_XWidth_in.y		= pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[2].at(Point2_Index);
		pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_Result_Point2_XWidth_out.y		= pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[4].at(Point2_Index);

#pragma endregion

		//이물검사를 위한 ROI 설정
		// Dopo Start	=  RECT.TOP
		// Dopo END		=  RECT.BOTTOM
		// Dopo END		=  RECT.LEFT
		// Dopo END		=  RECT.RIGHT

#pragma endregion

#pragma region 시뮬레이션 화면 그리기
		if (pView != NULL && pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].size() > 10)
		{
			double xa, ya;
			double xa2, ya2;
			double xa3;

			CPen penRight(PS_SOLID, 3, COLOR_GREEN);
			CPen penLeft(PS_SOLID, 3, RGB(255, 0, 255));
			CPen penOK(PS_SOLID, 3, COLOR_YELLOW);
			CPen penRed(PS_SOLID, 3, COLOR_RED);

#pragma region 시뮬레이션 In 표시			

			pDC->SelectObject(&penRight);
			pDC->SetTextColor(RGB(255, 18, 18));

			//KJH 2021-07-27	Start 찾은점과 End 찾은점으로 영역 변경
			//pDC->MoveTo( m_ELB_DiffInspResult[nCam].m_vTraceProfile[1].at(0),  m_ELB_DiffInspResult[nCam].m_vTraceProfile[0].at(0));
			//In Result Overlay Display Start - In방향 초록색 외각선 표시
			for (int i = 1; i < pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].size(); i++)
			{
				xa	= pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[1].at(i);
				ya	= pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].at(i);
				xa2 = pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[1].at(i - 1);
				ya2 = pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].at(i - 1);
				xa3 = pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[5].at(i);

				//KJH 2021-07-27	Start 찾은점과 End 찾은점으로 영역 변경
				if (ya < nDopo_Start || ya > nDopo_End)
				{
					pDC->MoveTo(xa, ya);
				}
				else
				{
					if (xa > xa3 + Limit_Line_Insp_Offset)
					{
						pDC->SelectObject(&penLeft);
					}
					else
					{
						pDC->SelectObject(&penRight);
					}

					pDC->MoveTo(xa2, ya2);
					pDC->LineTo(xa, ya);

					//Center Point 그림그리기
					if (i == Center_Index)
					{
						pDC->SelectObject(&penOK);
						pDC->MoveTo(xa - 20, ya);
						pDC->LineTo(xa + 20, ya);
						pDC->MoveTo(xa, ya);
						pDC->SelectObject(&penRight);
						
						if (bsplit)
						{
							strText.Format("[Center] %.4f mm", pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[2].at(i));
							pDC->TextOutA(xa + RightResultTextGap, ya, strText);
							cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(ya), int(xa - 20)), cv::Point(int(ya), int(xa + 20)), cv::Scalar(64, 255, 255), 3);
						}
						else
						{
							strText.Format("[Center] %.4f mm", pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[2].at(i));
							if (xa - LeftResultTextGap > 0)
							{
								pDC->TextOutA(xa - LeftResultTextGap, ya, strText);
							}
							else
							{
								pDC->TextOutA(0, ya, strText);
							}
							cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(ya), int(W - xa - 20)), cv::Point(int(ya), int(W - xa + 20)), cv::Scalar(64, 255, 255), 3);
						}
					}
					//Point 1 그림그리기
					if (i == Point1_Index)
					{
						pDC->SelectObject(&penOK);
						pDC->MoveTo(xa - 20, ya);
						pDC->LineTo(xa + 20, ya);
						pDC->MoveTo(xa, ya);
						pDC->SelectObject(&penRight);

						if (bsplit)
						{
							strText.Format("[Point 1] %.4f mm", pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[2].at(i));
							pDC->TextOutA(xa + RightResultTextGap, ya, strText);
							cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(ya), int(xa - 20)), cv::Point(int(ya), int(xa + 20)), cv::Scalar(64, 255, 255), 3);
						}
						else
						{
							strText.Format("[Point 1] %.4f mm", pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[2].at(i));
							if (xa - LeftResultTextGap > 0)
							{
								pDC->TextOutA(xa - LeftResultTextGap, ya, strText);
							}
							else
							{
								pDC->TextOutA(0, ya, strText);
							}
							cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(ya), int(W - xa - 20)), cv::Point(int(ya), int(W - xa + 20)), cv::Scalar(64, 255, 255), 3);
						}
					}
					//Point 2 그림그리기
					if (i == Point2_Index)
					{
						pDC->SelectObject(&penOK);
						pDC->MoveTo(xa - 20, ya);
						pDC->LineTo(xa + 20, ya);
						pDC->MoveTo(xa, ya);
						pDC->SelectObject(&penRight);

						if (bsplit)
						{
							strText.Format("[Point 2] %.4f mm", pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[2].at(i));
							pDC->TextOutA(xa + RightResultTextGap, ya, strText);
							cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(ya), int(xa - 20)), cv::Point(int(ya), int(xa + 20)), cv::Scalar(64, 255, 255), 3);
						}
						else
						{
							strText.Format("[Point 2] %.4f mm", pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[2].at(i));
							if (xa - LeftResultTextGap > 0)
							{
								pDC->TextOutA(xa - LeftResultTextGap, ya, strText);
							}
							else
							{
								pDC->TextOutA(0, ya, strText);
							}
							cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(ya), int(W - xa - 20)), cv::Point(int(ya), int(W - xa + 20)), cv::Scalar(64, 255, 255), 3);
						}
					}

					//연결선 그리기
					if (SeqGrabCount == 1)
					{
						if (xa > xa3 + Limit_Line_Insp_Offset)
						{
							cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(ya2), int(xa2)), cv::Point(int(ya), int(xa)), cv::Scalar(255, 0, 255), 3);
						}
						else
						{
							cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(ya2), int(xa2)), cv::Point(int(ya), int(xa)), cv::Scalar(64, 255, 64), 3);
						}
					}
					else
					{
						cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(ya2), int(W - xa2)), cv::Point(int(ya), int(W - xa)), cv::Scalar(64, 255, 64), 3);
					}

					//시뮬레이션 거리값 확인용
					if (i % 100 == 0)
					{
						if ((pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x < i && pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x != -1) ||
							(pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y != -1 && pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y > i))
						{
							if (bsplit)
							{
								strText.Format("[IN] %.4f mm", pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[2].at(i));
								pDC->TextOutA(xa + RightResultTextGap, ya, strText);
							}
							else
							{
								strText.Format("[IN] %.4f mm", pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[2].at(i));
								if (xa - LeftResultTextGap > 0)
								{
									pDC->TextOutA(xa - LeftResultTextGap, ya, strText);
								}
								else
								{
									pDC->TextOutA(0, ya, strText);
								}
							}

						}
					}
				}
			}
			//In Result Overlay Display End
#pragma endregion
#pragma region 시뮬레이션 Out 표시
			pDC->SelectObject(&penLeft);
			//KJH 2021-07-27	Start 찾은점과 End 찾은점으로 영역 변경
			//pDC->MoveTo( m_ELB_DiffInspResult[nCam].m_vTraceProfile[1].at(0),  m_ELB_DiffInspResult[nCam].m_vTraceProfile[0].at(0));
			//Out Result Overlay Display Start - Out방향 보라색 외각선 표시
			for (int i = 1; i < pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].size(); i++)
			{
				xa = pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[3].at(i);
				ya = pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].at(i);
				xa2 = pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[3].at(i - 1);
				ya2 = pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].at(i - 1);

				//KJH 2021-07-27	Start 찾은점과 End 찾은점으로 영역 변경
				if (ya < nDopo_Start || ya > nDopo_End)
				{
					pDC->MoveTo(xa, ya);
				}
				else
				{
					pDC->MoveTo(xa2, ya2);
					pDC->LineTo(xa, ya);
					
					//Center Point 그림그리기
					if (i == Center_Index)
					{
						pDC->SelectObject(&penOK);
						pDC->MoveTo(xa - 20, ya);
						pDC->LineTo(xa + 20, ya);
						pDC->MoveTo(xa, ya);
						pDC->SelectObject(&penLeft);

						if (bsplit)
						{
							cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(ya), int(xa - 20)), cv::Point(int(ya), int(xa + 20)), cv::Scalar(64, 255, 255), 3);
							strText.Format("[Center] %.4f mm", pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[4].at(i));
							if (xa - LeftResultTextGap > 0)
							{
								pDC->TextOutA(xa - LeftResultTextGap, ya, strText);
							}
							else
							{
								pDC->TextOutA(0, ya, strText);
							}
						}
						else
						{
							strText.Format("[Center] %.4f mm", pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[4].at(i));
							pDC->TextOutA(xa + RightResultTextGap, ya, strText);
							cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(ya), int(W - xa - 20)), cv::Point(int(ya), int(W - xa + 20)), cv::Scalar(64, 255, 255), 3);
						}
					}
					//Point 1 그림그리기
					if (i == Point1_Index)
					{
						pDC->SelectObject(&penOK);
						pDC->MoveTo(xa - 20, ya);
						pDC->LineTo(xa + 20, ya);
						pDC->MoveTo(xa, ya);
						pDC->SelectObject(&penLeft);

						if (bsplit)
						{
							cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(ya), int(xa - 20)), cv::Point(int(ya), int(xa + 20)), cv::Scalar(64, 255, 255), 3);
							strText.Format("[Point 1] %.4f mm", pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[4].at(i));
							if (xa - LeftResultTextGap > 0)
							{
								pDC->TextOutA(xa - LeftResultTextGap, ya, strText);
							}
							else
							{
								pDC->TextOutA(0, ya, strText);
							}
						}
						else
						{
							strText.Format("[Point 1] %.4f mm", pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[4].at(i));
							pDC->TextOutA(xa + RightResultTextGap, ya, strText);
							cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(ya), int(W - xa - 20)), cv::Point(int(ya), int(W - xa + 20)), cv::Scalar(64, 255, 255), 3);
						}
					}
					//Point 2 그림그리기
					if (i == Point2_Index)
					{
						pDC->SelectObject(&penOK);
						pDC->MoveTo(xa - 20, ya);
						pDC->LineTo(xa + 20, ya);
						pDC->MoveTo(xa, ya);
						pDC->SelectObject(&penLeft);

						if (bsplit)
						{
							cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(ya), int(xa - 20)), cv::Point(int(ya), int(xa + 20)), cv::Scalar(64, 255, 255), 3);
							strText.Format("[Point 2] %.4f mm", pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[4].at(i));
							if (xa - LeftResultTextGap > 0)
							{
								pDC->TextOutA(xa - LeftResultTextGap, ya, strText);
							}
							else
							{
								pDC->TextOutA(0, ya, strText);
							}
						}
						else
						{
							strText.Format("[Point 2] %.4f mm", pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[4].at(i));
							pDC->TextOutA(xa + RightResultTextGap, ya, strText);
							cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(ya), int(W - xa - 20)), cv::Point(int(ya), int(W - xa + 20)), cv::Scalar(64, 255, 255), 3);
						}
					}
					
					//연결선 그리기
					if (SeqGrabCount == 1)
					{
						cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(ya2), int(xa2)), cv::Point(int(ya), int(xa)), cv::Scalar(255, 0, 255), 3);
					}
					else
					{
						cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(ya2), int(W - xa2)), cv::Point(int(ya), int(W - xa)), cv::Scalar(255, 0, 255), 3);
					}

					//시뮬레이션 거리값 확인용
					if (i % 100 == 0)
					{
						if ((pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x < i && pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x != -1) ||
							(pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y != -1 && pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y > i))
						{

							//strText.Format("[L][%.1f, %.1f] %.4f mm", xa, ya, m_ELB_DiffInspResult[nCam].m_vTraceProfile[4].at(i));
							if (bsplit)
							{
								strText.Format("[OUT] %.4f mm", pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[4].at(i));
								if (xa - LeftResultTextGap > 0)
								{
									pDC->TextOutA(xa - LeftResultTextGap, ya, strText);
								}
								else
								{
									pDC->TextOutA(0, ya, strText);
								}
							}
							else
							{
								strText.Format("[OUT] %.4f mm", pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[4].at(i));
								pDC->TextOutA(xa + RightResultTextGap, ya, strText);
							}

						}
					}
				}
			}
			//Out Result Overlay Display End
#pragma endregion

#pragma region 시뮬레이션 Dopo Start End 표시
			//노란색
			pDC->SelectObject(&penOK);

			pDC->SetTextColor(COLOR_LIME);
			strText.Format("Dopo Start");
			pDC->TextOutA(W / 2 + 200, pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x - 20, strText);

			pDC->MoveTo(0, pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x);
			pDC->LineTo(W, pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x);
			cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x), int(0)), cv::Point(int(pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x), int(H)), cv::Scalar(0, 255, 255), 3);

			pDC->SetTextColor(COLOR_LIME);
			strText.Format("Dopo End");
			pDC->TextOutA(W / 2 + 200, pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y, strText);

			pDC->MoveTo(0, pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y);
			pDC->LineTo(W, pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y);
			cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y), int(0)), cv::Point(int(pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y), int(H)), cv::Scalar(0, 255, 255), 3);

			penOK.DeleteObject();
			penLeft.DeleteObject();
			penRight.DeleteObject();
			penRed.DeleteObject();
#pragma endregion
		}
#pragma endregion

#pragma region Auto 화면 그리기
		else
		{
			double xa, ya;
			double xa2, ya2;
#pragma region Auto In 표시

			for (int i = 1; i < pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].size(); i++)
			{
				xa = pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[1].at(i);
				ya = pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].at(i);
				xa2 = pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[1].at(i - 1);
				ya2 = pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].at(i - 1);

				if (ya < nDopo_Start || ya > nDopo_End)
				{
				}
				else
				{
					//연결선 그리기
					if (SeqGrabCount == 1)
					{
						cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(ya2), int(xa2)), cv::Point(int(ya), int(xa)), cv::Scalar(64, 255, 64), 3);

						//Center Point 그림그리기
						if (i == Center_Index)
						{
							cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(ya - 20), int(xa)), cv::Point(int(ya + 20), int(xa)), cv::Scalar(64, 255, 64), 3);
						}
						//Point 1 그림그리기
						if (i == Point1_Index)
						{
							cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(ya - 20), int(xa)), cv::Point(int(ya + 20), int(xa)), cv::Scalar(64, 255, 64), 3);
						}
						//Point 2 그림그리기
						if (i == Point2_Index)
						{
							cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(ya - 20), int(xa)), cv::Point(int(ya + 20), int(xa)), cv::Scalar(64, 255, 64), 3);
						}
					}
					else
					{
						cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(ya2), int(W - xa2)), cv::Point(int(ya), int(W - xa)), cv::Scalar(64, 255, 64), 3);

						//Center Point 그림그리기
						if (i == Center_Index)
						{
							cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(ya - 20), int(W - xa)), cv::Point(int(ya + 20), int(W - xa)), cv::Scalar(64, 255, 64), 3);
						}
						//Point 1 그림그리기
						if (i == Point1_Index)
						{
							cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(ya - 20), int(W - xa)), cv::Point(int(ya + 20), int(W - xa)), cv::Scalar(64, 255, 64), 3);
						}
						//Point 2 그림그리기
						if (i == Point2_Index)
						{
							cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(ya - 20), int(W - xa)), cv::Point(int(ya + 20), int(W - xa)), cv::Scalar(64, 255, 64), 3);
						}
					}
				}
			}
			//In Result Overlay Display End
#pragma endregion
#pragma region Auto Out 표시
			//Out Result Overlay Display Start - Out방향 보라색 외각선 표시
			for (int i = 1; i < pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].size(); i++)
			{
				xa = pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[3].at(i);
				ya = pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].at(i);
				xa2 = pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[3].at(i - 1);
				ya2 = pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].at(i - 1);

				//KJH 2021-07-27	Start 찾은점과 End 찾은점으로 영역 변경
				if (ya < nDopo_Start || ya > nDopo_End)
				{
				}
				else
				{
					if (SeqGrabCount == 1)
					{
						cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(ya2), int(xa2)), cv::Point(int(ya), int(xa)), cv::Scalar(255, 0, 255), 3);

						//Center Point 그림그리기
						if (i == Center_Index)
						{
							cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(ya - 20), int(xa)), cv::Point(int(ya + 20), int(xa)), cv::Scalar(64, 255, 64), 3);
						}
						//Point 1 그림그리기
						if (i == Point1_Index)
						{
							cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(ya - 20), int(xa)), cv::Point(int(ya + 20), int(xa)), cv::Scalar(64, 255, 64), 3);
						}
						//Point 2 그림그리기
						if (i == Point2_Index)
						{
							cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(ya - 20), int(xa)), cv::Point(int(ya + 20), int(xa)), cv::Scalar(64, 255, 64), 3);
						}
					}
					else
					{
						cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(ya2), int(W - xa2)), cv::Point(int(ya), int(W - xa)), cv::Scalar(255, 0, 255), 3);

						//Center Point 그림그리기
						if (i == Center_Index)
						{
							cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(ya - 20), int(W - xa)), cv::Point(int(ya + 20), int(W - xa)), cv::Scalar(64, 255, 64), 3);
						}
						//Point 1 그림그리기
						if (i == Point1_Index)
						{
							cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(ya - 20), int(W - xa)), cv::Point(int(ya + 20), int(W - xa)), cv::Scalar(64, 255, 64), 3);
						}
						//Point 2 그림그리기
						if (i == Point2_Index)
						{
							cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(ya - 20), int(W - xa)), cv::Point(int(ya + 20), int(W - xa)), cv::Scalar(64, 255, 64), 3);
						}
					}
				}
			}
			//Out Result Overlay Display End
#pragma endregion
#pragma region Dopo Start End 표시
			//노란색
			cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x), int(0)), cv::Point(int(pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x), int(H)), cv::Scalar(0, 255, 255), 3);
			cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y), int(0)), cv::Point(int(pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y), int(H)), cv::Scalar(0, 255, 255), 3);
#pragma endregion
		}
#pragma endregion

#pragma region Spec 판정하기

		double In_Spec				= pDlg->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInSpec(SeqGrabCount - 1);
		double In_Spec_Tolerance	= pDlg->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInSpecTorr(SeqGrabCount - 1);
		double Out_Spec				= pDlg->vt_job_info[nJob].model_info.getInspSpecParaInfo().getOutSpec(SeqGrabCount - 1);
		double Out_Spec_Tolerance	= pDlg->vt_job_info[nJob].model_info.getInspSpecParaInfo().getOutSpecTorr(SeqGrabCount - 1);
		double Total_Spec			= pDlg->vt_job_info[nJob].model_info.getInspSpecParaInfo().getTotalSpec(SeqGrabCount - 1);
		double Total_Spec_Tolerance	= pDlg->vt_job_info[nJob].model_info.getInspSpecParaInfo().getTotalSpecTorr(SeqGrabCount - 1);

		double In_SpecMin			= In_Spec - In_Spec_Tolerance;
		double In_SpecMax			= In_Spec + In_Spec_Tolerance;
		double Out_SpecMin			= Out_Spec - Out_Spec_Tolerance;
		double Out_SpecMax			= Out_Spec + Out_Spec_Tolerance;
		double Total_SpecMin		= Total_Spec - Total_Spec_Tolerance;
		double Total_SpecMax		= Total_Spec + Total_Spec_Tolerance;

		double MaxDistance_In		= -1,	MinDistance_In		= 99999;
		double MaxDistance_Out		= -1,	MinDistance_Out		= 99999;
		double MaxDistance_Total	= -1,	MinDistance_Total	= 99999;
		double MaxDistance_In_Pos	= -1,	MinDistance_In_Pos	= -1;
		double MaxDistance_Out_Pos	= -1,	MinDistance_Out_Pos	= -1;

		for (int i = 1; i < pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[0].size(); i++)
		{
			//우측 거리값
			double ValueRight	= fabs(pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[2].at(i));
			//좌측 거리값
			double ValueLeft	= fabs(pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[4].at(i));
			//총 도포 폭
			double ValueWidth	= fabs(ValueRight + ValueLeft);
			//Wet Out Limit Line 거리값
			double ValueLimitInsp = fabs(pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[5].at(i));

			//In쪽 Data Search

			if ((nDopo_Start + SearchValueOffset_In < i && nDopo_Start != -1) || (nDopo_End != -1 && nDopo_End - SearchValueOffset_In > i))
			{
				if (ValueRight >= MaxDistance_In)
				{
					MaxDistance_In		= ValueRight;
					MaxDistance_In_Pos	= pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[1].at(i);
				}
				if (ValueRight <= MinDistance_In && ValueRight > 0.02)
				{
					MinDistance_In		= ValueRight;
					MinDistance_In_Pos	= pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[1].at(i);
				}
			}

			//Out쪽 Data Search
			if ((nDopo_Start + SearchValueOffset_Out < i && nDopo_Start != -1) || (nDopo_End != -1 && nDopo_End - SearchValueOffset_Out > i))
			{
				if (ValueLeft >= MaxDistance_Out)
				{
					MaxDistance_Out = ValueLeft;
					MaxDistance_Out_Pos = pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[3].at(i);
				}
				if (ValueLeft <= MinDistance_Out && ValueLeft > 0.02)
				{
					MinDistance_Out = ValueLeft;
					MinDistance_Out_Pos = pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vTraceProfile[3].at(i);
				}
				if (ValueLeft > ValueLimitInsp)
				{
					bNG = FALSE;
				}
			}
			MinDistance_Total = MinDistance_In + MinDistance_Out;
			MaxDistance_Total = MaxDistance_In + MaxDistance_Out;
		}

		pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vELB_InMinMaxValue.x	= MinDistance_In;
		pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vELB_InMinMaxPos.x		= MinDistance_In_Pos;
		pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vELB_InMinMaxValue.y	= MaxDistance_In;
		pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vELB_InMinMaxPos.y		= MaxDistance_In_Pos;
		pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vELB_OutMinMaxValue.x	= MinDistance_Out;
		pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vELB_OutMinMaxPos.x		= MinDistance_Out_Pos;
		pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vELB_OutMinMaxValue.y	= MaxDistance_Out;
		pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vELB_OutMinMaxPos.y		= MaxDistance_Out_Pos;
		pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vELB_TotalMinMaxValue.x = MinDistance_Total;
		pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_vELB_TotalMinMaxValue.y = MaxDistance_Total;

		//Model Insp (Distance Inspection Sub Function)스펙 판정

		if (pDlg->m_vInspWetOutResult.size() > 1000)
			pDlg->m_vInspWetOutResult.clear();

		_stInsp_WetOut_Result temp;

		temp.dMin_In	= MinDistance_In;
		temp.dMax_In	= MaxDistance_In;
		temp.dMin_Out	= MinDistance_Out;
		temp.dMax_Out	= MaxDistance_Out;
		temp.dMin_Total = MinDistance_Total;
		temp.dMax_Total = MaxDistance_Total;

		pDlg->m_vInspWetOutResult.push_back(temp);
		
		CPoint InspRoiStart, InspRoiEnd;

		//Dust Insp Roi 설정하기
		if (MaxDistance_Out_Pos < 0 || MaxDistance_In_Pos < 0)
		{
			pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_InspRoi[nPos + 1] = pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_InspRoi[nPos];
		}
		else
		{
			if (bsplit)
			{
				InspRoiStart.x	= MaxDistance_Out_Pos;
				InspRoiStart.y	= pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x;
				InspRoiEnd.x	= MaxDistance_In_Pos;
				InspRoiEnd.y	= pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y;
			}
			else
			{
				InspRoiStart.x	= MaxDistance_In_Pos;
				InspRoiStart.y	= pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x;
				InspRoiEnd.x	= MaxDistance_Out_Pos;
				InspRoiEnd.y	= pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y;
			}
			pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_InspRoi[nPos + 1] = CRect(InspRoiStart, InspRoiEnd);
		}
		//Dust Insp Roi 설정하기 끝

		if (MaxDistance_In > In_SpecMax || MaxDistance_Out > Out_SpecMax)
		{
			bNG = FALSE;
		}

		if (MinDistance_In < In_SpecMin || MinDistance_Out < Out_SpecMin)
		{
			bNG = FALSE;
		}

		if (MinDistance_Total < Total_SpecMin || MaxDistance_Total > Total_SpecMax)
		{
			bNG = FALSE;
		}

		//캘리퍼 못찾은 경우 NG
		if (pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_bResultSummary == FALSE) bNG = FALSE;

#pragma endregion

#pragma region Spec 화면 그리기

		double distance = (pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y - pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x) * yres;

		if (pView != NULL)
		{
			CFont font, * oldFont;
			font.CreateFont(20, 20, 0, 0, FW_NORMAL, 0, 0, 0, DEFAULT_CHARSET, OUT_CHARACTER_PRECIS, CLIP_CHARACTER_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, "Arial");
			oldFont = pDC->SelectObject(&font);

			pDC->SetTextColor(COLOR_RED);
			strText.Format("[S: %.4f, E: %.4f] Distance = %.4f mm", pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x * yres, pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y * yres, distance);
			pDC->TextOutA(50, 20, strText);
			cv::putText(pDlg->m_matScanResultImage[nCam][SeqGrabCount], std::string(strText), cv::Point(20, 50), cv::FONT_ITALIC, 1, cv::Scalar(64, 64, 255), 3);

			pDC->SetTextColor(COLOR_GREEN);
			strText.Format("[Min(in): %.4f, Max(in): %.4f]mm", MinDistance_In, MaxDistance_In);
			pDC->TextOutA(50, 40, strText);
			cv::putText(pDlg->m_matScanResultImage[nCam][SeqGrabCount], std::string(strText), cv::Point(20, 100), cv::FONT_ITALIC, 1, cv::Scalar(64, 255, 64), 3);

			pDC->SetTextColor(RGB(255, 0, 255));
			strText.Format("[Min(out): %.4f, Max(out): %.4f]mm", MinDistance_Out, MaxDistance_Out);
			pDC->TextOutA(50, 60, strText);
			cv::putText(pDlg->m_matScanResultImage[nCam][SeqGrabCount], std::string(strText), cv::Point(20, 150), cv::FONT_ITALIC, 1, cv::Scalar(255, 0, 255), 3);

			pDC->SetTextColor(COLOR_YELLOW);
			strText.Format("[Min(total): %.4f, Max(total): %.4f]mm", MinDistance_Total, MaxDistance_Total);
			pDC->TextOutA(50, 80, strText);
			cv::putText(pDlg->m_matScanResultImage[nCam][SeqGrabCount], std::string(strText), cv::Point(20, 200), cv::FONT_ITALIC, 1, cv::Scalar(64, 255, 255), 3);

			pDC->SetTextColor(COLOR_RED);
			strText.Format("[In_Point 1: %.4f, Out_Point 1: %.4f, In_Center: %.4f, Out_Center: %.4f In_Point 2: %.4f, Out_Point 2: %.4f]mm", 
				pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_Result_Point1_XWidth_in.y, pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_Result_Point1_XWidth_out.y,
				pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_Result_Center_XWidth_in.y, pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_Result_Center_XWidth_out.y,
				pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_Result_Point2_XWidth_in.y, pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_Result_Point2_XWidth_out.y);
			pDC->TextOutA(50, 100, strText);
			cv::putText(pDlg->m_matScanResultImage[nCam][SeqGrabCount], std::string(strText), cv::Point(20, 250), cv::FONT_ITALIC, 1, cv::Scalar(64, 64, 255), 3);

			//JUDGE 디스플레이 추가 필요함(2021-08-23)

			pDC->SelectObject(oldFont);
			font.DeleteObject();

			pView->Invalidate();
		}
		else
		{
			strText.Format("[S: %.4f, E: %.4f] Distance = %.4f mm", pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.x * yres, pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_FindPoint.y * yres, distance);
			cv::putText(pDlg->m_matScanResultImage[nCam][SeqGrabCount], std::string(strText), cv::Point(20, 50), cv::FONT_ITALIC, 1, cv::Scalar(64, 64, 255), 3);

			strText.Format("[Min(in): %.4f, Max(in): %.4f]mm", MinDistance_In, MaxDistance_In);
			cv::putText(pDlg->m_matScanResultImage[nCam][SeqGrabCount], std::string(strText), cv::Point(20, 100), cv::FONT_ITALIC, 1, cv::Scalar(64, 255, 64), 3);

			strText.Format("[Min(out): %.4f, Max(out): %.4f]mm", MinDistance_Out, MaxDistance_Out);
			cv::putText(pDlg->m_matScanResultImage[nCam][SeqGrabCount], std::string(strText), cv::Point(20, 150), cv::FONT_ITALIC, 1, cv::Scalar(255, 0, 255), 3);

			strText.Format("[Min(total): %.4f, Max(total): %.4f]mm", MinDistance_Total, MaxDistance_Total);
			cv::putText(pDlg->m_matScanResultImage[nCam][SeqGrabCount], std::string(strText), cv::Point(20, 200), cv::FONT_ITALIC, 1, cv::Scalar(64, 255, 255), 3);

			strText.Format("[In_Point 1: %.4f, Out_Point 1: %.4f, In_Center: %.4f, Out_Center: %.4f In_Point 2: %.4f, Out_Point 2: %.4f]mm",
				pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_Result_Point1_XWidth_in.y, pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_Result_Point1_XWidth_out.y,
				pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_Result_Center_XWidth_in.y, pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_Result_Center_XWidth_out.y,
				pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_Result_Point2_XWidth_in.y, pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_Result_Point2_XWidth_out.y);
			cv::putText(pDlg->m_matScanResultImage[nCam][SeqGrabCount], std::string(strText), cv::Point(20, 250), cv::FONT_ITALIC, 1, cv::Scalar(64, 64, 255), 3);
		}
	}

#pragma endregion

#pragma region Dust Insp 영상처리

	if (pDlg->vt_job_info[nJob].model_info.getInspSpecParaInfo().getUseDustInsp())
		//if(1)
	{
		double ratio = 0.5;
		if (bsplit) ratio = 0.25;

		CRect roi = pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].m_ELB_InspRoi[nPos + 1];
		cv::Rect inspRoi = cv::Rect(roi.left, roi.top, roi.Width(), roi.Height());
		
		//Dust Insp Roi Display
		if (pView != NULL)
		{
			COLORREF color = COLOR_BLUE;
			stFigure tempFig;

			tempFig.ptBegin.x	= roi.left;
			tempFig.ptBegin.y	= roi.top;
			tempFig.ptEnd.x		= roi.right;
			tempFig.ptEnd.y		= roi.bottom;

			pView->addFigureRect(tempFig, 6, 4, color);
			if (bsplit)
			{
				cv::rectangle(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(roi.top, roi.left), cv::Point(roi.bottom, roi.right), CV_RGB(64, 64, 255), 5);
			}
			else
			{
				cv::rectangle(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(roi.top, W - roi.left), cv::Point(roi.bottom, W - roi.right), CV_RGB(64, 64, 255), 5);
			}
		}
		else
		{
			if (bsplit)
			{
				cv::rectangle(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(roi.top, roi.left), cv::Point(roi.bottom, roi.right), CV_RGB(64, 64, 255), 5);
			}
			else
			{
				cv::rectangle(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(roi.top, W - roi.left), cv::Point(roi.bottom, W - roi.right), CV_RGB(64, 64, 255), 5);
			}
		}

		int method = pDlg->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod();
		double xres = pDlg->GetMachine(nJob).getCameraResolutionX(0, 0);
		int dThresh = pDlg->vt_job_info[nJob].model_info.getInspSpecParaInfo().getInspDustThresh();

		cv::Mat srcImg(H, W, CV_8UC1, pImage);
		cv::Mat img, mask = GetRectmask(srcImg, inspRoi);
		cv::bitwise_not(mask, mask);

		srcImg.copyTo(img);
		cv::bitwise_and(img, mask, img);
		
		//CRect cRect = CRect(inspRoi.x, inspRoi.y, inspRoi.x + inspRoi.width, inspRoi.y + inspRoi.height);
		//cRect.InflateRect(300, 200);

		//inspRoi = cv::Rect(cRect.left, cRect.top, cRect.right- cRect.left, cRect.bottom - cRect.top);
		inspRoi = cv::Rect(20, 20, mask.cols - 40, mask.rows - 40);

		//inspRoi.x		= MAX(20, inspRoi.x);
		//inspRoi.y		= MAX(20, inspRoi.y);
		//inspRoi.width	= MIN(W - inspRoi.x-20, inspRoi.width);
		//inspRoi.height	= MIN(H - inspRoi.y-20, inspRoi.height);

		cv::Mat element15(15, 15, CV_8U, cv::Scalar::all(1));
		cv::Mat element3(8, 8, CV_8U, cv::Scalar::all(1));
		cv::Mat s_down, close; // 축소 해서 검사 진행
		cv::resize(img(inspRoi), s_down, cv::Size(), ratio, ratio, CV_INTER_CUBIC);

		cv::threshold(s_down, s_down, dThresh, 255, CV_THRESH_BINARY_INV);
		cv::erode(s_down, s_down, Mat::ones(cv::Size(3, 5), CV_8UC1), cv::Point(-1, -1), 1);
		fnRemoveNoise(s_down,50);

		morphologyEx(s_down, close, cv::MORPH_CLOSE, element15);
		//cvShowImage("MORPH_CLOSE", close);
		morphologyEx(close, close, cv::MORPH_OPEN, element3);
		//cvShowImage("MORPH_OPEN", close);

		int spec_S = MAX(10, int(pDlg->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecLSL(1) / xres) * ratio);	//최소 크기
		int spec_A = MAX(250, int(pDlg->vt_job_info[nJob].model_info.getAlignInfo().getDistanceInspSpecUSL(1) / xres) * ratio);	//최소 면적
		
		pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].nDefect = pFormMain->InspDustBurrInspection(nJob, close, 255-dThresh, inspRoi.x, inspRoi.y, 1/ ratio, spec_S, spec_A, true); //10,25

		if (pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].nDefect > 0)
		{
			for (int i = 0; i< int(pFormMain->m_vecDustResult.size()); i++)
			{
				pFormMain->m_vecDustResult.at(i).center.x	 = pFormMain->m_vecDustResult.at(i).center.x / ratio + inspRoi.x;
				pFormMain->m_vecDustResult.at(i).center.y	 = pFormMain->m_vecDustResult.at(i).center.y / ratio + inspRoi.y;
				pFormMain->m_vecDustResult.at(i).rr.center.x = pFormMain->m_vecDustResult.at(i).rr.center.x / ratio + inspRoi.x;
				pFormMain->m_vecDustResult.at(i).rr.center.y = pFormMain->m_vecDustResult.at(i).rr.center.y / ratio + inspRoi.y;
				pFormMain->m_vecDustResult.at(i).rr.size.width = pFormMain->m_vecDustResult.at(i).rr.size.width;
				pFormMain->m_vecDustResult.at(i).rr.size.height = pFormMain->m_vecDustResult.at(i).rr.size.height;
			}
		}

		if (pView != NULL && pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].nDefect > 0)
		{
			CString strText;

			CPen penNG(PS_SOLID, 5, COLOR_RED);
			pDC->SetTextColor(COLOR_RED);
			pDC->SelectObject(&penNG);

			for (int i = 0; i < pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].nDefect; i++)
			{
				cv::RotatedRect rr = pFormMain->m_vecDustResult.at(i).rr;
				cv::Rect brect = rr.boundingRect();
				cv::Point2f vertices2f[4];
				rr.points(vertices2f);

				cv::Point vertices[4];
				for (int j = 0; j < 4; ++j)
				{
					vertices[j] = vertices2f[j];
				}
				//Main Viewer 방향일때 표시
				pDC->MoveTo(vertices[0].x, vertices[0].y);					//좌하
				pDC->LineTo(vertices[1].x, vertices[1].y);					//좌하-->좌상
				pDC->LineTo(vertices[2].x, vertices[2].y);					//좌상->우상
				pDC->LineTo(vertices[3].x, vertices[3].y);					//우상->우하
				pDC->LineTo(vertices[0].x, vertices[0].y);					//우하->좌하

				cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(vertices[0].y), W - int(vertices[0].x)), cv::Point(int(vertices[1].y), W - int(vertices[1].x)), cv::Scalar(64, 64, 255), 3);
				cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(vertices[1].y), W - int(vertices[1].x)), cv::Point(int(vertices[2].y), W - int(vertices[2].x)), cv::Scalar(64, 64, 255), 3);
				cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(vertices[2].y), W - int(vertices[2].x)), cv::Point(int(vertices[3].y), W - int(vertices[3].x)), cv::Scalar(64, 64, 255), 3);
				cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(vertices[3].y), W - int(vertices[3].x)), cv::Point(int(vertices[0].y), W - int(vertices[0].x)), cv::Scalar(64, 64, 255), 3);

				//strText.Format("[S: %.4f, %.4fmm, A: %.2f]", pFormMain->m_vecDustResult.at(i).xw * xres, pFormMain->m_vecDustResult.at(i).yw * yres, pFormMain->m_vecDustResult.at(i).area / ratio);
				strText.Format("[S: %.0f, %.0f, A: %.0f]", pFormMain->m_vecDustResult.at(i).xw, pFormMain->m_vecDustResult.at(i).yw, pFormMain->m_vecDustResult.at(i).area);
				pDC->TextOutA(pFormMain->m_vecDustResult.at(i).center.x, pFormMain->m_vecDustResult.at(i).center.y, strText);
				cv::putText(pDlg->m_matScanResultImage[nCam][SeqGrabCount], std::string(strText), cv::Point(pFormMain->m_vecDustResult.at(i).center.y, W - pFormMain->m_vecDustResult.at(i).center.x), cv::FONT_ITALIC, 1, cv::Scalar(64, 64, 255), 3);
			}

			penNG.DeleteObject();

			bNG = false;
		}
		else if (pView == NULL && pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].nDefect > 0)
		{
			CString strText;

			for (int i = 0; i < pDlg->m_ELB_ScanInspResult_UT[nCam][SeqGrabCount].nDefect; i++)
			{
				cv::RotatedRect rr = pFormMain->m_vecDustResult.at(i).rr;
				cv::Rect brect = rr.boundingRect();
				cv::Point2f vertices2f[4];
				rr.points(vertices2f);

				cv::Point vertices[4];
				for (int j = 0; j < 4; ++j)
				{
					vertices[j] = vertices2f[j];
				}

				cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(vertices[0].x), int(vertices[0].y)), cv::Point(int(vertices[1].x), int(vertices[1].y)), cv::Scalar(64, 64, 255), 3);
				cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(vertices[1].x), int(vertices[1].y)), cv::Point(int(vertices[2].x), int(vertices[2].y)), cv::Scalar(64, 64, 255), 3);
				cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(vertices[2].x), int(vertices[2].y)), cv::Point(int(vertices[3].x), int(vertices[3].y)), cv::Scalar(64, 64, 255), 3);
				cv::line(pDlg->m_matScanResultImage[nCam][SeqGrabCount], cv::Point(int(vertices[3].x), int(vertices[3].y)), cv::Point(int(vertices[0].x), int(vertices[0].y)), cv::Scalar(64, 64, 255), 3);

				strText.Format("[S: %.4f,%.4fmm, A: %.2f]", brect.width * 4 * xres, brect.height * 4 * xres, pFormMain->m_vecDustResult.at(i).area * 4);
				cv::putText(pDlg->m_matScanResultImage[nCam][SeqGrabCount], std::string(strText), cv::Point(pFormMain->m_vecDustResult.at(i).center.x, pFormMain->m_vecDustResult.at(i).center.y), cv::FONT_ITALIC, 1, cv::Scalar(64, 64, 255), 1);
			}

			bNG = false;
		}
	}
#pragma endregion

	//JUDGE 디스플레이 추가 필요함(2021-08-23)
	//모든 검사 이후에 판정
	if (bNG == FALSE)
	{
		strText = "Judge: NG";
		cv::putText(pDlg->m_matScanResultImage[nCam][SeqGrabCount], std::string(strText), cv::Point(20, 300), cv::FONT_ITALIC, 1, cv::Scalar(64, 64, 255), 3);
	}
	else
	{
		strText = "Judge: OK";
		cv::putText(pDlg->m_matScanResultImage[nCam][SeqGrabCount], std::string(strText), cv::Point(20, 300), cv::FONT_ITALIC, 1, cv::Scalar(64, 255, 64), 3);
	}

	dstImg.release();
	img.release();
	First_dstImg.release();
	Second_dstImg.release();
	Sobel_dstlmg.release();


	if (pDlg->vt_job_info[nJob].model_info.getAlignInfo().getUseRealTimeTrace())
	{
		CString strfile;
		strfile.Format("D:\\m_matScanResultImage_overlay_%d_%d.jpg", nCam, SeqGrabCount);
		cv::imwrite(std::string(strfile), pDlg->m_matScanResultImage[nCam][SeqGrabCount]);
	}

	if (theApp.m_bEnforceOkFlag) bNG = TRUE;

	return bNG;
}

BOOL CScanInspWorker::write_result_scan_insp(CString serialNum, BOOL bjudge, int algorithm)
{
	EnterCriticalSection(&m_csScanWrite);

	CFormMainView* pFormMain = (CFormMainView*)m_pMainForm;
	CLET_AlignClientDlg* pDlg = (CLET_AlignClientDlg*)m_pM;

	CString strTemp, Time_str, Date_str, str_modelID, strSummary;
	CString strFileDir = pDlg->m_strResultDir + pDlg->m_strResultDate[algorithm];
	BOOL bWriteHeader = FALSE;
	BOOL bWriteHeaderSummary = FALSE;

	CString algo_name = pDlg->vt_job_info[algorithm].job_name.c_str();

	int SeqGrabCount = pDlg->m_nSeqScanInspGrabCount[algorithm];

	int nCamCount = pDlg->vt_job_info[algorithm].num_of_camera;
	std::vector<int> camBuf = pDlg->vt_job_info[algorithm].camera_index;
	int real_cam = camBuf[0];

	SYSTEMTIME	csTime;
	::GetLocalTime(&csTime);
	if (pDlg->m_bSimulationStart)
	{
		Time_str.Format("%02d:%02d:%02d", csTime.wHour, csTime.wMinute, csTime.wSecond);
		Date_str.Format("%04d%02d%02d", csTime.wYear, csTime.wMonth, csTime.wDay);
	}
	else
	{
		if (pDlg->m_strResultTime[algorithm] == "")
			Time_str.Format("%02d_%02d_%02d", csTime.wHour, csTime.wMinute, csTime.wSecond);
		else Time_str = pDlg->m_strResultTime[algorithm];
		
		if (pDlg->m_strResultDate[algorithm] == "")
			Date_str.Format("%04d%02d%02d", csTime.wYear, csTime.wMonth, csTime.wDay);
		else Date_str = pDlg->m_strResultDate[algorithm];
	}
	//D:\LET_AlignClient\Result\Files//
	strFileDir.Format("%sFiles\\", pDlg->m_strResultDir);
	if (_access(strFileDir, 0) != 0)	CreateDirectory(strFileDir, NULL);
	
	//D:\LET_AlignClient\Result\Files\DATE//
	strFileDir.Format("%s%s\\", strFileDir, Date_str);
	if (_access(strFileDir, 0) != 0)	CreateDirectory(strFileDir, NULL);

	//모델ID
	str_modelID.Format("%s", pDlg->vt_job_info[algorithm].model_info.getModelID());
	strFileDir.Format("%s\\%s", strFileDir, str_modelID);
	if (_access(strFileDir, 0) != 0)	CreateDirectory(strFileDir, NULL);

	strSummary.Format("%s\\%s_InspSummary_Result.csv", strFileDir, algo_name);

	//PanelID
	if (strlen(pDlg->m_ELB_ScanInspResult_UT[real_cam][SeqGrabCount].m_strPanelID.c_str()) <= 13)
	{
		strFileDir.Format("%s\\Not Exist Panel ID", strFileDir);
		if (_access(strFileDir, 0) != 0)	CreateDirectory(strFileDir, NULL);
	}
	
	strFileDir.Format("%s\\%s\\", strFileDir, pDlg->m_ELB_ScanInspResult_UT[real_cam][SeqGrabCount].m_strPanelID.c_str());
	if (_access(strFileDir, 0) != 0)	CreateDirectory(strFileDir, NULL);

	if (pDlg->m_bSimulationStart)	strFileDir.Format("%ssimulation_", strFileDir);
	strTemp.Format("%s%s_Result_%d.csv", strFileDir, algo_name, SeqGrabCount);

	if ((_access(strSummary, 0)) == -1)		bWriteHeaderSummary = TRUE;

	if ((_access(strTemp, 0)) == -1)
	{
		bWriteHeader = TRUE;
	}

	CStdioFile WriteFile, SummaryFile;
	if (WriteFile.Open(strTemp, CFile::modeNoTruncate | CFile::modeCreate | CFile::modeWrite | CFile::shareDenyNone | CFile::typeText) != TRUE)
	{
		CString str;
		str.Format("Fail to Open Logfile !!: %s", strTemp.Mid(strTemp.ReverseFind('\\') + 1));
		::SendMessageA(pDlg->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		LeaveCriticalSection(&m_csScanWrite);
		return 0;
	}
	if (SummaryFile.Open(strSummary, CFile::modeNoTruncate | CFile::modeCreate | CFile::modeReadWrite | CFile::shareDenyNone | CFile::typeText) != TRUE)
	{
		CString str;
		str.Format("Fail to Open Logfile !!: %s", strSummary.Mid(strSummary.ReverseFind('\\') + 1));
		::SendMessageA(pDlg->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		LeaveCriticalSection(&m_csScanWrite);
		return 0;
	}

	double xres					= pDlg->GetMachine(algorithm).getCameraResolutionX(real_cam, 0);
	double yres					= pDlg->GetMachine(algorithm).getCameraResolutionY(real_cam, 0);
	double distance				= (pDlg->m_ELB_ScanInspResult_UT[real_cam][SeqGrabCount].m_ELB_FindPoint.y - pDlg->m_ELB_ScanInspResult_UT[real_cam][SeqGrabCount].m_ELB_FindPoint.x) * yres;
	double minDistance_in		= pDlg->m_ELB_ScanInspResult_UT[real_cam][SeqGrabCount].m_vELB_InMinMaxValue.x;
	double maxDistance_in		= pDlg->m_ELB_ScanInspResult_UT[real_cam][SeqGrabCount].m_vELB_InMinMaxValue.y;
	double minDistance_out		= pDlg->m_ELB_ScanInspResult_UT[real_cam][SeqGrabCount].m_vELB_OutMinMaxValue.x;
	double maxDistance_out		= pDlg->m_ELB_ScanInspResult_UT[real_cam][SeqGrabCount].m_vELB_OutMinMaxValue.y;
	double minDistance			= pDlg->m_ELB_ScanInspResult_UT[real_cam][SeqGrabCount].m_vELB_TotalMinMaxValue.x;
	double maxDistance			= pDlg->m_ELB_ScanInspResult_UT[real_cam][SeqGrabCount].m_vELB_TotalMinMaxValue.y;
	double centerinDistance		= pDlg->m_ELB_ScanInspResult_UT[real_cam][SeqGrabCount].m_ELB_Result_Center_XWidth_in.y;
	double centeroutDistance	= pDlg->m_ELB_ScanInspResult_UT[real_cam][SeqGrabCount].m_ELB_Result_Center_XWidth_out.y;
	double point1inDistance		= pDlg->m_ELB_ScanInspResult_UT[real_cam][SeqGrabCount].m_ELB_Result_Point1_XWidth_in.y;
	double point1outDistance	= pDlg->m_ELB_ScanInspResult_UT[real_cam][SeqGrabCount].m_ELB_Result_Point1_XWidth_out.y;
	double point2inDistance		= pDlg->m_ELB_ScanInspResult_UT[real_cam][SeqGrabCount].m_ELB_Result_Point2_XWidth_in.y;
	double point2outDistance	= pDlg->m_ELB_ScanInspResult_UT[real_cam][SeqGrabCount].m_ELB_Result_Point2_XWidth_out.y;

	CString strHeader, strBody;

	strTemp.Format("TIME");							strHeader += strTemp;
	strTemp.Format("%s", Time_str);					strBody += strTemp;

	strTemp.Format(",MODEL ID");					strHeader += strTemp;
	strTemp.Format(",%s", pDlg->m_strCurrentModelName);	strBody += strTemp;

	strTemp.Format(",PANEL ID");					strHeader += strTemp;
	strTemp.Format(",%s", serialNum);				strBody += strTemp;

	strTemp.Format(",SEQ COUNT");					strHeader += strTemp;
	strTemp.Format(",%d", SeqGrabCount);			strBody += strTemp;

	strTemp.Format(",JUDGE");						strHeader += strTemp;
	strTemp.Format(",%s", bjudge ? "OK": "NG");	strBody += strTemp;

	strTemp.Format(",MIN DISTANCE_IN");				strHeader += strTemp;
	strTemp.Format(",%4.4f", minDistance_in);	    strBody += strTemp;

	strTemp.Format(",MAX DISTANCE_IN");				strHeader += strTemp;
	strTemp.Format(",%4.4f", maxDistance_in);	    strBody += strTemp;

	strTemp.Format(",MIN DISTANCE_OUT");			strHeader += strTemp;
	strTemp.Format(",%4.4f", minDistance_out);	    strBody += strTemp;

	strTemp.Format(",MAX DISTANCE_OUT");			strHeader += strTemp;
	strTemp.Format(",%4.4f", maxDistance_out);	    strBody += strTemp;

	strTemp.Format(",MIN DISTANCE");				strHeader += strTemp;
	strTemp.Format(",%4.4f", minDistance);			strBody += strTemp;

	strTemp.Format(",MAX DISTANCE");				strHeader += strTemp;
	strTemp.Format(",%4.4f", maxDistance);			strBody += strTemp;

	strTemp.Format(",POINT1 IN DISTANCE");			strHeader += strTemp;
	strTemp.Format(",%4.4f", point1inDistance);		strBody += strTemp;

	strTemp.Format(",POINT1 OUT DISTANCE");			strHeader += strTemp;
	strTemp.Format(",%4.4f", point1outDistance);	strBody += strTemp;

	strTemp.Format(",CENTER IN DISTANCE");			strHeader += strTemp;
	strTemp.Format(",%4.4f", centerinDistance);		strBody += strTemp;

	strTemp.Format(",CENTER OUT DISTANCE");			strHeader += strTemp;
	strTemp.Format(",%4.4f", centeroutDistance);	strBody += strTemp;

	strTemp.Format(",POINT2 IN DISTANCE");			strHeader += strTemp;
	strTemp.Format(",%4.4f", point2inDistance);		strBody += strTemp;

	strTemp.Format(",POINT2 OUT DISTANCE");			strHeader += strTemp;
	strTemp.Format(",%4.4f", point2outDistance);	strBody += strTemp;

	strTemp.Format(",LENGTH");						strHeader += strTemp;
	strTemp.Format(",%4.4f", distance);	            strBody += strTemp;

	strHeader += "\n";
	strBody += "\n";

	if (bWriteHeader)
	{
		WriteFile.WriteString(strHeader);
	}
	if (bWriteHeaderSummary)
	{
		SummaryFile.WriteString(strHeader);
	}

	SummaryFile.SeekToEnd();
	SummaryFile.WriteString(strBody);
	SummaryFile.Close();

	WriteFile.SeekToEnd();
	WriteFile.WriteString(strBody);

	strTemp.Format("Move_Count, Wetout_In_Pos, Wetout_In_Dist, Wetout_Out_Pos, Wetout_Out_Dist\n");
	WriteFile.WriteString(strTemp);

	double a[5];
	//for (int i = 0; i < m_ELB_ScanInspResult_UT[real_cam][SeqGrabCount].m_vTraceProfile[0].size(); i++)
	//찾은Data 기준으로 저장하자...
	for (int i = pDlg->m_ELB_ScanInspResult_UT[real_cam][SeqGrabCount].m_ELB_FindPoint_Index.x; i < pDlg->m_ELB_ScanInspResult_UT[real_cam][SeqGrabCount].m_ELB_FindPoint_Index.y; i++)
	{
		a[0] = pDlg->m_ELB_ScanInspResult_UT[real_cam][SeqGrabCount].m_vTraceProfile[0].at(i);
		a[1] = pDlg->m_ELB_ScanInspResult_UT[real_cam][SeqGrabCount].m_vTraceProfile[1].at(i);
		a[2] = pDlg->m_ELB_ScanInspResult_UT[real_cam][SeqGrabCount].m_vTraceProfile[2].at(i);
		a[3] = pDlg->m_ELB_ScanInspResult_UT[real_cam][SeqGrabCount].m_vTraceProfile[3].at(i);
		a[4] = pDlg->m_ELB_ScanInspResult_UT[real_cam][SeqGrabCount].m_vTraceProfile[4].at(i);

		strTemp.Format("%4.4f, %4.4f, %4.4f, %4.4f, %4.4f\n", a[0], a[1], a[2], a[3], a[4]);
		WriteFile.WriteString(strTemp);
	}

	WriteFile.Close();

	LeaveCriticalSection(&m_csScanWrite);
	return TRUE;
}