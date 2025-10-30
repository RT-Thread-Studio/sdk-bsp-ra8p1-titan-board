/*
 * Copyright (c) 2006-2021, RT-Thread Development Team
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2025-08-19     RTT          the first version
 */
#ifndef SRC_YOLO_RTTHREAD_H_
#define SRC_YOLO_RTTHREAD_H_

#include <rtthread.h>
#include <math.h>

// 模型相关参数
#define EPSILON                1e-7f
#define MAX_BOXES              16        /* 最大检测框数量，用于NMS */
#define CLASS_NUM              1
#define CONF_THRESH            0.8f
#define NMS_THRESH             0.45f
#define IOU_MODE_DIou          1        /* 1: IoU, 2: DIoU */
#define INPUT_W               192
#define INPUT_H               192
#define INPUT_SIZE INPUT_W*INPUT_H
#define GRID_SIZE_1 6
#define GRID_SIZE_2 12
#define ANCHORS 3 // 每个网格使用3个anchor

// anchor
static const int anchors[2][6] = {
    {37, 94, 83, 83, 60, 137},  // 第一组YOLO预测的anchors (mask 3,4,5)
    {15, 22, 24, 51, 60, 44}    // 第二组YOLO预测的anchors (mask 0,1,2)
};

#ifndef D2_FIX
#define D2_FIX(px) ((d2_point)((px) << 4))
#endif
#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif
#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef CLAMP
#define CLAMP(v,lo,hi) ( ((v)<(lo))?(lo):(((v)>(hi))?(hi):(v)) )
#endif

// 量化参数
static const float scale_in = 0.007843137718737125;       /* 模型输入量化的scale */
static const int16_t zero_point_in = -1;                   /* 模型输入量化的zero point */
static const float scale_out2 = 0.16396920382976532;
static const int16_t zero_point_out2 = 34;
static const float scale_out1 = 0.14357048273086548;
static const int16_t zero_point_out1 = 26;

// 输出长度
static const int16_t output1_len = GRID_SIZE_1 * GRID_SIZE_1 * (3 * (5 + CLASS_NUM));
static const int16_t output2_len = GRID_SIZE_2 * GRID_SIZE_2 * (3 * (5 + CLASS_NUM));

typedef struct
{
    int16_t x1, y1, x2, y2;
    float score;
    uint8_t cls;
} det_box_t;

static inline float sigmoidf_fast(float x)
{
    return 1.0f / (1.0f + expf(-x));
}

/**
 * 解码网络输出
 * @param out_f         输出浮点数据，长度 = grid*grid*3*(5+CLASS_NUM)
 * @param grid          网格大小，6 或 12
 * @param anchor_group  当前使用的 anchor 组，取值 0 或 1，每组 3 个 anchor
 * @param img_w,img_h   原始图片宽高
 * @param conf_thresh   置信度阈值，一般 >= 0.5
 * @param out_boxes     解码后的检测框数组
 * @param max_out       out_boxes 的最大数量
 * @return              实际解码出的框数量
 */
int16_t decode_output_layer(const float *out_f,
                               int16_t grid,
                               int16_t anchor_group,
                               int16_t img_w, int16_t img_h,
                               float conf_thresh,
                               det_box_t *out_boxes,
                               int16_t max_out);

void dequantize_int8(const int8_t *in, float *out,
                            size_t len, float scale, int32_t zp);

int8_t* image_quantization_task(uint8_t *in_frame);

void rgb565_to_gray_resize_192_and_quantization(const uint16_t *src, int16_t src_w, int16_t src_h, int8_t *i8_buf);

/**
 * NMS，支持IoU 或 DIoU
 * @param boxes        待筛选/检测的框
 * @param n            框数量
 * @param iou_thresh   阈值
 * @return             筛选后的框数量，不超过 MAX_BOXES
 */
int16_t nms_filter(det_box_t *boxes, int16_t n, float iou_thresh);

void sort_boxes_by_score(det_box_t *boxes, int16_t n);

float iou_rect(const det_box_t *a, const det_box_t *b);

float diou_rect(const det_box_t *a, const det_box_t *b);

void sort_boxes_by_score(det_box_t *boxes, int16_t n);

#endif /* SRC_YOLO_RTTHREAD_H_ */
