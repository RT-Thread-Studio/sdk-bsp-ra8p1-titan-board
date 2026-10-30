/* SPDX-License-Identifier: Apache-2.0 */
#include "omv_csi.h"
#include "titan_camera.h"
int omv_csi_init_upstream(void);
const char *omv_csi_name_upstream(omv_csi_t *csi);
static int mipi_reset(omv_csi_t *csi) { (void)csi; return titan_camera_sensor_reset(); }
static int mipi_framesize(omv_csi_t *csi, omv_csi_framesize_t size) {
    if (size != OMV_CSI_FRAMESIZE_QQVGA && size != OMV_CSI_FRAMESIZE_QVGA && size != OMV_CSI_FRAMESIZE_VGA) { return -1; }
    return titan_camera_sensor_size(csi->resolution[size][0], csi->resolution[size][1]);
}
static int mipi_pixformat(omv_csi_t *csi, pixformat_t format) {
    (void)csi;
    return format == PIXFORMAT_RGB565 || format == PIXFORMAT_GRAYSCALE || format == PIXFORMAT_YUV422 ? 0 : -1;
}
static int mipi_orientation(omv_csi_t *csi, int enable) {
    /* Core setters already abort/flush capture and update their logical
     * flags. Apply those flags to completed pixels in ra8_snapshot, keeping
     * the validated MIPI color phase independent of the requested direction. */
    (void)csi; (void)enable;
    return 0;
}
static int mipi_lens_correction(omv_csi_t *csi, int enable, int radi, int coef) {
    /* OV5640 uses a lens shading table, not radius/coefficient controls.
     * Its LENC enable is 0x5000[7]; the pinned parallel driver instead uses
     * 0x5300, which belongs to the sharpening controls. */
    (void)radi; (void)coef;
    const uint16_t isp_control = 0x5000;
    int previous = csi->read_reg(csi, isp_control);
    if (previous < 0) { return -1; }
    uint8_t value = (previous & 0x7fU) | (enable ? 0x80U : 0U);
    if (value == previous) { return 0; }
    /* Changing the ISP pipeline during a frame can interrupt CSI-2 packets.
     * Drain/close VIN and CSI first; the next snapshot reopens capture. */
    int result = csi->abort(csi, true, false);
    return result ? result : csi->write_reg(csi, isp_control, value);
}
int omv_csi_init(void) {
    int result = omv_csi_init_upstream();
    if (result) { return result; }
    omv_csi_t *csi = &csi_all[0];
    if (csi->chip_id != OV5640_ID) { return OMV_CSI_ERROR_ISC_UNSUPPORTED; }
    csi->reset = mipi_reset;
    csi->set_framesize = mipi_framesize;
    csi->set_pixformat = mipi_pixformat;
    csi->set_hmirror = mipi_orientation;
    csi->set_vflip = mipi_orientation;
    csi->set_lens_correction = mipi_lens_correction;
    csi->mono_bpp = 2;
    csi->rgb_swap = false;
    csi->yuv_format = SUBFORMAT_ID_YUV422;
    /* Parallel-port readout/JPEG/AF ioctls need CSI-2 variants. */
    csi->ioctl = NULL;
    csi->set_quality = NULL;
    rt_kprintf("[titan.csi] OV5640 ready\n");
    return 0;
}
const char *omv_csi_name(omv_csi_t *csi) { return omv_csi_name_upstream(csi); }
