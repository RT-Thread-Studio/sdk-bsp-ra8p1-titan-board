#include <rtthread.h>
#include "hal_data.h"
#include <rtdevice.h>

#include <board.h>
#include "sensor.h"
#include <lcd_port.h>
#include "model.h"
#include "pmu_ethosu.h"
#include "yolo_rtthread.h"

#define DBG_TAG     "main"
#define DBG_LVL     DBG_INFO
#include "rtdbg.h"

#define LED_PIN     BSP_IO_PORT_00_PIN_12

#define CAM_WIDTH   640
#define CAM_HEIGHT  480

static volatile bool led_status = false;

extern sensor_t sensor;
static uint8_t g_image_rgb565_sdram_buffer[CAM_WIDTH * CAM_HEIGHT * 2] BSP_PLACE_IN_SECTION(".ospi1_cs0_noinit") BSP_ALIGN_VARIABLE(8);


static void lcd_draw_jpg_with_frame(int32_t x, int32_t y,
                             const void *p, int32_t xSize, int32_t ySize,
                             uint32_t argb, d2_width thickness, det_box_t *boxes, int32_t box_num)
{
    if (thickness < 1) thickness = 1;

    uint32_t ModeSrc = d2_mode_rgb565;
    d2_device *hdl = d2_handle_obj_get();

    // 设定帧缓冲（背景/目标）
    d2_framebuffer(hdl, (uint16_t *)&fb_background[0],
                   LCD_WIDTH, LCD_WIDTH, LCD_HEIGHT, ModeSrc);

    // 选择渲染缓冲并设置裁剪区域（全屏）
    d2_selectrenderbuffer(hdl, d2_renderbuffer_get());
    d2_cliprect(hdl, 0, 0, LCD_WIDTH, LCD_HEIGHT);

    // 设定源图并把整帧图像 Blit 到屏幕
    d2_setblitsrc(hdl, (void *)p, xSize, xSize, ySize, ModeSrc);
    d2_blitcopy(hdl, xSize, ySize, 0, 0,
            D2_FIX(LCD_WIDTH), D2_FIX(LCD_HEIGHT),
                D2_FIX(x), D2_FIX(y), 0);


    // 颜色
    d2_setcolor(hdl, 0, argb);
    //
    d2_width drwa_thickness = D2_FIX(thickness);
    d2_u32 flags = d2_le_exclude_none;

    for(int i=0;i<box_num;i++){
        d2_point x1 = D2_FIX(boxes->x1);
        d2_point y1 = D2_FIX(boxes->y1);
        d2_point x2 = D2_FIX(boxes->x2);
        d2_point y2 = D2_FIX(boxes->y2);

        d2_renderline(hdl, x1, y1, x2, y1, drwa_thickness, flags);
        d2_renderline(hdl, x1, y1, x1, y2, drwa_thickness, flags);
        d2_renderline(hdl, x2, y1, x2, y2, drwa_thickness, flags);
        d2_renderline(hdl, x1, y2, x2, y2, drwa_thickness, flags);
    }

    d2_executerenderbuffer(hdl, d2_renderbuffer_get(), 0);
    d2_flushframe(hdl);
}

void hal_entry(void)
{
    LOG_I("========================================================================");
    LOG_I("This example project is an an npu accelerated ai face detection routine!");
    LOG_I("========================================================================");

    sensor_init();
    sensor_reset();
    sensor_set_pixformat(PIXFORMAT_RGB565);
    sensor_set_framesize(FRAMESIZE_VGA);

    // 使能 NPU PMU
    int16_t status = FSP_SUCCESS;
    status = RM_ETHOSU_Open(&g_rm_ethosu0_ctrl, &g_rm_ethosu0_cfg);
    if (status != FSP_SUCCESS)
    {
        LOG_E("Failed to start NPU");
        return;
    }

    int8_t *in_i8 = (int8_t *)rt_malloc(INPUT_W * INPUT_H * sizeof(int8_t));

    if (!in_i8)
    {
        LOG_E("malloc in_i8 failed!");
        return;
    }

    float *out_f1 = (float *)rt_malloc(output1_len * sizeof(float));
    float *out_f2 = (float *)rt_malloc(output2_len * sizeof(float));

    while(1)
    {
        rt_tick_t start = rt_tick_get();

        sensor_snapshot(&sensor, g_image_rgb565_sdram_buffer, 0);

        rgb565_to_gray_resize_192_and_quantization(g_image_rgb565_sdram_buffer, CAM_WIDTH, CAM_HEIGHT, in_i8);

        memcpy(GetModelInputPtr_serving_default_image_input_0(), in_i8, INPUT_SIZE);

        RunModel(false);

        int8_t* output1 = GetModelOutputPtr_StatefulPartitionedCall_0_70273(); // 6
        int8_t* output2 = GetModelOutputPtr_StatefulPartitionedCall_1_70283(); // 12

        dequantize_int8(output1, out_f1, output1_len, scale_out1, zero_point_out1);
        dequantize_int8(output2, out_f2, output2_len, scale_out2, zero_point_out2);

        int16_t total = 0;
        static det_box_t pool[540]; // 6*6*3 + 12*12*3 = 108 + 432 = 540

        total += decode_output_layer(out_f1, GRID_SIZE_1, 0, LCD_WIDTH, LCD_HEIGHT, CONF_THRESH,
                pool + total, (int16_t)(sizeof(pool)/sizeof(pool[0])) - total);

        total += decode_output_layer(out_f2, GRID_SIZE_2, 1, LCD_WIDTH, LCD_HEIGHT, CONF_THRESH,
                pool + total, (int16_t)(sizeof(pool)/sizeof(pool[0])) - total);
        // nms
        int32_t kept = nms_filter(pool, total, NMS_THRESH);
        int32_t out_n = MIN(kept, MAX_BOXES); // 拷贝前 kept（不超过 MAX_BOXES）

        d2_width thickness = 1;
        uint32_t argb = 0xFF00FF00;
        lcd_draw_jpg_with_frame(0, 0, g_image_rgb565_sdram_buffer, CAM_WIDTH, CAM_HEIGHT, argb, thickness, pool, out_n);

        rt_tick_t end = rt_tick_get();
        rt_kprintf("detect box num: %d\n", out_n);
        rt_kprintf("Time elapsed: %d ms\n", (end - start) * (1000 / RT_TICK_PER_SECOND));

        rt_thread_mdelay(5);

        led_status = !led_status;
        if(led_status)
        {
            rt_pin_write(LED_PIN, PIN_HIGH);
        }
        else
        {
            rt_pin_write(LED_PIN, PIN_LOW);
        }
    }
}
