// DlgMarkCopy.cpp: 구현 파일
//

//#include "pch.h"
#include "stdafx.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"
#include "DlgMarkCopy.h"
#include "afxdialogex.h"


// DlgMarkCopy 대화 상자

IMPLEMENT_DYNAMIC(DlgMarkCopy, CDialogEx)

DlgMarkCopy::DlgMarkCopy(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_DLG_MARK_COPY, pParent)
{
	m_pMain = (CLET_AlignClientDlg*)AfxGetMainWnd();

}

DlgMarkCopy::~DlgMarkCopy()
{
}

void DlgMarkCopy::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_CB_SRC_JOB, m_cbSrcJob);
	DDX_Control(pDX, IDC_CB_SRC_CAM, m_cbSrcCam);
	DDX_Control(pDX, IDC_CB_SRC_POS, m_cbSrcPos);
	DDX_Control(pDX, IDC_CB_SRC_INDEX, m_cbSrcIndex);
	DDX_Control(pDX, IDC_CB_DST_JOB, m_cbDstJob);
	DDX_Control(pDX, IDC_CB_DST_CAM, m_cbDstCam);
	DDX_Control(pDX, IDC_CB_DST_POS, m_cbDstPos);
	DDX_Control(pDX, IDC_CB_DST_INDEX, m_cbDstIndex);
}


BEGIN_MESSAGE_MAP(DlgMarkCopy, CDialogEx)
	ON_BN_CLICKED(IDC_BTN_MARK_COPY, &DlgMarkCopy::OnBnClickedBtnMarkCopy)
	ON_CBN_SELCHANGE(IDC_CB_SRC_JOB, &DlgMarkCopy::OnCbnSelchangeCbSrcJob)
	ON_CBN_SELCHANGE(IDC_CB_SRC_CAM, &DlgMarkCopy::OnCbnSelchangeCbSrcCam)
	ON_CBN_SELCHANGE(IDC_CB_DST_JOB, &DlgMarkCopy::OnCbnSelchangeCbDstJob)
	ON_CBN_SELCHANGE(IDC_CB_DST_CAM, &DlgMarkCopy::OnCbnSelchangeCbDstCam)
END_MESSAGE_MAP()


// DlgMarkCopy 메시지 처리기

// hsj 2023-01-02 Mark Copy Dialog 추가
BOOL DlgMarkCopy::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// TODO:  여기에 추가 초기화 작업을 추가합니다.

	for (int i = 0; i < m_pMain->vt_job_info.size(); i++)
	{
		CString strJob;
	
		strJob.Format("%s", m_pMain->vt_job_info[i].job_name.c_str());
		m_cbSrcJob.AddString(strJob);
		m_cbDstJob.AddString(strJob);
	}

	for (int i = 0; i < MAX_PATTERN_INDEX; i++)
	{
		CString strIndex;
		strIndex.Format("%d", i + 1);
		m_cbSrcIndex.AddString(strIndex);
		m_cbDstIndex.AddString(strIndex);
	}

	return TRUE;  // return TRUE unless you set the focus to a control
				  // 예외: OCX 속성 페이지는 FALSE를 반환해야 합니다.
}



void DlgMarkCopy::OnBnClickedBtnMarkCopy()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	CString strFilePath;

	int nsrcJob = ((CComboBox*)GetDlgItem(IDC_CB_SRC_JOB))->GetCurSel();
	int nsrcCam = ((CComboBox*)GetDlgItem(IDC_CB_SRC_CAM))->GetCurSel();
	int nsrcPos = ((CComboBox*)GetDlgItem(IDC_CB_SRC_POS))->GetCurSel();
	int nsrcIndex = ((CComboBox*)GetDlgItem(IDC_CB_SRC_INDEX))->GetCurSel();

	//COPY할 이미지
	strFilePath.Format("%s%s\\PAT%d%d%d.bmp", m_pMain->m_strCurrentModelPath, m_pMain->vt_job_info[nsrcJob].job_name.c_str(), nsrcCam, nsrcPos, nsrcIndex);

	cv::Mat srcMat = cv::imread(strFilePath.GetString(), cv::IMREAD_GRAYSCALE);

	//COPY되는 패턴 경로

	int ndstJob = ((CComboBox*)GetDlgItem(IDC_CB_DST_JOB))->GetCurSel();
	int ndstCam = ((CComboBox*)GetDlgItem(IDC_CB_DST_CAM))->GetCurSel();
	int ndstPos = ((CComboBox*)GetDlgItem(IDC_CB_DST_POS))->GetCurSel();
	int ndstIndex = ((CComboBox*)GetDlgItem(IDC_CB_DST_INDEX))->GetCurSel();

	strFilePath.Format("%s%s\\PAT%d%d%d.bmp", m_pMain->m_strCurrentModelPath, m_pMain->vt_job_info[ndstJob].job_name.c_str(), ndstCam, ndstPos, ndstIndex);

	cv::imwrite(strFilePath.GetString(), srcMat);

	m_pMain->GetMatching(ndstJob).getHalcon(ndstCam, ndstPos, ndstIndex).setModelRead(true);

	//ROI도 COPY
	CRect rectROI;

	if (m_pMain->vt_job_info[ndstIndex].model_info.getAlignInfo().getUseMultiplexRoi())
	{
		rectROI = m_pMain->GetMatching(ndstIndex).getSearchROI2(nsrcCam, nsrcPos, nsrcIndex);
		m_pMain->GetMatching(ndstIndex).setSearchROI2(ndstCam, ndstPos, rectROI, ndstIndex);
	}

	AfxMessageBox("Complete Copy!");
}


void DlgMarkCopy::OnCbnSelchangeCbSrcJob()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_cbSrcCam.ResetContent();

	int nJob = m_cbSrcJob.GetCurSel();

	CString str;

	for (int i = 0; i < m_pMain->vt_job_info[nJob].camera_index.size(); i++)
	{
		int cam = m_pMain->vt_job_info[nJob].camera_index[i];
		str.Format("%s", m_pMain->m_stCamInfo[cam].cName);
		m_cbSrcCam.AddString(str);
	}
}

void DlgMarkCopy::OnCbnSelchangeCbSrcCam()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_cbSrcPos.ResetContent();

	int nJob = m_cbSrcJob.GetCurSel();
	int nPos = m_pMain->vt_job_info[nJob].num_of_position;

	CString str;

	for (int i = 0; i < nPos; i++)
	{
		str.Format("Pos %d", i + 1);
		m_cbSrcPos.AddString(str);
	}
}


void DlgMarkCopy::OnCbnSelchangeCbDstJob()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_cbDstCam.ResetContent();

	int nJob = m_cbDstJob.GetCurSel();

	CString str;

	for (int i = 0; i < m_pMain->vt_job_info[nJob].camera_index.size(); i++)
	{
		int cam = m_pMain->vt_job_info[nJob].camera_index[i];
		str.Format("%s", m_pMain->m_stCamInfo[cam].cName);
		m_cbDstCam.AddString(str);
	}
}


void DlgMarkCopy::OnCbnSelchangeCbDstCam()
{
	// TODO: 여기에 컨트롤 알림 처리기 코드를 추가합니다.
	m_cbDstPos.ResetContent();

	int nJob = m_cbDstJob.GetCurSel();
	int nPos = m_pMain->vt_job_info[nJob].num_of_position;

	CString str;

	for (int i = 0; i < nPos; i++)
	{
		str.Format("Pos %d", i + 1);
		m_cbDstPos.AddString(str);
	}
}

