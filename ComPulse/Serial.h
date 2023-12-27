#pragma once

class Serial
{
public:
	Serial(void);
	~Serial(void);

	HRESULT Connect(const char* szPort);
	void Disconnect();
	bool IsConnected();

	const DCB& GetState();
	bool SetState(const DCB& dcb);

	//
	// DTR Control Flow Values.
	//
	//#define DTR_CONTROL_DISABLE    0x00
	//#define DTR_CONTROL_ENABLE     0x01
	//#define DTR_CONTROL_HANDSHAKE  0x02
	bool SetDTR(DWORD flow);

protected:
	HANDLE m_hDevice;
	DCB m_dcb;
};
