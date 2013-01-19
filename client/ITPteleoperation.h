/*********************************************
*
*
*  teleoperation.h
*
*    I define datastructures representing the 
*  information passed between master and slave 
*  in teleoperation.
*
*  Based on the wave variables naming schema:
*
*  u_struct passes from master to slave
*  v_struct passes from slave to master
*
*********************************************/

#ifndef TELEOPERATION_H
#define TELEOPERATION_H

//#pragma pack(push, 1)
#pragma pack(1)


/*
u_struct : structure passed from master to slave.
This struct defines an incremental movment packet type.

sequence     Packet's sequence number
pactyp       protocol version in use 
version      Protocol version number  (***SRI) 

delx[2]	     position increment
dely[2]
delz[2]
buttonstate[2]
grasp[2]        +32767 = 100% closing torque, -32768 = 100% opening
surgeon_mode    SURGEON_ENGAGED or SURGEON_DISENGAGED  (formerly Pedal_Down or Pedal_UP)
checksum
*/

struct u_struct {
	unsigned int sequence;
	unsigned int pactyp;  
	unsigned int version;

	int delx[2];
	int dely[2];
	int delz[2];
	double Qx[2];
	double Qy[2];
	double Qz[2];
	double Qw[2];
	int buttonstate[2];
	int grasp[2];         
	int surgeon_mode;     
	int checksum; 
};

/*
v_struct: Return DS from slave to master.
sequence
pactyp        protocol version in use
version       Protocol version number  (***SRI)
fx            X force
fy            Y force
fz            Z force
runlevel      Slave operating state
jointflags    bit flags for each joint limit (up to 16 joints).
checksum
*/
struct v_struct {
	unsigned int sequence;
	unsigned int pactyp;
	unsigned int version;
	int fx;
	int fy;
	int fz;
	int runlevel;
	unsigned int  jointflags;
	int checksum;
};

 // Calculate checksum for ITS packet type 1
inline int UDPChecksumI(struct u_struct *u)
{
  int chk=0;
  chk =  (u->surgeon_mode);
  chk += (u->delx[0])+(u->dely[0])+(u->delz[0]);
  chk += (u->delx[1])+(u->dely[1])+(u->delz[1]);
  chk += (u->buttonstate[0]);
  chk += (u->buttonstate[1]);
  chk += (int)(u->sequence);
  chk += (u->grasp[0])+(u->grasp[1]);
  return chk;
}

//#pragma pack(pop)

#endif //teleoperation_h

