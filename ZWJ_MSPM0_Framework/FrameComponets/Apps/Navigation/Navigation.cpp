#include "Navigation.hpp"
#include "MainStateMachine.hpp"
#include "System.hpp"
#include "bsp_delay.h"
#include "bsp_dwt.h"
#include "std_math.hpp"

// 长度单位为mm，角度单位为°
Navigation &navigation_app = Navigation::GetInstance();

void Navigation::Start() {
    this->bluetooth.Init(UART1);
    this->bluetooth.Enable();
}

void Navigation::Update() {
    if (!is_enabled)
        return;
    ExecuteStep();
}

void Navigation::SetEnable(bool enable) {
    is_enabled = enable;
}

void Navigation::Navigation_Control() {
    this->left_speed_cmd = BASE_SPEED + 25.0f;
    this->right_speed_cmd = BASE_SPEED;

    speed_mixer.SetNavigationSpeed(left_speed_cmd, right_speed_cmd);
}

void Navigation::ExecuteStep() {
    switch (current_step) {
    case STEP_IDLE:
        if (this->bluetooth.rx_buf[0] == 0xFF) {
            this->bluetooth.rx_buf[0] = 0;
            current_step = STEP_Recievex;
        }
        break;

    case STEP_Recievex: {
        if (this->bluetooth.rx_buf[0] != 0) {
            target_pos.x = ((uint8_t) (this->bluetooth.rx_buf[0])) * 100;
            this->bluetooth.rx_buf[0] = 0;
            current_step = STEP_Recievey;
            is_vaild = true;

        }

        break;
    }

    case STEP_Recievey: {
        if (this->bluetooth.rx_buf[0] != 0) {
            target_pos.y = ((uint8_t) (this->bluetooth.rx_buf[0])) * 100;
            this->bluetooth.rx_buf[0] = 0;

            float dx = target_pos.x - current_pos.x;
            float dy = target_pos.y - current_pos.y;
            
            float angle_rad = atan2f(dx, dy);
            target_angle = angle_rad * 180.0f / PI;
            
            target_angle = fmod(target_angle, 360.0f);
            if (target_angle > 180.0f)
                target_angle -= 360.0f;
            else if (target_angle < -180.0f)
                target_angle += 360.0f;
            dis = (sqrt((target_pos.x - 100) * (target_pos.x - 100) + (target_pos.y - 100) * (target_pos.y - 100)) /
                   1000);
            time = (dis / StdMath::RpmToMS(6.5, 90)); //可调参数
        } // 只有当y不为0的时候才进行坐标转换

        if (target_pos.x != 0 && target_pos.y != 0) {
            // 判断是否已对准（阈值可调整）
            this->left_speed_cmd = 36.5f; //可调参数
            this->right_speed_cmd = 0.0f;
            speed_mixer.SetNavigationSpeed(left_speed_cmd, right_speed_cmd);
            BspDelay_ms(target_angle * 32.5); //可调参数

            //             // 判断是否已对准（阈值可调整）
            // this->left_speed_cmd = 0.0f; //可调参数
            // this->right_speed_cmd = -20.0f;
            // speed_mixer.SetNavigationSpeed(left_speed_cmd, right_speed_cmd);
            // BspDelay_ms(target_angle * 38.36); //可调参数

            speed_mixer.SetNavigationSpeed(0.0, 0.0);
            current_step = STEP_StartNavi;
        } 

        break;
    }

    case STEP_StartNavi: {
        this->left_speed_cmd = BASE_SPEED +6.0f ; //可调参数
        this->right_speed_cmd = BASE_SPEED;
        speed_mixer.SetNavigationSpeed(left_speed_cmd, right_speed_cmd);
        if (is_no_enter) {
            first_enter_time = BspDwt_GetTimeline_Sec();
            is_no_enter = false;
        }

        if (BspDwt_GetTimeline_Sec() - first_enter_time > time) {
            current_step = STEP_FinishNavi;
        }

        break;
    }
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
    current_pos = {100.0f, 100.0f};
    bluetooth.rx_buf[0] = 0x00;
}