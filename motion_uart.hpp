
#ifndef _MOTION_UART_H
#define _MOTION_UART_H

#include <QSerialPort>

#include <mutex>

using namespace std;
using std::mutex;
//#include "uart.hpp"
//#include "motion_.hpp"

#define			AXIS_X			(0<<5)
#define			AXIS_Y0			(1<<5)
#define			AXIS_Y1			(2<<5)
#define			AXIS_Z			(3<<5)
#define			AXIS_4			(4<<5)
#define			AXIS_5			(5<<5)
#define			AXIS_6			(6<<5)
#define			AXIS_7			(7<<5)


#define			MOVE_STOP		0x00
#define			MOVE_START		0x01
#define			MOVE_DIST		0x02
#define			MOVE_TO			0x03

#define			READ_POS		0x0010
#define			READ_STATUS		0x0140
#define			UPDATE_START		0x0141
#define			UPDATE_FINISH		0x0142


#define			MODBUS_READ	0x0003
#define			MODBUS_WRITE	0x0006
#define			MODBUS_ACK	0x0009
#define			MODBUS_UPDATE	0x0010


class MotionUart : public QSerialPort
{
public:
	MotionUart(unsigned char addr, QObject *parent  = NULL);
	~MotionUart(){
	}

public:
	int ReadCarPos(int axis, int * pos);
	int ReadCarStatus();
	
	int MeasureMedia(int *x, int * width);
	int MeasureMachine();

	int PrintMove(void * param);

	int GoHome(int asix, int speed);

	int MoveStop(int asix);
	int MoveStart(int asix, int speed, int dir);

	int MoveTo(int asix, int speed, int dir, int dist);

	int SetMoveMode(unsigned int cmd, unsigned int param);

	int UpdateStart(int len);
	int UpdateFinish(unsigned char * hash, int len);
	int UpdateFirmwear(unsigned char * data, int len);
private:
	int WaitAck(unsigned char * buf, const struct timeval *tv);
	int WriteData(unsigned char * data, int len, const struct timeval *timeout = NULL);
	int ReadData(unsigned short addr, int len, unsigned char * buf, const struct timeval *tv = NULL);

	int ReadBuf(unsigned char * buf, const struct timeval *tv);
	int InitPackage(unsigned short cmd, unsigned char dir, unsigned char* src, int len, unsigned char * dst);
	int GenModbusMoveCmd(unsigned char * buf, unsigned short * addr, 
		int cmd, int axis, int dir = 0, int speed = 0, int dist = 0);
private:

	char Name[16];

	mutex Mux;	
	//lock
	//

	//QSerialPort *m_serial;

	unsigned short Toogle = 0x8000;

	const unsigned char Addr;
	struct timeval TimerOut; 
};


#endif


