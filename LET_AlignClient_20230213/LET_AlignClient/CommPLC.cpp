#include "stdafx.h"
#include "CommPLC.h"
#include "LET_AlignClient.h"
#include "LET_AlignClientDlg.h"
#include "ComPLC_RS.h"

#ifndef _USE_MC_PROTOCOL_
#ifndef TEST_RUN
#include "ActMulti.h"		// For ActEasyIF Contorol
#include "ActEther.h"		// For Ethernet Communication Contorol
#include "ActDefine.h"	// ACT Common Macro Header 

#include "ActMulti_i.c"	// For Ethernet Communication Contorol
#include "ActEther_i.c"	// ACT Common Macro Header 
#endif
#endif

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CCommPLC g_CommPLC;

template <class T> void SafeRelease(T **ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}


CCommPLC::CCommPLC()
{
	m_bOpened = FALSE;
#ifndef _USE_MC_PROTOCOL_
#ifndef TEST_RUN
	m_pActEasyIFIn = NULL;
	m_pActEasyIFOut = NULL;
#endif	
#endif
	InitializeCriticalSection(&m_csCommPLCIn);
	InitializeCriticalSection(&m_csCommPLCOut);

	m_csSpinLock = new CSimpleCriticalSection(2000); //20210924 Tkyuha -  GetBit 공유 문제 해결
}

CCommPLC::~CCommPLC()
{
	DeleteCriticalSection(&m_csCommPLCIn);
	DeleteCriticalSection(&m_csCommPLCOut);

	delete m_csSpinLock; //20210924 Tkyuha -  GetBit 공유 문제 해결

	m_bInitRead = FALSE;
	m_bRunPlc = FALSE;
}

BOOL CCommPLC::Init(int nStationIn, int nStationOut)
{
	m_pMain = (CLET_AlignClientDlg *)AfxGetMainWnd();
#ifndef _USE_MC_PROTOCOL_
#ifndef TEST_RUN
	if(m_bOpened) return TRUE;

	HRESULT hr;
	long	lRet;

	hr = CoCreateInstance(	CLSID_ActEasyIF, NULL, CLSCTX_INPROC_SERVER, IID_IActEasyIF, (LPVOID*)&m_pActEasyIFIn);
	if(hr != S_OK) 
	{
		return FALSE;
	}

	hr = CoCreateInstance(	CLSID_ActEasyIF, NULL, CLSCTX_INPROC_SERVER, IID_IActEasyIF, (LPVOID*)&m_pActEasyIFOut);
	if(hr != S_OK) 
	{
		return FALSE;
	}

	hr = m_pActEasyIFIn->put_ActLogicalStationNumber(nStationIn);	// Exec set-property method
	if(hr != S_OK) return FALSE;
	else {
		hr = m_pActEasyIFIn->Open(&lRet);
		if(hr != S_OK) 
		{
			return FALSE;
		}
	}
	
	hr = m_pActEasyIFOut->put_ActLogicalStationNumber(nStationOut);	// Exec set-property method
	if(hr != S_OK) return FALSE;
	else {
		hr = m_pActEasyIFOut->Open(&lRet);
		if(hr != S_OK) 
		{
			return FALSE;
		}
	}

	
	m_bOpened = TRUE;

	m_bRunPlc = TRUE;
#else
	return FALSE;
#endif
#endif

	return TRUE;
}

void CCommPLC::Close()
{
#ifndef _USE_MC_PROTOCOL_
#ifndef TEST_RUN
TRACE("CCommPLC::Close() 1\n");
	m_bRunPlc = FALSE;
	
	try{
		long lpInterfaceReturn;
		theLog.logmsg(LOG_PROCESS, "CCommPLC::Close() m_pActEasyIFIn Release");
		if( m_pActEasyIFIn != NULL ) 
		{
			m_pActEasyIFIn->Close(&lpInterfaceReturn);
			SafeRelease(&m_pActEasyIFIn);
		}

		TRACE("CCommPLC::Close() 5\n");
		theLog.logmsg(LOG_PROCESS, "CCommPLC::Close() m_pActEasyIFOut Release");
		if( m_pActEasyIFOut != NULL ) 
		{
			m_pActEasyIFOut->Close(&lpInterfaceReturn);
			SafeRelease(&m_pActEasyIFOut);
		}

		theLog.logmsg(LOG_PROCESS, "CCommPLC::Close() Release Complete");
		TRACE("CCommPLC::Close() 6\n");
		m_bOpened = FALSE;
	}
	catch(...)
	{
		theLog.logmsg(LOG_ERROR, "CCommPLC::Close() Fail Happen");
		m_bOpened = FALSE;
	}

#endif
#endif
}

BOOL CCommPLC::fnGetPLCStatus()
{
	return m_bRunPlc;
}

// Get 을 할경우 이더넷 모듈이 버벅거려서 전체를 읽어오고 
// Set 을 할경우는 개개별로 명령을 날린다
BOOL CCommPLC::GetBit(long nAddress)
{
	if( m_pMain == NULL ) return FALSE;
	BOOL bValue = 0;
#ifndef TEST_RUN
#ifndef _USE_MC_PROTOCOL_
	if(m_pActEasyIFIn != NULL)
	{
		EnterCriticalSection(&m_csCommPLCIn);
	
		int nDevice = (nAddress - m_pMain->getStartReadBitAddr()) / BIT_COUNT_PER_WORD;
		int nBit = (nAddress - m_pMain->getStartReadBitAddr()) % BIT_COUNT_PER_WORD;
		long nBitData = m_pMain->m_nBitData[nDevice];

		if(nBitData < 0 || nBitData > 65535)
		{
			LeaveCriticalSection(&m_csCommPLCIn);
			return FALSE;
		}

		bValue = nBitData >> nBit & 0x01;
		//	BOOL bValue = m_nBitData[nDevice] >> nBit & 0x01;
		LeaveCriticalSection(&m_csCommPLCIn);
	}
#else
	if (nAddress < m_pMain->getStartReadBitAddr() ||
		nAddress >  m_pMain->getStartReadBitAddr() + m_pMain->getSizeReadBit() * SIZE_PLC_WORD)
		return FALSE;

	m_csSpinLock->Lock();	//20210924 Tkyuha -  GetBit 공유 문제 해결

	int nDevice = (nAddress - m_pMain->getStartReadBitAddr()) / BIT_COUNT_PER_WORD;
	int nBit = (nAddress - m_pMain->getStartReadBitAddr()) % BIT_COUNT_PER_WORD;
	short nBitData = m_pMain->m_nBitData[nDevice];

	/*if(nBitData < 0 || nBitData > 65535)
		return FALSE;*/

	bValue = nBitData >> nBit & 0x01;

	m_csSpinLock->Unlock();	//20210924 Tkyuha -  GetBit 공유 문제 해결
#endif
#endif
	return bValue;
}

BOOL CCommPLC::GetWriteBit(long nAddress)
{
	if (m_pMain == NULL) return FALSE;
	BOOL bValue = 0;
#ifndef TEST_RUN
#ifndef _USE_MC_PROTOCOL_
	if (m_pActEasyIFIn != NULL)
	{
		EnterCriticalSection(&m_csCommPLCIn);

		int nDevice = (nAddress - m_pMain->getStartReadBitAddr()) / BIT_COUNT_PER_WORD;
		int nBit = (nAddress - m_pMain->getStartReadBitAddr()) % BIT_COUNT_PER_WORD;
		long nBitData = m_pMain->m_nBitData[nDevice];

		if (nBitData < 0 || nBitData > 65535)
		{
			LeaveCriticalSection(&m_csCommPLCIn);
			return FALSE;
		}

		bValue = nBitData >> nBit & 0x01;
		//	BOOL bValue = m_nBitData[nDevice] >> nBit & 0x01;
		LeaveCriticalSection(&m_csCommPLCIn);
}
#else

	if (nAddress < m_pMain->getStartWriteBitAddr() ||
		nAddress > m_pMain->getStartWriteBitAddr() + m_pMain->getSizeWriteBit() * SIZE_PLC_WORD)
		return FALSE;

	m_csSpinLock->Lock();	//20210924 Tkyuha -  GetBit 공유 문제 해결

	int nDevice = (nAddress - m_pMain->getStartWriteBitAddr()) / BIT_COUNT_PER_WORD;
	int nBit = (nAddress - m_pMain->getStartWriteBitAddr()) % BIT_COUNT_PER_WORD;
	short nBitData = m_pMain->m_nBitWriteData[nDevice];

	//if (nBitData < 0 || nBitData > 65535)
	//	return FALSE;

	bValue = nBitData >> nBit & 0x01;

	m_csSpinLock->Unlock();	//20210924 Tkyuha -  GetBit 공유 문제 해결
#endif
#endif
	return bValue;
}


BOOL CCommPLC::GetBit_Melsec(long nAddress)
{
	long bOnOff=0;
#ifndef TEST_RUN
#ifndef _USE_MC_PROTOCOL_
	long nReturnCode;
	CString strAddress;

	int nAddr1 = 0;
	strAddress.Format("L%d", nAddress );

	if(m_pActEasyIFIn != NULL)
	{
		EnterCriticalSection(&m_csCommPLCIn);
		BSTR bstr = strAddress.AllocSysString();
		m_pActEasyIFIn->GetDevice(bstr, &bOnOff, &nReturnCode); 
		::SysFreeString( bstr );
		LeaveCriticalSection(&m_csCommPLCIn);
	}
#else
	bOnOff = g_CommPLC_MC.fnReadBitL(nAddress);
#endif
#endif
	return bOnOff?TRUE:FALSE;
}

void CCommPLC::SetBit(long nAddress, BOOL bOnOff) 
{
#ifndef TEST_RUN
#ifndef _USE_MC_PROTOCOL_
	long nReturnCode;
	CString strAddress;
	strAddress.Format("L%d",nAddress);
	if(m_pActEasyIFOut != NULL)
	{
		EnterCriticalSection(&m_csCommPLCOut);
		BSTR bstr = strAddress.AllocSysString();
		m_pActEasyIFOut->SetDevice(bstr, (long)bOnOff, &nReturnCode); 
		::SysFreeString( bstr );
		LeaveCriticalSection(&m_csCommPLCOut);
	}
#else
	//g_CommPLC_MC.fnWriteBit(nAddress, bOnOff);
	g_CommPLC_MC.fnWriteBitM(nAddress, bOnOff);

	if( nAddress == PLC_WB_INSP_ACK_1ST_LAMI_INSPECTION )
	{
		CString strTemp;
		strTemp.Format("%d - %d", nAddress, bOnOff);
	}


#endif

#endif
}

void CCommPLC::SetBitL(long nAddress, BOOL bOnOff) 
{
#ifndef TEST_RUN
#ifndef _USE_MC_PROTOCOL_
	long nReturnCode;
	CString strAddress;
	strAddress.Format("L%d",nAddress);
	if(m_pActEasyIFOut != NULL)
	{
		EnterCriticalSection(&m_csCommPLCOut);
		BSTR bstr = strAddress.AllocSysString();
		m_pActEasyIFOut->SetDevice(bstr, (long)bOnOff, &nReturnCode); 
		::SysFreeString( bstr );
		LeaveCriticalSection(&m_csCommPLCOut);
	}
#else
	g_CommPLC_MC.fnWriteBitL(nAddress, bOnOff);
#endif

#endif
}

void CCommPLC::GetWord(long nAddress, long nDataSize, long *lpData)
{
#ifndef TEST_RUN

#ifndef _USE_MC_PROTOCOL_
	long nReturnCode;
	CString strAddress;
	strAddress.Format("D%d",nAddress);
	if(m_pActEasyIFIn!=NULL)
	{
		EnterCriticalSection(&m_csCommPLCIn);
		BSTR bstr = strAddress.AllocSysString();
		m_pActEasyIFIn->ReadDeviceBlock(bstr, nDataSize, lpData, &nReturnCode); 
		::SysFreeString(bstr);
		LeaveCriticalSection(&m_csCommPLCIn);
	}
#else
	g_CommPLC_MC.fnReadWord(nAddress, nDataSize, lpData);
#endif

#endif

}

void CCommPLC::SetWord(long nAddress, long nDataSize, long *lpData) 
{ 
#ifndef TEST_RUN
#ifndef _USE_MC_PROTOCOL_
	long nReturnCode;
	CString strAddress;
	strAddress.Format("D%d",nAddress);
	if(m_pActEasyIFOut!=NULL)
	{
		EnterCriticalSection(&m_csCommPLCOut);
		BSTR bstr = strAddress.AllocSysString();
		m_pActEasyIFOut->WriteDeviceBlock(bstr, nDataSize, lpData, &nReturnCode); 
		::SysFreeString(bstr);
		LeaveCriticalSection(&m_csCommPLCOut);
	}
#else
	g_CommPLC_MC.fnWriteWord(nAddress, nDataSize, lpData);
#endif

#endif
}


long CCommPLC::GetTotalWriteBitData(long nStarAddress, long nSize, long *lpData) 
{
	long nDataSize = nSize;
	long nReturnCode = 0;

#ifndef TEST_RUN
#ifndef _USE_MC_PROTOCOL_
	if(m_pActEasyIFIn != NULL)
	{
		EnterCriticalSection(&m_csCommPLCIn);
		CString strAddress;
		strAddress.Format("M%04d",nStarAddress);
		BSTR bstr = strAddress.AllocSysString();	
		m_pActEasyIFIn->ReadDeviceBlock(bstr, nDataSize, lpData, &nReturnCode);	
		::SysFreeString(bstr);
		LeaveCriticalSection(&m_csCommPLCIn);
	}
#else
	//g_CommPLC_MC.fnReadBitM(nStarAddress, nSize, lpData);
	g_CommPLC_MC.fnReadBitM(nStarAddress, nSize, lpData);
	m_bInitRead = TRUE;
#endif
#endif

	return nReturnCode;
}

long CCommPLC::GetTotalBitData(long nStarAddress, long nSize, long *lpData) 
{
	long nDataSize = nSize;
	long nReturnCode = 0;

#ifndef TEST_RUN
#ifndef _USE_MC_PROTOCOL_
	if(m_pActEasyIFIn != NULL)
	{
		EnterCriticalSection(&m_csCommPLCIn);
		CString strAddress;
		strAddress.Format("L%04d",nStarAddress);
		BSTR bstr = strAddress.AllocSysString();	
		m_pActEasyIFIn->ReadDeviceBlock(bstr, nDataSize, lpData, &nReturnCode);	
		::SysFreeString(bstr);
		LeaveCriticalSection(&m_csCommPLCIn);
	}
#else
	//g_CommPLC_MC.fnReadBit(nStarAddress, nSize, lpData);
	//g_CommPLC_MC.fnReadBitM(nStarAddress, nSize, lpData);
	g_CommPLC_MC.fnReadBitL(nStarAddress, nSize, lpData);
#endif
#endif

	return nReturnCode;
}

long CCommPLC::SetTotalBitData(long nStarAddress, long nSize, long *lpData) 
{
	long nDataSize = nSize;
	long nReturnCode=0;

#ifndef TEST_RUN
#ifndef _USE_MC_PROTOCOL_
	if(m_pActEasyIFOut!=NULL)
	{
		EnterCriticalSection(&m_csCommPLCOut);
		CString strAddress;
		strAddress.Format("L%04d",nStarAddress);
		BSTR bstr = strAddress.AllocSysString();
		m_pActEasyIFOut->WriteDeviceBlock(bstr, nDataSize, lpData, &nReturnCode);
		::SysFreeString(bstr);
		LeaveCriticalSection(&m_csCommPLCOut);
	}
#endif
#endif

	return nReturnCode;
}

void CCommPLC::fnReadDeviceBlock(BSTR nStarAddress, LONG nDataSize,
	LONG __RPC_FAR *lpData, LONG __RPC_FAR *lplReturnCode)
{
#ifndef _USE_MC_PROTOCOL_	
#ifndef TEST_RUN
	if(m_pActEasyIFIn != NULL)
	{
		EnterCriticalSection(&m_csCommPLCIn);
		m_pActEasyIFIn->ReadDeviceBlock(nStarAddress, nDataSize, lpData, lplReturnCode);
		LeaveCriticalSection(&m_csCommPLCIn);
	}
#endif
#endif	
}

long CCommPLC::GetTotalWordData(long nStarAddress, long nSize, long *lpData) 
{
	long nDataSize = nSize;
	long nReturnCode = 0;
#ifndef _USE_MC_PROTOCOL_
#ifndef TEST_RUN
	if(m_pActEasyIFIn != NULL)
	{
		EnterCriticalSection(&m_csCommPLCIn);
		CString strAddress;
		strAddress.Format("D%04d", nStarAddress);
		BSTR bstr = strAddress.AllocSysString();	
		m_pActEasyIFIn->ReadDeviceBlock(bstr, nDataSize, lpData, &nReturnCode);	
		::SysFreeString(bstr);	
		LeaveCriticalSection(&m_csCommPLCIn);
	}
#endif
#endif
	return nReturnCode;
}

long CCommPLC::SetTotalWordData(long nStarAddress, long nSize, long *lpData) 
{
	long nDataSize = nSize;
	long nReturnCode = 0;
#ifndef _USE_MC_PROTOCOL_
#ifndef TEST_RUN
	if(m_pActEasyIFOut!=NULL)
	{
		EnterCriticalSection(&m_csCommPLCOut);	
		CString strAddress;
		strAddress.Format("D%04d", nStarAddress);	
		BSTR bstr = strAddress.AllocSysString();
		m_pActEasyIFOut->WriteDeviceBlock(bstr, nDataSize, lpData, &nReturnCode);
		::SysAllocString(bstr);
		LeaveCriticalSection(&m_csCommPLCOut);
	}
#endif
#endif
	return nReturnCode;
}

void CCommPLC::SetWordZR(long nAddress, long nDataSize, long *lpData) 
{ 
#ifndef TEST_RUN
#ifndef _USE_MC_PROTOCOL_
	long nReturnCode;
	CString strAddress;
	strAddress.Format("ZR%d",nAddress);
	if(m_pActEasyIFOut!=NULL)
	{
		EnterCriticalSection(&m_csCommPLCOut);
		BSTR bstr = strAddress.AllocSysString();
		m_pActEasyIFOut->WriteDeviceBlock(bstr, nDataSize, lpData, &nReturnCode); 
		::SysFreeString(bstr);

		LeaveCriticalSection(&m_csCommPLCOut);
	}
#else
	g_CommPLC_MC.fnWriteWordZR(nAddress, nDataSize, lpData);
#endif
#endif
}

long CCommPLC::GetWordZR(long nAddress, long nDataSize, long *lpData)
{
	long nReturnCode=0;

#ifndef TEST_RUN
#ifndef _USE_MC_PROTOCOL_
	if(m_pActEasyIFIn != NULL)
	{
		EnterCriticalSection(&m_csCommPLCIn);
		CString strAddress;
		strAddress.Format("ZR%04d", nAddress);
		BSTR bstr = strAddress.AllocSysString();
		m_pActEasyIFIn->ReadDeviceBlock(bstr, nDataSize, lpData, &nReturnCode);
		::SysFreeString(bstr);
		LeaveCriticalSection(&m_csCommPLCIn);
	}
#else
	g_CommPLC_MC.fnReadWordZR(nAddress, nDataSize, lpData);
#endif
#endif
	return nReturnCode;
}

void CCommPLC::SetBitArray_ON(int num_of_addr, int* pAddr)
{
#ifndef TEST_RUN
#ifndef _USE_MC_PROTOCOL_
	long nReturnCode;
	CString strAddress;
	strAddress.Format("L%d", nAddress);
	if (m_pActEasyIFOut != NULL)
	{
		EnterCriticalSection(&m_csCommPLCOut);
		BSTR bstr = strAddress.AllocSysString();
		m_pActEasyIFOut->SetDevice(bstr, (long)bOnOff, &nReturnCode);
		::SysFreeString(bstr);
		LeaveCriticalSection(&m_csCommPLCOut);
	}
#else
	//g_CommPLC_MC.fnWriteBit(nAddress, bOnOff);
	g_CommPLC_MC.fnWriteBitArrayM_ON(num_of_addr, pAddr);
#endif

#endif
}

void CCommPLC::SetBitArray_OFF(int num_of_addr, int* pAddr)
{
#ifndef TEST_RUN
#ifndef _USE_MC_PROTOCOL_
	long nReturnCode;
	CString strAddress;
	strAddress.Format("L%d", nAddress);
	if (m_pActEasyIFOut != NULL)
	{
		EnterCriticalSection(&m_csCommPLCOut);
		BSTR bstr = strAddress.AllocSysString();
		m_pActEasyIFOut->SetDevice(bstr, (long)bOnOff, &nReturnCode);
		::SysFreeString(bstr);
		LeaveCriticalSection(&m_csCommPLCOut);
	}
#else
	//g_CommPLC_MC.fnWriteBit(nAddress, bOnOff);
	g_CommPLC_MC.fnWriteBitArrayM_OFF(num_of_addr, pAddr);
#endif

#endif
}
