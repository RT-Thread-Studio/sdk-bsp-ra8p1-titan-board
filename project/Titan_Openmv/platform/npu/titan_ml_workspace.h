/* SPDX-License-Identifier: Apache-2.0 */
#ifndef TITAN_ML_WORKSPACE_H
#define TITAN_ML_WORKSPACE_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include "py/obj.h"

#ifdef __cplusplus
extern "C" {
#endif

/* Immutable capacity, GC-owned raw storage. Only the VM may acquire/release.
 * The caller must release only a lease it successfully acquired. */
typedef struct titan_ml_workspace {
    mp_obj_base_t base;
    uint8_t *storage;
    uint8_t *data;
    size_t capacity;
    bool busy;
} titan_ml_workspace_t;

extern const mp_obj_type_t titan_ml_workspace_type;
titan_ml_workspace_t *titan_ml_workspace_get(mp_obj_t object);
/* Weak per-VM cache, no lease acquired. A new-pool miss retries after one GC
 * only when automatic GC is enabled. NULL means unavailable/busy/GC-locked.
 * Caller must immediately keep a stack/GC reference and acquire its guard. */
titan_ml_workspace_t *titan_ml_workspace_auto_get(void);
/* Quiescent VM boundary only; clears the weak cache without freeing storage. */
void titan_ml_workspace_auto_reset(void);
int titan_ml_workspace_try_acquire(titan_ml_workspace_t *workspace);
void titan_ml_workspace_release(titan_ml_workspace_t *workspace);

struct py_ml_model_obj;
int titan_ml_backend_init_shared(struct py_ml_model_obj *model,
                                 titan_ml_workspace_t *workspace);
titan_ml_workspace_t *titan_ml_model_workspace(struct py_ml_model_obj *model);

#ifdef __cplusplus
}
#endif
#endif
