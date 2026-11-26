#ifndef APP_MODE_H
#define APP_MODE_H



typedef enum {
    MODE_IDLE = 0,   /* 停车 */
    MODE_TRACK,      /* 循迹 */
    MODE_REMOTE,     /* 手柄遥控底盘 */
    MODE_ARM,        /* 手柄遥控机械臂 */
} AppMode;


void App_Mode_Init(void);   /* 初始化模式机 */


void App_Mode_Update(void); /* 更新模式机 */


AppMode App_Mode_Get(void); /* 获取当前模式 */

#endif