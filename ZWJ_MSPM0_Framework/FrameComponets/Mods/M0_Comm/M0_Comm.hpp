#ifndef M0_COMM_HPP
#define M0_COMM_HPP

#include <stdint.h>

class M0Comm {
public:
    void Init();
    float GetTargetLeft() const {
        return target_left_;
    }
    float GetTargetRight() const {
        return target_right_;
    }
    bool IsNewFrame() const {
        return new_frame_;
    }
    void ClearNewFrame() {
        new_frame_ = false;
    }
    void ProcessRxByte(uint8_t byte);

private:
    volatile float target_left_ = 0.0f;
    volatile float target_right_ = 0.0f;
    volatile bool new_frame_ = false;

    enum State { WAIT_HEADER, READING } state_ = WAIT_HEADER;
    uint8_t frame_buf_[9];
    uint8_t buf_idx_ = 0;
};

extern M0Comm m0_comm;

#ifdef __cplusplus
extern "C" {
#endif
// C 兼容接口：不暴露 C++ 类，仅传递字节给内部解析
void M0Comm_ProcessByte(uint8_t byte);
#ifdef __cplusplus
}
#endif

#endif