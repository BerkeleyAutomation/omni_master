/*****************************************************************************

omni_comm.h/omni_comm.cpp

These files declare and define objects for communicating with the 
robot slave and the Surgical User Interface.

Class created by KoSy
Modified by Hawkeye

*****************************************************************************/
#include <HDU/hduQuaternion.h>

class Omni_Comm
{
public:
	//Omni_Comm(stMA2UI_DATA*, stUI2MA_DATA*,unsigned short, char *);
	Omni_Comm(stMA2UI_DATA*, stUI2MA_DATA*);
	~Omni_Comm();

	// ** TCP Commnucation to GUI  **
	int Initialize_TCP_GUI();
	int Update_MA2UI(hduVector3Dd pos[2],int rl, unsigned int servo);
	int Check_UI2MA(int message);
	bool Check_Flag(int mode);
	void Set_Flag(int value,int mode);
	void Send_TCP();
	void Recv_TCP();
	int checksumUDPData();

	TCPSocket* tcpsocket;
	stMA2UI_DATA* pMa2UIdata;
	stUI2MA_DATA* pUI2Madata;

	// ** UDP Commnucation to Robot  **
	int Initialize_UDP_Robot(int argc, char*argv[]); 
	void Update_UDP_Data(hduVector3Dd pos[2], hduQuaternion qIncr[2], int bttn[2], int fp, unsigned int servo);
	void Send_UDP();
	//ComHeader msgHeader;
	u_struct msgHeader;
	UDPSocket udpsocket;
	unsigned short port_Robot;
	char addr_Robot[8][16];
	int iUDPaddr;
};
