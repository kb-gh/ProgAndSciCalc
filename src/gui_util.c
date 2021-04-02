/*****************************************************************************
 * File gui_util.c part of ProgAndSciCalc
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


#include "gui_util.h"

#if TARGET_GTK_VERSION != 2 && TARGET_GTK_VERSION != 3
#error "invalid GTK_TARGET_VERSION"
#endif


GtkWidget *gui_hseparator_new(void)
{
#if TARGET_GTK_VERSION == 2
    return gtk_hseparator_new();
#elif TARGET_GTK_VERSION == 3
    return gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
#endif
}

GtkWidget *gui_hbox_new(gboolean homogeneous, gint spacing)
{
#if TARGET_GTK_VERSION == 2
    return gtk_hbox_new(homogeneous, spacing);
#elif TARGET_GTK_VERSION == 3
    GtkWidget *hbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, spacing);
    if (homogeneous)
    {
        gtk_box_set_homogeneous(GTK_BOX(hbox), TRUE);
    }
    return hbox;
#endif
}

GtkWidget *gui_vbox_new(gboolean homogeneous, gint spacing)
{
#if TARGET_GTK_VERSION == 2
    return gtk_vbox_new(homogeneous, spacing);
#elif TARGET_GTK_VERSION == 3
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, spacing);
    if (homogeneous)
    {
        gtk_box_set_homogeneous(GTK_BOX(vbox), TRUE);
    }
    return vbox;
#endif
}

GtkWidget *gui_label_new(const gchar *str, gfloat xalign, gfloat yalign)
{
#if TARGET_GTK_VERSION == 2
    GtkWidget *lbl = gtk_label_new(str);
    gtk_misc_set_alignment(GTK_MISC(lbl), xalign, yalign);
    return lbl;
#elif TARGET_GTK_VERSION == 3
    GtkWidget *lbl = gtk_label_new(str);
    gtk_label_set_xalign(GTK_LABEL(lbl), xalign);
    gtk_label_set_yalign(GTK_LABEL(lbl), yalign);
    return lbl;
#endif
}
