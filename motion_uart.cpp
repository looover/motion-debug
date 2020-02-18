#include <stdio.h>
#include <string.h>
#include <stdint.h>

//#include "debug.hpp"
#include "motion_uart.hpp"

//#include "APIDataInterface.hpp"

#define		RETRY_MAX_NUM		3

//extern void hexdump(const uint8_t *p, unsigned int len);
extern unsigned short Crc16(unsigned char * puchMsg, unsigned short usDataLen);

MotionUart::MotionUart(unsigned char addr, struct timeval *tv) :
Addr(addr),
TimerOut(*tv)
{
	memset(Name, 0, 16);

	Detect();
}

int MotionUart::Detached()
{
	return 0;
}

int MotionUart::Detect()
{
	return 0;
}


#define		ACTIVE_CHECK()		\
do{\
}while(0)

#define		CMD_MOTION_MOVE_X	0x010C
#define		CMD_MOTION_MOVE_Y	0x010D

#define		CMD_MOTION_X_MOVETO		0x0120

#define		CMD_MOTION_Y_MOVE_BACKTO	0x0110	//
#define		CMD_MOTION_Y_MOVE_FORTO		0x0111	//

int MotionUart::GenModbusMoveCmd(unsigned char * buf, unsigned short * addr, 
	int cmd, int axis, int dir, int speed, int dist)
{
	*addr = axis << 5;
	
	int i = 0;

	buf[i++] = cmd;

	if(cmd != MOVE_STOP){
		buf[i++] = dir;
		buf[i++] = speed;
		buf[i++] = 0;
	}
	
	if((cmd == MOVE_DIST) || (cmd == MOVE_TO)){
		buf[i++] = dist & 0xFF;
		buf[i++] = (dist >> 8) & 0xFF;
		buf[i++] = (dist >> 16) & 0xFF;
		buf[i++] = (dist >> 24) & 0xFF;
	}

	return i;
}
int MotionUart::ReadCarStatus()
{
	ACTIVE_CHECK();

	int ret = -1;
	unsigned short buf[32];

	Mux.lock();

	//int len = InitPackage(READ_STATUS, cmd, data, len, buf);
	//if(ReadData(buf, len, &TimerOut) > 0){
	//	ret = 0;
	//}

	Mux.unlock();

	return ret;
}

int MotionUart::MoveTo(int axis, int speed, int dir, int dist)
{
	ACTIVE_CHECK();

	struct timeval tv;
	tv.tv_sec = 10;
	tv.tv_usec = 0;//3s

	unsigned char buf[32];
	unsigned char send[32];
	unsigned short addr = 0;
	int len = GenModbusMoveCmd(buf, &addr, MOVE_TO, axis, dir, speed, dist);
	
	len = InitPackage(addr, MODBUS_WRITE, buf, len, send);

	Mux.lock();
			
	int ret = WriteData(buf, len, &tv);

	Mux.unlock();

	return ret;
}

int MotionUart::MoveStart(int axis, int speed, int dir)
{
	ACTIVE_CHECK();

	struct timeval tv;
	tv.tv_sec = 10;
	tv.tv_usec = 0;//3s

	unsigned char buf[32];
	unsigned char send[32];
	
	unsigned short addr;
	int len = GenModbusMoveCmd(buf, &addr, MOVE_START, axis, dir, speed);

	len = InitPackage(addr, MODBUS_WRITE, buf, len, send);

	Mux.lock();

	int ret = WriteData(send, len, &tv);
	
	Mux.unlock();

	return ret;
}

int MotionUart::MoveStop(int axis)
{
	ACTIVE_CHECK();

	struct timeval tv;
	tv.tv_sec = 1;
	tv.tv_usec = 0;//100ms
	int ret = -1;

	unsigned char buf[32];
	unsigned char send[32];

	unsigned short addr;
	int len = GenModbusMoveCmd(buf, &addr, MOVE_STOP, axis);

	InitPackage(addr, MODBUS_WRITE, buf, len, send);

	Mux.lock();
	
	ret = WriteData(send, len, &tv);
	
	Mux.unlock();

	return ret;
}

int MotionUart::GoHome(int asix, int speed)
{
	ACTIVE_CHECK();

	return MoveTo(asix, speed, 0, 10);
}

int MotionUart::MeasureMedia(int *x, int * width)
{
	ACTIVE_CHECK();

	struct timeval tv;
	tv.tv_sec = 30;
	tv.tv_usec = 0;

	int ret = -1;

	Mux.lock();
	
	Mux.unlock();

	return ret;
}

int MotionUart::MeasureMachine()
{
	ACTIVE_CHECK();

	Mux.lock();
	Mux.unlock();
	return -1;
}

int MotionUart::ReadCarPos()
{
	ACTIVE_CHECK();

	int ret = -1;
	Mux.lock();

	struct timeval tv;
	tv.tv_sec = 30;
	tv.tv_usec = 0;

	unsigned short pos[2];
	//if(ReadData(0x0010, pos, 2, &tv) == 2){
	//	ret = (int)pos[0] | (int)pos[1] << 16;
	//}

	Mux.unlock();

	return ret;
}

int MotionUart::SetMoveMode(unsigned int cmd, unsigned int param)
{
	ACTIVE_CHECK();

	int ret = -1;
	unsigned short addr = 0x00;
	unsigned short tmp  = 0x00;

	Mux.lock();


	Mux.unlock();

	return ret;
}

int MotionUart::PrintMove(void * param)
{
	ACTIVE_CHECK();

	int ret = -1;

	Mux.lock();
	
	Mux.unlock();

	return ret;
}

int MotionUart::InitPackage(
	unsigned short cmd, 
	unsigned char dir,
	unsigned char * src, int len, unsigned char * dst)
{
        int i = 0;

	cmd |= Toogle;
		
        dst[i++] = Addr;
        dst[i++] = dir;
        dst[i++] = cmd >> 8;
        dst[i++] = cmd & 0x00FF;

        if(dir == MODBUS_READ || dir == MODBUS_WRITE){
                dst[i++] = len;
        }else{

		}
		
        //if(dir == MODBUS_WRITE)//for master
        if(src && dst)
		{
                for(int j = 0; j < len; j++){
                        dst[i++] = src[j];
                }
        }

        unsigned short crc = Crc16(dst, i);

        dst[i++] = crc >> 8;
        dst[i++] = crc & 0xFF;

        return i;
}


int MotionUart::WaitAck(unsigned char * buf, int len, const struct timeval *tv)
{
	unsigned char ack[32];

	int size = ReadBuf(ack, tv);	
	if(size > 0){
		if(Crc16(ack, size) == 0){
			return 0;
		}
		//PMD_PRINT("crc error");
		//hexdump(ack, size);
	}

	//PMD_PRINT("wait ack failed");
/*
	int ret = Rs485->Read(ack, 32, tv);

	unsigned char ack[5] = {0x81, 0x86, 0x02, 0xC2, 0x49};

	if(ret == 5){
		int i = 0;
		for(i = 0; i < 5; i++){
			if(ack[i] != rev[i]){
				break;
			}
		}
		if(i == 5){
			return 0;
		}
	}
*/

	return -1;
}

#define SWAP(a, b)	do{\
	typeof(a) tmp = a;\
	a = b;\
	b = tmp;\
}while(0)

#define	CHECK_HEADER()	\
do{\
	if(rev[0] != addr){\
			rev++;\
			cnt--;\
		}else{\
			break;\
		}\
}while(cnt);

int MotionUart::ReadBuf(unsigned char * buf, const struct timeval * tv)
{
	unsigned char recv_buf[64];
	unsigned char *rev = recv_buf;

	struct timeval timeout = *tv;
	
	int ret = -1;
	unsigned int cnt = 0;
	while(1){
		//int ret = Rs485->Read(rev + cnt, 32, &timeout);
		if(ret > 0){
			cnt += ret;
			if(cnt > 32){
				//PMD_PRINT("cnt%d > 32", cnt);
				return -1;
			}else{
				timeout.tv_sec = 0;
				timeout.tv_usec = 30000;//3ms
			}
		}else{
			break;
		}
	}

	if(cnt > 0){
		//CHECK_HEADER();
		memcpy(buf, rev, cnt);
	}

	return cnt;
}

int MotionUart::ReadData(unsigned char * buf, int len, const struct timeval* tv)
{
	unsigned char send[32];
	unsigned char recv[32];


	int retry = 0;
	while(retry++ < RETRY_MAX_NUM){
		//Rs485->Write(send, len);

		int size = ReadBuf(recv, tv);
		if(size > 0){
			if(Crc16(recv, size) == 0){
				size = (recv[2] / 2 > len) ? len : recv[2] / 2;
				for(int i = 0, j = 3; i < size; i++){
					buf[i] = recv[j++];
					buf[i] <<= 8;
					buf[i] += recv[j++];
				}
				return size;	//return short size
			}
			//PMD_PRINT("crc error");
		}else{
			//PMD_PRINT("read buf size=%d", size);
		}
	}

	return -1;
}

int MotionUart::WriteData(unsigned char * data, int len, const struct timeval *tv)
{
	unsigned char buf[32];
	if(tv == NULL){
		tv = &TimerOut;
	}

	int retry = 0;
	do{
		//Rs485->Write(buf, len);

		if(WaitAck(buf, len, tv) == 0){
			return 0;
		}
		//PMD_PRINT("retry...");
	}while(++retry < RETRY_MAX_NUM);

	return -1;
}




