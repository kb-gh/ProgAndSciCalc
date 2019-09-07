/*****************************************************************************
 * File display.h part of ProgAndSciCalc
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


#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdbool.h>
#include "calc.h"

typedef enum
{
    disp_mode_int,
    disp_mode_float,
} disp_mode_enum;

typedef enum
{
    disp_int_dec,
    disp_int_hex,
} disp_int_format_enum;

/* Use %g or %e */
typedef enum
{
    disp_float_gmode,
    disp_float_emode,
} disp_float_format_enum;

/* init, provide hex_grouping */
void display_init(int hg);

/* Display msg, this will automatically disable exponent entry mode. */
void display_set_text(const char *msg);

/* Get the text currently being displayed. */
const char *display_get_text(void);

/* Add digit, filters out excessive digits, leading zeros and multiple
 * dec points. Automatically does the right thing in exponent entry mode.
 * Returns true in one specific case, if in signed integer dec mode and user
 * enters the +ve equivalent of int_min eg. for width=8, user wants
 * to enter -128, which normally would be [1][2][8][+/-], but can't
 * enter 128 as it is out of range (max 127). This specific case
 * is recognised and it returns true when it sees 128 (or the appropriate
 * value for the other widths), for all other cases returns false. */
bool display_add(char c);

/* Remove last digit entered, automatically does the right thing in
 * exponent entry mode. */
void display_remove(void);

/* Get the display txt length. */
int display_get_text_length(void);

/* Set display into int or float mode */
void display_set_mode(disp_mode_enum mode);

/* Set dec or hex when in int mode */
void display_set_int_format(disp_int_format_enum format);

/* Set/get float format when in float mode */
void display_set_float_format(disp_float_format_enum format);
disp_float_format_enum display_get_float_format(void);

/* Add/remove minus sign, automatically does the right thing in
 * exponent entry mode. */
void disp_toggle_sign(void);

/* Display the value passed in, will use either ival or fval depending on
 * which mode is in use, in int mode will use dec or hex depending on int
 * format in use, in float mode will use either gmode or emode depending on
 * float format in use. It will automatically disable exponent entry mode. */
void display_set_val(uint64_t ival, stackf_t fval);

/* Get the value currently on the display, as ival if in int mode (fval will
 * be set to zero), or fval if in float mode (ival will be set to zero). */
void display_get_val(uint64_t *ival, stackf_t *fval);

/* Set/get exponent entry mode - this affects how the display responds to
 * display_add, display_remove, display_toggle_sign. */
void display_set_exp_entry(bool enable);
bool display_get_exp_entry(void);

/* Set callbacks to report warnings/errors to gui. */
void disp_set_warn_callback(void (*fn)(const char *msg));
void disp_set_error_callback(void (*fn)(const char *msg));


/* For calculating the best integer from a floating point, when doing
 * mode switch from float to integer mode.
 * eg. enter 80, perform some operations like cube root, then cube back,
 * displays (rounded) as 80, but might actually be 79.99999999999999 etc.
 * So when converting to integer it would be truncated to 79, which looks
 * a bit wrong. So convert from the (rounded) value on the display rather
 * than from the underlying floating point value.
 * Returns true if the value is in range of the current calc integer
 * width/signedness, else false (in which case *val will be set to
 * either int_min/int_max (as appropriate for the width) if signed, or to
 * 0 or uint_max (as appropriate for the width) if unsigned).
 */
bool display_get_best_integer(uint64_t *val, calc_width_enum width, bool use_unsigned);

/* set the grouping for hex digits, 0, 4, or 8 */
void display_set_hex_grouping(int hg);

/* set number of digits used by the display in floating point mode */
void display_set_num_float_digits(int n);
#endif
