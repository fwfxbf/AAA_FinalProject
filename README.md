# 电控B组的结营项目

## 一.文件结构

```
Robot/
├── Core/                 CubeMX 生成
├── Drivers/              ST HAL
├── Middlewares/          USB Host 库（有 USER PATCH）
├── User/                 ★ 自己写的代码全部放这里（CubeMX 不会碰）
│   ├── BSP/              P4：串口、时间、LED
│   │   ├── bsp_uart.c/.h
│   │   └── bsp_time.c/.h
│   ├── PAD/              P4：手柄抽象
│   │   ├── pad.h
│   │   ├── pad_xinput.c
│   │   └── pad_switch.c
│   ├── CHASSIS/          P1：底盘控制
│   │   ├── chassis.c/.h
│   │   ├── drv_motor.c/.h
│   │   └── drv_encoder.c/.h
│   ├── SENSOR/           P2：循迹
│   │   ├── track.c/.h
│   │   ├── line_pid.c/.h
│   │   └── bsp_line.c/.h
│   ├── ARM/              P3：机械臂
│   │   ├── arm.c/.h
│   │   ├── drv_servo.c/.h
│   │   └── arm_action.c/.h
│   └── APP/              P4：模式机、调度
│       ├── app_main.c/.h
│       └── app_mode.c/.h
└── USB_HOST/             P4：XInput 驱动（CubeMX 生成结构 + 我们加的类）
    ├── usb_host.c/.h
    └── usbh_xinput.c/.h
```

**每个目录 = 一个人的地盘。** 别人只能 `#include` 它的头文件，不能进去改 `.c`。


## 二.引脚分配(括号内为User Label)

### 麦轮
FL:Front-Left FR:Front-Right RL:Rear-Left RR:Rear-Right\
PE0(FL_IN1)前左轮IN1  PE9(FL_PWMM)前左轮PWM\
PE1(FL_IN2)前左轮IN2\
PE2(FR_IN1)前右轮IN1  PE11(FR_PWM)前右轮PWM\
PE3(FR_IN1)前右轮IN2\
PE4(RL_IN1)后左轮IN1  PE13(RL_PWM)后左轮PWM\
PE5(RL_IN1)后左轮IN2\
PE6(RR_IN1)后右轮IN1  PE14(RR_PWM)后右轮PWM\
PE7(RR_IN1)后右轮IN2
