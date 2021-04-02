/*****************************************************************************
 * File gui_util.h part of ProgAndSciCalc
 *
 * Copyright (C) 2021 Ken Bromham
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


#ifndef GUI_UTIL_H
#define GUI_UTIL_H

#include <stdbool.h>
#include <gtk/gtk.h>


GtkWidget *gui_hseparator_new(void);
GtkWidget *gui_hbox_new(gboolean homogeneous, gint spacing);
GtkWidget *gui_vbox_new(gboolean homogeneous, gint spacing);
GtkWidget *gui_label_new(const gchar *str, gfloat xalign, gfloat yalign);

#endif

