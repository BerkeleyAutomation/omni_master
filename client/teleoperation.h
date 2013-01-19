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

struct u_struct {
        unsigned int sequence;
//		int c_timestamp;
//		int s_timestamp;
        int delx[2];
        int dely[2];
        int delz[2];
        int delyaw[2];
        int delpitch[2];
        int delroll[2];
        int buttonstate[2];
        int footpedal;
        int checksum;
/*   unsigned int sequence; */
/*   int delx;         // X coordinate */
/*   int dely;         // Y coordinate */
/*   int delz;         // Z coordiante */
/*   int checksum; */
};

struct v_struct {
  unsigned int sequence;
  float fx;         // X coordinate
  float fy;         // Y coordinate
  float fz;         // Z coordiante
  int runlevel;
  int checksum;
};

#endif //teleoperation_h

