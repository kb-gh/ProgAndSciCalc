/*****************************************************************************
 * File display_print.h part of ProgAndSciCalc
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


#ifndef DISPLAY_PRINT_H
#define DISPLAY_PRINT_H

#include "calc_types.h"

void display_print_gmode(char *msg, stackf_t fval, int max_digits);
void display_print_emode(char *msg, stackf_t fval, int max_digits);
#endif
