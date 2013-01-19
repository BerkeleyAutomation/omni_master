/*

File: cameraAngle.h
Author: Hawkeye King
Written June-2008

This file implements rotation from the Omni frame to the camera view angle.

*/
void applyTransforms(hduVector3Dd &pos);
void omni2ITPTransform(hduMatrix &xform);

void convertToITP(hduVector3Dd &pos);