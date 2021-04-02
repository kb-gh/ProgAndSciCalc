/*****************************************************************************
 * File gui_history.c part of ProgAndSciCalc
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


#include <string.h>
#include <inttypes.h>
#include "gui_internal.h"
#include "display_print.h"

static GtkWidget *window_hist_float;
static GtkWidget *window_hist_int;

#define HISTORY_MAX_ITEMS 50
#define MOUSE_LEFT_BUT 1

static stackf_t history_fval[HISTORY_MAX_ITEMS];
static stackf_t history_fval_copy[HISTORY_MAX_ITEMS];

/* for integer mode, store signedness */
typedef struct
{
    bool is_unsigned;
    uint64_t ival;
} hist_int_t;

static hist_int_t history_ival[HISTORY_MAX_ITEMS];
static hist_int_t history_ival_copy[HISTORY_MAX_ITEMS];

/* To avoid casting int to pointer in callback user data, if it wants a
 * pointer then, OK, it can have a pointer to one of these */
static int row_cb[HISTORY_MAX_ITEMS];

static const char *click_val_msg = "Click on a value to return that value to the calculator";


/****************************************************************************
 * Integer
 *
 */

static void hist_destroy_int(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;

    window_hist_int = NULL;
}

static void cancel_button_clicked_int(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;

    gtk_widget_destroy(window_hist_int);
}

static void entry_enter_pressed_int(GtkWidget *widget, gpointer data)
{
    (void)widget;
    int row = *(int *)data;

    /* user may have switched mode to float, the result is only
     * intended to be used in integer mode */
    if (calc_get_mode() == calc_mode_integer)
    {
        stackf_t dzero;
        dfp_zero(&dzero);
        calc_give_arg(history_ival_copy[row].ival, dzero);
        calc_give_op(cop_peek);
        gtk_widget_destroy(window_hist_int);
    }
}

static gboolean entry_button_release_int(GtkWidget *widget,
                                         GdkEventButton *event,
                                         gpointer data)
{
    (void)widget;
    int row = *(int *)data;

    if (event->button == MOUSE_LEFT_BUT)
    {
        //printf("left button clicked on row %d\n", row);

        /* user may have switched mode to float, the result is only
         * intended to be used in integer mode */
        if (calc_get_mode() != calc_mode_integer)
            return FALSE;

        stackf_t dzero;
        dfp_zero(&dzero);
        calc_give_arg(history_ival_copy[row].ival, dzero);
        calc_give_op(cop_peek);
        gtk_widget_destroy(window_hist_int);
        return TRUE;
    }

    return FALSE;
}

#define TEXT_LEN_INT 42
static GtkWidget *create_table_int(void)
{
    int row;
    GtkWidget *table;
    GtkWidget *entry;
    /* 20 digits dec, 16 + 2 hex, 2 spaces, 2 brackets, this size is enough */
    char buf[50];

#if TARGET_GTK_VERSION == 2
    table = gtk_table_new(HISTORY_MAX_ITEMS, 1, TRUE);
#elif TARGET_GTK_VERSION == 3
    table = gtk_grid_new();
    gtk_grid_set_row_homogeneous(GTK_GRID(table), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(table), TRUE);
#endif

    for (row = 0; row < HISTORY_MAX_ITEMS; row++)
    {
        /* entry to display values */
        entry = gtk_entry_new();
        if (history_ival_copy[row].is_unsigned)
        {
            sprintf(buf, "%" PRIu64 "  (0x%" PRIX64 ")",
                    history_ival_copy[row].ival, history_ival_copy[row].ival);
        }
        else
        {
            sprintf(buf, "%" PRId64 "  (0x%" PRIX64 ")",
                    (int64_t)history_ival_copy[row].ival, history_ival_copy[row].ival);
        }
        gtk_entry_set_text(GTK_ENTRY(entry), buf);
#if TARGET_GTK_VERSION == 2
        gtk_entry_set_editable(GTK_ENTRY(entry), FALSE);
#elif TARGET_GTK_VERSION == 3
        gtk_editable_set_editable(GTK_EDITABLE(entry), FALSE);
#endif
        gtk_entry_set_max_length(GTK_ENTRY(entry), TEXT_LEN_INT);
        gtk_entry_set_width_chars(GTK_ENTRY(entry), TEXT_LEN_INT);
        /* want to return value to calc either by clicking mouse on the value
         * (button-release) or by using keyboard to give focus to the value
         * and press enter (activate) */
        g_signal_connect(entry, "activate",
                         G_CALLBACK(entry_enter_pressed_int), (gpointer)&row_cb[row]);
        gtk_widget_add_events(entry, GDK_BUTTON_RELEASE_MASK);
        g_signal_connect(entry, "button-release-event",
                         G_CALLBACK(entry_button_release_int), (gpointer)&row_cb[row]);
#if TARGET_GTK_VERSION == 2
        gtk_table_attach_defaults(GTK_TABLE(table), entry,
                                  0, 1, row, row+1);
#elif TARGET_GTK_VERSION == 3
        gtk_grid_attach(GTK_GRID(table), entry, 0, row, 1, 1);
#endif
    }

    return table;
}

static gboolean key_press_int(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    (void)widget;
    (void)data;

    switch (event->keyval)
    {
        case 'h':
        case 'H':
            gtk_widget_destroy(window_hist_int);
            return TRUE;
        default:
            return FALSE;
    }
}

static void history_open_int(void)
{
    if (window_hist_int != NULL)
    {
        /* the HIST button will toggle history window open/close, so close it if
         * the window is open */
        gtk_widget_destroy(window_hist_int);
        return;
    }

    GtkWidget *table;
    GtkWidget *vbox;
    GtkWidget *vbox_table;
    GtkWidget *separator;
    GtkWidget *button;
    GtkWidget *label;
    GtkWidget *scrolled;

    window_hist_int = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    /* So you can use the 'h' keyboard shortcut to open the hist window,
     * which now has focus, then press 'h' again to close it */
    g_signal_connect(window_hist_int, "key_press_event",
                     G_CALLBACK(key_press_int), NULL);

    /* take copy at the time the window is created */
    memcpy(history_ival_copy, history_ival, sizeof(history_ival_copy));

    gtk_window_set_title(GTK_WINDOW(window_hist_int), "History (Integer)");
    g_signal_connect(window_hist_int, "destroy",
                     G_CALLBACK(hist_destroy_int), NULL);
    gtk_container_set_border_width(GTK_CONTAINER(window_hist_int), 10);

    /* main vbox */
    vbox = gui_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window_hist_int), vbox);

    /* vbox for table, scrolled */
    vbox_table = gui_vbox_new(FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(vbox_table), 10);

    scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(scrolled, -1, 400);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    /* Add description label */
    label = gui_label_new(click_val_msg, 0.5, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

    separator = gui_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 5);

    /* Add the table */
    table = create_table_int();
    gtk_box_pack_start(GTK_BOX(vbox_table), table, TRUE, TRUE, 5);
#if TARGET_GTK_VERSION == 2
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled), vbox_table);
#elif TARGET_GTK_VERSION == 3
    gtk_container_add(GTK_CONTAINER(scrolled), vbox_table);
#endif
    gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);

    /* Cancel button */
    button = gtk_button_new_with_mnemonic("_Cancel");
    g_signal_connect(button, "clicked",
        G_CALLBACK(cancel_button_clicked_int), NULL);
    gtk_widget_set_size_request(button, 80, -1);
#if TARGET_GTK_VERSION == 2
    GtkWidget *align = gtk_alignment_new(1, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), button);
    gtk_box_pack_start(GTK_BOX(vbox), align, FALSE, FALSE, 10);
#elif TARGET_GTK_VERSION == 3
    gtk_widget_set_halign(button, GTK_ALIGN_END);
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 10);
#endif

    gtk_widget_show_all(window_hist_int);
}

static void history_add_int(uint64_t ival)
{
    if (ival == 0)
    {
        return;
    }

    uint64_t res;
    bool is_unsigned = calc_get_use_unsigned();

    if (is_unsigned)
    {
        res = ival;
    }
    else
    {
        /* sign extend in case it's negative */
        calc_width_enum width = calc_get_integer_width();
        int64_t si = calc_util_get_signed(ival, width);
        res = si;
    }

#ifdef HISTORY_FILTER_OUT_DUPLICATES
    /* filter out consecutive same values */
    if (res == history_ival[0].ival &&
        (res <= INT64_MAX || (is_unsigned == history_ival[0].is_unsigned)))
    {
        return;
    }
#endif

    /* only a small number so just shuffle along */
    for (int i = HISTORY_MAX_ITEMS - 1; i > 0; i--)
    {
        history_ival[i] = history_ival[i - 1];
    }
    history_ival[0].ival = res;
    history_ival[0].is_unsigned = is_unsigned;
}

/****************************************************************************
 * Floating
 *
 */

static void hist_destroy_float(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;

    window_hist_float = NULL;
}

static void cancel_button_clicked_float(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;

    gtk_widget_destroy(window_hist_float);
}

static void entry_enter_pressed_float(GtkWidget *widget, gpointer data)
{
    (void)widget;
    int row = *(int *)data;

    /* user may have switched mode to integer, the result is only
     * intended to be used in float mode */
    if (calc_get_mode() == calc_mode_float)
    {
        calc_give_arg(0, history_fval_copy[row]);
        calc_give_op(cop_peek);
        gtk_widget_destroy(window_hist_float);
    }
}

static gboolean entry_button_release_float(GtkWidget *widget,
                                           GdkEventButton *event,
                                           gpointer data)
{
    (void)widget;
    int row = *(int *)data;

    if (event->button == MOUSE_LEFT_BUT)
    {
        //printf("left button clicked on row %d\n", row);

        /* user may have switched mode to integer, the result is only
         * intended to be used in float mode */
        if (calc_get_mode() != calc_mode_float)
            return FALSE;

        calc_give_arg(0, history_fval_copy[row]);
        calc_give_op(cop_peek);
        gtk_widget_destroy(window_hist_float);
        return TRUE;
    }

    return FALSE;
}

#define TEXT_LEN_FLOAT 30
#define MAX_FLOAT_DIGITS_HIST 20
static GtkWidget *create_table_float(void)
{
    int row;
    GtkWidget *table;
    GtkWidget *entry;
    char buf[DFP_STRING_MAX];

#if TARGET_GTK_VERSION == 2
    table = gtk_table_new(HISTORY_MAX_ITEMS, 1, TRUE);
#elif TARGET_GTK_VERSION == 3
    table = gtk_grid_new();
    gtk_grid_set_row_homogeneous(GTK_GRID(table), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(table), TRUE);
#endif

    for (row = 0; row < HISTORY_MAX_ITEMS; row++)
    {
        /* entry to display values */
        entry = gtk_entry_new();

        display_print_gmode(buf, history_fval_copy[row], MAX_FLOAT_DIGITS_HIST);

        gtk_entry_set_text(GTK_ENTRY(entry), buf);
#if TARGET_GTK_VERSION == 2
        gtk_entry_set_editable(GTK_ENTRY(entry), FALSE);
#elif TARGET_GTK_VERSION == 3
        gtk_editable_set_editable(GTK_EDITABLE(entry), FALSE);
#endif
        gtk_entry_set_max_length(GTK_ENTRY(entry), TEXT_LEN_FLOAT);
        gtk_entry_set_width_chars(GTK_ENTRY(entry), TEXT_LEN_FLOAT);
        /* want to return value to calc either by clicking mouse on the value
         * (button-release) or by using keyboard to give focus to the value
         * and press enter (activate) */
        g_signal_connect(entry, "activate",
                         G_CALLBACK(entry_enter_pressed_float), (gpointer)&row_cb[row]);
        gtk_widget_add_events(entry, GDK_BUTTON_RELEASE_MASK);
        g_signal_connect(entry, "button-release-event",
                         G_CALLBACK(entry_button_release_float), (gpointer)&row_cb[row]);
#if TARGET_GTK_VERSION == 2
        gtk_table_attach_defaults(GTK_TABLE(table), entry,
                                  0, 1, row, row+1);
#elif TARGET_GTK_VERSION == 3
        gtk_grid_attach(GTK_GRID(table), entry, 0, row, 1, 1);
#endif
    }

    return table;
}

static gboolean key_press_float(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    (void)widget;
    (void)data;

    switch (event->keyval)
    {
        case 'h':
        case 'H':
            gtk_widget_destroy(window_hist_float);
            return TRUE;
        default:
            return FALSE;
    }
}

static void history_open_float(void)
{
    if (window_hist_float != NULL)
    {
        /* the HIST button will toggle history window open/close, so close it if
         * the window is open */
        gtk_widget_destroy(window_hist_float);
        return;
    }

    GtkWidget *table;
    GtkWidget *vbox;
    GtkWidget *vbox_table;
    GtkWidget *separator;
    GtkWidget *button;
    GtkWidget *label;
    GtkWidget *scrolled;

    window_hist_float = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    /* So you can use the 'h' keyboard shortcut to open the hist window,
     * which now has focus, then press 'h' again to close it */
    g_signal_connect(window_hist_float, "key_press_event",
                     G_CALLBACK(key_press_float), NULL);

    /* take copy at the time the window is created */
    memcpy(history_fval_copy, history_fval, sizeof(history_fval_copy));

    gtk_window_set_title(GTK_WINDOW(window_hist_float), "History (Floating)");
    g_signal_connect(window_hist_float, "destroy",
                     G_CALLBACK(hist_destroy_float), NULL);
    gtk_container_set_border_width(GTK_CONTAINER(window_hist_float), 10);

    /* main vbox */
    vbox = gui_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window_hist_float), vbox);

    /* vbox for table, scrolled */
    vbox_table = gui_vbox_new(FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(vbox_table), 10);

    scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(scrolled, -1, 400);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    /* Add description label */
    label = gui_label_new(click_val_msg, 0.5, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

    separator = gui_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 5);

    /* Add the table */
    table = create_table_float();
    gtk_box_pack_start(GTK_BOX(vbox_table), table, TRUE, TRUE, 5);
#if TARGET_GTK_VERSION == 2
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled), vbox_table);
#elif TARGET_GTK_VERSION == 3
    gtk_container_add(GTK_CONTAINER(scrolled), vbox_table);
#endif
    gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);

    /* Cancel button */
    button = gtk_button_new_with_mnemonic("_Cancel");
    g_signal_connect(button, "clicked",
        G_CALLBACK(cancel_button_clicked_float), NULL);
    gtk_widget_set_size_request(button, 80, -1);
#if TARGET_GTK_VERSION == 2
    GtkWidget *align = gtk_alignment_new(1, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), button);
    gtk_box_pack_start(GTK_BOX(vbox), align, FALSE, FALSE, 10);
#elif TARGET_GTK_VERSION == 3
    gtk_widget_set_halign(button, GTK_ALIGN_END);
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 10);
#endif

    gtk_widget_show_all(window_hist_float);
}

static void history_add_float(stackf_t fval)
{
    if (dfp_is_zero(&fval))
    {
        return;
    }

#ifdef HISTORY_FILTER_OUT_DUPLICATES
    /* Filter out consecutive same values. Not a robust test of equal value
     * but good enough. */
    if (memcmp(&history_fval[0], &fval, sizeof(fval)) == 0)
    {
        return;
    }
#endif

    /* only a small number so just shuffle along */
    for (int i = HISTORY_MAX_ITEMS - 1; i > 0; i--)
    {
        history_fval[i] = history_fval[i - 1];
    }
    history_fval[0] = fval;
}


/****************************************************************************
 * Common
 *
 */

void gui_history_init(void)
{
    for(int i = 0; i < HISTORY_MAX_ITEMS; i++)
    {
        row_cb[i] = i;
        dfp_zero(&history_fval[i]);
    }
}

void gui_history_open(void)
{
    if (calc_get_mode() == calc_mode_integer)
    {
        history_open_int();
    }
    else
    {
        history_open_float();
    }
}

void gui_history_add(uint64_t ival, stackf_t fval)
{
    if (calc_get_mode() == calc_mode_integer)
    {
        history_add_int(ival);
    }
    else
    {
        history_add_float(fval);
    }
}
