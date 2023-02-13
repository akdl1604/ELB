#ifndef __INLINE_MODULE_H__
#define __INLINE_MODULE_H__

#ifndef _USE_MC_PROTOCOL_
#ifndef TEST_RUN
	#include "ActMulti.h"	// For ActEasyIF Contorol
#endif
#endif

#define BIT_COUNT_PER_WORD	16
class CLET_AlignClientDlg;

class CCommPLC
{
public:
	CCommPLC();
	~CCommPLC();

	BOOL Init(int nStationIn, int nStationOut);
	void Close();
	BOOL m_bOpened;
#ifndef _USE_MC_PROTOCOL_
#ifndef TEST_RUN
	IActEasyIF	*m_pActEasyIFIn;			// ACT Control (Custom Interface)
	IActEasyIF	*m_pActEasyIFOut;			// ACT Control (Custom Interface
#endif
#endif

public:
	BOOL fnGetPLCStatus();

	BOOL GetBit(long nAddress);
	BOOL GetWriteBit(long nAddress);
	BOOL GetBit_Melsec(long nAddress);
	void SetBit(long nAddress, BOOL bOnOff);
	void SetBitL(long nAddress, BOOL bOnOff);
	void GetWord(long nAddress, long nDataSize, long *lpData);
	void SetWord(long nAddress, long nDataSize, long *lpData);
	long GetWordZR(long nAddress, long nDataSize, long *lpData);
	void SetWordZR(long nAddress, long nDataSize, long *lpData);
	void fnReadDeviceBlock(BSTR szDevice, /* [in] */ LONG lSize,
		 /* [size_is][out] */ LONG __RPC_FAR *lplData,
            /* [retval][out] */ LONG __RPC_FAR *lplReturnCode = 0);
public:
	CString m_strCellID;
	BOOL m_bJudgeNG;
	BOOL m_bRunPlc;
	CLET_AlignClientDlg *m_pMain;

	long GetTotalWriteBitData(long nStarAddress, long nSize, long *lpData);
	long GetTotalBitData(long nStarAddress, long nEndAddress, long *lpData);
	long GetTotalWordData(long nStarAddress, long nEndAddress, long *lpData);
	long SetTotalBitData(long nStarAddress, long nEndAddress, long *lpData);
	long SetTotalWordData(long nStarAddress, long nEndAddress, long *lpData);

	void SetBitArray_OFF(int num_of_addr, int* pAddr);
	void SetBitArray_ON(int num_of_addr, int* pAddr);
//	CRITICAL_SECTION m_csCommPLCReadBit;

//	CRITICAL_SECTION m_csCommPLCInBit;
//	CRITICAL_SECTION m_csCommPLCInWord;
//	CRITICAL_SECTION m_csCommPLCOutBit;
//	CRITICAL_SECTION m_csCommPLCOutWord;


	CRITICAL_SECTION m_csCommPLCIn;
	CRITICAL_SECTION m_csCommPLCOut;

	CSimpleCriticalSection* m_csSpinLock;	//20210924 Tkyuha -  GetBit 공유 문제 해결

	BOOL m_bInitRead;
};

extern CCommPLC g_CommPLC;

#endif //__INLINE_MODULE_H__jm