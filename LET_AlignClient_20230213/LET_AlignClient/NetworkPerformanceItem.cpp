#include "stdafx.h"
#include "NetworkPerformanceItem.h"

NetworkPerformanceScanner::NetworkPerformanceScanner()
{

}


NetworkPerformanceScanner::~NetworkPerformanceScanner()
{
}


#include <memoryapi.h>
void NetworkPerformanceScanner::GetDiskSpace(CString drive,int &Total_Size,int &Free_Size)
{
    ULARGE_INTEGER avail, total, free;
    avail.QuadPart = 0L;
    total.QuadPart = 0L;
    free.QuadPart = 0L;

    CString strMsg;

    GetDiskFreeSpaceEx(drive, &avail, &total, &free);

    // GByte 로 표현을 하기 위한 부분
    Total_Size = (int)(total.QuadPart >> 30);
    Free_Size = (int)(free.QuadPart >> 30);

    strMsg.Format(_T("Total Size: %d GB , Free Size : %d GB\n"), Total_Size, Free_Size);
}

// Wrong algorithm, this is just virtual memory of application not the whole system memory
void NetworkPerformanceScanner::GetMemoryUsage(ULONG64& Total_Size, ULONG64& Free_Size)
{
    MEMORYSTATUSEX mem{ sizeof(MEMORYSTATUSEX),};
    GlobalMemoryStatusEx(&mem);
    Total_Size = mem.ullTotalPhys;
    Free_Size = mem.ullAvailPhys;
}

CpuUsage::CpuUsage(void): m_nCpuUsage(-1), m_dwLastRun(0), m_lRunCount(0)
{
    ZeroMemory(&m_ftPrevSysKernel, sizeof(FILETIME));
    ZeroMemory(&m_ftPrevSysUser, sizeof(FILETIME));
    
    ZeroMemory(&m_ftPrevProcKernel, sizeof(FILETIME));
    ZeroMemory(&m_ftPrevProcUser, sizeof(FILETIME));
}

short CpuUsage::GetUsage()
{
    short nCpuCopy = m_nCpuUsage;
    if (::InterlockedIncrement(&m_lRunCount) == 1)
    {        
        if (!EnoughTimePassed())
        {
            ::InterlockedDecrement(&m_lRunCount);
            return nCpuCopy;
        }
        
        FILETIME ftSysIdle, ftSysKernel, ftSysUser;
        FILETIME ftProcCreation, ftProcExit, ftProcKernel, ftProcUser;
        
        if (!GetSystemTimes(&ftSysIdle, &ftSysKernel, &ftSysUser) || !GetProcessTimes(GetCurrentProcess(), &ftProcCreation,
            &ftProcExit, &ftProcKernel, &ftProcUser))
        {
            ::InterlockedDecrement(&m_lRunCount);
            return nCpuCopy;
        }
        
        if (!IsFirstRun())
        {
            ULONGLONG ftSysKernelDiff =   SubtractTimes(ftSysKernel, m_ftPrevSysKernel);
            ULONGLONG ftSysUserDiff =   SubtractTimes(ftSysUser, m_ftPrevSysUser);            
            ULONGLONG ftProcKernelDiff =   SubtractTimes(ftProcKernel, m_ftPrevProcKernel);
            ULONGLONG ftProcUserDiff =    SubtractTimes(ftProcUser, m_ftPrevProcUser);            
            ULONGLONG nTotalSys = ftSysKernelDiff + ftSysUserDiff;
            ULONGLONG nTotalProc = ftProcKernelDiff + ftProcUserDiff;
            
            if (nTotalSys > 0)
            {
                m_nCpuUsage = (short)((100.0 * nTotalProc) / nTotalSys);
            }
        }
        
        m_ftPrevSysKernel = ftSysKernel;
        m_ftPrevSysUser = ftSysUser;
        m_ftPrevProcKernel = ftProcKernel;
        m_ftPrevProcUser = ftProcUser;        
        m_dwLastRun = GetTickCount64();        
        nCpuCopy = m_nCpuUsage;
    }
    
    ::InterlockedDecrement(&m_lRunCount);
    
    return nCpuCopy;
}

 ULONGLONG CpuUsage::SubtractTimes(const FILETIME & ftA, const FILETIME & ftB)
{
    LARGE_INTEGER a, b;
    a.LowPart = ftA.dwLowDateTime;
    a.HighPart = ftA.dwHighDateTime;
    
    b.LowPart = ftB.dwLowDateTime;
    b.HighPart = ftB.dwHighDateTime;
    
    return a.QuadPart - b.QuadPart;
}

 bool CpuUsage::EnoughTimePassed()
 {
    const int minElapsedMS = 250;//milliseconds
    
    ULONGLONG dwCurrentTickCount = GetTickCount64();
    return (dwCurrentTickCount - m_dwLastRun) > minElapsedMS;
}