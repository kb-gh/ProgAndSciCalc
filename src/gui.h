/*****************************************************************************
 * File gui.h part of ProgAndSciCalc
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


#ifndef GUI_H
#define GUI_H

typedef enum
{
    FLOAT_DIGITS_8_ID,
    FLOAT_DIGITS_10_ID,
    FLOAT_DIGITS_12_ID,
    FLOAT_DIGITS_14_ID,
    FLOAT_DIGITS_16_ID,
    FLOAT_DIGITS_18_ID,
    FLOAT_DIGITS_20_ID,
    NUM_FLOAT_DIGITS_ID
} float_digits_enum;


void gui_init(int debug_lvl, float_digits_enum fd);
void gui_create(void);

#endif
