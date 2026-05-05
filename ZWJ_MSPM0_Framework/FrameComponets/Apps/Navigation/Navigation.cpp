#include "Navigation.hpp"
// 长度单位为mm，角度单位为°
Navigation &navigation_app = Navigation::GetInstance();

void Navigation::Start() {
    // mpu6050初始化及校准
    Sensor_calibration();
    // bluetooth初始化
    this->bluetooth.Init(UART1);
    this->bluetooth.Enable();
    // pid初始化
    yaw_pid.Init(2.5f, 0.1f, 0.08f, false);
    yaw_pid.SetLimit(5.0f, 80.0f, 0.9f);
}

void Navigation::Update() {
    if (!is_enabled)
        return;
    ExecuteStep();
}

void Navigation::SetEnable(bool enable) {
    if (enable && !is_enabled) {
        ResetStateMachine();
    }
    is_enabled = enable;
    if (!enable) {
    speed_mixer.SetNavigationSpeed(0, 0);
}
}

void Navigation::Sensor_calibration() {
    // mpu6050初始化及校准
    MPU6050_Init();
    MPU6050Gyro_calibrate();
}

// 可以保持和角度更新的频率一样，2ms
void Navigation::odometry_update() {
    float s_avg = (s_left + s_right) / 2.0f;
    float delta_theta = (s_left - s_right) / 156.5f * 180.0f / PI;
    // 可选：融合IMU和里程计角度（比如加权），而非直接覆盖
    current_angle = 0.8 * MPU6050_Getyaw() + 0.2 * (current_angle + delta_theta);

    current_pos.x += s_avg * sinf(current_angle * PI / 180.0f);
    current_pos.y += s_avg * cosf(current_angle * PI / 180.0f);
}
// 10ms一次
void Navigation::Navigation_Control() {
    // 1. 读取IMU角度（基准）
    float imu_yaw = MPU6050_Getyaw();

    this->s_left = motor_left.delta * 0.2617f;
    this->s_right = motor_right.delta * 0.2617f;
    // 3. 里程计更新（角度融合：IMU为主，里程计增量为辅，或择一）
    odometry_update();
    // 4. 角度误差归一化（必须补全）
    float error_theta = target_angle - imu_yaw;
    error_theta = fmod(error_theta, 360.0f);
    if (error_theta > 180)
        error_theta -= 360;
    if (error_theta < -180)
        error_theta += 360;
    // 5. PID计算（用IMU角度作为反馈）
    float delta_v = yaw_pid.Calc(0.0f, error_theta, 0.01f);

    this->left_speed_cmd = BASE_SPEED + delta_v;
    this->right_speed_cmd = BASE_SPEED - delta_v;

    speed_mixer.SetNavigationSpeed(left_speed_cmd, right_speed_cmd);
}

void Navigation::ExecuteStep() {
    switch (current_step) {
    case STEP_IDLE:
        if (this->bluetooth.rx_buf[0] == 0XFF) {
            current_step = STEP_RecieveCmd;
        }
        break;

    case STEP_RecieveCmd: {
        if ((this->bluetooth.rx_buf[1] != 0) && (this->bluetooth.rx_buf[2] != 0)) {
            target_pos.x = (uint8_t) this->bluetooth.rx_buf[1] * 100;
            target_pos.y = (uint8_t) this->bluetooth.rx_buf[2] * 100;
            // 解析十六进制数，要加一个坐标和实际距离的换算
            this->target_angle = target_pos.Angle();
        }
        float angle_error = fabs(current_angle - target_angle);
        // 角度误差归一化（修正±180°跨域问题）
        angle_error = fmod(angle_error, 360.0f);
        angle_error = (angle_error > 180) ? 360 - angle_error : angle_error;
        // 替代原while循环，改为单次判断+状态流转
        if (angle_error <= 3) {
            current_step = STEP_StartNavi; // 角度到位，进入前进
        } else {
            // 持续转向（PID或固定速度，避免单次执行）
            // 改用PID计算转向速度，或固定差速转向（需取消最小速度限幅，或临时调整）
            float delta_v = yaw_pid.Calc(0.0f, target_angle - current_angle, 0.01f);
            this->left_speed_cmd = delta_v;
            this->right_speed_cmd = - delta_v;

            speed_mixer.SetNavigationSpeed(left_speed_cmd, right_speed_cmd);
            // 保持当前状态，持续转向直到到位
            current_step = STEP_RecieveCmd;
        }
        break;
    }
    case STEP_StartNavi:
        Navigation_Control();
        if ((fabs(current_pos.x - target_pos.x) <= 500) && (fabs(current_pos.y - target_pos.y) <= 500)) {
            current_step = STEP_FinishNavi;
        }
        break;

    case STEP_FinishNavi:
        left_speed_cmd = 0.0f;
        right_speed_cmd = 0.0f;
        speed_mixer.SetNavigationSpeed(left_speed_cmd, right_speed_cmd);
        is_complete = true;
        break;
    }
}

void Navigation::ResetStateMachine() {
    current_step = STEP_IDLE;
    is_complete = false;
    left_speed_cmd = 0.0f;
    right_speed_cmd = 0.0f;
    current_pos = {0.0f, 0.0f};
    yaw_pid.Reset();
}