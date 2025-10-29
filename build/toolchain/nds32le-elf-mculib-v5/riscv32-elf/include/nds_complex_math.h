/******************************************************************************
 * Copyright (C) 2010-2018 Arm Limited or its affiliates. All rights reserved.*
 * Copyright (C) 2018-2021 Andes Technology Corporation. All rights reserved. *
 *                                                                            *
 * SPDX-License-Identifier: Apache-2.0                                        *
 *                                                                            *
 * Licensed under the Apache License, Version 2.0 (the License); you may      *
 * not use this file except in compliance with the License.                   *
 * You may obtain a copy of the License at                                    *
 *                                                                            *
 * www.apache.org/licenses/LICENSE-2.0                                        *
 *                                                                            *
 * Unless required by applicable law or agreed to in writing, software        *
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT    *
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.           *
 * See the License for the specific language governing permissions and        *
 * limitations under the License.                                             *
 ******************************************************************************/
#ifndef __NDS_COMPLEX_MATH_H__
#define __NDS_COMPLEX_MATH_H__

/**
 * @defgroup complex Complex Functions
 * This set of functions operates on complex data vectors.
 * The data in the input <code>src</code> vector and output <code>dst</code>
 * are arranged in the array as: [real, imag, real, imag, real, imag, ...).
 */
#ifdef  __cplusplus
extern "C"
{
#endif

#include "riscv_dsp_complex_math.h"

// Complex Conjugate
//
/// Conjugate the floating-point complex vector.
#define nds_cconj_f32    riscv_dsp_cconj_f32
#define nds_clx_conj_f32 riscv_dsp_cconj_f32

/// Conjugate the q15 complex vector.
#define nds_cconj_q15    riscv_dsp_cconj_q15
#define nds_clx_conj_q15 riscv_dsp_cconj_q15

/// Conjugate the q31 complex vector.
#define nds_cconj_q31    riscv_dsp_cconj_q31
#define nds_clx_conj_q31 riscv_dsp_cconj_q31

// Complex Dot Product
//
/// Compute the dot product of the floating-point complex vector.
#define nds_cdprod_f32       riscv_dsp_cdprod_f32
#define nds_clx_dot_prod_f32 riscv_dsp_cdprod_f32

/// Compute the dot product type2 of the floating-point complex vector.
#define nds_cdprod_typ2_f32       riscv_dsp_cdprod_typ2_f32
#define nds_clx_dot_prod_typ2_f32 riscv_dsp_cdprod_typ2_f32

/// Compute the dot product of the q15 complex vector.
#define nds_cdprod_q15       riscv_dsp_cdprod_q15
#define nds_clx_dot_prod_q15 riscv_dsp_cdprod_q15

/// Compute the dot product type2 of the q15 complex vector.
#define nds_cdprod_typ2_q15       riscv_dsp_cdprod_typ2_q15
#define nds_clx_dot_prod_typ2_q15 riscv_dsp_cdprod_typ2_q15

/// Compute the dot product of the q31 complex vector.
#define nds_cdprod_q31       riscv_dsp_cdprod_q31
#define nds_clx_dot_prod_q31 riscv_dsp_cdprod_q31

/// Compute the dot product type2 of the q31 complex vector.
#define nds_cdprod_typ2_q31       riscv_dsp_cdprod_typ2_q31
#define nds_clx_dot_prod_typ2_q31 riscv_dsp_cdprod_typ2_q31

// Complex Magnitude
//
/// Compute the magnitude of the floating-point complex vector.
#define nds_cmag_f32    riscv_dsp_cmag_f32
#define nds_clx_mag_f32 riscv_dsp_cmag_f32

/// Compute the magnitude of the q15 complex vector.
#define nds_cmag_q15    riscv_dsp_cmag_q15
#define nds_clx_mag_q15 riscv_dsp_cmag_q15

/// Compute the magnitude of the q31 complex vector.
#define nds_cmag_q31    riscv_dsp_cmag_q31
#define nds_clx_mag_q31 riscv_dsp_cmag_q31

// Complex Magnitude Squared
/// Compute the magnitude squared of the floating-point complex vector.
#define nds_cmag_sqr_f32    riscv_dsp_cmag_sqr_f32
#define nds_clx_mag_sqr_f32 riscv_dsp_cmag_sqr_f32

/// Compute the magnitude squared of the q15 complex vector.
#define nds_cmag_sqr_q15    riscv_dsp_cmag_sqr_q15
#define nds_clx_mag_sqr_q15 riscv_dsp_cmag_sqr_q15

/// Compute the magnitude squared of the q31 complex vector.
#define nds_cmag_sqr_q31    riscv_dsp_cmag_sqr_q31
#define nds_clx_mag_sqr_q31 riscv_dsp_cmag_sqr_q31

// Complex Multiplication
//
/// Multiply two folating-point complex vector.
#define nds_cmul_f32    riscv_dsp_cmul_f32
#define nds_clx_mul_f32 riscv_dsp_cmul_f32

/// Multiply two q15 complex vector.
#define nds_cmul_q15    riscv_dsp_cmul_q15
#define nds_clx_mul_q15 riscv_dsp_cmul_q15

///Multiply two q31 complex vector.
#define nds_cmul_q31    riscv_dsp_cmul_q31
#define nds_clx_mul_q31 riscv_dsp_cmul_q31

// Complex-by-Real Multiplication
//
/// Multiply the folating-point complex vector by a real vector.
#define nds_cmul_real_f32    riscv_dsp_cmul_real_f32
#define nds_clx_mul_real_f32 riscv_dsp_cmul_real_f32

/// Multiply the q15 complex vector by a real vector.
#define nds_cmul_real_q15    riscv_dsp_cmul_real_q15
#define nds_clx_mul_real_q15 riscv_dsp_cmul_real_q15

/// Multiply the q31 complex vector by a real vector.
#define nds_cmul_real_q31    riscv_dsp_cmul_real_q31
#define nds_clx_mul_real_q31 riscv_dsp_cmul_real_q31

#ifdef  __cplusplus
}
#endif
#endif // __NDS_COMPLEX_MATH_H__
