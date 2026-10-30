/* SPDX-License-Identifier: Apache-2.0 */
/* Pure TFLM helpers shared by the firmware backend and native allocator tests.
 * No MicroPython, USB, RTOS or hardware ownership changes are made here. */
#ifndef TITAN_TFLM_SHARED_INC
#define TITAN_TFLM_SHARED_INC
#include <cstring>
#include <cstdint>
#include <limits>
#include "tensorflow/lite/micro/micro_allocator.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/micro/arena_allocator/single_arena_buffer_allocator.h"
#include "tensorflow/lite/micro/arena_allocator/persistent_arena_buffer_allocator.h"
#include "tensorflow/lite/micro/arena_allocator/non_persistent_arena_buffer_allocator.h"
#include "tensorflow/lite/micro/memory_planner/greedy_memory_planner.h"
#include "tensorflow/lite/schema/schema_utils.h"

namespace titan_shared {
constexpr size_t kAlignment = 32;
constexpr size_t kTailMargin = 512;

struct Measurement {
    size_t head;
    size_t tail;
};

/* Expose the existing protected constructor without changing TFLM. Probe
 * allocator/planner bookkeeping stays on the C++ stack, outside its arena. */
class ProbeAllocator : public tflite::MicroAllocator {
public:
    ProbeAllocator(tflite::SingleArenaBufferAllocator *memory,
                   tflite::MicroMemoryPlanner *planner) : MicroAllocator(memory, planner) {}
    ~ProbeAllocator() {}
    TF_LITE_REMOVE_VIRTUAL_DELETE
};

inline size_t AlignUp(size_t size, size_t alignment) {
    return (size + alignment - 1) & ~(alignment - 1);
}

/* Exact allocation order used by pinned MicroAllocator::Create(persist, head).
 * Each factory object is in private persistent memory, never shared head. */
inline size_t FactoryBytes() {
    size_t size = 0;
#define TITAN_FACTORY_OBJECT(type)     static_assert(alignof(type) <= kAlignment, "Review shared allocator alignment");     size = AlignUp(size + sizeof(type), alignof(type))
    TITAN_FACTORY_OBJECT(tflite::PersistentArenaBufferAllocator);
    TITAN_FACTORY_OBJECT(tflite::NonPersistentArenaBufferAllocator);
    TITAN_FACTORY_OBJECT(tflite::GreedyMemoryPlanner);
    TITAN_FACTORY_OBJECT(tflite::MicroAllocator);
#undef TITAN_FACTORY_OBJECT
    return size;
}

inline size_t TailCapacity(const Measurement &measurement) {
    const size_t overhead = AlignUp(FactoryBytes(), kAlignment) + kTailMargin;
    if (measurement.tail > std::numeric_limits<size_t>::max() - overhead - kAlignment) return 0;
    return AlignUp(measurement.tail, kAlignment) + overhead;
}

inline TfLiteStatus Measure(const tflite::Model *model,
                            const tflite::MicroOpResolver &resolver,
                            uint8_t *arena, size_t capacity, Measurement *result) {
    if (!model || !arena || !result || (reinterpret_cast<uintptr_t>(arena) & 31U)) return kTfLiteError;
    *result = {};
    tflite::SingleArenaBufferAllocator memory(arena, capacity);
    tflite::GreedyMemoryPlanner planner;
    ProbeAllocator allocator(&memory, &planner);
    /* Zeroed arena is supplied by the caller so failure destructors cannot
     * see poisoned node registrations. Scope ends before the arena is freed. */
    tflite::MicroInterpreter interpreter(model, resolver, &allocator);
    TfLiteStatus status = interpreter.AllocateTensors();
    if (status == kTfLiteOk) {
        result->head = memory.GetNonPersistentUsedBytes();
        result->tail = memory.GetPersistentUsedBytes();
    }
    return status;
}

inline tflite::MicroAllocator *Create(const Measurement &measurement,
                                     uint8_t *persistent, size_t persistent_size,
                                     uint8_t *head, size_t head_size) {
    const size_t tail_required = TailCapacity(measurement);
    if (!persistent || !head || !tail_required || persistent_size < tail_required ||
        head_size < measurement.head || !measurement.head ||
        (reinterpret_cast<uintptr_t>(persistent) & 31U) ||
        (reinterpret_cast<uintptr_t>(head) & 31U) ||
        (persistent_size & 31U)) return nullptr;
    uintptr_t p = reinterpret_cast<uintptr_t>(persistent), h = reinterpret_cast<uintptr_t>(head);
    if (p > UINTPTR_MAX - persistent_size || h > UINTPTR_MAX - head_size ||
        (p < h + head_size && h < p + persistent_size)) return nullptr;
    /* Capacity is checked before the factory's unchecked placement-new paths. */
    return tflite::MicroAllocator::Create(persistent, persistent_size, head, head_size);
}

inline bool SupportedIO(tflite::TensorType type) {
    return type == tflite::TensorType_FLOAT32 || type == tflite::TensorType_UINT8 ||
           type == tflite::TensorType_INT8 || type == tflite::TensorType_UINT16 ||
           type == tflite::TensorType_INT16;
}

inline const char *Validate(const tflite::Model *model) {
    if (!model || !model->subgraphs() || model->subgraphs()->size() != 1 ||
        !model->buffers() || !model->operator_codes()) return "one static subgraph required";
    const auto *graph = model->subgraphs()->Get(0);
    if (!graph || !graph->tensors() || !graph->operators() || !graph->inputs() ||
        !graph->outputs()) return "incomplete shared graph";
    for (const auto *tensor : *graph->tensors()) {
        if (!tensor || tensor->is_variable() ||
            tensor->type() == tflite::TensorType_RESOURCE ||
            tensor->type() == tflite::TensorType_VARIANT) return "stateful tensor is unsupported";
        if (tensor->shape_signature()) {
            for (int dim : *tensor->shape_signature()) {
                if (dim < 0) return "dynamic tensors are unsupported";
            }
        }
    }
    for (const auto *indices : {graph->inputs(), graph->outputs()}) {
        if (indices->size() == 0) return "nonempty model IO required";
        for (int index : *indices) {
            if (index < 0 || static_cast<unsigned>(index) >= graph->tensors()->size()) return "invalid IO tensor";
            const auto *tensor = graph->tensors()->Get(index);
            if (!SupportedIO(tensor->type()) || !tensor->shape()) return "unsupported shared IO type/shape";
            size_t count = 1;
            for (int dim : *tensor->shape()) {
                if (dim <= 0 || count > std::numeric_limits<size_t>::max() / static_cast<unsigned>(dim)) {
                    return "invalid shared IO shape";
                }
                count *= static_cast<unsigned>(dim);
            }
            if (count > std::numeric_limits<size_t>::max() / 4U ||
                tensor->buffer() >= model->buffers()->size()) return "invalid shared IO extent";
            const auto *buffer = model->buffers()->Get(tensor->buffer());
            /* First release rejects constant outputs as well as inputs, so
             * every top-level data pointer has one simple head ownership rule. */
            if (!buffer || (buffer->data() && buffer->data()->size())) return "constant shared IO is unsupported";
        }
    }
    for (const auto *op : *graph->operators()) {
        if (!op || op->opcode_index() >= model->operator_codes()->size()) return "invalid shared operator";
        if (op->mutating_variable_inputs()) {
            for (bool mutates : *op->mutating_variable_inputs()) {
                if (mutates) return "mutating shared operator is unsupported";
            }
        }
        const auto *code = model->operator_codes()->Get(op->opcode_index());
        if (!code) return "invalid shared operator code";
        const auto builtin = tflite::GetBuiltinCode(code);
        if (builtin == tflite::BuiltinOperator_CUSTOM) {
            if (!code->custom_code() || std::strcmp(code->custom_code()->c_str(), "ethos-u")) {
                return "unknown shared custom operator";
            }
        } else if (builtin != tflite::BuiltinOperator_PAD &&
                   builtin != tflite::BuiltinOperator_DEQUANTIZE &&
                   builtin != tflite::BuiltinOperator_ADD &&
                   builtin != tflite::BuiltinOperator_MUL) {
            return "shared operator is outside the reviewed stateless set";
        }
    }
    return nullptr;
}

inline bool Contains(const uint8_t *head, size_t capacity, const TfLiteTensor *tensor) {
    if (!head || !tensor || !tensor->data.data || !tensor->bytes) return false;
    uintptr_t start = reinterpret_cast<uintptr_t>(head);
    uintptr_t data = reinterpret_cast<uintptr_t>(tensor->data.data);
    return data >= start && tensor->bytes <= capacity && data - start <= capacity - tensor->bytes;
}

inline bool ValidateIO(tflite::MicroInterpreter &interpreter, const uint8_t *head, size_t capacity) {
    for (size_t i = 0; i < interpreter.inputs_size(); ++i) {
        if (!Contains(head, capacity, interpreter.input(i))) return false;
    }
    for (size_t i = 0; i < interpreter.outputs_size(); ++i) {
        if (!Contains(head, capacity, interpreter.output(i))) return false;
    }
    return true;
}
} // namespace titan_shared
#endif
