
#ifndef _CREATECONNECTOR_H_
#define _CREATECONNECTOR_H_

#include <time.h>

#include "SerialPort.h"
#include "CreateData.h"

// Safe Mode
#define Create_Mode 131

class CreateConnector
{
public :
	bool Connect(const char *port)
	{
		TCHAR portT[MAX_PATH];
		mbstowcs( portT, port, strlen(port) + 1 );

		if( !serial.Open(portT, 57600) )
			return false;

		int length = sprintf(_buff, "%c%c", 128, Create_Mode);
		serial.Write(_buff, length);

		rxBuffer.clear();
		lastTime = clock();

		return true;
	}

	void Disconnect()
	{
		DriveDirect(0, 0);
		serial.Close();
	}

	bool ReadData(CreateData &data)
	{
		bool result = RequestData();
		data.Copy(_data);
		
		return result;
	}

	bool DriveDirect(int velLeft, int velRight)
	{
		//opcode = 145;
		int length = sprintf(_buff, "%c%c%c%c%c", 145, (velRight&0xFF00)>>8, velRight&0x00FF, (velLeft&0xFF00)>>8, velLeft&0x00FF);
		return serial.Write(_buff, length);
	}

	bool LEDs(bool advLed, bool playLed, int color, int intensity)
	{
		//opcode = 139;
		int index = 0;
		if( advLed ) index += 8;
		if( playLed ) index += 2;

		int length = sprintf(_buff, "%c%c%c%c", 139, index, color, intensity);
		return serial.Write(_buff, length);
	}

private:
	SerialPort serial;
	char _buff[MAX_PATH];
	std::string rxBuffer;
	int lastTime;

	CreateData	_data;

	bool RequestData()
	{
		if( !serial.IsOpen() ) return false;

		if( clock() - lastTime > 160 )
		{
			lastTime = clock();

			std::cout << "#";
			int length = sprintf(_buff, "%c%c", 142, 6); // Mode #6: Request all data (52 bytes)
			if( !serial.Write(_buff, length) ) return false;

		}

		int length = serial.Read(_buff, MAX_PATH, 0);

		for(int i = 0; i < length; i++)
			rxBuffer.push_back(_buff[i]);
		//std::cout << "rxBuffer : " << rxBuffer.size() << std::endl;

		if( rxBuffer.size() < 52 ) return false;
		if( rxBuffer.size() > 52 )
		{
			std::cout << "x";
			rxBuffer.clear();
			return false;
		}

		for(int i = 0; i < 52; i++)
			_buff[i] = rxBuffer[i];

		rxBuffer.clear();

		_data.bumper[0]			= _buff[0] & 0x02;
		_data.bumper[1]			= _buff[0] & 0x01;
		_data.wheeldrop[0]		= _buff[0] & 0x08;
		_data.wheeldrop[1]		= _buff[0] & 0x10;
		_data.wheeldrop[2]		= _buff[0] & 0x04;

		_data.wall				= _buff[1];
		_data.cliff[0]			= _buff[2];
		_data.cliff[1]			= _buff[3];
		_data.cliff[2]			= _buff[4];
		_data.cliff[3]			= _buff[5];
		_data.virtualWall		= _buff[6];
		_data.overcurrent[0]	= _buff[7] & 0x10;
		_data.overcurrent[1]	= _buff[7] & 0x08;
		_data.lowsideDriver[0]	= _buff[7] & 0x02;
		_data.lowsideDriver[1]	= _buff[7] & 0x01;
		_data.lowsideDriver[2]	= _buff[7] & 0x04;

		_data.infrared			= _buff[10];
		_data.button[0]			= _buff[11] & 0x01;
		_data.button[1]			= _buff[11] & 0x02;

		_data.distance			= (_buff[12]<<8)|((unsigned char)_buff[13]);
		_data.angle				= (_buff[14]<<8)|((unsigned char)_buff[15]);
		
		_data.chargingSate		= _buff[16];

		_data.voltage			= (_buff[17]<<8)|((unsigned char)_buff[18]);
		_data.current			= (_buff[19]<<8)|((unsigned char)_buff[20]);

		_data.battTemp			= _buff[21];

		_data.battCharge			= (_buff[22]<<8)|((unsigned char)_buff[23]);
		_data.battCap			= (_buff[24]<<8)|((unsigned char)_buff[25]);

		_data.wallSignal			= (_buff[26]<<8)|((unsigned char)_buff[27]);
		_data.cliffSignal[0]		= (_buff[28]<<8)|((unsigned char)_buff[29]);
		_data.cliffSignal[1]		= (_buff[30]<<8)|((unsigned char)_buff[31]);
		_data.cliffSignal[2]		= (_buff[32]<<8)|((unsigned char)_buff[33]);
		_data.cliffSignal[3]		= (_buff[34]<<8)|((unsigned char)_buff[35]);

		_data.digitalInput[0]	= _buff[36] & 0x10;
		_data.digitalInput[1]	= _buff[36] & 0x08;
		_data.digitalInput[2]	= _buff[36] & 0x04;
		_data.digitalInput[3]	= _buff[36] & 0x02;
		_data.digitalInput[4]	= _buff[36] & 0x01;
		_data.analogInput		= (_buff[37]<<8)|((unsigned char)_buff[38]);

		_data.charger[0]			= _buff[39] & 0x02;
		_data.charger[1]			= _buff[39] & 0x01;

		_data.oiMode				= _buff[40];
		_data.songNumber			= _buff[41];
		_data.songPlaying		= _buff[42];
		_data.streamNumber		= _buff[43];

		_data.requestVelocity	= (_buff[44]<<8)|((unsigned char)_buff[45]);
		_data.requestRadius		= (_buff[46]<<8)|((unsigned char)_buff[47]);
		_data.requestVelLeft		= (_buff[48]<<8)|((unsigned char)_buff[49]);
		_data.requestVelRight	= (_buff[50]<<8)|((unsigned char)_buff[51]);

		//cout << "Request Data" << _data << endl;

		return true;
	}
};
#endif // _CREATECONNECTOR_H_