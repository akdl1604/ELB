#pragma once

#ifndef __USER_DEFINE_ADDRESS_H__
#define __USER_DEFINE_ADDRESS_H__

#include <io.h>
#include "UI_Control\CallbackThread.h"
#include "UI_Control/LabelEx.h"
#include "UI_Control/ButtonEx.h"
#include "UI_Control\CustomTabCtrl.h"
#include "UI_Control/ViewerEx.h"
#include "Basler\Camera.h"
#include "KeyPadDlg.h"
#include "DlgMessage.h"
#include "DlgViewer.h"
#include "Caliper\DlgCaliper.h"
#include "SimpleSplashWnd.h"

#include "Caliper/MatrixAlgebra.h"
#include "Caliper/RANSAC_EllipseFittingAlgorithm.h"
#include "ImageProcessing/BetterBlobDetector.h"

#define JOB_INFO

#ifndef _NOT_USE_HALCON
#include "HalconCpp.h"
#pragma comment(lib, "halconcpp.lib")
#endif

#ifndef CAM_COUNT
#define CAM_COUNT 4
#endif

#ifndef NUM_POS
#define NUM_POS 4
#endif

#ifndef NUM_AXIS
#define NUM_AXIS 3
#endif
#ifndef MAX_PANEL
#define MAX_PANEL	100
#endif

#ifndef NUM_ALIGN_POINTS
#define NUM_ALIGN_POINTS 10
#endif NUM_ALIGN_POINTS

#define MAX_PATTERN_INDEX 5
#define MAX_MASK_INDEX 4

#define MAX_VIEWER 16

#define MAX_JOB 8
#define MAX_CAMERA 8
#define MAX_INDEX 2
#define MAX_LIGHT_COUNT 3
#define MAX_SAVE_IMAGE_RING_BUF 8

#define PACKET_HEADER_SIZE (sizeof(long) * 4)
#define MAX_TCP_SIZE (1024 * 12)

#define SIZE_MATCHING_RATE_DATA 2
#define SIZE_MATCHING_SETTING_DATA 1
#define SIZE_REVISION_DATA 6
#define NUM_CALIB_POINTS 200
#define NUM_ROTATE_CENTER_POINTS 2

#define MOTOR_SCALE 10000.
#define THETA_SCALE 10000.

#define ENC_SCALE 10000

#ifndef WM_VIEW_CONTROL
#define WM_VIEW_CONTROL WM_USER + 0x10
#endif

#define WM_MODEL_CHANGE	WM_USER + 1000
#define WM_TRAY_NOTIFICATION WM_APP + 1

#define _SCAN_GRAB_MAXCOUNT     6   // SCAN MAX COUNT

#define COLOR_ORANGE			RGB(255, 125, 0)
#define COLOR_LIME				RGB(128,255,128)
#define COLOR_DARK_LIME			RGB(64, 128, 64)
#define COLOR_RED				RGB(255, 64, 64)
#define COLOR_PINK				RGB(255,128,255)
#define COLOR_YELLOW			RGB(255,255, 64)
#define COLOR_BLUE				RGB( 64, 64,255)
#define COLOR_DARK_BLUE			RGB( 128, 128,255)
#define COLOR_GREEN				RGB( 64,255, 64)
#define COLOR_REALGREEN			RGB(  0,255,  0)
#define COLOR_DARK_GRAY			RGB(128,128,128)
#define COLOR_DDARK_GRAY		RGB( 96, 96, 96)
#define COLOR_PURPLE			RGB(255, 64, 255)
#define COLOR_DARKPURPLE		RGB(255, 128, 255)
#define COLOR_SKY_BLUE			RGB(0, 216, 255)
#define COLOR_GRAY				RGB(192,192,192)
#define COLOR_BLACK				RGB(  0,  0,  0)
#define COLOR_CONNECT			RGB( 41,192, 41)
#define COLOR_DISCONNECT		RGB( 45, 47, 49)
#define COLOR_WHITE				RGB(255,255,255)
#define COLOR_BTN_SELECT		RGB( 64, 92,247)
#define COLOR_UI_BODY			RGB( 45, 47, 49)
#define COLOR_UI_SIDE			RGB(128, 47, 49)
#define COLOR_BTN_BODY			RGB( 64, 64, 64)
#define COLOR_BTN_SIDE			RGB( 96, 96, 96)

#define COLOR_GRAPH_1			COLOR_RED
#define COLOR_GRAPH_2			RGB(255,128, 64)
#define COLOR_GRAPH_3			COLOR_GREEN
#define COLOR_GRAPH_4			COLOR_BLUE
#define COLOR_GRAPH_5			COLOR_LIME
#define COLOR_GRAPH_6			RGB(128, 64,128)
#define COLOR_GRAPH_7			RGB(  0,128,128)
#define COLOR_GRAPH_8			RGB(128,128,  0)
#define COLOR_GRAPH_9			RGB(128, 64,  0)
#define COLOR_GRAPH_10			RGB(255,  0,128)
#define COLOR_GRAPH_11			RGB(255,128, 64)
#define COLOR_GRAPH_12			RGB(255,255,100)

#define MSG_VIEWCTL_SET_MOUSE_POS    87898
#define MSG_PLC_CONNECT_STATUS        97898
#define MSG_PLC_CONNECT_STATUS_CONNECT    97899
#define MSG_VAT_CONNECT_STATUS_CONNECT    97900

#define df_MAX_CNT 20
int const C_CALIPER_POS_1 = 0;
int const C_CALIPER_POS_2 = 1;
int const C_CALIPER_POS_3 = 2;
int const C_CALIPER_POS_4 = 3;
int const C_CALIPER_POS_5 = 4;
int const C_CALIPER_POS_6 = 5;

#define MAX_INSP_POS 10   // dh.jung 2021-08-02 add ELB UT Scan 검사 위치. 현재 3군데 사용. (pos1 : 56mm, pos2 : 6mm, pos3 : 6mm) 

enum _TCP_SEND_MESSAGE {
	TCP_SEND_CONNECT = 10000,				// 기본 연결 확인 및 Model 관련 10000부터
	TCP_SEND_AUTO_READY = 10001,			// Auto Ready 할 상황이 아닌 경우 전송
	TCP_SEND_MODEL_CHANGE_NG = 10002,
	TCP_SEND_MODEL_MODIFY_NG = 10003,
	TCP_SEND_ALARM_MESSAGE = 10010,
	TCP_SEND_SELECT_CLIENT = 10100,			// 정밀 얼라인 선택
	TCP_SEND_PANEL_EXIST_RESULT = 10110,	// Send Panel Exist Result
	TCP_SEND_INPUT_ALIGN_RESULT_IMAGE = 10111,
	TCP_SEND_SHOW_SERVER = 10112,
	TCP_SEND_CHIP_EXIST_RESULT = 10120,		// Send Chip Exist Result

	TCP_SEND_PROCESS_OK = 12000,			// Prealign 관련 12000 부터
	TCP_SEND_PROCESS_NG = 12001,
	TCP_SEND_REVISION_DATA = 12002,
	TCP_SEND_CONFIRM_IMAGE_SAVE = 12003,
	TCP_SEND_RETRY_ALIGN = 12004,
	TCP_SEND_ALIGN_MARK_MISS = 12005,
	TCP_SEND_THICK_MISS = 12006,
	TCP_SEND_MEASURE_MISS = 12007,
	TCP_SEND_REVISION_OFFSET_DATA = 12008,
	TCP_SEND_ALIGN_CRACK_NG = 12010,
	TCP_SEND_ALIGN_L_CHECK_NG = 12011,
	TCP_SEND_MANUAL_MARK_INPUT = 12020,		//MANUAL MARK INPUT

	TCP_SEND_READ_BIT_DATA = 15000,
	TCP_SEND_WRITE_BIT_DATA = 15001,
	TCP_SEND_READ_WORD_DATA = 15002,
	TCP_SEND_WRITE_WORD_DATA = 15003,
	TCP_SEND_WRITE_ONEBIT_DATA = 15004,
	TCP_SEND_READ_ONEBIT_DATA = 15005,
	TCP_SEND_WRITE_POSDISTANCE_DATA = 15006,

	TCP_SEND_LIGHT_CONTROL_ACK = 18000,
	TCP_SEND_LIGHT_CONTROL_ARRAY_ACK = 18001,
	TCP_SEND_TRIGGER_ACK = 18002,
	TCP_SEND_TRIGGER_CURR_POS_ACK = 18003,
	TCP_SEND_LIGHT_CONTROL_ARRAY2_ACK = 18005,

	TCP_SEND_MOVE_TO_BEFORE_ALIGN_POS = 20000,
	TCP_SEND_CALIBRATION_REQUEST = 20001,
	TCP_SEND_CSCAM_REVISION_DATA = 20002,
	TCP_SEND_CSCAM_CALIB_DATA = 20003,
	TCP_SEND_CALIB_END = 20004,
	TCP_SEND_CSCAM_CALIB_ON = 20005,
	TCP_SEND_GET_ATTACH_MOTOR_POS = 20006,
	TCP_SEND_ROTATE_CENTER = 20007,
	TCP_SEND_OVERFLOW_INSPEND = 20008,
	TCP_SEND_MOLDSIZE_INSPEND = 20009,
	TCP_SEND_OVERFLOW_JUDGE = 20010,		//20180610
	TCP_SEND_2ND_GRAB_START = 20011,		//20180610	
	TCP_SEND_CALIBRATION_MODE_ON_ACK = 20020,

	TCP_SEND_INSPECTION_RESULT = 30000,
	TCP_SEND_INSPECTION_START = 30001,
	TCP_SEND_INSPECTION_LGRAB = 30002,
	TCP_SEND_INSPECTION_RGRAB = 30003,
	TCP_SEND_ATTACH_RESULT = 30004,
	TCP_SEND_DOPO_RESULT = 30005,
	TCP_SEND_DUSTMAX_RESULT = 30006,
	TCP_SEND_AAFS_DATA = 30007,


	TCP_SEND_LAMI_STAGE_INSPECTION_RESULT = 30010,
	TCP_SEND_COVER_GLASS_INSPECTION_RESULT = 30011,

	TCP_SEND_CAMERA_GRAB_END = 50000,
	TCP_SEND_CAMERA_PROCESS_END = 50001,
	TCP_SEND_CAMERA_GRAB_NG = 50002,
	TCP_SEND_1ST_GRAB_END = 50003,
	TCP_SEND_2ND_POS_COMPLETE_CHECK = 50004,

	TCP_SEND_LIGHT_ONOFF = 60000,
	TCP_SEND_LIGHT_BRIGHT = 60001,
	TCP_SEND_CAM_GAIN = 60002,
	TCP_SEND_CAM_EXPOSURE = 60003,

	TCP_SEND_DRAW_TARGET = 70000,

	TCP_SEND_THICK_RESULT = 80000,
	TCP_SEND_THICK_LIGHT_ONOFF = 80001,
	TCP_SEND_TAPE_LIGHT_ONOFF = 80002,

	TCP_SEND_LIGHT_CONTROL = 90000,
	TCP_SEND_LIGHT_CONTROL_NO_SAVE = 90002,

};

enum _TCP_RECV_MESSAGE {
	TCP_RECV_AUTO_READY = 10000,			// Main에서 Start 버튼 누른 경우
	TCP_RECV_AUTO_STOP = 10001,				// Main에서 Stop 버튼 누른 경우
	TCP_RECV_CREATE_MODEL = 10002,			// Main에서 Model을 생성한 경우
	TCP_RECV_MODEL_CHANGE = 10003,
	TCP_RECV_PANEL_ID = 10004,				// 검사 시작 전에 Panel ID 부터 전송
	TCP_RECV_DELETE_MODEL = 10005,
	TCP_RECV_MODIFY_MODEL = 10006,
	TCP_RECV_COPY_MODEL = 10007,
	TCP_RECV_RENAME_MODEL = 10008,
	TCP_RECV_CLOSE_PROGRAM = 10010,
	TCP_RECV_CLIENT_HIDE = 10011,
	TCP_RECV_SENSOR_SHAPE = 10012,
	TCP_RECV_PLC_TIME = 10099,				// 20.05.08 PLC 시간 설정.
	TCP_RECV_SELECT_CLIENT = 10100,			// 정밀 얼라인 선택
	TCP_RECV_SHOW_SERVER = 10112,
	TCP_RECV_SECOND_PANEL_ID = 10113,

	TCP_RECV_PLC_MODEL_CREATE = 10500,
	TCP_RECV_PLC_MODEL_CHANGE = 10501,
	TCP_RECV_PLC_MODEL_MODIFY = 10502,
	TCP_RECV_PLC_MODEL_DELETE = 10503,
	TCP_RECV_PLC_MODEL_RENAME = 10504,

	TCP_RECV_PROCESS_START = 12000,			// Prealign 관련 12000부터
	TCP_RECV_CONFIRM_IMAGE_SAVE = 12001,
	TCP_RECV_RETRY_ALIGN = 12004,
	TCP_RECV_ALIGN_MARK_MISS = 12005,
	TCP_RECV_MANUAL_MARK_INPUT = 12020,
	TCP_RECV_SECONDALING_START = 12100,		// Second Align Start
	TCP_RECV_PROCESS_GRAB = 12200,
	TCP_RECV_PROCESS_START_NEW = 12300,		// Start 시 여러 정보 한번에 전달 테스트

	TCP_RECV_READ_BIT_DATA = 15000,
	TCP_RECV_READ_WORD_DATA = 15002,
	TCP_RECV_WRITE_ONEBIT_DATA = 15004,
	TCP_RECV_READ_ONEBIT_DATA = 15005,
	TCP_RECV_WRITE_POSDISTANCE_DATA = 15006,

	TCP_RECV_LIGHT_CONTROL_ACK = 18000,
	TCP_RECV_LIGHT_CONTROL_ARRAY_ACK = 18001,

	TCP_RECV_CSCAM_MOVE_COMPLETE = 20000,
	TCP_RECV_CSCAM_MOVE_NG = 20001,
	TCP_RECV_CSCAM_REVISION_NG = 20002,
	TCP_RECV_CALIBRATION_ACK = 20003,
	TCP_RECV_CALIBRATION_NG = 20004,
	TCP_RECV_ATTACH_MOTOR_POS = 20006,
	TCP_RECV_OVERFLOW_INSPEND = 20008,
	TCP_RECV_MOLDSIZE_INSPEND = 20009,
	TCP_RECV_OVERFLOW_JUDGE = 20010,
	TCP_RECV_2ND_GRAB_START = 20011,
	TCP_RECV_CALIBRATION_MODE_ON_ACK = 20020,	// 19.10.29

	TCP_RECV_INSPECTION_RESULT = 30000,
	TCP_RECV_INSPECTION_START = 30001,
	TCP_RECV_INSPECTION_LGRAB = 30002,
	TCP_RECV_INSPECTION_RGRAB = 30003,
	TCP_RECV_INSPECTION_SHIFT = 30004,

	TCP_RECV_LAMI_STAGE_INSPECTION_START = 30010,
	TCP_RECV_COVER_GLASS_INSPECTION_START = 30011,	// 첫번째, 두번째 검사로 구분


	TCP_RECV_CAMERA_GRAB_END = 50000,
	TCP_RECV_CAMERA_PROCESS_END = 50001,

	TCP_RECV_1ST_GRAB_END_ACK = 50003,
	TCP_RECV_2ND_POS_COMPLETE_CHECK_ACK = 50004,

	TCP_RECV_LIGHT_ONOFF = 60000,
	TCP_RECV_LIGHT_BRIGHT = 60001,
	TCP_RECV_CAM_GAIN = 60002,
	TCP_RECV_CAM_EXPOSURE = 60003,

	TCP_RECV_DRAW_TARGET = 70000,

	TCP_RECV_THICK_RESULT = 80000,
	TCP_RECV_THICK_LIGHT_ONOFF = 80001,

	TCP_RECV_LIGHT_CONTROL = 90000,
	TCP_RECV_OCTAGON_CONTROL = 90009,

	TCP_RECV_DOPOSTAGE_CONTROL = 90100,
	TCP_RECV_RELOG_CONTROL = 90110,
	TCP_RECV_PEELER_RELOG_CONTROL = 90120,
	TCP_RECV_MODEL_NAMI = 99001,
};


enum FIND_INFO {
	FIND_OK = 0,
	FIND_ERR,
	FIND_MATCH,
};

enum _SOBEL_DIRECTION {
	SOBEL_UP,
	SOBEL_DN,
	SOBEL_LEFT,
	SOBEL_RIGHT,
	SOBEL_45, 
    SOBEL_135,
	SOBEL_225,
	SOBEL_315,
};

enum _EDGE_POLARITY_ {
	EDGE_LIGHT_TO_DARK = 0,
	EDGE_DARK_TO_LIGHT,
};
enum _MESSAGE_TYPE {
	MT_OK = 1,
	MT_OKCANCEL,
};
enum _EDGE_DIRECTION_ {
	EDGE_OUTER_TO_INNER = 0,
	EDGE_INNER_TO_OUTER,
};
enum _PANEL_EXIST_JUDGE_CONDITION {
	HIGHER_THAN_LIMIT = 0,	// 평균 밝기값이 Limit 보다 큰	경우 OK
	LOWER_THAN_LIMIT,		// 평균 밝기값이 Limit 보다 작은	경우 OK
};

enum _CHIP_RANSAC_DIRECTION {
	DIRECTION_LEFT = 0,
	DIRECTION_RIGHT,
	DIRECTION_DOWN,
	DIRECTION_UP,
	DIRECTION_EMPTY,
};

enum _CHIP_RANSAC_STANDARD {
	STANDARD = 2,
	STANDARD_LEFT = 0,
	STANDARD_RIGHT = 3,
	NOT_USE = 6,
};

enum _LIGHT_ID {
	LIGHT_CTRL1 = 0,
	LIGHT_CTRL2,
	LIGHT_CTRL3,
	LIGHT_CTRL4,

	MAX_LIGHT_CTRL,
};

enum _CORNER_TYPE {
	CORNER_LT = 0,
	CORNER_RT,
	CORNER_LB,
	CORNER_RB,
};

enum _PRODUCT_TYPE {
	_PRO_MODEL = 0,
	_CORE_MODEL,
	_CORE_MODEL_CALC,
};

enum _ALIGN_ORGIN_TYPE {
	_LEFT_TOP_X25_Y25 = 0,
	_CENTER_TOP_X50_Y25,
	_RIGHT_TOP_X75_Y25,
	_LEFT_CENTER_X25_Y50,
	_CENTER_CENTER_X50_Y50,
	_RIGHT_CENTER_X75_Y50,
	_LEFT_BOTTOM_X25_Y75,
	_CENTER_BOTTOM_X50_Y75,
	_RIGHT_BOTTOM_X75_Y75,
	_NOTCH_X85_Y50,
};

enum _PRE_PROCESS_ {
	PRE_NONE = 0,
	PRE_GAUSSIAN_3,
	PRE_MEDIAN_3,
	PRE_MEDIAN_5,
	PRE_MEDIAN_7,
	PRE_MEDIAN_11,
	PRE_CLOSE_1,
	PRE_CLOSE_3,
	PRE_CLOSE_5,
};

enum _EDGE_PROCESS_ {
	EDGE_SOBEL = 0,
	EDGE_ENHANCE_B,
	EDGE_ENHANCE_W_V2,
	EDGE_ENHANCE_W,
	EDGE_HISTO_W,
	EDGE_HISTO_B,
};

enum _MARK_FIND_METHOD {
	METHOD_MATCHING = 0,
	METHOD_RANSAC,
	METHOD_CALIPER,
	METHOD_MULTI_RANSAC,
	METHOD_LINE,
	METHOD_CIRCLE,
	METHOD_NOTCH,
	METHOD_ELLIPSE,
	METHOD_DIFF_INSP,
	METHOD_NOTCH_LINE,
	METHOD_NOTCH_INSP,
	METHOD_HOLE_INSP,
	METHOD_MATCHING_LINE_THETA,
	METHOD_UT_ELB,
	METHOD_MARK_EDGE,
};

enum _FORM_ {
	FORM_MAIN = 0,
	FORM_CAMERA,
	FORM_MODEL,
	FORM_MACHINE,
	FORM_INTERFACE,
	FORM_SYSTEM_OPTION,
	FORM_TREND,
	MAX_FORM,
};

// Pane 추가시 MAX_PANE도 추가 할 것
enum _PANE_ {
	PANE_MANUAL = 0,
	PANE_CAMERA,
	PANE_MODEL,
	PANE_MACHINE,
	PANE_AUTO,
	PANE_SIMULATION,
	MAX_PANE,
};

enum _PANE_BOTTOM {
	B_PANE_BOTTOM = 0,
	B_PANE_SIMULATION,
	MAX_B_PANE,
};

enum _MAIN_DIALOG {
	MSG_MD_ANALYSIS_PACKET = 0,
	MSG_MD_RELEASE_OBJECT,
	MSG_MD_CAMERA_LIVE,
	MSG_MD_CAMERA_STOP,
	MSG_MD_CAMERA_SINGLE_FRAME,
	MSG_MD_INC_COUNT,
	MSG_MD_SEND_REVISION_DATA,
	MSG_MD_SHOW_NEXTWINDOW,
	MSG_MD_PEELER_TEST,
	MSG_MD_OVERFLOW_TEST,
	MSG_MD_PREMOLDCALC_TEST, //20180611
	MSG_MD_GRAB_THREAD,
	MSG_MD_CAMERA_MULTI_FRAME,
	MSG_MD_DIFF_RESULT_WRITE,
};

enum _FORM_MAIN_VIEW {
	MSG_FMV_RELEASE_OBJECT = 100,
	MSG_FMV_THREAD_CREATE,
	MSG_FMV_JUDGE,
	MSG_FMV_DRAW_ALIGN_RESULT,
	MSG_FMV_DISPLAY_PANEL_EXIST,
	MSG_FMV_DISPLAY_PCB_DISTANCE,
	MSG_FMV_DISPLAY_PANEL_ASSEMBLE,
	MSG_FMV_SET_MODELNAME,
	MSG_FMV_GLASSID_UPDATE,
	MSG_FMV_DISPLAY_TARGET_DRAW_VIEW,
	MSG_FMV_SHOW_MANUAL_INPUT_MARK,
	MSG_FMV_SHOW_MANUAL_INPUT_MARK_1CAM,
	MSG_FMV_INIT_INTERFACE,
	MSG_FMV_TOOL_COMMAND,
	MSG_FMV_DRAW_LINE_RESULT,
	MSG_FMV_DISPLAY_SCAN_INSP,
	MSG_FMV_DISPLAY_FILM_DISTANCE,
	MSG_FMV_AUTO_SEQ_TEST,
	MSG_FMV_AUTO_LOGOFF_ADMIN_MODE,
	MSG_FMV_LOGIN_STATUS_DISPLAY,
};
enum _MODEL_VIEW {
	MSG_MV_DISP_MODEL_LIST = 0,
	MSG_MV_DELETE_MODEL,
	MSG_MV_DISP_MODEL_INFO,
	MSG_MV_DISP_MODEL_ID,
	MSG_MV_ENABLE_CONTROL,
	MSG_MV_SAVE_ALGO_DATA,
	MSG_MV_LOAD_ALGO_DATA,
	MSG_MV_SEND_CURR_MODEL,
	MSG_MV_PARAM_ALGO_DATA,
	MSG_MV_PARAM_ALGO_DATA_SAME_CENTER_ITEM, //lhj add 220503
};

enum _FORM_CAMERA_VIEW {
	MSG_FCV_SET_MOUSE_POS = 0,
	MSG_FCV_PATTERN_REGIST,
	MSG_FCV_PATTERN_VIEW,
	MSG_FCV_PATTERN_SEARCH,
	MSG_FCV_PATTERN_SET_ROI,
	MSG_FCV_PATTERN_SET_ROI2,	// 20.03.26
	MSG_FCV_PATTERN_DELETE,
	MSG_FCV_PATTERN_DISPLAY_RESULT,
	MSG_FCV_FOCUS_MEASURE,
	MSG_FCV_SET_SEARCH_POS,
	MSG_FCV_SET_CAMERA_POS,
	MSG_FCV_CAM_INSP_PROCESSING,
	MSG_FCV_INSP_SHOW_PROFILE,
	MSG_FCV_CALC_WHOLE_ROI,
	MSG_FCV_MANUAL_INSPECTION,
	MSG_FCV_SEARCH_CALIPER_INSP,
	MSG_FCV_DRAW_CALIPER_INSP,
	MSG_FCV_DRAW_DISTANCE_INSP,
	MSG_FCV_INSP_NOTCH,
	MSG_FCV_INSP_NOTCH_DOPO, 
	MSG_FCV_DRAW_REFERNCE_POS,
	MSG_FCV_DRAW_ALL_REFERNCE_POS, //2022-03-11 hsj All reference 등록 그리기 
	MSG_FCV_DRAW_FIXTURE_POS,
	MSG_FCV_INSP_SCAN, 
	MSG_FCV_INSP_NOTCH_NOZZLE,
	MSG_FCV_INSP_NOTCH_NOZZLE_ANGLE,
	MSG_FCV_INSP_NOZZLE_GAP, //20211007 Tkyuha 노즐 
	MSG_FCV_INSP_NOZZLE_DISTANCE_POLAR,
	MSG_FCV_INSP_NOZZLE_VIEW,
	MSG_FCV_INSP_NOTCH_NOZZLE_ANGLE_REVERSE,
	MSG_FCV_INSP_NOZZLE_DISTANCE_REVERSE_CENTER,
	MSG_FCV_INSP_NOZZLE_DISTANCE_REVERSE_SIDE,
	MSG_FCV_INSP_NOZZLE_DISTANCE_REVERSE_CENTER_REF,
	MSG_FCV_INSP_NOZZLE_DISTANCE_REVERSE_SIDE_REF,
	MSG_FCV_OPEN_IMAGE_FILENAME,
	MSG_FCV_INSP_NOZZLE_Z_HEIGHT,
	MSG_FCV_INSP_CENTER_NOZZLE_Y_CALC,
	MSG_FCV_INSP_INK_INSPECTION,
	MSG_FCV_INSP_SCRATCH_INSPECTION,
};

enum _FORM_MACHINE_VIEW {
	MSG_FMV_DISPLAY_MACHINE_VIEW = 100,
	MSG_FMV_DISPLAY_TRAGER_VIEW,
	MSG_FMV_DISPLAY_TARGET_LINE_VIEW,
	MSG_FMV_DISPLAY_CALIBRATION_AREA_VIEW,
	MSG_FMV_DISPLAY_INDIVIDUAL_DRAW_MARK,
	MSG_FMV_DISPLAY_4POINT_CENTER,
	MSG_FMV_SET_ALGORITHM_POS,
	MSG_FMV_SET_ALGORITHM_POS_POSITION,
	MSG_FMV_SET_ALGORITHM_POS_CAMERA,
	MSG_FMV_DISPLAY_MACHINE_VIEW_POS,
};

enum _PANE_AUTO {
	MSG_PA_ADD_PROCESS_HISTORY = 0,
	MSG_PA_UPDATE_INSP_COUNT,
	MSG_PA_UPDATE_INSP_LOAD,
	MSG_PA_UPDATE_INSP_UNLOAD,
	MSG_PA_UPDATE_INSP_RESULT,
	MSG_PA_UPDATE_ALIGN_RESULT,
	MSG_PA_SPEC_VIEW,
	MSG_PA_TIME,
	MSG_MF_UPDATE,
	MSG_PA_JUDGE_DISPLAY,
	MSG_PA_JUDGE_JOB,
	MSG_PA_SCAN_SPEC_VIEW,
	MSG_PA_RESET_RESULT_LIST,
	MSG_PA_RENEW_TITLE
};

enum _PANE_MACHINE_ {
	MSG_PMC_ADD_PROCESS_HISTORY = 0,
};

enum _PANE_MANUAL_ {
	MSG_PMN_MANUAL_COLOR_RESET = 0,
};

// YCS 2022-08-30 차트뷰 관련 메세지 추가
enum _DLG_CHART_VIEW_ {
	MSG_SPEC_REFRESH = 0,
	MSG_DLG_HIDE,
	MSG_DLG_SHOW,
	MSG_DLG_REFRESH
};

enum _HEADER_BAR {
	MSG_HB_CONNECT_SERVER = 0,
	MSG_HB_CONNECT_CAMERA,
	MSG_HB_CONNECT_PLC,
	MSG_HB_CONNECT_TRIGGER,
	MSG_HB_CONNECT_LIGHT,
};

enum _DLG_VIEWER {
	MSG_DLGVIEW_TOOL_COMMAND_FIRST = 0,
	MSG_DLGVIEW_TOOL_COMMAND_SECOND,
	MSG_DLGVIEW_TOOL_COMMAND_THIRD,
	MSG_DLGVIEW_TOOL_COMMAND_FOUR,
	MSG_DLGVIEW_TOOL_COMMAND_LIVE,
	MSG_DLGVIEW_TOOL_COMMAND_RECORD,
	MSG_DLGVIEW_TOOL_COMMAND_STOP,
};

enum _TIMER_DEF {
	TIMER_MOVE_CALIB_POS = 100,
	TIMER_MOVE_CALIB_ACK_POS = 200,
};

enum _TIMER_DEFINE {
	TIMER_TIME_CHECK = 1000,
	TIMER_PREALIGN_TIME_OUT_CHECK,
	TIMER_PLC_READ_DATA,
	TIMER_SERVER_CHECK = 8234,
	TIMER_PLC_CHECK = 8235,
	TIMER_INIT,
	TIMER_AUTO_SEARCH,
	TIMER_INIT_SHARE_MEMORY,
	TIMER_AUTO_FOCUS_MEASURE,
	TIMER_MANUAL_INPUT_TITLE,
	TIMER_PLC_CONNECT,
	TIMER_PLC_ALIVE_PING_PONG,
	TIMER_PLC_ALIVE_CHECK,
	TIMER_CLIENT_CONNECTION_CHECK,
	TIMER_CLIENT_HIDE,
	TIMER_BARCODE_ACK_CHECK,
	TIMER_GET_GMS_DATA,
	TIMER_GET_AUTO_MEASURE,
	TIMER_LMI_START_BIT,
	TIMER_MANUAL_MARK,
	TIMER_IMAGE_SAVE,
	TIMER_AUTO_LOGOFF_ADMIN_MODE,
	TIMER_CLEANING_LIVE_MODE
};

enum _ERROR_TYPE {
	ERR_TYPE_L_CHECK = 1,
	ERR_TYPE_CRACK,
	ERR_TYPE_DUMMY_DIR,
	ERR_TYPE_ANGLE_ERROR,
	ERR_TYPE_REVISION_FAIL,
	ERR_TYPE_REVISION_LIMIT,
	ERR_TYPE_CIRCLE_ERROR,
	MAX_ERR_TYPE,
};

enum _LIGHT_TYPE_ {
	LTYPE_ITEC = 0,
	LTYPE_LLIGHT_LPC_COT_SOCK = 4,
	LTYPE_LLIGHT_LPW_SOCK = 5,
	LTYPE_LLIGHT_LPW_232 = 6,
	LTYPE_LLIGHT_LPC_COT_232 = 7,
};

enum _CLIENT_TYPE {
	CLIENT_TYPE_ALIGN = 0,						// 일반 2캠 1Shot 얼라인	
	CLIENT_TYPE_1CAM_1SHOT_ALIGN = 1,			// 1Cam 1Shot 얼라인 (TYPE 0 = Mark / TYPE 1 = Caliper
	CLIENT_TYPE_1CAM_2SHOT_ALIGN = 2,			// 1Cam 2Shot 얼라인
	CLIENT_TYPE_2CAM_EACH_2SHOT_ALIGN = 3,		// 2Cam 개별적 2Shot 얼라인	
	CLIENT_TYPE_4CAM_1SHOT_ALIGN = 4,			// 2Cam 개별적 2Shot 얼라인	
	CLIENT_TYPE_Z_MEASURE = 5,					// 높이 측정..
	CLIENT_TYPE_EACH_2CAM_1SHOT_ALIGN = 6,		// 개별 1Cam 1Shot 얼라인
	CLIENT_TYPE_PANEL_EXIST_INSP = 7,			// Panel Exist
	CLIENT_TYPE_2CAM_1SHOT_DUST_INSP = 8,		// 2Cam 1 Shot 이물 검사
	CLIENT_TYPE_1CAM_1SHOT_DUST_INSP = 9,		// 1Cam 1 Shot 이물 검사
	CLIENT_TYPE_2CAM_1SHOT_NOZZLE_INSP = 10,	// Nozzle 검사
	CLIENT_TYPE_1CAM_2POS_REFERENCE = 11,		// 필름 부착기 Reel 얼라인 1Cam 1Shot 2개 얼라인
	CLIENT_TYPE_1CAM_1POS_REFERENCE = 12,
	CLIENT_TYPE_ALIGN_EXIST = 13,
	CLIENT_TYPE_COVER_ALIGN_INSP = 14,
	CLIENT_TYPE_STAGE_INSP = 15,
	CLIENT_TYPE_PCB_DISTANCE_INSP = 16,
	CLIENT_TYPE_ASSEMBLE_INSP = 17,
	CLIENT_TYPE_ELB_CENTER_ALIGN = 18,
	CLIENT_TYPE_NOZZLE_ALIGN = 19,
	CLIENT_TYPE_SCAN_INSP = 20,
	CLIENT_TYPE_NOZZLE_SIDE_VIEW = 21,			// 20211007 Tkyuha 노즐 갭측정용 모드
	CLIENT_TYPE_FILM_INSP = 22,					// 20211015 Tkyuha 필름 부착후 검사
	CLIENT_TYPE_CENTER_NOZZLE_ALIGN = 23,		// 20211205 Tkyuha 신규 노즐 얼라인
	CLIENT_TYPE_CENTER_SIDE_YGAP = 24,			// 20211205 Tkyuha 스테이지 와 노즐 높이 측정
	CLIENT_TYPE_1CAM_1SHOT_FILM = 25,			//KJH 2022-05-11 1Cam 1SHot Film Align[Grab Bit 추가 버전]
	CLIENT_TYPE_1CAM_4POS_ROBOT = 26,			//KJH 2022-08-04 1Cam 4POS 2OBJECT REFERENCE ROBOT ALIGN
	CLIENT_TYPE_ONLY_VIEW = 29,
	CLIENT_TYPE_1CAM_1SHOT_ALIGN_WITH_SCRATCH_INSP = 30,		// SJB 2022-10-03 Scratch Insp
};


enum _DISTANCE_INSP_METHOD {
	DISTANCE_INSP_METHOD_M_TO_M = 0,
	DISTANCE_INSP_METHOD_M_TO_E = 1,
	DISTANCE_INSP_METHOD_E_TO_E = 2,
	DISTANCE_INSP_METHOD_L_TO_M = 3,
	DISTANCE_INSP_METHOD_L_TO_E = 4,
	DISTANCE_INSP_METHOD_AUTO = 5,
};

enum _ELB_INSP_ERROR_CODE {
	ELB_INSP_ERROR_WET_IN_MIN = 0,
	ELB_INSP_ERROR_WET_IN_MAX,
	ELB_INSP_ERROR_WET_OUT_MIN,
	ELB_INSP_ERROR_WET_OUT_MAX,
	ELB_INSP_ERROR_B_MIN,
	ELB_INSP_ERROR_B_MAX,
	ELB_INSP_ERROR_R_MIN,
	ELB_INSP_ERROR_R_MAX,
	ELB_INSP_ERROR_LACK_TOTAL,
	ELB_INSP_ERROR_LACK_CONTINUE,
	ELB_INSP_ERROR_OVER_FLOW,
	ELB_INSP_ERROR_UNDER_FLOW,
	ELB_INSP_ERROR_LINE_WIDTH,
	ELB_INSP_MAX_ERROR_TYPE,
};

enum _EGL_INSP_ERROR_CODE {
	EGL_INSP_ERROR_OVERFLOW_MAX = 0,
	EGL_INSP_ERROR_OVERFLOW_MIN = 1,
};

enum _DISTANCE_INSP_DEFINE {
	FIND_PANEL = 0,
	FIND_PCB_BOTTOM = 1,

	POINT_PANEL = 0,							// Panel
	POINT_PCB_BOTTOM = 1,						// PCB or Bottom
	POINT_CROSS_1 = 2,							// 수직 선에 수선의발 내린지점
	POINT_CROSS_2 = 3,							// 수평 선에 수선의발 내린지점
	POINT_PROTRUDING = 4,						// Assemble 돌출부분

	CALIPER_PANEL_VERT = 0,
	CALIPER_PANEL_HORI = 1,
	CALIPER_PCB_VERT = 3,
	CALIPER_PCB_HORI = 4,

	FIND_CALIPER_PANEL_VERT = 0,
	FIND_CALIPER_PANEL_HORI = 1,
	FIND_CALIPER_PCB_VERT = 2,
	FIND_CALIPER_PCB_HORI = 3,
};

enum _DRAW_OBJECT_SIZE {
	MAIN_VIEW_PEN_THICKNESS = 2,
	MAIN_VIEW_PEN_LENGTH = 15,
	MAIN_VIEW_FONT_WIDTH = 60,
	MAIN_VIEW_FONT_HEIGHT = 80,
	CAM_VIEW_PEN_THICKNESS = 2,
	CAM_VIEW_PEN_LENGTH =15,
	CAM_VIEW_FONT_WIDTH =40,
	CAM_VIEW_FONT_HEIGHT =60,
};

enum INSP_COUNT
{
	INSP_1ST = 0,
	INSP_2ND,
	INSP_3RD,
	INSP_4TH,
	MAX_INSP_COUNT,
};

//KJH 2021-07-22 AlignInspAlgorithm
enum ELB_SEQNUM_ALIGN_INSP_ALOGORITHM
{
	ALIGN_CENTER_RESET			= 0,
	ALIGN_CENTER_GRAB			= 10,
	ALIGN_DUMMY_WAIT_GRAB_END	= 15,
	ALIGN_CG_WAIT_GRAB_END      = 17,
    ALIGN_NOZZLE_DIST_WAIT_GRAB_END = 18,		//20211005 Tkyuha 추가
	ALIGN_WAIT_OFF_REQ			= 19,
	ALIGN_TRACE_GRAB_START		= 20,
	ALIGN_TRACE_GRAB_END		= 21,
	ALIGN_TRACE_2ND_GRAB		= 16,
	ALIGN_INSP_PRE_GRAB			= 22,
	ALIGN_INSP_GRAB_START		= 30,
	ALIGN_INSP_GRAB_END			= 31,
	ALIGN_TRACE_MEASURE_END		= 32,
	ALIGN_DIFF_INSP_END			= 33,
	ALIGN_DUMMY_INSP_END		= 34,
    ALIGN_NOZZLE_DIST_END       = 35,
	ALIGN_TRACE_MANUAL_MARK     = 36,
	ALIGN_CG_GRAB_RESET         = 37,
	ALIGN_RESET_FLAG			= 120,
	ALIGN_NOZZLE_CHECK_GRAB_END = 210,
	ALIGN_NOZZLE_CHECK_END		= 220,
};

// HSJ 2022-01-29 Arrow directon
enum _DRAW_ARROW_DIRECTION {
	ARROW_LEFT = 0,
	ARROW_RIGHT,
	ARROW_DOWN,
	ARROW_UP,
};

enum _OFFSET_TARGET
{
	OFFSET_ROTATE_CENTER = 0,
	MAX_OBJECT
};

enum _USER_LEVEL
{
	MASTER = 0,
	SUPER,
	USER,
};

enum _ALARM_MESSAGE {
	ALARM_DIFF_LACK_WARNING = 0,
	ALARM_DIFF_LACK_INTERLOCK,
	ALARM_WETOUT_WARNING,			// YCS 2022-11-30 WetOut ASPC 인터락 관련 사전경고 메세지 추가
	MAX_ALARM_DLG
};

struct _stLoadBeforeImage {
	int nJob;
	int nViewer_index1;
	int nViewer_index2;
	CDialog* pDlg;
};

struct _stLoadScratchImage{
	CDialog* pDlg;
	int nJob;
	BYTE* ImgByte;
	CDlgViewer* viewerDlg;
	PSTBLOB vtBlob[10];
};

struct _stFindPattern {
	int nCam;
	int nPos;
	int nJob;
	int nSearchCount;
	CDialog *pDlg;
};

struct _stInspectionPattern {
	int nCam;
	int nJob;
	bool bEvent;
	CPoint ptStart;
	CPoint ptEnd;
	CDialog* pDlg;
	CViewerEx* mViewer;
};

struct _stPutinProBuffer {
	int nCam;
	int nPos;
	int nJob;
	CString strPath;
	CView *pView;
};


struct _stAlignResult {
	int nCam;
	int nPos;
	int nViewer;
	int nAlgorithm;
	BOOL bErase;
	//KJH 2019-05-29 Center Align 디스플레이 관련 인자 추가
	BOOL bType = FALSE;
};

class PF_FILM_INSP 
{
public:
	double panel_pos_x[2];
	double panel_pos_y[2];
	double film_pos_x[2];
	double film_pos_y[2];
	double dist_lx[3];
	double dist_ly[3];
	double dist_rx[3];
	double dist_ry[3];
	double panel_angle;
	double film_angle;
	double attach_angle;
	BOOL judge[4];
	BOOL final_judge;
	sLine line_panel;
	sLine normal_line_panel[3];
	int nExceptionJudgeCount;
	double dbTarget_Value[2];
	void clear() {
		memset(panel_pos_x, 0, sizeof(panel_pos_x));
		memset(panel_pos_y, 0, sizeof(panel_pos_y));
		memset(film_pos_x, 0, sizeof(film_pos_x));
		memset(film_pos_y, 0, sizeof(film_pos_y));
		memset(judge, 0, sizeof(judge));
		memset(dist_lx, 0.0, sizeof(dist_lx));
		memset(dist_ly, 0.0, sizeof(dist_ly));
		memset(dist_rx, 0.0, sizeof(dist_rx));
		memset(dist_ry, 0.0, sizeof(dist_ry));
		panel_angle = 0.0;
		film_angle = 0.0;
		attach_angle = 0.0;
		final_judge = FALSE;
		nExceptionJudgeCount = 0;
		memset(dbTarget_Value, 0.0, sizeof(dbTarget_Value));
	}

};

struct _stInsp_DistanceResult {
	int nCam;
	int nPos;
	BOOL   bFindPattern[2];	// [ PANEL / PCB ]
	BOOL  bFindLine[4];
	BOOL   bNG;
	sLine  line[4];		// 0, 1 : PANEL		2, 3	: PCB	
	double xPos[5];		// 0 : PANEL, 1 : PCB(or BOTTOM), 2 : 교차점1, 3 : 교차점2,  4 : 돌출부분
	double yPos[5];		// 0 : PANEL, 1 : PCB(or BOTTOM), 2 : 교차점1, 3 : 교차점2,  4 : 돌출부분
	double xPosWorld[2];
	double yPosWorld[2];
	double dDistance;
	double dWidthDistance;
	double dHeightDistance;
	double dCameraPos;				//카메라 포지션 티칭위치 
	double dCameraPosDistance;		//카메라 포지션 거리
	int	   nInspMethodType;
	BOOL   bInspMethodDir;

	double included_angle[2];	// [PANEL/BOTTOM]
	double avg_image_gray;

	double film_insp_dist_x[4];
	double film_insp_dist_y[4];
	double film_insp_pos_x[4];
	double film_insp_pos_y[4];
	double film_insp_film_angle;
	double film_insp_panel_angle;

};

struct _stSaveImage_DistanceResult {
	int nCam;
	int nPos;
	BOOL   bFindPattern[2];
	BOOL   bFindLine[4];
	BOOL   bNG;
	sLine  line[4];
	double xPos[5];
	double yPos[5];
	double xPosWorld[2];
	double yPosWorld[2];
	double dDistance;
	double dWidthDistance;
	double dHeightDistance;
	int	nInspMethodType;
	CDialog *pDlg;
	int nRetry;
	int nAlgorithm;
	int bFinal;
	int nIndex;
	BOOL   bInspMethodDir;
};

struct _stInsp_ELB_Result {
	std::vector<double> m_vTraceProfile[5]; // X,Y,T 프로파일 데이타
	cv::Point2d m_vELB_RotateCenter;
	cv::Point2d m_vELB_ReverseRotateCenter;
	cv::Point2d m_vELB_ImageRotateCenter;
	cv::Point2d m_ELB_ResultXY;
	cv::Point2d m_vELB_DistMinMax;		//	x = min , y = max
	CRect		m_ELB_InspRoi[2];
	sLine		m_ELB_ResultLine[2];
	double		m_ELB_ResultT;
	double		m_ELB_ResultAngle[10];
	BOOL		bType;
	BOOL		m_bResultSummary;
	double		m_nNozzleCheckDistance;
	int			m_nRadius;						//KJH 2021-09-17 반지름 기록
	double		m_RminResult;					//Tkyuha 2022-03-10 도포 반대편 1/2 평균값 계산 추가
	double		m_RminAngle;					//Tkyuha 2022-03-10 도포 반대편 1/2 평균값 계산 추가
	double		m_RmaxResult;					//Tkyuha 2022-03-10 도포 반대편 1/2 평균값 계산 추가
	double		m_RmaxAngle;					//Tkyuha 2022-03-10 도포 반대편 1/2 평균값 계산 추가
	double		m_RCheckangle[5];				//KJH 2022-05-05 특수 각에서의 R값 계산
	double		m_BminResult;					//KJH 2022-05-24 BM 시작부터 약액 시작부까지 거리 계산 추가
	double		m_BminAngle;					//KJH 2022-05-24 BM 시작부터 약액 시작부까지 거리 계산 추가
	double		m_BmaxResult;					//KJH 2022-05-24 BM 시작부터 약액 시작부까지 거리 계산 추가
	double		m_BmaxAngle;					//KJH 2022-05-24 BM 시작부터 약액 시작부까지 거리 계산 추가
	double		m_BCheckangle[8];				//KJH 2022-06-06 특수 각에서의 B값 계산
	int         m_nLackOfInkAngleCount_Judge;	//HTK 2022-06-25 FDC용 LackofInkAngleCount 보고
	int			m_nLackOfInkAngleCount_Continue;//KJH 2022-07-28 Log용 LackofInkAngleContinue값
	int         m_nLackOfInkAngleCount_Total;	//KJH 2022-07-01 Log용 LackofInkAngleTotal값
	BOOL		bRJudge;
	BOOL		bBJudge;
	// KBJ 2022-10-10 (1.LackOfCink 플래그 로그추가, 2.CINK2 에서만 뿌린 도포두께 로그추가, 3.에러타입)
	bool		_bLackOfInkAlgleJudge[360];
	double		_dbCink2DopoWidth[360];
	long		_nErrorType;
};

struct _stInsp_ELB_ScanResult {
	std::vector<double> m_vTraceProfile[6];				// [0] 진행 방향 , [1] In방향 pixel , [2] in 방향 거리계산값 , [3] Out 방향 pixel , [4] Out 방향 거리계산값, [5] Wet Out Limit Insp Line Pixel
	cv::Point2d m_vELB_InMinMaxValue;					// x = min , y = max
	cv::Point2d m_vELB_InMinMaxPos;						// x = min , y = max
	cv::Point2d m_vELB_OutMinMaxValue;					// x = min , y = max
	cv::Point2d m_vELB_OutMinMaxPos;					// x = min , y = max
	cv::Point2d m_vELB_TotalMinMaxValue;				// x = min , y = max
	cv::Point2d m_vELB_SearchLine;						// x = x1  , y = x2
	cv::Point2d m_vELB_SearchLine_Insp;					// x = x1  , y = x2
	cv::Point2d m_ELB_FindPoint;						// x = Dopo Start Pixel , y = Dopo End Pixel
	cv::Point2d m_ELB_FindPoint_Index;					// x = Dopo Start vector index  , y = Dopo End vector index
	CRect m_ELB_InspRoi[2];								// [0] Insp Roi , [1] Dust
	sLine m_ELB_ResultLine[2];							// [0] Line 1	, [1] Line 2
	sLine m_ELB_ResultLine_Insp[2];						// [0] Line 1	, [1] Line 2
	BOOL m_bResultSummary;
	cv::Point2d m_ELB_Result_Center_XWidth_in;			//UT 도포 중심 도포 폭 검사용			X : Index / Y : Value
	cv::Point2d m_ELB_Result_Center_XWidth_out;			//UT 도포 중심 도포 폭 검사용			X : Index / Y : Value
	cv::Point2d m_ELB_Result_Point1_XWidth_in;			//UT 도포 Point1 도포 폭 검사용		X : Index / Y : Value
	cv::Point2d m_ELB_Result_Point1_XWidth_out;			//UT 도포 Point1 도포 폭 검사용		X : Index / Y : Value
	cv::Point2d m_ELB_Result_Point2_XWidth_in;			//UT 도포 Point2 도포 폭 검사용		X : Index / Y : Value
	cv::Point2d m_ELB_Result_Point2_XWidth_out;			//UT 도포 Point2 도포 폭 검사용		X : Index / Y : Value
	int nDefect;										//이물검사 Count
	string m_strPanelID;
};

struct _stInsp_WetOut_Result {
	double dMin_In;
	double dMax_In;
	double dMin_Out;
	double dMax_Out;
	double dMin_Total;
	double dMax_Total;
};

// YCS 2022-08-20 필름검사 데이터 자료구조 추가 
struct _stInsp_FilmLength_Result
{
	double dLX;
	double dLY;
	double dRX;
	double dRY;
};

struct _stRef_AirPresHighVolts_V {
	double dAir;
	double dHighVolt;
	double dWet_Out;
};

struct _stPatternSelect {
	int nCam;
	int nPattern;
	int nPoint;
	int nPos;
	int nIndex;
	int nPanel;
	int nSide;
	int nShape;
	int nJob;
	int nTemp[4];
};

struct _stSaveImageInfo {
	double dRef_PosX[4];
	double dRef_PosY[4];
	double dPosX[4];
	double dPosY[4];
	double dRevision[4];
	double dRevision_object[3][2];
	BOOL bJudge;
	int nFound[4];
	int nCam;
	int nPos;
	int nIndex;
	CDialog *pDlg;
	int nRetry;
	int bFinal;
	int nAlgorithm;
	double dScore[4];
	double dPatAngle[4];
	double dChipAngle;
	int nFindIndex[2];
	double dLineX[NUM_POS][4];
	double dLineY[NUM_POS][4];
	BOOL b4Align;
	BOOL bUsing1Cam;
	double dFindX[4];
	double dFindY[4];
	double dCenterX;
	double dCenterY;
	int nNumPos;
	BOOL bCrackNG;
	BOOL bSeparNG;
	BOOL bAngleNG;
	BOOL bLcheckNG;
	BOOL bDummyDirNG;
	double dbLcheckLength;
	double dbIncludedAngle;

	double mark_pos_x[4];
	double mark_pos_y[4];
	double mark_score[4];
	int mark_found[4];

	sLine  line[8];  		// 2개 대상물에대한 얼라인 사용
	BOOL   bFindline[8]; 	// 2개 대상물에대한 얼라인 사용
};

typedef struct _ToPacketData {		 //전송할 데이터를 가질 구조체		
	UINT  ud_sid;					 // 클라이언트 아이디	
	UINT  ud_cmd;					 // 명령어
	UINT  ud_cmdType;				 // 명령어 종류
	UINT  ud_cmdCount;				 // 전송 데이터 사이즈
	BYTE  ud_Fdata[MAX_TCP_SIZE];        // 12k내용	
}ToPacketData, *pToPacketData;

struct _ST_PLC_MODEL_DATA_ {
	double dbPanelAlignKeyDistance;				// 패널 얼라인에 사용되는 마크간 거리
	double dbPanelWidth;						// 패널 가로 길이
	double dbPanelHeight;						// 패널 세로 길이
	double dbPanelThicknessSpec;				// 마이크로 입실론 패널 두께 스펙
	double dbPanelThicknessMin;					// 마이크로 입실론 패널 두께 공차
	double dbPanelThicknessMax;					// 마이크로 입실론 패널 두께 공차
	double dbStageEpsilonDistance;				// 스테이지 ↔ 입실론 간 거리
	double dbSensorAlignKeyDistance;			// 센서 얼라인에 사용되는 마크간 거리
	double dbSensorWidth;						// 센서 가로 길이(패널 부착면 기준)	
	double dbSensorHeight;						// 센서 세로 길이(패널 부착면 기준)
	double dbSensorThicknessSpec;				// 마이크로 입실론 센서 두께 스펙
	double dbSensorThicknessMin;				// 마이크로 입실론 센서 두께 공차
	double dbSensorThicknessMax;				// 마이크로 입실론 센서 두께 공차
	double dbHeaderEpsilonDistance;				// 헤더 ↔ 입실론 간 거리
	double dbEpsilonSensorFpcbDirection;		// 0 : 0도, 1 : 90도, 2 : 180도, 3 : 270도
	double dbAttachAlignXSpec;					// 합착 검사 : 부착 스펙 X
	double dbAttachAlignXTolerence;				// 합착 검사 : 부착 스펙 X 공차
	double dbAttachAlignYSpec;					// 합착 검사 : 부착 스펙 Y
	double dbAttachAlignYTolerence;				// 합착 검사 : 부착 스펙 Y 공차
	double dbYmapEchoPen1Type_ScrewSpeed;		// Ymap 1액형 스크류 속도
	double dbYmapEchoPen1Type_SuckBackSpeed;	// Ymap 1액형 SuckBack 속도
	double dbYmapEchoPen1Type_SuckBackTime;		// Ymap 1액형 SuckBack 시간
	double dbYmapEchoPen12ype_ScrewSpeed;		// Ymap 2액형 스크류 속도
	double dbYmapEchoPen12ype_SuckBackSpeed;	// Ymap 2액형 SuckBack 속도
	double dbYmapEchoPen12ype_SuckBackTime;		// Ymap 2액형 SuckBack 시간
	double dbYmapEchoPen12ype_RatioA;			// Ymap 2액형 A 비율
	double dbYmapEchoPen12ype_RatioB;			// Ymap 2액형 B 비율
	double dbDottingEchoPen1_Pressure;			// 도팅 에코펜1 압력
	double dbDottingEchoPen2_Pressure;			// 도팅 에코펜2 압력
	double dbLmiSpec;
	double dbLmiSpecMin;
	double dbLmiSpecMax;
	double dbLmiTiltSpec;
};
struct _ELB_MODEL_PROCESS_ADDRES_ {
	int PLC_WD_PLC_MODEL_CURRENT_ID;
	int PLC_WD_PLC_MODEL_TARGET_ID;
	int PLC_WD_PLC_MODEL_PANEL_DATA;	
	
	int PLC_WD_PLC_MODEL_NACK_CODE;	
	
	int PLC_RB_PLC_MODEL_CREAT;
	int PLC_RB_PLC_MODEL_CHANGE;
	int PLC_RB_PLC_MODEL_MODIFY;
	int PLC_RB_PLC_MODEL_DELETE;
	
	int PLC_WB_PLC_MODEL_CREAT_ACK;
	int PLC_WB_PLC_MODEL_CHANGE_ACK;
	int PLC_WB_PLC_MODEL_MODIFY_ACK;
	int PLC_WB_PLC_MODEL_DELETE_ACK;
	int PLC_WB_PLC_MODEL_PROCESS_ABLE;
	int PLC_WB_PLC_MODEL_NACK;
};

struct _stDrawInfo {
	int MAIN_VIEW_PEN_THICKNESS;
	int CAM_VIEW_PEN_THICKNESS;
	int nMcView_Pen_Thickness;
	int MAIN_VIEW_PEN_LENGTH;
	int CAM_VIEW_PEN_LENGTH;
	int nMcView_Pen_Length;
	int MAIN_VIEW_FONT_WIDTH;
	int CAM_VIEW_FONT_WIDTH;
	int nMcView_Font_Width;
	int MAIN_VIEW_FONT_HEIGHT;
	int CAM_VIEW_FONT_HEIGHT;
	int nMcView_Font_Height;
};

class CJobResult {
public:
	BOOL each_Judge[MAX_CAMERA][NUM_POS];
	BOOL is_simulation;
	int rcs_judge;	// 0 : N/A, 1 : OK, 2 : NG
	double length_check[MAX_CAMERA][NUM_POS];
	BOOL length_check_judge[MAX_CAMERA][NUM_POS];
	double included_angle[MAX_CAMERA][NUM_POS];
	BOOL included_angle_judge[MAX_CAMERA][NUM_POS];
	double revision_data[NUM_POS][NUM_AXIS];

	BOOL is_find_pattern[MAX_CAMERA][NUM_POS];			// 패턴 찾은 결과
	BOOL is_find_pattern_end[MAX_CAMERA][NUM_POS];		// 패턴 찾는 동작이 완료 되었는지

	class PF_FILM_INSP pf_film_insp_data;

	void clear() {
		memset(each_Judge, 0, sizeof(each_Judge));
		memset(length_check, 0, sizeof(length_check));
		memset(length_check_judge, 0, sizeof(length_check_judge));
		memset(included_angle, 0, sizeof(included_angle));
		memset(included_angle_judge, 0, sizeof(included_angle_judge));
		memset(revision_data, 0, sizeof(revision_data));
		is_simulation = FALSE;
		rcs_judge = 0;
		memset(&pf_film_insp_data, 0, sizeof(pf_film_insp_data));
	}
};


#define MAX_LIGHT_INDEX		4
#define MAX_LIGHT_PER_CAMERA	8


class CGlassInfo {
public:
	CGlassInfo() {
		m_dbGlassWidth = m_dbGlassHeight = 120;
		m_dbFrameWidth = m_dbFrameHeight = 100;
		m_dbTabWidth = m_dbTabHeight = 100;
		m_dbInch = 5.0;
		m_nPanelRowCount = 2;
		m_nPanelColumnCount = 3;
		m_nModelPanelType = 0;
		m_nPanelCountInTray = m_nPanelRowCount * m_nPanelColumnCount;	
		/*m_dbFiducialMarkPitchX = 50.02;
		m_dbFiducialMarkPitchY = 8.8;*/
	}
private:
	int m_nPanelRowCount;		// Tray에 담겨 있는 Panel의 총 행수
	int m_nPanelColumnCount;	// Trau에 담겨 있는 Panel의 총 열수
	int m_nPanelCountInTray;	// Tray에 담겨 있는 총 Panel 수
	/*double m_dbFiducialMarkPitchX;
	double m_dbFiducialMarkPitchY;*/
	double m_dbGlassWidth;
	double m_dbGlassHeight;
	double m_dbFrameWidth;
	double m_dbFrameHeight;
	double m_dbTabWidth;
	double m_dbTabHeight;
	double m_dbInch;
	int m_nModelPanelType;
	int m_Reserve1[100];
	double m_Reserve2[100];
public:	
	//void setFiducialMarkPitchX(double pitchX) { m_dbFiducialMarkPitchX = pitchX; }
	void setPanelRowCount(int nCount) { m_nPanelRowCount = nCount; }
	void setPanelColumnCount(int nCount) { m_nPanelColumnCount = nCount; }
	void setPanelCountInTray(int nCount) { m_nPanelCountInTray = nCount; }
	void setGlassWidth(double dbWidth) { m_dbGlassWidth = dbWidth; }
	void setGlassHeight(double dbHeight) { m_dbGlassHeight = dbHeight; }
	void setFrameWidth(double dbWidth) { m_dbFrameWidth = dbWidth; }
	void setFrameHeight(double dbHeight) { m_dbFrameHeight = dbHeight; }
	void setModelPanelType(int type) {  m_nModelPanelType=type; }
	void setGlassSize(double dbWidth, double dbHeight) { m_dbGlassWidth = dbWidth;  m_dbGlassHeight = dbHeight; }
	void setTabWidth(double dbWidth) { m_dbTabWidth = dbWidth; }
	void setTabHeight(double dbHeight) { m_dbTabHeight = dbHeight; }
	void setTabSize(double dbWidth, double dbHeight) { m_dbTabWidth = dbWidth; m_dbTabHeight = dbHeight; }
	void setInch(double dbInch) { m_dbInch = dbInch; }
	/*
	double getFiducialMarkPitchX() { return m_dbFiducialMarkPitchX; }
	double getFiducialMarkPitchY() { return m_dbFiducialMarkPitchY; }*/
	int getPanelRowCount() { return m_nPanelRowCount; }
	int getPanelColumnCount() { return m_nPanelColumnCount; }
	int getPanelCountInTray() { return m_nPanelCountInTray; }
	double getGlassWidth() { return m_dbGlassWidth; }
	double getGlassHeight() { return m_dbGlassHeight; }
	double getFrameWidth() { return m_dbFrameWidth; }
	double getFrameHeight() { return m_dbFrameHeight; }
	int getModelPanelType() { return m_nModelPanelType; }
	double getTabWidth() { return m_dbTabWidth; }
	double getTabHeight() { return m_dbTabHeight; }
	double getInch() { return m_dbInch; }
};

class CLightInfo {
public:
	CLightInfo() {
		for (int nCam = 0; nCam < MAX_CAMERA; nCam++)
		{
			for (int light = 0; light < MAX_LIGHT_PER_CAMERA; light++)
			{
				for (int nIndex = 0; nIndex < MAX_LIGHT_INDEX; nIndex++)
					m_dbLightBright[nCam][light][nIndex] = 0;
			}
		}
	}
private:

	int m_dbLightBright[MAX_CAMERA][MAX_LIGHT_PER_CAMERA][MAX_LIGHT_INDEX];
	// dh.jung 2021-05-26 add restore light bright
	int m_dbLightBrightRestore[MAX_CAMERA][MAX_LIGHT_PER_CAMERA][MAX_LIGHT_INDEX];		
	BOOL bSaveLightBrightFlage;
	// end
public:

	void setLightBright(int nCam, int nAxis, int nIndex, int dbValue) { m_dbLightBright[nCam][nAxis][nIndex] = dbValue; }
	int getLightBright(int nCam, int nAxis, int nIndex) { return m_dbLightBright[nCam][nAxis][nIndex]; }

	// dh.jung 2021-05-26 add restore light bright
	void lightBrightSaveFlag(BOOL bSaveFlag = FALSE) { bSaveLightBrightFlage = bSaveFlag; };
	BOOL getlightBrightSaveFlag() { return bSaveLightBrightFlage; };
	void setLightBrightRestore(int nCam, int nAxis, int nIndex, int dbValue) { m_dbLightBrightRestore[nCam][nAxis][nIndex] = dbValue; }
	int getLightBrightRestore(int nCam, int nAxis, int nIndex) { return m_dbLightBrightRestore[nCam][nAxis][nIndex]; }
	void loadLightBrightRestore(int nCam, int nAxis, int nIndex) { m_dbLightBright[nCam][nAxis][nIndex] = m_dbLightBrightRestore[nCam][nAxis][nIndex]; }
	void saveLightBrightRestore(int nCam, int nAxis, int nIndex) { m_dbLightBrightRestore[nCam][nAxis][nIndex] = m_dbLightBright[nCam][nAxis][nIndex]; }
	// end

};

class CMachineInfo {
public:
	CMachineInfo() {
		for (int nCam = 0; nCam < MAX_CAMERA; nCam++)
		{
			m_nExposureTime[nCam] = 10000;
			m_nGain[nCam] = 330;

			for (int nPos = 0; nPos < NUM_POS; nPos++)
			{
				m_dbRotateX[nCam][nPos] = 0;
				m_dbRotateY[nCam][nPos] = 0;
			}

			m_nAlignOrginPos[nCam] = 4; //_CENTER_CENTER_X50_Y50
		}

		m_nAlignmentType = 0;
	
		for (int nAxis = 0; nAxis < NUM_AXIS; nAxis++)
		{
			m_dbPrealignOffSet[nAxis] = 0;
		}
		m_dbRevisionLimit[0] = 3.0;	// 3mm
		m_dbRevisionLimit[1] = 3.0;	// 3mm
		m_dbRevisionLimit[2] = 1.5;	// degree 1.5

		m_nCalibrationType = 0;		
		m_dbDispenserOffsetT = 0.0;

		m_dbSamplingAlignOffsetX = 0.0;
		m_dbSamplingAlignOffsetY = 0.0;

		m_nCalibCountX = 10;
		m_nCalibCountY = 10;
		m_dbCalibRangeX = 12.0;
		m_dbCalibRangeY = 10.0;
		m_dbRotateRange = 1.5;
		m_nThetaReference = 0;
		m_dbRotateCount = 2;
		m_dbRotateStartPos = 0;
		m_dbRotateEndPos = 0;

		m_dbLCheckRefDist = 0.0;
		m_nThetaCalibCount = 10;

		memset(m_dRevisionOffset, 0, sizeof(m_dRevisionOffset));
		memset(m_dLCheckOffset, 0, sizeof(m_dLCheckOffset));
		memset(m_dInspOffset, 0, sizeof(m_dInspOffset));
		
	}

	double m_dRevisionOffset[NUM_AXIS];
	double m_dbRevisionLimit[NUM_AXIS];
	double m_dbPrealignOffSet[NUM_AXIS];
	int m_nAlignmentType;
	int m_nThetaCalibCount;
	double m_dbLCheckRefDist;
	int m_nThetaReference;
	double m_dbRotateRange;
	double m_dbRotateStartPos;
	double m_dbRotateEndPos;
	int m_dbRotateCount;
	int m_nCalibCountX;
	int m_nCalibCountY;
	double m_dbCalibRangeX;
	double m_dbCalibRangeY;
	double m_dbSamplingAlignOffsetX;
	double m_dbSamplingAlignOffsetY;
	double m_dbDispenserOffsetT;
	int m_nMethodCalcAngle;
	int m_nCalibrationType;
	int m_nExposureTime[MAX_CAMERA];
	int m_nGain[MAX_CAMERA];
	int m_nAlignOrginPos[MAX_CAMERA];
	double m_dbRotateX[MAX_CAMERA][NUM_POS];
	double m_dbRotateY[MAX_CAMERA][NUM_POS];
	int m_reserve1[100];
	double m_reserve2[100];
	double m_dThetaSlope[2];
	double m_dThetaOffset[2];

	double m_dLCheckOffset[NUM_POS];
	double m_dInspOffset[MAX_CAMERA][NUM_POS][2];	// [X/Y]

	BOOL m_bUseImageTheta;
public:
	BOOL getUseImageTheta() { return m_bUseImageTheta; }

	double getRevisionOffset(int nAxis) { return m_dRevisionOffset[nAxis]; }
	void setRevisionOffset(int nAxis, double offset) { m_dRevisionOffset[nAxis] = offset; }
	void setRevisionLimit(int nAxis, double dbLimit) { m_dbRevisionLimit[nAxis] = dbLimit; }
	void setPrealignOffSet(int nAxis, double dbValue) { m_dbPrealignOffSet[nAxis] = dbValue; }
	double getPrealignOffSet(int nAxis) { return m_dbPrealignOffSet[nAxis]; }
	double getRevisionLimit(int nAxis) { return m_dbRevisionLimit[nAxis]; }

	double get_lcheck_offset(int pos) { return m_dLCheckOffset[pos]; }
	void set_lcheck_offset(int pos, double off) {	m_dLCheckOffset[pos] = off; }

	double get_insp_offset(int cam, int pos, int index) { return m_dInspOffset[cam][pos][index]; }
	void set_insp_offset(int cam, int pos, int index, double off) { m_dInspOffset[cam][pos][index] = off; }

	int getCalibrationType() { return m_nCalibrationType; }
	void setCalibrationType(int nType) { m_nCalibrationType = nType; }
	double getLCheckRefDist() { return m_dbLCheckRefDist; }
	int getThetaCalibCount() { return m_nThetaCalibCount; }
	int getCalibrationCountX() { return m_nCalibCountX; }
	int getCalibrationCountY() { return m_nCalibCountY; }
	void setCalibrationCountX(int count) { m_nCalibCountX = count; }
	void setCalibrationCountY(int count) { m_nCalibCountY = count; }
	void setRotateRange(double range) { m_dbRotateRange = range; }
	void setRotateCount(int count) { m_dbRotateCount = count; }
	void setRotateStartPos(double range) { m_dbRotateStartPos = range; }
	void setRotateEndPos(double range) { m_dbRotateEndPos = range; }

	void setThetaCalibCount(int nCount) { m_nThetaCalibCount = nCount; }
	void setLCheckRefDist(double dist) { m_dbLCheckRefDist = dist; }
	void setCalibrationRangeX(double range) { m_dbCalibRangeX = range; }
	void setCalibrationRangeY(double range) { m_dbCalibRangeY = range; }
	void setAlignOrginPos(int nCam, int pos) {	m_nAlignOrginPos[nCam] = pos; }
	void setUseImageTheta(BOOL bUse) { m_bUseImageTheta = bUse; }

	double getCalibrationRangeX() { return m_dbCalibRangeX; }
	double getCalibrationRangeY() { return m_dbCalibRangeY; }
	double getRotateRange() { return m_dbRotateRange; }
	int getRotateCount() { return m_dbRotateCount; }
	int getThetaReference() { return m_nThetaReference; }
	double getRotateStartPos() { return m_dbRotateStartPos; }
	double getRotateEndPos() { return m_dbRotateEndPos; }

	void setThetaSlope(int pos, double v) { m_dThetaSlope[pos] = v; }
	double getThetaSlope(int pos) { return m_dThetaSlope[pos]; }
	void setThetaOffset(int pos, double v) { m_dThetaOffset[pos] = v; }
	double getThetaOffset(int pos) { return m_dThetaOffset[pos]; }

	double getSamplingAlignOffsetX() { return m_dbSamplingAlignOffsetX; }
	double getSamplingAlignOffsetY() { return m_dbSamplingAlignOffsetY; }
	void setSamplingAlignOffsetX(double offset) { m_dbSamplingAlignOffsetX = offset; }
	void setSamplingAlignOffsetY(double offset) { m_dbSamplingAlignOffsetY = offset; }
	void setThetaReference(int nRef) { m_nThetaReference = nRef; }

	void setDispenserOffsetT(double offsetT) { m_dbDispenserOffsetT = offsetT; }
	void setAlignmentType(int nType) { m_nAlignmentType = nType; }
	void setExposureTime(int nCam, int nValue) { m_nExposureTime[nCam] = nValue; }
	void setGain(int nCam, int nValue) { m_nGain[nCam] = nValue; }
	void setRotateCenterX(int nCam, int nPos, double dbValue) { m_dbRotateX[nCam][nPos] = dbValue; }
	void setRotateCenterY(int nCam, int nPos, double dbValue) { m_dbRotateY[nCam][nPos] = dbValue; }

	double getDispenserOffsetT() { return m_dbDispenserOffsetT; }
	int getAlignmentType() { return m_nAlignmentType; }
	int getExposureTime(int nCam) { return m_nExposureTime[nCam]; }
	int getGain(int nCam) { return m_nGain[nCam]; }
	double getRotateCenterX(int nCam, int nPos) { return m_dbRotateX[nCam][nPos]; }
	double getRotateCenterY(int nCam, int nPos) { return m_dbRotateY[nCam][nPos]; }
	int getAlignOrginPos(int nCam) { return m_nAlignOrginPos[nCam]; }
};

class CAlignInfo {
public:
	CAlignInfo() {
		for (int nPoint = 0; nPoint < NUM_ALIGN_POINTS; nPoint++)
		{
			m_dbAlignSpec[nPoint] = 6.15;
			m_dbAlignSpecMin[nPoint] = 0.15;
			m_dbAlignSpecMinMax[nPoint] = 0.1;
			m_dbAlignSpecMax[nPoint] = 0.15;
			m_dbAlignOffSet[nPoint] = 0.0;
			m_bUseAlignMeasure[nPoint] = FALSE;

#ifndef JOB_INFO
			m_bEnableLCheck[nPoint] = FALSE;
			m_dLCheckLimit[nPoint] = 0.1;
			m_nAlignmentTargetDir[nPoint] = 0;
			m_bEnableAlignBlackWhite[nPoint] = FALSE;
			m_nAlignmentTarget[nPoint] = 0;
			m_nMarkFindMethod[nPoint] = 0;
#endif
		}

#ifndef JOB_INFO
#else
		m_bEnableLCheck = FALSE;
		m_dLCheckTor = 0.1;
		m_dLCheckSpecX = 100.0;
		m_dLCheckSpecY = 100.0;
		m_nAlignmentTargetDir = 0;
		m_bEnableAlignBlackWhite = FALSE;
		m_nAlignmentTarget = 0;
		m_nMarkFindMethod = 0;
#endif
		m_nAngleCalcMethod = 0;
		m_nPolEdgeFindMethod = 0;
		m_bMarkReverseFind = FALSE;
		m_bEnableAlignMeasure = FALSE;
		m_bEnableReView = FALSE;		
		m_bEnableAlignLeftRightDiffJudge = FALSE;
		m_dbAlignLeftRightDiffSpec = 0.1;
		m_bUse4PointAlign = FALSE;
		m_bUsing1Cam = FALSE;		
		m_nInspMethod = 0;
		m_bRealTimeTrace = TRUE;
		m_nSearchEdgePolarity = 0;
		m_nInspResultCount = 0;
		m_iGrabDelay = 0;

		for (int i = 0; i < MAX_CAMERA; i++)
		{
			for (int n = 0; n < NUM_POS; n++)
			{
				for (int k = 0; k < MAX_INDEX; k++)
				{
					m_nRemoveNoiseSize[i][n][k] = 10;
					m_nSobelThreshold[i][n][k] = 10;
				}
			}
#ifndef JOB_INFO
			m_bRevisionReverse[i] = FALSE;
			m_bRevisionReverseX[i] = FALSE;
			m_bRevisionReverseY[i] = FALSE;
			m_bRevisionReverseT[i] = FALSE;
			m_bRevisionReverseXY[i] = FALSE;
			m_bEnableAlignSpecOutJudge[i] = FALSE;
			m_nInspectionMethodType[i] = 0;
			m_bUseFixtureMark[i] = FALSE;
			m_nExistPanelGray[i] = 50;
			m_nExistDummyGray[i] = 200;
			m_dbDistanceInspSpec[i] = 1.2;
			m_dbDistanceInspSpecUSL[i] = 0.5;
			m_nInspectionSubMethodType[i] = 0;
			m_dbFiducialMarkPitchX[i] = 50.02;
			m_dbFiducialMarkPitchY[i] = 8.8;
			m_dbFiducialSecondMarkPitchY[i] = 0;
			m_dbIncludedAngle[i] = 90.0;
			m_dbIncludedAngleLimit[i] = 1.0;
			m_bUseCrackInsp[i] = FALSE;
			m_dLCheckScale[i] = 1.0;
#endif	
		}

#ifndef JOB_INFO
#else
		m_bRevisionReverse = FALSE;
		m_bRevisionReverseX = FALSE;
		m_bRevisionReverseY = FALSE;
		m_bRevisionReverseT = FALSE;
		m_bRevisionReverseXY = FALSE;
		m_bUseRevisionReset = FALSE;
		m_bEnableAlignSpecOutJudge = FALSE;
		m_nEnableAlignSpecOutJudgeCount = 0;
		m_nInspectionMethodType = 0;
		m_bUseFixtureMark = FALSE;
		m_bUseReferenceMark = FALSE;
		m_nInspectionSubMethodType = 0;
		m_dbFiducialMarkPitchX = 50.02;
		m_dbFiducialMarkPitchY = 8.8;
		m_dbFiducialSecondMarkPitchY = 0;
		m_dbIncludedAngle = 90.0;
		m_dbIncludedAngleLimit = 1.0;
		m_bUseCrackInsp = FALSE;
		m_dLCheckScale = 1.0;
#endif
		m_nChipRansacType = 0;
		m_nChipRansacDirection = 0;
		m_nInspRangeX=200;
		m_nInspRangeY=200;
		m_nInspRangeLength=14.0;
		m_nSearchDirection = 0;
		
		for (int nCam = 0; nCam < MAX_CAMERA; nCam++)
		{
			for (int nPos = 0; nPos < NUM_POS; nPos++)
			{
				m_nCornerType[nCam][nPos] = 0;
				m_nEdgePolarity[nCam][nPos] = 0;
				m_nEdgeDirection[nCam][nPos] = 0;
			}
		}
	}
private:
	int m_nSearchArea[MAX_CAMERA][NUM_POS][MAX_INDEX];
	int m_nProjectionArea[MAX_CAMERA][NUM_POS][MAX_INDEX];
	int m_nRemoveNoiseSize[MAX_CAMERA][NUM_POS][MAX_INDEX];
	int m_nSobelThreshold[MAX_CAMERA][NUM_POS][MAX_INDEX];
	int m_nCornerType[MAX_CAMERA][NUM_POS];
	int m_nEdgePolarity[MAX_CAMERA][NUM_POS];
	int m_nEdgeDirection[MAX_CAMERA][NUM_POS];
	int m_nPreProcess[MAX_CAMERA][NUM_POS];
	int m_nEdgeProcess[MAX_CAMERA][NUM_POS];

	
	double m_dbHalconMinContrast[MAX_CAMERA];
	double m_dbHalconHighContrast[MAX_CAMERA];

#ifndef JOB_INFO
	BOOL m_bRevisionReverse[MAX_CAMERA];
	BOOL m_bRevisionReverseX[MAX_CAMERA];
	BOOL m_bRevisionReverseY[MAX_CAMERA];
	BOOL m_bRevisionReverseT[MAX_CAMERA];
	BOOL m_bRevisionReverseXY[MAX_CAMERA];
	double m_dLCheckScale[MAX_CAMERA];
	double m_dbFiducialMarkPitchX[MAX_CAMERA];
	double m_dbFiducialMarkPitchY[MAX_CAMERA];
	double m_dbFiducialSecondMarkPitchY[MAX_CAMERA];
#else
	BOOL m_bRevisionReverse;
	BOOL m_bRevisionReverseX;
	BOOL m_bRevisionReverseY;
	BOOL m_bRevisionReverseT;
	BOOL m_bRevisionReverseXY;
	BOOL m_bUseRevisionReset;

	double m_dLCheckScale;
	double m_dbFiducialMarkPitchX;
	double m_dbFiducialMarkPitchY;
	double m_dbFiducialSecondMarkPitchY;
	double m_dbDistanceInspHoleMetalSpec;
#endif

	double m_dbAlignLeftRightDiffSpec;
	BOOL m_bEnablePeelerInspection;
	BOOL m_bEnableAlignLeftRightDiffJudge;
	BOOL m_bEnableReView;
	BOOL m_bEnableAlignMeasure;
	BOOL m_bIsAlignReversePanel;
	BOOL m_bUseAlignImage;
	BOOL m_bUseDummyPortAlign;	// 20.03.03

	int m_nAlignmentTargetDir;
	double m_dLCheckTor;
	double m_dLCheckSpecX;
	double m_dLCheckSpecY;
	BOOL m_bEnableLCheck;
	int m_nLCheckReference;
	int m_nAngleCalcMethod;		// 각도 계산 방법
	int m_nMarkFindMethod;		// 마크 찾는 방식 : 0 : Matching, 1 : Edge Find;
	int m_nAlignmentTarget;		// Align 정렬 기준	; Fiducial Mark / Pol Edge
	BOOL m_bEnableAlignBlackWhite;
	BOOL m_bEnableAlignSpecOutJudge;
	int m_nEnableAlignSpecOutJudgeCount;
	int m_nInspectionMethodType;
	int m_nInspectionSubMethodType;
	int m_nInspectionSubDirctionType;

	BOOL m_bUseFixtureMark; //210110
	BOOL m_bUseReferenceMark;

	double m_dbDistanceInspSpecLSL[NUM_ALIGN_POINTS];
	double m_dbDistanceInspSubSpec[NUM_ALIGN_POINTS];
	double m_dbDistanceInspSpecY[NUM_ALIGN_POINTS];
	double m_dbDistanceInspSpecUSL[NUM_ALIGN_POINTS];
	double m_dbDistanceInspSubSpecTolerance[NUM_ALIGN_POINTS];
	double m_dbDistanceInspSpecYMinMax[NUM_ALIGN_POINTS];
	double m_dbDistanceInspSubInterval;
	double m_dbDistanceInspSubIntervalTolerance;

	BOOL m_bUseCrackInsp;
	BOOL m_bUseIncludedAngle;
	BOOL m_bUseBoxInsp;
	BOOL m_bUseDummySeparInsp;
	BOOL m_bUseMultiplexRoi; // 20.03.26
	double m_dbIncludedAngle;
	double m_dbIncludedAngleLimit;
	BOOL m_bUseDummyCornerInsp;
	BOOL m_bUseDoubleDispensingEnable;
	BOOL m_bLineOverflowDispMode;
	BOOL m_bCircle_Cink1_Cink2MergeMode;
	BOOL m_bLine_InspWithTraceResultMode;
	int m_nDummyCornerInspCam;
	int m_nDummyCornerFilter;
	double m_dbDummyIncludedAngle;
	double m_dbDummyIncludedAngleLimit;

	BOOL m_bUse4PointAlign;
	BOOL m_bUsing1Cam;			// 2Cam Align 에서만 임시로 사용. 1Cam으로 사용 할 수도 있어서
	BOOL m_bEnable2ShotAlign;
	int m_nPolEdgeFindMethod;
	
	BOOL m_bMarkReverseFind;
	double m_dbAlignSpec[NUM_ALIGN_POINTS];		// 2Point만 측정 할 것 같으나 혹시 몰라서...
	double m_dbAlignSpecMinMax[NUM_ALIGN_POINTS];
	double m_dbAlignSpecMin[NUM_ALIGN_POINTS];
	double m_dbAlignSpecMax[NUM_ALIGN_POINTS];
	double m_dbAlignOffSet[NUM_ALIGN_POINTS];
	BOOL m_bUseAlignMeasure[NUM_ALIGN_POINTS];
	int m_Reserve1[100];
	double m_Reserve2[100];
	int m_nChipRansacType;
	int m_nInspMethod;
	int m_nChipRansacDirection;			// New ChipRansac 사용시
	double m_nInspRangeX; 
	double m_nInspRangeY; 
	int m_nSearchEdgePolarity;
	int m_nSearchDirection;
	int m_iGrabDelay;

	int m_nInspResultCount;
	double m_nInspRangeLength;
	double m_nInspTraceInterval;
	double m_nInspInHoleSize;
	double m_nInspCamHoleSize; 
	double m_nInspSteelHoleSize;
	BOOL m_bRealTimeTrace;
	BOOL m_bUseFixTrace;
	BOOL m_bCycleTrace;
	BOOL m_bMetalTrace;
	BOOL m_bReverseOrder;
	BOOL m_bUseSubInspAlgorithm;
	BOOL m_bMetalTraceFirst;
	BOOL m_bAutoSaveAVI;
	BOOL m_bPNBase;
	BOOL m_bFilmReversePosition;
public:
	void setEnable2ShotAlign(BOOL bEnable) { m_bEnable2ShotAlign = bEnable; }
	void setUsing1Cam(BOOL bUse) { m_bUsing1Cam = bUse; }
	void setUse4PointAlign(BOOL bUse) { m_bUse4PointAlign = bUse; }

	void setLCheckTor(double limit) { m_dLCheckTor = limit; }
	void setLCheckSpecX(double spec) { m_dLCheckSpecX = spec; }
	void setLCheckSpecY(double spec) { m_dLCheckSpecY = spec; }

	void setLCheckReference(int ref) { m_nLCheckReference = ref;  }
	void setEnableLCheck(BOOL bSet) { m_bEnableLCheck = bSet; }
	void setAlignmentTarget(int nTarget) { m_nAlignmentTarget = nTarget; }
	void setAlignmentTargetDir(int nTarget) { m_nAlignmentTargetDir = nTarget; }
	void setMarkFindMethod(int nMethod) { m_nMarkFindMethod = nMethod; }
	void setAngleCalcMethod(int nMethod) { m_nAngleCalcMethod = nMethod; }
	
	void setEnableAlignBlackWhiteSelect(BOOL eEnable) { m_bEnableAlignBlackWhite = eEnable; }
	void setLCheckScale(double dist) { m_dLCheckScale = dist; }
	void setEnableAlignMeasureSpecOutJudge(BOOL eEnable) { m_bEnableAlignSpecOutJudge = eEnable; }
	void setEnableAlignMeasureSpecOutJudgeCount(int nCount) { m_nEnableAlignSpecOutJudgeCount = nCount; }

	void setAlignLeftRightDiffSpec(double dbSpec) { m_dbAlignLeftRightDiffSpec = dbSpec; }
	void setEnablePeelerInspection(BOOL eEnable) { m_bEnablePeelerInspection = eEnable; }
	void setEnableAlignLeftRightDiffJudge(BOOL eEnable) { m_bEnableAlignLeftRightDiffJudge = eEnable; }
	void setEnableReView(BOOL eEnable) { m_bEnableReView = eEnable; }
	void setEnableAlignMeasure(BOOL bEnable) { m_bEnableAlignMeasure = bEnable; }
	void setChipRansacType(int nType) { m_nChipRansacType = nType; }
	void setPolEdgeFindMethod(int method) { m_nPolEdgeFindMethod = method; }
	
	void setAlignSpec(int nPoint, double dbValue) { m_dbAlignSpec[nPoint] = dbValue; }
	void setAlignSpecMin(int nPoint, double dbValue) { m_dbAlignSpecMin[nPoint] = dbValue; }
	void setAlignSpecMinMax(int nPoint, double dbValue) { m_dbAlignSpecMinMax[nPoint] = dbValue; }
	void setAlignSpecMax(int nPoint, double dbValue) { m_dbAlignSpecMax[nPoint] = dbValue; }
	void setAlignOffset(int nPoint, double dbValue) { m_dbAlignOffSet[nPoint] = dbValue; }
	void setUseAlignMeasure(int nPoint, BOOL bUse) { m_bUseAlignMeasure[nPoint] = bUse; }
	void setMarkReverseFind(BOOL nMethod) { m_bMarkReverseFind = nMethod; }
	void setEnableAlignReverseSelect(BOOL eEnable) { m_bIsAlignReversePanel = eEnable; }
	void setInspMethod(int nMethod) { m_nInspMethod = nMethod; }
	void setEnableUseAlignImage(BOOL bEnable) { m_bUseAlignImage = bEnable; }
	void setChipRansacDirection(int nDirection) { m_nChipRansacDirection = nDirection; }
	void setUseDummyPortAlign(BOOL bUse) { m_bUseDummyPortAlign = bUse; }	// 20.03.03	
	void setCornerType(int nCam, int nPos, int nValue) { m_nCornerType[nCam][nPos] = nValue; }
	void setEdgePolarity(int nCam, int nPos, int nValue) { m_nEdgePolarity[nCam][nPos] = nValue; }
	void setEdgeDirection(int nCam, int nPos, int nValue) { m_nEdgeDirection[nCam][nPos] = nValue; }
	void setPreProcess(int nCam, int nPos, int nPre) { m_nPreProcess[nCam][nPos] = nPre; }
	void setEdgeProcess(int nCam, int nPos, int nEdge) { m_nEdgeProcess[nCam][nPos] = nEdge; }
	void setSearchArea(int nCam, int nPos, int nIndex, int nArea) { m_nSearchArea[nCam][nPos][nIndex] = nArea; }
	void setProjectionArea(int nCam, int nPos, int nIndex, int nArea) { m_nProjectionArea[nCam][nPos][nIndex] = nArea; }
	void setSobelTheshold(int nCam, int nPos, int nIndex, int nSize) { m_nSobelThreshold[nCam][nPos][nIndex] = nSize; }
	void setRemoveNoiseSize(int nCam, int nPos, int nIndex, int nSize) { m_nRemoveNoiseSize[nCam][nPos][nIndex] = nSize; }

	void setRevisionReverse(BOOL bReverse) { m_bRevisionReverse = bReverse; }
	void setRevisionReverseX(BOOL bReverse) { m_bRevisionReverseX = bReverse; }	//210110
	void setRevisionReverseY(BOOL bReverse) { m_bRevisionReverseY = bReverse; }	//210110
	void setRevisionReverseT(BOOL bReverse) { m_bRevisionReverseT = bReverse; }	//210110
	void setRevisionReverseXY(BOOL bReverse) { m_bRevisionReverseXY = bReverse; }	//210110	
	BOOL getUseRevisionReset() { return m_bUseRevisionReset; }
	void setUseRevisionReset(BOOL bValue) { m_bUseRevisionReset = bValue; }

	void setUseFixtureMark(BOOL bUse) { m_bUseFixtureMark = bUse; }	//210110
	void setUseReferenceMark(BOOL bUse) { m_bUseReferenceMark = bUse; }
	void setInspectionMethod(int type) { m_nInspectionMethodType = type; }
	void setInspectionSubMethod(int type) { m_nInspectionSubMethodType = type; }
	void setInspectionSubDir(int type) { m_nInspectionSubDirctionType = type; }
	void setInspRangeX(double rx) { m_nInspRangeX = rx; }
	void setInspRangeY(double ry) { m_nInspRangeY = ry; }
	void setSearchEdgePolarity(int dir) {		m_nSearchEdgePolarity = dir;	}
	void setSearchSearchDir(int dir) { m_nSearchDirection = dir; }
	void setInspRangeLength(double length) { m_nInspRangeLength = length; }
	void setInspTraceInterval(double length) { m_nInspTraceInterval = length; }
	void setInspResultCount(int n) { m_nInspResultCount = n; }
	void setInspInHoleSize(double length) { m_nInspInHoleSize = length;}
	void setInspCamHoleSize(double length) { m_nInspCamHoleSize = length; }
	void setInspSteelHoleSize(double length) { m_nInspSteelHoleSize = length; }
	void setUseRealTimeTrace(BOOL bUse) { m_bRealTimeTrace = bUse; }
	void setUseUseFixTrace(BOOL bUse) { m_bUseFixTrace = bUse; }
	void setUseCycleTrace(BOOL bUse) { m_bCycleTrace = bUse; }
	void setUseMetalTrace(BOOL bUse) { m_bMetalTrace = bUse; }
	void setUseReverseOrder(BOOL bUse) { m_bReverseOrder = bUse; }
	void setUseSubInspAlgorithm(BOOL bUse) { m_bUseSubInspAlgorithm = bUse; }
	void setUseMetalTraceFirst(BOOL bUse) { m_bMetalTraceFirst = bUse; }
	void setUseAutoSaveAVI(BOOL bUse) { m_bAutoSaveAVI = bUse; }
	void setUsePNBase(BOOL bUse) { m_bPNBase = bUse; }
	void setGrabDelay(int delayv) { m_iGrabDelay= delayv; }
	
	void setDistanceInspSpecLSL(int pos, double spec) { m_dbDistanceInspSpecLSL[pos] = spec; }
	void setDistanceInspSubSpec(int pos, double spec) { m_dbDistanceInspSubSpec[pos] = spec; }
	void setDistanceInspSpecY(int pos, double spec) { m_dbDistanceInspSpecY[pos] = spec; }
	void setDistanceInspSpecUSL(int pos, double specminmax) { m_dbDistanceInspSpecUSL[pos] = specminmax; }
	void setDistanceInspSubSpecXTolerance(int pos, double specminmax) { m_dbDistanceInspSubSpecTolerance[pos] = specminmax; }
	void setDistanceInspSpecYMinMax(int pos, double specminmax) { m_dbDistanceInspSpecYMinMax[pos] = specminmax; }
	void setInspectionSubInterval(double specminmax) { m_dbDistanceInspSubInterval = specminmax; }
	void setInspectionSubIntervalTolerance(double specminmax) { m_dbDistanceInspSubIntervalTolerance = specminmax; }
	void setUseCrackInsp(BOOL bSet) { m_bUseCrackInsp = bSet; }
	void setUseIncludedAngle(BOOL bSet) { m_bUseIncludedAngle = bSet; }
	void setIncludedAngle(double angle) { m_dbIncludedAngle = angle; }
	void setIncludedAngleLimit(double angle) { m_dbIncludedAngleLimit = angle; }
	void setUseDummyCornerInsp(BOOL bSet) { m_bUseDummyCornerInsp = bSet; }
	void setUseDoubleDispensingMode(BOOL bSet) { m_bUseDoubleDispensingEnable = bSet; }
	void setLineOverflowDispMode(BOOL bSet) { m_bLineOverflowDispMode = bSet; }
	void setCircle_Cink1_Cink2MergeMode(BOOL bSet) { m_bCircle_Cink1_Cink2MergeMode = bSet; }	
	void setLine_InspWithTraceResultMode(BOOL bSet) { m_bLine_InspWithTraceResultMode = bSet; }	
	void setDummyCornerInspCam(int nSCam) { m_nDummyCornerInspCam = nSCam; }
	void setDummyCornerFilter(int nFilter) { m_nDummyCornerFilter = nFilter; }
	void setDummyCornerIncludedAngle(double angle) { m_dbDummyIncludedAngle = angle; }
	void setDummyCornerIncludedAngleLimit(double angle) { m_dbDummyIncludedAngleLimit = angle; }
	void setUseBoxInsp(BOOL bUse) { m_bUseBoxInsp = bUse; }
	void setUseDummySeparInsp(BOOL bUse) { m_bUseDummySeparInsp = bUse; }
	void setUseMultiplexRoi(BOOL bUse) { m_bUseMultiplexRoi = bUse; }	// 20.03.26
	void setFiducialMarkPitchX(double pitchX) { m_dbFiducialMarkPitchX = pitchX; }
	void setFiducialSecondMarkPitchY(double pitchy) { m_dbFiducialSecondMarkPitchY = pitchy; }
	void setDistanceInspHoleMetal(double spec) { m_dbDistanceInspHoleMetalSpec = spec; }

	void setHalconMinContrast(int nCam, double contrast) { m_dbHalconMinContrast[nCam] = contrast; }
	void setHalconHighContrast(int nCam, double contrast) { m_dbHalconHighContrast[nCam] = contrast; }
	

	BOOL getEnable2ShotAlign() { return m_bEnable2ShotAlign; }
	BOOL getUsing1Cam() { return m_bUsing1Cam; }
	BOOL getUse4PointAlign() { return m_bUse4PointAlign; }
	
	double getAlignLeftRightDiffSpec() { return m_dbAlignLeftRightDiffSpec; }
	BOOL getEnablePeelerInspection() { return m_bEnablePeelerInspection; }
	BOOL getEnableAlignLeftRightDiffJudge() { return m_bEnableAlignLeftRightDiffJudge; }
	BOOL getEnableReView() { return m_bEnableReView; }
	BOOL getEnableAlignMeasure() { return m_bEnableAlignMeasure; }
	int getChipRansacType() { return m_nChipRansacType; }
	int getPolEdgeFindMethod() { return m_nPolEdgeFindMethod; }
	double getAlignSpec(int nPoint) { return m_dbAlignSpec[nPoint]; }
	double getAlignSpecMin(int nPoint) { return m_dbAlignSpecMin[nPoint]; }
	double getAlignSpecMinMax(int nPoint) { return m_dbAlignSpecMinMax[nPoint]; }
	double getAlignSpecMax(int nPoint) { return m_dbAlignSpecMax[nPoint]; }
	double getAlignOffset(int nPoint) { return m_dbAlignOffSet[nPoint]; }
	BOOL getUseAlignMeasure(int nPoint) { return m_bUseAlignMeasure[nPoint]; }
	
	int getLCheckReference() { return m_nLCheckReference; }
	double getLCheckTor() { return m_dLCheckTor; }
	double getLCheckSpecX() { return m_dLCheckSpecX; }
	double getLCheckSpecY() { return m_dLCheckSpecY; }
	int getGrabDelay() { return m_iGrabDelay; }
	
	int getAngleCalcMethod() { return m_nAngleCalcMethod; }
	BOOL getEnableLCheck() { return m_bEnableLCheck; }
	int getAlignmentTarget() { return m_nAlignmentTarget; }
	int getAlignmentTargetDir() { return m_nAlignmentTargetDir; }
	int getMarkFindMethod() { return m_nMarkFindMethod; }
	BOOL getEnableAlignBlackWhiteSelect() { return m_bEnableAlignBlackWhite; }
	double getLCheckScale() { return m_dLCheckScale; }
	BOOL getEnableAlignMeasureSpecOutJudge() { return m_bEnableAlignSpecOutJudge; }
	int getEnableAlignMeasureSpecOutJudgeCount() { return m_nEnableAlignSpecOutJudgeCount; }
	double getDistanceInspHoleMetal() { return m_dbDistanceInspHoleMetalSpec; }

	BOOL getMarkReverseFind() { return m_bMarkReverseFind; }
	BOOL getEnableAlignReverseSelect() { return m_bIsAlignReversePanel; }
	int getInspFindMethod() { return m_nInspMethod; }
	BOOL getEnableUseAlignImage() { return m_bUseAlignImage; }
	int getChipRansacDirection() { return m_nChipRansacDirection; }
	BOOL getUseDummyPortAlign() { return m_bUseDummyPortAlign; }

	BOOL getRevisionReverse() { return m_bRevisionReverse; }
	BOOL getRevisionReverseX() { return m_bRevisionReverseX; }	//210110
	BOOL getRevisionReverseY() { return m_bRevisionReverseY; }	//210110
	BOOL getRevisionReverseT() { return m_bRevisionReverseT; }	//210110
	BOOL getRevisionReverseXY() { return m_bRevisionReverseXY; }	//210110	
	int getInspectionMethod() { return m_nInspectionMethodType; }
	BOOL getUseFixtureMark() { return m_bUseFixtureMark; }	//210110
	BOOL getUseReferenceMark() { return m_bUseReferenceMark; }
	int getInspectionSubMethod() { return m_nInspectionSubMethodType; }
	int getInspectionSubDir() { return m_nInspectionSubDirctionType; }
	double getInspRangeX() { return m_nInspRangeX; }
	double getInspRangeY() { return m_nInspRangeY; }
	int getSearchEdgePolarity() { return m_nSearchEdgePolarity; }
	int getSearchSearchDir() { return m_nSearchDirection; }
	double getInspRangeLength() { return m_nInspRangeLength; }
	double getInspTraceInterval() { return m_nInspTraceInterval; }
	int getInspResultCount() { return m_nInspResultCount; }
	double getInspInHoleSize() { return m_nInspInHoleSize; }
	double getInspCamHoleSize() { return m_nInspCamHoleSize; }
	double getInspSteelHoleSize() { return m_nInspSteelHoleSize; }
	BOOL getUseRealTimeTrace() { return m_bRealTimeTrace; }
	BOOL getUseUseFixTrace() { return m_bUseFixTrace; }
	BOOL getUseCycleTrace() { return m_bCycleTrace; }
	BOOL getUseMetalTrace() { return m_bMetalTrace; }
	BOOL getUseReverseOrder() { return m_bReverseOrder; }
	BOOL getUseSubInspAlgorithm(){ return m_bUseSubInspAlgorithm; }
	BOOL getUseMetalTraceFirst(){ return m_bMetalTraceFirst; }
	BOOL getUseAutoSaveAVI() { return m_bAutoSaveAVI; }
	BOOL getUsePNBase() { return m_bPNBase; }

	double getDistanceInspSpecLSL(int pos) { return m_dbDistanceInspSpecLSL[pos]; }
	double getDistanceInspSubSpec(int pos) { return m_dbDistanceInspSubSpec[pos]; }
	double getDistanceInspSpecY(int pos) { return m_dbDistanceInspSpecY[pos]; }
	double getDistanceInspSpecUSL(int pos) { return m_dbDistanceInspSpecUSL[pos]; }
	double getDistanceInspSubSpecTolerance(int pos) { return m_dbDistanceInspSubSpecTolerance[pos]; }
	double getDistanceInspSpecYMinMax(int pos) { return m_dbDistanceInspSpecYMinMax[pos]; }
	double getInspectionSubInterval() { return m_dbDistanceInspSubInterval; }
	double getInspectionSubIntervalTolerance() { return m_dbDistanceInspSubIntervalTolerance; }
	int getUseCrackInsp() { return m_bUseCrackInsp; }
	int getUseIncludedAngle() { return m_bUseIncludedAngle; }
	double getIncludedAngle() { return m_dbIncludedAngle; }
	double getIncludedAngleLimit() { return m_dbIncludedAngleLimit; }
	BOOL getUseBoxInsp() { return m_bUseBoxInsp; }
	BOOL getUseDummySeparInsp() { return m_bUseDummySeparInsp; }
	BOOL getUseMultiplexRoi() { return m_bUseMultiplexRoi; }
	int getUseDummyCornerInsp() { return m_bUseDummyCornerInsp; }
	int getUseDoubleDispensingMode() { return m_bUseDoubleDispensingEnable; }	
	int getLineOverflowDispMode() { return m_bLineOverflowDispMode; }
	int getCircle_Cink1_Cink2MergeMode() { return m_bCircle_Cink1_Cink2MergeMode; }
	int getLine_InspWithTraceResultMode() { return m_bLine_InspWithTraceResultMode; }
	int getDummyCornerInspCam() { return m_nDummyCornerInspCam; }
	int getDummyCornerFilter() { return m_nDummyCornerFilter; }
	double getDummyCornerIncludedAngle() { return m_dbDummyIncludedAngle; }
	double getDummyCornerIncludedAngleLimit() { return m_dbDummyIncludedAngleLimit; }
	double getFiducialMarkPitchX() { return m_dbFiducialMarkPitchX; }
	double getFiducialMarkPitchY() { return m_dbFiducialMarkPitchY; }
	double getFiducialSecondMarkPitchY() { return m_dbFiducialSecondMarkPitchY; }
	
	double getHalconMinContrast(int nCam) { return m_dbHalconMinContrast[nCam]; }
	double getHalconHighContrast(int nCam) { return m_dbHalconHighContrast[nCam]; }
	int getCornerType(int nCam, int nPos) { return m_nCornerType[nCam][nPos]; }
	int getEdgePolarity(int nCam, int nPos) { return m_nEdgePolarity[nCam][nPos]; }
	int getEdgeDirection(int nCam, int nPos) { return m_nEdgeDirection[nCam][nPos]; }
	int getPreProcess(int nCam, int nPos) { return m_nPreProcess[nCam][nPos]; }
	int getEdgeProcess(int nCam, int nPos) { return m_nEdgeProcess[nCam][nPos]; }
	int getSearchArea(int nCam, int nPos, int nIndex) { return m_nSearchArea[nCam][nPos][nIndex]; }
	int getProjectionArea(int nCam, int nPos, int nIndex) { return m_nProjectionArea[nCam][nPos][nIndex]; }
	int getSobelThreshold(int nCam, int nPos, int nIndex) { return m_nSobelThreshold[nCam][nPos][nIndex]; }
	int getRemoveNoiseSize(int nCam, int nPos, int nIndex) { return m_nRemoveNoiseSize[nCam][nPos][nIndex]; }
};

class CAlignMeasureInpInfo {
public:
	CAlignMeasureInpInfo() {
		for (int i = 0; i < 4; i++)
		{
			m_InspTargetRoi[i] = CRect(-1, -1, -1, -1);
			m_InspMarkRoi[i] = CRect(-1, -1, -1, -1);
			m_nInspLineLength[i] = 0;
			m_nInspNoiseRemoveSize[i] = 100;
			m_nTargetLowContrast[i] = 20;
			m_nTargetHighContrast[i] = 50;
			m_nMarkLowContrast[i] = 20;
			m_nMarkHighContrast[i] = 50;
			m_dMatchingRate[i] = 0.7;
			m_dbTargetPosOffsetX[i] = 0;
			m_dbTargetPosOffsetY[i] = 0;
			m_dbMarkPosOffsetX[i] = 0;
			m_dbMarkPosOffsetY[i] = 0;
		}

		m_bWhiteDust = TRUE;
		m_bBlackChip = FALSE;
		m_bPeelerInsp = FALSE;
		m_bPeelerInspMode = TRUE;
		m_bPreMoldSizeCalcInsp = FALSE;
		m_bOverflowInsp = FALSE;

		m_nOutSearchRange = 0;
		m_nInSearchRange = 0;
		m_nUndispDopoWidth = 0;
		m_nUndispDopoLength = 0;
		m_bUseFindNewRect = FALSE;

		for (int i = 0; i < 3; i++)
		{
			m_nSearchPosOffset[i] = 0;
			m_nSearchRange[i] = 0;
			m_nSearchThreshold[i] = 0;
			m_nSearchEdgeColor[i] = 0;
			m_nSearchDirection[i] = 0;
		}
	}
private:
	CRect m_InspTargetRoi[4];
	CRect m_InspMarkRoi[4];

	int m_nInspLineLength[4];
	int m_nInspNoiseRemoveSize[4];

	int m_nTargetLowContrast[4];
	int m_nTargetHighContrast[4];
	int m_nMarkLowContrast[4];
	int m_nMarkHighContrast[4];

	double m_dMatchingRate[4];
	double m_dbTargetPosOffsetX[4];
	double m_dbTargetPosOffsetY[4];
	double m_dbMarkPosOffsetX[4];
	double m_dbMarkPosOffsetY[4];

	int m_iInspRoiOffset;
	int m_iInspRoiOffset2;
	int m_iInspRoiOffset3;
	int m_iInspRoiOffset4;
	double m_dFilterContrast;
	double m_dFilterSize;
	double m_dNozzleCircularity;
	double m_dInspRotateAngle;
	BOOL m_bWhiteDust;
	BOOL m_bEnableAutoRoi;
	BOOL m_bBlackChip;
	BOOL m_bPeelerInsp;
	BOOL m_bPeelerInspMode;
	BOOL m_bPreMoldSizeCalcInsp;
	BOOL m_bOverflowInsp;

	int m_nOutSearchRange;
	int m_nInSearchRange;
	int m_nUndispDopoWidth;
	int m_nUndispDopoLength;
	BOOL m_bUseFindNewRect;

	double m_nSearchPosOffset[3];
	double m_nSearchRange[3];
	double m_nSearchThreshold[3];
	int m_nSearchEdgeColor[3];
	int m_nSearchDirection[3];
	int m_nDopoStartPt;			// 도포 시작 포인트	; 
	int m_nDopoEndPt;			// 도포 끝 포인트	; 
public:
	void setSearchPosOffset(int nIndex, double nOffset) { m_nSearchPosOffset[nIndex] = nOffset; }
	void setSearchRange(int nIndex, double nRange) { m_nSearchRange[nIndex] = nRange; }
	void setSearchThreshold(int nIndex, double nThreshold) { m_nSearchThreshold[nIndex] = nThreshold; }
	void setSearchEdgeColor(int nIndex, int nColor) { m_nSearchEdgeColor[nIndex] = nColor; }
	void setSearchDirection(int nIndex, int nDir) { m_nSearchDirection[nIndex] = nDir; }

	void setInspTargetRoi(int pos, CRect roi) { m_InspTargetRoi[pos] = roi; }
	void setInspMarkRoi(int pos, CRect roi) { m_InspMarkRoi[pos] = roi; }
	void setInspLineLength(int pos, int length) { m_nInspLineLength[pos] = length; }
	void setInspNoiseRemoveSize(int pos, int removesize) { m_nInspNoiseRemoveSize[pos] = removesize; }
	void setInspTargetContrast(int pos, int low, int high) { m_nTargetLowContrast[pos] = low; m_nTargetHighContrast[pos] = high; }
	void setInspMarkContrast(int pos, int low, int high) { m_nMarkLowContrast[pos] = low; m_nMarkHighContrast[pos] = high; }
	void setMatchingRate(int pos, double rate) { m_dMatchingRate[pos] = rate; }
	void setInspTargetOffset(int pos, double x, double y) { m_dbTargetPosOffsetX[pos] = x; m_dbTargetPosOffsetY[pos] = y; }
	void setInspMarkOffset(int pos, double x, double y) { m_dbMarkPosOffsetX[pos] = x; m_dbMarkPosOffsetY[pos] = y; }

	void setAutoRoiOffset(int iInspRoiOffset) { m_iInspRoiOffset = iInspRoiOffset; }
	void setAutoRoiOffset2(int iInspRoiOffset) { m_iInspRoiOffset2 = iInspRoiOffset; }
	void setAutoRoiOffset3(int iInspRoiOffset) { m_iInspRoiOffset3 = iInspRoiOffset; }
	void setAutoRoiOffset4(int iInspRoiOffset) { m_iInspRoiOffset4 = iInspRoiOffset; }
	void setInspFilterContrast(double dFilterContrast) { m_dFilterContrast = dFilterContrast; }
	void setInspFilterSize(double dFilterSize) { m_dFilterSize = dFilterSize; }
	void setNozzleCircularity(double dNozzleCircularity) { m_dNozzleCircularity = dNozzleCircularity; }
	void setInspRotateAngle(double dInspRotateAngle) { m_dInspRotateAngle = dInspRotateAngle; }
	void setEnableAutoRoiseSelect(BOOL bEnableAutoRoi) { m_bEnableAutoRoi = bEnableAutoRoi; }
	void setInspWhiteDust(BOOL bwhite) { m_bWhiteDust = bwhite; }
	void setInspBlackChip(BOOL bBlack) { m_bBlackChip = bBlack; }
	void setInspPeeler(BOOL bPeelerInsp) { m_bPeelerInsp = bPeelerInsp; }
	void setInspPeelerMode(BOOL bPeelerInspmode) { m_bPeelerInspMode = bPeelerInspmode; }
	void setInspPreMoldSizeCalc(BOOL bPreMoldSizeCalc) { m_bPreMoldSizeCalcInsp = bPreMoldSizeCalc; }
	void setInspOverflowInspection(BOOL bInsp) { m_bOverflowInsp = bInsp; }

	void setInspOverflowOutSearchRange(int nRange) { m_nOutSearchRange = nRange; }
	void setInspOverflowInSearchRange(int nRange) { m_nInSearchRange = nRange; }
	void setInspOverflowUndispDopoLength(int nLength) { m_nUndispDopoLength = nLength; }
	void setInspOverflowUndispDopoWidth(int nWidth) { m_nUndispDopoWidth = nWidth; }
	void setUseFindNewRect(BOOL bUse) { m_bUseFindNewRect = bUse; }
	void setDopoStartPt(int nSel) { m_nDopoStartPt = nSel; }
	void setDopoEndPt(int nSel) { m_nDopoEndPt = nSel; }

	double getSearchPosOffset(int nIndex) { return m_nSearchPosOffset[nIndex]; }
	double getSearchRange(int nIndex) { return m_nSearchRange[nIndex]; }
	double getSearchThreshold(int nIndex) { return m_nSearchThreshold[nIndex]; }
	int getSearchEdgeColor(int nIndex) { return m_nSearchEdgeColor[nIndex]; }
	int getSearchDirection(int nIndex) { return m_nSearchDirection[nIndex]; }

	CRect getInspTargetRoi(int pos) { return m_InspTargetRoi[pos]; }
	CRect getInspMarkRoi(int pos) { return m_InspMarkRoi[pos]; }
	int getInspLineLength(int pos) { return m_nInspLineLength[pos]; }
	int getInspNoiseRemoveSize(int pos) { return m_nInspNoiseRemoveSize[pos]; }
	int getInspTargetContrastLow(int pos) { return m_nTargetLowContrast[pos]; }
	int getInspTargetContrastHigh(int pos) { return m_nTargetHighContrast[pos]; }
	int getInspMarkContrastLow(int pos) { return m_nMarkLowContrast[pos]; }
	int getInspMarkContrastHigh(int pos) { return m_nMarkHighContrast[pos]; }
	double getMatchingRate(int pos) { return m_dMatchingRate[pos]; }

	double getInspTargetOffsetX(int pos) { return m_dbTargetPosOffsetX[pos]; }
	double getInspTargetOffsetY(int pos) { return m_dbTargetPosOffsetY[pos]; }
	double getInspMarkOffsetX(int pos) { return m_dbMarkPosOffsetX[pos]; }
	double getInspMarkOffsetY(int pos) { return m_dbMarkPosOffsetY[pos]; }

	int getAutoRoiOffset() { return  m_iInspRoiOffset; }
	int getAutoRoiOffset2() { return  m_iInspRoiOffset2; }
	int getAutoRoiOffset3() { return  m_iInspRoiOffset3; }
	int getAutoRoiOffset4() { return  m_iInspRoiOffset4; }
	double getInspFilterContrast() { return  m_dFilterContrast; }
	double getInspFilterSize() { return m_dFilterSize; }
	double getNozzleCircularity() { return m_dNozzleCircularity; }
	double getInspRotateAngle() { return m_dInspRotateAngle; }
	BOOL getEnableAutoRoiseSelect() { return m_bEnableAutoRoi; }
	BOOL getInspWhiteDust() { return m_bWhiteDust; }
	BOOL getInspBlackChip() { return m_bBlackChip; }
	BOOL getInspPeeler() { return m_bPeelerInsp; }
	BOOL getInspPeelerMode() { return m_bPeelerInspMode; }
	BOOL getInspPreMoldSizeCalc() { return m_bPreMoldSizeCalcInsp; }
	BOOL getInspOverflowInspection() { return m_bOverflowInsp; }

	int getInspOverflowOutSearchRange() { return m_nOutSearchRange; }
	int getInspOverflowInSearchRange() { return m_nInSearchRange; }
	int getInspOverflowUndispDopoLength() { return m_nUndispDopoLength; }
	int getInspOverflowUndispDopoWidth() { return m_nUndispDopoWidth; }
	int IsUseFindNewRect() { return m_bUseFindNewRect; }
	int getDopoStartPt() { return m_nDopoStartPt; }
	int getDopoEndPt() { return m_nDopoEndPt; }
};

class CPanelExistInfo {
public:
	CPanelExistInfo() {
		memset(m_rcSearchROI, 0, sizeof(CRect) * MAX_PANEL);
		for (int i = 0; i < MAX_PANEL; i++)
			m_dbAvgGrayLimit[i] = 35.0;
		m_nJudgeCondition = 0;
		m_bEnableExistCheckByEdge = FALSE;
		m_bEnablePanelValidPositionCheck = FALSE;
	}
private:
	BOOL m_bEnableExistCheckByEdge;
	BOOL m_bEnablePanelValidPositionCheck;
	int m_nJudgeCondition;
	CRect m_rcSearchROI[MAX_PANEL];
	double m_dbAvgGrayLimit[MAX_PANEL];
	double m_dbPosOffsetX[MAX_PANEL][MAX_PATTERN_INDEX];	// Pattern Matching 용
	double m_dbPosOffsetY[MAX_PANEL][MAX_PATTERN_INDEX];	// Pattern Matching 용
	CRect m_rcPanelValidPos[MAX_PANEL];
	BOOL m_bUseMask[MAX_PANEL][MAX_PATTERN_INDEX];

	double m_dbPanelValidPosLeft[MAX_PANEL];
	double m_dbPanelValidPosTop[MAX_PANEL];
	double m_dbPanelValidPosRight[MAX_PANEL];
	double m_dbPanelValidPosBottom[MAX_PANEL];
public:
	BOOL getEnableExistCheckByEdge() { return m_bEnableExistCheckByEdge; }
	BOOL getUseMask(int nPanel, int nIndex) { return m_bUseMask[nPanel][nIndex]; }
	BOOL getEnablePanelValidPosCheck() { return m_bEnablePanelValidPositionCheck; }
	double getPanelValidPosLeft(int nPanel) { return m_dbPanelValidPosLeft[nPanel]; }
	double getPanelValidPosTop(int nPanel) { return m_dbPanelValidPosTop[nPanel]; }
	double getPanelValidPosRight(int nPanel) { return m_dbPanelValidPosRight[nPanel]; }
	double getPanelValidPosBottom(int nPanel) { return m_dbPanelValidPosBottom[nPanel]; }

	CRect getPanelValidPos(int nPanel) { return m_rcPanelValidPos[nPanel]; }
	double getPosOffsetX(int nPanel, int nIndex) { return m_dbPosOffsetX[nPanel][nIndex]; }
	double getPosOffsetY(int nPanel, int nIndex) { return m_dbPosOffsetY[nPanel][nIndex]; }
	int getJudgeCondition() { return m_nJudgeCondition; }
	double getAvgGrayLimit(int nPanel) { return m_dbAvgGrayLimit[nPanel]; }
	CRect getSearchROI(int nPanel) { return m_rcSearchROI[nPanel]; }

	void setEnableExistCheckByEdge(BOOL bEnable) { m_bEnableExistCheckByEdge = bEnable; }
	void setUseMask(int nPanel, int nIndex, BOOL bUse) { m_bUseMask[nPanel][nIndex] = bUse; }
	void setEnablePanelValidPosCheck(BOOL bEnable) { m_bEnablePanelValidPositionCheck = bEnable; }
	void setPanelValidPosLeft(int nPanel, double value) { m_dbPanelValidPosLeft[nPanel] = value; }
	void setPanelValidPosTop(int nPanel, double value) { m_dbPanelValidPosTop[nPanel] = value; }
	void setPanelValidPosRight(int nPanel, double value) { m_dbPanelValidPosRight[nPanel] = value; }
	void setPanelValidPosBottom(int nPanel, double value) { m_dbPanelValidPosBottom[nPanel] = value; }

	void setPanelValidPos(int nPanel, CRect rect) { m_rcPanelValidPos[nPanel] = rect; }
	void setPosOffsetX(int nPanel, int nIndex, double offset) { m_dbPosOffsetX[nPanel][nIndex] = offset; }
	void setPosOffsetY(int nPanel, int nIndex, double offset) { m_dbPosOffsetY[nPanel][nIndex] = offset; }
	void setJudgeCondition(int nCondition) { m_nJudgeCondition = nCondition; }
	void setAvgGrayLimit(int nPanel, double dbLimit) { m_dbAvgGrayLimit[nPanel] = dbLimit; }
	void setSearchROI(int nPanel, CRect rcROI) { m_rcSearchROI[nPanel] = rcROI; }
};

// dh.jung 2021-08-02 add 검사 spec, parameter
class CInspSpecPara {
public:
	CInspSpecPara() {
		for (int nInspPos = 0; nInspPos < MAX_INSP_POS; nInspPos++)
		{
			m_dInSpec[nInspPos] = 0.0;
			m_dInSpecTorr[nInspPos] = 0.0;
			m_dOutSpec[nInspPos] = 0.0;
			m_dOutSpecTorr[nInspPos] = 0.0;
			m_dTotalSpec[nInspPos] = 0.0;
			m_dTotalSpecTorr[nInspPos] = 0.0;
			m_dDistanceSpec[nInspPos] = 0.0;
			m_dDistanceSpecTorr[nInspPos] = 0.0;

			m_dStartSerchPoint[nInspPos] = 0.0;
			m_dEndSerchPoint[nInspPos] = 0.0;
			m_dSerchMinMaxJudgeOffsetIn[nInspPos] = 0.0;
			m_dSerchMinMaxJudgeOffsetOut[nInspPos] = 0.0;
			m_dDustInspIn[nInspPos] = 0.0;
			m_dDustInspOut[nInspPos] = 0.0;
			m_dLimitLineInspOffset[nInspPos] = 0.0;
			m_dInkCheckThresh[nInspPos] = 60;  //Tkyuha 211112 도포된 패널 투입 검사 Gray 밝기값
			m_dLamiCheckThresh[nInspPos] = 60; //Tkyuha 211112 미스라미 패널 투입 검사 Gray 밝기값
		}
		m_nTraceImageThresh = 0;
		m_nFindEdgeThresh = 0;

		m_dDustInspROISize = 0.0;
		m_bDustInspROIShape = FALSE;

		m_bDispensingStatusEnable = false;		//도포된 패널 유무 검사
		m_bLamiStatusEnable = false;		//라미틀어짐 검사
		m_bCenterDistanceEnable = false;		//홀 중심점 이격 검사
		m_bCenterCCDistanceEnable = false;		//홀 중심점 과 BM 이격 검사
		m_bNozzleToHoleEdgeEnable = false;		//노즐과 홀 사이 검사 Nozzle Pos Error
		m_bCircleMetalRadiusCheck = false;		//홀과 메탈 지름 비교하여 메탈이 작은 경우 에러 처리
		m_bCircleMetalCenterCheck = false;		//홀과 메탈 중심점 비교하여 거리가 차이가 나는 경우 에러 처리
		m_bNozzlePanelHoleCheck = false;		//패널 홀과 노즐 Y갭 측정후 예외처리
		m_bCircleFindCenterCheck = true; 		//홀 중심점과 카메라 중심점 거리 편차 체크 30픽셀
		m_bInspectionDebugCheck = true; 		//이미지 디버깅 해서 보여줌
		m_bCalcWetoutMetalEdge = false; 		// blue 2022.09.02 CINK1+CINK2(EGL)에서 wetout 계산시 GlassEdge 대신 MetalEdge 사용(F=기존 GlassEdge 사용)
		m_bWetoutInspectionSimulation = false;

		m_iDustInspGrabDelay = 0;
		m_iDropInspGrabDelay = 0;
		m_iCenterAlignGrabDelay = 0;
		m_iNozzleAlignGrabDelay = 0;
		m_iFilmInspGrabDelay = 0;
		m_dFilmInspExecptionSpec = 0.0;
		m_nFilmInspExecptionCount = 0;

		//KJH 2022-01-09 검사 flag Aligninfo->inspSpecinfo로 위치변경

		m_bInspDust = FALSE;
		m_bInspDiff = FALSE;
		m_bInputInspection = FALSE;

		m_nInspHighThresh = 0;
		m_nInspLowThresh = 0;

		m_nExistPanelGray = 50;
		m_nExistDummyGray = 200;

		m_nInspDustThresh = 0;
		m_dNozzleToStageGapOffset = 0;

		m_dCInkMinWidthSpec = 0.07;
		m_dLineLengthMinThick = 0;
		m_dLineLengthSpec = 2.5;
		m_dCamCenterToCG = 33.565;
		m_dInkUnderflowSpec = 0.1;
		m_dDummyMinSizeSpec = 0.1;

		//KJH 2022-01-12 WetoutInsp Para Insp Para로 분기
		m_iCCFindFilter = 0;
		m_bTestImageSave = FALSE;;

		m_bAdaptiveDispensingFlag = false;
		m_dBaseOffset = 0;
		m_dScaleOffset = 1;
		m_dDispensingRange = 90;

		m_bMetalSearchRemoveNoiseFlag = true;
		m_bTraceUsingAlignFlag = false;
		m_bSearchElbEndPosFlag = true;
		m_bELBCG_Edge_CalcModeEnable = false;
		m_bELBPartial_DispensingModeEnable = false;
		m_bELBInsp_ShiftSearchEnable = false;
		m_bAVI_Save_ImageTypeEnable = false;
		m_bUsePanelThresh = true;

		m_bPNSizeInspEnable = false;    	//PN Size 검사
		m_dPN_SizeSpec = 0;
		m_dPN_SizeSpecTorr = 0;

		m_bFilmReAttachCheckEnable = false;
		m_iFilmReAttachCheckThresh = 0;
		// YCS 2022-10-31 필름 유무검사 영역 사이즈 고정
		m_bFixedFilmReattachUse = 0;
		m_dFilmReattachFixedSize = 0.0;

		m_dFeriAlignFindThresh = 80;
		m_d45DegreeInkInspThresh = 150;
		m_b45DegreeInkInsp = false;

		// YCS 2022-11-19 필름 각도 비교 스펙 파라미터 
		m_dFilmAngleSpec = 0;
		m_bFilmAngleInspection = FALSE;

		// YCS 2022-12-01 Wetout_ASPC 스펙 추가
		m_dWetout_ASPCSpec = 0;
	}
private:

	// 검사 Spec
	double m_dInSpec[MAX_INSP_POS];
	double m_dInSpecTorr[MAX_INSP_POS];
	double m_dOutSpec[MAX_INSP_POS];
	double m_dOutSpecTorr[MAX_INSP_POS];
	double m_dTotalSpec[MAX_INSP_POS];
	double m_dTotalSpecTorr[MAX_INSP_POS];
	double m_dDistanceSpec[MAX_INSP_POS];
	double m_dDistanceSpecTorr[MAX_INSP_POS];

	// 검사 Parameter
	double m_dStartSerchPoint[MAX_INSP_POS];
	double m_dEndSerchPoint[MAX_INSP_POS];
	double m_dSerchMinMaxJudgeOffsetIn[MAX_INSP_POS];
	double m_dSerchMinMaxJudgeOffsetOut[MAX_INSP_POS];
	double m_dDustInspIn[MAX_INSP_POS];
	double m_dDustInspOut[MAX_INSP_POS];
	double m_dLimitLineInspOffset[MAX_INSP_POS];

	//KJH 2021-11-12 Trace Parameter 작업 시작
	int m_nTraceImageThresh;
	int m_nFindEdgeThresh;

	//KJH 2021-12-01 Dust Insp Image Para 추가
	double m_dDustInspROISize;
	BOOL m_bDustInspROIShape;

	int    m_dInkCheckThresh[MAX_INSP_POS]; //Tkyuha 211112 도포된 패널 투입 검사 Gray 밝기값
	int    m_dLamiCheckThresh[MAX_INSP_POS]; //Tkyuha 211112 미스라미 패널 투입 검사 Gray 밝기값

	bool m_bDispensingStatusEnable;  	//도포된 패널 유무 검사
	bool m_bLamiStatusEnable;        	//라미틀어짐 검사
	bool m_bCenterDistanceEnable;    	//홀 중심점 이격 검사
	bool m_bCenterCCDistanceEnable;    	//홀 중심점 과 BM 이격 검사
	bool m_bNozzleToHoleEdgeEnable; 	//노즐과 홀 사이 검사 Nozzle Pos Error
	bool m_bCircleMetalRadiusCheck; 	//홀과 메탈 지름 비교하여 메탈이 작은 경우 에러 처리
	bool m_bCircleMetalCenterCheck; 	//홀과 메탈 중심점 비교하여 거리가 차이가 나는 경우 에러 처리
	bool m_bNozzlePanelHoleCheck; 		//패널 홀과 노즐 Y갭 측정후 예외처리
	bool m_bCircleFindCenterCheck; 		//홀 중심점과 카메라 중심점 거리 편차 체크 30픽셀
	bool m_bInspectionDebugCheck;  		//이미지 디버깅 해서 보여줌
	bool m_bNozzleAngleSearchMode;		// 노즐 각도 찾는 방법 선택
	bool m_bCalcWetoutMetalEdge; 		// blue 2022.09.02 CINK1+CINK2(EGL)에서 wetout 계산시 GlassEdge 대신 MetalEdge 사용(F=기존 GlassEdge 사용)
	bool m_bWetoutInspectionSimulation; //Tkyuha 221109 wetout 검사를 한장으로 시뮬레이션 하기 위함

	int m_iDustInspGrabDelay;
	int m_iDropInspGrabDelay;
	int m_iCenterAlignGrabDelay;
	int m_iNozzleAlignGrabDelay;
	int m_iFilmInspGrabDelay;
	int m_iFilmInspMethod;
	bool m_bFilmReversePosition;

	double m_dFilmInspExecptionSpec;
	int m_nFilmInspExecptionCount;
	
	//hsj 2022-01-27 필름검사 scale 파라미터 추가
	bool m_bPanelCaliperEnable;
	bool m_bFilmCaliperEnable;
	bool m_bFilmScaleEnable;
	double m_dFilmLXScale;
	double m_dFilmRXScale;
	double m_dFilmLYScale;
	double m_dFilmRYScale;

	// hsj 2022-02-07 필름검사 L check 기능 추가
	bool m_bPanelLcheckEnable;
	double m_dPanelLcheckLength;
	double m_dPanelLcheckTorr;

	bool m_bFilmLcheckEnable;
	double m_dFilmLcheckLength;
	double m_dFilmLcheckTorr;

	bool m_bFilmReAttachCheckEnable;
	int m_iFilmReAttachCheckThresh;
	// YCS 2022-10-31 필름 유무검사 영역 사이즈 고정
	bool m_bFixedFilmReattachUse;
	double m_dFilmReattachFixedSize;

	bool m_bUseReverseThresh;

	bool m_bFilmUVCheckEnable;
	int m_iFilmUVCheckThresh;
	int m_iFilmUVCheckSpec;

	// hsj 2022-02-07 필름검사 threshold 기능 추가
	bool m_bUsePanelThresh;
	int m_nPanelThresh;

	//KJH 2022-01-09 검사 flag Aligninfo->inspSpecinfo로 위치변경

	BOOL m_bInspDust;
	BOOL m_bInspDiff;
	BOOL m_bInputInspection;

	int m_nInspHighThresh;
	int m_nInspLowThresh;

	int m_nExistPanelGray;
	int m_nExistDummyGray;

	int m_nInspDustThresh;
	double m_nInspMinSize;
	double m_dNozzleToStageGapOffset;
	//KJH 2022-01-12 WetoutInsp Para Insp Para로 분기
	int m_iCCFindFilter;
	BOOL m_bTestImageSave;

	bool m_bAdaptiveDispensingFlag;
	double m_dBaseOffset;
	double m_dScaleOffset;
	double m_dDispensingRange;
	bool m_bMetalSearchRemoveNoiseFlag;
	bool m_bTraceUsingAlignFlag;
	bool m_bSearchElbEndPosFlag;
	bool m_bELBCG_Edge_CalcModeEnable;
	double m_dELB_Line_Pocket_Size;
	bool m_bELBPartial_DispensingModeEnable;
	bool m_bELBInsp_ShiftSearchEnable;
	bool m_bAVI_Save_ImageTypeEnable;
	bool m_bLineShift_DispensingModeEnable;

	//KJH 2022-01-25 ELB Cover CInk mode
	BOOL m_bELBCoverCInkMode;
	double m_dCInkMinWidthSpec;
	double m_dLineLengthMinThick;
	double m_dLineLengthSpec;
	double m_dCamCenterToCG;
	double m_dInkUnderflowSpec;
	double m_dDummyMinSizeSpec;
	double m_dFeriAlignFindThresh;

	//KJH 2022-02-05 Metal Over flow Margin
	double m_iMetalOverflowMargin;

	//KJH 2022-04-09 PN 반지름 비교 검사 추가
	bool m_bPNSizeInspEnable;    	//PN Size 검사
	double m_dPN_SizeSpec;
	double m_dPN_SizeSpecTorr;

	//KJH 2022-04-29 MincountCheckInsp 추가
	bool m_bMinCountCheckInspModeEnable;
	int	 m_nMinCountCheckInspContinueCount;
	int  m_nMinCountCheckInspTotalCount;

	//KJH 2022-05-09 R Judge 사용 유무 추가
	bool   m_bWetoutRJudgeModeEnable;
	double m_nWetoutRMinSpec;
	double m_nWetoutRMaxSpec;

	//KJH 2022-05-25 BM In <-> Cink In Insp 추가
	bool   m_bWetoutBJudgeModeEnable;
	double m_nWetoutBMinSpec;
	double m_nWetoutBMaxSpec;
	bool   m_bWetoutMinMaxJudgeEnable;

	double m_dLineLengthFDC_Distance;

	//SJB 2022-11-11 Overflow Min 파라미터 추가
	double m_dLineOverflowMinSpec;

	//SJB 2022-11-19 CG Exposure Time 파라미터 추가
	int m_iCGGrabExposure;

	bool   m_bDiffRoiMerge;		// blue 2022.09.09

	//KJH 2022-04-29 ELB CENTER ALIGN LIVE VIEW 가이드 라인 그리기 추가
	double m_dGuideLineoffset;
	bool   m_bCenterNozzleYAlignRecalcEnable;
	bool   m_bSideNozzleZAlignRecalcEnable;
	bool   m_bCenterNozzleAlignMethod;
	int m_iCenterNozzleToPanelDistanceGray;

	//KJH2
	bool m_bCenterNozzleYLight;

	//KJH2
	int m_nNozzleYGapThreshold;
	double m_dbRotateCenterSpec;
	int m_nRotateCenterBinary;
	int m_nRotateCenterLight;

	double m_d45DegreeInkInspThresh;
	bool m_b45DegreeInkInsp;
	int m_d45DegreeStartDelay;
	int  m_d45DegreeIntervalDelay;

	//SJB 2022-10-03 Scratch Parameter
	bool m_bScratchCheck;
	int m_nScratchThresh;
	double m_dMaskingRadius;
	int m_nScratchInspSpec;
	int m_nScratchInspAreaSpec;

	// YCS 2022-11-19 필름 각도 비교 스펙 파라미터 
	double m_dFilmAngleSpec;
	BOOL m_bFilmAngleInspection;

	// YCS 2022-12-01 Wetout_ASPC 스펙 추가
	double m_dWetout_ASPCSpec;

public:
	// 검사 Spec 
	void setInSpec(int nInspPosNo, double dbValue) { m_dInSpec[nInspPosNo] = dbValue; }
	double getInSpec(int nInspPosNo) { return m_dInSpec[nInspPosNo]; }
	void setInSpecTorr(int nInspPosNo, double dbValue) { m_dInSpecTorr[nInspPosNo] = dbValue; }
	double getInSpecTorr(int nInspPosNo) { return m_dInSpecTorr[nInspPosNo]; }
	void setOutSpec(int nInspPosNo, double dbValue) { m_dOutSpec[nInspPosNo] = dbValue; }
	double getOutSpec(int nInspPosNo) { return m_dOutSpec[nInspPosNo]; }
	void setOutSpecTorr(int nInspPosNo, double dbValue) { m_dOutSpecTorr[nInspPosNo] = dbValue; }
	double getOutSpecTorr(int nInspPosNo) { return m_dOutSpecTorr[nInspPosNo]; }
	void setTotalSpec(int nInspPosNo, double dbValue) { m_dTotalSpec[nInspPosNo] = dbValue; }
	double getTotalSpec(int nInspPosNo) { return m_dTotalSpec[nInspPosNo]; }
	void setTotalSpecTorr(int nInspPosNo, double dbValue) { m_dTotalSpecTorr[nInspPosNo] = dbValue; }
	double getTotalSpecTorr(int nInspPosNo) { return m_dTotalSpecTorr[nInspPosNo]; }
	void setDistanceSpec(int nInspPosNo, double dbValue) { m_dDistanceSpec[nInspPosNo] = dbValue; }
	double getDistanceSpec(int nInspPosNo) { return m_dDistanceSpec[nInspPosNo]; }
	void setDistanceSpecTorr(int nInspPosNo, double dbValue) { m_dDistanceSpecTorr[nInspPosNo] = dbValue; }
	double getDistanceSpecTorr(int nInspPosNo) { return m_dDistanceSpecTorr[nInspPosNo]; }

	void setUseFilmReversePosition(BOOL bUse) { m_bFilmReversePosition = bUse; }
	BOOL getUseFilmReversePosition() { return m_bFilmReversePosition; }

	int getFilmInspMethod() { return m_iFilmInspMethod; }
	void setFilmInspMethod(int method) { m_iFilmInspMethod = method; }

	bool getPanelCaliperEnable() { return m_bPanelCaliperEnable; }
	void setPanelCaliperEnable(bool benable) { m_bPanelCaliperEnable = benable; }
	bool getFilmCaliperEnable() { return m_bFilmCaliperEnable; }
	void setFilmCaliperEnable(bool benable) { m_bFilmCaliperEnable = benable; }

	//hsj 2022-01-27 필름검사 scale 파라미터 추가
	bool getFilmScaleEnable() { return m_bFilmScaleEnable; }
	void setFilmScaleEnable(bool benable) { m_bFilmScaleEnable = benable; }

	double getFilmLXScale() { return m_dFilmLXScale; }
	void setFilmLXScale(double dscale) { m_dFilmLXScale = dscale; }
	double getFilmRXScale() { return m_dFilmRXScale; }
	void setFilmRXScale(double dscale) { m_dFilmRXScale = dscale; }
	double getFilmLYScale() { return m_dFilmLYScale; }
	void setFilmLYScale(double dscale) { m_dFilmLYScale = dscale; }
	double getFilmRYScale() { return m_dFilmRYScale; }
	void setFilmRYScale(double dscale) { m_dFilmRYScale = dscale; }

	// hsj 2022-02-07 필름검사 L check 기능 추가
	bool getPanelLcheckEnable() { return m_bPanelLcheckEnable; }
	void setPanelLcheckEnable(bool benable) { m_bPanelLcheckEnable = benable; }

	double getPanelLcheckLength() { return m_dPanelLcheckLength; }
	void setPanelLcheckLength(double dLength) { m_dPanelLcheckLength = dLength; }
	double getPanelLcheckTorr() { return m_dPanelLcheckTorr; }
	void setPanelLcheckTorr(double dTorr) { m_dPanelLcheckTorr = dTorr; }

	bool getFilmLcheckEnable() { return m_bFilmLcheckEnable; }
	void setFilmLcheckEnable(bool benable) { m_bFilmLcheckEnable = benable; }

	double getFilmLcheckLength() { return m_dFilmLcheckLength; }
	void setFilmLcheckLength(double dLength) { m_dFilmLcheckLength = dLength; }
	double getFilmLcheckTorr() { return m_dFilmLcheckTorr; }
	void setFilmLcheckTorr(double dTorr) { m_dFilmLcheckTorr = dTorr; }

	bool getFilmReAttachCheckEnable() { return m_bFilmReAttachCheckEnable; }
	void setFilmReAttachCheckEnable(bool benable) { m_bFilmReAttachCheckEnable = benable; }

	int getFilmReAttachCheckThresh() { return m_iFilmReAttachCheckThresh; }
	void setFilmReAttachCheckThresh(int thresh) { m_iFilmReAttachCheckThresh = thresh; }

	int getUseReverseThresh() { return m_bUseReverseThresh; }
	void setUseReverseThresh(bool bUse) { m_bUseReverseThresh = bUse; }

	bool getFilmUVCheckEnable() { return m_bFilmUVCheckEnable; }
	void setFilmUVCheckEnable(bool benable) { m_bFilmUVCheckEnable = benable; }

	int getFilmUVCheckThresh() { return m_iFilmUVCheckThresh; }
	void setFilmUVCheckThresh(int thresh) { m_iFilmUVCheckThresh = thresh; }

	int getFilmUVCheckSpec() { return m_iFilmUVCheckSpec; }
	void setFilmUVCheckSpec(int thresh) { m_iFilmUVCheckSpec = thresh; }

	// YCS 2022-10-31 필름 유무검사 영역 사이즈 고정
	bool getFixedFilmReattachUse() { return m_bFixedFilmReattachUse; }
	void setFixedFilmReattachUse(bool bUse) { m_bFixedFilmReattachUse = bUse; }
	double getFixedFilmReattachSize() { return m_dFilmReattachFixedSize; }
	void setFixedFilmReattachSize(double dSize) { m_dFilmReattachFixedSize = dSize; }

	// hsj 2022-02-07 필름검사 threshold 기능 추가
	bool getUsePanelThresh_Auto() { return m_bUsePanelThresh; }
	void setUsePanelThresh_Auto(bool bThresh) { m_bUsePanelThresh = bThresh; }
	int getPanelThresh() { return m_nPanelThresh; }
	void setPanelThresh(int nThresh) { m_nPanelThresh = nThresh; }

	// 검사 Parameter
	void setStartSerchPoint(int nInspPosNo, double dbValue) { m_dStartSerchPoint[nInspPosNo] = dbValue; }
	double getStartSerchPoint(int nInspPosNo) { return m_dStartSerchPoint[nInspPosNo]; }
	void setEndSerchPoint(int nInspPosNo, double dbValue) { m_dEndSerchPoint[nInspPosNo] = dbValue; }
	double getEndSerchPoint(int nInspPosNo) { return m_dEndSerchPoint[nInspPosNo]; }
	void setSerchMinMaxJudgeOffsetIn(int nInspPosNo, double dbValue) { m_dSerchMinMaxJudgeOffsetIn[nInspPosNo] = dbValue; }
	double getSerchMinMaxJudgeOffsetIn(int nInspPosNo) { return m_dSerchMinMaxJudgeOffsetIn[nInspPosNo]; }
	void setSerchMinMaxJudgeOffsetOut(int nInspPosNo, double dbValue) { m_dSerchMinMaxJudgeOffsetOut[nInspPosNo] = dbValue; }
	double getSerchMinMaxJudgeOffsetOut(int nInspPosNo) { return m_dSerchMinMaxJudgeOffsetOut[nInspPosNo]; }
	void setDustInspIn(int nInspPosNo, double dbValue) { m_dDustInspIn[nInspPosNo] = dbValue; }
	double getDustInspIn(int nInspPosNo) { return m_dDustInspIn[nInspPosNo]; }
	void setDustInspOut(int nInspPosNo, double dbValue) { m_dDustInspOut[nInspPosNo] = dbValue; }
	double getDustInspOut(int nInspPosNo) { return m_dDustInspOut[nInspPosNo]; }
	int getInkCheckThresh(int nInspPosNo) { return m_dInkCheckThresh[nInspPosNo]; }
	void setInkCheckThresh(int nInspPosNo, int iValue) { m_dInkCheckThresh[nInspPosNo] = iValue; }
	int getLamiCheckThresh(int nInspPosNo) { return m_dLamiCheckThresh[nInspPosNo]; }
	void setLamiCheckThresh(int nInspPosNo, int iValue) { m_dLamiCheckThresh[nInspPosNo] = iValue; }
	void setLimitLineInspOffset(int nInspPosNo, double dbValue) { m_dLimitLineInspOffset[nInspPosNo] = dbValue; }
	double getLimitLineInspOffset(int nInspPosNo) { return m_dLimitLineInspOffset[nInspPosNo]; }

	//KJH 2021-11-12 Trace Parameter 작업 시작
	void setTraceImageThresh(int dbValue) { m_nTraceImageThresh = dbValue; }
	int getTraceImageThresh() { return m_nTraceImageThresh; }
	void setFindEdgeThresh(int dbValue) { m_nFindEdgeThresh = dbValue; }
	int getFindEdgeThresh() { return m_nFindEdgeThresh; }

	//KJH 2021-12-01 Dust Insp Image Para 추가
	void setDiffInspROISize(double dbValue) { m_dDustInspROISize = dbValue; }
	double getDiffInspROISize() { return m_dDustInspROISize; }
	void setDiffInspROIShape(BOOL dbValue) { m_bDustInspROIShape = dbValue; }
	BOOL getDiffInspROIShape() { return m_bDustInspROIShape; }

	// 검사 Flag
	bool getDispensingStatusEnable() { return m_bDispensingStatusEnable; }
	void setDispensingStatusEnable(bool benable) { m_bDispensingStatusEnable = benable; }
	bool getLamiStatusEnable() { return m_bLamiStatusEnable; }
	void setLamiStatusEnable(bool benable) { m_bLamiStatusEnable = benable; }
	bool getCenterDistanceEnable() { return m_bCenterDistanceEnable; }
	void setCenterDistanceEnable(bool benable) { m_bCenterDistanceEnable = benable; }
	bool getCenterCCDistanceEnable() { return m_bCenterCCDistanceEnable; }
	void setCenterCCDistanceEnable(bool benable) { m_bCenterCCDistanceEnable = benable; }
	bool getNozzleToHoleEdgeEnable() { return m_bNozzleToHoleEdgeEnable; }
	void setNozzleToHoleEdgeEnable(bool benable) { m_bNozzleToHoleEdgeEnable = benable; }
	bool getCircleMetalRadiusCheck() { return m_bCircleMetalRadiusCheck; }
	void setCircleMetalRadiusCheck(bool benable) { m_bCircleMetalRadiusCheck = benable; }
	bool getCircleMetalCenterCheck() { return m_bCircleMetalCenterCheck; }
	void setCircleMetalCenterCheck(bool benable) { m_bCircleMetalCenterCheck = benable; }
	bool getCircleFindCenterCheck() { return m_bCircleFindCenterCheck; }
	void setCircleFindCenterCheck(bool benable) { m_bCircleFindCenterCheck = benable; }
	bool getNozzlePanelHoleCheck() { return m_bNozzlePanelHoleCheck; }
	void setNozzlePanelHoleCheck(bool benable) { m_bNozzlePanelHoleCheck = benable; }
	bool getInspectionDebugCheck() { return m_bInspectionDebugCheck; }
	void setInspectionDebugCheck(bool benable) { m_bInspectionDebugCheck = benable; }
	bool getNozzleAngleSearchMode() { return m_bNozzleAngleSearchMode; }
	void setNozzleAngleSearchMode(bool benable) { m_bNozzleAngleSearchMode = benable; }
	bool getCalcWetoutMetalEdge() { return m_bCalcWetoutMetalEdge; }	// blue 2022.09.02
	void setCalcWetoutMetalEdge(bool benable) { m_bCalcWetoutMetalEdge = benable; }
	bool getWetoutInspectionSimulation() { return m_bWetoutInspectionSimulation; }
	void setWetoutInspectionSimulation(bool benable) { m_bWetoutInspectionSimulation = benable; }
	
	// 카메라 Grab Delay
	int getDustInspGrabDelay() { return m_iDustInspGrabDelay; }
	void setDustInspGrabDelay(int delay) { m_iDustInspGrabDelay = delay; }
	int getDropInspGrabDelay() { return m_iDropInspGrabDelay; }
	void setDropInspGrabDelay(int delay) { m_iDropInspGrabDelay = delay; }
	int getCenterAlignGrabDelay() { return m_iCenterAlignGrabDelay; }
	void setCenterAlignGrabDelay(int delay) { m_iCenterAlignGrabDelay = delay; }
	int getNozzleAlignGrabDelay() { return m_iNozzleAlignGrabDelay; }
	void setNozzleAlignGrabDelay(int delay) { m_iNozzleAlignGrabDelay = delay; }
	int getFilmInspGrabDelay() { return m_iFilmInspGrabDelay; }
	void setFilmInspGrabDelay(int delay) { m_iFilmInspGrabDelay = delay; }

	double getFilmInspExceptionSpec() { return m_dFilmInspExecptionSpec; }
	void setFilmInspExceptionSpec(double dValue) { m_dFilmInspExecptionSpec = dValue; }
	int getFilmInspExceptionCount() { return m_nFilmInspExecptionCount; }
	void setFilmInspExceptionCount(int nValue) { m_nFilmInspExecptionCount = nValue; }

	void set45DegreeInkInspThresh(double dOtsuThresh) { m_d45DegreeInkInspThresh = dOtsuThresh; }
	void set45DegreeInkInspEnable(bool bEnableInspection) { m_b45DegreeInkInsp = bEnableInspection; }
	double get45DegreeInkInspThresh() { return m_d45DegreeInkInspThresh; }
	bool get45DegreeInkInspEnable() { return m_b45DegreeInkInsp; }

	void set45DegreenStartDelay(int delay) { m_d45DegreeStartDelay = delay; }
	void set45DegreenIntervalDelay(int delay) { m_d45DegreeIntervalDelay = delay; }
	int get45DegreenStartDelay() { return m_d45DegreeStartDelay; }
	int get45DegreenIntervalDelay() { return m_d45DegreeIntervalDelay; }

	//KJH 2022-01-09 검사 flag Aligninfo->inspSpecinfo로 위치변경

	BOOL getUseDustInsp() { return m_bInspDust; }
	BOOL getUseDiffInsp() { return m_bInspDiff; }
	BOOL getUseInputInspection() { return m_bInputInspection; }

	int	 getInspectionHighThresh() { return  m_nInspHighThresh; }
	int	 getInspectionLowThresh() { return  m_nInspLowThresh; }

	int getExistPanelGray() { return m_nExistPanelGray; }
	int getExistDummyGray() { return m_nExistDummyGray; }

	int getInspDustThresh() { return m_nInspDustThresh; }
	double getInspMinSize() { return m_nInspMinSize; }

	void setUseDustInsp(BOOL bUse) { m_bInspDust = bUse; }
	void setUseDiffInsp(BOOL bUse) { m_bInspDiff = bUse; }
	void setUseInputInspection(BOOL bUse) { m_bInputInspection = bUse; }

	void setInspectionHighThresh(int t) { m_nInspHighThresh = t; }
	void setInspectionLowThresh(int t) { m_nInspLowThresh = t; }

	void setExistPanelGray(int gray) { m_nExistPanelGray = gray; }
	void setExistDummyGray(int gray) { m_nExistDummyGray = gray; }

	void setInspDustThresh(int t) { m_nInspDustThresh = t; }
	void setInspMinSize(double dVal) { m_nInspMinSize = dVal; }

	double getNozzleToStageGapOffset() { return m_dNozzleToStageGapOffset; }
	void setNozzleToStageGapOffset(double t) { m_dNozzleToStageGapOffset = t; }

	//KJH 2022-01-12 WetoutInsp Para Insp Para로 분기
	int getCCFindFilter() { return m_iCCFindFilter; }
	void setCCFindFilter(int Threshold) { m_iCCFindFilter = Threshold; }

	BOOL getTestImageSave() { return m_bTestImageSave; }
	void setTestImageSave(BOOL OnOff) { m_bTestImageSave = OnOff; }

	// Adaptive Dispensing 관련
	bool getAdaptiveDispensingFlag() { return m_bAdaptiveDispensingFlag; }
	void setAdaptiveDispensingFlag(bool flag) { m_bAdaptiveDispensingFlag = flag; }
	double getAdaptiveDispensingBaseOffset() { return m_dBaseOffset; }
	void setAdaptiveDispensingBaseOffset(double offset) { m_dBaseOffset = offset; }
	double getAdaptiveDispensingScaleOffset() { return m_dScaleOffset; }
	void setAdaptiveDispensingScaleOffset(double offset) { m_dScaleOffset = offset; }
	double getAdaptiveDispensingRange() { return m_dDispensingRange; }
	void setAdaptiveDispensingRange(double offset) { m_dDispensingRange = offset; }

	//Circle search Parameter 
	bool getMetalSearchRemoveNoiseFlag() { return m_bMetalSearchRemoveNoiseFlag; }
	void setMetalSearchRemoveNoiseFlag(bool flag) { m_bMetalSearchRemoveNoiseFlag = flag; }
	//KJH 2022-01-25 ELB Cover CInk mode
	int getELBCoverCInkMode() { return m_bELBCoverCInkMode; }
	void setELBCoverCInkMode(BOOL Enable) { m_bELBCoverCInkMode = Enable; }
	//HTK 2022-02-17 Trace search using align value mode
	bool getTraceUsingAlignFlag() { return m_bTraceUsingAlignFlag; }
	void setTraceUsingAlignFlag(bool flag) { m_bTraceUsingAlignFlag = flag; }
	//HTK 2022-02-17 CInk Inspection search ELB endPositionmode
	bool getSearchElbEndPosFlag() { return m_bSearchElbEndPosFlag; }
	void setSearchElbEndPosFlag(bool flag) { m_bSearchElbEndPosFlag = flag; }

	bool getELBCG_Edge_CalcModeEnable() { return m_bELBCG_Edge_CalcModeEnable; }
	void setELBCG_Edge_CalcModeEnable(bool flag) { m_bELBCG_Edge_CalcModeEnable = flag; }

	double getELB_Line_Pocket_Size() { return m_dELB_Line_Pocket_Size; }
	void setELB_Line_Pocket_Size(double flag) { m_dELB_Line_Pocket_Size = flag; }

	bool getELBPartial_DispensingModeEnable() { return m_bELBPartial_DispensingModeEnable; }
	void setELBPartial_DispensingModeEnable(bool flag) { m_bELBPartial_DispensingModeEnable = flag; }

	bool getELBInsp_ShiftSearchEnable() { return m_bELBInsp_ShiftSearchEnable; }
	void setELBInsp_ShiftSearchEnable(bool flag) { m_bELBInsp_ShiftSearchEnable = flag; }

	bool getAVI_Save_ImageTypeEnable() { return m_bAVI_Save_ImageTypeEnable; }
	void setAVI_Save_ImageTypeEnable(bool flag) { m_bAVI_Save_ImageTypeEnable = flag; }

	bool getLineShift_DispensingModeEnable() { return m_bLineShift_DispensingModeEnable; }
	void setLineShift_DispensingModeEnable(bool flag) { m_bLineShift_DispensingModeEnable = flag; }

	//KJH 2022-02-05 Metal Over flow Margin
	double getMetalOverflowMargin() { return m_iMetalOverflowMargin; }
	void setMetalOverflowMargin(double Margin) { m_iMetalOverflowMargin = Margin; }
	double getCInkMinWidthSpec() { return m_dCInkMinWidthSpec; }
	void setCInkMinWidthSpec(double spec) { m_dCInkMinWidthSpec = spec; }
	double getLineLengthMinThick() { return m_dLineLengthMinThick; }
	void setLineLengthMinThick(double spec) { m_dLineLengthMinThick = spec; }
	double getLineLengthSpec() { return m_dLineLengthSpec; }
	void setLineLengthSpec(double spec) { m_dLineLengthSpec = spec; }
	double getCamCenterToCG() { return m_dCamCenterToCG; }
	void setCamCenterToCG(double spec) { m_dCamCenterToCG = spec; }
	double getInkUnderflowSpec() { return m_dInkUnderflowSpec; }
	void setInkUnderflowSpec(double Margin) { m_dInkUnderflowSpec = Margin; }
	//HTK 2022-03-16 DummyInspSpec 추가
	double getDummyMinSizeSpec() { return m_dDummyMinSizeSpec; }
	void setDummyMinSizeSpec(double Margin) { m_dDummyMinSizeSpec = Margin; }

	double getFeriAlignFindThresh() { return m_dFeriAlignFindThresh; }
	void setFeriAlignFindThresh(double t) { m_dFeriAlignFindThresh = t; }

	//KJH 2022-04-09 PN 반지름 비교 검사 추가
	bool getPNSizeInspEnable() { return m_bPNSizeInspEnable; }
	void setPNSizeInspEnable(bool benable) { m_bPNSizeInspEnable = benable; }
	double getPNSizeSpec() { return m_dPN_SizeSpec; }
	void setPNSizeSpec(double PN_Size) { m_dPN_SizeSpec = PN_Size; }
	double getPNSizeSpecTorr() { return m_dPN_SizeSpecTorr; }
	void setPNSizeSpecTorr(double PN_SizeTorr) { m_dPN_SizeSpecTorr = PN_SizeTorr; }

	//KJH 2022-04-29 MincountCheckInsp 추가
	//KJH 2022-05-09 Mincount 상시 사용으로 변경
	//bool getMinCountCheckInspModeEnable() { return m_bMinCountCheckInspModeEnable; }
	//void setMinCountCheckInspModeEnable(bool flag) { m_bMinCountCheckInspModeEnable = flag; }
	int getMinCountCheckInspContinueCount() { return m_nMinCountCheckInspContinueCount; }
	void setMinCountCheckInspContinueCount(int value) { m_nMinCountCheckInspContinueCount = value; }
	int getMinCountCheckInspTotalCount() { return m_nMinCountCheckInspTotalCount; }
	void setMinCountCheckInspTotalCount(int value) { m_nMinCountCheckInspTotalCount = value; }

	//KJH 2022-05-09 R Judge 사용 유무 추가
	bool getWetoutRJudgeModeEnable() { return m_bWetoutRJudgeModeEnable; }
	void setWetoutRJudgeModeEnable(bool flag) { m_bWetoutRJudgeModeEnable = flag; }
	void setWetoutRMinSpec(double dval) { m_nWetoutRMinSpec = dval; }
	double getWetoutRMinSpec() { return m_nWetoutRMinSpec; }
	void setWetoutRMaxSpec(double dval) { m_nWetoutRMaxSpec = dval; }
	double getWetoutRMaxSpec() { return m_nWetoutRMaxSpec; }

	//KJH 2022-05-25 BM In <-> Cink In Insp 추가
	bool getWetoutBJudgeModeEnable() { return m_bWetoutBJudgeModeEnable; }
	void setWetoutBJudgeModeEnable(bool flag) { m_bWetoutBJudgeModeEnable = flag; }
	void setWetoutBMinSpec(double dval) { m_nWetoutBMinSpec = dval; }
	double getWetoutBMinSpec() { return m_nWetoutBMinSpec; }
	void setWetoutBMaxSpec(double dval) { m_nWetoutBMaxSpec = dval; }
	double getWetoutBMaxSpec() { return m_nWetoutBMaxSpec; }
	bool getWetoutMinMaxJudgeModeEnable() { return m_bWetoutMinMaxJudgeEnable; }
	void setWetoutMinMaxJudgeModeEnable(bool flag) { m_bWetoutMinMaxJudgeEnable = flag; }

	void setLineLengthFDC_Distance(double dval) { m_dLineLengthFDC_Distance = dval; }
	double getLineLengthFDC_Distance() { return m_dLineLengthFDC_Distance; }

	//SJB 2022-11-11 Overflow Min 파라미터 추가
	void setLineOverflowMinSpec(double dval) { m_dLineOverflowMinSpec = dval; }
	double getLineOverflowMinSpec() { return m_dLineOverflowMinSpec; }

	//SJB 2022-11-19 CG Exposure Time 파라미터 추가
	void setCGGrabExposure(int ival) { m_iCGGrabExposure = ival; }
	int getCGGrabExposure() { return m_iCGGrabExposure; }

	// blue 2022.09.09
	bool getDiffRoiMergeEnable() { return m_bDiffRoiMerge; }
	void setDiffRoiMergeEnable(bool flag) { m_bDiffRoiMerge = flag; }

	//KJH 2022-04-29 ELB CENTER ALIGN LIVE VIEW 가이드 라인 그리기 추가
	double getCenterLiveViewGuideLineOffset() { return m_dGuideLineoffset; }
	void setCenterLiveViewGuideLineOffset(double offset) { m_dGuideLineoffset = offset; }
	bool getCenterNozzleYAlignRecalc() { return m_bCenterNozzleYAlignRecalcEnable; }
	void setCenterNozzleYAlignRecalc(bool flag) { m_bCenterNozzleYAlignRecalcEnable = flag; }

	// KBJ 2022-12-06 SIDE NOZZLE RECALC
	bool getSideNozzleZAlignRecalc() { return m_bSideNozzleZAlignRecalcEnable; }
	void setSideNozzleZAlignRecalc(bool flag) { m_bSideNozzleZAlignRecalcEnable = flag; }

	int getCenterNozzleToPanelDistanceLight() { return m_iCenterNozzleToPanelDistanceGray; }
	void setCenterNozzleToPanelDistanceLight(int gray) { m_iCenterNozzleToPanelDistanceGray = gray; }
	bool getCenterNozzleAlignMethod() { return m_bCenterNozzleAlignMethod; }
	void setCenterNozzleAlignMethod(bool flag) { m_bCenterNozzleAlignMethod = flag; }
	
	//KJH2 
	bool getUseCenterNozzleYLight() { return m_bCenterNozzleYLight; }
	void setUseCenterNozzleYLight(bool bUse) { m_bCenterNozzleYLight = bUse; }

	//KJH2
	int getNozzleYGapThreshold() { return m_nNozzleYGapThreshold; }
	void setNozzleYGapThreshold(int nval) { m_nNozzleYGapThreshold = nval; }

	//KBJ
	double getRotateC_Spec() { return m_dbRotateCenterSpec; }
	void setRotateC_Spec(double nval) { m_dbRotateCenterSpec = nval; }

	int getRotateC_binary_value() { return m_nRotateCenterBinary; }
	void setRotateC_binary_value(int nval) { m_nRotateCenterBinary = nval; }

	int getRotateC_light_value() { return m_nRotateCenterLight; }
	void setRotateC_light_value(int nval) { m_nRotateCenterLight = nval; }

	//SJB 2022-10-03 Scratch Parameter
	bool getScratchCheck() { return m_bScratchCheck; }
	void setScratchCheck(bool nval) { m_bScratchCheck = nval; }

	int getScratchThresh() { return m_nScratchThresh; }
	void setScratchThresh(int nval) { m_nScratchThresh = nval; }

	double getScratchMaskingRadius() { return m_dMaskingRadius; }
	void setScratchMaskingRadius(double nval) { m_dMaskingRadius = nval; }

	int getScratchInspSpec() { return m_nScratchInspSpec; } 
	void setScratchInspSpec(int nval) { m_nScratchInspSpec = nval; }

	int getScratchInspAreaSpec() { return m_nScratchInspAreaSpec; }
	void setScratchInspAreaSpec(int nval) { m_nScratchInspAreaSpec = nval; }

	// YCS 2022-11-19 필름 각도 비교 스펙 파라미터
	double getFilmAngleSpec() { return m_dFilmAngleSpec; }
	void setFilmAngleSpec(double dSpec) { m_dFilmAngleSpec = dSpec; }

	bool getFilmAngleInspectionUse() { return m_bFilmAngleInspection; }
	void setFilmAngleInspectionUse(bool bUse) { m_bFilmAngleInspection = bUse; }

	// YCS 2022-12-01 Wetout_ASPC 스펙 추가
	double getWetoutASPCSpec() { return m_dWetout_ASPCSpec; }
	void setWetoutASPCSpec(double dValue) { m_dWetout_ASPCSpec = dValue; }
};

class CModel {
public:
	CModel() {
		memset(m_cModelID, 0, sizeof(m_cModelID));
		sprintf_s(m_cModelID, "Default");
	}

private:
	CLightInfo m_LightInfo;
	CMachineInfo m_MachineInfo;
	CGlassInfo m_GlassInfo;
	CAlignInfo m_AlignInfo;
	CAlignMeasureInpInfo m_AlignMeasureInfo;
	CPanelExistInfo m_PanelExistInfo;

	// dh.jung 2021-08-02 add
	CInspSpecPara m_InspSpecParaInfo;

	char m_cModelID[100];
public:
	CLightInfo & getLightInfo() { return m_LightInfo; }
	CGlassInfo & getGlassInfo() { return m_GlassInfo; }
	CMachineInfo & getMachineInfo() { return m_MachineInfo; }
	CAlignInfo & getAlignInfo() { return m_AlignInfo; }
	CAlignMeasureInpInfo & getMeasureInpInfo() { return m_AlignMeasureInfo; }
	CPanelExistInfo & getPanelExistInfo() { return m_PanelExistInfo; }

	// dh.jung 2021-08-02 add
	CInspSpecPara& getInspSpecParaInfo() { return m_InspSpecParaInfo; }

	char *getModelID() { return m_cModelID; }
	void setModelID(CString str) { sprintf_s(m_cModelID, "%s", (LPCTSTR)str); }
	void setGlassInfo(CGlassInfo tempGlassInfo) { memcpy(&m_GlassInfo, &tempGlassInfo, sizeof(CGlassInfo)); }
	void setMachineInfo(CMachineInfo tempMachineInfo) { memcpy(&m_MachineInfo, &tempMachineInfo, sizeof(CMachineInfo)); }
	void setAlignInfo(CAlignInfo tempAlignInfo) { memcpy(&m_AlignInfo, &tempAlignInfo, sizeof(CAlignInfo)); }
	void setMeasureInpInfo(CAlignMeasureInpInfo tempAlignInfo) { memcpy(&m_AlignMeasureInfo, &tempAlignInfo, sizeof(CAlignMeasureInpInfo)); }

	// dh.jung 2021-08-02 add
	void setInspSpecParaInfo(CInspSpecPara tempInspSpecParaInfo) { memcpy(&m_InspSpecParaInfo, &tempInspSpecParaInfo, sizeof(CInspSpecPara)); }

};

struct _stCameraInfo {
	int w;
	int h;
	int depth;
	int size;
	int flip_dir;
	int buse;
	char cSerial[MAX_PATH];
	char cName[MAX_PATH];

	BOOL use_cmd_reverse_x;
	BOOL reverse_x;
	BOOL use_cmd_reverse_y;
	BOOL reverse_y;
	BOOL use_cmd_fix_frame;
	int frame_rate;
	BOOL use_cmd_packet_size;
	int packet_size;
};

struct stLIGHTCTRLINFO {
	int nNumOfLightCtrl;
	int nType[MAX_LIGHT_CTRL];
	int nPort[MAX_LIGHT_CTRL];
	int nBaudrate[MAX_LIGHT_CTRL];
	int nSockPort[MAX_LIGHT_CTRL];
	char cSockIP[MAX_LIGHT_CTRL][MAX_PATH];
	char cName[MAX_LIGHT_CTRL][MAX_PATH];
};

struct stDARW_MACHINE_INFO {
	int nCount;
	int nCam;
	int nPos;
	BOOL bClear;
};

class CJobPlcAddrInfo {
public:	
	CJobPlcAddrInfo() {
		use_main_object_id = FALSE;
		use_sub_object_id = FALSE;
		read_bit_start = 15000;						// Job의 Bit Read 시작 주소
		write_bit_start = 15000;					// Job의 Bit Write 시작 주소
		read_word_start = 15000;					// Job의 Word Read 시작 주소
		write_word_start = 15000;					// Job의 Word Write 시작 주소
		read_main_object_id_start = 15000;			// Job의 Main ID 읽어야 될 경우
		read_sub_object_id_start;					// Job의 Sub ID 읽어야 될 경우
		size_main_object_id = 20;					// Word 단위
		size_sub_object_id = 20;					// Word 단위
	}

	BOOL use_main_object_id;
	BOOL use_sub_object_id;
	int read_bit_start;							// Job의 Bit Read 시작 주소
	int write_bit_start;						// Job의 Bit Write 시작 주소
	int read_word_start;						// Job의 Word Read 시작 주소
	int write_word_start;						// Job의 Word Write 시작 주소
	int read_main_object_id_start;				// Job의 Main ID 읽어야 될 경우
	int read_sub_object_id_start;				// Job의 Sub ID 읽어야 될 경우
	int size_main_object_id;					// Word 단위
	int size_sub_object_id;						// Word 단위
	int write_word_sub_start;                   // Word 결과 데이타
};

class CJobLightInfo {
public:
	CJobLightInfo() {
		num_of_using_light = 0;
		last_channel = 0;
		last_index = 0;
		channel_name.clear();
		index_name.clear();
		channel_id.clear();
		controller_id.clear();
	}

	int num_of_using_light;						// 카메라당 사용할 조명(채널) 개수
	vector<std::string> channel_name;			// 채널별 사용하는 조명 이름
	vector<std::string> index_name;				// 여러 샷을 해야 될 경우 index 구분
	vector<int> channel_id;						// 몇번 몇번 채널을 사용할 건지
	vector<int> controller_id;					// 몇번 몇번 컨트롤러를 사용할 건지
	int last_channel;							// 화면 전환시 이전 값을 기억하기 위함??
	int last_index;								// 화면 전환시 이전 값을 기억하기 위함??
};

class CJobInfo {
public:
	CJobInfo() {
		job_name.clear();
		main_object_id.clear();
		sub_object_id.clear();
		camera_index.clear();
		viewer_index.clear();
	}

	const char * get_job_name() { return job_name.c_str(); }
	const char * get_main_object_id() { return main_object_id.c_str(); }
	const char * get_sub_object_id() { return sub_object_id.c_str(); }
	
	int algo_method;
	std::string job_name;
	std::string main_object_id;
	std::string sub_object_id;
	int num_of_position;
	int num_of_camera;
	vector<int> camera_index;
	vector<CPoint> camera_size;	// 


	vector < vector<std::string>> position_name;			// 카메라별 포지션 이름
	vector < vector < vector<std::string>>> caliper_name;	// 카메라별 포지션 이름


	int num_of_viewer;
	vector<int> viewer_index;				// main viewer에서 몇번 viewer를 사용할건지.
	CJobPlcAddrInfo  plc_addr_info;
	vector<CJobLightInfo> light_info;
	CModel model_info;

	//CInspSpecPara insp_spec;				// dh.jung 2021-08-02 add 

	int machine_view_count_x;				// 해당 Job에 대한 machine viewer창 구성을 어떻게 할것인지
	int machine_view_count_y;				// 해당 Job에 대한 machine viewer창 구성을 어떻게 할것인지
	vector<int> machine_viewer_index;		// 해당 Job에 대한 machine viewer창의 순서를 어떻게 할것인지.	// main viewer에서 사용한 viewer 번호를 적자.	카메라 개수보다 많을수 없다.

	int main_view_count_x;
	int main_view_count_y;
	vector<int> main_viewer_index;			// 해당 Job에 대한 개별 main viewer창의 순서를 어떻게 할것인지.
};

class CViewerInfo {
public:
	std::string viewer_name;
	int image_width;
	int image_height;
	double resolution_x;
	double resolution_y;
	BOOL use_viewer_popup;
	int viewer_position;
	int camera_index;
	int image_index;
	int camera_status;
};

class CDisplayViewInfo {
public:
	CDisplayViewInfo() {
		m_pView = NULL;

		m_strPanelID = "1234567890";
		m_strViewerName = "2Point Align";

		m_bUseViewerPopup = FALSE;

		m_nViewImageWidth = 3088;
		m_nViewImageHeight = 2064;
		m_nCameraID = 0;
		m_nFlipDir = 0;

		m_nLastLightChannel = 0;
		m_nLastLightIndex = 0;

		for (int i = 0; i < MAX_LIGHT_PER_CAMERA; i++)
		{
			m_nLightController[i] = 0;
			m_nLightChannel[i] = 0;
			m_nLightChannelName[i].Format("%d", i);
		}

		m_dbResolutionX = 0.0096;
		m_dbResolutionY = 0.0096;
	}
private:
	CViewerEx *m_pView;
	CString m_strPanelID;
	CString m_strViewerName;

	int num_of_using_light;
	CString m_nLightChannelName[MAX_LIGHT_PER_CAMERA];
	CString m_nLightIndexName[4];

	BOOL m_bUseViewerPopup;

	int m_nViewImageWidth;
	int m_nViewImageHeight;
	int m_nCameraID;
	int m_nFlipDir;
	int m_nMethod;
	int m_nMarkCount;
	int m_nCaliperCount;

	int m_nViewerPos;

	int m_nLightController[MAX_LIGHT_PER_CAMERA];
	int m_nLightChannel[MAX_LIGHT_PER_CAMERA];

	double m_dbResolutionX;
	double m_dbResolutionY;
public:
	int m_nLastLightChannel;
	int m_nLastLightIndex;

	CViewerEx *getViewer() { return m_pView; }
	CString getPanelID() { return m_strPanelID; }
	CString  getViewerName() { return m_strViewerName; }
	CString  getLightChannelName(int i) { return m_nLightChannelName[i]; }
	CString  getLightIndexName(int i) { return m_nLightIndexName[i]; }
	BOOL geUseViewerPopup() { return m_bUseViewerPopup; }
	int getViewImageWidth() { return m_nViewImageWidth; }
	int getViewImageHeight() { return m_nViewImageHeight; }
	int getCameraID() { return m_nCameraID; }
	int getLightController(int i) { return m_nLightController[i]; }
	int getLightChannel(int i) { return m_nLightChannel[i]; }
	int get_num_of_using_light() { return num_of_using_light; }
	double getResolutionX() { return m_dbResolutionX; }
	double getResolutionY() { return m_dbResolutionY; }
	int getFlipDir() { return m_nFlipDir; }
	int getMethod() { return m_nMethod; }
	int getMarkCount() { return m_nMarkCount; }
	int getCaliperCount() { return m_nCaliperCount; }
	int get_viewer_pos() { return m_nViewerPos; }

	void setViewer(CViewerEx *v) { m_pView = v; }
	void setPanelID(CString v) { m_strPanelID = v; }
	void setViewerName(CString v) { m_strViewerName = v; }
	void setLightChannelName(int i, CString v) { m_nLightChannelName[i] = v; }
	void setLightIndexName(int i, CString v) { m_nLightIndexName[i] = v; }
	void seUseViewerPopup(BOOL v) { m_bUseViewerPopup = v; }
	void setViewImageWidth(int v) { m_nViewImageWidth = v; }
	void setViewImageHeight(int v) { m_nViewImageHeight = v; }
	void setCameraID(int v) { m_nCameraID = v; }
	void setLightController(int ch, int v) { m_nLightController[ch] = v; }
	void setLightChannel(int i, int v) { m_nLightChannel[i] = v; }
	void setResolutionX(double v) { m_dbResolutionX = v; }
	void setResolutionY(double v) { m_dbResolutionY = v; }
	void setFlipDir(int v) { m_nFlipDir = v; }
	void setMethod(int v) { m_nMethod = v; }
	void setMarkCount(int v) { m_nMarkCount = v; }
	void setCaliperCount(int v) { m_nCaliperCount = v; }
	void set_num_of_using_light(int v) { num_of_using_light = v; }
	void set_viewer_pos(int v) { m_nViewerPos = v; }
};

class CAlgorithmInfo {
public:
	CAlgorithmInfo() {
		m_nAlgoCount = 1;
		m_addPanel_ID_WordStart = 0;
		m_vAlgoCameraNum.clear();
		m_vAlgoMethod.clear();
		m_vAlgoName.clear();
		m_vAlgoCameraIndex.clear();
		m_vAlgoCameraNum.push_back(2);
		m_vAlgoMethod.push_back(2);
		m_vAlgoName.push_back("2PointAlign");

		std::vector<int> cam;
		cam.push_back(0);
		cam.push_back(1);
		m_vAlgoCameraIndex.push_back(cam);
	}
	~CAlgorithmInfo() {
		m_vAlgoCameraNum.clear();
		m_vAlgoMethod.clear();
		m_vAlgoName.clear();
		m_vAlgoCameraIndex.clear();
		m_addReadBitStart.clear();
		m_addWriteBitStart.clear();
		m_addReadWordStart.clear();
		m_addWriteWordStart.clear();
		m_vModel.clear();
	}
private:
	int m_nAlgoCount;
	int m_addPanel_ID_WordStart;
	std::vector<int> m_vAlgoCameraNum;
	std::vector<int> m_vAlgoMethod;
	std::vector<int> m_vAlgoNumPos;
	std::vector<std::string> m_vAlgoName;
	std::vector<std::vector<int>> m_vAlgoCameraIndex;

	std::vector<int>  m_addReadBitStart;
	std::vector<int>  m_addWriteBitStart;
	std::vector<int>  m_addReadWordStart;
	std::vector<int>  m_addWriteWordStart;

	std::vector<CModel> m_vModel;

public:
	void  algorithmBufferInit() {
		m_vAlgoCameraNum.clear();
		m_vAlgoMethod.clear();
		m_vAlgoName.clear();
		m_vAlgoCameraIndex.clear();
		m_addReadBitStart.clear();
		m_addWriteBitStart.clear();
		m_addReadWordStart.clear();
		m_addWriteWordStart.clear();
		m_vModel.clear();
	}


	int   getAlgorithmReadIDWordStart() { return m_addPanel_ID_WordStart; }
	int   getAlgorithmReadBitStart(int i) { return m_addReadBitStart.at(i); }
	int   getAlgorithmWriteBitStart(int i) { return m_addWriteBitStart.at(i); }
	int   getAlgorithmReadWordStart(int i) { return m_addReadWordStart.at(i); }
	int   getAlgorithmWriteWordStart(int i) { return m_addWriteWordStart.at(i); }
	int   getAlgorithmCount() { return m_nAlgoCount; }
	int   getAlgorithmCameraNum(int i) { return m_vAlgoCameraNum.at(i); }
	int   getAlgorithmMethod(int i) { return m_vAlgoMethod.at(i); }
	int   getAlgorithmNumPos(int i) { return m_vAlgoNumPos.at(i); }
	std::string getAlgorithmName(int i) { return m_vAlgoName.at(i); }
	std::vector<int> getAlgorithmCameraIndex(int i) { return m_vAlgoCameraIndex.at(i); }

	int getAlgoFromCam(int nCam) {
		for (int i = 0; i < getAlgorithmCount(); i++) {
			std::vector<int> AlgoCam = getAlgorithmCameraIndex(i);
			for (int j = 0; j < AlgoCam.size(); j++) {
				if (AlgoCam.at(j) == nCam)	return i;
			}
		}
		return 0;
	}

	void   setAlgorithmReadBitStart(int i, int v) {
		if (m_addReadBitStart.size() <= i) m_addReadBitStart.push_back(15000);
		m_addReadBitStart.at(i) = v;
	}
	void   setAlgorithmWriteBitStart(int i, int v) {
		if (m_addWriteBitStart.size() <= i) m_addWriteBitStart.push_back(15000);
		m_addWriteBitStart.at(i) = v;
	}
	void   setAlgorithmReadWordStart(int i, int v) {
		if (m_addReadWordStart.size() <= i) m_addReadWordStart.push_back(15000);
		m_addReadWordStart.at(i) = v;
	}
	void   setAlgorithmWriteWordStart(int i, int v) {
		if (m_addWriteWordStart.size() <= i) m_addWriteWordStart.push_back(15000);
		m_addWriteWordStart.at(i) = v;
	}
	void   setAlgorithmCount(int v) { m_nAlgoCount = v; }
	void   setAlgorithmReadIDWordStart(int v) { m_addPanel_ID_WordStart = v; }
	void   setAlgorithmCameraNum(int i, int v) {
		if (m_vAlgoCameraNum.size() <= i) m_vAlgoCameraNum.push_back(2);
		m_vAlgoCameraNum.at(i) = v;
	}
	void   setAlgorithmMethod(int i, int v) {
		if (m_vAlgoMethod.size() <= i) m_vAlgoMethod.push_back(2);
		m_vAlgoMethod.at(i) = v;
	}
	void   setAlgorithmNumPos(int i, int v) {
		if (m_vAlgoNumPos.size() <= i) m_vAlgoNumPos.push_back(2);
		m_vAlgoNumPos.at(i) = v;
	}
	void   setAlgorithmName(int i, std::string v) {
		if (m_vAlgoName.size() <= i) m_vAlgoName.push_back("2PointAlign");
		m_vAlgoName.at(i) = v;
	}
	void   setAlgorithmCameraIndex(int i, std::vector<int> v) {
		if (m_vAlgoCameraIndex.size() <= i)
		{
			std::vector<int> cam;
			cam.push_back(0);	cam.push_back(1);
			m_vAlgoCameraIndex.push_back(cam);
		}
		m_vAlgoCameraIndex.at(i) = v;
	}
};

// kbj 2021-12-31 시스템옵션 공통으로 파라미터 추가.
struct _stSystemCommon {
	BOOL use_plc_check;
	BOOL use_plc_model_process;
	int save_term_file_history;	// history 파일 저장 기간 (단위: 일)
	int save_term_file_excel;	// excel 파일 저장 기간 (단위: 일)
	int save_term_file_image;	// image 파일 저장 기간 (단위: 일)
	int save_term_file_video;	// 비디오 파일 저장 기간 (단위: 일)
	int cycle_time_to_delete_file_auto_mode;	// Auto Mode 일때 파일 지우는 사이클 시간 (단위: 초)
	int daily_production_reset_hour;
};

// kbj 2021-12-31 시스템옵션 Job 각각의 파라미터 추가.
class CSystemOption {
public:
	struct _stELB_Option {
		BOOL Align_DebuggingImage_Save;
		BOOL Trace_DebuggingImage_Save;
		BOOL Inspection_DebuggingImage_Save;
		double Auto_Save_Video_Time;
	};

	CSystemOption() {
		_stELB_Option stELB;
		stELB.Align_DebuggingImage_Save = FALSE;
		stELB.Trace_DebuggingImage_Save = FALSE;
		stELB.Inspection_DebuggingImage_Save = FALSE;
		stELB.Auto_Save_Video_Time = 4.5;

		jpg_compress_rate_Raw = 90;
		jpg_compress_rate_Result = 90;
		jpg_compress_rate = 90;
		save_image_type = 0;
		result_image_save = TRUE;
		raw_image_save = TRUE;
		ok_image_save = TRUE;
		ng_image_save = TRUE;
		use_rdp_report = FALSE;
		rdp_image_type = 0;			// 0 : Raw Image, 1 : Result Image, 2 : 둘다
		replacement_data = -9999.0;	// 밴딩,합착검사에서 찾지못한 포인트에 대해서 대체하는 값
		use_light_on_off_system = FALSE;		// 조명 항시 킬지 말지 설정
		show_calib_direction = FALSE;
		use_result_image_capture = FALSE;
	}
	_stELB_Option stELB;
	int jpg_compress_rate_Raw; // 원본이미지 압축율 
	int jpg_compress_rate_Result; // 결과이미지 압축율
	int jpg_compress_rate;		// jpg 압축율 ( 10 ~ 90 ) 낮을수록 이미지 꺠짐
	int save_image_type;		// 0 : jpg, 1 : bmp
	BOOL result_image_save;		// result image save할지 말지
	BOOL use_result_image_capture; // result image Capture로 할지 말지
	BOOL raw_image_save;		// raw image save할지 말지
	BOOL ok_image_save;			// ok 이미지 저장 유무
	BOOL ng_image_save;			// ng 이미지 저장 유무
	BOOL use_rdp_report;		// rdp 보고 할지 말지
	int rdp_image_type;			// 0 : Raw Image, 1 : Result Image, 2 : 둘다
	BOOL use_light_on_off_system;		// 조명 항시 킬지 말지 설정
	BOOL show_calib_direction;

	double replacement_data;	// 밴딩,합착검사에서 찾지못한 포인트에 대해서 대체하는 값
};

class NOTCH_LINE_DATA {
public:
	cv::Point2f pt_up;
	cv::Point2f pt_down;
	cv::Point2f pt_mid;
};

class NOTCH_WETOUT_DATA {
public:
	BOOL judge;	// TRUE - OK, FALSE - NG
	vector<cv::Point2f> pt_up;
	vector<cv::Point2f> pt_up_limit;
	vector<cv::Point2f> pt_down;
	vector<cv::Point2f> pt_center;
	vector<cv::Point2f> pt_metal;
	vector<double> dist_up;
	vector<double> dist_down;
	vector<double> width_up_down;
	vector<double> dist_metal;

	vector<bool> judge_up;
	vector<bool> judge_down;
	vector<bool> judge_metal;

	cv::Point2f pt_up_min;
	cv::Point2f pt_up_max;
	cv::Point2f pt_down_min;
	cv::Point2f pt_down_max;
	cv::Point2f pt_width_max;
	cv::Point2f pt_width_min;
	cv::Point2f pt_left;
	cv::Point2f pt_right;

	
public:
	double dist_up_max;
	double dist_up_min;
	double dist_up_avg;
	double dist_down_max;
	double dist_down_min;
	double dist_down_avg;
	double dist_width_max;
	double dist_width_min;
	double dist_width_avg;
	double dist_left;
	double dist_right;
};


class JOB_SEQ_INFO {
public:
	CDialog *pDlg;
	int job_id;
};

namespace
{
	const BYTE kLOCKTYPE_NONE = 0;
	const BYTE kLOCKTYPE_EXCLUSIVE = 1;
	const BYTE kLOCKTYPE_SHARED = 2;
}

/**
 * 크리티컬 섹션을 사용하기 쉽게 한 Wrapper 클래스
 *크리티컬 섹션의 초기화 및 해제 작업을 하지 않아도 됩니다. Lock 및 UnLock의 인터페이스를 통일합니다ㅣ.
 * 2021-09-24
 */

class CSimpleCriticalSection
{
public:
	CSimpleCriticalSection() : m_bInit(FALSE) { InitializeCriticalSection(&m_cs); m_bInit = TRUE; }
	CSimpleCriticalSection(DWORD dwSpinCount) : m_bInit(FALSE) { m_bInit = InitializeCriticalSectionAndSpinCount(&m_cs, dwSpinCount); }
	~CSimpleCriticalSection() { DeleteCriticalSection(&m_cs); }

	void Lock(BYTE byLockType = kLOCKTYPE_EXCLUSIVE) { EnterCriticalSection(&m_cs); }
	void Unlock(BYTE byLockType = kLOCKTYPE_EXCLUSIVE) { LeaveCriticalSection(&m_cs); }
	BOOL IsInit() { return m_bInit; }

#if (_WIN32_WINNT >= 0x0400)
	BOOL TryLock() { return TryEnterCriticalSection(&m_cs); }//_WIN32_WINNT=0x400
#endif


private:
	CRITICAL_SECTION    m_cs;
	BOOL                m_bInit;
};

/**
 * Scope범위에서만 Lock이 설정되도록 하는 클래습니다.
 * 템플릿인자 클래스는 Lock 및 UnLock함수를 구현해야 합니다.
 * 2021-09-24
 */
template<typename SimpleLockType>
class CScopeLock
{
public:
	CScopeLock(SimpleLockType* pcs, BYTE byLockType = kLOCKTYPE_EXCLUSIVE) : m_pcsObjcect(pcs), m_byLockType(byLockType)
	{
		if (m_pcsObjcect && m_pcsObjcect->IsInit())
		{
			m_pcsObjcect->Lock(m_byLockType);
		}
	}
	~CScopeLock()
	{
		if (m_pcsObjcect && m_pcsObjcect->IsInit())
		{
			m_pcsObjcect->Unlock(m_byLockType);
		}
	}
private:
	// CFSScopeLock은 NonCopyable
	CScopeLock(const SimpleLockType&);
	const CScopeLock<SimpleLockType>& operator=(const SimpleLockType&);

	SimpleLockType* m_pcsObjcect;
	BYTE            m_byLockType;
};

#endif

