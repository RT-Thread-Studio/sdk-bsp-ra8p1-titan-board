/* SPDX-License-Identifier: Apache-2.0 */
/* Check only operators used by a subgraph, before allocating an arena. */
#include <float.h>

typedef struct {
    float scale;
    int64_t zero_point;
} titan_tflm_scalar_quant_t;

static const Tensor *titan_tflm_contract_tensor(const SubGraph *graph, int index, const char *name) {
    const auto *tensors = graph->tensors();
    if (!tensors || index < 0 || static_cast<unsigned>(index) >= tensors->size() ||
        !tensors->Get(index)) {
        mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM %s has invalid tensor index"), name);
    }
    return tensors->Get(index);
}

static titan_tflm_scalar_quant_t titan_tflm_int8_quant(const Tensor *tensor, const char *name) {
    const auto *quant = tensor->quantization();
    if (!quant || !quant->scale() || quant->scale()->size() != 1 ||
        !quant->zero_point() || quant->zero_point()->size() != 1) {
        mp_raise_msg_varg(&mp_type_ValueError,
            MP_ERROR_TEXT("TFLM %s requires per-tensor INT8 quantization"), name);
    }
    const float scale = quant->scale()->Get(0);
    const int64_t zero = quant->zero_point()->Get(0);
    if (!(scale > 0.0f) || scale > FLT_MAX || zero < -128 || zero > 127) {
        mp_raise_msg_varg(&mp_type_ValueError,
            MP_ERROR_TEXT("TFLM %s has invalid INT8 scale or zero_point"), name);
    }
    return {scale, zero};
}

/* These pinned kernels do not fully validate the contracts below in Prepare.
 * Maximum/Minimum compare raw integer values; Relu6 clamps them without
 * rescaling. Relu supports rescaling and must retain that valid capability. */
static void titan_tflm_check_tensor_contract(const SubGraph *graph, const Operator *op,
                                            BuiltinOperator builtin) {
    const bool compare = builtin == BuiltinOperator_MAXIMUM || builtin == BuiltinOperator_MINIMUM;
    const bool relu = builtin == BuiltinOperator_RELU;
    const bool relu6 = builtin == BuiltinOperator_RELU6;
    if (!compare && !relu && !relu6) { return; }
    const char *name = EnumNameBuiltinOperator(builtin);
    const unsigned inputs = compare ? 2U : 1U;
    if (!op->inputs() || op->inputs()->size() != inputs ||
        !op->outputs() || op->outputs()->size() != 1) {
        mp_raise_msg_varg(&mp_type_ValueError,
            MP_ERROR_TEXT("TFLM %s expects %u inputs and one output"), name, inputs);
    }
    const Tensor *input = titan_tflm_contract_tensor(graph, op->inputs()->Get(0), name);
    const Tensor *output = titan_tflm_contract_tensor(graph, op->outputs()->Get(0), name);
    const Tensor *second = compare ? titan_tflm_contract_tensor(graph, op->inputs()->Get(1), name) : input;
    const TensorType type = input->type();
    if (type != output->type() || type != second->type()) {
        mp_raise_msg_varg(&mp_type_ValueError,
            MP_ERROR_TEXT("TFLM %s expects matching tensor types"), name);
    }
    if (type != TensorType_FLOAT32 && type != TensorType_INT8 &&
        !(compare && (type == TensorType_INT32 || type == TensorType_INT64))) {
        mp_raise_msg_varg(&mp_type_ValueError,
            MP_ERROR_TEXT("TFLM %s does not support tensor type %d"), name, static_cast<int>(type));
    }
    if (type == TensorType_INT8) {
        const auto a = titan_tflm_int8_quant(input, name);
        const auto b = compare ? titan_tflm_int8_quant(second, name) : a;
        const auto y = titan_tflm_int8_quant(output, name);
        if (!relu && (a.scale != b.scale || a.zero_point != b.zero_point ||
                      a.scale != y.scale || a.zero_point != y.zero_point)) {
            mp_raise_msg_varg(&mp_type_ValueError,
                MP_ERROR_TEXT("TFLM %s requires identical scale and zero_point"), name);
        }
    }
}

static void titan_tflm_check_model_ops(const Model *model, const MicroOpsResolver &resolver) {
    const auto *codes = model->operator_codes();
    const auto *graphs = model->subgraphs();
    if (!codes || !graphs) {
        mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM model has no graph/operator table"));
    }
    for (unsigned graph_index = 0; graph_index < graphs->size(); ++graph_index) {
        const auto *operators = graphs->Get(graph_index)->operators();
        if (!operators) { continue; }
        for (unsigned index = 0; index < operators->size(); ++index) {
            unsigned code_index = operators->Get(index)->opcode_index();
            if (code_index >= codes->size()) {
                mp_raise_msg(&mp_type_ValueError, MP_ERROR_TEXT("TFLM operator index out of range"));
            }
            const auto *opcode = codes->Get(code_index);
            const auto builtin = GetBuiltinCode(opcode);
            if (builtin == BuiltinOperator_CUSTOM) {
                const char *name = opcode->custom_code() ? opcode->custom_code()->c_str() : "";
                if (!name[0] || !resolver.FindOp(name)) {
                    mp_raise_msg_varg(&mp_type_ValueError, MP_ERROR_TEXT("Unsupported TFLM custom op: %s"), name);
                }
            } else if (!resolver.FindOp(builtin)) {
                mp_raise_msg_varg(&mp_type_ValueError,
                    MP_ERROR_TEXT("Unsupported TFLM op: %s (code=%d, version=%d)"),
                    EnumNameBuiltinOperator(builtin), (int)builtin, (int)opcode->version());
            }
            titan_tflm_check_tensor_contract(graphs->Get(graph_index), operators->Get(index), builtin);
        }
    }
}
