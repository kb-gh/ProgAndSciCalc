/*****************************************************************************
 * File config.h part of ProgAndSciCalc
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


#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>
#include "gui.h"
#include "calc.h"

#define MAIN_FONTSIZE_DEFAULT 22
#define MAIN_FONTSIZE_MIN 16
#define MAIN_FONTSIZE_MAX 32

#define BIN_FONTSIZE_DEFAULT 9
#define BIN_FONTSIZE_MIN 8
#define BIN_FONTSIZE_MAX 14

#define BUT_HEIGHT_DEFAULT 28
#define BUT_HEIGHT_MIN 20
#define BUT_HEIGHT_MAX 50

#define RANDOM_N_MIN 1
#define RANDOM_N_MAX 10000
#define RANDOM_N_DEFAULT 100


void config_init(void);
void config_save(void);
const char *config_get_dirname(void);

/* specifies integer or floating mode at startup */
void config_set_calc_mode(calc_mode_enum mode);
calc_mode_enum config_get_calc_mode(void);

/* specifies the number of digits to use at startup */
void config_set_float_digits(float_digits_enum fd);
float_digits_enum config_get_float_digits(void);

/* specifies the integer width at startup */
void config_set_integer_width(calc_width_enum width);
calc_width_enum config_get_integer_width(void);

/* specifies signed/unsigned at startup */
void config_set_use_unsigned(bool en);
bool config_get_use_unsigned(void);


/* Changes to all the following will also take effect while calc is running */

void config_set_main_disp_fontsize(int fs);
int config_get_main_disp_fontsize(void);

void config_set_bin_disp_fontsize(int fs);
int config_get_bin_disp_fontsize(void);

void config_set_but_height(int h);
int config_get_but_height(void);

void config_set_hex_grouping(int hg);
int config_get_hex_grouping(void);

void config_set_random_01(bool en);
bool config_get_random_01(void);

void config_set_random_n(int n);
int config_get_random_n(void);

void config_set_use_sct_rounding(bool en);
bool config_get_use_sct_rounding(void);

void config_set_warn_on_signed_overflow(bool en);
bool config_get_warn_on_signed_overflow(void);

void config_set_warn_on_unsigned_overflow(bool en);
bool config_get_warn_on_unsigned_overflow(void);
#endif
