#ifndef __ARM_H
#define __ARM_H

#include "main.h"
#include <stdbool.h>
#include <stdint.h>

#define ARM_JOINTS 6

typedef enum { J_BASE=0, J_SHOULDER, J_ELBOW, J_WRIST_P, J_WRIST_R, J_GRIPPER } ArmJoint;

void Arm_Init(void);

/* 单关节，异步：在 ms 毫秒内平滑转到 deg（不要瞬间跳） */
void Arm_MoveJoint(ArmJoint j, int16_t deg, uint16_t ms);

/* 多关节同时，用于定一个姿态 */
void Arm_MoveTo(const int16_t target[ARM_JOINTS], uint16_t ms);

/* 预存动作组（抓取、放下、回零） */
void Arm_PlayAction(uint8_t action_id);
bool Arm_IsBusy(void);

void Arm_Tick(void);

/* Arm_PlayAction 的 action_id 取值 */
enum {
    ARM_ACTION_GRAB = 0,     /* 伸出去 -> 闭合夹爪 -> 提起 */
    ARM_ACTION_RELEASE,      /* 伸出去 -> 张开夹爪 -> 回零 */
    ARM_ACTION_HOME,         /* 直接回零位 */
    ARM_ACTION_COUNT
};

#endif
