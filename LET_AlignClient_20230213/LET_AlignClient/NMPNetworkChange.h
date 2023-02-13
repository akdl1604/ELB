#pragma once
class CNMPNetworkChange
{
public:
	CNMPNetworkChange();
	~CNMPNetworkChange();

	void ReturnTheStringFrmMultiSz(LPCTSTR String, int Size, CStringArray* arRetStr);
public:	
	
	BOOL ViewNumberOfCard(CStringArray* arCard, CStringArray* arCardName);
	void NMPReturnNONetwork(int& GateWay, int& IPAddress, int& DNSSubnet, int ver);		
	bool GetInterfaceNameOfCard(CStringArray* _interfaceName, CStringArray* _DeviceName);
	bool GetNumberOfCard();
	bool GetAddressOfMAC(CStringArray* arMacAdd);

	CString GetCurrentIPAdress(CString _DevieName);
	CString GetCurrentSubnetMask(CString _DevieName);
	CString GetCurrentGateWay(CString _DevieName);

	//this function send the current status in program
	BOOL ViewGateway(CStringArray& card, CStringArray& ipaddress);
	BOOL ViewSubnetMask(CStringArray& card, CStringArray& ipaddress);
	BOOL ViewDNSSubnet(CStringArray& card, CStringArray& ipaddress, int var);
	BOOL ViewIPAddress(CStringArray& card, CStringArray& ipaddress);
	void ViewGateWayMetrics(CStringArray& card, CStringArray& ipaddress);
	////function change the cuurnet status
	bool ChangeIPAddress(CString cardName, char newIPAddress[], char newMaskAddress[]= "255,255,255,0");
	BOOL ChangeGateway(CString card, CStringArray& ipaddress);
	BOOL ChangeDNSSubnet(CString card, CStringArray& ipaddress, int ver);
	BOOL ChangeSubnetMask(CString card, CStringArray& ipaddress);
	BOOL ChangeIpAddress(CString card, CStringArray& ipaddress);
	BOOL ChangeGateWayMetrics(CString card, CStringArray& ipaddress);

	CStringArray _interfaceName, _DeviceName;
	std::vector< std::string > _ip_Address, _subnet_Mask,_gate_Way;

private:
	std::vector< IP_ADAPTER_INFO> m_vecAdapterList;
};

