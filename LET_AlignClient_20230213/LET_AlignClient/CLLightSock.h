#pragma once
#include "Socket.h"

const int BIT_SCR_RST_ETH = 0;
const int BIT_SCR_SAVE_RG = 4;

const int BIT_RCR_RST_TC = 5;
const int BIT_RCR_INIT_ETH = 2;

class CLLightSock
{
	enum _OP_CODE_ {
		OP_WRITE = 0,
		OP_READ,
	};

	enum _LPC_REGISTER_ {
		REG_SCR = 0x2C,				// System Control Register
		REG_RCR = 0x2D,				// Reset Control Register
		REG_SSR = 0x30,				// System Status Register
		RED_EEADR = 0x51,			// Ethernet Environment Address & Data Register
		REG_COTR_CNT = 0x60,		// Cross Over Trigger Register - Used Trigger Count
		REG_COTR_PAGE_CFG = 0x61,	// Cross Over Trigger Register - Page Configure
		REG_GENC_LIMIT = 0x74,		// Generate Clock Limit Register
		REG_GENC_CLOCK = 0x78,		// Generate Clock Clock Frequency Register
		REG_GENC_DUTY = 0x7A,		// Generate Clock Duty Register
		REG_GENC_ON_DELAY = 0x7C,	// Generate Clock Delay Register
	};

	enum _LPW_REGISTER_ {
		REG_CSVR = 0x21,			// Channel Select & Value Register

	};

public:
	CLLightSock();
	~CLLightSock();

private:
	BOOL m_bRecv;
	TCP::cSocket mi_Socket;
	TCP::cSocket::cHash<SOCKET, DWORD> mi_SocketList;
	HANDLE mh_Thread;
	SOCKET m_hSock;
	DWORD m_dwCurrentReceiveSize;
	DWORD m_nTimeout;
	UINT m_nTimerID;

	static ULONG WINAPI ProcessEventThread(void* p_Param);
	void ProcessEvents();
	DWORD SendData(char *pData, int nLength);
	CString CopyToString(char* s8_Buf, DWORD u32_Bytes);
	void ProcessReceivedDataNormal(TCP::cSocket::cMemory* pi_RecvMem);

public:
	int m_nID;
	HWND m_hMsgHandle;		// 메세지 전달할 핸들
	int m_WM_VIEW_CONTROL;
	int m_MSG_CONNECT;		// 연결 처리 메세지
	int m_MSG_DISCONNECT;	// 연결 해제 처리 메세지
	int m_MSG_RETURN_DATA;	// 데이터 리턴값 처리 메세지

	BOOL IsRecv() { return m_bRecv; }
	BOOL Connect(CString strSocketIp, int nSockPort);
	void CloseSockets();

	///////////////////////////////
	// COT 타입 컨트롤러 설정 함수
	int setCotCount(int nCount);	// 사용할 Trigger 개수 ( 1 ~  8 )	
	int restartEthernet();			// Ethernet Configure 레지스터에 반영된 값으로 시스템 리셋
	int saveRegistData();			// Register의 설정을 내부 메모리에 저장

	int setDefaultEthernet();
	int resetTriggerCount();
	int setTriggerPage(int nPage, int pOnOff[16], int pValue[16]);

	int setGenerateClockLimit(int nLimit);		// 출력 수를 지정할 수 있다.	0이면 제한 없고, 10이면, 트리거 입력이 들어오더라도 10개까지만 출력 됨.
	int setGenerateClockFrequency(int nFreq);	// 클럭 주파수를 설정.
	int setGenerateClockDuty(int nDuty);		// 한 펄스에 대한 High/Low 레벨의 비율을 설정.
	int setGenerateClockDelay(int nDelay);

	///////////////////////////////

	///////////////////
	// LPW 타입 컨트롤러 설정 함수
	int set_light_value(int nChannel, int nValue);
};

