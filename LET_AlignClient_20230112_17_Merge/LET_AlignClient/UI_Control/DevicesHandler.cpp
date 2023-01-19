#include "DevicesHandler.h"

#include "WinUtils.h"

#include <iostream>

#include <windows.h>
#include <initguid.h>
#include <devguid.h>
#include <cfgmgr32.h>
#include <setupapi.h>
#include <Iphlpapi.h>

#pragma comment(lib, "Iphlpapi.lib")
#pragma comment (lib, "setupapi.lib")

template<typename ... Args> 
std::string string_format(const std::string& format, Args ... args) 
{
	size_t size = snprintf(nullptr, 0, format.c_str(), args ...) + 1; // Extra space for '\0' 
	if (size <= 0) { throw std::runtime_error("Error during formatting."); } 
	std::unique_ptr<char[]> buf(new char[size]); 
	snprintf(buf.get(), size, format.c_str(), args ...); 
	return std::string(buf.get(), buf.get() + size - 1); // We don't want the '\0' inside 
}

//성공 1, 실패 0 리턴
BOOL GetIP(std::vector<std::string> &strIPArray, std::vector<std::string>& strIDescription)
{
	std::string strIPAddress;
	std::string strAdapterDescription;

	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;

	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	pAdapterInfo = new IP_ADAPTER_INFO[ulOutBufLen];

	if (pAdapterInfo == NULL)	return FALSE;

	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
	{
		delete pAdapterInfo;
		pAdapterInfo = new IP_ADAPTER_INFO[ulOutBufLen];
		if (pAdapterInfo == NULL)		return FALSE;
	}

	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR)
	{
		pAdapter = pAdapterInfo;
		while (pAdapter)
		{
			strIPAddress = string_format("%s", pAdapter->IpAddressList.IpAddress.String);
			strAdapterDescription = string_format("%s", pAdapter->Description);
			strIPArray.push_back(strIPAddress);
			strIDescription.push_back(strAdapterDescription);
			pAdapter = pAdapter->Next;
		}
	}

	delete pAdapterInfo;
	pAdapterInfo = NULL;
	

	return TRUE;
}

//성공 1, 실패 0 리턴
BOOL GetMacAddress(std::vector<std::string> strMacArray, std::string strSpecifiedIP)
{
	PIP_ADAPTER_INFO pAdapterInfo;
	PIP_ADAPTER_INFO pAdapter = NULL;
	DWORD dwRetVal = 0;

	ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
	pAdapterInfo = new IP_ADAPTER_INFO[ulOutBufLen];

	if (pAdapterInfo == NULL)	return FALSE;

	if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
	{
		delete pAdapterInfo;
		pAdapterInfo = new IP_ADAPTER_INFO[ulOutBufLen];
		if (pAdapterInfo == NULL)	return FALSE;
	}

	if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR)
	{
		pAdapter = pAdapterInfo;
		while (pAdapter)
		{
			std::string strMacAddress;
			strMacAddress = string_format("%02X:%02X:%02X:%02X:%02X:%02X",
				pAdapter->Address[0],pAdapter->Address[1],pAdapter->Address[2],
				pAdapter->Address[3],pAdapter->Address[4],pAdapter->Address[5]);

			std::string strIPAddress;
			strIPAddress = string_format("%s", pAdapter->IpAddressList.IpAddress.String);

			if (strSpecifiedIP.length() > 0)
			{
				if (strIPAddress.find(strSpecifiedIP) >= 0)
				{
					strMacArray.push_back(strMacAddress);
					break;
				}
			}
			else	strMacArray.push_back(strMacAddress);

			pAdapter = pAdapter->Next;
		}
	}

	delete pAdapterInfo;
	pAdapterInfo = NULL;
	return TRUE;
}

DevicesHandler::DevicesHandler()
{
	handle = SetupDiGetClassDevs(&GUID_DEVCLASS_NET, nullptr, nullptr, DIGCF_PRESENT);
}

DevicesHandler::~DevicesHandler()
{
	if (handle == INVALID_HANDLE_VALUE) {
		return;
	}

	if (!SetupDiDestroyDeviceInfoList(handle)) {
		return;
	}
}

bool DevicesHandler::enumerateNics(std::vector<std::string> &listadapter)
{
	if (handle == INVALID_HANDLE_VALUE)		return false;

	std::vector<std::string> strIPArray;
	std::vector<std::string> strIDescription;

	GetIP(strIPArray, strIDescription);

	SP_DEVINFO_DATA deviceInfoData;
	deviceInfoData.cbSize = sizeof(deviceInfoData);

	for (DWORD i = 0; SetupDiEnumDeviceInfo(handle, i, &deviceInfoData); ++i) {
		DWORD requiredSize;

		if (!SetupDiGetDeviceRegistryProperty(handle, &deviceInfoData, SPDRP_FRIENDLYNAME, nullptr, nullptr, 0, &requiredSize)) {
			const DWORD err = GetLastError();
			if (err != ERROR_INSUFFICIENT_BUFFER)				return false;
		}

		std::string name(requiredSize, 0);

		if (!SetupDiGetDeviceRegistryProperty(handle, &deviceInfoData, SPDRP_FRIENDLYNAME, nullptr, reinterpret_cast<PBYTE>(&name[0]), static_cast<DWORD>(name.size()), nullptr)) {
			return false;
		}

		int ij = 0;
		std::string cIp="-";
		for (const auto& elem : strIDescription)
		{
			if (elem.find(name.c_str()) != std::string::npos)			break;
			ij++;
		}
		if (strIPArray.size() > ij && strIPArray.size() > 0)
			cIp = strIPArray.at(ij);

		std::string listValue= string_format("ID=%d(%s) Name=",i, cIp.c_str())+ name;

		listadapter.push_back(listValue);
	}

	const DWORD err = GetLastError();
	if (err != ERROR_NO_MORE_ITEMS) 		return false;

	strIPArray.clear();
	strIDescription.clear();

	return true;
}

bool DevicesHandler::toggleNic(const unsigned long &index, const Action &action)
{
	if (!WinUtils::isRunningAsAdmin()) 		return false;
	if (handle == INVALID_HANDLE_VALUE) 		return false;

	SP_DEVINFO_DATA deviceInfoData;
	deviceInfoData.cbSize = sizeof(deviceInfoData);

	if (!SetupDiEnumDeviceInfo(handle, index, &deviceInfoData)) 		return false;

	SP_PROPCHANGE_PARAMS propChangeParams;

	switch (action) {
		case Disable:			propChangeParams.StateChange = DICS_DISABLE;			break;
		case Enable:			propChangeParams.StateChange = DICS_ENABLE;			break;
		case Toggle: {
			ULONG status, problem;
			const CONFIGRET ret = CM_Get_DevNode_Status(&status, &problem, deviceInfoData.DevInst, 0);
			if (ret != CR_SUCCESS)				return false;
			propChangeParams.StateChange = problem == CM_PROB_DISABLED ? DICS_ENABLE : DICS_DISABLE;
			break;
		}
		default:		return false;
	}

	propChangeParams.HwProfile = 0;
	propChangeParams.Scope = DICS_FLAG_CONFIGSPECIFIC;
	propChangeParams.ClassInstallHeader.InstallFunction = DIF_PROPERTYCHANGE;
	propChangeParams.ClassInstallHeader.cbSize = sizeof(propChangeParams.ClassInstallHeader);

	if (!SetupDiSetClassInstallParams(handle, &deviceInfoData, &propChangeParams.ClassInstallHeader, sizeof(propChangeParams))) 		return false;

	if (!SetupDiCallClassInstaller(DIF_PROPERTYCHANGE, handle, &deviceInfoData)) 
	{
		SetupDiSetClassInstallParams(handle, &deviceInfoData, nullptr, 0);
		return false;
	}

	return true;
}
