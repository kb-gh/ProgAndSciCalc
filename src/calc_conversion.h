/*****************************************************************************
 * File calc_conversion.h part of ProgAndSciCalc
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


#ifndef CALC_CONVERSION_H
#define CALC_CONVERSION_H


/* Unit Conversion Categories */
typedef enum
{
    ucv_length,
    ucv_area,
    ucv_volume,
    ucv_mass,
    ucv_speed,
    ucv_pressure,
    ucv_force,
    ucv_energy,
    ucv_power,
    ucv_temperature,
    ucv_torque,
    ucv_fuel_economy,
    num_ucv
} ucv_enum;

/* Functions for unit conversions */
void calc_conversion_fix_value(ucv_enum category);
void calc_conversion_apply_result(ucv_enum category, int row);

int calc_conversion_get_num_rows(ucv_enum category);

void calc_conversion_set_convert_from_row(ucv_enum category, int row);
void calc_conversion_get_name(ucv_enum category, int row, char *buf, int buf_len);
void calc_conversion_get_value(ucv_enum category, int row, char *buf, int buf_len);

/* stick this one function in here as well, used from constants gui */
void calc_constants_apply_result(const char *value);
#endif
