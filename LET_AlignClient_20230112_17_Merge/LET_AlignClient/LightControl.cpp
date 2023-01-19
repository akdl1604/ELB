
#include "StdAfx.h"
#include "LightControl.h"

CLightControl::CLightControl(void)
{	
	m_nID = 0;
	m_nLightType = 0;
	m_hCommWnd = NULL;
	m_WM_VIEW_CONTROL = -1;
	m_MSG_LIGHT_RECV_ACK = -1;

	m_hDummyEvent = CreateEvent(NULL, TRUE, FALSE, FALSE);
	memset(m_rxBuff, 0, sizeof(m_rxBuff));

	m_rxTag = 0;
	m_bThreadEnd = FALSE;
	m_bRecvData = FALSE;

	InitializeCriticalSection(&m_csSend);
}

CLightControl::~CLightControl(void)
{
	DeleteCriticalSection(&m_csSend);
}

BOOL CLightControl::OpenPort(int nPort, int nBaudrate)
{
	BOOL bRet = 0;
	CString strPort, strBaudrate;

	strPort.Format("\\\\.\\COM%d", nPort);
	strBaudrate.Format("%d", nBaudrate);

	if( m_Comm.m_bConnected )m_Comm.ClosePort();

	bRet = m_Comm.OpenPort(strPort, strBaudrate, "8", "NO", "NO", "1");

	if( bRet ) {
		m_Comm.m_bConnected = bRet;
		AfxBeginThread(processReceiveLight, this);		
	}

	return bRet;
}

UINT processReceiveLight(LPVOID pParam)
{
	CLightControl *pLight = (CLightControl *)pParam;
	BYTE aByte = 0;
	int size = 0;
	char  datum = 0;

	pLight->m_rxTag = 0;
	pLight->m_Comm.m_QueueRead.Clear();
	memset(pLight->m_rxBuff, 0, sizeof( pLight->m_rxBuff ) );

	pLight->m_bThreadEnd = FALSE;

	while( pLight->m_Comm.m_bConnected ) {
		if( pLight->m_Comm.m_bReserveMsg ) 
		{
			size = pLight->m_Comm.m_QueueRead.GetSize();
			for (int i = 0; i < size; i++)
			{
				pLight->m_Comm.m_QueueRead.GetByte(&aByte);
				datum = aByte;

				// 배열 사이즈를 넘어 갈수록 0으로 초기화 한다.
				if( pLight->m_rxTag > LIGHT_RECV_BUF_SIZE - 1)
				{
					pLight->m_rxTag = 0;
					pLight->m_Comm.m_QueueRead.Clear();
				}

				if( datum != NULL )	pLight->m_rxBuff[pLight->m_rxTag++] = datum;
			}

			// 데이터가 수신 될 때 마다 ACK를 날린다.
			if( pLight->m_hCommWnd != NULL && pLight->m_WM_VIEW_CONTROL != -1 && pLight->m_MSG_LIGHT_RECV_ACK != -1 )
				//::SendMessageA(pLight->m_hCommWnd, pLight->m_WM_VIEW_CONTROL, pLight->m_MSG_LIGHT_RECV_ACK, pLight->m_nID);
				::SendMessageA(pLight->m_hCommWnd, pLight->m_WM_VIEW_CONTROL, pLight->m_MSG_LIGHT_RECV_ACK, MAKELPARAM(pLight->m_nID, TRUE));
			
			pLight->m_bRecvData = TRUE;
			pLight->m_Comm.m_bReserveMsg = FALSE;
		}

		::WaitForSingleObject(pLight->m_hDummyEvent, 100);
	}

	pLight->m_bThreadEnd = TRUE;

	return 0;
}

BOOL CLightControl::setBrightness(int nCh, int nValue)
{
	if( m_Comm.m_bConnected != TRUE ) return FALSE;

	BYTE cData[MAX_PATH] = {0, };
	CString sData;

	
	// 0 : 아이텍(DPxx)
	// 1 : TSM	
	// 2 : 아이텍(DPxx)
	// 3 : L-Light(LICONT-LS-500W-16CH
	// 4 : L-Light(LPC-16CH-500W)
	// 5 : L-Light(LPC-8CH-COT) Socket Type
	// 6 : L-Light(LPW-16CH) Socket Type
	if (m_nLightType == 0)
	{
		sData.Format(_T(":L%X%03d\r\n"), nCh + 1, nValue);
	}
	else if (m_nLightType == 1)
	{
		sData.Format(_T("\x002%02dWR%03d\x003\r\n"), nCh + 1, nValue);  // [STX:0x02] + channel(2글자) + WR + value(3글자) + [ETX0x03]
	}
	else if (m_nLightType == 2)
	{
		if( nCh == 15 )	sData.Format(":1LG%03d\r\n", nValue);
		else			sData.Format(":1L%X%03d\r\n", nCh + 1, nValue);
	}
	else if (m_nLightType == 3)
	{
		double dValue = nValue / 10.0;
		sData.Format("%cCH%02dW%03d.%d%c", CODE_STX, nCh + 1, (int)dValue, nValue % 10, CODE_ETX);
	}
	else if( m_nLightType == 4 )
	{
		sData.Format("%cCH%02dW%04d%c", CODE_STX, nCh + 1, nValue, CODE_ETX);
	}
	else if (m_nLightType == 7)
	{
		sData.Format("%cCH%02dS%04d%c\r\n", CODE_STX, nCh + 1, nValue, CODE_ETX);
	}
	sendSerialData(sData);
	//m_Comm.WriteComm(cData, strlen((char*)cData));
	return TRUE;
}



BOOL CLightControl::sendSerialData(CString str)
{
	BOOL bSuccess = FALSE;

	if (m_Comm.m_bConnected != TRUE) return bSuccess;

	EnterCriticalSection(&m_csSend);

	char sSendMsg[MAX_PATH] = { 0, };
	sprintf(sSendMsg, "%s", (LPCTSTR)str);

	m_Comm.WriteComm((BYTE *)sSendMsg,(DWORD) strlen(sSendMsg));

	LeaveCriticalSection(&m_csSend);

	bSuccess = TRUE;
	return bSuccess;
}

//int CLightControl::set_light_value(int nChannel, int nValue)
//{
//	BYTE data[255] = { 0, };
//	int nIndex = 0;
//
//	data[nIndex++] = 0x02;			// Start
//	data[nIndex++] = 'C';
//	data[nIndex++] = 'H';
//	data[nIndex++] = 'C';
//	data[nIndex++] = 0x03;			// Start
////	int nRet = SendData((char *)data, nIndex);
//	CString strTemp;
//	strTemp.Format("%s", data);
//	//int nRet = sendSerialData(strTemp);
//	int nRet = m_Comm.WriteComm((BYTE*)data, nIndex);
//
//	return nRet;
//}


int CLightControl::set_light_value(int nChannel, int nValue)
{
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

//	int nRet = SendData((char *)data, nIndex);
	CString strTemp;
	strTemp.Format("%s", data);
	//int nRet = sendSerialData(strTemp);
	int nRet = m_Comm.WriteComm((BYTE *)data, nIndex);

	return nRet;
}