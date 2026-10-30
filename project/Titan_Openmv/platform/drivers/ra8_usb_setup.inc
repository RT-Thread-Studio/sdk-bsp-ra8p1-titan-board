/* SPDX-License-Identifier: Apache-2.0 */
/* Queued SETUP events own their pending count across bus resets. */
bool ra8_usb_setup_pending(void)
{
    /* Atomic volatile byte read; callable with the DCD IRQ already masked. */
    return _usbd_queued_setup != 0;
}
static bool ra8_core_ep0_completion_obsolete(uint8_t ep_addr, uint32_t length)
{
    if (!ra8_usb_setup_pending()) { return false; }
    const tusb_control_request_t *request = &_usbd_dev.ctrl_xfer.request;
    const uint8_t status_ep = (request->wLength != 0 && request->bmRequestType_bit.direction)
                           ? TU_EP0_OUT : TU_EP0_IN;
    /* A completed STATUS ACK may precede the host's next SETUP normally.
     * Deliver it (CDC applies DTR here); only DATA may prime an old stage. */
    return length != 0 || ep_addr != status_ep;
}
static void ra8_core_setup_received(void)
{
    rt_base_t level = rt_hw_interrupt_disable();
    ++_usbd_queued_setup;
    rt_hw_interrupt_enable(level);
}
static bool ra8_core_setup_take(void)
{
    rt_base_t level = rt_hw_interrupt_disable();
    uint8_t pending = _usbd_queued_setup;
    if (pending != 0) { _usbd_queued_setup = pending - 1; }
    rt_hw_interrupt_enable(level);
    return pending == 1;
}
static void ra8_core_setup_drop(void)
{
    rt_base_t level = rt_hw_interrupt_disable();
    if (_usbd_queued_setup != 0) { --_usbd_queued_setup; }
    rt_hw_interrupt_enable(level);
}
