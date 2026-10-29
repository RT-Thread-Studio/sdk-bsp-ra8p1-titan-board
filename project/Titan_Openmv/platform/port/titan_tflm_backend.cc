// Project-maintained OpenMV TFLM adapter; upstream license follows.
/*
 * Copyright (C) 2023-2024 OpenMV, LLC.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Any redistribution, use, or modification in source or binary form
 *    is done solely for personal benefit and not for any commercial
 *    purpose or for monetary gain. For commercial licensing options,
 *    please contact openmv@openmv.io
 *
 * THIS SOFTWARE IS PROVIDED BY THE LICENSOR AND COPYRIGHT OWNER "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE LICENSOR OR COPYRIGHT
 * OWNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
 * OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * TensorFlow Lite Micro ML backend.
 */
#if MICROPY_PY_ML_TFLM
#include <string.h>
#include <stdint.h>

#include "imlib_config.h"
#ifdef BSP_USING_OPENMV_NPU
#include "titan_npu.h"
#include "titan_memory.h"
#endif
#include "omv_common.h"

#include "tensorflow/lite/micro/micro_op_resolver.h"
#include "tensorflow/lite/micro/micro_mutable_op_resolver.h"
#include "tensorflow/lite/micro/cortex_m_generic/debug_log_callback.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/schema/schema_utils.h"
#include "../npu/titan_tflm_shared.inc"

#ifndef NDEBUG
void operator delete(void *) {

}

void operator delete(void *, unsigned int) {

}
#endif

extern "C" {
#include "py/runtime.h"
#include "py/obj.h"
#include "py/objlist.h"
#include "py/objtuple.h"
#include "py/binary.h"
#include "py/gc.h"
#include "py_ml.h"
#include "py/nlr.h"
#include "titan_ml_memory.h"
#include "titan_ml_workspace.h"
#include "common/omv_profiler.h"

using namespace tflite;
#define TF_ARENA_EXTRA      (512)
#define TF_ARENA_ALIGN      (32)
typedef MicroMutableOpResolver<49> MicroOpsResolver;

typedef struct ml_backend_state {
    titan_ml_workspace_t *workspace; // Strong GC owner of shared activation storage.
    uint8_t *persistent_storage; // Private aligned tail backing allocation.
    MicroAllocator *shared_allocator;
    uint8_t *arena_storage; // GC root for the aligned arena allocation.
    uint8_t *arena;
    const Model *model;
    MicroOpsResolver *resolver;
    MicroInterpreter *interpreter;
} ml_backend_state_t;

typedef struct {
    nlr_jump_callback_node_t node;
    ml_backend_state_t *state;
    py_ml_model_obj_t *model;
} titan_shared_init_guard_t;

static void titan_shared_init_cleanup(void *arg) {
    titan_shared_init_guard_t *guard = (titan_shared_init_guard_t *)arg;
    ml_backend_state_t *state = guard->state;
    if (guard->model->state == state) {
        guard->model->state = NULL;
        guard->model->memory_addr = 0;
        guard->model->memory_size = 0;
    }
    if (state->interpreter) state->interpreter->~MicroInterpreter();
    if (state->resolver) state->resolver->~MicroOpsResolver();
    m_free(state->interpreter);
    m_free(state->resolver);
    m_free(state->persistent_storage);
    // Shared arena_storage stays NULL. The workspace belongs to its GC owner.
    m_free(state->arena_storage);
    m_free(state);
}

void abort(void) {
    while (1) {
        ;
    }
}

void ml_backend_log_handler(const char *s) {
    if (strcmp(s, "\r\n")) {
        mp_printf(MP_PYTHON_PRINTER, "tflm_backend: %s\n", s);
    }
}

static bool ml_backend_valid_dataype(TfLiteType type) {
    return (type == kTfLiteUInt8 ||
            type == kTfLiteInt8 ||
            type == kTfLiteUInt16 ||
            type == kTfLiteInt16 ||
            type == kTfLiteFloat32);
}

static char ml_backend_map_dtype(TfLiteType type) {
    if (type == kTfLiteUInt8) {
        return 'B';
    } else if (type == kTfLiteInt8) {
        return 'b';
    } else if (type == kTfLiteUInt16) {
        return 'H';
    } else if (type == kTfLiteInt16) {
        return 'h';
    } else {
        return 'f';
    }
}

static void ml_backend_init_ops_resolver(MicroOpsResolver *resolver) {
    if (resolver->AddAdd() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddConv2D() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddDepthwiseConv2D() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddFullyConnected() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddReshape() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddSoftmax() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddQuantize() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddDequantize() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddAveragePool2D() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddMaxPool2D() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddMul() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddLogistic() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddConcatenation() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddPad() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddResizeNearestNeighbor() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddStridedSlice() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddTranspose() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddSub() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddMaximum() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddPack() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddResizeBilinear() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddMean() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddPrelu() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddShape() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddMinimum() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddRelu() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddRelu6() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddLeakyRelu() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddHardSwish() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddTanh() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddPadV2() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddSqueeze() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddExpandDims() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddSlice() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddSplit() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddUnpack() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddSplitV() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddGather() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddGatherNd() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddCast() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddDiv() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddReduceMax() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddSum() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddAbs() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddNeg() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddSquare() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddExp() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
    if (resolver->AddLog() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM resolver full"));
    }
#ifdef BSP_USING_OPENMV_NPU
    if (resolver->AddEthosU() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Ethos-U resolver unavailable"));
    }
#endif
}

#include "../npu/titan_tflm_model_check.inc"

static int titan_ml_backend_init_internal(py_ml_model_obj_t *model, titan_ml_workspace_t *workspace) {
    if (workspace) {
        uintptr_t start = (uintptr_t)workspace->data;
        if (!workspace->storage || !workspace->data || !workspace->busy || (start & 31U) ||
            start < 0x22000000UL || start > 0x221d4000UL ||
            workspace->capacity > 0x221d4000UL - start) {
            mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Invalid or unclaimed SRAM workspace"));
        }
    }
#ifdef BSP_USING_OPENMV_NPU
    if (titan_npu_init() != 0) {
        mp_raise_msg(&mp_type_OSError, MP_ERROR_TEXT("Ethos-U55 initialization failed"));
    }
#endif
    RegisterDebugLogCallback(ml_backend_log_handler);

    // Parse the model's data.
    const Model *tflite_model = GetModel(model->data);
    if (tflite_model->version() != TFLITE_SCHEMA_VERSION) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Unsupported model schema"));
    }

    // Initialize a temporary op resolver.
    MicroOpsResolver resolver;
    ml_backend_init_ops_resolver(&resolver);
    titan_tflm_check_model_ops(tflite_model, resolver);
    if (workspace) {
        const char *invalid = titan_shared::Validate(tflite_model);
        if (invalid) {
            mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("Shared model: %s"), invalid);
        }
    }

    gc_info_t info;
    gc_info(&info);
    // Allocate a temporary interpreter to get the optimal arena size.
    size_t arena_size = info.max_free * MICROPY_BYTES_PER_GC_BLOCK;
    if (arena_size <= TF_ARENA_ALIGN) {
        mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("No TFLM arena"));
    }
    arena_size -= TF_ARENA_ALIGN;
    uint8_t *arena_storage;
    if (workspace) {
        arena_storage = (uint8_t *)titan_ml_hyperram_alloc(arena_size + TF_ARENA_ALIGN);
        if (!arena_storage) {
            mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("No HyperRAM for shared model probe"));
        }
        memset(arena_storage, 0, arena_size + TF_ARENA_ALIGN);
    } else {
        arena_storage = m_new0(uint8_t, arena_size + TF_ARENA_ALIGN);
    }
    uint8_t *arena_memory = (uint8_t *)OMV_ALIGN_TO((uintptr_t)arena_storage, TF_ARENA_ALIGN);
    const size_t probe_bytes = arena_size;
    TfLiteStatus probe_status;
    titan_shared::Measurement shared_measure = {};
    if (workspace) {
        probe_status = titan_shared::Measure(tflite_model, resolver, arena_memory, arena_size, &shared_measure);
    } else {
        MicroInterpreter interpreter(tflite_model, resolver, arena_memory, arena_size);
        probe_status = interpreter.AllocateTensors();
        if (probe_status == kTfLiteOk) {
            arena_size = OMV_ALIGN_TO(interpreter.arena_used_bytes(), TF_ARENA_ALIGN) + TF_ARENA_EXTRA;
        }
    } // Destroy the probe before returning its backing storage.
    m_free(arena_storage);
    if (probe_status != kTfLiteOk) {
        mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM probe allocation failed (arena=%u bytes; see TFLM log)"), (unsigned)probe_bytes);
    }

    size_t persistent_bytes = workspace ? titan_shared::TailCapacity(shared_measure) : 0;
    if (workspace && (!persistent_bytes || shared_measure.head > workspace->capacity ||
                      shared_measure.head > SIZE_MAX - persistent_bytes)) {
        mp_raise_msg_varg(&mp_type_MemoryError,
            MP_ERROR_TEXT("Shared activation needs %u bytes (workspace=%u)"),
            (unsigned)shared_measure.head, (unsigned)workspace->capacity);
    }
    if (workspace) arena_size = shared_measure.head + persistent_bytes;

    // Allocate the persistent model state and interpreter.
    ml_backend_state_t *state = m_new0(ml_backend_state_t, 1);
    titan_shared_init_guard_t guard = {{}, state, model};
    if (workspace) nlr_push_jump_callback(&guard.node, titan_shared_init_cleanup);
    state->workspace = workspace;
    state->model = GetModel(model->data);
    if (workspace) {
        state->persistent_storage = (uint8_t *)titan_ml_hyperram_alloc(persistent_bytes + TF_ARENA_ALIGN);
        if (!state->persistent_storage) {
            mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("No HyperRAM for shared model state"));
        }
        memset(state->persistent_storage, 0, persistent_bytes + TF_ARENA_ALIGN);
        uint8_t *persistent = (uint8_t *)OMV_ALIGN_TO((uintptr_t)state->persistent_storage, TF_ARENA_ALIGN);
        state->shared_allocator = titan_shared::Create(shared_measure, persistent, persistent_bytes,
                                                      workspace->data, workspace->capacity);
        if (!state->shared_allocator) {
            mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("Shared allocator capacity or alignment"));
        }
        state->arena = workspace->data;
    } else {
        state->arena_storage = (uint8_t *)titan_ml_arena_alloc(arena_size + TF_ARENA_ALIGN);
        if (!state->arena_storage) {
            m_free(state);
            mp_raise_msg(&mp_type_MemoryError, MP_ERROR_TEXT("No memory for model arena"));
        }
        memset(state->arena_storage, 0, arena_size + TF_ARENA_ALIGN);
        state->arena = (uint8_t *)OMV_ALIGN_TO((uintptr_t)state->arena_storage, TF_ARENA_ALIGN);
    }
#ifdef BSP_USING_OPENMV_NPU
    if (!titan_memory_is_bus_accessible(state->arena, workspace ? workspace->capacity : arena_size)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("NPU arena is not bus accessible"));
    }
#endif
    state->resolver = new(m_new0(MicroOpsResolver, 1)) MicroOpsResolver();
    ml_backend_init_ops_resolver(state->resolver);
    if (workspace) {
        state->interpreter = new(m_new0(MicroInterpreter, 1)) MicroInterpreter(state->model,
                                                                               *state->resolver,
                                                                               state->shared_allocator);
    } else {
        state->interpreter = new(m_new0(MicroInterpreter, 1)) MicroInterpreter(state->model,
                                                                               *state->resolver,
                                                                               state->arena,
                                                                               arena_size);
    }
    if (state->interpreter->AllocateTensors() != kTfLiteOk) {
        if (workspace) {
            mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Shared model AllocateTensors failed"));
        }
        state->interpreter->~MicroInterpreter();
        state->resolver->~MicroOpsResolver();
        m_free(state->interpreter);
        m_free(state->resolver);
        m_free(state->arena_storage);
        m_free(state);
        mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM persistent allocation failed (arena=%u bytes; see TFLM log)"), (unsigned)arena_size);
    }

    if (workspace && !titan_shared::ValidateIO(*state->interpreter, workspace->data, workspace->capacity)) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Shared IO is outside activation storage"));
    }
    // Initialize the model's state.
    model->state = state;
    model->memory_addr = (uint32_t) state->arena;
    model->memory_size = arena_size;

    // Initialize the model's inputs.
    model->inputs_size = state->interpreter->inputs_size();
    model->input_shape = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->inputs_size, NULL));
    model->input_scale = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->inputs_size, NULL));
    model->input_zero_point = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->inputs_size, NULL));
    model->input_dtype = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->inputs_size, NULL));

    for (size_t i = 0; i < model->inputs_size; i++) {
        TfLiteTensor *input = state->interpreter->input(i);

        // Check input data type.
        if (!ml_backend_valid_dataype(input->type)) {
            mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("Unsupported input data type %d"), input->type);
        }

        mp_obj_tuple_t *o = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(input->dims->size, NULL));
        for (int j = 0; j < input->dims->size; j++) {
            o->items[j] = mp_obj_new_int(input->dims->data[j]);
        }

        float input_scale = input->params.scale;
        model->input_shape->items[i] = MP_OBJ_FROM_PTR(o);
        model->input_scale->items[i] = mp_obj_new_float((input_scale == 0.0f) ? 1.0f : input_scale);
        model->input_zero_point->items[i] = mp_obj_new_int(input->params.zero_point);
        model->input_dtype->items[i] = mp_obj_new_int(ml_backend_map_dtype(input->type));
    }

    // Initialize the model's outputs.
    model->outputs_size = state->interpreter->outputs_size();
    model->output_shape = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->outputs_size, NULL));
    model->output_scale = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->outputs_size, NULL));
    model->output_zero_point = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->outputs_size, NULL));
    model->output_dtype = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(model->outputs_size, NULL));

    for (size_t i = 0; i < model->outputs_size; i++) {
        TfLiteTensor *output = state->interpreter->output(i);

        // Check output data type.
        if (!ml_backend_valid_dataype(output->type)) {
            mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("Unsupported output data type %d"), output->type);
        }

        mp_obj_tuple_t *o = (mp_obj_tuple_t *) MP_OBJ_TO_PTR(mp_obj_new_tuple(output->dims->size, NULL));
        for (int j = 0; j < output->dims->size; j++) {
            o->items[j] = mp_obj_new_int(output->dims->data[j]);
        }

        float output_scale = output->params.scale;
        model->output_shape->items[i] = MP_OBJ_FROM_PTR(o);
        model->output_scale->items[i] = mp_obj_new_float((output_scale == 0.0f) ? 1.0f : output_scale);
        model->output_zero_point->items[i] = mp_obj_new_int(output->params.zero_point);
        model->output_dtype->items[i] = mp_obj_new_int(ml_backend_map_dtype(output->type));
    }

    if (workspace) nlr_pop_jump_callback(false);
    return 0;
}

int ml_backend_init_model(py_ml_model_obj_t *model) {
    return titan_ml_backend_init_internal(model, NULL);
}

int titan_ml_backend_init_shared(py_ml_model_obj_t *model, titan_ml_workspace_t *workspace) {
    if (!workspace) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Shared workspace is required"));
    }
    return titan_ml_backend_init_internal(model, workspace);
}

titan_ml_workspace_t *titan_ml_model_workspace(py_ml_model_obj_t *model) {
    ml_backend_state_t *state = model ? (ml_backend_state_t *)model->state : NULL;
    return state ? state->workspace : NULL;
}

int ml_backend_run_inference(py_ml_model_obj_t *model) {
    OMV_PROFILER_ENTER(ml_backend_run_inference);

    RegisterDebugLogCallback(ml_backend_log_handler);
    ml_backend_state_t *state = (ml_backend_state_t *) model->state;

    if (state->interpreter->Invoke() != kTfLiteOk) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Invoke failed"));
    }

    OMV_PROFILER_EXIT(ml_backend_run_inference);
    return 0;
}

void *ml_backend_get_input(py_ml_model_obj_t *model, size_t index) {
    ml_backend_state_t *state = (ml_backend_state_t *) model->state;
    if (index < state->interpreter->inputs_size()) {
        return state->interpreter->input(index)->data.data;
    }
    mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Invalid input tensor index"));
}

void *ml_backend_get_output(py_ml_model_obj_t *model, size_t index) {
    ml_backend_state_t *state = (ml_backend_state_t *) model->state;
    if (index < state->interpreter->outputs_size()) {
        return state->interpreter->output(index)->data.data;
    }
    mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("Invalid output tensor index"));
}
} // extern "C"
#endif // MICROPY_PY_ML_TFLM
