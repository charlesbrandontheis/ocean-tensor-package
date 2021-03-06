/* ------------------------------------------------------------------------ */
/* Copyright 2018, IBM Corp.                                                */
/*                                                                          */
/* Licensed under the Apache License, Version 2.0 (the "License");          */
/* you may not use this file except in compliance with the License.         */
/* You may obtain a copy of the License at                                  */
/*                                                                          */
/*    http://www.apache.org/licenses/LICENSE-2.0                            */
/*                                                                          */
/* Unless required by applicable law or agreed to in writing, software      */
/* distributed under the License is distributed on an "AS IS" BASIS,        */
/* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. */
/* See the License for the specific language governing permissions and      */
/* limitations under the License.                                           */
/* ------------------------------------------------------------------------ */

#ifndef __SOLID_GPU_INDEX1_H__
#define __SOLID_GPU_INDEX1_H__

#include "solid.h"
#include "solid/base/gpu/generate_macros.h"


/* ------------------------------------------------------------------------ */
/* Interface                                                                */
/* ------------------------------------------------------------------------ */
/* The index launch macros for GPU assume that the following                */
/* variables have been defined:                                             */
/*   int           ndims;                                                   */
/*   size_t       *size;                                                    */
/*   solid_int64 **offsets;                                                 */
/*   ptrdiff_t    *strides;                                                 */
/*   void         *ptr;                                                     */
/*   int           result;          // Updated to reflect success / failure */
/* ------------------------------------------------------------------------ */
/* Exposed variables in kernel:                                             */
/*   solid_c_type(TYPE) *_ptr                                               */
/*   size_t              _index                                             */
/* ------------------------------------------------------------------------ */


/* ------------------------------------------------------------------------ */
/* Tensor information structure                                             */
/* ------------------------------------------------------------------------ */

typedef struct
{  size_t        size[SOLID_MAX_TENSOR_DIMS];
   solid_int64  *offsets[SOLID_MAX_TENSOR_DIMS];
   ptrdiff_t     strides[SOLID_MAX_TENSOR_DIMS];
   char         *ptr;
   size_t        nelem;
   int           ndims;
} solid_index1_data;


/* ------------------------------------------------------------------------ */
/* Analysis function declaration                                            */
/* ------------------------------------------------------------------------ */
SOLID_API int solid_gpu_index1_analyze(int ndims, const size_t *size, solid_int64 **offsets,
                                       const ptrdiff_t *strides, void *ptr,
                                       int elemsize, solid_index1_data *data,
                                       solid_gpu_config *config);


/* ------------------------------------------------------------------------ */
/* Name macros                                                              */
/* ------------------------------------------------------------------------ */

#define SOLID_KERNEL_INDEX1_NAME_B(PREFIX)  PREFIX##_kernel
#define SOLID_KERNEL_INDEX1_NAME(PREFIX)    SOLID_KERNEL_INDEX1_NAME_B(PREFIX)


/* ------------------------------------------------------------------------ */
/* Kernel interface macros                                                  */
/* ------------------------------------------------------------------------ */

#define SOLID_KERNEL_INDEX1_ITF_0(PREFIX) \
   SOLID_KERNEL_INDEX1_NAME(PREFIX)(solid_index1_data data)
#define SOLID_KERNEL_INDEX1_ITF_1(PREFIX) \
   SOLID_KERNEL_INDEX1_NAME(PREFIX)(solid_index1_data data, SOLID_KERNEL_PARAM_PREFIX(PREFIX) param)
#define SOLID_KERNEL_INDEX1_ITF(PREFIX, FLAG_PARAM) \
   SOLID_KERNEL_INDEX1_ITF_##FLAG_PARAM(PREFIX)


/* ------------------------------------------------------------------------ */
/* Create all cuda kernels                                                  */
/* ------------------------------------------------------------------------ */

/* Create kernel functions */
#define SOLID_CREATE_KERNELS_INDEX1(PREFIX, FLAG_PARAM, DTYPE, CODE) \
   SOLID_CREATE_KERNEL_INDEX1(PREFIX, DTYPE, FLAG_PARAM, CODE, 512)

/* Create types and kernels */
#define SOLID_KERNELS_INDEX1_PREFIX_B(PREFIX, DTYPE, FLAG_PARAM, PARAM, CODE) \
   SOLID_CREATE_KERNEL_TYPES(PREFIX, FLAG_PARAM, PARAM) \
   SOLID_CREATE_KERNELS_INDEX1(PREFIX, FLAG_PARAM, DTYPE, CODE)

#define SOLID_KERNELS_INDEX1_PREFIX(PREFIX, DTYPE, FLAG_PARAM, PARAM, CODE) \
   SOLID_KERNELS_INDEX1_PREFIX_B(PREFIX, DTYPE, FLAG_PARAM, PARAM, CODE)


/* Main interfaces */
#define SOLID_KERNELS_INDEX1_FULL(DTYPE, NAME, PARAM, CODE) \
   SOLID_KERNELS_INDEX1_PREFIX(SOLID_FUNCTION_TYPE(NAME, DTYPE), DTYPE, 1, PARAM, CODE)

#define SOLID_KERNELS_INDEX1_PARAM(NAME, PARAM, CODE) \
   SOLID_KERNELS_INDEX1_PREFIX(SOLID_FUNCTION_TYPE(NAME, SDXTYPE), SDXTYPE, 1, PARAM, CODE)

#define SOLID_KERNELS_INDEX1_TYPE(DTYPE, NAME, CODE) \
   SOLID_KERNELS_INDEX1_PREFIX(SOLID_FUNCTION_TYPE(NAME, DTYPE), DTYPE, 0, { }, CODE)

#define SOLID_KERNELS_INDEX1(NAME, CODE) \
   SOLID_KERNELS_INDEX1_PREFIX(SOLID_FUNCTION_TYPE(NAME, SDXTYPE), SDXTYPE, 0, { }, CODE)


/* ------------------------------------------------------------------------ */
/* Launching the kernels                                                    */
/* ------------------------------------------------------------------------ */

#define SOLID_SUBMIT_INDEX1_PARAM_0(PARAM) \
      (__index_data)
#define SOLID_SUBMIT_INDEX1_PARAM_1(PARAM) \
      (__index_data, PARAM)
#define SOLID_SUBMIT_INDEX1_PARAM(FLAG_PARAM, PARAM) \
      SOLID_SUBMIT_INDEX1_PARAM_##FLAG_PARAM(PARAM)

#define SOLID_SUBMIT_INDEX1(PREFIX, CONFIG, SHAREDMEM, STREAM, FLAG_PARAM, PARAM) \
      SOLID_KERNEL_INDEX1_NAME(PREFIX)\
      <<<(CONFIG)->blocks, (CONFIG)->threads, SHAREDMEM, STREAM>>>\
      SOLID_SUBMIT_INDEX1_PARAM(FLAG_PARAM, PARAM)

#define SOLID_LAUNCH_INDEX1_CODE(PREFIX, DTYPE, STREAM, FLAG_PARAM, PARAM, RESULT) \
   {  solid_index1_data  __index_data; \
      solid_gpu_config   __index_kernel_config; \
      cudaError_t        __cuda_status; \
      \
      /* Initialize the data */ \
      (RESULT) = solid_gpu_index1_analyze(ndims, size, offsets, strides, ptr, \
                                          sizeof(SOLID_C_TYPE(DTYPE)), \
                                          &(__index_data), \
                                          &(__index_kernel_config)); \
      \
      /* Call the kernel */ \
      if (((RESULT) == 0) && (__index_data.nelem > 0)) \
      {  /* Launch the kernel and check the result */ \
         SOLID_SUBMIT_INDEX1(PREFIX, &(__index_kernel_config), 0, STREAM, FLAG_PARAM, PARAM); \
         if ((__cuda_status = cudaGetLastError()) != cudaSuccess) \
         {  SOLID_ERROR_MESSAGE("Cuda error: %s", cudaGetErrorString(__cuda_status)); \
            (RESULT) = -1; \
         } \
      } \
   }


/* Generate the launch code */
#define SOLID_LAUNCH_INDEX1_PREFIX_B(PREFIX, DTYPE, STREAM, FLAG_PARAM, PARAM, RESULT) \
   SOLID_LAUNCH_INDEX1_CODE(PREFIX, DTYPE, STREAM, FLAG_PARAM, PARAM, RESULT)

#define SOLID_LAUNCH_INDEX1_PREFIX(PREFIX, DTYPE, STREAM, FLAG_PARAM, PARAM, RESULT) \
   SOLID_LAUNCH_INDEX1_PREFIX_B(PREFIX, DTYPE, STREAM, FLAG_PARAM, PARAM, RESULT)


/* Main interfaces */
#define SOLID_LAUNCH_INDEX1_FULL(DTYPE, NAME, STREAM, PARAM, RESULT) \
   SOLID_LAUNCH_INDEX1_PREFIX(SOLID_FUNCTION_TYPE(NAME, DTYPE), DTYPE, STREAM, 1, PARAM, RESULT)

#define SOLID_LAUNCH_INDEX1_PARAM(NAME, STREAM, PARAM, RESULT) \
   SOLID_LAUNCH_INDEX1_PREFIX(SOLID_FUNCTION_TYPE(NAME, SDXTYPE), SDXTYPE, STREAM, 1, PARAM, RESULT)

#define SOLID_LAUNCH_INDEX1_TYPE(DTYPE, NAME, STREAM, RESULT) \
   SOLID_LAUNCH_INDEX1_PREFIX(SOLID_FUNCTION_TYPE(NAME, DTYPE), DTYPE, STREAM, 0, NULL, RESULT)

#define SOLID_LAUNCH_INDEX1(NAME, STREAM, RESULT) \
   SOLID_LAUNCH_INDEX1_PREFIX(SOLID_FUNCTION_TYPE(NAME, SDXTYPE), SDXTYPE, STREAM, 0, NULL, RESULT)


/* --------------------------------------------------------------------- */
/* KERNELS                                                               */
/* --------------------------------------------------------------------- */

#define SOLID_CREATE_KERNEL_INDEX1(PREFIX, DTYPE, FLAG_PARAM, CODE, BOUNDS) \
   __launch_bounds__(BOUNDS) \
   __global__ void SOLID_KERNEL_INDEX1_ITF(PREFIX, FLAG_PARAM) \
   {  SOLID_C_TYPE(DTYPE) *_ptr; \
      ptrdiff_t  _offset; \
      size_t     _index; \
      \
      for (_index = blockIdx.x * blockDim.x + threadIdx.x; \
           _index < data.nelem; \
           _index += gridDim.x * blockDim.x) \
      { \
         /* Determine the offsets */ \
         {  solid_int64 *_offsetPtr; \
            ptrdiff_t _idx; \
            ptrdiff_t _s; \
            int _i; \
            \
            _idx = _index; \
            _offset = 0; \
            for (_i = 0; _i < data.ndims; _i++) \
            {  _s       = _idx % data.size[_i]; \
               _idx     = _idx / data.size[_i]; \
               if ((_offsetPtr = data.offsets[_i]) == NULL) \
                    _offset += _s * data.strides[_i]; \
               else _offset += _offsetPtr[_s]; \
            } \
         } \
         \
         /* Determine the data pointers */ \
         _ptr = (SOLID_C_TYPE(DTYPE) *)(data.ptr + _offset); \
         \
         /* Expand the code */ \
         CODE \
      } \
   }

#endif

