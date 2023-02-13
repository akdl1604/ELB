#pragma once

#include <iomanip>
class PCLFileterLib
{
public:
	static std::string convertWord(short num) {
		std::ostringstream ss;
		ss << std::hex << num;
		std::string word = ss.str();
		std::stringstream fourword;
		fourword << std::setfill('0') << std::right << std::setw(4) << word;
		std::string fourword_str = fourword.str();
		return fourword_str;
	}
	static int convertNum(std::string word) {
		unsigned short x;
		std::stringstream ss;
		ss << std::hex << word;
		ss >> x;
		return static_cast<short>(x);
	}
};

class McPLC_Control
{
private:
	 std::map<std::string, int> deviceMap;

	 HANDLE            mh_Thread;
	 HANDLE            m_hWowEvent;
	 UINT              m_nTimerID;
	 BOOL              mb_End;

	 bool             mb_Connect;	 

	 void StartTimer();
	 void StopTimer();
	 
	 int socket_recv(int socket, char* buffer, int size);
	 static ULONG WINAPI ProcessEventThread(void* p_Param);	 
public:
	std::string inAddr;
	std::string bit_rdevice;
	std::string bit_wdevice;
	std::string word_rdevice;
	std::string word_wdevice;

	int inPort;
	int dstSocket;
	int bit_rstartaddress;
	int bit_rsize;

	BYTE*bitmemdata;

	McPLC_Control();
	~McPLC_Control();

	void ProcessEvents();
	void SetReadSize(int add, int size=200);  // 가장먼저 해줄것
	bool GetPLCStatus() {	return mb_Connect;	}
	void MMTimerHandler(UINT nIDEvent);

	void plc_connect(std::string address, int port);
	void plc_connect_stop();

	bool plc_bit_status(int add);

	int plc_read_word_data(std::string device,int DeviceNum);
	int plc_read_word_data(std::string device, int DeviceNum, int* WordData, int nLength);
	void plc_write_word_data(std::string device, int DeviceNum, int WordData);
	void plc_write_word_data(std::string device, int DeviceNum, int *WordData,int nLength);

	bool plc_read_bit_data(std::string device, int DeviceNum);
	bool plc_read_bit_data(std::string device, int DeviceNum, int nLength);
	void plc_write_bit_data(std::string device, int DeviceNum, bool bitData);
};

