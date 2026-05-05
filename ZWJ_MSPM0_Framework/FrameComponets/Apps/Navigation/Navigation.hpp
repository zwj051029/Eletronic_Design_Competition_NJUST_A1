#pragma once

#include "Positioner.hpp"
#include "SysDefs.hpp"
#include "System.hpp"
#include "SpeedMixer.hpp"
#include "bluetooth.hpp"
#include "mpu6050.hpp"
#include "pid.hpp"
#include "std_math.hpp"
#include "bsp_delay.h"
#include "motor_at8236.hpp"

class Navigation : public Application {
    friend class RobotSystem;

    SINGLETON(Navigation) : Application("Navigation") {
        prescaler = 1;
    };
    APPLICATION_OVERRIDE;

private:
    typedef enum NavigationStep {
        STEP_IDLE = 0,       // 空闲
        STEP_RecieveCmd = 1, // 接收命令
        STEP_StartNavi = 2,  // 开始导航
        STEP_FinishNavi = 3  // 结束导航
    } Step;

    BlueTooth bluetooth;

    const uint32_t BASE_SPEED = 60.0f; // 默认速度

    Pids yaw_pid;

public:
    bool is_enabled = false;
    bool is_complete = false;

    Vec2 target_pos{0.0f, 0.0f};
    Vec2 current_pos{0.0f, 0.0f};

    float target_angle = 0.0f;
    float current_angle = 0.0f;

    float target_odometry = 0.0f;
    float current_odometry = 0.0f;

    float s_left = 0.0f;
    float s_right = 0.0f;

    float left_speed_cmd = 0.0f;
    float right_speed_cmd = 0.0f;

    Step current_step = STEP_IDLE;

    void SetEnable(bool enable);
    
    void Sensor_calibration();
    void odometry_update();
    void Navigation_Control();

    void ExecuteStep();
    void ResetStateMachine();
};

extern Navigation &navigation_app;