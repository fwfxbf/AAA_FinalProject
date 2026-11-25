#ifndef __CHASSIS_H
#define __CHASSIS_H
#include <stdint.h>
/* ================================坐标系===========================================
 车体俯视（x 轴指向车头，y 轴指向车左，右手系 z 向上）：
 *
 *              车头 (vx > 0)
 *                  ▲
 *                  │
 *      FL ◣        │        ◢ FR       
 *                  │
 *   车左(vy>0) ◀───┼───▶ 车右(vy<0)
 *                  │
 *      RL ◤        │        ◥ RR
 *                  ▼
 *              车尾 (vx < 0)
 *
 * Chassis_SetVelocity(vx, vy, w) 的量纲与正方向：
 *   vx : 正 = 向车头方向平移      单位 mm/s    
 *   vy : 正 = 向车左侧平移        单位 mm/s    
 *   w  : 正 = 逆时针旋转(俯视)    单位 度/秒   
 *        
 * 四个轮子编号（与 MotorId_t 的 WHEEL_FL/FR/RL/RR 一一对应）：
 *   FL = 左前    FR = 右前
 *   RL = 左后    RR = 右后
 *
 * 符号速查（记忆法：平移看"往哪边走"，自转看"左右反向"）：
 *   (vx>0, vy=0, w=0)  → 四轮同速正转            前进
 *   (vx<0, vy=0, w=0)  → 四轮同速反转            后退
 *   (vx=0, vy>0, w=0)  → FL/RR 反转、FR/RL 正转  左移
 *   (vx=0, vy=0, w>0)  → FL/RL 反转、FR/RR 正转  左转（逆时针）
 *  ==========================================================================*/
typedef enum {
    CHASSIS_STOP = 0,      /* ??(????,????) */
    CHASSIS_FWD,           /* ??:??? */
    CHASSIS_BACK,          /* ?? */
    CHASSIS_LEFT,          /* ???:??? */
    CHASSIS_RIGHT,         /* ??? */
    CHASSIS_TURN_LEFT,     /* ????(???) */
    CHASSIS_TURN_RIGHT,    /* ????(???) */
    CHASSIS_MOTION_NUM
} Chassis_Motion_t;
void  Chassis_Init(void);
void  Chassis_SetVelocity(int16_t vx, int16_t vy, int16_t w);
void  Chassis_Stop(void);
void  Chassis_GetWheelSpeed(int16_t out[4]); 
void  Chassis_Task(void);
void Chassis_Move(Chassis_Motion_t dir, int16_t speed);
#endif