/*****************************************************************************
 * File calc_internal.h part of ProgAndSciCalc
 *
 * Copyright (C) 2018 Ken Bromham
 *
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */


#ifndef CALC_INTERNAL_H
#define CALC_INTERNAL_H

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>

#include "calc.h"


void calc_error(const char *msg);
void calc_warn(const char *msg);

stackf_t calc_get_fval_top_of_stack(void);

/* integer unary operators */
uint64_t iop_plusminus(uint64_t arg);
uint64_t iop_complement(uint64_t arg);
uint64_t iop_square(uint64_t arg);
//uint64_t iop_square_root(uint64_t arg);
//uint64_t iop_2powx(uint64_t arg);
uint64_t iop_left_shift(uint64_t arg);
uint64_t iop_right_shift(uint64_t arg);
uint64_t iop_rol(uint64_t arg);
uint64_t iop_ror(uint64_t arg);

/* integer binary operators */
uint64_t bin_iop_add(uint64_t a, uint64_t b);
uint64_t bin_iop_sub(uint64_t a, uint64_t b);
uint64_t bin_iop_mul(uint64_t a, uint64_t b);
uint64_t bin_iop_div(uint64_t a, uint64_t b);
uint64_t bin_iop_mod(uint64_t a, uint64_t b);
uint64_t bin_iop_and(uint64_t a, uint64_t b);
uint64_t bin_iop_or(uint64_t a, uint64_t b);
uint64_t bin_iop_xor(uint64_t a, uint64_t b);
uint64_t bin_iop_gcd(uint64_t a, uint64_t b);
uint64_t bin_iop_left_shift(uint64_t a, uint64_t b);
uint64_t bin_iop_right_shift(uint64_t a, uint64_t b);


/* float unary operators */
stackf_t fop_plusminus(stackf_t arg);
stackf_t fop_square(stackf_t arg);
stackf_t fop_square_root(stackf_t arg);
stackf_t fop_one_over_x(stackf_t arg);
stackf_t fop_log(stackf_t arg);
stackf_t fop_inv_log(stackf_t arg);
stackf_t fop_ln(stackf_t arg);
stackf_t fop_inv_ln(stackf_t arg);
stackf_t fop_sin(stackf_t arg);
stackf_t fop_inv_sin(stackf_t arg);
stackf_t fop_cos(stackf_t arg);
stackf_t fop_inv_cos(stackf_t arg);
stackf_t fop_tan(stackf_t arg);
stackf_t fop_inv_tan(stackf_t arg);
stackf_t fop_sinh(stackf_t arg);
stackf_t fop_inv_sinh(stackf_t arg);
stackf_t fop_cosh(stackf_t arg);
stackf_t fop_inv_cosh(stackf_t arg);
stackf_t fop_tanh(stackf_t arg);
stackf_t fop_inv_tanh(stackf_t arg);
stackf_t fop_fact(stackf_t arg);

/* float binary operators */
stackf_t bin_fop_add(stackf_t a, stackf_t b);
stackf_t bin_fop_sub(stackf_t a, stackf_t b);
stackf_t bin_fop_mul(stackf_t a, stackf_t b);
stackf_t bin_fop_div(stackf_t a, stackf_t b);
stackf_t bin_fop_mod(stackf_t a, stackf_t b);
stackf_t bin_fop_pow(stackf_t a, stackf_t b);
stackf_t bin_fop_root(stackf_t a, stackf_t b);


#endif

