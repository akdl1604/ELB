// DlgINISetting.cpp: 구현 파일
//

//#include "pch.h"
#include "stdafx.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"
#include "DlgINISetting.h"
#include "afxdialogex.h"
#include "../KeyPadDlg.h"
#include "../VirtualKeyBoard/VirtualKeyBoard.h"
#include <iphlpapi.h>
#include "UI_Control/DevicesHandler.h"

#ifdef __CAMERA_H__
#include <pylon/gige/GigETransportLayer.h>
using namespace Pylon;
#endif
#pragma comment(lib, "iphlpapi.lib")

// DlgINISetting 대화 상자
#define MSG_INI_JOB_VIEWER 10000
#define MSG_INI_JOB_POSITION 10001
#define MSG_INI_JOB_VIEW_VIEWER 10002
#define MSG_INI_JOB_MAIN_VIEWER 10003
#define MSG_INI_JOB_MACHINE_VIEWER 10004
#define MSG_INI_JOB_CAM_VIEWER 10005
#define MSG_INI_JOB_CALIPER 10006
#define MSG_INI_SETTING_LIGHT 10007
#define MSG_INI_JOB_LIGHT_INDEX_NAME 10008
#define MSG_INI_SETTING_PLC 10009
#define MSG_INI_SETTING_VIEWER 10010
#define MSG_INI_CAMERA_VIEWER 10011
#define MSG_INI_SERVER_VIEWER 10012
#define MSG_INI_LIGHT_CONTROL_VIEWER 10013
#define MSG_INI_PLC_ADDRESS_VIEWER 10014

IMPLEMENT_DYNAMIC(DlgINISetting, CDialogEx)

DlgINISetting::DlgINISetting(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_INI_SETTING, pParent)
{
	m_pMain = NULL;
	m_nNumOfView = 0;	
}

DlgINISetting::~DlgINISetting()
{
	m_CameraSerial.clear();
	m_CameraName.clear();
}

void DlgINISetting::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CB_SELECT_VIEWER, m_cbSelectViewer);
	DDX_Control(pDX, IDC_GRID_JOB_VIEWER, m_grid_job_viewer);
	DDX_Control(pDX, IDC_GRID_JOB_POSITION_VIEWER, m_grid_position_viewer);
	DDX_Control(pDX, IDC_GRID_JOB_VIEW_VIEWER, m_grid_job_view_viewer);
	DDX_Control(pDX, IDC_GRID_JOB_MAIN_VIEWER, m_grid_job_main_viewer);
	DDX_Control(pDX, IDC_GRID_JOB_MACHINE_VIEWER, m_grid_job_machine_viewer);
	DDX_Control(pDX, IDC_GRID_CAMERA_VIEWER, m_grid_camera_viewer);
	DDX_Control(pDX, IDC_GRID_JOB_CALIPER_VIEWER, m_grid_caliper_viewer);
	DDX_Control(pDX, IDC_GRID_JOB_LIGHT_DATA, m_grid_job_light_viewer);
	DDX_Control(pDX, IDC_GRID_JOB_LIGHT_INDEX_NAME_VIEWER, m_grid_light_index_name_viewer);
	DDX_Control(pDX, IDC_GRID_PLC_DATA, m_grid_plc_data);
	DDX_Control(pDX, IDC_GRID_SELECT_VIEWER, m_grid_select_viewer);
	DDX_Control(pDX, IDC_GRID_JOB_CAM_VIEWER, m_grid_job_cam_viewer);
	DDX_Control(pDX, IDC_GRID_SERVER_VIEWER, m_grid_server_viewer);
	DDX_Control(pDX, IDC_GRID_LIGHT_CONTROL_VIEWER, m_grid_light_control_viewer);
	DDX_Control(pDX, IDC_GRID_PLC_ADDRESS_VIEWER, m_grid_plc_address_viewer);
	DDX_Control(pDX, IDC_EDIT_JOB_COUNT_MODIFY, m_edt_jobcount);
	DDX_Control(pDX, IDC_CB_SELECT_JOB, m_cb_select_job);
	DDX_Control(pDX, IDC_CB_SELECT_LIGHT_CAMERA, m_cbSelectLightCam);
	DDX_Control(pDX, IDC_CB_SELECT_CAMERA, m_cbSelectCamera);
	DDX_Control(pDX, IDC_CB_SELECT_POS, m_cbSelectPos);
    DDX_Control(pDX, IDC_LIST_NETWORK, m_listNetworkAdapter);
}


BEGIN_MESSAGE_MAP(DlgINISetting, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_INI_SETTING_SAVE, &DlgINISetting::OnBnClickedBtnIniSettingSave)
	ON_CBN_SELCHANGE(IDC_CB_SELECT_VIEWER, &DlgINISetting::OnCbnSelchangeCbSelectViewer)
	ON_MESSAGE(WM_VIEW_CONTROL, OnViewControl)
	ON_BN_CLICKED(IDC_BTN_LIGHT_CONTROL_MODIFY, &DlgINISetting::OnBnClickedBtnLightControlModify)
	ON_BN_CLICKED(IDC_BTN_VIEWER_COUNT_MODIFY_OK, &DlgINISetting::OnBnClickedBtnViewerCountModifyOk)
	ON_BN_CLICKED(IDC_BTN_JOB_COUNT_MODIFY_OK, &DlgINISetting::OnBnClickedBtnJobCountModifyOk)
	ON_CBN_SELCHANGE(IDC_CB_SELECT_JOB, &DlgINISetting::OnCbnSelchangeCbSelectJob)
	ON_BN_CLICKED(IDC_BTN_CAMERA_COUNT_MODIFY_OK, &DlgINISetting::OnBnClickedBtnCameraCountModifyOk)
	ON_CBN_SELCHANGE(IDC_CB_SELECT_CAMERA, &DlgINISetting::OnCbnSelchangeCbSelectCamera)
	ON_CBN_SELCHANGE(IDC_CB_SELECT_POS, &DlgINISetting::OnCbnSelchangeCbSelectPos)
	ON_BN_CLICKED(IDC_BTN_POS_MODIFY_OK, &DlgINISetting::OnBnClickedBtnPosModifyOk)
	ON_BN_CLICKED(IDC_BTN_JOB_CAMERA_COUNT_MODIFY_OK, &DlgINISetting::OnBnClickedBtnJobCameraCountModifyOk)
	ON_BN_CLICKED(IDC_BTN_MACHINE_COUNT_MODIFY_OK, &DlgINISetting::OnBnClickedBtnMachineCountModifyOk)
	ON_BN_CLICKED(IDC_BTN_MAIN_COUNT_MODIFY_OK, &DlgINISetting::OnBnClickedBtnMainCountModifyOk)
	ON_CBN_SELCHANGE(IDC_CB_SELECT_LIGHT_CAMERA, &DlgINISetting::OnCbnSelchangeCbSelectLightCamera)
	ON_BN_CLICKED(IDC_BTN_JOB_USE_LIGHT_MODIFY_OK, &DlgINISetting::OnBnClickedBtnJobUseLightModifyOk)
	ON_BN_CLICKED(IDC_BTN_INI_EXIT, &DlgINISetting::OnBnClickedBtnIniExit)
    ON_BN_CLICKED(IDC_BTN_INI_CAM_MATCHIG, &DlgINISetting::OnBnClickedBtnIniCamMatchig)
	ON_LBN_DBLCLK(IDC_LIST_NETWORK, &DlgINISetting::OnDblclkListNetwork)
    ON_WM_TIMER()
	ON_CBN_SELCHANGE(IDC_CB_NUM_OF_USING_CAMERA4, &DlgINISetting::OnCbnSelchangeCbNumOfUsingCamera4)
END_MESSAGE_MAP()


// DlgINISetting 메시지 처리기


BOOL DlgINISetting::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.
	//ShowWindow(SW_SHOWMAXIMIZED);
	CenterWindow();
	m_pMain = (CLET_AlignClientDlg *)GetParent();
	InitINIData();
	InitCameraData();	
	getAdapterList();

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}


//jobini 읽어오기
void DlgINISetting::read_job_config()
{
	CString strKey, strData, strSection;
	TCHAR szData[MAX_PATH] = { 0, };
	
	CString strFilePath;
	//CJob temp_job_info;
	
	//Cjob.assign(m_pMain->vt_job_info.size(), temp_job_info);

	for (int i = 0; i < m_pMain->vt_job_info.size(); i++)
	{
		strFilePath.Format("%sJob%d_Info.ini", m_pMain->m_strConfigDir, i + 1);
		/////////////////////////////////////////////////////////JOB
		strSection.Format("JOB_INFO");
		strKey.Format("JOB_NAME");
		strData.Format("%s", m_pMain->vt_job_info[i].job_name.c_str());
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_strJobName[i].Format(_T("%s"), szData);

		strKey.Format("ALGO_METHOD");
		strData.Format("%d", m_pMain->vt_job_info[i].algo_method);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_nJobMethod[i] = atoi(szData);
		//////////////////////////////////////////////////// CAMERA
		strSection.Format("CAMERA_INFO");
		strKey.Format("NUM_OF_CAMERA");
		strData.Format("%d", m_pMain->vt_job_info[i].num_of_camera);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_nJobCamCount[i] = atoi(szData);

		for (int j = 0; j < m_pMain->vt_job_info[i].num_of_camera; j++)
		{
			strKey.Format("CAMERA_INDEX%d", j + 1);
			strData.Format("%d", m_pMain->vt_job_info[i].camera_index.at(j));
			::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
			m_nJobCamIndex[i][j] = atoi(szData);
		}

		///////////////////////////////////////////////////////////// POSITION
		strSection.Format("POSITION_INFO");
		strKey.Format("NUM_OF_POSITION");
		strData.Format("%d", m_pMain->vt_job_info[i].num_of_position);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_nJobPosition[i] = atoi(szData);

		for (int j = 0; j < m_pMain->vt_job_info[i].num_of_camera; j++)
		{
			for (int k = 0; k < m_pMain->vt_job_info[i].num_of_position; k++)
			{
				strKey.Format("POSITION_NAME%d_%d", j + 1, k + 1);
				strData.Format("%s", m_pMain->vt_job_info[i].position_name[j][k].c_str());
				::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
				m_stPosName[i][j][k].Format(_T("%s"), szData);
				CString stPosIndex = "";
			}
		}
		////////////////////////////////////////////////////////////////// VIEWER
		strSection.Format("VIEWER_INFO");
		strKey.Format("NUM_OF_VIEWER");
		strData.Format("%d", m_pMain->vt_job_info[i].num_of_viewer);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_nJobViewerCount[i] = atoi(szData);

		for (int j = 0; j < m_pMain->vt_job_info[i].num_of_viewer; j++)
		{
			strKey.Format("VIEWER_INDEX%d", j + 1);
			strData.Format("%d", m_pMain->vt_job_info[i].viewer_index.at(j));
			::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
			m_nJobViewerIndex[i][j] = atoi(szData);
		}

		strKey.Format("MAIN_VIEW_COUNT_X");
		strData.Format("%d", m_pMain->vt_job_info[i].main_view_count_x);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_nMainViewCountX[i] = atoi(szData);

		strKey.Format("MAIN_VIEW_COUNT_Y");
		strData.Format("%d", m_pMain->vt_job_info[i].main_view_count_y);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_nMainViewCountY[i] = atoi(szData);

		for (int j = 0; j < m_pMain->vt_job_info[i].num_of_viewer; j++)
		{
			strKey.Format("MAIN_VIEW_INDEX%d", j + 1);
			strData.Format("%d", m_pMain->vt_job_info[i].main_viewer_index[j]);
			::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
			m_nMainViewIndex[i][j] = atoi(szData);
		}

		strKey.Format("MACHINE_VIEW_COUNT_X");
		strData.Format("%d", m_pMain->vt_job_info[i].machine_view_count_x);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_nMachineViewCountX[i] = atoi(szData);

		strKey.Format("MACHINE_VIEW_COUNT_Y");
		strData.Format("%d", m_pMain->vt_job_info[i].machine_view_count_y);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_nMachineViewCountY[i] = atoi(szData);

		for (int j = 0; j < m_pMain->vt_job_info[i].num_of_camera; j++)
		{
			strKey.Format("MACHINE_VIEW_INDEX%d", j + 1);
			strData.Format("%d", m_pMain->vt_job_info[i].machine_viewer_index[j]);
			::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
			m_nMachineViewIndex[i][j] = atoi(szData);
		}
		
		////////////////////////////////////////////////////////////////// PLC
		strSection.Format("PLC_ADDR_INFO");
		strKey.Format("ADDR_READ_BIT_START", i + 1);
		strData.Format("%d", m_pMain->vt_job_info[i].plc_addr_info.read_bit_start);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_nJobReadBitStart[i] = atoi(szData);

		strKey.Format("ADDR_WRITE_BIT_START", i + 1);
		strData.Format("%d", m_pMain->vt_job_info[i].plc_addr_info.write_bit_start);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_nJobWriteBitStart[i] = atoi(szData);

		strKey.Format("ADDR_READ_WORD_START", i + 1);
		strData.Format("%d", m_pMain->vt_job_info[i].plc_addr_info.read_word_start);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_nJobReadWordStart[i] = atoi(szData);

		strKey.Format("ADDR_WRITE_WORD_START", i + 1);
		strData.Format("%d", m_pMain->vt_job_info[i].plc_addr_info.write_word_start);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_nJobWriteWordStart[i] = atoi(szData);

		strKey.Format("USE_MAIN_OBJECT_ID");
		strData.Format("%d", m_pMain->vt_job_info[i].plc_addr_info.use_main_object_id);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_nJobUseMainObjectID[i] = atoi(szData);

		strKey.Format("ADDR_MAIN_OBJECT_ID_START");
		strData.Format("%d", m_pMain->vt_job_info[i].plc_addr_info.read_main_object_id_start);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_nJobMainObjectIdStart[i] = atoi(szData);

		strKey.Format("USE_SUB_OBJECT_ID");
		strData.Format("%d", m_pMain->vt_job_info[i].plc_addr_info.use_sub_object_id);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_nJobUseSubObjectID[i] = atoi(szData);

		strKey.Format("ADDR_SUB_OBJECT_ID_START");
		strData.Format("%d", m_pMain->vt_job_info[i].plc_addr_info.read_sub_object_id_start);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_nJobSubObjectIdStart[i] = atoi(szData);

		////////////////////////////////////////////////////////////////// LIGHT
		strSection.Format("LIGHT_INFO");
		
		//CJobLight temp_light_info;
		//Cjob[i].Clight.assign(m_pMain->vt_job_info[i].num_of_camera, temp_light_info);

		for (int j = 0; j < m_pMain->vt_job_info[i].num_of_camera; j++)
		{
			strKey.Format("NUM_OF_USING_LIGHT%d", j + 1);
			strData.Format("%d", m_pMain->vt_job_info[i].light_info[j].num_of_using_light);
			::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
			m_nNumOfUsingLightCount[i][j] = atoi(szData);
			
			//Cjob[i].Clight[j].vJobLightCtrl.push_back(m_nNumOfUsingLightCount[i][j]);

			for (int n = 0; n < m_pMain->vt_job_info[i].light_info[j].num_of_using_light; n++)
			{
				strKey.Format("LIGHT_CTRL_ID%d_%d", j + 1, n + 1);
				strData.Format("%d", m_pMain->vt_job_info[i].light_info[j].controller_id.at(n));
				::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
				m_nLightControlId[i][j][n] = atoi(szData);

				strKey.Format("LIGHT_CHANNEL_ID%d_%d", j + 1, n + 1);
				strData.Format("%d", m_pMain->vt_job_info[i].light_info[j].channel_id.at(n));
				::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
				m_nLightChannelId[i][j][n] = atoi(szData);

				strKey.Format("LIGHT_CHANNEL_NAME%d_%d", j + 1, n + 1);
				strData.Format("%s", m_pMain->vt_job_info[i].light_info[j].channel_name.at(n));  
				::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
				m_stLightChannelName[i][j][n].Format("%s", szData);

			}

			for (int n = 0; n < MAX_LIGHT_INDEX; n++)
			{
				strKey.Format("LIGHT_INDEX_NAME%d_%d", j + 1, n + 1);
				strData.Format("%s", m_pMain->vt_job_info[i].light_info[j].index_name.at(n).c_str());
				::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
				m_stLightIndexName[i][j][n].Format("%s", szData);
			}
		}
		////////////////////////////////////////////////////////////////// CALIPER
		strSection.Format("CALIPER_INFO");

		for (int k = 0; k < m_pMain->vt_job_info[i].num_of_camera; k++)
		{
			for (int j = 0; j < m_pMain->vt_job_info[i].num_of_position; j++)
			{
				for (int n = 0; n < MAX_CALIPER_LINES; n++)
				{
					strKey.Format("CALIPER_NAME%d_%d_%d", k + 1, j + 1, n + 1);
					strData.Format("%s", m_pMain->vt_job_info[i].caliper_name[k][j][n].c_str());
					::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
					m_stCaliperName[i][k][j][n].Format("%s", szData);
				}
			}
		}
	}

}

//viewer ini 읽어오기
void DlgINISetting::read_view_config()
{
	CString strKey, strData, strSection;
	TCHAR szData[MAX_PATH] = { 0, };
	
	CString strFilePath;
	strFilePath.Format(m_pMain->m_iniViewerFile.GetPathName());

	SetDlgItemInt(IDC_EDIT_VIEWER_COUNT_MODIFY, (int)m_pMain->vt_viewer_info.size());
	m_nNumOfViewer = (int)m_pMain->vt_viewer_info.size();

	CString strCam = "All View";

	m_cbSelectViewer.ResetContent();

	m_cbSelectViewer.AddString(strCam);

	for (int i = 0; i < m_pMain->vt_viewer_info.size(); i++)
	{
		strCam.Format("View %d번", i + 1);
		m_cbSelectViewer.AddString(strCam);
	}

	m_cbSelectViewer.SetCurSel(0);

	strSection.Format("VIEWER_INFO");

	strKey.Format("VIEWER_COUNT_X");
	strData.Format("%d", m_pMain->m_nViewX_Num);
	::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
	SetDlgItemInt(IDC_EDIT_VIEWER_COUNT_X, m_pMain->m_nViewX_Num);
	m_nViewerCntX = atoi(szData);

	strKey.Format("VIEWER_COUNT_Y");
	strData.Format("%d", m_pMain->m_nViewY_Num);
	::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
	SetDlgItemInt(IDC_EDIT_VIEWER_COUNT_Y, m_pMain->m_nViewY_Num);
	m_nViewerCntY = atoi(szData);

	for (int i = 0; i < m_pMain->vt_viewer_info.size(); i++)
	{
		strSection.Format("VIEWER%d_INFO", i + 1);

		strKey.Format("NAME");
		strData.Format("%s", m_pMain->vt_viewer_info[i].viewer_name.c_str());
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_strName[i].Format("%s", szData);

		strKey.Format("WIDTH");
		strData.Format("%d", m_pMain->vt_viewer_info[i].image_width);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_nWidth[i] = atoi(szData);

		strKey.Format("HEIGHT");
		strData.Format("%d", m_pMain->vt_viewer_info[i].image_height);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_nHeight[i] = atoi(szData);

		strKey.Format("USE_POPUP");
		strData.Format("%d", m_pMain->vt_viewer_info[i].use_viewer_popup);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_nPopup[i] = atoi(szData);

		/*strKey.Format("RESOLUTION_X");
		strData.Format("%f", m_pMain->vt_viewer_info[i].resolution_x);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_dResolutionX[i] = atof(szData);

		strKey.Format("RESOLUTION_Y");
		strData.Format("%f", m_pMain->vt_viewer_info[i].resolution_y);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_dResolutionY[i] = atof(szData);*/

		strKey.Format("VIEWER_POS");
		strData.Format("%d", m_pMain->vt_viewer_info[i].viewer_position);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_nViewerPos[i] = atoi(szData);

		strKey.Format("CAMERA_INDEX");
		strData.Format("%d", m_pMain->vt_viewer_info[i].camera_index);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_nCameraIdx[i] = atoi(szData);

	}

}

//camera 읽어오기
void DlgINISetting::read_camera_config()
{
	CString strKey, strData, strSection;
	TCHAR szData[MAX_PATH] = { 0, };
	
	CString strFilePath;
	strFilePath.Format(m_pMain->m_iniCameraFile.GetPathName());

	for (int i = 0; i < MAX_CAMERA; i++)
	{
		strSection.Format("CAMERA%d_INFO", i + 1);

		strKey.Format("NAME");
		strData.Format("%s", m_pMain->m_stCamInfo[i].cName);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_strCamName[i].Format("%s", szData);

		strKey.Format("SERIAL");
		strData.Format("%s", m_pMain->m_stCamInfo[i].cSerial);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_nCamSerial[i].Format("%s", szData);

		strKey.Format("WIDTH");
		strData.Format("%d", m_pMain->m_stCamInfo[i].w);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_nCamWidth[i] = atoi(szData);

		strKey.Format("HEIGHT");
		strData.Format("%d", m_pMain->m_stCamInfo[i].h);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_nCamHeight[i] = atoi(szData);

		strKey.Format("DEPTH");
		strData.Format("%d", m_pMain->m_stCamInfo[i].depth);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_nCamDepth[i] = atoi(szData);

		strKey.Format("SIZE");
		strData.Format("%d", m_pMain->m_stCamInfo[i].size);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_nCamSize[i] = atoi(szData);

		strKey.Format("FLIP_DIR");
		strData.Format("%d", m_pMain->m_stCamInfo[i].flip_dir);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_nFlipDir[i] = atoi(szData);
	}
}

//LET_AlignClient.ini 읽어오기
void DlgINISetting::read_client_config()
{
	CString strKey, strData, strSection;
	TCHAR szData[MAX_PATH] = { 0, };
	
	CString strFilePath;
	strFilePath.Format(m_pMain->m_iniFile.GetPathName());

	//server info
	strSection = "SERVER_INFO";

	strKey.Format("CLIENT_NAME");
	strData.Format("%s", m_pMain->m_strClientName);
	::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
	m_stClientName.Format(_T("%s"), szData);

	strKey.Format("CLIENT_ID");
	strData.Format("%d", m_pMain->m_nClientID);
	::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
	m_nClientId = atoi(szData);

	strKey.Format("PLC_IP");
	strData.Format("%s", m_pMain->m_strPlcIP);
	::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
	m_strPlcIp.Format(_T("%s"), szData);

	strKey.Format("PLC_PORT");
	strData.Format("%d", m_pMain->m_nSockPlcPort);
	::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
	m_nPlcPort = atoi(szData);

	strKey.Format("CLIENT_COM_FOLDER");
	strData.Format("%s", m_pMain->m_strComFolerName);
	::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
	m_stFolderName.Format(_T("%s"), szData);

	strKey.Format("NUM_OF_JOB_COUNT");
	strData.Format("%d", m_pMain->vt_job_info.size());
	::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
	m_nJobCount = atoi(szData);

	strKey.Format("DUMMY_START_POSX");
	strData.Format("%d", m_pMain->m_dDummyStart_posXY.x);
	::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
	m_nDummyStartX = atoi(szData);

	strKey.Format("DUMMY_START_POSY");
	strData.Format("%d", m_pMain->m_dDummyStart_posXY.y);
	::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
	m_nDummyStartY = atoi(szData);

	strKey.Format("DUMMY_INSP_HEIGHT");
	strData.Format("%d", m_pMain->m_dDummyInspRangeHeight);
	::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
	m_nDummyInspHeight = atoi(szData);

	// Tkyuha 파라미터로 변경됨
	//strKey.Format("USE_ELB_NOZZLE_TO_PANELDIST_CHECK"); 
	//strData.Format("%d", m_pMain->m_bCheckNozzleToPanelDistance);
	//::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
	//m_bUseElbNozzleToPaneldistCheck = atoi(szData);


	SetDlgItemInt(IDC_EDIT_JOB_COUNT_MODIFY, (int)m_pMain->vt_job_info.size());
	vJobCount.push_back((int)m_pMain->vt_job_info.size());

	m_cb_select_job.ResetContent();

	CString strJob = "All Job";
	m_cb_select_job.AddString(strJob);

	for (int i = 0; i < m_pMain->vt_job_info.size(); i++)
	{
		strJob.Format("Job %d번", i + 1);
		m_cb_select_job.AddString(strJob);
		m_cb_select_job.SetCurSel(0);
	}

	//light info
	strSection = "LIGHT_CTRL_INFO";

	SetDlgItemInt(IDC_EDIT_LIGHT_CONTROL_MODIFY, m_pMain->m_stLightCtrlInfo.nNumOfLightCtrl);
	m_nNumOfLightControl = m_pMain->m_stLightCtrlInfo.nNumOfLightCtrl;

	m_nNumOfCamera = m_pMain->m_nNumCamera;
	SetDlgItemInt(IDC_EDIT_CAMERA_COUNT, m_pMain->m_nNumCamera);

	for (int i = 0; i < MAX_LIGHT_CTRL; i++)
	{
		strKey.Format("TYPE%d", i + 1);
		strData.Format("%d", m_pMain->m_stLightCtrlInfo.nType[i]);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_nLightType[i] = atoi(szData);

		strKey.Format("PORT%d", i + 1);
		strData.Format("%d", m_pMain->m_stLightCtrlInfo.nPort[i]);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_nLightPort[i] = atoi(szData);

		strKey.Format("BAUDRATE%d", i + 1);
		strData.Format("%d", m_pMain->m_stLightCtrlInfo.nBaudrate[i]);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_nLightBaudrate[i] = atoi(szData);

		strKey.Format("SOCK_PORT%d", i + 1);
		strData.Format("%d", m_pMain->m_stLightCtrlInfo.nSockPort[i]);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_nLightSockPort[i] = atoi(szData);

		strKey.Format("SOCK_IP%d", i + 1);
		strData.Format("%s", m_pMain->m_stLightCtrlInfo.cSockIP[i]);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_strLightSockIp[i].Format(_T("%s"), szData);

		strKey.Format("NAME%d", i + 1);
		strData.Format("%s", m_pMain->m_stLightCtrlInfo.cName[i]);
		::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
		m_stLightName[i].Format(_T("%s"), szData);
	}
}

//PLC Adress ini 읽어오기
void DlgINISetting::read_plc_address_config()
{
	CString strKey, strData, strSection;
	TCHAR szData[MAX_PATH] = { 0, };

	CString strFilePath= m_pMain->m_strSystemDir + "PLCAddress.ini";
	
	//set_param
	strSection = "SET_PARAM";

	strKey.Format("START_READ_BIT_ADDRESS");
	strData.Format("%d", m_pMain->m_nStartReadBitAddr_Org);
	::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
	m_nStartReadBitAddress = atoi(szData);

	strKey.Format("START_WRITE_BIT_ADDRESS");
	strData.Format("%d", m_pMain->m_nStartWriteBitAddr_Org);
	::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
	m_nStartWriteBitAddress = atoi(szData);

	strKey.Format("START_READ_WORD_ADDRESS");
	strData.Format("%d", m_pMain->m_nStartReadWordAddr);
	::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
	m_nStartReadWordAddress = atoi(szData);

	strKey.Format("START_WRITE_WORD_ADDRESS");
	strData.Format("%d", m_pMain->m_nStartWriteWordAddr);
	::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
	m_nStartWriteWordAddress = atoi(szData);

	strKey.Format("SIZE_READ_BIT_ADDRESS");
	strData.Format("%d", m_pMain->m_nSizeReadBit_Org);
	::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
	m_nSizeReadBitAddress = atoi(szData);

	strKey.Format("SIZE_READ_WORD_ADDRESS");
	strData.Format("%d", m_pMain->m_nSizeReadWord);
	::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
	m_nSizeReadWordAddress = atoi(szData);

	strKey.Format("SIZE_WRITE_BIT_ADDRESS");
	strData.Format("%d", m_pMain->m_nSizeWriteBit_Org);
	::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
	m_nSizeWriteBitAddress = atoi(szData);

	strKey.Format("SIZE_WRITE_WORD_ADDRESS");
	strData.Format("%d", m_pMain-> m_nSizeWriteWord);
	::GetPrivateProfileString(strSection, strKey, strData, szData, MAX_PATH, strFilePath);
	m_nSizeWriteWordAddress = atoi(szData);
}

//저장 버튼
void DlgINISetting::OnBnClickedBtnIniSettingSave()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	if (m_pMain->fnSetMessage(2, "Do You Save the Data?") != TRUE)	return;

	for (int i = 0; i < m_nJobCount - 1; i++)
	{
		if (strcmp(m_strJobName[i], m_strJobName[i + 1]) == 0)
		{
			AfxMessageBox(_T("Duplicate JOB name."));
			return;
		}
	}

	int nCamCount = GetDlgItemInt(IDC_EDIT_CAMERA_COUNT);

	if (nCamCount > MAX_CAMERA || nCamCount < 0)
	{
		AfxMessageBox(_T("Out of input range! (Range : 0~8)"));
		return;
	}
	else
	{
		CSimpleSplashWnd _splash(this, IDB_BMP_LOADING);

		_splash.SetStatusText(_T("	Save Job INI Data.."), 10);
		save_job_config();

		_splash.SetStatusText(_T("  Save Viewer INI Data.."), 30);
		save_view_config();

		_splash.SetStatusText(_T("  Save Camera INI Data.."), 50);
		save_camera_config();

		_splash.SetStatusText(_T("  Save Client INI Data.."), 70);
		save_client_config();

		_splash.SetStatusText(_T("  Save Plc Address INI Data.."), 100);
		save_plc_address_config();
	}
}

//GRID 초기화
void DlgINISetting::init_grid_view_setting(CGridCtrl &grid, int col, int row)
{
	BOOL bVirtualMode = FALSE;

	grid.SetEditable(FALSE);
	//grid.SetEditable(TRUE);
	grid.SetVirtualMode(bVirtualMode);
	grid.SetListMode(TRUE);
	grid.EnableDragAndDrop(FALSE);
	grid.EnableSelection(FALSE);
	grid.SetFixedRowSelection(TRUE);
	grid.SetFixedColumnSelection(TRUE);
	grid.SetFrameFocusCell(FALSE);
	grid.SetTrackFocusCell(FALSE);
	grid.SetRowResize(FALSE);
	grid.SetColumnResize(FALSE);

	grid.SetRowCount(row);
	grid.SetColumnCount(col);
	grid.SetFixedRowCount(1);

	grid.SetRowHeight(0, 25);
	grid.SetBkColor(RGB(197, 197, 187));
	grid.SetTextBkColor(RGB(235, 235, 235));

	for (int i = 0; i < grid.GetColumnCount(); i++)
	{
		grid.SetItemBkColour(0, i, RGB(146, 146, 141));
		grid.SetItemFgColour(0, i, RGB(255, 255, 255));
	}

	grid.AutoSizeColumns();
	grid.ExpandColumnsToFit();
}

//VIEWER DRAW GRID
void DlgINISetting::draw_grid_view_name(CGridCtrl &grid, int col, int row)
{

	DWORD dwTextStyle = DT_CENTER | DT_VCENTER | DT_SINGLELINE;    // Text Style
	int nRowCount = 1;
	int i = m_cbSelectViewer.GetCurSel();

	for (int row = 0; row < grid.GetRowCount(); row++)
	{
		for (int col = 0; col < grid.GetColumnCount(); col++)
		{
			GV_ITEM Item;
			Item.mask = GVIF_TEXT | GVIF_FORMAT;
			Item.row = row;
			Item.col = col;

			// Fixed Cell Title
			if (row == 0 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Index"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row == 0 && col == 1)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Name"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row == 0 && col == 2)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Width"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col == 3)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Height"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col == 4)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Viewer Pos Num"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			/*else if (row == 0 && col == 5)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Resolution X"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col == 6)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Resolution Y"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}*/
			else if (row == 0 && col == 5)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Use PopUp"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col == 6)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Cam Index"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else
			{
				Item.nFormat = dwTextStyle;
			}
			if (i == 0)
			{
				for (int k = 0; k < m_nNumOfViewer; k++)
				{
					if (row == k + 1 && col == 0)
					{
						Item.nFormat = dwTextStyle;
						Item.strText.Format(_T("%d"), row);
						UINT state = grid.GetItemState(row, col);
						grid.SetItemState(row, col, state | GVIS_READONLY);
					}
					else if (row == k + 1 && col == 1)
					{
						Item.nFormat = dwTextStyle;
						Item.strText.Format(_T("%s"), m_strName[k]);
						UINT state = grid.GetItemState(row, col);
						grid.SetItemState(row, col, state | GVIS_READONLY);
					}
					else if (row == k + 1 && col == 2)
					{
						Item.nFormat = dwTextStyle;
						Item.strText.Format(_T("%d"), m_nWidth[k]);
						UINT state = grid.GetItemState(row, col);
						grid.SetItemState(row, col, state | GVIS_READONLY);
					}
					else if (row == k + 1 && col == 3)
					{
						Item.nFormat = dwTextStyle;
						Item.strText.Format(_T("%d"), m_nHeight[k]);
						UINT state = grid.GetItemState(row, col);
						grid.SetItemState(row, col, state | GVIS_READONLY);
					}
					else if (row == k + 1 && col == 4)
					{
						Item.nFormat = dwTextStyle;
						Item.strText.Format(_T("%d"), m_nViewerPos[k]);
						UINT state = grid.GetItemState(row, col);
						grid.SetItemState(row, col, state | GVIS_READONLY);
					}
					/*else if (row == k + 1 && col == 5)
					{
						Item.nFormat = dwTextStyle;
						Item.strText.Format(_T("%f"), m_dResolutionX[k]);
						UINT state = grid.GetItemState(row, col);
						grid.SetItemState(row, col, state | GVIS_READONLY);
					}
					else if (row == k + 1 && col == 6)
					{
						Item.nFormat = dwTextStyle;
						Item.strText.Format(_T("%f"), m_dResolutionY[k]);
						UINT state = grid.GetItemState(row, col);
						grid.SetItemState(row, col, state | GVIS_READONLY);
					}*/
					else if (row == k + 1 && col == 5)
					{
						Item.nFormat = dwTextStyle;
						Item.strText.Format(_T("%d"), m_nPopup[k]);
						UINT state = grid.GetItemState(row, col);
						grid.SetItemState(row, col, state | GVIS_READONLY);
					}
					else if (row == k + 1 && col == 6)
					{
						Item.nFormat = dwTextStyle;
						Item.strText.Format(_T("%d"), m_nCameraIdx[k]);
						UINT state = grid.GetItemState(row, col);
						grid.SetItemState(row, col, state | GVIS_READONLY);
					}
				}
			}
			else
			{
				if (row != 0 && col == 0)
				{
					Item.nFormat = dwTextStyle;
					Item.strText.Format(_T("%d"), i);
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_READONLY);
				}
				else if (row != 0 && col == 1)
				{
					Item.nFormat = dwTextStyle;
					Item.strText.Format(_T("%s"), m_strName[i - 1]);
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_READONLY);
				}
				else if (row != 0 && col == 2)
				{
					Item.nFormat = dwTextStyle;
					Item.strText.Format(_T("%d"), m_nWidth[i - 1]);
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_READONLY);
				}
				else if (row != 0 && col == 3)
				{
					Item.nFormat = dwTextStyle;
					Item.strText.Format(_T("%d"), m_nHeight[i - 1]);
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_READONLY);
				}
				else if (row != 0 && col == 4)
				{
					Item.nFormat = dwTextStyle;
					Item.strText.Format(_T("%d"), m_nViewerPos[i - 1]);
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_READONLY);
				}
				/*else if (row != 0 && col == 5)
				{
					Item.nFormat = dwTextStyle;
					Item.strText.Format(_T("%f"), m_dResolutionX[i - 1]);
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_READONLY);
				}
				else if (row != 0 && col == 6)
				{
					Item.nFormat = dwTextStyle;
					Item.strText.Format(_T("%f"), m_dResolutionY[i - 1]);
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_READONLY);
				}*/
				else if (row != 0 && col == 5)
				{
					Item.nFormat = dwTextStyle;
					Item.strText.Format(_T("%d"), m_nPopup[i - 1]);
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_READONLY);
				}
				else if (row != 0 && col == 6)
				{
					Item.nFormat = dwTextStyle;
					Item.strText.Format(_T("%d"), m_nCameraIdx[i - 1]);
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_READONLY);
				}
			}
			if (row > 0)
			{
				if (nRowCount <= m_nNumOfViewer)
					grid.SetItemBkColour(row, col, RGB(235, 235, 235));
				else
					grid.SetItemBkColour(row, col, RGB(255, 255, 255));

				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state);
			}

			grid.SetItem(&Item);

		}

		if (row > 0)
			nRowCount++;

		if (nRowCount > 10)
			nRowCount = 1;

		grid.RedrawRow(row);

	}

	grid.AutoSizeColumns();
	grid.ExpandColumnsToFit();
}

//job light grid draw
void DlgINISetting::draw_grid_job_light_data(CGridCtrl &grid, int col, int row)
{
	DWORD dwTextStyle = DT_CENTER | DT_VCENTER | DT_SINGLELINE;    // Text Style
	int nRowCount = 1;
	int i = m_cb_select_job.GetCurSel();
	int j = m_cbSelectLightCam.GetCurSel();

	for (int row = 0; row < grid.GetRowCount(); row++)
	{
		for (int col = 0; col < grid.GetColumnCount(); col++)
		{
			GV_ITEM Item;
			Item.mask = GVIF_TEXT | GVIF_FORMAT;
			Item.row = row;
			Item.col = col;

			// Fixed Cell Title
			if (row == 0 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Channel Name"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col == 1)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Channel Id"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col == 2)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Control Id"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else
			{
				Item.nFormat = dwTextStyle;
			}

			for (int n = 0; n < m_nNumOfUsingLightCount[i - 1][j]; n++)
			{
				if (row == n + 1 && col == 0)
				{
					Item.nFormat = dwTextStyle;
					Item.strText.Format(_T("%s"), m_stLightChannelName[i - 1][j][n]);
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_READONLY);
				}
				else if (row == n + 1 && col == 1)
				{
					Item.nFormat = dwTextStyle;
					Item.strText.Format(_T("%d"), m_nLightChannelId[i - 1][j][n]);
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_READONLY);
				}
				else if (row == n + 1 && col == 2)
				{
					Item.nFormat = dwTextStyle;
					Item.strText.Format(_T("%d"), m_nLightControlId[i - 1][j][n]);
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_READONLY);
				}

			}

			if (row > 0)
			{
				if (nRowCount <= m_nNumOfUsingLightCount[i - 1][j])
					grid.SetItemBkColour(row, col, RGB(235, 235, 235));
				else
					grid.SetItemBkColour(row, col, RGB(255, 255, 255));

				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state);
			}

			grid.SetItem(&Item);

		}

		if (row > 0)
			nRowCount++;

		if (nRowCount > 10)
			nRowCount = 1;

		grid.RedrawRow(row);
	}

	grid.AutoSizeColumns();
	grid.ExpandColumnsToFit();
}

//job plc grid draw
void DlgINISetting::draw_grid_plc_data(CGridCtrl &grid, int col, int row)
{

	DWORD dwTextStyle = DT_CENTER | DT_VCENTER | DT_SINGLELINE;    // Text Style
	int nRowCount = 1;
	
	int i = m_cb_select_job.GetCurSel();
	
	for (int row = 0; row < grid.GetRowCount(); row++)
	{
		for (int col = 0; col < grid.GetColumnCount(); col++)
		{
			GV_ITEM Item;
			Item.mask = GVIF_TEXT | GVIF_FORMAT;
			Item.row = row;
			Item.col = col;

			// Fixed Cell Title
			if (row == 0 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Name"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			if (row == 0 && col == 1)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Value"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			if (row == 1 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Read Bit Start Address"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 2 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Write Bit Start Address"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 3 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Read Word Start Address"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 4 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Write Word Start Address"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 5 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Use Main Object Id"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 6 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Main Object Id Start Address"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 7 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Use Sub Object Id"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 8 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Sub Object Id Start Address"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 1 && col == 1)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format("%d", m_nJobReadBitStart[i - 1]);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row == 2 && col == 1)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("%d"), m_nJobWriteBitStart[i - 1]);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row == 3 && col == 1)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format("%d", m_nJobReadWordStart[i - 1]);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row == 4 && col == 1)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format("%d", m_nJobWriteWordStart[i - 1]);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row == 5 && col == 1)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format("%d", m_nJobUseMainObjectID[i - 1]);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row == 6 && col == 1)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format("%d", m_nJobMainObjectIdStart[i - 1]);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row == 7 && col == 1)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format("%d", m_nJobUseSubObjectID[i - 1]);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row == 8 && col == 1)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format("%d", m_nJobSubObjectIdStart[i - 1]);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else
			{
				Item.nFormat = dwTextStyle;
			}


			if (row > 0)
			{
				if (nRowCount < 9)
					grid.SetItemBkColour(row, col, RGB(235, 235, 235));
				else
					grid.SetItemBkColour(row, col, RGB(255, 255, 255));

				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state);
			}

			grid.SetItem(&Item);
		}

		if (row > 0)
			nRowCount++;

		if (nRowCount > 10)
			nRowCount = 1;

		grid.RedrawRow(row);
	}
	grid.AutoSizeColumns();
	grid.ExpandColumnsToFit(); 
	grid.ExpandRowsToFit();
}

//JOB DRAW GRID
void DlgINISetting::draw_grid_job(CGridCtrl &grid, int col, int row)
{
	DWORD dwTextStyle = DT_CENTER | DT_VCENTER | DT_SINGLELINE;    // Text Style
	int nRowCount = 1;
	int ncbJob = m_cb_select_job.GetCurSel();

	for (int row = 0; row < grid.GetRowCount(); row++)
	{
		for (int col = 0; col < grid.GetColumnCount(); col++)
		{
			GV_ITEM Item;
			Item.mask = GVIF_TEXT | GVIF_FORMAT;
			Item.row = row;
			Item.col = col;

			// Fixed Cell Title
			if (row == 0 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Job Count"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row == 0 && col == 1)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Job Name"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col == 2)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Job Method"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col == 3)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Camera Count"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col == 4)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Viewer Count"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else
			{
				Item.nFormat = dwTextStyle;
			}
			if (ncbJob != 0)
			{
				if (row != 0 && col == 0)
				{
					Item.nFormat = dwTextStyle;
					Item.strText.Format(_T("%d"), ncbJob);
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_READONLY);
				}
				else if (row != 0 && col == 1)
				{

					Item.nFormat = dwTextStyle;
					Item.strText.Format(_T("%s"), m_strJobName[ncbJob - 1]);
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_READONLY);
				}
				else if (row != 0 && col == 2)
				{
					Item.nFormat = dwTextStyle;
					Item.strText.Format(_T("%d"), m_nJobMethod[ncbJob - 1]);
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_READONLY);
				}
				else if (row != 0 && col == 3)
				{
					Item.nFormat = dwTextStyle;
					Item.strText.Format(_T("%d"), m_nJobCamCount[ncbJob - 1]);
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_READONLY);
				}
				else if (row != 0 && col == 4)
				{
					Item.nFormat = dwTextStyle;
					Item.strText.Format(_T("%d"), m_nJobViewerCount[ncbJob - 1]);
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_READONLY);
				}
			}
			else
			{
				for (int k = 0; k < m_nJobCount; k++)
				{
					if (row == k + 1 && col == 0)
					{
						Item.nFormat = dwTextStyle;
						Item.strText.Format(_T("%d"), row);
						UINT state = grid.GetItemState(row, col);
						grid.SetItemState(row, col, state | GVIS_READONLY);
					}
					else if (row == k + 1 && col == 1)
					{

						Item.nFormat = dwTextStyle;
						Item.strText.Format(_T("%s"), m_strJobName[k]);
						UINT state = grid.GetItemState(row, col);
						grid.SetItemState(row, col, state | GVIS_READONLY);
					}
					else if (row == k + 1 && col == 2)
					{
						Item.nFormat = dwTextStyle;
						Item.strText.Format(_T("%d"), m_nJobMethod[k]);
						UINT state = grid.GetItemState(row, col);
						grid.SetItemState(row, col, state | GVIS_READONLY);
					}
					else if (row == k + 1 && col == 3)
					{
						Item.nFormat = dwTextStyle;
						Item.strText.Format(_T("%d"), m_nJobCamCount[k]);
						UINT state = grid.GetItemState(row, col);
						grid.SetItemState(row, col, state | GVIS_READONLY);
					}
					else if (row == k + 1 && col == 4)
					{
						Item.nFormat = dwTextStyle;
						Item.strText.Format(_T("%d"), m_nJobViewerCount[k]);
						UINT state = grid.GetItemState(row, col);
						grid.SetItemState(row, col, state | GVIS_READONLY);
					}
				}
			}

			if (row > 0)
			{
				if (nRowCount <= m_nJobCount)
					grid.SetItemBkColour(row, col, RGB(235, 235, 235));
				else
					grid.SetItemBkColour(row, col, RGB(255, 255, 255));

				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state);
			}

			grid.SetItem(&Item);

		}

		if (row > 0)
			nRowCount++;

		if (nRowCount > 10)
			nRowCount = 1;

		grid.RedrawRow(row);
	}
	grid.AutoSizeColumns();
	grid.ExpandColumnsToFit();
}

//job camerae grid
void DlgINISetting::draw_grid_job_cam(CGridCtrl &grid, int col, int row)
{
	DWORD dwTextStyle = DT_CENTER | DT_VCENTER | DT_SINGLELINE;    // Text Style
	int nRowCount = 1;
	int nSelectJob = m_cb_select_job.GetCurSel();
	int nIndex = m_nJobCamCount[nSelectJob - 1];

	for (int row = 0; row < grid.GetRowCount(); row++)
	{
		for (int col = 0; col < grid.GetColumnCount(); col++)
		{
			GV_ITEM Item;
			Item.mask = GVIF_TEXT | GVIF_FORMAT;
			Item.row = row;
			Item.col = col;

			// Fixed Cell Title
			if (row == 0 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Cam Index"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}

			else if (row == 0 && col == 1)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Cam Index Num"));
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else
			{
				Item.nFormat = dwTextStyle;
			}

			if (row != 0 && col == 0)
			{

				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("%d"), row);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row != 0 && col == 1)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("%d"), m_nJobCamIndex[nSelectJob - 1][row - 1]);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}


			if (row > 0)
			{
				if (nRowCount <= nIndex)
					grid.SetItemBkColour(row, col, RGB(235, 235, 235));
				else
					grid.SetItemBkColour(row, col, RGB(255, 255, 255));

				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state);
			}
			grid.SetItem(&Item);

		}

		if (row > 0)
			nRowCount++;

		if (nRowCount > 10)
			nRowCount = 1;

		grid.RedrawRow(row);
	}
	grid.AutoSizeColumns();
	grid.ExpandColumnsToFit();
}

//job machine draw
void DlgINISetting::draw_grid_job_machine_viewer(CGridCtrl &grid, int col, int row)
{
	DWORD dwTextStyle = DT_CENTER | DT_VCENTER | DT_SINGLELINE;    // Text Style
	int nRowCount = 1;
	int nSelectJob = m_cb_select_job.GetCurSel();
	int nRow = m_nMachineViewCountX[nSelectJob - 1] * m_nMachineViewCountY[nSelectJob - 1];

	for (int row = 0; row < grid.GetRowCount(); row++)
	{
		for (int col = 0; col < grid.GetColumnCount(); col++)
		{
			GV_ITEM Item;
			Item.mask = GVIF_TEXT | GVIF_FORMAT;
			Item.row = row;
			Item.col = col;

			// Fixed Cell Title

			if (row == 0 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Machine View Index"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col == 1)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Value"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row != 0 && col == 0)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("%d"), row);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row != 0 && col == 1)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("%d"), m_nMachineViewIndex[nSelectJob - 1][row - 1]);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else
			{
				Item.nFormat = dwTextStyle;
			}

			if (row > 0)
			{
				if (nRowCount <= nRow)
					grid.SetItemBkColour(row, col, RGB(235, 235, 235));
				else
					grid.SetItemBkColour(row, col, RGB(255, 255, 255));

				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state);
			}
			grid.SetItem(&Item);

		}

		if (row > 0)
			nRowCount++;

		if (nRowCount > 10)
			nRowCount = 1;

		grid.RedrawRow(row);
	}
	grid.AutoSizeColumns();
	grid.ExpandColumnsToFit();
}

//job main draw
void DlgINISetting::draw_grid_job_main_viewer(CGridCtrl &grid, int col, int row)
{
	DWORD dwTextStyle = DT_CENTER | DT_VCENTER | DT_SINGLELINE;    // Text Style
	int nRowCount = 1;
	int nSelectJob = m_cb_select_job.GetCurSel();
	int nRow = m_nMainViewCountX[nSelectJob - 1] * m_nMainViewCountY[nSelectJob - 1];

	for (int row = 0; row < grid.GetRowCount(); row++)
	{
		for (int col = 0; col < grid.GetColumnCount(); col++)
		{
			GV_ITEM Item;
			Item.mask = GVIF_TEXT | GVIF_FORMAT;
			Item.row = row;
			Item.col = col;

			// Fixed Cell Title

			if (row == 0 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Main View Index"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col == 1)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Value"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row != 0 && col == 0)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("%d"), row);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row != 0 && col == 1)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("%d"), m_nMainViewIndex[nSelectJob - 1][row - 1]);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else
			{
				Item.nFormat = dwTextStyle;
			}

			if (row > 0)
			{
				if (nRowCount <= nRow)
					grid.SetItemBkColour(row, col, RGB(235, 235, 235));
				else
					grid.SetItemBkColour(row, col, RGB(255, 255, 255));

				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state);
			}
			grid.SetItem(&Item);

		}

		if (row > 0)
			nRowCount++;

		if (nRowCount > 10)
			nRowCount = 1;

		grid.RedrawRow(row);
	}
	grid.AutoSizeColumns();
	grid.ExpandColumnsToFit();
}

//job position draw
void DlgINISetting::draw_grid_position_viewer(CGridCtrl &grid, int col, int row)
{
	DWORD dwTextStyle = DT_CENTER | DT_VCENTER | DT_SINGLELINE;    // Text Style
	int nRowCount = 1;
	int nSelectJob = m_cb_select_job.GetCurSel();
	int nPosition = m_nJobCamCount[nSelectJob - 1] * m_nJobPosition[nSelectJob - 1];
	

	for (int row = 0; row < grid.GetRowCount(); row++)
	{
		for (int col = 0; col < grid.GetColumnCount(); col++)
		{
			GV_ITEM Item;
			Item.mask = GVIF_TEXT | GVIF_FORMAT;
			Item.row = row;
			Item.col = col;

			// Fixed Cell Title

			if (row == 0 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Cam - Pos"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col == 1)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Name"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else
			{
				Item.nFormat = dwTextStyle;
			}

			for (int j = 0; j < m_nJobCamCount[nSelectJob - 1]; j++)
			{
				for (int k = 0; k < m_nJobPosition[nSelectJob - 1]; k++)
				{
					int offset = 0;

					if (j != 0)
					{
						offset += j*m_nJobPosition[nSelectJob - 1];
					}

					if (row == offset + k + 1 && col == 0)
					{
						Item.nFormat = dwTextStyle;
						Item.strText.Format(_T("%d - %d"), j + 1, k + 1);
						UINT state = grid.GetItemState(row, col);
						grid.SetItemState(row, col, state | GVIS_READONLY);
					}

					else if (row == offset + k + 1 && col == 1)
					{
						Item.nFormat = dwTextStyle;
						Item.strText.Format(_T("%s"), m_stPosName[nSelectJob - 1][j][k]);
						UINT state = grid.GetItemState(row, col);
						grid.SetItemState(row, col, state | GVIS_READONLY);
					}
				}
			}

			if (row > 0)
			{

				if (nRowCount <= nPosition)
					grid.SetItemBkColour(row, col, RGB(235, 235, 235));
				else
					grid.SetItemBkColour(row, col, RGB(255, 255, 255));

				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state);
			}
			grid.SetItem(&Item);

		}

		if (row > 0)
			nRowCount++;

		if (nRowCount > 10)
			nRowCount = 1;

		grid.RedrawRow(row);
	}
	grid.AutoSizeColumns();
	grid.ExpandColumnsToFit();
}

//job view draw
void DlgINISetting::draw_grid_job_view_viewer(CGridCtrl &grid, int col, int row)
{
	DWORD dwTextStyle = DT_CENTER | DT_VCENTER | DT_SINGLELINE;    // Text Style
	int nRowCount = 1;
	int nSelectJob = m_cb_select_job.GetCurSel();
	int nIndex = m_nJobViewerCount[nSelectJob - 1];

	for (int row = 0; row < grid.GetRowCount(); row++)
	{
		for (int col = 0; col < grid.GetColumnCount(); col++)
		{
			GV_ITEM Item;
			Item.mask = GVIF_TEXT | GVIF_FORMAT;
			Item.row = row;
			Item.col = col;

			// Fixed Cell Title

			if (row == 0 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Viewer Index"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col == 1)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Viewer Index Num"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row != 0 && col == 0)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("%d"), row);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row != 0 && col == 1)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("%d"), m_nJobViewerIndex[nSelectJob - 1][row - 1]);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else
			{
				Item.nFormat = dwTextStyle;
			}

			if (row > 0)
			{

				if (nRowCount <= nIndex)
					grid.SetItemBkColour(row, col, RGB(235, 235, 235));
				else
					grid.SetItemBkColour(row, col, RGB(255, 255, 255));

				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state);
			}
			grid.SetItem(&Item);

		}

		if (row > 0)
			nRowCount++;

		if (nRowCount > 10)
			nRowCount = 1;

		grid.RedrawRow(row);
	}
	grid.AutoSizeColumns();
	grid.ExpandColumnsToFit();
}

//job light index name draw
void DlgINISetting::draw_grid_job_index_name_viewer(CGridCtrl &grid, int col, int row)
{
	DWORD dwTextStyle = DT_CENTER | DT_VCENTER | DT_SINGLELINE;    // Text Style
	int nRowCount = 1;
	int nSelectJob = m_cb_select_job.GetCurSel();
	int nSelectLight = m_cbSelectLightCam.GetCurSel();
	
	for (int row = 0; row < grid.GetRowCount(); row++)
	{
		for (int col = 0; col < grid.GetColumnCount(); col++)
		{
			GV_ITEM Item;
			Item.mask = GVIF_TEXT | GVIF_FORMAT;
			Item.row = row;
			Item.col = col;

			// Fixed Cell Title

			if (row == 0 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Light Name Index"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col == 1)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Name"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row != 0 && col == 0)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("%d"), row);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else
			{
				Item.nFormat = dwTextStyle;
			}

			for (int k = 0; k < MAX_LIGHT_INDEX; k++)
			{
				if (row == k + 1 && col == 1)
				{
					Item.nFormat = dwTextStyle;
					Item.strText.Format(_T("%s"), m_stLightIndexName[nSelectJob - 1][nSelectLight][k]);
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_READONLY);
				}
			}

			if (row > 0)
			{
				if (nRowCount <= MAX_LIGHT_INDEX)
					grid.SetItemBkColour(row, col, RGB(235, 235, 235));
				else
					grid.SetItemBkColour(row, col, RGB(255, 255, 255));

				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state);
			}
			grid.SetItem(&Item);

		}

		if (row > 0)
			nRowCount++;

		if (nRowCount > 10)
			nRowCount = 1;

		grid.RedrawRow(row);
	}
	grid.AutoSizeColumns();
	grid.ExpandColumnsToFit();
}

//job caliper draw
void DlgINISetting::draw_grid_job_caliper_viewer(CGridCtrl &grid, int col, int row)
{
	DWORD dwTextStyle = DT_CENTER | DT_VCENTER | DT_SINGLELINE;    // Text Style
	int nRowCount = 1;
	int nSelectJob = m_cb_select_job.GetCurSel();
	int nCamera = m_cbSelectCamera.GetCurSel();
	int nPos = m_cbSelectPos.GetCurSel();

	for (int row = 0; row < grid.GetRowCount(); row++)
	{
		for (int col = 0; col < grid.GetColumnCount(); col++)
		{
			GV_ITEM Item;
			Item.mask = GVIF_TEXT | GVIF_FORMAT;
			Item.row = row;
			Item.col = col;

			// Fixed Cell Title

			if (row == 0 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Pos - Index"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col == 1)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Caliper Name"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else
			{
				Item.nFormat = dwTextStyle;
			}

			for (int j = 0; j < MAX_CALIPER_LINES; j++)
			{
				if (row == j + 1 && col == 0)
				{
					Item.nFormat = dwTextStyle;
					Item.strText.Format(_T("%d - %d"), nPos + 1, j + 1);
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_READONLY);

				}
				if (row == j + 1 && col == 1)
				{
					Item.nFormat = dwTextStyle;
					Item.strText.Format(_T("%s"), m_stCaliperName[nSelectJob - 1][nCamera][nPos][j]);
					UINT state = grid.GetItemState(row, col);
					grid.SetItemState(row, col, state | GVIS_READONLY);
				}
			}

			if (row > 0)
			{

				if (nRowCount <= MAX_CALIPER_LINES)
					grid.SetItemBkColour(row, col, RGB(235, 235, 235));
				else
					grid.SetItemBkColour(row, col, RGB(255, 255, 255));

				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state);
			}
			grid.SetItem(&Item);

		}

		if (row > 0)
			nRowCount++;

		if (nRowCount > 10)
			nRowCount = 1;

		grid.RedrawRow(row);
	}
	grid.AutoSizeColumns();
	grid.ExpandColumnsToFit();
	grid.ExpandRowsToFit();
}

//CAMERA DRAW CHART
void DlgINISetting::draw_grid_camera_viewer(CGridCtrl &grid, int col, int row)
{
	DWORD dwTextStyle = DT_CENTER | DT_VCENTER | DT_SINGLELINE;    // Text Style
	int nRowCount = 1;

	for (int row = 0; row < grid.GetRowCount(); row++)
	{
		for (int col = 0; col < grid.GetColumnCount(); col++)
		{
			GV_ITEM Item;
			Item.mask = GVIF_TEXT | GVIF_FORMAT;
			Item.row = row;
			Item.col = col;

			// Fixed Cell Title
			if (row == 0 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Index"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row == 0 && col == 1)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Name"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row == 0 && col == 2)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Serial"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col == 3)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Width"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col ==4)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Height"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col == 5)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("FLIP DIR"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row != 0 && col == 0)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("%d"),row);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row != 0 && col == 1)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("%s"), m_strCamName[row - 1]);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row != 0 && col == 2)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("%s"), m_nCamSerial[row - 1]);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row != 0 && col == 3)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("%d"), m_nCamWidth[row - 1]);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row != 0 && col == 4)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("%d"), m_nCamHeight[row - 1]);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row != 0 && col == 5)
			{
				Item.nFormat = dwTextStyle;
				m_nCamSize[row - 1] = m_nCamWidth[row - 1] * m_nCamHeight[row - 1];
				Item.strText.Format(_T("%d"), m_nFlipDir[row - 1]);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else
			{
				Item.nFormat = dwTextStyle;
			}

			if (row > 0)
			{
				if (nRowCount <= m_nNumOfCamera)
					grid.SetItemBkColour(row, col, RGB(235, 235, 235));
				else
					grid.SetItemBkColour(row, col, RGB(255, 255, 255));

				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state);
			}

			grid.SetItem(&Item);

		}

		if (row > 0)
			nRowCount++;

		if (nRowCount > 10)
			nRowCount = 1;

		grid.RedrawRow(row);
	}
	grid.AutoSizeColumns();
	grid.ExpandColumnsToFit();
}

//SERVER DRAW CHART
void DlgINISetting::draw_grid_server_viewer(CGridCtrl &grid, int col, int row)
{
	DWORD dwTextStyle = DT_CENTER | DT_VCENTER | DT_SINGLELINE;    // Text Style
	int nRowCount = 1;

	for (int row = 0; row < grid.GetRowCount(); row++)
	{
		for (int col = 0; col < grid.GetColumnCount(); col++)
		{
			GV_ITEM Item;
			Item.mask = GVIF_TEXT | GVIF_FORMAT;
			Item.row = row;
			Item.col = col;

			// Fixed Cell Title
			if (row == 0 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Name"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row == 1 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Client Name"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row == 2 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Client Id"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 3 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Plc Ip"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 4 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Plc Port"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 5 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Client Folder"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row ==6 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Dummy Start Pos X"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 7 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Dummy Start Pos Y"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 8 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Dummy Inspection Height"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 9 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Use Elb Nozzle To Paneldist Check"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}

			else if (row == 0 && col == 1)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("Value"));
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row == 1 && col == 1)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("%s"), m_stClientName);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row == 2 && col == 1)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("%d"), m_nClientId);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row == 3 && col == 1)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("%s"), m_strPlcIp);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row == 4 && col == 1)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("%d"), m_nPlcPort);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row == 5 && col == 1)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("%s"), m_stFolderName);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row == 6 && col == 1)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("%d"), m_nDummyStartX);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row == 7 && col == 1)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("%d"), m_nDummyStartY);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row == 8 && col == 1)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("%d"), m_nDummyInspHeight);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row == 9 && col == 1)
			{
			Item.nFormat = DT_CENTER | DT_WORDBREAK;
			Item.strText.Format(_T("%d"), m_bUseElbNozzleToPaneldistCheck);
			UINT state = grid.GetItemState(row, col);
			grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else
			{
				Item.nFormat = dwTextStyle;
			}

			if (row > 0)
			{
				if (nRowCount <= row)
					grid.SetItemBkColour(row, col, RGB(235, 235, 235));
				else
					grid.SetItemBkColour(row, col, RGB(255, 255, 255));

				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state);
			}

			grid.SetItem(&Item);

		}

		if (row > 0)
			nRowCount++;

		if (nRowCount > 10)
			nRowCount = 1;

		grid.RedrawRow(row);
	}
	grid.AutoSizeColumns();
	grid.ExpandColumnsToFit();
	//grid.ExpandRowsToFit();
}

//LIGHT CONTROL DRAW CHART
void DlgINISetting::draw_grid_light_control_viewer(CGridCtrl &grid, int col, int row)
{
	DWORD dwTextStyle = DT_CENTER | DT_VCENTER | DT_SINGLELINE;    // Text Style
	int nRowCount = 1;

	for (int row = 0; row < grid.GetRowCount(); row++)
	{
		for (int col = 0; col < grid.GetColumnCount(); col++)
		{
			GV_ITEM Item;
			Item.mask = GVIF_TEXT | GVIF_FORMAT;
			Item.row = row;
			Item.col = col;

			// Fixed Cell Title
			if (row == 0 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Index"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row == 0 && col == 1)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Name"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row == 0 && col == 2)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Type"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col == 3)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Port"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col == 4)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Baudrate"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col == 5)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Sock Port"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 0 && col == 6)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Sock IP"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row != 0 && col == 0)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("%d"),row);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row != 0 && col == 1)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("%s"), m_stLightName[row - 1]);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row != 0 && col == 2)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("%d"), m_nLightType[row - 1]);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row != 0 && col == 3)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("%d"), m_nLightPort[row - 1]);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row != 0 && col == 4)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("%d"), m_nLightBaudrate[row - 1]);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row != 0 && col == 5)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("%d"), m_nLightSockPort[row - 1]);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row != 0 && col == 6)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("%s"), m_strLightSockIp[row - 1]);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else
			{
				Item.nFormat = dwTextStyle;
			}

			if (row > 0)
			{
				if (nRowCount <= m_nNumOfLightControl)
					grid.SetItemBkColour(row, col, RGB(235, 235, 235));
				else
					grid.SetItemBkColour(row, col, RGB(255, 255, 255));

				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state);
			}

			grid.SetItem(&Item);

		}

		if (row > 0)
			nRowCount++;

		if (nRowCount > 10)
			nRowCount = 1;

		grid.RedrawRow(row);
	}
	grid.AutoSizeColumns();
	grid.ExpandColumnsToFit();
}

//PLC ADDRESS DRAW CHART
void DlgINISetting::draw_grid_plc_address_viewer(CGridCtrl &grid, int col, int row)
{
	DWORD dwTextStyle = DT_CENTER | DT_VCENTER | DT_SINGLELINE;    // Text Style
	int nRowCount = 1;

	int i = m_cb_select_job.GetCurSel();

	for (int row = 0; row < grid.GetRowCount(); row++)
	{
		for (int col = 0; col < grid.GetColumnCount(); col++)
		{
			GV_ITEM Item;
			Item.mask = GVIF_TEXT | GVIF_FORMAT;
			Item.row = row;
			Item.col = col;

			// Fixed Cell Title
			if (row == 0 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Name"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			if (row == 0 && col == 1)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Value"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			if (row == 1 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Start Read Bit Address"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 2 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Start Write Bit Address"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 3 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Start Read Word Address"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 4 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Start Write Word Address"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 5 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Size Read Bit Address"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 6 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Size Read Word Address"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 7 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Size Write Bit Address"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 8 && col == 0)
			{
				Item.nFormat = DT_CENTER | DT_WORDBREAK;
				Item.strText.Format(_T("Size Write Word Address"), col);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_FILTER | GVIS_EDITABLE | GVIS_READONLY);
			}
			else if (row == 1 && col == 1)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format("%d", m_nStartReadBitAddress);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row == 2 && col == 1)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format(_T("%d"), m_nStartWriteBitAddress);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row == 3 && col == 1)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format("%d", m_nStartReadWordAddress);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row == 4 && col == 1)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format("%d", m_nStartWriteWordAddress);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row == 5 && col == 1)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format("%d", m_nSizeReadBitAddress);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row == 6 && col == 1)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format("%d", m_nSizeReadWordAddress);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row == 7 && col == 1)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format("%d", m_nSizeWriteBitAddress);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else if (row == 8 && col == 1)
			{
				Item.nFormat = dwTextStyle;
				Item.strText.Format("%d", m_nSizeWriteWordAddress);
				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state | GVIS_READONLY);
			}
			else
			{
				Item.nFormat = dwTextStyle;
			}


			if (row > 0)
			{
				if (nRowCount < 9)
					grid.SetItemBkColour(row, col, RGB(235, 235, 235));
				else
					grid.SetItemBkColour(row, col, RGB(255, 255, 255));

				UINT state = grid.GetItemState(row, col);
				grid.SetItemState(row, col, state);
			}

			grid.SetItem(&Item);
		}

		if (row > 0)
			nRowCount++;

		if (nRowCount > 10)
			nRowCount = 1;

		grid.RedrawRow(row);
	}
	grid.AutoSizeColumns();
	grid.ExpandColumnsToFit();
	grid.ExpandRowsToFit();
}

//Viewer select cb
void DlgINISetting::OnCbnSelchangeCbSelectViewer()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_nNumOfView = ((CComboBox *)GetDlgItem(IDC_GRID_SELECT_VIEWER))->GetCurSel();

	int n_channel = 0;

	int nView = m_cbSelectViewer.GetCurSel();

	if (nView == 0)
	{
		init_grid_view_setting(m_grid_select_viewer, 7, m_nNumOfViewer + 1);
		draw_grid_view_name(m_grid_select_viewer, 7, m_nNumOfViewer + 1);
	}
	else
	{
		init_grid_view_setting(m_grid_select_viewer, 7, 2);
		draw_grid_view_name(m_grid_select_viewer, 7, 2);
	}

}

BOOL DlgINISetting::OnNotify(WPARAM wParam, LPARAM lParam, LRESULT* pResult)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	// Click : 4294967196
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////JOB ini
	if (wParam == (WPARAM)m_grid_job_viewer.GetDlgCtrlID())
	{
		GV_DISPINFO *pDispInfo = (GV_DISPINFO*)lParam;
		if (4294967196 == pDispInfo->hdr.code)
		{
			SendMessage(WM_VIEW_CONTROL, MSG_INI_JOB_VIEWER, MAKELPARAM(pDispInfo->item.row, pDispInfo->item.col));
		}
	}
	if (wParam == (WPARAM)m_grid_position_viewer.GetDlgCtrlID())
	{
		GV_DISPINFO *pDispInfo = (GV_DISPINFO*)lParam;
		if (4294967196 == pDispInfo->hdr.code)
		{
			SendMessage(WM_VIEW_CONTROL, MSG_INI_JOB_POSITION, MAKELPARAM(pDispInfo->item.row, pDispInfo->item.col));
		}
	}
	if (wParam == (WPARAM)m_grid_job_view_viewer.GetDlgCtrlID())
	{
		GV_DISPINFO *pDispInfo = (GV_DISPINFO*)lParam;
		if (4294967196 == pDispInfo->hdr.code)
		{
			SendMessage(WM_VIEW_CONTROL, MSG_INI_JOB_VIEW_VIEWER, MAKELPARAM(pDispInfo->item.row, pDispInfo->item.col));
		}
	}
	if (wParam == (WPARAM)m_grid_job_main_viewer.GetDlgCtrlID())
	{
		GV_DISPINFO *pDispInfo = (GV_DISPINFO*)lParam;
		if (4294967196 == pDispInfo->hdr.code)
		{
			SendMessage(WM_VIEW_CONTROL, MSG_INI_JOB_MAIN_VIEWER, MAKELPARAM(pDispInfo->item.row, pDispInfo->item.col));
		}
	}
	if (wParam == (WPARAM)m_grid_job_machine_viewer.GetDlgCtrlID())
	{
		GV_DISPINFO *pDispInfo = (GV_DISPINFO*)lParam;
		if (4294967196 == pDispInfo->hdr.code)
		{
			SendMessage(WM_VIEW_CONTROL, MSG_INI_JOB_MACHINE_VIEWER, MAKELPARAM(pDispInfo->item.row, pDispInfo->item.col));
		}
	}
	if (wParam == (WPARAM)m_grid_job_cam_viewer.GetDlgCtrlID())
	{
		GV_DISPINFO *pDispInfo = (GV_DISPINFO*)lParam;
		if (4294967196 == pDispInfo->hdr.code)
		{
			SendMessage(WM_VIEW_CONTROL, MSG_INI_JOB_CAM_VIEWER, MAKELPARAM(pDispInfo->item.row, pDispInfo->item.col));
		}
	}
	if (wParam == (WPARAM)m_grid_caliper_viewer.GetDlgCtrlID())
	{
		GV_DISPINFO *pDispInfo = (GV_DISPINFO*)lParam;
		if (4294967196 == pDispInfo->hdr.code)
		{
			SendMessage(WM_VIEW_CONTROL, MSG_INI_JOB_CALIPER, MAKELPARAM(pDispInfo->item.row, pDispInfo->item.col));
		}
	}
	if (wParam == (WPARAM)m_grid_job_light_viewer.GetDlgCtrlID())
	{
		GV_DISPINFO *pDispInfo = (GV_DISPINFO*)lParam;
		if (4294967196 == pDispInfo->hdr.code)
		{
			SendMessage(WM_VIEW_CONTROL, MSG_INI_SETTING_LIGHT, MAKELPARAM(pDispInfo->item.row, pDispInfo->item.col));
		}
	}
	if (wParam == (WPARAM)m_grid_light_index_name_viewer.GetDlgCtrlID())
	{
		GV_DISPINFO *pDispInfo = (GV_DISPINFO*)lParam;
		if (4294967196 == pDispInfo->hdr.code)
		{
			SendMessage(WM_VIEW_CONTROL, MSG_INI_JOB_LIGHT_INDEX_NAME, MAKELPARAM(pDispInfo->item.row, pDispInfo->item.col));
		}
	}
	if (wParam == (WPARAM)m_grid_plc_data.GetDlgCtrlID())
	{
		GV_DISPINFO *pDispInfo = (GV_DISPINFO*)lParam;
		if (4294967196 == pDispInfo->hdr.code)
		{
			SendMessage(WM_VIEW_CONTROL, MSG_INI_SETTING_PLC, MAKELPARAM(pDispInfo->item.row, pDispInfo->item.col));
		}
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////VIEWER ini
	if (wParam == (WPARAM)m_grid_select_viewer.GetDlgCtrlID())
	{

		GV_DISPINFO *pDispInfo = (GV_DISPINFO*)lParam;
		if (4294967196 == pDispInfo->hdr.code)
		{
			SendMessage(WM_VIEW_CONTROL, MSG_INI_SETTING_VIEWER, MAKELPARAM(pDispInfo->item.row, pDispInfo->item.col));
		}
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////CAMERA ini
	if (wParam == (WPARAM)m_grid_camera_viewer.GetDlgCtrlID())
	{
		GV_DISPINFO *pDispInfo = (GV_DISPINFO*)lParam;
		if (4294967196 == pDispInfo->hdr.code)
		{
			SendMessage(WM_VIEW_CONTROL, MSG_INI_CAMERA_VIEWER, MAKELPARAM(pDispInfo->item.row, pDispInfo->item.col));
		}
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////CLIENT ini
	if (wParam == (WPARAM)m_grid_server_viewer.GetDlgCtrlID())
	{
		GV_DISPINFO *pDispInfo = (GV_DISPINFO*)lParam;
		if (4294967196 == pDispInfo->hdr.code)
		{
			SendMessage(WM_VIEW_CONTROL, MSG_INI_SERVER_VIEWER, MAKELPARAM(pDispInfo->item.row, pDispInfo->item.col));
		}
	}
	if (wParam == (WPARAM)m_grid_light_control_viewer.GetDlgCtrlID())
	{
		GV_DISPINFO *pDispInfo = (GV_DISPINFO*)lParam;
		if (4294967196 == pDispInfo->hdr.code)
		{
			SendMessage(WM_VIEW_CONTROL, MSG_INI_LIGHT_CONTROL_VIEWER, MAKELPARAM(pDispInfo->item.row, pDispInfo->item.col));
		}
	}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////PLC ADDRESS ini
	if (wParam == (WPARAM)m_grid_plc_address_viewer.GetDlgCtrlID())
	{
		GV_DISPINFO *pDispInfo = (GV_DISPINFO*)lParam;
		if (4294967196 == pDispInfo->hdr.code)
		{
			SendMessage(WM_VIEW_CONTROL, MSG_INI_PLC_ADDRESS_VIEWER, MAKELPARAM(pDispInfo->item.row, pDispInfo->item.col));
		}
	}
	return CDialogEx::OnNotify(wParam, lParam, pResult);

}

LRESULT DlgINISetting::OnViewControl(WPARAM wParam, LPARAM lParam)
{

	switch (wParam) {
	case MSG_INI_JOB_VIEWER: fnMSG_INI_JOB_VIEWER(lParam);		break;
	case MSG_INI_JOB_POSITION:fnMSG_INI_JOB_POSITION(lParam);		break;
	case MSG_INI_JOB_VIEW_VIEWER:fnMSG_INI_JOB_VIEW_VIEWER(lParam);		break;
	case MSG_INI_JOB_MAIN_VIEWER:fnMSG_INI_JOB_MAIN_VIEWER(lParam);		break;
	case  MSG_INI_JOB_MACHINE_VIEWER:fnMSG_INI_JOB_MACHINE_VIEWER(lParam);		break;
	case MSG_INI_JOB_CAM_VIEWER:fnMSG_INI_JOB_CAM_VIEWER(lParam);		break;
	case  MSG_INI_JOB_CALIPER: fnMSG_INI_JOB_CALIPER(lParam);		break;
	case MSG_INI_SETTING_LIGHT:fnMSG_INI_SETTING_LIGHT(lParam);		break;
	case  MSG_INI_JOB_LIGHT_INDEX_NAME:fnMSG_INI_JOB_LIGHT_INDEX_NAME(lParam);		break;
	case  MSG_INI_SETTING_PLC:fnMSG_INI_SETTING_PLC(lParam);		break;

	case MSG_INI_SETTING_VIEWER:fnMSG_INI_SETTING_VIEWER(lParam);		break;

	case  MSG_INI_CAMERA_VIEWER: fnMSG_INI_CAMERA_VIEWER(lParam);		break;
		
	case MSG_INI_SERVER_VIEWER: fnMSG_INI_SERVER_VIEWER(lParam);		break;
	case MSG_INI_LIGHT_CONTROL_VIEWER:fnMSG_INI_LIGHT_CONTROL_VIEWER(lParam);		break;

	case MSG_INI_PLC_ADDRESS_VIEWER:fnMSG_INI_PLC_ADDRESS_VIEWER(lParam);		break;
	}

	return 0;

}

//Job MSG
void DlgINISetting::fnMSG_INI_JOB_VIEWER(LPARAM lParam)
{

	int row = LOWORD(lParam);
	int col = HIWORD(lParam);

	if (row < 0 || col < 0) return;
	if (row == 0 || col == 0) return;
	if (row == 0 && col != 0) return;

	CKeyPadDlg dlg;
	CVirtualKeyBoard stdlg;
	CString strTemp_NEW;
	CString strTemp_OLD = m_grid_job_viewer.GetItemText(row, col);
	
	if (col != 1)
	{
		dlg.SetValueString(false, strTemp_OLD);
		if (dlg.DoModal() != IDOK)	return;
		dlg.GetValue(strTemp_NEW);
		m_grid_job_viewer.SetItemText(row, col, strTemp_NEW);
	}
	else
	{
		stdlg.SetValueString(false, strTemp_OLD);
		if (stdlg.DoModal() != IDOK)	return;
		stdlg.GetValue(strTemp_NEW);
		m_grid_job_viewer.SetItemText(row, col, strTemp_NEW);
	}
	
	int nJob = m_cb_select_job.GetCurSel();

	if (nJob == 0)
	{
		if (row != 0 && col == 1) m_strJobName[row - 1] = strTemp_NEW;
		else if (row != 0 && col == 2)
		{
			if (atoi(strTemp_NEW) < 0)
			{
				AfxMessageBox(_T("Out of input range! (Range : 0~)"));
				return;
			}
			
			m_nJobMethod[row - 1] = atoi(strTemp_NEW);
		}
		else if (row != 0 && col == 3)
		{
			if (atoi(strTemp_NEW) < 1||atoi(strTemp_NEW)>8)
			{
				AfxMessageBox(_T("Out of input range! (Range : 1~8)"));
				return;
			}
			
			ModifyJobCamCount(row, atoi(strTemp_NEW));
		}
		else if (row != 0 && col == 4)
		{
			if (atoi(strTemp_NEW) < 1 || atoi(strTemp_NEW) > 16)
			{
				AfxMessageBox(_T("Out of input range!(Range : 1~16)"));
				return;
			}
			
			m_nJobViewerCount[row - 1] = atoi(strTemp_NEW);
		}
	}
	else
	{
		if (row == 1 && col == 1) m_strJobName[nJob - 1] = strTemp_NEW;
		else if (row == 1 && col == 2)
		{
			if (atoi(strTemp_NEW) < 0)
			{
				AfxMessageBox(_T("Out of input range! (Range : 0~)"));
				return;
			}
			
			m_nJobMethod[nJob - 1] = atoi(strTemp_NEW);
		}
		else if (row == 1 && col == 3)
		{
			if (atoi(strTemp_NEW) < 1 || atoi(strTemp_NEW) > 8)
			{
				AfxMessageBox(_T("Out of input range! (Range : 1~8)"));
				return;
			}
			
			ModifyJobCamCount(row, atoi(strTemp_NEW));
		}
		else if (row == 1 && col == 4)
		{
			if (atoi(strTemp_NEW) < 1 || atoi(strTemp_NEW) > 16)
			{
				AfxMessageBox(_T("Out of input range! (Range : 1~16)"));
				return;
			}

			for (int i = m_nJobViewerCount[nJob - 1]; i < atoi(strTemp_NEW); i++)
			{
				m_nJobViewerIndex[nJob - 1][i] = 0;
			}

			m_nJobViewerCount[nJob - 1] = atoi(strTemp_NEW);

			init_grid_view_setting(m_grid_job_view_viewer, 2, m_nJobViewerCount[nJob - 1] + 1);
			draw_grid_job_view_viewer(m_grid_job_view_viewer, 2, m_nJobViewerCount[nJob - 1] + 1);

		}
	}
	m_grid_job_viewer.RedrawRow(row);
}
void DlgINISetting::fnMSG_INI_JOB_POSITION(LPARAM lParam) 
{
	int row = LOWORD(lParam);
	int col = HIWORD(lParam);

	if (row < 0 || col < 0) return;
	if (row == 0 || col == 0) return;
	if (row == 0 && col != 0) return;

	
	CVirtualKeyBoard stdlg;
	CString strTemp_NEW;
	CString strTemp_OLD = m_grid_position_viewer.GetItemText(row, col);
	
	stdlg.SetValueString(false, strTemp_OLD);

	if (stdlg.DoModal() != IDOK)	return;

	stdlg.GetValue(strTemp_NEW);
	m_grid_position_viewer.SetItemText(row, col, strTemp_NEW);
	
	int nJob = m_cb_select_job.GetCurSel();
	int nCam = (row-1)/m_nJobCamCount[nJob - 1];

	if (row !=0 && col == 1) m_stPosName[nJob - 1][nCam][row-1] = strTemp_NEW;

	m_grid_position_viewer.RedrawRow(row);
}
void DlgINISetting::fnMSG_INI_JOB_VIEW_VIEWER(LPARAM lParam)
{
	int row = LOWORD(lParam);
	int col = HIWORD(lParam);

	if (row < 0 || col < 0) return;
	if (row == 0 || col == 0) return;
	if (row == 0 && col != 0) return;

	CKeyPadDlg dlg;
	CString strTemp_NEW;
	CString strTemp_OLD = m_grid_job_view_viewer.GetItemText(row, col);
	dlg.SetValueString(false, strTemp_OLD);

	if (dlg.DoModal() != IDOK)	return;

	dlg.GetValue(strTemp_NEW);
	if (atoi(strTemp_NEW) < 0||atoi(strTemp_NEW)>15)
	{
		AfxMessageBox(_T("Out of input range! (Range : 0~15)"));
		return;
	}

	m_grid_job_view_viewer.SetItemText(row, col, strTemp_NEW);

	int nJob = m_cb_select_job.GetCurSel();

	
	if (row =!0 && col == 1) m_nJobViewerIndex[nJob - 1][row - 1] = atoi(strTemp_NEW);
		
	m_grid_job_view_viewer.RedrawRow(row);

}
void DlgINISetting::fnMSG_INI_JOB_MAIN_VIEWER(LPARAM lParam)
{
	int row = LOWORD(lParam);
	int col = HIWORD(lParam);

	if (row < 0 || col < 0) return;
	if (row == 0 || col == 0) return;
	if (row == 0 && col != 0) return;

	CKeyPadDlg dlg;
	CString strTemp_NEW;
	CString strTemp_OLD = m_grid_job_main_viewer.GetItemText(row, col);
	dlg.SetValueString(false, strTemp_OLD);

	if (dlg.DoModal() != IDOK)	return;

	dlg.GetValue(strTemp_NEW);
	if (atoi(strTemp_NEW) < 0 || atoi(strTemp_NEW) > 15)
	{
		AfxMessageBox(_T("Out of input range! (Range : 0~15)"));
		return;
	}
	m_grid_job_main_viewer.SetItemText(row, col, strTemp_NEW);

	int nJob = m_cb_select_job.GetCurSel();
	
	if (row = !0 && col == 1) m_nMainViewIndex[nJob - 1][row - 1] = atoi(strTemp_NEW);

	m_grid_job_main_viewer.RedrawRow(row);

}
void DlgINISetting::fnMSG_INI_JOB_MACHINE_VIEWER(LPARAM lParam) 
{
	
	int row = LOWORD(lParam);
	int col = HIWORD(lParam);

	if (row < 0 || col < 0) return;
	if (row == 0 || col == 0) return;
	if (row == 0 && col != 0) return;

	CKeyPadDlg dlg;
	CString strTemp_NEW;
	CString strTemp_OLD = m_grid_job_machine_viewer.GetItemText(row, col);
	dlg.SetValueString(false, strTemp_OLD);

	if (dlg.DoModal() != IDOK)	return;

	dlg.GetValue(strTemp_NEW);
	if (atoi(strTemp_NEW) < 0 || atoi(strTemp_NEW) > 15)
	{
		AfxMessageBox(_T("Out of input range! (Range : 0~15)"));
		return;
	}
	m_grid_job_machine_viewer.SetItemText(row, col, strTemp_NEW);

	int nJob = m_cb_select_job.GetCurSel();

	if (row = !0 && col == 1) m_nMachineViewIndex[nJob - 1][row - 1] = atoi(strTemp_NEW);

	m_grid_job_machine_viewer.RedrawRow(row);
}
void DlgINISetting::fnMSG_INI_JOB_CAM_VIEWER(LPARAM lParam)
{
	int row = LOWORD(lParam);
	int col = HIWORD(lParam);

	if (row < 0 || col < 0) return;
	if (row == 0 || col == 0) return;
	if (row == 0 && col != 0) return;

	CKeyPadDlg dlg;
	CString strTemp_NEW;
	CString strTemp_OLD = m_grid_job_cam_viewer.GetItemText(row, col);
	dlg.SetValueString(false, strTemp_OLD);

	if (dlg.DoModal() != IDOK)	return;

	dlg.GetValue(strTemp_NEW);
	if (atoi(strTemp_NEW) < 0 || atoi(strTemp_NEW) > 7)
	{
		AfxMessageBox(_T("Out of input range! (Range : 0~7)"));
		return;
	}
	m_grid_job_cam_viewer.SetItemText(row, col, strTemp_NEW);
	
	int nJob = m_cb_select_job.GetCurSel();

	if (row != 0 && col == 1) m_nJobCamIndex[nJob - 1][row - 1] = atoi(strTemp_NEW);
	
	m_grid_job_cam_viewer.RedrawRow(row);
}
void DlgINISetting::fnMSG_INI_JOB_CALIPER(LPARAM lParam)
{
	int row = LOWORD(lParam);
	int col = HIWORD(lParam);

	if (row < 0 || col < 0) return;
	if (row == 0 || col == 0) return;
	if (row == 0 && col != 0) return;

	CVirtualKeyBoard stdlg;
	CString strTemp_NEW;
	CString strTemp_OLD = m_grid_caliper_viewer.GetItemText(row, col);
	
	stdlg.SetValueString(false, strTemp_OLD);

	if (stdlg.DoModal() != IDOK)	return;

	stdlg.GetValue(strTemp_NEW);
	m_grid_caliper_viewer.SetItemText(row, col, strTemp_NEW);
	
	int nJob = m_cb_select_job.GetCurSel();
	int nCamera = m_cbSelectCamera.GetCurSel();
	int nPos = m_cbSelectPos.GetCurSel();

	if (row !=0&& col == 1)  m_stCaliperName[nJob - 1][nCamera][nPos][row-1] = strTemp_NEW;
	
	m_grid_caliper_viewer.RedrawRow(row);

}
void DlgINISetting::fnMSG_INI_SETTING_LIGHT(LPARAM lParam)
{
	int row = LOWORD(lParam);
	int col = HIWORD(lParam);

	if (row < 0 || col < 0) return;
	if (row == 0) return;

	CKeyPadDlg dlg;
	CVirtualKeyBoard stdlg;
	CString strTemp_NEW;
	CString strTemp_OLD = m_grid_job_light_viewer.GetItemText(row, col);
	if (col != 0)
	{
		dlg.SetValueString(false, strTemp_OLD);
		if (dlg.DoModal() != IDOK)	return;
		dlg.GetValue(strTemp_NEW);
		m_grid_job_light_viewer.SetItemText(row, col, strTemp_NEW);
	}
	else
	{
		stdlg.SetValueString(false, strTemp_OLD);
		if (stdlg.DoModal() != IDOK)	return;
		stdlg.GetValue(strTemp_NEW);
		m_grid_job_light_viewer.SetItemText(row, col, strTemp_NEW);
	}

	int nJob = m_cb_select_job.GetCurSel();
	int nLight = m_cbSelectLightCam.GetCurSel();

	if (row != 0 && col == 0)	m_stLightChannelName[nJob - 1][nLight][row - 1] = strTemp_NEW;
	else if (row != 0 && col == 1) 
	{
		if (atoi(strTemp_NEW) < 0)
		{
			AfxMessageBox(_T("Out of input range! (Range : 0~)"));
			return;
		}
		m_nLightChannelId[nJob - 1][nLight][row - 1] = atoi(strTemp_NEW);
	}
	else if (row != 0 && col == 2)
	{
		if (atoi(strTemp_NEW) < 1)
		{
			AfxMessageBox(_T("Out of input range! (Range : 1~)"));
			return;
		}
		m_nLightControlId[nJob - 1][nLight][row - 1] = atoi(strTemp_NEW);
	}

	m_grid_job_light_viewer.RedrawRow(row);
}
void DlgINISetting::fnMSG_INI_JOB_LIGHT_INDEX_NAME(LPARAM lParam)
{
	int row = LOWORD(lParam);
	int col = HIWORD(lParam);

	if (row < 0 || col < 0) return;
	if (row == 0 || col == 0) return;
	if (row == 0 && col != 0) return;

	CVirtualKeyBoard stdlg;
	CString strTemp_NEW;
	CString strTemp_OLD = m_grid_light_index_name_viewer.GetItemText(row, col);
	
	stdlg.SetValueString(false, strTemp_OLD);

	if (stdlg.DoModal() != IDOK)	return;

	stdlg.GetValue(strTemp_NEW);
	m_grid_light_index_name_viewer.SetItemText(row, col, strTemp_NEW);
	
	int nJob = m_cb_select_job.GetCurSel();
	int nLight = m_cbSelectLightCam.GetCurSel();
	
	if (row != 0 && col == 1)  m_stLightIndexName[nJob - 1][nLight][row - 1] = strTemp_NEW;

	m_grid_light_index_name_viewer.RedrawRow(row);
}
void DlgINISetting::fnMSG_INI_SETTING_PLC(LPARAM lParam)
{
	int row = LOWORD(lParam);
	int col = HIWORD(lParam);

	if (row < 0 || col < 0) return;
	if (row == 0 || col == 0) return;
	if (row == 0 && col != 0) return;

	CKeyPadDlg dlg;
	CString strTemp_NEW;
	CString strTemp_OLD = m_grid_plc_data.GetItemText(row, col);
	dlg.SetValueString(false, strTemp_OLD);

	if (dlg.DoModal() != IDOK)	return;

	dlg.GetValue(strTemp_NEW);
	m_grid_plc_data.SetItemText(row, col, strTemp_NEW);
	
	int nJob = m_cb_select_job.GetCurSel();
	
	if (row == 0 && col == 1)  m_nJobReadBitStart[nJob - 1] = atoi(strTemp_NEW);
	else if (row == 1 && col == 1)  m_nJobWriteBitStart[nJob - 1] = atoi(strTemp_NEW);
	else if (row == 2 && col == 1)  m_nJobReadWordStart[nJob - 1] = atoi(strTemp_NEW);
	else if (row == 3 && col == 1)  m_nJobWriteWordStart[nJob - 1] = atoi(strTemp_NEW);
	else if (row == 4 && col == 1)  m_nJobUseMainObjectID[nJob - 1] = atoi(strTemp_NEW);
	else if (row == 5 && col == 1)   m_nJobMainObjectIdStart[nJob - 1] = atoi(strTemp_NEW);
	else if (row == 6 && col == 1) m_nJobUseSubObjectID[nJob - 1] = atof(strTemp_NEW);
	else if (row == 7 && col == 1)  m_nJobSubObjectIdStart[nJob - 1] = atof(strTemp_NEW);

	m_grid_plc_data.RedrawRow(row);
}

//VIEWER MSG
void DlgINISetting::fnMSG_INI_SETTING_VIEWER(LPARAM lParam)
{
	int row = LOWORD(lParam);
	int col = HIWORD(lParam);

	if (row < 0 || col < 0) return;
	if (row == 0||col==0) return;

	CKeyPadDlg dlg;
	CVirtualKeyBoard stdlg;
	CString strTemp_NEW;
	CString strTemp_OLD = m_grid_select_viewer.GetItemText(row, col);
	if (col != 1)
	{
		dlg.SetValueString(false, strTemp_OLD);
		if (dlg.DoModal() != IDOK)	return;
		dlg.GetValue(strTemp_NEW);
		if (atoi(strTemp_NEW) < 0)
		{
			AfxMessageBox(_T("Out of input range! (Range : 0~)"));
			return;
		}
		m_grid_select_viewer.SetItemText(row, col, strTemp_NEW);
	}
	else
	{
		stdlg.SetValueString(false, strTemp_OLD);
		if (stdlg.DoModal() != IDOK)	return;
		stdlg.GetValue(strTemp_NEW);
		m_grid_select_viewer.SetItemText(row, col, strTemp_NEW);
	}

	int nView = m_cbSelectViewer.GetCurSel();
	
	if (nView == 0)
	{
		if (row != 0 && col == 1) m_strName[row - 1] = strTemp_NEW;
		else if (row != 0 && col == 2) m_nWidth[row - 1] = atoi(strTemp_NEW);
		else if (row != 0 && col == 3) m_nHeight[row - 1] = atoi(strTemp_NEW);
		else if (row != 0 && col == 4)
		{
			if (atoi(strTemp_NEW) < 0 || atoi(strTemp_NEW) > 15)
			{
				AfxMessageBox(_T("Out of input range! (Range : 0~15)"));
				return;
			}

			m_nViewNumPos[row - 1] = atoi(strTemp_NEW);
		}
		//else if (row != 0 && col == 5) m_dResolutionX[row - 1] = atof(strTemp_NEW);
		//else if (row != 0 && col == 6) m_dResolutionY[row - 1] = atof(strTemp_NEW);
		else if (row != 0 && col ==5) m_nPopup[row - 1] = atoi(strTemp_NEW);
		else if (row != 0 && col == 6) m_nCameraIdx[row - 1] = atoi(strTemp_NEW);
	}
	else
	{
		if (row != 0 && col == 1) m_strName[nView - 1] = strTemp_NEW;
		else if (row != 0 && col == 2) m_nWidth[nView - 1] = atoi(strTemp_NEW);
		else if (row != 0 && col == 3) m_nHeight[nView - 1] = atoi(strTemp_NEW);
		else if (row != 0 && col == 4)
		{
			if (atoi(strTemp_NEW) < 0 || atoi(strTemp_NEW) > 15)
			{
				AfxMessageBox(_T("Out of input range! (Range : 0~15)"));
				return;
			}

			m_nViewNumPos[nView - 1] = atoi(strTemp_NEW);
		}
		//else if (row != 0 && col == 5) m_dResolutionX[nView - 1] = atof(strTemp_NEW);
		//else if (row != 0 && col == 6) m_dResolutionY[nView - 1] = atof(strTemp_NEW);
		else if (row != 0 && col == 5) m_nPopup[nView - 1] = atoi(strTemp_NEW);
		else if (row != 0 && col == 6) m_nCameraIdx[nView - 1] = atoi(strTemp_NEW);
	}
	m_grid_select_viewer.RedrawRow(row);
}

//CAMERA MSG
void DlgINISetting::fnMSG_INI_CAMERA_VIEWER(LPARAM lParam)
{
	int row = LOWORD(lParam);
	int col = HIWORD(lParam);

	if (row < 0 || col < 0) return;
	if (row == 0||col==0) return;

	CKeyPadDlg dlg;
	CVirtualKeyBoard stdlg;
	CString strTemp_NEW;
	CString strTemp_OLD = m_grid_camera_viewer.GetItemText(row, col);
	if (col != 1&&col!=2)
	{
		dlg.SetValueString(false, strTemp_OLD);
		if (dlg.DoModal() != IDOK)	return;
		dlg.GetValue(strTemp_NEW);
		if (atoi(strTemp_NEW) < 0)
		{
			AfxMessageBox(_T("Out of input range! (Range : 0~)"));
			return;
		}
		m_grid_camera_viewer.SetItemText(row, col, strTemp_NEW);
	}
	else
	{
		stdlg.SetValueString(false, strTemp_OLD);
		if (stdlg.DoModal() != IDOK)	return;
		stdlg.GetValue(strTemp_NEW);
		m_grid_camera_viewer.SetItemText(row, col, strTemp_NEW);
	}

	if (row != 0 && col == 1) m_strCamName[row - 1] = strTemp_NEW;
	else if (row != 0 && col == 2) m_nCamSerial[row - 1] =strTemp_NEW;
	else if (row != 0 && col == 3) m_nCamWidth[row - 1] = atoi(strTemp_NEW);
	else if (row != 0 && col == 4) m_nCamHeight[row - 1] = atoi(strTemp_NEW);
	else if (row != 0 && col == 5) m_nFlipDir[row - 1] = atoi(strTemp_NEW);

	m_grid_camera_viewer.RedrawRow(row);
}

//CLIENT MSG
void DlgINISetting::fnMSG_INI_SERVER_VIEWER(LPARAM lParam)
{
	int row = LOWORD(lParam);
	int col = HIWORD(lParam);

	if (row < 0 || col < 0) return;
	if (row == 0 || col == 0) return;
	if (row == 0 && col != 0) return;

	CKeyPadDlg dlg;
	CVirtualKeyBoard stdlg;
	CString strTemp_NEW;
	CString strTemp_OLD = m_grid_server_viewer.GetItemText(row, col);
	if (row != 1&&row!=3&&row!=5)
	{
		dlg.SetValueString(false, strTemp_OLD);
		if (dlg.DoModal() != IDOK)	return;
		dlg.GetValue(strTemp_NEW);
		if (atoi(strTemp_NEW) < 0)
		{
			AfxMessageBox(_T("Out of input range! (Range : 0~)"));
			return;
		}
		m_grid_server_viewer.SetItemText(row, col, strTemp_NEW);
	}
	else
	{
		stdlg.SetValueString(false, strTemp_OLD);
		if (stdlg.DoModal() != IDOK)	return;
		stdlg.GetValue(strTemp_NEW);
		m_grid_server_viewer.SetItemText(row, col, strTemp_NEW);
	}

	if (row == 1 && col == 1)  m_stClientName = strTemp_NEW;
	else if (row == 2 && col == 1) m_nClientId = atoi(strTemp_NEW);
	else if (row == 3 && col == 1)  m_strPlcIp = strTemp_NEW;
	else if (row == 4 && col == 1)  m_nPlcPort = atoi(strTemp_NEW);
	else if (row == 5 && col == 1)   m_stFolderName = strTemp_NEW;
	else if (row == 6 && col == 1)   m_nDummyStartX = atoi(strTemp_NEW);
	else if (row == 7 && col == 1)   m_nDummyStartY = atoi(strTemp_NEW);
	else if (row == 8 && col == 1)   m_nDummyInspHeight = atoi(strTemp_NEW);
	else if (row == 9 && col == 1)   m_bUseElbNozzleToPaneldistCheck = atoi(strTemp_NEW);

	m_grid_server_viewer.RedrawRow(row);
}
void DlgINISetting::fnMSG_INI_LIGHT_CONTROL_VIEWER(LPARAM lParam)
{

	int row = LOWORD(lParam);
	int col = HIWORD(lParam);

	if (row < 0 || col < 0) return;
	if (row == 0 || col == 0) return;

	CKeyPadDlg dlg;
	CVirtualKeyBoard stdlg;
	CString strTemp_NEW;
	CString strTemp_OLD = m_grid_light_control_viewer.GetItemText(row, col);
	if (col != 1&&col!=6)
	{
		dlg.SetValueString(false, strTemp_OLD);
		if (dlg.DoModal() != IDOK)	return;
		dlg.GetValue(strTemp_NEW);
		if (atoi(strTemp_NEW) < 0)
		{
			AfxMessageBox(_T("Out of input range! (Range : 0~)"));
			return;
		}
		m_grid_light_control_viewer.SetItemText(row, col, strTemp_NEW);
	}
	else
	{
		stdlg.SetValueString(false, strTemp_OLD);
		if (stdlg.DoModal() != IDOK)	return;
		stdlg.GetValue(strTemp_NEW);
		m_grid_light_control_viewer.SetItemText(row, col, strTemp_NEW);
	}

	if(row != 0 && col == 1)m_stLightName[row-1]= strTemp_NEW;
	else if (row != 0 && col == 2) m_nLightType[row - 1] = atoi(strTemp_NEW);
	else if (row != 0 && col == 3)m_nLightPort[row - 1] = atoi(strTemp_NEW);
	else if (row != 0 && col == 4) m_nLightBaudrate[row - 1] = atoi(strTemp_NEW);
	else if (row != 0 && col == 5) m_nLightSockPort[row - 1] = atoi(strTemp_NEW);
	else if (row != 0 && col == 6) m_strLightSockIp[row - 1] = strTemp_NEW;

	m_grid_light_control_viewer.RedrawRow(row);
}

//PLC ADDRESS MSG
void DlgINISetting::fnMSG_INI_PLC_ADDRESS_VIEWER(LPARAM lParam)
{
	int row = LOWORD(lParam);
	int col = HIWORD(lParam);

	if (row < 0 || col < 0) return;
	if (row == 0 || col == 0) return;
	if (row == 0 && col != 0) return;

	CKeyPadDlg dlg;
	CString strTemp_NEW;
	CString strTemp_OLD = m_grid_plc_address_viewer.GetItemText(row, col);
	dlg.SetValueString(false, strTemp_OLD);

	if (dlg.DoModal() != IDOK)	return;

	dlg.GetValue(strTemp_NEW);
	if (atoi(strTemp_NEW) < 0)
	{
		AfxMessageBox(_T("Out of input range! (Range : 0~)"));
		return;
	}
	m_grid_plc_address_viewer.SetItemText(row, col, strTemp_NEW);

	if (row == 0 && col == 1)  m_nStartReadBitAddress = atoi(strTemp_NEW);
	else if (row == 1 && col == 1)  m_nStartWriteBitAddress = atoi(strTemp_NEW);
	else if (row == 2 && col == 1) m_nStartReadWordAddress = atoi(strTemp_NEW);
	else if (row == 3 && col == 1)  m_nStartWriteWordAddress = atoi(strTemp_NEW);
	else if (row == 4 && col == 1)  m_nSizeReadBitAddress = atoi(strTemp_NEW);
	else if (row == 5 && col == 1)  m_nSizeReadWordAddress = atoi(strTemp_NEW);
	else if (row == 6 && col == 1) m_nSizeWriteBitAddress = atoi(strTemp_NEW);
	else if (row == 7 && col == 1) m_nSizeWriteWordAddress = atoi(strTemp_NEW);

	m_grid_plc_address_viewer.RedrawRow(row);
}
//JOB 저장
void DlgINISetting::save_job_config()
{
	CString strKey, strData, strSection;
	TCHAR szData[MAX_PATH] = { 0, };
	
	CString strFilePath;

	for (int i = 0; i < m_nJobCount; i++)
	{
		strFilePath.Format("%sJob%d_Info.ini", m_pMain->m_strConfigDir, i + 1);
		/////////////////////////////////////////////////////////JOB
		strSection.Format("JOB_INFO");
		strKey.Format("JOB_NAME");
		strData.Format("%s", m_strJobName[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);

		//Machine의 job 파일명 바꾸기
		CString strNewMachineFilePath, strOldMachineFilePath;
		
		if (i < m_pMain->vt_job_info.size())
		{
			strOldMachineFilePath.Format("%s%s\\", m_pMain->m_strMachineDir, m_pMain->vt_job_info[i].job_name.c_str());
			strNewMachineFilePath.Format("%s%s\\", m_pMain->m_strMachineDir, m_strJobName[i]);
			rename(strOldMachineFilePath, strNewMachineFilePath);
		}
		
		strKey.Format("ALGO_METHOD");
		strData.Format("%d", m_nJobMethod[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);

		//////////////////////////////////////////////////// CAMERA
		strSection.Format("CAMERA_INFO");
		strKey.Format("NUM_OF_CAMERA");
		strData.Format("%d", m_nJobCamCount[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
	
		for (int j = 0; j < m_nJobCamCount[i]; j++)
		{
			strKey.Format("CAMERA_INDEX%d", j + 1);
			strData.Format("%d", m_nJobCamIndex[i][j]);
			::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
		}

		///////////////////////////////////////////////////////////// POSITION
		strSection.Format("POSITION_INFO");
		strKey.Format("NUM_OF_POSITION");
		strData.Format("%d", m_nJobPosition[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);

		for (int j = 0; j < m_nJobCamCount[i]; j++)
		{
			for (int k = 0; k < m_nJobPosition[i]; k++)
			{
				strKey.Format("POSITION_NAME%d_%d", j + 1, k + 1);
				strData.Format("%s", m_stPosName[i][j][k]);
				::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
			}
		} 
		////////////////////////////////////////////////////////////////// VIEWER
		strSection.Format("VIEWER_INFO");
		strKey.Format("NUM_OF_VIEWER");
		strData.Format("%d", m_nJobViewerCount[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
		
		for (int j = 0; j < m_nJobViewerCount[i]; j++)
		{
			strKey.Format("VIEWER_INDEX%d", j + 1);
			strData.Format("%d", m_nJobViewerIndex[i][j]);
			::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
		}

		strKey.Format("MAIN_VIEW_COUNT_X");
		strData.Format("%d", m_nMainViewCountX[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);

		strKey.Format("MAIN_VIEW_COUNT_Y");
		strData.Format("%d", m_nMainViewCountY[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
		
		for (int j = 0; j < m_nJobViewerCount[i]; j++)
		{
			strKey.Format("MAIN_VIEW_INDEX%d", j + 1);
			strData.Format("%d", m_nMainViewIndex[i][j]);
			::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
		}

		strKey.Format("MACHINE_VIEW_COUNT_X");
		strData.Format("%d", m_nMachineViewCountX[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
		
		strKey.Format("MACHINE_VIEW_COUNT_Y");
		strData.Format("%d", m_nMachineViewCountY[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
		
		for (int j = 0; j <m_nJobCamCount[i]; j++)
		{
			strKey.Format("MACHINE_VIEW_INDEX%d", j + 1);
			strData.Format("%d", m_nMachineViewIndex[i][j]);
			::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
		}

		////////////////////////////////////////////////////////////////// PLC
		strSection.Format("PLC_ADDR_INFO");
		strKey.Format("ADDR_READ_BIT_START", i + 1);
		strData.Format("%d", m_nJobReadBitStart[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
	
		strKey.Format("ADDR_WRITE_BIT_START", i + 1);
		strData.Format("%d", m_nJobWriteBitStart[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
		
		strKey.Format("ADDR_READ_WORD_START", i + 1);
		strData.Format("%d", m_nJobReadWordStart[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);

		strKey.Format("ADDR_WRITE_WORD_START", i + 1);
		strData.Format("%d", m_nJobWriteWordStart[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
	
		strKey.Format("USE_MAIN_OBJECT_ID");
		strData.Format("%d", m_nJobUseMainObjectID[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
		
		strKey.Format("ADDR_MAIN_OBJECT_ID_START");
		strData.Format("%d", m_nJobMainObjectIdStart[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
	
		strKey.Format("USE_SUB_OBJECT_ID");
		strData.Format("%d", m_nJobUseSubObjectID[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
		
		strKey.Format("ADDR_SUB_OBJECT_ID_START");
		strData.Format("%d", m_nJobSubObjectIdStart[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
		
		////////////////////////////////////////////////////////////////// LIGHT
		strSection.Format("LIGHT_INFO");
		
		for (int j = 0; j < m_nJobCamCount[i]; j++)
		{
			strKey.Format("NUM_OF_USING_LIGHT%d", j + 1);
			strData.Format("%d", m_nNumOfUsingLightCount[i][j]);
			::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
			
			for (int n = 0; n < m_nNumOfUsingLightCount[i][j]; n++)
			{
				strKey.Format("LIGHT_CTRL_ID%d_%d", j + 1, n + 1);
				strData.Format("%d", m_nLightControlId[i][j][n]);
				::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
				
				strKey.Format("LIGHT_CHANNEL_ID%d_%d", j + 1, n + 1);
				strData.Format("%d", m_nLightChannelId[i][j][n]);
				::WritePrivateProfileString(strSection, strKey, strData, strFilePath);

				strKey.Format("LIGHT_CHANNEL_NAME%d_%d", j + 1, n + 1);
				strData.Format("%s", m_stLightChannelName[i][j][n]);
				::WritePrivateProfileString(strSection, strKey, strData, strFilePath);

			}

			for (int n = 0; n < MAX_LIGHT_INDEX; n++)
			{
				strKey.Format("LIGHT_INDEX_NAME%d_%d", j + 1, n + 1);
				strData.Format("%s", m_stLightIndexName[i][j][n]);
				::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
			}
		}
		////////////////////////////////////////////////////////////////// CALIPER
		strSection.Format("CALIPER_INFO");

		for (int k = 0; k <m_nJobCamCount[i]; k++)
		{
			for (int j = 0; j < m_nJobPosition[i]; j++)
			{
				for (int n = 0; n < MAX_CALIPER_LINES; n++)
				{
					strKey.Format("CALIPER_NAME%d_%d_%d",k+1,j+1,n+1);
					strData.Format("%s", m_stCaliperName[i][k][j][n]);
					::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
				}
			}
		}
	}
}

//VIEWER 저장 
void DlgINISetting::save_view_config()
{
	CString strKey, strData, strSection;
	TCHAR szData[MAX_PATH] = { 0, };
	
	CString strFilePath;
	strFilePath.Format(m_pMain->m_iniViewerFile.GetPathName());

	strSection.Format("VIEWER_INFO");

	strKey.Format("NUM_OF_VIEWER");
	strData.Format("%d", m_nNumOfViewer);
	::WritePrivateProfileString(strSection, strKey, strData, strFilePath);

	strKey.Format("VIEWER_COUNT_X");
	strData.Format("%d", m_nViewerCntX);
	::WritePrivateProfileString(strSection, strKey, strData, strFilePath);

	strKey.Format("VIEWER_COUNT_Y");
	strData.Format("%d", m_nViewerCntY);
	::WritePrivateProfileString(strSection, strKey, strData, strFilePath);

	for (int i = 0; i < m_nNumOfViewer; i++)
	{
		strSection.Format("VIEWER%d_INFO", i + 1);

		strKey.Format("NAME");
		strData.Format("%s", m_strName[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);

		strKey.Format("WIDTH");
		strData.Format("%d", m_nWidth[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);

		strKey.Format("HEIGHT");
		strData.Format("%d", m_nHeight[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);

		strKey.Format("USE_POPUP");
		strData.Format("%d", m_nPopup[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);

		/*strKey.Format("RESOLUTION_X");
		strData.Format("%f", m_dResolutionX[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);

		strKey.Format("RESOLUTION_Y");
		strData.Format("%f", m_dResolutionY[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);*/

		strKey.Format("VIEWER_POS");
		strData.Format("%d", m_nViewerPos[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);

		strKey.Format("CAMERA_INDEX");
		strData.Format("%d", m_nCameraIdx[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
	}
}

//CAMERA 저장
void DlgINISetting::save_camera_config()
{
	CString strKey, strData, strSection;
	TCHAR szData[MAX_PATH] = { 0, };
	
	CString strFilePath;
	strFilePath.Format(m_pMain->m_iniCameraFile.GetPathName());

	for (int i = 0; i < m_nNumOfCamera; i++)
	{
		strSection.Format("CAMERA%d_INFO", i + 1);

		strKey.Format("NAME");
		strData.Format("%s", m_strCamName[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);

		strKey.Format("SERIAL");
		strData.Format("%s", m_nCamSerial[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);

		strKey.Format("WIDTH");
		strData.Format("%d", m_nCamWidth[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);

		strKey.Format("HEIGHT");
		strData.Format("%d", m_nCamHeight[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);

		strKey.Format("DEPTH");
		strData.Format("%d", m_nCamDepth[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);

		strKey.Format("SIZE");
		strData.Format("%d", m_nCamSize[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);

		strKey.Format("FLIP_DIR");
		strData.Format("%d", m_nFlipDir[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
	}
}

//client 저장
void DlgINISetting::save_client_config()
{
	CString strKey, strData, strSection;
	TCHAR szData[MAX_PATH] = { 0, };
	
	CString strFilePath;
	strFilePath.Format(m_pMain->m_iniFile.GetPathName());

	//server info
	strSection = "SERVER_INFO";

	strKey.Format("CLIENT_NAME");
	strData.Format("%s", m_stClientName);
	::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
	
	strKey.Format("CLIENT_ID");
	strData.Format("%d", m_nClientId);
	::WritePrivateProfileString(strSection, strKey, strData, strFilePath);

	strKey.Format("CAMERA_NUM");
	strData.Format("%d", m_nNumOfCamera);
	::WritePrivateProfileString("SERVER_INFO", strKey, strData, strFilePath);
	
	strKey.Format("PLC_IP");
	strData.Format("%s", m_strPlcIp);
	::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
	
	strKey.Format("PLC_PORT");
	strData.Format("%d", m_nPlcPort);
	::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
	
	strKey.Format("CLIENT_COM_FOLDER");
	strData.Format("%s", m_stFolderName);
	::WritePrivateProfileString(strSection, strKey, strData, strFilePath);

	strKey.Format("NUM_OF_JOB_COUNT");
	strData.Format("%d", m_nJobCount);
	::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
	
	strKey.Format("DUMMY_START_POSX");
	strData.Format("%d", m_nDummyStartX);
	::WritePrivateProfileString(strSection, strKey, strData, strFilePath);

	strKey.Format("DUMMY_START_POSY");
	strData.Format("%d", m_nDummyStartY);
	::WritePrivateProfileString(strSection, strKey, strData, strFilePath);

	strKey.Format("DUMMY_INSP_HEIGHT");
	strData.Format("%d", m_nDummyInspHeight);
	::WritePrivateProfileString(strSection, strKey, strData, strFilePath);

	strKey.Format("USE_ELB_NOZZLE_TO_PANELDIST_CHECK");
	strData.Format("%d", m_bUseElbNozzleToPaneldistCheck);
	::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
	
	//light info
	strSection = "LIGHT_CTRL_INFO";

	int lightIndex = GetDlgItemInt(IDC_EDIT_LIGHT_CONTROL_MODIFY);

	strKey.Format("NUM_OF_LIGHT_CTRL");
	strData.Format("%d", m_nNumOfLightControl);
	::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
	
	for (int i = 0; i < lightIndex; i++)
	{
		strKey.Format("NAME%d", i + 1);
		strData.Format("%s", m_stLightName[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);

		strKey.Format("TYPE%d", i + 1);
		strData.Format("%d", m_nLightType[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);

		strKey.Format("PORT%d", i + 1);
		strData.Format("%d", m_nLightPort[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);

		strKey.Format("BAUDRATE%d", i + 1);
		strData.Format("%d", m_nLightBaudrate[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);

		strKey.Format("SOCK_PORT%d", i + 1);
		strData.Format("%d", m_nLightSockPort[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);

		strKey.Format("SOCK_IP%d", i + 1);
		strData.Format("%s", m_strLightSockIp[i]);
		::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
	}

}

//PLC ADDRESS 저장
void DlgINISetting::save_plc_address_config()
{
	CString strKey, strData, strSection;
	TCHAR szData[MAX_PATH] = { 0, };

	CString strFilePath = m_pMain->m_strSystemDir + "PLCAddress.ini";

	//set_param
	strSection = "SET_PARAM";

	strKey.Format("START_READ_BIT_ADDRESS");
	strData.Format("%d", m_nStartReadBitAddress);
	::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
	
	strKey.Format("START_WRITE_BIT_ADDRESS");
	strData.Format("%d", m_nStartWriteBitAddress);
	::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
	
	strKey.Format("START_READ_WORD_ADDRESS");
	strData.Format("%d", m_nStartReadWordAddress);
	::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
	
	strKey.Format("START_WRITE_WORD_ADDRESS");
	strData.Format("%d", m_nStartWriteWordAddress);
	::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
	 
	strKey.Format("SIZE_READ_BIT_ADDRESS");
	strData.Format("%d", m_nSizeReadBitAddress);
	::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
	
	strKey.Format("SIZE_READ_WORD_ADDRESS");
	strData.Format("%d", m_nSizeReadWordAddress);
	::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
	
	strKey.Format("SIZE_WRITE_BIT_ADDRESS");
	strData.Format("%d", m_nSizeWriteBitAddress);
	::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
	
	strKey.Format("SIZE_WRITE_WORD_ADDRESS");
	strData.Format("%d", m_nSizeWriteWordAddress);
	::WritePrivateProfileString(strSection, strKey, strData, strFilePath);
}

//INIT INI DATA LOAD
void DlgINISetting::InitINIData()
{
	//light info
	read_client_config();

	init_grid_view_setting(m_grid_light_control_viewer, 7, m_nNumOfLightControl + 1);
	draw_grid_light_control_viewer(m_grid_light_control_viewer, 7, m_nNumOfLightControl + 1);

	//JOB ini
	read_job_config();

	int nJobCount = (int)m_pMain->vt_job_info.size();
	int nLightCtrlCount = m_pMain->m_stLightCtrlInfo.nNumOfLightCtrl;

	init_grid_view_setting(m_grid_job_viewer, 5, nJobCount + 1);
	draw_grid_job(m_grid_job_viewer, 5, nJobCount + 1);

	JobAll();

	//server info
	init_grid_view_setting(m_grid_server_viewer, 2, 10);
	draw_grid_server_viewer(m_grid_server_viewer, 2, 10);

	//camera ini
	read_camera_config();

	init_grid_view_setting(m_grid_camera_viewer, 6, m_nNumOfCamera + 1);
	draw_grid_camera_viewer(m_grid_camera_viewer, 6, m_nNumOfCamera + 1);

	//viewer ini
	read_view_config();

	init_grid_view_setting(m_grid_select_viewer,7, m_nNumOfViewer + 1);
	draw_grid_view_name(m_grid_select_viewer, 7, m_nNumOfViewer + 1);

	//plc address ini
	read_plc_address_config();
	init_grid_view_setting(m_grid_plc_address_viewer, 2, 9);
	draw_grid_plc_address_viewer(m_grid_plc_address_viewer, 2,9);

}

//LIGHT CONTROLLER MODIFY btn
void DlgINISetting::OnBnClickedBtnLightControlModify()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int nNewControllerCount = GetDlgItemInt(IDC_EDIT_LIGHT_CONTROL_MODIFY);

	if (nNewControllerCount > 4 || nNewControllerCount < 0)
	{
		AfxMessageBox(_T("Out of input range! (Range : 0~4)"));
		SetDlgItemInt(IDC_EDIT_LIGHT_CONTROL_MODIFY, m_nNumOfLightControl);
	}
	else
	{
		m_nNumOfLightControl = nNewControllerCount;

		init_grid_view_setting(m_grid_light_control_viewer, 7, nNewControllerCount + 1);
		draw_grid_light_control_viewer(m_grid_light_control_viewer, 7, nNewControllerCount + 1);
	}
}

void DlgINISetting::OnBnClickedBtnViewerCountModifyOk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int nNewViewerCount = GetDlgItemInt(IDC_EDIT_VIEWER_COUNT_MODIFY);

	if (nNewViewerCount > 16 || nNewViewerCount < 0)
	{
		AfxMessageBox(_T("Out of input range! (Range :0~16)"));
		SetDlgItemInt(IDC_EDIT_VIEWER_COUNT_MODIFY, m_nNumOfViewer);
	}
	else
	{
		m_nNumOfViewer = nNewViewerCount;

		m_cbSelectViewer.ResetContent();

		CString strCam = "All View";

		m_cbSelectViewer.AddString(strCam);

		for (int i = 0; i < m_nNumOfViewer; i++)
		{
			strCam.Format("View %d번", i + 1);
			m_cbSelectViewer.AddString(strCam);
		}

		m_cbSelectViewer.SetCurSel(0);

		int nViewX = GetDlgItemInt(IDC_EDIT_VIEWER_COUNT_X);
		int nViewY = GetDlgItemInt(IDC_EDIT_VIEWER_COUNT_Y);

		//짝수이고 1이면
		if (nNewViewerCount % 2 == 0 || nNewViewerCount == 1)
		{
			if (nNewViewerCount != nViewX * nViewY)
			{   
				AfxMessageBox("Enter the X and Y values correctly.");
				SetDlgItemInt(IDC_EDIT_VIEWER_COUNT_X, m_nViewerCntX);
				SetDlgItemInt(IDC_EDIT_VIEWER_COUNT_Y, m_nViewerCntY);
			}
			else
			{
				init_grid_view_setting(m_grid_select_viewer, 7, m_nNumOfViewer + 1);
				draw_grid_view_name(m_grid_select_viewer,7, m_nNumOfViewer + 1);

				m_nViewerCntX = nViewX;
				m_nViewerCntY = nViewY;
			}
		}
		else // 홀수이고 1이아니면
		{
			if (nNewViewerCount + 1 != nViewX * nViewY)
			{
				AfxMessageBox("Enter the X and Y values correctly.");
				SetDlgItemInt(IDC_EDIT_VIEWER_COUNT_X, m_nViewerCntX);
				SetDlgItemInt(IDC_EDIT_VIEWER_COUNT_Y, m_nViewerCntY);
			}
			else
			{
				init_grid_view_setting(m_grid_select_viewer, 7, m_nNumOfViewer + 1);
				draw_grid_view_name(m_grid_select_viewer, 7, m_nNumOfViewer + 1);

				m_nViewerCntX = nViewX;
				m_nViewerCntY = nViewY;
			}
		}
	}
}

BOOL DlgINISetting::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 여기에 특수화된 코드를 추가 및/또는 기본 클래스를 호출합니다.
	if (pMsg->message == WM_KEYDOWN)
	{
		if (pMsg->wParam == VK_RETURN|| pMsg->wParam == VK_ESCAPE) // ENTER키 눌릴 시
			return TRUE;
	}
	return CDialogEx::PreTranslateMessage(pMsg);
}


//job count modify btn
void DlgINISetting::OnBnClickedBtnJobCountModifyOk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int nNewJobCount = GetDlgItemInt(IDC_EDIT_JOB_COUNT_MODIFY);
	

	if (nNewJobCount > 8 || nNewJobCount < 0)
	{
		AfxMessageBox(_T("Out of input range! (Range :0~8)"));
		SetDlgItemInt(IDC_EDIT_JOB_COUNT_MODIFY, m_nJobCount);
	}
	else
	{
		//새로 추가된 항목 초기화
		JobDataInit();
		
		m_nJobCount = nNewJobCount;
		vJobCount.push_back(m_nJobCount);
		
		m_cb_select_job.ResetContent();

		CString strCam = "All Job";

		m_cb_select_job.AddString(strCam);

		for (int i = 0; i < m_nJobCount; i++)
		{
			strCam.Format("Job %d번", i + 1);
			m_cb_select_job.AddString(strCam);
		}

		m_cb_select_job.SetCurSel(0);

		JobAll();
	}
}

//job select cb
void DlgINISetting::OnCbnSelchangeCbSelectJob()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int nJob = m_cb_select_job.GetCurSel();

	if (nJob != 0)
	{
		//JOB INFO
		JobNotAll();
		
		//Job Position Grid
		SetDlgItemInt(IDC_EDIT_POS_MODIFY, m_nJobPosition[nJob - 1]);

		int nPosition = m_nJobCamCount[nJob - 1] * m_nJobPosition[nJob - 1];
		init_grid_view_setting(m_grid_position_viewer, 2, nPosition + 1);
		draw_grid_position_viewer(m_grid_position_viewer, 2, nPosition + 1);

		//Job Machine Grid
		SetDlgItemInt(IDC_EDIT_MACHINE_COUNT_X, m_nMachineViewCountX[nJob - 1]);
		SetDlgItemInt(IDC_EDIT_MACHINE_COUNT_Y, m_nMachineViewCountY[nJob - 1]);

		int nMachineIndex = m_nMachineViewCountX[nJob - 1] * m_nMachineViewCountY[nJob - 1];

		init_grid_view_setting(m_grid_job_machine_viewer, 2, nMachineIndex + 1);
		draw_grid_job_machine_viewer(m_grid_job_machine_viewer, 2, nMachineIndex + 1);

		//job main grid
		SetDlgItemInt(IDC_EDIT_MAIN_COUNT_X, m_nMainViewCountX[nJob - 1]);
		SetDlgItemInt(IDC_EDIT_MAIN_COUNT_Y, m_nMainViewCountY[nJob - 1]);

		int nMainIndex = m_nMainViewCountX[nJob - 1] * m_nMainViewCountY[nJob - 1];

		init_grid_view_setting(m_grid_job_main_viewer, 2, nMainIndex + 1);
		draw_grid_job_main_viewer(m_grid_job_main_viewer, 2, nMainIndex + 1);


		//Job Cameara Grid
		SetDlgItemInt(IDC_EDIT_CAMERA_COUNT_MODIFY, m_nJobCamCount[nJob - 1]);
		
		init_grid_view_setting(m_grid_job_cam_viewer, 2, m_nJobCamCount[nJob - 1] + 1);
		draw_grid_job_cam(m_grid_job_cam_viewer, 2, m_nJobCamCount[nJob - 1] + 1);

		m_cbSelectCamera.ResetContent();

		CString strCaliper = "";

		for (int i = 0; i < m_nJobCamCount[nJob - 1]; i++)
		{
			strCaliper.Format("Cam %d", i + 1);
			m_cbSelectCamera.AddString(strCaliper);
		}

		m_cbSelectCamera.SetCurSel(0);

		m_cbSelectPos.ResetContent();

		CString strPos = "";

		for (int i = 0; i < m_nJobPosition[nJob - 1]; i++)
		{
			strPos.Format("Pos %d", i + 1);
			m_cbSelectPos.AddString(strPos);
		}

		m_cbSelectPos.SetCurSel(0);

		//Job Caliper Grid
		init_grid_view_setting(m_grid_caliper_viewer, 2, MAX_CALIPER_LINES + 1);
		draw_grid_job_caliper_viewer(m_grid_caliper_viewer, 2, MAX_CALIPER_LINES + 1);

		//Job View Grid
		init_grid_view_setting(m_grid_job_view_viewer, 2, m_nJobViewerCount[nJob - 1] + 1);
		draw_grid_job_view_viewer(m_grid_job_view_viewer, 2, m_nJobViewerCount[nJob - 1] + 1);

		//Job Plc Grid
		init_grid_view_setting(m_grid_plc_data, 2, 9);
		draw_grid_plc_data(m_grid_plc_data, 2, 9);

		//Light Grid

		m_cbSelectLightCam.ResetContent();

		CString strLight = "";

		for (int i = 0; i < m_nJobCamCount[nJob-1]; i++)
		{
			strLight.Format("Cam %d번", i + 1);
			m_cbSelectLightCam.AddString(strLight);
		}

		m_cbSelectLightCam.SetCurSel(0);

		int nLight =m_cbSelectLightCam.GetCurSel();
		int nLightIndex = m_nNumOfUsingLightCount[nJob - 1][nLight];
		
		SetDlgItemInt(IDC_EDIT_USE_LIGHT, m_nNumOfUsingLightCount[nJob - 1][nLight]);
		
		init_grid_view_setting(m_grid_job_light_viewer, 3, nLightIndex + 1);
		draw_grid_job_light_data(m_grid_job_light_viewer, 3, nLightIndex + 1);

		//light index name
		init_grid_view_setting(m_grid_light_index_name_viewer, 2, 5);
		draw_grid_job_index_name_viewer(m_grid_light_index_name_viewer, 2, 5);
	}
	else //all일때
	{
		JobAll();
	}
}

//job light select cb
void DlgINISetting::OnCbnSelchangeCbSelectLightCamera()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int nJob = m_cb_select_job.GetCurSel();
	int nCam = m_cbSelectLightCam.GetCurSel();
	int nLightIndex = m_nNumOfUsingLightCount[nJob - 1][nCam];

	SetDlgItemInt(IDC_EDIT_USE_LIGHT, m_nNumOfUsingLightCount[nJob-1][nCam]);

	//light ini
	init_grid_view_setting(m_grid_job_light_viewer, 3, nLightIndex + 1);
	draw_grid_job_light_data(m_grid_job_light_viewer, 3, nLightIndex + 1);

	//light index name
	init_grid_view_setting(m_grid_light_index_name_viewer, 2, 5);
	draw_grid_job_index_name_viewer(m_grid_light_index_name_viewer, 2, 5);

}

//CAMERA COUNT MODIFY OK BTN
void DlgINISetting::OnBnClickedBtnCameraCountModifyOk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int nNewCameraCount = GetDlgItemInt(IDC_EDIT_CAMERA_COUNT);

	if (nNewCameraCount > 8 || nNewCameraCount < 0)
	{
		AfxMessageBox(_T("Out of input range! (Range :0~8)"));
		SetDlgItemInt(IDC_EDIT_CAMERA_COUNT, m_nNumOfCamera);
	}
	else
	{
		m_nNumOfCamera = nNewCameraCount;

		init_grid_view_setting(m_grid_camera_viewer, 6, m_nNumOfCamera + 1);
		draw_grid_camera_viewer(m_grid_camera_viewer, 6, m_nNumOfCamera + 1);
	}
}

//Caliper-camera select CB
void DlgINISetting::OnCbnSelchangeCbSelectCamera()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	init_grid_view_setting(m_grid_caliper_viewer, 2, MAX_CALIPER_LINES + 1);
	draw_grid_job_caliper_viewer(m_grid_caliper_viewer, 2, MAX_CALIPER_LINES + 1);
}

//Caliper-pos select CB
void DlgINISetting::OnCbnSelchangeCbSelectPos()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	init_grid_view_setting(m_grid_caliper_viewer, 2, MAX_CALIPER_LINES + 1);
	draw_grid_job_caliper_viewer(m_grid_caliper_viewer, 2, MAX_CALIPER_LINES + 1);
}

//Position Modify Btn
void DlgINISetting::OnBnClickedBtnPosModifyOk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int nJob = m_cb_select_job.GetCurSel();
	int nNewPosCount = GetDlgItemInt(IDC_EDIT_POS_MODIFY);

	if (nNewPosCount > 8 || nNewPosCount < 0)
	{
		AfxMessageBox(_T("Out of input range! (Range :0~8)"));
		SetDlgItemInt(IDC_EDIT_POS_MODIFY, m_nJobPosition[nJob - 1]);
	}
	else
	{
		m_nJobPosition[nJob - 1] = nNewPosCount;

		m_cbSelectPos.ResetContent();

		CString strPos = "";

		for (int i = 0; i < m_nJobPosition[nJob - 1]; i++)
		{
			strPos.Format("Pos %d", i + 1);
			m_cbSelectPos.AddString(strPos);
		}

		m_cbSelectPos.SetCurSel(0);

		int nPosition = m_nJobCamCount[nJob - 1] * m_nJobPosition[nJob - 1];
		init_grid_view_setting(m_grid_position_viewer, 2, nPosition + 1);
		draw_grid_position_viewer(m_grid_position_viewer, 2, nPosition + 1);
	}
}

//Job Camera Count Modify Btn
void DlgINISetting::OnBnClickedBtnJobCameraCountModifyOk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int nNewCameraCount = GetDlgItemInt(IDC_EDIT_CAMERA_COUNT_MODIFY);
	int nJob = m_cb_select_job.GetCurSel();
	ModifyJobCamCount(nJob,nNewCameraCount);
}

//Machine View Count Modify Btn
void DlgINISetting::OnBnClickedBtnMachineCountModifyOk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int nJob = m_cb_select_job.GetCurSel();

	int nMachineX = GetDlgItemInt(IDC_EDIT_MACHINE_COUNT_X);
	int nMachineY= GetDlgItemInt(IDC_EDIT_MACHINE_COUNT_Y);

	int nMachine = nMachineX * nMachineY;

	if (nMachine%2!=0 && nMachine!=1)//입력값의 곱이 짝수가 아니면 
	{
		AfxMessageBox("Enter the X and Y values correctly.");
		SetDlgItemInt(IDC_EDIT_MACHINE_COUNT_X, m_nMachineViewCountX[nJob-1]);
		SetDlgItemInt(IDC_EDIT_MACHINE_COUNT_Y, m_nMachineViewCountY[nJob - 1]);
	}
	else//짝수면
	{
		//camera수가 짝수이고 1이면
		if (m_nJobCamCount[nJob - 1] % 2 == 0 || m_nJobCamCount[nJob - 1] == 1)
		{
			if (m_nJobCamCount[nJob - 1] != nMachine)
			{
				AfxMessageBox("Enter the X and Y values correctly.");
				SetDlgItemInt(IDC_EDIT_MACHINE_COUNT_X, m_nMachineViewCountX[nJob - 1]);
				SetDlgItemInt(IDC_EDIT_MACHINE_COUNT_Y, m_nMachineViewCountY[nJob - 1]);
			}
			else
			{
				m_nMachineViewCountX[nJob - 1] = nMachineX;
				m_nMachineViewCountY[nJob - 1] = nMachineY;

				int nMachineIndex = m_nMachineViewCountX[nJob - 1] * m_nMachineViewCountY[nJob - 1];

				init_grid_view_setting(m_grid_job_machine_viewer, 2, nMachineIndex + 1);
				draw_grid_job_machine_viewer(m_grid_job_machine_viewer, 2, nMachineIndex + 1);
			}
		}
		else // 홀수이고 1이아니면
		{
			if (m_nJobCamCount[nJob - 1] + 1 != nMachine)
			{
				AfxMessageBox("Enter the X and Y values correctly.");
				SetDlgItemInt(IDC_EDIT_MACHINE_COUNT_X, m_nMachineViewCountX[nJob - 1]);
				SetDlgItemInt(IDC_EDIT_MACHINE_COUNT_Y, m_nMachineViewCountY[nJob - 1]);
			}
			else
			{
				m_nMachineViewCountX[nJob - 1] = nMachineX;
				m_nMachineViewCountY[nJob - 1] = nMachineY;

				int nMachineIndex = m_nMachineViewCountX[nJob - 1] * m_nMachineViewCountY[nJob - 1];

				init_grid_view_setting(m_grid_job_machine_viewer, 2, nMachineIndex + 1);
				draw_grid_job_machine_viewer(m_grid_job_machine_viewer, 2, nMachineIndex + 1);
			}
		}

	}
}

//JOB 추가 시 추가된 JOB정보에 대한 초기화
void DlgINISetting::JobDataInit()
{
	int nNewJobCount = GetDlgItemInt(IDC_EDIT_JOB_COUNT_MODIFY);
	int temp = 0;

	for (int i = 0; i < vJobCount.size() - 1; i++)
	{
		for (int j = i + 1; j < vJobCount.size(); j++)
		{
			if (vJobCount.at(i) < vJobCount.at(j))
			{
				temp = vJobCount.at(i);
				vJobCount.at(i) = vJobCount.at(j);
				vJobCount.at(j) = temp;
			}
		}
	}
	
	int maxJobCount = vJobCount.at(0);

	//CJob temp_job_info;
	//Cjob.assign(MAX_CAMERA, temp_job_info);

	if (maxJobCount < nNewJobCount)
	{
		for (int i = maxJobCount; i < nNewJobCount; i++)
		{
			///////////////////////////////////////////////////////// JOB
			m_strJobName[i] = "2PointAlign";
			m_nJobMethod[i] = 0;
			///////////////////////////////////////////////////////// CAMERA
			m_nJobCamCount[i] = 1;
			
			for (int j = 0; j < m_nJobCamCount[i]; j++)
			{
				m_nJobCamIndex[i][j] = 0;
			}
			///////////////////////////////////////////////////////// POSITION
			m_nJobPosition[i] = 1;
			for (int j = 0; j < m_nJobCamCount[i]; j++)
			{
				for (int k = 0; k < m_nJobPosition[i]; k++)
				{
					m_stPosName[i][j][k].Format("Pos %d", k + 1);
				}
			}
			////////////////////////////////////////////////////////// VIEWER
			m_nJobViewerCount[i] = 1;
			m_nMachineViewCountX[i] = 1;
			m_nMachineViewCountY[i] = 1;
			for (int j = 0; j < m_nJobCamCount[i]; j++)
			{
				m_nMachineViewIndex[i][j] = 0;
			}

			for (int j = 0; j < m_nJobViewerCount[i]; j++)
			{
				m_nJobViewerIndex[i][j] = 0;
			}
			////////////////////////////////////////////////////////// Main VIEWER
			m_nMainViewCountX[i] = 1;
			m_nMainViewCountY[i] = 1;

			for (int j = 0; j < m_nJobViewerCount[i]; j++)
			{
				m_nMainViewIndex[i][j] = 0;
			}
			////////////////////////////////////////////////////////// Machine VIEWER
			m_nMachineViewCountX[i] = 1;
			m_nMachineViewCountY[i] = 1;

			for (int j = 0; j < m_nJobViewerCount[i]; j++)
			{
				m_nMachineViewIndex[i][j] = 0;
			}
			/////////////////////////////////////////////////////////// PLC
			m_nJobReadBitStart[i] = 15100;
			m_nJobWriteBitStart[i] = 15100;
			m_nJobReadWordStart[i] = 15100;
			m_nJobWriteWordStart[i] = 15100;
			m_nJobUseMainObjectID[i] = 0;
			m_nJobMainObjectIdStart[i] = 15100;
			m_nJobUseSubObjectID[i] = 0;
			m_nJobSubObjectIdStart[i] = 15100;
			//////////////////////////////////////////////////////////// LIGHT
			
			for (int j = 0; j < m_nNumOfLightControl; j++)
			{
				m_nNumOfUsingLightCount[i][j] = 1;
				int k = 0;
				
				for (int n = 0; n < m_nNumOfUsingLightCount[i][j]; n++)
				{
					m_nLightControlId[i][j][n] = 1;
					m_nLightChannelId[i][j][n] = 0;
					m_stLightChannelName[i][j][n] = "Reserved";
				}

				for (int n = 0; n < MAX_LIGHT_INDEX; n++)
				{
					m_stLightIndexName[i][j][n] = "Reserved";
				}
			}
			//////////////////////////////////////////////////////////// CALIPER
			for (int k = 0; k < m_nJobCamCount[i]; k++)
			{
				for (int j = 0; j < m_nJobPosition[i]; j++)
				{
					for (int n = 0; n < MAX_CALIPER_LINES; n++)
					{
						m_stCaliperName[i][k][j][n].Format("Line %d", n);
					}
				}
			}
		}
	}
}

//Main Viewer Modify Btn
void DlgINISetting::OnBnClickedBtnMainCountModifyOk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int nJob = m_cb_select_job.GetCurSel();

	int nMainX = GetDlgItemInt(IDC_EDIT_MAIN_COUNT_X);
	int nMainY = GetDlgItemInt(IDC_EDIT_MAIN_COUNT_Y);

	int nMain = nMainX * nMainY;

	if (nMain % 2 != 0 && nMain != 1)//입력값의 곱이 짝수가 아니면 
	{
		AfxMessageBox("Enter the X and Y values correctly.");
		SetDlgItemInt(IDC_EDIT_MAIN_COUNT_X, m_nMainViewCountX[nJob - 1]);
		SetDlgItemInt(IDC_EDIT_MAIN_COUNT_Y, m_nMainViewCountY[nJob - 1]);
	}
	else//짝수면
	{
		m_nMainViewCountX[nJob - 1] = nMainX;
		m_nMainViewCountY[nJob - 1] = nMainY;

		int nMainIndex = m_nMainViewCountX[nJob - 1] * m_nMainViewCountY[nJob - 1];

		init_grid_view_setting(m_grid_job_main_viewer, 2, nMainIndex + 1);
		draw_grid_job_main_viewer(m_grid_job_main_viewer, 2, nMainIndex + 1);
	}
}

//job cb가 all일때 표시 유무
void DlgINISetting::JobAll()
{
	init_grid_view_setting(m_grid_job_viewer, 5, m_nJobCount + 1);
	draw_grid_job(m_grid_job_viewer, 5, m_nJobCount + 1);

	GetDlgItem(IDC_EDIT_POS_MODIFY)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_VIEWER14)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_POSITION_COUNT)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_BTN_POS_MODIFY_OK)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_GRID_JOB_POSITION_VIEWER)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_STATIC_VIEWER10)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_GRID_JOB_VIEW_VIEWER)->ShowWindow(SW_HIDE);
	
	GetDlgItem(IDC_STATIC_VIEWER15)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_GRID_JOB_MAIN_VIEWER)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_MAIN_X)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_MAIN_Y)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_BTN_MAIN_COUNT_MODIFY_OK)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_MAIN_COUNT_X)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_MAIN_COUNT_Y)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_STATIC_VIEWER11)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_MACHINE_X)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_MACHINE_Y)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_GRID_JOB_MACHINE_VIEWER)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_BTN_MACHINE_COUNT_MODIFY_OK)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_MACHINE_COUNT_X)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_MACHINE_COUNT_Y)->ShowWindow(SW_HIDE);

	GetDlgItem(IDC_EDIT_CAMERA_COUNT_MODIFY)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_VIEWER5)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_CAMERA_COUNT)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_GRID_JOB_CAM_VIEWER)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_CB_SELECT_CAMERA)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_CB_SELECT_POS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_BTN_JOB_CAMERA_COUNT_MODIFY_OK)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_START_CAMERA_INDEX)->ShowWindow(SW_HIDE);
	
	GetDlgItem(IDC_STATIC_VIEWER13)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_CALIPER_CAM)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_CALIPER_POS)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_GRID_JOB_CALIPER_VIEWER)->ShowWindow(SW_HIDE);
	
	GetDlgItem(IDC_STATIC_VIEWER2)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_GRID_JOB_LIGHT_DATA)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_USE_LIGHT_CNT)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_BTN_JOB_USE_LIGHT_MODIFY_OK)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_CB_SELECT_LIGHT_CAMERA)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_USE_LIGHT)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_GRID_JOB_LIGHT_INDEX_NAME_VIEWER)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_START_LIGHT_CTRL_ID)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_START_CHANNEL_CTRL_ID)->ShowWindow(SW_HIDE);
	
	GetDlgItem(IDC_STATIC_VIEWER3)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_GRID_PLC_DATA)->ShowWindow(SW_HIDE);

	//
	GetDlgItem(IDC_GROUP_VIEWER_INI)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_VIEWER9)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_VIEWER_COUNT_X)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_VIEWER_COUNT_Y)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_VIEWER_COUNT)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_VIEWER_COUNT_MODIFY)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_VIEWER_COUNT_X)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_VIEWER_COUNT_Y)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_BTN_VIEWER_COUNT_MODIFY_OK)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_VIEWER1)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_CB_SELECT_VIEWER)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_GRID_SELECT_VIEWER)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_GROUP_CAMERA_INI)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_SELECT_INI2)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_CAMERA_COUNT2)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_CAMERA_COUNT)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_BTN_CAMERA_COUNT_MODIFY_OK)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_ALGO_COUNT3)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_270)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_0)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_180)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_90)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_GRID_CAMERA_VIEWER)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_GROUP_ALIGN_INI)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_ALGO_COUNT4)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_VIEWER12)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_ALGO_COUNT5)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_VIEWER7)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_0ITEC)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_1TSM)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_3LLIGHT)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_4LLIGHT)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_5LLIGHT)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_6LLIGHT)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_GRID_SERVER_VIEWER)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_LIGHT_CONTROL_MODIFY)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_BTN_LIGHT_CONTROL_MODIFY)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_GRID_LIGHT_CONTROL_VIEWER)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_GROUP_PLC_INI)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_VIEWER16)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_GRID_PLC_ADDRESS_VIEWER)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_2ITEC)->ShowWindow(SW_SHOW);
}

//job cb가 all이 아닐때 표시 유무
void DlgINISetting::JobNotAll()
{
	init_grid_view_setting(m_grid_job_viewer, 5, 2);
	draw_grid_job(m_grid_job_viewer, 5, 2);

	GetDlgItem(IDC_EDIT_POS_MODIFY)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_VIEWER14)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_POSITION_COUNT)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_BTN_POS_MODIFY_OK)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_GRID_JOB_POSITION_VIEWER)->ShowWindow(SW_SHOW);

	GetDlgItem(IDC_STATIC_VIEWER10)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_GRID_JOB_VIEW_VIEWER)->ShowWindow(SW_SHOW);

	GetDlgItem(IDC_STATIC_VIEWER15)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_GRID_JOB_MAIN_VIEWER)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_MAIN_X)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_MAIN_Y)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_BTN_MAIN_COUNT_MODIFY_OK)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_MAIN_COUNT_X)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_MAIN_COUNT_Y)->ShowWindow(SW_SHOW);

	GetDlgItem(IDC_STATIC_VIEWER11)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_MACHINE_X)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_MACHINE_Y)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_GRID_JOB_MACHINE_VIEWER)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_BTN_MACHINE_COUNT_MODIFY_OK)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_MACHINE_COUNT_X)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_MACHINE_COUNT_Y)->ShowWindow(SW_SHOW);

	GetDlgItem(IDC_EDIT_CAMERA_COUNT_MODIFY)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_VIEWER5)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_CAMERA_COUNT)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_GRID_JOB_CAM_VIEWER)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_CB_SELECT_CAMERA)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_CB_SELECT_POS)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_BTN_JOB_CAMERA_COUNT_MODIFY_OK)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_START_CAMERA_INDEX)->ShowWindow(SW_SHOW);

	GetDlgItem(IDC_STATIC_VIEWER13)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_CALIPER_CAM)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_CALIPER_POS)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_GRID_JOB_CALIPER_VIEWER)->ShowWindow(SW_SHOW);

	GetDlgItem(IDC_STATIC_VIEWER2)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_GRID_JOB_LIGHT_DATA)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_USE_LIGHT_CNT)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_BTN_JOB_USE_LIGHT_MODIFY_OK)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_CB_SELECT_LIGHT_CAMERA)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_EDIT_USE_LIGHT)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_GRID_JOB_LIGHT_INDEX_NAME_VIEWER)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_START_LIGHT_CTRL_ID)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_STATIC_START_CHANNEL_CTRL_ID)->ShowWindow(SW_SHOW);

	GetDlgItem(IDC_STATIC_VIEWER3)->ShowWindow(SW_SHOW);
	GetDlgItem(IDC_GRID_PLC_DATA)->ShowWindow(SW_SHOW);

	///
	GetDlgItem(IDC_GROUP_VIEWER_INI)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_VIEWER9)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_VIEWER_COUNT_X)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_VIEWER_COUNT_Y)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_VIEWER_COUNT)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_VIEWER_COUNT_MODIFY)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_VIEWER_COUNT_X)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_VIEWER_COUNT_Y)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_BTN_VIEWER_COUNT_MODIFY_OK)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_VIEWER1)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_CB_SELECT_VIEWER)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_GRID_SELECT_VIEWER)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_GROUP_CAMERA_INI)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_SELECT_INI2)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_CAMERA_COUNT2)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_CAMERA_COUNT)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_BTN_CAMERA_COUNT_MODIFY_OK)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_ALGO_COUNT3)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_270)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_0)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_180)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_90)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_GRID_CAMERA_VIEWER)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_GROUP_ALIGN_INI)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_ALGO_COUNT4)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_VIEWER12)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_ALGO_COUNT5)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_VIEWER7)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_0ITEC)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_1TSM)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_3LLIGHT)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_4LLIGHT)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_5LLIGHT)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_6LLIGHT)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_GRID_SERVER_VIEWER)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_EDIT_LIGHT_CONTROL_MODIFY)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_BTN_LIGHT_CONTROL_MODIFY)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_GRID_LIGHT_CONTROL_VIEWER)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_GROUP_PLC_INI)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_VIEWER16)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_GRID_PLC_ADDRESS_VIEWER)->ShowWindow(SW_HIDE);
	GetDlgItem(IDC_STATIC_2ITEC)->ShowWindow(SW_HIDE);
	
}

//num of use light controller count modify btn
void DlgINISetting::OnBnClickedBtnJobUseLightModifyOk()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	int nNewUseLightCnt = GetDlgItemInt(IDC_EDIT_USE_LIGHT);
	int nJob = m_cb_select_job.GetCurSel();
	int nLight = m_cbSelectLightCam.GetCurSel();
	int temp = 0;

	//최대값 구하기
	/*for (int i = 0; i < Cjob[nJob-1].Clight[nLight].vJobLightCtrl.size()-1; i++)
	{
		for (int j = i + 1; j < Cjob[nJob - 1].Clight[nLight].vJobLightCtrl.size(); j++)
		{
			if (Cjob[nJob - 1].Clight[nLight].vJobLightCtrl.at(i) < Cjob[nJob - 1].Clight[nLight].vJobLightCtrl.at(j))
			{
				temp = Cjob[nJob - 1].Clight[nLight].vJobLightCtrl.at(i);
				Cjob[nJob - 1].Clight[nLight].vJobLightCtrl.at(i) = Cjob[nJob - 1].Clight[nLight].vJobLightCtrl.at(j);
				Cjob[nJob - 1].Clight[nLight].vJobLightCtrl.at(j) =temp;
			}
		}
	}

	int nMaxLightCtrl = Cjob[nJob - 1].Clight[nLight].vJobLightCtrl.at(0);
	*/
	//사용하는 Light control 개수가 달려있는 Light Control 개수보다 많으면 안됨
	if (nNewUseLightCnt <= m_nNumOfLightControl)
	{
		//light data 초기화
		/*if (nMaxLightCtrl < nNewUseLightCnt)
		{
			for (int n = nMaxLightCtrl; n < nNewUseLightCnt; n++)
			{
				m_stLightChannelName[nJob - 1][nLight][n] = "Reserved";
				m_nLightChannelId[nJob - 1][nLight][n] = 0;
				m_nLightControlId[nJob - 1][nLight][n] = 1;
			}

			m_nNumOfUsingLightCount[nJob - 1][nLight] = nNewUseLightCnt;
			Cjob[nJob - 1].Clight[nLight].vJobLightCtrl.push_back(m_nNumOfUsingLightCount[nJob - 1][nLight]);

			int nLightIndex = m_nNumOfUsingLightCount[nJob - 1][nLight];

			init_grid_view_setting(m_grid_job_light_viewer, 3, nLightIndex + 1);
			draw_grid_job_light_data(m_grid_job_light_viewer, 3, nLightIndex + 1);
		}
		else
		{*/
		for (int n = m_nNumOfUsingLightCount[nJob - 1][nLight]; n < nNewUseLightCnt; n++)
		{
			m_stLightChannelName[nJob - 1][nLight][n] = "Reserved";
			m_nLightChannelId[nJob - 1][nLight][n] = 0;
			m_nLightControlId[nJob - 1][nLight][n] = 1;
		}

		m_nNumOfUsingLightCount[nJob - 1][nLight] = nNewUseLightCnt;
		int nLightIndex = m_nNumOfUsingLightCount[nJob - 1][nLight];
		//Cjob[nJob - 1].Clight[nLight].vJobLightCtrl.push_back(m_nNumOfUsingLightCount[nJob - 1][nLight]);

		init_grid_view_setting(m_grid_job_light_viewer, 3, nLightIndex + 1);
		draw_grid_job_light_data(m_grid_job_light_viewer, 3, nLightIndex + 1);
		//}
	}
	else
	{
		CString str = "";
		str.Format("More than the current number of Light Controls! (Range :1~%d)", m_nNumOfLightControl);
		AfxMessageBox(str);
		SetDlgItemInt(IDC_EDIT_USE_LIGHT, m_nNumOfUsingLightCount[nJob - 1][nLight]);
		return;
	}

}

//camera 개수 바꾸면
void DlgINISetting::ModifyJobCamCount(int nSelect,int nCam)
{
	int nJob = m_cb_select_job.GetCurSel();

	if (nJob == 0)
	{
		nJob = nSelect;
	}

	if (nCam > 8 || nCam < 0)
	{
		AfxMessageBox(_T("Out of input range! (Range :0~8)"));
		SetDlgItemInt(IDC_EDIT_CAMERA_COUNT_MODIFY, m_nJobCamCount[nJob - 1]);
	}
	else
	{
		//viewer개수는 camera개수보다 많거나 같아야 한다.
		if (m_nJobViewerCount[nJob - 1] < nCam)
		{
			//viewer index 초기화
			for (int j = m_nJobViewerCount[nJob - 1]; j < nCam; j++)
			{
				m_nJobViewerIndex[nJob - 1][j] = 0;
			}

			m_nJobViewerCount[nJob - 1] = nCam;
		}

		//Machine view index, cam index 초기화
		if (nCam > m_nJobCamCount[nJob - 1])
		{
			for (int j = m_nJobCamCount[nJob - 1]; j < nCam; j++)
			{
				m_nMachineViewIndex[nJob - 1][j] = 0;
				m_nJobCamIndex[nJob - 1][j] = 0;
			}
		}

		//camera 추가된 정보 초기화
		for (int j = m_nJobCamCount[nJob - 1]; j < nCam; j++)
		{
			m_nNumOfUsingLightCount[nJob - 1][j] = 1;

			for (int n = 0; n < m_nNumOfUsingLightCount[nJob - 1][j]; n++)
			{
				m_nLightControlId[nJob - 1][j][n] = 1;
				m_nLightChannelId[nJob - 1][j][n] = 0;
				m_stLightChannelName[nJob - 1][j][n] = "Reserved";
			}
			for (int n = 0; n < MAX_LIGHT_INDEX; n++)
			{
				m_stLightIndexName[nJob - 1][j][n] = "Reserved";
			}
		}

		m_nJobCamCount[nJob - 1] = nCam;
		
		int nJob = m_cb_select_job.GetCurSel();

		if (nJob != 0)
		{
			SetDlgItemInt(IDC_EDIT_CAMERA_COUNT_MODIFY, m_nJobCamCount[nJob - 1]);

			init_grid_view_setting(m_grid_job_viewer, 5, 2);
			draw_grid_job(m_grid_job_viewer, 5, 2);

			m_cbSelectCamera.ResetContent();
			m_cbSelectLightCam.ResetContent();

			CString strCam = "";

			for (int i = 0; i < m_nJobCamCount[nJob - 1]; i++)
			{
				strCam.Format("Cam %d", i + 1);
				m_cbSelectCamera.AddString(strCam);
				m_cbSelectLightCam.AddString(strCam);
			}

			m_cbSelectCamera.SetCurSel(0);
			m_cbSelectLightCam.SetCurSel(0);

			int nPosition = m_nJobCamCount[nJob - 1] * m_nJobPosition[nJob - 1];

			init_grid_view_setting(m_grid_position_viewer, 2, nPosition + 1);
			draw_grid_position_viewer(m_grid_position_viewer, 2, nPosition + 1);

			init_grid_view_setting(m_grid_job_cam_viewer, 2, m_nJobCamCount[nJob - 1] + 1);
			draw_grid_job_cam(m_grid_job_cam_viewer, 2, m_nJobCamCount[nJob - 1] + 1);

			int nCamSum = 0;

			for (int i = 0; i < m_nJobCount; i++)
			{
				nCamSum += m_nJobCamCount[i];
			}

			SetDlgItemInt(IDC_EDIT_CAMERA_COUNT, nCamSum);
		}
		else
		{
			init_grid_view_setting(m_grid_job_viewer, 5, m_nJobCount+1);
			draw_grid_job(m_grid_job_viewer, 5, m_nJobCount+1);

			int nCamSum = 0;

			for (int i = 0; i < m_nJobCount; i++)
			{
				nCamSum += m_nJobCamCount[i];
			}

			SetDlgItemInt(IDC_EDIT_CAMERA_COUNT, nCamSum);
		}
	}
}

//종료 버튼
void DlgINISetting::OnBnClickedBtnIniExit()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CFormMainView* pMain = (CFormMainView*)m_pMain->m_pForm[FORM_MAIN];

	if (m_pMain->fnSetMessage(2, "Exit the INI Program?") != TRUE)	return;
	::SendMessage(GetSafeHwnd(), WM_CLOSE, NULL, NULL);

	//HSJ 2021-01-06 INI파일 종료 시 프로그램도 끌지 말지
	if (m_pMain->fnSetMessage(2, "Exit the Program?") != TRUE)	return;

	m_pMain->m_bProgramEnd = TRUE;
	Delay(100);

	GetParent()->SendMessage(WM_CLOSE);
}

void DlgINISetting::InitCameraData()
{
	int camCount = GetCameraList(m_CameraSerial, m_CameraName);

	if (camCount > 0)
	{
		CString strCam;
		for (int i = 0; i < camCount; i++)
		{
			strCam.Format("Cam_%d", i + 1);
			((CComboBox*)GetDlgItem(IDC_CB_NUM_OF_USING_CAMERA2))->AddString(strCam);
			strCam.Format("%s", m_CameraName.at(i).c_str());
			((CComboBox*)GetDlgItem(IDC_CB_NUM_OF_USING_CAMERA4))->AddString(strCam);
		}
		((CComboBox*)GetDlgItem(IDC_CB_NUM_OF_USING_CAMERA2))->SetCurSel(0);
		((CComboBox*)GetDlgItem(IDC_CB_NUM_OF_USING_CAMERA4))->SetCurSel(0);

		strCam.Format("%s(%s)", m_CameraName.at(0).c_str(), m_CameraSerial.at(0).c_str());
		GetDlgItem(IDC_STATIC_CAMERA_NAME)->SetWindowTextA(strCam);
	}
}
void DlgINISetting::OnBnClickedBtnIniCamMatchig()
{
	int ID = ((CComboBox*)GetDlgItem(IDC_CB_NUM_OF_USING_CAMERA2))->GetCurSel();
	int SELCAM = ((CComboBox*)GetDlgItem(IDC_CB_NUM_OF_USING_CAMERA4))->GetCurSel();

	if (SELCAM >= 0 && SELCAM < m_CameraName.size())
	{
		if (AfxMessageBox("Apply Camera Matching ?", MB_YESNO) != IDYES) return;

		CString strKey, strData, strSection;
		strSection.Format("CAMERA%d_INFO", ID + 1);
		strKey.Format("SERIAL");
		strData.Format("%s", m_CameraSerial.at(SELCAM).c_str());

		CString strFilePath, m_strSettingFilePath;
		m_strSettingFilePath = m_pMain->m_strConfigDir;
		strFilePath.Format("%sCamera.ini", m_strSettingFilePath);

		::WritePrivateProfileStringA(strSection, strKey, strData, strFilePath);

		m_nCamSerial[ID] = m_CameraSerial.at(SELCAM).c_str();
		draw_grid_camera_viewer(m_grid_camera_viewer, 6, m_nNumOfCamera + 1);
	}
}


int DlgINISetting::GetCameraList(vector<string> &serial, vector<string> &name)
{
	int  nDevNo = 0;

#ifdef __CAMERA_H__
	try
	{
		CTlFactory* m_ptlFactory;
		DeviceInfoList_t			m_DIList;

		m_ptlFactory = &CTlFactory::GetInstance();
		INT nDevCnt = m_ptlFactory->EnumerateDevices(m_DIList);

		if (nDevCnt > 0)
		{
			// Get Serial Number
			CString strSerial,strName;

			for (DeviceInfoList_t::iterator it = m_DIList.begin(); it != m_DIList.end(); it++)
			{
				strSerial = (*it).GetSerialNumber().c_str();
				strName = (*it).GetUserDefinedName().c_str();

				serial.push_back(string(strSerial));
				name.push_back(string(strName));

				nDevNo++;
			}
		}
	}
	catch (GenICam::GenericException& e)
	{
		CString strError = e.what();
		nDevNo = 0;
	}
#endif
#ifdef  _DAHUHA
	CSystem& systemObj = CSystem::getInstance();
	Dahua::Infra::TVector<ICameraPtr>	m_vCameraPtrList;
	bool bRet = systemObj.discovery(m_vCameraPtrList);

	ICameraPtr cameraSptr;
	CString strSerial, strName;

	for (int i = 0; i < m_vCameraPtrList.size(); i++)
	{
		cameraSptr = m_vCameraPtrList[i];		
		strSerial = cameraSptr->getSerialNumber();
		strName = cameraSptr->getName();

		serial.push_back(string(strSerial));
		name.push_back(string(strName));

		nDevNo++;
	}
#endif
	return nDevNo;
}

int DlgINISetting::GetIpConfigCameraList()
{
	int  nDevNo = 0;

#ifdef __CAMERA_H__
	try
	{
		//CTlFactory* m_ptlFactory;
		//DeviceInfoList_t			m_DIList;

		//m_ptlFactory = &CTlFactory::GetInstance();

		//IGigETransportLayer* pTl = dynamic_cast<IGigETransportLayer*>(m_ptlFactory->CreateTl(Pylon::BaslerGigEDeviceClass));
		//if (pTl == NULL)	return nDevNo;
		//INT nDevCnt = pTl->EnumerateAllDevices(m_DIList);

		//if (nDevCnt > 0)
		//{
		//	bool isDhcp = false;
		//	bool isStatic = true;

		//	String_t macAddress = "";
		//	String_t ipAddress = "";
		//	String_t subnetMask = "255.255.255.0";
		//	String_t defaultGateway = "0.0.0.0";
		//	String_t userDefinedName = "";

		//	for (DeviceInfoList_t::iterator it = m_DIList.begin(); it != m_DIList.end(); it++)
		//	{
		//		userDefinedName = it->GetUserDefinedName();
		//		macAddress == it->GetMacAddress();

		//		bool setOk = pTl->BroadcastIpConfiguration(macAddress, isStatic, isDhcp,
		//			ipAddress, subnetMask, defaultGateway, userDefinedName);

		//		if (setOk)					pTl->RestartIpConfiguration(macAddress);
		//
		//		nDevNo++;
		//	}
		//}

		//m_ptlFactory->ReleaseTl(pTl);
	}
	catch (GenICam::GenericException& e)
	{
		CString strError = e.what();
		nDevNo = 0;
	}
#endif

	return nDevNo;
}

void DlgINISetting::getAdapterList() // 20211227
{
	std::vector<std::string> listadapter;
	DevicesHandler device;
	device.enumerateNics(listadapter);

	for (auto ir = listadapter.begin(); ir != listadapter.end(); ++ir)
		m_listNetworkAdapter.AddString((*ir).c_str());
}

bool DlgINISetting::runCmdWindow(unsigned long index, bool benable)
{
	CString utf8String, param;

	utf8String.Format("%s\\toggle-nic.exe", m_pMain->m_szWorkingPath);
	param.Format("%d --%s", index, benable ? "enable" : "disable");

	SHELLEXECUTEINFO sei = { sizeof(sei) };
	sei.lpVerb = "runas";
	sei.lpFile = utf8String;
	sei.hwnd = NULL;
	sei.lpParameters = param;
	sei.nShow = SW_NORMAL;

	if (!ShellExecuteEx(&sei))
	{
		DWORD dwError = GetLastError();
		if (dwError == ERROR_CANCELLED)					return false;
	}
	else
	{
		return false;
	}

	return true;
}

void DlgINISetting::OnDblclkListNetwork()
{
	int nList = 0;
	nList = m_listNetworkAdapter.GetCurSel();   

	runCmdWindow(nList, false);
	SetTimer(nList, 2000, NULL);
}


void DlgINISetting::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 여기에 메시지 처리기 코드를 추가 및/또는 기본값을 호출합니다.
	KillTimer(nIDEvent);

	runCmdWindow(nIDEvent, true);

	CDialogEx::OnTimer(nIDEvent);
}


void DlgINISetting::OnCbnSelchangeCbNumOfUsingCamera4()
{
	CString strCam;
	int SELCAM = ((CComboBox*)GetDlgItem(IDC_CB_NUM_OF_USING_CAMERA4))->GetCurSel();

	if (SELCAM >= 0 && SELCAM < m_CameraName.size())
	{
		strCam.Format("%s(%s)", m_CameraName.at(SELCAM).c_str(), m_CameraSerial.at(SELCAM).c_str());
		GetDlgItem(IDC_STATIC_CAMERA_NAME)->SetWindowTextA(strCam);
	}
}
