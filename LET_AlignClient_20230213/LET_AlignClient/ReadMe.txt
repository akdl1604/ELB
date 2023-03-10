================================================================================
    MFC 라이브러리 : LET_AlignClient 프로젝트 개요
================================================================================

응용 프로그램 마법사에서 이 LET_AlignClient 응용 프로그램을 
만들었습니다. 이 응용 프로그램은 MFC의 기본 사용법을 보여 줄 뿐만 아니라 응용 
프로그램작성을 위한 기본 구조를 제공합니다.

이 파일에는 LET_AlignClient 응용 프로그램을 구성하는 각 파일에 대한
요약 설명이 포함되어 있습니다.

LET_AlignClient.vcxproj
    응용 프로그램 마법사를 사용하여 생성된 VC++ 프로젝트의 주 프로젝트 파일입니다. 
    파일을 생성한 Visual C++ 버전에 대한 정보와 응용 프로그램 마법사를 사용하여 선택한 
    플랫폼, 구성 및 프로젝트 기능에 대한 정보가 들어 있습니다.

LET_AlignClient.vcxproj.filters
    응용 프로그램 마법사를 사용하여 생성된 VC++ 프로젝트의 필터 파일입니다. 
    이 파일에는 프로젝트의 파일과 필터 간의 연결 정보가 들어 있습니다. 이러한 
    연결은 특정 노드에서 유사한 확장명으로 그룹화된 파일을 표시하기 위해 
    IDE에서 사용됩니다. 예를 들어 ".cpp" 파일은 "소스 파일" 필터와 연결되어 
    있습니다.


LET_AlignClient.h
    응용 프로그램의 기본 헤더 파일입니다. 여기에는 다른 프로젝트 관련
    헤더(Resource.h 포함)가 들어 있고 CLET_AlignClientApp 응용 프로그램
    클래스를 선언합니다.

LET_AlignClient.cpp
    응용 프로그램 클래스 CLET_AlignClientApp이(가) 들어 있는 기본 응용 프로그램
    소스 파일입니다.

LET_AlignClient.rc
    프로그램에서 사용하는 모든 Microsoft Windows 리소스의 목록입니다.
 여기에는 RES 하위 디렉터리에 저장된 아이콘, 비트맵 및 커서가
    포함됩니다. 이 파일은 Microsoft Visual C++에서 직접
    편집할 수 있습니다. 프로젝트 리소스는 1042에 있습니다.

res\LET_AlignClient.ico
    응용 프로그램의 아이콘으로 사용되는 아이콘 파일입니다. 이 아이콘은
    주 리소스 파일인 LET_AlignClient.rc에 의해 포함됩니다.

res\LET_AlignClient.rc2
    이 파일에는 Microsoft Visual C++ 이외의 다른 도구에서 편집한 리소스가 
    들어 있습니다. 리소스 편집기로 편집할 수 없는 모든 리소스는
    이 파일에 넣어야 합니다.


/////////////////////////////////////////////////////////////////////////////

응용 프로그램 마법사에서 하나의 대화 상자 클래스가 만들어집니다.

LET_AlignClientDlg.h, LET_AlignClientDlg.cpp - 대화 상자
    이 파일에는 CLET_AlignClientDlg 클래스가 들어 있습니다. 이 클래스는
    응용 프로그램의 주 대화 상자 동작을 정의합니다. 대화 상자의 템플릿은
    Microsoft Visual C++에서 편집할 수 있는 LET_AlignClient.rc에 있습니다.


/////////////////////////////////////////////////////////////////////////////

기타 기능:

ActiveX 컨트롤
    응용 프로그램에서 ActiveX 컨트롤을 사용할 수 있도록 지원합니다.

Windows 소켓
    응용 프로그램에서 TCP/IP 네트워크를 사용한 통신을 지원합니다.

/////////////////////////////////////////////////////////////////////////////

기타 표준 파일:

StdAfx.h, StdAfx.cpp
    이 파일은 미리 컴파일된 헤더 파일(PCH)인 LET_AlignClient.pch와
    미리 컴파일된 형식 파일인 StdAfx.obj를 빌드하는 데 사용됩니다.

Resource.h
    새 리소스 ID를 정의하는 표준 헤더 파일입니다.
    Microsoft Visual C++에서 이 파일을 읽고 업데이트합니다.

LET_AlignClient.manifest
	응용 프로그램 매니페스트 파일은 Windows XP에서 특정 버전의 Side-by-Side 
	어셈블리에 대한 응용 프로그램 종속성을 설명하는 데 사용됩니다. 로더는 이 정보를 
	사용하여 어셈블리 캐시에서 적절한 어셈블리를 로드하거나 응용 프로그램에서 전용 
	어셈블리를 로드합니다. 응용 프로그램 매니페스트는 응용 프로그램 실행 파일과 같은 
	폴더에 설치된 외부 .manifest 파일로 재배포를 위해 포함되거나 리소스의 형태로 
	실행 파일에 포함될 수 있습니다. 
/////////////////////////////////////////////////////////////////////////////

기타 참고:

응용 프로그램 마법사는 "TODO:"를 사용하여 추가하거나 사용자 지정해야 하는
소스 코드 부분을 나타냅니다.

응용 프로그램이 공유 DLL에서 MFC를 사용하는 경우 해당 MFC DLL을 
재배포해야 합니다. 그리고 응용 프로그램이 운영 체제의 로캘과 
다른 언어를 사용하는 경우 해당 지역화된 리소스인 MFC100XXX.DLL을 
재배포해야 합니다. 이 두가지 항목에 대한 자세한 내용은 
MSDN 설명서의 Visual C++ 응용 프로그램 재배포 항목을 
참조하십시오.

/////////////////////////////////////////////////////////////////////////////
//2021-08-06 KJH 카메라 Tap 미갱신 현상 수정
//KJH 2021-09-02 mBar->kPa
//2021-09-13 KJH Nozzle Check Grab 추가(2021-09-10 추가)
//2021-09-13 Model관련 통신 우선 막음 / Main View Total만 사용하게 우선 막음
//2021-09-13 Trace Image LOG 만들지도 않았다.. 막음
//2021-09-14 InspNozzleDistance 제작
//2021-09-16 InspNozzleDistance 로그 구조 생성 , Center Align 로그 구조 변경
//2021-09-17 InspNozzleDistance 엑셀 로그 추가 , Center Align 이미지 로그 구조 변경
//KJH 2021-09-17 Phone 설비 Center Align Log 변경
//KJH 2021-09-17 반지름 기록
//2021-09-18 Total View 막아둔거 품.. 1개 화면 확대 후 뒤에 화면 클릭시 데드락걸림
//2021-09-25 FDC 보고 준비
//2021-09-27 Circle 신규 알고리즘 합본 진행함
//KJH 2021-10-21 FDC 보고함수 추가_Circle
//KJH 2021-10-26 알고리즘 분리( 0 : KJH 방식 / 1 : Tkyuha 방식)
//m_dBMCircleRadius -> m_dCircleRadius_CC
//KJH 2021-10-30 Pane Cam Index 0번 고정현상 수정
//KJH 2021-10-30 Save End Check 변수 추가
//KJH 2021-11-01 Circle Trace Data 역방향 계산 후 전송 추가
//KJH 2021-11-06 Nozzle Insp 관련 Test용 버튼 생성
//KJH 2021-11-06 2진법 영상 처리 추가
//KJH 2021-11-08 UT INSP 조건 추가
//KJH 2021-11-12 Trace Parameter 작업 시작
//KJH 2021-11-15 Center Align에 왜 Trace 관련 로그가있음???
//KJH 2021-11-15 Trace 결과로 계산되는 값임 Align Save에 어울리지 않음
//KJH 2021-11-17 ELB Server Check 기능 우선 막음
//KJH 2021-11-30 강제 OK모드관련 암호 기능 추가
//KJH 2021-11-30 DustInsp 판정이 최상위 판정이던거 수정함
//KJH 2021-12-01 Dust Insp Image Para 추가
//KJH 2021-12-02 Dust Insp로 검출시 ROI 표시
//KJH 2021-12-07 Trace 이후 자재 검사 ExtractProfileData에 옴기기 작업 시작
//KJH 2021-12-07 Trace Image & 자재 검사 관련 이미지 로그 추가 (Tact 문제 생기면 쓰레드 처리 필요)
//KJH 2021-12-11 Total Value 이상현상 수정
//KJH 2021-12-11 액튐 검사 예외영역 Master MP 영역에서 실제 MP영역으로 변경 + Margin 3
//KJH 2021-12-20 Display와 Combo box Mark Index 매칭작업
//KJH 2021-12-21 revisiondata 0 전송 가능!
//KJH 2021-12-25 검사 Start Pos 수정
//KJH 2021-12-25 Wetout Insp 과검 개선
//KJH 2021-12-25 과검 개선 , 임시 모니터링용도
//KJH 2021-12-25 긴급 대응용 추후 검사 파라미터로 분리예정
//KJH 2021-12-25 CIRCLE_RADIUS + 30 -> CIRCLE_RADIUS MP 도포 대응으로 변경
//KJH 2021-12-26 합본 후 Diff Insp 인자 매칭 작업
//KJH 2021-12-29 Pattern Index 추가
//KJH 2021-12-29 Tap에 맞는 조명으로 초기화
//KJH 2021-12-29 MarkIndex추가
//KJH 2021-12-31 Machine View Camera Index Debug
//KJH 2021-12-31 Camera Grab Delay 기능 추가
//KJH 2022-01-03 State Insp OK,NG 1:1 매칭
//KJH 2022-01-03 암호 상시로 변경
//KJH 2022-01-03 Camera ini 못읽어오는거 예외처리
//KJH 2022-01-03 현장에서 인터페이스 새로 맞춤(Nozzle Gap과 겹침)
//KJH 2022-01-03 State Insp OK,NG 1:1 매칭
//KJH 2022-01-05 숨겨지는 Viewer ToolBar 추가작업
//KJH 2022-01-05 Center Nozzle Align 시작하기 전에 ExposureTime 변경
//KJH 2022-01-07 암호 체계 변경
//KJH 2022-01-07 Rotate Auto Calc (Caliper -> Circle)
//KJH 2022-01-10 캘리브 bit변경 ( 48 : Side , 49 : Center, 50 : Gap)
//KJH 2022-01-12 CC Find Filter Insp Para로 변경
//KJH 2022-01-12 WetoutInsp Para Insp Para로 분기
//KJH 2022-01-19 Black Nozzle Search
//KJH 2022-01-19 White Nozzle Search
//KJH 2022-01-19 Nozzle Align 4회 1Cycle 반복가능하게 변경
//KJH 2022-01-25 MP Edge Noise제거 기능 삭제(오인식 다발)
//KJH 2022-01-25 Circle Trace 전처리 추가(MP 검출시 필요, PN시 확인 필요)
//KJH 2022-01-25 ELB Cover CInk mode
//KJH 2022-01-30 MP Trace 영상처리 보강작업
//KJH 2022-02-01 검사 기준 변경 boundingRect Size -> crackStats
//KJH 2022-02-01 이물검사 쓰레드로 이동
//KJH 2022-02-01 이물검사 알고리즘 변경
//KJH 2022-02-03 CC Edge Find insert
//KJH 2022-02-03 DustInsp관련 불필요 로그 저장 개선
//KJH 2022-02-03 MP Align 조건 추가
//KJH 2022-02-03 MP 관련 이물검사 추가 및 CC Edge 찾기 활성화
//KJH 2022-02-03 PreDustInsp 전처리 구조 변경
//KJH 2022-02-03 CircleAlign rename
//KJH 2022-02-03 EmptyInspection 개발중
//KJH 2022-02-05 Metal Over flow Margin
//KJH 2022-03-10 L-Check Error일 경우에도 Manual Mark 가능하게 변경
//KJH 2022-03-10 PF Matching 사용시 Image Index 0 이미지 그리기
//KJH 2022-03-12 PF INSP FDC 보고 추가
//KJH 2022-03-12 MatchingRate Setting Value FDC 보고 추가
//KJH 2022-03-15 고객 요청으로 각 L, R 에서의 거리로 스펙 판정 변경
//KJH 2022-03-16 Act,Seq Time Display 추가
//KJH 2022-03-17 VisionActTime Display 추가
//KJH 2022-03-17 Trend Insert Result CS 추가
//KJH 2022-03-24 Dopo length 오인식 예외처리 추가
//KJH 2022-04-01 Reel Align Init bit관련 예외처리 추가
//KJH 2022-04-02 짧은 Tape 검사시 거리값 계산은 스칼라 계산. 입력은 벡터로받아야 되는 문제(가이드선 때문에 스펙은 벡터로)
//KJH 2022-04-08 Min Spec 0보다 작을때 검사 판정안하도록 변경
//KJH 2022-04-08 lParam = 0으로 변경 / 1은 Center만 사용할때 , 0은 공용
//KJH 2022-04-09 PN 반지름 비교 검사 추가
//KJH 2022-04-13 Calibration Log 추가
//KJH 2022-04-14 PF Attach Align L Check 디버깅
//KJH 2022-04-22 CameraDistance 방식 삭제
//KJH 2022-04-23 LCheck Scale 기능 추가
//KJH 2022-04-23 Auto시 Center Camera CenterAlign Expousure값으로 강제 조정
//KJH 2022-04-25 Camera별 Display추가
//KJH 2022-04-26 Exposure Restore 기능 추가
//KJH 2022-04-27 조명 원복기능추가
//KJH 2022-04-28 PN 반지름 계산 누락 추가
//KJH 2022-04-29 Dopo Start시 동영상 녹화 기능
//KJH 2022-04-29 InkLack Insp 기능추가(연속성 검사, 총량 검사)
//KJH 2022-04-29 CInkLack Insp 기능추가(연속성 검사, 총량 검사) HTK 2022-05-19 CInkLack Insp 중복으로 제거
//KJH 2022-04-29 MincountCheckInsp 추가
//KJH 2022-04-29 RDP 담당자 요청으로 디스크 경로 삭제
//KJH 2022-04-29 ELB CENTER ALIGN LIVE VIEW 가이드 라인 그리기 추가
//KJH 2022-05-02 Cink Wetout Insp 검사시 CC까지 Interlock
//KJH 2022-05-03 Spec 비교 추가 , Min Spec과 별개로 사이 검사 추가
//KJH 2022-05-04 Wetout 검사 결과 이미지 먼저 만들어 놓고 result 파일 저장
//KJH 2022-05-05 R Diff 계산방식 변경 마주보는 사이 거리 측정
//KJH 2022-05-05 72도 단위에서 90도로 변경 0, 90 , 180, 270 , 0으로 변경[마지막 0 data는 미사용]
//KJH 2022-05-05 Nozzle XYZ 검사 기능 추가
//KJH 2022-05-05 Nozzle XYZ 검사 기능 추가 Pos 같은거 사용해야함
//KJH 2022-05-05 특수 각에서의 R값 계산
//KJH 2022-05-06 이미지 Shift 기능 사용시 getProcBuffer 영향주는거 수정
//KJH 2022-05-06 CC to PN 검사 Display 삭제
//KJH 2022-05-09 Mincount 상시 사용으로 변경
//KJH 2022-05-09 원본, 차영상 둘다 검정일때 미도포인 경계선으로 판정 추가
//KJH 2022-05-09 Gap Offset Line 얇게 변경
//KJH 2022-05-09 R Judge 사용 유무 추가
//KJH 2022-05-09 Nozzle Align Display Size 변경
//KJH 2022-05-10 Fixture 날짜 저장
//KJH 2022-05-10 Fixture 엑셀에 저장
//KJH 2022-05-10 Fixture 등록시 날짜 표시
//KJH 2022-05-11 1Cam 1SHot Film Align[Grab Bit 추가 버전]
//KJH 2022-05-11 Film Align Grab bit 관련 Process추가
//KJH 2022-05-11 Film Align Grab bit 추가
//KJH 2022-05-13 Nozzle XYZ Insp Log 분리
//KJH 2022-05-14 미도포 검출 안되던거 수정...
//KJH 2022-05-14 CInk2 모드일때 Overflow는 Spec으로 판정하는걸로 변경
//KJH 2022-05-14 김경철 수석님 요청 사항으로 Nozzle Align Log 미 저장, XYZ 검사만 저장하는걸로
//KJH 2022-05-14 Nozzle XYZ Insp 추가로 검사 후 Center Align ExposureTime으로 변경
//KJH 2022-05-16 도포 검사 Start 위치 +15 -> +10 으로 변경
//KJH 2022-05-16 ELB Center Align 시작하기 전에 ExposureTIme 변경
//KJH 2022-05-18 Record중 View Gray화면 현상 의심 부분 수정
//KJH 2022-05-19 CG Thickness 받아서 Z Gap 판정하기
//KJH 2022-05-19 XYZ 검사시 Spec 판정하기
//KJH 2022-05-19 Z Gap Insp result File 생성
//KJH 2022-05-19 Nozzle Calibration시 Cam Bit 미리 켜주기
//KJH 2022-05-24 BM 시작부터 약액 시작부까지 거리 계산 추가
//KJH 2022-05-25 Length -> BMinSpec으로 UI 변경
//KJH 2022-05-25 BM In <-> Cink In Insp 추가
//KJH 2022-05-26 Display 동기화 작업진행
//KJH 2022-05-26 미도포 검사시 수학좌표계와 화면좌표계의 0도 차이 반영
//KJH 2022-05-26 도포 최소,최대 폭 디스플레이 간소화
//KJH 2022-05-26 Wetout_Out MIN,MAX 좌표 표시 추가
//KJH 2022-05-26 Display UI 개선 작업 진행
//KJH 2022-05-26 6View -> 9 View 분기작업
//KJH 2022-05-26 Individual Calibration 날짜 저장
//KJH 2022-05-28 Roi 화면 밖으로 등록시 마커 좌표 오류 수정
//KJH 2022-05-31 BM Egde 밖에서 안쪽으로 검색하던거 안쪽에서 바깥으로 방향 변경
//KJH 2022-06-01 Nozzle Gap 조명 켜지기 기다리기 위해 Delay 추가
//KJH 2022-06-06 GrabTime 삭제요청에 따른 삭제 작업
//KJH 2022-06-06 R Display제거 / Length는 Line에서 사용함
//KJH 2022-06-06 B 특수각 로그 및 보고관련 추가
//KJH 2022-06-07 9View CenterAlign Live막기
//KJH 2022-06-08 Search Start 15->30으로 변경
//KJH 2022-06-29 Find two Limit 생성용
//KJH 2022-06-29 Wetout_out Limit 제한 추가
//KJH 2022-06-29 WetoutSpec관련 변수명 변경
//KJH 2022-06-29 Metal Over Flow 2번째 포인트가 있을때만 검사하기
//KJH 2022-06-28 Lack of CInk Total Count FDC보고 추가
//KJH 2022-06-28 Film Feeding 검사를 이용한 Auto Feeding Time Offset 계산
//KJH 2022-06-28 나중에 파라미터 작업 진행 - 위에꺼 연동
//KJH 2022-06-29 Nozzle Align 분기 필요함 (X,Y,Z,T)
//KJH 2022-06-29 WetoutSpec관련 변수명 변경
//KJH 2022-06-30 GridSpecView Length Title 변경
//KJH 2022-06-30 Machine창 Origin Box Display 삭제
//KJH 2022-06-30 CInk2에서 Metal 기준으로 Lack 검사하는걸로 변경
//KJH 2022-07-01 1Cam 2Pos 2Object 관련 이미지로그 Text변경
//KJH 2022-07-01 Trace NG PopUp 분기 추가
//KJH 2022-07-01 Lack Of Cink Total Count 로그 추가
//KJH 2022-07-02 Nozzle Z Gap Offset Master 권한에서만 저장되도록 변경
//KJH 2022-07-08 Login 기능 추가
//KJH 2022-07-12 View 저장기능 활용을 위한 TotalOverla관련인자 추가
//KJH 2022-07-12 View 복사기능 추가
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//2021-10-26 합본 진행중
//210909 Tkyuha wetoutMin/WetoutMat 비율
//20210909 Tkyuha 포인트 선택 영역 계산 8등분 해서 면적이 넓은 영역 6등분 선택
//20210910 Tkyuha 로그 파일 작성 완료 시점 위치 변경
//20210910 Tkyuha 한번에 읽어 오기
//20210914 Tkyuha 끊어진것 이어 주기
//20210914 Tkyuha 아래 내용 변경
//20210914 Tkyuha 위쪽 영역은 확장 해도 상관 없음 => 틀어짐에 대한 대응
//20210915 Tkyuha 예외 처리
//20210915 Tkyuha 알고리즘 노이즈 제거 추가
//20210915 Tkyuha 선택 영역만 남기고 나머지는 제거
//20210915 Tkyuha 오른쪽으로 기울어진 경우 왼쪽 Top 이 벗어남
//20210915 Tkyuha 모든게 제거 되어서 확장해도 됨
//20210915 Tkyuha 센터라인을 찾은 경우만 하도록 예외처리
//20210923 Tkyuha Circle에서만 사용 하는 파라미터
//20210923 Tkyuha Circle에서만 사용 하는 파라미터
//20210923 Tkyuha Trace 이미지 오늘 날짜에 저장
//20210924 Tkyuha 예외 처리 추가
//20210924 Tkyuha 이미지 Grab에 문제 발생시 제거
//20210924 Tkyuha 동기화 객체 생성 DeadLock 방지
//20210924 Tkyuha -  GetBit 공유 문제 해결
//20210927 Tkyuha 노즐 높이 측정 테스트용 - KJH 우선 주석
//20211026 Tkyuha 노즐 높이 측정 테스트용 - KJH 변수명 변경 알고리즘 선택방향으로 변경예정(KJH 방식 / TkyuHa 방식)
//20210927 Tkyuha 예외처리
//20210910 Tkyuha 한번에 읽어 오기, 20210927 순서 바꿈
//20210928 Tkyuha Peak를 찾아서 표시
//20210928 Tkyuha Peak 찾기위한 Raw Data
//20210928 Tkyuha Peak Search
//20210928 Tkyuha draw Peak
//20211006 Tkyuha 로그 위치 변경
//20211007 Tkyuha 원의 최외곽을 따라서 도포 하는 옵션 살리기 위해
//20211007 Tkyuha 예외영역 추가
//Tkyuha 21-11-22 OrginX, OrginY 추가
//Tkyuha 2021-11-22 Concept 변경 호의 길이를 반영
//Tkyuha 2021-12-21 DataBase에 저장
//Tkyuha 2021-12-24 Delay 설정
//Tkyuha 2021-12-24 분포가 큰 경우 Ignore 기능 보완
//Tkyuha 2021-12-26 액튐 과검 관련 이미지 처리 추가
// Adaptive Dispensing 사용 기울기 20220103 Tkyuha
// Adaptive Dispensing 사용 시작 각도 20220103 Tkyuha
// Adaptive Dispensing 사용 종료 각도 20220103 Tkyuha
// MLESAC ADD 20220118 Tkyuha
// Tkyuha 20220125 조명 예외 처리
//HTK 2022-02-17 Trace search using align value mode
//Tkyuha 2022-03-10 도포 반대편 1/2 평균값 계산 추가
//HTK 2022-02-17 CInk Inspection search ELB endPositionmode
//HTK 2022-03-10 CC Edge Calc Display
//HTK 2022-03-16 DummyInspSpec 추가
//HTK 2022-03-21 두 원의 중점사이 각도 구하기
//HTK 2022-03-21 MP,PN 간격이 가장 큰 각도 계산하여 일부분만 도포하는 모드를 위한 계산
//HTK 2022-03-21 MP,PN 간격이 가장 큰 각도 계산
//HTK 2022-03-29 Metal HIAA Mode에서 차영상 , 원본영상 둘 다 비교하도록 변경
//HTK 2022-03-30 Halcon Scale Mark Search 기능 추가
//HTK 2022-03-30 Halcon Read Para Job별로 분기
//HTK 2022-04-02 Mask처리시 이미지 사이즈 변경되는 문제 디버깅
//HTK 2022-04-04 Trace TactTime 추가
//HTK 2022-04-04 Align TactTime 추가
//HTK 2022-04-04 Line 도포시 Z Up Pos 이후 사선도포용 Offset
//HTK 2022-04-04 Fit이미지 Overlay 배율따라가게 변경
//HTK 2022-04-09 Clink 도포 유무 검사 초기값 변경
//HTK 2022-04-13 Mark Angle Search
//HTK 2022-04-28 DrawFigure 순서 변경
//HTK 2022-04-28 초기 Drag&Drop 기능 활성화
//HTK 2022-05-14 CINK1, CINK2 대응 해서 삭제
//HTK 2022-05-19 CInk1 모드일때 미도포 관련 예외처리
//HTK 2022-05-19 미도포검사시 전체 도포가 안된 경우 하고 마지막 확인
//HTK 2022-05-19 DiffWetout검사시 Noise 제거 목적으로 Median Blur 수행
//Tkyuha 2022-05-25 미도포 검출
//HTK 2022-05-30 본사수정내용
//HTK 2022-05-30 본사수정내용 [Manual Insp 추가]
//HTK 2022-06-14 저장 이미지에 두번째 마크 안그려지는것 수정(1Cam1ShotAlign)
//HTK 2022-06-14 Wetout FDC 보고 관련 5개에서 8개로 변경
//HTK 2022-06-17 Nozzle XY 검사중 다시 한번 Y만 정밀 Search , 검증후 적용 예정
//HTK 2022-06-24 CInk2에서 CInk1에서 발생한 Lack of Ink 사전 검사 후 최종 합산
//HTK 2022-06-25 FDC용 LackofInkAngleCount 보고
//HTK 2022-06-29 MLESAC 단위벡터값 계산 방식 변경(기울기를 이용한 X단위 벡터, Y단위 벡터 계산)
//HTK 2022-06-29 MLESAC error가 없는 경우 예외처리 추가(분모가 0이되어서 수직선으로 표시됨)
//HTK 2022-06-29 Caliper Method시 Angle 판정 추가
//HTK 2022-06-29 PN Edge 정보 Trace 좌표기준으로 계산하도록 변경
//HTK 2022-06-29 Wetout MIn,Max Judge 관련 Enable Mode 추가
//HTK 2022-06-29 Database에 Job이름 저장
//HTK 2022-06-29 Wetout Min,Max Judge 사용유무 추가
//HTK 2022-06-29 Nozzle Align Y Recalc
//HTK 2022-06-29 Align Revision Error Type 추가
//HTK 2022-06-29 한 매 물류시 각 OK, NG Bit On 조건 활성화
//HTK 2022-06-29 Convyer Align 분기작업
//HTK 2022-06-29 절대값 표시에서 Spec과 차이값 표시로 변경
//HTK 2022-06-29 X,Y,T 보정값 저장
//HTK 2022-06-29 Mark View창에 Tracker 기능 추가
//HTK 2022-06-29 고객요청으로 Circle시 Lack of CInk Total Count Display로 변경
//HTK 2022-06-29 Dahuha Grab상태 Bool 추가
//HTK 2022-06-29 Mark View창에 Tracker 기능 추가
//HTK 2022-07-04 RotateCenter Insp 기능 추가
//HTK 2022-07-11 Film Insp Grab bit & Reset추가
//HTK 2022-07-11 Diff Insp Grab bit & Reset추가
//HTK 2022-07-11 Main View 변경시 화면 재 그리도록 변경
//HTK 2022-07-11 Nozzle#45 검사기능 추가로 인한 모델파라미터 생성
//HTK 2022-07-11 Alive Ping Pong 관련 인터페이스 수정
//HTK 2022-07-12 DAHUHA Single Grab 구조변경
//HTK 2022-07-12 Nozzel#45에 약액상태 검사 추가
//HTK 2022-08-16 미검 검사 추가, 안쪽 거리에서 얼라인 거리 까지 침범 한경우 Overflow로 처리 하기 위함
// 초기화 동기화 객체 Tkyuha 20221121
// 동기화 시킴 Tkyuha 20221121
// Scratch Inspection 20221121 Tkyuha 멀티 쓰레드 대응
// 20221122 Tkyuha 스크레치 테스트 확인 // 느려지는 경우 else문으로 사용 할것
// HTK 2023-01-04 나비모양 과검 제거 목적
/////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
// kbj 2021-12-27 1cam1shot prcoess 시뮬레이션이면 보정량 보내지 않도록 추가.
// kbj 2021-12-27 1cam2shot prcoess 시뮬레이션이면 보정량 보내지 않도록 추가.
// kbj 2021-12-27 1cam2pos prcoess 시뮬레이션이면 보정량 보내지 않도록 추가.
// kbj 2021-12-27 CenterAlignProcess 시뮬레이션이면 보정량 보내지 않도록 추가.
// kbj 2021-12-27 NozzleAlign 시뮬레이션이면 보정량 보내지 않도록 추가.
// kbj 2021-12-27 CenterNozzleAlign 시뮬레이션이면 보정량 보내지 않도록 추가.
// kbj 2021-12-27 조건문에 each_job_method 추가
// kbj 2021-12-29 모델 파라미터 JOB 마다 필요 항목만 보이도록.
// kbj 2021-12-31 시뮬레이션 Bottom_Pane 이름 변경 및 수정.
// kbj 2021-12-31 시뮬레이션 Pane 추가
// kbj 2021-12-31 시스템옵션 Form 추가
// kbj 2021-12-31 시스템옵션 공통으로 파라미터 추가.
// kbj 2021-12-31 시스템옵션 Job 각각의 파라미터 추가.
// kbj 2012-12-31 추가된 시스템 옵션 읽는 부분추가.
// kbj 2021-12-31 elb 검사이미지 압축률 ui 인자로 수정.
// kbj 2022-01-01 시뮬레이션 일때 OK 신호 살리지 않도록 수정.
// kbj 2022-01-01 시뮬레이션 일때 NG 신호 살리지 않도록 수정.
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
// KBJ 2022-02-23 ELB_Center_Align 시뮬레이션 추가.
// KBJ 2022-02-23 Divide CenterAlign
// KBJ 2022-02-23 CenterAlignProcess 시뮬레이션이면 Rotate Center 보내지 않도록 추가.
// KBJ 2022-02-23 Search Button is not use that
// KBJ 2022-02-24 Checking
// KBJ 2022-02-24 Nozzle View Set Panel Naming
// KBJ 2022-03-04 opposition count
// KBJ 2022-03-16 SetInspPara VIew into the Model View
// KBJ 2022-04-18 ROI 영역 수정.
// KBJ 220623 한번씩 다 찾겠끔 추가.
// KBJ 2022-07-02 Result Image & Video List 기능 추가
// KBJ 2022-07-05 Manual Mark 1Cam 기능 추가
// KBJ 2022-07-05 Film Insp Manual 기능 추가
// KBJ 2022-07-06 SpecGrid Title 재변경
// KBJ 2022-07-06 Align Revision Limit NG Error Message 추가
// KBJ 2022-07-06 Align L Check NG Error Message 추가
// KBJ 2022-07-07 1Cam 2Pos Reference nPos 0일때만 이미지 한번 그리도록 수정
// KBJ 2022-07-12 Total에만 사용하고 Main Viewer에서 사용 안하는경우가 있어 주석처리
// KBJ 2022-07-12 Gap Line 너무 두꺼운거 같아서 PDC->Viewer로 그리기 변경
// KBJ 2022-07-12 Conveyer Align 이전 이미지 옴기기 기능 추가
// KBJ 2022-07-12 Reference 이미지 저장 기능 추가
// KBJ 2022-07-12 Reference flag Reset 추가
// KBJ 2022-07-13 3회 연속 Lack Of INK NG 일시 팝업 
// KBJ 2022-07-13 Z-Gap 캘리퍼 무시하도록 설정.
// KBJ 2022-07-18 필름검사 밀린 이미지 찍히는 원인인거 같은데 일단 release() 안되어 있어서 추가
// KBJ 2022-07-18 필름검사 저장 뷰어 캡처로 수정
// KBJ 2022-07-18 캡처저장시 필요하에 이미지 저장버퍼 위치 수정
// KBJ 2022-07-19 노즐 메뉴얼로 거리검사 그리는 직선이 카메라 중앙이 아닌 카메라 중앙선과 일직선으로 보이게끔 수정
// KBJ 2022-07-19 노즐 메뉴얼로 거리검사 x 찾는 위치 보완(왼쪽과 오른쪽 둘다 찾아 중간값으로 계산)
// KBJ 2022-07-19 이미지 저장 캡처로 되는방법 추가
// KBJ 2022-07-22 Conveyer 이전 화면 Load --> Caputre 변경
// KBJ 2022-07-22 Conveyer 이전 화면 뷰어 Dlg 비활성화
// KBJ 2022-07-22 Center Align Live 화면 빨간선 매번그리기
// KBJ 2022-07-23 찾는 비디오 이름 NOZZLE# -> NOZZLE로 변경
// KBJ 2022-08-09 프로그램 실행 시 오토모드로 전환되도록 수정.
// KBJ 2022-08-09 Auto일때도 Log 지우기.
// KBJ 2022-08-16 Lami Error 판단하는 범위 수정.
// KBJ 2022-08-16 CINK1 찾은 B값에서 부터 150 픽셀 연속적인지 확인
// KBJ 2022-08-18 ZGap Caliper 미사용
// KBJ 2022-08-20 이미지 저장 부분 및 엑셀 파일 저장부분 수정
// KBJ 2022-08-22 수정
// KBJ 2022-08-25 프로그램 재시작 후 트레이스 치면 Lack of ink 오검출 하는 현상 있어서 한번더 하도록 추가.
// KBJ 2022-08-25 Rotate 이진화 추가
// KBJ 2022-09-01 add Delay
// KBJ 2022-09-01 change parameter
// KBJ 2022-09-01 Control Param
// KBJ 2022-09-02 WaitForSingleObject 100 -> 300
// KBJ 2022-09-03
// KBJ 2022-09-05 Rotate 이진화
// KBJ 2022-09-05 Rotate 판정스펙 파라미터로 추가.
// KBJ 2022-09-05 Rotate Light 파라미터 추가.
// KBJ 2022-09-05 Rotate 파라미터 추가
// KBJ 2022-09-07 센터얼라인 조건 200 -> 150으로 변경
// KBJ 2022-09-07 메탈과 가까운 부분 무시하는거 추가
// KBJ 2022-09-07 센터얼라인 FindCircleAlign_PN,MP 인자 추가
// KBJ 2022-09-07 센터얼라인 결과 이미지용 벡터
// KBJ 2022-09-08 Rotate 그랩 End 펄스 부분 추가
// KBJ 2022-09-08 Zgpa Offset 저장부분 지워져서 다시 추가
// KBJ 2022-09-12 센터얼라인 3번까지 찾도록 추가.
// KBJ 2022-09-12 CINK2 외각도포 찾는 이미지 추가 및 수정.
// KBJ 2022-09-12 -60 -> -80.
// KBJ 2022-09-19 distanceT 100 -> 150
// KBJ 2022-09-19 CINK2 에서도 검사
// KBJ 2022-09-19 연속성으로 판단하도록 수정
// KBJ 2022-09-19 CINK1은 차형상 이미지
// KBJ 2022-09-19 CINK2은 원본 이미지
// KBJ 2022-09-19 CINK2는 이전에 도포되어있어서 픽셀 크기 증가
// KBJ 2022-09-22 모델 프로세스 2022버젼 추가
// KBJ 2022-09-22 모델 프로세스 2022버젼 추가! 테스트 중
// KBJ 2022-09-22 노즐 클리닝 카메라 라이브 추가
// KBJ 2022-09-22 예외처리 수정
// KBJ 2022-09-24 연속 3도 나오면 NG추가 아직 정하지 않음
// KBJ 2022-09-27 Trace 에서 찾은 것 다시 표시
// KBJ 2022-10-04 연속 3도 나오면 NG추가
// KBJ 2022-10-05 Trace할때 Lack Of Ink 검사와, Cink2 도포후 검사 Lack Of Ink가 같은 스펙을 쓰고 있어서 변경
// KBJ 2022-10-05 추가
// KBJ 2022-10-05 8월 11일 이후 고객사 변경사항으로 이전에 발생한 카운트 합산하지 않으므로 주석처리.
// KBJ 2022-10-10 (1.LackOfCink 플래그 로그추가, 2.CINK2 에서만 뿌린 도포두께 로그추가, 3.에러타입)
// KBJ 2022-10-10 LackOfInk 엑셀로그파일용으로 추가. (1)
// KBJ 2022-10-10 CINK2에서만 바른 도포두께 찾기 (2)
// KBJ 2022-10-10 검사 에러타입추가. (3)
// KBJ 2022-10-10 ErrorCode 전송
// KBJ 2022-10-10 ThreshHold 파라미터 수정. CINK2에서는 사용 안함.
// KBJ 2022-10-16 Auto모드가 아닐때에도 동작하도록 수정.
// KBJ 2022-10-13 Z-Gap 글자 표시 수정
// KBJ 2022-10-17
// KBJ 2022-10-20 Manual Mark Bit Reset 추가
// KBJ 2022-10-22 마크 못찾을시 NG 비트 살림
// KBJ 2022-10-27 PLC 에서 Manul(or Auto) Rotate 시 동일한 어드레스 사용하여 스킵
// KBJ 2022-11-14 회전중심과 Rotate 시 동일한 어드레스 사용하여 예외처리 추가
// KBJ 2022-11-15 Reset Bit TimeOut 관련 추가
// KBJ 2022-11-15 Reset Bit TimeOut 수정
// KBJ 2022-11-19 WaitForSingleObject 100 -> 300
// KBJ 2022-11-30 가장 마지막에 찾은 걸로 되도록 수정
// KBJ 2022-11-30 CG 사이드 조명 킬시 빛이 반사되어 끄는게 낫다고 판단 Light Off
// KBJ 2022-11-30 Trace Y 스펙 아웃 알림창 및 오버레이 추가
// KBJ 2022-11-30 Center Align 각도 스펙 아웃 알림창 및 오버레이 추가
// KBJ 2022-11-30 비디오 폴더 분배
// KBJ 2022-11-30 이미지 폴더 분배
// KBJ 2022-11-30 쓰지않아서 안그리도록
// KBJ 2022-11-30 결과창 스펙파라미터 적용부분 수정
// KBJ 2022-11-30 결과 시간을 도포 시간과 동일하게 하기 위하여 껏다킨 경우가 아니면 안읽도록
// KBJ 2022-12-03 캘리퍼 선택 라인 이름 바뀌도록 수정
// KBJ 2022-12-14 Wetout, Overflow 구간 데이터 범위 2mm내로 생성으로 변경(0값 데이터 전송 오류)
// KBJ 2023-01-11 OAM(Once AttachFilm Mode) 추가

/////////////////////////////////////////////////////////////////////////////
// hsj 2021-12-25 algorithm_PF_Film_insp에 결과창 띄우도록 추가.
// hsj 2021-12-26 PanAuto에 결과창 Tab으로 나누고 함수 정리.
// hsj 2021-12-31 Simulation 이미지 저장 시 파일명 시간(%02d:%02d:%02d:%03d->%02d_%02d_%02d_%03d)수정.
// hsj 2022-01-01 display_PF_Film_insp_save 함수 추가.
// hsj 2022-01-03 DlgINISetting.cpp, DlgINISetting.h 조립기 소스로 업데이트 및 ELB에 맞춰 INI내용 추가, UI수정.
// hsj 2022-01-05 Main View Tab이동시 해당 결과창도 같이 바뀌게
// hsj 2021-01-06 INI파일 종료 시 프로그램도 끌지 말지
// hsj 2022-01-07 카메라 플립(고객사요청)으로 인한 스펙 위치표시 수정
// hsj 2022-01-07 결과창 색깔 바꾸기
// hsj 2022-01-07 필름검사 Method 파라미터 추가
// hsj 2022-01-07 Method가 Multi일때,
// hsj 2022-01-08 고객요청사항으로 디스플레이에 모든 길이 나타나게 계산값 수정
// hsj 2022-01-08 고객요청으로 필름검사 모든 기준(Center,Left,Right)에서 나타날 수 있게 디스플레이 변경
// hsj 2022-01-09 ReelAlign 결과창 추가
// hsj 2022-01-09 검사 method에 따른 결과값 
// hsj 2022-01-09 결과창에 ID표시 추가
// hsj 2022-01-10 스펙 편차 선 표시
// hsj 2022-01-10 사용 안해서 우선 주석처리
// hsj 2022-01-10 주석풀음
// hsj 2022-01-10 align spec, inspection spec 창 구별
// hsj 2022-01-11 inspection이 있을때 없을때,
// hsj 2022-01-11 nozzle view align 보정값 결과창에 띄우기
// hsj 2022-01-12 file 결과값 저장 시 method에 따라서 다르게...
// hsj 2022-01-12 이미지 저장 시 method에 따라 값 다르게 저장
// hsj 2022-01-12 center nozzle align simulation 추가
// hsj 2022-01-13 Main View Tab이동시 해당 스펙창도 같이 바뀌게
// hsj 2022-01-13 initialize req,ack bit add
// hsj 2022-01-15 캘리퍼 이용하여 패널 엣지 찾기
// hsj 2022-01-15 dy가 0일때 아주 근사한 선으로 만들기
// hsj 2022-01-17 필름검사 마크 찾을 시 캘리퍼 사용 유무
// hsj 2022-01-27 x찾는 식 수정
// hsj 2022-01-27 필름검사 scale 파라미터 추가
// hsj 2022-01-27 필름검사 scale 값 반영
// hsj 2022-01-31 reference 사용유무
// hsj 2022-02-04 예외처리 추가
// hsj 2022-02-08 필름검사 L check 기능 추가
// hsj 2022-02-07 필름검사 threshold 기능 추가
// HSJ 2022-02-14 레퍼런스저장
// HSJ 2022-02-14 thread_process로 위치변경
// hsj 2022-02-16 중앙선 기울기 윗선과 아랫선의 중간값으로
// hsj 2022-02-16 켈리브레이션 방향으로 스펙 위치표시
// hsj 2022-03-10 reference 한번에 등록
// hsj 2022-03-11 All reference 등록 그리기 
// hsj 2022-03-11 Reference 등록시 날짜 표시
// hsj 2022-03-14 1cam 2pos 일때 pos 2개 등록 날짜 나타내기
// hsj 2022-03-14 reference 날짜 표시
// HSJ 2022-03-15 
// HSJ 2022-03-16 method 따라 스펙선 위치 나눔
// hsj 2022-09-20 cleanning camera live bit 추가
// hsj 2022-09-23 ELB ZONE1~4에서만 동작하도록 수정
// hsj 2022-10-17 노즐얼라인 search방법 분기
// hsj 2022-10-17 show_parameter_ELB와 겹치는 부분이 있어서 순서 바꿈
// hsj 2022-10-17 스크래치 체크되어있으면 파라미터 설정하는거 무조건 보이게
// hsj 2022-10-17 scratch thresh 저장되게 수정
// hsj 2022-10-21 header display값 안떠서 추가
// hsj 2022-10-21 결과값 이상하게 나와서 수정
// hsj 2022-10-28 스크래치 bit 추가
// hsj 2022-10-29 고객사 요청으로 rotate 스펙값 구분
// hsj 2022-10-29 image 저장 판정 구분하기 위해서 위치 이동
// hsj 2022-10-31 scratch process add
// hsj 2022-10-31 image draw
// hsj 2023-01-02 Mark Copy Dialog 추가
// hsj 2023-01-02 Equipment Name 추가
// hsj 2023-01-02 calibration 값 나타내기 , 검증해야함..
// hsj 2023-01-02 더블클릭하면 리스트 업 되도록
// hsj 2023-01-02 문자열 끝에 엔터가 있으면 엔터 빼주기

/////////////////////////////////////////////////////////////////////////////
// JSY 2022-11-05 검사항목 추가
// JSY 2022-11-05 Liquid Inspection Judge
// JSY 2022-11-05 이물검사 결과 추가 Dust Insp
// JSY 2022-11-05 액튐검사 결과 추가 Liquid Drops
// JSY 2022-11-07 Scratch 검사항목 추가
// JSY 2022-11-07 Scratch 판정 유무 저장 기능 추가
// JSY 2022-11-07 Scratch Judge 결과 저장 추가
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
// Lincoln Lee - 2022-07-12 Conveyer Align 구조 개발
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
//JSH 2022-03-16 진입 조건에서 L-Check 삭제
//JSH 2022-03-16 L-Check Error일 경우에도 Manual Mark 가능하게 변경
//JSH 2022-03-16 L-Check Error일 경우에도 Manual Mark 가능하게 변경 => 현재 매뉴얼 마크 기능 OFF LCHECK  에러 인경우
//JSH 2022-03-17 Mark 등록 시 기존 Index에 저장된 Masking은 삭제 한다. ( 확인 중)
///////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////
//2022.06.18 ksm 전체 Reference 추가 시 Flag
//2022.06.22 ksm 현재 사용하지않지만 다른 값에 덮어쓰여서 삭제
//2022.06.29 ksm Camera 및 Algorithm Dialog 창 ESC 사용 안되도록 수정
//2022.07.01 ksm Unloading Align Angle Limit 적용
//2022.07.05 ksm Job Select 후 List 보이기
//2022.09.14 ksm Y GAP INSP 0 -> 3번 Viewer로 이동
//2022.09.14 ksm Resolution Set Camera 적용되게 수정
//2022.09.16 ksm 먼저 못찾으면 thresh 값 낮추면서 찾아보기 
//2022.09.27 ksm Video들 한번 찾고 경로의 마지막 이미지, 영상으로 띄우기
//2022.10.04 ksm Inspection 이미지 사이즈 안맞으면 죽는현상 예외처리
//2022.10.18 ksm ROI가 없는 경우 이미지 전체 스캔()
/////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////
// YCS 2022-07-28 CV 보정값 건내주는 배열 인덱스 수정
// YCS 2022-07-29 회전중심 타임 아웃 메세지 수정 및 프로세스 히스토리 추가
// YCS 2022-08-20 차트 다이얼로그 추가
// YCS 2022-08-25 탄착군 다이얼로그 숨김
// YCS 2022-08-26 데이터 개수 제한
// YCS 2022-08-28 데이터 100개 이상이면 오래된 데이터부터 지운다
// YCS 2022-08-30 모델 변경시 차트 다시 그리도록 추가
// YCS 2022-08-30 차트뷰 관련 메세지 추가
// YCS 2022-09-05 최종 검사데이터만 디스플레이하도록 위치 변경
// YCS 2022-09-06 Recent와 DB 차트가 겹쳐 보이는 현상 때문에 추가. 
// YCS 2022-09-06 Auto 시 다이얼로그 SHOW
// YCS 2022-10-27 필름 3회 연속 틀어짐 비트 reset
// YCS 2022-10-27 필름 연속 3회 틀어지면 판정 NG에서 경알람 발생으로 변경
// YCS 2022-10-31 필름 유무검사 영역 사이즈 고정
// YCS 2022-11-08 Attach / Pre Align도 마크 카운트 파라미터 사용하도록 추가
// YCS 2022-11-08 Pre Align 매뉴얼 마크 진입 조건에 마크 카운트 파라미터 추가
// YCS 2022-11-08 Attach Align 매뉴얼 마크 진입 조건에 마크 카운트 파라미터 추가
// YCS 2022-11-08 마스터 권한 시 Metal overflow margin 활성화
// YCS 2022-11-09 필름얼라인용 필름 상부 라인 찾기 함수
// YCS 2022-11-19 필름얼라인 각도 비교 스펙 파라미터 분리
// YCS 2022-11-19 필름 각도 비교 스펙 파라미터
// YCS 2022-11-19
// YCS 2022-11-21 Center LiveView 가이드라인 클리어 추가
// YCS 2022-11-21 Center LiveView 가이드라인 추가
// YCS 2022-11-21 Center Align Live YGAP 가이드라인 매번그리기
// YCS 2022-11-23 마크카운트 컨트롤 활성화 추가
// YCS 2022-11-17 스크래치 뷰어에도 패널아이디 보이도록 표시 (배경색 Green or Red 어케하누)
// YCS 2022-11-28 AutoMode 변경시 Exposure 변경 시퀀스 추가
// YCS 2022-11-28 카메라 연결 체크 추가
// YCS 2022-11-28 병준짱 요청사항
// YCS 2022-11-30 필름 각도검사 오버레이 통합
// YCS 2022-11-30 오버레이 한 줄로 통합 - 텍스트 크기 변경 시 겹치는 현상 발생
// YCS 2022-11-30 Z Gap 프로세스 로그 수정 - OK NG 상관없이 모두 NG라고 로그 남김
// YCS 2022-11-30 Z Gap 결과저장 파라미터 변경. 패턴 판정 -> Z GAP스펙 판정
// YCS 2022-11-30 시간폴더 추가. 시뮬레이션인 경우 시간폴더 생성하지 않음
// YCS 2022-11-30 필름 각도검사 오버레이 통합
// YCS 2022-11-30 폰트 사이즈 변경 13.5f -> 16.5f (GraphicLabel.h ==> static void Init())
// YCS 2022-11-30 WetOut ASPC 인터락 관련 사전경고 메세지 추가
// YCS 2022-11-30 WetOut 데이터와 WetOut_ASPC Spec 비교 - 이준재 프로 요청사항
// YCS 2022-12-01 CINK2일 때, Wetout Judge Disable이라도 무조건 WetOut NG 발생시키도록 조건 수정 (김진용 프로 요청: Spec Out이 OK로 나가면 유출이다.)
// YCS 2022-12-01 CIRCLE일 때 검사결과 그리드 색깔 조건에 ASPC 스펙 추가
// YCS 2022-12-01 Wetout_ASPC 스펙
// YCS 2022-12-01 Wetout_ASPC 스펙 추가
// YCS 2022-12-02 파라미터 설명 수정
// YCS 2022-12-24 PN to MP Y 거리 구하기
// YCS 2022-12-24 총 도포 회전 각도
// YCS 2022-12-24 Line Count와 각도 분리
// YCS 2022-12-24 PN To Metal 측정모드 파라미터 추가
// YCS 2022-12-24 마스터 권한 시 활성화
// YCS 2022-12-28 노즐 각도 FDC 보고
// YCS 2023-01-04 액튐 개선 
// YCS 2023-01-06 모델 생성시 캘리퍼 복사 추가
// YCS 2023-01-07 액튐 결과 저장. FDC 보고용으로 추가
// YCS 2023-01-20 팁사이즈 NG시 알람창 
// YCS 2023-01-21 팁 사이즈와 노즐얼라인 각도검사 이미지 크기 분기 (사이즈가 작아 노즐얼라인 각도 검사 시퀀스에서 NG 발생함)
///////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////
//kmb 220811 Auto 상태 일때 종료시 메세지 추가
//kmb 220811 Auto 상태에서 시퀀스 진행시 Exit 못하게 예외처리
//kmb 220811 Auto 중에도 EXIT 버튼 활성화
//kmb 220811 시퀀스 진행중 stop 메세지 변경 AfxMSg -> fmSetMessge 로 변경 EXIT 와 동일
//kmb 220811 Processing 체크 해제해도 이미지 그대로인 현상 수정
//kmb 220811 Mark Processing Check 추가
//kmb 220905 check Master Parameter
//KMB 220924 SYSTEM OPTION에 따라 저장이미지 확장자 변경
//KMB 220924 결과이미지, 원본이미지 압축율 파라미터 분리
//KMB 20221020 Auto시 Cleanning Camera Light off 추가
//KMB 20221020 Toal View 변경시 모든뷰어 Fit모드 추가
//kmb 221105 Z Gap Offset 변경 로그 추가
//kmb 2022-11-19 REEL Revision Limit NG 구분 
//kmb 230112 Method MarkEdge 추가
////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////
//KJH2 2022-08-17 Colum 색상 번경
//KJH2 2022-08-17 Metal_Trace_First -> Y Light Enable/Disalbe 으로 판단
//KJH2 2022-08-17 Panel Gray -> Nozzle YGapThreshold 값으로 측정
//KJH2 2022-08-20 Rotate Data 생성
//KJH2 2022-08-23 Attach Align(PC7), Film Inspection(PC8) 파라미터 분리
//KJH2 2022-10-24 캘리브레이션, 회전중심 날짜 갱신
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
// PJH 2022-09-02 Z_GAP 결과창 추가
// PJH ZGAP 시뮬레이션 없어서 추가할려니까 시퀀스 수정 필요해 함수 추가
// PJH 22-10-05 EGL 요청 VIEW Cam Light On
// PJH 22-10-12 WETOUT 검사에서 무시하는 영역 액튐 검사에서도 무시
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
// LYS 2022-09-05 Z_GAP 시뮬레이션 결과창 Data 출력
// LYS 2023-01-03 Circle/Line Gird Col 생성 분리
// LYS 2023-01-04 직선도포일 때, Inspection Length 변경하지 않는다.
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
// SJB 2022-10-03 Scratch Insp(PC1), Scratch Inspection(PC1) 파라미터 분리
// SJB 2022-10-03 Scratch Inspection 파라미터 분리
// SJB 2022-10-03 Scratch 검사기능 추가로 인한 모델파라미터 생성
// SJB 2022-10-03 Scratch Parameter
// SJB 2022-10-03 Scratch Insp
// SJB 2022-10-24 캘리브레이션 뷰어 Resolution 즉시 적용
// SJB 2022-10-31 CGDection 이미지 경로 수정
// SJB 2022-10-31 CGDection 이미지 FindLine 그리기
// SJB 2022-10-31 Insp NG시 Comment 내용 출력 (미완)
// SJB 2022-10-31 Insp Overflow Metal Line 도포 전 Trace 측정 시 Caliper 데이터로 측정
// SJB 2022-11-01 Trace 측정 시 Metal Line 선 표시
// SJB 2022-11-02 Insp CG 값 Text 출력 위치 변경
// SJB 2022-11-04 CG Grab Process 로그 추가
// SJB 2022-11-07 Wetout, Overflow Min Max Draw 시점 변경
// SJB 2022-11-11 Overflow Min 판정 ErrorCode 추가
// SJB 2022-11-11 Overflow Min 파라미터 추가
// SJB 2022-11-11 Overflow Min Grid 수정
// SJB 2022-11-11 Overflow Min Log Sheet 수정
// SJB 2022-11-11 Overflow Min 오버레이 수정
// SJB 2022-11-14 Overflow Process Log 수정(검토 해봐야됨)
// SJB 2022-11-15 Overflow Min Value 저장 수정
// SJB 2022-11-16 Overflow Min Spec 저장 수정
// SJB 2022-11-19 CG Exposure Time 파라미터 추가
// SJB 2022-11-19 CG Grab Exposure Time 파라미터 값으로 변경
// SJB 2022-11-23 CG Grab 후 Exposure Time 변경 시점 수정
// SJB 2022-11-29 Trace 메탈 기준 Overflow 측정 상무님 소스로 변경
// SJB 2022-11-29 Dist, CG_Start 텍스트 오버레이 수정
// SJB 2022-11-29 Wetout, Overflow 오버레이 수정(색 통일)
// SJB 2022-11-29 Wetout, Overflow Min Max 오버레이 추가
// SJB 2022-11-29 Pocket Length 사용 안함
// SJB 2022-11-30 Result Grid 0row 항목 텍스트 색 변경
// SJB 2022-12-02 Min, Max 저장 이미지 색상 통일
// SJB 2022-12-02 Dopo 외곽선 저장 이미지 색상 통일
// SJB 2022-12-02 이미지 저장 Panel, Metal 라인 추가
// SJB 2022-12-12 상단 INSP 결과 텍스트 Line, Circle 구분
// SJB 2022-12-19 Nozzle Tip Size 판정 추가
// SJB 2022-12-19 Nozzle Tip Size 파라미터 추가
// SJB 2022-12-29 Nozzle Angle Search 빛 반사 오검 전처리 추가(침식, 팽창)
// SJB 2023-01-02 CG to Start, End 파라미터 추가
// SJB 2023-01-06 이미지 Bluring으로 인한 Wetout, Overflow Insp Offset 보정 추가
// SJB 2023-01-06 Wetout Min 판정 추가
// SJB 2023-01-06 Wetout Min Value 저장 수정
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
// JCY 2022-12-24 Peri 도포 FDC 항목 추가 및 CG-Start, Length 텍스트 수정 
// JCY 2022-12-26 trace 라인생성 각도 추가
// JCY 2022-12-29 FDC 항목 추가 액튐 유무
// JCY 2023-01-04 Nozzle Tip 계산 시퀀스 추가
// JCY 2023-01-12 CG Judge Area 파라미터 추가
// JCY 2023-01-17 데이터 시트 함수 홀/라인 분기 
// JCY 2023-01-17 C3부터 데이터 삭제 요청(김시현 프로)
// JCY 2023-01-19 Wetout, overflow Min 좌표 설정
//////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////
//KTY 2023-01-03 Grid CG to Start 값 추가
//KTY 2023-01-03 Line/Circle 구분
//KTY 2023-01-06 Line 도포 시 CG-start 값
//KTY 2023-01-06 Line 도포 시 CG-end 값
//////////////////////////////////////////////////////////////////