#pragma once

#ifndef __COMPLC_RS_H__
#define __COMPLC_RS_H__

#include "Socket.h"
#include "ZObject/ZEvent.h"
#include "ZObject/ZQueue.h"

#ifndef WM_VIEW_CONTROL
#define WM_VIEW_CONTROL WM_USER + 0x1097
#endif

#ifndef MSG_UPDATE_CONNECTION_STATUS
#define MSG_UPDATE_CONNECTION_STATUS	8279
#endif

typedef struct stSendMsgPlc
{
	HANDLE EventHandle;			//Receive 받은 후 Event를 주기 위한 Handle
	char* pMCProtocolMsg;		//Send Message
	int nDataLength;				//Send할 Message의 크기
	int* nReturn;			//Time Out을 알기위한 long pointer void
	void* lpReadValue;			//Read된 Value를 알기위한 long pointer void형식 (현재 source가 long 혹은 short으로 되어있네...)
	int nReadDataLength;
	int nDatatype;				//0 Int, 1 short

	stSendMsgPlc()
	{
		EventHandle = NULL;
		pMCProtocolMsg = NULL;
		nReturn = 0;
		nDataLength = 0;	
		lpReadValue = NULL;
		nReadDataLength = 0;
		nDatatype = 0;
	}

}stSendMsgPlc, *pstSendMsgPlc;

class CELBLineCAMProgramDlg;

#define BIT_COUNT_PER_WORD	16

class CComPLC_RS
{
public:
	CComPLC_RS(void);
	~CComPLC_RS(void);

	BOOL m_bOpened;

private:
	CRITICAL_SECTION m_csComm;
	BOOL m_bProgramEnd;
	BOOL m_bConnected;
	CString m_strSocketIp;
	CString m_strSocketPort;
	DWORD m_nTimeout;
	UINT m_nTimerID;
	CString	m_strReceive;
	CString m_recvUDP_Data;
	HWND m_hCommWnd;
	HANDLE            mh_Thread;
	TCP::cSocket      mi_Socket;
	TCP::cSocket::cHash<SOCKET, DWORD> mi_SocketList;

	HANDLE m_hWowEvent;	

	BYTE m_pReceiveBuffer[2048];
	int m_nRecvLength;

	/// <summary>
	/// Semaphore Handle
	/// </summary>
	CZSemaphore* m_pSemaMessage;

	CZQueue<stSendMsgPlc> m_pSendData;

	bool m_bIsThreadAlive;

	bool m_bIsThreadEnd;	

public:
	int m_nLastCmd;
	int m_nID;
	bool m_bEndCommand;
	bool m_bReceiveDataStarted;
	CLET_AlignClientDlg* m_pMain;

	BOOL isConnect() { return m_bConnected; }

	void CloseSockets();
	void ProcessEvents();
	static ULONG WINAPI ProcessEventThread(void* p_Param);

	void StartTimer();
	void StopTimer();
	void MMTimerHandler(UINT nIDEvent);

	// TCP/IP
	BOOL Connect();
	BOOL Connect(CString strSocketIp, CString strSocketPort);

	void setSocketIp(CString strIp) { m_strSocketIp = strIp; }
	void setSocketPort(CString strPort) { m_strSocketPort = strPort; }
	void setMsgHwnd(HWND hCommWnd);
	BOOL SendTo(SOCKET h_Socket, char *cData, int nLen);
	int fn_SendCommandData(char *cData, int nLen);

	// Command
	BOOL Init(int nStationIn = 0, int nStationOut = 1);
	BOOL Close();
	BOOL GetBit(long nAddress);
	void SetBit(long nAddress, BOOL bOnOff);
	void GetWord(long nAddress, long nDataSize, long *lpData);
	long GetTotalBitData(long nStarAddress, long nSize, long *lpData);
	void SetWord(long nAddress, long nDataSize, long *lpData);

	int fnReadBitM(int nAddr_Srt, int nLength, short *pData10, int nReadSize = 0);
	int fnReadBitM(int nAddr_Srt, int nLength, long *pData11, int nReadSize = 0);
	int fnReadBitL(int nAddr_Srt, int nLength, short *pData1, int nReadSize = 0);
	int fnReadBitL(int nAddr_Srt, int nLength, long *pData2, int nReadSize = 0);
	bool fnReadBitL(int nAddr);

	int fnWriteBit(int nAddr, bool bOnOff);
	int fnWriteBitL(int nAddr, bool bOnOff);
	int fnWriteBitM(int nAddr, bool bOnOff);

	
	int fnReadWord(int nAddr_Srt, int nLength, short *pData3, int nReadSize = 0);
	int fnReadWord(int nAddr_Srt, int nLength, long *pData4, int nReadSize =0);
	int fnReadWordD(int nAddr_Srt, int nLength, short *pData3, int nReadSize = 0);
	int fnReadWordD(int nAddr_Srt, int nLength, long *pData4, int nReadSize = 0);
	int fnWriteWord(int nAddr, int nLength, short *pData5);
	int fnWriteWord(int nAddr, int nLength, long *pData6);	

	int fnReadWord32(int nAddr_Srt, int nLength, int *pData8, int nReadSize = 0);
	int fnWriteWord32(int nAddr, int nLength, int *pData9);

	int fnReadWordZR(int nAddr_Srt, int nLength, short* pData7, int nReadSize = 0);
	int fnReadWordZR(int nAddr_Srt, int nLength, long* pData7, int nReadSize = 0);	
	int fnWriteWordZR(int nAddr, int nLength, short* pData7);
	int fnWriteWordZR(int nAddr, int nLength, long* pData7);
	long GetTotalWriteBitData(long nStarAddress, long nSize, long *lpData);
	int fnWriteBitArrayM_OFF(int num_of_addr, int* pAddr);
	int fnWriteBitArrayM_ON(int num_of_addr, int* pAddr);

	/// <summary>
	/// Thread Process Function
	/// </summary>
	/// <param name="lpParam_">해당 Class Pointer</param>
	/// <returns></returns>
	static UINT ThreadMessage(LPVOID lpParam_);

	void WaitEvent(INT32 nMilliSec_);
	void MyDoEvents();
};

//extern CComPLC_RS g_CommTriggerPLC;
extern CComPLC_RS g_CommPLC_MC;
#endif