#pragma once

#include <stdint.h>
#include <string.h>

// 定义滤波库支持的最大窗口大小（必须固定，C++不支持动态数组）
#define FILTER_MAX_WINDOW_SIZE 9

class Filter {
public:
    /**
     * @brief 滑动平均滤波（平滑数据，抑制随机噪声）
     * @param buffer 数据缓存
     * @param newVal 新采样值
     * @param windowSize 窗口大小(1~FILTER_MAX_WINDOW_SIZE)
     * @param index 缓存索引(引用传递)
     * @return 滤波后的值
     */
    static float MovingAverage(float *buffer, float newVal, uint8_t windowSize, uint8_t &index);

    /**
     * @brief 中值滤波（剔除突发跳变/异常值）
     * @param buffer 数据缓存
     * @param newVal 新采样值
     * @param windowSize 窗口大小(必须奇数，1~FILTER_MAX_WINDOW_SIZE)
     * @param index 缓存索引(引用传递)
     * @return 滤波后的值
     */
    static float Median(float *buffer, float newVal, uint8_t windowSize, uint8_t &index);

    /**
     * @brief 一阶互补滤波（平衡响应速度+平滑性，推荐超声波使用）
     * @param newVal 新采样值
     * @param lastFilteredVal 上一次滤波结果
     * @param alpha 滤波系数(0~1，越小越平滑)
     * @return 滤波后的值
     */
    static float FirstOrderComplementary(float newVal, float lastFilteredVal, float alpha);
};