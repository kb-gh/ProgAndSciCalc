/*****************************************************************************
 * File calc.h part of ProgAndSciCalc
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


#ifndef CALC_H
#define CALC_H

#include <stdbool.h>
#include "calc_types.h"

/* operations to pass into calculator */
typedef enum
{
    cop_nop = 0, /* do nothing */
    cop_peek,    /* just calls back with top of stack */

    cop_eq,      /* equals */

    /* binary ops */
    cop_add,
    cop_sub,
    cop_mul,
    cop_div,
    cop_mod,     /* modulo */
    cop_pow,     /* x to power y */
    cop_root,    /* x root 3 is cube root of x etc. */
    cop_and,
    cop_or,
    cop_xor,
    cop_lsftn,    /* left shift n places*/
    cop_rsftn,    /* right shift n places */

    /* unary ops */
    cop_pm,      /* plus/minus, essentially a unary minus */
    cop_com,     /* complement (invert bits) */
    cop_sqr,     /* square */
    cop_sqrt,    /* square root */
    cop_2powx,   /* 2 to power x (currently unused) */
    cop_onedx,   /* 1/x */
    cop_log,     /* log10 */
    cop_inv_log, /* inv log10 */
    cop_ln,      /* ln (base e) */
    cop_inv_ln,  /* inv ln */
    cop_sin,
    cop_inv_sin,
    cop_cos,
    cop_inv_cos,
    cop_tan,
    cop_inv_tan,
    cop_sinh,
    cop_inv_sinh,
    cop_cosh,
    cop_inv_cosh,
    cop_tanh,
    cop_inv_tanh,
    cop_lsft,    /* left shift 1 place */
    cop_rsft,    /* right shift 1 place */
    cop_fact,    /* factorial */

    cop_pi,   /* PI */
    cop_eul,  /* euler's number e (currently unused) */

    cop_parl, /* parentheses left */
    cop_parr, /* parentheses right */

    cop_ms,   /* memory store */
    cop_mr,   /* memory recall */
    cop_mp,   /* memory plus */
    cop_ms2,   /* memory store2 */
    cop_mr2,   /* memory recall2 */
    cop_mp2,   /* memory plus2 */

    cop_rand, /* random number */
    cop_gcd,  /* greatest common divisor */

    cop_rol,  /* rotate (circular shift) left 1 place */
    cop_ror,  /* rotate (circular shift) right 1 place */

    cop_int_min, /* get calculator to enter int_min */

} calc_op_enum;

typedef enum
{
    calc_mode_integer,
    calc_mode_float,
} calc_mode_enum;


typedef enum
{
    calc_angle_deg,
    calc_angle_rad,
    calc_angle_grad,
} calc_angle_enum;

/* widths for integer mode */
typedef enum
{
    calc_width_8,
    calc_width_16,
    calc_width_32,
    calc_width_64,
    num_calc_widths
} calc_width_enum;


/* One off initialisation at startup. */
void calc_init(int debug_lvl,
               calc_mode_enum mode,
               int rand_range,
               bool sct_round,
               calc_width_enum width,
               bool int_unsigned,
               bool warn_signed,
               bool warn_unsigned);

/* Clear/reset calc. Can be used at any time, in addition should be used
 * before starting for the first time and following a mode switch. */
void calc_clear(void);

/* Give value entered by user to calculator.
 * If calc is in integer mode, ival is used, fval don't care (suggest set to dfp zero).
 * If calc is in float mode, fval is used, ival don't care (suggest set to 0). */
void calc_give_arg(uint64_t ival, stackf_t fval);

/* Give operation to calculator. */
void calc_give_op(calc_op_enum cop);

/* Set/get the current mode (integer or float) */
void calc_set_mode(calc_mode_enum mode);
calc_mode_enum calc_get_mode(void);

/* Set/get the current angle units */
void calc_set_angle(calc_angle_enum angle);
calc_angle_enum calc_get_angle(void);

/* Set/get repeated equals option */
void calc_set_repeated_equals(bool enable);
bool calc_get_repeated_equals(void);

/* Set callback that calculator uses to return result to gui. */
void calc_set_result_callback(void (*fn)(uint64_t, stackf_t));

/* Set callback that calculator uses to return value that you might
 * want to add to history (optional). */
void calc_set_history_callback(void (*fn)(uint64_t, stackf_t));

/* Set callback that calculator uses to report parentheses level. */
void calc_set_num_paren_callback(void (*fn)(int));

/* Set callback for getting the best integer value when converting
 * from a floating point value to integer (during mode switch) */
void calc_set_get_best_integer_callback(bool (*fn)(uint64_t *, bool *));

/* Set callbacks to report warnings/errors to gui. */
void calc_set_warn_callback(void (*fn)(const char *msg));
void calc_set_error_callback(void (*fn)(const char *msg));

/* return the binary op at the top of the bop stack, or cop_nop if empty */
calc_op_enum calc_get_top_of_bop_stack(void);

/* range == 0 means 0 <= r < 1
 * range > 0 means  1 <= r <= range (r will be integer) */
void calc_set_random_range(int range);

/* use extra rounding for sin cos tan */
void calc_set_use_sct_rounding(bool en);
bool calc_get_use_sct_rounding(void);

/* Is there a non zero value stored in memory m */
bool calc_get_mem_non_zero(unsigned int m);

/* select signed or unsigned for integer mode */
void calc_set_use_unsigned(bool en);
bool calc_get_use_unsigned(void);

/* set integer width to 8, 16, 32 or 64 bits */
void calc_set_integer_width(calc_width_enum width);
calc_width_enum calc_get_integer_width(void);

void calc_set_warn_on_signed_overflow(bool en);
bool calc_get_warn_on_signed_overflow(void);

void calc_set_warn_on_unsigned_overflow(bool en);
bool calc_get_warn_on_unsigned_overflow(void);

/* special case for toggling bits in the binary display */
void calc_binary_bit_xor(uint64_t bitmask);

/* Convert content of str using strtoull.
 * The result is returned in *val and is truncated according to the width.
 * Return false if the strtoull converison overflows or if the result
 * was outside the range of the width. */
bool calc_util_unsigned_str_to_ival(const char *str,
                                    calc_width_enum width,
                                    uint64_t *val,
                                    int base);

/* Convert content of str using strtoll.
 * The result is returned in *val and is truncated according to the width.
 * Return false if the strtoll converison overflows or if the result
 * was outside the range of the width. */
bool calc_util_signed_str_to_ival(const char *str,
                                  calc_width_enum width,
                                  uint64_t *val,
                                  int base);


/* The values in the uint64 are always masked off to the width eg. for
 * width=8, a value of -1 is held as 0x00000000000000ff. This function
 * takes that uint64 value and converts to int64 with sign extension,
 * so afterwards the int64 would hold 0xffffffffffffffff */
int64_t calc_util_get_signed(uint64_t x, calc_width_enum width);

/* is x in the signed range of the width (useful for widths 8,16,32,
 * will always return true for width 64) */
bool calc_util_is_in_signed_range(int64_t x, calc_width_enum width);

/* is x in the unsigned range of the width (useful for widths 8,16,32,
 * will always return true for width 64) */
bool calc_util_is_in_unsigned_range(uint64_t x, calc_width_enum width);

/* mask off x to the width */
void calc_util_mask_width(uint64_t *x, calc_width_enum width);


/* When changing from float to integer mode, or pasting in a value in
 * integer mode, decide if the current integer width is enough, or does
 * it need to be increased.
 * uval is either a positive value in range of u64 (negative==false) or
 * represents a negative value in range of s64 (negative==true).
 * A value in the range of INT8_MIN up to UINT8_MAX requires width 8.
 * A value in the range of INT16_MIN up to UINT16_MAX requires width 16 etc.
 * Return the width required, which will be >= current_width.*/
calc_width_enum calc_util_get_changed_width(uint64_t uval, bool negative,
                                            calc_width_enum current_width);
#endif
