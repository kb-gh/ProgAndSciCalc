/*****************************************************************************
 * File gui_menu_constants.c part of ProgAndSciCalc
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
#include <string.h>
#include "gui_internal.h"
#include "calc_conversion.h"
#include "config.h"

#define MAX_CATEGORIES 20
#define MAX_VALUES_PER_CATEGORY 15

#define MAX_CATEGORY_NAME_LEN 25
#define MAX_NAME_LEN 40
#define MAX_VALUE_LEN 42

#define NAME_DISPLAY_TEXT_LEN (MAX_NAME_LEN)
#define VALUE_DISPLAY_TEXT_LEN (MAX_VALUE_LEN)

static const char *CONSTFILE = "constants";

/* window to display Constants, there will only ever be one */
static GtkWidget *window_constants;

static int num_categories;
static int category_selected;

/* To avoid casting int to pointer in callback user data, if it wants a
 * pointer then, OK, it can have a pointer to one of these */
static int cat_cb[MAX_CATEGORIES];
static int row_cb[MAX_VALUES_PER_CATEGORY];

typedef struct
{
    char name[MAX_CATEGORY_NAME_LEN + 1];
    int num_rows;
    char row_name[MAX_VALUES_PER_CATEGORY][MAX_NAME_LEN + 1];
    char row_val[MAX_VALUES_PER_CATEGORY][MAX_VALUE_LEN + 1];
} cat_info_t;

static cat_info_t *cat_table[MAX_CATEGORIES];


static void constants_destroy(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;

    window_constants = NULL;
}

/* Callback for close button on the Conversion window */
static void cancel_button_clicked(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;

    gtk_widget_destroy(window_constants);
}

static void entry_enter_pressed(GtkWidget *widget, gpointer data)
{
    (void)widget;
    int row = *(int *)data;

    /* user may have switched mode to integer, the conversion result is only
     * intended to be used in float mode */
    if (calc_get_mode() == calc_mode_float)
    {
        calc_constants_apply_result(cat_table[category_selected]->row_val[row]);
        gtk_widget_destroy(window_constants);
    }
}

#define MOUSE_LEFT_BUT 1
static gboolean entry_button_release(GtkWidget *widget, GdkEventButton *event,
                                    gpointer data)
{
    (void)widget;
    int row = *(int *)data;

    if (event->button == MOUSE_LEFT_BUT)
    {
        /* user may have switched mode to integer, the conversion result is only
         * intended to be used in float mode */
        if (calc_get_mode() != calc_mode_float)
            return FALSE;

        calc_constants_apply_result(cat_table[category_selected]->row_val[row]);
        gtk_widget_destroy(window_constants);
        return TRUE;
    }

    return FALSE;
}


static GtkWidget *create_table(int category)
{
    int row, num_rows;
    GtkWidget *table;
    GtkWidget *label;
    GtkWidget *entry;

    cat_info_t *cat = cat_table[category];
    num_rows = cat->num_rows;

    table = gtk_table_new(num_rows, 2, FALSE);

    /* main table rows */
    for (row = 0; row < num_rows; row++)
    {
        /* label for name */
        label = gtk_label_new(cat->row_name[row]);
        gtk_label_set_width_chars(GTK_LABEL(label), NAME_DISPLAY_TEXT_LEN);
        gtk_label_set_max_width_chars(GTK_LABEL(label), NAME_DISPLAY_TEXT_LEN);
        gtk_misc_set_alignment(GTK_MISC(label), 0.5, 0.5);
        gtk_table_attach_defaults(GTK_TABLE(table), label,
                                  0, 1, row, row+1);

        /* entry for value */
        entry = gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(entry), cat->row_val[row]);
        gtk_entry_set_editable(GTK_ENTRY(entry), FALSE);
        gtk_entry_set_max_length(GTK_ENTRY(entry), VALUE_DISPLAY_TEXT_LEN);
        gtk_entry_set_width_chars(GTK_ENTRY(entry), VALUE_DISPLAY_TEXT_LEN);
        /* want to return value to calc either by clicking mouse on the value
         * (button-release) or by using keyboard to give focus to the value
         * and press enter (activate) */
        row_cb[row] = row;
        g_signal_connect(entry, "activate",
                         G_CALLBACK(entry_enter_pressed), (gpointer)&row_cb[row]);
        gtk_widget_add_events(entry, GDK_BUTTON_RELEASE_MASK);
        g_signal_connect(entry, "button-release-event",
                         G_CALLBACK(entry_button_release),(gpointer)&row_cb[row]);
        gtk_table_attach_defaults(GTK_TABLE(table), entry,
                                  1, 2, row, row+1);

    }

    return table;
}

#define WINDOW_TITLE_BUF_SIZE (MAX_CATEGORY_NAME_LEN + 20)

static void constants_activate(GtkWidget *widget, gpointer data)
{
    (void)widget;
    if (data == NULL)
        return;

    int category = *(int *)data;

    if (window_constants != NULL)
    {
        /* a constants window already exists */
        return;
    }

    category_selected = category;

    GtkWidget *table;
    GtkWidget *vbox;
    GtkWidget *separator;
    GtkWidget *button;
    GtkWidget *label;
    char buf[WINDOW_TITLE_BUF_SIZE];

    window_constants = gtk_window_new(GTK_WINDOW_TOPLEVEL);

    snprintf(buf, WINDOW_TITLE_BUF_SIZE, "Constants %s", cat_table[category]->name);
    gtk_window_set_title(GTK_WINDOW(window_constants), buf);
    g_signal_connect(window_constants, "destroy",
                     G_CALLBACK(constants_destroy), NULL);
    gtk_container_set_border_width(GTK_CONTAINER(window_constants), 10);

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window_constants), vbox);

    /* Add description label */
    label = gtk_label_new("Click on a value to return that value to the calculator");
    gtk_box_pack_start(GTK_BOX(vbox), label, FALSE, FALSE, 0);

    separator = gtk_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 5);

    /* add the table */
    table = create_table(category);
    gtk_box_pack_start(GTK_BOX(vbox), table, TRUE, TRUE, 5);

    /* Cancel button */
    GtkWidget *align = gtk_alignment_new(1, 0, 0, 0);
    button = gtk_button_new_with_mnemonic("_Cancel");
    g_signal_connect(button, "clicked",
        G_CALLBACK(cancel_button_clicked), NULL);
    gtk_widget_set_size_request(button, 80, -1);
    gtk_container_add(GTK_CONTAINER(align), button);
    gtk_box_pack_start(GTK_BOX(vbox), align, FALSE, FALSE, 10);

    gtk_widget_show_all(window_constants);
}

void constants_menu_add(GtkWidget *menubar)
{
    GtkWidget *c_menu;
    GtkWidget *c_root_mi;

    c_menu = gtk_menu_new();
    c_root_mi = gtk_menu_item_new_with_mnemonic("Cons_tants");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(c_root_mi), c_menu);

    if (num_categories == 0)
    {
        GtkWidget *mi;
        mi = gtk_menu_item_new_with_label("(Empty)");
        gtk_menu_shell_append(GTK_MENU_SHELL(c_menu), mi);
        g_signal_connect(G_OBJECT(mi), "activate",
                         G_CALLBACK(constants_activate), NULL);
    }
    else
    {
        for (int i = 0; i < num_categories; i++)
        {
            GtkWidget *mi;
            mi = gtk_menu_item_new_with_label(cat_table[i]->name);
            gtk_menu_shell_append(GTK_MENU_SHELL(c_menu), mi);
            cat_cb[i] = i;
            g_signal_connect(G_OBJECT(mi), "activate",
                             G_CALLBACK(constants_activate), (gpointer)&cat_cb[i]);
        }
    }
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), c_root_mi);
    if (calc_get_mode() == calc_mode_integer)
        gtk_widget_set_sensitive(c_root_mi, FALSE);
    else
        gtk_widget_set_sensitive(c_root_mi, TRUE);
}

#define LINE_SIZE 200
static void load_entries(FILE *fp)
{
    char buf[LINE_SIZE];
    int cat_index = 0;
    int row_count = 0;
    cat_info_t *current_cat = NULL;

    while (fgets(buf, LINE_SIZE, fp))
    {
        if (buf[0] == '\n' || buf[0] == '#')
            continue;

        /* chomp newline */
        char *nl = strchr(buf, '\n');
        if (nl)
        {
            *nl = 0;
        }
        char *leftb = strchr(buf, '[');
        char *rightb = strchr(buf, ']');
        char *eqs = strchr(buf, '=');

        if (leftb && rightb && leftb < rightb)
        {
            /* Looks like a category line */
            if (current_cat)
            {
                /* store details of prev category */
                current_cat->num_rows = row_count;
                cat_index++;
            }
            /* start new category */
            if (cat_index >= MAX_CATEGORIES)
            {
                fprintf(stderr, "num constants categories exceeded limit,"
                                " ignoring extra categories\n");
                current_cat = NULL;
                break;
            }
            current_cat = malloc(sizeof(*current_cat));
            cat_table[cat_index] = current_cat;
            row_count = 0;
            if (current_cat)
            {
                /* store category name */
                *rightb = 0;
                current_cat->name[0] = 0;
                strncat(current_cat->name, leftb + 1, MAX_CATEGORY_NAME_LEN);
            }
        }
        else if (current_cat && eqs)
        {
            /* Looks like an entry, split into name and value */
            *eqs = 0;
            if (row_count < MAX_VALUES_PER_CATEGORY)
            {
                current_cat->row_name[row_count][0] = 0;
                strncat(current_cat->row_name[row_count], buf, MAX_NAME_LEN);
                current_cat->row_val[row_count][0] = 0;
                strncat(current_cat->row_val[row_count], eqs + 1, MAX_VALUE_LEN);
                row_count++;
            }
            else
            {
                fprintf(stderr, "num constants category entries exceeded limit,"
                                " ignoring extra entries\n");
            }
        }
    }
    if (current_cat)
    {
        /* store details of last category */
        current_cat->num_rows = row_count;
        cat_index++;
    }
    num_categories = cat_index;
    //printf("num_categories %d\n", num_categories);

    if (num_categories > MAX_CATEGORIES)
    {
        /* should never happen */
        fprintf(stderr, "BUG somehow ended up with too many constants categories\n");
        num_categories = MAX_CATEGORIES;
    }
}


/* load entries from constants file if it exists */
void gui_menu_constants_init(void)
{
    FILE *fp;

    gchar *filename = g_build_filename(g_get_home_dir(),
                                       config_get_dirname(),
                                       CONSTFILE, NULL);
    fp = fopen(filename, "r");
    if (fp == NULL)
    {
        //printf("couldn't open file %s for reading\n", filename);
        g_free(filename);
        return;
    }
    g_free(filename);

    load_entries(fp);

    fclose(fp);
}


void gui_menu_constants_deinit(void)
{
    for (int i = 0; i < num_categories; i++)
    {
        free(cat_table[i]);
    }
}

