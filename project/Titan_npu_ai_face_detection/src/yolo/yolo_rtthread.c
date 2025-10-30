#include "yolo_rtthread.h"

int16_t decode_output_layer(const float *out_f,
                               int16_t grid,
                               int16_t anchor_group,
                               int16_t img_w, int16_t img_h,
                               float conf_thresh,
                               det_box_t *out_boxes,
                               int16_t max_out)
{
    const int16_t stride = (5 + CLASS_NUM);
    const int16_t cells = grid * grid;
    int16_t count = 0;

    int *anchor = anchors[anchor_group];

    for (int16_t i = 0; i < grid; ++i)
    {
        for (int16_t j = 0; j < grid; ++j)
        {
            for (int16_t k = 0; k < ANCHORS; ++k)
            {
                int16_t base = (((i * grid) + j) * ANCHORS + k) * stride;

                float tx   = out_f[base + 0];
                float ty   = out_f[base + 1];
                float tw   = out_f[base + 2];
                float th   = out_f[base + 3];
                float conf = out_f[base + 4];

                // 对ONNX/TFLite输出做Sigmoid归一化
                conf = sigmoidf_fast(conf);
                if (conf < conf_thresh)
                    continue;

                // 计算边界框，采用Darknet/YOLO公式
                float bx = (j + sigmoidf_fast(tx)) / grid;  // x中心坐标归一化
                float by = (i + sigmoidf_fast(ty)) / grid;  // y中心坐标归一化
                float bw = anchor[k * 2] * expf(tw) / INPUT_W; // 宽度归一化
                float bh = anchor[k * 2 + 1] * expf(th) / INPUT_H; // 高度归一化

                // 转为图像坐标
                int x1 = (int)((bx - bw / 2) * img_w);
                int y1 = (int)((by - bh / 2) * img_h);
                int x2 = (int)((bx + bw / 2) * img_w);
                int y2 = (int)((by + bh / 2) * img_h);

                // 边界裁剪
                x1 = CLAMP(x1, 0, img_w - 1);
                y1 = CLAMP(y1, 0, img_h - 1);
                x2 = CLAMP(x2, 0, img_w - 1);
                y2 = CLAMP(y2, 0, img_h - 1);

                if (x2 <= x1 || y2 <= y1)
                    continue;

                if (count < max_out)
                {
                    out_boxes[count].x1 = x1;
                    out_boxes[count].y1 = y1;
                    out_boxes[count].x2 = x2;
                    out_boxes[count].y2 = y2;
                    out_boxes[count].score = conf;
                    out_boxes[count].cls = 0; // 默认类别0
                    count++;
                }
            }
        }
    }
    RT_UNUSED(cells);
    return count;
}

void rgb565_to_gray_resize_192_and_quantization(const uint16_t *src, int16_t src_w, int16_t src_h, int8_t *i8_buf)
{
    const int16_t dst_w = 192;
    const int16_t dst_h = 192;

    for (int16_t y = 0; y < dst_h; y++)
    {
        int16_t sy = (y * src_h) / dst_h;           // 源图像对应的行
        const uint16_t *row = src + sy * src_w;

        for (int16_t x = 0; x < dst_w; x++)
        {
            int16_t sx = (x * src_w) / dst_w;       // 源图像对应的列
            uint16_t pix = row[sx];

            // 提取RGB565颜色
            uint8_t r = (pix >> 11) & 0x1F;
            uint8_t g = (pix >> 5) & 0x3F;
            uint8_t b = (pix) & 0x1F;

            // 扩展为8位
            r = (r << 3) | (r >> 2);
            g = (g << 2) | (g >> 4);
            b = (b << 3) | (b >> 2);

            // 灰度化公式 (ITU-R BT.601 标准加权)
            float gray = (float)( (r * 299 + g * 587 + b * 114) / 1000 );

            // 量化
            float xn = (gray - 127.5f) / 127.5f;
            int8_t qi = (int8_t)(xn / scale_in + (float)zero_point_in);
            if (qi < -128) qi = -128; else if (qi > 127) qi = 127;

            i8_buf[y * dst_w + x] = qi;
        }
    }
}

int8_t* image_quantization_task(uint8_t *in_frame)
{
    if (!in_frame) return RT_NULL;
    int32_t num_pixels = INPUT_W * INPUT_H;

    int8_t *i8_buf = (int8_t*)rt_malloc(num_pixels*sizeof(int8_t));
    if (!i8_buf) { rt_kprintf("[ERR] malloc i8_buf\n"); return RT_NULL; }

    for (int16_t i=0;i<num_pixels;i++){
        float xn = (((float)in_frame[i]) - 127.5f) / 127.5f;

        int8_t qi = (int8_t)(xn / scale_in + (float)zero_point_in);
        if (qi < -128) qi = -128; else if (qi > 127) qi = 127;
        i8_buf[i] = (int8_t)qi;
    }
    return i8_buf;
}

void dequantize_int8(const int8_t *in, float *out,
                            size_t len, float scale, int32_t zp)
{
    if (scale == 0.0) {
        // 直接把整数值转为浮点值
        for (size_t i = 0; i < len; i++) {
            out[i] = (float)in[i];
        }
    } else {
        // 使用量化公式进行反量化
        for (size_t i = 0; i < len; i++) {
            out[i] = (float)(in[i] - zp) * scale;
        }
    }
}

float iou_rect(const det_box_t *a, const det_box_t *b)
{
    int16_t xx1 = MAX(a->x1, b->x1);
    int16_t yy1 = MAX(a->y1, b->y1);
    int16_t xx2 = MIN(a->x2, b->x2);
    int16_t yy2 = MIN(a->y2, b->y2);

    int16_t w = xx2 - xx1;
    int16_t h = yy2 - yy1;
    if (w <= 0 || h <= 0) return 0.0f;

    float inter = (float)(w * h);
    float areaA = (float)((a->x2 - a->x1) * (a->y2 - a->y1));
    float areaB = (float)((b->x2 - b->x1) * (b->y2 - b->y1));
    float uni   = areaA + areaB - inter + EPSILON;

    return inter / uni;
}

float diou_rect(const det_box_t *a, const det_box_t *b)
{
    // IoU
    float iou = iou_rect(a, b);

    // 计算中心距离平方
    float ax = 0.5f * (a->x1 + a->x2);
    float ay = 0.5f * (a->y1 + a->y2);
    float bx = 0.5f * (b->x1 + b->x2);
    float by = 0.5f * (b->y1 + b->y2);
    float center_dist2 = (ax - bx) * (ax - bx) + (ay - by) * (ay - by);

    // 包围框最小矩形对角线平方
    int16_t x1 = MIN(a->x1, b->x1);
    int16_t y1 = MIN(a->y1, b->y1);
    int16_t x2 = MAX(a->x2, b->x2);
    int16_t y2 = MAX(a->y2, b->y2);
    float c2 = (float)((x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1)) + EPSILON;

    // DIoU = IoU - (中心距离^2 / 包围框对角线^2)
    return iou - center_dist2 / c2;
}

void sort_boxes_by_score(det_box_t *boxes, int16_t n)
{
    for (int16_t i = 0; i < n - 1; ++i)
    {
        int16_t best = i;
        for (int16_t j = i + 1; j < n; ++j)
            if (boxes[j].score > boxes[best].score) best = j;
        if (best != i)
        {
            det_box_t tmp = boxes[i];
            boxes[i] = boxes[best];
            boxes[best] = tmp;
        }
    }
}

int16_t nms_filter(det_box_t *boxes, int16_t n, float iou_thresh)
{
    if (n <= 0) return 0;

    sort_boxes_by_score(boxes, n);

    uint8_t removed_flags[GRID_SIZE_2*GRID_SIZE_2*3 + GRID_SIZE_1*GRID_SIZE_1*3]; // 记录被移除的框
    int16_t flags_n = MIN((int16_t)sizeof(removed_flags), n);
    memset(removed_flags, 0, flags_n);

    int16_t keep = 0;
    for (int16_t i = 0; i < n && keep < MAX_BOXES; ++i)
    {
        if (removed_flags[i]) continue;
        // 选择 boxes[i]
        for (int16_t j = i + 1; j < n; ++j)
        {
            if (removed_flags[j]) continue;

            float over;
#if IOU_MODE_DIou == 2
            over = diou_rect(&boxes[i], &boxes[j]);
            // DIoU 大于阈值时，将重叠框标记移除
            if (over > iou_thresh) removed_flags[j] = 1;
#else
            over = iou_rect(&boxes[i], &boxes[j]);
            if (over > iou_thresh) removed_flags[j] = 1;
#endif
        }
        // 把当前框移动到 keep 位置，保证数组前面都是保留框
        if (keep != i)
        {
            boxes[keep] = boxes[i];
        }
        keep++;
    }
    return keep;
}
