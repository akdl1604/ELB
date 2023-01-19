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
	HWND m_hMsgHandle;		// �޼��� ������ �ڵ�
	int m_WM_VIEW_CONTROL;
	int m_MSG_CONNECT;		// ���� ó�� �޼���
	int m_MSG_DISCONNECT;	// ���� ���� ó�� �޼���
	int m_MSG_RETURN_DATA;	// ������ ���ϰ� ó�� �޼���

	BOOL IsRecv() { return m_bRecv; }
	BOOL Connect(CString strSocketIp, int nSockPort);
	void CloseSockets();

	///////////////////////////////
	// COT Ÿ�� ��Ʈ�ѷ� ���� �Լ�
	int setCotCount(int nCount);	// ����� Trigger ���� ( 1 ~  8 )	
	int restartEthernet();			// Ethernet Configure �������Ϳ� �ݿ��� ������ �ý��� ����
	int saveRegistData();			// Register�� ������ ���� �޸𸮿� ����

	int setDefaultEthernet();
	int resetTriggerCount();
	int setTriggerPage(int nPage, int pOnOff[16], int pValue[16]);

	int setGenerateClockLimit(int nLimit);		// ��� ���� ������ �� �ִ�.	0�̸� ���� ����, 10�̸�, Ʈ���� �Է��� �������� 10�������� ��� ��.
	int setGenerateClockFrequency(int nFreq);	// Ŭ�� ���ļ��� ����.
	int setGenerateClockDuty(int nDuty);		// �� �޽��� ���� High/Low ������ ������ ����.
	int setGenerateClockDelay(int nDelay);

	///////////////////////////////

	///////////////////
	// LPW Ÿ�� ��Ʈ�ѷ� ���� �Լ�
	int set_light_value(int nChannel, int nValue);
};

