/*****************************************************************************

omni_comm.h/omni_comm.cpp

These files declare and define objects for communicating with the 
robot slave and the Surgical User Interface.

Class created by KoSy
Modified by Hawkeye

*****************************************************************************/


#include "stdafx.h"
#include <iostream>
#include <assert.h>

#include <cstdio>
#include <cassert>
#include <conio.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>


#include <HD/hd.h>
#include <HDU/hduVector.h>
//#include <HDU/hduError.h>

#include "PracticalSocket/PracticalSocket.h"
#include <CommonDS.h>
#include "ITPteleoperation.h"

#include "omni_comm.h"

#define FALSE 0
#define TRUE 1
#define OMNI1 0
#define OMNI2 1
#define TIMEOUT_SECS 0
#define TIMEOUT_USECS 100

#define THOUSAND 1000
#define MILLION  (THOUSAND*THOUSAND)
#define round(x) (x)

/*Identifier for two Omni device -- Name it wisely as you wish */

#define OMNI_NAME_1 "Omni_Left"
#define OMNI_NAME_2 "Omni_Right"

//#define ADDR_ROBOT0 "128.95.31.71"
#define ADDR_ROBOT0 "128.32.37.222"
#define ADDR_ROBOT1 "128.95.31.133"
#define ADDR_ROBOT2 "128.95.31.76"
#define ADDR_ROBOT3 "128.95.31.125"
#define ADDR_ROBOT4 "128.95.31.113"
#define ADDR_ROBOT5 "128.95.31.144"
#define ADDR_ROBOT6 "128.95.31.199"

extern stMA2UI_DATA Ma2UIdata;
extern stUI2MA_DATA UI2Madata;
extern unsigned short c_port_UI;
extern const char* c_addr_UI;

// Three camera rotation angles
extern double th1, th2, th3;
extern int useITP;

//Global variable for world coordinate scale uniform in all 3 axis.
extern HDdouble scale;


///////////////////////////////////////////////////////////////////
// Implementation 
///////////////////////////////////////////////////////////////////
Omni_Comm::Omni_Comm(stMA2UI_DATA* pma2ui, stUI2MA_DATA* pui2ma)
{
	pMa2UIdata=pma2ui;
	pUI2Madata=pui2ma;

	strcpy_s(addr_Robot[0], ADDR_ROBOT0);
	strcpy_s(addr_Robot[1], ADDR_ROBOT1);
	strcpy_s(addr_Robot[2], ADDR_ROBOT2);
	strcpy_s(addr_Robot[3], ADDR_ROBOT3);
	strcpy_s(addr_Robot[4], ADDR_ROBOT4);
	strcpy_s(addr_Robot[5], ADDR_ROBOT5);
	strcpy_s(addr_Robot[6], ADDR_ROBOT6);
	port_Robot = 36000;
	iUDPaddr=0;
}


Omni_Comm::~Omni_Comm()
{
}


void Omni_Comm::Set_Flag(int value, int mode)
{
	if(value == TRUE)
		pUI2Madata->flag01 |= mode;
	else if(value == FALSE)
		pUI2Madata->flag01 &= (ALL_ONES - mode);
}


bool Omni_Comm::Check_Flag(int mode)
{
	return pUI2Madata->flag01&mode ? TRUE : FALSE;
}


int Omni_Comm::Update_MA2UI(hduVector3Dd pos[2],int rl, unsigned int servo)
{
	pMa2UIdata->delx[0] = int(round(pos[OMNI1][0]*THOUSAND));
	pMa2UIdata->dely[0] = int(round(pos[OMNI1][1]*THOUSAND));
	pMa2UIdata->delz[0] = int(round(pos[OMNI1][2]*THOUSAND));
	pMa2UIdata->delx[1] = int(round(pos[OMNI2][0]*THOUSAND));
	pMa2UIdata->dely[1] = int(round(pos[OMNI2][1]*THOUSAND));
	pMa2UIdata->delz[1] = int(round(pos[OMNI2][2]*THOUSAND));
	pMa2UIdata->tick = servo; 
	pMa2UIdata->runlevel = rl;
	pMa2UIdata->checksum = pMa2UIdata->delx[0] +  pMa2UIdata->dely[0] +  pMa2UIdata->delz[0] +  pMa2UIdata->runlevel + (int)pMa2UIdata->tick;   

	return 1;
}


int Omni_Comm::Check_UI2MA(int message)
{
	if(pUI2Madata->checksum == (pUI2Madata->scale + pUI2Madata->flag01 + pUI2Madata->UDPaddr+ (int)pUI2Madata->tick)) { 
		scale= (double)pUI2Madata->scale/100.0;

		// Set camera angle 1
		if (th1 != pUI2Madata->camAngle[0]){
			th1 = pUI2Madata->camAngle[0];
			cout << "Camera angle 1 changed to "<< th1 <<" degrees.\n";
		}
		// Set camera angle 2
		if (th2 != pUI2Madata->camAngle[1]){
			th2 = pUI2Madata->camAngle[1];
			cout << "Camera angle 2 changed to "<< th2 <<" degrees.\n";
		}
		// Set camera angle 3
		if (th3 != pUI2Madata->camAngle[2]){
			th3 = pUI2Madata->camAngle[2];
			cout << "Camera angle 3 changed to "<< th3 <<" degrees.\n";
		}
		if (useITP != pUI2Madata->useITP) {
			useITP = pUI2Madata->useITP;
			if (useITP)
				cout << "Using ITP coordinate reference.\n";
			else
				cout << "Using Omni native coordinate reference.\n";
		}

		// Robot's IP address Update
		if(iUDPaddr!=pUI2Madata->UDPaddr) {
			iUDPaddr = pUI2Madata->UDPaddr;
			//sprintf_s(the_addr,"%d.%d.%d.%d",(unsigned int)((iUDPaddr&0xff000000)>>24),(unsigned int)((iUDPaddr&0xff0000)>>16), (unsigned int)((iUDPaddr&0xff00)>>8),(iUDPaddr&0x00ff));
			cout << "Surgical Robot's IP WAS NOT chaged into " << (unsigned int)((iUDPaddr&0xff000000)>>24) << "." << (unsigned int)((iUDPaddr&0xff0000)>>16) << "." << (unsigned int)((iUDPaddr&0xff00)>>8) << "." << (iUDPaddr&0x00ff) <<endl;
		}
		if(message) {
			cout << "Received Data is fine. " << endl;
		}
		return TRUE;
	} else {
		pUI2Madata->flag01 &= (ALL_ONES - FPEDAL_RIGHT);
		cout << " Data is strange !!" << endl;
		return FALSE;
	}
}


int Omni_Comm::Initialize_TCP_GUI()
{
	Sleep(1);

	cout << "Initializing TCP server... " << endl;
	tcpsocket = new TCPSocket;
	try {
		tcpsocket->connect(c_addr_UI, c_port_UI);
	}
	catch( SocketException SE ) {
		cout << "Failed to connect TCP server.\n";
		return FALSE;
	}
	cout << "TCP connection started. ^^" << endl;

	hduVector3Dd zero_pos[2];
	Update_MA2UI(zero_pos, 0, 0);

	Send_TCP();
	Recv_TCP();

	
	return (Check_UI2MA(1) ? TRUE : FALSE);
}


void Omni_Comm::Send_TCP()
{
	try{
		tcpsocket->send(&Ma2UIdata, sizeof(Ma2UIdata));
	}
	catch (SocketException SE){
		cerr << "failed to send_tcp\n";
	}
}


void Omni_Comm::Recv_TCP()
{
	try {
		tcpsocket->recv(&UI2Madata, sizeof(UI2Madata));
	}
	catch (SocketException SE){
		cerr << "failed to recv_tcp\n";
	}
}

int Omni_Comm::Initialize_UDP_Robot(int argc, char*argv[])
{
	strcpy_s(addr_Robot[0],ADDR_ROBOT0);
	strcpy_s(addr_Robot[1],ADDR_ROBOT1);
	strcpy_s(addr_Robot[2],ADDR_ROBOT2);
	strcpy_s(addr_Robot[3],ADDR_ROBOT3);
	strcpy_s(addr_Robot[4],ADDR_ROBOT4);
	strcpy_s(addr_Robot[5],ADDR_ROBOT5);
	strcpy_s(addr_Robot[6],ADDR_ROBOT6);
	cout << "Servers: " << addr_Robot[0] << endl << addr_Robot[1] << endl << addr_Robot[2] << endl << addr_Robot[3] << endl << addr_Robot[4] << endl << addr_Robot[5] << endl << addr_Robot[6] << endl;
	cout << "Scale factor= " << scale << endl;

	return 1;
}


void Omni_Comm::Send_UDP()
{
	if(Check_Flag(BASIC_START)) {
		try {
			//cout << "sending udp so many times" << endl;
			udpsocket.sendTo(&msgHeader,sizeof(u_struct),addr_Robot[0],port_Robot);
			udpsocket.sendTo(&msgHeader,sizeof(u_struct),addr_Robot[1],port_Robot);
			udpsocket.sendTo(&msgHeader,sizeof(u_struct),addr_Robot[2],port_Robot);
			//udpsocket.sendTo(&msgHeader,sizeof(u_struct),addr_Robot[3],port_Robot);
			udpsocket.sendTo(&msgHeader,sizeof(u_struct),addr_Robot[4],port_Robot);
			udpsocket.sendTo(&msgHeader,sizeof(u_struct),addr_Robot[5],port_Robot);
			udpsocket.sendTo(&msgHeader,sizeof(u_struct),addr_Robot[6],port_Robot);
		}
		catch (SocketException SE){
			cerr << "failed to send_UDP\n";
		}
	}
}

void Omni_Comm::Update_UDP_Data(hduVector3Dd pos[2], hduQuaternion qIncr[2], int bttn[2], int fp, unsigned int servo)
{

	static int i;
	msgHeader.delx[0]		= int(round(pos[OMNI1][0]*THOUSAND));
	msgHeader.dely[0]		= int(round(pos[OMNI1][1]*THOUSAND));
	msgHeader.delz[0]		= int(round(pos[OMNI1][2]*THOUSAND));
	//msgHeader.Qx[0]			= int(round( qIncr[OMNI1].v()[0]*1e9 ));
	//msgHeader.Qy[0]			= int(round( qIncr[OMNI1].v()[1]*1e9));
	//msgHeader.Qz[0]			= int(round( qIncr[OMNI1].v()[2]*1e9));
	//msgHeader.Qw[0]			= int(round( qIncr[OMNI1].s()*1e9 ));
	msgHeader.Qx[0]			=  qIncr[OMNI1].v()[0];
	msgHeader.Qy[0]			=  qIncr[OMNI1].v()[1];
	msgHeader.Qz[0]			=  qIncr[OMNI1].v()[2];
	msgHeader.Qw[0]			=  qIncr[OMNI1].s();

	msgHeader.delx[1]		= int(round(pos[OMNI2][0]*THOUSAND));
	msgHeader.dely[1]		= int(round(pos[OMNI2][1]*THOUSAND));
	msgHeader.delz[1]		= int(round(pos[OMNI2][2]*THOUSAND));
	//msgHeader.Qx[1]			= int(round( qIncr[OMNI2].v()[0]*1e9 ));
	//msgHeader.Qy[1]			= int(round( qIncr[OMNI2].v()[1]*1e9 ));
	//msgHeader.Qz[1]			= int(round( qIncr[OMNI2].v()[2]*1e9 ));
	//msgHeader.Qw[1]			= int(round( qIncr[OMNI2].s()*1e9 ));
	msgHeader.Qx[1]			=  qIncr[OMNI2].v()[0];
	msgHeader.Qy[1]			=  qIncr[OMNI2].v()[1];
	msgHeader.Qz[1]			=  qIncr[OMNI2].v()[2];
	msgHeader.Qw[1]			=  qIncr[OMNI2].s();

	msgHeader.buttonstate[OMNI1]= bttn[OMNI1];
	msgHeader.buttonstate[OMNI2]= bttn[OMNI2];
	msgHeader.grasp[OMNI1]= bttn[OMNI1];
	msgHeader.grasp[OMNI2]= bttn[OMNI2];
	msgHeader.sequence=servo;
	msgHeader.surgeon_mode=fp;
	msgHeader.checksum=checksumUDPData();

	
}
int Omni_Comm::checksumUDPData(){
	int chk=0;
	chk =  (msgHeader.surgeon_mode);
	chk += (msgHeader.delx[0])+(msgHeader.dely[0])+(msgHeader.delz[0]);
	chk += (msgHeader.delx[1])+(msgHeader.dely[1])+(msgHeader.delz[1]);
	chk += (msgHeader.buttonstate[0]);
	chk += (msgHeader.buttonstate[1]);
	chk += (int)(msgHeader.sequence);
	return chk;
}