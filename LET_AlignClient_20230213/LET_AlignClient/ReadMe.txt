================================================================================
    MFC ���̺귯�� : LET_AlignClient ������Ʈ ����
================================================================================

���� ���α׷� �����翡�� �� LET_AlignClient ���� ���α׷��� 
��������ϴ�. �� ���� ���α׷��� MFC�� �⺻ ������ ���� �� �Ӹ� �ƴ϶� ���� 
���α׷��ۼ��� ���� �⺻ ������ �����մϴ�.

�� ���Ͽ��� LET_AlignClient ���� ���α׷��� �����ϴ� �� ���Ͽ� ����
��� ������ ���ԵǾ� �ֽ��ϴ�.

LET_AlignClient.vcxproj
    ���� ���α׷� �����縦 ����Ͽ� ������ VC++ ������Ʈ�� �� ������Ʈ �����Դϴ�. 
    ������ ������ Visual C++ ������ ���� ������ ���� ���α׷� �����縦 ����Ͽ� ������ 
    �÷���, ���� �� ������Ʈ ��ɿ� ���� ������ ��� �ֽ��ϴ�.

LET_AlignClient.vcxproj.filters
    ���� ���α׷� �����縦 ����Ͽ� ������ VC++ ������Ʈ�� ���� �����Դϴ�. 
    �� ���Ͽ��� ������Ʈ�� ���ϰ� ���� ���� ���� ������ ��� �ֽ��ϴ�. �̷��� 
    ������ Ư�� ��忡�� ������ Ȯ������� �׷�ȭ�� ������ ǥ���ϱ� ���� 
    IDE���� ���˴ϴ�. ���� ��� ".cpp" ������ "�ҽ� ����" ���Ϳ� ����Ǿ� 
    �ֽ��ϴ�.


LET_AlignClient.h
    ���� ���α׷��� �⺻ ��� �����Դϴ�. ���⿡�� �ٸ� ������Ʈ ����
    ���(Resource.h ����)�� ��� �ְ� CLET_AlignClientApp ���� ���α׷�
    Ŭ������ �����մϴ�.

LET_AlignClient.cpp
    ���� ���α׷� Ŭ���� CLET_AlignClientApp��(��) ��� �ִ� �⺻ ���� ���α׷�
    �ҽ� �����Դϴ�.

LET_AlignClient.rc
    ���α׷����� ����ϴ� ��� Microsoft Windows ���ҽ��� ����Դϴ�.
 ���⿡�� RES ���� ���͸��� ����� ������, ��Ʈ�� �� Ŀ����
    ���Ե˴ϴ�. �� ������ Microsoft Visual C++���� ����
    ������ �� �ֽ��ϴ�. ������Ʈ ���ҽ��� 1042�� �ֽ��ϴ�.

res\LET_AlignClient.ico
    ���� ���α׷��� ���������� ���Ǵ� ������ �����Դϴ�. �� ��������
    �� ���ҽ� ������ LET_AlignClient.rc�� ���� ���Ե˴ϴ�.

res\LET_AlignClient.rc2
    �� ���Ͽ��� Microsoft Visual C++ �̿��� �ٸ� �������� ������ ���ҽ��� 
    ��� �ֽ��ϴ�. ���ҽ� ������� ������ �� ���� ��� ���ҽ���
    �� ���Ͽ� �־�� �մϴ�.


/////////////////////////////////////////////////////////////////////////////

���� ���α׷� �����翡�� �ϳ��� ��ȭ ���� Ŭ������ ��������ϴ�.

LET_AlignClientDlg.h, LET_AlignClientDlg.cpp - ��ȭ ����
    �� ���Ͽ��� CLET_AlignClientDlg Ŭ������ ��� �ֽ��ϴ�. �� Ŭ������
    ���� ���α׷��� �� ��ȭ ���� ������ �����մϴ�. ��ȭ ������ ���ø���
    Microsoft Visual C++���� ������ �� �ִ� LET_AlignClient.rc�� �ֽ��ϴ�.


/////////////////////////////////////////////////////////////////////////////

��Ÿ ���:

ActiveX ��Ʈ��
    ���� ���α׷����� ActiveX ��Ʈ���� ����� �� �ֵ��� �����մϴ�.

Windows ����
    ���� ���α׷����� TCP/IP ��Ʈ��ũ�� ����� ����� �����մϴ�.

/////////////////////////////////////////////////////////////////////////////

��Ÿ ǥ�� ����:

StdAfx.h, StdAfx.cpp
    �� ������ �̸� �����ϵ� ��� ����(PCH)�� LET_AlignClient.pch��
    �̸� �����ϵ� ���� ������ StdAfx.obj�� �����ϴ� �� ���˴ϴ�.

Resource.h
    �� ���ҽ� ID�� �����ϴ� ǥ�� ��� �����Դϴ�.
    Microsoft Visual C++���� �� ������ �а� ������Ʈ�մϴ�.

LET_AlignClient.manifest
	���� ���α׷� �Ŵ��佺Ʈ ������ Windows XP���� Ư�� ������ Side-by-Side 
	��������� ���� ���� ���α׷� ���Ӽ��� �����ϴ� �� ���˴ϴ�. �δ��� �� ������ 
	����Ͽ� ������� ĳ�ÿ��� ������ ��������� �ε��ϰų� ���� ���α׷����� ���� 
	��������� �ε��մϴ�. ���� ���α׷� �Ŵ��佺Ʈ�� ���� ���α׷� ���� ���ϰ� ���� 
	������ ��ġ�� �ܺ� .manifest ���Ϸ� ������� ���� ���Եǰų� ���ҽ��� ���·� 
	���� ���Ͽ� ���Ե� �� �ֽ��ϴ�. 
/////////////////////////////////////////////////////////////////////////////

��Ÿ ����:

���� ���α׷� ������� "TODO:"�� ����Ͽ� �߰��ϰų� ����� �����ؾ� �ϴ�
�ҽ� �ڵ� �κ��� ��Ÿ���ϴ�.

���� ���α׷��� ���� DLL���� MFC�� ����ϴ� ��� �ش� MFC DLL�� 
������ؾ� �մϴ�. �׸��� ���� ���α׷��� � ü���� ��Ķ�� 
�ٸ� �� ����ϴ� ��� �ش� ����ȭ�� ���ҽ��� MFC100XXX.DLL�� 
������ؾ� �մϴ�. �� �ΰ��� �׸� ���� �ڼ��� ������ 
MSDN �������� Visual C++ ���� ���α׷� ����� �׸��� 
�����Ͻʽÿ�.

/////////////////////////////////////////////////////////////////////////////
//2021-08-06 KJH ī�޶� Tap �̰��� ���� ����
//KJH 2021-09-02 mBar->kPa
//2021-09-13 KJH Nozzle Check Grab �߰�(2021-09-10 �߰�)
//2021-09-13 Model���� ��� �켱 ���� / Main View Total�� ����ϰ� �켱 ����
//2021-09-13 Trace Image LOG �������� �ʾҴ�.. ����
//2021-09-14 InspNozzleDistance ����
//2021-09-16 InspNozzleDistance �α� ���� ���� , Center Align �α� ���� ����
//2021-09-17 InspNozzleDistance ���� �α� �߰� , Center Align �̹��� �α� ���� ����
//KJH 2021-09-17 Phone ���� Center Align Log ����
//KJH 2021-09-17 ������ ���
//2021-09-18 Total View ���Ƶа� ǰ.. 1�� ȭ�� Ȯ�� �� �ڿ� ȭ�� Ŭ���� ������ɸ�
//2021-09-25 FDC ���� �غ�
//2021-09-27 Circle �ű� �˰����� �պ� ������
//KJH 2021-10-21 FDC �����Լ� �߰�_Circle
//KJH 2021-10-26 �˰����� �и�( 0 : KJH ��� / 1 : Tkyuha ���)
//m_dBMCircleRadius -> m_dCircleRadius_CC
//KJH 2021-10-30 Pane Cam Index 0�� �������� ����
//KJH 2021-10-30 Save End Check ���� �߰�
//KJH 2021-11-01 Circle Trace Data ������ ��� �� ���� �߰�
//KJH 2021-11-06 Nozzle Insp ���� Test�� ��ư ����
//KJH 2021-11-06 2���� ���� ó�� �߰�
//KJH 2021-11-08 UT INSP ���� �߰�
//KJH 2021-11-12 Trace Parameter �۾� ����
//KJH 2021-11-15 Center Align�� �� Trace ���� �αװ�����???
//KJH 2021-11-15 Trace ����� ���Ǵ� ���� Align Save�� ��︮�� ����
//KJH 2021-11-17 ELB Server Check ��� �켱 ����
//KJH 2021-11-30 ���� OK������ ��ȣ ��� �߰�
//KJH 2021-11-30 DustInsp ������ �ֻ��� �����̴��� ������
//KJH 2021-12-01 Dust Insp Image Para �߰�
//KJH 2021-12-02 Dust Insp�� ����� ROI ǥ��
//KJH 2021-12-07 Trace ���� ���� �˻� ExtractProfileData�� �ȱ�� �۾� ����
//KJH 2021-12-07 Trace Image & ���� �˻� ���� �̹��� �α� �߰� (Tact ���� ����� ������ ó�� �ʿ�)
//KJH 2021-12-11 Total Value �̻����� ����
//KJH 2021-12-11 ��Ʀ �˻� ���ܿ��� Master MP �������� ���� MP�������� ���� + Margin 3
//KJH 2021-12-20 Display�� Combo box Mark Index ��Ī�۾�
//KJH 2021-12-21 revisiondata 0 ���� ����!
//KJH 2021-12-25 �˻� Start Pos ����
//KJH 2021-12-25 Wetout Insp ���� ����
//KJH 2021-12-25 ���� ���� , �ӽ� ����͸��뵵
//KJH 2021-12-25 ��� ������ ���� �˻� �Ķ���ͷ� �и�����
//KJH 2021-12-25 CIRCLE_RADIUS + 30 -> CIRCLE_RADIUS MP ���� �������� ����
//KJH 2021-12-26 �պ� �� Diff Insp ���� ��Ī �۾�
//KJH 2021-12-29 Pattern Index �߰�
//KJH 2021-12-29 Tap�� �´� �������� �ʱ�ȭ
//KJH 2021-12-29 MarkIndex�߰�
//KJH 2021-12-31 Machine View Camera Index Debug
//KJH 2021-12-31 Camera Grab Delay ��� �߰�
//KJH 2022-01-03 State Insp OK,NG 1:1 ��Ī
//KJH 2022-01-03 ��ȣ ��÷� ����
//KJH 2022-01-03 Camera ini ���о���°� ����ó��
//KJH 2022-01-03 ���忡�� �������̽� ���� ����(Nozzle Gap�� ��ħ)
//KJH 2022-01-03 State Insp OK,NG 1:1 ��Ī
//KJH 2022-01-05 �������� Viewer ToolBar �߰��۾�
//KJH 2022-01-05 Center Nozzle Align �����ϱ� ���� ExposureTime ����
//KJH 2022-01-07 ��ȣ ü�� ����
//KJH 2022-01-07 Rotate Auto Calc (Caliper -> Circle)
//KJH 2022-01-10 Ķ���� bit���� ( 48 : Side , 49 : Center, 50 : Gap)
//KJH 2022-01-12 CC Find Filter Insp Para�� ����
//KJH 2022-01-12 WetoutInsp Para Insp Para�� �б�
//KJH 2022-01-19 Black Nozzle Search
//KJH 2022-01-19 White Nozzle Search
//KJH 2022-01-19 Nozzle Align 4ȸ 1Cycle �ݺ������ϰ� ����
//KJH 2022-01-25 MP Edge Noise���� ��� ����(���ν� �ٹ�)
//KJH 2022-01-25 Circle Trace ��ó�� �߰�(MP ����� �ʿ�, PN�� Ȯ�� �ʿ�)
//KJH 2022-01-25 ELB Cover CInk mode
//KJH 2022-01-30 MP Trace ����ó�� �����۾�
//KJH 2022-02-01 �˻� ���� ���� boundingRect Size -> crackStats
//KJH 2022-02-01 �̹��˻� ������� �̵�
//KJH 2022-02-01 �̹��˻� �˰����� ����
//KJH 2022-02-03 CC Edge Find insert
//KJH 2022-02-03 DustInsp���� ���ʿ� �α� ���� ����
//KJH 2022-02-03 MP Align ���� �߰�
//KJH 2022-02-03 MP ���� �̹��˻� �߰� �� CC Edge ã�� Ȱ��ȭ
//KJH 2022-02-03 PreDustInsp ��ó�� ���� ����
//KJH 2022-02-03 CircleAlign rename
//KJH 2022-02-03 EmptyInspection ������
//KJH 2022-02-05 Metal Over flow Margin
//KJH 2022-03-10 L-Check Error�� ��쿡�� Manual Mark �����ϰ� ����
//KJH 2022-03-10 PF Matching ���� Image Index 0 �̹��� �׸���
//KJH 2022-03-12 PF INSP FDC ���� �߰�
//KJH 2022-03-12 MatchingRate Setting Value FDC ���� �߰�
//KJH 2022-03-15 ���� ��û���� �� L, R ������ �Ÿ��� ���� ���� ����
//KJH 2022-03-16 Act,Seq Time Display �߰�
//KJH 2022-03-17 VisionActTime Display �߰�
//KJH 2022-03-17 Trend Insert Result CS �߰�
//KJH 2022-03-24 Dopo length ���ν� ����ó�� �߰�
//KJH 2022-04-01 Reel Align Init bit���� ����ó�� �߰�
//KJH 2022-04-02 ª�� Tape �˻�� �Ÿ��� ����� ��Į�� ���. �Է��� ���ͷι޾ƾ� �Ǵ� ����(���̵弱 ������ ������ ���ͷ�)
//KJH 2022-04-08 Min Spec 0���� ������ �˻� �������ϵ��� ����
//KJH 2022-04-08 lParam = 0���� ���� / 1�� Center�� ����Ҷ� , 0�� ����
//KJH 2022-04-09 PN ������ �� �˻� �߰�
//KJH 2022-04-13 Calibration Log �߰�
//KJH 2022-04-14 PF Attach Align L Check �����
//KJH 2022-04-22 CameraDistance ��� ����
//KJH 2022-04-23 LCheck Scale ��� �߰�
//KJH 2022-04-23 Auto�� Center Camera CenterAlign Expousure������ ���� ����
//KJH 2022-04-25 Camera�� Display�߰�
//KJH 2022-04-26 Exposure Restore ��� �߰�
//KJH 2022-04-27 ���� ��������߰�
//KJH 2022-04-28 PN ������ ��� ���� �߰�
//KJH 2022-04-29 Dopo Start�� ������ ��ȭ ���
//KJH 2022-04-29 InkLack Insp ����߰�(���Ӽ� �˻�, �ѷ� �˻�)
//KJH 2022-04-29 CInkLack Insp ����߰�(���Ӽ� �˻�, �ѷ� �˻�) HTK 2022-05-19 CInkLack Insp �ߺ����� ����
//KJH 2022-04-29 MincountCheckInsp �߰�
//KJH 2022-04-29 RDP ����� ��û���� ��ũ ��� ����
//KJH 2022-04-29 ELB CENTER ALIGN LIVE VIEW ���̵� ���� �׸��� �߰�
//KJH 2022-05-02 Cink Wetout Insp �˻�� CC���� Interlock
//KJH 2022-05-03 Spec �� �߰� , Min Spec�� ������ ���� �˻� �߰�
//KJH 2022-05-04 Wetout �˻� ��� �̹��� ���� ����� ���� result ���� ����
//KJH 2022-05-05 R Diff ����� ���� ���ֺ��� ���� �Ÿ� ����
//KJH 2022-05-05 72�� �������� 90���� ���� 0, 90 , 180, 270 , 0���� ����[������ 0 data�� �̻��]
//KJH 2022-05-05 Nozzle XYZ �˻� ��� �߰�
//KJH 2022-05-05 Nozzle XYZ �˻� ��� �߰� Pos ������ ����ؾ���
//KJH 2022-05-05 Ư�� �������� R�� ���
//KJH 2022-05-06 �̹��� Shift ��� ���� getProcBuffer �����ִ°� ����
//KJH 2022-05-06 CC to PN �˻� Display ����
//KJH 2022-05-09 Mincount ��� ������� ����
//KJH 2022-05-09 ����, ������ �Ѵ� �����϶� �̵����� ��輱���� ���� �߰�
//KJH 2022-05-09 Gap Offset Line ��� ����
//KJH 2022-05-09 R Judge ��� ���� �߰�
//KJH 2022-05-09 Nozzle Align Display Size ����
//KJH 2022-05-10 Fixture ��¥ ����
//KJH 2022-05-10 Fixture ������ ����
//KJH 2022-05-10 Fixture ��Ͻ� ��¥ ǥ��
//KJH 2022-05-11 1Cam 1SHot Film Align[Grab Bit �߰� ����]
//KJH 2022-05-11 Film Align Grab bit ���� Process�߰�
//KJH 2022-05-11 Film Align Grab bit �߰�
//KJH 2022-05-13 Nozzle XYZ Insp Log �и�
//KJH 2022-05-14 �̵��� ���� �ȵǴ��� ����...
//KJH 2022-05-14 CInk2 ����϶� Overflow�� Spec���� �����ϴ°ɷ� ����
//KJH 2022-05-14 ���ö ������ ��û �������� Nozzle Align Log �� ����, XYZ �˻縸 �����ϴ°ɷ�
//KJH 2022-05-14 Nozzle XYZ Insp �߰��� �˻� �� Center Align ExposureTime���� ����
//KJH 2022-05-16 ���� �˻� Start ��ġ +15 -> +10 ���� ����
//KJH 2022-05-16 ELB Center Align �����ϱ� ���� ExposureTIme ����
//KJH 2022-05-18 Record�� View Grayȭ�� ���� �ǽ� �κ� ����
//KJH 2022-05-19 CG Thickness �޾Ƽ� Z Gap �����ϱ�
//KJH 2022-05-19 XYZ �˻�� Spec �����ϱ�
//KJH 2022-05-19 Z Gap Insp result File ����
//KJH 2022-05-19 Nozzle Calibration�� Cam Bit �̸� ���ֱ�
//KJH 2022-05-24 BM ���ۺ��� ��� ���ۺα��� �Ÿ� ��� �߰�
//KJH 2022-05-25 Length -> BMinSpec���� UI ����
//KJH 2022-05-25 BM In <-> Cink In Insp �߰�
//KJH 2022-05-26 Display ����ȭ �۾�����
//KJH 2022-05-26 �̵��� �˻�� ������ǥ��� ȭ����ǥ���� 0�� ���� �ݿ�
//KJH 2022-05-26 ���� �ּ�,�ִ� �� ���÷��� ����ȭ
//KJH 2022-05-26 Wetout_Out MIN,MAX ��ǥ ǥ�� �߰�
//KJH 2022-05-26 Display UI ���� �۾� ����
//KJH 2022-05-26 6View -> 9 View �б��۾�
//KJH 2022-05-26 Individual Calibration ��¥ ����
//KJH 2022-05-28 Roi ȭ�� ������ ��Ͻ� ��Ŀ ��ǥ ���� ����
//KJH 2022-05-31 BM Egde �ۿ��� �������� �˻��ϴ��� ���ʿ��� �ٱ����� ���� ����
//KJH 2022-06-01 Nozzle Gap ���� ������ ��ٸ��� ���� Delay �߰�
//KJH 2022-06-06 GrabTime ������û�� ���� ���� �۾�
//KJH 2022-06-06 R Display���� / Length�� Line���� �����
//KJH 2022-06-06 B Ư���� �α� �� �������� �߰�
//KJH 2022-06-07 9View CenterAlign Live����
//KJH 2022-06-08 Search Start 15->30���� ����
//KJH 2022-06-29 Find two Limit ������
//KJH 2022-06-29 Wetout_out Limit ���� �߰�
//KJH 2022-06-29 WetoutSpec���� ������ ����
//KJH 2022-06-29 Metal Over Flow 2��° ����Ʈ�� �������� �˻��ϱ�
//KJH 2022-06-28 Lack of CInk Total Count FDC���� �߰�
//KJH 2022-06-28 Film Feeding �˻縦 �̿��� Auto Feeding Time Offset ���
//KJH 2022-06-28 ���߿� �Ķ���� �۾� ���� - ������ ����
//KJH 2022-06-29 Nozzle Align �б� �ʿ��� (X,Y,Z,T)
//KJH 2022-06-29 WetoutSpec���� ������ ����
//KJH 2022-06-30 GridSpecView Length Title ����
//KJH 2022-06-30 Machineâ Origin Box Display ����
//KJH 2022-06-30 CInk2���� Metal �������� Lack �˻��ϴ°ɷ� ����
//KJH 2022-07-01 1Cam 2Pos 2Object ���� �̹����α� Text����
//KJH 2022-07-01 Trace NG PopUp �б� �߰�
//KJH 2022-07-01 Lack Of Cink Total Count �α� �߰�
//KJH 2022-07-02 Nozzle Z Gap Offset Master ���ѿ����� ����ǵ��� ����
//KJH 2022-07-08 Login ��� �߰�
//KJH 2022-07-12 View ������ Ȱ���� ���� TotalOverla�������� �߰�
//KJH 2022-07-12 View ������ �߰�
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//2021-10-26 �պ� ������
//210909 Tkyuha wetoutMin/WetoutMat ����
//20210909 Tkyuha ����Ʈ ���� ���� ��� 8��� �ؼ� ������ ���� ���� 6��� ����
//20210910 Tkyuha �α� ���� �ۼ� �Ϸ� ���� ��ġ ����
//20210910 Tkyuha �ѹ��� �о� ����
//20210914 Tkyuha �������� �̾� �ֱ�
//20210914 Tkyuha �Ʒ� ���� ����
//20210914 Tkyuha ���� ������ Ȯ�� �ص� ��� ���� => Ʋ������ ���� ����
//20210915 Tkyuha ���� ó��
//20210915 Tkyuha �˰����� ������ ���� �߰�
//20210915 Tkyuha ���� ������ ����� �������� ����
//20210915 Tkyuha ���������� ������ ��� ���� Top �� ���
//20210915 Tkyuha ���� ���� �Ǿ Ȯ���ص� ��
//20210915 Tkyuha ���Ͷ����� ã�� ��츸 �ϵ��� ����ó��
//20210923 Tkyuha Circle������ ��� �ϴ� �Ķ����
//20210923 Tkyuha Circle������ ��� �ϴ� �Ķ����
//20210923 Tkyuha Trace �̹��� ���� ��¥�� ����
//20210924 Tkyuha ���� ó�� �߰�
//20210924 Tkyuha �̹��� Grab�� ���� �߻��� ����
//20210924 Tkyuha ����ȭ ��ü ���� DeadLock ����
//20210924 Tkyuha -  GetBit ���� ���� �ذ�
//20210927 Tkyuha ���� ���� ���� �׽�Ʈ�� - KJH �켱 �ּ�
//20211026 Tkyuha ���� ���� ���� �׽�Ʈ�� - KJH ������ ���� �˰����� ���ù������� ���濹��(KJH ��� / TkyuHa ���)
//20210927 Tkyuha ����ó��
//20210910 Tkyuha �ѹ��� �о� ����, 20210927 ���� �ٲ�
//20210928 Tkyuha Peak�� ã�Ƽ� ǥ��
//20210928 Tkyuha Peak ã������ Raw Data
//20210928 Tkyuha Peak Search
//20210928 Tkyuha draw Peak
//20211006 Tkyuha �α� ��ġ ����
//20211007 Tkyuha ���� �ֿܰ��� ���� ���� �ϴ� �ɼ� �츮�� ����
//20211007 Tkyuha ���ܿ��� �߰�
//Tkyuha 21-11-22 OrginX, OrginY �߰�
//Tkyuha 2021-11-22 Concept ���� ȣ�� ���̸� �ݿ�
//Tkyuha 2021-12-21 DataBase�� ����
//Tkyuha 2021-12-24 Delay ����
//Tkyuha 2021-12-24 ������ ū ��� Ignore ��� ����
//Tkyuha 2021-12-26 ��Ʀ ���� ���� �̹��� ó�� �߰�
// Adaptive Dispensing ��� ���� 20220103 Tkyuha
// Adaptive Dispensing ��� ���� ���� 20220103 Tkyuha
// Adaptive Dispensing ��� ���� ���� 20220103 Tkyuha
// MLESAC ADD 20220118 Tkyuha
// Tkyuha 20220125 ���� ���� ó��
//HTK 2022-02-17 Trace search using align value mode
//Tkyuha 2022-03-10 ���� �ݴ��� 1/2 ��հ� ��� �߰�
//HTK 2022-02-17 CInk Inspection search ELB endPositionmode
//HTK 2022-03-10 CC Edge Calc Display
//HTK 2022-03-16 DummyInspSpec �߰�
//HTK 2022-03-21 �� ���� �������� ���� ���ϱ�
//HTK 2022-03-21 MP,PN ������ ���� ū ���� ����Ͽ� �Ϻκи� �����ϴ� ��带 ���� ���
//HTK 2022-03-21 MP,PN ������ ���� ū ���� ���
//HTK 2022-03-29 Metal HIAA Mode���� ������ , �������� �� �� ���ϵ��� ����
//HTK 2022-03-30 Halcon Scale Mark Search ��� �߰�
//HTK 2022-03-30 Halcon Read Para Job���� �б�
//HTK 2022-04-02 Maskó���� �̹��� ������ ����Ǵ� ���� �����
//HTK 2022-04-04 Trace TactTime �߰�
//HTK 2022-04-04 Align TactTime �߰�
//HTK 2022-04-04 Line ������ Z Up Pos ���� �缱������ Offset
//HTK 2022-04-04 Fit�̹��� Overlay �������󰡰� ����
//HTK 2022-04-09 Clink ���� ���� �˻� �ʱⰪ ����
//HTK 2022-04-13 Mark Angle Search
//HTK 2022-04-28 DrawFigure ���� ����
//HTK 2022-04-28 �ʱ� Drag&Drop ��� Ȱ��ȭ
//HTK 2022-05-14 CINK1, CINK2 ���� �ؼ� ����
//HTK 2022-05-19 CInk1 ����϶� �̵��� ���� ����ó��
//HTK 2022-05-19 �̵����˻�� ��ü ������ �ȵ� ��� �ϰ� ������ Ȯ��
//HTK 2022-05-19 DiffWetout�˻�� Noise ���� �������� Median Blur ����
//Tkyuha 2022-05-25 �̵��� ����
//HTK 2022-05-30 �����������
//HTK 2022-05-30 ����������� [Manual Insp �߰�]
//HTK 2022-06-14 ���� �̹����� �ι�° ��ũ �ȱ׷����°� ����(1Cam1ShotAlign)
//HTK 2022-06-14 Wetout FDC ���� ���� 5������ 8���� ����
//HTK 2022-06-17 Nozzle XY �˻��� �ٽ� �ѹ� Y�� ���� Search , ������ ���� ����
//HTK 2022-06-24 CInk2���� CInk1���� �߻��� Lack of Ink ���� �˻� �� ���� �ջ�
//HTK 2022-06-25 FDC�� LackofInkAngleCount ����
//HTK 2022-06-29 MLESAC �������Ͱ� ��� ��� ����(���⸦ �̿��� X���� ����, Y���� ���� ���)
//HTK 2022-06-29 MLESAC error�� ���� ��� ����ó�� �߰�(�и� 0�̵Ǿ ���������� ǥ�õ�)
//HTK 2022-06-29 Caliper Method�� Angle ���� �߰�
//HTK 2022-06-29 PN Edge ���� Trace ��ǥ�������� ����ϵ��� ����
//HTK 2022-06-29 Wetout MIn,Max Judge ���� Enable Mode �߰�
//HTK 2022-06-29 Database�� Job�̸� ����
//HTK 2022-06-29 Wetout Min,Max Judge ������� �߰�
//HTK 2022-06-29 Nozzle Align Y Recalc
//HTK 2022-06-29 Align Revision Error Type �߰�
//HTK 2022-06-29 �� �� ������ �� OK, NG Bit On ���� Ȱ��ȭ
//HTK 2022-06-29 Convyer Align �б��۾�
//HTK 2022-06-29 ���밪 ǥ�ÿ��� Spec�� ���̰� ǥ�÷� ����
//HTK 2022-06-29 X,Y,T ������ ����
//HTK 2022-06-29 Mark Viewâ�� Tracker ��� �߰�
//HTK 2022-06-29 ������û���� Circle�� Lack of CInk Total Count Display�� ����
//HTK 2022-06-29 Dahuha Grab���� Bool �߰�
//HTK 2022-06-29 Mark Viewâ�� Tracker ��� �߰�
//HTK 2022-07-04 RotateCenter Insp ��� �߰�
//HTK 2022-07-11 Film Insp Grab bit & Reset�߰�
//HTK 2022-07-11 Diff Insp Grab bit & Reset�߰�
//HTK 2022-07-11 Main View ����� ȭ�� �� �׸����� ����
//HTK 2022-07-11 Nozzle#45 �˻��� �߰��� ���� ���Ķ���� ����
//HTK 2022-07-11 Alive Ping Pong ���� �������̽� ����
//HTK 2022-07-12 DAHUHA Single Grab ��������
//HTK 2022-07-12 Nozzel#45�� ��׻��� �˻� �߰�
//HTK 2022-08-16 �̰� �˻� �߰�, ���� �Ÿ����� ����� �Ÿ� ���� ħ�� �Ѱ�� Overflow�� ó�� �ϱ� ����
// �ʱ�ȭ ����ȭ ��ü Tkyuha 20221121
// ����ȭ ��Ŵ Tkyuha 20221121
// Scratch Inspection 20221121 Tkyuha ��Ƽ ������ ����
// 20221122 Tkyuha ��ũ��ġ �׽�Ʈ Ȯ�� // �������� ��� else������ ��� �Ұ�
// HTK 2023-01-04 ������ ���� ���� ����
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// kbj 2021-12-27 1cam1shot prcoess �ùķ��̼��̸� ������ ������ �ʵ��� �߰�.
// kbj 2021-12-27 1cam2shot prcoess �ùķ��̼��̸� ������ ������ �ʵ��� �߰�.
// kbj 2021-12-27 1cam2pos prcoess �ùķ��̼��̸� ������ ������ �ʵ��� �߰�.
// kbj 2021-12-27 CenterAlignProcess �ùķ��̼��̸� ������ ������ �ʵ��� �߰�.
// kbj 2021-12-27 NozzleAlign �ùķ��̼��̸� ������ ������ �ʵ��� �߰�.
// kbj 2021-12-27 CenterNozzleAlign �ùķ��̼��̸� ������ ������ �ʵ��� �߰�.
// kbj 2021-12-27 ���ǹ��� each_job_method �߰�
// kbj 2021-12-29 �� �Ķ���� JOB ���� �ʿ� �׸� ���̵���.
// kbj 2021-12-31 �ùķ��̼� Bottom_Pane �̸� ���� �� ����.
// kbj 2021-12-31 �ùķ��̼� Pane �߰�
// kbj 2021-12-31 �ý��ۿɼ� Form �߰�
// kbj 2021-12-31 �ý��ۿɼ� �������� �Ķ���� �߰�.
// kbj 2021-12-31 �ý��ۿɼ� Job ������ �Ķ���� �߰�.
// kbj 2012-12-31 �߰��� �ý��� �ɼ� �д� �κ��߰�.
// kbj 2021-12-31 elb �˻��̹��� ����� ui ���ڷ� ����.
// kbj 2022-01-01 �ùķ��̼� �϶� OK ��ȣ �츮�� �ʵ��� ����.
// kbj 2022-01-01 �ùķ��̼� �϶� NG ��ȣ �츮�� �ʵ��� ����.
// kbj 2022-01-05 Processing check
// kbj 2022-01-05 add Rotate(1) = Rotate(0)
// kbj 2021-01-05 Add grayimage Release()
// kbj 2022-01-05 SaveImage systemOption
// kbj 2022-01-05 Move to film inspection of write_result 
// kbj 2022-01-07 live simulation mode.
// kbj 2022-01-07 RotateCenter is not used caliper mode.
// kbj 2022-01-09 add revisiondata from object
// kbj 2022-02-05 add lcheck 1cam1shot
// kbj 2022-02-05 add lcheck 1cam1shot2object
// kbj 2022-02-05 Draw text of revision data
// KBJ 2022-02-23 If panel_id of memoried is same current panel_id that divide folder_name to time.
// KBJ 2022-02-23 Changed Trace image path and add RawImage.
// Kbj 2022-02-23 Trace RawImage save
// KBJ 2022-02-23 ELB_Center_Align �ùķ��̼� �߰�.
// KBJ 2022-02-23 Divide CenterAlign
// KBJ 2022-02-23 CenterAlignProcess �ùķ��̼��̸� Rotate Center ������ �ʵ��� �߰�.
// KBJ 2022-02-23 Search Button is not use that
// KBJ 2022-02-24 Checking
// KBJ 2022-02-24 Nozzle View Set Panel Naming
// KBJ 2022-03-04 opposition count
// KBJ 2022-03-16 SetInspPara VIew into the Model View
// KBJ 2022-04-18 ROI ���� ����.
// KBJ 220623 �ѹ��� �� ã�ڲ� �߰�.
// KBJ 2022-07-02 Result Image & Video List ��� �߰�
// KBJ 2022-07-05 Manual Mark 1Cam ��� �߰�
// KBJ 2022-07-05 Film Insp Manual ��� �߰�
// KBJ 2022-07-06 SpecGrid Title �纯��
// KBJ 2022-07-06 Align Revision Limit NG Error Message �߰�
// KBJ 2022-07-06 Align L Check NG Error Message �߰�
// KBJ 2022-07-07 1Cam 2Pos Reference nPos 0�϶��� �̹��� �ѹ� �׸����� ����
// KBJ 2022-07-12 Total���� ����ϰ� Main Viewer���� ��� ���ϴ°�찡 �־� �ּ�ó��
// KBJ 2022-07-12 Gap Line �ʹ� �β���� ���Ƽ� PDC->Viewer�� �׸��� ����
// KBJ 2022-07-12 Conveyer Align ���� �̹��� �ȱ�� ��� �߰�
// KBJ 2022-07-12 Reference �̹��� ���� ��� �߰�
// KBJ 2022-07-12 Reference flag Reset �߰�
// KBJ 2022-07-13 3ȸ ���� Lack Of INK NG �Ͻ� �˾� 
// KBJ 2022-07-13 Z-Gap Ķ���� �����ϵ��� ����.
// KBJ 2022-07-18 �ʸ��˻� �и� �̹��� ������ �����ΰ� ������ �ϴ� release() �ȵǾ� �־ �߰�
// KBJ 2022-07-18 �ʸ��˻� ���� ��� ĸó�� ����
// KBJ 2022-07-18 ĸó����� �ʿ��Ͽ� �̹��� ������� ��ġ ����
// KBJ 2022-07-19 ���� �޴���� �Ÿ��˻� �׸��� ������ ī�޶� �߾��� �ƴ� ī�޶� �߾Ӽ��� ���������� ���̰Բ� ����
// KBJ 2022-07-19 ���� �޴���� �Ÿ��˻� x ã�� ��ġ ����(���ʰ� ������ �Ѵ� ã�� �߰������� ���)
// KBJ 2022-07-19 �̹��� ���� ĸó�� �Ǵ¹�� �߰�
// KBJ 2022-07-22 Conveyer ���� ȭ�� Load --> Caputre ����
// KBJ 2022-07-22 Conveyer ���� ȭ�� ��� Dlg ��Ȱ��ȭ
// KBJ 2022-07-22 Center Align Live ȭ�� ������ �Ź��׸���
// KBJ 2022-07-23 ã�� ���� �̸� NOZZLE# -> NOZZLE�� ����
// KBJ 2022-08-09 ���α׷� ���� �� ������� ��ȯ�ǵ��� ����.
// KBJ 2022-08-09 Auto�϶��� Log �����.
// KBJ 2022-08-16 Lami Error �Ǵ��ϴ� ���� ����.
// KBJ 2022-08-16 CINK1 ã�� B������ ���� 150 �ȼ� ���������� Ȯ��
// KBJ 2022-08-18 ZGap Caliper �̻��
// KBJ 2022-08-20 �̹��� ���� �κ� �� ���� ���� ����κ� ����
// KBJ 2022-08-22 ����
// KBJ 2022-08-25 ���α׷� ����� �� Ʈ���̽� ġ�� Lack of ink ������ �ϴ� ���� �־ �ѹ��� �ϵ��� �߰�.
// KBJ 2022-08-25 Rotate ����ȭ �߰�
// KBJ 2022-09-01 add Delay
// KBJ 2022-09-01 change parameter
// KBJ 2022-09-01 Control Param
// KBJ 2022-09-02 WaitForSingleObject 100 -> 300
// KBJ 2022-09-03
// KBJ 2022-09-05 Rotate ����ȭ
// KBJ 2022-09-05 Rotate �������� �Ķ���ͷ� �߰�.
// KBJ 2022-09-05 Rotate Light �Ķ���� �߰�.
// KBJ 2022-09-05 Rotate �Ķ���� �߰�
// KBJ 2022-09-07 ���;���� ���� 200 -> 150���� ����
// KBJ 2022-09-07 ��Ż�� ����� �κ� �����ϴ°� �߰�
// KBJ 2022-09-07 ���;���� FindCircleAlign_PN,MP ���� �߰�
// KBJ 2022-09-07 ���;���� ��� �̹����� ����
// KBJ 2022-09-08 Rotate �׷� End �޽� �κ� �߰�
// KBJ 2022-09-08 Zgpa Offset ����κ� �������� �ٽ� �߰�
// KBJ 2022-09-12 ���;���� 3������ ã���� �߰�.
// KBJ 2022-09-12 CINK2 �ܰ����� ã�� �̹��� �߰� �� ����.
// KBJ 2022-09-12 -60 -> -80.
// KBJ 2022-09-19 distanceT 100 -> 150
// KBJ 2022-09-19 CINK2 ������ �˻�
// KBJ 2022-09-19 ���Ӽ����� �Ǵ��ϵ��� ����
// KBJ 2022-09-19 CINK1�� ������ �̹���
// KBJ 2022-09-19 CINK2�� ���� �̹���
// KBJ 2022-09-19 CINK2�� ������ �����Ǿ��־ �ȼ� ũ�� ����
// KBJ 2022-09-22 �� ���μ��� 2022���� �߰�
// KBJ 2022-09-22 �� ���μ��� 2022���� �߰�! �׽�Ʈ ��
// KBJ 2022-09-22 ���� Ŭ���� ī�޶� ���̺� �߰�
// KBJ 2022-09-22 ����ó�� ����
// KBJ 2022-09-24 ���� 3�� ������ NG�߰� ���� ������ ����
// KBJ 2022-09-27 Trace ���� ã�� �� �ٽ� ǥ��
// KBJ 2022-10-04 ���� 3�� ������ NG�߰�
// KBJ 2022-10-05 Trace�Ҷ� Lack Of Ink �˻��, Cink2 ������ �˻� Lack Of Ink�� ���� ������ ���� �־ ����
// KBJ 2022-10-05 �߰�
// KBJ 2022-10-05 8�� 11�� ���� ������ ����������� ������ �߻��� ī��Ʈ �ջ����� �����Ƿ� �ּ�ó��.
// KBJ 2022-10-10 (1.LackOfCink �÷��� �α��߰�, 2.CINK2 ������ �Ѹ� �����β� �α��߰�, 3.����Ÿ��)
// KBJ 2022-10-10 LackOfInk �����α����Ͽ����� �߰�. (1)
// KBJ 2022-10-10 CINK2������ �ٸ� �����β� ã�� (2)
// KBJ 2022-10-10 �˻� ����Ÿ���߰�. (3)
// KBJ 2022-10-10 ErrorCode ����
// KBJ 2022-10-10 ThreshHold �Ķ���� ����. CINK2������ ��� ����.
// KBJ 2022-10-16 Auto��尡 �ƴҶ����� �����ϵ��� ����.
// KBJ 2022-10-13 Z-Gap ���� ǥ�� ����
// KBJ 2022-10-17
// KBJ 2022-10-20 Manual Mark Bit Reset �߰�
// KBJ 2022-10-22 ��ũ ��ã���� NG ��Ʈ �츲
// KBJ 2022-10-27 PLC ���� Manul(or Auto) Rotate �� ������ ��巹�� ����Ͽ� ��ŵ
// KBJ 2022-11-14 ȸ���߽ɰ� Rotate �� ������ ��巹�� ����Ͽ� ����ó�� �߰�
// KBJ 2022-11-15 Reset Bit TimeOut ���� �߰�
// KBJ 2022-11-15 Reset Bit TimeOut ����
// KBJ 2022-11-19 WaitForSingleObject 100 -> 300
// KBJ 2022-11-30 ���� �������� ã�� �ɷ� �ǵ��� ����
// KBJ 2022-11-30 CG ���̵� ���� ų�� ���� �ݻ�Ǿ� ���°� ���ٰ� �Ǵ� Light Off
// KBJ 2022-11-30 Trace Y ���� �ƿ� �˸�â �� �������� �߰�
// KBJ 2022-11-30 Center Align ���� ���� �ƿ� �˸�â �� �������� �߰�
// KBJ 2022-11-30 ���� ���� �й�
// KBJ 2022-11-30 �̹��� ���� �й�
// KBJ 2022-11-30 �����ʾƼ� �ȱ׸�����
// KBJ 2022-11-30 ���â �����Ķ���� ����κ� ����
// KBJ 2022-11-30 ��� �ð��� ���� �ð��� �����ϰ� �ϱ� ���Ͽ� ����Ų ��찡 �ƴϸ� ���е���
// KBJ 2022-12-03 Ķ���� ���� ���� �̸� �ٲ�� ����
// KBJ 2022-12-14 Wetout, Overflow ���� ������ ���� 2mm���� �������� ����(0�� ������ ���� ����)
// KBJ 2023-01-11 OAM(Once AttachFilm Mode) �߰�

/////////////////////////////////////////////////////////////////////////////
// hsj 2021-12-25 algorithm_PF_Film_insp�� ���â ��쵵�� �߰�.
// hsj 2021-12-26 PanAuto�� ���â Tab���� ������ �Լ� ����.
// hsj 2021-12-31 Simulation �̹��� ���� �� ���ϸ� �ð�(%02d:%02d:%02d:%03d->%02d_%02d_%02d_%03d)����.
// hsj 2022-01-01 display_PF_Film_insp_save �Լ� �߰�.
// hsj 2022-01-03 DlgINISetting.cpp, DlgINISetting.h ������ �ҽ��� ������Ʈ �� ELB�� ���� INI���� �߰�, UI����.
// hsj 2022-01-05 Main View Tab�̵��� �ش� ���â�� ���� �ٲ��
// hsj 2021-01-06 INI���� ���� �� ���α׷��� ���� ����
// hsj 2022-01-07 ī�޶� �ø�(�������û)���� ���� ���� ��ġǥ�� ����
// hsj 2022-01-07 ���â ���� �ٲٱ�
// hsj 2022-01-07 �ʸ��˻� Method �Ķ���� �߰�
// hsj 2022-01-07 Method�� Multi�϶�,
// hsj 2022-01-08 ������û�������� ���÷��̿� ��� ���� ��Ÿ���� ��갪 ����
// hsj 2022-01-08 ������û���� �ʸ��˻� ��� ����(Center,Left,Right)���� ��Ÿ�� �� �ְ� ���÷��� ����
// hsj 2022-01-09 ReelAlign ���â �߰�
// hsj 2022-01-09 �˻� method�� ���� ����� 
// hsj 2022-01-09 ���â�� IDǥ�� �߰�
// hsj 2022-01-10 ���� ���� �� ǥ��
// hsj 2022-01-10 ��� ���ؼ� �켱 �ּ�ó��
// hsj 2022-01-10 �ּ�Ǯ��
// hsj 2022-01-10 align spec, inspection spec â ����
// hsj 2022-01-11 inspection�� ������ ������,
// hsj 2022-01-11 nozzle view align ������ ���â�� ����
// hsj 2022-01-12 file ����� ���� �� method�� ���� �ٸ���...
// hsj 2022-01-12 �̹��� ���� �� method�� ���� �� �ٸ��� ����
// hsj 2022-01-12 center nozzle align simulation �߰�
// hsj 2022-01-13 Main View Tab�̵��� �ش� ����â�� ���� �ٲ��
// hsj 2022-01-13 initialize req,ack bit add
// hsj 2022-01-15 Ķ���� �̿��Ͽ� �г� ���� ã��
// hsj 2022-01-15 dy�� 0�϶� ���� �ٻ��� ������ �����
// hsj 2022-01-17 �ʸ��˻� ��ũ ã�� �� Ķ���� ��� ����
// hsj 2022-01-27 xã�� �� ����
// hsj 2022-01-27 �ʸ��˻� scale �Ķ���� �߰�
// hsj 2022-01-27 �ʸ��˻� scale �� �ݿ�
// hsj 2022-01-31 reference �������
// hsj 2022-02-04 ����ó�� �߰�
// hsj 2022-02-08 �ʸ��˻� L check ��� �߰�
// hsj 2022-02-07 �ʸ��˻� threshold ��� �߰�
// HSJ 2022-02-14 ���۷�������
// HSJ 2022-02-14 thread_process�� ��ġ����
// hsj 2022-02-16 �߾Ӽ� ���� ������ �Ʒ����� �߰�������
// hsj 2022-02-16 �̸��극�̼� �������� ���� ��ġǥ��
// hsj 2022-03-10 reference �ѹ��� ���
// hsj 2022-03-11 All reference ��� �׸��� 
// hsj 2022-03-11 Reference ��Ͻ� ��¥ ǥ��
// hsj 2022-03-14 1cam 2pos �϶� pos 2�� ��� ��¥ ��Ÿ����
// hsj 2022-03-14 reference ��¥ ǥ��
// HSJ 2022-03-15 
// HSJ 2022-03-16 method ���� ���弱 ��ġ ����
// hsj 2022-09-20 cleanning camera live bit �߰�
// hsj 2022-09-23 ELB ZONE1~4������ �����ϵ��� ����
// hsj 2022-10-17 �������� search��� �б�
// hsj 2022-10-17 show_parameter_ELB�� ��ġ�� �κ��� �־ ���� �ٲ�
// hsj 2022-10-17 ��ũ��ġ üũ�Ǿ������� �Ķ���� �����ϴ°� ������ ���̰�
// hsj 2022-10-17 scratch thresh ����ǰ� ����
// hsj 2022-10-21 header display�� �ȶ��� �߰�
// hsj 2022-10-21 ����� �̻��ϰ� ���ͼ� ����
// hsj 2022-10-28 ��ũ��ġ bit �߰�
// hsj 2022-10-29 ������ ��û���� rotate ���尪 ����
// hsj 2022-10-29 image ���� ���� �����ϱ� ���ؼ� ��ġ �̵�
// hsj 2022-10-31 scratch process add
// hsj 2022-10-31 image draw
// hsj 2023-01-02 Mark Copy Dialog �߰�
// hsj 2023-01-02 Equipment Name �߰�
// hsj 2023-01-02 calibration �� ��Ÿ���� , �����ؾ���..
// hsj 2023-01-02 ����Ŭ���ϸ� ����Ʈ �� �ǵ���
// hsj 2023-01-02 ���ڿ� ���� ���Ͱ� ������ ���� ���ֱ�

/////////////////////////////////////////////////////////////////////////////
// JSY 2022-11-05 �˻��׸� �߰�
// JSY 2022-11-05 Liquid Inspection Judge
// JSY 2022-11-05 �̹��˻� ��� �߰� Dust Insp
// JSY 2022-11-05 ��Ʀ�˻� ��� �߰� Liquid Drops
// JSY 2022-11-07 Scratch �˻��׸� �߰�
// JSY 2022-11-07 Scratch ���� ���� ���� ��� �߰�
// JSY 2022-11-07 Scratch Judge ��� ���� �߰�
/////////////////////////////////////////////////////////////////////////////

// Lincoln Lee - 220210
// Lincoln Lee - 220219
// Lincoln Lee - 220221
// Lincoln Lee - 220222 - Modify MaskMarkingDlg
// Lincoln Lee - 220415 - Easier ellipse mask
// Lincoln Lee - 220415 - Convert mark rotation from range to halcon run length format
// Lincoln Lee - 2022/04/15
// Lincoln Lee - 220528 - Error Dialog color blinking instead of show-hide
// Lincoln Lee - 220528 - More precisely pattern region
// Lincoln Lee - 220530 - Easier to moving caliper
// Lincoln Lee - 220530 - Nearest instead of CUBIC
// Lincoln Lee - 220530 - Maybe better look (Origin Line, Mask Graphic)
// Lincoln Lee - 2022-06-29 Still Mode by FillMode
// Lincoln Lee - 2022-07-12 Conveyer Align ���� ����
// Lincoln Lee - 2022-08-20 New ViewerEx, new mark mask drawer, remove 2 times caliper loading, new ShowImage API,...
// Lincoln Lee - 2022/08/22 - Fix Incorrect mark offset
// Lincoln Lee - 2022/08/22 - Adding GraphicPolyline, GraphicLines
// Lincoln Lee - 2022/08/26 - Separate dependencies to multiples files for easier adding or remove depends
// Lincoln Lee - 2022/08/26 - Adding cv344 to depends so we can easily switch the whole project to cv344 instead of cv2.6.13
// Lincoln Lee - 2022/08/26 - Working with both Basler and Dahua Camera (Automatic detect based on Serial Number)
//                          - By Adding CameraObject for abstract camera, BaslerCamera (For basler), DahuaCamera (For dahua)
// *******************************************************************/
// Note that from now ViewerEx is much more powerful than itself in the past
// ViewerEx have two layers for displaying graphics (Soft Layer, Hard Layer)
// - Hard-Layer
// + graphics are always stick with screen (when drag move, zoom image, Hard graphics stay still)
// + support for (negative) position for coordinate from max-min (positive for min-max) (currently just GraphicLabel)
//
// -Soft-Layer
// + graphics witll stick with image (when drag move, zoom image, soft graphics will moving together with image)
//
// - Static Graphics Object currently supported: 
// + GraphicPoint
// + GraphicPoints
// + GraphicLine
// + GraphicCircle(Ellipse)
// + GraphicRectangle
// + GraphicArrow
// + GraphicDimesional
// + GraphicLabel
//
// - Interactive Graphics Object currently supported:
// + GraphicInteractiveDimensional
// + GraphicInteractiveCaliper
// + GraphicRegionalSelection (Rectangle, Ellipse)
// + GraphicMaskDrawing (Demo how to make custom InteractiveGraphic) (this object used to create mark masking)
//
// - The offical way to display graphic is just add graphic to viewer (viewer.AddSoftGraphic, viewer.AddHardGraphic)
// please don't directed drawing to dc, becus there actually no real DC anymore.
// (but this time i'm make FakeDC to redirect drawing API to adding graphic to viewer for back-compatible)
// - And becus of there alot of things has to be changed, so I cannot note the changed here,
// or adding comment to changed location, I'm forgot where the code is changed :'(
// *******************************************************************/
///////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////
//JSH 2022-03-16 ���� ���ǿ��� L-Check ����
//JSH 2022-03-16 L-Check Error�� ��쿡�� Manual Mark �����ϰ� ����
//JSH 2022-03-16 L-Check Error�� ��쿡�� Manual Mark �����ϰ� ���� => ���� �Ŵ��� ��ũ ��� OFF LCHECK  ���� �ΰ��
//JSH 2022-03-17 Mark ��� �� ���� Index�� ����� Masking�� ���� �Ѵ�. ( Ȯ�� ��)
///////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//2022.06.18 ksm ��ü Reference �߰� �� Flag
//2022.06.22 ksm ���� ������������� �ٸ� ���� ������� ����
//2022.06.29 ksm Camera �� Algorithm Dialog â ESC ��� �ȵǵ��� ����
//2022.07.01 ksm Unloading Align Angle Limit ����
//2022.07.05 ksm Job Select �� List ���̱�
//2022.09.14 ksm Y GAP INSP 0 -> 3�� Viewer�� �̵�
//2022.09.14 ksm Resolution Set Camera ����ǰ� ����
//2022.09.16 ksm ���� ��ã���� thresh �� ���߸鼭 ã�ƺ��� 
//2022.09.27 ksm Video�� �ѹ� ã�� ����� ������ �̹���, �������� ����
//2022.10.04 ksm Inspection �̹��� ������ �ȸ����� �״����� ����ó��
//2022.10.18 ksm ROI�� ���� ��� �̹��� ��ü ��ĵ()
/////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
// YCS 2022-07-28 CV ������ �ǳ��ִ� �迭 �ε��� ����
// YCS 2022-07-29 ȸ���߽� Ÿ�� �ƿ� �޼��� ���� �� ���μ��� �����丮 �߰�
// YCS 2022-08-20 ��Ʈ ���̾�α� �߰�
// YCS 2022-08-25 ź���� ���̾�α� ����
// YCS 2022-08-26 ������ ���� ����
// YCS 2022-08-28 ������ 100�� �̻��̸� ������ �����ͺ��� �����
// YCS 2022-08-30 �� ����� ��Ʈ �ٽ� �׸����� �߰�
// YCS 2022-08-30 ��Ʈ�� ���� �޼��� �߰�
// YCS 2022-09-05 ���� �˻絥���͸� ���÷����ϵ��� ��ġ ����
// YCS 2022-09-06 Recent�� DB ��Ʈ�� ���� ���̴� ���� ������ �߰�. 
// YCS 2022-09-06 Auto �� ���̾�α� SHOW
// YCS 2022-10-27 �ʸ� 3ȸ ���� Ʋ���� ��Ʈ reset
// YCS 2022-10-27 �ʸ� ���� 3ȸ Ʋ������ ���� NG���� ��˶� �߻����� ����
// YCS 2022-10-31 �ʸ� �����˻� ���� ������ ����
// YCS 2022-11-08 Attach / Pre Align�� ��ũ ī��Ʈ �Ķ���� ����ϵ��� �߰�
// YCS 2022-11-08 Pre Align �Ŵ��� ��ũ ���� ���ǿ� ��ũ ī��Ʈ �Ķ���� �߰�
// YCS 2022-11-08 Attach Align �Ŵ��� ��ũ ���� ���ǿ� ��ũ ī��Ʈ �Ķ���� �߰�
// YCS 2022-11-08 ������ ���� �� Metal overflow margin Ȱ��ȭ
// YCS 2022-11-09 �ʸ�����ο� �ʸ� ��� ���� ã�� �Լ�
// YCS 2022-11-19 �ʸ������ ���� �� ���� �Ķ���� �и�
// YCS 2022-11-19 �ʸ� ���� �� ���� �Ķ����
// YCS 2022-11-19
// YCS 2022-11-21 Center LiveView ���̵���� Ŭ���� �߰�
// YCS 2022-11-21 Center LiveView ���̵���� �߰�
// YCS 2022-11-21 Center Align Live YGAP ���̵���� �Ź��׸���
// YCS 2022-11-23 ��ũī��Ʈ ��Ʈ�� Ȱ��ȭ �߰�
// YCS 2022-11-17 ��ũ��ġ ���� �гξ��̵� ���̵��� ǥ�� (���� Green or Red �����ϴ�)
// YCS 2022-11-28 AutoMode ����� Exposure ���� ������ �߰�
// YCS 2022-11-28 ī�޶� ���� üũ �߰�
// YCS 2022-11-28 ����¯ ��û����
// YCS 2022-11-30 �ʸ� �����˻� �������� ����
// YCS 2022-11-30 �������� �� �ٷ� ���� - �ؽ�Ʈ ũ�� ���� �� ��ġ�� ���� �߻�
// YCS 2022-11-30 Z Gap ���μ��� �α� ���� - OK NG ������� ��� NG��� �α� ����
// YCS 2022-11-30 Z Gap ������� �Ķ���� ����. ���� ���� -> Z GAP���� ����
// YCS 2022-11-30 �ð����� �߰�. �ùķ��̼��� ��� �ð����� �������� ����
// YCS 2022-11-30 �ʸ� �����˻� �������� ����
// YCS 2022-11-30 ��Ʈ ������ ���� 13.5f -> 16.5f (GraphicLabel.h ==> static void Init())
// YCS 2022-11-30 WetOut ASPC ���Ͷ� ���� ������� �޼��� �߰�
// YCS 2022-11-30 WetOut �����Ϳ� WetOut_ASPC Spec �� - ������ ���� ��û����
// YCS 2022-12-01 CINK2�� ��, Wetout Judge Disable�̶� ������ WetOut NG �߻���Ű���� ���� ���� (������ ���� ��û: Spec Out�� OK�� ������ �����̴�.)
// YCS 2022-12-01 CIRCLE�� �� �˻��� �׸��� ���� ���ǿ� ASPC ���� �߰�
// YCS 2022-12-01 Wetout_ASPC ����
// YCS 2022-12-01 Wetout_ASPC ���� �߰�
// YCS 2022-12-02 �Ķ���� ���� ����
// YCS 2022-12-24 PN to MP Y �Ÿ� ���ϱ�
// YCS 2022-12-24 �� ���� ȸ�� ����
// YCS 2022-12-24 Line Count�� ���� �и�
// YCS 2022-12-24 PN To Metal ������� �Ķ���� �߰�
// YCS 2022-12-24 ������ ���� �� Ȱ��ȭ
// YCS 2022-12-28 ���� ���� FDC ����
// YCS 2023-01-04 ��Ʀ ���� 
// YCS 2023-01-06 �� ������ Ķ���� ���� �߰�
// YCS 2023-01-07 ��Ʀ ��� ����. FDC ���������� �߰�
// YCS 2023-01-20 �������� NG�� �˶�â 
// YCS 2023-01-21 �� ������� �������� �����˻� �̹��� ũ�� �б� (����� �۾� �������� ���� �˻� ���������� NG �߻���)
///////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
//kmb 220811 Auto ���� �϶� ����� �޼��� �߰�
//kmb 220811 Auto ���¿��� ������ ����� Exit ���ϰ� ����ó��
//kmb 220811 Auto �߿��� EXIT ��ư Ȱ��ȭ
//kmb 220811 ������ ������ stop �޼��� ���� AfxMSg -> fmSetMessge �� ���� EXIT �� ����
//kmb 220811 Processing üũ �����ص� �̹��� �״���� ���� ����
//kmb 220811 Mark Processing Check �߰�
//kmb 220905 check Master Parameter
//KMB 220924 SYSTEM OPTION�� ���� �����̹��� Ȯ���� ����
//KMB 220924 ����̹���, �����̹��� ������ �Ķ���� �и�
//KMB 20221020 Auto�� Cleanning Camera Light off �߰�
//KMB 20221020 Toal View ����� ����� Fit��� �߰�
//kmb 221105 Z Gap Offset ���� �α� �߰�
//kmb 2022-11-19 REEL Revision Limit NG ���� 
//kmb 230112 Method MarkEdge �߰�
////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////
//KJH2 2022-08-17 Colum ���� ����
//KJH2 2022-08-17 Metal_Trace_First -> Y Light Enable/Disalbe ���� �Ǵ�
//KJH2 2022-08-17 Panel Gray -> Nozzle YGapThreshold ������ ����
//KJH2 2022-08-20 Rotate Data ����
//KJH2 2022-08-23 Attach Align(PC7), Film Inspection(PC8) �Ķ���� �и�
//KJH2 2022-10-24 Ķ���극�̼�, ȸ���߽� ��¥ ����
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
// PJH 2022-09-02 Z_GAP ���â �߰�
// PJH ZGAP �ùķ��̼� ��� �߰��ҷ��ϱ� ������ ���� �ʿ��� �Լ� �߰�
// PJH 22-10-05 EGL ��û VIEW Cam Light On
// PJH 22-10-12 WETOUT �˻翡�� �����ϴ� ���� ��Ʀ �˻翡���� ����
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
// LYS 2022-09-05 Z_GAP �ùķ��̼� ���â Data ���
// LYS 2023-01-03 Circle/Line Gird Col ���� �и�
// LYS 2023-01-04 ���������� ��, Inspection Length �������� �ʴ´�.
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
// SJB 2022-10-03 Scratch Insp(PC1), Scratch Inspection(PC1) �Ķ���� �и�
// SJB 2022-10-03 Scratch Inspection �Ķ���� �и�
// SJB 2022-10-03 Scratch �˻��� �߰��� ���� ���Ķ���� ����
// SJB 2022-10-03 Scratch Parameter
// SJB 2022-10-03 Scratch Insp
// SJB 2022-10-24 Ķ���극�̼� ��� Resolution ��� ����
// SJB 2022-10-31 CGDection �̹��� ��� ����
// SJB 2022-10-31 CGDection �̹��� FindLine �׸���
// SJB 2022-10-31 Insp NG�� Comment ���� ��� (�̿�)
// SJB 2022-10-31 Insp Overflow Metal Line ���� �� Trace ���� �� Caliper �����ͷ� ����
// SJB 2022-11-01 Trace ���� �� Metal Line �� ǥ��
// SJB 2022-11-02 Insp CG �� Text ��� ��ġ ����
// SJB 2022-11-04 CG Grab Process �α� �߰�
// SJB 2022-11-07 Wetout, Overflow Min Max Draw ���� ����
// SJB 2022-11-11 Overflow Min ���� ErrorCode �߰�
// SJB 2022-11-11 Overflow Min �Ķ���� �߰�
// SJB 2022-11-11 Overflow Min Grid ����
// SJB 2022-11-11 Overflow Min Log Sheet ����
// SJB 2022-11-11 Overflow Min �������� ����
// SJB 2022-11-14 Overflow Process Log ����(���� �غ��ߵ�)
// SJB 2022-11-15 Overflow Min Value ���� ����
// SJB 2022-11-16 Overflow Min Spec ���� ����
// SJB 2022-11-19 CG Exposure Time �Ķ���� �߰�
// SJB 2022-11-19 CG Grab Exposure Time �Ķ���� ������ ����
// SJB 2022-11-23 CG Grab �� Exposure Time ���� ���� ����
// SJB 2022-11-29 Trace ��Ż ���� Overflow ���� �󹫴� �ҽ��� ����
// SJB 2022-11-29 Dist, CG_Start �ؽ�Ʈ �������� ����
// SJB 2022-11-29 Wetout, Overflow �������� ����(�� ����)
// SJB 2022-11-29 Wetout, Overflow Min Max �������� �߰�
// SJB 2022-11-29 Pocket Length ��� ����
// SJB 2022-11-30 Result Grid 0row �׸� �ؽ�Ʈ �� ����
// SJB 2022-12-02 Min, Max ���� �̹��� ���� ����
// SJB 2022-12-02 Dopo �ܰ��� ���� �̹��� ���� ����
// SJB 2022-12-02 �̹��� ���� Panel, Metal ���� �߰�
// SJB 2022-12-12 ��� INSP ��� �ؽ�Ʈ Line, Circle ����
// SJB 2022-12-19 Nozzle Tip Size ���� �߰�
// SJB 2022-12-19 Nozzle Tip Size �Ķ���� �߰�
// SJB 2022-12-29 Nozzle Angle Search �� �ݻ� ���� ��ó�� �߰�(ħ��, ��â)
// SJB 2023-01-02 CG to Start, End �Ķ���� �߰�
// SJB 2023-01-06 �̹��� Bluring���� ���� Wetout, Overflow Insp Offset ���� �߰�
// SJB 2023-01-06 Wetout Min ���� �߰�
// SJB 2023-01-06 Wetout Min Value ���� ����
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
// JCY 2022-12-24 Peri ���� FDC �׸� �߰� �� CG-Start, Length �ؽ�Ʈ ���� 
// JCY 2022-12-26 trace ���λ��� ���� �߰�
// JCY 2022-12-29 FDC �׸� �߰� ��Ʀ ����
// JCY 2023-01-04 Nozzle Tip ��� ������ �߰�
// JCY 2023-01-12 CG Judge Area �Ķ���� �߰�
// JCY 2023-01-17 ������ ��Ʈ �Լ� Ȧ/���� �б� 
// JCY 2023-01-17 C3���� ������ ���� ��û(����� ����)
// JCY 2023-01-19 Wetout, overflow Min ��ǥ ����
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//KTY 2023-01-03 Grid CG to Start �� �߰�
//KTY 2023-01-03 Line/Circle ����
//KTY 2023-01-06 Line ���� �� CG-start ��
//KTY 2023-01-06 Line ���� �� CG-end ��
//////////////////////////////////////////////////////////////////