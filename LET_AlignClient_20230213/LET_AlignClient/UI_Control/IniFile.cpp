// IniFile.cpp: implementation of the CIniFile class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "BplDispenserServer.h"
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


BOOL CIniFile::WritePrivateProfileInt ( LPCTSTR lpszSection, LPCSTR lpKeyName, int nVal)
{
    char    szVal[50];
    itoa( nVal, szVal, 10 );
    return ::WritePrivateProfileString( lpszSection, lpKeyName, szVal, m_szFileName );
}

BOOL CIniFile::WritePrivateProfileDouble (  LPCTSTR lpszSection,  LPCSTR lpKeyName,  double dVal)
{
    char    szVal[50];
    sprintf( szVal, "%.10f", dVal );
    return ::WritePrivateProfileString( lpszSection, lpKeyName, szVal, m_szFileName );
}

double CIniFile::GetPrivateProfileDouble(LPCTSTR lpszSection, LPCSTR lpKeyName,double nDefault  )
{
    char    szVal[50]={0};
    double    dVal = nDefault;
    ::GetPrivateProfileString( lpszSection, lpKeyName, "", szVal, 50, m_szFileName );
    if ( strlen(szVal) > 0 )     sscanf( szVal, "%lf", &dVal );
    return dVal;
}

BOOL CIniFile::WritePrivateProfileBOOL ( LPCTSTR lpszSection,  LPCSTR lpKeyName, BOOL bVal )
{
    return ::WritePrivateProfileInt(lpszSection, lpKeyName, bVal, m_szFileName);
}

BOOL CIniFile::GetPrivateProfileBOOL(LPCTSTR lpszSection,LPCSTR lpKeyName,BOOL   bDefault )
{
    return ::GetPrivateProfileInt(lpszSection, lpKeyName, bDefault, m_szFileName);    
}
