/*****************************************************************************
 * File display_widget.c part of ProgAndSciCalc
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


#include <gtk/gtk.h>
#include <stdio.h>

#include "display_widget.h"
#include "config.h"

static GtkWidget *display;
static GtkWidget *bin_display;
static GtkWidget *vbox;

/* Used when the binary display is out of use (float mode) - the way I have
 * done the layout, it relies on the width of the text in the binary
 * display, so goes a bit wrong if it is empty.
 * (8 * 8) + 7 = 71 characters. */
static const char *bin_disp_unused =
    "                                                                       ";


GtkWidget *display_widget_create(const char *main_msg, const char *bin_msg)
{
    vbox = gtk_vbox_new(FALSE, 0);
    gtk_widget_show(vbox);

    char font_str[40];

    /* main display */
    display = gtk_label_new(main_msg);
    gtk_misc_set_alignment(GTK_MISC(display), 1.0, 0.5);
    gtk_widget_show(display);
    gtk_box_pack_start(GTK_BOX(vbox), display, FALSE, FALSE, 0);

    sprintf(font_str, "mono bold %d", config_get_main_disp_fontsize());
    PangoFontDescription *pfd =
        pango_font_description_from_string(font_str);
    gtk_widget_modify_font(display, pfd);
    pango_font_description_free(pfd);

    /* secondary binary display */
    if (bin_msg == NULL)
        bin_msg = bin_disp_unused;
    bin_display = gtk_label_new(bin_msg);
    gtk_misc_set_alignment(GTK_MISC(bin_display), 1.0, 0.5);
    gtk_widget_show(bin_display);
    gtk_box_pack_start(GTK_BOX(vbox), bin_display, FALSE, FALSE, 0);

    sprintf(font_str, "mono %d", config_get_bin_disp_fontsize());
    pfd = pango_font_description_from_string(font_str);
    gtk_widget_modify_font(bin_display, pfd);
    pango_font_description_free(pfd);

    return vbox;
}

void display_widget_main_set_text(const char *msg)
{
    gtk_label_set_text(GTK_LABEL(display), msg);
}

void display_widget_bin_set_text(const char *msg)
{
    if (msg == NULL)
        msg = bin_disp_unused;
    gtk_label_set_text(GTK_LABEL(bin_display), msg);
}
