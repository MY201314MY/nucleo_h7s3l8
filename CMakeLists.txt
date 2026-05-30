# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)

find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(with_mcuboot)

# Verify that this sample is built through sysbuild to ensure MCUboot is
# automatically included and that sample specific MCUboot configurations are
# used when building MCUboot.
test_sysbuild()

target_sources(app PRIVATE 
    src/main.cpp
    src/operation.c
    src/bsp/uart.c

    src/nucleo_iks01a1/nucleo_iks01a1.c
)

add_compile_definitions(CMSIS_NN)

set(TFLITE_MICRO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/tflite)

target_sources(app PRIVATE
    ${TFLITE_MICRO_DIR}/signal/micro/kernels/delay.cc
    ${TFLITE_MICRO_DIR}/signal/micro/kernels/energy.cc
    ${TFLITE_MICRO_DIR}/signal/micro/kernels/fft_auto_scale_common.cc
    ${TFLITE_MICRO_DIR}/signal/micro/kernels/fft_auto_scale_kernel.cc
    ${TFLITE_MICRO_DIR}/signal/micro/kernels/filter_bank.cc
    ${TFLITE_MICRO_DIR}/signal/micro/kernels/filter_bank_log.cc
    ${TFLITE_MICRO_DIR}/signal/micro/kernels/filter_bank_spectral_subtraction.cc
    ${TFLITE_MICRO_DIR}/signal/micro/kernels/filter_bank_square_root.cc
    ${TFLITE_MICRO_DIR}/signal/micro/kernels/filter_bank_square_root_common.cc
    ${TFLITE_MICRO_DIR}/signal/micro/kernels/framer.cc
    ${TFLITE_MICRO_DIR}/signal/micro/kernels/irfft.cc
    ${TFLITE_MICRO_DIR}/signal/micro/kernels/overlap_add.cc
    ${TFLITE_MICRO_DIR}/signal/micro/kernels/pcan.cc
    ${TFLITE_MICRO_DIR}/signal/micro/kernels/rfft.cc
    ${TFLITE_MICRO_DIR}/signal/micro/kernels/stacker.cc
    ${TFLITE_MICRO_DIR}/signal/micro/kernels/window.cc
    ${TFLITE_MICRO_DIR}/signal/src/circular_buffer.cc
    ${TFLITE_MICRO_DIR}/signal/src/energy.cc
    ${TFLITE_MICRO_DIR}/signal/src/fft_auto_scale.cc
    ${TFLITE_MICRO_DIR}/signal/src/filter_bank.cc
    ${TFLITE_MICRO_DIR}/signal/src/filter_bank_log.cc
    ${TFLITE_MICRO_DIR}/signal/src/filter_bank_spectral_subtraction.cc
    ${TFLITE_MICRO_DIR}/signal/src/filter_bank_square_root.cc
    ${TFLITE_MICRO_DIR}/signal/src/irfft_float.cc
    ${TFLITE_MICRO_DIR}/signal/src/irfft_int16.cc
    ${TFLITE_MICRO_DIR}/signal/src/irfft_int32.cc
    ${TFLITE_MICRO_DIR}/signal/src/kiss_fft_wrappers/kiss_fft_float.cc
    ${TFLITE_MICRO_DIR}/signal/src/kiss_fft_wrappers/kiss_fft_int16.cc
    ${TFLITE_MICRO_DIR}/signal/src/kiss_fft_wrappers/kiss_fft_int32.cc
    ${TFLITE_MICRO_DIR}/signal/src/log.cc
    ${TFLITE_MICRO_DIR}/signal/src/max_abs.cc
    ${TFLITE_MICRO_DIR}/signal/src/msb_32.cc
    ${TFLITE_MICRO_DIR}/signal/src/msb_64.cc
    ${TFLITE_MICRO_DIR}/signal/src/overlap_add.cc
    ${TFLITE_MICRO_DIR}/signal/src/pcan_argc_fixed.cc
    ${TFLITE_MICRO_DIR}/signal/src/rfft_float.cc
    ${TFLITE_MICRO_DIR}/signal/src/rfft_int16.cc
    ${TFLITE_MICRO_DIR}/signal/src/rfft_int32.cc
    ${TFLITE_MICRO_DIR}/signal/src/square_root_32.cc
    ${TFLITE_MICRO_DIR}/signal/src/square_root_64.cc
    ${TFLITE_MICRO_DIR}/signal/src/window.cc
    ${TFLITE_MICRO_DIR}/tensorflow/compiler/mlir/lite/core/api/error_reporter.cc
    ${TFLITE_MICRO_DIR}/tensorflow/compiler/mlir/lite/schema/schema_utils.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/core/api/flatbuffer_conversions.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/core/api/tensor_utils.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/core/c/common.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/kernels/internal/common.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/kernels/internal/portable_tensor_utils.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/kernels/internal/quantization_util.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/kernels/internal/reference/comparisons.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/kernels/internal/reference/portable_tensor_utils.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/kernels/internal/runtime_shape.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/kernels/internal/tensor_ctypes.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/kernels/internal/tensor_utils.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/kernels/kernel_util.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/arena_allocator/non_persistent_arena_buffer_allocator.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/arena_allocator/persistent_arena_buffer_allocator.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/arena_allocator/recording_single_arena_buffer_allocator.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/arena_allocator/single_arena_buffer_allocator.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/debug_log.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/flatbuffer_utils.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/hexdump.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/activations.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/activations_common.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/cmsis_nn/add.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/add_common.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/add_n.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/arg_min_max.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/assign_variable.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/cmsis_nn/batch_matmul.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/batch_matmul_common.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/batch_to_space_nd.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/broadcast_args.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/broadcast_to.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/call_once.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/cast.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/ceil.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/circular_buffer.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/circular_buffer_common.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/comparisons.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/concatenation.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/cmsis_nn/conv.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/conv_common.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/cumsum.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/decode.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/decode_state.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/decode_state_huffman.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/decode_state_lut.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/decode_state_prune.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/decompress.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/decompress_common.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/depth_to_space.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/cmsis_nn/depthwise_conv.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/depthwise_conv_common.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/dequantize.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/dequantize_common.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/detection_postprocess.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/div.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/dynamic_update_slice.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/elementwise.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/elu.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/embedding_lookup.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/ethosu.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/exp.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/expand_dims.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/fill.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/floor.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/floor_div.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/floor_mod.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/cmsis_nn/fully_connected.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/fully_connected_common.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/gather.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/gather_nd.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/hard_swish.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/hard_swish_common.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/if.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/kernel_runner.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/kernel_util.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/l2_pool_2d.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/l2norm.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/leaky_relu.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/leaky_relu_common.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/log_softmax.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/logical.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/logical_common.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/logistic.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/logistic_common.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/lstm_eval.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/lstm_eval_common.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/cmsis_nn/maximum_minimum.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/micro_tensor_utils.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/mirror_pad.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/cmsis_nn/mul.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/mul_common.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/neg.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/pack.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/cmsis_nn/pad.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/pad_common.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/cmsis_nn/pooling.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/pooling_common.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/prelu.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/prelu_common.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/quantize.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/quantize_common.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/read_variable.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/reduce.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/reduce_common.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/reshape.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/reshape_common.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/resize_bilinear.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/resize_nearest_neighbor.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/reverse.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/round.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/select.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/shape.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/slice.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/cmsis_nn/softmax.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/softmax_common.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/space_to_batch_nd.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/space_to_depth.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/split.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/split_v.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/squared_difference.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/squeeze.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/strided_slice.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/strided_slice_common.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/sub.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/sub_common.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/cmsis_nn/svdf.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/svdf_common.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/tanh.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/cmsis_nn/transpose.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/transpose_common.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/cmsis_nn/transpose_conv.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/cmsis_nn/unidirectional_sequence_lstm.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/unpack.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/var_handle.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/while.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/kernels/zeros_like.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/memory_helpers.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/memory_planner/greedy_memory_planner.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/memory_planner/linear_memory_planner.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/memory_planner/non_persistent_buffer_planner_shim.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/micro_allocation_info.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/micro_allocator.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/micro_context.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/micro_interpreter.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/micro_interpreter_context.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/micro_interpreter_graph.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/micro_log.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/micro_op_resolver.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/micro_profiler.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/micro_resource_variable.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/micro_time.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/micro_utils.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/recording_micro_allocator.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/system_setup.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/test_helper_custom_ops.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/test_helpers.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/tflite_bridge/flatbuffer_conversions_bridge.cc
    ${TFLITE_MICRO_DIR}/tensorflow/lite/micro/tflite_bridge/micro_error_reporter.cc
    ${TFLITE_MICRO_DIR}/third_party/kissfft/kiss_fft.c
    ${TFLITE_MICRO_DIR}/third_party/kissfft/tools/kiss_fftr.c
)

file(GLOB_RECURSE CMSIS_NN_SRCS
    "${TFLITE_MICRO_DIR}/third_party/cmsis_nn/Source/ActivationFunctions/*.c"
    "${TFLITE_MICRO_DIR}/third_party/cmsis_nn/Source/ConvolutionFunctions/*.c"
    "${TFLITE_MICRO_DIR}/third_party/cmsis_nn/Source/NNSupportFunctions/*.c"
    "${TFLITE_MICRO_DIR}/third_party/cmsis_nn/Source/ReshapeFunctions/*.c"
    "${TFLITE_MICRO_DIR}/third_party/cmsis_nn/Source/TransposeFunctions/*.c"
    "${TFLITE_MICRO_DIR}/third_party/cmsis_nn/Source/BasicMathFunctions/*.c"
    "${TFLITE_MICRO_DIR}/third_party/cmsis_nn/Source/FullyConnectedFunctions/*.c"
    "${TFLITE_MICRO_DIR}/third_party/cmsis_nn/Source/PadFunctions/*.c"
    "${TFLITE_MICRO_DIR}/third_party/cmsis_nn/Source/SoftmaxFunctions/*.c"
    "${TFLITE_MICRO_DIR}/third_party/cmsis_nn/Source/LSTMFunctions/*.c"
    "${TFLITE_MICRO_DIR}/third_party/cmsis_nn/Source/PoolingFunctions/*.c"
    "${TFLITE_MICRO_DIR}/third_party/cmsis_nn/Source/SVDFunctions/*.c"
)

target_sources(app PRIVATE ${CMSIS_NN_SRCS})

target_include_directories(app PRIVATE
    ${TFLITE_MICRO_DIR}                                     # Corresponds to #include "tensorflow/..." and "signal/..."
    ${TFLITE_MICRO_DIR}/third_party/flatbuffers/include     # flatbuffers headers
    ${TFLITE_MICRO_DIR}/third_party/gemmlowp                # fixed-point math library
    ${TFLITE_MICRO_DIR}/third_party/kissfft                 # FFT library used for signal processing
    ${TFLITE_MICRO_DIR}/third_party/ruy                     # ruy shim for matrix multiplication acceleration

    ${TFLITE_MICRO_DIR}/third_party/cmsis_nn
    ${TFLITE_MICRO_DIR}/third_party/cmsis_nn/Include
)

target_compile_options(app PRIVATE
    -fno-exceptions
    -ffunction-sections
    -fdata-sections
    -DTF_LITE_STATIC_MEMORY
    -DTF_LITE_DISABLE_X86_NEON
)

target_compile_options(app INTERFACE $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>)



