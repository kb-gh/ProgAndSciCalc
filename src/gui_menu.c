/*****************************************************************************
 * File gui_menu.c part of ProgAndSciCalc
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


#include "gui_internal.h"

GtkWidget *create_menu(void)
{
    GtkWidget *menu_vbox;
    GtkWidget *menubar;

    menu_vbox = gtk_vbox_new(FALSE, 0);
    menubar = gtk_menu_bar_new();

    /* Options */
    options_menu_add(menubar);

    /* Help */
    help_menu_add(menubar);

    /* Unit Conversions */
    conversion_menu_add(menubar);

    /* Constants */
    constants_menu_add(menubar);

    gtk_box_pack_start(GTK_BOX(menu_vbox), menubar, FALSE, FALSE, 0);
    gtk_widget_show_all(menu_vbox);

    return menu_vbox;
}
