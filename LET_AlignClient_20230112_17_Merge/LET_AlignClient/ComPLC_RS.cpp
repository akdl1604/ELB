#include "stdAfx.h"
#include "ComPLC_RS.h"
#include "LET_AlignClientDlg.h"
#include "mmsystem.h"

#pragma comment (lib, "winmm")

#define L_AREA
#define EVENT_NAME "Event Object Receive"
////////////////////////
// Device Code(0x92)  ===>L 영역
// Device Code(0x90)  ===>M 영역
// enum {
//SM = 0x91, SD = 0xA9, X = 0x9C, Y = 0x9D, M = 0x90, L = 0x92, F = 0x93, V = 0x94, B = 0xA0, D = 0xA8, W = 0xB4,
//TS = 0xC1, TC = 0xC0, SS = 0xC7, SC = 0xC6, SN = 0xC8, CS = 0xC4, CC = 0xC3, CN = 0xC5, SB = 0xA1, SW = 0xB5, S = 0x98,
//DX = 0xA2, DY = 0xA3, Z = 0xCC, R = 0xAF, ZR = 0xB0
//};
//////////////////////////


CComPLC_RS g_CommPLC_MC;

CComPLC_RS::CComPLC_RS(void)
{
	m_nTimerID = NULL;
	m_nTimeout = 2000;
	m_hCommWnd = NULL;
	m_bConnected = FALSE;
	m_bOpened = FALSE;

	InitializeCriticalSection(&m_csComm);
	m_nLastCmd = -1;
	m_bEndCommand = false;
	m_bReceiveDataStarted = false;

	//============
	//[ZHyunTM 20221121]singlelock -> Semaphore로 변경한다.
	CString strTemp;

	strTemp.Format(_T("CComPLC_RS_%d"), this);

	m_pSemaMessage = new CZSemaphore(0, 20, strTemp);

	m_bIsThreadAlive = true;

	m_bIsThreadEnd = false;

	m_pSendData.Initialize(20);
	//============

	m_hWowEvent = CreateEvent(0, FALSE, FALSE, 0);



	AfxBeginThread(ThreadMessage, (LPVOID)this);
}


CComPLC_RS::~CComPLC_RS(void)
{
	m_bIsThreadAlive = false;
	m_pSemaMessage->Release(1);

	m_pSemaMessage->Close();

	while (1)
	{
		WaitEvent(10);
		if (m_bIsThreadEnd) break;
	}

	if (m_pSemaMessage != NULL)
	{
		delete m_pSemaMessage;
	}

	m_bProgramEnd = TRUE;
	StopTimer();
	DeleteCriticalSection(&m_csComm);

	CloseHandle(m_hWowEvent);
}

void CALLBACK TimerFunction_COMplc_RS(UINT wTimerID, UINT msg, DWORD_PTR dwUser, DWORD dw1, DWORD dw2)
{

	CComPLC_RS *obj = (CComPLC_RS *)dwUser;
	obj->MMTimerHandler(wTimerID);
}

void CComPLC_RS::StartTimer()
{
	return;

	TIMECAPS tc;
	timeGetDevCaps(&tc, sizeof(TIMECAPS));
	DWORD resolution = MIN(MAX(tc.wPeriodMin, 0), tc.wPeriodMax);
	timeBeginPeriod(resolution);

	// create the timer
	m_nTimerID = timeSetEvent(m_nTimeout, resolution, (LPTIMECALLBACK)::TimerFunction_COMplc_RS, (DWORD_PTR)this, TIME_PERIODIC);
}
//----------------------------------------------------------------------------------------
void CComPLC_RS::StopTimer()
{
	// destroy the timer
	if (m_nTimerID != NULL)
	{
		timeKillEvent(m_nTimerID);
		timeEndPeriod(m_nTimeout);
	}
}
//----------------------------------------------------------------------------------------

void CComPLC_RS::MMTimerHandler(UINT nIDEvent) // called every elTime milliseconds
{
	StopTimer();
	Connect();
}

ULONG WINAPI CComPLC_RS::ProcessEventThread(void* p_Param)
{
	CComPLC_RS* p_This = (CComPLC_RS*)p_Param;
	p_This->ProcessEvents();
	CloseHandle(p_This->mh_Thread);
	return 0;
}
void CComPLC_RS::ProcessEvents()
{
	BOOL b_Server = (mi_Socket.GetState() & TCP::cSocket::E_Server);
	BOOL bReConnect = FALSE;
	while (TRUE)
	{
		TCP::cSocket::cMemory* pi_RecvMem;
		SOCKET  h_Socket;
		DWORD u32_Event, u32_IP, u32_Read, u32_Sent;
		DWORD u32_Err = mi_Socket.ProcessEvents(&u32_Event, &u32_IP, &h_Socket, &pi_RecvMem, &u32_Read, &u32_Sent);

		if (u32_Err == ERROR_TIMEOUT) 			continue;
		if (u32_Event) // ATTENTION: u32_Event may be == 0 -> do nothing.
		{
			char s8_Events[200];
			mi_Socket.FormatEvents(u32_Event, s8_Events);

			if (u32_Event & FD_CLOSE)
			{
				StartTimer();
				::SendMessageA(m_hCommWnd, WM_VIEW_CONTROL, MSG_PLC_CONNECT_STATUS, MAKELPARAM(FALSE, m_nID));
				m_bConnected = FALSE;
				m_bOpened = FALSE;
			}

			if (u32_Event & FD_CONNECT)
			{
				if (mi_Socket.GetState() & TCP::cSocket::E_Connected)
				{
					mi_Socket.GetAllConnectedSockets(&mi_SocketList);

					if (mi_SocketList.GetCount() > 0)
					{
						::SendMessageA(m_hCommWnd, WM_VIEW_CONTROL, MSG_PLC_CONNECT_STATUS_CONNECT, MAKELPARAM(TRUE, m_nID));
						m_bConnected = TRUE;
						m_bOpened = TRUE;
					}
				}
			}

			if (u32_Event & FD_READ && pi_RecvMem) // pi_RecvMem may be NULL if an error occurred!!
			{
				if (!m_bReceiveDataStarted)
				{
					m_nRecvLength = 0;
					m_bReceiveDataStarted = true;
				}

				BYTE *pBuffer = m_pReceiveBuffer + m_nRecvLength;
				DWORD u32_Len = pi_RecvMem->GetLength();

				memcpy(pBuffer, pi_RecvMem->GetBuffer(), u32_Len);
				pi_RecvMem->DeleteLeft(u32_Len);

				m_nRecvLength += u32_Len;

				if (m_bReceiveDataStarted && pi_RecvMem->GetLength() == 0)
				{
					m_bEndCommand = true;
					m_bReceiveDataStarted = false;
					SetEvent(m_hWowEvent);
				}
			}
		}

		if (u32_Err)
		{
			if (u32_Err == WSAECONNREFUSED)				bReConnect = TRUE;
			if (u32_Err != WSAECONNABORTED && // e.g. after the other side was killed in TaskManager 
				u32_Err != WSAECONNRESET && // Connection reset by peer.
				u32_Err != WSAECONNREFUSED && // FD_ACCEPT with already 62 clients connected
				u32_Err != WSAESHUTDOWN)      // Sending data to a socket just in the short timespan 
				break;                        //   between shutdown() and closesocket()
		}
	};

	CloseSockets();
	if (bReConnect && !m_bProgramEnd)
	{
		StartTimer();
	}
}
void CComPLC_RS::CloseSockets()
{
	if (mi_Socket.GetSocketCount())
	{
		mi_Socket.Close();
	}
}
BOOL CComPLC_RS::Connect(CString strSocketIp, CString strSocketPort)
{
	if (mi_Socket.GetSocketCount())		return TRUE;

	m_strSocketIp = strSocketIp;
	m_strSocketPort = strSocketPort;

	DWORD u32_IP = inet_addr(strSocketIp);
	DWORD u32_ID;
	DWORD u32_EventTimeout = INFINITE;
	DWORD u32_Err = mi_Socket.ConnectTo(u32_IP, atoi(strSocketPort), u32_EventTimeout, 0);

	if (u32_Err)
	{
		CloseSockets();
		StartTimer();
		return FALSE;
	}

	mh_Thread = CreateThread(0, 0, ProcessEventThread, this, 0, &u32_ID);
	return TRUE;
}
BOOL CComPLC_RS::Connect()
{
	if (mi_Socket.GetSocketCount())		return TRUE;

	DWORD u32_IP = inet_addr(m_strSocketIp);
	DWORD u32_ID;
	DWORD u32_EventTimeout = INFINITE;
	DWORD u32_Err = mi_Socket.ConnectTo(u32_IP, atoi(m_strSocketPort), u32_EventTimeout, 0);

	if (u32_Err)
	{
		CloseSockets();
		StartTimer();
		return FALSE;
	}

	mh_Thread = CreateThread(0, 0, ProcessEventThread, this, 0, &u32_ID);

	return TRUE;
}
void CComPLC_RS::setMsgHwnd(HWND hCommWnd)
{
	m_hCommWnd = hCommWnd;
}
BOOL CComPLC_RS::fn_SendCommandData(char *cData, int nLen)
{
	if (mi_SocketList.GetCount() <= 0) mi_Socket.GetAllConnectedSockets(&mi_SocketList);
	if (mi_SocketList.GetCount() < 1) return FALSE;

	SOCKET h_Socket = mi_SocketList.GetKeyByIndex(0);

	DWORD u32_Err = mi_Socket.SendTo(h_Socket, cData, nLen);

	switch (u32_Err)
	{
	case 0:		return TRUE;
	case WSAEWOULDBLOCK:		//AfxMessageBox(_T("WSAEWOULDBLOCK -> The data will be send after the next FD_WRITE event."));
		return FALSE;
	case WSA_IO_PENDING:		//AfxMessageBox(_T("WSA_IO_PENDING -> Error: A previous Send operation is still pending. This data will not be sent."));
		return FALSE;
	default:
		CloseSockets();
		StartTimer();
		return FALSE;
	}
}
BOOL CComPLC_RS::Init(int nStationIn, int nStationOut)
{
	m_pMain = (CLET_AlignClientDlg*)AfxGetMainWnd();

	if (m_bOpened) return TRUE;

	setMsgHwnd(m_pMain->m_hWnd);

	return Connect("192.168.2.39", "3000");
}
BOOL CComPLC_RS::Close()
{
	return TRUE;
}

BOOL CComPLC_RS::GetBit(long nAddress)
{
	if (m_pMain == NULL) return FALSE;

	BOOL bValue = fnReadBitL(int(nAddress)) ? TRUE : FALSE;

	return bValue;
}
void CComPLC_RS::SetBit(long nAddress, BOOL bOnOff)
{
	fnWriteBit(int(nAddress), (bOnOff == TRUE) ? true : false);
}
void CComPLC_RS::GetWord(long nAddress, long nDataSize, long *lpData)
{
	fnReadWord(int(nAddress), int(nDataSize), (short *)(lpData));
}
void CComPLC_RS::SetWord(long nAddress, long nDataSize, long *lpData)
{
	fnWriteWord(int(nAddress), int(nDataSize), (short *)(lpData));
}
long CComPLC_RS::GetTotalBitData(long nStarAddress, long nSize, long *lpData)
{
	long nReturnCode = 0;

	nReturnCode = fnReadBitL(int(nStarAddress), int(nSize), (short *)(lpData));

	return nReturnCode;
}
bool CComPLC_RS::fnReadBitL(int nAddr)
{
	if (m_bConnected == FALSE) return false;

	int nIndex = 0;
	int m_nLen_S = 21;

	char* m_pData_S = new char[25];
	ZeroMemory(m_pData_S, 25);

	m_pData_S[0] = 0x50;	// Sub Head L
	m_pData_S[1] = 0x00;	// Sub Head H
	m_pData_S[2] = 0x00;	// Network Number
	m_pData_S[3] = (char)0xFF;	// PLC Number
	m_pData_S[4] = (char)0xFF;	// 요구 상대 국번 IO Number H
	m_pData_S[5] = 0x03;	// 요구 상대 국번 IO Number L

	m_pData_S[6] = 0x00;	// 요구 상대 모듈 국번호
	m_pData_S[7] = 0x0C;	// 요구 Data 길이 L
	m_pData_S[8] = 0x00;	// 요구 Data 길이 R ( Data 길이 - 8 )

	m_pData_S[9] = 0x10;	// CPU 감시 Timer L
	m_pData_S[10] = 0x00;	// CPU 감시 Timer H

	m_pData_S[11] = 0x01;	// Command L
	m_pData_S[12] = 0x04;	// Command H

	m_pData_S[13] = 0x00;	// Sub Command L
	m_pData_S[14] = 0x00;	// Sub Command H

	// Start Address를 Hexa로
	//int nStartAddr_L = nAddr_Srt & 0xFF;
	//int nStartAddr_H = nAddr_Srt >> 8;

	m_pData_S[15] = (nAddr & 0xFF);//0xA8;	// 선두 Device L : 16진수 L25000 > L61A8
	m_pData_S[16] = (nAddr >> 8) & 0xFF;//0x61;	// 선두 Device M
	m_pData_S[17] = 0x00;	// 선두 Device H
	m_pData_S[18] = (char)0x92;	// Device Code(0x92)

	m_pData_S[19] = 0x01;	// nLength & 0xFF;	// Device Count L : 16진수
	m_pData_S[20] = 0x00;	// (nLength >> 8) & 0xFF;	// Device Count H

	//=====

	int bIsRet = 0;	
	int nValue = 0;
	stSendMsgPlc stData;
	stData.nDataLength = m_nLen_S;
	stData.pMCProtocolMsg = m_pData_S;
	stData.EventHandle = CreateEvent(0, FALSE, FALSE, 0);
	stData.nReturn = &bIsRet;
	stData.lpReadValue = &nValue;
	stData.nReadDataLength = 1;
	stData.nDatatype = 0;

	m_pSendData.Push(stData);

	m_pSemaMessage->Release(1);

	if (WaitForSingleObject(stData.EventHandle, INFINITE) == WAIT_OBJECT_0)
	{
		//처리 완료
		CloseHandle(stData.EventHandle);
	}

	if (bIsRet != 0) return false;

	if (nValue == 0) return false;
	else return true;
	
	//====================
	return bIsRet;
}
int CComPLC_RS::fnReadBitL(int nAddr_Srt, int nLength, short *pData1, int nReadSize)
{

	if (m_bConnected == FALSE) return 0;

	int nIndex = 0;
	int m_nLen_S = 21;

	char* m_pData_S = new char[25];
	ZeroMemory(m_pData_S, 25);

	m_pData_S[0] = 0x50;	// Sub Head L
	m_pData_S[1] = 0x00;	// Sub Head H
	m_pData_S[2] = 0x00;	// Network Number
	m_pData_S[3] = (char)0xFF;	// PLC Number
	m_pData_S[4] = (char)0xFF;	// 요구 상대 국번 IO Number H
	m_pData_S[5] = 0x03;	// 요구 상대 국번 IO Number L

	m_pData_S[6] = 0x00;	// 요구 상대 모듈 국번호
	m_pData_S[7] = 0x0C;	// 요구 Data 길이 L
	m_pData_S[8] = 0x00;	// 요구 Data 길이 R ( Data 길이 - 8 )

	m_pData_S[9] = 0x10;	// CPU 감시 Timer L
	m_pData_S[10] = 0x00;	// CPU 감시 Timer H

	m_pData_S[11] = 0x01;	// Command L
	m_pData_S[12] = 0x04;	// Command H

	m_pData_S[13] = 0x00;	// Sub Command L
	m_pData_S[14] = 0x00;	// Sub Command H

	// Start Address를 Hexa로
	//int nStartAddr_L = nAddr_Srt & 0xFF;
	//int nStartAddr_H = nAddr_Srt >> 8;

	m_pData_S[15] = (nAddr_Srt & 0xFF);//0xA8;	// 선두 Device L : 16진수 L25000 > L61A8
	m_pData_S[16] = (nAddr_Srt >> 8) & 0xFF;//0x61;	// 선두 Device M
	m_pData_S[17] = (nAddr_Srt >> 16) & 0xFF;//0x61;	// 선두 Device M
	//m_pData_S[17] = 0x00;	// 선두 Device H
	m_pData_S[18] = (char)0x92;	// Device Code(0x92)

	int nLen_L = nAddr_Srt & 0xFF;
	int nLen_H = nAddr_Srt >> 8;
	m_pData_S[19] = nLength & 0xFF;	// Device Count L : 16진수
	m_pData_S[20] = (nLength >> 8) & 0xFF;	// Device Count H

	//=====

	int bIsRet = false;
	stSendMsgPlc stData;
	stData.nDataLength = m_nLen_S;
	stData.pMCProtocolMsg = m_pData_S;
	stData.EventHandle = CreateEvent(0, FALSE, FALSE, 0);
	stData.nReturn = &bIsRet;
	stData.lpReadValue = pData1;
	stData.nReadDataLength = 2;
	stData.nDatatype = 1;

	m_pSendData.Push(stData);

	m_pSemaMessage->Release(1);

	if (WaitForSingleObject(stData.EventHandle, INFINITE) == WAIT_OBJECT_0)
	{
		//처리 완료
		CloseHandle(stData.EventHandle);
	}

	
	return 0;
}
int CComPLC_RS::fnReadBitL(int nAddr_Srt, int nLength, long *pData2, int nReadSize)
{

	if (m_bConnected == FALSE) return 0;

	int nIndex = 0;
	int m_nLen_S = 21;

	char* m_pData_S = new char[25];
	ZeroMemory(m_pData_S, 25);

	m_pData_S[0] = 0x50;	// Sub Head L
	m_pData_S[1] = 0x00;	// Sub Head H
	m_pData_S[2] = 0x00;	// Network Number
	m_pData_S[3] = (char)0xFF;	// PLC Number
	m_pData_S[4] = (char)0xFF;	// 요구 상대 국번 IO Number H
	m_pData_S[5] = 0x03;	// 요구 상대 국번 IO Number L

	m_pData_S[6] = 0x00;	// 요구 상대 모듈 국번호
	m_pData_S[7] = 0x0C;	// 요구 Data 길이 L
	m_pData_S[8] = 0x00;	// 요구 Data 길이 R ( Data 길이 - 8 )

	m_pData_S[9] = 0x10;	// CPU 감시 Timer L
	m_pData_S[10] = 0x00;	// CPU 감시 Timer H

	m_pData_S[11] = 0x01;	// Command L
	m_pData_S[12] = 0x04;	// Command H

	m_pData_S[13] = 0x00;	// Sub Command L
	m_pData_S[14] = 0x00;	// Sub Command H

	// Start Address를 Hexa로
	//int nStartAddr_L = nAddr_Srt & 0xFF;
	//int nStartAddr_H = nAddr_Srt >> 8;

	m_pData_S[15] = (nAddr_Srt & 0xFF);//0xA8;	// 선두 Device L : 16진수 L25000 > L61A8
	m_pData_S[16] = (nAddr_Srt >> 8) & 0xFF;//0x61;	// 선두 Device M
	m_pData_S[17] = (nAddr_Srt >> 16) & 0xFF;//0x61;	// 선두 Device H
	m_pData_S[18] = (char)0x92;	// Device Code(0x92)
	//ReadBit M영역으로 변경 0x90(M) 0x92(L) 2019.07.22	

	int nLen_L = nAddr_Srt & 0xFF;
	int nLen_H = nAddr_Srt >> 8;
	m_pData_S[19] = nLength & 0xFF;	// Device Count L : 16진수
	m_pData_S[20] = (nLength >> 8) & 0xFF;	// Device Count H

	//=====
	int bIsRet = false;
	stSendMsgPlc stData;
	stData.nDataLength = m_nLen_S;
	stData.pMCProtocolMsg = m_pData_S;
	stData.EventHandle = CreateEvent(0, FALSE, FALSE, 0);
	stData.nReturn = &bIsRet;
	stData.lpReadValue = pData2;
	stData.nReadDataLength = 2;	

	m_pSendData.Push(stData);

	m_pSemaMessage->Release(1);

	if (WaitForSingleObject(stData.EventHandle, INFINITE) == WAIT_OBJECT_0)
	{
		//처리 완료
		CloseHandle(stData.EventHandle);
	}
	
	return 0;
}
int CComPLC_RS::fnWriteBit(int nAddr, bool bOnOff)
{
	if (m_bConnected == FALSE) return 0;

	int nIndex = 0;
	//BYTE m_pData_S[25];
	char* m_pData_S = new char[25];
	ZeroMemory(m_pData_S, 25);

	m_pData_S[nIndex++] = 0x50;	// Sub Head L
	m_pData_S[nIndex++] = 0x00;	// Sub Head H
	m_pData_S[nIndex++] = 0x00;	// Network Number
	m_pData_S[nIndex++] = (char)0xFF;	// PLC Number
	m_pData_S[nIndex++] = (char)0xFF;	// 요구 상대 국번 IO Number H
	m_pData_S[nIndex++] = 0x03;	// 요구 상대 국번 IO Number L
	m_pData_S[nIndex++] = 0x00;	// 요구 상대 모듈 국번호
	nIndex += 2;
	//m_pData_S[7] = 0x0C;	// 요구 Data 길이 L
	//m_pData_S[8] = 0x00;	// 요구 Data 길이 R ( Data 길이 - 8 )

	m_pData_S[nIndex++] = 0x10;	// CPU 감시 Timer L
	m_pData_S[nIndex++] = 0x00;	// CPU 감시 Timer H

	m_pData_S[nIndex++] = 0x02;	// Command L
	m_pData_S[nIndex++] = 0x14;	// Command H

	m_pData_S[nIndex++] = 0x01;	// Sub Command L
	m_pData_S[nIndex++] = 0x00;	// Sub Command H

	m_pData_S[nIndex++] = 0x01;	// Bit 수
	m_pData_S[nIndex++] = (nAddr >> 0) & 0xFF;//0xA8;	// 선두 Device L : 16진수 L25000 > L61A8
	m_pData_S[nIndex++] = (nAddr >> 8) & 0xFF;//0x61;	// 선두 Device M
	m_pData_S[nIndex++] = (nAddr >> 16) & 0xFF;//선두 Device H
#ifdef L_AREA
	m_pData_S[nIndex++] = (char)0x92;
#else
	m_pData_S[nIndex++] = 0x90;	// Device Code(0x92) ?
#endif

	//ReadBit M영역으로 변경 0x90(M) 0x92(L) 2019.07.22	

	
	m_pData_S[nIndex++] = bOnOff ? 1 : 0;	// On/Off Value

	int m_nLen_S = nIndex;
	nIndex -= 9;
	m_pData_S[7] = (nIndex >> 0) & 0xFF;	// 요구 Data 길이 L
	m_pData_S[8] = (nIndex >> 8) & 0xFF;	// 요구 Data 길이 R ( Data 길이 - 8 )


	//=====
	int bIsRet = false;
	stSendMsgPlc stData;
	stData.nDataLength = m_nLen_S;
	stData.pMCProtocolMsg = m_pData_S;
	stData.EventHandle = CreateEvent(0, FALSE, FALSE, 0);
	stData.nReturn = &bIsRet;
	stData.lpReadValue = NULL;
	stData.nReadDataLength = 0;

	m_pSendData.Push(stData);

	m_pSemaMessage->Release(1);

	if (WaitForSingleObject(stData.EventHandle, INFINITE) == WAIT_OBJECT_0)
	{
		//처리 완료
		CloseHandle(stData.EventHandle);
	}
	
	return 0;
}
int CComPLC_RS::fnWriteBitL(int nAddr, bool bOnOff)
{
	if (m_bConnected == FALSE) return 0;

	int nIndex = 0;

	char* m_pData_S = new char[25];
	ZeroMemory(m_pData_S, 25);

	m_pData_S[nIndex++] = 0x50;	// Sub Head L
	m_pData_S[nIndex++] = 0x00;	// Sub Head H
	m_pData_S[nIndex++] = 0x00;	// Network Number
	m_pData_S[nIndex++] = (char)0xFF;	// PLC Number
	m_pData_S[nIndex++] = (char)0xFF;	// 요구 상대 국번 IO Number H
	m_pData_S[nIndex++] = 0x03;	// 요구 상대 국번 IO Number L
	m_pData_S[nIndex++] = 0x00;	// 요구 상대 모듈 국번호
	nIndex += 2;
	//m_pData_S[7] = 0x0C;	// 요구 Data 길이 L
	//m_pData_S[8] = 0x00;	// 요구 Data 길이 R ( Data 길이 - 8 )

	m_pData_S[nIndex++] = 0x10;	// CPU 감시 Timer L
	m_pData_S[nIndex++] = 0x00;	// CPU 감시 Timer H

	m_pData_S[nIndex++] = 0x02;	// Command L
	m_pData_S[nIndex++] = 0x14;	// Command H

	m_pData_S[nIndex++] = 0x01;	// Sub Command L
	m_pData_S[nIndex++] = 0x00;	// Sub Command H

	m_pData_S[nIndex++] = 0x01;	// Bit 수
	m_pData_S[nIndex++] = (nAddr >> 0) & 0xFF;//0xA8;	// 선두 Device L : 16진수 L25000 > L61A8
	m_pData_S[nIndex++] = (nAddr >> 8) & 0xFF;//0x61;	// 선두 Device M
	m_pData_S[nIndex++] = (nAddr >> 16) & 0xFF;//선두 Device H
	m_pData_S[nIndex++] = (char)0x92;	// Device Code(0x92) ?
	m_pData_S[nIndex++] = bOnOff ? 1 : 0;	// On/Off Value

	//ReadBit M영역으로 변경 0x90(M) 0x92(L) 2019.07.22	

	int m_nLen_S = nIndex;
	nIndex -= 9;
	m_pData_S[7] = (nIndex >> 0) & 0xFF;	// 요구 Data 길이 L
	m_pData_S[8] = (nIndex >> 8) & 0xFF;	// 요구 Data 길이 R ( Data 길이 - 8 )

	//=====
	int bIsRet = false;
	stSendMsgPlc stData;
	stData.nDataLength = m_nLen_S;
	stData.pMCProtocolMsg = m_pData_S;
	stData.EventHandle = CreateEvent(0, FALSE, FALSE, 0);
	stData.nReturn = &bIsRet;
	//stData.lpReadValue = pData2;
	//stData.nReadDataLength = 2;

	m_pSendData.Push(stData);

	m_pSemaMessage->Release(1);

	if (WaitForSingleObject(stData.EventHandle, INFINITE) == WAIT_OBJECT_0)
	{
		//처리 완료
		CloseHandle(stData.EventHandle);
	}

	
	return 0;
}
int CComPLC_RS::fnWriteBitM(int nAddr, bool bOnOff)
{
	if (m_bConnected == FALSE) return 0;

	int nIndex = 0;

	char* m_pData_S = new char[25];
	ZeroMemory(m_pData_S, 25);

	m_pData_S[nIndex++] = 0x50;	// Sub Head L
	m_pData_S[nIndex++] = 0x00;	// Sub Head H
	m_pData_S[nIndex++] = 0x00;	// Network Number
	m_pData_S[nIndex++] = (char)0xFF;	// PLC Number
	m_pData_S[nIndex++] = (char)0xFF;	// 요구 상대 국번 IO Number H
	m_pData_S[nIndex++] = 0x03;	// 요구 상대 국번 IO Number L
	m_pData_S[nIndex++] = 0x00;	// 요구 상대 모듈 국번호
	nIndex += 2;
	//m_pData_S[7] = 0x0C;	// 요구 Data 길이 L
	//m_pData_S[8] = 0x00;	// 요구 Data 길이 R ( Data 길이 - 8 )

	m_pData_S[nIndex++] = 0x10;	// CPU 감시 Timer L
	m_pData_S[nIndex++] = 0x00;	// CPU 감시 Timer H

	m_pData_S[nIndex++] = 0x02;	// Command L
	m_pData_S[nIndex++] = 0x14;	// Command H

	m_pData_S[nIndex++] = 0x01;	// Sub Command L
	m_pData_S[nIndex++] = 0x00;	// Sub Command H

	m_pData_S[nIndex++] = 0x01;	// Bit 수
	m_pData_S[nIndex++] = (nAddr >> 0) & 0xFF;//0xA8;	// 선두 Device L : 16진수 L25000 > L61A8
	m_pData_S[nIndex++] = (nAddr >> 8) & 0xFF;//0x61;	// 선두 Device M
	m_pData_S[nIndex++] = (nAddr >> 16) & 0xFF;//선두 Device H
	m_pData_S[nIndex++] = (char)0x90;	// Device Code(0x92) ?
	m_pData_S[nIndex++] = bOnOff ? 1 : 0;	// On/Off Value

	//ReadBit M영역으로 변경 0x90(M) 0x92(L) 2019.07.22	

	int m_nLen_S = nIndex;
	nIndex -= 9;
	m_pData_S[7] = (nIndex >> 0) & 0xFF;	// 요구 Data 길이 L
	m_pData_S[8] = (nIndex >> 8) & 0xFF;	// 요구 Data 길이 R ( Data 길이 - 8 )

	//=====
	int bIsRet = false;
	stSendMsgPlc stData;
	stData.nDataLength = m_nLen_S;
	stData.pMCProtocolMsg = m_pData_S;
	stData.EventHandle = CreateEvent(0, FALSE, FALSE, 0);
	stData.nReturn = &bIsRet;
	//stData.lpReadValue = NULL;
	//stData.nReadDataLength = 0;

	m_pSendData.Push(stData);

	m_pSemaMessage->Release(1);

	if (WaitForSingleObject(stData.EventHandle, INFINITE) == WAIT_OBJECT_0)
	{
		//처리 완료
		CloseHandle(stData.EventHandle);
	}
		
	return 0;
}
int CComPLC_RS::fnReadWord(int nAddr_Srt, int nLength, short *pData3, int nReadSize)
{
	if (m_bConnected == FALSE) return 0;

	int nIndex = 0;
	int m_nLen_S = 21;

	char* m_pData_S = new char[25];
	ZeroMemory(m_pData_S, 25);

	m_pData_S[0] = 0x50;	// Sub Head L
	m_pData_S[1] = 0x00;	// Sub Head H
	m_pData_S[2] = 0x00;	// Network Number
	m_pData_S[3] = (char)0xFF;	// PLC Number
	m_pData_S[4] = (char)0xFF;	// 요구 상대 국번 IO Number H
	m_pData_S[5] = 0x03;	// 요구 상대 국번 IO Number L
	m_pData_S[6] = 0x00;	// 요구 상대 모듈 국번호
	m_pData_S[7] = 0x0C;	// 요구 Data 길이 L
	m_pData_S[8] = 0x00;	// 요구 Data 길이 R ( Data 길이 - 8 )
	m_pData_S[9] = 0x10;	// CPU 감시 Timer L
	m_pData_S[10] = 0x00;	// CPU 감시 Timer H
	m_pData_S[11] = 0x01;	// Command L
	m_pData_S[12] = 0x04;	// Command H
	m_pData_S[13] = 0x00;	// Sub Command L
	m_pData_S[14] = 0x00;	// Sub Command H
	// Start Address를 Hexa로
	m_pData_S[15] = (nAddr_Srt & 0xFF);//0xA8;	// 선두 Device L : 16진수 L25000 > L61A8
	m_pData_S[16] = (nAddr_Srt >> 8) & 0xFF;//0x61;	// 선두 Device M
	m_pData_S[17] = 0x00;	// 선두 Device H
	m_pData_S[18] = (char)0xAF;	// Device Code
	m_pData_S[19] = nLength & 0xFF;	// Device Count L : 16진수
	m_pData_S[20] = (nLength >> 8) & 0xFF;	// Device Count H

	//=====
	int bIsRet = false;
	stSendMsgPlc stData;
	stData.nDataLength = m_nLen_S;
	stData.pMCProtocolMsg = m_pData_S;
	stData.EventHandle = CreateEvent(0, FALSE, FALSE, 0);
	stData.nReturn = &bIsRet;
	stData.lpReadValue = pData3;
	stData.nReadDataLength = 2;
	stData.nDatatype = 1;

	m_pSendData.Push(stData);

	m_pSemaMessage->Release(1);

	if (WaitForSingleObject(stData.EventHandle, INFINITE) == WAIT_OBJECT_0)
	{
		//처리 완료
		CloseHandle(stData.EventHandle);
	}
	
	return 0;
}
int CComPLC_RS::fnReadWordD(int nAddr_Srt, int nLength, short *pData3, int nReadSize)
{
	return 0;

	if (m_bConnected == FALSE) return 0;

	int nIndex = 0;
	int m_nLen_S = 21;
	BYTE m_pData_S[25];

	m_pData_S[0] = 0x50;	// Sub Head L
	m_pData_S[1] = 0x00;	// Sub Head H
	m_pData_S[2] = 0x00;	// Network Number
	m_pData_S[3] = 0xFF;	// PLC Number
	m_pData_S[4] = 0xFF;	// 요구 상대 국번 IO Number H
	m_pData_S[5] = 0x03;	// 요구 상대 국번 IO Number L
	m_pData_S[6] = 0x00;	// 요구 상대 모듈 국번호
	m_pData_S[7] = 0x0C;	// 요구 Data 길이 L
	m_pData_S[8] = 0x00;	// 요구 Data 길이 R ( Data 길이 - 8 )
	m_pData_S[9] = 0x10;	// CPU 감시 Timer L
	m_pData_S[10] = 0x00;	// CPU 감시 Timer H
	m_pData_S[11] = 0x01;	// Command L
	m_pData_S[12] = 0x04;	// Command H
	m_pData_S[13] = 0x00;	// Sub Command L
	m_pData_S[14] = 0x00;	// Sub Command H
	// Start Address를 Hexa로
	m_pData_S[15] = (nAddr_Srt & 0xFF);//0xA8;	// 선두 Device L : 16진수 L25000 > L61A8
	m_pData_S[16] = (nAddr_Srt >> 8) & 0xFF;//0x61;	// 선두 Device M
	m_pData_S[17] = (nAddr_Srt >> 16) & 0xFF;	// 선두 Device H
	m_pData_S[18] = 0xA8;	// Device Code
	m_pData_S[19] = nLength & 0xFF;	// Device Count L : 16진수
	m_pData_S[20] = (nLength >> 8) & 0xFF;	// Device Count H

	ResetEvent(m_hWowEvent);
	fn_SendCommandData((char*)m_pData_S, m_nLen_S);
	///////////////////////////////////////////////////////////////////////////////////////
	int nRet = WaitForSingleObject(m_hWowEvent, 2000);
	if (nRet == WAIT_TIMEOUT)
	{
		CString strTemp;
		int nDataCount = (int)(*(m_pReceiveBuffer + 7)) + (int)((*(m_pReceiveBuffer + 8)) << 8) - 2;

		strTemp.Format("fnReadWord1 - Timeout : %d", nDataCount);
		//if (m_pMain)	::SendMessageA(m_pMain->m_pPane[PANE_MANUAL]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);
	}
	else
	{
		int nDataCount = (int)(*(m_pReceiveBuffer + 7)) + (int)((*(m_pReceiveBuffer + 8)) << 8) - 2;
		BYTE *pBuffer = m_pReceiveBuffer + 11;

		if (nLength != (nDataCount / 2))
		{
			CString strTemp;
			strTemp.Format("nAddr3 : D%d, nLength : %d, nCount : %d", nAddr_Srt, nLength, nDataCount);
			//::SendMessageA(m_pMain->m_pPane[PANE_MANUAL]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);
		}

		for (int ni = 0; ni < nLength / 2; ni++) //20180930 수정
			pData3[ni] = pBuffer[ni * 2] | pBuffer[ni * 2 + 1] << 8;
	}


	return 0;
}
int CComPLC_RS::fnReadWord(int nAddr_Srt, int nLength, long *pData4, int nReadSize)
{
	if (m_bConnected == FALSE) return 0;

	int nIndex = 0;
	int m_nLen_S = 21;

	char* m_pData_S = new char[25];
	ZeroMemory(m_pData_S, 25);

	m_pData_S[0] = 0x50;	// Sub Head L
	m_pData_S[1] = 0x00;	// Sub Head H
	m_pData_S[2] = 0x00;	// Network Number
	m_pData_S[3] = (char)0xFF;	// PLC Number
	m_pData_S[4] = (char)0xFF;	// 요구 상대 국번 IO Number H
	m_pData_S[5] = 0x03;	// 요구 상대 국번 IO Number L
	m_pData_S[6] = 0x00;	// 요구 상대 모듈 국번호
	m_pData_S[7] = 0x0C;	// 요구 Data 길이 L
	m_pData_S[8] = 0x00;	// 요구 Data 길이 R ( Data 길이 - 8 )
	m_pData_S[9] = 0x10;	// CPU 감시 Timer L
	m_pData_S[10] = 0x00;	// CPU 감시 Timer H
	m_pData_S[11] = 0x01;	// Command L
	m_pData_S[12] = 0x04;	// Command H
	m_pData_S[13] = 0x00;	// Sub Command L
	m_pData_S[14] = 0x00;	// Sub Command H
	// Start Address를 Hexa로
	m_pData_S[15] = (nAddr_Srt & 0xFF);//0xA8;	// 선두 Device L : 16진수 L25000 > L61A8
	m_pData_S[16] = (nAddr_Srt >> 8) & 0xFF;//0x61;	// 선두 Device M
	m_pData_S[17] = 0x00;	// 선두 Device H
	m_pData_S[18] = (char)0xAF;	// Device Code
	m_pData_S[19] = nLength & 0xFF;	// Device Count L : 16진수
	m_pData_S[20] = (nLength >> 8) & 0xFF;	// Device Count H

	//=====
	int bIsRet = false;
	stSendMsgPlc stData;
	stData.nDataLength = m_nLen_S;
	stData.pMCProtocolMsg = m_pData_S;
	stData.EventHandle = CreateEvent(0, FALSE, FALSE, 0);
	stData.nReturn = &bIsRet;
	stData.lpReadValue = pData4;
	stData.nReadDataLength = nLength;

	m_pSendData.Push(stData);

	m_pSemaMessage->Release(1);

	if (WaitForSingleObject(stData.EventHandle, INFINITE) == WAIT_OBJECT_0)
	{
		//처리 완료
		CloseHandle(stData.EventHandle);
	}
		
	return 0;
}
int CComPLC_RS::fnReadWordD(int nAddr_Srt, int nLength, long *pData4, int nReadSize)
{
	return 0;

	if (m_bConnected == FALSE) return 0;

	int nIndex = 0;
	int m_nLen_S = 21;
	BYTE m_pData_S[25] = { 0, };

	m_pData_S[0] = 0x50;	// Sub Head L
	m_pData_S[1] = 0x00;	// Sub Head H
	m_pData_S[2] = 0x00;	// Network Number
	m_pData_S[3] = 0xFF;	// PLC Number
	m_pData_S[4] = 0xFF;	// 요구 상대 국번 IO Number H
	m_pData_S[5] = 0x03;	// 요구 상대 국번 IO Number L
	m_pData_S[6] = 0x00;	// 요구 상대 모듈 국번호
	m_pData_S[7] = 0x0C;	// 요구 Data 길이 L
	m_pData_S[8] = 0x00;	// 요구 Data 길이 R ( Data 길이 - 8 )
	m_pData_S[9] = 0x10;	// CPU 감시 Timer L
	m_pData_S[10] = 0x00;	// CPU 감시 Timer H
	m_pData_S[11] = 0x01;	// Command L
	m_pData_S[12] = 0x04;	// Command H
	m_pData_S[13] = 0x00;	// Sub Command L
	m_pData_S[14] = 0x00;	// Sub Command H
	// Start Address를 Hexa로
	m_pData_S[15] = (nAddr_Srt & 0xFF);//0xA8;	// 선두 Device L : 16진수 L25000 > L61A8
	m_pData_S[16] = (nAddr_Srt >> 8) & 0xFF;//0x61;	// 선두 Device M
	m_pData_S[17] = (nAddr_Srt >> 16) & 0xFF;	// 선두 Device H
	m_pData_S[18] = 0xA8;	// Device Code
	m_pData_S[19] = nLength & 0xFF;	// Device Count L : 16진수
	m_pData_S[20] = (nLength >> 8) & 0xFF;	// Device Count H

	ResetEvent(m_hWowEvent);
	fn_SendCommandData((char*)m_pData_S, m_nLen_S);
	///////////////////////////////////////////////////////////////////////////////////////
	int nRet = WaitForSingleObject(m_hWowEvent, 2000);
	if (nRet == WAIT_TIMEOUT)
	{
		CString strTemp;
		int nDataCount = (int)(*(m_pReceiveBuffer + 7)) + (int)((*(m_pReceiveBuffer + 8)) << 8) - 2;

		strTemp.Format("fnReadWord2 - Timeout : %d", nDataCount);
		//if (m_pMain)	::SendMessageA(m_pMain->m_pPane[PANE_MANUAL]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);
	}
	else
	{
		int nDataCount = (int)(*(m_pReceiveBuffer + 7)) + (int)((*(m_pReceiveBuffer + 8)) << 8) - 2;

		BYTE *pBuffer = m_pReceiveBuffer + 11;

		if (nLength != (nDataCount / 2))
		{
			CString strTemp;
			strTemp.Format("nAddr4 : D%d, nLength : %d, nCount : %d", nAddr_Srt, nLength, nDataCount);
			//::SendMessageA(m_pMain->m_pPane[PANE_MANUAL]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);
		}

		short *lData = new short[nDataCount / 2];

		for (int ni = 0; ni < nDataCount / 2; ni++)
		{
			*(lData + ni) = *(m_pReceiveBuffer + 11 + (ni * 2)) + ((*(m_pReceiveBuffer + 11 + (ni * 2) + 1)) << 8);
			*(pData4 + ni) = *(lData + ni);
		}

		delete[]lData;
	}


	return 0;
}
int CComPLC_RS::fnWriteWord(int nAddr, int nLength, short *pData5)
{
	if (m_bConnected == FALSE) return 0;
	if (nLength > 1024) return 0;


	int nIndex = 0;
	int ni;

	char* m_pData_S = new char[1024];
	ZeroMemory(m_pData_S, 1024);

	m_pData_S[nIndex++] = 0x50;	// Sub Head L
	m_pData_S[nIndex++] = 0x00;	// Sub Head H
	m_pData_S[nIndex++] = 0x00;	// Network Number
	m_pData_S[nIndex++] = (char)0xFF;	// PLC Number
	m_pData_S[nIndex++] = (char)0xFF;	// 요구 상대 국번 IO Number H
	m_pData_S[nIndex++] = 0x03;	// 요구 상대 국번 IO Number L
	m_pData_S[nIndex++] = 0x00;	// 요구 상대 모듈 국번호
	nIndex += 2;
	//m_pData_S[7] = 0x0C;	// 요구 Data 길이 L
	//m_pData_S[8] = 0x00;	// 요구 Data 길이 R ( Data 길이 - 8 )

	m_pData_S[nIndex++] = 0x10;	// CPU 감시 Timer L
	m_pData_S[nIndex++] = 0x00;	// CPU 감시 Timer H

	m_pData_S[nIndex++] = 0x01;	// Command L
	m_pData_S[nIndex++] = 0x14;	// Command H

	m_pData_S[nIndex++] = 0x00;	// Sub Command L
	m_pData_S[nIndex++] = 0x00;	// Sub Command H

	m_pData_S[nIndex++] = (nAddr >> 0) & 0xFF;//0xA8;	// 선두 Device L : 16진수 L25000 > L61A8
	m_pData_S[nIndex++] = (nAddr >> 8) & 0xFF;//0x61;	// 선두 Device M
	m_pData_S[nIndex++] = (nAddr >> 16) & 0xFF;//선두 Device H
	m_pData_S[nIndex++] = (char)0xA8;	// Device Code(0x92) ?
	m_pData_S[nIndex++] = (nLength >> 0) & 0xFF;	// Device Count L
	m_pData_S[nIndex++] = (nLength >> 8) & 0xFF;	// Device Count H

	for (ni = 0; ni < nLength; ni++)
	{
		m_pData_S[nIndex++] = (*(pData5 + ni) >> 0) & 0x00FF;	// Data L
		m_pData_S[nIndex++] = (*(pData5 + ni) >> 8) & 0x00FF;	// Data H
	}

	//memcpy(&m_pData_S[nIndex],pData,sizeof(long)*nLength);

	int m_nLen_S = nIndex;

	nIndex -= 9;
	m_pData_S[7] = (nIndex >> 0) & 0xFF;	// 요구 Data 길이 L
	m_pData_S[8] = (nIndex >> 8) & 0xFF;	// 요구 Data 길이 R ( Data 길이 - 8 )

	//=====
	int bIsRet = false;
	stSendMsgPlc stData;
	stData.nDataLength = m_nLen_S;
	stData.pMCProtocolMsg = m_pData_S;
	stData.EventHandle = CreateEvent(0, FALSE, FALSE, 0);
	stData.nReturn = &bIsRet;

	m_pSendData.Push(stData);

	m_pSemaMessage->Release(1);

	if (WaitForSingleObject(stData.EventHandle, INFINITE) == WAIT_OBJECT_0)
	{
		//처리 완료
		CloseHandle(stData.EventHandle);
	}

	return 0;
}
int CComPLC_RS::fnWriteWord(int nAddr, int nLength, long *pData6)
{
	if (m_bConnected == FALSE) return 0;
	if (nLength > 1024) return 0;

	int nIndex = 0;
	int ni;

	char* m_pData_S = new char[1024];
	ZeroMemory(m_pData_S, 1024);

	m_pData_S[nIndex++] = 0x50;	// Sub Head L
	m_pData_S[nIndex++] = 0x00;	// Sub Head H
	m_pData_S[nIndex++] = 0x00;	// Network Number
	m_pData_S[nIndex++] = (char)0xFF;	// PLC Number
	m_pData_S[nIndex++] = (char)0xFF;	// 요구 상대 국번 IO Number H
	m_pData_S[nIndex++] = 0x03;	// 요구 상대 국번 IO Number L
	m_pData_S[nIndex++] = 0x00;	// 요구 상대 모듈 국번호
	nIndex += 2;
	//m_pData_S[7] = 0x0C;	// 요구 Data 길이 L
	//m_pData_S[8] = 0x00;	// 요구 Data 길이 R ( Data 길이 - 8 )

	m_pData_S[nIndex++] = 0x10;	// CPU 감시 Timer L
	m_pData_S[nIndex++] = 0x00;	// CPU 감시 Timer H

	m_pData_S[nIndex++] = 0x01;	// Command L
	m_pData_S[nIndex++] = 0x14;	// Command H

	m_pData_S[nIndex++] = 0x00;	// Sub Command L
	m_pData_S[nIndex++] = 0x00;	// Sub Command H

	m_pData_S[nIndex++] = (nAddr >> 0) & 0xFF;//0xA8;	// 선두 Device L : 16진수 L25000 > L61A8
	m_pData_S[nIndex++] = (nAddr >> 8) & 0xFF;//0x61;	// 선두 Device M
	m_pData_S[nIndex++] = (nAddr >> 16) & 0xFF;//선두 Device H
	m_pData_S[nIndex++] = (char)0xA8;	// Device Code(0x92) ?
	m_pData_S[nIndex++] = (nLength >> 0) & 0xFF;	// Device Count L
	m_pData_S[nIndex++] = (nLength >> 8) & 0xFF;	// Device Count H

	for (ni = 0; ni < nLength; ni++)
	{
		m_pData_S[nIndex++] = (*(pData6 + ni) >> 0) & 0xFF;	// Data L
		m_pData_S[nIndex++] = (*(pData6 + ni) >> 8) & 0xFF;	// Data H
	}

	//memcpy(&m_pData_S[nIndex],pData,sizeof(long)*nLength);

	int m_nLen_S = nIndex;

	nIndex -= 9;
	m_pData_S[7] = (nIndex >> 0) & 0xFF;	// 요구 Data 길이 L
	m_pData_S[8] = (nIndex >> 8) & 0xFF;	// 요구 Data 길이 R ( Data 길이 - 8 )

	//=====
	int bIsRet = false;
	stSendMsgPlc stData;
	stData.nDataLength = m_nLen_S;
	stData.pMCProtocolMsg = m_pData_S;
	stData.EventHandle = CreateEvent(0, FALSE, FALSE, 0);
	stData.nReturn = &bIsRet;

	m_pSendData.Push(stData);

	m_pSemaMessage->Release(1);

	if (WaitForSingleObject(stData.EventHandle, INFINITE) == WAIT_OBJECT_0)
	{
		//처리 완료
		CloseHandle(stData.EventHandle);
	}

	return 0;
}
int CComPLC_RS::fnReadWord32(int nAddr_Srt, int nLength, int *pData8, int nReadSize)
{
	return 0;

	if (m_bConnected == FALSE) return 0;

	EnterCriticalSection(&m_csComm);

	int nIndex = 0;
	int nDataLen = (nLength * 2);
	int m_nLen_S = 21;
	BYTE m_pData_S[25];

	m_pData_S[0] = 0x50;	// Sub Head L
	m_pData_S[1] = 0x00;	// Sub Head H
	m_pData_S[2] = 0x00;	// Network Number
	m_pData_S[3] = 0xFF;	// PLC Number
	m_pData_S[4] = 0xFF;	// 요구 상대 국번 IO Number H
	m_pData_S[5] = 0x03;	// 요구 상대 국번 IO Number L
	m_pData_S[6] = 0x00;	// 요구 상대 모듈 국번호
	m_pData_S[7] = 0x0C;	// 요구 Data 길이 L
	m_pData_S[8] = 0x00;	// 요구 Data 길이 R ( Data 길이 - 8 )
	m_pData_S[9] = 0x10;	// CPU 감시 Timer L
	m_pData_S[10] = 0x00;	// CPU 감시 Timer H
	m_pData_S[11] = 0x01;	// Command L
	m_pData_S[12] = 0x04;	// Command H
	m_pData_S[13] = 0x00;	// Sub Command L
	m_pData_S[14] = 0x00;	// Sub Command H
	// Start Address를 Hexa로
	m_pData_S[15] = (nAddr_Srt & 0xFF);//0xA8;	// 선두 Device L : 16진수 L25000 > L61A8
	m_pData_S[16] = (nAddr_Srt >> 8) & 0xFF;//0x61;	// 선두 Device M
	m_pData_S[17] = 0x00;	// 선두 Device H
	m_pData_S[18] = 0xA8;	// Device Code
	m_pData_S[19] = (nDataLen >> 0) & 0xFF;	// Device Count L : 16진수
	m_pData_S[20] = (nDataLen >> 8) & 0xFF;	// Device Count H


	ResetEvent(m_hWowEvent);
	fn_SendCommandData((char*)m_pData_S, m_nLen_S);
	///////////////////////////////////////////////////////////////////////////////////////
	int nRet = WaitForSingleObject(m_hWowEvent, 2000);
	if (nRet == WAIT_TIMEOUT)
	{
		CString strTemp;
		int nDataCount = (int)(*(m_pReceiveBuffer + 7)) + (int)((*(m_pReceiveBuffer + 8)) << 8) - 2;
		strTemp.Format("fnReadWord32 - Timeout : %d", nDataCount);
		//if (m_pMain)	::SendMessageA(m_pMain->m_pPane[PANE_MANUAL]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);
	}
	else
	{
		int nDataCount = (int)(*(m_pReceiveBuffer + 7)) + (int)((*(m_pReceiveBuffer + 8)) << 8) - 2;	// BYTE 단위의 개수
		for (int ni = 0; ni < nDataCount; ni += 4)
		{
			*(pData8 + (ni >> 2)) = *(m_pReceiveBuffer + 11 + ni) + ((*(m_pReceiveBuffer + 11 + ni + 1)) << 8) + ((*(m_pReceiveBuffer + 11 + ni + 2)) << 16) + ((*(m_pReceiveBuffer + 11 + ni + 3)) << 24);
		}
	}

	LeaveCriticalSection(&m_csComm);

	return 0;
}
int CComPLC_RS::fnWriteWord32(int nAddr, int nLength, int *pData9)
{
	return 0;

	if (m_bConnected == FALSE) return 0;
	if (nLength > 512) return 0;

	EnterCriticalSection(&m_csComm);

	short pData16[2048] = { 0, };

	for (int ni = 0; ni < nLength; ni++)
	{
		pData16[ni] = (short)((pData9[ni] >> 0) & 0xFFFF);
		//pData16[ni * 2 + 1] = (short)((pData[ni] >> 16) & 0xFFFF);
	}

	int nError = fnWriteWord(nAddr, nLength, pData16);


	LeaveCriticalSection(&m_csComm);

	return nError;
}

//M영역 리딩 추가 Kim.Jong.Ho 2019-07-22
int CComPLC_RS::fnReadBitM(int nAddr_Srt, int nLength, short *pData10, int nReadSize)
{

	if (m_bConnected == FALSE) return 0;

	//EnterCriticalSection(&m_csComm);

	int nIndex = 0;
	int m_nLen_S = 21;
	//BYTE m_pData_S[25];
	char* m_pData_S = new char[25];
	ZeroMemory(m_pData_S, 25);

	m_pData_S[0] = 0x50;	// Sub Head L
	m_pData_S[1] = 0x00;	// Sub Head H
	m_pData_S[2] = 0x00;	// Network Number
	m_pData_S[3] = (char)0xFF;	// PLC Number
	m_pData_S[4] = (char)0xFF;	// 요구 상대 국번 IO Number H
	m_pData_S[5] = 0x03;	// 요구 상대 국번 IO Number L

	m_pData_S[6] = 0x00;	// 요구 상대 모듈 국번호
	m_pData_S[7] = 0x0C;	// 요구 Data 길이 L
	m_pData_S[8] = 0x00;	// 요구 Data 길이 R ( Data 길이 - 8 )

	m_pData_S[9] = 0x10;	// CPU 감시 Timer L
	m_pData_S[10] = 0x00;	// CPU 감시 Timer H

	m_pData_S[11] = 0x01;	// Command L
	m_pData_S[12] = 0x04;	// Command H

	m_pData_S[13] = 0x00;	// Sub Command L
	m_pData_S[14] = 0x00;	// Sub Command H

	// Start Address를 Hexa로
	//int nStartAddr_L = nAddr_Srt & 0xFF;
	//int nStartAddr_H = nAddr_Srt >> 8;

	m_pData_S[15] = (nAddr_Srt & 0xFF);//0xA8;	// 선두 Device L : 16진수 L25000 > L61A8
	m_pData_S[16] = (nAddr_Srt >> 8) & 0xFF;//0x61;	// 선두 Device M
	m_pData_S[16] = (nAddr_Srt >> 16) & 0xFF;//0x61;	// 선두 Device M
	//m_pData_S[17] = 0x00;	// 선두 Device H
	//ReadBit M영역으로 변경 0x90(M) 0x92(L) 2019.07.22	
	m_pData_S[18] = (char)0x90;	// Device Code(0x92)

	int nLen_L = nAddr_Srt & 0xFF;
	int nLen_H = nAddr_Srt >> 8;
	m_pData_S[19] = nLength & 0xFF;	// Device Count L : 16진수
	m_pData_S[20] = (nLength >> 8) & 0xFF;	// Device Count H

	//=====
	int bIsRet = false;
	stSendMsgPlc stData;
	stData.nDataLength = m_nLen_S;
	stData.pMCProtocolMsg = m_pData_S;
	stData.EventHandle = CreateEvent(0, FALSE, FALSE, 0);
	stData.nReturn = &bIsRet;
	stData.nDatatype = 1;
	stData.lpReadValue = pData10;
	stData.nReadDataLength = nLength;

	m_pSendData.Push(stData);

	m_pSemaMessage->Release(1);

	if (WaitForSingleObject(stData.EventHandle, INFINITE) == WAIT_OBJECT_0)
	{
		//처리 완료
		CloseHandle(stData.EventHandle);
	}

	return 0;
}
int CComPLC_RS::fnReadBitM(int nAddr_Srt, int nLength, long *pData11, int nReadSize)
{
	if (m_bConnected == FALSE) return 0;

	//EnterCriticalSection(&m_csComm);

	int nIndex = 0;
	int m_nLen_S = 21;
	//BYTE m_pData_S[25];
	char* m_pData_S = new char[25];
	ZeroMemory(m_pData_S, 25);

	m_pData_S[0] = 0x50;	// Sub Head L
	m_pData_S[1] = 0x00;	// Sub Head H
	m_pData_S[2] = 0x00;	// Network Number
	m_pData_S[3] = (char)0xFF;	// PLC Number
	m_pData_S[4] = (char)0xFF;	// 요구 상대 국번 IO Number H
	m_pData_S[5] = 0x03;	// 요구 상대 국번 IO Number L

	m_pData_S[6] = 0x00;	// 요구 상대 모듈 국번호
	m_pData_S[7] = 0x0C;	// 요구 Data 길이 L
	m_pData_S[8] = 0x00;	// 요구 Data 길이 R ( Data 길이 - 8 )

	m_pData_S[9] = 0x10;	// CPU 감시 Timer L
	m_pData_S[10] = 0x00;	// CPU 감시 Timer H

	m_pData_S[11] = 0x01;	// Command L
	m_pData_S[12] = 0x04;	// Command H

	m_pData_S[13] = 0x00;	// Sub Command L
	m_pData_S[14] = 0x00;	// Sub Command H

	// Start Address를 Hexa로
	//int nStartAddr_L = nAddr_Srt & 0xFF;
	//int nStartAddr_H = nAddr_Srt >> 8;

	m_pData_S[15] = (nAddr_Srt & 0xFF);//0xA8;	// 선두 Device L : 16진수 L25000 > L61A8
	m_pData_S[16] = (nAddr_Srt >> 8) & 0xFF;//0x61;	// 선두 Device M
	m_pData_S[17] = 0x00;	// 선두 Device H
	//ReadBit M영역으로 변경 0x90(M) 0x92(L) 2019.07.22	
	m_pData_S[18] = (char)0x90;	// Device Code(0x92)

	int nLen_L = nAddr_Srt & 0xFF;
	int nLen_H = nAddr_Srt >> 8;
	m_pData_S[19] = nLength & 0xFF;	// Device Count L : 16진수
	m_pData_S[20] = (nLength >> 8) & 0xFF;	// Device Count H

	//=====
	int bIsRet = false;
	stSendMsgPlc stData;
	stData.nDataLength = m_nLen_S;
	stData.pMCProtocolMsg = m_pData_S;
	stData.EventHandle = CreateEvent(0, FALSE, FALSE, 0);
	stData.nReturn = &bIsRet;
	stData.nDatatype = 0;
	stData.lpReadValue = pData11;
	stData.nReadDataLength = nLength;

	m_pSendData.Push(stData);

	m_pSemaMessage->Release(1);

	if (WaitForSingleObject(stData.EventHandle, INFINITE) == WAIT_OBJECT_0)
	{
		//처리 완료
		CloseHandle(stData.EventHandle);
	}
		
	return 0;
}

int CComPLC_RS::fnReadWordZR(int nAddr_Srt, int nLength, short* pData7, int nReadSize)
{
	if (m_bConnected == FALSE) return 0;

	int nIndex = 0;
	int m_nLen_S = 21;

	char* m_pData_S = new char[25];
	ZeroMemory(m_pData_S, 25);

	m_pData_S[0] = 0x50;	// Sub Head L
	m_pData_S[1] = 0x00;	// Sub Head H
	m_pData_S[2] = 0x00;	// Network Number
	m_pData_S[3] = (char)0xFF;	// PLC Number
	m_pData_S[4] = (char)0xFF;	// 요구 상대 국번 IO Number H
	m_pData_S[5] = 0x03;	// 요구 상대 국번 IO Number L
	m_pData_S[6] = 0x00;	// 요구 상대 모듈 국번호
	m_pData_S[7] = 0x0C;	// 요구 Data 길이 L
	m_pData_S[8] = 0x00;	// 요구 Data 길이 R ( Data 길이 - 8 )
	m_pData_S[9] = 0x10;	// CPU 감시 Timer L
	m_pData_S[10] = 0x00;	// CPU 감시 Timer H
	m_pData_S[11] = 0x01;	// Command L
	m_pData_S[12] = 0x04;	// Command H
	m_pData_S[13] = 0x00;	// Sub Command L
	m_pData_S[14] = 0x00;	// Sub Command H
	// Start Address를 Hexa로
	m_pData_S[15] = (nAddr_Srt & 0xFF);//0xA8;	// 선두 Device L : 16진수 L25000 > L61A8
	m_pData_S[16] = (nAddr_Srt >> 8) & 0xFF;//0x61;	// 선두 Device M
	m_pData_S[17] = 0x00;	// 선두 Device H
	m_pData_S[18] = (char)0xB0;	// Device Code
	m_pData_S[19] = nLength & 0xFF;	// Device Count L : 16진수
	m_pData_S[20] = (nLength >> 8) & 0xFF;	// Device Count H

	//=====
	int bIsRet = false;
	stSendMsgPlc stData;
	stData.nDataLength = m_nLen_S;
	stData.pMCProtocolMsg = m_pData_S;
	stData.EventHandle = CreateEvent(0, FALSE, FALSE, 0);
	stData.nReturn = &bIsRet;
	stData.nDatatype = 1;
	stData.lpReadValue = pData7;
	stData.nReadDataLength = nLength;

	m_pSendData.Push(stData);

	m_pSemaMessage->Release(1);

	if (WaitForSingleObject(stData.EventHandle, INFINITE) == WAIT_OBJECT_0)
	{
		//처리 완료
		CloseHandle(stData.EventHandle);
	}

	return 0;
}
int CComPLC_RS::fnReadWordZR(int nAddr_Srt, int nLength, long *pData7, int nReadSize)
{
	if (m_bConnected == FALSE) return 0;

	int nIndex = 0;
	int m_nLen_S = 21;
	//BYTE m_pData_S[25];
	char* m_pData_S = new char[25];
	ZeroMemory(m_pData_S, 25);

	m_pData_S[0] = 0x50;	// Sub Head L
	m_pData_S[1] = 0x00;	// Sub Head H
	m_pData_S[2] = 0x00;	// Network Number
	m_pData_S[3] = (char)0xFF;	// PLC Number
	m_pData_S[4] = (char)0xFF;	// 요구 상대 국번 IO Number H
	m_pData_S[5] = 0x03;	// 요구 상대 국번 IO Number L
	m_pData_S[6] = 0x00;	// 요구 상대 모듈 국번호
	m_pData_S[7] = 0x0C;	// 요구 Data 길이 L
	m_pData_S[8] = 0x00;	// 요구 Data 길이 R ( Data 길이 - 8 )
	m_pData_S[9] = 0x10;	// CPU 감시 Timer L
	m_pData_S[10] = 0x00;	// CPU 감시 Timer H
	m_pData_S[11] = 0x01;	// Command L
	m_pData_S[12] = 0x04;	// Command H
	m_pData_S[13] = 0x00;	// Sub Command L
	m_pData_S[14] = 0x00;	// Sub Command H
	// Start Address를 Hexa로
	m_pData_S[15] = (nAddr_Srt & 0xFF);//0xA8;	// 선두 Device L : 16진수 L25000 > L61A8
	m_pData_S[16] = (nAddr_Srt >> 8) & 0xFF;//0x61;	// 선두 Device M
	m_pData_S[17] = (nAddr_Srt >> 16) & 0xFF;//0x61;	// 선두 Device M
	m_pData_S[18] = (char)0xB0;	// Device Code
	m_pData_S[19] = nLength & 0xFF;	// Device Count L : 16진수
	m_pData_S[20] = (nLength >> 8) & 0xFF;	// Device Count H

	//10: 0x90-m 0x92-L 0x93-F 0x94-v 0xA8-D //16: 0xA0-B 0x9C-X 0x9D-y 0xB4-w

	//=====
	int bIsRet = false;
	stSendMsgPlc stData;
	stData.nDataLength = m_nLen_S;
	stData.pMCProtocolMsg = m_pData_S;
	stData.EventHandle = CreateEvent(0, FALSE, FALSE, 0);
	stData.nReturn = &bIsRet;
	stData.nDatatype = 0;
	stData.lpReadValue = pData7;
	stData.nReadDataLength = nLength;

	m_pSendData.Push(stData);

	m_pSemaMessage->Release(1);

	if (WaitForSingleObject(stData.EventHandle, INFINITE) == WAIT_OBJECT_0)
	{
		//처리 완료
		CloseHandle(stData.EventHandle);
	}

	return 0;
}

int CComPLC_RS::fnWriteWordZR(int nAddr, int nLength, short* pData7)
{
	if (m_bConnected == FALSE) return 0;
	if (nLength > 4096) return 0;

	int nIndex = 0;
	int ni;

	char* m_pData_S = new char[4096];
	ZeroMemory(m_pData_S, 4096);

	m_pData_S[nIndex++] = 0x50;	// Sub Head L
	m_pData_S[nIndex++] = 0x00;	// Sub Head H
	m_pData_S[nIndex++] = 0x00;	// Network Number
	m_pData_S[nIndex++] = (char)0xFF;	// PLC Number
	m_pData_S[nIndex++] = (char)0xFF;	// 요구 상대 국번 IO Number H
	m_pData_S[nIndex++] = 0x03;	// 요구 상대 국번 IO Number L
	m_pData_S[nIndex++] = 0x00;	// 요구 상대 모듈 국번호
	nIndex += 2;

	m_pData_S[nIndex++] = 0x10;	// CPU 감시 Timer L
	m_pData_S[nIndex++] = 0x00;	// CPU 감시 Timer H

	m_pData_S[nIndex++] = 0x01;	// Command L
	m_pData_S[nIndex++] = 0x14;	// Command H

	m_pData_S[nIndex++] = 0x00;	// Sub Command L
	m_pData_S[nIndex++] = 0x00;	// Sub Command H

	m_pData_S[nIndex++] = (nAddr >> 0) & 0xFF;//0xA8;	// 선두 Device L : 16진수 L25000 > L61A8
	m_pData_S[nIndex++] = (nAddr >> 8) & 0xFF;//0x61;	// 선두 Device M
	m_pData_S[nIndex++] = (nAddr >> 16) & 0xFF;//선두 Device H
	m_pData_S[nIndex++] = (char)0xB0;	// Device Code(0x92) ?
	m_pData_S[nIndex++] = (nLength >> 0) & 0xFF;	// Device Count L
	m_pData_S[nIndex++] = (nLength >> 8) & 0xFF;	// Device Count H

	for (ni = 0; ni < nLength; ni++)
	{
		m_pData_S[nIndex++] = (*(pData7 + ni) >> 0) & 0x00FF;	// Data L
		m_pData_S[nIndex++] = (*(pData7 + ni) >> 8) & 0x00FF;	// Data H
	}

	int m_nLen_S = nIndex;

	nIndex -= 9;
	m_pData_S[7] = (nIndex >> 0) & 0xFF;	// 요구 Data 길이 L
	m_pData_S[8] = (nIndex >> 8) & 0xFF;	// 요구 Data 길이 R ( Data 길이 - 8 )

	//=====
	int bIsRet = false;
	stSendMsgPlc stData;
	stData.nDataLength = m_nLen_S;
	stData.pMCProtocolMsg = m_pData_S;
	stData.EventHandle = CreateEvent(0, FALSE, FALSE, 0);
	stData.nReturn = &bIsRet;
	//stData.nDatatype = 0;
	//stData.lpReadValue = pData7;
	//stData.nReadDataLength = nLength;

	m_pSendData.Push(stData);

	m_pSemaMessage->Release(1);

	if (WaitForSingleObject(stData.EventHandle, INFINITE) == WAIT_OBJECT_0)
	{
		//처리 완료
		CloseHandle(stData.EventHandle);
	}


	return 0;
}

int CComPLC_RS::fnWriteWordZR(int nAddr, int nLength, long* pData7)
{
	if (m_bConnected == FALSE) return 0;
	if (nLength > 4096) return 0;

	int nIndex = 0;
	int ni;

	char* m_pData_S = new char[4096];
	ZeroMemory(m_pData_S, 4096);

	m_pData_S[nIndex++] = 0x50;	// Sub Head L
	m_pData_S[nIndex++] = 0x00;	// Sub Head H
	m_pData_S[nIndex++] = 0x00;	// Network Number
	m_pData_S[nIndex++] = (char)0xFF;	// PLC Number
	m_pData_S[nIndex++] = (char)0xFF;	// 요구 상대 국번 IO Number H
	m_pData_S[nIndex++] = 0x03;	// 요구 상대 국번 IO Number L
	m_pData_S[nIndex++] = 0x00;	// 요구 상대 모듈 국번호
	nIndex += 2;

	m_pData_S[nIndex++] = 0x10;	// CPU 감시 Timer L
	m_pData_S[nIndex++] = 0x00;	// CPU 감시 Timer H

	m_pData_S[nIndex++] = 0x01;	// Command L
	m_pData_S[nIndex++] = 0x14;	// Command H

	m_pData_S[nIndex++] = 0x00;	// Sub Command L
	m_pData_S[nIndex++] = 0x00;	// Sub Command H

	m_pData_S[nIndex++] = (nAddr >> 0) & 0xFF;//0xA8;	// 선두 Device L : 16진수 L25000 > L61A8
	m_pData_S[nIndex++] = (nAddr >> 8) & 0xFF;//0x61;	// 선두 Device M
	m_pData_S[nIndex++] = (nAddr >> 16) & 0xFF;//선두 Device H
	m_pData_S[nIndex++] = (char)0xB0;	// Device Code(0x92) ?
	m_pData_S[nIndex++] = (nLength >> 0) & 0xFF;	// Device Count L
	m_pData_S[nIndex++] = (nLength >> 8) & 0xFF;	// Device Count H

	for (ni = 0; ni < nLength; ni++)
	{
		m_pData_S[nIndex++] = (*(pData7 + ni) >> 0) & 0xFF;	// Data L
		m_pData_S[nIndex++] = (*(pData7 + ni) >> 8) & 0xFF;	// Data H
	}

	int m_nLen_S = nIndex;

	nIndex -= 9;
	m_pData_S[7] = (nIndex >> 0) & 0xFF;	// 요구 Data 길이 L
	m_pData_S[8] = (nIndex >> 8) & 0xFF;	// 요구 Data 길이 R ( Data 길이 - 8 )

	//=====
	int bIsRet = false;
	stSendMsgPlc stData;
	stData.nDataLength = m_nLen_S;
	stData.pMCProtocolMsg = m_pData_S;
	stData.EventHandle = CreateEvent(0, FALSE, FALSE, 0);
	stData.nReturn = &bIsRet;
	//stData.nDatatype = 0;
	//stData.lpReadValue = pData7;
	//stData.nReadDataLength = nLength;

	m_pSendData.Push(stData);

	m_pSemaMessage->Release(1);

	if (WaitForSingleObject(stData.EventHandle, INFINITE) == WAIT_OBJECT_0)
	{
		//처리 완료
		CloseHandle(stData.EventHandle);
	}

	return 0;
}

long CComPLC_RS::GetTotalWriteBitData(long nStarAddress, long nSize, long *lpData)
{
	long nReturnCode = 0;

	nReturnCode = fnReadBitM(int(nStarAddress), int(nSize), (short *)(lpData));

	return nReturnCode;
}

int CComPLC_RS::fnWriteBitArrayM_ON(int num_of_addr, int* pAddr)
{
	if (m_bConnected == FALSE) return 0;

	EnterCriticalSection(&m_csComm);

	int nIndex = 0;
	BYTE m_pData_S[250];

	m_pData_S[nIndex++] = 0x50;	// Sub Head L
	m_pData_S[nIndex++] = 0x00;	// Sub Head H
	m_pData_S[nIndex++] = 0x00;	// Network Number
	m_pData_S[nIndex++] = 0xFF;	// PLC Number
	m_pData_S[nIndex++] = 0xFF;	// 요구 상대 국번 IO Number H
	m_pData_S[nIndex++] = 0x03;	// 요구 상대 국번 IO Number L
	m_pData_S[nIndex++] = 0x00;	// 요구 상대 모듈 국번호
	nIndex += 2;
	//m_pData_S[7] = 0x0C;	// 요구 Data 길이 L
	//m_pData_S[8] = 0x00;	// 요구 Data 길이 R ( Data 길이 - 8 )

	m_pData_S[nIndex++] = 0x10;	// CPU 감시 Timer L
	m_pData_S[nIndex++] = 0x00;	// CPU 감시 Timer H

	m_pData_S[nIndex++] = 0x02;	// Command L
	m_pData_S[nIndex++] = 0x14;	// Command H

	m_pData_S[nIndex++] = 0x01;	// Sub Command L
	m_pData_S[nIndex++] = 0x00;	// Sub Command H

	/*
	m_pData_S[nIndex++] = 0x01;	// Bit 수
	m_pData_S[nIndex++] = (nAddr >> 0) & 0xFF;//0xA8;	// 선두 Device L : 16진수 L25000 > L61A8
	m_pData_S[nIndex++] = (nAddr >> 8) & 0xFF;//0x61;	// 선두 Device M
	m_pData_S[nIndex++] = (nAddr >> 16) & 0xFF;//선두 Device H
	m_pData_S[nIndex++] = 0x90;	// Device Code(0x92) ?
	m_pData_S[nIndex++] = bOnOff ? 1 : 0;	// On/Off Value
	*/

	m_pData_S[nIndex++] = num_of_addr;	// Bit 수

	for (int i = 0; i < num_of_addr; i++)
	{
		m_pData_S[nIndex++] = (pAddr[i] >> 0) & 0xFF;//0xA8;	// 선두 Device L : 16진수 L25000 > L61A8
		m_pData_S[nIndex++] = (pAddr[i] >> 8) & 0xFF;//0x61;	// 선두 Device M
		m_pData_S[nIndex++] = (pAddr[i] >> 16) & 0xFF;//선두 Device H
		m_pData_S[nIndex++] = 0x90;	// Device Code(0x92) ?
		m_pData_S[nIndex++] = 1;	// On/Off Value
	}

	//ReadBit M영역으로 변경 0x90(M) 0x92(L) 2019.07.22	

	int m_nLen_S = nIndex;
	nIndex -= 9;
	m_pData_S[7] = (nIndex >> 0) & 0xFF;	// 요구 Data 길이 L
	m_pData_S[8] = (nIndex >> 8) & 0xFF;	// 요구 Data 길이 R ( Data 길이 - 8 )

	ResetEvent(m_hWowEvent);
	fn_SendCommandData((char*)m_pData_S, m_nLen_S);
	///////////////////////////////////////////////////////////////////////////////////////
	int nRet = WaitForSingleObject(m_hWowEvent, 2000);
	if (nRet == WAIT_TIMEOUT)
	{
		CString strTemp;
		strTemp.Format("fnWriteBitArrayM_ON - Timeout");
		if (m_pMain)	::SendMessageA(m_pMain->m_pPane[PANE_MANUAL]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);
	}

	LeaveCriticalSection(&m_csComm);

	return 0;
}

int CComPLC_RS::fnWriteBitArrayM_OFF(int num_of_addr, int* pAddr)
{
	if (m_bConnected == FALSE) return 0;

	EnterCriticalSection(&m_csComm);

	int nIndex = 0;
	//BYTE m_pData_S[250];
	char* m_pData_S = NULL;
	m_pData_S = new char[250];

	m_pData_S[nIndex++] = 0x50;	// Sub Head L
	m_pData_S[nIndex++] = 0x00;	// Sub Head H
	m_pData_S[nIndex++] = 0x00;	// Network Number
	m_pData_S[nIndex++] = (char)0xFF;	// PLC Number
	m_pData_S[nIndex++] = (char)0xFF;	// 요구 상대 국번 IO Number H
	m_pData_S[nIndex++] = 0x03;	// 요구 상대 국번 IO Number L
	m_pData_S[nIndex++] = 0x00;	// 요구 상대 모듈 국번호
	nIndex += 2;
	//m_pData_S[7] = 0x0C;	// 요구 Data 길이 L
	//m_pData_S[8] = 0x00;	// 요구 Data 길이 R ( Data 길이 - 8 )

	m_pData_S[nIndex++] = 0x10;	// CPU 감시 Timer L
	m_pData_S[nIndex++] = 0x00;	// CPU 감시 Timer H

	m_pData_S[nIndex++] = 0x02;	// Command L
	m_pData_S[nIndex++] = 0x14;	// Command H

	m_pData_S[nIndex++] = 0x01;	// Sub Command L
	m_pData_S[nIndex++] = 0x00;	// Sub Command H


	/*
	m_pData_S[nIndex++] = 0x01;	// Bit 수
	m_pData_S[nIndex++] = (nAddr >> 0) & 0xFF;//0xA8;	// 선두 Device L : 16진수 L25000 > L61A8
	m_pData_S[nIndex++] = (nAddr >> 8) & 0xFF;//0x61;	// 선두 Device M
	m_pData_S[nIndex++] = (nAddr >> 16) & 0xFF;//선두 Device H
	m_pData_S[nIndex++] = 0x90;	// Device Code(0x92) ?
	m_pData_S[nIndex++] = bOnOff ? 1 : 0;	// On/Off Value
	*/

	m_pData_S[nIndex++] = num_of_addr;	// Bit 수

	for (int i = 0; i < num_of_addr; i++)
	{
		m_pData_S[nIndex++] = (pAddr[i] >> 0) & 0xFF;//0xA8;	// 선두 Device L : 16진수 L25000 > L61A8
		m_pData_S[nIndex++] = (pAddr[i] >> 8) & 0xFF;//0x61;	// 선두 Device M
		m_pData_S[nIndex++] = (pAddr[i] >> 16) & 0xFF;//선두 Device H
		m_pData_S[nIndex++] = (char)0x90;	// Device Code(0x92) ?
		m_pData_S[nIndex++] = 0;	// On/Off Value
	}

	//ReadBit M영역으로 변경 0x90(M) 0x92(L) 2019.07.22	

	int m_nLen_S = nIndex;
	nIndex -= 9;
	m_pData_S[7] = (nIndex >> 0) & 0xFF;	// 요구 Data 길이 L
	m_pData_S[8] = (nIndex >> 8) & 0xFF;	// 요구 Data 길이 R ( Data 길이 - 8 )

	ResetEvent(m_hWowEvent);
	fn_SendCommandData((char*)m_pData_S, m_nLen_S);
	///////////////////////////////////////////////////////////////////////////////////////
	int nRet = WaitForSingleObject(m_hWowEvent, 2000);
	if (nRet == WAIT_TIMEOUT)
	{
		CString strTemp;
		strTemp.Format("fnWriteBitArrayM_OFF - Timeout");
		if (m_pMain)	::SendMessageA(m_pMain->m_pPane[PANE_MANUAL]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);
	}
	//아래부분이 구현되면 삭제한다.
	if (m_pData_S != NULL)
	{
		delete[] m_pData_S;
	}
	
	LeaveCriticalSection(&m_csComm);

	return 0;
}

UINT CComPLC_RS::ThreadMessage(LPVOID lpParam_)
{
	CComPLC_RS* pThis = reinterpret_cast<CComPLC_RS*>(lpParam_);

	while (pThis->m_bIsThreadAlive)
	{
		if (pThis->m_pSemaMessage->Wait(INFINITE) == WAIT_OBJECT_0)
		{
			pstSendMsgPlc pdata = pThis->m_pSendData.Pull();

			if (pdata == NULL)
				continue;

			if (pdata->nDataLength == 0) continue;

			ResetEvent(pdata->EventHandle);			

			pThis->fn_SendCommandData(pdata->pMCProtocolMsg, pdata->nDataLength);

			int nRes = WaitForSingleObject(pThis->m_hWowEvent, 2000);
			if (nRes == WAIT_TIMEOUT)
			{
				CString strTemp;
				strTemp.Format("fnWriteBitArrayM_OFF - Timeout");
				//Log 남기는거 확인하자.
				//if (pThis->m_pMain)	::SendMessageA(pThis->m_pMain->m_pPane[PANE_MANUAL]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);				
				
				*pdata->nReturn = 1;
			}
			else    //여기서 Message를 처리한다.
			{
				

				if (pdata->lpReadValue != NULL)
				{
					//pdata->lpReadValue;
					if (pdata->nReadDataLength == 1)
					{
						(int&)pdata->lpReadValue = pThis->m_pReceiveBuffer[11] & 0x01;
					}
					else    //여기는 Pointer이다 사이즈가 있는...
					{
						if (pdata->nDatatype == 0)   //Int Size
						{
							if (pdata->pMCProtocolMsg[18] == 0xAF)
							{
								int nDataCount = (int)(*(pThis->m_pReceiveBuffer + 7)) + (int)((*(pThis->m_pReceiveBuffer + 8)) << 8) - 2;

								BYTE* pBuffer = pThis->m_pReceiveBuffer + 11;

								//if (data->nLength != (nDataCount / 2))
								//{
								//	CString strTemp;
								//	strTemp.Format("nAddr4 : R%d, nLength : %d, nCount : %d", nAddr_Srt, nLength, nDataCount);
								//	//::SendMessageA(m_pMain->m_pPane[PANE_MANUAL]->m_hWnd, WM_VIEW_CONTROL, MSG_PMC_ADD_PROCESS_HISTORY, (LPARAM)&strTemp);
								//}

								short* lData = new short[nDataCount / 2];

								for (int ni = 0; ni < nDataCount / 2; ni++)
								{
									*(lData + ni) = *(pThis->m_pReceiveBuffer + 11 + (ni * 2)) + ((*(pThis->m_pReceiveBuffer + 11 + (ni * 2) + 1)) << 8);
									*((long*)pdata->lpReadValue + ni) = *(lData + ni);
								}

								delete[]lData;
							}
							else
							{
								int nDataCount = (int)(*(pThis->m_pReceiveBuffer + 7)) + (int)((*(pThis->m_pReceiveBuffer + 8)) << 8) - 2;

								unsigned short* pTemp = new unsigned short[nDataCount];

								for (int ni = 0; ni < nDataCount / 2; ni++)
								{
									*(pTemp + ni) = *(pThis->m_pReceiveBuffer + 11 + (ni * 2)) + ((*(pThis->m_pReceiveBuffer + 11 + (ni * 2) + 1)) << 8);
									*((long*)pdata->lpReadValue + ni) = *(pTemp + ni);
									//*(pData + ni) = *(m_pReceiveBuffer + 11 + (ni * 2)) + ((*(m_pReceiveBuffer + 11 + (ni * 2) + 1)) << 8);
								}

								delete[]pTemp;
							}
						}
						else if (pdata->nDatatype == 1)    //short size
						{
							if (pdata->pMCProtocolMsg[18] == 0xAF)	// Device Code)
							{
								int nDataCount = (int)(*(pThis->m_pReceiveBuffer + 7)) + (int)((*(pThis->m_pReceiveBuffer + 8)) << 8) - 2;
								BYTE* pBuffer = pThis->m_pReceiveBuffer + 11;

								if (pdata->nDataLength != (nDataCount / 2))
								{
									
								}
								else
								{
									for (int ni = 0; ni < pdata->nDataLength / 2; ni++) //20180930 수정
										((short*)pdata->lpReadValue)[ni] = pBuffer[ni * 2] | pBuffer[ni * 2 + 1] << 8;
								}
								
							}
							else
							{
								int nDataCount = (int)(*(pThis->m_pReceiveBuffer + 7)) + (int)((*(pThis->m_pReceiveBuffer + 8)) << 8) - 2;
								//for (int ni = 0; ni < nDataCount; ni++)
								//{
								//	*((short*)pdata->lpReadValue + ni) = *(pThis->m_pReceiveBuffer + 11 + (ni * 2)) + ((*(pThis->m_pReceiveBuffer + 11 + (ni * 2) + 1)) << 8);
								//}

								memcpy((short*)pdata->lpReadValue, pThis->m_pReceiveBuffer + 11, nDataCount);
							}
						}

					}
				}

				*pdata->nReturn = 0;
			}

			SetEvent(pdata->EventHandle);		

			delete[] pdata->pMCProtocolMsg;
			pdata->pMCProtocolMsg = NULL;
		}

		Sleep(5);
	}

	pThis->m_bIsThreadEnd = true;

	return 0;
}

void CComPLC_RS::WaitEvent(INT32 nMilliSec_)
{
	INT32 nMilliSec = 0;
	nMilliSec = (nMilliSec_ / 10);	// Divide Sleep Rate

	if (nMilliSec < 0)
		nMilliSec = 1;
	else {
		for (INT32 i = 0; i < nMilliSec; i++) {
			Sleep(10);
			MyDoEvents();
		}
	}
}
void CComPLC_RS::MyDoEvents()
{
	MSG msg;  // 정보를 받을 MSG 구조의 변수 선언

	// PeekMessage 함수는 메시지를 위한 쓰레드 메시지 큐를 점검해서, 메시지가 있다면 NonZero, 메시지가 없다면 Zero 반환. 
	if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) == TRUE)
	{   // PM_REMOVE = 처리후에 큐에서 메시지 제거
		// 메세지가 있을 때의 처리
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}