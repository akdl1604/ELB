#include "StdAfx.h"
#include "Prealign.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"

typedef struct _PointD {
	double x;
	double y;
	_PointD() :x(0), y(0) 
	{
	}
}PointD;

// (p1, p2)를 이은 직선과 (p3, p4)를 이은 직선의 교차점을 구하는 함수
// Function to get intersection point with line connecting points (p1, p2) and another line (p3, p4).
PointD IntersectionPoint1(const PointD* p1, const PointD* p2, const PointD* p3, const PointD* p4)
{
	PointD ret;

	if (((p1->x - p2->x)*(p3->y - p4->y) - (p1->y - p2->y)*(p3->x - p4->x)) != 0)
	{
		ret.x = ((p1->x*p2->y - p1->y*p2->x)*(p3->x - p4->x) - (p1->x - p2->x)*(p3->x*p4->y - p3->y*p4->x)) / ((p1->x - p2->x)*(p3->y - p4->y) - (p1->y - p2->y)*(p3->x - p4->x));
		ret.y = ((p1->x*p2->y - p1->y*p2->x)*(p3->y - p4->y) - (p1->y - p2->y)*(p3->x*p4->y - p3->y*p4->x)) / ((p1->x - p2->x)*(p3->y - p4->y) - (p1->y - p2->y)*(p3->x - p4->x));
	}
	return ret;
}

// y = m1*x + b1, y = m2*x + b2 두 직선의 교차점을 구하는 함수
// Function to get intersection point of two lines y = m1*x + b1, y = m2*x + b2
PointD IntersectionPoint2(double m1, double b1, double m2, double b2) 
{
	PointD ret;
	if ((m1 - m2) != 0)
	{
		ret.x = (b2 - b1) / (m1 - m2);
		ret.y = m1 * (b2 - b1) / (m1 - m2) + b1;
	}

	return ret;
}

// a1*x + b1*y + c1 = 0, a2*x + b2*y + c2 = 0 두 직선의 교차점을 구하는 함수
// Function to get intersection point of two lines a1*x + b1*y + c1 = 0, a2*x + b2*y + c2 = 0
PointD IntersectionPoint3(double a1, double b1, double c1, double a2, double b2, double c2) 
{
	PointD ret;
	if (b1 != 0 && (a1*b2 - a2 * b1) != 0)
	{
		ret.x = (b1*c2 - b2 * c1) / (a1*b2 - a2 * b1);
		ret.y = -a1 / b1 * (b1*c2 - b2 * c1) / (a1*b2 - a2 * b1) - c1 / b1;
	}

	return ret;
}

CPrealign::CPrealign(void)
{
	m_nStageType = 0;
	m_bReverseAxisX = FALSE;
	m_bReverseAxisY = FALSE;
	m_bReverseAxisT = FALSE;
	m_dbPatternPitchX = 0.0;
	m_dbPatternPitchY = 0.0;
	m_nAlignmentType = 0;
	m_nCameraPosType = 0;

	m_nRevisionDirX = 1;
	m_nRevisionDirY = 1;

	for (int i = 0; i < MAX_CAMERA; i++)
	{
		m_dbPosX[i] = 0.0;
		m_dbPosY[i] = 0.0;
		m_dbRotateX[i] = 0.0;
		m_dbRotateY[i] = 0.0;

		mark_offset_x[i] = 0.0;
		mark_offset_y[i] = 0.0;
	}
	
	for (int i = 0; i < NUM_AXIS; i++)
	{
		m_dbRevisionData[i] = 0.0;
		m_dRevisionOffset[i] = 0.0;
	}

	m_nMotorType = 0;

	for (int i = 0; i < 4; i++)
	{
		m_dbCurrRevisionX[i] = m_dbPrevRevisionX[i] = 0.0;
		m_dbCurrRevisionY[i] = m_dbPrevRevisionY[i] = 0.0;
		m_dbCurrRevisionT[i] = m_dbPrevRevisionT[i] = 0.0;
		m_dbCurrPosX[i] = m_dbPrevPosX[i] = 0.0;
		m_dbCurrPosY[i] = m_dbPrevPosY[i] = 0.0;
	}

	m_bRevisionRotateXY = FALSE;

	use_image_theta = FALSE;
	image_theta = 0.0;
}

CPrealign::~CPrealign(void)
{

}

void CPrealign::calcRotate(double rx, double ry, double x1, double y1, double rangle, double *x2, double *y2)
{
	double tmpx, tmpy;
	tmpx = x1 - rx;
	tmpy = y1 - ry;

	*x2 = (tmpx * cos(rangle) - tmpy * sin(rangle) + rx);
	*y2 = (tmpx * sin(rangle) + tmpy * cos(rangle) + ry);

	// transformPixelToWorld
	// transformToWorld
}

int CPrealign::calcRevision(int nCamL, int nCamR, int nCamPosType, BOOL bRevisionReverse, BOOL bRotateOffsetPreCalc)
{
	double posX[2] = { 0, }, posY[2] = { 0, };
	double rotateX[2] = { 0, }, rotateY[2] = { 0, };
	double dbAlignX[2] = { 0, }, dbAlignY[2] = { 0, }, dbAlignTH = 0.0, dbAlignTH2 = 0.0;
	double dbDist, dbDiagonal, dbDistX = 1;

	// Pattern 로봇 좌표계
	posX[0] = getPosX(nCamL);	posY[0] = getPosY(nCamL);
	posX[1] = getPosX(nCamR);	posY[1] = getPosY(nCamR);

	// 카메라별 회전 중심
	rotateX[0] = getRotateX(nCamL);
	rotateY[0] = getRotateY(nCamL);
	rotateX[1] = getRotateX(nCamR);
	rotateY[1] = getRotateY(nCamR);

	// 카메라 관계에 따라 Pitch 및 Diff 값 설정
	if (nCamPosType == CAM_HORI)
	{
		dbDist = posY[0] - posY[1];
		dbDiagonal = m_dbPatternPitchX;// m_dbPatternPitchX;
	}
	else
	{
		dbDist = posX[0] - posX[1];
		dbDiagonal = m_dbPatternPitchY;
	}

	if (m_bReverseAxisT)
		dbDist = -dbDist;

	// Theta 계산
	if (dbDiagonal == 0) return -1;

	dbAlignTH = asin(dbDist / dbDiagonal) / CV_PI * 180.0;

	if (get_use_image_theta())
		dbAlignTH = image_theta / CV_PI * 180.0;

	if (bRotateOffsetPreCalc == FALSE)	
		dbAlignTH += m_dRevisionOffset[2];

	for (int i = 0; i < 2; i++)
	{
		m_dbPrevRevisionX[i] = m_dbCurrRevisionX[i];
		m_dbPrevRevisionY[i] = m_dbCurrRevisionY[i];
		m_dbPrevRevisionT[i] = m_dbCurrRevisionT[i];
		m_dbPrevPosX[i] = m_dbCurrPosX[i];
		m_dbPrevPosY[i] = m_dbCurrPosY[i];
	}

	// 카메라별 X, Y 보정값 계산
	try {
		calcRotate(rotateX[0], rotateY[0], posX[0], posY[0], dbAlignTH * CV_PI / 180.0, &dbAlignX[0], &dbAlignY[0]);
		calcRotate(rotateX[1], rotateY[1], posX[1], posY[1], dbAlignTH * CV_PI / 180.0, &dbAlignX[1], &dbAlignY[1]);
	}
	catch (...)
	{
		return -1;
	}

	// 글라스를 받는 쪽이 보정하는 경우 전체 보정량을 반전할 필요가 있다.
	if (bRevisionReverse)
	{
		dbAlignX[0] = -dbAlignX[0];
		dbAlignX[1] = -dbAlignX[1];
		dbAlignY[0] = -dbAlignY[0];
		dbAlignY[1] = -dbAlignY[1];
		dbAlignTH = -dbAlignTH;
	}

	m_dbCurrRevisionX[0] = -dbAlignX[0];
	m_dbCurrRevisionY[0] = -dbAlignY[0];
	m_dbCurrRevisionT[0] = -dbAlignTH;
	m_dbCurrPosX[0] = posX[0];
	m_dbCurrPosY[0] = posY[0];

	m_dbCurrRevisionX[1] = -dbAlignX[1];
	m_dbCurrRevisionY[1] = -dbAlignY[1];
	m_dbCurrRevisionT[1] = -dbAlignTH;
	m_dbCurrPosX[1] = posX[1];
	m_dbCurrPosY[1] = posY[1];
	
	if (m_bReverseAxisX)
	{
		dbAlignX[0] *= -1;
		dbAlignX[1] *= -1;
	}
	
	if (m_bReverseAxisY)
	{
		dbAlignY[0] *= -1;
		dbAlignY[1] *= -1;
	}

	dbAlignX[0] += m_dRevisionOffset[AXIS_X];
	dbAlignX[1] += m_dRevisionOffset[AXIS_X];

	dbAlignY[0] += m_dRevisionOffset[AXIS_Y];
	dbAlignY[1] += m_dRevisionOffset[AXIS_Y];

	// y값을 그대로 x로
	// x값을 -y로
	if (m_bReverseAxisXY)
	{
		double tempX = dbAlignX[0];
		double tempY = dbAlignY[0];

		dbAlignX[0] = dbAlignY[0];
		dbAlignY[0] = -tempX;

		tempX = dbAlignX[1];
		tempY = dbAlignY[1];
		dbAlignX[1] = dbAlignY[1];
		dbAlignY[1] = -tempX;
	}

	m_dbLCheckX[0] = dbAlignX[0];
	m_dbLCheckX[1] = dbAlignX[1];
	m_dbLCheckY[0] = dbAlignY[0];
	m_dbLCheckY[1] = dbAlignY[1];
	m_dbLCheckX[2] = (dbAlignX[0] + dbAlignX[1]) / 2.0;
	m_dbLCheckY[2] = (dbAlignY[0] + dbAlignY[1]) / 2.0;

	switch (getAlignmentType()) {
	case ALIGNMENT_LEFT:
		setRevisionData(dbAlignX[0], dbAlignY[0], dbAlignTH);
		break;
	case ALIGNMENT_RIGHT:
		setRevisionData(dbAlignX[1], dbAlignY[1], dbAlignTH);
		break;
	case ALIGNMENT_CENTER:
		setRevisionData((dbAlignX[0] + dbAlignX[1]) / 2.0, (dbAlignY[0] + dbAlignY[1]) / 2.0, dbAlignTH);
		break;
	default:
		setRevisionData(dbAlignX[0], dbAlignY[0], dbAlignTH);
		break;
	}

	return 0;
}

int CPrealign::calcRevision(int nCamL, int nCamR, double theta, BOOL bRevisionReverse, BOOL bRotateOffsetPreCalc)	// 보정량 계산 함수
{
	double posX[2] = { 0, }, posY[2] = { 0, };
	double rotateX[2] = { 0, }, rotateY[2] = { 0, };
	double dbAlignX[2] = { 0, }, dbAlignY[2] = { 0, }, dbAlignTH = 0.0, dbAlignTH2 = 0.0;
	double dbDistX = 1;

	// Pattern 로봇 좌표계
	posX[0] = getPosX(nCamL);	posY[0] = getPosY(nCamL);
	posX[1] = getPosX(nCamR);	posY[1] = getPosY(nCamR);

	// 카메라별 회전 중심
	rotateX[0] = getRotateX(nCamL);
	rotateY[0] = getRotateY(nCamL);
	rotateX[1] = getRotateX(nCamR);
	rotateY[1] = getRotateY(nCamR);

	dbAlignTH = theta;
	if (bRotateOffsetPreCalc == FALSE)	dbAlignTH += m_dRevisionOffset[2];

	for (int i = 0; i < 2; i++)
	{
		m_dbPrevRevisionX[i] = m_dbCurrRevisionX[i];
		m_dbPrevRevisionY[i] = m_dbCurrRevisionY[i];
		m_dbPrevRevisionT[i] = m_dbCurrRevisionT[i];
		m_dbPrevPosX[i] = m_dbCurrPosX[i];
		m_dbPrevPosY[i] = m_dbCurrPosY[i];
	}

	// 카메라별 X, Y 보정값 계산
	try {
		calcRotate(rotateX[0], rotateY[0], posX[0], posY[0], dbAlignTH * CV_PI / 180.0, &dbAlignX[0], &dbAlignY[0]);
		calcRotate(rotateX[1], rotateY[1], posX[1], posY[1], dbAlignTH * CV_PI / 180.0, &dbAlignX[1], &dbAlignY[1]);
	}
	catch (...)
	{
		return -1;
	}

	// 글라스를 받는 쪽이 보정하는 경우 전체 보정량을 반전할 필요가 있다.
	if (bRevisionReverse)
	{
		dbAlignX[0] = -dbAlignX[0];
		dbAlignX[1] = -dbAlignX[1];
		dbAlignY[0] = -dbAlignY[0];
		dbAlignY[1] = -dbAlignY[1];
		dbAlignTH = -dbAlignTH;
	}

	m_dbCurrRevisionX[0] = -dbAlignX[0];
	m_dbCurrRevisionY[0] = -dbAlignY[0];
	m_dbCurrRevisionT[0] = -dbAlignTH;
	m_dbCurrPosX[0] = posX[0];
	m_dbCurrPosY[0] = posY[0];

	m_dbCurrRevisionX[1] = -dbAlignX[1];
	m_dbCurrRevisionY[1] = -dbAlignY[1];
	m_dbCurrRevisionT[1] = -dbAlignTH;
	m_dbCurrPosX[1] = posX[1];
	m_dbCurrPosY[1] = posY[1];

	if (m_bReverseAxisX)
	{
		dbAlignX[0] *= -1;
		dbAlignX[1] *= -1;
	}

	if (m_bReverseAxisY)
	{
		dbAlignY[0] *= -1;
		dbAlignY[1] *= -1;
	}

	dbAlignX[0] += m_dRevisionOffset[AXIS_X];
	dbAlignX[1] += m_dRevisionOffset[AXIS_X];

	dbAlignY[0] += m_dRevisionOffset[AXIS_Y];
	dbAlignY[1] += m_dRevisionOffset[AXIS_Y];

	// y값을 그대로 x로
	// x값을 -y로
	if (m_bReverseAxisXY)
	{
		double tempX = dbAlignX[0];
		double tempY = dbAlignY[0];

		dbAlignX[0] = dbAlignY[0];
		dbAlignY[0] = -tempX;

		tempX = dbAlignX[1];
		tempY = dbAlignY[1];
		dbAlignX[1] = dbAlignY[1];
		dbAlignY[1] = -tempX;
	}

	m_dbLCheckX[0] = dbAlignX[0];
	m_dbLCheckX[1] = dbAlignX[1];
	m_dbLCheckY[0] = dbAlignY[0];
	m_dbLCheckY[1] = dbAlignY[1];
	m_dbLCheckX[2] = (dbAlignX[0] + dbAlignX[1]) / 2.0;
	m_dbLCheckY[2] = (dbAlignY[0] + dbAlignY[1]) / 2.0;

	switch (getAlignmentType()) {
	case ALIGNMENT_LEFT:
		setRevisionData(dbAlignX[0], dbAlignY[0], dbAlignTH);
		break;
	case ALIGNMENT_RIGHT:
		setRevisionData(dbAlignX[1], dbAlignY[1], dbAlignTH);
		break;
	case ALIGNMENT_CENTER:
		setRevisionData((dbAlignX[0] + dbAlignX[1]) / 2.0, (dbAlignY[0] + dbAlignY[1]) / 2.0, dbAlignTH);
		break;
	default:
		setRevisionData(dbAlignX[0], dbAlignY[0], dbAlignTH);
		break;
	}

	return 0;
}

int CPrealign::calcRevisionRobot(int nCamL, int nCamR, int nCamPosType, BOOL bRevisionReverse, BOOL bRotateOffsetPreCalc)
{
	// Lincoln Lee - 2022-07-12 Conveyer Align 구조 개발

	double posX[4] = { 0, }, posY[4] = { 0, };
	double rotateX[2] = { 0, }, rotateY[2] = { 0, };
	double dbAlignX[2] = { 0, }, dbAlignY[2] = { 0, }, dbAlignTH = 0.0, dbAlignTH2 = 0.0;
	double dbDist, dbDiagonal, dbDistX = 1;

	// Pattern 로봇 좌표계
	posX[0] = getPosX(nCamL);		posY[0] = getPosY(nCamL);
	posX[1] = getPosX(nCamR);		posY[1] = getPosY(nCamR);

	posX[2] = getPosX(nCamL + 2);	posY[2] = getPosY(nCamL + 2);
	posX[3] = getPosX(nCamR + 2);	posY[3] = getPosY(nCamR + 2);

	// 카메라별 회전 중심
	rotateX[0] = getRotateX(nCamL);
	rotateY[0] = getRotateY(nCamL);
	rotateX[1] = getRotateX(nCamR);
	rotateY[1] = getRotateY(nCamR);

	// 카메라 관계에 따라 Pitch 및 Diff 값 설정
	if (nCamPosType == CAM_HORI)
	{
		dbDist = (posY[0]) - (posY[1]);
		dbDiagonal = m_dbPatternPitchX;// m_dbPatternPitchX;
	}
	else
	{
		dbDist = (posX[0]) - (posX[1]);
		dbDiagonal = m_dbPatternPitchY;
	}

	if (m_bReverseAxisT)
		dbDist = -dbDist;

	// Theta 계산
	if (dbDiagonal == 0) return -1;

	dbAlignTH = asin(dbDist / dbDiagonal) / CV_PI * 180.0;

	if (get_use_image_theta())
		dbAlignTH = image_theta / CV_PI * 180.0;

	if (bRotateOffsetPreCalc == FALSE)
		dbAlignTH += m_dRevisionOffset[2];

	for (int i = 0; i < 2; i++)
	{
		m_dbPrevRevisionX[i] = m_dbCurrRevisionX[i];
		m_dbPrevRevisionY[i] = m_dbCurrRevisionY[i];
		m_dbPrevRevisionT[i] = m_dbCurrRevisionT[i];
		m_dbPrevPosX[i] = m_dbCurrPosX[i];
		m_dbPrevPosY[i] = m_dbCurrPosY[i];
	}

	// 카메라별 X, Y 보정값 계산
	try {
		// Lincoln Lee - Alignment
		//+ - (not sure)
		// others i think it is correct
		// - is 90% correct
		calcRotate(0, 0, rotateX[0], rotateY[0], dbAlignTH * CV_PI / 180.0, &dbAlignX[0], &dbAlignY[0]);
		calcRotate(0, 0, rotateX[1], rotateY[1], dbAlignTH * CV_PI / 180.0, &dbAlignX[1], &dbAlignY[1]);

		for (int i = 0; i < 2; i++)
		{
			dbAlignX[i] -= rotateX[i];
			dbAlignY[i] -= rotateY[i];

			dbAlignX[i] += posX[i];
			dbAlignY[i] += posY[i];
		}

		/*dbAlignX[0] -= rotateX[0];
		dbAlignY[0] -= rotateY[0];

		dbAlignX[1] -= rotateX[1];
		dbAlignY[1] -= rotateY[1];

		dbAlignX[0] += posX[0];
		dbAlignY[0] += posY[0];

		dbAlignX[1] += posX[1];
		dbAlignY[1] += posY[1];*/
	}
	catch (...)
	{
		return -1;
	}

	// 글라스를 받는 쪽이 보정하는 경우 전체 보정량을 반전할 필요가 있다.
	if (bRevisionReverse)
	{
		dbAlignX[0] = -dbAlignX[0];
		dbAlignX[1] = -dbAlignX[1];
		dbAlignY[0] = -dbAlignY[0];
		dbAlignY[1] = -dbAlignY[1];
		dbAlignTH = -dbAlignTH;
	}

	m_dbCurrRevisionX[0] = -dbAlignX[0];
	m_dbCurrRevisionY[0] = -dbAlignY[0];
	m_dbCurrRevisionT[0] = -dbAlignTH;
	m_dbCurrPosX[0] = posX[0];
	m_dbCurrPosY[0] = posY[0];

	m_dbCurrRevisionX[1] = -dbAlignX[1];
	m_dbCurrRevisionY[1] = -dbAlignY[1];
	m_dbCurrRevisionT[1] = -dbAlignTH;
	m_dbCurrPosX[1] = posX[1];
	m_dbCurrPosY[1] = posY[1];

	if (m_bReverseAxisX)
	{
		dbAlignX[0] *= -1;
		dbAlignX[1] *= -1;
	}

	if (m_bReverseAxisY)
	{
		dbAlignY[0] *= -1;
		dbAlignY[1] *= -1;
	}

	dbAlignX[0] += m_dRevisionOffset[AXIS_X];
	dbAlignX[1] += m_dRevisionOffset[AXIS_X];

	dbAlignY[0] += m_dRevisionOffset[AXIS_Y];
	dbAlignY[1] += m_dRevisionOffset[AXIS_Y];

	// y값을 그대로 x로
	// x값을 -y로
	if (m_bReverseAxisXY)
	{
		double tempX = dbAlignX[0];
		double tempY = dbAlignY[0];

		dbAlignX[0] = dbAlignY[0];
		dbAlignY[0] = -tempX;

		tempX = dbAlignX[1];
		tempY = dbAlignY[1];
		dbAlignX[1] = dbAlignY[1];
		dbAlignY[1] = -tempX;
	}

	m_dbLCheckX[0] = dbAlignX[0];
	m_dbLCheckX[1] = dbAlignX[1];
	m_dbLCheckY[0] = dbAlignY[0];
	m_dbLCheckY[1] = dbAlignY[1];
	m_dbLCheckX[2] = (dbAlignX[0] + dbAlignX[1]) / 2.0;
	m_dbLCheckY[2] = (dbAlignY[0] + dbAlignY[1]) / 2.0;

	switch (getAlignmentType()) {
	case ALIGNMENT_LEFT:
		setRevisionData(dbAlignX[0], dbAlignY[0], dbAlignTH);
		break;
	case ALIGNMENT_RIGHT:
		setRevisionData(dbAlignX[1], dbAlignY[1], dbAlignTH);
		break;
	case ALIGNMENT_CENTER:
		setRevisionData((dbAlignX[0] + dbAlignX[1]) / 2.0, (dbAlignY[0] + dbAlignY[1]) / 2.0, dbAlignTH);
		break;
	default:
		setRevisionData(dbAlignX[0], dbAlignY[0], dbAlignTH);
		break;
	}

	return 0;
}


int CPrealign::calcRevision4Cam_Center(int nCamLT, int nCamRT, int nCamLB, int nCamRB, int nCamPosType, BOOL bRevisionReverse, BOOL bRotateOffsetPreCalc)
{
	double posX[4] = { 0, }, posY[4] = { 0, };
	double rotateX[4] = { 0, }, rotateY[4] = { 0, };
	double dbAlignX[4] = { 0, }, dbAlignY[4] = { 0, }, dbAlignTH[4] = { 0.0, };
	double dbAlignAvgX = 0.0, dbAlignAvgY = 0.0, dbAlignAvgTH = 0.0;
	double  dbDistX = 1;

	PointD fPoint[4], dbResultPoint;

	double absPosX[4] = { 0, };
	double absPosY[4] = { 0, };


	const int LT = 0;
	const int RT = 1;
	const int LB = 2;
	const int RB = 3;

	const int TOP = 0;
	const int LEFT = 1;
	const int RIGHT = 2;
	const int BOTTOM = 3;

	double angle[4] = { 0, };

	// Pattern 로봇 좌표계
	posX[LT] = getPosX(nCamLT);	posY[LT] = getPosY(nCamLT);
	posX[RT] = getPosX(nCamRT);	posY[RT] = getPosY(nCamRT);
	posX[LB] = getPosX(nCamLB);	posY[LB] = getPosY(nCamLB);
	posX[RB] = getPosX(nCamRB);	posY[RB] = getPosY(nCamRB);

	// 카메라별 회전 중심
	rotateX[LT] = getRotateX(nCamLT);
	rotateY[LT] = getRotateY(nCamLT);
	rotateX[RT] = getRotateX(nCamRT);
	rotateY[RT] = getRotateY(nCamRT);
	rotateX[LB] = getRotateX(nCamLB);
	rotateY[LB] = getRotateY(nCamLB);
	rotateX[RB] = getRotateX(nCamRB);
	rotateY[RB] = getRotateY(nCamRB);


	absPosX[LT] = rotateX[LT] + posX[LT];
	absPosY[LT] = rotateY[LT] + posY[LT];

	absPosX[RT] = rotateX[RT] + posX[RT];
	absPosY[RT] = rotateY[RT] + posY[RT];

	absPosX[LB] = rotateX[LB] + posX[LB];
	absPosY[LB] = rotateY[LB] + posY[LB];

	absPosX[RB] = rotateX[RB] + posX[RB];
	absPosY[RB] = rotateY[RB] + posY[RB];

	double rotate_dx[4] = { 0, };
	double rotate_dy[4] = { 0, };
	double abs_dx[4] = { 0, };
	double abs_dy[4] = { 0, };

	rotate_dx[TOP] = rotateX[RT] - rotateX[LT];
	rotate_dx[LEFT] = rotateX[LT] - rotateX[LB];
	rotate_dx[RIGHT] = rotateX[RT] - rotateX[RB];
	rotate_dx[BOTTOM] = rotateX[RB] - rotateX[LB];

	rotate_dy[TOP] = rotateY[RT] - rotateY[LT];
	rotate_dy[LEFT] = rotateY[LT] - rotateY[LB];
	rotate_dy[RIGHT] = rotateY[RT] - rotateY[RB];
	rotate_dy[BOTTOM] = rotateY[RB] - rotateY[LB];


	double diagonal_x = m_dbPatternPitchX;// 1438.0;
	double diagonal_y = m_dbPatternPitchY;// 813.0;

	double avg_rad_t = 0.0;

	switch (getAlignmentType()) {
	case ALIGNMENT_2P_TOP:
	case ALIGNMENT_4P_TOP:
		avg_rad_t = asin((posY[RT] - posY[LT]) / diagonal_x);	break;
	case ALIGNMENT_2P_LEFT:
	case ALIGNMENT_4P_LEFT:
		avg_rad_t = asin((posX[LB] - posX[LT]) / diagonal_y);	break;
	case ALIGNMENT_2P_RIGHT:
	case ALIGNMENT_4P_RIGHT:
		avg_rad_t = asin((posX[RB] - posX[RT]) / diagonal_y);	break;
	case ALIGNMENT_2P_BOTTOM:
	case ALIGNMENT_4P_BOTTOM:
		avg_rad_t = asin((posY[RT] - posY[LT]) / diagonal_x);	break;
	case ALIGNMENT_4P_CENTER:
	default:
	{
		angle[TOP] = asin((posY[RT] - posY[LT]) / diagonal_x);
		angle[LEFT] = asin((posX[LB] - posX[LT]) / diagonal_y);
		angle[RIGHT] = asin((posX[RB] - posX[RT]) / diagonal_y);
		angle[BOTTOM] = asin((posY[RB] - posY[LB]) / diagonal_x);
		avg_rad_t = (angle[TOP] + angle[LEFT] + angle[RIGHT] + angle[BOTTOM]) / 4.0;
	}
	break;
	}

	if (m_bReverseAxisT)
	{
		avg_rad_t = -avg_rad_t;
	}

	avg_rad_t += (m_dRevisionOffset[AXIS_T] / 180.0 * CV_PI);

	double revX[4] = { 0. };
	double revY[4] = { 0. };
	double total_rev_x = 0.0;
	double total_rev_y = 0.0;
	double avg_rev_x = 0.0;
	double avg_rev_y = 0.0;
	double avg_rev_t = 0.0;
	double denominator = 4.0;

	for (int i = 0; i < 4; i++)
	{
		calcRotate(rotateX[i], rotateY[i], posX[i], posY[i], avg_rad_t, &revX[i], &revY[i]);
	}

	switch (getAlignmentType()) {
	case ALIGNMENT_2P_TOP:
		{
			total_rev_x += revX[LT];
			total_rev_y += revY[LT];
			total_rev_x += revX[RT];
			total_rev_y += revY[RT];
			denominator = 2.0;
	}
		break;
	case ALIGNMENT_2P_LEFT:
		{
			total_rev_x += revX[LT];
			total_rev_y += revY[LT];
			total_rev_x += revX[LB];
			total_rev_y += revY[LB];
			denominator = 2.0;
	}
		break;
	case ALIGNMENT_2P_RIGHT:
		{
			total_rev_x += revX[RT];
			total_rev_y += revY[RT];
			total_rev_x += revX[RB];
			total_rev_y += revY[RB];
			denominator = 2.0;
	}
		break;
	case ALIGNMENT_2P_BOTTOM:
		{
			total_rev_x += revX[LB];
			total_rev_y += revY[LB];
			total_rev_x += revX[RB];
			total_rev_y += revY[RB];
			denominator = 2.0;
	}
		break;
	default:

		for (int i = 0; i < 4; i++)
		{
			total_rev_x += revX[i];
			total_rev_y += revY[i];
		}
		denominator = 4.0;
		break;
	}	
	
	avg_rev_x = total_rev_x / denominator;
	avg_rev_y = total_rev_y / denominator;

	/*
	avg_rev_x = -avg_rev_x;
	avg_rev_y = -avg_rev_y;
	*/

	// 글라스를 받는 쪽이 보정하는 경우 전체 보정량을 반전할 필요가 있다.
	if (bRevisionReverse)
	{
		avg_rev_x = -avg_rev_x;
		avg_rev_y = -avg_rev_y;
	}


	avg_rev_x += m_dRevisionOffset[AXIS_X];
	avg_rev_y += m_dRevisionOffset[AXIS_Y];


	if (m_bReverseAxisX) avg_rev_x = -avg_rev_x;
	if (m_bReverseAxisY) avg_rev_y = -avg_rev_y;

	// x값을 -y로	
	if (m_bReverseAxisXY)
	{
		double tempX = avg_rev_x;
		double tempY = avg_rev_y;
		avg_rev_x = avg_rev_y;
		avg_rev_y = -tempX;
	}

	avg_rev_t = avg_rad_t / 3.141592 * 180.0;
	setRevisionData(avg_rev_x, avg_rev_y, avg_rev_t);

	return 0;
}

void CPrealign::calcRotateOffset(double Theta, double &xMove, double &yMove)
{
	double posX[2] = { 0, }, posY[2] = { 0, };
	double rotateX[2] = { 0, }, rotateY[2] = { 0, };
	double dbAlignX[2] = { 0, }, dbAlignY[2] = { 0, }, dbAlignTH = 0.0, dbAlignTH2 = 0.0;

	// 카메라별 회전 중심
	rotateX[0] = getRotateX(0);	rotateY[0] = getRotateY(0);
	rotateX[1] = getRotateX(1);	rotateY[1] = getRotateY(1);

	// 카메라별 X, Y 보정값 계산
	try {
		calcRotate(rotateX[0], rotateY[0], posX[0], posY[0], Theta * CV_PI / 180.0, &dbAlignX[0], &dbAlignY[0]);
		calcRotate(rotateX[1], rotateY[1], posX[1], posY[1], Theta * CV_PI / 180.0, &dbAlignX[1], &dbAlignY[1]);
	}
	catch (...)
	{
		return;
	}

	dbAlignX[0] *= -1;		dbAlignX[1] *= -1;
	dbAlignY[0] *= -1;		dbAlignY[1] *= -1;

	// m_bReverseAxisX, Y가 고정 되어 있어, 기존 코드 영향 때문에 변수 추가.
	dbAlignX[0] *= m_nRevisionDirX;	dbAlignX[1] *= m_nRevisionDirX;
	dbAlignY[0] *= m_nRevisionDirY;	dbAlignY[1] *= m_nRevisionDirY;

	if (m_bReverseAxisT)	dbAlignTH *= -1;

	switch (getAlignmentType()) {
	case ALIGNMENT_LEFT:
		xMove = dbAlignX[0]; yMove = dbAlignY[0];
		break;
	case ALIGNMENT_RIGHT:
		xMove = dbAlignX[1]; yMove = dbAlignY[1];
		break;
	case ALIGNMENT_CENTER:
		xMove = (dbAlignX[0] + dbAlignX[1]) / 2.0; yMove = (dbAlignY[0] + dbAlignY[1]) / 2.0;
		break;
	}
}

// retT_M1~3    : 1~3번 모터 각도(degree)
// nAxisDir1~3  : 1~3번 모터 방향이 X인지 Y인지  (X == 0)
// dR           : UWV 반지름(mm)
// initT        : 현재 Theta(degree)
// dT           : 이동할 Theta(degree)
// delta1~3     : dT만큼 움직이기 위한 각 축별 이동량
void CPrealign::calcUVW(double refT_M1, double refT_M2, double refT_M3, int nAxisDir1, int nAxisDir2, int nAxisDir3, double dR, double initT, double dT, double *delta1, double *delta2, double *delta3)
{
	if (nAxisDir1 == UVW_X)
		*delta1 = dR * cos((refT_M1 + dT + initT) * CV_PI / 180.0) - dR * cos((refT_M1 + initT) * CV_PI / 180.0);
	else
		*delta1 = dR * sin((refT_M1 + dT + initT) * CV_PI / 180.0) - dR * sin((refT_M1 + initT) * CV_PI / 180.0);

	if (nAxisDir2 == UVW_X)
		*delta2 = dR * cos((refT_M2 + dT + initT) * CV_PI / 180.0) - dR * cos((refT_M2 + initT) * CV_PI / 180.0);
	else
		*delta2 = dR * sin((refT_M2 + dT + initT) * CV_PI / 180.0) - dR * sin((refT_M2 + initT) * CV_PI / 180.0);

	if (nAxisDir3 == UVW_X)
		*delta3 = dR * cos((refT_M3 + dT + initT) * CV_PI / 180.0) - dR * cos((refT_M3 + initT) * CV_PI / 180.0);
	else
		*delta3 = dR * sin((refT_M3 + dT + initT) * CV_PI / 180.0) - dR * sin((refT_M3 + initT) * 3.141592 / 180.0);
}
