#pragma once

#ifdef _SDV_VAT

#include <vat.h>

class SamsungVATWraper
{
public:
    SamsungVATWraper();
    virtual ~SamsungVATWraper();

public:
    bool InitVAT_DLL();
    bool Agent_Initial(char* modelName, char* version);
    bool Agent_IsConnected(); // agent connect 상태 확인
    bool Agent_Disconnect();  // agent disconnect

    // Get DL Result(Bytes) ,// Get DL Result(Bitmap) // Get DL Result(File)
    DLRst Agent_ReqDL_Bytes(char cellID[], ToolType type, char model[], int timeout, char pos[], byte * imgbytes, int size, int width, int stride, int height, CString & error);
    DLRst Agent_ReqDL_Bitmap(char cellID[], ToolType type, char model[], int timeout, char pos[], Gdiplus::Bitmap * img, CString & error);
    DLRst Agent_ReqDL_File(char cellID[], ToolType type, char model[], int timeout, char pos[], CString imgpath, CString & error);
    
    // image validation (imagepath) /// 학습 이미지 전송(imagepath) // 딥러닝 결과를 Agent에 전달
    void Agent_SendLogicRst(char curtime[], char cellID[], FindType type, char model[], char pos[], char imagepath[], Judge judge, double rstX, double rstY, double rstT, double rstScore, char description[]);
    //void Agent_SendManualRst(float rstX, float rstY, float sX, float sY, float lX, float lY, char cellID[], char model[], char pos[], char imagepath[], ManualReason reason, CString & error);
	void Agent_SendManualRst_Bytes(float rstX, float rstY, float sX, float sY, float lX, float lY, char cellID[], char model[], char pos[], byte * imgbytes, int size, int width, int stride, int height, ManualReason reason, CStringW & error);
	void Agent_SendDLImg(FindType type, char model[], char pos[], char imgpath[], double rstX, double rstY, double rstT, double rstScore, char description[]);

    // Model Copy //c++ model copy 중 cancel 할 때 //Agent로 Vision Model Name 전송
    CString Agent_ModelCopy(char model[], int timeout, char destpath[], char eqp[], CString & error);
    void Agent_SetModelCopyCancel(bool cancel);
    void Agent_SendModel(char model[]);

    // Golden Mark 요청(Bitmap)// Golden Mark 복사
    CString Agent_ATTUpdate(char model[], int timeout, char pos[], Gdiplus::Bitmap * img, CString & error);
    CString Agent_ATTCopy(char model[], int timeout, char pos[], char eqp[], CString & error);
    eErr_code Agent_WriteMatchingHistoryLog(int resultcnt, MarkFinderResult results[], eMarkFinderType finalMf, WCHAR processName[], WCHAR mark_Pos[], WCHAR cellID[]);
    eErr_code Agent_WriteProcessHistoryLog(eProcessType _type, ProcessResult _processResult);
	eErr_code Agent_UpdateMarkParameter_Bitmap(MarkTool _mTool, Gdiplus::Bitmap * _markImg);
	eErr_code Agent_UpdateMarkParameter_Bytes(MarkTool _mTool, BITBYTE _markImg);
	eErr_code Agent_UpdateMarkParameter_Mat(MarkTool _mTool, cv::Mat _markImg);
	eErr_code Agent_WriteVisionCamInfo(vector<VisionCamInfo> _lstCaminfo);
	eErr_code Agent_WritePCStatus(PCStatus _pcstatus);
	eErr_code Agent_SendProcessImage_Bitmap(Gdiplus::Bitmap * _Image, eMarkFinderType _mfType, CStringW _processName = L"NoInfo", CStringW _camPos = L"NoInfo", CStringW _cellID = L"NoInfo");
	eErr_code Agent_SendProcessImage_Mat(cv::Mat _Image, eMarkFinderType _mfType, CStringW _processName = L"NoInfo", CStringW _camPos = L"NoInfo", CStringW _cellID = L"NoInfo");

public:

    bool m_bInitalize;
    BOOL m_bConnected;
};

#endif