#include "stdafx.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"
#include <vector>

#include "SamsungVATWraper.h"

#ifdef _SDV_VAT
#pragma comment(lib,"SDV_VAT/VATdll_CPP.lib")

SamsungVATWraper::SamsungVATWraper()
{
	
	m_bInitalize = false;
}

SamsungVATWraper::~SamsungVATWraper()
{
	if (m_bInitalize && IsConnected())
		Disconnect();
}
bool SamsungVATWraper::InitVAT_DLL() 
{
	bool check = InitialDLL();
	return true;
}
bool SamsungVATWraper::Agent_Initial(char* modelName, char* version)
{
	CString rString = InitialAgentW(modelName, version);
	m_bInitalize = true;

	return m_bInitalize;
}
bool SamsungVATWraper::Agent_IsConnected()
{
	return IsConnected();
}
bool SamsungVATWraper::Agent_Disconnect()
{
	return Disconnect();
}
DLRst SamsungVATWraper::Agent_ReqDL_Bytes(char cellID[], ToolType type, char model[], int timeout, char pos[], byte* imgbytes, int size, int width, int stride, int height, CString& error)
{
	//const char msgError[MAX_PATH] = { 0, };
	const char* msgError = new char[MAX_PATH];		

	DLRst rst = DLRst();
	//timeout = 500;
	rst = ReqDL_BytesW(cellID, type, model, timeout, pos, imgbytes, size, width, stride, height, (LPCSTR)msgError);
	error.Format("%s", msgError);
		
	//delete[] msgError;
	
	return rst;
}
DLRst SamsungVATWraper::Agent_ReqDL_Bitmap(char cellID[], ToolType type, char model[], int timeout, char pos[], Gdiplus::Bitmap* img, CString& error)
{
	const char* msgError = new char[MAX_PATH];
	DLRst rst = ReqDL_BitmapW(cellID, type, model, timeout, pos, img, (LPCSTR)msgError);
	error.Format("%s", msgError);
	return rst;
}
DLRst SamsungVATWraper::Agent_ReqDL_File(char cellID[], ToolType type, char model[], int timeout, char pos[], CString imgpath, CString& error)
{
	const char* msgError = new char[MAX_PATH];
	DLRst rst;
	//rst = ReqDL_FileW(cellID, type, model, timeout, pos, imgpath, (LPCSTR)msgError);
	error.Format("%s", msgError);
	return rst;
}
void SamsungVATWraper::Agent_SendLogicRst(char curtime[], char cellID[], FindType type, char model[], char pos[], char imagepath[], Judge judge, double rstX, double rstY, double rstT, double rstScore, char description[])
{
	SendLogicRst(curtime, cellID, type, model, pos, imagepath, judge, rstX, rstY, rstT, rstScore, description);
}

void SamsungVATWraper::Agent_SendManualRst_Bytes(float rstX, float rstY, float sX, float sY, float lX, float lY, char cellID[], char model[], char pos[], byte * imgbytes, int size, int width, int stride, int height, ManualReason reason, CStringW & error)
{
	//const char* msgError = new char[MAX_PATH];
	//LPCSTR msgError = new char[MAX_PATH];

	SendManualRst_Bytes(rstX, rstY, sX, sY, lX, lY, cellID, model, pos, imgbytes, size, width, stride, height, reason, error);	
}
//void SamsungVATWraper::Agent_SendManualRst(float rstX, float rstY, float sX, float sY, float lX, float lY, char cellID[], char model[], char pos[], char imagepath[], ManualReason reason, CString& error)
//{
	//const char* msgError = new char[MAX_PATH];
	//LPCSTR msgError = new char[MAX_PATH];

	//SendManualRstW(rstX, rstY, sX, sY, lX, lY, cellID, model, pos, imagepath, reason, msgError);

	//error.Format("%s", msgError);
//}
void SamsungVATWraper::Agent_SendDLImg(FindType type, char model[], char pos[], char imgpath[], double rstX, double rstY, double rstT, double rstScore, char description[])
{
	SendDLImg(type, model, pos, imgpath, rstX, rstY, rstT, rstScore, description);
}
CString SamsungVATWraper::Agent_ModelCopy(char model[], int timeout, char destpath[], char eqp[], CString& error)
{
	CString rstring;
	const char* msgError = new char[MAX_PATH];

	rstring =  ModelCopyW(model, timeout, destpath, eqp, (LPCSTR)msgError);
	error.Format("%s", msgError);
	return rstring;
}
void SamsungVATWraper::Agent_SetModelCopyCancel(bool cancel)
{
	SetModelCopyCancel(cancel);
}
void SamsungVATWraper::Agent_SendModel(char model[])
{
	SendModel(model);
}
CString SamsungVATWraper::Agent_ATTUpdate(char model[], int timeout, char pos[], Gdiplus::Bitmap* img, CString& error)
{
	CString rstring;
	const char* msgError = new char[MAX_PATH];

	rstring = ATTUpdateW(model, timeout, pos, img, (LPCSTR)msgError);
	return rstring;
}
CString SamsungVATWraper::Agent_ATTCopy(char model[], int timeout, char pos[], char eqp[], CString& error)
{
	CString rstring;
	const char* msgError = new char[MAX_PATH];

	rstring = ATTCopyW(model, timeout, pos, eqp, (LPCSTR)msgError);
	return rstring;
}
eErr_code SamsungVATWraper::Agent_WriteMatchingHistoryLog(int resultcnt, MarkFinderResult results[], eMarkFinderType finalMf, WCHAR processName[], WCHAR mark_Pos[], WCHAR cellID[])
{
	//return WriteMatchingHistoryLog(resultcnt, results, finalMf, processName, mark_Pos, cellID);
	std::vector<MarkFinderResult> _MFResults;
	_MFResults.push_back(results[0]);
	_MFResults.push_back(results[1]);
	_MFResults.push_back(results[2]);

	eErr_code err;
	err = WriteMatchingHistoryLog(_MFResults, finalMf, processName, mark_Pos, cellID);

	_MFResults.clear();

	return err;


}
eErr_code SamsungVATWraper::Agent_WriteProcessHistoryLog(eProcessType _type, ProcessResult _processResult)
{
	eErr_code err = WriteProcessHistoryLog(_type, _processResult);

	return err;
}

eErr_code SamsungVATWraper::Agent_UpdateMarkParameter_Bitmap(MarkTool _mTool, Gdiplus::Bitmap * _markImg)
{
	eErr_code err = eErr_code::Nomal;
	err = UpdateMarkParameter_Bitmap(_mTool, _markImg);

	return err;
}
eErr_code SamsungVATWraper::Agent_UpdateMarkParameter_Bytes(MarkTool _mTool, BITBYTE _markImg)
{
	eErr_code err = eErr_code::Nomal;	

	err = UpdateMarkParameter_Bytes(_mTool, _markImg);
	
	return err;

}

eErr_code SamsungVATWraper::Agent_UpdateMarkParameter_Mat(MarkTool _mTool, cv::Mat _markImg)
{
	eErr_code err = eErr_code::Nomal;

	err = UpdateMarkParameter_Mat(_mTool, _markImg);

	return err;

}


eErr_code SamsungVATWraper::Agent_WriteVisionCamInfo(vector<VisionCamInfo> _lstCaminfo)
{
	eErr_code err = eErr_code::Nomal;

	err = WriteVisionCamInfo(_lstCaminfo);

	return err;
}

eErr_code SamsungVATWraper::Agent_WritePCStatus(PCStatus _pcstatus)
{
	eErr_code err = eErr_code::Nomal;

	err = WritePCStatus(_pcstatus);

	return err;
}


eErr_code SamsungVATWraper::Agent_SendProcessImage_Bitmap(Gdiplus::Bitmap * _Image, eMarkFinderType _mfType, CStringW _processName, CStringW _camPos, CStringW _cellID)
{
	eErr_code err = eErr_code::Nomal;
	err =  SendProcessImage_Bitmap(_Image, _mfType, _processName,_camPos,_cellID);

	return err;
}

eErr_code SamsungVATWraper::Agent_SendProcessImage_Mat(cv::Mat _Image, eMarkFinderType _mfType, CStringW _processName, CStringW _camPos, CStringW _cellID)
{
	eErr_code err = eErr_code::Nomal;
	err = SendProcessImage_Mat(_Image, _mfType, _processName, _camPos, _cellID);

	return err;
}

#endif