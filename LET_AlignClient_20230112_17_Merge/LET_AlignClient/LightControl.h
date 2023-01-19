#pragma once
#include "afxwin.h"
#include "CommThread.h"

#ifndef CODE_STX
#define CODE_STX	2
#endif

#ifndef CODE_ETX
#define CODE_ETX	3
#endif

#define LIGHT_RECV_BUF_SIZE 2048

class CLightControl :	public CWnd
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

private:
	int m_nLightType;
	BOOL m_bRecvData;
	HWND m_hCommWnd;
	CCommThread m_Comm;
	CRITICAL_SECTION m_csSend;
public:
	CLightControl(void);
	~CLightControl(void);

	int m_WM_VIEW_CONTROL;
	int m_MSG_LIGHT_RECV_ACK;
	int m_nID;
	HANDLE m_hDummyEvent;
	char  m_rxBuff[LIGHT_RECV_BUF_SIZE];
	int   m_rxTag;

	BOOL m_bThreadEnd;

	CCommThread& GetComm() { return m_Comm; }
	BOOL OpenPort(int nPort, int nBaudrate);
	BOOL setBrightness(int nCh, int nValue);

	void setLightType(int nType) { m_nLightType = nType; }
	void setCommWnd(HWND hWnd) { m_hCommWnd = hWnd; }
	
	int getLightType() { return m_nLightType; }
	HWND getCommWnd() { return m_hCommWnd; }
	int set_light_value(int nChannel, int nValue);
	BOOL sendSerialData(CString str);

	friend UINT processReceiveLight(LPVOID pParam);
};

