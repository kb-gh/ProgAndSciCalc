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
#include "display.h"
#include "display_widget.h"
#include "calc.h"
#include "config.h"


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
