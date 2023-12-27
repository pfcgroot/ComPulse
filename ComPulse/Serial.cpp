#include "StdAfx.h"
#include "Serial.h"

Serial::Serial(void)
{
	m_hDevice = INVALID_HANDLE_VALUE;
	ZeroMemory(&m_dcb,sizeof(m_dcb));
}

Serial::~Serial(void)
{
	Disconnect();
}

HRESULT Serial::Connect(const char* szPort)
{
	int msec_per_byte = 0;
	DWORD hr = ERROR_SUCCESS;

	if (m_hDevice!=INVALID_HANDLE_VALUE)
		return ERROR_ACCESS_DENIED;

	char buf[256];
	if (szPort[0]!='\\')
		strcpy_s(buf, sizeof(buf), "\\\\.\\"); 
	strcat_s(buf,sizeof(buf), szPort);
	if(strstr(szPort, "baud")==0)
		strcat_s(buf,sizeof(buf)," baud=9600 parity=N data=8 stop=1"); 

	// first part of string must be device name (i.e. \\.\COM1)
	size_t pos = strcspn(buf," \t;,");
	if (pos<strlen(buf))
		buf[pos++] = '\0';
	else
		pos=0;

	// open I/O handle to COM device
	m_hDevice = CreateFile(
		buf,							// pointer to name of the file (device)
		GENERIC_READ|GENERIC_WRITE,		// access (read-write) mode 
		0,								// share mode 
		NULL,							// pointer to security descriptor 
		OPEN_EXISTING,					// how to create 
		FILE_ATTRIBUTE_NORMAL,			// file attributes: enable async. I/O
		0								// handle to file with attributes to copy  
		);

	if (INVALID_HANDLE_VALUE==m_hDevice)
	{
		//MessageBox(NULL,buf,"cannot open",MB_OK);
		hr = GetLastError();
		goto exit;
	}

	if (!BuildCommDCB(buf+pos, &m_dcb)) 
	{
		hr = GetLastError();
		goto exit;
	}
	m_dcb.fAbortOnError = 0;
	if (!SetCommState(m_hDevice, &m_dcb))
	{
		hr = GetLastError();
		goto exit;
	}
	return hr;

exit:
	Disconnect();
	return hr;
}

void Serial::Disconnect()
{
	if (m_hDevice!=INVALID_HANDLE_VALUE)
	{
		CloseHandle(m_hDevice);
		m_hDevice = INVALID_HANDLE_VALUE;
	}
}

bool Serial::IsConnected()
{
	return (m_hDevice!=INVALID_HANDLE_VALUE);
}


const DCB& Serial::GetState()
{
	if (!GetCommState(m_hDevice, &m_dcb))
		ZeroMemory(&m_dcb,sizeof(m_dcb));
	return m_dcb;
}

bool Serial::SetState(const DCB& dcb)
{
	m_dcb = dcb;
	if (!SetCommState(m_hDevice, &m_dcb))
	{
		ZeroMemory(&m_dcb,sizeof(m_dcb));
		return false;
	}
	return true;
}

bool Serial::SetDTR(DWORD flow)
{
//	m_dcb.fRtsControl = RTS_CONTROL_ENABLE; // clear bit to set high (+10V)
	m_dcb.fDtrControl = flow; // DTR_CONTROL_DISABLE==set bit to set low (-10V)
	return SetState(m_dcb);
}

/*
// AmsSerial.cpp : Defines the entry point for the DLL application.
//

#include "stdafx.h"
#include "IO_RS232.h"
#pragma comment( lib, "version" )	// for GetFileVersionInfoSize() etc.
#include <stdio.h> // for sprintf_s and sscanf in GetDllVersion
#include <stdlib.h> // for _MAX_PATH
#include <set> // for storing open port handles

#ifdef _MANAGED
#pragma managed(push, off)
#endif

//static HANDLE _hDevice = INVALID_HANDLE_VALUE; 
static std::set<HANDLE> _handles;
static HMODULE _hDllModule = NULL;

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH: 
		_hDllModule = hModule;
		break;
	case DLL_PROCESS_DETACH: 
		IO_RS232_Disconnect(INVALID_HANDLE_VALUE); // close all
		break;
	}
    return TRUE;
}

extern "C"
{
	IO_RS232_DLLX BOOL DLLENTRY IO_RS232_IsConnected(HANDLE hDevice)
	{
		BOOL b = FALSE;
		if (hDevice!=INVALID_HANDLE_VALUE)
			b = _handles.find(hDevice) != _handles.end();
		return b;
	}

	IO_RS232_DLLX VOID DLLENTRY IO_RS232_Disconnect(HANDLE hDevice)
	{
		if (hDevice!=INVALID_HANDLE_VALUE)
		{
			CloseHandle(hDevice);
			_handles.erase(hDevice);
		}
		else
		{
			for (std::set<HANDLE>::iterator i=_handles.begin(); i!=_handles.end(); i++)
				CloseHandle(*i);
			_handles.clear();
		}
	}

	IO_RS232_DLLX HANDLE DLLENTRY IO_RS232_Connect(LPCSTR szPort / *\\.\COM1 baud=1200 parity=N data=8 stop=1 * /)
	{
	}

	IO_RS232_DLLX DWORD DLLENTRY IO_RS232_SendBuffer(HANDLE hDevice, LPCSTR pBuf, INT nBytes)
	{
		DWORD nBytesWritten = 0;
		DWORD hr = ERROR_SUCCESS;
	#if COUT_LEVEL>1
		int i;
	#endif

		if (hDevice==INVALID_HANDLE_VALUE)
		{
			hr = ERROR_DEVICE_NOT_CONNECTED;
			goto _exit;
		}

		if (!WriteFile(hDevice, pBuf, nBytes, &nBytesWritten, NULL))
		{
			hr = GetLastError();
			goto _exit;
		}

		if (nBytesWritten!=nBytes)
		{
	#if COUT_LEVEL>0
			std::cout << "DID NOT SEND #bytes:" << nBytes-nBytesWritten << std::endl;
	#endif
			hr = ERROR_BAD_LENGTH;
			goto _exit;
		}

	#if COUT_LEVEL>1
		std::cout << "send stream: "  << std::endl;
		char buf [256];
		for (i=0; i<nBytes; i++)
		{
			sprintf(buf,"%02x",(int)pBuf[i]);
			std::cout << buf;
		}
		std::cout << std::endl;
		std::cout.setf(std::ios::dec);
	#endif

		return ERROR_SUCCESS;

	_exit:
		return hr; // NO Error() in non-interface functions!!!
	}

	// virtual
	IO_RS232_DLLX DWORD DLLENTRY IO_RS232_ReceiveBuffer(HANDLE hDevice, LPSTR pBuf, INT nBytes)
	{
		DWORD nBytesRead = 0;
		DWORD hr = ERROR_SUCCESS;
	#if COUT_LEVEL>1
		int i;
	#endif

		if (hDevice==INVALID_HANDLE_VALUE)
		{
			hr = ERROR_DEVICE_NOT_CONNECTED;
			goto _exit;
		}


		if (!ReadFile(hDevice, pBuf, nBytes, &nBytesRead, NULL))
		{
			hr = GetLastError();
			goto _exit;
		}

		if (nBytesRead!=nBytes)
		{
	#if COUT_LEVEL>0
			if (nBytesRead==0)
				std::cout << "NOTHING RECEIVED!   requested #" << nBytes << std::endl;
			else
				std::cout << "MISSING #bytes:" << nBytes-nBytesRead << std::endl;
	#endif
			hr = ERROR_BAD_LENGTH;
			goto _exit;
		}

	#if COUT_LEVEL>1
		std::cout << "received stream: "  << std::endl;
		char buf [256];
		for (i=0; i<nBytes; i++)
		{	sprintf(buf,"%02x",(int)pBuf[i]);
			std::cout << buf;
		}
		std::cout << std::endl;
		std::cout.setf(std::ios::dec);
	#endif

		return ERROR_SUCCESS;

	_exit:
		return hr; // NO Error() in non-interface functions!!!
	}

	IO_RS232_DLLX VOID DLLENTRY IO_RS232_ResetSerial(HANDLE hDevice)
	{
		if (hDevice==INVALID_HANDLE_VALUE)
			return;
		PurgeComm(hDevice,PURGE_TXABORT|PURGE_RXABORT|PURGE_TXCLEAR|PURGE_RXCLEAR);
		Sleep(10); 
	}

	IO_RS232_DLLX DWORD DLLENTRY IO_RS232_ReadShort(HANDLE hDevice, SHORT* s)
	{
		if (hDevice==INVALID_HANDLE_VALUE)
			return ERROR_DEVICE_NOT_CONNECTED;

		DWORD hr = IO_RS232_ReceiveBuffer(hDevice, (LPSTR)s,sizeof(SHORT));
	//	if (hr==ERROR_SUCCESS && _amsModel==1)
	//		*s = (((*s)&0x00FF)<<8) | (((*s)&0xFF00)>>8);

		return hr;
	}

	IO_RS232_DLLX DWORD DLLENTRY IO_RS232_ReadByte(HANDLE hDevice, CHAR* c)
	{
		if (hDevice==INVALID_HANDLE_VALUE)
			return ERROR_DEVICE_NOT_CONNECTED;

		return IO_RS232_ReceiveBuffer(hDevice, (LPSTR)c,sizeof(CHAR));
	}



} // extern "C"

*/
