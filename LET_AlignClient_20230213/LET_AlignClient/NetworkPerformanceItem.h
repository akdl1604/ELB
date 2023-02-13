#pragma once
using namespace std;

#pragma comment(lib, "iphlpapi.lib")

class NetworkPerformanceItem
{
public:
    NetworkPerformanceItem() {};
    ~NetworkPerformanceItem() {};

    INT ProcessId;
    INT State;
    std::string LocalAddress;
    std::string RemoteAddress;
    int LocalPort;
    int RemotePort;
    LONG BytesOut;
    LONG BytesIn;
    LONG OutboundBandwidth;
    LONG InboundBandwidth;
    int Pass = 0;
    std::string CollectionTime;

};

class NetworkPerformanceScanner
{
public:
    NetworkPerformanceScanner();
    ~NetworkPerformanceScanner();

    void GetDiskSpace(CString drive, int& Total_Size, int& Free_Size);
    void GetMemoryUsage(ULONG64& Total_Size, ULONG64& Free_Size);
};

class CpuUsage
{
public:
    CpuUsage(void);

    short  GetUsage();
private:
    ULONGLONG SubtractTimes(const FILETIME & ftA, const FILETIME & ftB);
    bool EnoughTimePassed();
    inline bool IsFirstRun() const { return (m_dwLastRun == 0); }
    
    //system total times
    FILETIME m_ftPrevSysKernel;
    FILETIME m_ftPrevSysUser;
    
    //process times
    FILETIME m_ftPrevProcKernel;
    FILETIME m_ftPrevProcUser;
    
    short m_nCpuUsage;
    ULONGLONG m_dwLastRun;
    
    volatile LONG m_lRunCount;
};
