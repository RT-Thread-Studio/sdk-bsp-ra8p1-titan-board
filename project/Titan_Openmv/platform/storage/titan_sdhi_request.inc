/* Project-owned request path for the SDK SDHI host adapter. */
void ra_sdhi_request(struct rt_mmcsd_host *host, struct rt_mmcsd_req *req)
{
    struct rthw_sdio *sdio = host->private_data;
    sdhi_instance_ctrl_t *ctrl = sdio->sdhi_des.instance->p_ctrl;
    struct rt_mmcsd_data *data = req->cmd ? req->cmd->data : RT_NULL;
    rt_err_t result = RT_EOK;
    rt_uint32_t size = 0;

    RTHW_SDIO_LOCK(sdio);
    if (req->cmd)
    {
        req->cmd->err = RT_EOK;
        if (!titan_sdhi_present())
        {
            result = -RT_EIO;
        }
        else if (data && (data->flags & DATA_DIR_WRITE) && titan_sdhi_write_protected())
        {
            result = -RT_EIO;
        }
        else if (data)
        {
            data->err = RT_EOK;
            size = data->blks * data->blksize;
            if (!data->buf || !data->blks || !data->blksize ||
                data->blksize > 512 || data->blks > SDIO_BUFF_SIZE / data->blksize)
            {
                result = -RT_EINVAL;
            }
            else
            {
                /* SDHI/DMAC must never target the VM's DTCM stack or partial
                 * cache lines in a Python/FatFs allocation. The complete DMA
                 * transaction owns this aligned SRAM buffer under the mutex.
                 * SCR/CMD6 transfers can be shorter than a cache line. */
                data->flags &= ~DATA_STREAM;
                if (data->flags & DATA_DIR_WRITE)
                {
                    rt_memcpy(sdio->cache_buf, data->buf, size);
                    SCB_CleanDCache_by_Addr((uint32_t *)sdio->cache_buf, SDIO_BUFF_SIZE);
                    __DSB();
                    result = transfer_write(ctrl, data->blks, data->blksize, sdio->cache_buf);
                }
                else if (data->flags & DATA_DIR_READ)
                {
                    SCB_CleanInvalidateDCache_by_Addr((uint32_t *)sdio->cache_buf, SDIO_BUFF_SIZE);
                    __DSB();
                    result = transfer_read(ctrl, data->blks, data->blksize, sdio->cache_buf);
                }
                else
                {
                    result = -RT_EINVAL;
                }
                ctrl->p_reg->SD_STOP = data->blks > 1U ? 0x100U : 0U;
                if (data->blks > 1U) { ctrl->p_reg->SD_SECCNT = data->blks; }
                ctrl->p_reg->SD_SIZE = data->blksize;
            }
        }
        if (result == RT_EOK)
        {
            result = command_send(ctrl, req->cmd);
        }
        if (result != RT_EOK)
        {
            if (req->cmd->err == RT_EOK) { req->cmd->err = result; }
            if (data) {
                data->err = req->cmd->err;
            }
            /* A failed command must not leave DMAC running while another
             * request reuses the bounce buffer. */
            ctrl->p_reg->SD_DMAEN = 0;
            sdio->sdhi_des.instance->p_cfg->p_lower_lvl_transfer->p_api->disable(
                sdio->sdhi_des.instance->p_cfg->p_lower_lvl_transfer->p_ctrl);
        }
        else if (data && (data->flags & DATA_DIR_READ))
        {
            __DSB();
            SCB_InvalidateDCache_by_Addr((uint32_t *)sdio->cache_buf, SDIO_BUFF_SIZE);
            __DSB();
            rt_memcpy(data->buf, sdio->cache_buf, size);
        }
    }
    if (req->stop && titan_sdhi_present())
    {
        command_send(ctrl, req->stop);
    }
    RTHW_SDIO_UNLOCK(sdio);
    mmcsd_req_complete(sdio->host);
}
