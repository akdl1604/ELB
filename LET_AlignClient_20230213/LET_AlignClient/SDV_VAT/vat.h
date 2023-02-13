#pragma once
#pragma comment(lib, "gdiplus.lib") //라이브러리사용

#define VAT_EXPORTS

#ifdef VAT_EXPORTS
#define VATLIBRARY_API __declspec(dllexport)
#else
#define VATLIBRARY_API __declspec(dllimport)
#endif

#ifdef _MIL_USE
#include <Mil.h>
#endif
#include <atlstr.h>
#include <gdiplus.h>
#include <vector>
//#include "opencv2/core/core.hpp"
//#include "opencv2/imgproc/imgproc.hpp"
//#include "opencv2/imgcodecs.hpp"
#import "Vision.Agent.tlb" no_namespace named_guids 

using namespace std;

//비전, Agent가 공용으로 쓰는 리스트가 저장된 위치
const char Path_PosList[ ] = "D:\\Agent\\EQPList\\PosList.txt";
const char Path_EqpList[ ] = "D:\\Agent\\EQPList\\EQPList.txt";

#pragma region [enum]
enum eErrorType
{
    None_Error,             //Error없음
    NullPosName,      //posName null값 입력
    NullImage,        //Image Null
    NullSourceEqp,    //대상 호기 null
    NullModel,        //Model Name null
    NullPath,         //Path Null
    NullDefects,      //defect Info Null  
    Timeout,          //설정 시간 내 Agent 응답 없음
    MarkNotFound,     //Deep Learning으로 마크 찾지 못함
    DLStatusFail,     //딥러닝 실패(실행 문제)
    DLJudgeFail,      //딥러닝 실패(실행했으나 실제 결과 NG)
    FailImageSave,    //이미지 저장 및 전달 실패
    FailINISave,      //INI파일 저장 실패
    FailCopy,         //agent에서 파일 카피 실패
    ConfirmFail,      //AutoTune Confirm fail 
    UserCancel,       //업데이트 중 사용자 취소
    OnProcessRunning, //요청 내용 처리 중
    etc,              //기타 정의안됨           
};
enum eLineEdgePola
{
    DarkToLight,
    LightToDark,
    Any,
};
/// <summary>
/// 모니터링용 로그에 사용
/// </summary>
enum eMarkFinderType
{
    Vision_Mark = 0,
    Golden_Mark = 1,
    Deep_Learning = 2,
    Manual_Align = 3,
    Optional_Align = 4,
    No_Use = 5,
    Vision_Fail = 6,
};
enum eMarkFindResult
{
    OK = 0,
    NG = 1,
    None = 2,
};
enum eProcessType
{
    Align = 0,
    Inspection_Distance = 1,
    Inspection_Blob = 2,
    Inspection_Pattern = 3,
};
enum eInsp_Result
{
    Insp_OK = 0,
    Spec_Out = 1,
    Manual_Confirm = 2,
    Not_Insp_Proc = 3,
};
enum eOnOff
{
    ON = 0,
    OFF = 1,
};
enum eDistanceInspPos
{
    Width = 0,
    Height = 1,
    Width1 = 2,
    Width2 = 3,
    Height1 = 4,
    Height2 = 5,
};
enum eErr_code
{
    Nomal = 0, //정상 상태
    MFType_MissSelect = 1, //MarkFinder Type은 No_Use로 선택될 수 없다.
    MFType_Overwriting = 2, //MarkFinder Type은 중복으로 사용될 수 없다.
    WritingLog_Fail = 3, //Log Writing Fail
    Use_Old_VersionDLL = 4, //구버젼 함수 호출
    Info_Not_Enough = 5, //ProcessName,MarkPos이 없는 경우
    ProcLog_Align_RevData_Err = 6,
    ProcLog_Align_DataCnt_Err = 7,
    Fail_SaveImage = 8,
};
enum eLightType
{
    Back,
    Coax,
    ring,
    Dome,
    Side,
    Complex,
};

#pragma endregion

#pragma region [struct]
struct XY
{
    float X;
    float Y;

    XY() {
        X = 0; Y = 0;
    }
    XY(float x, float y) {
        X = x; Y = y;
    }
};

struct XYT
{
    double X;
    double Y;
    double T;

    XYT() {
        X = 0; Y = 0; T = 0;
    }
    XYT(double x, double y, double t) {
        X = x; Y = y; T = t;
    }
};

struct DLRst {
    double X;
    double Y;
    double T;
    double Score;
    double HX;
    double HY;
    double HT1;
    double HScore;
    double VX;
    double VY;
    double VT1;
    double VScore;
    DLRst() {
        X = 0; Y = 0; T = 0; Score = 0;
        HX = 0; HY = 0; HT1 = 0; HScore = 0;
        VX = 0; VY = 0; VT1 = 0; VScore = 0;
    }
    DLRst(double x, double y, double t, double score,
        double hx = 0, double hy = 0, double ht = 0, double hscore = 0,
        double vx = 0, double vy = 0, double vt = 0, double vscore = 0,
        bool status = false, bool judgement = false) {
        X = x; Y = y; T = t; Score = score;
        HX = hx; HY = hy; HT1 = ht; HScore = hscore;
        VX = vx; VY = vy; VT1 = vt; VScore = vscore;
    }
};

struct InpsResult
{
    CStringW inspecName;
    double tor;
    double spec;
    double value;
    bool judge;
    InpsResult() {
        inspecName = ""; tor = 0;  spec = 0; value = 0; judge = true;
    }
};

struct MarkResult
{
    CStringW MarkName;
    double PixelX;
    double PixelY;
    double MarkTheta;
    double Score;
    double Score_Spec;
    double RobotX;
    double RobotY;

    eMarkFinderType findType;
    double FixedPosX;
    double FixedPosY;

    MarkResult() {
        MarkName = "";   PixelX = 0; PixelY = 0; MarkTheta = 0; Score = 0; Score_Spec = 0; RobotX = 0; RobotY = 0; FixedPosX = 0; FixedPosY = 0;
        findType = (eMarkFinderType)Vision_Mark;
    }
};

struct ProcessResult
{
    CStringW processName;
    CStringW cellID;
    XYT revData;
    XYT visionOffset;
    eInsp_Result inspResult;
    std::vector<InpsResult> lenghCheck;
    std::vector<MarkResult> markPosiion;
    std::vector<InpsResult> inspPosition;
    double processTime;
    int retryCount;
    ProcessResult() {
        processName = "";
        cellID = "";
        revData = XYT();
        visionOffset = XYT();
        lenghCheck.clear();
        markPosiion.clear();
        inspPosition.clear();
        inspResult = (eInsp_Result)Not_Insp_Proc;
        processTime = 0;
        retryCount = 0;
    }
};

struct MarkFinderResult
{
    eMarkFinderType MFType;
    eOnOff Use;
    eMarkFindResult Result;
    eMarkFindResult Lcheck;
    MarkFinderResult() {
        MFType = (eMarkFinderType)No_Use;
        Use = (eOnOff)OFF;
        Result = (eMarkFindResult)None;
        Lcheck = (eMarkFindResult)None;
    }
    MarkFinderResult(eMarkFinderType _type, eOnOff _use = (eOnOff)OFF, eMarkFindResult _result = (eMarkFindResult)None, eMarkFindResult _check = (eMarkFindResult)None) {
        MFType = _type;
        Use = _use;
        Result = _result;
        Lcheck = _check;
    }
};

struct VisionCamInfo
{
    CStringW camPosition;
    CStringW camModel;
    int expoTime;

    //분해능
    XY resolution;
    double lensScale;

    //화소수 X*Y
    XY pixelCount;
    XY fov;

    VisionCamInfo() {
        camPosition = ""; expoTime = 0;
        /* resolution = XY();*/ lensScale = 0;
        //pixelCount = XY(); fov = XY();
    }
};

struct LightInfo
{
    eLightType lightType;
    int setValue;
    LightInfo() {
        lightType = (eLightType)Back;
        setValue = 0;
    }
};

struct PCStatus
{
    int curHdd;
    int maxHdd;
    int curMemory;
    int maxMemory;
    PCStatus() {
        curHdd = 0;
        maxHdd = 0;
        curMemory = 0;
        maxMemory = 0;
    }
    PCStatus(int _curHdd, int _maxHdd, int _curMemory, int _maxMemory) {
        curHdd = _curHdd;
        maxHdd = _maxHdd;
        curMemory = _curMemory;
        maxMemory = _maxMemory;
    }
};

struct LineParam
{
    CStringW toolName;
    int caliperNum;
    int ignoreNum;
    double searchLength;
    double projectionLength;
    XY startPoint;
    XY endPoint;

    eLineEdgePola edgePola;

    int threshold;
    int filterHalfPixel;

    LineParam() {
        toolName = "NoInfo";
        caliperNum = 0;
        ignoreNum = 0;
        searchLength = 0;
        projectionLength = 0;
        //startPoint = XY();
        //endPoint = XY();
        edgePola = (eLineEdgePola)Any;
        threshold = 0;
        filterHalfPixel = 0;
    }
};

struct PatternParam
{
    XY origin_Point;
    XY mark_Origin;
    double mark_Height;
    double mark_Width;

    double angle_Low;
    double angle_High;
    double scale_Low;
    double scale_High;

    XY roi_Origin;
    double roi_Width;
    double roi_Height;

    PatternParam() {
        //origin_Point = XY();
        //roi_Origin = XY();
        roi_Height = 0;
        roi_Width = 0;
        //mark_Origin = XY();
        mark_Width = 0;
        mark_Height = 0;
        angle_Low = 0;
        angle_High = 0;
        scale_Low = 1;
        scale_High = 1;
    }
};

struct MarkTool
{
    CStringW processName;
    CStringW posName;
    CStringW modelName;
    int index;

    std::vector<LineParam> lst_lineParam;
    PatternParam ptnParam;
    std::vector<LightInfo> lst_light;
    int grabDelay;

    MarkTool() {
        processName = "NoInfo";
        posName = "NoInfo";
        modelName = "NoInfo";
        //lst_lineParam.clear();
        //ptnParam = PatternParam();
        //lst_light.clear();
        grabDelay = 0;
    }
};

//이미지 정보
struct BITBYTE {
    INT height;
    INT width;
    INT stride;
    BYTE* bytes = nullptr;
    Gdiplus::PixelFormat format;
};

#pragma endregion

// DLL 초기화
extern "C" VATLIBRARY_API 
bool InitialDLL();

// agent initail
extern "C" VATLIBRARY_API 
CString InitialAgent(char model[ ], char version[ ]);

// agent connect 상태 확인
extern "C" VATLIBRARY_API 
bool IsConnected();

// agent disconnect
extern "C" VATLIBRARY_API 
bool Disconnect();

// Get DL Result(Bytes)
extern "C" VATLIBRARY_API 
DLRst ReqDL_Bytes(char cellID[ ], ToolType type, char model[ ], int timeout, char pos[ ], byte * imgbytes, int size, int width, int stride, int height, CString & error);

// Get DL Result(Bitmap)
extern "C" VATLIBRARY_API
DLRst ReqDL_Bitmap(char cellID[ ], ToolType type, char model[ ], int timeout, char pos[ ], Gdiplus::Bitmap * img, CString & error);

// Get DL Result(File)
extern "C" VATLIBRARY_API 
DLRst ReqDL_File(char cellID[ ], ToolType type, char model[ ], int timeout, char pos[ ], CString imgpath, CString & error);

#ifdef _MIL_USE
// Get DL Result(MIL_ID)
extern "C" VATLIBRARY_API 
DLRst ReqDL_MILID(char cellID[], ToolType type, char model[], int timeout, char pos[], MIL_ID img, CString & error);
#endif

// image validation (imagepath)
extern "C" VATLIBRARY_API 
void SendLogicRst(char curtime[ ], char cellID[ ], FindType type, char model[ ], char pos[ ], char imagepath[ ], Judge judge, double rstX, double rstY, double rstT, double rstScore, char description[ ]);

#ifdef _MIL_USE
// image validation (MIL_ID)
extern "C" VATLIBRARY_API 
void SendLogicRst_MILID(char curtime[], char cellID[], FindType type, char model[], char pos[], MIL_ID img, Judge judge, double rstX, double rstY, double rstT, double rstScore, char description[]);
#endif

//extern "C" VATLIBRARY_API 
//// 학습 이미지 전송(imagepath)
//void SendManualRst(float rstX, float rstY, float sX, float sY, float lX, float lY, char cellID[ ], char model[ ], char pos[ ], char imagepath[ ], ManualReason reason, CString & error);

extern "C" VATLIBRARY_API
// 학습 이미지 전송(imagepath)
void SendManualRst_Bytes(float rstX, float rstY, float sX, float sY, float lX, float lY, char cellID[], char model[], char pos[], byte * imgbytes, int size, int width, int stride, int height, ManualReason reason, CStringW & error);

#ifdef _MIL_USE
// 학습 이미지 전송(MIL_ID)
extern "C" VATLIBRARY_API
void SendManualRst_MILID(float rstX, float rstY, float sX, float sY, float lX, float lY, char cellID[], char model[], char pos[], MIL_ID img, ManualReason reason, CString & error);
#endif

// 딥러닝 결과를 Agent에 전달
extern "C" VATLIBRARY_API 
void SendDLImg(FindType type, char model[ ], char pos[ ], char imgpath[ ], double rstX, double rstY, double rstT, double rstScore, char description[ ]);

// Model Copy
extern "C" VATLIBRARY_API 
CString ModelCopy(char model[ ], int timeout, char destpath[ ], char eqp[ ], CString & error);

//c++ model copy 중 cancel 할 때
extern "C" VATLIBRARY_API
void SetModelCopyCancel(bool cancel);

//Agent로 Vision Model Name 전송
extern "C" VATLIBRARY_API 
void SendModel(char model[ ]);

// Golden Mark 요청(Bitmap)
extern "C" VATLIBRARY_API 
CString ATTUpdate(char model[ ], int timeout, char pos[ ], Gdiplus::Bitmap * img, CString & error);

#ifdef _MIL_USE
// Golden Mark 요청(MIL_ID)
extern "C" VATLIBRARY_API 
CString ATTUpdate_MILID(char model[], int timeout, char pos[], MIL_ID img, CString & error);
#endif

// Golden Mark 복사
extern "C" VATLIBRARY_API 
CString ATTCopy(char model[ ], int timeout, char pos[ ], char eqp[ ], CString & error);




// agent 연결 (멀티바이트)
extern "C" VATLIBRARY_API
LPCSTR InitialAgentW(char model[], char version[]);

// Get DL Result(Bytes, 멀티바이트)
extern "C" VATLIBRARY_API
DLRst ReqDL_BytesW(char cellID[], ToolType type, char model[], int timeout, char pos[], byte * imgbytes, int size, int width, int stride, int height, LPCSTR & error);

// Get DL Result(Bitmap, 멀티바이트)
extern "C" VATLIBRARY_API
DLRst ReqDL_BitmapW(char cellID[], ToolType type, char model[], int timeout, char pos[], Gdiplus::Bitmap * img, LPCSTR & error);

// Get DL Result(File, 멀티바이트)
extern "C" VATLIBRARY_API
DLRst ReqDL_FileW(char cellID[], ToolType type, char model[], int timeout, char pos[], CStringW imgpath, LPCSTR & error);

#ifdef _MIL_USE
// Get DL Result(MIL_ID, 멀티바이트)
extern "C" VATLIBRARY_API
DLRst ReqDL_MILIDW(char cellID[], ToolType type, char model[], int timeout, char pos[], MIL_ID img, LPCSTR & error);
#endif

//// 학습 이미지 전송(imagepath, 멀티바이트)
//extern "C" VATLIBRARY_API
//void SendManualRstW(float rstX, float rstY, float sX, float sY, float lX, float lY, char cellID[], char model[], char pos[], char imagepath[], ManualReason reason, LPCSTR& error);

#ifdef _MIL_USE
// 학습 이미지 전송(MIL_ID, 멀티바이트)
extern "C" VATLIBRARY_API
void SendManualRst_MILIDW(float rstX, float rstY, float sX, float sY, float lX, float lY, char cellID[], char model[], char pos[], MIL_ID img, ManualReason reason, LPCSTR & error);
#endif

// Model Copy(멀티바이트)
extern "C" VATLIBRARY_API
LPCSTR ModelCopyW(char model[], int timeout, char destpath[], char eqp[], LPCSTR & error);

// Golden Mark 요청(Bitmap, 멀티바이트)
extern "C" VATLIBRARY_API
LPCSTR ATTUpdateW(char model[], int timeout, char pos[], Gdiplus::Bitmap * img, LPCSTR & error);

#ifdef _MIL_USE
// Golden Mark 요청(MIL_ID, 멀티바이트)
extern "C" VATLIBRARY_API
LPCSTR ATTUpdate_MILIDW(char model[], int timeout, char pos[], MIL_ID img, LPCSTR & error);
#endif

// Golden Mark 복사(멀티바이트)
extern "C" VATLIBRARY_API
LPCSTR ATTCopyW(char model[], int timeout, char pos[], char eqp[], LPCSTR & error);




extern "C" VATLIBRARY_API
eErr_code WriteMatchingHistoryLog(vector<MarkFinderResult> _MFResults, eMarkFinderType _FinalMF, CStringW _processName, CStringW _mark_Pos, CStringW _cellID);

extern "C" VATLIBRARY_API
eErr_code WriteProcessHistoryLog(eProcessType _type, ProcessResult _procResult);

extern "C" VATLIBRARY_API
eErr_code SendProcessImage_Bitmap(Gdiplus::Bitmap * _Image, eMarkFinderType _mfType, CStringW _processName, CStringW _camPos, CStringW _cellID);

extern "C" VATLIBRARY_API
eErr_code SendProcessImage_Mat(cv::Mat _Image, eMarkFinderType _mfType, CStringW _processName, CStringW _camPos, CStringW _cellID);

extern "C" VATLIBRARY_API
eErr_code SendProcessImage_Bytes(BITBYTE _Image, eMarkFinderType _mfType, CStringW _processName, CStringW _camPos, CStringW _cellID);

extern "C" VATLIBRARY_API
eErr_code WriteVisionCamInfo(vector<VisionCamInfo> _lstCaminfo);

extern "C" VATLIBRARY_API
eErr_code WritePCStatus(PCStatus _pcstatus);

extern "C" VATLIBRARY_API
eErr_code UpdateMarkParameter_Bitmap(MarkTool _mTool, Gdiplus::Bitmap * _markImg);

extern "C" VATLIBRARY_API
eErr_code UpdateMarkParameter_Mat(MarkTool _mTool, cv::Mat _markImg);

extern "C" VATLIBRARY_API
eErr_code UpdateMarkParameter_Bytes(MarkTool _mTool, BITBYTE _markImg);

extern "C" VATLIBRARY_API
eErr_code UpdateLineParameter(MarkTool _lineTool);