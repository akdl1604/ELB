#pragma once

#ifndef __PREALIGN_H__
#define __PREALIGN_H__

#ifndef NUM_AXIS
#define NUM_AXIS 3
#endif

#ifndef PI
#define PI 3.141592
#endif

#ifndef __AXIS_INFO__
#define __AXIS_INFO__
enum _AXIS_INFO {
	AXIS_X = 0,
	AXIS_Y,
	AXIS_T,
	AXIS_Z,
};
#endif

enum _CAM_INFO {
	CAM1 = 0,
	CAM2,
	CAM3,
	CAM4,
	CAM5,
	CAM6,
	CAM7,
	CAM8,
};

enum _CAM_POS_TYPE {
	CAM_HORI = 0,
	CAM_VERT,
};

enum _ALIGNMENT_TYPE {
	ALIGNMENT_LEFT = 0,
	ALIGNMENT_RIGHT,
	ALIGNMENT_CENTER,
	ALIGNMENT_4P_CENTER,
	ALIGNMENT_4P_TOP,
	ALIGNMENT_4P_LEFT,
	ALIGNMENT_4P_RIGHT,
	ALIGNMENT_4P_BOTTOM,
	ALIGNMENT_2P_TOP,
	ALIGNMENT_2P_LEFT,
	ALIGNMENT_2P_RIGHT,
	ALIGNMENT_2P_BOTTOM,
};

enum _MOTOR_TYPE {
	MOTOR_XYT = 0,
	MOTOR_UVW = 1,
};

enum _UVW_AXIS_INFO_ {
	UVW_X = 0, 
	UVW_Y = 1,
};

class CPrealign
{
public:
	CPrealign(void);
	~CPrealign(void);

private:
	int m_nStageType;	// 0 : XYT, 1 : UVW
	int m_nMotorType;

	// 기존 코드 땨문에 임의로 부호 변경하는거 하나 더 만듬...ㅠㅠ
	int m_nRevisionDirX;
	int m_nRevisionDirY;

	double m_dbLCheckX[4];
	double m_dbLCheckY[4];
	double m_dbPosX[MAX_CAMERA];
	double m_dbPosY[MAX_CAMERA];
	double m_dbRevisionData[NUM_AXIS];
	double m_dbRotateX[MAX_CAMERA];
	double m_dbRotateY[MAX_CAMERA];
	double m_dbPatternPitchX;	// Fiducial Mark 간격
	double m_dbPatternPitchY;	// Fiducial Mark 간격

	int m_nAlignmentType;	// 보정 타입 0 : 왼쪽 정렬, 1 : 오른쪽 기준, 2: 가운데 정렬
	int m_nCameraPosType;	// 카메라 관계 0 : 수평 방향, 1 : 수직 방향
	double m_dbPrevRevisionX[4];
	double m_dbPrevRevisionY[4];
	double m_dbPrevRevisionT[4];
	double m_dbPrevPosX[4];
	double m_dbPrevPosY[4];
	double m_dbCurrRevisionX[4];
	double m_dbCurrRevisionY[4];
	double m_dbCurrRevisionT[4];
	double m_dbCurrPosX[4];
	double m_dbCurrPosY[4];
	double m_dRevisionOffset[3];

	double mark_offset_x[MAX_CAMERA];
	double mark_offset_y[MAX_CAMERA];

	BOOL use_image_theta;
	double image_theta;
	BOOL m_bRevisionRotateXY;	// 반영 하는 곳이 90도 회전하여 내려 놓는 경우
public:
	CDialog *pDlg;

	BOOL m_bReverseAxisX;
	BOOL m_bReverseAxisY;
	BOOL m_bReverseAxisT;
	BOOL m_bReverseAxisXY;

	void set_use_image_theta(BOOL v) { use_image_theta = v; }
	void set_image_theta(double v) { image_theta = v; }
	BOOL get_use_image_theta() { return use_image_theta;}

	BOOL getRevisionRotate_XY() { return m_bRevisionRotateXY; }

	void calcRotate(double rx, double ry, double x1, double y1, double rangle, double *x2, double *y2);
	double getRevisionOffset(int nAxis) { return m_dRevisionOffset[nAxis]; }
	double getLcheckX(int nIndex) { return m_dbLCheckX[nIndex]; }
	double getLcheckY(int nIndex) { return m_dbLCheckY[nIndex]; }
	double getPrevRevisionX(int nDir) { return m_dbPrevRevisionX[nDir]; }
	double getPrevRevisionY(int nDir) { return m_dbPrevRevisionY[nDir]; }
	double getPrevRevisionT(int nDir) { return m_dbPrevRevisionT[nDir]; }
	double getPrevPosX(int nDir) { return m_dbPrevPosX[nDir]; }
	double getPrevPosY(int nDir) { return m_dbPrevPosY[nDir]; }

	double getCurrRevisionX(int nDir) { return m_dbCurrRevisionX[nDir]; }
	double getCurrRevisionY(int nDir) { return m_dbCurrRevisionY[nDir]; }
	double getCurrRevisionT(int nDir) { return m_dbCurrRevisionT[nDir]; }
	double getCurrPosX(int nDir) { return m_dbCurrPosX[nDir]; }
	double getCurrPosY(int nDir) { return m_dbCurrPosY[nDir]; }

	void setRevisionRotate_XY(BOOL bSet) { m_bRevisionRotateXY = bSet; }

	void setRevisionDirX(int nDir) { m_nRevisionDirX = nDir; }
	void setRevisionDirY(int nDir) { m_nRevisionDirY = nDir; }

	void setRevisionOffset(int nAxis, double offset) { m_dRevisionOffset[nAxis] = offset; }
	void setPatternPitchX(double pitch) { m_dbPatternPitchX = pitch; }
	void setPatternPitchY(double pitch) { m_dbPatternPitchY = pitch; }
	void setReverseAxisX(BOOL bSet) { m_bReverseAxisX = bSet; }
	void setReverseAxisY(BOOL bSet) { m_bReverseAxisY = bSet; }
	void setReverseAxisT(BOOL bSet) { m_bReverseAxisT = bSet; }
	void setReverseAxisXY(BOOL bSet) { m_bReverseAxisXY = bSet; }

	void setPosX(int pos, double v) { m_dbPosX[pos] = v; }
	void setPosY(int pos, double v) { m_dbPosY[pos] = v; }
	double getPosX(int pos) { return m_dbPosX[pos]; }
	double getPosY(int pos) { return m_dbPosY[pos]; }

	void setAlignmentType(int nType) { m_nAlignmentType = nType; }
	void setCameraPosType(int nType) { m_nCameraPosType = nType; }
	void setStageType(int nType) { m_nStageType = nType; }
	int getCameraPosType() { return m_nCameraPosType; }
	int getAlignmentType() { return m_nAlignmentType; }
	int getStageType() { return m_nStageType; }

	void setRotateX(int nCam, double value) { m_dbRotateX[nCam] = value; }
	void setRotateY(int nCam, double value) { m_dbRotateY[nCam] = value; }
	double getRotateX(int nCam) { return m_dbRotateX[nCam]; }
	double getRotateY(int nCam) { return m_dbRotateY[nCam]; }

	void set_mark_offset_x(int nCam, double value) { mark_offset_x[nCam] = value; }
	void set_mark_offset_y(int nCam, double value) { mark_offset_y[nCam] = value; }
	double get_mark_offset_x(int nCam) { return mark_offset_x[nCam]; }
	double get_mark_offset_y(int nCam) { return mark_offset_y[nCam]; }

	void setRevisionX(double value) { m_dbRevisionData[AXIS_X] = value; }
	void setRevisionY(double value) { m_dbRevisionData[AXIS_Y] = value; }
	void setRevisionT(double value) { m_dbRevisionData[AXIS_T] = value; }
	void setRevisionZ(double value) { m_dbRevisionData[AXIS_Z] = value; }
	void setRevisionData(double valueX, double valueY, double valueT) { m_dbRevisionData[AXIS_X] = valueX; m_dbRevisionData[AXIS_Y] = valueY; m_dbRevisionData[AXIS_T] = valueT; }

	double getRevisionX() { return m_dbRevisionData[AXIS_X]; }
	double getRevisionY() { return m_dbRevisionData[AXIS_Y]; }
	double getRevisionT() { return m_dbRevisionData[AXIS_T]; }
	double getRevisionZ() { return m_dbRevisionData[AXIS_Z]; }
	double *getRevisionData() { return m_dbRevisionData; }
	
	int calcRevision(int nCamL, int nCamR, int nCamPosType, BOOL bRevisionReverse = FALSE, BOOL bRotateOffsetPreCalc = FALSE);	// 보정량 계산 함수
	int calcRevision(int nCamL, int nCamR, double theta, BOOL bRevisionReverse = FALSE, BOOL bRotateOffsetPreCalc = FALSE);	// 보정량 계산 함수
	int calcRevisionRobot(int nCamL, int nCamR, int nCamPosType, BOOL bRevisionReverse = FALSE, BOOL bRotateOffsetPreCalc = FALSE);
	int calcRevision4Cam_Center(int nCamLT, int nCamRT, int nCamLB, int nCamRB, int nCamPosType, BOOL bRevisionReverse = FALSE, BOOL bRotateOffsetPreCalc = FALSE);

	void calcUVW(double refT_M1, double refT_M2, double refT_M3, int nAxisDir1, int nAxisDir2, int nAxisDir3,	// UVW 보정량 계산 함수
	double dR, double initT, double dT, double *delta1, double *delta2, double *delta3);
	void calcRotateOffset(double Theta, double &xMove, double &yMove);
};

#endif