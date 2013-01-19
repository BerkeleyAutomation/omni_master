/*****************************************************************************

SUI_HK

Surgical Robot Master control using PHANToM OMNI
Ganesh Sankaranarayanan
BioRobotics Laboratory
08/04/05
Modified by Kosy 10/24/05
Modified by Hawkeye 4/17/06
Modified by Ganesh 5/17/06

*******************************************************************************/
#include "stdafx.h"
#include <HDU/hduQuaternion.h>

#include "PracticalSocket/PracticalSocket.h"
#include <CommonDS.h>
#include "ITPteleoperation.h"
#include "omni_comm.h"
#include "coordinateSystems.h"

#include <iostream>
#include <string>

#define FALSE 0
#define TRUE 1
#define OMNI1 0
#define OMNI2 1
int omnis[2] = {OMNI1,OMNI2};
#define TIMEOUT_SECS 0
#define TIMEOUT_USECS 100

#define THOUSAND 1000
#define MILLION  (THOUSAND*THOUSAND)
#define round(x) (x)

/*Identifier for two Omni device -- Name it wisely as you wish */

#define OMNI_NAME_1 "Omni_Left"
#define OMNI_NAME_2 "Omni_Right"
std::string omniNames[2] = {std::string(OMNI_NAME_1),std::string(OMNI_NAME_2)};

//Global variable for world coordinate scale uniform in all 3 axis.
HDdouble scale = 0.1;

// Three camera rotation angles
extern double th1, th2, th3;

int supportedCalibrationStyles;
int calibrationStyle;

//Global variable for reading device button
int gold_button_graspON;
int green_button_graspON;

hduVector3Dd g_position[2];
hduVector3Dd p_position[2];
hduVector3Dd g_rotation[2];
hduVector3Dd p_rotation[2];
hduVector3Dd pos_delta[2];
hduVector3Dd rot_delta[2];
hduVector3Dd pos_offset[2];
hduVector3Dd rot_offset[2];
int g_footpedal = 0;
unsigned int servo=0;
bool indexing= false;

#define ADDR_BUBBLES   "192.168.0.101"
#define ADDR_BUTTERCUP "192.168.0.100"

extern stMA2UI_DATA Ma2UIdata;
extern stUI2MA_DATA UI2Madata;

// Global Haptic device identifier

HHD omniId_1,omniId_2; /*Dual Master device */

HHD omniIds[2];

// Global variable for testing.
bool oneOmni=false;
bool leftDevice=false;
bool rightDevice=false;
bool greenArm=false;
bool goldArm=false;
hduVector3Dd temp_delta;
const hduQuaternion q_identity;

int temp_bttn;

// Communication object for communication with SUI and Slave robot
Omni_Comm comm(&Ma2UIdata, &UI2Madata);

/*******************************************************************************
Incremental position update callback function
*******************************************************************************/
HDCallbackCode HDCALLBACK PositionUpdate(void *data)
{
	//cout << "in posup" << std::endl;
	servo++;
	const double K = 0.1;
	const double B=0.001;
	HDint nButtons = 0, footpedal;
	int bttn[2] = {0,0};
	hduVector3Dd position[2];
	hduVector3Dd rotation[2];
	hduMatrix xform;
	static hduQuaternion qPrev[2];
	hduQuaternion qCurr[2], qIncr[2];

	HDErrorInfo error;
	int rl2sui;

	hdBeginFrame(omniId_1);					// Get OMNI 1 stuff
	hdMakeCurrentDevice(omniId_1);

	// Get button state using bitmask:
	hdGetIntegerv(HD_CURRENT_BUTTONS, &nButtons);
	if ( nButtons & HD_DEVICE_BUTTON_2 ) // button 2 releases grasp
		bttn[OMNI1] = -1;
	else if ( nButtons & HD_DEVICE_BUTTON_1 ) // button 1 engages grasp
		bttn[OMNI1] = 1;

	footpedal = comm.Check_Flag(FPEDAL_RIGHT) ? TRUE : FALSE;

	// Get omni position in pedal dn state
	if( comm.Check_Flag(FPEDAL_RIGHT) && comm.Check_Flag(BASIC_START) )
	{
		// footpedal is down and SUI is running.	
		hdGetDoublev(HD_CURRENT_TRANSFORM, xform);
		omni2ITPTransform(xform);	//xform is multiplied with stuff and returned as xform
		position[OMNI1] = hduVector3Dd(xform[3][0], xform[3][1], xform[3][2]);

		// Get position increment
		applyTransforms(position[OMNI1]);
		hduVecScaleInPlace(position[OMNI1],scale);		// position scaling
		hduVecSubtract(pos_delta[OMNI1],position[OMNI1],p_position[OMNI1]);		// delta = current - previous
		p_position[OMNI1][0] = position[OMNI1][0];
		p_position[OMNI1][1] = position[OMNI1][1];
		p_position[OMNI1][2] = position[OMNI1][2];
		
		// Get quaternion increment
		hduMatrix rotMat( xform );
		rotMat.getRotationMatrix(rotMat);
		qCurr[OMNI1] = hduQuaternion(rotMat);
		qIncr[OMNI1] = qPrev[OMNI1].inverse()*qCurr[OMNI1];
		qPrev[OMNI1] = qCurr[OMNI1];

		qIncr[OMNI1] = qCurr[OMNI1];

		rl2sui =  3-(comm.Check_Flag(FPEDAL_RIGHT));	// send pedal state to SGUI as runlevel.
	} 
	// Send zero deltas in pedal up state.  (send last button state)
	else 
	{
		indexing=true;
		hdGetDoublev(HD_CURRENT_TRANSFORM, xform);
		omni2ITPTransform(xform);
		p_position[OMNI1] = hduVector3Dd(xform[3][0], xform[3][1], xform[3][2]);

		// Get current quaternion
		hduMatrix rotMat( xform );
		rotMat.getRotationMatrix(rotMat);
		qPrev[OMNI1] = hduQuaternion(rotMat);

		// Set zero increments
		hduVecSet(pos_delta[OMNI1],0.0,0.0,0.0);
		qIncr[OMNI1] = q_identity;

		bttn[OMNI1] = 0;
		rl2sui = 3-comm.Check_Flag(FPEDAL_RIGHT);
	}

	//Output force to prevent user moving too fast
	hdEndFrame(omniId_1);

	////////// For the second omni device ////////////
	hdBeginFrame(omniId_2);
	hdMakeCurrentDevice(omniId_2);

	// Get button state using bitmask:
	hdGetIntegerv(HD_CURRENT_BUTTONS, &nButtons);
	if ( nButtons & HD_DEVICE_BUTTON_2 ) // button 2 releases grasp
		bttn[OMNI2] = -1;
	else if ( nButtons & HD_DEVICE_BUTTON_1 ) // button 1 engages grasp
		bttn[OMNI2] = 1;

	// Get omni position in pedal dn state
	if( comm.Check_Flag(FPEDAL_RIGHT) && comm.Check_Flag(BASIC_START) ) 
	{
		// footpedal is down and SUI is running.
		hdGetDoublev(HD_CURRENT_TRANSFORM, xform);
		omni2ITPTransform(xform);
		position[OMNI2] = hduVector3Dd(xform[3][0], xform[3][1], xform[3][2]);

		// Get position increment
		applyTransforms(position[OMNI2]);				// Rotation to match omni frame with camera view.
		hduVecScaleInPlace(position[OMNI2], scale);		// position scaling
		hduVecSubtract(pos_delta[OMNI2], position[OMNI2], p_position[OMNI2]);		// delta = current - previous
		hduVecSubtract(rot_delta[OMNI2], rotation[OMNI2], p_rotation[OMNI2]);		// delta = current - previous
		p_position[OMNI2][0] = position[OMNI2][0];
		p_position[OMNI2][1] = position[OMNI2][1];
		p_position[OMNI2][2] = position[OMNI2][2];

		// Get quaternion increment
		hduMatrix rotMat( xform );
		rotMat.getRotationMatrix(rotMat);
		qCurr[OMNI2] = hduQuaternion(rotMat);
		qIncr[OMNI2] = qPrev[OMNI2].inverse()*qCurr[OMNI2];
		qPrev[OMNI2] = qCurr[OMNI2];

		qIncr[OMNI2] = qCurr[OMNI2];

		rl2sui =  3-(comm.Check_Flag(FPEDAL_RIGHT));	// Send pedal state as runlevel to SGUI

	} 
	// Send zero deltas in pedal up state.  (send last button state)
	else 
	{
		hdGetDoublev(HD_CURRENT_TRANSFORM, xform);
		omni2ITPTransform(xform);
		p_position[OMNI2] = hduVector3Dd(xform[3][0], xform[3][1], xform[3][2]);
		// Get quaternion as prev_quternion
		hduMatrix rotMat( xform );
		rotMat.getRotationMatrix(rotMat);
		qPrev[OMNI2] = hduQuaternion(rotMat);

		hduVecSet(pos_delta[OMNI2],0.0,0.0,0.0);
		qIncr[OMNI2] = q_identity;

		bttn[OMNI2] = 0;
		rl2sui = 3-comm.Check_Flag(FPEDAL_RIGHT);
	}

	hdEndFrame(omniId_2);
	///////////////////////////////////////////////////

	if(comm.Check_Flag(BASIC_PROGRAM)) { // when UI allows 'Server' run
		comm.Update_UDP_Data(pos_delta, qIncr, bttn, footpedal, servo );  // Update robot command packet
		comm.Send_UDP();
		//cout << "sent udp" << endl;
	}
	//else {	// when UI terminates 'Server'
	//	comm.tcpsocket->cleanUp();	
	//}

	if(servo%100 == 1) {
		comm.Update_MA2UI(pos_delta, rl2sui, servo);			// Update SUI data
		comm.Send_TCP();
	}
	if(servo%1000 == 1) {
		//printf("f:%d s:%d IP:%d t:%d Omni_t:%d\n", UI2Madata.flag01, UI2Madata.scale, UI2Madata.UDPaddr, (int)UI2Madata.tick, servo );
		/*printf("delPos[omni1]: %d %d %d\tdelPos[omni2]: %d %d %d\n",
		int(round(pos_delta[0][0]*THOUSAND)),
		int(round(pos_delta[0][1]*THOUSAND)),
		int(round(pos_delta[0][2]*THOUSAND)),
		int(round(pos_delta[1][0]*THOUSAND)),
		int(round(pos_delta[1][1]*THOUSAND)),
		int(round(pos_delta[1][2]*THOUSAND)));*/
	}

	if (HD_DEVICE_ERROR(error = hdGetError())) {
		std::cerr << "an error occurred A" << std::endl;
		//hduPrintError(stderr, &error, "Error during main scheduler callback\n");
		//		if (hduIsSchedulerError(&error))
		//return HD_CALLBACK_DONE;
	}

	return HD_CALLBACK_CONTINUE;
}




/**
*    TESTING MODE
*/
HDCallbackCode HDCALLBACK Testing_2(void *data)
{
	servo++;
	int nButtons = 0, bttn[2];
	hduVector3Dd position[2];
	hduVector3Dd rotation[2];
	HDErrorInfo error;
	int footpedal = g_footpedal;

	hdBeginFrame(omniId_1);					// Get OMNI 1 stuff
	hdMakeCurrentDevice(omniId_1);

	// Get button state using bitmask:
	hdGetIntegerv(HD_CURRENT_BUTTONS, &nButtons);
	if ( nButtons & HD_DEVICE_BUTTON_2 ) // button 2 releases grasp
		gold_button_graspON = false;
	else if ( nButtons & HD_DEVICE_BUTTON_1 ) // button 1 engages grasp
		gold_button_graspON = true;

	// Get omni position in pedal dn state
	if( footpedal ) {	// footpedal is down and SUI is running.
		hdGetDoublev(HD_CURRENT_POSITION, position[OMNI1]);		// get omni1 position
		hdGetDoublev(HD_CURRENT_GIMBAL_ANGLES, rotation[OMNI1]);	// get omni1 orientation

		applyTransforms(position[OMNI1]);				// Rotation to match omni frame with camera view.
		hduVecScaleInPlace(position[OMNI1],scale);		// position scaling
		hduVecSubtract(pos_delta[OMNI1],position[OMNI1],p_position[OMNI1]);		// delta = current - previous
		hduVecSubtract(rot_delta[OMNI1],rotation[OMNI1],p_rotation[OMNI1]);		// delta = current - previous

		p_position[OMNI1][0] = position[OMNI1][0];
		p_position[OMNI1][1] = position[OMNI1][1];
		p_position[OMNI1][2] = position[OMNI1][2];
		p_rotation[OMNI1][0] = rotation[OMNI1][0];
		p_rotation[OMNI1][1] = rotation[OMNI1][1];
		p_rotation[OMNI1][2] = rotation[OMNI1][2];

		bttn[OMNI1] = gold_button_graspON;
	} 

	// Send zero deltas in pedal up state.  (send last button state)
	else 
	{
		hduVector3Dd zero_pos(0,0,0);

		indexing=true;
		hduVecSet(p_position[OMNI1],0.0,0.0,0.0);
		hduVecSet(p_rotation[OMNI1],0.0,0.0,0.0);
		hduVecSet(pos_delta[OMNI1],0.0,0.0,0.0);
		hduVecSet(rot_delta[OMNI1],0.0,0.0,0.0);
		bttn[OMNI1] = gold_button_graspON;
	}

	//hdEndFrame(omniId_1);

	//////////// For the second omni device ////////////
	//hdBeginFrame(omniId_2);
	//hdMakeCurrentDevice(omniId_2);

	// Get button state using bitmask:
	hdGetIntegerv(HD_CURRENT_BUTTONS, &nButtons);
	if ( nButtons & HD_DEVICE_BUTTON_2 ) // button 2 releases grasp
		green_button_graspON = false;
	else if ( nButtons & HD_DEVICE_BUTTON_1 ) // button 1 engages grasp
		green_button_graspON = true;

	// Get omni position in pedal dn state
	if( footpedal ) 
	{	// footpedal is down and SUI is running.
		hdGetDoublev(HD_CURRENT_POSITION, position[OMNI2]);		// get omni2 position
		hdGetDoublev(HD_CURRENT_GIMBAL_ANGLES, rotation[OMNI2]);	// get omni2 orientation

		applyTransforms(position[OMNI2]);				// Rotation to match omni frame with camera view.
		hduVecScaleInPlace(position[OMNI2],scale);		// position scaling
		hduVecSubtract(pos_delta[OMNI2],position[OMNI2],p_position[OMNI2]);		// delta = current - previous
		hduVecSubtract(rot_delta[OMNI2],rotation[OMNI2],p_rotation[OMNI2]);		// delta = current - previous

		p_position[OMNI2][0] = position[OMNI2][0];
		p_position[OMNI2][1] = position[OMNI2][1];
		p_position[OMNI2][2] = position[OMNI2][2];
		p_rotation[OMNI2][0] = rotation[OMNI2][0];
		p_rotation[OMNI2][1] = rotation[OMNI2][1];
		p_rotation[OMNI2][2] = rotation[OMNI2][2];

		bttn[OMNI2] = green_button_graspON;
	} 

	// Send zero deltas in pedal up state.  (send last button state)
	else
	{
		hduVector3Dd zero_pos(0,0,0);
		indexing=true;
		hduVecSet(p_position[OMNI2],0.0,0.0,0.0);
		hduVecSet(p_rotation[OMNI2],0.0,0.0,0.0);
		hduVecSet(pos_delta[OMNI2],0.0,0.0,0.0);
		hduVecSet(rot_delta[OMNI2],0.0,0.0,0.0);

		bttn[OMNI2] = green_button_graspON;
	}

	//hdEndFrame(omniId_2);
	hdEndFrame(omniId_1);
	///////////////////////////////////////////////////

//	comm.Update_UDP_Data(pos_delta, rot_delta, bttn, footpedal, servo );						// Update robot command packet
//	comm.Send_UDP();

	if (HD_DEVICE_ERROR(error = hdGetError())) {
		std::cerr << "an error occurred 1" << std::endl;
		//hduPrintError(stderr, &error, "Error during main scheduler callback\n");
	}

	if(servo%1000 == 1) {
		printf("Sent 1000 things\n");
	}


	return HD_CALLBACK_CONTINUE;
}

/**
*    TESTING MODE
*/
HDCallbackCode HDCALLBACK Testing(void *data)
{
	servo++;
	int bttn[2], footpedal;
	HDErrorInfo error;
	double d=0;
	static int countloop = 0;
	const double scale = 0.02;


	countloop++;
	if (countloop<1000) 
		d=-0.02;
	else	
		d = scale * sin( 2* 3.1415 * countloop/2000);

	hduVecSet(pos_delta[OMNI2], d, d, d );
	hduVecSet(rot_delta[OMNI2], 0.0, 0.0, 0.0);
	hduVecSet(pos_delta[OMNI1], d, d, d);
	hduVecSet(rot_delta[OMNI1], 0.0, 0.0, 0.0 );

	footpedal = g_footpedal;
	bttn[OMNI2] = FALSE;
	bttn[OMNI1] = FALSE;

//	comm.Update_UDP_Data(pos_delta, rot_delta, bttn, footpedal, servo );						// Update robot command packet
//	comm.Send_UDP();

	if(servo%1000 == 1) {
		printf("Sent 1000 things\n");
	}

	if (HD_DEVICE_ERROR(error = hdGetError())) {
		std::cerr << "an error occurred 2" << std::endl;
		//hduPrintError(stderr, &error, "Error during main scheduler callback\n");
	}

	return HD_CALLBACK_CONTINUE;
}

/******************************************************************************
main function
Initialize the device, create a callback to handle sphere forces, terminate
upon key press.
******************************************************************************/
int main(int argc, char* argv[])
{
	char *input="testing";

	if(argc>=2){
		if(strcmp(argv[1],input)!=0){
			printf("The input format is omniclient [testing] [0[GoldArm]/1[GreenArm]]\n");
			return -1;
		}

		oneOmni=true;
		if(argc>=3 && atoi(argv[2])==1){
			greenArm=true;
			goldArm=false;
			printf("You Have Selected Green  Arm to Control \n");}
		else {
			greenArm=false;
			goldArm=true;
			printf("You Have Selected Gold Arm (or default) to Control \n");
		}
	}

	else{
		printf("The Master is Selected to Work in Normal Mode\n");
		oneOmni=false;
		goldArm=true;
		greenArm=true;
	}

	HDErrorInfo error;
	char ch = 'r';
	int hstdin = (int) GetStdHandle(STD_INPUT_HANDLE);
	fd_set readSet;
	TIMEVAL timeout = {TIMEOUT_SECS, TIMEOUT_USECS};
	int nfound;
	int the_tcpsock;

	// initialize communication with the robot //
	comm.Initialize_UDP_Robot(argc, argv); 

	while(!comm.Initialize_TCP_GUI()) {
	cout << "TCPIP initialization failed retry in 5 sec." << endl;
	Sleep(5);
	//return 1;
	}
	cout << "KS : TCPIP initialized " << endl;

	// Initialize FD_SET //
	FD_ZERO(&readSet);
	the_tcpsock = comm.tcpsocket->sockDesc;
	FD_SET(the_tcpsock, &readSet);

	omniId_1 = hdInitDevice(OMNI_NAME_1);			// Initialize the first omni device 
	if (HD_DEVICE_ERROR(error = hdGetError())) 
	{
		std::cerr << "an error occurred 3" << std::endl;
		//hduPrintError(stderr, &error, "Failed to initialize first haptic device");
		fprintf(stderr, "\nPress any key to quit.\n");
		_getch();
		return -1;

	}
	hdEnable(HD_FORCE_OUTPUT);				// enable force for the first device
	omniId_2 = hdInitDevice(OMNI_NAME_2);			// Initialize the first omni device 
	if (HD_DEVICE_ERROR(error = hdGetError())) 
	{
			std::cerr << "an error occurred 4" << std::endl;
			//hduPrintError(stderr, &error, "Failed to initialize second haptic device");
			fprintf(stderr, "\nPress any key to quit.\n");
			_getch();
			return -1;
	}
	hdEnable(HD_FORCE_OUTPUT);				// enable force for the first device

	if(oneOmni && rightDevice && leftDevice){
		printf("You Are Trying to Run Testing With Both Omni's ON.\n We Are Defualting To Left Omni\n");
		rightDevice=false;
	}

	hdSetSchedulerRate(1000);				// Set the scheduler rate
	if (HD_DEVICE_ERROR(error = hdGetError())) {
		std::cerr << "an error occurred 5" << std::endl;
		//hduPrintError(stderr, &error, "Failed to start scheduler");
		fprintf(stderr, "\nPress any key to quit.\n");
		_getch();
		return -1;
	}

	// Debug for variables
	if(oneOmni)
		printf("One Omni mode is enabled\n");
	if(greenArm)
		printf("Green Arm is Controlled\n");
	if(goldArm)
		printf("Gold Arm is Controlled\n");

	HDSchedulerHandle hMasterCallback;
	// Application loop - schedule our call to the main callback. //
	if(!oneOmni){
		printf("The Master is Operating on Normal Mode With Two Omni's\n");
		hMasterCallback = hdScheduleAsynchronous(
			PositionUpdate, 0, HD_MAX_SCHEDULER_PRIORITY);		// Setup timer callback function
	}
	else{
		printf("The Master is Operating With One Omni\n");
		hMasterCallback = hdScheduleAsynchronous(
			PositionUpdate, 0, HD_MAX_SCHEDULER_PRIORITY);
	}

	hdStartScheduler();					// Start the servo loop scheduler
	cout << "KS : Scheduler started." << endl;

	while ( ch != 'q') {// 	ENABLE ME   && comm.Check_Flag(BASIC_PROGRAM) ) {
		timeout.tv_sec = TIMEOUT_SECS;
		timeout.tv_usec = TIMEOUT_USECS;
		nfound = select( 1 , &readSet, NULL, NULL, &timeout );		// check the TCP Socket
		if ( nfound < 0 ) {						// Socket error
			cerr << "Socket error "<<nfound<<" : "<<WSAGetLastError()<<" \n";
			cout << "sockedesc:"<< comm.tcpsocket->sockDesc<<endl;
			//			Sleep(10);
			//			break;
		}
		if ( nfound > 0 && FD_ISSET(the_tcpsock, &readSet)) {			// TCP socket ready to read
			//cout << "recv TCP...\n";
			comm.Recv_TCP();
			comm.Check_UI2MA(0);// 0 means not to dispay the message
		} 
		FD_CLR(the_tcpsock,&readSet);					// Reset fd_set
		FD_SET(the_tcpsock,&readSet);					//  ""

		if( _kbhit() )							// check for keyboard input`
		{
			ch = _getch();
			switch(ch)
			{
			case 'd':
				cout << "set pedal down\n";
				g_footpedal = 1;
				break;
			case 'e':
				cout << "set pedal up\n";
				g_footpedal = 0;
				break;
			}
		}
		if (!hdWaitForCompletion(hMasterCallback, HD_WAIT_CHECK_STATUS)) {
			fprintf(stderr, "\nThe main scheduler callback has exited\n");
			fprintf(stderr, "\nPress any key to quit.\n");
			_getch();
			break;
		}
	}

	try {
		comm.tcpsocket->cleanUp();
	} 
	catch (SocketException SE) {
		cerr<< "ERROR: Socket cleanUp failed.\n";
	}

	hdStopScheduler();					// Stop the servo loop scheduler
	hdUnschedule(hMasterCallback);		// un-schedule callback function
	hdDisableDevice(omniId_1);			// Disable omni 1
	hdDisableDevice(omniId_2);			// Disable omni 2

	cout << "Ended application loop\n Press any key and exit." << endl;
	_getch();
	return 0;
} // end: main


