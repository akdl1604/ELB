#pragma once


// CCramerSRuleDlg ��ȭ �����Դϴ�.

class CCramerSRuleDlg : public CDialogEx
{
	DECLARE_DYNAMIC(CCramerSRuleDlg)

public:
	CCramerSRuleDlg(CWnd* pParent = NULL);   // ǥ�� �������Դϴ�.
	virtual ~CCramerSRuleDlg();

// ��ȭ ���� �������Դϴ�.
	enum { IDD = IDD_CRAMERSRULE_DIALOG };

	double camX,camY;
	double revisionX,revisionY;
	double remainX,remainY;
	double theta;
	double m_drotateX,m_drotateY;
protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV �����Դϴ�.

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClicked1009();
	virtual BOOL OnInitDialog();

	void SetCameraXY(double x,double y);
	void SetRevisionXY(double x,double y);
	void SetRemainXY(double x,double y);
	void SetTheta(double angle);
	void GetRotateXY(double *x,double *y);
};
