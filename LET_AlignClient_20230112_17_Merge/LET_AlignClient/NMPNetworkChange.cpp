#include "stdafx.h"
#include "NMPNetworkChange.h"
#include <netcon.h>

CNMPNetworkChange::CNMPNetworkChange()
{
}


CNMPNetworkChange::~CNMPNetworkChange()
{
	m_vecAdapterList.clear();
	_ip_Address.clear();
	_subnet_Mask.clear();
	_gate_Way.clear();

	_interfaceName.RemoveAll();
	_DeviceName.RemoveAll();	
}

bool CNMPNetworkChange::GetInterfaceNameOfCard(CStringArray* _interfaceName, CStringArray* _DeviceName)
{
	CString str;
	char Name[1024] = { 0, };
	HRESULT hr = E_FAIL;

	CoInitialize(NULL);

	INetConnectionManager* pNetConnectionManager = NULL;

	hr = CoCreateInstance(CLSID_ConnectionManager,	NULL,
		CLSCTX_LOCAL_SERVER | CLSCTX_NO_CODE_DOWNLOAD,
		IID_INetConnectionManager,(LPVOID*)& pNetConnectionManager);

	if (SUCCEEDED(hr))
	{
		/*Get an enumurator for the set of connections on the system */
		IEnumNetConnection* pEnumNetConnection;
		pNetConnectionManager->EnumConnections(NCME_DEFAULT, &pEnumNetConnection);
		ULONG ulCount = 0;
		BOOL fFound = FALSE;
		hr = HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND);
		HRESULT hrT = S_OK;

		do
		{
			NETCON_PROPERTIES* pProps = NULL;
			INetConnection* pConn;

			hrT = pEnumNetConnection->Next(1, &pConn, &ulCount);

			if (SUCCEEDED(hrT) && 1 == ulCount)
			{
				hrT = pConn->GetProperties(&pProps);

				if (S_OK == hrT)
				{
					WideCharToMultiByte(CP_ACP, 0, pProps->pszwName, -1, Name, 1024, NULL, NULL);
					str.Format("%s", Name);
					_interfaceName->Add(str);

					WideCharToMultiByte(CP_ACP, 0, pProps->pszwDeviceName, -1, Name, 1024, NULL, NULL);
					str.Format("%s", Name);
					_DeviceName->Add(str);

					CoTaskMemFree(pProps->pszwName);
					CoTaskMemFree(pProps->pszwDeviceName); CoTaskMemFree(pProps);
				}

				pConn->Release();
				pConn = NULL;
			}

		} while (SUCCEEDED(hrT) && 1 == ulCount && !fFound);

		if (FAILED(hrT))
		{
			hr = hrT;
		}

		pEnumNetConnection->Release();
	}

	if (FAILED(hr) && hr != HRESULT_FROM_WIN32(ERROR_RETRY))
	{
		return false;
	}

	pNetConnectionManager->Release();
	CoUninitialize();

	return true;
}

bool CNMPNetworkChange::GetNumberOfCard()
{
	CString str;

	IP_ADAPTER_INFO* pAdapterInfo;
	ULONG            ulOutBufLen;
	DWORD            dwRetVal;

	pAdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
	ulOutBufLen = sizeof(IP_ADAPTER_INFO);

	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) != ERROR_SUCCESS) {
		free(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
	}

	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) != ERROR_SUCCESS) {
		AfxMessageBox("GetAdaptersInfo call failed ");
	}

	IP_ADAPTER_INFO _adapter;
	PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
	while (pAdapter) {
		memcpy(&_adapter, pAdapter, sizeof(IP_ADAPTER_INFO));

		_ip_Address.push_back(_adapter.IpAddressList.IpAddress.String);
		_subnet_Mask.push_back(_adapter.IpAddressList.IpMask.String);
		_gate_Way.push_back(_adapter.GatewayList.IpAddress.String);

		m_vecAdapterList.push_back(_adapter);
		pAdapter = pAdapter->Next;
	}

	if (pAdapterInfo)
		free(pAdapterInfo);	

	return true;
}

bool CNMPNetworkChange::GetAddressOfMAC(CStringArray* arMacAdd)
{
	ULONG buffer_length = 0;

	if (::GetAdaptersInfo(NULL, &buffer_length) == ERROR_BUFFER_OVERFLOW) {
		CString str, adapter_info_string;
		char* p_adapter = new char[buffer_length];
		IP_ADAPTER_INFO* p_pos;

		if (GetAdaptersInfo((IP_ADAPTER_INFO*)p_adapter, &buffer_length) == ERROR_SUCCESS) {
			p_pos = (IP_ADAPTER_INFO*)p_adapter;
			while (p_pos != NULL) {
				adapter_info_string.Format("%s(%s) : ", p_pos->Description, p_pos->IpAddressList.IpAddress.String);

				for (unsigned int i = 0; i < p_pos->AddressLength; i++) {
					str.Format("%02X", (unsigned int)(p_pos->Address[i] & 0x000000FF));
					if (i < p_pos->AddressLength - 1) str += "-";
					adapter_info_string += str;
				}
				arMacAdd->Add(adapter_info_string);

				p_pos = p_pos->Next; // 다음 어뎁터를 위한 정보로 이동한다.
			}
		}

		delete[] p_adapter;
	}

	return true;
}

bool CNMPNetworkChange::ChangeIPAddress(CString cardName, char newIPAddress[], char newMaskAddress[])
{
	DWORD dwRetVal = 0;
	PIP_ADAPTER_INFO pAdapter = NULL;

	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	PIP_ADAPTER_INFO pAdapterInfo = (IP_ADAPTER_INFO*)malloc(sizeof(IP_ADAPTER_INFO));
	if (pAdapterInfo == NULL)
	{
		return false;
	}

	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
	{
		free(pAdapterInfo);
		pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
		if (pAdapterInfo == NULL)
		{
			return false;
		}
	}

	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR)
	{
		pAdapter = pAdapterInfo;
		while (pAdapter)
		{
			if (strcmp(cardName, pAdapter->Description) == 0)
			{
				IPAddr addr = inet_addr(newIPAddress);
				IPMask mask = inet_addr(newMaskAddress);
				ULONG context, instance;

				if (DeleteIPAddress(pAdapter->IpAddressList.Context) != NO_ERROR ||
					AddIPAddress(addr, mask, pAdapter->Index, &context, &instance) != NO_ERROR)
				{
					return false;
				}
				return true;
			}
			pAdapter = pAdapter->Next;
		}
		return false;
	}

	return true;
}

CString CNMPNetworkChange::GetCurrentIPAdress(CString _DevieName)
{
	CString ip = "0.0.0.0";

	if (m_vecAdapterList.size() <= 0) return ip;

	for (int i = 0; i < m_vecAdapterList.size(); i++)
	{
		if (strcmp(m_vecAdapterList[i].Description, _DevieName) == 0)
		{
			ip.Format("%s", _ip_Address[i].c_str());
			break;
		}
	}

	return ip;
}
CString CNMPNetworkChange::GetCurrentSubnetMask(CString _DevieName)
{
	CString ip = "255.255.255.0";

	if (m_vecAdapterList.size() <= 0) return ip;

	for (int i = 0; i < m_vecAdapterList.size(); i++)
	{
		if (strcmp(m_vecAdapterList[i].Description, _DevieName) == 0)
		{
			ip.Format("%s", _subnet_Mask[i].c_str());
			break;
		}
	}

	return ip;
}
CString CNMPNetworkChange::GetCurrentGateWay(CString _DevieName)
{
	CString ip = "0.0.0.0";

	if (m_vecAdapterList.size() <= 0) return ip;

	for (int i = 0; i < m_vecAdapterList.size(); i++)
	{
		if (strcmp(m_vecAdapterList[i].Description, _DevieName) == 0)
		{
			ip.Format("%s", _gate_Way[i].c_str());
			break;
		}
	}

	return ip;
}

BOOL CNMPNetworkChange::ChangeIpAddress(CString card, CStringArray& ipaddress)
{
	char* szValue = new char[100];
	CString str;
	DWORD pdw = 99;
	CRegKey key;

	str.Format("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards\\%s", card);

	if (key.Create(HKEY_LOCAL_MACHINE, str) == ERROR_SUCCESS)
	{
		key.QueryValue(szValue, "ServiceName", &pdw);
		key.Close();

		str.Format("SYSTEM\\CurrentControlSet\\Services\\TcpIp\\Parameters\\Interfaces\\%s", szValue);
		key.Create(HKEY_LOCAL_MACHINE, str);

		unsigned char keyname[600];
		int k = 0;
		for (int j = 0; j < ipaddress.GetSize(); j++)
		{
			for (int i = 0; i < ipaddress.GetAt(j).GetLength(); i++, k++)
				keyname[k] = ipaddress.GetAt(j).GetAt(i);
			keyname[k] = '\0';
			keyname[++k] = '\0';
		}

		keyname[++k] = '\0';
		keyname[++k] = '\0';

		if (::RegSetValueEx(key.m_hKey, "IPAddress", 0, REG_MULTI_SZ, keyname, k) != ERROR_SUCCESS)
		{
			key.Close();
			return FALSE;
		}
		key.Close();

	}
	delete[] szValue;
	return TRUE;

}

BOOL CNMPNetworkChange::ChangeSubnetMask(CString card, CStringArray & ipaddress)
{
	char szValue[100];
	CString str;
	DWORD pdw = 99;
	CRegKey key;
	str.Format("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards\\%s", card);

	if (key.Create(HKEY_LOCAL_MACHINE, str) == ERROR_SUCCESS)
	{
		key.QueryValue(szValue, "ServiceName", &pdw);
		key.Close();

		str.Format("SYSTEM\\CurrentControlSet\\Services\\TcpIp\\Parameters\\Interfaces\\%s", szValue);
		key.Create(HKEY_LOCAL_MACHINE, str);


		unsigned char keyname[600];
		int k = 0;
		for (int j = 0; j < ipaddress.GetSize(); j++)
		{
			for (int i = 0; i < ipaddress.GetAt(j).GetLength(); i++, k++)
				keyname[k] = ipaddress.GetAt(j).GetAt(i);
			keyname[k] = '\0';
			keyname[++k] = '\0';
		}

		keyname[++k] = '\0';
		keyname[++k] = '\0';

		if (::RegSetValueEx(key.m_hKey, "SubnetMask", 0, REG_MULTI_SZ, keyname, k) != ERROR_SUCCESS)
		{
			key.Close();
			return FALSE;
		}
		key.Close();

	}
	return TRUE;
}

BOOL CNMPNetworkChange::ChangeDNSSubnet(CString card, CStringArray & ipaddress, int Ver)
{
	char szValue[100];
	CString str;
	DWORD pdw = 99;
	CRegKey key;

	if (Ver == 0)//ie single user
	{
		str.Format("System\\CurrentControlSet\\Services\\VxD\\MSTCP");

		if (key.Open(HKEY_LOCAL_MACHINE, str) == ERROR_SUCCESS)
		{
			unsigned char keyname[600];
			int k = 0;

			for (int j = 0; j < ipaddress.GetSize(); j++)
			{
				for (int i = 0; i < ipaddress.GetAt(j).GetLength(); i++, k++)
					keyname[k] = ipaddress.GetAt(j).GetAt(i);

				keyname[k++] = ',';
			}

			keyname[--k] = '\0';

			if (::RegSetValueEx(key.m_hKey, "NameServer", 0, REG_SZ, keyname, k) != ERROR_SUCCESS)
			{
				key.Close();
				return FALSE;
			}
			key.Close();
		}
	}
	else
	{
		str.Format("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards\\%s", card);

		if (key.Create(HKEY_LOCAL_MACHINE, str) == ERROR_SUCCESS)
		{
			key.QueryValue(szValue, "ServiceName", &pdw);
			key.Close();

			str.Format("SYSTEM\\CurrentControlSet\\Services\\TcpIp\\Parameters\\Interfaces\\%s", szValue);
			key.Open(HKEY_LOCAL_MACHINE, str);

			unsigned char keyname[1000];
			int k = 0;

			for (int j = 0; j < ipaddress.GetSize(); j++)
			{
				for (int i = 0; i < ipaddress.GetAt(j).GetLength(); i++, k++)
					keyname[k] = ipaddress.GetAt(j).GetAt(i);

				keyname[k++] = ',';
			}

			keyname[--k] = '\0';

			if (::RegSetValueEx(key.m_hKey, "NameServer", 0, REG_SZ, keyname, k) != ERROR_SUCCESS)
			{
				key.Close();
				return FALSE;
			}
			key.Close();
		}
	}
	return TRUE;
}

BOOL CNMPNetworkChange::ChangeGateway(CString card, CStringArray & ipaddress)
{
	char szValue[100];
	CString str;
	DWORD pdw = 99;
	CRegKey key;

	str.Format("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards\\%s", card);

	if (key.Create(HKEY_LOCAL_MACHINE, str) == ERROR_SUCCESS)
	{
		key.QueryValue(szValue, "ServiceName", &pdw);
		key.Close();

		str.Format("SYSTEM\\CurrentControlSet\\Services\\TcpIp\\Parameters\\Interfaces\\%s", szValue);
		key.Create(HKEY_LOCAL_MACHINE, str);

		unsigned char keyname[600];
		int k = 0;
		for (int j = 0; j < ipaddress.GetSize(); j++)
		{
			for (int i = 0; i < ipaddress.GetAt(j).GetLength(); i++, k++)
				keyname[k] = ipaddress.GetAt(j).GetAt(i);
			keyname[k] = '\0';
			keyname[++k] = '\0';
		}

		keyname[++k] = '\0';
		keyname[++k] = '\0';

		if (::RegSetValueEx(key.m_hKey, "DefaultGateway", 0, REG_MULTI_SZ, keyname, k) != ERROR_SUCCESS)
		{
			key.Close();
			return FALSE;
		}
		key.Close();
	}
	return TRUE;
}

BOOL CNMPNetworkChange::ViewIPAddress(CStringArray & card, CStringArray & ipaddress)
{
	char* szValue = new char[600];
	CString str;
	DWORD pdw = 599;
	int i = 0;
	CRegKey key;

	for (int flag = 1; flag <= 100; flag++)
	{
		szValue[0] = NULL;
		pdw = 599;
		key.Close();

		str.Format("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards\\%d", flag);

		if (key.Open(HKEY_LOCAL_MACHINE, str, KEY_READ) == ERROR_SUCCESS)
		{
			key.QueryValue(szValue, "ServiceName", &pdw);
			key.Close();

			str.Format("SYSTEM\\CurrentControlSet\\Services\\TcpIp\\Parameters\\Interfaces\\%s", szValue);

			if (key.Open(HKEY_LOCAL_MACHINE, str, KEY_READ) != ERROR_SUCCESS)
			{
			}
			char* szValue1 = new char[2000];
			pdw = 1999;

			RegQueryValueEx(key.m_hKey,	TEXT("IPAddress"),	NULL,NULL,	(LPBYTE)szValue1,	&pdw);

			char* temp = new char[20];
			int j = 0;
			str.Format("%d", flag);

			for (i = 0; i < (int)pdw; i++)
			{
				if (szValue1[i] != NULL)
				{
					temp[j++] = szValue1[i];
				}
				else
				{
					temp[j] = NULL;
					if (strcmp(temp, "") != 0)
					{
						card.Add(str);
						ipaddress.Add(temp);
					}
					j = 0;
				}
			}
			delete[] temp;
			delete[] szValue1;
			key.Close();
		}
	}
	delete[] szValue;
	return TRUE;
}

BOOL CNMPNetworkChange::ViewDNSSubnet(CStringArray & card, CStringArray & ipaddress, int Ver)
{
	char* szValue = new char[600];
	CString str;
	DWORD pdw = 599;
	int i = 0;
	CRegKey key;
	
	if (Ver == 0)//means single user
	{
		str.Format("System\\CurrentControlSet\\Services\\VxD\\MSTCP");

		key.Open(HKEY_LOCAL_MACHINE, str, KEY_READ);

		char* szValue1 = new char[1000];
		pdw = 999;

		RegQueryValueEx(key.m_hKey,	TEXT("NameServer"),	NULL,NULL,(LPBYTE)szValue1,&pdw);
		char* temp = new char[20];

		int j = 0;
		str.Format("%d", 1);

		for (i = 0; i < (int)pdw; i++)
		{
			if ((szValue1[i] == ',') || (szValue1[i] == NULL))
			{
				temp[j] = NULL;
				if (strcmp(temp, "") != 0)
				{
					card.Add(str);
					ipaddress.Add(temp);
				}
				j = 0;
			}
			else
			{
				temp[j++] = szValue1[i];
			}
		}

		key.Close();
	}
	else
	{
		for (int flag = 1; flag <= 100; flag++)
		{
			szValue[0] = NULL;
			pdw = 599;
			key.Close();

			str.Format("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards\\%d", flag);

			if (key.Open(HKEY_LOCAL_MACHINE, str, KEY_READ) == ERROR_SUCCESS)
			{
				key.QueryValue(szValue, "ServiceName", &pdw);
				key.Close();

				str.Format("SYSTEM\\CurrentControlSet\\Services\\TcpIp\\Parameters\\Interfaces\\%s", szValue);

				if (key.Open(HKEY_LOCAL_MACHINE, str, KEY_READ) != ERROR_SUCCESS)
				{
				}

				char* szValue1 = new char[1000];
				pdw = 999;

				RegQueryValueEx(key.m_hKey,	TEXT("NameServer"),	NULL,	NULL,(LPBYTE)szValue1,	&pdw);

				char* temp = new char[20];

				int j = 0;
				str.Format("%d", flag);
				for (i = 0; i < (int)pdw; i++)
				{
					if ((szValue1[i] == ',') || (szValue1[i] == NULL))
					{
						temp[j] = NULL;
						if (strcmp(temp, "") != 0)
						{
							card.Add(str);
							ipaddress.Add(temp);
						}
						j = 0;
					}
					else
					{
						temp[j++] = szValue1[i];
					}
				}

				delete[] szValue1;
				delete[] temp;
				//CLOSING THE KEY
				key.Close();
			}
		}
	}
	delete[] szValue;
	return TRUE;
}

BOOL CNMPNetworkChange::ViewSubnetMask(CStringArray & card, CStringArray & ipaddress)
{
	char* szValue = new char[600];
	CString str;
	DWORD pdw = 599;
	int i = 0;
	CRegKey key;
	   
	for (int flag = 1; flag <= 100; flag++)
	{
		szValue[0] = NULL;
		pdw = 599;
		key.Close();

		str.Format("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards\\%d", flag);

		if (key.Open(HKEY_LOCAL_MACHINE, str, KEY_READ) == ERROR_SUCCESS)
		{
			key.QueryValue(szValue, "ServiceName", &pdw);
			key.Close();

			str.Format("SYSTEM\\CurrentControlSet\\Services\\TcpIp\\Parameters\\Interfaces\\%s", szValue);

			if (key.Open(HKEY_LOCAL_MACHINE, str, KEY_READ) != ERROR_SUCCESS)
			{
			}

			char* szValue1 = new char[2000];
			pdw = 1999;

			RegQueryValueEx(key.m_hKey,	TEXT("SubnetMask"),	NULL,NULL,(LPBYTE)szValue1,	&pdw);

			char* temp = new char[20];
			int j = 0;
			str.Format("%d", flag);

			for (i = 0; i < (int)pdw; i++)
			{
				if (szValue1[i] != NULL)
				{
					temp[j++] = szValue1[i];
				}
				else
				{
					temp[j] = NULL;
					if (strcmp(temp, "") != 0)
					{
						card.Add(str);
						ipaddress.Add(temp);
					}
					j = 0;
				}
			}
			delete[] temp;
			delete[] szValue1;
			key.Close();
		}
	}
	delete[] szValue;
	return TRUE;
}

BOOL CNMPNetworkChange::ViewGateway(CStringArray & card, CStringArray & ipaddress)
{
	char* szValue = new char[600];
	CString str;
	DWORD pdw = 599;
	int i = 0;
	CRegKey key;

	for (int flag = 1; flag <= 100; flag++)
	{
		szValue[0] = NULL;
		pdw = 599;
		key.Close();

		str.Format("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards\\%d", flag);

		if (key.Open(HKEY_LOCAL_MACHINE, str, KEY_READ) == ERROR_SUCCESS)
		{
			key.QueryValue(szValue, "ServiceName", &pdw);
			key.Close();

			str.Format("SYSTEM\\CurrentControlSet\\Services\\TcpIp\\Parameters\\Interfaces\\%s", szValue);

			if (key.Open(HKEY_LOCAL_MACHINE, str, KEY_READ) != ERROR_SUCCESS)
			{
			}

			char* szValue1 = new char[1000];
			pdw = 999;

			RegQueryValueEx(key.m_hKey,	TEXT("DefaultGateway"),	NULL,NULL,(LPBYTE)szValue1,&pdw);

			char* temp = new char[20];
			int j = 0;
			str.Format("%d", flag);

			for (i = 0; i < (int)pdw; i++)
			{
				if (szValue1[i] != NULL)
				{
					temp[j++] = szValue1[i];
				}
				else
				{
					temp[j] = NULL;
					if (strcmp(temp, "") != 0)
					{
						card.Add(str);
						ipaddress.Add(temp);
					}
					j = 0;
				}
			}
			delete[] temp;
			delete[] szValue1;
			key.Close();
		}
	}
	delete[] szValue;
	return TRUE;
}

void CNMPNetworkChange::NMPReturnNONetwork(int& GateWay, int& IPAddress, int& DNSSubnet, int ver)
{
	CStringArray card, ipaddress;

	if (ver == 0)		ViewDNSSubnet(card, ipaddress, 0);
	else		ViewDNSSubnet(card, ipaddress, 1);

	DNSSubnet = card.GetSize();

	card.RemoveAll();
	ipaddress.RemoveAll();

	////// gateway
	ViewGateway(card, ipaddress);
	GateWay = card.GetSize();

	////// ipaddress
	card.RemoveAll();
	ipaddress.RemoveAll();

	ViewIPAddress(card, ipaddress);
	IPAddress = card.GetSize();

	card.RemoveAll();
	ipaddress.RemoveAll();
}

//this function send the number 
BOOL CNMPNetworkChange::ViewNumberOfCard(CStringArray * arCard, CStringArray * arCardName)
{
	char* szValue = new char[600];
	CString str;
	DWORD pdw = 599;
	int i = 0;
	CRegKey key;

	for (int flag = 1; flag <= 100; flag++)
	{
		szValue[0] = NULL;
		pdw = 599;
		key.Close();
		//this flag variable check number of network card in computer

		str.Format("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards\\%d", flag);

		if (key.Open(HKEY_LOCAL_MACHINE, str, KEY_READ) == ERROR_SUCCESS)
		{
			//adding card
			str.Format("%d", flag);
			arCard->Add(str);
			//addingcard name
			key.QueryValue(szValue, "Description", &pdw);
			key.Close();

			arCardName->Add(CString(szValue));
		}
	}

	return TRUE;
}

//this function will return string for the value
void CNMPNetworkChange::ReturnTheStringFrmMultiSz(LPCTSTR String, int Size, CStringArray * arRetStr)
{
	char* temp = new char[20];
	int j = 0;

	for (int i = 0; i < (int)Size; i++)
	{
		if (String[i] != NULL)
		{
			temp[j++] = String[i];
		}
		else
		{
			temp[j] = NULL;
			if (strcmp(temp, "") != 0)
			{
				arRetStr->Add(temp);
			}
			j = 0;
		}
	}
	delete[] temp;
}

//this function call the gateway application
void CNMPNetworkChange::ViewGateWayMetrics(CStringArray & card, CStringArray & ipaddress)
{
	char* szValue = new char[600];
	CString str;
	DWORD pdw = 599;
	int i = 0;
	CRegKey key;
	
	for (int flag = 1; flag <= 100; flag++)
	{
		szValue[0] = NULL;
		pdw = 599;
		key.Close();
		//this flag variable check number of network card in computer

		str.Format("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards\\%d", flag);

		if (key.Open(HKEY_LOCAL_MACHINE, str, KEY_READ) == ERROR_SUCCESS)
		{
			key.QueryValue(szValue, "ServiceName", &pdw);
			key.Close();
			
			str.Format("SYSTEM\\CurrentControlSet\\Services\\TcpIp\\Parameters\\Interfaces\\%s", szValue);

			if (key.Open(HKEY_LOCAL_MACHINE, str, KEY_READ) != ERROR_SUCCESS)
			{
			}
			char* szValue1 = new char[1000];
			pdw = 999;
			
			RegQueryValueEx(key.m_hKey,	TEXT("DefaultGatewayMetric"),NULL,NULL,	(LPBYTE)szValue1,&pdw);

			char* temp = new char[20];
			int j = 0;
			str.Format("%d", flag);

			for (i = 0; i < (int)pdw; i++)
			{
				if (szValue1[i] != NULL)
				{
					temp[j++] = szValue1[i];
				}
				else
				{
					temp[j] = NULL;
					if (strcmp(temp, "") != 0)
					{

						card.Add(str);
						ipaddress.Add(temp);
					}
					j = 0;
				}
			}
			delete[] temp;
			delete[] szValue1;
			key.Close();
		}
	}
	delete[] szValue;
	return;
}

BOOL CNMPNetworkChange::ChangeGateWayMetrics(CString card, CStringArray & ipaddress)
{
	char* szValue = new char[100];
	CString str;
	DWORD pdw = 99;
	CRegKey key;

	str.Format("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\NetworkCards\\%s", card);

	if (key.Create(HKEY_LOCAL_MACHINE, str) == ERROR_SUCCESS)
	{
		key.QueryValue(szValue, "ServiceName", &pdw);
		key.Close();

		str.Format("SYSTEM\\CurrentControlSet\\Services\\TcpIp\\Parameters\\Interfaces\\%s", szValue);
		key.Create(HKEY_LOCAL_MACHINE, str);

		unsigned char keyname[600];
		int k = 0;

		for (int j = 0; j < ipaddress.GetSize(); j++)
		{
			for (int i = 0; i < ipaddress.GetAt(j).GetLength(); i++, k++)
				keyname[k] = ipaddress.GetAt(j).GetAt(i);

			keyname[k] = '\0';
			keyname[++k] = '\0';
		}

		keyname[++k] = '\0';
		keyname[++k] = '\0';

		if (::RegSetValueEx(key.m_hKey, "DefaultGatewayMetric", 0, REG_MULTI_SZ, keyname, k) != ERROR_SUCCESS)
		{
			key.Close();
			return FALSE;
		}

		key.Close();
	}
	delete[] szValue;
	return TRUE;
}