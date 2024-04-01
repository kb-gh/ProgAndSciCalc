/*****************************************************************************
 * File gui_menu_options.c part of ProgAndSciCalc
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

/* Settings General */
static GtkWidget *window_settings;
static GtkWidget *entry_fs_main;
static GtkWidget *entry_fs_bin;
static GtkWidget *entry_fs_binop_lbl;
static GtkWidget *entry_but_height;
static calc_mode_enum calc_mode;
static bool replace_zero;

/* Settings Floating */
static GtkWidget *window_settings_float;
static float_digits_enum float_digits;
static GtkWidget *entry_ran_n;
static bool random_01;
static bool sct_round;

/* Settings Integer */
static GtkWidget *window_settings_int;
static calc_width_enum integer_width;
static bool use_unsigned;
static int hex_group;
static bool warn_on_signed_overflow;
static bool warn_on_unsigned_overflow;


static void settings_destroy(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;

    window_settings = NULL;
}

static void settings_float_destroy(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;

    window_settings_float = NULL;
}

static void settings_int_destroy(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;

    window_settings_int = NULL;
}


/* helper to read text entries */
static int get_entry_val(GtkWidget *entry, int min, int max, int def)
{
    int res = (int)strtol(gtk_entry_get_text(GTK_ENTRY(entry)), NULL, 10);
    if (res < min || res > max)
        res = def;
    return res;
}


/****************************************************************************
 * Settings General
 */

/* Callback for Close button on the Settings General window */
static void settings_cancel_button_clicked(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;

    gtk_widget_destroy(window_settings);
}

/* Callback for OK button on the Settings General window */
static void settings_ok_button_clicked(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;

    /* fontsize, will require program restart (or at least a mode change)
     * to take effect */
    int fs;
    fs = get_entry_val(entry_fs_main,
                       MAIN_FONTSIZE_MIN,
                       MAIN_FONTSIZE_MAX,
                       MAIN_FONTSIZE_DEFAULT);
    config_set_main_disp_fontsize(fs);
    fs = get_entry_val(entry_fs_bin,
                       BIN_FONTSIZE_MIN,
                       BIN_FONTSIZE_MAX,
                       BIN_FONTSIZE_DEFAULT);
    config_set_bin_disp_fontsize(fs);
    fs = get_entry_val(entry_fs_binop_lbl,
                       BINOP_LBL_FONTSIZE_MIN,
                       BINOP_LBL_FONTSIZE_MAX,
                       BINOP_LBL_FONTSIZE_DEFAULT);
    config_set_binop_lbl_fontsize(fs);
    fs = get_entry_val(entry_but_height,
                       BUT_HEIGHT_MIN,
                       BUT_HEIGHT_MAX,
                       BUT_HEIGHT_DEFAULT);
    config_set_but_height(fs);

    /* this is only used to set the startup value */
    config_set_calc_mode(calc_mode);

    /* will take effect next time display updated */
    config_set_replace_zero_with_o(replace_zero);

    gtk_widget_destroy(window_settings);
}


#define FS_TEXT_LEN 2
/* fontsizes and button height */
static void add_fontsize(GtkWidget *vbox)
{
    GtkWidget *hbox;
    GtkWidget *lbl;
    char msg[80];
    char buf[40];

    lbl = gui_label_new("Sizing (requires mode change or program restart to take effect)", 0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), lbl, FALSE, FALSE, 5);

    hbox = gui_hbox_new(FALSE, 4);
    sprintf(msg, "main display fontsize (%d to %d, default %d)",
            MAIN_FONTSIZE_MIN, MAIN_FONTSIZE_MAX, MAIN_FONTSIZE_DEFAULT);
    lbl = gui_label_new(msg, 0.5, 0.5);
    entry_fs_main = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry_fs_main), FS_TEXT_LEN);
    gtk_entry_set_width_chars(GTK_ENTRY(entry_fs_main), FS_TEXT_LEN);
    sprintf(buf, "%d", config_get_main_disp_fontsize());
    gtk_entry_set_text(GTK_ENTRY(entry_fs_main), buf);
    gtk_box_pack_start(GTK_BOX(hbox), lbl, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), entry_fs_main, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    hbox = gui_hbox_new(FALSE, 4);
    sprintf(msg, "binary display fontsize (%d to %d, default %d)",
            BIN_FONTSIZE_MIN, BIN_FONTSIZE_MAX, BIN_FONTSIZE_DEFAULT);
    lbl = gui_label_new(msg, 0.5, 0.5);
    entry_fs_bin = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry_fs_bin), FS_TEXT_LEN);
    gtk_entry_set_width_chars(GTK_ENTRY(entry_fs_bin), FS_TEXT_LEN);
    sprintf(buf, "%d", config_get_bin_disp_fontsize());
    gtk_entry_set_text(GTK_ENTRY(entry_fs_bin), buf);
    gtk_box_pack_start(GTK_BOX(hbox), lbl, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), entry_fs_bin, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    hbox = gui_hbox_new(FALSE, 4);
    sprintf(msg, "pending binary operator label fontsize (%d to %d, default %d)",
            BINOP_LBL_FONTSIZE_MIN, BINOP_LBL_FONTSIZE_MAX, BINOP_LBL_FONTSIZE_DEFAULT);
    lbl = gui_label_new(msg, 0.5, 0.5);
    entry_fs_binop_lbl = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry_fs_binop_lbl), FS_TEXT_LEN);
    gtk_entry_set_width_chars(GTK_ENTRY(entry_fs_binop_lbl), FS_TEXT_LEN);
    sprintf(buf, "%d", config_get_binop_lbl_fontsize());
    gtk_entry_set_text(GTK_ENTRY(entry_fs_binop_lbl), buf);
    gtk_box_pack_start(GTK_BOX(hbox), lbl, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), entry_fs_binop_lbl, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);

    hbox = gui_hbox_new(FALSE, 4);
    sprintf(msg, "button height (%d to %d, default %d)",
            BUT_HEIGHT_MIN, BUT_HEIGHT_MAX, BUT_HEIGHT_DEFAULT);
    lbl = gui_label_new(msg, 0.5, 0.5);
    entry_but_height = gtk_entry_new();
    gtk_entry_set_max_length(GTK_ENTRY(entry_but_height), FS_TEXT_LEN);
    gtk_entry_set_width_chars(GTK_ENTRY(entry_but_height), FS_TEXT_LEN);
    sprintf(buf, "%d", config_get_but_height());
    gtk_entry_set_text(GTK_ENTRY(entry_but_height), buf);
    gtk_box_pack_start(GTK_BOX(hbox), lbl, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), entry_but_height, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
}

typedef struct
{
    char *name;
    calc_mode_enum id;
} CALC_MODE_RB;
#define NUM_CALC_MODE_RB 2
static const CALC_MODE_RB calc_mode_rb[NUM_CALC_MODE_RB] =
{
    { "integer", calc_mode_integer },
    { "floating", calc_mode_float },
};

static void calc_mode_rb_toggle(GtkWidget *widget, gpointer data)
{
    const CALC_MODE_RB *info = data;
    gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

    if (active)
    {
        calc_mode = info->id;
    }
}

static void add_calc_mode(GtkWidget *vbox)
{
    GtkWidget *lbl;
    GtkWidget *button;
    GSList *group = NULL;

    calc_mode = config_get_calc_mode();

    lbl = gui_label_new("Calculator mode at startup", 0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), lbl, FALSE, FALSE, 0);

    for (int i = 0; i < NUM_CALC_MODE_RB; i++)
    {
        button = gtk_radio_button_new_with_label(group, calc_mode_rb[i].name);
        group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(button));
        if (calc_mode_rb[i].id == calc_mode)
        {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
        }
        g_signal_connect(button, "toggled",
                         G_CALLBACK(calc_mode_rb_toggle),
                         (gpointer)&calc_mode_rb[i]);
        gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
    }
}

static void replace_zero_button_toggle(GtkWidget *widget, gpointer data)
{
    (void)data;
    gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    replace_zero = active;
}

static const char *replace_zero_str =
"If you don't like the zero character in the monospace font used for\n"
"the main and binary displays, try this low tech solution to replace\n"
"zero characetrs with a capital O, it might look nicer.";

static void add_replace_zero(GtkWidget *vbox)
{
    GtkWidget *lbl;
    GtkWidget *button;

    lbl = gui_label_new(replace_zero_str, 0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), lbl, FALSE, FALSE, 0);

    replace_zero = config_get_replace_zero_with_o();

    button = gtk_check_button_new_with_label("Replace zero with O");
    if (replace_zero)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
    g_signal_connect(button, "toggled",
		     G_CALLBACK(replace_zero_button_toggle), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
}

/* Callback for menu Options->Settings (General) */
static void settings_activate(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;

    GtkWidget *vbox;
    GtkWidget *separator;
    GtkWidget *button;
    GtkWidget *hbox_but;

    if (window_settings != NULL)
        return;

    window_settings = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window_settings), "Settings (General)");
    g_signal_connect(window_settings, "destroy",
                     G_CALLBACK(settings_destroy), NULL);
    gtk_container_set_border_width(GTK_CONTAINER(window_settings), 10);

    vbox = gui_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window_settings), vbox);

    separator = gui_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 5);

    add_fontsize(vbox);
    separator = gui_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 5);

    add_calc_mode(vbox);
    separator = gui_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 5);

    add_replace_zero(vbox);
    separator = gui_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 5);

    // OK and Cancel buttons

    hbox_but = gui_hbox_new(TRUE, 0);
    button = gtk_button_new_with_mnemonic("_OK");
    gtk_box_pack_start(GTK_BOX(hbox_but), button, TRUE, TRUE, 0);
    g_signal_connect(button, "clicked",
        G_CALLBACK(settings_ok_button_clicked), NULL);

    button = gtk_button_new_with_mnemonic("_Cancel");
    gtk_box_pack_start(GTK_BOX(hbox_but), button, TRUE, TRUE, 0);
    g_signal_connect(button, "clicked",
        G_CALLBACK(settings_cancel_button_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_but, FALSE, FALSE, 10);

    gtk_widget_show_all(window_settings);
}


/****************************************************************************
 * Settings Floating
 */

/* Callback for OK button on the Settings Float window */
static void settings_float_ok_button_clicked(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;

    /* this is only used to set the startup value */
    config_set_float_digits(float_digits);

    /* random number, update calc so will take effect on next RAND use */
    config_set_random_01(random_01);
    int rn = get_entry_val(entry_ran_n,
                           RANDOM_N_MIN,
                           RANDOM_N_MAX,
                           RANDOM_N_DEFAULT);
    config_set_random_n(rn);
    if (config_get_random_01())
        calc_set_random_range(0);
    else
        calc_set_random_range(rn);

    /* sct rounding, update calc so it will take effect on next use */
    config_set_use_sct_rounding(sct_round);
    calc_set_use_sct_rounding(sct_round);

    gtk_widget_destroy(window_settings_float);
}

/* Callback for Close button on the Settings Float window */
static void settings_float_cancel_button_clicked(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;

    gtk_widget_destroy(window_settings_float);
}

static void float_digits_rb_toggle(GtkWidget *widget, gpointer data)
{
    const FLOAT_DIGITS_RB *info = data;
    gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

    //printf("float digits toggle %d active %d\n", info->id, active);

    if (active)
    {
        float_digits = info->id;
    }
}

static void add_float_digits(GtkWidget *vbox)
{
    GtkWidget *lbl;
    GtkWidget *hbox;
    GtkWidget *button;
    GSList *group = NULL;

    float_digits = config_get_float_digits();

    hbox = gui_hbox_new(TRUE, 0);

    lbl = gui_label_new("Number of display digits at startup", 0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), lbl, FALSE, FALSE, 0);

    for (int i = 0; i < NUM_FLOAT_DIGITS_ID; i++)
    {
        button = gtk_radio_button_new_with_label(group, float_digits_rb[i].name);
        group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(button));
        if (float_digits_rb[i].id == float_digits)
        {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
        }
        g_signal_connect(button, "toggled",
                         G_CALLBACK(float_digits_rb_toggle),
                         (gpointer)&float_digits_rb[i]);
        gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 10);
    }
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);
}


typedef struct
{
    char *name;
    int  id;
} RAN_RB;

#define RAN_RB_01_ID 0
#define RAN_RB_N_ID 1
#define NUM_RAN_RB 2
static const RAN_RB ran_rb[NUM_RAN_RB] =
{
    { "0 <= r < 1", RAN_RB_01_ID },
    { "1 <= r <= n (integers)", RAN_RB_N_ID },
};

static void random_rb_toggle(GtkWidget *widget, gpointer data)
{
    const RAN_RB *ranrb = data;
    gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    //printf("random_rb toggle %d active %d\n", ranrb->id, (int)active);
    if (active)
    {
        if (ranrb->id == RAN_RB_01_ID)
        {
            random_01 = true;
            /* shouldn't be possible to get here and it's NULL, but just in case */
            if (entry_ran_n != NULL)
                gtk_widget_set_sensitive(entry_ran_n, FALSE);
        }
        else if (ranrb->id == RAN_RB_N_ID)
        {
            random_01 = false;
            /* shouldn't be possible to get here and it's NULL, but just in case */
            if (entry_ran_n != NULL)
                gtk_widget_set_sensitive(entry_ran_n, TRUE);
        }
    }
}

#define RAN_N_TEXT_LEN 5

static void add_random_num(GtkWidget *vbox)
{
    GtkWidget *hbox;
    GtkWidget *lbl;
    GtkWidget *button = NULL;
    GSList *group;
    int i;
    char buf[40];
    char msg[80];

    random_01 = config_get_random_01();

    lbl = gui_label_new("Range for random number function", 0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), lbl, FALSE, FALSE, 0);

    /* radio buttons */
    for (i = 0; i < NUM_RAN_RB; i++)
    {
        if (i == 0)
        {
            button = gtk_radio_button_new_with_label(NULL, ran_rb[i].name);
            if (random_01)
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
        }
        else
        {
            group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(button));
            button = gtk_radio_button_new_with_label(group, ran_rb[i].name);
            if (!random_01)
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
        }

        /* This doesn't ever result in random_rb_toggle getting called with
         * with active=true while we are still creating the widgets, which
         * is handy since we haven't created entry_ran_n yet. */
        g_signal_connect(button, "toggled",
                         G_CALLBACK(random_rb_toggle), (gpointer)&ran_rb[i]);
        gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
    }

    /* choose n entry */
    hbox = gui_hbox_new(FALSE, 4);
    sprintf(msg, "Choose n (%d to %d, default %d)",
            RANDOM_N_MIN, RANDOM_N_MAX, RANDOM_N_DEFAULT);
    lbl = gui_label_new(msg, 0.5, 0.5);
    entry_ran_n = gtk_entry_new();
    //printf("entry_ran_n created\n");
    gtk_entry_set_max_length(GTK_ENTRY(entry_ran_n), RAN_N_TEXT_LEN);
    gtk_entry_set_width_chars(GTK_ENTRY(entry_ran_n), RAN_N_TEXT_LEN);
    sprintf(buf, "%d", config_get_random_n());
    gtk_entry_set_text(GTK_ENTRY(entry_ran_n), buf);
    gtk_box_pack_start(GTK_BOX(hbox), lbl, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), entry_ran_n, FALSE, FALSE, 0);
    if (random_01)
        gtk_widget_set_sensitive(entry_ran_n, FALSE);
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 0);
}

static void sct_button_toggle(GtkWidget *widget, gpointer data)
{
    (void)data;
    gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    sct_round = active;
}

static const char *sct_round_str =
"With the exception of sin(x) for small x, if the absolute value of a\n"
"sin or cos result is close to zero (less than 1E-30), just call it zero.\n"
"This could be considered an evil hack and so is optional.\n"
"It will also affect tan (calculated as sin/cos).";

static void add_sct_rounding(GtkWidget *vbox)
{
    GtkWidget *lbl;
    GtkWidget *button;

    lbl = gui_label_new(sct_round_str, 0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), lbl, FALSE, FALSE, 0);

    sct_round = config_get_use_sct_rounding();

    button = gtk_check_button_new_with_label("Enable sin/cos/tan rounding");
    if (sct_round)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
    g_signal_connect(button, "toggled",
		     G_CALLBACK(sct_button_toggle), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
}

/* Callback for menu Options->Settings (Floating) */
static void settings_float_activate(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;

    GtkWidget *vbox;
    GtkWidget *separator;
    GtkWidget *button;
    GtkWidget *hbox_but;

    if (window_settings_float != NULL)
        return;

    entry_ran_n = NULL;

    window_settings_float = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window_settings_float), "Settings (Floating)");
    g_signal_connect(window_settings_float, "destroy",
                     G_CALLBACK(settings_float_destroy), NULL);
    gtk_container_set_border_width(GTK_CONTAINER(window_settings_float), 10);

    vbox = gui_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window_settings_float), vbox);

    separator = gui_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 5);

    add_float_digits(vbox);
    separator = gui_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 5);

    add_random_num(vbox);
    separator = gui_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 5);

    add_sct_rounding(vbox);
    separator = gui_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 5);

    // OK and Cancel buttons

    hbox_but = gui_hbox_new(TRUE, 0);
    button = gtk_button_new_with_mnemonic("_OK");
    gtk_box_pack_start(GTK_BOX(hbox_but), button, TRUE, TRUE, 0);
    g_signal_connect(button, "clicked",
        G_CALLBACK(settings_float_ok_button_clicked), NULL);

    button = gtk_button_new_with_mnemonic("_Cancel");
    gtk_box_pack_start(GTK_BOX(hbox_but), button, TRUE, TRUE, 0);
    g_signal_connect(button, "clicked",
        G_CALLBACK(settings_float_cancel_button_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_but, FALSE, FALSE, 10);

    gtk_widget_show_all(window_settings_float);
}


/****************************************************************************
 * Settings Integer
 */

/* Callback for OK button on the Settings Integer window */
static void settings_int_ok_button_clicked(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;

    /* only used to set the startup value */
    config_set_integer_width(integer_width);
    config_set_use_unsigned(use_unsigned);

    /* hex grouping, update display so will take effect on next display
     * update */
    config_set_hex_grouping(hex_group);
    display_set_hex_grouping(hex_group);

    config_set_warn_on_signed_overflow(warn_on_signed_overflow);
    calc_set_warn_on_signed_overflow(warn_on_signed_overflow);

    config_set_warn_on_unsigned_overflow(warn_on_unsigned_overflow);
    calc_set_warn_on_unsigned_overflow(warn_on_unsigned_overflow);

    gtk_widget_destroy(window_settings_int);
}

/* Callback for Close button on the Settings Integer window */
static void settings_int_cancel_button_clicked(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;

    gtk_widget_destroy(window_settings_int);
}

static void int_width_rb_toggle(GtkWidget *widget, gpointer data)
{
    const INT_WIDTH_RB *info = data;
    gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    if (active)
    {
        integer_width = info->id;
    }
}

static void add_integer_width_group(GtkWidget *vbox)
{
    GtkWidget *lbl;
    GtkWidget *hbox;
    GtkWidget *button;
    GSList *group = NULL;

    integer_width = config_get_integer_width();

    hbox = gui_hbox_new(TRUE, 0);

    lbl = gui_label_new("Integer width at startup", 0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), lbl, FALSE, FALSE, 0);

    for (int i = 0; i < num_calc_widths; i++)
    {
        button = gtk_radio_button_new_with_label(group, int_width_rb[i].name);
        group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(button));
        if (int_width_rb[i].id == integer_width)
        {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
        }
        g_signal_connect(button, "toggled",
                         G_CALLBACK(int_width_rb_toggle),
                         (gpointer)&int_width_rb[i]);
        gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 10);
    }
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);
}


static void signed_unsigned_rb_toggle(GtkWidget *widget, gpointer data)
{
    const INT_SIGNED_RB *info = data;
    gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

    if (active)
    {
        if (info->id == INT_USE_SIGNED_ID)
            use_unsigned = false;
        else if (info->id == INT_USE_UNSIGNED_ID)
            use_unsigned = true;
    }
}

static void add_signed_unsigned_group(GtkWidget *vbox)
{
    GtkWidget *lbl;
    GtkWidget *hbox;
    GtkWidget *button;
    GSList *group = NULL;
    int i;

    use_unsigned = config_get_use_unsigned();

    hbox = gui_hbox_new(TRUE, 0);

    lbl = gui_label_new("Signed or unsigned at startup", 0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), lbl, FALSE, FALSE, 0);

    for (i = 0; i < NUM_INT_SIGNED_RB; i++)
    {
        button = gtk_radio_button_new_with_label(group, int_signed_rb[i].name);
        group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(button));
        if ((i == INT_USE_SIGNED_ID && !use_unsigned) ||
            (i == INT_USE_UNSIGNED_ID && use_unsigned))
        {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
        }
        g_signal_connect(button, "toggled",
                         G_CALLBACK(signed_unsigned_rb_toggle),
                         (gpointer)&int_signed_rb[i]);
        gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 10);
    }
    gtk_box_pack_start(GTK_BOX(vbox), hbox, FALSE, FALSE, 5);
}



typedef struct
{
    char *name;
    int  id;
} HEX_GROUP_RB;

#define HEX_GROUP_0_ID 0
#define HEX_GROUP_4_ID 1
#define HEX_GROUP_8_ID 2
#define NUM_HEX_GROUP_RB 3
static const HEX_GROUP_RB hg_rb[NUM_HEX_GROUP_RB] =
{
    { "none", HEX_GROUP_0_ID },
    { "4 digits", HEX_GROUP_4_ID },
    { "8 digits", HEX_GROUP_8_ID },
};

static void hex_group_rb_toggle(GtkWidget *widget, gpointer data)
{
    const HEX_GROUP_RB *hgrb = data;
    gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    //printf("hex group toggle %d active %d\n", hgrb->id, (int)active);
    if (active)
    {
        if (hgrb->id == HEX_GROUP_0_ID)
            hex_group = 0;
        else if (hgrb->id == HEX_GROUP_4_ID)
            hex_group = 4;
        else if (hgrb->id == HEX_GROUP_8_ID)
            hex_group = 8;
    }
}

static void add_hex_group(GtkWidget *vbox)
{
    GtkWidget *lbl;
    GtkWidget *button;
    GSList *group = NULL;
    int i;

    hex_group = config_get_hex_grouping();

    lbl = gui_label_new("Grouping for numbers when displayed in hex", 0, 0.5);
    gtk_box_pack_start(GTK_BOX(vbox), lbl, FALSE, FALSE, 0);

    /* radio buttons */
    for (i = 0; i < NUM_HEX_GROUP_RB; i++)
    {
        button = gtk_radio_button_new_with_label(group, hg_rb[i].name);
        group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(button));

        if (i == 0)
        {
            //printf("hex group 0\n");
            if (hex_group == 0)
            {
                //printf("hex group 0 set active\n");
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
            }
        }
        else if (i == 1)
        {
            //printf("hex group 1\n");
            if (hex_group == 4)
            {
                //printf("hex group 1 set active\n");
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
            }
        }
        else if (i == 2)
        {
            //printf("hex group 2\n");
            if (hex_group == 8)
            {
                //printf("hex group 2 set active\n");
                gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
            }
        }
        g_signal_connect(button, "toggled",
                         G_CALLBACK(hex_group_rb_toggle), (gpointer)&hg_rb[i]);
        gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
    }
}

static void warn_unsigned_button_toggle(GtkWidget *widget, gpointer data)
{
    (void)data;
    gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    warn_on_unsigned_overflow = active;
}

static void add_warn_unsigned_overflow(GtkWidget *vbox)
{
    GtkWidget *button;

    warn_on_unsigned_overflow = config_get_warn_on_unsigned_overflow();

    button = gtk_check_button_new_with_label("Warn on unsigned overflow/underflow");
    if (warn_on_unsigned_overflow)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
    g_signal_connect(button, "toggled",
		     G_CALLBACK(warn_unsigned_button_toggle), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
}

static void warn_signed_button_toggle(GtkWidget *widget, gpointer data)
{
    (void)data;
    gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    warn_on_signed_overflow = active;
}

static void add_warn_signed_overflow(GtkWidget *vbox)
{
    GtkWidget *button;

    warn_on_signed_overflow = config_get_warn_on_signed_overflow();

    button = gtk_check_button_new_with_label("Warn on signed overflow/underflow");
    if (warn_on_signed_overflow)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
    g_signal_connect(button, "toggled",
		     G_CALLBACK(warn_signed_button_toggle), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), button, FALSE, FALSE, 0);
}

/* Callback for menu Options->Settings (Integer) */
static void settings_int_activate(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;

    GtkWidget *vbox;
    GtkWidget *separator;
    GtkWidget *button;
    GtkWidget *hbox_but;

    if (window_settings_int != NULL)
        return;

    window_settings_int = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window_settings_int), "Settings (Integer)");
    g_signal_connect(window_settings_int, "destroy",
                     G_CALLBACK(settings_int_destroy), NULL);
    gtk_container_set_border_width(GTK_CONTAINER(window_settings_int), 10);

    vbox = gui_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window_settings_int), vbox);

    separator = gui_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 5);

    add_integer_width_group(vbox);
    separator = gui_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 5);

    add_signed_unsigned_group(vbox);
    separator = gui_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 5);

    add_hex_group(vbox);
    separator = gui_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 5);

    add_warn_signed_overflow(vbox);
    separator = gui_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 5);

    add_warn_unsigned_overflow(vbox);
    separator = gui_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 5);

    // OK and Cancel buttons

    hbox_but = gui_hbox_new(TRUE, 0);
    button = gtk_button_new_with_mnemonic("_OK");
    gtk_box_pack_start(GTK_BOX(hbox_but), button, TRUE, TRUE, 0);
    g_signal_connect(button, "clicked",
        G_CALLBACK(settings_int_ok_button_clicked), NULL);

    button = gtk_button_new_with_mnemonic("_Cancel");
    gtk_box_pack_start(GTK_BOX(hbox_but), button, TRUE, TRUE, 0);
    g_signal_connect(button, "clicked",
        G_CALLBACK(settings_int_cancel_button_clicked), NULL);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_but, FALSE, FALSE, 10);

    gtk_widget_show_all(window_settings_int);
}

void options_menu_add(GtkWidget *menubar)
{
    GtkWidget *options_menu;
    GtkWidget *options_root_mi;
    GtkWidget *settings_mi;
    GtkWidget *settings_float_mi;
    GtkWidget *settings_int_mi;

    options_menu = gtk_menu_new();
    options_root_mi = gtk_menu_item_new_with_mnemonic("_Options");
    settings_mi = gtk_menu_item_new_with_label("Settings (General)");
    settings_float_mi = gtk_menu_item_new_with_label("Settings (Floating)");
    settings_int_mi = gtk_menu_item_new_with_label("Settings (Integer)");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(options_root_mi), options_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(options_menu), settings_mi);
    gtk_menu_shell_append(GTK_MENU_SHELL(options_menu), settings_float_mi);
    gtk_menu_shell_append(GTK_MENU_SHELL(options_menu), settings_int_mi);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), options_root_mi);

    g_signal_connect(G_OBJECT(settings_mi), "activate",
                     G_CALLBACK(settings_activate), NULL);
    g_signal_connect(G_OBJECT(settings_float_mi), "activate",
                     G_CALLBACK(settings_float_activate), NULL);
    g_signal_connect(G_OBJECT(settings_int_mi), "activate",
                     G_CALLBACK(settings_int_activate), NULL);
}
