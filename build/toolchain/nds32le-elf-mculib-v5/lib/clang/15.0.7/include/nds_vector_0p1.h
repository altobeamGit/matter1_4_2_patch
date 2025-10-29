/*===---- nds_vector_0p1.h - Andes vector instrinsic wrapper --------------===*\
 *
 * Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
 * See https://llvm.org/LICENSE.txt for license information.
 * SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
 *
\*===----------------------------------------------------------------------===*/

#ifndef _NDS_VECTOR_INTRINSIC_WRAPPER_H
#define _NDS_VECTOR_INTRINSIC_WRAPPER_H

#if __riscv_v_intrinsic >= 11000
#include <rvv-non-policy_intrin0p1.h>
#include <rvv-overloaded-non-policy_intrin0p1.h>
#include <rvv-overloaded-policy_intrin0p1.h>
#include <rvv-policy_intrin0p1.h>
#endif

#endif
