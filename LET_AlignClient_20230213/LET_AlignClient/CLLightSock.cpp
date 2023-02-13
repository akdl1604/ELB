#include "stdafx.h"
#include "CLLightSock.h"


CLLightSock::CLLightSock()
{
	m_hSock = NULL;
	m_bRecv = FALSE;
	m_nID = 0;
	m_hMsgHandle = FALSE;
	m_WM_VIEW_CONTROL = FALSE;
	m_MSG_CONNECT = FALSE;
	m_MSG_DISCONNECT = FALSE;
	m_MSG_RETURN_DATA = FALSE;
}


CLLightSock::~CLLightSock()
{
}


BOOL CLLightSock::Connect(CString strSocketIp, int nSockPort)
{
	if (mi_Socket.GetSocketCount())		return TRUE;

	DWORD u32_IP = inet_addr(strSocketIp);
	DWORD u32_ID;
	DWORD u32_EventTimeout = INFINITE;
	DWORD u32_Err = mi_Socket.ConnectTo(u32_IP, nSockPort, u32_EventTimeout, 0);

	if (u32_Err)
	{
		CloseSockets();
	//	StartTimer();
		return FALSE;
	}

	mh_Thread = CreateThread(0, 0, ProcessEventThread, this, 0, &u32_ID);
	return TRUE;
}


ULONG WINAPI CLLightSock::ProcessEventThread(void* p_Param)
{
	CLLightSock* p_This = (CLLightSock *)p_Param;
	p_This->ProcessEvents();
	CloseHandle(p_This->mh_Thread);
	return 0;
}


void CLLightSock::ProcessEvents()
{
	BOOL b_Server = (mi_Socket.GetState() & TCP::cSocket::E_Server);

	while (TRUE) // Loop runs until the main window was closed or a severe error occurred
	{
		TCP::cSocket::cMemory* pi_RecvMem;
		SOCKET  h_Socket;
		DWORD u32_Event, u32_IP, u32_Read, u32_Sent;
		DWORD u32_Err = mi_Socket.ProcessEvents(&u32_Event, &u32_IP, &h_Socket, &pi_RecvMem, &u32_Read, &u32_Sent);

		if (u32_Err == ERROR_TIMEOUT) 		continue;

		CString s_Msg, s_Events;
		if (u32_Event) // ATTENTION: u32_Event may be == 0 -> do nothing.
		{
			char s8_Events[200];
			mi_Socket.FormatEvents(u32_Event, s8_Events);
			s_Events += s8_Events;

			if (u32_Event & FD_ACCEPT) {}
			if (u32_Event & FD_CLOSE)
			{
			//	findDisconnectSocket(h_Socket);
				m_hSock = NULL;
				if (m_hMsgHandle != NULL && m_WM_VIEW_CONTROL != NULL && m_MSG_DISCONNECT != NULL)
					::SendMessage(m_hMsgHandle, m_WM_VIEW_CONTROL, m_MSG_DISCONNECT, MAKELPARAM(m_nID, FALSE));
			}

			if (u32_Event & FD_CONNECT) {
				m_hSock = h_Socket;

				if( m_hMsgHandle != NULL && m_WM_VIEW_CONTROL != NULL && m_MSG_CONNECT != NULL )
					::SendMessage(m_hMsgHandle, m_WM_VIEW_CONTROL, m_MSG_CONNECT, MAKELPARAM(m_nID, TRUE));
			}
			
			if (u32_Event & FD_READ && pi_RecvMem) 		ProcessReceivedDataNormal(pi_RecvMem);
				//ProcessReceivedDataPrefix(pi_RecvMem, h_Socket);
		}

		if (u32_Err)
		{
			if (u32_Err == WSAENOTCONN)
			{
				m_hSock = NULL;
				if (m_hMsgHandle != NULL && m_WM_VIEW_CONTROL != NULL && m_MSG_DISCONNECT != NULL)
					::SendMessage(m_hMsgHandle, m_WM_VIEW_CONTROL, m_MSG_DISCONNECT, m_nID);
				break;
			}
			if (u32_Err != WSAECONNABORTED && u32_Err != WSAECONNRESET && u32_Err != WSAECONNREFUSED && u32_Err != WSAESHUTDOWN)    break;
		}
	}; // end loop

	CloseSockets();

}

void CLLightSock::ProcessReceivedDataNormal(TCP::cSocket::cMemory* pi_RecvMem)
{
	CString s_String, strTmp;
	char*  s8_Buf = pi_RecvMem->GetBuffer();
	DWORD u32_Len = pi_RecvMem->GetLength();

	m_bRecv = TRUE;
	CString str = CopyToString(s8_Buf, u32_Len);

	if (m_hMsgHandle != NULL && m_WM_VIEW_CONTROL != NULL && m_MSG_RETURN_DATA != NULL)
		::SendMessage(m_hMsgHandle, m_WM_VIEW_CONTROL, m_MSG_RETURN_DATA, LPARAM(&str));

	pi_RecvMem->DeleteLeft(u32_Len);
}

CString CLLightSock::CopyToString(char* s8_Buf, DWORD u32_Bytes)
{
	DWORD u32_StrLen = u32_Bytes / sizeof(TCHAR);

	CString s_String;
	char*  s8_String = (char*)s_String.GetBuffer(u32_StrLen + 1);
	memcpy(s8_String, s8_Buf, u32_Bytes);
	s_String.ReleaseBuffer(u32_StrLen);

	return s_String;
}

void CLLightSock::CloseSockets()
{
	if (mi_Socket.GetSocketCount())
	{
		mi_Socket.Close();
	}
}


DWORD CLLightSock::SendData(char *pData, int nLength)
{
	if (m_hSock == NULL)	return -1;
	return mi_Socket.SendTo(m_hSock, pData, nLength);
}


int CLLightSock::setCotCount(int nCount)
{
	if (m_hSock == NULL) return -1;

	BYTE data[255] = { 0, };
	int nIndex = 0;

	data[nIndex++] = 0x01;			// Start
	data[nIndex++] = OP_WRITE;		// OP Code
	data[nIndex++] = 0x01;			// Data Length
	data[nIndex++] = REG_COTR_CNT;	// Register
	data[nIndex++] = nCount;		// Data
	data[nIndex++] = 0x04;			// End

	int nRet = SendData((char *)data, nIndex);
	return nRet;
}


int CLLightSock::restartEthernet()
{
	if (m_hSock == NULL) return -1;

	BYTE data[255] = { 0, };
	int nIndex = 0;

	data[nIndex++] = 0x01;						// Start
	data[nIndex++] = OP_WRITE;					// OP Code
	data[nIndex++] = 0x01;						// Data Length
	data[nIndex++] = REG_SCR;					// Register
	data[nIndex++] = 1 << BIT_SCR_RST_ETH;		// Data
	data[nIndex++] = 0x04;						// End

	int nRet = SendData((char *)data, nIndex);
	return nRet;
}


int CLLightSock::saveRegistData()
{
	if (m_hSock == NULL) return -1;

	BYTE data[255] = { 0, };
	int nIndex = 0;

	data[nIndex++] = 0x01;					// Start
	data[nIndex++] = OP_WRITE;				// OP Code
	data[nIndex++] = 0x01;					// Data Length
	data[nIndex++] = REG_SCR;				// Register
	data[nIndex++] = 1 << BIT_SCR_SAVE_RG;	// Data
	data[nIndex++] = 0x04;					// End

	int nRet = SendData((char *)data, nIndex);
	return nRet;
}

int CLLightSock::resetTriggerCount()
{
	if (m_hSock == NULL) return -1;

	BYTE data[255] = { 0, };
	int nIndex = 0;

	data[nIndex++] = 0x01;					// Start
	data[nIndex++] = OP_WRITE;				// OP Code
	data[nIndex++] = 0x01;					// Data Length
	data[nIndex++] = REG_RCR;				// Register
	data[nIndex++] = 1 << BIT_RCR_RST_TC;	// Data
	data[nIndex++] = 0x04;					// End

	int nRet = SendData((char *)data, nIndex);
	return nRet;
}

int CLLightSock::setDefaultEthernet()
{
	if (m_hSock == NULL) return -1;

	BYTE data[255] = { 0, };
	int nIndex = 0;

	data[nIndex++] = 0x01;					// Start
	data[nIndex++] = OP_WRITE;				// OP Code
	data[nIndex++] = 0x01;					// Data Length
	data[nIndex++] = REG_RCR;				// Register
	data[nIndex++] = 1 << BIT_RCR_INIT_ETH;	// Data
	data[nIndex++] = 0x04;					// End

	int nRet = SendData((char *)data, nIndex);
	return nRet;
}


int CLLightSock::setTriggerPage(int nPage, int pOnOff[16], int pValue[16])
{
	BYTE data[255] = { 0, };
	int nIndex = 0;

	data[nIndex++] = 0x01;							// Start
	data[nIndex++] = OP_WRITE;						// OP Code
	data[nIndex++] = 20;							// Data Length
	data[nIndex++] = REG_COTR_PAGE_CFG + nPage;		// Register
	
	// Data 20Byte 중 
	// 첫 2바이트는 Bit별로 Channel On/Off
	// 다음 2바이트는 Reserve
	// 나머지 16바이트는 채널별 조명 값

	BYTE bOnOff[2] = { 0, };
	// 컨트롤러 쪽에서 Bit 가중치를 반대로 봐서, 반대로 보내야 됨 -_-
	for (int i = 0; i < 16; i++)
	{
		if (pOnOff[i] != 0)
		{
			bOnOff[(16 - i - 1) / 8] |= (1 << i % 8);
		}
	}

	// 채널별 On/Off
	data[nIndex++] = bOnOff[0];
	data[nIndex++] = bOnOff[1];
	
	// Reserve
	nIndex++;
	nIndex++;

	// 채널별 조명값
	for (int i = 0; i < 16; i++)
	{
		if (pValue[i] < 0)			data[nIndex++] = 0;
		else if (pValue[i] > 100)	data[nIndex++] = 100;
		else						data[nIndex++] = pValue[i];
	}
	
	data[nIndex++] = 0x04;							// End

	int nRet = SendData((char *)data, nIndex);
	return nRet;
}

int CLLightSock::setGenerateClockLimit(int nLimit)
{
	if (m_hSock == NULL) return -1;

	BYTE data[255] = { 0, };
	int nIndex = 0;

	data[nIndex++] = 0x01;					// Start
	data[nIndex++] = OP_WRITE;				// OP Code
	data[nIndex++] = 0x04;					// Data Length
	data[nIndex++] = REG_GENC_LIMIT;		// Register

	// Limit : 0 ~ 100000
	// 0이면 Limit 없음.
	if (nLimit < 0)			nLimit = 0;
	if( nLimit > 100000)	nLimit = 100000;

	data[nIndex++] = (nLimit >> 24) & 0xFF;	// Data
	data[nIndex++] = (nLimit >> 16) & 0xFF;	// Data
	data[nIndex++] = (nLimit >> 8) & 0xFF;	// Data
	data[nIndex++] = (nLimit >> 0) & 0xFF;	// Data

	data[nIndex++] = 0x04;					// End

	int nRet = SendData((char *)data, nIndex);
	return nRet;
}

int CLLightSock::setGenerateClockFrequency(int nFreq)
{
	if (m_hSock == NULL) return -1;

	BYTE data[255] = { 0, };
	int nIndex = 0;

	data[nIndex++] = 0x01;					// Start
	data[nIndex++] = OP_WRITE;				// OP Code
	data[nIndex++] = 0x02;					// Data Length
	data[nIndex++] = REG_GENC_CLOCK;		// Register

	// Limit : 0 ~ 100000
	// 0이면 Limit 없음.
	if (nFreq < 10)		nFreq = 10;
	if (nFreq > 25)		nFreq = 25;

	data[nIndex++] = (nFreq >> 8) & 0xFF;	// Data
	data[nIndex++] = (nFreq >> 0) & 0xFF;	// Data

	data[nIndex++] = 0x04;					// End

	int nRet = SendData((char *)data, nIndex);
	return nRet;
}


int CLLightSock::setGenerateClockDuty(int nDuty)
{
	if (m_hSock == NULL) return -1;

	BYTE data[255] = { 0, };
	int nIndex = 0;

	data[nIndex++] = 0x01;					// Start
	data[nIndex++] = OP_WRITE;				// OP Code
	data[nIndex++] = 0x02;					// Data Length
	data[nIndex++] = REG_GENC_DUTY;			// Register

	// 동작 범위 : 10 ~ 90 %
	if (nDuty < 10)		nDuty = 10;
	if (nDuty > 90)		nDuty = 90;

	data[nIndex++] = (nDuty >> 8) & 0xFF;	// Data
	data[nIndex++] = (nDuty >> 0) & 0xFF;	// Data

	data[nIndex++] = 0x04;					// End

	int nRet = SendData((char *)data, nIndex);
	return nRet;
}

int CLLightSock::setGenerateClockDelay(int nDelay)
{
	if (m_hSock == NULL) return -1;

	BYTE data[255] = { 0, };
	int nIndex = 0;

	data[nIndex++] = 0x01;					// Start
	data[nIndex++] = OP_WRITE;				// OP Code
	data[nIndex++] = 0x04;					// Data Length
	data[nIndex++] = REG_GENC_ON_DELAY;		// Register

	// 동작 범위 : 10 ~ 90 %
	if (nDelay < 0)			nDelay = 0;
	if (nDelay > 100000)	nDelay = 100000;

	data[nIndex++] = (nDelay >> 24) & 0xFF;	// Data
	data[nIndex++] = (nDelay >> 16) & 0xFF;	// Data
	data[nIndex++] = (nDelay >> 8) & 0xFF;	// Data
	data[nIndex++] = (nDelay >> 0) & 0xFF;	// Data

	data[nIndex++] = 0x04;					// End

	int nRet = SendData((char *)data, nIndex);
	return nRet;
}


int CLLightSock::set_light_value(int nChannel, int nValue)

{
	if (m_hSock == NULL) return -1;

	BYTE data[255] = { 0, };
	int nIndex = 0;

	data[nIndex++] = 0x01;			// Start
	data[nIndex++] = OP_WRITE;		// OP Code
	data[nIndex++] = 0x03;			// Data Length
	data[nIndex++] = REG_CSVR;		// Register

	// CH1 ~ CH16
	if (nChannel < 0)		nChannel = 0;
	if (nChannel > 15)		nChannel = 15;
	if (nValue < 0)			nValue = 0;
	if (nValue > 255)		nValue = 255;

	int ch = 1 << nChannel;

	data[nIndex++] = (ch >> 8) & 0xFF;	// Data	(CH16 ~  CH 9)
	data[nIndex++] = (ch >> 0) & 0xFF;	// Data (CH8 ~ CH1)
	data[nIndex++] = nValue;					// Data (Value)

	data[nIndex++] = 0x04;					// End

	int nRet = SendData((char *)data, nIndex);
	return nRet;
}

