// IniFile.h: interface for the CIniFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_INIFILE_H__F76636C8_6D25_4EE6_A6B1_6891C543FE44__INCLUDED_)
#define AFX_INIFILE_H__F76636C8_6D25_4EE6_A6B1_6891C543FE44__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class CIniFile  
{
public:
	inline LPCTSTR GetPathName() { return m_szFileName; };
	BOOL GetProfileStruct(LPCTSTR lpszSection, LPCTSTR lpKeyName, LPVOID lpStruct, UINT uSizeStruct);
	UINT GetProfileInt(LPCTSTR lpszSection, LPCTSTR lpKeyName, INT nDefault);
	DWORD GetProfileString(LPCTSTR lpszSection, LPCTSTR lpKeyName, LPCTSTR lpDefault, LPTSTR lpReturnedString, DWORD nSize);
	DWORD GetProfileSection(LPCTSTR lpszSection, LPTSTR lpReturnedString, DWORD nSize);
	DWORD GetProfileSectionNames(LPTSTR lpszReturnBuffer, DWORD nSize);
	BOOL WriteProfileStruct(LPCTSTR lpszSection, LPCTSTR lpKeyName, LPVOID lpStruct, UINT uSizeStruct);
	BOOL WriteProfileString(LPCTSTR lpszSection, LPCTSTR lpKeyName, LPCTSTR lpString);
	BOOL WriteProfileSection(LPCTSTR lpszSection, LPCTSTR lpString);
	
	BOOL WritePrivateProfileInt ( LPCTSTR lpszSection, LPCSTR lpKeyName, int nVal);
	BOOL WritePrivateProfileDouble (  LPCTSTR lpszSection,  LPCSTR lpKeyName,  double dVal);
	BOOL WritePrivateProfileBOOL ( LPCTSTR lpszSection,  LPCSTR lpKeyName, BOOL bVal );
	double GetPrivateProfileDouble(LPCTSTR lpszSection, LPCSTR lpKeyName,double nDefault=0.0  );	
	BOOL GetPrivateProfileBOOL(LPCTSTR lpszSection,LPCSTR lpKeyName,BOOL   bDefault=FALSE );

	void SetFileName(LPCTSTR lpFileName);

	CIniFile(LPCTSTR lpFileName);
	CIniFile();
	virtual ~CIniFile();

private:
	TCHAR m_szFileName[MAX_PATH];	//INI 파일이 저장되는 풀패스.
							//NULL일 경우 win.ini에 저장된다.
							//경로가 없을 경우 windows폴더에 저장된다.
};

#endif // !defined(AFX_INIFILE_H__F76636C8_6D25_4EE6_A6B1_6891C543FE44__INCLUDED_)
