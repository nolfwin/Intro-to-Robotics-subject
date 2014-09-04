
#include "SerialPort.h"

SerialPort::SerialPort()
{
	m_PortHandle	= INVALID_HANDLE_VALUE;
	wrRetries		= 10;
	rdRetries		= 15;
}

SerialPort::~SerialPort()
{
	Close();
}

bool SerialPort::Open(TCHAR* PortName, 
					  unsigned long BaudRate, 
					  unsigned char ByteSize, 
					  unsigned char Parity, 
					  unsigned char StopBits, 
					  unsigned long DesiredAccess)
{
	Close();
	m_PortHandle = CreateFile(PortName, DesiredAccess, 0, NULL, OPEN_EXISTING, 0, 0);
	if (m_PortHandle != INVALID_HANDLE_VALUE)
	{
		//dcb.DCBlength			= sizeof(dcb);
		if ( !GetCommState(m_PortHandle, &dcb) ) return false;
		dcb.ByteSize			= ByteSize;
		dcb.StopBits			= StopBits;
		dcb.Parity				= Parity;
		dcb.fOutxCtsFlow		= 0;
		dcb.fOutxDsrFlow		= 0;
		dcb.fBinary				= 1;
		dcb.fParity				= 0;
		dcb.fNull				= 0;
		dcb.fOutX				= 0;
		dcb.fInX				= 0;

		if ( !SetCommState(m_PortHandle, &dcb) )	{ Close(); return false; }
		if ( !ChangeBaudRate(BaudRate) )			{ Close(); return false; }
		if ( !SetHardwareControl(false) )			{ Close(); return false; }
		return true;
	}
	else
	{
		return false; // Use GetLastError() to know the reason
	}
}

void SerialPort::Close()
{
	if (m_PortHandle != INVALID_HANDLE_VALUE)
	{
		// disable event notification
		SetCommMask( m_PortHandle, 0 ) ;
		// drop DTR
		EscapeCommFunction( m_PortHandle, CLRDTR ) ;
		// purge any outstanding reads/writes and close device handle
		PurgeComm( m_PortHandle, PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR );

		CloseHandle(m_PortHandle);
		m_PortHandle = INVALID_HANDLE_VALUE;
	}
}

bool SerialPort::IsOpen()
{
	return (m_PortHandle != INVALID_HANDLE_VALUE);
}

bool SerialPort::ChangeBaudRate(int baudRate)
{
	if ( !GetCommState(m_PortHandle, &dcb) ) return false;
	dcb.BaudRate = baudRate;
	if ( !SetCommState(m_PortHandle, &dcb) ) return false;
	return true;
}
bool SerialPort::SetHardwareControl(bool hardwareControl)
{
	if ( !GetCommState(m_PortHandle, &dcb) ) return false;

	if (hardwareControl == 0) /* set control lines */
	{
		dcb.fRtsControl = RTS_CONTROL_ENABLE;
		dcb.fDtrControl = DTR_CONTROL_ENABLE;
	}
	else
	{
		dcb.fRtsControl = RTS_CONTROL_DISABLE;
		dcb.fDtrControl = DTR_CONTROL_DISABLE;
	}

	if ( !SetCommState(m_PortHandle, &dcb) ) return false;
	return true;
}

int SerialPort::ReadByte(char* buffer, unsigned int msWait)
{
	if (m_PortHandle == INVALID_HANDLE_VALUE) return false;

	COMSTAT stat;
	unsigned long ret;
	unsigned long lastTime;

	if (msWait > 0)
	{
		lastTime = timeGetTime();
		while (timeGetTime() - lastTime < msWait) 
		{
			if (!ClearCommError(m_PortHandle, &ret, &stat))
				return -1;
			if (stat.cbInQue < 1)
				Sleep(2);
			else
				break;
		}
	}
	if (!ClearCommError(m_PortHandle, &ret, &stat))
		return -1;
	if (stat.cbInQue == 0)
		return 0;

	if ( ReadFile(m_PortHandle, (void *)buffer, 1, &ret, NULL) )
		return (int)ret;
	return -1;
}

int SerialPort::Read(char* buffer, unsigned long bufferSize, unsigned int msWait)
{
	if (m_PortHandle == INVALID_HANDLE_VALUE) return false;

	COMSTAT stat;
	unsigned long ret;
	unsigned int numToRead;
	unsigned long lastTime;

	if (msWait > 0)
	{
		lastTime = timeGetTime();
		while (timeGetTime() - lastTime < msWait) 
		{
			if (!ClearCommError(m_PortHandle, &ret, &stat))
				return -1;
			if (stat.cbInQue < bufferSize)
				Sleep(2);
			else
				break;
		}
	}
	if (!ClearCommError(m_PortHandle, &ret, &stat))
		return -1;
	if (stat.cbInQue == 0)
		return 0;
	if (stat.cbInQue > bufferSize)
		numToRead = bufferSize;
	else
		numToRead = stat.cbInQue;

	if ( ReadFile(m_PortHandle, (void *)buffer, numToRead, &ret, NULL) )
		return (int)ret;
	return -1;
}

bool SerialPort::Write(const char* Buffer, unsigned long BufferSize)
{
	if (m_PortHandle == INVALID_HANDLE_VALUE) 
	{
		return false;
	}

	int triesLeft = wrRetries;
	unsigned long Res(0);
	int errorCode;

	while(triesLeft > 0) 
	{
		errorCode = WriteFile(m_PortHandle, Buffer, BufferSize, &Res, 0);

		if (errorCode == 0)		return false;
		if (Res == BufferSize)	return true;

		Buffer		+= Res;
		BufferSize	-= Res;
		triesLeft--;
	}
	return false;
}

bool SerialPort::Get_CD_State()
{
	if (m_PortHandle != INVALID_HANDLE_VALUE)
	{
		unsigned long ModemStat;
		if (GetCommModemStatus(m_PortHandle, &ModemStat))
		{
			return (ModemStat & MS_RLSD_ON) > 0; //Not sure
		}
		else 
		{
			return false;
		}
	}
	else 
	{
		return false;
	}
}

bool SerialPort::Get_CTS_State()
{
	if (m_PortHandle != INVALID_HANDLE_VALUE)
	{
		unsigned long ModemStat;
		if (GetCommModemStatus(m_PortHandle, &ModemStat))
		{
			return (ModemStat & MS_CTS_ON) > 0;
		}
		else 
		{
			return false;
		}
	}
	else 
	{
		return false;
	}
}

bool SerialPort::Get_DSR_State()
{ 
	if (m_PortHandle != INVALID_HANDLE_VALUE)
	{
		unsigned long ModemStat;
		if (GetCommModemStatus(m_PortHandle, &ModemStat))
		{
			return (ModemStat & MS_DSR_ON) > 0;
		}
		else 
		{
			return false;
		}
	}
	else 
	{
		return false;
	}
}

bool SerialPort::Get_RI_State()
{ 
	if (m_PortHandle != INVALID_HANDLE_VALUE)
	{
		unsigned long ModemStat;
		if (GetCommModemStatus(m_PortHandle, &ModemStat))
		{
			return (ModemStat & MS_RING_ON) > 0;
		}
		else 
		{
			return false;
		}
	}
	else 
	{
		return false;
	}
}

void SerialPort::Set_DTR_State(bool state)
{ 
	if (m_PortHandle != INVALID_HANDLE_VALUE)
	{
		EscapeCommFunction(m_PortHandle, (state?SETDTR:CLRDTR));
	}
}

void SerialPort::Set_RTS_State(bool state)
{ 
	if (m_PortHandle != INVALID_HANDLE_VALUE)
	{
		EscapeCommFunction(m_PortHandle, (state?SETRTS:CLRRTS));
	}
}
