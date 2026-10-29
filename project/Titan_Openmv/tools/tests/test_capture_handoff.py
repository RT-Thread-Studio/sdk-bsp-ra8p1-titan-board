"""Exercise the production framebuffer/CSI handoff with real OpenMV queues.

VIN callbacks are injected during task-side cache maintenance. Queue publication
is checked for the short IRQ exclusion used by the production handoff.
This host test does not emulate the HyperRAM controller or the Cortex-M85 cache.
"""
import os
from pathlib import Path
import re
import shutil
import subprocess
import sys
import tempfile

ROOT = Path(__file__).resolve().parents[2]
sys.path.insert(0, str(ROOT))
from tools.host_tools import configure_host_tools


def extract(source, name):
    match = re.search(r'^.*\b' + re.escape(name) + r'\([^;]*?\)\s*\{', source, re.M)
    if not match:
        raise ValueError('Function not found: ' + name)
    start = match.start()
    end = match.end()
    depth = 1
    while depth:
        depth += (source[end] == '{') - (source[end] == '}')
        end += 1
    return source[start:end]


def main():
    fb = (ROOT / 'platform/port/titan_framebuffer_core.c').read_text()
    csi = (ROOT / 'platform/openmv/ra8_csi.c').read_text()
    queue = (ROOT / 'ThirdParty/openmv/common/queue.c').read_text()
    # Hook the real queue just before publishing an item; no queue algorithm
    # is replaced by the test. Initial pool setup is outside the running phase.
    queue = queue.replace('q->items[old_tail] = item;',
                          'handoff_publication(q);\n    q->items[old_tail] = item;')
    functions = '\n'.join(extract(fb, name) for name in (
        'framebuffer_pool_start', 'framebuffer_acquire', 'framebuffer_release'))
    functions += '\n' + '\n'.join(extract(csi, name) for name in (
        'direct_valid', 'direct_take_buffer', 'direct_complete_buffer', 'direct_claim'))
    harness = r'''
#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "queue.h"
#define OMV_ALIGN_TO(n, a) (((n) + (a) - 1) & ~((a) - 1))
#define FRAMEBUFFER_ALIGNMENT 32
#define FB_MAINFB_ID 0
#define PIXFORMAT_INVALID 0
#define __DCACHE_PRESENT 1
enum { FB_FLAG_USED=2, FB_FLAG_FREE=4, FB_FLAG_PEEK=8,
       FB_FLAG_CHECK_LAST=64, FB_FLAG_INVALIDATE=128, VB_FLAG_USED=2 };
typedef struct {
    int32_t offset;
    uint32_t flags;
    _Alignas(32) uint8_t data[];
} vbuffer_t;
typedef struct {
    char *raw_base;
    size_t buf_size, buf_count;
    queue_t *free_queue, *used_queue;
    unsigned pixfmt;
} framebuffer_t;
typedef int rt_base_t;
static framebuffer_t fb;
static vbuffer_t *volatile direct_ready;
static volatile bool direct_fault;
static int lock_depth;
static bool running, in_irq;
static unsigned transfers, publications, cache_injections;
static uint8_t *banks[3];
static framebuffer_t *framebuffer_get(unsigned id) { assert(id == 0); return &fb; }
static rt_base_t rt_hw_interrupt_disable(void) { return lock_depth++; }
static void rt_hw_interrupt_enable(rt_base_t level) {
    assert(lock_depth == level + 1);
    lock_depth = level;
}
static void framebuffer_reset(vbuffer_t *buffer) { memset(buffer, 0, sizeof(*buffer)); }
static bool titan_memory_is_bus_accessible(const void *data, size_t size) {
    return data && size == fb.buf_size;
}
static void frame(void);
static void SCB_InvalidateDCache_by_Addr(void *data, size_t size) {
    assert(!lock_depth && size == fb.buf_size);
    for (unsigned i = 0; i < 3; i++) { assert(data != banks[i]); }
    // The released buffer is CPU-owned until cache maintenance finishes.
    if (running) { frame(); frame(); cache_injections += 2; }
}
static void handoff_publication(queue_t *q) {
    if (running && q == fb.free_queue) {
        assert(!in_irq && lock_depth);
        publications++;
    }
}
/* QUEUE */
/* FUNCTIONS */

static void frame(void) {
    assert(!lock_depth && !in_irq);
    in_irq = true;
    rt_base_t level = rt_hw_interrupt_disable();
    unsigned bank = transfers % 3;
    uint8_t *replacement = direct_take_buffer(&fb);
    assert(replacement && !direct_fault);
    for (unsigned i = 0; i < 3; i++) { assert(replacement != banks[i]); }
    uint8_t *completed = banks[bank];
    banks[bank] = replacement;
    direct_complete_buffer(&fb, completed, ++transfers);
    assert(!direct_fault);
    rt_hw_interrupt_enable(level);
    in_irq = false;
}
static unsigned owner_index(uint8_t *data) {
    uintptr_t first = (uintptr_t)framebuffer_pool_start(&fb, fb.buf_count) + sizeof(vbuffer_t);
    uintptr_t offset = (uintptr_t)data - first;
    assert(offset % 128 == 0 && offset / 128 < fb.buf_count);
    return offset / 128;
}
static void check_owners(void) {
    unsigned owners[5] = {0};
    for (unsigned i = 0; i < 3; i++) { owners[owner_index(banks[i])]++; }
    if (direct_ready) { owners[owner_index(direct_ready->data)]++; }
    queue_t *queues[] = {fb.free_queue, fb.used_queue};
    for (unsigned n = 0; n < 2; n++) {
        queue_t *q = queues[n];
        size_t head = atomic_load(&q->head), tail = atomic_load(&q->tail);
        while (head != tail) {
            owners[owner_index(((vbuffer_t *)q->items[head])->data)]++;
            head = (head + 1) % (q->capacity + 1);
        }
    }
    for (unsigned i = 0; i < 5; i++) { assert(owners[i] == 1); }
}
int main(void) {
    _Alignas(128) static char storage[2048];
    fb = (framebuffer_t){.raw_base=storage, .buf_size=128-sizeof(vbuffer_t), .buf_count=5};
    size_t qsize = queue_calc_size(fb.buf_count);
    queue_init(&fb.free_queue, fb.buf_count, storage);
    queue_init(&fb.used_queue, fb.buf_count, storage + qsize);
    for (unsigned i = 0; i < 5; i++) {
        vbuffer_t *buffer = (vbuffer_t *)(framebuffer_pool_start(&fb, 5) + i * 128);
        framebuffer_reset(buffer);
        assert(queue_push(fb.free_queue, buffer));
    }
    for (unsigned i = 0; i < 3; i++) { banks[i] = direct_take_buffer(&fb); assert(banks[i]); }
    running = true;
    for (unsigned i = 0; i < 100000; i++) {
        if (!queue_is_empty(fb.used_queue)) {
            assert(framebuffer_release(&fb, FB_FLAG_USED | FB_FLAG_INVALIDATE));
        }
        frame();
        // Slow Python processing must only recycle the private ready slot.
        if (i % 7 == 0) { frame(); frame(); frame(); }
        assert(direct_claim(&fb));
        assert(!direct_fault && !lock_depth && !in_irq);
        check_owners();
    }
    assert(publications == 99999 && cache_injections == 199998);
    printf("PASS: 100000 production handoffs, %u DMA swaps, %u cache-window IRQs; no duplicate/lost owners\n",
           transfers, cache_injections);
    return 0;
}
'''
    harness = harness.replace('/* QUEUE */', queue).replace('/* FUNCTIONS */', functions)
    env = os.environ.copy()
    configure_host_tools(env, str(ROOT))
    compiler = env.get('MPY_HOST_CC') or shutil.which('gcc', path=env['PATH'])
    if not compiler:
        raise RuntimeError('A native GCC is required')
    with tempfile.TemporaryDirectory(prefix='titan-handoff-') as directory:
        source = Path(directory) / 'handoff.c'
        executable = Path(directory) / ('handoff.exe' if os.name == 'nt' else 'handoff')
        source.write_text(harness)
        subprocess.run([compiler, '-std=c11', '-O2', '-Wall', '-Wextra', '-Werror',
                        '-I' + str(ROOT/'ThirdParty/openmv/common'), str(source), '-o', str(executable)],
                       check=True, env=env)
        subprocess.run([str(executable)], check=True, env=env)


if __name__ == '__main__':
    main()
