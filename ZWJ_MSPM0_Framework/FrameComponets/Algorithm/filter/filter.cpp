#include "filter.hpp"

// 冒泡排序（嵌入式轻量实现）
static void BubbleSort(float *arr, uint8_t len) {
    for (uint8_t i = 0; i < len - 1; i++) {
        for (uint8_t j = 0; j < len - i - 1; j++) {
            if (arr[j] > arr[j + 1]) {
                float temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }
}

// 滑动平均滤波
float Filter::MovingAverage(float *buffer, float newVal, uint8_t windowSize, uint8_t &index) {
    if (windowSize == 0 || windowSize > FILTER_MAX_WINDOW_SIZE || buffer == nullptr) {
        return newVal;
    }

    // 存入新数据
    buffer[index] = newVal;
    index = (index + 1) % windowSize;

    // 计算平均值
    float sum = 0.0f;
    uint8_t validNum = 0;
    for (uint8_t i = 0; i < windowSize; i++) {
        // 过滤无效值(-1)
        if (buffer[i] >= 0.0f) {
            sum += buffer[i];
            validNum++;
        }
    }

    return (validNum > 0) ? (sum / validNum) : -1.0f;
}

// 中值滤波（修复VLA错误，用固定最大窗口数组）
float Filter::Median(float *buffer, float newVal, uint8_t windowSize, uint8_t &index) {
    if (windowSize % 2 == 0 || windowSize == 0 || windowSize > FILTER_MAX_WINDOW_SIZE || buffer == nullptr) {
        return newVal;
    }

    // 存入新数据
    buffer[index] = newVal;
    index = (index + 1) % windowSize;

    // 拷贝临时数组排序（用固定最大窗口大小，兼容C++标准）
    float tempBuf[FILTER_MAX_WINDOW_SIZE];
    memcpy(tempBuf, buffer, windowSize * sizeof(float));
    BubbleSort(tempBuf, windowSize);

    // 返回中值
    return tempBuf[windowSize / 2];
}

// 一阶互补滤波
float Filter::FirstOrderComplementary(float newVal, float lastFilteredVal, float alpha) {
    if (newVal < 0) {
        return lastFilteredVal; // 无效值不更新
    }
    return alpha * newVal + (1 - alpha) * lastFilteredVal;
}