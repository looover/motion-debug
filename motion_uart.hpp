
#ifndef _MOTION_UART_H
#define _MOTION_UART_H

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

#define			OFFSET_MOVE		0x00
#define			READ_POS		0x10
#define			SET_SPEED		0x14
#define			SET_ACC			0x1C

#define			READ_STATUS		0x0100

#define		MODBUS_READ	0x0003
#define		MODBUS_WRITE	0x0006
#define		MODBUS_PRINT	0x0010

#define		SET_SPEED		0x0036
#define		STE_SPEED_LOW		0x0000
#define		STE_SPEED_MIDDLE	0x0001
#define		STE_SPEED_HIGH		0x0002

#define		SET_DIR			0x0030
#define		SET_DIR_UNIDIR		0x0000
#define		SET_DIR_BIR		0x0001

class MotionUart
{
public:
	MotionUart(unsigned char addr, struct timeval * tv);
	~MotionUart(){
	}

public:
	virtual int Detect();
	virtual int Detached();

	virtual int ReadCarPos();
	virtual int ReadCarStatus();

	virtual int MeasureMedia(int *x, int * width);
	virtual int MeasureMachine();

	virtual int PrintMove(void * param);

	virtual int GoHome(int asix, int speed);

	virtual int MoveStop(int asix);
	virtual int MoveStart(int asix, int speed, int dir);

	virtual int MoveTo(int asix, int speed, int dir, int dist);

	virtual int SetMoveMode(unsigned int cmd, unsigned int param);
private:


	int WaitAck(unsigned char * buf, int len, const struct timeval *tv);
	int WriteData(unsigned char * data, int len, const struct timeval *timeout = NULL);
	int ReadData(unsigned char * buf, int len, const struct timeval *tv);

	int ReadBuf(unsigned char * buf, const struct timeval *tv);
	int InitPackage(unsigned short cmd, unsigned char dir, unsigned char* src, int len, unsigned char * dst);
	int GenModbusMoveCmd(unsigned char * buf, unsigned short * addr, 
		int cmd, int axis, int dir = 0, int speed = 0, int dist = 0);
private:

	char Name[16];

	mutex Mux;	
	//lock
	//

	unsigned short Toogle = 0x8000;

	const unsigned char Addr;
	const struct timeval TimerOut; 
};


#endif


