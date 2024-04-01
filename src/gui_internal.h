/*****************************************************************************
 * File gui_internal.h part of ProgAndSciCalc
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


#ifndef GUI_INTERNAL_H
#define GUI_INTERNAL_H

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <gtk/gtk.h>

#include "gui.h"
#include "gui_util.h"
#include "display.h"
#include "display_widget.h"
#include "calc.h"
#include "config.h"


/* Info for number of float digits radio buttons */
typedef struct
{
    char *name;
    float_digits_enum id;
} FLOAT_DIGITS_RB;
extern const FLOAT_DIGITS_RB float_digits_rb[NUM_FLOAT_DIGITS_ID];


/* Info for integer width radio buttons */
typedef struct
{
    char *name;
    calc_width_enum id;
} INT_WIDTH_RB;
extern const INT_WIDTH_RB int_width_rb[num_calc_widths];

/* Info for integer signed/unsigned radio buttons */
typedef struct
{
    char *name;
    int id;
} INT_SIGNED_RB;
#define INT_USE_SIGNED_ID 0
#define INT_USE_UNSIGNED_ID 1
#define NUM_INT_SIGNED_RB 2
extern const INT_SIGNED_RB int_signed_rb[NUM_INT_SIGNED_RB];

/* Message used by various gui files */
extern const char *gui_click_val_msg;

GtkWidget *create_menu(void);

void options_menu_add(GtkWidget *menubar);
void help_menu_add(GtkWidget *menubar);
void conversion_menu_add(GtkWidget *menubar);
void constants_menu_add(GtkWidget *menubar);

void gui_menu_constants_init(void);
void gui_menu_constants_deinit(void);

void gui_history_init(void);
void gui_history_open(void);
void gui_history_add(uint64_t ival, stackf_t fval);

#endif
