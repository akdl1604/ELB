#include "stdafx.h"
#include "McPLC_Control.h"
#include <pthread.h> 
#include "mmsystem.h"
#include <iomanip>

#pragma comment (lib, "winmm")
pthread_mutex_t  mutex_t = PTHREAD_MUTEX_INITIALIZER;

void CALLBACK TimerFunction_PLC(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	McPLC_Control* obj = (McPLC_Control*)dwUser;
	obj->MMTimerHandler(uTimerID);
}

void McPLC_Control::StartTimer()
{
	TIMECAPS tc;
	timeGetDevCaps(&tc, sizeof(TIMECAPS));
	DWORD resolution = MIN(MAX(tc.wPeriodMin, 0), tc.wPeriodMax);
	timeBeginPeriod(resolution);

	// create the timer
	m_nTimerID = timeSetEvent(2000, resolution, (LPTIMECALLBACK)::TimerFunction_PLC, (DWORD_PTR)this, TIME_PERIODIC);
}
//----------------------------------------------------------------------------------------

void McPLC_Control::StopTimer()
{
	// destroy the timer
	if (m_nTimerID != NULL)
	{
		timeKillEvent(m_nTimerID);
		timeEndPeriod(2000);
	}
}
//----------------------------------------------------------------------------------------

void McPLC_Control::MMTimerHandler(UINT nIDEvent) // called every elTime milliseconds
{
	StopTimer();
	plc_connect(inAddr, inPort);
}

ULONG WINAPI McPLC_Control::ProcessEventThread(void* p_Param)
{
	McPLC_Control* p_This = (McPLC_Control*)p_Param;
	p_This->ProcessEvents();
	CloseHandle(p_This->mh_Thread);
	return 0;
}

void McPLC_Control::ProcessEvents()
{
	while (mb_End)
	{
		pthread_mutex_lock(&mutex_t);
		plc_read_bit_data(bit_rdevice, bit_rstartaddress, bit_rsize);
		pthread_mutex_unlock(&mutex_t);

		WaitForSingleObject(m_hWowEvent, 10);
	};
}

McPLC_Control::McPLC_Control(void)
{
	deviceMap = { {"SM", 0x91}, {"SD", 0xA9}, {"X", 0x9C}, {"Y", 0x9D}, {"M", 0x90}, {"L", 0x92}, {"F", 0x93}, {"V",0x94}, {"B", 0xA0}, {"D", 0xA8}, {"W", 0xB4},
				{"TS", 0xC1}, {"TC", 0xC0}, {"SS", 0xC7}, {"SC", 0xC6}, {"SN", 0xC8}, {"CS", 0xC4}, {"CC", 0xC3}, {"CN", 0xC5}, {"SB", 0xA1}, {"SW", 0xB5}, {"S", 0x98},
				{"DX", 0xA2}, {"DY", 0xA3}, {"Z", 0xCC}, {"R", 0xAF}, {"ZR", 0xB0} };

	bitmemdata = NULL;

	mh_Thread = NULL;
	mb_End = TRUE;
	m_nTimerID = NULL;

	bit_rdevice = "L";
	bit_wdevice = "M";
	word_rdevice = "R";
	word_wdevice = "D";

	bit_rstartaddress = 25000;
	bit_rsize = 200;

	mb_Connect = false;

	bitmemdata = new BYTE[bit_rsize];
	m_hWowEvent = CreateEvent(0, FALSE, FALSE, 0);
}


McPLC_Control::~McPLC_Control(void)
{
	mb_End = FALSE;
	WaitForSingleObject(m_hWowEvent, 2000);

	pthread_mutex_destroy(&mutex_t);
	if (bitmemdata != NULL) delete bitmemdata;

	CloseHandle(m_hWowEvent);
}

void McPLC_Control::SetReadSize(int add,int size)
{
	bit_rstartaddress = add;
	bit_rsize = size;

	if (bitmemdata != NULL) delete bitmemdata;
	bitmemdata = new BYTE[size];
}

void McPLC_Control::plc_connect(std::string address,int port) {
	// ip address,port number
	mb_Connect = false;
	inAddr = address;
	inPort = port;
	// WSAStartup
	DWORD u32_ID;
	WSADATA data;
	int err_startup = WSAStartup(MAKEWORD(2, 0), &data);
	if (err_startup != 0)  return;

	// sockaddr_in structure init
	struct sockaddr_in dstAddr;
	memset(&dstAddr, 0, sizeof(dstAddr));
	dstAddr.sin_port = htons((unsigned short)inPort);
	dstAddr.sin_family = AF_INET;
	const char* stdstr = inAddr.c_str();
	inet_pton(AF_INET, stdstr, &dstAddr.sin_addr.S_un.S_addr);

	dstSocket = socket(AF_INET, SOCK_STREAM, 0);

	struct timeval timeout; //setting timeout to socket
	timeout.tv_sec = 3;
	timeout.tv_usec = 0;
	setsockopt(dstSocket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)); //timeout ÁöÁ¤

	// connect
	int err_connect = connect(dstSocket, (struct sockaddr*)&dstAddr, sizeof(dstAddr));
	if (err_connect != 0)
	{
		StartTimer();
		return;
	}

	mh_Thread = CreateThread(0, 0, ProcessEventThread, this, 0, &u32_ID);

	mb_Connect = true;
}

void McPLC_Control::plc_connect_stop() 
{
	closesocket(dstSocket);
	WSACleanup();
}

bool McPLC_Control::plc_bit_status(int add)
{
	bool rv = false;
	int index = add - bit_rstartaddress;

	if (bit_rsize > index && index >= 0)
		rv =  bitmemdata[index] ? true : false;
	
	return rv;
}

int McPLC_Control::plc_read_word_data(std::string device, int DeviceNum) 
{
	int outWord=0;

	// generate send message
	std::stringstream hexnum, fillnum;
	int num01, num02, num03;
	hexnum << std::hex << DeviceNum;
	fillnum << std::setfill('0') << std::right << std::setw(6) << hexnum.str();
	std::string devnumstr = fillnum.str();

	std::istringstream(devnumstr.substr(0, 2)) >> std::hex >> num01;
	std::istringstream(devnumstr.substr(2, 2)) >> std::hex >> num02;
	std::istringstream(devnumstr.substr(4, 2)) >> std::hex >> num03;

	int dev = deviceMap[device];

	char sendmessage[] = {
		(char)0x50,(char)0x00,				
		(char)0x00,							
		(char)0xFF,							
		(char)0xFF,(char)0x03,				
		(char)0x00,							
		(char)0x0C,(char)0x00,				
		(char)0x20,(char)0x00,				
		(char)0x01,(char)0x04,				
		(char)0x00,(char)0x00,				
		(char)num03,(char)num02,(char)num01,
		(char)dev,
		(char)0x01,(char)0x00				
	};

	pthread_mutex_unlock(&mutex_t);
	// sned message
	int send_err = send(dstSocket, sendmessage, 21, 0);
	if (send_err == -1) { pthread_mutex_unlock(&mutex_t); return 0; }
	// receive response
	char buf2[1024];
	memset(buf2, 0, sizeof(buf2));
	int recv_err = recv(dstSocket, buf2, sizeof(buf2), 0);
	//int recv_err = socket_recv(dstSocket, buf2, );
	if (recv_err == -1) { pthread_mutex_unlock(&mutex_t); return 0; }

	pthread_mutex_unlock(&mutex_t);
	// check mc protocol level response
	char buffer[3];
	std::string retcode = "9999";
	std::string contents = "0000";
	std::string ret = "";
	for (int i = 0; i < recv_err; i++) {

		snprintf(buffer, sizeof(buffer), "%02X", buf2[i] & 0x000000FF);
		ret.append(buffer);
		if (i == 9) {
			std::string tmp = buffer;
			retcode.replace(2, 2, tmp);
		}
		if (i == 10) {
			std::string tmp = buffer;
			retcode.replace(0, 2, tmp);
		}
		if (i == 11) {
			std::string tmp = buffer;
			contents.replace(2, 2, tmp);
		}
		if (i == 12) {
			std::string tmp = buffer;
			contents.replace(0, 2, tmp);
		}
	}

	if (retcode != "0000")  return 0;
	else
	{
		outWord = PCLFileterLib::convertNum(contents);
	}

	return outWord;
}

int McPLC_Control::plc_read_word_data(std::string device, int DeviceNum, int* WordData, int nLength) {
	int outWord=0;

	// generate send message
	std::stringstream hexnum, fillnum;
	int num01, num02, num03;
	hexnum << std::hex << DeviceNum;
	fillnum << std::setfill('0') << std::right << std::setw(6) << hexnum.str();
	std::string devnumstr = fillnum.str();

	std::istringstream(devnumstr.substr(0, 2)) >> std::hex >> num01;
	std::istringstream(devnumstr.substr(2, 2)) >> std::hex >> num02;
	std::istringstream(devnumstr.substr(4, 2)) >> std::hex >> num03;

	int dev = deviceMap[device];

	char sendmessage[] = {
		(char)0x50,(char)0x00,
		(char)0x00,
		(char)0xFF,
		(char)0xFF,(char)0x03,
		(char)0x00,
		(char)0x0C,(char)0x00,
		(char)0x20,(char)0x00,
		(char)0x01,(char)0x04,
		(char)0x00,(char)0x00,
		(char)num03,(char)num02,(char)num01,
		(char)dev,
		(char)(nLength & 0xFF),(char)((nLength >> 8) & 0xFF)
	};
	pthread_mutex_unlock(&mutex_t);
	// sned message
	int send_err = send(dstSocket, sendmessage, 21, 0);
	if (send_err == -1) { pthread_mutex_unlock(&mutex_t); return 0; }

	// receive response
	char buf2[512];
	memset(buf2, 0, sizeof(buf2));
	int recv_err = recv(dstSocket, buf2, sizeof(buf2), 0);
	//int recv_err = socket_recv(dstSocket, buf2, );
	if (recv_err == -1) { pthread_mutex_unlock(&mutex_t); return 0; }

	pthread_mutex_unlock(&mutex_t);
	// check mc protocol level response
	char buffer[3];
	std::string retcode = "9999";
	std::string contents = "0000";
	std::string ret = "";
	for (int i = 0; i < recv_err; i++) {

		snprintf(buffer, sizeof(buffer), "%02X", buf2[i] & 0x000000FF);
		ret.append(buffer);
		if (i == 9) {
			std::string tmp = buffer;
			retcode.replace(2, 2, tmp);
		}
		if (i == 10) {
			std::string tmp = buffer;
			retcode.replace(0, 2, tmp);
		}
		if (i == 11) {
			std::string tmp = buffer;
			contents.replace(2, 2, tmp);
		}
		if (i == 12) {
			std::string tmp = buffer;
			contents.replace(0, 2, tmp);
		}
	}

	if (retcode != "0000")  return 0;
	else
	{
		for (int ni = 0; ni < nLength / 2; ni++) 
			WordData[ni] = buf2[11+ni * 2] | buf2[12+ni * 2] << 8;
	}

	return outWord;
}

void McPLC_Control::plc_write_word_data(std::string device, int DeviceNum, int WordData) {
	// generate send message
	std::stringstream hexnum, fillnum;
	int num01, num02, num03;
	hexnum << std::hex << DeviceNum;
	fillnum << std::setfill('0') << std::right << std::setw(6) << hexnum.str();
	std::string devnumstr = fillnum.str();

	std::istringstream(devnumstr.substr(0, 2)) >> std::hex >> num01;
	std::istringstream(devnumstr.substr(2, 2)) >> std::hex >> num02;
	std::istringstream(devnumstr.substr(4, 2)) >> std::hex >> num03;

	std::string word = PCLFileterLib::convertWord(WordData);
	int wordnum01, wordnum02;
	std::istringstream(word.substr(0, 2)) >> std::hex >> wordnum01;
	std::istringstream(word.substr(2, 2)) >> std::hex >> wordnum02;
	int dev = deviceMap[device];

	char sendmessage[] = {
		(char)0x50,(char)0x00,				
		(char)0x00,							
		(char)0xFF,							
		(char)0xFF,(char)0x03,				
		(char)0x00,	
		(char)0x0E,(char)0x00,				
		(char)0x20,(char)0x00,				
		(char)0x01,(char)0x14,				
		(char)0x00,(char)0x00,				
		(char)num03,(char)num02,(char)num01,
		(char)dev,							
		(char)0x01,(char)0x00,				
		(char)wordnum02,(char)wordnum01		
	};

	pthread_mutex_unlock(&mutex_t);
	// sned message
	int send_err = send(dstSocket, sendmessage, 23, 0);
	if (send_err == -1) { pthread_mutex_unlock(&mutex_t); return; }

	// receive response
	char buf2[512];
	memset(buf2, 0, sizeof(buf2));
	int recv_err = recv(dstSocket, buf2, sizeof(buf2), 0);
	//int recv_err = socket_recv(dstSocket, buf2, );
	if (recv_err == -1) { pthread_mutex_unlock(&mutex_t); return; }
	pthread_mutex_unlock(&mutex_t);
	// check mc protocol level response
	char buffer[3];
	std::string retcode = "9999";
	std::string ret = "";
	for (int i = 0; i < recv_err; i++) {
		//std::cout << std::hex << buf[i];
		snprintf(buffer, sizeof(buffer), "%02X", buf2[i] & 0x000000FF);
		ret.append(buffer);
		if (i == 9) {
			std::string tmp = buffer;
			retcode.replace(2, 2, tmp);
		}
		if (i == 10) {
			std::string tmp = buffer;
			retcode.replace(0, 2, tmp);
		}
	}
	if (retcode != "0000")  return;
}

void McPLC_Control::plc_write_word_data(std::string device, int DeviceNum, int* WordData, int nLength)
{
	// generate send message
	std::stringstream hexnum, fillnum;
	int num01, num02, num03;
	hexnum << std::hex << DeviceNum;
	fillnum << std::setfill('0') << std::right << std::setw(6) << hexnum.str();
	std::string devnumstr = fillnum.str();

	std::istringstream(devnumstr.substr(0, 2)) >> std::hex >> num01;
	std::istringstream(devnumstr.substr(2, 2)) >> std::hex >> num02;
	std::istringstream(devnumstr.substr(4, 2)) >> std::hex >> num03;

	int dev = deviceMap[device];

	char sendmessage[1024] = {
		(char)0x50,(char)0x00,
		(char)0x00,
		(char)0xFF,
		(char)0xFF,(char)0x03,
		(char)0x00,
		(char)((nLength+ 0x0C) & 0xFF),(char)(((nLength + 0x0C) >> 8) & 0xFF),
		(char)0x20,(char)0x00,
		(char)0x01,(char)0x14,
		(char)0x00,(char)0x00,
		(char)num03,(char)num02,(char)num01,
		(char)dev,
		(char)(nLength & 0xFF),(char)((nLength >> 8) & 0xFF),
	};

	for (int ni = 0; ni < nLength; ni++)
	{
		sendmessage[21 + ni] = (*(WordData + ni) >> 0) & 0x00FF;	// Data L
		sendmessage[22 + ni] = (*(WordData + ni) >> 8) & 0x00FF;	// Data H
	}

	pthread_mutex_unlock(&mutex_t);
	// sned message
	int send_err = send(dstSocket, sendmessage, 23, 0);
	if (send_err == -1) { pthread_mutex_unlock(&mutex_t); return; }

	// receive response
	char buf2[1024];
	memset(buf2, 0, sizeof(buf2));
	int recv_err = recv(dstSocket, buf2, sizeof(buf2), 0);
	//int recv_err = socket_recv(dstSocket, buf2, );
	if (recv_err == -1) { pthread_mutex_unlock(&mutex_t); return; }
	pthread_mutex_unlock(&mutex_t);
	// check mc protocol level response
	char buffer[3];
	std::string retcode = "9999";
	std::string ret = "";
	for (int i = 0; i < recv_err; i++) {
		snprintf(buffer, sizeof(buffer), "%02X", buf2[i] & 0x000000FF);
		ret.append(buffer);
		if (i == 9) {
			std::string tmp = buffer;
			retcode.replace(2, 2, tmp);
		}
		if (i == 10) {
			std::string tmp = buffer;
			retcode.replace(0, 2, tmp);
		}
	}
	if (retcode != "0000") return;
}

bool McPLC_Control::plc_read_bit_data(std::string device, int DeviceNum) {
	bool outBool;

	// generate send message      
	std::stringstream hexnum, fillnum;
	int num01, num02, num03;
	hexnum << std::hex << DeviceNum;
	fillnum << std::setfill('0') << std::right << std::setw(6) << hexnum.str();
	std::string devnumstr = fillnum.str();

	std::istringstream(devnumstr.substr(0, 2)) >> std::hex >> num01;
	std::istringstream(devnumstr.substr(2, 2)) >> std::hex >> num02;
	std::istringstream(devnumstr.substr(4, 2)) >> std::hex >> num03;
	int dev = deviceMap[device];

	char sendmessage[] = {
		(char)0x50,(char)0x00,				
		(char)0x00,							
		(char)0xFF,							
		(char)0xFF,(char)0x03,				
		(char)0x00,							
		(char)0x0C,(char)0x00,				
		(char)0x20,(char)0x00,				
		(char)0x01,(char)0x04,				
		(char)0x00,(char)0x00,				
		(char)num03,(char)num02,(char)num01, // address
		(char)dev,							 // read area
		(char)0x01,(char)0x00				// size
	};

	pthread_mutex_unlock(&mutex_t);
	// sned message
	int send_err = send(dstSocket, sendmessage, 21, 0);
	if (send_err == -1) { pthread_mutex_unlock(&mutex_t); return false; }

	// receive response
	char buf2[512];
	memset(buf2, 0, sizeof(buf2));
	int recv_err = recv(dstSocket, buf2, sizeof(buf2), 0);
	//int recv_err = socket_recv(dstSocket, buf2, );
	if (recv_err == -1) { pthread_mutex_unlock(&mutex_t); return false; }
	pthread_mutex_unlock(&mutex_t);
	// check mc protocol level response
	char buffer[3];
	std::string retcode = "9999";
	std::string contents = "0000";
	std::string ret = "";
	for (int i = 0; i < recv_err; i++) {
		snprintf(buffer, sizeof(buffer), "%02X", buf2[i] & 0x000000FF);
		ret.append(buffer);
		if (i == 9) {
			std::string tmp = buffer;
			retcode.replace(2, 2, tmp);
		}
		if (i == 10) {
			std::string tmp = buffer;
			retcode.replace(0, 2, tmp);
		}
		if (i == 11) {
			std::string tmp = buffer;
			contents.replace(2, 2, tmp);
		}
		if (i == 12) {
			std::string tmp = buffer;
			contents.replace(0, 2, tmp);
		}
	}
	if (retcode != "0000") return false;
	else
	{
		if (PCLFileterLib::convertNum(contents) == 1) {
			outBool = true;
		}
		else {
			outBool = false;
		}
	}

	return outBool;
}

bool McPLC_Control::plc_read_bit_data(std::string device, int DeviceNum,int nLength) {
	bool outBool=true;

	// generate send message      
	std::stringstream hexnum, fillnum;
	int num01, num02, num03;
	hexnum << std::hex << DeviceNum;
	fillnum << std::setfill('0') << std::right << std::setw(6) << hexnum.str();
	std::string devnumstr = fillnum.str();

	std::istringstream(devnumstr.substr(0, 2)) >> std::hex >> num01;
	std::istringstream(devnumstr.substr(2, 2)) >> std::hex >> num02;
	std::istringstream(devnumstr.substr(4, 2)) >> std::hex >> num03;
	int dev = deviceMap[device];

	char sendmessage[] = {
		(char)0x50,(char)0x00,
		(char)0x00,
		(char)0xFF,
		(char)0xFF,(char)0x03,
		(char)0x00,
		(char)0x0C,(char)0x00,
		(char)0x20,(char)0x00,
		(char)0x01,(char)0x04,
		(char)0x00,(char)0x00,
		(char)num03,(char)num02,(char)num01, // address
		(char)dev,							 // read area
		(char)(nLength & 0xFF),(char)((nLength >> 8) & 0xFF)				// size
	};

	pthread_mutex_unlock(&mutex_t);
	// sned message
	int send_err = send(dstSocket, sendmessage, 21, 0);
	if (send_err == -1) { pthread_mutex_unlock(&mutex_t); return false; }


	// receive response
	char buf2[512];
	memset(buf2, 0, sizeof(buf2));
	int recv_err = recv(dstSocket, buf2, sizeof(buf2), 0);
	//int recv_err = socket_recv(dstSocket, buf2, );
	if (recv_err == -1) { pthread_mutex_unlock(&mutex_t); return false; }
	pthread_mutex_unlock(&mutex_t);
	// check mc protocol level response
	char buffer[3];
	std::string retcode = "9999";
	std::string contents = "0000";
	std::string ret = "";
	for (int i = 0; i < recv_err; i++) {
		snprintf(buffer, sizeof(buffer), "%02X", buf2[i] & 0x000000FF);
		ret.append(buffer);
		if (i == 9) {
			std::string tmp = buffer;
			retcode.replace(2, 2, tmp);
		}
		if (i == 10) {
			std::string tmp = buffer;
			retcode.replace(0, 2, tmp);
		}
		if (i == 11) {
			std::string tmp = buffer;
			contents.replace(2, 2, tmp);
		}
		if (i == 12) {
			std::string tmp = buffer;
			contents.replace(0, 2, tmp);
		}
	}
	if (retcode != "0000")		return false;
	else
	{
		int inc = 0;
		char bitarray;

		for (int ni = 0; ni < nLength/2; ni++)
		{
			bitarray = *(buf2 + 11 + (ni * 2)) + ((*(buf2 + 12 + (ni * 2))) << 8);

			for (int j = 0; j < SIZE_PLC_WORD; j++)
					bitmemdata[inc++] = bitarray >> j & 0x1;
		}
	}

	return outBool;
}


void McPLC_Control::plc_write_bit_data(std::string device, int DeviceNum, bool bitData) {
	// generate send message
	std::stringstream hexnum, fillnum;
	int num01, num02, num03;
	hexnum << std::hex << DeviceNum;
	fillnum << std::setfill('0') << std::right << std::setw(6) << hexnum.str();
	std::string devnumstr = fillnum.str();

	std::istringstream(devnumstr.substr(0, 2)) >> std::hex >> num01;
	std::istringstream(devnumstr.substr(2, 2)) >> std::hex >> num02;
	std::istringstream(devnumstr.substr(4, 2)) >> std::hex >> num03;

	std::string word = PCLFileterLib::convertWord(bitData == true ? 1 : 0);
	int wordnum01, wordnum02;
	std::istringstream(word.substr(0, 2)) >> std::hex >> wordnum01;
	std::istringstream(word.substr(2, 2)) >> std::hex >> wordnum02;
	int dev = deviceMap[device];

	char sendmessage[] = {
		(char)0x50,(char)0x00,				
		(char)0x00,							
		(char)0xFF,							
		(char)0xFF,(char)0x03,				
		(char)0x00,							
		(char)0x0E,(char)0x00,				
		(char)0x20,(char)0x00,				
		(char)0x01,(char)0x14,				
		(char)0x00,(char)0x00,				
		(char)num03,(char)num02,(char)num01,
		(char)dev,							
		(char)0x01,(char)0x00,				
		(char)wordnum02,(char)wordnum01		
	};
	pthread_mutex_unlock(&mutex_t);
	// sned message
	int send_err = send(dstSocket, sendmessage, 23, 0);
	if (send_err == -1) { pthread_mutex_unlock(&mutex_t); return; }

	// receive response
	char buf2[512];
	memset(buf2, 0, sizeof(buf2));
	int recv_err = recv(dstSocket, buf2, sizeof(buf2), 0);
	//int recv_err = socket_recv(dstSocket, buf2, );
	if (recv_err == -1) { pthread_mutex_unlock(&mutex_t); return; }
	pthread_mutex_unlock(&mutex_t);
	// check mc protocol level response
	char buffer[3];
	std::string retcode = "9999";
	std::string ret = "";
	for (int i = 0; i < recv_err; i++) {
		snprintf(buffer, sizeof(buffer), "%02X", buf2[i] & 0x000000FF);
		ret.append(buffer);
		if (i == 9) {
			std::string tmp = buffer;
			retcode.replace(2, 2, tmp);
		}
		if (i == 10) {
			std::string tmp = buffer;
			retcode.replace(0, 2, tmp);
		}
	}
	if (retcode != "0000")  return;
}

int McPLC_Control::socket_recv(int socket, char* buffer, int size)
{
	int total_received=0;
	int received=0;

	assert(buffer);
	assert(size > 0);

#ifdef _UDEF
	while (size)
	{
		received = recv(socket, buffer, size, 0);

		if (received <= 0) break;

		total_received += received;
		size -= received;
		buffer += received;
	}

	return received > 0 ? total_received : received;
#else
	total_received = recv(socket, buffer, size, MSG_WAITALL);
	return total_received;
#endif
}