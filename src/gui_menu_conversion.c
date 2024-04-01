/*****************************************************************************
 * File gui_menu_conversion.c part of ProgAndSciCalc
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


#include <stdlib.h>
#include "gui_internal.h"
#include "calc_conversion.h"

/* window to display Conversion, will only ever allow one window to exist
 * at a time */
static GtkWidget *window_conversion;


typedef struct
{
    char *name;
    ucv_enum category;
} UCV_INFO;

static const UCV_INFO ucv_info[num_ucv] =
{
    { "length", ucv_length },
    { "area", ucv_area },
    { "volume", ucv_volume },
    { "mass", ucv_mass },
    { "speed", ucv_speed },
    { "pressure", ucv_pressure },
    { "force", ucv_force },
    { "energy", ucv_energy },
    { "power", ucv_power },
    { "temperature", ucv_temperature },
    { "torque", ucv_torque },
    { "fuel_economy", ucv_fuel_economy }
};


/* An assumption for the max number of rows in any conversion category */
#define MAX_CONVERSION_ROWS 20
/* Keep track of which value entry is on which row for radio button callback */
static GtkWidget *value_entries[MAX_CONVERSION_ROWS];

/* store the num_rows for the category currently selected */
static int num_rows;

/* store the category */
static ucv_enum current_category;

/* To change font properties of the label on the selected row */
#if TARGET_GTK_VERSION == 2
static const GdkColor col_selected = {0, 0xd000, 0x0000, 0x0000};
#elif TARGET_GTK_VERSION == 3
static GtkCssProvider *css_provider;
#endif

#define MAX_BUF_LEN 80

static void conversion_destroy(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;

#if TARGET_GTK_VERSION == 3
    g_object_unref(css_provider);
    css_provider = NULL;
#endif

    window_conversion = NULL;
}

/* Callback for close button on the Conversion window */
static void cancel_button_clicked(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;

    gtk_widget_destroy(window_conversion);
}


static void set_entry_col_normal(GtkWidget *lbl)
{
#if TARGET_GTK_VERSION == 2
    gtk_widget_modify_text(lbl, GTK_STATE_NORMAL, NULL);
#elif TARGET_GTK_VERSION == 3
    GtkStyleContext *context = gtk_widget_get_style_context(lbl);
    gtk_style_context_remove_provider(context, GTK_STYLE_PROVIDER(css_provider));
#endif
}

static void set_entry_col_selected(GtkWidget *lbl)
{
#if TARGET_GTK_VERSION == 2
    gtk_widget_modify_text(lbl, GTK_STATE_NORMAL, &col_selected);
#elif TARGET_GTK_VERSION == 3
    GtkStyleContext *context = gtk_widget_get_style_context(lbl);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(css_provider), GTK_STYLE_PROVIDER_PRIORITY_USER);
#endif
}

static void update_all_entry_values(void)
{
    char buf[MAX_BUF_LEN];
    for (int row = 0; row < num_rows; row++)
    {
        calc_conversion_get_value(current_category, row, buf, MAX_BUF_LEN);
        gtk_entry_set_text(GTK_ENTRY(value_entries[row]), buf);
    }
}

static void rb_toggle(GtkWidget *widget, gpointer data)
{
    int row = (int)(uintptr_t)data;

    gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    if (active)
    {
        //printf("row %d active\n", row);
        /* update the convert from row */
        calc_conversion_set_convert_from_row(current_category, row);
        update_all_entry_values();
        set_entry_col_selected(value_entries[row]);
    }
    else
    {
        //printf("row %d inactive\n", row);
        set_entry_col_normal(value_entries[row]);
    }
}


static void entry_enter_pressed(GtkWidget *widget, gpointer data)
{
    (void)widget;
    int row = (int)(uintptr_t)data;

    /* user may have switched mode to integer, the conversion result is only
     * intended to be used in float mode */
    if (calc_get_mode() == calc_mode_float)
    {
        calc_conversion_apply_result(current_category, row);
        gtk_widget_destroy(window_conversion);
    }
}

#define MOUSE_LEFT_BUT 1

static gboolean entry_button_release(GtkWidget *widget, GdkEventButton *event,
                                    gpointer data)
{
    (void)widget;
    int row = (int)(uintptr_t)data;

    if (event->button == MOUSE_LEFT_BUT)
    {
        //printf("left button clicked on row %d\n", row);

        /* user may have switched mode to integer, the conversion result is only
         * intended to be used in float mode */
        if (calc_get_mode() != calc_mode_float)
            return FALSE;

        calc_conversion_apply_result(current_category, row);
        gtk_widget_destroy(window_conversion);
        return TRUE;
    }

    return FALSE;
}

#define TEXT_LEN 22

static GtkWidget *create_table(ucv_enum category)
{
    int row;
    GtkWidget *table;
    GtkWidget *button = NULL;
    GtkWidget *entry;
    GSList *group;
    char buf[MAX_BUF_LEN];

    /* reset the convert from row to 0 */
    calc_conversion_set_convert_from_row(category, 0);

    /* get the number of rows for this category */
    num_rows = calc_conversion_get_num_rows(category);
    if (num_rows > MAX_CONVERSION_ROWS)
    {
        fprintf(stderr, "num conversion rows exceeded limit, ignoring extra rows \n");
        num_rows = MAX_CONVERSION_ROWS;
    }

#if TARGET_GTK_VERSION == 2
    table = gtk_table_new(num_rows, 2, TRUE);
#elif TARGET_GTK_VERSION == 3
    table = gtk_grid_new();
    gtk_grid_set_row_homogeneous(GTK_GRID(table), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(table), TRUE);
#endif

    for (row = 0; row < num_rows; row++)
    {
        /* radio button */
        calc_conversion_get_name(category, row, buf, MAX_BUF_LEN);
        if (row == 0)
        {
            button = gtk_radio_button_new_with_label(NULL, buf);
        }
        else
        {
            group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(button));
            button = gtk_radio_button_new_with_label(group, buf);
        }
#if TARGET_GTK_VERSION == 2
        gtk_table_attach_defaults(GTK_TABLE(table), button,
                                  0, 1, row, row+1);
#elif TARGET_GTK_VERSION == 3
        gtk_grid_attach(GTK_GRID(table), button, 0, row, 1, 1);
#endif

        /* entry to display values */
        calc_conversion_get_value(category, row, buf, MAX_BUF_LEN);
        entry = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(entry), buf);
#if TARGET_GTK_VERSION == 2
        gtk_entry_set_editable(GTK_ENTRY(entry), FALSE);
#elif TARGET_GTK_VERSION == 3
        gtk_editable_set_editable(GTK_EDITABLE(entry), FALSE);
#endif
        gtk_entry_set_max_length(GTK_ENTRY(entry), TEXT_LEN);
        gtk_entry_set_width_chars(GTK_ENTRY(entry), TEXT_LEN);
        /* want to return value to calc either by clicking mouse on the value
         * (button-release) or by using keyboard to give focus to the value
         * and press enter (activate) */
        g_signal_connect(entry, "activate",
                         G_CALLBACK(entry_enter_pressed), (gpointer)(uintptr_t)row);
        gtk_widget_add_events(entry, GDK_BUTTON_RELEASE_MASK);
        g_signal_connect(entry, "button-release-event",
                         G_CALLBACK(entry_button_release), (gpointer)(uintptr_t)row);
#if TARGET_GTK_VERSION == 2
        gtk_table_attach_defaults(GTK_TABLE(table), entry,
                                  1, 2, row, row+1);
#elif TARGET_GTK_VERSION == 3
        gtk_grid_attach(GTK_GRID(table), entry, 1, row, 1, 1);
#endif

        /* store entry pointer */
        value_entries[row] = entry;


        /* connect radio button toggled signal */
        g_signal_connect(button, "toggled",
                         G_CALLBACK(rb_toggle), (gpointer)(uintptr_t)row);
        if (row == 0)
        {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
            set_entry_col_selected(entry);
        }
    }

    return table;
}


/* Callback for menu conversion, create Conversion window for the
 * category selected */
static void ucv_activate(GtkWidget *widget, gpointer data)
{
    (void)widget;
    UCV_INFO *info = data;

    if (window_conversion != NULL)
    {
        /* a conversion window already exists */
        return;
    }

#if TARGET_GTK_VERSION == 3
    css_provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(css_provider, "*{font-style: italic; color: #d00000}", -1, NULL);
#endif

    /* The conversion routine will take the current calc top of stack as the
     * value to be converted. The user, however, may have entered a value
     * which has yet to be passed into calc, in which case pass it in now. */
    gui_give_arg_if_pending();
    /* Fix the value to convert now, before creating table. */
    calc_conversion_fix_value(info->category);
    current_category = info->category;

    GtkWidget *table;
    GtkWidget *vbox;
    GtkWidget *separator;
    GtkWidget *button;
    GtkWidget *label;
    char buf[80];

    window_conversion = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    sprintf(buf, "Unit Conversion %s", info->name);
    gtk_window_set_title(GTK_WINDOW(window_conversion), buf);
    g_signal_connect(window_conversion, "destroy",
                     G_CALLBACK(conversion_destroy), NULL);
    gtk_container_set_border_width(GTK_CONTAINER(window_conversion), 10);

    vbox = gui_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window_conversion), vbox);

    /* Add description label */
    label = gui_label_new(gui_click_val_msg, 0.5, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

    separator = gui_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 5);

    /* Add 'convert from' label */
    label = gui_label_new("Select 'Convert From' Units", 0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

    /* Add the table */
    table = create_table(info->category);
    gtk_box_pack_start(GTK_BOX(vbox), table, TRUE, TRUE, 5);

    /* Cancel button */
    button = gtk_button_new_with_mnemonic("_Cancel");
    g_signal_connect(button, "clicked",
        G_CALLBACK(cancel_button_clicked), NULL);
    gtk_widget_set_size_request(button, 80, -1);
#if TARGET_GTK_VERSION == 2
    GtkWidget *align = gtk_alignment_new(1, 0, 0, 0);
    gtk_container_add(GTK_CONTAINER(align), button);
    gtk_box_pack_start(GTK_BOX(vbox), align, FALSE, FALSE, 10);
#elif TARGET_GTK_VERSION == 3
    gtk_widget_set_halign(button, GTK_ALIGN_END);
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 10);
#endif

    gtk_widget_show_all(window_conversion);
}


void conversion_menu_add(GtkWidget *menubar)
{
    GtkWidget *ucv_menu;
    GtkWidget *ucv_root_mi;

    ucv_menu = gtk_menu_new();
    ucv_root_mi = gtk_menu_item_new_with_mnemonic("Con_version");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(ucv_root_mi), ucv_menu);
    for (unsigned i = 0; i < num_ucv; i++)
    {
        GtkWidget *mi;
        mi = gtk_menu_item_new_with_label(ucv_info[i].name);
        gtk_menu_shell_append(GTK_MENU_SHELL(ucv_menu), mi);
        g_signal_connect(G_OBJECT(mi), "activate",
                     G_CALLBACK(ucv_activate), (gpointer)&ucv_info[i]);
    }
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), ucv_root_mi);
    if (calc_get_mode() == calc_mode_integer)
        gtk_widget_set_sensitive(ucv_root_mi, FALSE);
    else
        gtk_widget_set_sensitive(ucv_root_mi, TRUE);
}
