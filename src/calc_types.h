/*****************************************************************************
 * File calc_types.h part of ProgAndSciCalc
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


#ifndef CALC_TYPES_H
#define CALC_TYPES_H

#include "decNumber/decQuad.h"
#include "decNumber/decimal128.h" // interface to decNumber

extern decContext dfp_context;


/* For integer mode, all values are stored and passed around as a uint64_t.
 * There is an assumption of a twos complement target where the bit
 * pattern is the same for unsigned/signed, and in particular a conversion
 * from unsigned to signed will never do anything unexpected (ie. for the
 * case where the unsigned value is outside the range of the signed
 * type). */

/* The type used for floating point mode */
typedef decQuad stackf_t;

/* make it easier to change type from eg. decQuad to decDouble */
#define dfp_add(r, a, b, c) decQuadAdd(r, a, b, c)
#define dfp_subtract(r, a, b, c) decQuadSubtract(r, a, b, c)
#define dfp_multiply(r, a, b, c) decQuadMultiply(r, a, b, c)
#define dfp_divide(r, a, b, c) decQuadDivide(r, a, b, c)
#define dfp_remainder(r, a, b, c) decQuadRemainder(r, a, b, c)
#define dfp_minus(r, a, c) decQuadMinus(r, a, c)
#define dfp_zero(r) decQuadZero(r)
#define dfp_abs(r, a, c) decQuadAbs(r, a, c)

#define dfp_to_string(a, s)  decQuadToString(a, s)
#define dfp_from_string(r, s, c) decQuadFromString(r, s, c)

#define dfp_to_number(a, n) decQuadToNumber(a, n)
#define dfp_from_number(a, n, c)  decQuadFromNumber(a, n, c)

#define dfp_compare(r, lhs, rhs, c) decQuadCompare(r, lhs, rhs, c)
#define dfp_is_negative(a) decQuadIsNegative(a)
#define dfp_is_integer(a) decQuadIsInteger(a)
#define dfp_is_zero(a) decQuadIsZero(a)
#define dfp_is_infinite(a)  decQuadIsInfinite(a)
#define dfp_is_nan(a) decQuadIsNaN(a)

#define dfp_to_int32(a, c, round) decQuadToInt32(a, c, round)
#define dfp_from_int32(a, i) decQuadFromInt32(a, i)

#define dfp_context_clear_status(c) ((c)->status = 0)

/* 43 chars */
#define DFP_STRING_MAX  DECQUAD_String

#endif

