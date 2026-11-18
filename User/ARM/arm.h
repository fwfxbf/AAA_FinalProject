#ifndef ARM_H
#define ARM_H

#include<stdbool.h>
#include<stdint.h>

#define ARM_JOINTS 6

typedef enum { J_BASE=0,J_SHOULDER,J_ELBOW,J_WRIST_P,J_WRIST_R,J_GRIPPER }ArmJoint;

void Arm_Init(void);
void Arm_MoveJoint(ArmJoint j,int16_t deg,uint16_t ms);
void Arm_MoveTo(const int16_t target[ARM_JOINTS],uint16_t ms);
void Arm_PlayAction(uint8_t action_id);
bool Arm_IsBusy(void);
void Arm_Tick(void);

#endif
