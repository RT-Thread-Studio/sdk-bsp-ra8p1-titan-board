/* SPDX-License-Identifier: Apache-2.0 */
#include <string.h>
#include "py/runtime.h"
#include "py/gc.h"
#include "titan_ml_workspace.h"

#if MICROPY_PY_THREAD || !MICROPY_ENABLE_FINALISER
#error "Workspace requires VM-only ownership and GC finalizers"
#endif

#define TITAN_WORKSPACE_ALIGNMENT (32U)
#define TITAN_WORKSPACE_SRAM_START (0x22000000UL)
#define TITAN_WORKSPACE_SRAM_BYTES (0x001d4000UL)
#define TITAN_WORKSPACE_AUTO_BYTES (1184U * 1024U)

/* Deliberately not a GC root. Attached backend state holds the Workspace.
 * The native finalizer clears this cache before gc.c frees any dead blocks. */
static titan_ml_workspace_t *auto_workspace;

static bool workspace_is_sram(uintptr_t address, size_t size)
{
    return address >= TITAN_WORKSPACE_SRAM_START &&
           size <= TITAN_WORKSPACE_SRAM_BYTES &&
           address - TITAN_WORKSPACE_SRAM_START <= TITAN_WORKSPACE_SRAM_BYTES - size;
}

/* 0=success, 1=allocation failed, 2=not SRAM. self must already be a live,
 * initialized GC object before m_malloc_maybe can trigger a collection. */
static int workspace_allocate_storage(titan_ml_workspace_t *self, size_t capacity)
{
    self->storage = NULL;
    self->data = NULL;
    self->capacity = capacity;
    self->busy = false;
    self->storage = m_malloc_maybe(capacity + TITAN_WORKSPACE_ALIGNMENT);
    if (!self->storage) { return 1; }
    uintptr_t aligned = ((uintptr_t)self->storage + TITAN_WORKSPACE_ALIGNMENT - 1U) &
                        ~(uintptr_t)(TITAN_WORKSPACE_ALIGNMENT - 1U);
    if (!workspace_is_sram(aligned, capacity)) {
        m_free(self->storage);
        self->storage = NULL;
        return 2;
    }
    self->data = (uint8_t *)aligned;
    memset(self->storage, 0, capacity + TITAN_WORKSPACE_ALIGNMENT);
    return 0;
}

static mp_obj_t workspace_make_new(const mp_obj_type_t *type, size_t n_args,
                                   size_t n_kw, const mp_obj_t *args)
{
    mp_arg_check_num(n_args, n_kw, 1, 1, false);
    mp_int_t requested = mp_obj_get_int(args[0]);
    if (requested <= 0) {
        mp_raise_ValueError(MP_ERROR_TEXT("Workspace capacity must be positive"));
    }
    if ((mp_uint_t)requested > TITAN_WORKSPACE_SRAM_BYTES) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Workspace does not fit SRAM"));
    }
    size_t capacity = ((size_t)requested + TITAN_WORKSPACE_ALIGNMENT - 1U) &
                      ~(size_t)(TITAN_WORKSPACE_ALIGNMENT - 1U);
    titan_ml_workspace_t *self = mp_obj_malloc_with_finaliser(titan_ml_workspace_t, type);
    /* Preserve normal GC threshold and gc.disable behavior. */
    int result = workspace_allocate_storage(self, capacity);
    if (result == 1) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("No contiguous SRAM for Workspace"));
    }
    if (result == 2) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Workspace allocation fell outside SRAM"));
    }
    return MP_OBJ_FROM_PTR(self);
}

static mp_obj_t workspace_del(mp_obj_t self_in)
{
    if (auto_workspace == MP_OBJ_TO_PTR(self_in)) {
        auto_workspace = NULL;
    }
    /* GC owns both object and raw storage. No free, allocation or callback. */
    return mp_const_none;
}
static MP_DEFINE_CONST_FUN_OBJ_1(workspace_del_obj, workspace_del);
static const mp_rom_map_elem_t workspace_locals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___del__), MP_ROM_PTR(&workspace_del_obj) },
};
static MP_DEFINE_CONST_DICT(workspace_locals_dict, workspace_locals_table);

static void workspace_print(const mp_print_t *print, mp_obj_t self_in,
                            mp_print_kind_t kind)
{
    (void)kind;
    const titan_ml_workspace_t *self = MP_OBJ_TO_PTR(self_in);
    mp_printf(print, "<Workspace capacity=%u address=%p busy=%u>",
              (unsigned)self->capacity, self->data, (unsigned)self->busy);
}

titan_ml_workspace_t *titan_ml_workspace_get(mp_obj_t object)
{
    if (object == mp_const_none) { return NULL; }
    if (!mp_obj_is_type(object, &titan_ml_workspace_type)) {
        mp_raise_TypeError(MP_ERROR_TEXT("workspace must be Workspace or None"));
    }
    return MP_OBJ_TO_PTR(object);
}

static titan_ml_workspace_t *workspace_auto_create(void)
{
    /* A maybe allocator avoids swallowing unrelated NLR exceptions. Set the
     * finalizer flag at allocation, initialize all fields before another GC,
     * and publish the weak pointer only after storage allocation succeeds. */
    titan_ml_workspace_t *self = gc_alloc(sizeof(*self), GC_ALLOC_FLAG_HAS_FINALISER);
    if (!self) { return NULL; }
    memset(self, 0, sizeof(*self));
    self->base.type = &titan_ml_workspace_type;
    if (workspace_allocate_storage(self, TITAN_WORKSPACE_AUTO_BYTES) != 0) {
        m_free(self);
        return NULL;
    }
    auto_workspace = self;
    return self;
}

titan_ml_workspace_t *titan_ml_workspace_auto_get(void)
{
    /* Never consult an unrooted cache from GC/finalizer context. No Python
     * threads are enabled; caller must immediately install/acquire its guard. */
    if (gc_is_locked()) { return NULL; }
    titan_ml_workspace_t *self = auto_workspace;
    if (self) { return self->busy ? NULL : self; }

    self = workspace_auto_create();
    if (!self && MP_STATE_MEM(gc_auto_collect_enabled) && !gc_is_locked()) {
        /* GC normally spills to HyperRAM before reclaiming dead SRAM objects.
         * One retry recovers temporary anchor-construction allocations without
         * changing gc.disable(), replacing a live pool, or moving any tensors. */
        gc_collect();
        self = auto_workspace;
        if (self) { return self->busy ? NULL : self; }
        self = workspace_auto_create();
    }
    return self;
}

void titan_ml_workspace_auto_reset(void)
{
    /* Called only at a quiescent VM boundary, before gc_init. Models/GC own
     * storage, so clearing the cache must not free memory or change a lease. */
    auto_workspace = NULL;
}

int titan_ml_workspace_try_acquire(titan_ml_workspace_t *workspace)
{
    if (!workspace) { return 0; }
    if (workspace->busy) { return -1; }
    workspace->busy = true;
    return 0;
}

void titan_ml_workspace_release(titan_ml_workspace_t *workspace)
{
    if (workspace) { workspace->busy = false; }
}

MP_DEFINE_CONST_OBJ_TYPE(
    titan_ml_workspace_type,
    MP_QSTR_Workspace,
    MP_TYPE_FLAG_NONE,
    make_new, workspace_make_new,
    print, workspace_print,
    locals_dict, &workspace_locals_dict
    );
