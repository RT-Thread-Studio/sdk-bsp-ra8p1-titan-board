/* SPDX-License-Identifier: Apache-2.0 */
/* Keep the official stdio channel implementation; own its VM lifetime here. */
#include <string.h>
#include "py/runtime.h"
#include "py/gc.h"
#include "py/nlr.h"
#include "omv_protocol.h"
#include "titan_protocol.h"
#include "titan_vision.h"

#if OMV_PROTOCOL_DEFAULT_CHANNELS != 0
#error "Titan registers lifecycle-wrapped channels; disable default registration"
#endif

static omv_protocol_channel_t titan_stdin_channel;
static omv_protocol_channel_t titan_stream_channel;
static vstr_t *script_vstr;
static bool stdin_ready;
static bool exec_requested;

static void titan_stdio_clear(void)
{
    /* Stop marking before freeing, including if an outer NLR cleanup repeats. */
    vstr_t *vstr = script_vstr;
    script_vstr = NULL;
    stdin_ready = false;
    exec_requested = false;
    if (vstr) {
        if (vstr->buf) {
            vstr_clear(vstr);
        }
        memset(vstr, 0, sizeof(*vstr));
    }
}

/* Only the official stdio translation unit redirects vstr_init here.
 * Capture the real vstr pointer, without depending on its private context layout. */
void titan_stdio_vstr_init(vstr_t *vstr, size_t alloc)
{
    titan_stdio_clear();
    if (vstr->buf) {
        vstr_clear(vstr);
    }
    memset(vstr, 0, sizeof(*vstr));
    script_vstr = vstr;
    /* If allocation raises, buf remains NULL and the init guard drops the root. */
    vstr_init(vstr, alloc);
}

void titan_protocol_gc_collect(void)
{
    if (script_vstr) {
        void *buffer = script_vstr->buf;
        gc_collect_root(&buffer, 1);
    }
}

static void titan_stdio_init_failed(void *unused)
{
    (void)unused;
    titan_stdio_clear();
}

static int titan_stdin_init(const omv_protocol_channel_t *channel)
{
    stdin_ready = false;
    exec_requested = false;
    nlr_jump_callback_node_t cleanup;
    nlr_push_jump_callback(&cleanup, titan_stdio_init_failed);
    int result = omv_stdin_channel.init ? omv_stdin_channel.init(channel) : 0;
    stdin_ready = result == 0;
    nlr_pop_jump_callback(result != 0);
    return result;
}

static int titan_stdin_deinit(const omv_protocol_channel_t *channel)
{
    (void)channel;
    /* Do not call pyexec_hook(false): it would send a new protocol event. */
    titan_stdio_clear();
    return 0;
}

static bool titan_stdin_poll(const omv_protocol_channel_t *channel)
{
    return stdin_ready && omv_stdin_channel.poll && omv_stdin_channel.poll(channel);
}

static int titan_stdin_write(const omv_protocol_channel_t *channel,
                             uint32_t offset, size_t size, const void *data)
{
    exec_requested = false;
    return stdin_ready && omv_stdin_channel.write ?
           omv_stdin_channel.write(channel, offset, size, data) : -1;
}

static bool titan_stdin_exec(const omv_protocol_channel_t *channel)
{
    /* The bootstrap can leave its wait as soon as an asset loads, including
     * while the IDE is still uploading. A nonempty vstr is not an EXEC. */
    if (!stdin_ready || !exec_requested || !omv_stdin_channel.exec) { return false; }
    exec_requested = false;
    return omv_stdin_channel.exec(channel);
}

static int titan_stdin_ioctl(const omv_protocol_channel_t *channel,
                             uint32_t cmd, size_t len, void *arg)
{
    if (!stdin_ready || !omv_stdin_channel.ioctl) { return -1; }
    if (cmd == OMV_CHANNEL_IOCTL_STDIN_STOP || cmd == OMV_CHANNEL_IOCTL_STDIN_RESET) {
        exec_requested = false;
    }
    int result = omv_stdin_channel.ioctl(channel, cmd, len, arg);
    if (cmd == OMV_CHANNEL_IOCTL_STDIN_EXEC) { exec_requested = result == 0; }
    return result;
}

static void titan_channels_init_failed(void *unused)
{
    (void)unused;
    omv_protocol_deinit();
    /* An init that raised may not yet have been registered for deinit. */
    titan_stdio_clear();
}

static size_t titan_stream_size(const omv_protocol_channel_t *channel)
{
    /* Even an empty official stream calls image_size(), which is in the
     * external image. Keep the channel present, but empty until verified. */
    return titan_vision_is_ready() ? omv_stream_channel.size(channel) : 0;
}

static const void *titan_stream_readp(const omv_protocol_channel_t *channel,
                                     uint32_t offset, size_t size)
{
    if (!titan_vision_is_ready()) { return NULL; }
    size_t available = channel->size(channel);
    if (offset > available || size > available - offset) { return NULL; }
    return omv_stream_channel.readp(channel, offset, size);
}

int titan_protocol_init_default(void)
{
    titan_stdio_clear();
    nlr_jump_callback_node_t cleanup;
    nlr_push_jump_callback(&cleanup, titan_channels_init_failed);
    if (omv_protocol_init_default() != 0) {
        nlr_pop_jump_callback(true);
        return -1;
    }

    titan_stdin_channel = omv_stdin_channel;
    titan_stdin_channel.init = titan_stdin_init;
    titan_stdin_channel.deinit = titan_stdin_deinit;
    titan_stdin_channel.poll = titan_stdin_poll;
    /* Official deinit retains static channel entries. Reject late traffic
     * until the next successful init restores the tracked script buffer. */
    titan_stdin_channel.write = titan_stdin_write;
    titan_stdin_channel.exec = titan_stdin_exec;
    titan_stdin_channel.ioctl = titan_stdin_ioctl;
    titan_stream_channel = omv_stream_channel;
    titan_stream_channel.size = titan_stream_size;
    titan_stream_channel.readp = titan_stream_readp;
    const omv_protocol_channel_t *channels[] = {
        &omv_usb_channel, &titan_stdin_channel, &omv_stdout_channel, &titan_stream_channel,
#if OMV_PROFILER_ENABLE
        &omv_profile_channel,
#endif
    };
    for (size_t i = 0; i < sizeof(channels) / sizeof(channels[0]); ++i) {
        /* Success is the channel ID, not necessarily zero. */
        if (omv_protocol_register_channel(channels[i]) < 0) {
            nlr_pop_jump_callback(true);
            return -1;
        }
    }
    nlr_pop_jump_callback(false);
    return 0;
}
