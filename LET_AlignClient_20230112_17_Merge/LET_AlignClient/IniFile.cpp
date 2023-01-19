// IniFile.cpp: implementation of the CIniFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include  "LET_AlignClient.h"
#include "IniFile.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CIniFile::CIniFile()
{
	::memset(m_szFileName, 0x00, MAX_PATH);
}

CIniFile::CIniFile(LPCTSTR lpFileName)
{
	::lstrcpy(m_szFileName, lpFileName);
}

CIniFile::~CIniFile()
{

}

void CIniFile::SetFileName(LPCTSTR lpFileName)
{
	::lstrcpy(m_szFileName, lpFileName);
}

BOOL CIniFile::WriteProfileSection(LPCTSTR lpszSection, LPCTSTR lpString)
{
	return ::WritePrivateProfileSection(lpszSection, lpString, m_szFileName);
}


BOOL CIniFile::WriteProfileString(LPCTSTR lpszSection, LPCTSTR lpKeyName, LPCTSTR lpString)
{
	return ::WritePrivateProfileString(lpszSection, lpKeyName, lpString, m_szFileName);
}

BOOL CIniFile::WriteProfileStruct(LPCTSTR lpszSection, LPCTSTR lpKeyName, LPVOID lpStruct, UINT uSizeStruct)
{
	return ::WritePrivateProfileStruct(lpszSection, lpKeyName, lpStruct, uSizeStruct, m_szFileName);
}

DWORD CIniFile::GetProfileSectionNames(LPTSTR lpszReturnBuffer, DWORD nSize)
{
	return ::GetPrivateProfileSectionNames(lpszReturnBuffer, nSize, m_szFileName);
}

DWORD CIniFile::GetProfileSection(LPCTSTR lpszSection, LPTSTR lpReturnedString, DWORD nSize)
{
	return ::GetPrivateProfileSection(lpszSection, lpReturnedString, nSize, m_szFileName);
}

DWORD CIniFile::GetProfileString(LPCTSTR lpszSection, LPCTSTR lpKeyName, LPCTSTR lpDefault, LPTSTR lpReturnedString, DWORD nSize)
{
	return ::GetPrivateProfileString(lpszSection, lpKeyName, lpDefault, lpReturnedString, nSize, m_szFileName);
}

UINT CIniFile::GetProfileInt(LPCTSTR lpszSection, LPCTSTR lpKeyName, INT nDefault)
{
	return ::GetPrivateProfileInt(lpszSection, lpKeyName, nDefault, m_szFileName);
}

BOOL CIniFile::GetProfileStruct(LPCTSTR lpszSection, LPCTSTR lpKeyName, LPVOID lpStruct, UINT uSizeStruct)
{
	return ::GetPrivateProfileStruct(lpszSection, lpKeyName, lpStruct, uSizeStruct, m_szFileName);
}
