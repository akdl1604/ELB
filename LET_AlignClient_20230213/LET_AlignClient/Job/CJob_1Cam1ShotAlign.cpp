#include "stdafx.h"
#include "CJob_1Cam1ShotAlign.h"
#include "../LET_AlignClientDlg.h"
#include "../CommPLC.h"

extern CJob_1Cam1ShotAlign g_Job_1Cam1ShotAlign[MAX_JOB];

CJob_1Cam1ShotAlign::CJob_1Cam1ShotAlign()
{
	m_pMain = NULL;
	m_nNumPos = 2;
	m_nJobID = 0;
	m_calib_seq = 0;
	m_calib_timeout_cnt = 0;
	m_calib_cnt_x = 0;
	m_calib_cnt_y = 0;

	is_seq_start = FALSE;

	m_dPanelLength = 0.0;
	m_dFilmLength = 0.0;
}

CJob_1Cam1ShotAlign::~CJob_1Cam1ShotAlign()
{
}

int CJob_1Cam1ShotAlign::grab_start(int next_seq, BOOL is_auto)
{
	int nCam = 0;
	int nRet = 0;
	int real_cam = pJob->camera_index[nCam];

	if (is_auto)
	{
		m_pMain->add_process_history("Grab Start");
		m_nAutoSeq = next_seq;
		//m_pMain->set_seq_process(m_nJobID, next_seq);
	}
	else
	{
		m_pMain->add_process_history_machine("Grab Start");
		m_calib_timeout_cnt = 0;
		m_calib_seq = next_seq;
	}

	m_pMain->set_grab_end(real_cam, FALSE);
	m_pMain->single_grab(real_cam);
	m_pMain->reset_grab_timeout_count(m_nJobID);
	return nRet;
}

int CJob_1Cam1ShotAlign::grab_complete_check(int next_seq, BOOL is_auto)
{
	int nCam = 0;
	int nRet = 0;
	BOOL bGrabComplete = TRUE;

	int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[nCam];
	if (m_pMain->get_grab_end(real_cam) != TRUE)	bGrabComplete = FALSE;

	if (is_auto)
	{
		m_pMain->increase_grab_timeout_count(m_nJobID);

		if (m_pMain->get_grab_timeout_count(m_nJobID) > 300)
		{
			m_nAutoSeq = next_seq;
			//	m_pMain->set_seq_process(m_nJobID, 0);
			m_pMain->add_process_history("Grab TimeOut");
			m_pMain->set_live_display(real_cam, FALSE);
			nRet = 2;
		}

		if (bGrabComplete)
		{
			m_nAutoSeq = next_seq;
			//	m_pMain->set_seq_process(m_nJobID, next_seq);
			m_pMain->add_process_history("Grab Complete");
		}
	}
	else
	{
		if (m_calib_timeout_cnt++ > 300)
		{
			m_calib_timeout_cnt = 0;
			m_calib_seq = 0;
			m_pMain->add_process_history_machine("Grab TimeOut");
		}

		if (bGrabComplete)
		{
			m_calib_timeout_cnt = 0;
			m_calib_seq = next_seq;
			m_pMain->add_process_history_machine("Grab Complete");
		}
	}
	return nRet;
}

BOOL CJob_1Cam1ShotAlign::find_pattern(int next_seq, BOOL is_auto)
{
	int nCam = 0;
	BOOL bFind = TRUE;

	int real_cam = m_pMain->vt_job_info[m_nJobID].camera_index[nCam];
	int w = m_pMain->vt_job_info[m_nJobID].camera_size[nCam].x;
	int h = m_pMain->vt_job_info[m_nJobID].camera_size[nCam].y;

	for (int nPos = 0; nPos < m_nNumPos; nPos++)
	{
		if (m_pMain->m_bSimulationStart != TRUE)
			m_pMain->copyMemory(m_pMain->getProcBuffer(real_cam, nPos), m_pMain->getSrcBuffer(real_cam), w * h);
		pMatching->findPattern(m_pMain->getProcBuffer(real_cam, nPos), nCam, nPos, w, h);

		if (pMatching->getFindInfo(nCam, nPos).GetFound() != FIND_OK)
			bFind = FALSE;
	}

	if (is_auto)
	{
		m_pMain->add_process_history("Find Pattern");
		//if (bFind) m_pMain->set_seq_process(m_nJobID, next_seq);
		if (bFind) m_nAutoSeq = next_seq;

	}
	else
	{
		m_pMain->add_process_history_machine("Find Pattern");
		if (bFind) m_calib_seq = next_seq;
	}
	return bFind;
}

int CJob_1Cam1ShotAlign::grab_retry_process(int return_seq, int manual_input_seq)
{
	int nRet = 0;
	m_pMain->increase_prealign_grab_retry(m_nJobID);

	if (m_pMain->get_prealign_grab_retry(m_nJobID) < 2)
		//	m_pMain->set_seq_process(m_nJobID, return_seq);
		m_nAutoSeq = return_seq;
	else
	{
		//KJH 2022-03-10 L-Check Error?? ???????? Manual Mark ???????? ????
		//JSH 2022-03-16 ???? ???????? L-Check ????
		if (m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getEnableAlignMeasureSpecOutJudge())
		{
			CString str;

			BOOL bFind[2];
			bFind[0] = m_pMain->GetMatching(m_nJobID).getFindInfo(0, 0).GetFound();
			bFind[1] = m_pMain->GetMatching(m_nJobID).getFindInfo(0, 1).GetFound();

			str.Format("Manual Mark Input - %d - %d", bFind[0], bFind[1]);
			m_pMain->add_process_history(str);
			::SendMessageA(m_pMain->m_pForm[FORM_MAIN]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_SHOW_MANUAL_INPUT_MARK, MAKELPARAM(m_nJobID, TRUE));

			//m_pMain->set_seq_process(m_nJobID, manual_input_seq);
			m_nAutoSeq = manual_input_seq;
		}
		else
		{
			nRet = 2;
			//	m_pMain->set_seq_process(m_nJobID, 0);
			m_nAutoSeq = 0;
		}
	}

	return nRet;
}

void CJob_1Cam1ShotAlign::init_job(int nJob)
{
	m_nJobID = nJob;
	pPrealign = &m_pMain->GetPrealign(m_nJobID);
	pMachine = &m_pMain->GetMachine(m_nJobID);
	pModel = &m_pMain->vt_job_info[m_nJobID].model_info;
	pMatching = &m_pMain->GetMatching(m_nJobID);
	pJob = &m_pMain->vt_job_info[m_nJobID];
	pResult = &m_pMain->vt_result_info[m_nJobID];
}

int CJob_1Cam1ShotAlign::calculate_revision_data()
{
	int nRet = 0;

	int nCam = 0;
	double posX = 0.0;
	double posY = 0.0;
	double worldX = 0.0, worldY = 0.0;

	// ?????? ?????? ?? ?? ??????
	for (int nPos = 0; nPos < m_nNumPos; nPos++)
	{
		posX = pMatching->getFindInfo(nCam, nPos).GetXPos();
		posY = pMatching->getFindInfo(nCam, nPos).GetYPos();

		pMachine->PixelToWorld(nCam, nPos, posX, posY, &worldX, &worldY);

		pPrealign->setPosX(nPos, worldX);
		pPrealign->setPosY(nPos, worldY);
	}

	// ?????? ????
	int nCamL = 0;
	int nCamR = 1;
	pPrealign->setPatternPitchX(pModel->getAlignInfo().getFiducialMarkPitchX());
	pPrealign->setRotateX(nCamL, pMachine->getRotateX(nCam, nCamL));
	pPrealign->setRotateY(nCamL, pMachine->getRotateY(nCam, nCamL));
	pPrealign->setRotateX(nCamR, pMachine->getRotateX(nCam, nCamR));
	pPrealign->setRotateY(nCamR, pMachine->getRotateY(nCam, nCamR));
	pPrealign->pDlg = m_pMain;

	// mm?? Offset ????
	pPrealign->setRevisionOffset(AXIS_X, pModel->getMachineInfo().getRevisionOffset(AXIS_X));
	pPrealign->setRevisionOffset(AXIS_Y, pModel->getMachineInfo().getRevisionOffset(AXIS_Y));
	pPrealign->setRevisionOffset(AXIS_T, pModel->getMachineInfo().getRevisionOffset(AXIS_T));

	if (pPrealign->calcRevision(nCamL, nCamR, m_pMain->vt_job_info[m_nJobID].model_info.getAlignInfo().getAlignmentTargetDir()) != 0)
	{
		m_pMain->add_process_history("calc Revision Failed");
		nRet = 2;

		m_nAutoSeq = 0;
		//m_pMain->set_seq_process(m_nJobID, 0);
		pResult->revision_data[0][AXIS_X] = 0.0;
		pResult->revision_data[0][AXIS_Y] = 0.0;
		pResult->revision_data[0][AXIS_T] = 0.0;
	}

	CString strTemp;
	strTemp.Format("X: %.3f, Y: %.3f, T: %.3f", pPrealign->getRevisionX(), pPrealign->getRevisionY(), pPrealign->getRevisionT());
	m_pMain->add_process_history(strTemp);
	return nRet;
}

int CJob_1Cam1ShotAlign::included_angle_check_process()
{
	int nCam = 0;
	int nRet = 0;
	CString str;

	double angle_spec = pModel->getAlignInfo().getIncludedAngle();
	double angle_limit = pModel->getAlignInfo().getIncludedAngleLimit();

	if (pModel->getAlignInfo().getMarkFindMethod() != METHOD_CALIPER) return nRet;

	for (int nPos = 0; nPos < 2; nPos++)
	{
		double angle = pMatching->getFindInfo(nCam, nPos).get_included_angle();

		if (fabs(angle - angle_spec) > angle_limit)
		{
			nRet = 2;
			/*m_pMain->m_stNgInfo.err_type[nCam][nPos] = ERR_TYPE_ANGLE_ERROR;
			m_pMain->m_stNgInfo.included_angle[nCam][nPos] = angle;

			m_pMain->m_nErrorType2[nCam][nPos] = ERR_TYPE_ANGLE_ERROR;
			m_pMain->m_dbIncludedAngle = angle;
			m_pMain->m_bIncludedAngleNG = TRUE;*/

			str.Format("[%d] Included Angle Error!! \r\nAngle: %.3f, Spec: %.3f, Limit: %.3f", nPos + 1, angle, angle_spec, angle_limit);
			m_pMain->add_process_history(str);
		}
	}

	return nRet;
}

int CJob_1Cam1ShotAlign::length_check_process(BOOL prealign_test)
{
	BOOL ret = TRUE;
	CString str;

	int nCam = 0;
	int nPos = 0;
	double pixel_x[2] = { 0, };
	double pixel_y[2] = { 0, };
	double world_x[2] = { 0, };
	double world_y[2] = { 0, };

	pixel_x[0] = pMatching->getFindInfo(nCam, nPos + 0).GetXPos();
	pixel_y[0] = pMatching->getFindInfo(nCam, nPos + 0).GetYPos();
	pixel_x[1] = pMatching->getFindInfo(nCam, nPos + 1).GetXPos();
	pixel_y[1] = pMatching->getFindInfo(nCam, nPos + 1).GetYPos();

	pMachine->PixelToWorld(0, 0, pixel_x[0], pixel_y[0], &world_x[0], &world_y[0]);
	pMachine->PixelToWorld(0, 0, pixel_x[1], pixel_y[1], &world_x[1], &world_y[1]);

	double dx = world_x[1] - world_x[0];
	double dy = world_y[1] - world_y[0];
	double dist = sqrt(dx * dx + dy * dy);
	dist += pModel->getMachineInfo().get_lcheck_offset(0);

	double dist_tor = pModel->getAlignInfo().getLCheckTor();
	double dist_spec = pModel->getAlignInfo().getLCheckSpecX();
	int target_dir = pModel->getAlignInfo().getAlignmentTargetDir();	// vertical or horizontal

	if (target_dir == 1)	// vertical
		dist_spec = pModel->getAlignInfo().getLCheckSpecY();

	str.Format("length: %.3f", dist);

	m_pMain->m_dLcheckCurrentValue[m_nJobID] = dist;

	if (prealign_test)	::SendMessageA(m_pMain->m_pPane[PANE_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
	else				::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);

	m_pMain->vt_result_info[m_nJobID].length_check[nCam][nPos] = dist;
	m_pMain->vt_result_info[m_nJobID].length_check_judge[nCam][nPos] = TRUE;

	if (fabs(dist_spec - dist) > dist_tor)
	{
		str.Format("[%s] L Check NG!!  limit: %.3f, tor: %.3f", m_pMain->vt_job_info[m_nJobID].get_job_name(), dist_spec, dist_tor);
		if (prealign_test)	::SendMessageA(m_pMain->m_pPane[PANE_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&str);
		else				::SendMessageA(m_pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&str);
		m_pMain->vt_result_info[m_nJobID].each_Judge[0][0] = FALSE;
		m_pMain->vt_result_info[m_nJobID].length_check_judge[nCam][nPos] = FALSE;
		ret = FALSE;
	}

	return ret;
}

int CJob_1Cam1ShotAlign::send_revision_data()
{
	int nRet = 0;

	double x = m_pMain->m_dbRevisionData[m_nJobID][AXIS_X] = pPrealign->getRevisionX();
	double y = m_pMain->m_dbRevisionData[m_nJobID][AXIS_Y] = pPrealign->getRevisionY();
	double t = m_pMain->m_dbRevisionData[m_nJobID][AXIS_T] = pPrealign->getRevisionT();

	m_pMain->sendRevisionData(x, y, t, m_nJobID);

	CString str;
	str.Format("X: %.4f Y: %.4f T: %.4f", m_pMain->m_dbRevisionData[m_nJobID][AXIS_X], m_pMain->m_dbRevisionData[m_nJobID][AXIS_Y], m_pMain->m_dbRevisionData[m_nJobID][AXIS_T]);
	m_pMain->add_process_history(str);
	return nRet;
}

int CJob_1Cam1ShotAlign::manual_input_mark_process(int return_seq)
{
	int nRet = 0;

	if (m_pMain->m_nManualInputMarkResult[m_nJobID] == 1)
	{
		m_pMain->KillTimer(TIMER_MANUAL_MARK);
		m_nAutoSeq = return_seq;
		//m_pMain->set_seq_process(m_nJobID, return_seq);
	}
	else if (m_pMain->m_nManualInputMarkResult[m_nJobID] == 2)
	{
		m_nAutoSeq = 0;
		//m_pMain->set_seq_process(m_nJobID, 0);
		nRet = 2;
	}

	return nRet;
}

int CJob_1Cam1ShotAlign::do_calibation_seq()
{
	CString str;
	int nRet = 0;
	int return_seq = 0;
	int next_seq = 0;

	int nLimitX = pModel->getMachineInfo().getCalibrationCountX();
	int nLimitY = pModel->getMachineInfo().getCalibrationCountY();
	int nOriLimitX = pModel->getMachineInfo().getCalibrationCountX();
	int nOriLimitY = pModel->getMachineInfo().getCalibrationCountY();

	int nCam = 0;
	int w = pJob->camera_size[0].x;
	int h = pJob->camera_size[0].y;
	int num_of_cam = pJob->num_of_camera;
	int num_of_pos = 2;

	switch (m_calib_seq) {
	case 0:
		nRet = 2;
		break;
	case 100:	// Calibration ???? ???? ????; PLC???? enable ?????? ???? ?????? calibration seq ??????.
	{
		next_seq = 200;
		nRet = calibration_enable_check(next_seq);
	}
	break;
	case 200:	// Calibration Mode ON		PLC???? calibration mode ???????????? ??????
	{
		next_seq = 250;
		nRet = calibration_mode_on(next_seq);
	}
	break;
	case 250:	// Wait Calib Mode On Ack Signal		PLC?? ack ???????? moving ???? ???? ????.
	{
		next_seq = 300;
		nRet = wait_calibration_mode_on_ack(next_seq);
	}
	break;
	case 300:	// ???????? Grab
	{
		next_seq = 400;
		nRet = grab_start(next_seq, FALSE);
		Sleep(300);
	}
	break;
	case 400:	// Grab ???? ????
	{
		next_seq = 500;
		nRet = grab_complete_check(next_seq, FALSE);
	}
	break;
	case 500:	// ???????????? ???????? ?????? ????
	{
		Sleep(1000);

		next_seq = 600;
		BOOL bFind = find_pattern(next_seq, FALSE);
		if (bFind != TRUE)	nRet = 2;
	}
	break;
	case 600:		// ???????? ???? ???? ????
	{
		int next_seq = 1000;
		memory_calibration_origin(next_seq);
	}
	break;
	case 1000:	// Calibration ???? ?????? ??????
	{
		next_seq = 1100;
		nRet = reset_calibration_data(next_seq);
	}
	break;
	case 1100:
	{
		if (++m_calib_cnt_y == nLimitY)	// Y ???? ????
		{
			m_calib_seq = 2000;
			break;
		}

		m_calib_seq = 1200;
	}
	break;
	case 1200:	// ???? ???? ???? ????
	{
		if (++m_calib_cnt_x == nOriLimitX)
		{
			m_calib_cnt_x = -1;
			m_calib_seq = 1100;
			break;
		}

		next_seq = 1300;
		nRet = calibration_move_request(next_seq);
	}
	break;
	case 1300:	// ???? ???? ???? ???? ????
	{
		next_seq = 1400;
		nRet = wait_calibration_move_ack(next_seq);
		Sleep(100);
	}
	break;
	case 1400:	// Grab ????
	{
		Sleep(2000);

		next_seq = 1500;
		nRet = grab_start(next_seq, FALSE);
	}
	break;
	case 1500:	// Grab ???? ????
	{
		next_seq = 1600;
		nRet = grab_complete_check(next_seq, FALSE);
	}
	break;
	case 1600:	// Find Pattern
	{
		Sleep(1000);

		next_seq = 1700;
		BOOL bFind = find_pattern(next_seq, FALSE);
		if (bFind != TRUE)	nRet = 2;
	}
	break;
	case 1700:
	{
		next_seq = 1200;
		add_calibration_data(next_seq);
	}
	break;
	case 2000:	// Cabliration ?????? ????
	{
		BOOL bSuccess = calculate_calibration_data();

		if (bSuccess == TRUE)	nRet = 1;
		else					nRet = 2;
	}
	break;
	}

	Sleep(100);
	return nRet;
}

int CJob_1Cam1ShotAlign::do_auto_seq()
{
	int nRet = 0;
	int return_seq = 0;
	int next_seq = 0;
	int manual_input_seq = 0;

	if (m_pMain == NULL) return 2;

	switch (m_pMain->get_seq_process(m_nJobID)) {	// process_1cam_1shot_align()
	case 0:
		break;
	case 100:
	{
		// ???????? ???? ????.
		next_seq = 200;
		nRet = grab_start(next_seq, TRUE);
	}
	break;
	case 200:
	{
		// ???????? ???? ???????? ???? ????.
		next_seq = 300;
		nRet = grab_complete_check(next_seq, TRUE);
	}
	break;
	case 300:
	{
		// ?????? ????.
		return_seq = 100;
		next_seq = 400;
		manual_input_seq = 500;
		BOOL bFind = find_pattern(next_seq, TRUE);

		// ?? ???? ???? ?????????? retry process
		if (bFind != TRUE)	nRet = grab_retry_process(return_seq, manual_input_seq);
	}
	break;
	case 400:
	{
		// ???????? ???? ????
		nRet = calculate_revision_data();
		if (nRet == 2)	break;

		// ???????? ???? ????
		// kbj 2021-12-25 1cam1shot prcoess	?????????????? ?????? ?????? ?????? ????.
		if (m_pMain->m_bSimulationStart != TRUE)
		{
			send_revision_data();
		}

		// L Check ????
		if (pModel->getAlignInfo().getEnableLCheck())
		{
			//BOOL ret = m_pMain->calc_lcheck_1cam_2pos(m_nJobID);
			BOOL ret = length_check_process(FALSE);
			if (ret == TRUE)
			{
				nRet = 2;
				break;
			}
		}

		// ?????? ????
		if (pModel->getAlignInfo().getUseIncludedAngle())
		{
			nRet = included_angle_check_process();
			if (nRet == 2)	break;
		}

		m_pMain->set_seq_process(m_nJobID, 0);
		nRet = 1;
	}
	break;
	case 500:
	{
		// ???? ???? ???? ???? ????
		return_seq = 400;
		nRet = manual_input_mark_process(return_seq);
	}
	break;
	case 1000:
	{
		// ????????????
		find_pattern(400, TRUE);
	}
	break;
	}

	return nRet;
}

void CJob_1Cam1ShotAlign::simulation()
{
	BYTE* pImage = m_pMain->getCameraViewBuffer();

	int w = pJob->camera_size[m_nJobID].x;
	int h = pJob->camera_size[m_nJobID].y;

	pMatching->findPattern(pImage, 0, 0, w, h);
	pMatching->findPattern(pImage, 0, 1, w, h);

	m_pMain->m_pDlgCaliper->m_pCaliperParam->updateUIFromData();
	m_pMain->m_pDlgCaliper->m_pCaliperParam->updateCoordinates();

	CViewerEx* pViewer = &((CFormCameraView*)m_pMain->m_pForm[FORM_CAMERA])->m_ViewerCamera;
	draw_result(0, 0, pViewer, TRUE);
	draw_result(0, 1, pViewer, FALSE);
}

void CJob_1Cam1ShotAlign::draw_result_Matching(int nCam, int nPos, CViewerEx* pViewer, BOOL bErase)
{
	if (bErase)
	{
		pViewer->ClearOverlayDC();
		pViewer->clearAllFigures();
	}

	CString strText;
	int nTextX, nTextY;
	COLORREF color;
	stFigure tempFig;
	stFigureText tempFigText;

	CFindInfo* pInfo = &pMatching->getFindInfo(nCam, nPos);

	////////////// Mark?? ROI
	if (pInfo->get_mark_found() == FIND_OK)					color = COLOR_LIME;
	else if (pInfo->get_mark_found() == FIND_ERR)			color = COLOR_RED;
	else													color = COLOR_YELLOW;

	double posX = pInfo->GetXPos();
	double posY = pInfo->GetYPos();
	double score = pInfo->getScore();
	int index = pInfo->GetFoundPatternNum();

	CRect rectROI = pMatching->getSearchROI(nCam, nPos);

	int nMarkSize = 40;
	tempFig.ptBegin.x = posX - nMarkSize;
	tempFig.ptBegin.y = posY;
	tempFig.ptEnd.x = posX + nMarkSize;
	tempFig.ptEnd.y = posY;
	pViewer->addFigureLine(tempFig, 1, 1, color);

	tempFig.ptBegin.x = posX;
	tempFig.ptBegin.y = posY - nMarkSize;
	tempFig.ptEnd.x = posX;
	tempFig.ptEnd.y = posY + nMarkSize;
	pViewer->addFigureLine(tempFig, 1, 1, color);


	// ???? ROI 
	tempFig.ptBegin.x = rectROI.left;
	tempFig.ptBegin.y = rectROI.top;
	tempFig.ptEnd.x = rectROI.right;
	tempFig.ptEnd.y = rectROI.bottom;
	pViewer->addFigureRect(tempFig, 1, 1, COLOR_WHITE);


	// Draw Text
	posX = pInfo->GetXPos();
	posY = pInfo->GetYPos();

	int offY = 100;
	nTextX = 20;
	nTextY = offY + 80 * nPos;

	strText.Format("[%d - %d] X: %.3f Y: %.3f - %.3f%% [%d]", nCam + 1, nPos + 1, posX, posY, score, index + 1);

	tempFigText.ptBegin.x = nTextX;
	tempFigText.ptBegin.y = nTextY;
	tempFigText.ptBeginFit.x = nTextX;
	tempFigText.ptBeginFit.y = nTextY;
	tempFigText.textString = strText;
	pViewer->addFigureText(tempFigText, 15, 15, COLOR_LIME);

	pViewer->Invalidate();
}

void CJob_1Cam1ShotAlign::draw_result_RANSAC(int nCam, int nPos, CViewerEx* pViewer, BOOL bErase)
{
	if (bErase)
	{
		pViewer->clearAllFigures();
		pViewer->ClearOverlayDC();
	}

	CString strText;
	int nTextX, nTextY;
	stFigure tempFig;
	stFigureText tempFigText;
	COLORREF color;
	CFindInfo* pInfo = &pMatching->getFindInfo(nCam, nPos);
	CRect rectROI = pMatching->getSearchROI(nCam, nPos);

	// ROI ????
	tempFig.ptBegin.x = rectROI.left;
	tempFig.ptBegin.y = rectROI.top;
	tempFig.ptEnd.x = rectROI.right;
	tempFig.ptEnd.y = rectROI.bottom;
	pViewer->addFigureLine(tempFig, 1, 1, COLOR_WHITE);

	// ???? ???? ?????? ???? ROI
	rectROI = pMatching->m_rcRealROI[nCam][nPos][0];
	tempFig.ptBegin.x = rectROI.left;
	tempFig.ptBegin.y = rectROI.top;
	tempFig.ptEnd.x = rectROI.right;
	tempFig.ptEnd.y = rectROI.bottom;
	pViewer->addFigureLine(tempFig, 1, 1, COLOR_YELLOW);

	rectROI = pMatching->m_rcRealROI[nCam][nPos][1];
	tempFig.ptBegin.x = rectROI.left;
	tempFig.ptBegin.y = rectROI.top;
	tempFig.ptEnd.x = rectROI.right;
	tempFig.ptEnd.y = rectROI.bottom;
	pViewer->addFigureLine(tempFig, 1, 1, COLOR_YELLOW);


	if (pInfo->GetFound() == FIND_OK)
	{
		sLine line_hori = pMatching->m_lineHori[nCam][nPos];
		sLine line_vert = pMatching->m_lineVert[nCam][nPos];

		tempFig.ptBegin.x = line_hori.pt[0].x;
		tempFig.ptBegin.y = line_hori.pt[0].y;
		tempFig.ptEnd.x = line_hori.pt[1].x;
		tempFig.ptEnd.y = line_hori.pt[1].y;
		pViewer->addFigureLine(tempFig, 1, 1, COLOR_LIME);

		tempFig.ptBegin.x = line_vert.pt[0].x;
		tempFig.ptBegin.y = line_vert.pt[0].y;
		tempFig.ptEnd.x = line_vert.pt[1].x;
		tempFig.ptEnd.y = line_vert.pt[1].y;
		pViewer->addFigureLine(tempFig, 1, 1, COLOR_LIME);
	}

	// cross hair
	if (pInfo->GetFound() == FIND_OK)
	{
		int nSize = 20;
		tempFig.ptBegin.x = pInfo->GetXPos() - nSize;
		tempFig.ptBegin.y = pInfo->GetYPos();
		tempFig.ptEnd.x = pInfo->GetXPos() + nSize;
		tempFig.ptEnd.y = pInfo->GetYPos();
		pViewer->addFigureLine(tempFig, 1, 1, COLOR_BLUE);

		tempFig.ptBegin.x = pInfo->GetXPos();
		tempFig.ptBegin.y = pInfo->GetYPos() - nSize;
		tempFig.ptEnd.x = pInfo->GetXPos();
		tempFig.ptEnd.y = pInfo->GetYPos() + nSize;
		pViewer->addFigureLine(tempFig, 1, 1, COLOR_BLUE);
	}

	// Draw Text
	double posX = pInfo->GetXPos();
	double posY = pInfo->GetYPos();

	int offY = 100;
	nTextX = 20;
	nTextY = offY + 80 * nPos;

	strText.Format("[%d - %d] X: %.3f Y: %.3f", nCam + 1, nPos + 1, posX, posY);
	tempFigText.ptBegin.x = nTextX;
	tempFigText.ptBegin.y = nTextY;
	tempFigText.ptBeginFit.x = nTextX;
	tempFigText.ptBeginFit.y = nTextY;
	tempFigText.textString = strText;
	pViewer->addFigureText(tempFigText, 15, 15, COLOR_LIME);


	if (pModel->getAlignInfo().getUseIncludedAngle())
	{
		double angle = pMatching->getFindInfo(nCam, nPos).get_included_angle();
		double angle_spec = pModel->getAlignInfo().getIncludedAngle();
		double angle_limit = pModel->getAlignInfo().getIncludedAngleLimit();

		if (fabs(angle - angle_spec) > angle_limit)	color = COLOR_RED;
		else										color = COLOR_LIME;

		offY = 260;
		nTextY = offY + 80 * nPos;

		strText.Format("[%d - %d] Angle: %.3f??", nCam + 1, nPos + 1, angle);

		tempFigText.ptBegin.x = nTextX;
		tempFigText.ptBegin.y = nTextY;
		tempFigText.ptBeginFit.x = nTextX;
		tempFigText.ptBeginFit.y = nTextY;
		tempFigText.textString = strText;
		pViewer->addFigureText(tempFigText, 15, 15, color);
	}

	pViewer->Invalidate();
}

void CJob_1Cam1ShotAlign::draw_result_Caliper(int nCam, int nPos, CViewerEx* pViewer, BOOL bErase)
{
	if (bErase)
	{
		pViewer->ClearOverlayDC();
		pViewer->clearAllFigures();

	}
	auto pDC = pViewer->getOverlayDC();

	CString strText;
	int nTextX, nTextY;
	COLORREF color;
	stFigure tempFig;
	stFigureText tempFigText;
	CFindInfo* pInfo = &pMatching->getFindInfo(nCam, nPos);

	////////////// Mark?? ROI
	if (pInfo->get_mark_found() == FIND_OK)			color = COLOR_LIME;
	else if (pInfo->get_mark_found() == FIND_ERR)	color = COLOR_RED;
	else											color = COLOR_YELLOW;

	double posX = pInfo->get_mark_x();
	double posY = pInfo->get_mark_y();
	CRect rectROI = pMatching->getSearchROI(nCam, nPos);

	int nSize = 40;
	// draw mark
	tempFig.ptBegin.x = posX - nSize;
	tempFig.ptBegin.y = posY;
	tempFig.ptEnd.x = posX + nSize;
	tempFig.ptEnd.y = posY;
	pViewer->addFigureLine(tempFig, 1, 1, color);

	tempFig.ptBegin.x = posX;
	tempFig.ptBegin.y = posY - nSize;
	tempFig.ptEnd.x = posX;
	tempFig.ptEnd.y = posY + nSize;
	pViewer->addFigureLine(tempFig, 1, 1, color);

	// draw roi
	tempFig.ptBegin.x = rectROI.left;
	tempFig.ptBegin.y = rectROI.top;
	tempFig.ptEnd.x = rectROI.right;
	tempFig.ptEnd.y = rectROI.bottom;
	pViewer->addFigureRect(tempFig, 1, 1, COLOR_WHITE);

	int real_cam = pJob->camera_index[nCam];

	// Draw Lines
	for (int i = 0; i < 2; i++)
	{
		m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][i].draw_final_result(pDC);

		BOOL bFindLine = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][i].getIsMakeLine();
		sLine line_info = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][i].m_lineInfo;

		if (bFindLine)
		{
			tempFig.ptBegin.x = line_info.pt[0].x;
			tempFig.ptBegin.y = line_info.pt[0].y;
			tempFig.ptEnd.x = line_info.pt[1].x;
			tempFig.ptEnd.y = line_info.pt[1].y;
			pViewer->addFigureLine(tempFig, 1, 1, COLOR_LIME);
		}
	}

	// cross hair
	if (pInfo->GetFound() == FIND_OK)
	{
		int nSize = 20;
		tempFig.ptBegin.x = pInfo->GetXPos() - nSize;
		tempFig.ptBegin.y = pInfo->GetYPos();
		tempFig.ptEnd.x = pInfo->GetXPos() + nSize;
		tempFig.ptEnd.y = pInfo->GetYPos();
		pViewer->addFigureLine(tempFig, 1, 1, COLOR_BLUE);

		tempFig.ptBegin.x = pInfo->GetXPos();
		tempFig.ptBegin.y = pInfo->GetYPos() - nSize;
		tempFig.ptEnd.x = pInfo->GetXPos();
		tempFig.ptEnd.y = pInfo->GetYPos() + nSize;
		pViewer->addFigureLine(tempFig, 1, 1, COLOR_BLUE);
	}

	// Draw Text
	posX = pInfo->GetXPos();
	posY = pInfo->GetYPos();

	int offY = 100;
	nTextX = 20;
	nTextY = offY + 80 * nPos;

	strText.Format("[%d - %d] X: %.3f Y: %.3f", nCam + 1, nPos + 1, posX, posY);
	tempFigText.ptBegin.x = nTextX;
	tempFigText.ptBegin.y = nTextY;
	tempFigText.ptBeginFit.x = nTextX;
	tempFigText.ptBeginFit.y = nTextY;
	tempFigText.textString = strText;
	pViewer->addFigureText(tempFigText, 15, 15, COLOR_LIME);

	if (pModel->getAlignInfo().getUseIncludedAngle())
	{
		double angle = pMatching->getFindInfo(nCam, nPos).get_included_angle();
		double angle_spec = pModel->getAlignInfo().getIncludedAngle();
		double angle_limit = pModel->getAlignInfo().getIncludedAngleLimit();

		if (fabs(angle - angle_spec) > angle_limit)	pDC->SetTextColor(COLOR_RED);
		else										pDC->SetTextColor(COLOR_GREEN);

		offY = 260;
		nTextY = offY + 80 * nPos;

		strText.Format("[%d - %d] Angle: %.3f??", nCam + 1, nPos + 1, angle);
		tempFigText.ptBegin.x = nTextX;
		tempFigText.ptBegin.y = nTextY;
		tempFigText.ptBeginFit.x = nTextX;
		tempFigText.ptBeginFit.y = nTextY;
		tempFigText.textString = strText;
		pViewer->addFigureText(tempFigText, 15, 15, COLOR_LIME);
	}

	pViewer->Invalidate();
}

void CJob_1Cam1ShotAlign::draw_result(int nCam, int nPos, CViewerEx* pViewer, BOOL bErase)
{
	int nMethod = pModel->getAlignInfo().getMarkFindMethod();

	switch (nMethod) {
	case METHOD_MATCHING:	draw_result_Matching(nCam, nPos, pViewer, bErase);	break;
	case METHOD_RANSAC:		draw_result_RANSAC(nCam, nPos, pViewer, bErase);	break;
	case METHOD_CALIPER:	draw_result_Caliper(nCam, nPos, pViewer, bErase);	break;
	}
}

void CJob_1Cam1ShotAlign::draw_mark(FakeDC* pDC, double posX, double posY, int size)
{
	pDC->AddPoint(float(posX), float(posY), size, 2);
	//pDC->MoveTo(posX - size, posY);
	//pDC->LineTo(posX + size, posY);
	//pDC->MoveTo(posX, posY - size);
	//pDC->LineTo(posX, posY + size);
}

void CJob_1Cam1ShotAlign::draw_roi(FakeDC* pDC, CRect rectROI)
{
	pDC->AddGraphic(new GraphicRectangle(rectROI));
	//pDC->MoveTo(rectROI.left, rectROI.top);
	//pDC->LineTo(rectROI.right, rectROI.top);
	//pDC->LineTo(rectROI.right, rectROI.bottom);
	//pDC->LineTo(rectROI.left, rectROI.bottom);
	//pDC->LineTo(rectROI.left, rectROI.top);
}

void CJob_1Cam1ShotAlign::save_image(_stSaveImageInfo* pInfo)
{
	int nCam = pInfo->nCam;
	int nPos = pInfo->nPos;
	int bJudge = pInfo->bJudge;
	int nIndex = pInfo->nIndex;

	int real_cam = pJob->camera_index[nCam];
	int w = pJob->camera_size[nCam].x;
	int h = pJob->camera_size[nCam].y;

	CString str;
	cv::Mat matGrayImage(h, w, CV_8UC1, m_pMain->getSaveImageBuffer(real_cam, nIndex));
	cv::Mat matColorImage;
	cv::cvtColor(matGrayImage, matColorImage, CV_GRAY2BGR);

	cv::Scalar color = cv::Scalar(0, 0, 255);
	cv::Scalar colorOK = cv::Scalar(0, 255, 0);
	cv::Scalar colorNG = cv::Scalar(0, 0, 255);
	cv::Scalar colorMatch = cv::Scalar(255, 0, 255);
	cv::Scalar colorLine = cv::Scalar(255, 0, 0);	// B G R

	int nOffset = 20;

	// draw mark & line
	for (int nPos = 0; nPos < 2; nPos++)
	{
		if (pInfo->nFound[nPos] == FIND_ERR)	continue;

		if (bJudge)		color = colorOK;
		else
		{
			if (pInfo->nFound[nPos] == FIND_OK)	color = colorOK;
			else								color = colorMatch;
		}

		if (pModel->getAlignInfo().getMarkFindMethod() == METHOD_MATCHING)
		{
			// draw mark
			//HTK 2022-06-14 ???? ???????? ?????? ???? ???????????? ????(1Cam1ShotAlign)
			line(matColorImage, cv::Point((int)pInfo->dPosX[nPos] - nOffset, (int)pInfo->dPosY[nPos]), cv::Point((int)pInfo->dPosX[nPos] + nOffset, (int)pInfo->dPosY[nPos]), cv::Scalar(255, 0, 0), 2);
			line(matColorImage, cv::Point((int)pInfo->dPosX[nPos], (int)pInfo->dPosY[nPos] - nOffset), cv::Point((int)pInfo->dPosX[nPos], (int)pInfo->dPosY[nPos] + nOffset), cv::Scalar(255, 0, 0), 2);

			str.Format("[%d] X: %.1f Y: %.1f T: %.1f  %.1f%% - %d ", nPos + 1, pInfo->dPosX[nPos], pInfo->dPosY[nPos], pInfo->dPatAngle[nPos], pInfo->dScore[nPos], pInfo->nFindIndex[nPos] + 1);
			putText(matColorImage, std::string(str), cv::Point(50, 150 + nPos * 100), cv::FONT_HERSHEY_SIMPLEX, 2., color, 2);
		}

		if (pModel->getAlignInfo().getMarkFindMethod() == METHOD_RANSAC)
		{
			// draw line
			line(matColorImage, cv::Point((int)pInfo->dLineX[nPos][0], (int)pInfo->dLineY[nPos][0]), cv::Point((int)pInfo->dLineX[nPos][1], (int)pInfo->dLineY[nPos][1]), color, 2);
			line(matColorImage, cv::Point((int)pInfo->dLineX[nPos][2], (int)pInfo->dLineY[nPos][2]), cv::Point((int)pInfo->dLineX[nPos][3], (int)pInfo->dLineY[nPos][3]), color, 2);

			// draw cross hair
			line(matColorImage, cv::Point((int)pInfo->dPosX[nPos] - nOffset, (int)pInfo->dPosY[nPos]), cv::Point((int)pInfo->dPosX[nPos] + nOffset, (int)pInfo->dPosY[nPos]), cv::Scalar(255, 0, 0), 2);
			line(matColorImage, cv::Point((int)pInfo->dPosX[nPos], (int)pInfo->dPosY[nPos] - nOffset), cv::Point((int)pInfo->dPosX[nPos], (int)pInfo->dPosY[nPos] + nOffset), cv::Scalar(255, 0, 0), 2);

			str.Format("[%d] X: %.1f Y: %.1f ", nPos + 1, pInfo->dPosX[nPos], pInfo->dPosY[nPos]);
			putText(matColorImage, std::string(str), cv::Point(50, 150 + nPos * 100), cv::FONT_HERSHEY_SIMPLEX, 2., color, 2);
		}

		if (pModel->getAlignInfo().getMarkFindMethod() == METHOD_CALIPER)
		{
			// draw line
			line(matColorImage, cv::Point((int)pInfo->dLineX[nPos][0], (int)pInfo->dLineY[nPos][0]), cv::Point((int)pInfo->dLineX[nPos][1], (int)pInfo->dLineY[nPos][1]), color, 2);
			line(matColorImage, cv::Point((int)pInfo->dLineX[nPos][2], (int)pInfo->dLineY[nPos][2]), cv::Point((int)pInfo->dLineX[nPos][3], (int)pInfo->dLineY[nPos][3]), color, 2);

			// draw cross hair
			line(matColorImage, cv::Point((int)pInfo->dPosX[nPos] - nOffset, (int)pInfo->dPosY[nPos]), cv::Point((int)pInfo->dPosX[nPos] + nOffset, (int)pInfo->dPosY[nPos]), cv::Scalar(255, 0, 0), 2);
			line(matColorImage, cv::Point((int)pInfo->dPosX[nPos], (int)pInfo->dPosY[nPos] - nOffset), cv::Point((int)pInfo->dPosX[nPos], (int)pInfo->dPosY[nPos] + nOffset), cv::Scalar(255, 0, 0), 2);

			// draw mark
			line(matColorImage, cv::Point((int)pInfo->mark_pos_x[nPos] - nOffset, (int)pInfo->mark_pos_y[nPos]), cv::Point((int)pInfo->mark_pos_x[nPos] + nOffset, (int)pInfo->mark_pos_y[nPos]), cv::Scalar(255, 0, 0), 2);
			line(matColorImage, cv::Point((int)pInfo->mark_pos_x[nPos], (int)pInfo->mark_pos_y[nPos] - nOffset), cv::Point((int)pInfo->mark_pos_x[nPos], (int)pInfo->mark_pos_y[nPos] + nOffset), cv::Scalar(255, 0, 0), 2);

			str.Format("[%d] X: %.1f Y: %.1f ", nPos + 1, pInfo->dPosX[nPos], pInfo->dPosY[nPos]);
			putText(matColorImage, std::string(str), cv::Point(50, 150 + nPos * 100), cv::FONT_HERSHEY_SIMPLEX, 2., color, 2);
		}

		// draw ROI
		CRect rect = pMatching->getSearchROI(nCam, nPos);
		cv::Rect rectROI;
		rectROI.x = rect.left;
		rectROI.y = rect.top;
		rectROI.width = rect.Width();
		rectROI.height = rect.Height();

		cv::rectangle(matColorImage, rectROI, cv::Scalar(255, 255, 255));
	}

	// draw judge
	str.Format("X: %.4f Y: %.4f T: %.4f", pInfo->dRevision[AXIS_X], pInfo->dRevision[AXIS_Y], pInfo->dRevision[AXIS_T]);
	putText(matColorImage, std::string(str), cv::Point(w / 2, h / 2), cv::FONT_HERSHEY_SIMPLEX, 1., color, 2);

	if (bJudge)		// OK
	{
		str.Format("Retry: %d", pInfo->nRetry);
		putText(matColorImage, std::string(str), cv::Point(50, 50), cv::FONT_HERSHEY_SIMPLEX, 2., color, 2);
	}
	else
	{
		if (pInfo->bCrackNG)			str = "Crack NG";
		else if (pInfo->bAngleNG)		str = "Angle NG";
		else if (pInfo->bLcheckNG)		str = "Angle NG";
		else if (pInfo->bDummyDirNG)	str = "Dummy Dir NG";
		else							str = "NG";

		putText(matColorImage, std::string(str), cv::Point(50, 50), cv::FONT_HERSHEY_SIMPLEX, 1., color, 2);
	}

	cv::imwrite(m_pMain->m_cSaveImageRingBufPath[m_nJobID][nCam][nIndex], matColorImage);

	cv::imwrite(m_pMain->m_cSaveImageRingBufPath_Raw[m_nJobID][nCam][nIndex], matGrayImage);
	matColorImage.release();

	str.Format("Cam: %d - %d Image Save Complete ", nCam, nIndex);
	m_pMain->add_process_history(str);
}

void CJob_1Cam1ShotAlign::write_process_log(BOOL bJudge)
{
	CStdioFile WriteFile;
	SYSTEMTIME	csTime;
	CString strTemp, strTime;
	CString strSerial = pJob->get_main_object_id();
	CString strFileDir = m_pMain->m_strResultDir + m_pMain->m_strResultDate[m_nJobID];
	BOOL bWriteHeader = FALSE;

	strFileDir = m_pMain->m_strResultDir + "Files\\";
	if (_access(strFileDir, 0) != 0)	CreateDirectory(strFileDir, NULL);
	strFileDir = m_pMain->m_strResultDir + "Files\\" + m_pMain->m_strResultDate[m_nJobID];
	if (_access(strFileDir, 0) != 0)	CreateDirectory(strFileDir, NULL);
	strFileDir.Format("%sFiles\\%s\\%s", m_pMain->m_strResultDir, m_pMain->m_strResultDate[m_nJobID], m_pMain->m_strCurrentModelName);
	if (_access(strFileDir, 0) != 0)	CreateDirectory(strFileDir, NULL);
	strTemp.Format("%s\\%s_Result.csv", strFileDir, m_pMain->m_strResultDate[m_nJobID]);

	if ((_access(strTemp, 0)) == -1)	bWriteHeader = TRUE;

	::GetLocalTime(&csTime);
	strTime.Format("%02d:%02d:%02d.%03d ", csTime.wHour, csTime.wMinute, csTime.wSecond, csTime.wMilliseconds);

	if (!WriteFile.Open(strTemp, CFile::modeNoTruncate | CFile::modeCreate | CFile::modeWrite | CFile::shareDenyNone | CFile::typeText))
	{
		return;
	}

	if (bWriteHeader)
	{
		strTemp = "MODEL ID,CELL ID,TIME,MARK_X1,MARK_Y1,MARK_X2,MARK_Y2,X,Y,T,MARK1-SCORE(%),MARK2-SCORE(%),JUDGE\n";
		WriteFile.WriteString(strTemp);
	}

	WriteFile.SeekToEnd();

	strTemp.Format(_T("%s,%s,%s,%.1f,%.1f,%.1f,%.1f,%.3f,%.3f,%.3f,%.1f,%.1f, %s\n"),
		m_pMain->m_strCurrentModelName,
		strSerial,
		strTime,
		pMatching->getFindInfo(0, 0).GetXPos(),
		pMatching->getFindInfo(0, 0).GetYPos(),
		pMatching->getFindInfo(0, 1).GetXPos(),
		pMatching->getFindInfo(0, 1).GetYPos(),
		m_pMain->m_dbRevisionData[m_nJobID][AXIS_X],
		m_pMain->m_dbRevisionData[m_nJobID][AXIS_Y],
		m_pMain->m_dbRevisionData[m_nJobID][AXIS_T],
		pMatching->getFindInfo(0, 0).getScore(),
		pMatching->getFindInfo(0, 1).getScore(),
		bJudge ? "OK" : "NG"
	);

	WriteFile.WriteString(strTemp);
	WriteFile.Close();
}

void CJob_1Cam1ShotAlign::save_result_image(BOOL bJudge, BOOL bRetryEnd)
{
	CString str, str_ImageType;
	SYSTEMTIME time;
	::GetLocalTime(&time);

	double posX = 0.0, posY = 0.0;
	int of = 20;

	CString strImageDir = m_pMain->m_strImageDir + m_pMain->m_strResultDate[m_nJobID];
	if (_access(strImageDir, 0) != 0)	CreateDirectory(strImageDir, NULL);

	strImageDir.Format("%s\\%s", strImageDir, pModel->getModelID());
	if (_access(strImageDir, 0) != 0)	CreateDirectory(strImageDir, NULL);

	strImageDir.Format("%s\\%s", strImageDir, pJob->get_job_name());
	if (_access(strImageDir, 0) != 0)	CreateDirectory(strImageDir, NULL);

	CString strTime = _T("");
	strTime.Format(_T("%02d_%02d_%02d_%03d"), time.wHour, time.wMinute, time.wSecond, time.wMilliseconds);


	{
		if (bJudge)
		{
			strImageDir.Format("%s\\OK", strImageDir);
			if (_access(strImageDir, 0) != 0)	CreateDirectory(strImageDir, NULL);
		}
		else
		{
			strImageDir.Format("%s\\NG", strImageDir);
			if (_access(strImageDir, 0) != 0)	CreateDirectory(strImageDir, NULL);
		}

		if (strlen(pJob->get_main_object_id()) <= 13)
		{
			strImageDir.Format("%s\\Not Exist Panel ID", strImageDir);
			if (_access(strImageDir, 0) != 0)	CreateDirectory(strImageDir, NULL);
		}

		strImageDir.Format("%s\\%s", strImageDir, pJob->get_main_object_id());
		if (_access(strImageDir, 0) != 0)	CreateDirectory(strImageDir, NULL);

		int nCam = 0;
		int nPos = 0;
		int index = m_pMain->m_nSaveImageRingBufIndex[m_nJobID][nCam];
		int real_cam = pJob->camera_index[nCam];
		int w = pJob->camera_size[nCam].x;
		int h = pJob->camera_size[nCam].y;
		//KMB 220924 SYSTEM OPTION?? ???? ?????????? ?????? ????
		if (m_pMain->vt_system_option[real_cam].save_image_type == FALSE)
			str_ImageType = "jpg";
		else
			str_ImageType = "bmp";
		
		/*str.Format("%s\\%s\\%s\\%s_%s_Img.jpg", strImageDir, bJudge ? "OK" : "NG", pJob->get_main_object_id(), pJob->get_main_object_id(), strTime);
		sprintf(m_pMain->m_cSaveImageRingBufPath[m_nJobID][nCam][index], "%s", str);
		str.Format("%s\\%s\\%s\\%s_%s_ImgRaw.jpg", strImageDir, bJudge ? "OK": "NG", pJob->get_main_object_id(), pJob->get_main_object_id(), strTime);
		sprintf(m_pMain->m_cSaveImageRingBufPath_Raw[m_nJobID][nCam][index], "%s", str);*/

		if (strlen(pJob->get_main_object_id()) <= 13)
		{
			str.Format("%s\\NO_NAME_%s_%s_Img.%s", strImageDir,		// ?????? ???? ????????
				strTime,											// ????
				m_pMain->m_stCamInfo[real_cam].cName,				// ?????? ????
				str_ImageType);										// Image Type					
		}
		else
		{
			//str.Format("%s\\%s\\%s\\%s\\%s_%s_%s_Img.jpg", strImageDir, bJudge ? "OK" : "NG", str_algo, vt_job_info[algo].main_object_id.c_str(), vt_job_info[algo].main_object_id.c_str(), strTime, m_stCamInfo[real_cam].cName);
			str.Format("%s\\%s_%s_%s_Img.%s", strImageDir,		// ?????? ???? ????????
				pJob->get_main_object_id(),		// ????????
				strTime,										// ????
				m_pMain->m_stCamInfo[real_cam].cName,				// ?????? ????
				str_ImageType);										// Image Type
		}
		sprintf(m_pMain->m_cSaveImageRingBufPath[m_nJobID][nCam][index], _T("%s"), (LPCTSTR)str);

		//KJH 2021-08-14 PANEL ID ?????? ???? ?????? ?????? ????
		if (strlen(pJob->get_main_object_id()) <= 13)
		{
			//str.Format("%s\\%s\\%s\\%s\\%s_%s_%s_ImgRaw.jpg", strImageDir, bJudge ? "OK" : "NG", str_algo, vt_job_info[algo].main_object_id.c_str(), vt_job_info[algo].main_object_id.c_str(), strTime, m_stCamInfo[real_cam].cName);
			str.Format("%s\\NO_NAME_%s_%s_ImgRaw.%s", strImageDir,		// ?????? ???? ????????
				strTime,												// ????
				m_pMain->m_stCamInfo[real_cam].cName,					// ?????? ????
				str_ImageType);											// Image Type
		}
		else
		{
			//str.Format("%s\\%s\\%s\\%s\\%s_%s_%s_ImgRaw.jpg", strImageDir, bJudge ? "OK" : "NG", str_algo, vt_job_info[algo].main_object_id.c_str(), vt_job_info[algo].main_object_id.c_str(), strTime, m_stCamInfo[real_cam].cName);
			str.Format("%s\\%s_%s_%s_ImgRaw.%s", strImageDir,			// ?????? ???? ????????
				pJob->get_main_object_id(),								// ????????
				strTime,												// ????
				m_pMain->m_stCamInfo[real_cam].cName,					// ?????? ????
				str_ImageType);											// Image Type
		}
		sprintf(m_pMain->m_cSaveImageRingBufPath_Raw[m_nJobID][nCam][index], _T("%s"), (LPCTSTR)str);

		/*str.Format("%s\\%s\\%s\\%s_%s_Img.jpg", strImageDir, bJudge ? "OK" : "NG", pJob->get_main_object_id(), pJob->get_main_object_id(), strTime);
		sprintf(m_pMain->m_cSaveImageRingBufPath[m_nJobID][nCam][index], "%s", str);
		str.Format("%s\\%s\\%s\\%s_%s_ImgRaw.jpg", strImageDir, bJudge ? "OK" : "NG", pJob->get_main_object_id(), pJob->get_main_object_id(), strTime);
		sprintf(m_pMain->m_cSaveImageRingBufPath_Raw[m_nJobID][nCam][index], "%s", str);*/


		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].dPosX[0] = pMatching->getFindInfo(nCam, nPos).GetXPos();
		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].dPosY[0] = pMatching->getFindInfo(nCam, nPos).GetYPos();
		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].dPosX[1] = pMatching->getFindInfo(nCam, nPos + 1).GetXPos();
		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].dPosY[1] = pMatching->getFindInfo(nCam, nPos + 1).GetYPos();
		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].nFound[0] = pMatching->getFindInfo(nCam, nPos).GetFound();
		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].nFound[1] = pMatching->getFindInfo(nCam, nPos + 1).GetFound();

		if (pModel->getAlignInfo().getMarkFindMethod() == METHOD_CALIPER)
		{
			m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].mark_pos_x[0] = pMatching->getFindInfo(nCam, nPos).get_mark_x();
			m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].mark_pos_y[0] = pMatching->getFindInfo(nCam, nPos).get_mark_y();
			m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].mark_pos_x[1] = pMatching->getFindInfo(nCam, nPos + 1).get_mark_x();
			m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].mark_pos_y[1] = pMatching->getFindInfo(nCam, nPos + 1).get_mark_y();
			m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].mark_score[0] = pMatching->getFindInfo(nCam, nPos).get_mark_score();
			m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].mark_score[1] = pMatching->getFindInfo(nCam, nPos + 1).get_mark_score();
			m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].mark_found[0] = pMatching->getFindInfo(nCam, nPos).get_mark_found();
			m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].mark_found[1] = pMatching->getFindInfo(nCam, nPos + 1).get_mark_found();
		}

		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].nCam = nCam;
		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].nPos = 0;
		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].nAlgorithm = m_nJobID;
		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].dRevision[AXIS_X] = m_pMain->m_dbRevisionData[m_nJobID][AXIS_X];
		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].dRevision[AXIS_Y] = m_pMain->m_dbRevisionData[m_nJobID][AXIS_Y];
		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].dRevision[AXIS_T] = m_pMain->m_dbRevisionData[m_nJobID][AXIS_T];
		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].bJudge = bJudge;
		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].nRetry = m_pMain->m_nRetryCount[m_nJobID] - 1;
		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].bFinal = bRetryEnd;
		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].nNumPos = 2;
		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].bAngleNG = FALSE;
		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].nIndex = index;

		if (pModel->getAlignInfo().getUseIncludedAngle())
		{
			if (pResult->included_angle_judge[nCam][nPos])
			{
				m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].bAngleNG = TRUE;
				m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].dbIncludedAngle = pResult->included_angle[nCam][nPos];
			}
		}

		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].bLcheckNG = FALSE;
		if (pModel->getAlignInfo().getEnableLCheck())
		{
			if (pResult->length_check_judge[nCam][nPos])
			{
				m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].bLcheckNG = TRUE;
				m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].dbLcheckLength = pResult->length_check[nCam][nPos];
			}
		}

		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].dLineX[0][0] = pMatching->m_lineHori[nCam][nPos + 0].pt[0].x;	// ?????? ???? X
		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].dLineY[0][0] = pMatching->m_lineHori[nCam][nPos + 0].pt[0].y;	// ?????? ???? Y
		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].dLineX[0][1] = pMatching->m_lineHori[nCam][nPos + 0].pt[1].x;	// ?????? ?? X
		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].dLineY[0][1] = pMatching->m_lineHori[nCam][nPos + 0].pt[1].y;	// ?????? ?? Y
		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].dLineX[0][2] = pMatching->m_lineVert[nCam][nPos + 0].pt[0].x;	// ?????? ???? x
		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].dLineY[0][2] = pMatching->m_lineVert[nCam][nPos + 0].pt[0].y; // ?????? ???? y
		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].dLineX[0][3] = pMatching->m_lineVert[nCam][nPos + 0].pt[1].x; // ?????? ?? x
		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].dLineY[0][3] = pMatching->m_lineVert[nCam][nPos + 0].pt[1].y; // ?????? ?? y

		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].dLineX[1][0] = pMatching->m_lineHori[nCam][nPos + 1].pt[0].x;	// ?????? ???? X
		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].dLineY[1][0] = pMatching->m_lineHori[nCam][nPos + 1].pt[0].y;	// ?????? ???? Y
		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].dLineX[1][1] = pMatching->m_lineHori[nCam][nPos + 1].pt[1].x;	// ?????? ?? X
		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].dLineY[1][1] = pMatching->m_lineHori[nCam][nPos + 1].pt[1].y;	// ?????? ?? Y
		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].dLineX[1][2] = pMatching->m_lineVert[nCam][nPos + 1].pt[0].x;	// ?????? ???? x
		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].dLineY[1][2] = pMatching->m_lineVert[nCam][nPos + 1].pt[0].y; // ?????? ???? y
		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].dLineX[1][3] = pMatching->m_lineVert[nCam][nPos + 1].pt[1].x; // ?????? ?? x
		m_pMain->m_stSaveImageInfo[m_nJobID][nCam][index].dLineY[1][3] = pMatching->m_lineVert[nCam][nPos + 1].pt[1].y; // ?????? ?? y

		m_pMain->copyMemory(m_pMain->getSaveImageBuffer(real_cam, index), m_pMain->getProcBuffer(real_cam, nPos), w * h);

		m_pMain->m_nSaveImageRingBufIndex[m_nJobID][nCam] = (m_pMain->m_nSaveImageRingBufIndex[m_nJobID][nCam] + 1) % MAX_SAVE_IMAGE_RING_BUF;
	}
}

int CJob_1Cam1ShotAlign::calibration_enable_check(int next_seq)
{
	CString str;
	int read_bit_addr = pJob->plc_addr_info.read_bit_start + 5;
	BOOL bEnable = g_CommPLC.GetBit(read_bit_addr);

	if (bEnable != TRUE)
	{
		str.Format("PLC Calib Enable(L%d) Signal Off!!", read_bit_addr);
		m_pMain->add_process_history_machine(str);
		return 2;
	}
	else
	{
		str.Format("PLC Calib Enable(L%d) Signal On", read_bit_addr);
		m_pMain->add_process_history_machine(str);
	}

	m_calib_seq = next_seq;
	return 0;
}

int CJob_1Cam1ShotAlign::calibration_mode_on(int next_seq)
{
	CString str;
	int write_bit_addr = pJob->plc_addr_info.write_bit_start + 6;
	g_CommPLC.SetBit(write_bit_addr, TRUE);

	m_calib_seq = next_seq;
	m_calib_timeout_cnt = 0;

	str.Format("Calib Mode On(M%d)", write_bit_addr);
	m_pMain->add_process_history_machine(str);

	return 0;
}

int CJob_1Cam1ShotAlign::wait_calibration_mode_on_ack(int next_seq)
{
	CString str;
	int read_bit_addr = pJob->plc_addr_info.read_bit_start + 6;
	BOOL bAck = g_CommPLC.GetBit(read_bit_addr);

	if (m_calib_timeout_cnt % 50 == 0)
	{
		str.Format("Wait PLC ?? PC Mode On(L%d) Ack Signal....", read_bit_addr);
		m_pMain->add_process_history_machine(str);
	}

	if (m_calib_timeout_cnt++ > 1000)
	{
		m_calib_timeout_cnt = 0;
		str.Format("Timeout - Wait PLC ?? PC Mode On Ack Signal");
		m_pMain->add_process_history_machine(str);

		return 2;
	}

	if (bAck)
	{
		m_calib_timeout_cnt = 0;
		m_calib_seq = next_seq;
	}

	return 0;
}

int CJob_1Cam1ShotAlign::memory_calibration_origin(int next_seq)
{
	CString str;
	int nCam = 0;
	int num_of_pos = pJob->num_of_position;

	for (int nPos = 0; nPos < num_of_pos; nPos++)
	{
		int real_cam = pJob->camera_index[nCam];
		CFindInfo* pInfo = &pMatching->getFindInfo(nCam, nPos);
		pMachine->setCalibStartX(nCam, nPos, pInfo->GetXPos());
		pMachine->setCalibStartY(nCam, nPos, pInfo->GetYPos());

		((CFormMachineView*)m_pMain->m_pForm[FORM_MACHINE])->m_dbCalibTargetX[real_cam] = pInfo->GetXPos();
		((CFormMachineView*)m_pMain->m_pForm[FORM_MACHINE])->m_dbCalibTargetY[real_cam] = pInfo->GetYPos();

		str.Format("Pos %d) %.1f%% %s", real_cam + 1, pInfo->getScore(), pInfo->GetFound() == FIND_OK ? "OK" : "NG");
		m_pMain->add_process_history_machine(str);
	}

	::SendMessageA(m_pMain->m_pForm[FORM_MACHINE]->m_hWnd, WM_VIEW_CONTROL, MSG_FMV_DISPLAY_MACHINE_VIEW, 0);

	m_calib_seq = next_seq;
	return 0;
}

int CJob_1Cam1ShotAlign::reset_calibration_data(int next_seq)
{
	int nCam = 0;
	int num_of_pos = pJob->num_of_position;

	// Reset Calib Data
	for (int nPos = 0; nPos < num_of_pos; nPos++)
	{
		m_pMain->GetMachine(m_nJobID).getPtImage(nCam, nPos).clear();
		m_pMain->GetMachine(m_nJobID).getPtMotor(nCam, nPos).clear();
	}

	m_calib_cnt_x = -1;
	m_calib_cnt_y = -1;

	m_calib_seq = next_seq;

	return 0;
}

int CJob_1Cam1ShotAlign::calibration_move_request(int next_seq)
{
	CString str;
	double dbData[3] = { 0, };
	double dbStepX, dbStepY;	// 1.2mm,	0.9mm
	double dbOffsetX = 0.0, dbOffsetY = 0.0;

	int nCam = 0;
	int nLimitX = pModel->getMachineInfo().getCalibrationCountX();
	int nLimitY = pModel->getMachineInfo().getCalibrationCountY();
	int nOriLimitX = pModel->getMachineInfo().getCalibrationCountX();
	int nOriLimitY = pModel->getMachineInfo().getCalibrationCountY();

	int num_of_pos = 2;

	if (nLimitX <= 1)	dbStepX = 0.0;
	else				dbStepX = pModel->getMachineInfo().getCalibrationRangeX() / (nLimitX - 1);

	if (nLimitY <= 1)	dbStepY = 0.0;
	else				dbStepY = pModel->getMachineInfo().getCalibrationRangeY() / (nLimitY - 1);

	if (nLimitX == 0)
	{
		dbStepX = 0;
		nLimitX = 3;
	}
	if (nLimitY == 0)
	{
		dbStepY = 0;
		nLimitY = 3;
	}

	// ???? ?????? ???? 
	if (m_calib_cnt_x == 0)
	{
		if (m_calib_cnt_y == 0)
		{
			dbData[AXIS_X] = -dbStepX * ((nLimitX - 1) / 2.0);
			dbData[AXIS_Y] = -dbStepY * ((nLimitY - 1) / 2.0);
			dbData[AXIS_T] = 0;
		}
		else
		{
			dbData[AXIS_X] = (-dbStepX) * (nLimitX - 1);
			dbData[AXIS_Y] = dbStepY;
			dbData[AXIS_T] = 0;
		}
	}
	else
	{
		dbData[AXIS_X] = dbStepX;
		dbData[AXIS_Y] = 0;
		dbData[AXIS_T] = 0;
	}

	for (int nPos = 0; nPos < num_of_pos; nPos++)
	{
		cv::Point2f pt;
		pt.x = float((m_calib_cnt_x - (nLimitX - 1) / 2.0) * dbStepX + dbOffsetX);
		pt.y = float((m_calib_cnt_y - (nLimitY - 1) / 2.0) * dbStepY + dbOffsetY);
		m_pMain->GetMachine(m_nJobID).addMotorPoint(nCam, nPos, pt);
	}

	if (nLimitX == 0)	dbData[AXIS_X] = 0;
	if (nLimitY == 0)	dbData[AXIS_Y] = 0;

	pResult->revision_data[0][AXIS_X] = dbData[AXIS_X];
	pResult->revision_data[0][AXIS_Y] = dbData[AXIS_Y];
	pResult->revision_data[0][AXIS_T] = dbData[AXIS_T];

	int write_bit_addr = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start + 7;
	int write_word_addr = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_word_start;

	m_pMain->sendRevisionData(dbData[AXIS_X], dbData[AXIS_Y], dbData[AXIS_T], m_nJobID);
	g_CommPLC.SetBit(write_bit_addr, TRUE);

	str.Format("PC ?? PLC Send Calib Data(D%d)", write_word_addr);
	m_pMain->add_process_history_machine(str);

	str.Format("PC ?? PLC Revision Pos Move Signal(M%d) Req", write_bit_addr);
	m_pMain->add_process_history_machine(str);

	str.Format("[%d / %d] X: %.3f, Y: %.3f", m_calib_cnt_x, m_calib_cnt_y, dbData[AXIS_X], dbData[AXIS_Y]);
	m_pMain->add_process_history_machine(str);

	m_calib_timeout_cnt = 0;
	m_calib_seq = next_seq;

	return 0;
}

int CJob_1Cam1ShotAlign::wait_calibration_move_ack(int next_seq)
{
	CString str;
	int write_bit_addr = m_pMain->vt_job_info[m_nJobID].plc_addr_info.write_bit_start + 7;
	int read_bit_addr = m_pMain->vt_job_info[m_nJobID].plc_addr_info.read_bit_start + 7;

	BOOL revision_ack = g_CommPLC.GetBit(read_bit_addr);

	if (m_calib_timeout_cnt % 50 == 0)
	{
		str.Format("Wait PLC ?? PC Revision Ack(L%d) Signal....", read_bit_addr);
		m_pMain->add_process_history_machine(str);
	}

	if (++m_calib_timeout_cnt > 1000)
	{
		m_calib_seq = 0;
		str.Format("PLC ?? Revision Pos Moving(L%d) Time Out", read_bit_addr);
		m_pMain->add_process_history_machine(str);
		g_CommPLC.SetBit(write_bit_addr, FALSE);
		return 2;
	}

	if (revision_ack == TRUE)
	{
		str.Format("PLC ?? PC Revision Pos Move(L%d) Complete", read_bit_addr);
		m_pMain->add_process_history_machine(str);
		m_calib_seq = next_seq;
		g_CommPLC.SetBit(write_bit_addr, FALSE);
	}

	return 0;
}

int CJob_1Cam1ShotAlign::add_calibration_data(int next_seq)
{
	CString str;
	int nCam = 0;
	int num_of_pos = 2;

	for (int nPos = 0; nPos < num_of_pos; nPos++)
	{
		int real_cam = pJob->camera_index[0];
		CFindInfo* pInfo = &m_pMain->GetMatching(m_nJobID).getFindInfo(nCam, nPos);
		cv::Point2f pt;
		pt.x = float(pInfo->GetXPos());
		pt.y = float(pInfo->GetYPos());
		pMachine->addImagePoint(nCam, nPos, pt);

		str.Format("Pos %d: (%d, %d) (%.1f %.1f)", real_cam + 1, m_calib_cnt_x, m_calib_cnt_y, pInfo->GetXPos(), pInfo->GetYPos());
		m_pMain->add_process_history_machine(str);
		((CFormMachineView*)m_pMain->m_pForm[FORM_MACHINE])->m_ptrCalib[real_cam].push_back(pt);
	}

	m_calib_seq = next_seq;	// ????

	return 0;
}

BOOL CJob_1Cam1ShotAlign::calculate_calibration_data()
{
	CString str;
	int nCam = 0;
	int num_of_pos = 2;

	int nLimitX = pModel->getMachineInfo().getCalibrationCountX();
	int nLimitY = pModel->getMachineInfo().getCalibrationCountY();
	int nOriLimitX = pModel->getMachineInfo().getCalibrationCountX();
	int nOriLimitY = pModel->getMachineInfo().getCalibrationCountY();

	int w = pJob->camera_size[nCam].x;
	int h = pJob->camera_size[nCam].y;

	if (nOriLimitX == 1)
	{
		int nDir = m_pMain->m_iniFile.GetProfileIntA("SERVER_INFO", "VIRTUAL_COORDI_DIR", -1);

		for (int nPos = 0; nPos < num_of_pos; nPos++)
		{
			pMachine->MakeVirtualCoordinates(&pMachine->getPtImage(nCam, nPos), &pMachine->getPtMotor(nCam, nPos), nDir, 0);
		}

		nLimitX = nLimitY;
	}
	else if (nOriLimitY == 1)
	{
		int nDir = m_pMain->m_iniFile.GetProfileIntA("SERVER_INFO", "VIRTUAL_COORDI_DIR", -1);

		for (int nPos = 0; nPos < num_of_pos; nPos++)
		{
			pMachine->MakeVirtualCoordinates(&pMachine->getPtImage(nCam, nPos), &pMachine->getPtMotor(nCam, nPos), nDir, 1);
		}

		nLimitY = nLimitX;
	}

	m_pMain->GetMachine(m_nJobID).setNumCalPointsX(nLimitX);
	m_pMain->GetMachine(m_nJobID).setNumCalPointsY(nLimitY);

	for (int nPos = 0; nPos < num_of_pos; nPos++)
	{
		int real_cam = pJob->camera_index[nCam];
		if (pMachine->calculateCalibration(nCam, nPos) == 0)
		{
			double cX, cY;

			int n = pModel->getMachineInfo().getAlignOrginPos(nCam);
			double xr = 0.5;
			double yr = 0.5;
			switch (n)
			{
			case _LEFT_TOP_X25_Y25:  xr = 0.25; yr = 0.25; break;
			case _CENTER_TOP_X50_Y25:  xr = 0.5; yr = 0.25; break;
			case _RIGHT_TOP_X75_Y25:  xr = 0.75; yr = 0.25; break;
			case _LEFT_CENTER_X25_Y50:  xr = 0.25; yr = 0.5; break;
			case _CENTER_CENTER_X50_Y50:  xr = 0.5; yr = 0.5; break;
			case _RIGHT_CENTER_X75_Y50:  xr = 0.75; yr = 0.5; break;
			case _LEFT_BOTTOM_X25_Y75:  xr = 0.25; yr = 0.75; break;
			case _CENTER_BOTTOM_X50_Y75:  xr = 0.5; yr = 0.75; break;
			case _RIGHT_BOTTOM_X75_Y75:  xr = 0.75; yr = 0.75; break;
			case _NOTCH_X85_Y50:		xr = 0.85; yr = 0.5; break;
			}

			pMachine->PixelToWorld(nCam, nPos, w * xr, h * yr, &cX, &cY, TRUE);

			pMachine->setOffsetX(nCam, nPos, cX);
			pMachine->setOffsetY(nCam, nPos, cY);
			pMachine->calculateResolution(nCam, nPos, nLimitX, nLimitY);

			CString str_path;
			str_path.Format("%s%s\\", m_pMain->m_strMachineDir, m_pMain->vt_job_info[m_nJobID].job_name.c_str());
			pMachine->saveCalibrationData(nCam, nPos, str_path);
		}
		else
		{
			str.Format("Cam %d Calculate Calibration Data Failed.", nCam + 1);
			m_pMain->add_process_history_machine(str);

			return FALSE;
		}
	}

	return TRUE;
}

UINT thread_find_pattern_1cam_1shot_align(void* pParam)		// 17.12.30 jmLee		Auto ???????? ?????? ???? ??????..
{
	struct _stFindPattern* pPattern = (struct _stFindPattern*)pParam;
	CLET_AlignClientDlg* pMain = (CLET_AlignClientDlg*)pPattern->pDlg;
	CFormMainView* pFormMain = (CFormMainView*)pMain->m_pForm[FORM_MAIN];


	int nCam = pPattern->nCam;
	int nPos = pPattern->nPos;
	int nJob = pPattern->nJob;
	int nSCount = pPattern->nSearchCount;

	CJobInfo* pJobInfo = &pMain->vt_job_info[nJob];

	int real_cam = pJobInfo->camera_index[nCam];
	int W = pJobInfo->camera_size[nCam].x;
	int H = pJobInfo->camera_size[nCam].y;

	BOOL bFind = TRUE;

	if (pMain->m_bSimulationStart != TRUE)
		pMain->copyMemory(pMain->getProcBuffer(real_cam, nPos), pMain->getSrcBuffer(real_cam), W * H);

	CString strTemp;
	strTemp.Format("[%s] cam: %d, pos: %d, count: %d, size: %d", pMain->vt_job_info[nJob].job_name.c_str(), nCam + 1, nPos, nSCount, W * H);
	::SendMessageA(pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);


	int method = pMain->vt_job_info[nJob].model_info.getAlignInfo().getMarkFindMethod();

	if (method == METHOD_RANSAC)
	{
		pMain->GetMatching(nJob).findPattern(pMain->getProcBuffer(real_cam, nPos), nCam, nPos, W, H);
	}
	else if (method == METHOD_MATCHING)
	{
		pMain->GetMatching(nJob).findPattern(pMain->getProcBuffer(real_cam, nPos), nCam, nPos, W, H);
	}
	else if (method == METHOD_CALIPER)
	{
		g_Job_1Cam1ShotAlign[nJob].find_pattern_caliper(pMain->getProcBuffer(real_cam, nPos), nCam, nPos, W, H);
	}
	else
	{
		pMain->GetMatching(nJob).getFindInfo(nCam, nPos).SetFound(FIND_ERR);

		strTemp.Format("[%s] find pattern method unknown!!", pMain->vt_job_info[nJob].get_job_name());
		::SendMessageA(pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);
	}

	if (pMain->GetMatching(nJob).getFindInfo(nCam, nPos).GetFound() != FIND_OK)
		bFind = FALSE;

	if (bFind == FALSE)
	{
		pMain->vt_result_info[nJob].is_find_pattern[nCam][nPos] = FALSE;

		strTemp.Format("[%s] cam %d pos %d Find NG", pMain->vt_job_info[nJob].get_job_name(), nCam + 1, nPos + 1);
		::SendMessageA(pMain->m_pPane[PANE_AUTO]->m_hWnd, WM_VIEW_CONTROL, MSG_PA_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);
	}
	else
	{
		pMain->vt_result_info[nJob].is_find_pattern[nCam][nPos] = TRUE;
	}

	pMain->vt_result_info[nJob].is_find_pattern_end[nCam][nPos] = TRUE;

	return 0;
}

int CJob_1Cam1ShotAlign::find_pattern_caliper(BYTE* pImage, int nCam, int nPos, int W, int H)
{
	int ret = 0;
	BOOL fixture_mark_use = pModel->getAlignInfo().getUseFixtureMark();
	double dx = 0.0, dy = 0.0, dt = 0.0;
	double posx = 0.0, posy = 0.0;
	int real_cam = pJob->camera_index[nCam];

	if (fixture_mark_use)
	{
		pMatching->findPattern(pImage, nCam, nPos, W, H);

		double fixtureX = pMatching->getFixtureX(nCam, nPos);
		double fixtureY = pMatching->getFixtureY(nCam, nPos);
		double posX = pMatching->getFindInfo(nCam, nPos).GetXPos();
		double posY = pMatching->getFindInfo(nCam, nPos).GetYPos();

		CFindInfo* pFindInfo = &pMatching->getFindInfo(nCam, nPos);
		pFindInfo->SetMatchingXPos(posX);
		pFindInfo->SetMatchingYPos(posY);

		if (fixtureX != 0.0 && fixtureY != 0.0)
		{
			dx = posX - fixtureX;
			dy = posY - fixtureY;
			dt = 0.0;
		}
	}

	BOOL bFindLine[2];
	sLine line_info[2];

	m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][0].processCaliper(pImage, W, H, dx, dy, dt, TRUE);
	m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][1].processCaliper(pImage, W, H, dx, dy, dt, TRUE);

	line_info[0] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][0].m_lineInfo;
	line_info[1] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][1].m_lineInfo;
	bFindLine[0] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][0].getIsMakeLine();
	bFindLine[1] = m_pMain->m_pDlgCaliper->m_Caliper[real_cam][nPos][1].getIsMakeLine();

	if (bFindLine[0] && bFindLine[1])
	{
		sLine lineVert = line_info[0];
		sLine lineHori = line_info[1];

		// 21.02.13 ?????? Matching Rate?? ????.
		double dAngle = m_pMain->calcIncludedAngle(lineHori, lineVert);
		double dRate = m_pMain->make_included_angle_to_matching(dAngle, m_nJobID);

		CCaliper::cramersRules(-lineHori.a, 1, -lineVert.a, 1, lineHori.b, lineVert.b, &posx, &posy);

		CFindInfo pFindInfo = pMatching->getFindInfo(nCam, nPos);
		pFindInfo.SetXPos(posx);
		pFindInfo.SetYPos(posy);
		pFindInfo.SetFound(FIND_OK);
		//KJH 2021-08-14 Caliper Angle Log ????
		pFindInfo.SetCaliperAngle(dAngle);
		pFindInfo.SetScore(dRate);
		pFindInfo.SetLine(0, lineHori);
		pFindInfo.SetLine(1, lineVert);
		pFindInfo.SetFindline(0, TRUE);
		pFindInfo.SetFindline(1, TRUE);
		pMatching->setFindInfo(nCam, nPos, pFindInfo);
	}
	else
	{
		ret = 2;

		CFindInfo pFindInfo = pMatching->getFindInfo(nCam, nPos);
		pFindInfo.SetXPos(0.0);
		pFindInfo.SetYPos(0.0);
		pFindInfo.SetFound(FIND_ERR);
		//KJH 2021-08-14 Caliper Angle Log ????
		pFindInfo.SetCaliperAngle(0.0);
		pFindInfo.SetScore(0.0);
		if (bFindLine[0] == TRUE)
		{
			sLine  lineVert = line_info[0];
			pFindInfo.SetLine(0, lineVert);
			pFindInfo.SetFindline(0, FALSE);
		}
		else if (bFindLine[1] == TRUE)
		{
			sLine lineHori = line_info[1];
			pFindInfo.SetLine(1, lineHori);
			pFindInfo.SetFindline(1, FALSE);
		}
		pMatching->setFindInfo(nCam, nPos, pFindInfo);
	}

	return ret;
}

UINT thread_1cam_1shot_align(void* pParam)
{
	JOB_SEQ_INFO* pInfo = (JOB_SEQ_INFO*)pParam;
	CLET_AlignClientDlg* pMain = (CLET_AlignClientDlg*)pInfo->pDlg;
	int job_id = pInfo->job_id;

	do {
		int nret = g_Job_1Cam1ShotAlign[job_id].do_auto_seq_plc();

		if (nret == 1)		// OK
		{

		}
		else if (nret == 2)	// NG
		{

		}

		//do_auto_seq_plc();


	} while (pMain->m_bProgramEnd != TRUE);


	return 0;
}

int CJob_1Cam1ShotAlign::do_auto_seq_plc()
{
	CString str_temp;
	int nRet = 0;
	int return_seq = 0;
	int next_seq = 0;
	int manual_input_seq = 0;

	switch (m_nAutoSeq) {	// process_1cam_1shot_align()
	case 0:

		m_nAutoSeq = 50;
		break;
	case 50:
	{
		int addr = pJob->plc_addr_info.read_bit_start;

		BOOL is_req = g_CommPLC.GetBit(addr);

		if (is_req)
		{
			is_seq_start = TRUE;
			str_temp.Format("[%s] process start", pJob->get_job_name());
			m_pMain->add_process_history(str_temp);
			m_nAutoSeq = 100;
			break;
		}
	}
	break;
	case 100:
	{
		// ???????? ???? ????.
		next_seq = 200;
		nRet = grab_start(next_seq, TRUE);
	}
	break;
	case 200:
	{
		// ???????? ???? ???????? ???? ????.
		next_seq = 300;
		nRet = grab_complete_check(next_seq, TRUE);
	}
	break;
	case 300:
	{
		// ?????? ????.
		return_seq = 100;
		next_seq = 400;
		manual_input_seq = 500;
		BOOL bFind = find_pattern(next_seq, TRUE);

		// ?? ???? ???? ?????????? retry process
		if (bFind != TRUE)	nRet = grab_retry_process(return_seq, manual_input_seq);
	}
	break;
	case 400:
	{
		// ???????? ???? ????
		nRet = calculate_revision_data();
		if (nRet == 2)	break;

		// ???????? ???? ????
		send_revision_data();

		// L Check ????
		if (pModel->getAlignInfo().getEnableLCheck())
		{
			BOOL ret = m_pMain->calc_lcheck_1cam_2pos(m_nJobID);

			if (ret == TRUE)
			{
				nRet = 2;
				break;
			}
		}

		// ?????? ????
		if (pModel->getAlignInfo().getUseIncludedAngle())
		{
			nRet = included_angle_check_process();
			if (nRet == 2)	break;
		}

		m_pMain->set_seq_process(m_nJobID, 0);
		nRet = 1;
	}
	break;
	case 500:
	{
		// ???? ???? ???? ???? ????
		return_seq = 400;
		nRet = manual_input_mark_process(return_seq);
	}
	break;
	}

	return nRet;
}

void CJob_1Cam1ShotAlign::run_thread(CDialog* pDlg, int job_id)
{
	JOB_SEQ_INFO* info = new JOB_SEQ_INFO;
	info->job_id = job_id;
	info->pDlg = pDlg;
	AfxBeginThread(thread_1cam_1shot_align, info);
}
