/*****************************************************************************
 * File gui_menu_help.c part of ProgAndSciCalc
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

#define VERSION "1.0"

extern GtkWidget *window_main;

static const char *instructions[] =
{
"MODE\n"
"Integer  - Performs operations on integers, 8, 16, 32 or 64 bits, signed or unsigned.\n"
"           The main display can be decimal or hex, the value is also shown in binary\n"
"           under the main display.\n"
"Floating - uses decQuad decimal floating point type (34 digits internally). The main display\n"
"           can be configured to use from 8 to 20 digits (20 is chosen as the max because\n"
"           it's plenty, and 20 is sufficient to precisely represent all integers in the\n"
"           range of an unsigned 64 bit integer, in case that might be useful).\n"
"           Also features unit conversions, and can optionally read in constants from a\n"
"           user supplied text file.\n\n"
"Use the [MODE] button to switch between modes. The current value (possibly within some\n"
"limitations) is passed on when switching mode, see Mode Change section further down.",

"DIRECTORY FOR CONFIG AND CONSTANTS FILES\n"
"If you want the calculator to save configuration settings (otherwise it will always start\n"
"up with default configuration), you need to create a directory .ProgAndSciCalc under your\n"
"home directory. The calculator will then create a config file under this location ie.\n"
"  ~/.ProgAndSciCalc/config\n"
"This is also the location where you can optionally place a constants file (described in\n"
"more detail further down).",

"BINARY OPERATORS\n"
"[+] [-] [*] [/] are the usual arithmetic operations\n"
"[mod] is modulo division (ie. remainder)\n"
"[pow] is x to power y eg. 2 [pow] 3 = 8\n"
"[root] is the reverse eg. 8 [root] 3 = 2\n"
"[<< n] [>> n] are left shift and right shift by n eg. 20 [<<] 2 = 80, 20 [>>] 2 = 5\n"
"[and] [or] [xor] are bitwise operations\n"
"[gcd] is greatest common divisor eg. 9 [gcd] 6 = 3\n\n"
"Precedence, from low to high, is ADD_SUB, MUL_DIV, POWER_ROOT.\n"
"Associativity in all cases (including POWER_ROOT) is left to right.\n"
"eg. 1 + 2 * 3 = 7\n"
"Parentheses are available to override the natural precedence.\n"
"left/right shift has the same precedence as MUL_DIV.\n"
"gcd has the same precedence as MUL_DIV.\n"
"and/or/xor have the same precedence as ADD_SUB.\n\n"
"Repeated Equals checkbox\n"
" If checked, each time [=] is entered it will re-evaluate the last binary operator\n"
" For example :-\n"
"  10 + 2 = 12 = 14 = 16,  3 = 5,  7 = 9\n"
"  also can do eg.\n"
"  10 + = 20 = 30 = 40\n"
"  10 + 3 * = 19 = 28 = 37\n"
" compared to when unchecked\n"
"  10 + 2 = 12 = 12 = 12,  3 = 3,  7 = 7\n"
"  10 + = 10 = 10 = 10 (it discards the dangling +)\n"
"  10 + 3 * = 13 = 13 = 13 (it discards the dangling *)",

"UNARY OPERATORS\n"
"Unary operators are entered after the argument eg. 100 [log] = 2\n"
"Unary operators bind the tightest, above any binary op eg. 10 + 2 * 3 [sqr] = 28\n"
"[+/-] unary minus\n"
"  In Integer mode, always treated as a unary minus operation.\n"
"  In Floating mode, treated as a unary minus operation when applied to a result,\n"
"  when entering a value it just gives a minus to the display.\n"
"  Will also change sign of exponent when entering in exponent form.\n"
"[not] invert bits\n"
"[sqr] square,  [sqrt] square root,  [1/x] one divided by x\n"
"[log] log10,  use [INV][log] for the inverse ie. 10^x\n"
"[ln] natural log, use [INV][ln] to get the inverse ie. e^x\n"
"[sin] [cos] [tan] trig functions, or sinh, cosh, tanh if [HYP] is selected\n"
"  use [INV][sin] etc. to get inverse\n"
"[<<] [>>] are left shift and right shift by 1 place\n"
"[rol] [ror] rotate (circular shift) left or right by 1 place\n"
"[x!] factorial, beware if x is not an integer value it is rounded up/down to closest integer",

"MISCELLANEOUS\n"
"[=] equals, evaluate all operations outstanding\n"
"[DEC] [HEX] number base,  [DEG] [RAD] [GRAD] angle units for sin, cos, tan\n"
"[F-E] force result to be displayed in exponent form\n"
"[EXP] to enter a number in exponent form\n"
"[PI] to enter PI\n"
"[M1S] memory 1 store,  [M1R] memory 1 recall,  [M1+] memory 1 plus\n"
"[M2S] memory 2 store,  [M2R] memory 2 recall,  [M2+] memory 2 plus\n"
"  memories are not saved across program restart\n"
"[RAND] random number (range can be set in Options->Settings)\n"
"[(] [)] there are 4 levels of nested parentheses\n"
"  a bracketed expression can only be started after a binary operator, or after [=] or [CLR]\n"
"[<---] to edit a user entered value\n"
"[HYP] change sin to sinh etc.\n"
"[INV] provides inverse of log, ln, sin(h), cos(h), tan(h)\n"
"[HIST] opens (or closes if already open) history window,\n"
"  history is not saved across program restart\n"
"[CLR] clear",

"KEYBOARD SHORTCUTS\n"
"Some of the more common functions are mapped to keyboard shortcuts, case insensitive.\n"
"  Button                Shortcut\n"
"---------------------------------------\n"
" 0-9, A-F, point       themselves\n"
"  + - * /              themselevs\n"
"    =                    RETURN\n"
"   CLR                ESCAPE, DELETE\n"
"  <---                  BACKSPACE\n"
"  MODE                     m\n"
"   +/-                     p\n"
"   DEC                     z\n"
"   HEX                     x\n"
"   EXP                     e\n"
"   INV                     i\n"
"   F-E                     f\n"
"  HIST                     h\n"
"   ()                  themselves\n\n"
"It is possible to access other buttons from keyboard by using cursor keys to highlight\n"
"the chosen button then pressing SPACEBAR.",

"MODE CHANGE\n"
"When changing modes, the current value is passed on to the new mode.\n"
"When switching from Integer mode to Floating mode, the current integer value on the\n"
"display is converted to decQuad.\n"
"When switching from Floating mode to Integer mode, the current float value on the\n"
"display is converted (truncated) to integer. Note that this is the one case where\n"
"the rounded display value is used rather than the underlying decQuad, so the result\n"
"may depend on the number of digits selected for display. If out of range, the result\n"
"will be int_min or int_max if signed, or 0 or uint_max if unsigned. (If out of range\n"
"due to width/signedness, it's a bit unfriendly, but at this point you can change the\n"
"width/signedness, go back to Floating mode, retrieve the original value from History,\n"
"then finally return to Integer mode).\n\n"
"Example 1, enter 123.456 in Floating mode, switch to Integer, now have 123, switch\n"
"back to Floating mode, now have 123. The original value 123.456 can be retrieved\n"
"from History if required.\n"
"Example 2, enter 12345678901234 in Integer mode, switch to Floating at 10 digits,\n"
"value is displayed as 1.23456789e+13, switch back to Integer, it takes the display\n"
"value so now have 12345678900000. The original value 12345678901234 can be retrieved\n"
"from History if required. If set to use 14 digits it would display in Floating mode\n"
"as 12345678901234 in which case on switching back to Integer you would get the\n"
"original 12345678901234. If needs be, setting the number of digits in Floating mode\n"
"to 20 will allow any integer value to be displayed in full and avoid rounding issues.",

"UNIT CONVERSIONS\n"
"The Conversion menu provides unit conversion tables. The current calculator value\n"
"is taken as the value to convert. If this value is 0 then, with the exception of\n"
"temperature conversion, the value 1 is used instead.",

"CONSTANTS FILE FORMAT\n"
"This is an optional text file named constants, which you should place here :-\n"
"  ~/.ProgAndSciCalc/constants\n"
"The format is a category name in square brackets, followed by the entries for that\n"
"category as name=value, without any spaces around the =\n"
"For example :\n\n"
"[Scientific]\n"
"speed of light=299792458\n"
"Planck constant=6.62607004e-34\n"
"[Other]\n"
"something=-1.23456789\n"
"something else=9.87654321e10\n\n"
"The limits are :-\n"
"max 20 categories\n"
"max 15 entries per category\n"
"category name max 25 char\n"
"entry name max 40 char\n"
"entry value max 42 char",

"COPY and PASTE\n"
"Ctrl-C will copy the display value (copies to both default and primary clipboards)\n"
"Ctrl-V will paste from the default clipboard\n"
"Ctrl-W will paste from the primary clipboard\n"
"Pasted value is converted according to the current mode, and for Integer mode,\n"
"according to whether display is set as DEC or HEX.",
};



/* window to display Instructions */
static GtkWidget *window_instruct;

static void instruct_destroy(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;

    window_instruct = NULL;
}

/* Callback for close button on the Instructions window */
static void instruct_close_button_clicked(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;

    gtk_widget_destroy(window_instruct);
}

/* Callback for menu Help->Instructions, create Instructions window */
static void instruct_activate(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;

    GtkWidget *vbox;
    GtkWidget *vbox_instruct;
    GtkWidget *label;
    GtkWidget *scrolled;
    GtkWidget *align;
    GtkWidget *button;

    if (window_instruct != NULL)
        return;

    window_instruct = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window_instruct), "Instructions");
    g_signal_connect(window_instruct, "destroy",
                     G_CALLBACK(instruct_destroy), NULL);
    gtk_container_set_border_width(GTK_CONTAINER(window_instruct), 10);

    vbox = gtk_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window_instruct), vbox);

    PangoFontDescription *pfd =
        pango_font_description_from_string("mono");

    scrolled = gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_size_request(scrolled, -1, 400);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled),
                                   GTK_POLICY_NEVER, GTK_POLICY_AUTOMATIC);

    vbox_instruct = gtk_vbox_new(FALSE, 0);
    gtk_container_set_border_width(GTK_CONTAINER(vbox_instruct), 10);
    for (unsigned i = 0; i < sizeof(instructions) / sizeof(*instructions); i++)
    {
        label = gtk_label_new(instructions[i]);
        gtk_widget_modify_font(label, pfd);
        gtk_misc_set_alignment(GTK_MISC(label), 0.0, 0.5);
        gtk_box_pack_start(GTK_BOX(vbox_instruct), label, FALSE, FALSE, 10);
    }
    gtk_scrolled_window_add_with_viewport(GTK_SCROLLED_WINDOW(scrolled), vbox_instruct);
    gtk_box_pack_start(GTK_BOX(vbox), scrolled, TRUE, TRUE, 0);

    pango_font_description_free(pfd);


    align = gtk_alignment_new(1, 0, 0, 0);
    button = gtk_button_new_with_mnemonic("_Close");
    gtk_widget_set_size_request(button, 80, -1);
    gtk_container_add(GTK_CONTAINER(align), button);
    gtk_box_pack_start(GTK_BOX(vbox), align, FALSE, FALSE, 10);

    g_signal_connect(button, "clicked",
        G_CALLBACK(instruct_close_button_clicked), NULL);

    gtk_widget_show_all(window_instruct);
}

#if 1
static void about_activate(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;
    gchar *authors[] = {"Ken Bromham", NULL};
    gtk_show_about_dialog(GTK_WINDOW(window_main),
                          "program-name", "ProgAndSciCalc",
                          "title", "About ProgAndSciCalc",
                          "authors", authors,
                          "version", VERSION,
                          "license", "GPLv3-or-later", NULL);
}
#else
static void dialog_response(GtkDialog *dialog, gint response_id, gpointer data)
{
    (void)response_id;
    (void)data;
    gtk_widget_destroy(GTK_WIDGET(dialog));
}
static void about_activate(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;

    GtkWidget *dialog;
    dialog = gtk_message_dialog_new(
                GTK_WINDOW(window_main),
                GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_OTHER,
                GTK_BUTTONS_CLOSE,
                "ProgAndSciCalc version " VERSION
                "\nWritten By   Ken Bromham\nLicense GPLv3-or-later");
    g_signal_connect(dialog, "response",
                     G_CALLBACK(dialog_response), NULL);
    gtk_window_set_title(GTK_WINDOW(dialog), "About ProgAndSciCalc");
    gtk_widget_show(dialog);
}
#endif


void help_menu_add(GtkWidget *menubar)
{
    GtkWidget *help_menu;
    GtkWidget *help_root_mi;
    GtkWidget *instructions_mi;

    help_menu = gtk_menu_new();
    help_root_mi = gtk_menu_item_new_with_mnemonic("_Help");
    instructions_mi = gtk_menu_item_new_with_label("Instructions");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_root_mi), help_menu);
    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), instructions_mi);
    gtk_menu_shell_append(GTK_MENU_SHELL(menubar), help_root_mi);

    g_signal_connect(G_OBJECT(instructions_mi), "activate",
                     G_CALLBACK(instruct_activate), NULL);

    GtkWidget *about_mi;
    about_mi = gtk_menu_item_new_with_label("About");
    gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), about_mi);
    g_signal_connect(G_OBJECT(about_mi), "activate",
                     G_CALLBACK(about_activate), NULL);
}
