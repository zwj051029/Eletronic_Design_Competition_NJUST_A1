#pragma once

#include "SysDefs.hpp"
#include "System.hpp"
#include "filter.hpp"
#include "pid.hpp"
#include "std_math.hpp"
#include "std_sensor.hpp"

class Track : public Application {
    SINGLETON(Track) : Application("Track") {
        prescaler = 1;
    };
    APPLICATION_OVERRIDE;

public:
    float track_error = 0.0f;
    float speed_diff = 0.0f;
    float base_speed = 80.0f; // 电机基础速度

    bool is_dashed_line = false; // 是否处于虚线
    bool is_finish_line = false; // 终点/起点线（中间四黑）
    App::Status last_status = App::Normal;

    void SetEnable(bool enable) {
        is_enabled = enable;
    }
    void SetBaseSpeed(float speed);
    App::Status GetStatus() override;

private:
    GpioSensor gray_sensor;
    Pids track_pid;

    // 灰度权重（左负右正，中间靠近0）
    const float weights[8] = {-1.2f, -0.9f, -0.6f, -0.3f, 0.3f, 0.6f, 0.9f, 1.2f}; // 先用着，后面再调
    bool gray_state[8];                                                            // true=黑线(1), false=白线(0)

    float max_speed_diff = 10.0f;
    // 巡线灵敏度   数值太大：容易丢线；数值太小：容易误判、抖动
    float line_threshold = 0.3f; // 有效线宽阈值（权重和低于此值视为丢线）

    // 虚线检测变量
    uint16_t dash_gap_counter = 0;          // 中间传感器低电平累计帧数
    const uint16_t DASH_GAP_THRESHOLD = 10; // 低电平帧数阈值(识别为虚线空隙)
    const uint16_t DASH_HOLD_FRAMES = 20;   // 连续检测到线后保持的帧数(避免抖动)

    void ProcessGrayData();  // 处理灰度数据
    void DetectDashedLine(); // 识别虚线
    void DetectFinishLine(); // 检测十字线
    void ResetController();  // 重置控制器
};

extern Track &track_app;