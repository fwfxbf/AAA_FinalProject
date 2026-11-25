#ifndef APP_MODE_H
#define APP_MODE_H



typedef enum {
    MODE_IDLE = 0,   /* 停车待命(左十字键) */
    MODE_TRACK,      /* 循迹(LS) */
    MODE_REMOTE,     /* 手柄遥控(RS) */
} AppMode;


void App_Mode_Init(void);   /* 初始化模式机 */


void App_Mode_Update(void); /* 更新模式机 */


AppMode App_Mode_Get(void); /* 获取当前模式 */

#endif