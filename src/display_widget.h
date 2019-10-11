/*****************************************************************************
 * File display_widget.h part of ProgAndSciCalc
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


#ifndef DISPLAY_WIDGET_H
#define DISPLAY_WIDGET_H

#include <gtk/gtk.h>
#include "calc.h"



GtkWidget *display_widget_create(const char *main_msg, const char *bin_msg);

/* set text on the main display */
void display_widget_main_set_text(const char *msg);

/* set value on the binary display */
void display_widget_bin_set_val(uint64_t ival, calc_width_enum width);

#endif
