/* generated HAL source file - do not edit */
#include "hal_data.h"
/* Macros to tie dynamic ELC links to adc_b_trigger_sync_elc option in adc_b_trigger_t. */
#define ADC_B_TRIGGER_ADC_B0        ADC_B_TRIGGER_SYNC_ELC
#define ADC_B_TRIGGER_ADC_B0_B      ADC_B_TRIGGER_SYNC_ELC
#define ADC_B_TRIGGER_ADC_B1        ADC_B_TRIGGER_SYNC_ELC
#define ADC_B_TRIGGER_ADC_B1_B      ADC_B_TRIGGER_SYNC_ELC
#define RA_NOT_DEFINED (0)

#if (1) // Define Virtual Channel 0 if it's assigned to a scan group.
const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_0_cfg =
{ .channel_id = ADC_B_VIRTUAL_CHANNEL_0,

  .channel_cfg_bits.group = (1),
  .channel_cfg_bits.channel = ADC_CHANNEL_0,
  .channel_cfg_bits.differential = 0,
  .channel_cfg_bits.sample_table_id = ADC_B_SAMPLING_STATE_TABLE_0,

  .channel_control_a_bits.digital_filter_id = 0x0,
  .channel_control_a_bits.offset_table_id = ADC_B_USER_OFFSET_TABLE_SELECTION_DISABLED,
  .channel_control_a_bits.gain_table_id = ADC_B_USER_GAIN_TABLE_SELECTION_DISABLED,

  .channel_control_b_bits.addition_average_mode = (ADC_B_ADD_AVERAGE_OFF),
  .channel_control_b_bits.addition_average_count = ADC_B_ADD_AVERAGE_1,
  .channel_control_b_bits.compare_match_enable = false,

  .channel_control_c_bits.limiter_clip_table_id = ADC_B_LIMIT_CLIP_TABLE_SELECTION_NONE,
  .channel_control_c_bits.channel_data_format = (ADC_B_DATA_FORMAT_16_BIT),
  .channel_control_c_bits.data_is_unsigned = true, };
#endif

#if (1) // Define Virtual Channel 1 if it's assigned to a scan group.
const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_1_cfg =
{ .channel_id = ADC_B_VIRTUAL_CHANNEL_1,

  .channel_cfg_bits.group = (1),
  .channel_cfg_bits.channel = ADC_CHANNEL_1,
  .channel_cfg_bits.differential = 0,
  .channel_cfg_bits.sample_table_id = ADC_B_SAMPLING_STATE_TABLE_0,

  .channel_control_a_bits.digital_filter_id = 0x0,
  .channel_control_a_bits.offset_table_id = ADC_B_USER_OFFSET_TABLE_SELECTION_DISABLED,
  .channel_control_a_bits.gain_table_id = ADC_B_USER_GAIN_TABLE_SELECTION_DISABLED,

  .channel_control_b_bits.addition_average_mode = (ADC_B_ADD_AVERAGE_OFF),
  .channel_control_b_bits.addition_average_count = ADC_B_ADD_AVERAGE_1,
  .channel_control_b_bits.compare_match_enable = false,

  .channel_control_c_bits.limiter_clip_table_id = ADC_B_LIMIT_CLIP_TABLE_SELECTION_NONE,
  .channel_control_c_bits.channel_data_format = (ADC_B_DATA_FORMAT_16_BIT),
  .channel_control_c_bits.data_is_unsigned = true, };
#endif

#if (1) // Define Virtual Channel 2 if it's assigned to a scan group.
const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_2_cfg =
{ .channel_id = ADC_B_VIRTUAL_CHANNEL_2,

  .channel_cfg_bits.group = (1),
  .channel_cfg_bits.channel = ADC_CHANNEL_2,
  .channel_cfg_bits.differential = 0,
  .channel_cfg_bits.sample_table_id = ADC_B_SAMPLING_STATE_TABLE_0,

  .channel_control_a_bits.digital_filter_id = 0x0,
  .channel_control_a_bits.offset_table_id = ADC_B_USER_OFFSET_TABLE_SELECTION_DISABLED,
  .channel_control_a_bits.gain_table_id = ADC_B_USER_GAIN_TABLE_SELECTION_DISABLED,

  .channel_control_b_bits.addition_average_mode = (ADC_B_ADD_AVERAGE_OFF),
  .channel_control_b_bits.addition_average_count = ADC_B_ADD_AVERAGE_1,
  .channel_control_b_bits.compare_match_enable = false,

  .channel_control_c_bits.limiter_clip_table_id = ADC_B_LIMIT_CLIP_TABLE_SELECTION_NONE,
  .channel_control_c_bits.channel_data_format = (ADC_B_DATA_FORMAT_16_BIT),
  .channel_control_c_bits.data_is_unsigned = true, };
#endif

#if (1) // Define Virtual Channel 3 if it's assigned to a scan group.
const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_3_cfg =
{ .channel_id = ADC_B_VIRTUAL_CHANNEL_3,

  .channel_cfg_bits.group = (1),
  .channel_cfg_bits.channel = ADC_CHANNEL_3,
  .channel_cfg_bits.differential = 0,
  .channel_cfg_bits.sample_table_id = ADC_B_SAMPLING_STATE_TABLE_0,

  .channel_control_a_bits.digital_filter_id = 0x0,
  .channel_control_a_bits.offset_table_id = ADC_B_USER_OFFSET_TABLE_SELECTION_DISABLED,
  .channel_control_a_bits.gain_table_id = ADC_B_USER_GAIN_TABLE_SELECTION_DISABLED,

  .channel_control_b_bits.addition_average_mode = (ADC_B_ADD_AVERAGE_OFF),
  .channel_control_b_bits.addition_average_count = ADC_B_ADD_AVERAGE_1,
  .channel_control_b_bits.compare_match_enable = false,

  .channel_control_c_bits.limiter_clip_table_id = ADC_B_LIMIT_CLIP_TABLE_SELECTION_NONE,
  .channel_control_c_bits.channel_data_format = (ADC_B_DATA_FORMAT_16_BIT),
  .channel_control_c_bits.data_is_unsigned = true, };
#endif

#if (0) // Define Virtual Channel 4 if it's assigned to a scan group.
                               const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_4_cfg =
                               {
                                   .channel_id             = ADC_B_VIRTUAL_CHANNEL_4,

                                   .channel_cfg_bits.group           = (0),
                                   .channel_cfg_bits.channel         = ADC_CHANNEL_0,
                                   .channel_cfg_bits.differential    = 0,
                                   .channel_cfg_bits.sample_table_id = ADC_B_SAMPLING_STATE_TABLE_0,

                                   .channel_control_a_bits.digital_filter_id = 0x0,
                                   .channel_control_a_bits.offset_table_id = ADC_B_USER_OFFSET_TABLE_SELECTION_DISABLED,
                                   .channel_control_a_bits.gain_table_id = ADC_B_USER_GAIN_TABLE_SELECTION_DISABLED,

                                   .channel_control_b_bits.addition_average_mode = (ADC_B_ADD_AVERAGE_OFF),
                                   .channel_control_b_bits.addition_average_count = ADC_B_ADD_AVERAGE_1,
                                   .channel_control_b_bits.compare_match_enable = false,

                                   .channel_control_c_bits.limiter_clip_table_id = ADC_B_LIMIT_CLIP_TABLE_SELECTION_NONE,
                                   .channel_control_c_bits.channel_data_format = (ADC_B_DATA_FORMAT_12_BIT),
                                   .channel_control_c_bits.data_is_unsigned = true,
                               };
                               #endif

#if (0) // Define Virtual Channel 5 if it's assigned to a scan group.
                               const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_5_cfg =
                               {
                                   .channel_id             = ADC_B_VIRTUAL_CHANNEL_5,

                                   .channel_cfg_bits.group           = (0),
                                   .channel_cfg_bits.channel         = ADC_CHANNEL_0,
                                   .channel_cfg_bits.differential    = 0,
                                   .channel_cfg_bits.sample_table_id = ADC_B_SAMPLING_STATE_TABLE_0,

                                   .channel_control_a_bits.digital_filter_id = 0x0,
                                   .channel_control_a_bits.offset_table_id = ADC_B_USER_OFFSET_TABLE_SELECTION_DISABLED,
                                   .channel_control_a_bits.gain_table_id = ADC_B_USER_GAIN_TABLE_SELECTION_DISABLED,

                                   .channel_control_b_bits.addition_average_mode = (ADC_B_ADD_AVERAGE_OFF),
                                   .channel_control_b_bits.addition_average_count = ADC_B_ADD_AVERAGE_1,
                                   .channel_control_b_bits.compare_match_enable = false,

                                   .channel_control_c_bits.limiter_clip_table_id = ADC_B_LIMIT_CLIP_TABLE_SELECTION_NONE,
                                   .channel_control_c_bits.channel_data_format = (ADC_B_DATA_FORMAT_12_BIT),
                                   .channel_control_c_bits.data_is_unsigned = true,
                               };
                               #endif

#if (0) // Define Virtual Channel 6 if it's assigned to a scan group.
                               const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_6_cfg =
                               {
                                   .channel_id             = ADC_B_VIRTUAL_CHANNEL_6,

                                   .channel_cfg_bits.group           = (0),
                                   .channel_cfg_bits.channel         = ADC_CHANNEL_0,
                                   .channel_cfg_bits.differential    = 0,
                                   .channel_cfg_bits.sample_table_id = ADC_B_SAMPLING_STATE_TABLE_0,

                                   .channel_control_a_bits.digital_filter_id = 0x0,
                                   .channel_control_a_bits.offset_table_id = ADC_B_USER_OFFSET_TABLE_SELECTION_DISABLED,
                                   .channel_control_a_bits.gain_table_id = ADC_B_USER_GAIN_TABLE_SELECTION_DISABLED,

                                   .channel_control_b_bits.addition_average_mode = (ADC_B_ADD_AVERAGE_OFF),
                                   .channel_control_b_bits.addition_average_count = ADC_B_ADD_AVERAGE_1,
                                   .channel_control_b_bits.compare_match_enable = false,

                                   .channel_control_c_bits.limiter_clip_table_id = ADC_B_LIMIT_CLIP_TABLE_SELECTION_NONE,
                                   .channel_control_c_bits.channel_data_format = (ADC_B_DATA_FORMAT_12_BIT),
                                   .channel_control_c_bits.data_is_unsigned = true,
                               };
                               #endif

#if (0) // Define Virtual Channel 7 if it's assigned to a scan group.
                               const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_7_cfg =
                               {
                                   .channel_id             = ADC_B_VIRTUAL_CHANNEL_7,

                                   .channel_cfg_bits.group           = (0),
                                   .channel_cfg_bits.channel         = ADC_CHANNEL_0,
                                   .channel_cfg_bits.differential    = 0,
                                   .channel_cfg_bits.sample_table_id = ADC_B_SAMPLING_STATE_TABLE_0,

                                   .channel_control_a_bits.digital_filter_id = 0x0,
                                   .channel_control_a_bits.offset_table_id = ADC_B_USER_OFFSET_TABLE_SELECTION_DISABLED,
                                   .channel_control_a_bits.gain_table_id = ADC_B_USER_GAIN_TABLE_SELECTION_DISABLED,

                                   .channel_control_b_bits.addition_average_mode = (ADC_B_ADD_AVERAGE_OFF),
                                   .channel_control_b_bits.addition_average_count = ADC_B_ADD_AVERAGE_1,
                                   .channel_control_b_bits.compare_match_enable = false,

                                   .channel_control_c_bits.limiter_clip_table_id = ADC_B_LIMIT_CLIP_TABLE_SELECTION_NONE,
                                   .channel_control_c_bits.channel_data_format = (ADC_B_DATA_FORMAT_12_BIT),
                                   .channel_control_c_bits.data_is_unsigned = true,
                               };
                               #endif

#if (0) // Define Virtual Channel 8 if it's assigned to a scan group.
                               const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_8_cfg =
                               {
                                   .channel_id             = ADC_B_VIRTUAL_CHANNEL_8,

                                   .channel_cfg_bits.group           = (0),
                                   .channel_cfg_bits.channel         = ADC_CHANNEL_0,
                                   .channel_cfg_bits.differential    = 0,
                                   .channel_cfg_bits.sample_table_id = ADC_B_SAMPLING_STATE_TABLE_0,

                                   .channel_control_a_bits.digital_filter_id = 0x0,
                                   .channel_control_a_bits.offset_table_id = ADC_B_USER_OFFSET_TABLE_SELECTION_DISABLED,
                                   .channel_control_a_bits.gain_table_id = ADC_B_USER_GAIN_TABLE_SELECTION_DISABLED,

                                   .channel_control_b_bits.addition_average_mode = (ADC_B_ADD_AVERAGE_OFF),
                                   .channel_control_b_bits.addition_average_count = ADC_B_ADD_AVERAGE_1,
                                   .channel_control_b_bits.compare_match_enable = false,

                                   .channel_control_c_bits.limiter_clip_table_id = ADC_B_LIMIT_CLIP_TABLE_SELECTION_NONE,
                                   .channel_control_c_bits.channel_data_format = (ADC_B_DATA_FORMAT_12_BIT),
                                   .channel_control_c_bits.data_is_unsigned = true,
                               };
                               #endif

#if (0) // Define Virtual Channel 9 if it's assigned to a scan group.
                               const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_9_cfg =
                               {
                                   .channel_id             = ADC_B_VIRTUAL_CHANNEL_9,

                                   .channel_cfg_bits.group           = (0),
                                   .channel_cfg_bits.channel         = ADC_CHANNEL_0,
                                   .channel_cfg_bits.differential    = 0,
                                   .channel_cfg_bits.sample_table_id = ADC_B_SAMPLING_STATE_TABLE_0,

                                   .channel_control_a_bits.digital_filter_id = 0x0,
                                   .channel_control_a_bits.offset_table_id = ADC_B_USER_OFFSET_TABLE_SELECTION_DISABLED,
                                   .channel_control_a_bits.gain_table_id = ADC_B_USER_GAIN_TABLE_SELECTION_DISABLED,

                                   .channel_control_b_bits.addition_average_mode = (ADC_B_ADD_AVERAGE_OFF),
                                   .channel_control_b_bits.addition_average_count = ADC_B_ADD_AVERAGE_1,
                                   .channel_control_b_bits.compare_match_enable = false,

                                   .channel_control_c_bits.limiter_clip_table_id = ADC_B_LIMIT_CLIP_TABLE_SELECTION_NONE,
                                   .channel_control_c_bits.channel_data_format = (ADC_B_DATA_FORMAT_12_BIT),
                                   .channel_control_c_bits.data_is_unsigned = true,
                               };
                               #endif

#if (0) // Define Virtual Channel 10 if it's assigned to a scan group.
                               const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_10_cfg =
                               {
                                   .channel_id             = ADC_B_VIRTUAL_CHANNEL_10,

                                   .channel_cfg_bits.group           = (0),
                                   .channel_cfg_bits.channel         = ADC_CHANNEL_0,
                                   .channel_cfg_bits.differential    = 0,
                                   .channel_cfg_bits.sample_table_id = ADC_B_SAMPLING_STATE_TABLE_0,

                                   .channel_control_a_bits.digital_filter_id = 0x0,
                                   .channel_control_a_bits.offset_table_id = ADC_B_USER_OFFSET_TABLE_SELECTION_DISABLED,
                                   .channel_control_a_bits.gain_table_id = ADC_B_USER_GAIN_TABLE_SELECTION_DISABLED,

                                   .channel_control_b_bits.addition_average_mode = (ADC_B_ADD_AVERAGE_OFF),
                                   .channel_control_b_bits.addition_average_count = ADC_B_ADD_AVERAGE_1,
                                   .channel_control_b_bits.compare_match_enable = false,

                                   .channel_control_c_bits.limiter_clip_table_id = ADC_B_LIMIT_CLIP_TABLE_SELECTION_NONE,
                                   .channel_control_c_bits.channel_data_format = (ADC_B_DATA_FORMAT_12_BIT),
                                   .channel_control_c_bits.data_is_unsigned = true,
                               };
                               #endif

#if (0) // Define Virtual Channel 11 if it's assigned to a scan group.
                               const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_11_cfg =
                               {
                                   .channel_id             = ADC_B_VIRTUAL_CHANNEL_11,

                                   .channel_cfg_bits.group           = (0),
                                   .channel_cfg_bits.channel         = ADC_CHANNEL_0,
                                   .channel_cfg_bits.differential    = 0,
                                   .channel_cfg_bits.sample_table_id = ADC_B_SAMPLING_STATE_TABLE_0,

                                   .channel_control_a_bits.digital_filter_id = 0x0,
                                   .channel_control_a_bits.offset_table_id = ADC_B_USER_OFFSET_TABLE_SELECTION_DISABLED,
                                   .channel_control_a_bits.gain_table_id = ADC_B_USER_GAIN_TABLE_SELECTION_DISABLED,

                                   .channel_control_b_bits.addition_average_mode = (ADC_B_ADD_AVERAGE_OFF),
                                   .channel_control_b_bits.addition_average_count = ADC_B_ADD_AVERAGE_1,
                                   .channel_control_b_bits.compare_match_enable = false,

                                   .channel_control_c_bits.limiter_clip_table_id = ADC_B_LIMIT_CLIP_TABLE_SELECTION_NONE,
                                   .channel_control_c_bits.channel_data_format = (ADC_B_DATA_FORMAT_12_BIT),
                                   .channel_control_c_bits.data_is_unsigned = true,
                               };
                               #endif

#if (0) // Define Virtual Channel 12 if it's assigned to a scan group.
                               const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_12_cfg =
                               {
                                   .channel_id             = ADC_B_VIRTUAL_CHANNEL_12,

                                   .channel_cfg_bits.group           = (0),
                                   .channel_cfg_bits.channel         = ADC_CHANNEL_0,
                                   .channel_cfg_bits.differential    = 0,
                                   .channel_cfg_bits.sample_table_id = ADC_B_SAMPLING_STATE_TABLE_0,

                                   .channel_control_a_bits.digital_filter_id = 0x0,
                                   .channel_control_a_bits.offset_table_id = ADC_B_USER_OFFSET_TABLE_SELECTION_DISABLED,
                                   .channel_control_a_bits.gain_table_id = ADC_B_USER_GAIN_TABLE_SELECTION_DISABLED,

                                   .channel_control_b_bits.addition_average_mode = (ADC_B_ADD_AVERAGE_OFF),
                                   .channel_control_b_bits.addition_average_count = ADC_B_ADD_AVERAGE_1,
                                   .channel_control_b_bits.compare_match_enable = false,

                                   .channel_control_c_bits.limiter_clip_table_id = ADC_B_LIMIT_CLIP_TABLE_SELECTION_NONE,
                                   .channel_control_c_bits.channel_data_format = (ADC_B_DATA_FORMAT_12_BIT),
                                   .channel_control_c_bits.data_is_unsigned = true,
                               };
                               #endif

#if (0) // Define Virtual Channel 13 if it's assigned to a scan group.
                               const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_13_cfg =
                               {
                                   .channel_id             = ADC_B_VIRTUAL_CHANNEL_13,

                                   .channel_cfg_bits.group           = (0),
                                   .channel_cfg_bits.channel         = ADC_CHANNEL_0,
                                   .channel_cfg_bits.differential    = 0,
                                   .channel_cfg_bits.sample_table_id = ADC_B_SAMPLING_STATE_TABLE_0,

                                   .channel_control_a_bits.digital_filter_id = 0x0,
                                   .channel_control_a_bits.offset_table_id = ADC_B_USER_OFFSET_TABLE_SELECTION_DISABLED,
                                   .channel_control_a_bits.gain_table_id = ADC_B_USER_GAIN_TABLE_SELECTION_DISABLED,

                                   .channel_control_b_bits.addition_average_mode = (ADC_B_ADD_AVERAGE_OFF),
                                   .channel_control_b_bits.addition_average_count = ADC_B_ADD_AVERAGE_1,
                                   .channel_control_b_bits.compare_match_enable = false,

                                   .channel_control_c_bits.limiter_clip_table_id = ADC_B_LIMIT_CLIP_TABLE_SELECTION_NONE,
                                   .channel_control_c_bits.channel_data_format = (ADC_B_DATA_FORMAT_12_BIT),
                                   .channel_control_c_bits.data_is_unsigned = true,
                               };
                               #endif

#if (0) // Define Virtual Channel 14 if it's assigned to a scan group.
                               const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_14_cfg =
                               {
                                   .channel_id             = ADC_B_VIRTUAL_CHANNEL_14,

                                   .channel_cfg_bits.group           = (0),
                                   .channel_cfg_bits.channel         = ADC_CHANNEL_0,
                                   .channel_cfg_bits.differential    = 0,
                                   .channel_cfg_bits.sample_table_id = ADC_B_SAMPLING_STATE_TABLE_0,

                                   .channel_control_a_bits.digital_filter_id = 0x0,
                                   .channel_control_a_bits.offset_table_id = ADC_B_USER_OFFSET_TABLE_SELECTION_DISABLED,
                                   .channel_control_a_bits.gain_table_id = ADC_B_USER_GAIN_TABLE_SELECTION_DISABLED,

                                   .channel_control_b_bits.addition_average_mode = (ADC_B_ADD_AVERAGE_OFF),
                                   .channel_control_b_bits.addition_average_count = ADC_B_ADD_AVERAGE_1,
                                   .channel_control_b_bits.compare_match_enable = false,

                                   .channel_control_c_bits.limiter_clip_table_id = ADC_B_LIMIT_CLIP_TABLE_SELECTION_NONE,
                                   .channel_control_c_bits.channel_data_format = (ADC_B_DATA_FORMAT_12_BIT),
                                   .channel_control_c_bits.data_is_unsigned = true,
                               };
                               #endif

#if (0) // Define Virtual Channel 15 if it's assigned to a scan group.
                               const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_15_cfg =
                               {
                                   .channel_id             = ADC_B_VIRTUAL_CHANNEL_15,

                                   .channel_cfg_bits.group           = (0),
                                   .channel_cfg_bits.channel         = ADC_CHANNEL_0,
                                   .channel_cfg_bits.differential    = 0,
                                   .channel_cfg_bits.sample_table_id = ADC_B_SAMPLING_STATE_TABLE_0,

                                   .channel_control_a_bits.digital_filter_id = 0x0,
                                   .channel_control_a_bits.offset_table_id = ADC_B_USER_OFFSET_TABLE_SELECTION_DISABLED,
                                   .channel_control_a_bits.gain_table_id = ADC_B_USER_GAIN_TABLE_SELECTION_DISABLED,

                                   .channel_control_b_bits.addition_average_mode = (ADC_B_ADD_AVERAGE_OFF),
                                   .channel_control_b_bits.addition_average_count = ADC_B_ADD_AVERAGE_1,
                                   .channel_control_b_bits.compare_match_enable = false,

                                   .channel_control_c_bits.limiter_clip_table_id = ADC_B_LIMIT_CLIP_TABLE_SELECTION_NONE,
                                   .channel_control_c_bits.channel_data_format = (ADC_B_DATA_FORMAT_12_BIT),
                                   .channel_control_c_bits.data_is_unsigned = true,
                               };
                               #endif

#if (0) // Define Virtual Channel 16 if it's assigned to a scan group.
                               const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_16_cfg =
                               {
                                   .channel_id             = ADC_B_VIRTUAL_CHANNEL_16,

                                   .channel_cfg_bits.group           = (0),
                                   .channel_cfg_bits.channel         = ADC_CHANNEL_0,
                                   .channel_cfg_bits.differential    = 0,
                                   .channel_cfg_bits.sample_table_id = ADC_B_SAMPLING_STATE_TABLE_0,

                                   .channel_control_a_bits.digital_filter_id = 0x0,
                                   .channel_control_a_bits.offset_table_id = ADC_B_USER_OFFSET_TABLE_SELECTION_DISABLED,
                                   .channel_control_a_bits.gain_table_id = ADC_B_USER_GAIN_TABLE_SELECTION_DISABLED,

                                   .channel_control_b_bits.addition_average_mode = (ADC_B_ADD_AVERAGE_OFF),
                                   .channel_control_b_bits.addition_average_count = ADC_B_ADD_AVERAGE_1,
                                   .channel_control_b_bits.compare_match_enable = false,

                                   .channel_control_c_bits.limiter_clip_table_id = ADC_B_LIMIT_CLIP_TABLE_SELECTION_NONE,
                                   .channel_control_c_bits.channel_data_format = (ADC_B_DATA_FORMAT_12_BIT),
                                   .channel_control_c_bits.data_is_unsigned = true,
                               };
                               #endif

#if (0) // Define Virtual Channel 17 if it's assigned to a scan group.
                               const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_17_cfg =
                               {
                                   .channel_id             = ADC_B_VIRTUAL_CHANNEL_17,

                                   .channel_cfg_bits.group           = (0),
                                   .channel_cfg_bits.channel         = ADC_CHANNEL_0,
                                   .channel_cfg_bits.differential    = 0,
                                   .channel_cfg_bits.sample_table_id = ADC_B_SAMPLING_STATE_TABLE_0,

                                   .channel_control_a_bits.digital_filter_id = 0x0,
                                   .channel_control_a_bits.offset_table_id = ADC_B_USER_OFFSET_TABLE_SELECTION_DISABLED,
                                   .channel_control_a_bits.gain_table_id = ADC_B_USER_GAIN_TABLE_SELECTION_DISABLED,

                                   .channel_control_b_bits.addition_average_mode = (ADC_B_ADD_AVERAGE_OFF),
                                   .channel_control_b_bits.addition_average_count = ADC_B_ADD_AVERAGE_1,
                                   .channel_control_b_bits.compare_match_enable = false,

                                   .channel_control_c_bits.limiter_clip_table_id = ADC_B_LIMIT_CLIP_TABLE_SELECTION_NONE,
                                   .channel_control_c_bits.channel_data_format = (ADC_B_DATA_FORMAT_12_BIT),
                                   .channel_control_c_bits.data_is_unsigned = true,
                               };
                               #endif

#if (0) // Define Virtual Channel 18 if it's assigned to a scan group.
                               const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_18_cfg =
                               {
                                   .channel_id             = ADC_B_VIRTUAL_CHANNEL_18,

                                   .channel_cfg_bits.group           = (0),
                                   .channel_cfg_bits.channel         = ADC_CHANNEL_0,
                                   .channel_cfg_bits.differential    = 0,
                                   .channel_cfg_bits.sample_table_id = ADC_B_SAMPLING_STATE_TABLE_0,

                                   .channel_control_a_bits.digital_filter_id = 0x0,
                                   .channel_control_a_bits.offset_table_id = ADC_B_USER_OFFSET_TABLE_SELECTION_DISABLED,
                                   .channel_control_a_bits.gain_table_id = ADC_B_USER_GAIN_TABLE_SELECTION_DISABLED,

                                   .channel_control_b_bits.addition_average_mode = (ADC_B_ADD_AVERAGE_OFF),
                                   .channel_control_b_bits.addition_average_count = ADC_B_ADD_AVERAGE_1,
                                   .channel_control_b_bits.compare_match_enable = false,

                                   .channel_control_c_bits.limiter_clip_table_id = ADC_B_LIMIT_CLIP_TABLE_SELECTION_NONE,
                                   .channel_control_c_bits.channel_data_format = (ADC_B_DATA_FORMAT_12_BIT),
                                   .channel_control_c_bits.data_is_unsigned = true,
                               };
                               #endif

#if (0) // Define Virtual Channel 19 if it's assigned to a scan group.
                               const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_19_cfg =
                               {
                                   .channel_id             = ADC_B_VIRTUAL_CHANNEL_19,

                                   .channel_cfg_bits.group           = (0),
                                   .channel_cfg_bits.channel         = ADC_CHANNEL_0,
                                   .channel_cfg_bits.differential    = 0,
                                   .channel_cfg_bits.sample_table_id = ADC_B_SAMPLING_STATE_TABLE_0,

                                   .channel_control_a_bits.digital_filter_id = 0x0,
                                   .channel_control_a_bits.offset_table_id = ADC_B_USER_OFFSET_TABLE_SELECTION_DISABLED,
                                   .channel_control_a_bits.gain_table_id = ADC_B_USER_GAIN_TABLE_SELECTION_DISABLED,

                                   .channel_control_b_bits.addition_average_mode = (ADC_B_ADD_AVERAGE_OFF),
                                   .channel_control_b_bits.addition_average_count = ADC_B_ADD_AVERAGE_1,
                                   .channel_control_b_bits.compare_match_enable = false,

                                   .channel_control_c_bits.limiter_clip_table_id = ADC_B_LIMIT_CLIP_TABLE_SELECTION_NONE,
                                   .channel_control_c_bits.channel_data_format = (ADC_B_DATA_FORMAT_12_BIT),
                                   .channel_control_c_bits.data_is_unsigned = true,
                               };
                               #endif

#if (0) // Define Virtual Channel 20 if it's assigned to a scan group.
                               const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_20_cfg =
                               {
                                   .channel_id             = ADC_B_VIRTUAL_CHANNEL_20,

                                   .channel_cfg_bits.group           = (0),
                                   .channel_cfg_bits.channel         = ADC_CHANNEL_0,
                                   .channel_cfg_bits.differential    = 0,
                                   .channel_cfg_bits.sample_table_id = ADC_B_SAMPLING_STATE_TABLE_0,

                                   .channel_control_a_bits.digital_filter_id = 0x0,
                                   .channel_control_a_bits.offset_table_id = ADC_B_USER_OFFSET_TABLE_SELECTION_DISABLED,
                                   .channel_control_a_bits.gain_table_id = ADC_B_USER_GAIN_TABLE_SELECTION_DISABLED,

                                   .channel_control_b_bits.addition_average_mode = (ADC_B_ADD_AVERAGE_OFF),
                                   .channel_control_b_bits.addition_average_count = ADC_B_ADD_AVERAGE_1,
                                   .channel_control_b_bits.compare_match_enable = false,

                                   .channel_control_c_bits.limiter_clip_table_id = ADC_B_LIMIT_CLIP_TABLE_SELECTION_NONE,
                                   .channel_control_c_bits.channel_data_format = (ADC_B_DATA_FORMAT_12_BIT),
                                   .channel_control_c_bits.data_is_unsigned = true,
                               };
                               #endif

#if (0) // Define Virtual Channel 21 if it's assigned to a scan group.
                               const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_21_cfg =
                               {
                                   .channel_id             = ADC_B_VIRTUAL_CHANNEL_21,

                                   .channel_cfg_bits.group           = (0),
                                   .channel_cfg_bits.channel         = ADC_CHANNEL_0,
                                   .channel_cfg_bits.differential    = 0,
                                   .channel_cfg_bits.sample_table_id = ADC_B_SAMPLING_STATE_TABLE_0,

                                   .channel_control_a_bits.digital_filter_id = 0x0,
                                   .channel_control_a_bits.offset_table_id = ADC_B_USER_OFFSET_TABLE_SELECTION_DISABLED,
                                   .channel_control_a_bits.gain_table_id = ADC_B_USER_GAIN_TABLE_SELECTION_DISABLED,

                                   .channel_control_b_bits.addition_average_mode = (ADC_B_ADD_AVERAGE_OFF),
                                   .channel_control_b_bits.addition_average_count = ADC_B_ADD_AVERAGE_1,
                                   .channel_control_b_bits.compare_match_enable = false,

                                   .channel_control_c_bits.limiter_clip_table_id = ADC_B_LIMIT_CLIP_TABLE_SELECTION_NONE,
                                   .channel_control_c_bits.channel_data_format = (ADC_B_DATA_FORMAT_12_BIT),
                                   .channel_control_c_bits.data_is_unsigned = true,
                               };
                               #endif

#if (0) // Define Virtual Channel 22 if it's assigned to a scan group.
                               const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_22_cfg =
                               {
                                   .channel_id             = ADC_B_VIRTUAL_CHANNEL_22,

                                   .channel_cfg_bits.group           = (0),
                                   .channel_cfg_bits.channel         = ADC_CHANNEL_0,
                                   .channel_cfg_bits.differential    = 0,
                                   .channel_cfg_bits.sample_table_id = ADC_B_SAMPLING_STATE_TABLE_0,

                                   .channel_control_a_bits.digital_filter_id = 0x0,
                                   .channel_control_a_bits.offset_table_id = ADC_B_USER_OFFSET_TABLE_SELECTION_DISABLED,
                                   .channel_control_a_bits.gain_table_id = ADC_B_USER_GAIN_TABLE_SELECTION_DISABLED,

                                   .channel_control_b_bits.addition_average_mode = (ADC_B_ADD_AVERAGE_OFF),
                                   .channel_control_b_bits.addition_average_count = ADC_B_ADD_AVERAGE_1,
                                   .channel_control_b_bits.compare_match_enable = false,

                                   .channel_control_c_bits.limiter_clip_table_id = ADC_B_LIMIT_CLIP_TABLE_SELECTION_NONE,
                                   .channel_control_c_bits.channel_data_format = (ADC_B_DATA_FORMAT_12_BIT),
                                   .channel_control_c_bits.data_is_unsigned = true,
                               };
                               #endif

#if (0) // Define Virtual Channel 23 if it's assigned to a scan group.
                               const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_23_cfg =
                               {
                                   .channel_id             = ADC_B_VIRTUAL_CHANNEL_23,

                                   .channel_cfg_bits.group           = (0),
                                   .channel_cfg_bits.channel         = ADC_CHANNEL_0,
                                   .channel_cfg_bits.differential    = 0,
                                   .channel_cfg_bits.sample_table_id = ADC_B_SAMPLING_STATE_TABLE_0,

                                   .channel_control_a_bits.digital_filter_id = 0x0,
                                   .channel_control_a_bits.offset_table_id = ADC_B_USER_OFFSET_TABLE_SELECTION_DISABLED,
                                   .channel_control_a_bits.gain_table_id = ADC_B_USER_GAIN_TABLE_SELECTION_DISABLED,

                                   .channel_control_b_bits.addition_average_mode = (ADC_B_ADD_AVERAGE_OFF),
                                   .channel_control_b_bits.addition_average_count = ADC_B_ADD_AVERAGE_1,
                                   .channel_control_b_bits.compare_match_enable = false,

                                   .channel_control_c_bits.limiter_clip_table_id = ADC_B_LIMIT_CLIP_TABLE_SELECTION_NONE,
                                   .channel_control_c_bits.channel_data_format = (ADC_B_DATA_FORMAT_12_BIT),
                                   .channel_control_c_bits.data_is_unsigned = true,
                               };
                               #endif

#if (0) // Define Virtual Channel 24 if it's assigned to a scan group.
                               const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_24_cfg =
                               {
                                   .channel_id             = ADC_B_VIRTUAL_CHANNEL_24,

                                   .channel_cfg_bits.group           = (0),
                                   .channel_cfg_bits.channel         = ADC_CHANNEL_0,
                                   .channel_cfg_bits.differential    = 0,
                                   .channel_cfg_bits.sample_table_id = ADC_B_SAMPLING_STATE_TABLE_0,

                                   .channel_control_a_bits.digital_filter_id = 0x0,
                                   .channel_control_a_bits.offset_table_id = ADC_B_USER_OFFSET_TABLE_SELECTION_DISABLED,
                                   .channel_control_a_bits.gain_table_id = ADC_B_USER_GAIN_TABLE_SELECTION_DISABLED,

                                   .channel_control_b_bits.addition_average_mode = (ADC_B_ADD_AVERAGE_OFF),
                                   .channel_control_b_bits.addition_average_count = ADC_B_ADD_AVERAGE_1,
                                   .channel_control_b_bits.compare_match_enable = false,

                                   .channel_control_c_bits.limiter_clip_table_id = ADC_B_LIMIT_CLIP_TABLE_SELECTION_NONE,
                                   .channel_control_c_bits.channel_data_format = (ADC_B_DATA_FORMAT_12_BIT),
                                   .channel_control_c_bits.data_is_unsigned = true,
                               };
                               #endif

#if (0) // Define Virtual Channel 25 if it's assigned to a scan group.
                               const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_25_cfg =
                               {
                                   .channel_id             = ADC_B_VIRTUAL_CHANNEL_25,

                                   .channel_cfg_bits.group           = (0),
                                   .channel_cfg_bits.channel         = ADC_CHANNEL_0,
                                   .channel_cfg_bits.differential    = 0,
                                   .channel_cfg_bits.sample_table_id = ADC_B_SAMPLING_STATE_TABLE_0,

                                   .channel_control_a_bits.digital_filter_id = 0x0,
                                   .channel_control_a_bits.offset_table_id = ADC_B_USER_OFFSET_TABLE_SELECTION_DISABLED,
                                   .channel_control_a_bits.gain_table_id = ADC_B_USER_GAIN_TABLE_SELECTION_DISABLED,

                                   .channel_control_b_bits.addition_average_mode = (ADC_B_ADD_AVERAGE_OFF),
                                   .channel_control_b_bits.addition_average_count = ADC_B_ADD_AVERAGE_1,
                                   .channel_control_b_bits.compare_match_enable = false,

                                   .channel_control_c_bits.limiter_clip_table_id = ADC_B_LIMIT_CLIP_TABLE_SELECTION_NONE,
                                   .channel_control_c_bits.channel_data_format = (ADC_B_DATA_FORMAT_12_BIT),
                                   .channel_control_c_bits.data_is_unsigned = true,
                               };
                               #endif

#if (0) // Define Virtual Channel 26 if it's assigned to a scan group.
                               const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_26_cfg =
                               {
                                   .channel_id             = ADC_B_VIRTUAL_CHANNEL_26,

                                   .channel_cfg_bits.group           = (0),
                                   .channel_cfg_bits.channel         = ADC_CHANNEL_0,
                                   .channel_cfg_bits.differential    = 0,
                                   .channel_cfg_bits.sample_table_id = ADC_B_SAMPLING_STATE_TABLE_0,

                                   .channel_control_a_bits.digital_filter_id = 0x0,
                                   .channel_control_a_bits.offset_table_id = ADC_B_USER_OFFSET_TABLE_SELECTION_DISABLED,
                                   .channel_control_a_bits.gain_table_id = ADC_B_USER_GAIN_TABLE_SELECTION_DISABLED,

                                   .channel_control_b_bits.addition_average_mode = (ADC_B_ADD_AVERAGE_OFF),
                                   .channel_control_b_bits.addition_average_count = ADC_B_ADD_AVERAGE_1,
                                   .channel_control_b_bits.compare_match_enable = false,

                                   .channel_control_c_bits.limiter_clip_table_id = ADC_B_LIMIT_CLIP_TABLE_SELECTION_NONE,
                                   .channel_control_c_bits.channel_data_format = (ADC_B_DATA_FORMAT_12_BIT),
                                   .channel_control_c_bits.data_is_unsigned = true,
                               };
                               #endif

#if (0) // Define Virtual Channel 27 if it's assigned to a scan group.
                               const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_27_cfg =
                               {
                                   .channel_id             = ADC_B_VIRTUAL_CHANNEL_27,

                                   .channel_cfg_bits.group           = (0),
                                   .channel_cfg_bits.channel         = ADC_CHANNEL_0,
                                   .channel_cfg_bits.differential    = 0,
                                   .channel_cfg_bits.sample_table_id = ADC_B_SAMPLING_STATE_TABLE_0,

                                   .channel_control_a_bits.digital_filter_id = 0x0,
                                   .channel_control_a_bits.offset_table_id = ADC_B_USER_OFFSET_TABLE_SELECTION_DISABLED,
                                   .channel_control_a_bits.gain_table_id = ADC_B_USER_GAIN_TABLE_SELECTION_DISABLED,

                                   .channel_control_b_bits.addition_average_mode = (ADC_B_ADD_AVERAGE_OFF),
                                   .channel_control_b_bits.addition_average_count = ADC_B_ADD_AVERAGE_1,
                                   .channel_control_b_bits.compare_match_enable = false,

                                   .channel_control_c_bits.limiter_clip_table_id = ADC_B_LIMIT_CLIP_TABLE_SELECTION_NONE,
                                   .channel_control_c_bits.channel_data_format = (ADC_B_DATA_FORMAT_12_BIT),
                                   .channel_control_c_bits.data_is_unsigned = true,
                               };
                               #endif

#if (0) // Define Virtual Channel 28 if it's assigned to a scan group.
                               const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_28_cfg =
                               {
                                   .channel_id             = ADC_B_VIRTUAL_CHANNEL_28,

                                   .channel_cfg_bits.group           = (0),
                                   .channel_cfg_bits.channel         = ADC_CHANNEL_0,
                                   .channel_cfg_bits.differential    = 0,
                                   .channel_cfg_bits.sample_table_id = ADC_B_SAMPLING_STATE_TABLE_0,

                                   .channel_control_a_bits.digital_filter_id = 0x0,
                                   .channel_control_a_bits.offset_table_id = ADC_B_USER_OFFSET_TABLE_SELECTION_DISABLED,
                                   .channel_control_a_bits.gain_table_id = ADC_B_USER_GAIN_TABLE_SELECTION_DISABLED,

                                   .channel_control_b_bits.addition_average_mode = (ADC_B_ADD_AVERAGE_OFF),
                                   .channel_control_b_bits.addition_average_count = ADC_B_ADD_AVERAGE_1,
                                   .channel_control_b_bits.compare_match_enable = false,

                                   .channel_control_c_bits.limiter_clip_table_id = ADC_B_LIMIT_CLIP_TABLE_SELECTION_NONE,
                                   .channel_control_c_bits.channel_data_format = (ADC_B_DATA_FORMAT_12_BIT),
                                   .channel_control_c_bits.data_is_unsigned = true,
                               };
                               #endif

#if (0) // Define Virtual Channel 29 if it's assigned to a scan group.
                               const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_29_cfg =
                               {
                                   .channel_id             = ADC_B_VIRTUAL_CHANNEL_29,

                                   .channel_cfg_bits.group           = (0),
                                   .channel_cfg_bits.channel         = ADC_CHANNEL_0,
                                   .channel_cfg_bits.differential    = 0,
                                   .channel_cfg_bits.sample_table_id = ADC_B_SAMPLING_STATE_TABLE_0,

                                   .channel_control_a_bits.digital_filter_id = 0x0,
                                   .channel_control_a_bits.offset_table_id = ADC_B_USER_OFFSET_TABLE_SELECTION_DISABLED,
                                   .channel_control_a_bits.gain_table_id = ADC_B_USER_GAIN_TABLE_SELECTION_DISABLED,

                                   .channel_control_b_bits.addition_average_mode = (ADC_B_ADD_AVERAGE_OFF),
                                   .channel_control_b_bits.addition_average_count = ADC_B_ADD_AVERAGE_1,
                                   .channel_control_b_bits.compare_match_enable = false,

                                   .channel_control_c_bits.limiter_clip_table_id = ADC_B_LIMIT_CLIP_TABLE_SELECTION_NONE,
                                   .channel_control_c_bits.channel_data_format = (ADC_B_DATA_FORMAT_12_BIT),
                                   .channel_control_c_bits.data_is_unsigned = true,
                               };
                               #endif

#if (0) // Define Virtual Channel 30 if it's assigned to a scan group.
                               const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_30_cfg =
                               {
                                   .channel_id             = ADC_B_VIRTUAL_CHANNEL_30,

                                   .channel_cfg_bits.group           = (0),
                                   .channel_cfg_bits.channel         = ADC_CHANNEL_0,
                                   .channel_cfg_bits.differential    = 0,
                                   .channel_cfg_bits.sample_table_id = ADC_B_SAMPLING_STATE_TABLE_0,

                                   .channel_control_a_bits.digital_filter_id = 0x0,
                                   .channel_control_a_bits.offset_table_id = ADC_B_USER_OFFSET_TABLE_SELECTION_DISABLED,
                                   .channel_control_a_bits.gain_table_id = ADC_B_USER_GAIN_TABLE_SELECTION_DISABLED,

                                   .channel_control_b_bits.addition_average_mode = (ADC_B_ADD_AVERAGE_OFF),
                                   .channel_control_b_bits.addition_average_count = ADC_B_ADD_AVERAGE_1,
                                   .channel_control_b_bits.compare_match_enable = false,

                                   .channel_control_c_bits.limiter_clip_table_id = ADC_B_LIMIT_CLIP_TABLE_SELECTION_NONE,
                                   .channel_control_c_bits.channel_data_format = (ADC_B_DATA_FORMAT_12_BIT),
                                   .channel_control_c_bits.data_is_unsigned = true,
                               };
                               #endif

#if (0) // Define Virtual Channel 31 if it's assigned to a scan group.
                               const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_31_cfg =
                               {
                                   .channel_id             = ADC_B_VIRTUAL_CHANNEL_31,

                                   .channel_cfg_bits.group           = (0),
                                   .channel_cfg_bits.channel         = ADC_CHANNEL_0,
                                   .channel_cfg_bits.differential    = 0,
                                   .channel_cfg_bits.sample_table_id = ADC_B_SAMPLING_STATE_TABLE_0,

                                   .channel_control_a_bits.digital_filter_id = 0x0,
                                   .channel_control_a_bits.offset_table_id = ADC_B_USER_OFFSET_TABLE_SELECTION_DISABLED,
                                   .channel_control_a_bits.gain_table_id = ADC_B_USER_GAIN_TABLE_SELECTION_DISABLED,

                                   .channel_control_b_bits.addition_average_mode = (ADC_B_ADD_AVERAGE_OFF),
                                   .channel_control_b_bits.addition_average_count = ADC_B_ADD_AVERAGE_1,
                                   .channel_control_b_bits.compare_match_enable = false,

                                   .channel_control_c_bits.limiter_clip_table_id = ADC_B_LIMIT_CLIP_TABLE_SELECTION_NONE,
                                   .channel_control_c_bits.channel_data_format = (ADC_B_DATA_FORMAT_12_BIT),
                                   .channel_control_c_bits.data_is_unsigned = true,
                               };
                               #endif

#if (0) // Define Virtual Channel 32 if it's assigned to a scan group.
                               const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_32_cfg =
                               {
                                   .channel_id             = ADC_B_VIRTUAL_CHANNEL_32,

                                   .channel_cfg_bits.group           = (0),
                                   .channel_cfg_bits.channel         = ADC_CHANNEL_0,
                                   .channel_cfg_bits.differential    = 0,
                                   .channel_cfg_bits.sample_table_id = ADC_B_SAMPLING_STATE_TABLE_0,

                                   .channel_control_a_bits.digital_filter_id = 0x0,
                                   .channel_control_a_bits.offset_table_id = ADC_B_USER_OFFSET_TABLE_SELECTION_DISABLED,
                                   .channel_control_a_bits.gain_table_id = ADC_B_USER_GAIN_TABLE_SELECTION_DISABLED,

                                   .channel_control_b_bits.addition_average_mode = (ADC_B_ADD_AVERAGE_OFF),
                                   .channel_control_b_bits.addition_average_count = ADC_B_ADD_AVERAGE_1,
                                   .channel_control_b_bits.compare_match_enable = false,

                                   .channel_control_c_bits.limiter_clip_table_id = ADC_B_LIMIT_CLIP_TABLE_SELECTION_NONE,
                                   .channel_control_c_bits.channel_data_format = (ADC_B_DATA_FORMAT_12_BIT),
                                   .channel_control_c_bits.data_is_unsigned = true,
                               };
                               #endif

#if RA_NOT_DEFINED // Define Virtual Channel 33 if it's assigned to a scan group.
                               const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_33_cfg =
                               {
                                   .channel_id             = ADC_B_VIRTUAL_CHANNEL_33,

                                   .channel_cfg_bits.group           = RA_NOT_DEFINED,
                                   .channel_cfg_bits.channel         = RA_NOT_DEFINED,
                                   .channel_cfg_bits.differential    = 0,
                                   .channel_cfg_bits.sample_table_id = RA_NOT_DEFINED,

                                   .channel_control_a_bits.digital_filter_id = RA_NOT_DEFINED,
                                   .channel_control_a_bits.offset_table_id = RA_NOT_DEFINED,
                                   .channel_control_a_bits.gain_table_id = RA_NOT_DEFINED,

                                   .channel_control_b_bits.addition_average_mode = RA_NOT_DEFINED,
                                   .channel_control_b_bits.addition_average_count = RA_NOT_DEFINED,
                                   .channel_control_b_bits.compare_match_enable = false,

                                   .channel_control_c_bits.limiter_clip_table_id = RA_NOT_DEFINED,
                                   .channel_control_c_bits.channel_data_format = RA_NOT_DEFINED,
                                   .channel_control_c_bits.data_is_unsigned = false,
                               };
                               #endif

#if RA_NOT_DEFINED // Define Virtual Channel 34 if it's assigned to a scan group.
                               const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_34_cfg =
                               {
                                   .channel_id             = ADC_B_VIRTUAL_CHANNEL_34,

                                   .channel_cfg_bits.group           = RA_NOT_DEFINED,
                                   .channel_cfg_bits.channel         = RA_NOT_DEFINED,
                                   .channel_cfg_bits.differential    = 0,
                                   .channel_cfg_bits.sample_table_id = RA_NOT_DEFINED,

                                   .channel_control_a_bits.digital_filter_id = RA_NOT_DEFINED,
                                   .channel_control_a_bits.offset_table_id = RA_NOT_DEFINED,
                                   .channel_control_a_bits.gain_table_id = RA_NOT_DEFINED,

                                   .channel_control_b_bits.addition_average_mode = RA_NOT_DEFINED,
                                   .channel_control_b_bits.addition_average_count = RA_NOT_DEFINED,
                                   .channel_control_b_bits.compare_match_enable = false,

                                   .channel_control_c_bits.limiter_clip_table_id = RA_NOT_DEFINED,
                                   .channel_control_c_bits.channel_data_format = RA_NOT_DEFINED,
                                   .channel_control_c_bits.data_is_unsigned = false,
                               };
                               #endif

#if RA_NOT_DEFINED // Define Virtual Channel 35 if it's assigned to a scan group.
                               const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_35_cfg =
                               {
                                   .channel_id             = ADC_B_VIRTUAL_CHANNEL_35,

                                   .channel_cfg_bits.group           = RA_NOT_DEFINED,
                                   .channel_cfg_bits.channel         = RA_NOT_DEFINED,
                                   .channel_cfg_bits.differential    = 0,
                                   .channel_cfg_bits.sample_table_id = RA_NOT_DEFINED,

                                   .channel_control_a_bits.digital_filter_id = RA_NOT_DEFINED,
                                   .channel_control_a_bits.offset_table_id = RA_NOT_DEFINED,
                                   .channel_control_a_bits.gain_table_id = RA_NOT_DEFINED,

                                   .channel_control_b_bits.addition_average_mode = RA_NOT_DEFINED,
                                   .channel_control_b_bits.addition_average_count = RA_NOT_DEFINED,
                                   .channel_control_b_bits.compare_match_enable = false,

                                   .channel_control_c_bits.limiter_clip_table_id = RA_NOT_DEFINED,
                                   .channel_control_c_bits.channel_data_format = RA_NOT_DEFINED,
                                   .channel_control_c_bits.data_is_unsigned = false,
                               };
                               #endif

#if RA_NOT_DEFINED // Define Virtual Channel 36 if it's assigned to a scan group.
                               const adc_b_virtual_channel_cfg_t g_adc0_virtual_channel_36_cfg =
                               {
                                   .channel_id             = ADC_B_VIRTUAL_CHANNEL_36,

                                   .channel_cfg_bits.group           = RA_NOT_DEFINED,
                                   .channel_cfg_bits.channel         = RA_NOT_DEFINED,
                                   .channel_cfg_bits.differential    = 0,
                                   .channel_cfg_bits.sample_table_id = RA_NOT_DEFINED,

                                   .channel_control_a_bits.digital_filter_id = RA_NOT_DEFINED,
                                   .channel_control_a_bits.offset_table_id = RA_NOT_DEFINED,
                                   .channel_control_a_bits.gain_table_id = RA_NOT_DEFINED,

                                   .channel_control_b_bits.addition_average_mode = RA_NOT_DEFINED,
                                   .channel_control_b_bits.addition_average_count = RA_NOT_DEFINED,
                                   .channel_control_b_bits.compare_match_enable = false,

                                   .channel_control_c_bits.limiter_clip_table_id = RA_NOT_DEFINED,
                                   .channel_control_c_bits.channel_data_format = RA_NOT_DEFINED,
                                   .channel_control_c_bits.data_is_unsigned = false,
                               };
                               #endif
#if (((1) == 1)||((1) == 1)||((1) == 1)||((1) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||(RA_NOT_DEFINED == 1)||(RA_NOT_DEFINED == 1)||(RA_NOT_DEFINED == 1)||(RA_NOT_DEFINED == 1))
const adc_b_virtual_channel_cfg_t *const g_adc0_group_0_virtual_channels[] =
{
#if ((1) == 1)
  &g_adc0_virtual_channel_0_cfg,
#endif

#if ((1) == 1)
  &g_adc0_virtual_channel_1_cfg,
#endif

#if ((1) == 1)
  &g_adc0_virtual_channel_2_cfg,
#endif

#if ((1) == 1)
  &g_adc0_virtual_channel_3_cfg,
#endif

#if ((0) == 1)
                                      &g_adc0_virtual_channel_4_cfg,
                                  #endif

#if ((0) == 1)
                                      &g_adc0_virtual_channel_5_cfg,
                                  #endif

#if ((0) == 1)
                                      &g_adc0_virtual_channel_6_cfg,
                                  #endif

#if ((0) == 1)
                                      &g_adc0_virtual_channel_7_cfg,
                                  #endif

#if ((0) == 1)
                                      &g_adc0_virtual_channel_8_cfg,
                                  #endif

#if ((0) == 1)
                                      &g_adc0_virtual_channel_9_cfg,
                                  #endif

#if ((0) == 1)
                                      &g_adc0_virtual_channel_10_cfg,
                                  #endif

#if ((0) == 1)
                                      &g_adc0_virtual_channel_11_cfg,
                                  #endif

#if ((0) == 1)
                                      &g_adc0_virtual_channel_12_cfg,
                                  #endif

#if ((0) == 1)
                                      &g_adc0_virtual_channel_13_cfg,
                                  #endif

#if ((0) == 1)
                                      &g_adc0_virtual_channel_14_cfg,
                                  #endif

#if ((0) == 1)
                                      &g_adc0_virtual_channel_15_cfg,
                                  #endif

#if ((0) == 1)
                                      &g_adc0_virtual_channel_16_cfg,
                                  #endif

#if ((0) == 1)
                                      &g_adc0_virtual_channel_17_cfg,
                                  #endif

#if ((0) == 1)
                                      &g_adc0_virtual_channel_18_cfg,
                                  #endif

#if ((0) == 1)
                                      &g_adc0_virtual_channel_19_cfg,
                                  #endif

#if ((0) == 1)
                                      &g_adc0_virtual_channel_20_cfg,
                                  #endif

#if ((0) == 1)
                                      &g_adc0_virtual_channel_21_cfg,
                                  #endif

#if ((0) == 1)
                                      &g_adc0_virtual_channel_22_cfg,
                                  #endif

#if ((0) == 1)
                                      &g_adc0_virtual_channel_23_cfg,
                                  #endif

#if ((0) == 1)
                                      &g_adc0_virtual_channel_24_cfg,
                                  #endif

#if ((0) == 1)
                                      &g_adc0_virtual_channel_25_cfg,
                                  #endif

#if ((0) == 1)
                                      &g_adc0_virtual_channel_26_cfg,
                                  #endif

#if ((0) == 1)
                                      &g_adc0_virtual_channel_27_cfg,
                                  #endif

#if ((0) == 1)
                                      &g_adc0_virtual_channel_28_cfg,
                                  #endif

#if ((0) == 1)
                                      &g_adc0_virtual_channel_29_cfg,
                                  #endif

#if ((0) == 1)
                                      &g_adc0_virtual_channel_30_cfg,
                                  #endif

#if ((0) == 1)
                                      &g_adc0_virtual_channel_31_cfg,
                                  #endif

#if ((0) == 1)
                                      &g_adc0_virtual_channel_32_cfg,
                                  #endif

#if (RA_NOT_DEFINED == 1)
                                      &g_adc0_virtual_channel_33_cfg,
                                  #endif

#if (RA_NOT_DEFINED == 1)
                                      &g_adc0_virtual_channel_34_cfg,
                                  #endif

#if (RA_NOT_DEFINED == 1)
                                      &g_adc0_virtual_channel_35_cfg,
                                  #endif

#if (RA_NOT_DEFINED == 1)
                                      &g_adc0_virtual_channel_36_cfg,
                                  #endif
        };
#endif
#if (((1) == 2)||((1) == 2)||((1) == 2)||((1) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||(RA_NOT_DEFINED == 2)||(RA_NOT_DEFINED == 2)||(RA_NOT_DEFINED == 2)||(RA_NOT_DEFINED == 2))
const adc_b_virtual_channel_cfg_t *const g_adc0_group_1_virtual_channels[] = {
                                  #if ((1) == 2)
                                      &g_adc0_virtual_channel_0_cfg,
                                  #endif

                                  #if ((1) == 2)
                                      &g_adc0_virtual_channel_1_cfg,
                                  #endif

                                  #if ((1) == 2)
                                      &g_adc0_virtual_channel_2_cfg,
                                  #endif

                                  #if ((1) == 2)
                                      &g_adc0_virtual_channel_3_cfg,
                                  #endif

                                  #if ((0) == 2)
                                      &g_adc0_virtual_channel_4_cfg,
                                  #endif

                                  #if ((0) == 2)
                                      &g_adc0_virtual_channel_5_cfg,
                                  #endif

                                  #if ((0) == 2)
                                      &g_adc0_virtual_channel_6_cfg,
                                  #endif

                                  #if ((0) == 2)
                                      &g_adc0_virtual_channel_7_cfg,
                                  #endif

                                  #if ((0) == 2)
                                      &g_adc0_virtual_channel_8_cfg,
                                  #endif

                                  #if ((0) == 2)
                                      &g_adc0_virtual_channel_9_cfg,
                                  #endif

                                  #if ((0) == 2)
                                      &g_adc0_virtual_channel_10_cfg,
                                  #endif

                                  #if ((0) == 2)
                                      &g_adc0_virtual_channel_11_cfg,
                                  #endif

                                  #if ((0) == 2)
                                      &g_adc0_virtual_channel_12_cfg,
                                  #endif

                                  #if ((0) == 2)
                                      &g_adc0_virtual_channel_13_cfg,
                                  #endif

                                  #if ((0) == 2)
                                      &g_adc0_virtual_channel_14_cfg,
                                  #endif

                                  #if ((0) == 2)
                                      &g_adc0_virtual_channel_15_cfg,
                                  #endif

                                  #if ((0) == 2)
                                      &g_adc0_virtual_channel_16_cfg,
                                  #endif

                                  #if ((0) == 2)
                                      &g_adc0_virtual_channel_17_cfg,
                                  #endif

                                  #if ((0) == 2)
                                      &g_adc0_virtual_channel_18_cfg,
                                  #endif

                                  #if ((0) == 2)
                                      &g_adc0_virtual_channel_19_cfg,
                                  #endif

                                  #if ((0) == 2)
                                      &g_adc0_virtual_channel_20_cfg,
                                  #endif

                                  #if ((0) == 2)
                                      &g_adc0_virtual_channel_21_cfg,
                                  #endif

                                  #if ((0) == 2)
                                      &g_adc0_virtual_channel_22_cfg,
                                  #endif

                                  #if ((0) == 2)
                                      &g_adc0_virtual_channel_23_cfg,
                                  #endif

                                  #if ((0) == 2)
                                      &g_adc0_virtual_channel_24_cfg,
                                  #endif

                                  #if ((0) == 2)
                                      &g_adc0_virtual_channel_25_cfg,
                                  #endif

                                  #if ((0) == 2)
                                      &g_adc0_virtual_channel_26_cfg,
                                  #endif

                                  #if ((0) == 2)
                                      &g_adc0_virtual_channel_27_cfg,
                                  #endif

                                  #if ((0) == 2)
                                      &g_adc0_virtual_channel_28_cfg,
                                  #endif

                                  #if ((0) == 2)
                                      &g_adc0_virtual_channel_29_cfg,
                                  #endif

                                  #if ((0) == 2)
                                      &g_adc0_virtual_channel_30_cfg,
                                  #endif

                                  #if ((0) == 2)
                                      &g_adc0_virtual_channel_31_cfg,
                                  #endif

                                  #if ((0) == 2)
                                      &g_adc0_virtual_channel_32_cfg,
                                  #endif

                                  #if (RA_NOT_DEFINED == 2)
                                      &g_adc0_virtual_channel_33_cfg,
                                  #endif

                                  #if (RA_NOT_DEFINED == 2)
                                      &g_adc0_virtual_channel_34_cfg,
                                  #endif

                                  #if (RA_NOT_DEFINED == 2)
                                      &g_adc0_virtual_channel_35_cfg,
                                  #endif

                                  #if (RA_NOT_DEFINED == 2)
                                      &g_adc0_virtual_channel_36_cfg,
                                  #endif
                                  };
                                  #endif
#if (((1) == 3)||((1) == 3)||((1) == 3)||((1) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||(RA_NOT_DEFINED == 3)||(RA_NOT_DEFINED == 3)||(RA_NOT_DEFINED == 3)||(RA_NOT_DEFINED == 3))
const adc_b_virtual_channel_cfg_t *const g_adc0_group_2_virtual_channels[] = {
                                  #if ((1) == 3)
                                      &g_adc0_virtual_channel_0_cfg,
                                  #endif

                                  #if ((1) == 3)
                                      &g_adc0_virtual_channel_1_cfg,
                                  #endif

                                  #if ((1) == 3)
                                      &g_adc0_virtual_channel_2_cfg,
                                  #endif

                                  #if ((1) == 3)
                                      &g_adc0_virtual_channel_3_cfg,
                                  #endif

                                  #if ((0) == 3)
                                      &g_adc0_virtual_channel_4_cfg,
                                  #endif

                                  #if ((0) == 3)
                                      &g_adc0_virtual_channel_5_cfg,
                                  #endif

                                  #if ((0) == 3)
                                      &g_adc0_virtual_channel_6_cfg,
                                  #endif

                                  #if ((0) == 3)
                                      &g_adc0_virtual_channel_7_cfg,
                                  #endif

                                  #if ((0) == 3)
                                      &g_adc0_virtual_channel_8_cfg,
                                  #endif

                                  #if ((0) == 3)
                                      &g_adc0_virtual_channel_9_cfg,
                                  #endif

                                  #if ((0) == 3)
                                      &g_adc0_virtual_channel_10_cfg,
                                  #endif

                                  #if ((0) == 3)
                                      &g_adc0_virtual_channel_11_cfg,
                                  #endif

                                  #if ((0) == 3)
                                      &g_adc0_virtual_channel_12_cfg,
                                  #endif

                                  #if ((0) == 3)
                                      &g_adc0_virtual_channel_13_cfg,
                                  #endif

                                  #if ((0) == 3)
                                      &g_adc0_virtual_channel_14_cfg,
                                  #endif

                                  #if ((0) == 3)
                                      &g_adc0_virtual_channel_15_cfg,
                                  #endif

                                  #if ((0) == 3)
                                      &g_adc0_virtual_channel_16_cfg,
                                  #endif

                                  #if ((0) == 3)
                                      &g_adc0_virtual_channel_17_cfg,
                                  #endif

                                  #if ((0) == 3)
                                      &g_adc0_virtual_channel_18_cfg,
                                  #endif

                                  #if ((0) == 3)
                                      &g_adc0_virtual_channel_19_cfg,
                                  #endif

                                  #if ((0) == 3)
                                      &g_adc0_virtual_channel_20_cfg,
                                  #endif

                                  #if ((0) == 3)
                                      &g_adc0_virtual_channel_21_cfg,
                                  #endif

                                  #if ((0) == 3)
                                      &g_adc0_virtual_channel_22_cfg,
                                  #endif

                                  #if ((0) == 3)
                                      &g_adc0_virtual_channel_23_cfg,
                                  #endif

                                  #if ((0) == 3)
                                      &g_adc0_virtual_channel_24_cfg,
                                  #endif

                                  #if ((0) == 3)
                                      &g_adc0_virtual_channel_25_cfg,
                                  #endif

                                  #if ((0) == 3)
                                      &g_adc0_virtual_channel_26_cfg,
                                  #endif

                                  #if ((0) == 3)
                                      &g_adc0_virtual_channel_27_cfg,
                                  #endif

                                  #if ((0) == 3)
                                      &g_adc0_virtual_channel_28_cfg,
                                  #endif

                                  #if ((0) == 3)
                                      &g_adc0_virtual_channel_29_cfg,
                                  #endif

                                  #if ((0) == 3)
                                      &g_adc0_virtual_channel_30_cfg,
                                  #endif

                                  #if ((0) == 3)
                                      &g_adc0_virtual_channel_31_cfg,
                                  #endif

                                  #if ((0) == 3)
                                      &g_adc0_virtual_channel_32_cfg,
                                  #endif

                                  #if (RA_NOT_DEFINED == 3)
                                      &g_adc0_virtual_channel_33_cfg,
                                  #endif

                                  #if (RA_NOT_DEFINED == 3)
                                      &g_adc0_virtual_channel_34_cfg,
                                  #endif

                                  #if (RA_NOT_DEFINED == 3)
                                      &g_adc0_virtual_channel_35_cfg,
                                  #endif

                                  #if (RA_NOT_DEFINED == 3)
                                      &g_adc0_virtual_channel_36_cfg,
                                  #endif
                                  };
                                  #endif
#if (((1) == 4)||((1) == 4)||((1) == 4)||((1) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||(RA_NOT_DEFINED == 4)||(RA_NOT_DEFINED == 4)||(RA_NOT_DEFINED == 4)||(RA_NOT_DEFINED == 4))
const adc_b_virtual_channel_cfg_t *const g_adc0_group_3_virtual_channels[] = {
                                  #if ((1) == 4)
                                      &g_adc0_virtual_channel_0_cfg,
                                  #endif

                                  #if ((1) == 4)
                                      &g_adc0_virtual_channel_1_cfg,
                                  #endif

                                  #if ((1) == 4)
                                      &g_adc0_virtual_channel_2_cfg,
                                  #endif

                                  #if ((1) == 4)
                                      &g_adc0_virtual_channel_3_cfg,
                                  #endif

                                  #if ((0) == 4)
                                      &g_adc0_virtual_channel_4_cfg,
                                  #endif

                                  #if ((0) == 4)
                                      &g_adc0_virtual_channel_5_cfg,
                                  #endif

                                  #if ((0) == 4)
                                      &g_adc0_virtual_channel_6_cfg,
                                  #endif

                                  #if ((0) == 4)
                                      &g_adc0_virtual_channel_7_cfg,
                                  #endif

                                  #if ((0) == 4)
                                      &g_adc0_virtual_channel_8_cfg,
                                  #endif

                                  #if ((0) == 4)
                                      &g_adc0_virtual_channel_9_cfg,
                                  #endif

                                  #if ((0) == 4)
                                      &g_adc0_virtual_channel_10_cfg,
                                  #endif

                                  #if ((0) == 4)
                                      &g_adc0_virtual_channel_11_cfg,
                                  #endif

                                  #if ((0) == 4)
                                      &g_adc0_virtual_channel_12_cfg,
                                  #endif

                                  #if ((0) == 4)
                                      &g_adc0_virtual_channel_13_cfg,
                                  #endif

                                  #if ((0) == 4)
                                      &g_adc0_virtual_channel_14_cfg,
                                  #endif

                                  #if ((0) == 4)
                                      &g_adc0_virtual_channel_15_cfg,
                                  #endif

                                  #if ((0) == 4)
                                      &g_adc0_virtual_channel_16_cfg,
                                  #endif

                                  #if ((0) == 4)
                                      &g_adc0_virtual_channel_17_cfg,
                                  #endif

                                  #if ((0) == 4)
                                      &g_adc0_virtual_channel_18_cfg,
                                  #endif

                                  #if ((0) == 4)
                                      &g_adc0_virtual_channel_19_cfg,
                                  #endif

                                  #if ((0) == 4)
                                      &g_adc0_virtual_channel_20_cfg,
                                  #endif

                                  #if ((0) == 4)
                                      &g_adc0_virtual_channel_21_cfg,
                                  #endif

                                  #if ((0) == 4)
                                      &g_adc0_virtual_channel_22_cfg,
                                  #endif

                                  #if ((0) == 4)
                                      &g_adc0_virtual_channel_23_cfg,
                                  #endif

                                  #if ((0) == 4)
                                      &g_adc0_virtual_channel_24_cfg,
                                  #endif

                                  #if ((0) == 4)
                                      &g_adc0_virtual_channel_25_cfg,
                                  #endif

                                  #if ((0) == 4)
                                      &g_adc0_virtual_channel_26_cfg,
                                  #endif

                                  #if ((0) == 4)
                                      &g_adc0_virtual_channel_27_cfg,
                                  #endif

                                  #if ((0) == 4)
                                      &g_adc0_virtual_channel_28_cfg,
                                  #endif

                                  #if ((0) == 4)
                                      &g_adc0_virtual_channel_29_cfg,
                                  #endif

                                  #if ((0) == 4)
                                      &g_adc0_virtual_channel_30_cfg,
                                  #endif

                                  #if ((0) == 4)
                                      &g_adc0_virtual_channel_31_cfg,
                                  #endif

                                  #if ((0) == 4)
                                      &g_adc0_virtual_channel_32_cfg,
                                  #endif

                                  #if (RA_NOT_DEFINED == 4)
                                      &g_adc0_virtual_channel_33_cfg,
                                  #endif

                                  #if (RA_NOT_DEFINED == 4)
                                      &g_adc0_virtual_channel_34_cfg,
                                  #endif

                                  #if (RA_NOT_DEFINED == 4)
                                      &g_adc0_virtual_channel_35_cfg,
                                  #endif

                                  #if (RA_NOT_DEFINED == 4)
                                      &g_adc0_virtual_channel_36_cfg,
                                  #endif
                                  };
                                  #endif
#if (((1) == 5)||((1) == 5)||((1) == 5)||((1) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||(RA_NOT_DEFINED == 5)||(RA_NOT_DEFINED == 5)||(RA_NOT_DEFINED == 5)||(RA_NOT_DEFINED == 5))
const adc_b_virtual_channel_cfg_t *const g_adc0_group_4_virtual_channels[] = {
                                  #if ((1) == 5)
                                      &g_adc0_virtual_channel_0_cfg,
                                  #endif

                                  #if ((1) == 5)
                                      &g_adc0_virtual_channel_1_cfg,
                                  #endif

                                  #if ((1) == 5)
                                      &g_adc0_virtual_channel_2_cfg,
                                  #endif

                                  #if ((1) == 5)
                                      &g_adc0_virtual_channel_3_cfg,
                                  #endif

                                  #if ((0) == 5)
                                      &g_adc0_virtual_channel_4_cfg,
                                  #endif

                                  #if ((0) == 5)
                                      &g_adc0_virtual_channel_5_cfg,
                                  #endif

                                  #if ((0) == 5)
                                      &g_adc0_virtual_channel_6_cfg,
                                  #endif

                                  #if ((0) == 5)
                                      &g_adc0_virtual_channel_7_cfg,
                                  #endif

                                  #if ((0) == 5)
                                      &g_adc0_virtual_channel_8_cfg,
                                  #endif

                                  #if ((0) == 5)
                                      &g_adc0_virtual_channel_9_cfg,
                                  #endif

                                  #if ((0) == 5)
                                      &g_adc0_virtual_channel_10_cfg,
                                  #endif

                                  #if ((0) == 5)
                                      &g_adc0_virtual_channel_11_cfg,
                                  #endif

                                  #if ((0) == 5)
                                      &g_adc0_virtual_channel_12_cfg,
                                  #endif

                                  #if ((0) == 5)
                                      &g_adc0_virtual_channel_13_cfg,
                                  #endif

                                  #if ((0) == 5)
                                      &g_adc0_virtual_channel_14_cfg,
                                  #endif

                                  #if ((0) == 5)
                                      &g_adc0_virtual_channel_15_cfg,
                                  #endif

                                  #if ((0) == 5)
                                      &g_adc0_virtual_channel_16_cfg,
                                  #endif

                                  #if ((0) == 5)
                                      &g_adc0_virtual_channel_17_cfg,
                                  #endif

                                  #if ((0) == 5)
                                      &g_adc0_virtual_channel_18_cfg,
                                  #endif

                                  #if ((0) == 5)
                                      &g_adc0_virtual_channel_19_cfg,
                                  #endif

                                  #if ((0) == 5)
                                      &g_adc0_virtual_channel_20_cfg,
                                  #endif

                                  #if ((0) == 5)
                                      &g_adc0_virtual_channel_21_cfg,
                                  #endif

                                  #if ((0) == 5)
                                      &g_adc0_virtual_channel_22_cfg,
                                  #endif

                                  #if ((0) == 5)
                                      &g_adc0_virtual_channel_23_cfg,
                                  #endif

                                  #if ((0) == 5)
                                      &g_adc0_virtual_channel_24_cfg,
                                  #endif

                                  #if ((0) == 5)
                                      &g_adc0_virtual_channel_25_cfg,
                                  #endif

                                  #if ((0) == 5)
                                      &g_adc0_virtual_channel_26_cfg,
                                  #endif

                                  #if ((0) == 5)
                                      &g_adc0_virtual_channel_27_cfg,
                                  #endif

                                  #if ((0) == 5)
                                      &g_adc0_virtual_channel_28_cfg,
                                  #endif

                                  #if ((0) == 5)
                                      &g_adc0_virtual_channel_29_cfg,
                                  #endif

                                  #if ((0) == 5)
                                      &g_adc0_virtual_channel_30_cfg,
                                  #endif

                                  #if ((0) == 5)
                                      &g_adc0_virtual_channel_31_cfg,
                                  #endif

                                  #if ((0) == 5)
                                      &g_adc0_virtual_channel_32_cfg,
                                  #endif

                                  #if (RA_NOT_DEFINED == 5)
                                      &g_adc0_virtual_channel_33_cfg,
                                  #endif

                                  #if (RA_NOT_DEFINED == 5)
                                      &g_adc0_virtual_channel_34_cfg,
                                  #endif

                                  #if (RA_NOT_DEFINED == 5)
                                      &g_adc0_virtual_channel_35_cfg,
                                  #endif

                                  #if (RA_NOT_DEFINED == 5)
                                      &g_adc0_virtual_channel_36_cfg,
                                  #endif
                                  };
                                  #endif
#if (((1) == 6)||((1) == 6)||((1) == 6)||((1) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||(RA_NOT_DEFINED == 6)||(RA_NOT_DEFINED == 6)||(RA_NOT_DEFINED == 6)||(RA_NOT_DEFINED == 6))
const adc_b_virtual_channel_cfg_t *const g_adc0_group_5_virtual_channels[] = {
                                  #if ((1) == 6)
                                      &g_adc0_virtual_channel_0_cfg,
                                  #endif

                                  #if ((1) == 6)
                                      &g_adc0_virtual_channel_1_cfg,
                                  #endif

                                  #if ((1) == 6)
                                      &g_adc0_virtual_channel_2_cfg,
                                  #endif

                                  #if ((1) == 6)
                                      &g_adc0_virtual_channel_3_cfg,
                                  #endif

                                  #if ((0) == 6)
                                      &g_adc0_virtual_channel_4_cfg,
                                  #endif

                                  #if ((0) == 6)
                                      &g_adc0_virtual_channel_5_cfg,
                                  #endif

                                  #if ((0) == 6)
                                      &g_adc0_virtual_channel_6_cfg,
                                  #endif

                                  #if ((0) == 6)
                                      &g_adc0_virtual_channel_7_cfg,
                                  #endif

                                  #if ((0) == 6)
                                      &g_adc0_virtual_channel_8_cfg,
                                  #endif

                                  #if ((0) == 6)
                                      &g_adc0_virtual_channel_9_cfg,
                                  #endif

                                  #if ((0) == 6)
                                      &g_adc0_virtual_channel_10_cfg,
                                  #endif

                                  #if ((0) == 6)
                                      &g_adc0_virtual_channel_11_cfg,
                                  #endif

                                  #if ((0) == 6)
                                      &g_adc0_virtual_channel_12_cfg,
                                  #endif

                                  #if ((0) == 6)
                                      &g_adc0_virtual_channel_13_cfg,
                                  #endif

                                  #if ((0) == 6)
                                      &g_adc0_virtual_channel_14_cfg,
                                  #endif

                                  #if ((0) == 6)
                                      &g_adc0_virtual_channel_15_cfg,
                                  #endif

                                  #if ((0) == 6)
                                      &g_adc0_virtual_channel_16_cfg,
                                  #endif

                                  #if ((0) == 6)
                                      &g_adc0_virtual_channel_17_cfg,
                                  #endif

                                  #if ((0) == 6)
                                      &g_adc0_virtual_channel_18_cfg,
                                  #endif

                                  #if ((0) == 6)
                                      &g_adc0_virtual_channel_19_cfg,
                                  #endif

                                  #if ((0) == 6)
                                      &g_adc0_virtual_channel_20_cfg,
                                  #endif

                                  #if ((0) == 6)
                                      &g_adc0_virtual_channel_21_cfg,
                                  #endif

                                  #if ((0) == 6)
                                      &g_adc0_virtual_channel_22_cfg,
                                  #endif

                                  #if ((0) == 6)
                                      &g_adc0_virtual_channel_23_cfg,
                                  #endif

                                  #if ((0) == 6)
                                      &g_adc0_virtual_channel_24_cfg,
                                  #endif

                                  #if ((0) == 6)
                                      &g_adc0_virtual_channel_25_cfg,
                                  #endif

                                  #if ((0) == 6)
                                      &g_adc0_virtual_channel_26_cfg,
                                  #endif

                                  #if ((0) == 6)
                                      &g_adc0_virtual_channel_27_cfg,
                                  #endif

                                  #if ((0) == 6)
                                      &g_adc0_virtual_channel_28_cfg,
                                  #endif

                                  #if ((0) == 6)
                                      &g_adc0_virtual_channel_29_cfg,
                                  #endif

                                  #if ((0) == 6)
                                      &g_adc0_virtual_channel_30_cfg,
                                  #endif

                                  #if ((0) == 6)
                                      &g_adc0_virtual_channel_31_cfg,
                                  #endif

                                  #if ((0) == 6)
                                      &g_adc0_virtual_channel_32_cfg,
                                  #endif

                                  #if (RA_NOT_DEFINED == 6)
                                      &g_adc0_virtual_channel_33_cfg,
                                  #endif

                                  #if (RA_NOT_DEFINED == 6)
                                      &g_adc0_virtual_channel_34_cfg,
                                  #endif

                                  #if (RA_NOT_DEFINED == 6)
                                      &g_adc0_virtual_channel_35_cfg,
                                  #endif

                                  #if (RA_NOT_DEFINED == 6)
                                      &g_adc0_virtual_channel_36_cfg,
                                  #endif
                                  };
                                  #endif
#if (((1) == 7)||((1) == 7)||((1) == 7)||((1) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||(RA_NOT_DEFINED == 7)||(RA_NOT_DEFINED == 7)||(RA_NOT_DEFINED == 7)||(RA_NOT_DEFINED == 7))
const adc_b_virtual_channel_cfg_t *const g_adc0_group_6_virtual_channels[] = {
                                  #if ((1) == 7)
                                      &g_adc0_virtual_channel_0_cfg,
                                  #endif

                                  #if ((1) == 7)
                                      &g_adc0_virtual_channel_1_cfg,
                                  #endif

                                  #if ((1) == 7)
                                      &g_adc0_virtual_channel_2_cfg,
                                  #endif

                                  #if ((1) == 7)
                                      &g_adc0_virtual_channel_3_cfg,
                                  #endif

                                  #if ((0) == 7)
                                      &g_adc0_virtual_channel_4_cfg,
                                  #endif

                                  #if ((0) == 7)
                                      &g_adc0_virtual_channel_5_cfg,
                                  #endif

                                  #if ((0) == 7)
                                      &g_adc0_virtual_channel_6_cfg,
                                  #endif

                                  #if ((0) == 7)
                                      &g_adc0_virtual_channel_7_cfg,
                                  #endif

                                  #if ((0) == 7)
                                      &g_adc0_virtual_channel_8_cfg,
                                  #endif

                                  #if ((0) == 7)
                                      &g_adc0_virtual_channel_9_cfg,
                                  #endif

                                  #if ((0) == 7)
                                      &g_adc0_virtual_channel_10_cfg,
                                  #endif

                                  #if ((0) == 7)
                                      &g_adc0_virtual_channel_11_cfg,
                                  #endif

                                  #if ((0) == 7)
                                      &g_adc0_virtual_channel_12_cfg,
                                  #endif

                                  #if ((0) == 7)
                                      &g_adc0_virtual_channel_13_cfg,
                                  #endif

                                  #if ((0) == 7)
                                      &g_adc0_virtual_channel_14_cfg,
                                  #endif

                                  #if ((0) == 7)
                                      &g_adc0_virtual_channel_15_cfg,
                                  #endif

                                  #if ((0) == 7)
                                      &g_adc0_virtual_channel_16_cfg,
                                  #endif

                                  #if ((0) == 7)
                                      &g_adc0_virtual_channel_17_cfg,
                                  #endif

                                  #if ((0) == 7)
                                      &g_adc0_virtual_channel_18_cfg,
                                  #endif

                                  #if ((0) == 7)
                                      &g_adc0_virtual_channel_19_cfg,
                                  #endif

                                  #if ((0) == 7)
                                      &g_adc0_virtual_channel_20_cfg,
                                  #endif

                                  #if ((0) == 7)
                                      &g_adc0_virtual_channel_21_cfg,
                                  #endif

                                  #if ((0) == 7)
                                      &g_adc0_virtual_channel_22_cfg,
                                  #endif

                                  #if ((0) == 7)
                                      &g_adc0_virtual_channel_23_cfg,
                                  #endif

                                  #if ((0) == 7)
                                      &g_adc0_virtual_channel_24_cfg,
                                  #endif

                                  #if ((0) == 7)
                                      &g_adc0_virtual_channel_25_cfg,
                                  #endif

                                  #if ((0) == 7)
                                      &g_adc0_virtual_channel_26_cfg,
                                  #endif

                                  #if ((0) == 7)
                                      &g_adc0_virtual_channel_27_cfg,
                                  #endif

                                  #if ((0) == 7)
                                      &g_adc0_virtual_channel_28_cfg,
                                  #endif

                                  #if ((0) == 7)
                                      &g_adc0_virtual_channel_29_cfg,
                                  #endif

                                  #if ((0) == 7)
                                      &g_adc0_virtual_channel_30_cfg,
                                  #endif

                                  #if ((0) == 7)
                                      &g_adc0_virtual_channel_31_cfg,
                                  #endif

                                  #if ((0) == 7)
                                      &g_adc0_virtual_channel_32_cfg,
                                  #endif

                                  #if (RA_NOT_DEFINED == 7)
                                      &g_adc0_virtual_channel_33_cfg,
                                  #endif

                                  #if (RA_NOT_DEFINED == 7)
                                      &g_adc0_virtual_channel_34_cfg,
                                  #endif

                                  #if (RA_NOT_DEFINED == 7)
                                      &g_adc0_virtual_channel_35_cfg,
                                  #endif

                                  #if (RA_NOT_DEFINED == 7)
                                      &g_adc0_virtual_channel_36_cfg,
                                  #endif
                                  };
                                  #endif
#if (((1) == 8)||((1) == 8)||((1) == 8)||((1) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||(RA_NOT_DEFINED == 8)||(RA_NOT_DEFINED == 8)||(RA_NOT_DEFINED == 8)||(RA_NOT_DEFINED == 8))
const adc_b_virtual_channel_cfg_t *const g_adc0_group_7_virtual_channels[] = {
                                  #if ((1) == 8)
                                      &g_adc0_virtual_channel_0_cfg,
                                  #endif

                                  #if ((1) == 8)
                                      &g_adc0_virtual_channel_1_cfg,
                                  #endif

                                  #if ((1) == 8)
                                      &g_adc0_virtual_channel_2_cfg,
                                  #endif

                                  #if ((1) == 8)
                                      &g_adc0_virtual_channel_3_cfg,
                                  #endif

                                  #if ((0) == 8)
                                      &g_adc0_virtual_channel_4_cfg,
                                  #endif

                                  #if ((0) == 8)
                                      &g_adc0_virtual_channel_5_cfg,
                                  #endif

                                  #if ((0) == 8)
                                      &g_adc0_virtual_channel_6_cfg,
                                  #endif

                                  #if ((0) == 8)
                                      &g_adc0_virtual_channel_7_cfg,
                                  #endif

                                  #if ((0) == 8)
                                      &g_adc0_virtual_channel_8_cfg,
                                  #endif

                                  #if ((0) == 8)
                                      &g_adc0_virtual_channel_9_cfg,
                                  #endif

                                  #if ((0) == 8)
                                      &g_adc0_virtual_channel_10_cfg,
                                  #endif

                                  #if ((0) == 8)
                                      &g_adc0_virtual_channel_11_cfg,
                                  #endif

                                  #if ((0) == 8)
                                      &g_adc0_virtual_channel_12_cfg,
                                  #endif

                                  #if ((0) == 8)
                                      &g_adc0_virtual_channel_13_cfg,
                                  #endif

                                  #if ((0) == 8)
                                      &g_adc0_virtual_channel_14_cfg,
                                  #endif

                                  #if ((0) == 8)
                                      &g_adc0_virtual_channel_15_cfg,
                                  #endif

                                  #if ((0) == 8)
                                      &g_adc0_virtual_channel_16_cfg,
                                  #endif

                                  #if ((0) == 8)
                                      &g_adc0_virtual_channel_17_cfg,
                                  #endif

                                  #if ((0) == 8)
                                      &g_adc0_virtual_channel_18_cfg,
                                  #endif

                                  #if ((0) == 8)
                                      &g_adc0_virtual_channel_19_cfg,
                                  #endif

                                  #if ((0) == 8)
                                      &g_adc0_virtual_channel_20_cfg,
                                  #endif

                                  #if ((0) == 8)
                                      &g_adc0_virtual_channel_21_cfg,
                                  #endif

                                  #if ((0) == 8)
                                      &g_adc0_virtual_channel_22_cfg,
                                  #endif

                                  #if ((0) == 8)
                                      &g_adc0_virtual_channel_23_cfg,
                                  #endif

                                  #if ((0) == 8)
                                      &g_adc0_virtual_channel_24_cfg,
                                  #endif

                                  #if ((0) == 8)
                                      &g_adc0_virtual_channel_25_cfg,
                                  #endif

                                  #if ((0) == 8)
                                      &g_adc0_virtual_channel_26_cfg,
                                  #endif

                                  #if ((0) == 8)
                                      &g_adc0_virtual_channel_27_cfg,
                                  #endif

                                  #if ((0) == 8)
                                      &g_adc0_virtual_channel_28_cfg,
                                  #endif

                                  #if ((0) == 8)
                                      &g_adc0_virtual_channel_29_cfg,
                                  #endif

                                  #if ((0) == 8)
                                      &g_adc0_virtual_channel_30_cfg,
                                  #endif

                                  #if ((0) == 8)
                                      &g_adc0_virtual_channel_31_cfg,
                                  #endif

                                  #if ((0) == 8)
                                      &g_adc0_virtual_channel_32_cfg,
                                  #endif

                                  #if (RA_NOT_DEFINED == 8)
                                      &g_adc0_virtual_channel_33_cfg,
                                  #endif

                                  #if (RA_NOT_DEFINED == 8)
                                      &g_adc0_virtual_channel_34_cfg,
                                  #endif

                                  #if (RA_NOT_DEFINED == 8)
                                      &g_adc0_virtual_channel_35_cfg,
                                  #endif

                                  #if (RA_NOT_DEFINED == 8)
                                      &g_adc0_virtual_channel_36_cfg,
                                  #endif
                                  };
                                  #endif
#if (((1) == 9)||((1) == 9)||((1) == 9)||((1) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||(RA_NOT_DEFINED == 9)||(RA_NOT_DEFINED == 9)||(RA_NOT_DEFINED == 9)||(RA_NOT_DEFINED == 9))
const adc_b_virtual_channel_cfg_t *const g_adc0_group_8_virtual_channels[] = {
                                  #if ((1) == 9)
                                      &g_adc0_virtual_channel_0_cfg,
                                  #endif

                                  #if ((1) == 9)
                                      &g_adc0_virtual_channel_1_cfg,
                                  #endif

                                  #if ((1) == 9)
                                      &g_adc0_virtual_channel_2_cfg,
                                  #endif

                                  #if ((1) == 9)
                                      &g_adc0_virtual_channel_3_cfg,
                                  #endif

                                  #if ((0) == 9)
                                      &g_adc0_virtual_channel_4_cfg,
                                  #endif

                                  #if ((0) == 9)
                                      &g_adc0_virtual_channel_5_cfg,
                                  #endif

                                  #if ((0) == 9)
                                      &g_adc0_virtual_channel_6_cfg,
                                  #endif

                                  #if ((0) == 9)
                                      &g_adc0_virtual_channel_7_cfg,
                                  #endif

                                  #if ((0) == 9)
                                      &g_adc0_virtual_channel_8_cfg,
                                  #endif

                                  #if ((0) == 9)
                                      &g_adc0_virtual_channel_9_cfg,
                                  #endif

                                  #if ((0) == 9)
                                      &g_adc0_virtual_channel_10_cfg,
                                  #endif

                                  #if ((0) == 9)
                                      &g_adc0_virtual_channel_11_cfg,
                                  #endif

                                  #if ((0) == 9)
                                      &g_adc0_virtual_channel_12_cfg,
                                  #endif

                                  #if ((0) == 9)
                                      &g_adc0_virtual_channel_13_cfg,
                                  #endif

                                  #if ((0) == 9)
                                      &g_adc0_virtual_channel_14_cfg,
                                  #endif

                                  #if ((0) == 9)
                                      &g_adc0_virtual_channel_15_cfg,
                                  #endif

                                  #if ((0) == 9)
                                      &g_adc0_virtual_channel_16_cfg,
                                  #endif

                                  #if ((0) == 9)
                                      &g_adc0_virtual_channel_17_cfg,
                                  #endif

                                  #if ((0) == 9)
                                      &g_adc0_virtual_channel_18_cfg,
                                  #endif

                                  #if ((0) == 9)
                                      &g_adc0_virtual_channel_19_cfg,
                                  #endif

                                  #if ((0) == 9)
                                      &g_adc0_virtual_channel_20_cfg,
                                  #endif

                                  #if ((0) == 9)
                                      &g_adc0_virtual_channel_21_cfg,
                                  #endif

                                  #if ((0) == 9)
                                      &g_adc0_virtual_channel_22_cfg,
                                  #endif

                                  #if ((0) == 9)
                                      &g_adc0_virtual_channel_23_cfg,
                                  #endif

                                  #if ((0) == 9)
                                      &g_adc0_virtual_channel_24_cfg,
                                  #endif

                                  #if ((0) == 9)
                                      &g_adc0_virtual_channel_25_cfg,
                                  #endif

                                  #if ((0) == 9)
                                      &g_adc0_virtual_channel_26_cfg,
                                  #endif

                                  #if ((0) == 9)
                                      &g_adc0_virtual_channel_27_cfg,
                                  #endif

                                  #if ((0) == 9)
                                      &g_adc0_virtual_channel_28_cfg,
                                  #endif

                                  #if ((0) == 9)
                                      &g_adc0_virtual_channel_29_cfg,
                                  #endif

                                  #if ((0) == 9)
                                      &g_adc0_virtual_channel_30_cfg,
                                  #endif

                                  #if ((0) == 9)
                                      &g_adc0_virtual_channel_31_cfg,
                                  #endif

                                  #if ((0) == 9)
                                      &g_adc0_virtual_channel_32_cfg,
                                  #endif

                                  #if (RA_NOT_DEFINED == 9)
                                      &g_adc0_virtual_channel_33_cfg,
                                  #endif

                                  #if (RA_NOT_DEFINED == 9)
                                      &g_adc0_virtual_channel_34_cfg,
                                  #endif

                                  #if (RA_NOT_DEFINED == 9)
                                      &g_adc0_virtual_channel_35_cfg,
                                  #endif

                                  #if (RA_NOT_DEFINED == 9)
                                      &g_adc0_virtual_channel_36_cfg,
                                  #endif
                                  };
                                  #endif

#if (1) // Define Scan Group 0 if it's enabled
const adc_b_group_cfg_t g_adc0_group_0_cfg =
{ .scan_group_id = ADC_GROUP_ID_0,
  .converter_selection = (adc_b_unit_id_t) (0),
  .scan_group_enable = (1),
  .scan_end_interrupt_enable = (1),
  .external_trigger_enable_mask = (ADC_B_EXTERNAL_TRIGGER_NONE),
  .elc_trigger_enable_mask = (elc_peripheral_t) (0x00),
  .gpt_trigger_enable_mask = (ADC_B_GPT_TRIGGER_NONE),

  .self_diagnosis_mask = (ADC_B_SELF_DIAGNOSIS_DISABLED << R_ADC_B0_ADSGDCR0_DIAGVAL_Pos),

  .limit_clip_interrupt_enable = (1),
  .virtual_channel_count = 0
          + (((1) == 1) + ((1) == 1) + ((1) == 1) + ((1) == 1) + ((0) == 1) + ((0) == 1) + ((0) == 1) + ((0) == 1)
                  + ((0) == 1) + ((0) == 1) + ((0) == 1) + ((0) == 1) + ((0) == 1) + ((0) == 1) + ((0) == 1)
                  + ((0) == 1) + ((0) == 1) + ((0) == 1) + ((0) == 1) + ((0) == 1) + ((0) == 1) + ((0) == 1)
                  + ((0) == 1) + ((0) == 1) + ((0) == 1) + ((0) == 1) + ((0) == 1) + ((0) == 1) + ((0) == 1)
                  + ((0) == 1) + ((0) == 1) + ((0) == 1) + ((0) == 1) + (RA_NOT_DEFINED == 1) + (RA_NOT_DEFINED == 1)
                  + (RA_NOT_DEFINED == 1) + (RA_NOT_DEFINED == 1)),
#if (((1) == 1)||((1) == 1)||((1) == 1)||((1) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||((0) == 1)||(RA_NOT_DEFINED == 1)||(RA_NOT_DEFINED == 1)||(RA_NOT_DEFINED == 1)||(RA_NOT_DEFINED == 1))

  .p_virtual_channels = (adc_b_virtual_channel_cfg_t**) g_adc0_group_0_virtual_channels,
#else
                         .p_virtual_channels = NULL,
                         #endif
        };
#endif

#if (0) // Define Scan Group 1 if it's enabled
                     const adc_b_group_cfg_t g_adc0_group_1_cfg =
                     {
                         .scan_group_id                   = ADC_GROUP_ID_1,
                         .converter_selection             = (adc_b_unit_id_t)(0),
                         .scan_group_enable               = (0),
                         .scan_end_interrupt_enable       = (1),
                         .external_trigger_enable_mask    = ( ADC_B_EXTERNAL_TRIGGER_NONE),
                         .elc_trigger_enable_mask         = (elc_peripheral_t)( 0x00),
                         .gpt_trigger_enable_mask         = ( ADC_B_GPT_TRIGGER_NONE),

                         .self_diagnosis_mask             = (ADC_B_SELF_DIAGNOSIS_DISABLED << R_ADC_B0_ADSGDCR0_DIAGVAL_Pos),

                         .limit_clip_interrupt_enable     = (1),
                         .virtual_channel_count           = 0 + (
                         ((1) == 2)+((1) == 2)+((1) == 2)+((1) == 2)+((0) == 2)+((0) == 2)+((0) == 2)+((0) == 2)+((0) == 2)+((0) == 2)+((0) == 2)+((0) == 2)+((0) == 2)+((0) == 2)+((0) == 2)+((0) == 2)+((0) == 2)+((0) == 2)+((0) == 2)+((0) == 2)+((0) == 2)+((0) == 2)+((0) == 2)+((0) == 2)+((0) == 2)+((0) == 2)+((0) == 2)+((0) == 2)+((0) == 2)+((0) == 2)+((0) == 2)+((0) == 2)+((0) == 2)+(RA_NOT_DEFINED == 2)+(RA_NOT_DEFINED == 2)+(RA_NOT_DEFINED == 2)+(RA_NOT_DEFINED == 2)),
#if (((1) == 2)||((1) == 2)||((1) == 2)||((1) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||((0) == 2)||(RA_NOT_DEFINED == 2)||(RA_NOT_DEFINED == 2)||(RA_NOT_DEFINED == 2)||(RA_NOT_DEFINED == 2))

                         .p_virtual_channels = (adc_b_virtual_channel_cfg_t**)g_adc0_group_1_virtual_channels,
                         #else
                         .p_virtual_channels = NULL,
                         #endif
                     };
                     #endif

#if (0) // Define Scan Group 2 if it's enabled
                     const adc_b_group_cfg_t g_adc0_group_2_cfg =
                     {
                         .scan_group_id                   = ADC_GROUP_ID_2,
                         .converter_selection             = (adc_b_unit_id_t)(0),
                         .scan_group_enable               = (0),
                         .scan_end_interrupt_enable       = (1),
                         .external_trigger_enable_mask    = ( ADC_B_EXTERNAL_TRIGGER_NONE),
                         .elc_trigger_enable_mask         = (elc_peripheral_t)( 0x00),
                         .gpt_trigger_enable_mask         = ( ADC_B_GPT_TRIGGER_NONE),

                         .self_diagnosis_mask             = (ADC_B_SELF_DIAGNOSIS_DISABLED << R_ADC_B0_ADSGDCR0_DIAGVAL_Pos),

                         .limit_clip_interrupt_enable     = (1),
                         .virtual_channel_count           = 0 + (
                         ((1) == 3)+((1) == 3)+((1) == 3)+((1) == 3)+((0) == 3)+((0) == 3)+((0) == 3)+((0) == 3)+((0) == 3)+((0) == 3)+((0) == 3)+((0) == 3)+((0) == 3)+((0) == 3)+((0) == 3)+((0) == 3)+((0) == 3)+((0) == 3)+((0) == 3)+((0) == 3)+((0) == 3)+((0) == 3)+((0) == 3)+((0) == 3)+((0) == 3)+((0) == 3)+((0) == 3)+((0) == 3)+((0) == 3)+((0) == 3)+((0) == 3)+((0) == 3)+((0) == 3)+(RA_NOT_DEFINED == 3)+(RA_NOT_DEFINED == 3)+(RA_NOT_DEFINED == 3)+(RA_NOT_DEFINED == 3)),
#if (((1) == 3)||((1) == 3)||((1) == 3)||((1) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||((0) == 3)||(RA_NOT_DEFINED == 3)||(RA_NOT_DEFINED == 3)||(RA_NOT_DEFINED == 3)||(RA_NOT_DEFINED == 3))

                         .p_virtual_channels = (adc_b_virtual_channel_cfg_t**)g_adc0_group_2_virtual_channels,
                         #else
                         .p_virtual_channels = NULL,
                         #endif
                     };
                     #endif

#if (0) // Define Scan Group 3 if it's enabled
                     const adc_b_group_cfg_t g_adc0_group_3_cfg =
                     {
                         .scan_group_id                   = ADC_GROUP_ID_3,
                         .converter_selection             = (adc_b_unit_id_t)(0),
                         .scan_group_enable               = (0),
                         .scan_end_interrupt_enable       = (1),
                         .external_trigger_enable_mask    = ( ADC_B_EXTERNAL_TRIGGER_NONE),
                         .elc_trigger_enable_mask         = (elc_peripheral_t)( 0x00),
                         .gpt_trigger_enable_mask         = ( ADC_B_GPT_TRIGGER_NONE),

                         .self_diagnosis_mask             = (ADC_B_SELF_DIAGNOSIS_DISABLED << R_ADC_B0_ADSGDCR0_DIAGVAL_Pos),

                         .limit_clip_interrupt_enable     = (1),
                         .virtual_channel_count           = 0 + (
                         ((1) == 4)+((1) == 4)+((1) == 4)+((1) == 4)+((0) == 4)+((0) == 4)+((0) == 4)+((0) == 4)+((0) == 4)+((0) == 4)+((0) == 4)+((0) == 4)+((0) == 4)+((0) == 4)+((0) == 4)+((0) == 4)+((0) == 4)+((0) == 4)+((0) == 4)+((0) == 4)+((0) == 4)+((0) == 4)+((0) == 4)+((0) == 4)+((0) == 4)+((0) == 4)+((0) == 4)+((0) == 4)+((0) == 4)+((0) == 4)+((0) == 4)+((0) == 4)+((0) == 4)+(RA_NOT_DEFINED == 4)+(RA_NOT_DEFINED == 4)+(RA_NOT_DEFINED == 4)+(RA_NOT_DEFINED == 4)),
#if (((1) == 4)||((1) == 4)||((1) == 4)||((1) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||((0) == 4)||(RA_NOT_DEFINED == 4)||(RA_NOT_DEFINED == 4)||(RA_NOT_DEFINED == 4)||(RA_NOT_DEFINED == 4))

                         .p_virtual_channels = (adc_b_virtual_channel_cfg_t**)g_adc0_group_3_virtual_channels,
                         #else
                         .p_virtual_channels = NULL,
                         #endif
                     };
                     #endif

#if (0) // Define Scan Group 4 if it's enabled
                     const adc_b_group_cfg_t g_adc0_group_4_cfg =
                     {
                         .scan_group_id                   = ADC_GROUP_ID_4,
                         .converter_selection             = (adc_b_unit_id_t)(0),
                         .scan_group_enable               = (0),
                         .scan_end_interrupt_enable       = (1),
                         .external_trigger_enable_mask    = ( ADC_B_EXTERNAL_TRIGGER_NONE),
                         .elc_trigger_enable_mask         = (elc_peripheral_t)( 0x00),
                         .gpt_trigger_enable_mask         = ( ADC_B_GPT_TRIGGER_NONE),

                         .self_diagnosis_mask             = (ADC_B_SELF_DIAGNOSIS_DISABLED << R_ADC_B0_ADSGDCR0_DIAGVAL_Pos),

                         .limit_clip_interrupt_enable     = (1),
                         .virtual_channel_count           = 0 + (
                         ((1) == 5)+((1) == 5)+((1) == 5)+((1) == 5)+((0) == 5)+((0) == 5)+((0) == 5)+((0) == 5)+((0) == 5)+((0) == 5)+((0) == 5)+((0) == 5)+((0) == 5)+((0) == 5)+((0) == 5)+((0) == 5)+((0) == 5)+((0) == 5)+((0) == 5)+((0) == 5)+((0) == 5)+((0) == 5)+((0) == 5)+((0) == 5)+((0) == 5)+((0) == 5)+((0) == 5)+((0) == 5)+((0) == 5)+((0) == 5)+((0) == 5)+((0) == 5)+((0) == 5)+(RA_NOT_DEFINED == 5)+(RA_NOT_DEFINED == 5)+(RA_NOT_DEFINED == 5)+(RA_NOT_DEFINED == 5)),
#if (((1) == 5)||((1) == 5)||((1) == 5)||((1) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||((0) == 5)||(RA_NOT_DEFINED == 5)||(RA_NOT_DEFINED == 5)||(RA_NOT_DEFINED == 5)||(RA_NOT_DEFINED == 5))

                         .p_virtual_channels = (adc_b_virtual_channel_cfg_t**)g_adc0_group_4_virtual_channels,
                         #else
                         .p_virtual_channels = NULL,
                         #endif
                     };
                     #endif

#if (0) // Define Scan Group 5 if it's enabled
                     const adc_b_group_cfg_t g_adc0_group_5_cfg =
                     {
                         .scan_group_id                   = ADC_GROUP_ID_5,
                         .converter_selection             = (adc_b_unit_id_t)(0),
                         .scan_group_enable               = (0),
                         .scan_end_interrupt_enable       = (1),
                         .external_trigger_enable_mask    = ( ADC_B_EXTERNAL_TRIGGER_NONE),
                         .elc_trigger_enable_mask         = (elc_peripheral_t)( 0x00),
                         .gpt_trigger_enable_mask         = ( ADC_B_GPT_TRIGGER_NONE),

                         .self_diagnosis_mask             = (ADC_B_SELF_DIAGNOSIS_DISABLED << R_ADC_B0_ADSGDCR0_DIAGVAL_Pos),

                         .limit_clip_interrupt_enable     = (1),
                         .virtual_channel_count           = 0 + (
                         ((1) == 6)+((1) == 6)+((1) == 6)+((1) == 6)+((0) == 6)+((0) == 6)+((0) == 6)+((0) == 6)+((0) == 6)+((0) == 6)+((0) == 6)+((0) == 6)+((0) == 6)+((0) == 6)+((0) == 6)+((0) == 6)+((0) == 6)+((0) == 6)+((0) == 6)+((0) == 6)+((0) == 6)+((0) == 6)+((0) == 6)+((0) == 6)+((0) == 6)+((0) == 6)+((0) == 6)+((0) == 6)+((0) == 6)+((0) == 6)+((0) == 6)+((0) == 6)+((0) == 6)+(RA_NOT_DEFINED == 6)+(RA_NOT_DEFINED == 6)+(RA_NOT_DEFINED == 6)+(RA_NOT_DEFINED == 6)),
#if (((1) == 6)||((1) == 6)||((1) == 6)||((1) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||((0) == 6)||(RA_NOT_DEFINED == 6)||(RA_NOT_DEFINED == 6)||(RA_NOT_DEFINED == 6)||(RA_NOT_DEFINED == 6))

                         .p_virtual_channels = (adc_b_virtual_channel_cfg_t**)g_adc0_group_5_virtual_channels,
                         #else
                         .p_virtual_channels = NULL,
                         #endif
                     };
                     #endif

#if (0) // Define Scan Group 6 if it's enabled
                     const adc_b_group_cfg_t g_adc0_group_6_cfg =
                     {
                         .scan_group_id                   = ADC_GROUP_ID_6,
                         .converter_selection             = (adc_b_unit_id_t)(0),
                         .scan_group_enable               = (0),
                         .scan_end_interrupt_enable       = (1),
                         .external_trigger_enable_mask    = ( ADC_B_EXTERNAL_TRIGGER_NONE),
                         .elc_trigger_enable_mask         = (elc_peripheral_t)( 0x00),
                         .gpt_trigger_enable_mask         = ( ADC_B_GPT_TRIGGER_NONE),

                         .self_diagnosis_mask             = (ADC_B_SELF_DIAGNOSIS_DISABLED << R_ADC_B0_ADSGDCR0_DIAGVAL_Pos),

                         .limit_clip_interrupt_enable     = (1),
                         .virtual_channel_count           = 0 + (
                         ((1) == 7)+((1) == 7)+((1) == 7)+((1) == 7)+((0) == 7)+((0) == 7)+((0) == 7)+((0) == 7)+((0) == 7)+((0) == 7)+((0) == 7)+((0) == 7)+((0) == 7)+((0) == 7)+((0) == 7)+((0) == 7)+((0) == 7)+((0) == 7)+((0) == 7)+((0) == 7)+((0) == 7)+((0) == 7)+((0) == 7)+((0) == 7)+((0) == 7)+((0) == 7)+((0) == 7)+((0) == 7)+((0) == 7)+((0) == 7)+((0) == 7)+((0) == 7)+((0) == 7)+(RA_NOT_DEFINED == 7)+(RA_NOT_DEFINED == 7)+(RA_NOT_DEFINED == 7)+(RA_NOT_DEFINED == 7)),
#if (((1) == 7)||((1) == 7)||((1) == 7)||((1) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||((0) == 7)||(RA_NOT_DEFINED == 7)||(RA_NOT_DEFINED == 7)||(RA_NOT_DEFINED == 7)||(RA_NOT_DEFINED == 7))

                         .p_virtual_channels = (adc_b_virtual_channel_cfg_t**)g_adc0_group_6_virtual_channels,
                         #else
                         .p_virtual_channels = NULL,
                         #endif
                     };
                     #endif

#if (0) // Define Scan Group 7 if it's enabled
                     const adc_b_group_cfg_t g_adc0_group_7_cfg =
                     {
                         .scan_group_id                   = ADC_GROUP_ID_7,
                         .converter_selection             = (adc_b_unit_id_t)(0),
                         .scan_group_enable               = (0),
                         .scan_end_interrupt_enable       = (1),
                         .external_trigger_enable_mask    = ( ADC_B_EXTERNAL_TRIGGER_NONE),
                         .elc_trigger_enable_mask         = (elc_peripheral_t)( 0x00),
                         .gpt_trigger_enable_mask         = ( ADC_B_GPT_TRIGGER_NONE),

                         .self_diagnosis_mask             = (ADC_B_SELF_DIAGNOSIS_DISABLED << R_ADC_B0_ADSGDCR0_DIAGVAL_Pos),

                         .limit_clip_interrupt_enable     = (1),
                         .virtual_channel_count           = 0 + (
                         ((1) == 8)+((1) == 8)+((1) == 8)+((1) == 8)+((0) == 8)+((0) == 8)+((0) == 8)+((0) == 8)+((0) == 8)+((0) == 8)+((0) == 8)+((0) == 8)+((0) == 8)+((0) == 8)+((0) == 8)+((0) == 8)+((0) == 8)+((0) == 8)+((0) == 8)+((0) == 8)+((0) == 8)+((0) == 8)+((0) == 8)+((0) == 8)+((0) == 8)+((0) == 8)+((0) == 8)+((0) == 8)+((0) == 8)+((0) == 8)+((0) == 8)+((0) == 8)+((0) == 8)+(RA_NOT_DEFINED == 8)+(RA_NOT_DEFINED == 8)+(RA_NOT_DEFINED == 8)+(RA_NOT_DEFINED == 8)),
#if (((1) == 8)||((1) == 8)||((1) == 8)||((1) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||((0) == 8)||(RA_NOT_DEFINED == 8)||(RA_NOT_DEFINED == 8)||(RA_NOT_DEFINED == 8)||(RA_NOT_DEFINED == 8))

                         .p_virtual_channels = (adc_b_virtual_channel_cfg_t**)g_adc0_group_7_virtual_channels,
                         #else
                         .p_virtual_channels = NULL,
                         #endif
                     };
                     #endif

#if (0) // Define Scan Group 8 if it's enabled
                     const adc_b_group_cfg_t g_adc0_group_8_cfg =
                     {
                         .scan_group_id                   = ADC_GROUP_ID_8,
                         .converter_selection             = (adc_b_unit_id_t)(0),
                         .scan_group_enable               = (0),
                         .scan_end_interrupt_enable       = (1),
                         .external_trigger_enable_mask    = ( ADC_B_EXTERNAL_TRIGGER_NONE),
                         .elc_trigger_enable_mask         = (elc_peripheral_t)( 0x00),
                         .gpt_trigger_enable_mask         = ( ADC_B_GPT_TRIGGER_NONE),

                         .self_diagnosis_mask             = (ADC_B_SELF_DIAGNOSIS_DISABLED << R_ADC_B0_ADSGDCR0_DIAGVAL_Pos),

                         .limit_clip_interrupt_enable     = (1),
                         .virtual_channel_count           = 0 + (
                         ((1) == 9)+((1) == 9)+((1) == 9)+((1) == 9)+((0) == 9)+((0) == 9)+((0) == 9)+((0) == 9)+((0) == 9)+((0) == 9)+((0) == 9)+((0) == 9)+((0) == 9)+((0) == 9)+((0) == 9)+((0) == 9)+((0) == 9)+((0) == 9)+((0) == 9)+((0) == 9)+((0) == 9)+((0) == 9)+((0) == 9)+((0) == 9)+((0) == 9)+((0) == 9)+((0) == 9)+((0) == 9)+((0) == 9)+((0) == 9)+((0) == 9)+((0) == 9)+((0) == 9)+(RA_NOT_DEFINED == 9)+(RA_NOT_DEFINED == 9)+(RA_NOT_DEFINED == 9)+(RA_NOT_DEFINED == 9)),
#if (((1) == 9)||((1) == 9)||((1) == 9)||((1) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||((0) == 9)||(RA_NOT_DEFINED == 9)||(RA_NOT_DEFINED == 9)||(RA_NOT_DEFINED == 9)||(RA_NOT_DEFINED == 9))

                         .p_virtual_channels = (adc_b_virtual_channel_cfg_t**)g_adc0_group_8_virtual_channels,
                         #else
                         .p_virtual_channels = NULL,
                         #endif
                     };
                     #endif
#if ((1)||(0)||(0)||(0)||(0)||(0)||(0)||(0)||(0))
const adc_b_group_cfg_t *const g_adc0_scan_cfg_groups[] =
{
#if (0 != (1))
  &g_adc0_group_0_cfg,
#endif

#if (0 != (0))
                           &g_adc0_group_1_cfg,
                           #endif

#if (0 != (0))
                           &g_adc0_group_2_cfg,
                           #endif

#if (0 != (0))
                           &g_adc0_group_3_cfg,
                           #endif

#if (0 != (0))
                           &g_adc0_group_4_cfg,
                           #endif

#if (0 != (0))
                           &g_adc0_group_5_cfg,
                           #endif

#if (0 != (0))
                           &g_adc0_group_6_cfg,
                           #endif

#if (0 != (0))
                           &g_adc0_group_7_cfg,
                           #endif

#if (0 != (0))
                           &g_adc0_group_8_cfg,
                           #endif
        };
#endif

const adc_b_scan_cfg_t g_adc0_scan_cfg =
{ .group_count = (0 + (0 != (1)) + (0 != (0)) + (0 != (0)) + (0 != (0)) + (0 != (0)) + (0 != (0)) + (0 != (0))
        + (0 != (0)) + (0 != (0))),
#if ((0 != (1))||(0 != (0))||(0 != (0))||(0 != (0))||(0 != (0))||(0 != (0))||(0 != (0))||(0 != (0))||(0 != (0)))
  .p_adc_groups = (adc_b_group_cfg_t**) g_adc0_scan_cfg_groups,
#else
                       .p_adc_groups = NULL,
                       #endif
        };

const adc_b_isr_cfg_t g_adc0_isr_cfg =
{ .calibration_end_ipl_adc_0 = (12),
  .calibration_end_ipl_adc_1 = (12),
  .limit_clip_ipl = (12),
  .conversion_error_ipl_adc_0 = (12),
  .conversion_error_ipl_adc_1 = (12),
  .overflow_error_ipl_adc_0 = (12),
  .overflow_error_ipl_adc_1 = (12),

  .scan_end_ipl_group_0 = (12),
  .scan_end_ipl_group_1 = (12),
  .scan_end_ipl_group_2 = (12),
  .scan_end_ipl_group_3 = (12),
  .scan_end_ipl_group_4 = (12),
  .scan_end_ipl_group_5 = (BSP_IRQ_DISABLED),
  .scan_end_ipl_group_6 = (BSP_IRQ_DISABLED),
  .scan_end_ipl_group_7 = (BSP_IRQ_DISABLED),
  .scan_end_ipl_group_8 = (BSP_IRQ_DISABLED),
  .scan_end_ipl_group_5678 = (BSP_IRQ_DISABLED),
  .fifo_overflow_ipl = (12),
  .fifo_read_ipl_group_0 = (12),
  .fifo_read_ipl_group_1 = (12),
  .fifo_read_ipl_group_2 = (12),
  .fifo_read_ipl_group_3 = (12),
  .fifo_read_ipl_group_4 = (12),
  .fifo_read_ipl_group_5 = (BSP_IRQ_DISABLED),
  .fifo_read_ipl_group_6 = (BSP_IRQ_DISABLED),
  .fifo_read_ipl_group_7 = (BSP_IRQ_DISABLED),
  .fifo_read_ipl_group_8 = (BSP_IRQ_DISABLED),
  .fifo_read_ipl_group_5678 = (BSP_IRQ_DISABLED),
#if defined(VECTOR_NUMBER_ADC_CALEND0) && ( (12) != BSP_IRQ_DISABLED )
    .calibration_end_irq_adc_0 = VECTOR_NUMBER_ADC_CALEND0,
#else
  .calibration_end_irq_adc_0 = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_ADC_CALEND1) && ( (12) != BSP_IRQ_DISABLED )
    .calibration_end_irq_adc_1 = VECTOR_NUMBER_ADC_CALEND1,
#else
  .calibration_end_irq_adc_1 = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_ADC_LIMCLPI) && ( (12) != BSP_IRQ_DISABLED )
    .limit_clip_irq = VECTOR_NUMBER_ADC_LIMCLPI,
#else
  .limit_clip_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_ADC_ERR0) && ( (12) != BSP_IRQ_DISABLED )
    .conversion_error_irq_adc_0 = VECTOR_NUMBER_ADC_ERR0,
#else
  .conversion_error_irq_adc_0 = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_ADC_ERR1) && ( (12) != BSP_IRQ_DISABLED )
    .conversion_error_irq_adc_1 = VECTOR_NUMBER_ADC_ERR1,
#else
  .conversion_error_irq_adc_1 = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_ADC_RESOVF0) && ( (12) != BSP_IRQ_DISABLED )
    .overflow_error_irq_adc_0 = VECTOR_NUMBER_ADC_RESOVF0,
#else
  .overflow_error_irq_adc_0 = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_ADC_RESOVF1) && ( (12) != BSP_IRQ_DISABLED )
    .overflow_error_irq_adc_1 = VECTOR_NUMBER_ADC_RESOVF1,
#else
  .overflow_error_irq_adc_1 = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_ADC_ADI0) && ( (12) != BSP_IRQ_DISABLED )
    .scan_end_irq_group_0 = VECTOR_NUMBER_ADC_ADI0,
#else
  .scan_end_irq_group_0 = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_ADC_ADI1) && ( (12) != BSP_IRQ_DISABLED )
    .scan_end_irq_group_1 = VECTOR_NUMBER_ADC_ADI1,
#else
  .scan_end_irq_group_1 = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_ADC_ADI2) && ( (12) != BSP_IRQ_DISABLED )
    .scan_end_irq_group_2 = VECTOR_NUMBER_ADC_ADI2,
#else
  .scan_end_irq_group_2 = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_ADC_ADI3) && ( (12) != BSP_IRQ_DISABLED )
    .scan_end_irq_group_3 = VECTOR_NUMBER_ADC_ADI3,
#else
  .scan_end_irq_group_3 = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_ADC_ADI4) && ( (12) != BSP_IRQ_DISABLED )
    .scan_end_irq_group_4 = VECTOR_NUMBER_ADC_ADI4,
#else
  .scan_end_irq_group_4 = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_ADC_ADI5) && ( (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED )
    .scan_end_irq_group_5 = VECTOR_NUMBER_ADC_ADI5,
#else
  .scan_end_irq_group_5 = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_ADC_ADI6) && ( (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED )
    .scan_end_irq_group_6 = VECTOR_NUMBER_ADC_ADI6,
#else
  .scan_end_irq_group_6 = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_ADC_ADI7) && ( (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED )
    .scan_end_irq_group_7 = VECTOR_NUMBER_ADC_ADI7,
#else
  .scan_end_irq_group_7 = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_ADC_ADI8) && ( (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED )
    .scan_end_irq_group_8 = VECTOR_NUMBER_ADC_ADI8,
#else
  .scan_end_irq_group_8 = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_ADC_ADI5678) && ( (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED )
    .scan_end_irq_group_5678 = VECTOR_NUMBER_ADC_ADI5678,
#else
  .scan_end_irq_group_5678 = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_ADC_FIFOOVF) && ( (12) != BSP_IRQ_DISABLED )
    .fifo_overflow_irq = VECTOR_NUMBER_ADC_FIFOOVF,
#else
  .fifo_overflow_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_ADC_FIFOREQ0) && ( (12) != BSP_IRQ_DISABLED )
    .fifo_read_irq_group_0 = VECTOR_NUMBER_ADC_FIFOREQ0,
#else
  .fifo_read_irq_group_0 = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_ADC_FIFOREQ1) && ( (12) != BSP_IRQ_DISABLED )
    .fifo_read_irq_group_1 = VECTOR_NUMBER_ADC_FIFOREQ1,
#else
  .fifo_read_irq_group_1 = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_ADC_FIFOREQ2) && ( (12) != BSP_IRQ_DISABLED )
    .fifo_read_irq_group_2 = VECTOR_NUMBER_ADC_FIFOREQ2,
#else
  .fifo_read_irq_group_2 = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_ADC_FIFOREQ3) && ( (12) != BSP_IRQ_DISABLED )
    .fifo_read_irq_group_3 = VECTOR_NUMBER_ADC_FIFOREQ3,
#else
  .fifo_read_irq_group_3 = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_ADC_FIFOREQ4) && ( (12) != BSP_IRQ_DISABLED )
    .fifo_read_irq_group_4 = VECTOR_NUMBER_ADC_FIFOREQ4,
#else
  .fifo_read_irq_group_4 = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_ADC_FIFOREQ5) && ( (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED )
    .fifo_read_irq_group_5 = VECTOR_NUMBER_ADC_FIFOREQ5,
#else
  .fifo_read_irq_group_5 = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_ADC_FIFOREQ6) && ( (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED )
    .fifo_read_irq_group_6 = VECTOR_NUMBER_ADC_FIFOREQ6,
#else
  .fifo_read_irq_group_6 = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_ADC_FIFOREQ7) && ( (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED )
    .fifo_read_irq_group_7 = VECTOR_NUMBER_ADC_FIFOREQ7,
#else
  .fifo_read_irq_group_7 = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_ADC_FIFOREQ8) && ( (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED )
    .fifo_read_irq_group_8 = VECTOR_NUMBER_ADC_FIFOREQ8,
#else
  .fifo_read_irq_group_8 = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_ADC_FIFOREQ5678) && ( (BSP_IRQ_DISABLED) != BSP_IRQ_DISABLED )
    .fifo_read_irq_group_5678 = VECTOR_NUMBER_ADC_FIFOREQ5678,
#else
  .fifo_read_irq_group_5678 = FSP_INVALID_VECTOR,
#endif
        };

const adc_b_extended_cfg_t g_adc0_cfg_extend =
        { .clock_control_data = ((ADC_B_CLOCK_SOURCE_ADC << R_ADC_B0_ADCLKCR_CLKSEL_Pos)
                | (ADC_B_CLOCK_DIV_1 << R_ADC_B0_ADCLKCR_DIVR_Pos)),
          .sync_operation_control = (((1) << R_ADC_B0_ADSYCR_ADSYDIS0_Pos) | ((1) << R_ADC_B0_ADSYCR_ADSYDIS1_Pos)
                  | (100 << R_ADC_B0_ADSYCR_ADSYCYC_Pos)),
          .adc_b_converter_mode[0] =
          { .mode = (ADC_B_CONVERTER_MODE_CONTINUOUS_SCAN), .method = (ADC_B_CONVERSION_METHOD_SAR) },
          .adc_b_converter_mode[1] =
          { .mode = (ADC_B_CONVERTER_MODE_SINGLE_SCAN), .method = (ADC_B_CONVERSION_METHOD_SAR) },
          .converter_selection_0 = (((0) << R_ADC_B0_ADSGCR0_SGADS0_Pos) | ((0) << R_ADC_B0_ADSGCR0_SGADS1_Pos)
                  | ((0) << R_ADC_B0_ADSGCR0_SGADS2_Pos) | ((0) << R_ADC_B0_ADSGCR0_SGADS3_Pos)),
          .converter_selection_1 = (((0) << R_ADC_B0_ADSGCR1_SGADS4_Pos) | ((0) << R_ADC_B0_ADSGCR1_SGADS5_Pos)
                  | ((0) << R_ADC_B0_ADSGCR1_SGADS6_Pos) | ((0) << R_ADC_B0_ADSGCR1_SGADS7_Pos)),
          .converter_selection_2 = ((0) << R_ADC_B0_ADSGCR2_SGADS8_Pos),

          .fifo_enable_mask = (((0) << R_ADC_B0_ADFIFOCR_FIFOEN0_Pos) | ((0) << R_ADC_B0_ADFIFOCR_FIFOEN1_Pos)
                  | ((0) << R_ADC_B0_ADFIFOCR_FIFOEN2_Pos) | ((0) << R_ADC_B0_ADFIFOCR_FIFOEN3_Pos)
                  | ((0) << R_ADC_B0_ADFIFOCR_FIFOEN4_Pos) | ((0) << R_ADC_B0_ADFIFOCR_FIFOEN5_Pos)
                  | ((0) << R_ADC_B0_ADFIFOCR_FIFOEN6_Pos) | ((0) << R_ADC_B0_ADFIFOCR_FIFOEN7_Pos)
                  | ((0) << R_ADC_B0_ADFIFOCR_FIFOEN8_Pos)),
          .fifo_interrupt_enable_mask = (((0) << R_ADC_B0_ADFIFOINTCR_FIFOIE0_Pos)
                  | ((0) << R_ADC_B0_ADFIFOINTCR_FIFOIE1_Pos) | ((0) << R_ADC_B0_ADFIFOINTCR_FIFOIE2_Pos)
                  | ((0) << R_ADC_B0_ADFIFOINTCR_FIFOIE3_Pos) | ((0) << R_ADC_B0_ADFIFOINTCR_FIFOIE4_Pos)
                  | ((0) << R_ADC_B0_ADFIFOINTCR_FIFOIE5_Pos) | ((0) << R_ADC_B0_ADFIFOINTCR_FIFOIE6_Pos)
                  | ((0) << R_ADC_B0_ADFIFOINTCR_FIFOIE7_Pos) | ((0) << R_ADC_B0_ADFIFOINTCR_FIFOIE8_Pos)),
          .fifo_interrupt_level0 = ((0 << R_ADC_B0_ADFIFOINTLR0_FIFOILV0_Pos)
                  | (0 << R_ADC_B0_ADFIFOINTLR0_FIFOILV1_Pos)),
          .fifo_interrupt_level1 = ((0 << R_ADC_B0_ADFIFOINTLR1_FIFOILV2_Pos)
                  | (0 << R_ADC_B0_ADFIFOINTLR1_FIFOILV3_Pos)),
          .fifo_interrupt_level2 = ((0 << R_ADC_B0_ADFIFOINTLR2_FIFOILV4_Pos)
                  | (0 << R_ADC_B0_ADFIFOINTLR2_FIFOILV5_Pos)),
          .fifo_interrupt_level3 = ((0 << R_ADC_B0_ADFIFOINTLR3_FIFOILV6_Pos)
                  | (0 << R_ADC_B0_ADFIFOINTLR3_FIFOILV7_Pos)),
          .fifo_interrupt_level4 = (0 << R_ADC_B0_ADFIFOINTLR4_FIFOILV8_Pos),

          .start_trigger_delay_0 = ((0 << R_ADC_B0_ADTRGDLR0_TRGDLY0_Pos) | (0 << R_ADC_B0_ADTRGDLR0_TRGDLY1_Pos)),
          .start_trigger_delay_1 = ((0 << R_ADC_B0_ADTRGDLR1_TRGDLY2_Pos) | (0 << R_ADC_B0_ADTRGDLR1_TRGDLY3_Pos)), .start_trigger_delay_2 =
                  ((0 << R_ADC_B0_ADTRGDLR2_TRGDLY4_Pos) | (0 << R_ADC_B0_ADTRGDLR2_TRGDLY5_Pos)),
          .start_trigger_delay_3 = ((0 << R_ADC_B0_ADTRGDLR3_TRGDLY6_Pos) | (0 << R_ADC_B0_ADTRGDLR3_TRGDLY7_Pos)), .start_trigger_delay_4 =
                  ((0 << R_ADC_B0_ADTRGDLR4_TRGDLY8_Pos)),
          .calibration_adc_state = ((95 << R_ADC_B0_ADCALSTCR_CALADSST_Pos) | (5 << R_ADC_B0_ADCALSTCR_CALADCST_Pos)), .calibration_sample_and_hold =
                  ((95 << R_ADC_B0_ADCALSHCR_CALSHSST_Pos) | (5 << R_ADC_B0_ADCALSHCR_CALSHHST_Pos)),
          .p_isr_cfg = &g_adc0_isr_cfg, .sampling_state_tables =
          { ((95 << R_ADC_B0_ADSSTR0_SST0_Pos) | (95 << R_ADC_B0_ADSSTR0_SST1_Pos)), ((95 << R_ADC_B0_ADSSTR1_SST2_Pos)
                    | (95 << R_ADC_B0_ADSSTR1_SST3_Pos)),
            ((95 << R_ADC_B0_ADSSTR2_SST4_Pos) | (95 << R_ADC_B0_ADSSTR2_SST5_Pos)), ((95 << R_ADC_B0_ADSSTR3_SST6_Pos)
                    | (95 << R_ADC_B0_ADSSTR3_SST7_Pos)),
            ((95 << R_ADC_B0_ADSSTR4_SST8_Pos) | (95 << R_ADC_B0_ADSSTR4_SST9_Pos)), ((95 << R_ADC_B0_ADSSTR5_SST10_Pos)
                    | (95 << R_ADC_B0_ADSSTR5_SST11_Pos)),
            ((95 << R_ADC_B0_ADSSTR6_SST12_Pos) | (95 << R_ADC_B0_ADSSTR6_SST13_Pos)), ((95
                    << R_ADC_B0_ADSSTR7_SST14_Pos) | (95 << R_ADC_B0_ADSSTR7_SST15_Pos)), },
          .sample_and_hold_enable_mask = (ADC_B_SAMPLE_AND_HOLD_MASK_NONE), .sample_and_hold_config_012 = ((95
                  << R_ADC_B0_ADSHSTR1_SHSST_Pos) | (5 << R_ADC_B0_ADSHSTR0_SHHST_Pos)),
          .sample_and_hold_config_456 = ((95 << R_ADC_B0_ADSHSTR1_SHSST_Pos) | (5 << R_ADC_B0_ADSHSTR1_SHHST_Pos)), .conversion_state =
                  ((5 << R_ADC_B0_ADCNVSTR_CST0_Pos) | (5 << R_ADC_B0_ADCNVSTR_CST1_Pos)),
          .user_offset_tables =
          { 0, 0, 0, 0, 0, 0, 0, 0, },
          .user_gain_tables =
          { ((1U << R_ADC_B0_ADUGTR0_UGAINI_Pos) + (uint32_t) (0 * (R_ADC_B0_ADUGTR0_UGAINF_Msk + 1))), ((1U
                  << R_ADC_B0_ADUGTR1_UGAINI_Pos) + (uint32_t) (0 * (R_ADC_B0_ADUGTR1_UGAINF_Msk + 1))),
            ((1U << R_ADC_B0_ADUGTR2_UGAINI_Pos) + (uint32_t) (0 * (R_ADC_B0_ADUGTR2_UGAINF_Msk + 1))), ((1U
                    << R_ADC_B0_ADUGTR3_UGAINI_Pos) + (uint32_t) (0 * (R_ADC_B0_ADUGTR3_UGAINF_Msk + 1))),
            ((1U << R_ADC_B0_ADUGTR4_UGAINI_Pos) + (uint32_t) (0 * (R_ADC_B0_ADUGTR4_UGAINF_Msk + 1))), ((1U
                    << R_ADC_B0_ADUGTR5_UGAINI_Pos) + (uint32_t) (0 * (R_ADC_B0_ADUGTR5_UGAINF_Msk + 1))),
            ((1U << R_ADC_B0_ADUGTR6_UGAINI_Pos) + (uint32_t) (0 * (R_ADC_B0_ADUGTR6_UGAINF_Msk + 1))), ((1U
                    << R_ADC_B0_ADUGTR7_UGAINI_Pos) + (uint32_t) (0 * (R_ADC_B0_ADUGTR7_UGAINF_Msk + 1))), },
          .limiter_clip_interrupt_enable_mask = (0x00), .limiter_clip_tables =
          { (0 | 0 << R_ADC_B0_ADLIMTR0_LIMU_Pos), (0 | 0 << R_ADC_B0_ADLIMTR1_LIMU_Pos), (0
                    | 0 << R_ADC_B0_ADLIMTR2_LIMU_Pos),
            (0 | 0 << R_ADC_B0_ADLIMTR3_LIMU_Pos), (0 | 0 << R_ADC_B0_ADLIMTR4_LIMU_Pos), (0
                    | 0 << R_ADC_B0_ADLIMTR5_LIMU_Pos),
            (0 | 0 << R_ADC_B0_ADLIMTR6_LIMU_Pos), (0 | 0 << R_ADC_B0_ADLIMTR7_LIMU_Pos), },

#if (1 == 0)
    .pga_gain[0] = ADC_B_PGA_GAIN_SINGLE_ENDED_2_500,
    #elif (2 == 0)
    .pga_gain[0] = ADC_B_PGA_GAIN_DIFFERENTIAL_1_500,
    #else
          .pga_gain[0] = ADC_B_PGA_GAIN_DISABLED,
#endif

#if (1 == 0)
    .pga_gain[1] = ADC_B_PGA_GAIN_SINGLE_ENDED_2_500,
    #elif (2 == 0)
    .pga_gain[1] = ADC_B_PGA_GAIN_DIFFERENTIAL_1_500,
    #else
          .pga_gain[1] = ADC_B_PGA_GAIN_DISABLED,
#endif

#if (1 == 0)
    .pga_gain[2] = ADC_B_PGA_GAIN_SINGLE_ENDED_2_500,
    #elif (2 == 0)
    .pga_gain[2] = ADC_B_PGA_GAIN_DIFFERENTIAL_1_500,
    #else
          .pga_gain[2] = ADC_B_PGA_GAIN_DISABLED,
#endif

#if (1 == 0)
    .pga_gain[3] = ADC_B_PGA_GAIN_SINGLE_ENDED_2_500,
    #elif (2 == 0)
    .pga_gain[3] = ADC_B_PGA_GAIN_DIFFERENTIAL_1_500,
    #else
          .pga_gain[3] = ADC_B_PGA_GAIN_DISABLED,
#endif

          /* For debug only! Prolonged use of PGA Monitor function may deteriorate PGA characteristics. See user manual for more information.*/
          .pga_debug_monitor_mask_b.unit_0 = 0,
          .pga_debug_monitor_mask_b.unit_1 = 0, .pga_debug_monitor_mask_b.unit_2 = 0, .pga_debug_monitor_mask_b.unit_3 =
                  0,
          .adc_filter_selection[0].bits = ((R_ADC_B0_ADDFSR0_DFSEL0_Msk
                  & (ADC_B_DIGITAL_FILTER_MODE_SINC3 << R_ADC_B0_ADDFSR0_DFSEL0_Pos))
                  | (R_ADC_B0_ADDFSR0_DFSEL1_Msk & (ADC_B_DIGITAL_FILTER_MODE_SINC3 << R_ADC_B0_ADDFSR0_DFSEL1_Pos))
                  | (R_ADC_B0_ADDFSR0_DFSEL2_Msk & (ADC_B_DIGITAL_FILTER_MODE_SINC3 << R_ADC_B0_ADDFSR0_DFSEL2_Pos))
                  | (R_ADC_B0_ADDFSR0_DFSEL3_Msk & (ADC_B_DIGITAL_FILTER_MODE_SINC3 << R_ADC_B0_ADDFSR0_DFSEL3_Pos))),
          .adc_filter_selection[1].bits = ((R_ADC_B0_ADDFSR1_DFSEL0_Msk
                  & (ADC_B_DIGITAL_FILTER_MODE_SINC3 << R_ADC_B0_ADDFSR1_DFSEL0_Pos))
                  | (R_ADC_B0_ADDFSR1_DFSEL1_Msk & (ADC_B_DIGITAL_FILTER_MODE_SINC3 << R_ADC_B0_ADDFSR1_DFSEL1_Pos))
                  | (R_ADC_B0_ADDFSR1_DFSEL2_Msk & (ADC_B_DIGITAL_FILTER_MODE_SINC3 << R_ADC_B0_ADDFSR1_DFSEL2_Pos))
                  | (R_ADC_B0_ADDFSR1_DFSEL3_Msk & (ADC_B_DIGITAL_FILTER_MODE_SINC3 << R_ADC_B0_ADDFSR1_DFSEL3_Pos))), };

const adc_cfg_t g_adc0_cfg =
{ .unit = 0xFFFC, .mode = (adc_mode_t) 0, // Unused
  .resolution = (adc_resolution_t) 0, // Unused
  .alignment = ADC_ALIGNMENT_RIGHT,
  .trigger = (adc_trigger_t) 0, // Unused
  .p_callback = NULL,
#if defined(NULL)
    .p_context           = NULL,
#else
  .p_context = (void*) &NULL,
#endif
  .p_extend = &g_adc0_cfg_extend,

  .scan_end_irq = FSP_INVALID_VECTOR,
  .scan_end_ipl = BSP_IRQ_DISABLED,
  .scan_end_b_irq = FSP_INVALID_VECTOR,
  .scan_end_b_ipl = BSP_IRQ_DISABLED, };

adc_b_instance_ctrl_t g_adc0_ctrl;

const adc_instance_t g_adc0 =
{ .p_ctrl = &g_adc0_ctrl, .p_cfg = &g_adc0_cfg, .p_channel_cfg = &g_adc0_scan_cfg, .p_api = &g_adc_on_adc_b, };

#undef RA_NOT_DEFINED
sci_b_uart_instance_ctrl_t g_uart8_ctrl;

sci_b_baud_setting_t g_uart8_baud_setting =
        {
        /* Baud rate calculated with 0.160% error. */.baudrate_bits_b.abcse = 0,
          .baudrate_bits_b.abcs = 0, .baudrate_bits_b.bgdm = 1, .baudrate_bits_b.cks = 0, .baudrate_bits_b.brr = 64, .baudrate_bits_b.mddr =
                  (uint8_t) 256,
          .baudrate_bits_b.brme = false };

/** UART extended configuration for UARTonSCI HAL driver */
const sci_b_uart_extended_cfg_t g_uart8_cfg_extend =
{ .clock = SCI_B_UART_CLOCK_INT, .rx_edge_start = SCI_B_UART_START_BIT_FALLING_EDGE, .noise_cancel =
          SCI_B_UART_NOISE_CANCELLATION_DISABLE,
  .rx_fifo_trigger = SCI_B_UART_RX_FIFO_TRIGGER_MAX, .p_baud_setting = &g_uart8_baud_setting, .flow_control =
          SCI_B_UART_FLOW_CONTROL_RTS,
#if 0xFF != 0xFF
                .flow_control_pin       = BSP_IO_PORT_FF_PIN_0xFF,
                #else
  .flow_control_pin = (bsp_io_port_pin_t) UINT16_MAX,
#endif
  .rs485_setting =
  { .enable = SCI_B_UART_RS485_DISABLE,
    .polarity = SCI_B_UART_RS485_DE_POLARITY_HIGH,
    .assertion_time = 1,
    .negation_time = 1, } };

/** UART interface configuration */
const uart_cfg_t g_uart8_cfg =
{ .channel = 8, .data_bits = UART_DATA_BITS_8, .parity = UART_PARITY_OFF, .stop_bits = UART_STOP_BITS_1, .p_callback =
          user_uart8_callback,
  .p_context = NULL, .p_extend = &g_uart8_cfg_extend,
#define RA_NOT_DEFINED (1)
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
  .p_transfer_tx = NULL,
#else
                .p_transfer_tx       = &RA_NOT_DEFINED,
#endif
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
  .p_transfer_rx = NULL,
#else
                .p_transfer_rx       = &RA_NOT_DEFINED,
#endif
#undef RA_NOT_DEFINED
  .rxi_ipl = (12),
  .txi_ipl = (12), .tei_ipl = (12), .eri_ipl = (12),
#if defined(VECTOR_NUMBER_SCI8_RXI)
                .rxi_irq             = VECTOR_NUMBER_SCI8_RXI,
#else
  .rxi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI8_TXI)
                .txi_irq             = VECTOR_NUMBER_SCI8_TXI,
#else
  .txi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI8_TEI)
                .tei_irq             = VECTOR_NUMBER_SCI8_TEI,
#else
  .tei_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI8_ERI)
                .eri_irq             = VECTOR_NUMBER_SCI8_ERI,
#else
  .eri_irq = FSP_INVALID_VECTOR,
#endif
        };

/* Instance structure to use this module. */
const uart_instance_t g_uart8 =
{ .p_ctrl = &g_uart8_ctrl, .p_cfg = &g_uart8_cfg, .p_api = &g_uart_on_sci_b };
void g_hal_init(void)
{
    g_common_init ();
}
