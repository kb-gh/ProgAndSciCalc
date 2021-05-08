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
#include "calc.h"
#include "gui.h"
#include "gui_util.h"

static GtkWidget *display;
static GtkWidget *bin_display_top;
static GtkWidget *top_eventbox;
static GtkWidget *bin_display_bot;
static GtkWidget *bot_eventbox;

/* Used when the binary display is out of use (float mode) - fill with spaces
 * to avoid possibility of the layout size changing.
 * (9 * 4) + 6 = 42 characters. */
static const char *bin_disp_unused =
    "                                          ";

static bool top_pressed;
static bool bot_pressed;

/*
 * Details for the secondary display showing value in binary
 *
 * bbbb-bbbb  bbbb-bbbb  bbbb-bbbb  bbbb-bbbb  bbbb-bbbb  bbbb-bbbb  bbbb-bbbb  bbbb-bbbb
 *                                           ^^
 *                              will split here for top/bot displays
 */

/* 64 bit numbers */
#define MAX_BIN_BYTES 8
/* add a hyphen between each nybble within a byte, hence 9 chars */
#define MAX_BIN_CHARS (MAX_BIN_BYTES * 9)
/* 2 spaces between each byte (7 * 2) = 14 */
#define MAX_BIN_CHARS_PLUS_SPACES (MAX_BIN_CHARS + 14)
/* 1 for null termination */
#define MAX_BIN_CHARS_PLUS_SPACES_PLUS_NULL (MAX_BIN_CHARS_PLUS_SPACES + 1)
static char bin_disp_str_spaced[MAX_BIN_CHARS_PLUS_SPACES_PLUS_NULL];


/* return bit clicked (0 - 31), or -1 if not clicked on any bit */
static int get_bit_clicked(GtkWidget *widget, GdkEventButton *event)
{
    GtkAllocation alloc;
    gtk_widget_get_allocation(widget, &alloc);
    int dw = alloc.width;
    //int dh = alloc.height;
    //printf("*** display w=%d  h=%d\n", dw, dh);

    int x = event->x;
    //int y = event->y;
    //printf("position x=%d  y=%d\n", x, y);

    /* get the font width for one char (monospace). Will be same for top and bot. */
    int fw, fh;
    PangoLayout *pl = gtk_widget_create_pango_layout(bin_display_bot, "0");
    pango_layout_get_pixel_size(pl, &fw, &fh);
    g_object_unref(pl);
    //printf("font w=%d  h=%d\n", fw, fh);

    /*
     * The bounds of each byte within the display, byte0 is the rightmost byte.
     * Assumes a hyphen between each nybble, and 2 spaces between each byte.
     * bbbb-bbbb  bbbb-bbbb  bbbb-bbbb  bbbb-bbbb
     *
     * byte0_left = dw - (9 * fw);
     * byte1_left = dw - ((9 * 2 + 2) * fw);
     * byte2_left = dw - ((9 * 3 + 4) * fw);
     * byte3_left = dw - ((9 * 4 + 6) * fw);
     *
     * byte0_right = byte0_left + (9 * fw) - 1;
     * byte1_right = byte1_left + (9 * fw) - 1;
     * byte2_right = byte2_left + (9 * fw) - 1;
     * byte3_right = byte3_left + (9 * fw) - 1;
     */
    int byte_left[4];
    int byte_right[4];

    for (int i = 0; i < 4; i++)
    {
        byte_left[i] = dw - ((9 * (i+1) + (2*i)) * fw);
        byte_right[i] = byte_left[i] + (9 * fw) - 1;
    }

    int byte_selected = -1;

    for (int i = 0; i < 4; i++)
    {
        if (x >= byte_left[i] && x <= byte_right[i])
        {
            byte_selected = i;
            break;
        }
    }

    /* not clicked within any byte */
    if (byte_selected < 0)
        return -1;

    //printf("byte_selected %d\n", byte_selected);

    int bit_left[8];
    int bit_right[8];
    int marker = byte_right[byte_selected];
    bit_right[0] = marker;
    marker -= fw;
    bit_right[1] = marker;
    marker -= fw;
    bit_right[2] = marker;
    marker -= fw;
    bit_right[3] = marker;
    marker -= fw;
    marker -= fw; // for hyphen
    bit_right[4] = marker;
    marker -= fw;
    bit_right[5] = marker;
    marker -= fw;
    bit_right[6] = marker;
    marker -= fw;
    bit_right[7] = marker;

    for (int i = 0; i < 8; i++)
    {
        bit_left[i] = bit_right[i] - fw + 1;
    }

    int bit_selected = -1;

    for (int i = 0; i < 8; i++)
    {
        if (x >= bit_left[i] && x <= bit_right[i])
        {
            bit_selected = i;
            break;
        }
    }

    /* clicked on the hyphen */
    if (bit_selected < 0)
        return -1;

    //printf("bit_selected %d\n", bit_selected);

    return byte_selected * 8 + bit_selected;
}


#define MOUSE_LEFT_BUT 1
#define MOUSE_RIGHT_BUT 3

static gboolean button_release_top(GtkWidget *widget, GdkEventButton *event,
                                   gpointer user_data)
{
    (void)widget;
    (void)user_data;
    (void)event;
    top_pressed = false;
    return TRUE;
}

static gboolean button_press_top(GtkWidget *widget, GdkEventButton *event,
                                 gpointer user_data)
{
    (void)user_data;

    if (calc_get_mode() != calc_mode_integer)
        return TRUE;

    if (event->button != MOUSE_LEFT_BUT)
        return TRUE;

    if (calc_get_integer_width() != calc_width_64)
        return TRUE;

    if (top_pressed)
    {
        /* filter out repeated press without a release in between */
        return TRUE;
    }
    top_pressed = true;

    int bit_clicked = get_bit_clicked(widget, event);
    if (bit_clicked < 0)
        return TRUE;

    bit_clicked += 32;
    //printf("bit clicked %d\n", bit_clicked);

    gui_give_arg_if_pending();
    calc_binary_bit_xor(1ULL << bit_clicked);

    return TRUE;
}


static gboolean button_release_bot(GtkWidget *widget, GdkEventButton *event,
                                   gpointer user_data)
{
    (void)widget;
    (void)user_data;
    (void)event;
    bot_pressed = false;
    return TRUE;
}

static gboolean button_press_bot(GtkWidget *widget, GdkEventButton *event,
                                 gpointer user_data)
{
    (void)user_data;

    if (calc_get_mode() != calc_mode_integer)
        return TRUE;

    if (event->button != MOUSE_LEFT_BUT)
        return TRUE;

    if (bot_pressed)
    {
        /* filter out repeated press without a release in between */
        return TRUE;
    }
    bot_pressed = true;

    int bit_clicked = get_bit_clicked(widget, event);
    if (bit_clicked < 0)
        return TRUE;

    calc_width_enum calc_width = calc_get_integer_width();
    if (calc_width == calc_width_8)
    {
        if (bit_clicked > 7)
            return TRUE;
    }
    else if (calc_width == calc_width_16)
    {
        if (bit_clicked > 15)
            return TRUE;
    }

    //printf("bit clicked %d\n", bit_clicked);

    gui_give_arg_if_pending();
    calc_binary_bit_xor(1ULL << bit_clicked);

    return TRUE;
}

GtkWidget *display_widget_create(const char *main_msg, const char *bin_msg)
{
    GtkWidget *vbox;
    vbox = gui_vbox_new(FALSE, 0);
    gtk_widget_show(vbox);

    char font_str[100];

    /* main display */
    display = gui_label_new(main_msg, 1.0, 0.5);
    gtk_widget_show(display);
    gtk_box_pack_start(GTK_BOX(vbox), display, FALSE, FALSE, 0);

#if TARGET_GTK_VERSION == 2
    sprintf(font_str, "mono bold %d", config_get_main_disp_fontsize());
    PangoFontDescription *pfd =
        pango_font_description_from_string(font_str);
    gtk_widget_modify_font(display, pfd);
    pango_font_description_free(pfd);
#elif TARGET_GTK_VERSION == 3
    sprintf(font_str, "*{font-family: mono; font-weight: bold; font-size: %dpt;}", config_get_main_disp_fontsize());
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, font_str, -1, NULL);
    GtkStyleContext *context = gtk_widget_get_style_context(display);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(provider);
#endif

    /* secondary binary display */
    if (bin_msg == NULL)
        bin_msg = bin_disp_unused;

    top_eventbox = gtk_event_box_new();
    gtk_event_box_set_above_child(GTK_EVENT_BOX(top_eventbox), TRUE);
    gtk_event_box_set_visible_window(GTK_EVENT_BOX(top_eventbox), FALSE);

    bin_display_top = gui_label_new(bin_msg, 1.0, 0.5);
    gtk_widget_show(bin_display_top);
    gtk_widget_show(top_eventbox);

    gtk_container_add(GTK_CONTAINER(top_eventbox), bin_display_top);
    gtk_box_pack_start(GTK_BOX(vbox), top_eventbox, FALSE, FALSE, 0);


    bot_eventbox = gtk_event_box_new();
    gtk_event_box_set_above_child(GTK_EVENT_BOX(bot_eventbox), TRUE);
    gtk_event_box_set_visible_window(GTK_EVENT_BOX(bot_eventbox), FALSE);

    bin_display_bot = gui_label_new(bin_msg, 1.0, 0.5);
    gtk_widget_show(bin_display_bot);
    gtk_widget_show(bot_eventbox);

    gtk_container_add(GTK_CONTAINER(bot_eventbox), bin_display_bot);
    gtk_box_pack_start(GTK_BOX(vbox), bot_eventbox, FALSE, FALSE, 0);

#if TARGET_GTK_VERSION == 2
    sprintf(font_str, "mono %d", config_get_bin_disp_fontsize());
    pfd = pango_font_description_from_string(font_str);
    gtk_widget_modify_font(bin_display_top, pfd);
    gtk_widget_modify_font(bin_display_bot, pfd);
    pango_font_description_free(pfd);
#elif TARGET_GTK_VERSION == 3
    sprintf(font_str, "*{font-family: mono; font-size: %dpt;}", config_get_bin_disp_fontsize());
    provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, font_str, -1, NULL);
    context = gtk_widget_get_style_context(bin_display_top);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    context = gtk_widget_get_style_context(bin_display_bot);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(provider);
#endif

    gtk_widget_add_events(top_eventbox,
                          GDK_BUTTON_PRESS_MASK |
                          GDK_BUTTON_RELEASE_MASK);
    g_signal_connect(top_eventbox, "button-press-event",
                     G_CALLBACK(button_press_top), NULL);
    g_signal_connect(top_eventbox, "button-release-event",
                     G_CALLBACK(button_release_top), NULL);

    gtk_widget_add_events(bot_eventbox,
                          GDK_BUTTON_PRESS_MASK |
                          GDK_BUTTON_RELEASE_MASK);
    g_signal_connect(bot_eventbox, "button-press-event",
                     G_CALLBACK(button_press_bot), NULL);
    g_signal_connect(bot_eventbox, "button-release-event",
                     G_CALLBACK(button_release_bot), NULL);

    return vbox;
}


/* default monospace font puts dot in middle of zero. If you don't like it,
 * try replace zero with capital O */
#define MAX_REPLACE_SIZE 100
static void replace_zero_with_o(char *m, const char *msg)
{
    int count = 0;
    while(*msg && count < MAX_REPLACE_SIZE - 1)
    {
        if (*msg == '0')
        {
            *m = 'O';
        }
        else
        {
            *m = *msg;
        }
        m++;
        msg++;
        count++;
    }
    *m = '\0';
}

void display_widget_main_set_text(const char *msg)
{
    if (config_get_replace_zero_with_o())
    {
        char m[MAX_REPLACE_SIZE];
        replace_zero_with_o(m, msg);
        gtk_label_set_text(GTK_LABEL(display), m);
    }
    else
    {
        gtk_label_set_text(GTK_LABEL(display), msg);
    }
}

static void display_widget_bin_set_text_top(const char *msg)
{
    if (msg == NULL)
    {
        msg = bin_disp_unused;
    }
    if (config_get_replace_zero_with_o())
    {
        char m[MAX_REPLACE_SIZE];
        replace_zero_with_o(m, msg);
        gtk_label_set_text(GTK_LABEL(bin_display_top), m);
    }
    else
    {
        gtk_label_set_text(GTK_LABEL(bin_display_top), msg);
    }
}

static void display_widget_bin_set_text_bot(const char *msg)
{
    if (msg == NULL)
    {
        msg = bin_disp_unused;
    }
    if (config_get_replace_zero_with_o())
    {
        char m[MAX_REPLACE_SIZE];
        replace_zero_with_o(m, msg);
        gtk_label_set_text(GTK_LABEL(bin_display_bot), m);
    }
    else
    {
        gtk_label_set_text(GTK_LABEL(bin_display_bot), msg);
    }
}

static void set_bin_display(uint64_t ival, calc_width_enum width)
{
    char *p;
    int bytes;
    int unused_bytes;

    if (width == calc_width_8)
        bytes = 1;
    else if (width == calc_width_16)
        bytes = 2;
    else if (width == calc_width_32)
        bytes = 4;
    else
        bytes = 8;

    unused_bytes = MAX_BIN_BYTES - bytes;

    /* fill in 1s and 0s working backwards */
    p = bin_disp_str_spaced + MAX_BIN_CHARS_PLUS_SPACES_PLUS_NULL;
    for (int i = 0; i < bytes; i++)
    {
        if (i == 0)
        {
            /* null terminate */
            *--p = 0;
        }
        else
        {
            /* add 2 spaces between each byte */
            *--p = ' ';
            *--p = ' ';
        }
        /* nybble */
        for (int j = 0; j < 4; j++)
        {
            int digit = ival & 1;
            *--p = digit + '0';
            ival >>= 1;
        }
        /* add hyphen */
        *--p = '-';
        /* nybble */
        for (int j = 0; j < 4; j++)
        {
            int digit = ival & 1;
            *--p = digit + '0';
            ival >>= 1;
        }
    }
    for (int i = 0; i < unused_bytes; i++)
    {
        /* add spaces between each byte */
        *--p = ' ';
        *--p = ' ';
        /* 9 to allow for what would have been the hyphen between nybbles */
        for (int j = 0; j < 9; j++)
        {
            /* whatever character you want to use to represent unused bit */
            *--p = ' ';
        }
    }
    /* sanity check */
    if (ival != 0 || p != bin_disp_str_spaced)
    {
        fprintf(stderr, "display width mask error\n");
    }

    /* So the whole 64 bit number is in one array, now split into top and bot
     * display, top display is the higher 32 bits ie. the leftmost bits in the
     * array. */
    bin_disp_str_spaced[42] = '\0';
    display_widget_bin_set_text_top(bin_disp_str_spaced);
    display_widget_bin_set_text_bot(bin_disp_str_spaced + 44);
}

void display_widget_bin_set_val(uint64_t ival, calc_width_enum width)
{
    set_bin_display(ival, width);
}
