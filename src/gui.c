/*****************************************************************************
 * File gui.c part of ProgAndSciCalc
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


/* decimal or hex when in integer mode */
typedef enum
{
    gui_radix_dec,
    gui_radix_hex,
} gui_radix_enum;

/* the main calculator window for the application */
GtkWidget *window_main;

static int debug_level;

/* Identifies buttons in the button grid */
typedef enum
{
    bid_0,
    bid_1,
    bid_2,
    bid_3,
    bid_4,
    bid_5,
    bid_6,
    bid_7,
    bid_8,
    bid_9,
    bid_A,
    bid_B,
    bid_C,
    bid_D,
    bid_E,
    bid_F,
    bid_pnt,  /* dec point */

    bid_eq,   /* equals */

    /* binary operators */
    bid_add,
    bid_sub,
    bid_mul,
    bid_div,
    bid_mod,  /* modulo */
    bid_pow,
    bid_root,
    bid_and,
    bid_or,
    bid_xor,
    bid_lsftn,  /* left shift n places (binary op) */
    bid_rsftn,  /* right shift n places (binary op) */
    bid_gcd,   /* greatest common divisor */


    /* unary operators */
    bid_pm,    /* plus/minus */
    bid_com,   /* complement (invert bits) */
    bid_sqr,   /* square */
    bid_sqrt,  /* square root */
    //bid_2powx, /* 2 to power x */
    bid_onedx, /* 1 / x */
    bid_log,   /* log, ln can be modified by INV */
    bid_ln,
    bid_sin,   /* sin,cos,tan can be modifed by HYP */
    bid_cos,
    bid_tan,
    bid_fact,
    bid_lsft,  /* left shift 1 place (unary op) */
    bid_rsft,  /* right shift 1 place (unary op) */
    bid_rol, /* rotate (circular shift) left 1 place */
    bid_ror, /* rotate (circular shift) right 1 place */

    bid_ms,   /* memory store */
    bid_mr,   /* memory recall */
    bid_mp,   /* memory plus */
    bid_ms2,
    bid_mr2,
    bid_mp2,

    bid_parl, /* left parentheses */
    bid_parr, /* right parentheses */

    bid_dec,  /* dec radix in integer mode */
    bid_hex,  /* hex radix in integer mode */

    bid_deg,  /* angle mode degree */
    bid_rad,  /* angle mode rad */
    bid_grad, /* angle mode grad */

    bid_inv,  /* inv for log, ln, pow, sin, cos, tan, sinh, cosh, tanh */
    bid_fe,   /* force result to be displayed in exp notation */
    bid_exp,  /* to enter num in exp notation */
    bid_hyp,  /* sin->sinh cos->cosh tan->tanh */

    bid_pi,    /* PI */
    //bid_eul,   /* e(uler) */

    bid_rand, /* random number */
    bid_clr,  /* clear */

    bid_flip, /* flip mode between integer/floating */

    bid_hist, /* history */

    /* that's all displayable buttons */
    bid_num_displayable,

    /* these are just for layout purposes */
    bid_blank,
    bid_wide,  /* used for layout to create double width */

} button_id_enum;


/* Info for number of float digits radio buttons */
typedef struct
{
    char *name;
    float_digits_enum id;
} FLOAT_DIGITS_RB;
static const FLOAT_DIGITS_RB float_digits_rb[NUM_FLOAT_DIGITS_ID] =
{
    { "8",  FLOAT_DIGITS_8_ID },
    { "10", FLOAT_DIGITS_10_ID },
    { "12", FLOAT_DIGITS_12_ID },
    { "14", FLOAT_DIGITS_14_ID },
    { "16", FLOAT_DIGITS_16_ID },
    { "18", FLOAT_DIGITS_18_ID },
    { "20", FLOAT_DIGITS_20_ID },
};
static int float_digits_from_id(float_digits_enum fd)
{
    switch (fd)
    {
    case FLOAT_DIGITS_8_ID:
        return 8;
    case FLOAT_DIGITS_10_ID:
        return 10;
    case FLOAT_DIGITS_12_ID:
        return 12;
    case FLOAT_DIGITS_14_ID:
        return 14;
    case FLOAT_DIGITS_16_ID:
        return 16;
    case FLOAT_DIGITS_18_ID:
        return 18;
    default:
        return 20;
    }
}

/* Info for integer width and signed/unsigned radio buttons */
typedef struct
{
    char *name;
    calc_width_enum id;
} INT_WIDTH_RB;
static const INT_WIDTH_RB int_width_rb[num_calc_widths] =
{
    { "8",  calc_width_8 },
    { "16", calc_width_16 },
    { "32", calc_width_32 },
    { "64", calc_width_64 },
};

typedef struct
{
    char *name;
    int id;
} INT_SIGNED_RB;
#define INT_USE_SIGNED_ID 0
#define INT_USE_UNSIGNED_ID 1
#define NUM_INT_SIGNED_RB 2
static const INT_SIGNED_RB int_signed_rb[NUM_INT_SIGNED_RB] =
{
    { "signed", INT_USE_SIGNED_ID },
    { "unsigned", INT_USE_UNSIGNED_ID },
};



/* Any widgets that need to be remembered */
static GtkWidget *lbl_status;
static GtkWidget *lbl_mem;
static GtkWidget *lbl_pending_bin_op;
static GtkWidget *but_backspace;
static GtkWidget *but_repeat_eq;
static GtkWidget *but_grid[bid_num_displayable];
static GtkWidget *rbut_int_width[num_calc_widths];
static GtkWidget *rbut_int_signed[NUM_INT_SIGNED_RB];
/* Used when creating the button grid, for either deg/rad/grad or dec/hex */
static GSList *rb_list_grid;


/* Declare some static functions here to save moving things around */
static void gui_recreate(void);
static void set_hex_buttons(button_id_enum bid);
static void set_angle_buttons(button_id_enum bid);
static void update_status_label(void);
static void update_mem_label(void);
static void gui_result_callback(uint64_t ival, stackf_t fval);
static void gui_history_callback(uint64_t ival, stackf_t fval);
static void gui_set_num_used_parentheses(int n);
static void gui_warn(const char *msg);
static void gui_error(const char *msg);
static void set_inv_button(bool selected);
static void set_hyp_button(bool selected);
static void show_pending_bin_op(void);
static void clipboard_copy(void);
static void clipboard_paste_default(void);
static void clipboard_paste_primary(void);

/* helps with F<->E switch (switch display between float<->exponent style) */
static disp_float_format_enum last_float_format;


/* Has the user entered anything ie. is there an argument waiting to be
 * sent to calculator */
static bool arg_pending;
/* INV button selected */
static bool inv_selected;
/* HYP button selected */
static bool hyp_selected;
/* decimal or hex */
static gui_radix_enum gui_radix;

/* set true at end of gui_recreate */
static bool gui_created;

/* The buttton grid is made up of 3 tables, each table is 6 x 3 */
#define BT_ROWS 6
#define BT_COLS 3


/* Information about each button in the button grid */
typedef struct
{
    char *name;         /* label */
    button_id_enum id;  /* identifies the button in the gui */
    calc_op_enum cop;   /* operation (if any) to send to calculator engine */
} button_info;

/* LEFT table in float mode */
static const button_info binfo_left_float[BT_ROWS][BT_COLS] =
{
    { {"DEG", bid_deg, 0},        {"RAD", bid_rad, 0},          {"GRA", bid_grad, 0} },
    { {"HYP", bid_hyp, 0},        {"F-E", bid_fe, cop_peek},    {"EXP", bid_exp, 0} },
    { {"RAND", bid_rand, cop_rand},  {"x!",  bid_fact, cop_fact},   {"PI", bid_pi, cop_pi} },
    { {"pow", bid_pow, cop_pow},  {"root", bid_root, cop_root}, {"1/x", bid_onedx, cop_onedx} },
    { {"INV", bid_inv, 0},        {"log", bid_log, cop_log},    {"ln", bid_ln, cop_ln} },
    { {"sin", bid_sin, cop_sin},  {"cos", bid_cos, cop_cos},    {"tan", bid_tan, cop_tan} },
};

/* LEFT table in integer mode */
static const button_info binfo_left_int[BT_ROWS][BT_COLS] =
{
    { {"DEC", bid_dec, 0},           {"HEX", bid_hex, 0},            {NULL, bid_blank, 0} },
    { {"rol", bid_rol, cop_rol},     {"<<", bid_lsft, cop_lsft},     {">>", bid_rsft, cop_rsft} },
    { {"ror", bid_ror, cop_ror},     {"<< n", bid_lsftn, cop_lsftn}, {">> n", bid_rsftn, cop_rsftn} },
    { {"and", bid_and, cop_and},     {"or", bid_or, cop_or},         {"xor", bid_xor, cop_xor} },
    { {"D", bid_D, 0},               {"E", bid_E, 0},                {"F", bid_F, 0} },
    { {"A", bid_A, 0},               {"B", bid_B, 0},                {"C", bid_C, 0} },
};

/* CENTRE table in float mode */
static const button_info binfo_main_float[BT_ROWS][BT_COLS] =
{
    { {NULL, bid_blank, 0},  {"MODE", bid_flip, 0}, {NULL, bid_blank, 0} },
    { {NULL, bid_blank, 0},  {"HIST", bid_hist, 0}, {NULL, bid_blank, 0} },
    { {"7", bid_7, 0},       {"8", bid_8, 0},       {"9", bid_9, 0} },
    { {"4", bid_4, 0},       {"5", bid_5, 0},       {"6", bid_6, 0} },
    { {"1", bid_1, 0},       {"2", bid_2, 0},       {"3", bid_3, 0} },
    { {"0", bid_0, 0},       {".", bid_pnt, 0},     {"+/-", bid_pm, cop_pm} },
};

/* CENTRE table in integer mode, not quite the same */
static const button_info binfo_main_int[BT_ROWS][BT_COLS] =
{
    { {NULL, bid_blank, 0}, {"MODE", bid_flip, 0},      {NULL, bid_blank, 0} },
    { {NULL, bid_blank, 0}, {"HIST", bid_hist, 0},      {NULL, bid_blank, 0} },
    { {"7", bid_7, 0},      {"8", bid_8, 0},            {"9", bid_9, 0} },
    { {"4", bid_4, 0},      {"5", bid_5, 0},            {"6", bid_6, 0} },
    { {"1", bid_1, 0},      {"2", bid_2, 0},            {"3", bid_3, 0} },
    { {"0", bid_0, 0},      {"not", bid_com, cop_com},  {"+/-", bid_pm, cop_pm} },
};

/* RIGHT table in float mode */
static const button_info binfo_right_float[BT_ROWS][BT_COLS] =
{
    { {"M1S", bid_ms, cop_ms},   {"M1R", bid_mr, cop_mr},      {"M1+", bid_mp, cop_mp} },
    { {"M2S", bid_ms2, cop_ms2}, {"M2R", bid_mr2, cop_mr2},    {"M2+", bid_mp2, cop_mp2} },
    { {"(", bid_parl, cop_parl}, {")", bid_parr, cop_parr},    {"mod", bid_mod, cop_mod} },
    { {"*", bid_mul, cop_mul},   {"/", bid_div, cop_div},      {"sqr", bid_sqr, cop_sqr} },
    { {"+", bid_add, cop_add},   {"-", bid_sub, cop_sub},      {"sqrt", bid_sqrt, cop_sqrt} },
    { {"=", bid_eq, cop_eq},     {NULL, bid_wide, 0},          {"CLR", bid_clr, 0} },
};

/* RIGHT table in integer mode, not quite the same */
static const button_info binfo_right_int[BT_ROWS][BT_COLS] =
{
    { {"M1S", bid_ms, cop_ms},   {"M1R", bid_mr, cop_mr},      {"M1+", bid_mp, cop_mp} },
    { {"M2S", bid_ms2, cop_ms2}, {"M2R", bid_mr2, cop_mr2},    {"M2+", bid_mp2, cop_mp2} },
    { {"(", bid_parl, cop_parl}, {")", bid_parr, cop_parr},    {"mod", bid_mod, cop_mod} },
    { {"*", bid_mul, cop_mul},   {"/", bid_div, cop_div},      {"sqr", bid_sqr, cop_sqr} },
    { {"+", bid_add, cop_add},   {"-", bid_sub, cop_sub},      {"gcd", bid_gcd, cop_gcd} },
    { {"=", bid_eq, cop_eq},     {NULL, bid_wide, 0},          {"CLR", bid_clr, 0} },
};

/* Normally this will get initialised from config */
static float_digits_enum float_digits_id = FLOAT_DIGITS_10_ID;


/* helper function when entering digits */
static bool digit_entered(const button_info *binfo)
{
    if (!arg_pending)
    {
        /* start a new argument */
        last_float_format = disp_float_gmode;
        display_set_text(binfo->name);
        arg_pending = true;
        gtk_widget_set_sensitive(but_backspace, TRUE);
        return false;
    }
    else
    {
        /* append digit to existing argument */
        return display_add(binfo->name[0]);
    }
}

/* helper function to pass arg to calculator */
static void give_arg_if_pending(void)
{
    uint64_t ival;
    stackf_t fval;

    if (arg_pending)
    {
        display_get_val(&ival, &fval);
        calc_give_arg(ival, fval);
    }
    /* arg_pending cleared in gui_result_callback */
}

/* helper function for decimal point */
static void enter_point(void)
{
    if (calc_get_mode() == calc_mode_integer)
        return;

    if (!arg_pending)
    {
        last_float_format = disp_float_gmode;
        display_set_text("0.");
        arg_pending = true;
        gtk_widget_set_sensitive(but_backspace, TRUE);
    }
    else
    {
        (void)display_add('.');
    }
}

/* get the inv operation when INV selected */
static calc_op_enum cop_or_inv(calc_op_enum cop)
{
    if (!inv_selected)
        return cop;

    switch (cop)
    {
        case cop_log:
            return cop_inv_log;
        case cop_ln:
            return cop_inv_ln;
        case cop_sin:
            return cop_inv_sin;
        case cop_cos:
            return cop_inv_cos;
        case cop_tan:
            return cop_inv_tan;
        case cop_sinh:
            return cop_inv_sinh;
        case cop_cosh:
            return cop_inv_cosh;
        case cop_tanh:
            return cop_inv_tanh;
        default:
            return cop;
    }
}

/* get the hyp equivalent when HYP selected */
static calc_op_enum cop_or_hyp(calc_op_enum cop)
{
    if (!hyp_selected)
        return cop;

    switch (cop)
    {
        case cop_sin:
            return cop_sinh;
        case cop_cos:
            return cop_cosh;
        case cop_tan:
            return cop_tanh;
        default:
            return cop;
    }
}

/* switch between integer and flost mode */
static void mode_toggle(void)
{
    give_arg_if_pending();

    if (calc_get_mode() == calc_mode_integer)
    {
        calc_set_mode(calc_mode_float);
    }
    else
    {
        calc_set_mode(calc_mode_integer);
    }

    /* blow away widgets and redraw in new mode */
    GList *children, *current;

    /* I hope this ends up freeing all gui memory */
    children = gtk_container_get_children(GTK_CONTAINER(window_main));
    for (current = children; current != NULL; current = current->next)
    {
        gtk_widget_destroy(GTK_WIDGET(current->data));
    }
    g_list_free(children);

    gui_recreate();
}


/* Callback for most buttons (the special cases that aren't handled here are
 * deg/rad/grad, inv, hyp, dec/hex, backspace) */
static void button_click(GtkWidget *widget, gpointer data)
{
    (void)widget;
    const button_info *binfo = data;

    if (debug_level > 0)
    {
        g_print("************ clicked %s   id %d  cop %d\n",
                binfo->name, binfo->id, binfo->cop);
    }

    switch (binfo->id)
    {
        case bid_0:
        case bid_1:
        case bid_2:
        case bid_3:
        case bid_4:
        case bid_5:
        case bid_6:
        case bid_7:
        case bid_8:
        case bid_9:
            if (digit_entered(binfo))
            {
                /* should only ever return true if in integer dec mode, and user
                 * has just entered the +ve equivalent of int min */
                calc_give_op(cop_int_min);
            }
            break;

        case bid_A:
        case bid_B:
        case bid_C:
        case bid_D:
        case bid_E:
        case bid_F:
            if (calc_get_mode() == calc_mode_integer &&
                gui_radix == gui_radix_hex)
            {
                (void)digit_entered(binfo);
            }
            break;

        case bid_pnt:
            enter_point();
            break;

        case bid_clr:
            calc_clear();
            break;

        case bid_eq:
        case bid_add:
        case bid_sub:
        case bid_mul:
        case bid_div:
        case bid_mod:
        case bid_pow:
        case bid_root:
        case bid_and:
        case bid_or:
        case bid_xor:
        case bid_com:
        case bid_sqr:
        case bid_sqrt:
        case bid_lsft:
        case bid_rsft:
        case bid_lsftn:
        case bid_rsftn:
        //case bid_2powx:
        case bid_onedx:
        case bid_gcd:
        case bid_fact:
        case bid_rol:
        case bid_ror:
            give_arg_if_pending();
            calc_give_op(binfo->cop);
            break;

        case bid_ms:
        case bid_mr:
        case bid_mp:
        case bid_ms2:
        case bid_mr2:
        case bid_mp2:
            give_arg_if_pending();
            calc_give_op(binfo->cop);
            update_mem_label();
            break;

        case bid_pm:
            if (calc_get_mode() == calc_mode_integer)
            {
                /* Always treat as a unary minus operation in integer mode */
                give_arg_if_pending();
                calc_give_op(binfo->cop);
            }
            else
            {
                if (arg_pending)
                {
                    /* In the process of entering a value, just change the
                     * display, display takes care of whether or
                     * not it's in exponent entry mode. */
                    disp_toggle_sign();
                }
                else
                {
                    /* When applied to result value, treat as
                     * a unary minus operation */
                    calc_give_op(binfo->cop);
                }
            }
            break;

        case bid_log:
        case bid_ln:
            give_arg_if_pending();
            calc_give_op(cop_or_inv(binfo->cop));
            break;

        case bid_sin:
        case bid_cos:
        case bid_tan:
            give_arg_if_pending();
            calc_give_op(cop_or_inv(cop_or_hyp(binfo->cop)));
            break;

        case bid_parl:
        case bid_parr:
            give_arg_if_pending();
            calc_give_op(binfo->cop);
            break;

        case bid_pi:
        //case bid_eul:
        case bid_rand:
            calc_give_op(binfo->cop);
            break;

        case bid_exp:
            if (calc_get_mode() == calc_mode_float)
            {
                if (!arg_pending)
                {
                    display_set_text("1");
                    arg_pending = true;
                    gtk_widget_set_sensitive(but_backspace, TRUE);
                }
                display_set_exp_entry(!display_get_exp_entry());
            }
            break;

        case bid_fe:
            give_arg_if_pending();
            if (last_float_format == disp_float_gmode)
            {
                display_set_float_format(disp_float_emode);
            }
            calc_give_op(binfo->cop);
            break;

        case bid_hist:
            /* if already open this will actually close it */
            gui_history_open();
            return;

        case bid_flip:
            mode_toggle();
            return;

        default:
            break;
    }

    /* INV button is a one shot deal, so it always gets cleared by the
     * next button press. (The check for id != bid_inv should be unnecessary
     * since it doesn't have click signal handler, but it can't hurt). */
    if (binfo->id != bid_inv && but_grid[bid_inv] != NULL)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(but_grid[bid_inv]), FALSE);
}

/* Callback for INV button toggled */
static void inv_button_toggle(GtkWidget *widget, gpointer data)
{
    (void)data;
    gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    set_inv_button(active);
}

/* Callback for HYP button toggled */
static void hyp_button_toggle(GtkWidget *widget, gpointer data)
{
    (void)data;
    gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    set_hyp_button(active);
}

/* Callback for DEG/RAD/GRA radio button toggled */
static void deg_rad_grad_rb_toggle(GtkWidget *widget, gpointer data)
{
    const button_info *binfo = data;
    gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

    //printf("deg_rad_grad toggle %d %d\n", (int)active, binfo->id);

    if (!gui_created)
    {
        /* No need to do anything in response to a gtk_toggle_button_set_active
         * during gui_recreate. */
        //printf("deg_rad_grad toggle %d ignore at start up\n", binfo->id);
        return;
    }

    if (active && calc_get_mode() == calc_mode_float)
    {
        if (binfo->id == bid_deg)
            calc_set_angle(calc_angle_deg);
        else if (binfo->id == bid_rad)
            calc_set_angle(calc_angle_rad);
        else
            calc_set_angle(calc_angle_grad);

        set_angle_buttons(binfo->id);
    }
}

/* Callback for DEC/HEX radio button toggled */
static void dec_hex_rb_toggle(GtkWidget *widget, gpointer data)
{
    const button_info *binfo = data;
    gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

    //printf("dec hex toggle %d %d\n", (int)active, binfo->id);

    if (!gui_created)
    {
        /* No need to do anything in response to a gtk_toggle_button_set_active
         * during gui_recreate. */
        //printf("dec_hex toggle %d ignore at start up\n", binfo->id);
        return;
    }

    if (active && calc_get_mode() == calc_mode_integer)
    {
        give_arg_if_pending();

        if (binfo->id == bid_dec)
            gui_radix = gui_radix_dec;
        else
            gui_radix = gui_radix_hex;

        set_hex_buttons(binfo->id);
        /* this will update display */
        calc_give_op(cop_peek);
    }
}

/* Handle special case buttons. For example, deg/rad/grad buttons form a logical
 * radio button group, inv button is logically a checkbox etc. */
static GtkWidget *create_special_button(const button_info *binfo)
{
    GtkWidget *button = NULL;

    if (binfo->id == bid_inv)
    {
        button = gtk_check_button_new_with_label(binfo->name);
        g_signal_connect(button, "toggled",
                         G_CALLBACK(inv_button_toggle), NULL);
    }
    else if (binfo->id == bid_hyp)
    {
        button = gtk_check_button_new_with_label(binfo->name);
        g_signal_connect(button, "toggled",
                         G_CALLBACK(hyp_button_toggle), NULL);
    }
    else if (binfo->id == bid_deg || binfo->id == bid_rad || binfo->id == bid_grad)
    {
        button = gtk_radio_button_new_with_label(rb_list_grid, binfo->name);
        rb_list_grid = gtk_radio_button_get_group(GTK_RADIO_BUTTON(button));
        g_signal_connect(button, "toggled",
                         G_CALLBACK(deg_rad_grad_rb_toggle), (gpointer)binfo);
    }
    else if (binfo->id == bid_dec || binfo->id == bid_hex)
    {
        button = gtk_radio_button_new_with_label(rb_list_grid, binfo->name);
        rb_list_grid = gtk_radio_button_get_group(GTK_RADIO_BUTTON(button));
        g_signal_connect(button, "toggled",
                         G_CALLBACK(dec_hex_rb_toggle), (gpointer)binfo);
    }

    return button;
}

/* Creates a table of 6 x 3 buttons */
static GtkWidget *create_6by3_button_table(const button_info binfo[][BT_COLS])
{
    GtkWidget *table;
    GtkWidget *button;

    int but_height = config_get_but_height();

#if TARGET_GTK_VERSION == 2
    table = gtk_table_new(BT_ROWS, BT_COLS, TRUE);
    gtk_table_set_row_spacings(GTK_TABLE(table), 4);
    gtk_table_set_col_spacings(GTK_TABLE(table), 4);
#elif TARGET_GTK_VERSION == 3
    table = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(table), 4);
    gtk_grid_set_column_spacing(GTK_GRID(table), 4);
    gtk_grid_set_row_homogeneous(GTK_GRID(table), TRUE);
    gtk_grid_set_column_homogeneous(GTK_GRID(table), TRUE);
#endif

    for (int i = 0; i < BT_ROWS; i++)
    {
        for (int j = 0; j < BT_COLS; j++)
        {
            if (binfo[i][j].name != NULL)
            {
                int top = i;
                int bot = i + 1;
                int left = j;
                int right = j + 1;
                /* create a double width button */
                if (j < BT_COLS - 1 && binfo[i][j + 1].id == bid_wide)
                    right++;

                button = create_special_button(&binfo[i][j]);
                if (button == NULL)
                {
                    /* The normal case for most buttons */
                    button = gtk_button_new_with_label(binfo[i][j].name);
                    g_signal_connect(button, "clicked",
                                     G_CALLBACK(button_click), (gpointer)&binfo[i][j]);
                }

#if TARGET_GTK_VERSION == 2
                gtk_table_attach_defaults(GTK_TABLE(table), button,
                                          left, right, top, bot);
#elif TARGET_GTK_VERSION == 3
                gtk_grid_attach(GTK_GRID(table), button,
                                left, top, right - left, bot - top);
#endif
                gtk_widget_set_size_request(button, -1, but_height);
                gtk_widget_show(button);
                but_grid[binfo[i][j].id] = button;
            }
        }
    }
    gtk_widget_show(table);
    return table;
}


/* Callback for backspace button */
static void backspace_click(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;

    if (gtk_widget_get_sensitive(but_backspace))
    {
        display_remove();
    }
}


/* Callback for chkbox */
static void chk_toggle(GtkWidget *widget, gpointer data)
{
    (void)data;
    gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));
    calc_set_repeated_equals(active);
}


/* Window destroy callback */
static void destroy(GtkWidget *widget, gpointer data)
{
    (void)widget;
    (void)data;

    gui_menu_constants_deinit();
    gtk_main_quit();
}


static gboolean key_click_any_mode(GtkWidget *but)
{
    gtk_button_clicked(GTK_BUTTON(but));
    return TRUE;
}

static gboolean key_click_if_integer_mode(GtkWidget *but)
{
    if (calc_get_mode() == calc_mode_integer)
    {
        gtk_button_clicked(GTK_BUTTON(but));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static gboolean key_click_if_float_mode(GtkWidget *but)
{
    if (calc_get_mode() == calc_mode_float)
    {
        gtk_button_clicked(GTK_BUTTON(but));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/* Should probably include these from somewhere */
#define RETURN_KEY   0xff0d
#define ESC_KEY      0xff1b
#define BSP_KEY      0xff08
#define DELETE_KEY   0xffff

#define CTRL_MASK (0x04)
/* ALT modifier is probably 0x08 */
#define ALT_MASK (0x08)

#define NUM_KEYPAD_0  0xffb0
#define NUM_KEYPAD_1  0xffb1
#define NUM_KEYPAD_2  0xffb2
#define NUM_KEYPAD_3  0xffb3
#define NUM_KEYPAD_4  0xffb4
#define NUM_KEYPAD_5  0xffb5
#define NUM_KEYPAD_6  0xffb6
#define NUM_KEYPAD_7  0xffb7
#define NUM_KEYPAD_8  0xffb8
#define NUM_KEYPAD_9  0xffb9
#define NUM_KEYPAD_MUL  0xffaa
#define NUM_KEYPAD_ADD  0xffab
#define NUM_KEYPAD_SUB  0xffad
#define NUM_KEYPAD_PNT  0xffae
#define NUM_KEYPAD_DIV  0xffaf
#define NUM_KEYPAD_ENTER 0xff8d


/* Callback for key_press event */
static gboolean key_press(GtkWidget *widget, GdkEventKey *event, gpointer data)
{
    (void)widget;
    (void)data;

    /* Not sure if this is how you are meant to do keyboard shortcuts.
     * Seems to work though. */

    if (debug_level > 1)
        g_print("key_press keyval %u  state %u\n", event->keyval, event->state);

    switch (event->keyval)
    {
        case '0':
        case NUM_KEYPAD_0:
            return key_click_any_mode(but_grid[bid_0]);
        case '1':
        case NUM_KEYPAD_1:
            if (event->state & CTRL_MASK)
            {
                /* 8 bit */
                return key_click_if_integer_mode(rbut_int_width[0]);
            }
            return key_click_any_mode(but_grid[bid_1]);
        case '2':
        case NUM_KEYPAD_2:
            if (event->state & CTRL_MASK)
            {
                /* 16 bit */
                return key_click_if_integer_mode(rbut_int_width[1]);
            }
            return key_click_any_mode(but_grid[bid_2]);
        case '3':
        case NUM_KEYPAD_3:
            if (event->state & CTRL_MASK)
            {
                /* 32 bit */
                return key_click_if_integer_mode(rbut_int_width[2]);
            }
            return key_click_any_mode(but_grid[bid_3]);
        case '4':
        case NUM_KEYPAD_4:
            if (event->state & CTRL_MASK)
            {
                /* 64 bit */
                return key_click_if_integer_mode(rbut_int_width[3]);
            }
            return key_click_any_mode(but_grid[bid_4]);
        case '5':
        case NUM_KEYPAD_5:
            return key_click_any_mode(but_grid[bid_5]);
        case '6':
        case NUM_KEYPAD_6:
            return key_click_any_mode(but_grid[bid_6]);
        case '7':
        case NUM_KEYPAD_7:
            return key_click_any_mode(but_grid[bid_7]);
        case '8':
        case NUM_KEYPAD_8:
            return key_click_any_mode(but_grid[bid_8]);
        case '9':
        case NUM_KEYPAD_9:
            return key_click_any_mode(but_grid[bid_9]);

        case 'a':
        case 'A':
            return key_click_if_integer_mode(but_grid[bid_A]);
        case 'b':
        case 'B':
            return key_click_if_integer_mode(but_grid[bid_B]);
        case 'c':
        case 'C':
            if (event->state & CTRL_MASK)
            {
                clipboard_copy();
                return TRUE;
            }
            else
            {
                return key_click_if_integer_mode(but_grid[bid_C]);
            }
        case 'd':
        case 'D':
            return key_click_if_integer_mode(but_grid[bid_D]);
        case 'e':
        case 'E':
            return key_click_if_integer_mode(but_grid[bid_E]) ||
                   key_click_if_float_mode(but_grid[bid_exp]);
        case 'f':
        case 'F':
            return key_click_if_integer_mode(but_grid[bid_F]) ||
                   key_click_if_float_mode(but_grid[bid_fe]);

        case 'h':
        case 'H':
            /* ignore if ALT modified, avoid confusion with Help menu */
            if (event->state & ALT_MASK)
                return FALSE;
            return key_click_any_mode(but_grid[bid_hist]);

        case 'i':
        case 'I':
            return key_click_if_float_mode(but_grid[bid_inv]);

        case '.':
        case NUM_KEYPAD_PNT:
            return key_click_if_float_mode(but_grid[bid_pnt]);

        case 'm':
        case 'M':
            return key_click_any_mode(but_grid[bid_flip]);

        case '+':
        case NUM_KEYPAD_ADD:
            return key_click_any_mode(but_grid[bid_add]);
        case '-':
        case NUM_KEYPAD_SUB:
            return key_click_any_mode(but_grid[bid_sub]);
        case '*':
        case NUM_KEYPAD_MUL:
            return key_click_any_mode(but_grid[bid_mul]);
        case '/':
        case NUM_KEYPAD_DIV:
            return key_click_any_mode(but_grid[bid_div]);
        case RETURN_KEY:
        case NUM_KEYPAD_ENTER:
            /* not using the = key since it is easy to mistake with + */
            return key_click_any_mode(but_grid[bid_eq]);

        case '(':
            return key_click_any_mode(but_grid[bid_parl]);
        case ')':
            return key_click_any_mode(but_grid[bid_parr]);

        case 'z':
        case 'Z':
            return key_click_if_integer_mode(but_grid[bid_dec]);
        case 'x':
        case 'X':
            return key_click_if_integer_mode(but_grid[bid_hex]);

        case 'p':
        case 'P':
            return key_click_any_mode(but_grid[bid_pm]);

        case 'v':
        case 'V':
            if (event->state & CTRL_MASK)
            {
                clipboard_paste_default();
                return TRUE;
            }
            else
            {
                return FALSE;
            }

        case 'w':
        case 'W':
            if (event->state & CTRL_MASK)
            {
                clipboard_paste_primary();
                return TRUE;
            }
            else
            {
                return FALSE;
            }

        case 's':
        case 'S':
            return key_click_if_integer_mode(rbut_int_signed[INT_USE_SIGNED_ID]);
        case 'u':
        case 'U':
            return key_click_if_integer_mode(rbut_int_signed[INT_USE_UNSIGNED_ID]);

        case ESC_KEY:
        case DELETE_KEY:
            return key_click_any_mode(but_grid[bid_clr]);
        case BSP_KEY:
            return key_click_any_mode(but_backspace);

        case 'r':
        case 'R':
            return key_click_any_mode(but_repeat_eq);

        default:
            return FALSE;
    }
}


void gui_init(int debug_lvl, float_digits_enum fd)
{
    debug_level = debug_lvl;
    float_digits_id = fd;
    gui_radix = gui_radix_dec;
    calc_set_result_callback(gui_result_callback);
    calc_set_history_callback(gui_history_callback);
    calc_set_num_paren_callback(gui_set_num_used_parentheses);
    calc_set_warn_callback(gui_warn);
    calc_set_error_callback(gui_error);
    calc_set_get_best_integer_callback(display_get_best_integer);
    disp_set_warn_callback(gui_warn);
    disp_set_error_callback(gui_error);
    gui_menu_constants_init();
    gui_history_init();
}

/* Construct the gui, only ever done once */
void gui_create(void)
{
    window_main = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window_main), "ProgAndSciCalc");
    g_signal_connect(window_main, "destroy",
                     G_CALLBACK(destroy), NULL);
    gtk_container_set_border_width(GTK_CONTAINER(window_main), 10);
    g_signal_connect(window_main, "key_press_event",
                     G_CALLBACK(key_press), NULL);

    gui_recreate();
}

/* Callback for int width radio buttons */
static void int_width_rb_toggle(GtkWidget *widget, gpointer data)
{
    const INT_WIDTH_RB *info = data;
    gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

    //printf("int width toggle %d active %d\n", info->id, active);
    if (!gui_created)
    {
        return;
    }

    if (active)
    {
        give_arg_if_pending();
        calc_set_integer_width(info->id);
        /* this will update display */
        calc_give_op(cop_peek);
    }
}

/* Callback for int signed/unsigned radio buttons */
static void int_signed_rb_toggle(GtkWidget *widget, gpointer data)
{
    const INT_SIGNED_RB *info = data;
    gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

    //printf("int signed toggle %d active %d\n", info->id, active);
    if (!gui_created)
    {
        return;
    }

    if (active)
    {
        give_arg_if_pending();

        if (info->id == INT_USE_SIGNED_ID)
            calc_set_use_unsigned(false);
        else
            calc_set_use_unsigned(true);

        /* this will update display */
        calc_give_op(cop_peek);
    }
}


static void add_integer_width_rb(GtkWidget *hbox)
{
    GtkWidget *button;
    GSList *group = NULL;
    int i;

    /* Create another hbox for the widths */
    GtkWidget *hbox_w = gui_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox), hbox_w, FALSE, FALSE, 0);
    gtk_widget_show(hbox_w);

    GtkWidget *lbl = gui_label_new("Width", 0, 0.5);
    gtk_widget_show(lbl);
    gtk_box_pack_start(GTK_BOX(hbox_w), lbl, FALSE, FALSE, 0);

    for (i = 0; i < num_calc_widths; i++)
    {
        button = gtk_radio_button_new_with_label(group, int_width_rb[i].name);
        group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(button));
        if (i == (int)calc_get_integer_width())
        {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
        }
        g_signal_connect(button, "toggled",
                         G_CALLBACK(int_width_rb_toggle),
                         (gpointer)&int_width_rb[i]);
        gtk_widget_show(button);
        gtk_box_pack_start(GTK_BOX(hbox_w), button, FALSE, FALSE, 6);
        rbut_int_width[i] = button;
    }

    group = NULL;

    /* some faffing around to push signed/unsigned buttons over to the
     * right hand side */
    GtkWidget *hbox_su = gui_hbox_new(TRUE, 0);

    for (i = 0; i < NUM_INT_SIGNED_RB; i++)
    {
        button = gtk_radio_button_new_with_label(group, int_signed_rb[i].name);
        group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(button));
        if ((i == INT_USE_SIGNED_ID && !calc_get_use_unsigned()) ||
            (i == INT_USE_UNSIGNED_ID && calc_get_use_unsigned()))
        {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
        }
        g_signal_connect(button, "toggled",
                         G_CALLBACK(int_signed_rb_toggle),
                         (gpointer)&int_signed_rb[i]);
        gtk_widget_show(button);
        gtk_box_pack_start(GTK_BOX(hbox_su), button, FALSE, FALSE, 10);
        rbut_int_signed[i] = button;
    }
    gtk_widget_show(hbox_su);
#if TARGET_GTK_VERSION == 2
    GtkWidget *align = gtk_alignment_new(1, 0, 0, 0);
    gtk_widget_show(align);
    gtk_container_add(GTK_CONTAINER(align), hbox_su);
    gtk_box_pack_start(GTK_BOX(hbox), align, TRUE, TRUE, 0);
#elif TARGET_GTK_VERSION == 3
    gtk_widget_set_halign(hbox_su, GTK_ALIGN_END);
    gtk_box_pack_start(GTK_BOX(hbox), hbox_su, TRUE, TRUE, 0);
#endif
}

/* Callback for float digits radio buttons */
static void float_digits_rb_toggle(GtkWidget *widget, gpointer data)
{
    const FLOAT_DIGITS_RB *info = data;
    gboolean active = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget));

    //printf("float digits toggle %d active %d\n", info->id, active);
    if (!gui_created)
    {
        return;
    }

    if (active)
    {
        float_digits_id = info->id;
        give_arg_if_pending();
        display_set_num_float_digits(float_digits_from_id(float_digits_id));
        /* this will update display */
        calc_give_op(cop_peek);
    }
}

static void add_float_digits_rb(GtkWidget *hbox)
{
    GtkWidget *button;
    GSList *group = NULL;
    int i;

    GtkWidget *lbl = gui_label_new("Digits", 0, 0.5);
    gtk_widget_show(lbl);
    gtk_box_pack_start(GTK_BOX(hbox), lbl, FALSE, FALSE, 0);

    for (i = 0; i < NUM_FLOAT_DIGITS_ID; i++)
    {
        button = gtk_radio_button_new_with_label(group, float_digits_rb[i].name);
        group = gtk_radio_button_get_group(GTK_RADIO_BUTTON(button));
        if (i == (int)float_digits_id)
        {
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(button), TRUE);
        }
        g_signal_connect(button, "toggled",
                         G_CALLBACK(float_digits_rb_toggle),
                         (gpointer)&float_digits_rb[i]);
        gtk_widget_show(button);
        gtk_box_pack_start(GTK_BOX(hbox), button, FALSE, FALSE, 10);
    }
}



/* Most of the gui is recreated when changing mode
 * from Integer to Float and vice versa. */
static void gui_recreate(void)
{
    GtkWidget *display;
    GtkWidget *table;
    GtkWidget *vbox;
    GtkWidget *hbox_display;
    GtkWidget *hbox_buttons;
    GtkWidget *hbox_status;
    GtkWidget *hbox_pending_mem;
    GtkWidget *hbox_digits_width;
    GtkWidget *separator;
    GtkWidget *chk_repeat_eq;

    /* make sure to start out with these NULL */
    for (int i = 0; i < bid_num_displayable; i++)
    {
        but_grid[i] = NULL;
    }
    rb_list_grid = NULL;

    /* and wouldn't hurt to NULL these */
    lbl_status = NULL;
    lbl_mem = NULL;
    lbl_pending_bin_op = NULL;
    but_backspace = NULL;
    but_repeat_eq = NULL;
    for (int i = 0; i < num_calc_widths; i++)
    {
        rbut_int_width[i] = NULL;
    }
    for (int i = 0; i < NUM_INT_SIGNED_RB; i++)
    {
        rbut_int_signed[i] = NULL;
    }

    gui_created = false;

    /* Outer vbox */
    vbox = gui_vbox_new(FALSE, 0);
    gtk_container_add(GTK_CONTAINER(window_main), vbox);
    gtk_widget_show(vbox);

    /* Add menu bar into outer vbox */
    gtk_box_pack_start(GTK_BOX(vbox), create_menu(), FALSE, FALSE, 0);

    /* Add hbox_display into outer vbox */
    hbox_display = gui_hbox_new(FALSE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_display, FALSE, FALSE, 4);
    gtk_widget_show(hbox_display);

    /* Add display to hbox_display */
    display = display_widget_create(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(hbox_display), display, TRUE, TRUE, 4);
    /* Add backspace button to hbox_display */
    but_backspace = gtk_button_new_with_label("<---");
    g_signal_connect(but_backspace, "clicked",
                     G_CALLBACK(backspace_click), NULL);
    gtk_widget_show(but_backspace);
    gtk_box_pack_start(GTK_BOX(hbox_display), but_backspace, FALSE, FALSE, 4);


    /* Add separator into outer vbox */
    separator = gui_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 4);
    gtk_widget_show(separator);


    /* Add hbox_buttons into outer vbox */
    hbox_buttons = gui_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_buttons, TRUE, TRUE, 0);
    gtk_widget_show(hbox_buttons);


    /* Add left button table into hbox_buttons */
    if (calc_get_mode() == calc_mode_integer)
        table = create_6by3_button_table(binfo_left_int);
    else
        table = create_6by3_button_table(binfo_left_float);
    gtk_box_pack_start(GTK_BOX(hbox_buttons), table, TRUE, TRUE, 8);

    /* Add main buttons into hbox_buttons */
    if (calc_get_mode() == calc_mode_integer)
        table = create_6by3_button_table(binfo_main_int);
    else
        table = create_6by3_button_table(binfo_main_float);
    gtk_box_pack_start(GTK_BOX(hbox_buttons), table, TRUE, TRUE, 0);

    /* Add right button table into hbox_buttons */
    if (calc_get_mode() == calc_mode_integer)
        table = create_6by3_button_table(binfo_right_int);
    else
        table = create_6by3_button_table(binfo_right_float);
    gtk_box_pack_start(GTK_BOX(hbox_buttons), table, TRUE, TRUE, 8);


    /* Add another separator into outer vbox */
    separator = gui_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 4);
    gtk_widget_show(separator);


    /* Add hbox_status into outer vbox */
    hbox_status = gui_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(vbox), hbox_status, FALSE, FALSE, 0);
    gtk_widget_show(hbox_status);

    /* Add checkbox into hbox_status */
    chk_repeat_eq = gtk_check_button_new_with_label("Repeated Equals");
    g_signal_connect(chk_repeat_eq, "toggled",
                     G_CALLBACK(chk_toggle), NULL);
    gtk_box_pack_start(GTK_BOX(hbox_status), chk_repeat_eq, TRUE, TRUE, 0);
    but_repeat_eq = chk_repeat_eq;
    gtk_widget_show(chk_repeat_eq);

    /* Add a status label into hbox_status */
    lbl_status = gui_label_new(NULL, 0.5, 0.5);
    gtk_widget_show(lbl_status);
    gtk_box_pack_start(GTK_BOX(hbox_status), lbl_status, TRUE, TRUE, 0);

    /* Create another hbox for pending_bin_op and M1/M2 labels, and
     * add into hbox_status */
    hbox_pending_mem = gui_hbox_new(TRUE, 0);
    gtk_box_pack_start(GTK_BOX(hbox_status), hbox_pending_mem, TRUE, TRUE, 0);
    gtk_widget_show(hbox_pending_mem);

    /* Add a pending_bin_op label into hbox_pending_mem (indicates the pending
     * bin op at the top of stack, if any) */
    lbl_pending_bin_op = gui_label_new(NULL, 0.5, 0.5);
#if TARGET_GTK_VERSION == 2
    PangoFontDescription *pfd =
        pango_font_description_from_string("mono bold 12");
    gtk_widget_modify_font(lbl_pending_bin_op, pfd);
    pango_font_description_free(pfd);
#elif TARGET_GTK_VERSION == 3
    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, "*{font: mono bold 12;}", -1, NULL);
    GtkStyleContext *lbl_context = gtk_widget_get_style_context(lbl_pending_bin_op);
    gtk_style_context_add_provider(lbl_context,
                                   GTK_STYLE_PROVIDER(provider),
                                   GTK_STYLE_PROVIDER_PRIORITY_USER);
    g_object_unref(provider);
#endif
    gtk_widget_show(lbl_pending_bin_op);
    gtk_box_pack_start(GTK_BOX(hbox_pending_mem), lbl_pending_bin_op, TRUE, TRUE, 0);

    /* Add a M1/M2 label into hbox_pending_mem (indicates when non zero value
     * stored in memories) */
    lbl_mem = gui_label_new(NULL, 1.0, 0.5);
    gtk_widget_show(lbl_mem);
    gtk_box_pack_start(GTK_BOX(hbox_pending_mem), lbl_mem, TRUE, TRUE, 0);


    /* Add another separator into outer vbox */
    separator = gui_hseparator_new();
    gtk_box_pack_start(GTK_BOX(vbox), separator, FALSE, FALSE, 4);
    gtk_widget_show(separator);

    /* Add hbox_digits_width (float digits or integer width) into outer vbox */
    hbox_digits_width = gui_hbox_new(FALSE, 0);
    gtk_widget_show(hbox_digits_width);

    /* Add either float digits or integer width radio buttons into
     * hbox_digits_width */
    if (calc_get_mode() == calc_mode_integer)
    {
        add_integer_width_rb(hbox_digits_width);
    }
    else
    {
        add_float_digits_rb(hbox_digits_width);
    }
    gtk_box_pack_start(GTK_BOX(vbox), hbox_digits_width, FALSE, FALSE, 0);


    /* Put things in sane state */
    if (calc_get_mode() == calc_mode_integer)
    {
        display_set_mode(disp_mode_int);
        /* display mode int format is called from within set_hex_buttons */

        if (gui_radix == gui_radix_dec)
        {
            set_hex_buttons(bid_dec);
            /* This appears to do nothing if it is already active (which it is
             * by default after creation, presumably as it's created first) */
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(but_grid[bid_dec]), TRUE);
        }
        else
        {
            set_hex_buttons(bid_hex);
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(but_grid[bid_hex]), TRUE);
        }
    }
    else
    {
        display_set_mode(disp_mode_float);
        display_set_float_format(disp_float_gmode);
        display_set_num_float_digits(float_digits_from_id(float_digits_id));

        if (calc_get_angle() == calc_angle_deg)
        {
            set_angle_buttons(bid_deg);
            /* This appears to do nothing if it is already active (which it is
             * by default after creation, presumably as it's created first) */
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(but_grid[bid_deg]), TRUE);
        }
        else if (calc_get_angle() == calc_angle_rad)
        {
            set_angle_buttons(bid_rad);
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(but_grid[bid_rad]), TRUE);
        }
        else
        {
            set_angle_buttons(bid_grad);
            gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(but_grid[bid_grad]), TRUE);
        }
    }

    if (calc_get_repeated_equals())
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(chk_repeat_eq), TRUE);

    update_status_label();
    update_mem_label();
    inv_selected = false;
    if (hyp_selected && but_grid[bid_hyp] != NULL)
        gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(but_grid[bid_hyp]), TRUE);

    gtk_button_clicked(GTK_BUTTON(but_grid[bid_clr]));

    gtk_widget_grab_focus(but_grid[bid_eq]);

    //printf("create finished\n");
    gui_created = true;
    gtk_widget_show(window_main);
}


/* Make hex A-F buttons active/inactive */
static void set_hex_buttons(button_id_enum bid)
{
    //printf("set hex buttons %d\n", bid);

    if (bid == bid_dec)
    {
        display_set_int_format(disp_int_dec);

        gtk_widget_set_sensitive(but_grid[bid_A], FALSE);
        gtk_widget_set_sensitive(but_grid[bid_B], FALSE);
        gtk_widget_set_sensitive(but_grid[bid_C], FALSE);
        gtk_widget_set_sensitive(but_grid[bid_D], FALSE);
        gtk_widget_set_sensitive(but_grid[bid_E], FALSE);
        gtk_widget_set_sensitive(but_grid[bid_F], FALSE);
    }
    else if (bid == bid_hex)
    {
        display_set_int_format(disp_int_hex);

        gtk_widget_set_sensitive(but_grid[bid_A], TRUE);
        gtk_widget_set_sensitive(but_grid[bid_B], TRUE);
        gtk_widget_set_sensitive(but_grid[bid_C], TRUE);
        gtk_widget_set_sensitive(but_grid[bid_D], TRUE);
        gtk_widget_set_sensitive(but_grid[bid_E], TRUE);
        gtk_widget_set_sensitive(but_grid[bid_F], TRUE);
    }
    update_status_label();
}

/* Change of angle units */
static void set_angle_buttons(button_id_enum bid)
{
    (void)bid;
    //printf("set angle buttons %d\n", bid);
    update_status_label();
}


/* Change labels on sin/cos/tan buttons */
static void set_trig_but_labels(void)
{
    if (inv_selected && hyp_selected)
    {
        gtk_button_set_label(GTK_BUTTON(but_grid[bid_sin]), "asinh");
        gtk_button_set_label(GTK_BUTTON(but_grid[bid_cos]), "acosh");
        gtk_button_set_label(GTK_BUTTON(but_grid[bid_tan]), "atanh");
    }
    else if (inv_selected)
    {
        gtk_button_set_label(GTK_BUTTON(but_grid[bid_sin]), "asin");
        gtk_button_set_label(GTK_BUTTON(but_grid[bid_cos]), "acos");
        gtk_button_set_label(GTK_BUTTON(but_grid[bid_tan]), "atan");
    }
    else if (hyp_selected)
    {
        gtk_button_set_label(GTK_BUTTON(but_grid[bid_sin]), "sinh");
        gtk_button_set_label(GTK_BUTTON(but_grid[bid_cos]), "cosh");
        gtk_button_set_label(GTK_BUTTON(but_grid[bid_tan]), "tanh");
    }
    else
    {
        gtk_button_set_label(GTK_BUTTON(but_grid[bid_sin]), "sin");
        gtk_button_set_label(GTK_BUTTON(but_grid[bid_cos]), "cos");
        gtk_button_set_label(GTK_BUTTON(but_grid[bid_tan]), "tan");
    }
}

/* Change state of INV */
static void set_inv_button(bool selected)
{
    if (calc_get_mode() == calc_mode_integer)
        return;

    inv_selected = selected;

    if (selected)
    {
        gtk_button_set_label(GTK_BUTTON(but_grid[bid_log]), "10^x");
        gtk_button_set_label(GTK_BUTTON(but_grid[bid_ln]), "e^x");
    }
    else
    {
        gtk_button_set_label(GTK_BUTTON(but_grid[bid_log]), "log");
        gtk_button_set_label(GTK_BUTTON(but_grid[bid_ln]), "ln");

    }
    set_trig_but_labels();
}

/* Change state of HYP */
static void set_hyp_button(bool selected)
{
    if (calc_get_mode() == calc_mode_integer)
        return;

    hyp_selected = selected;
    set_trig_but_labels();
}


/* update msg displayed in the status label */
static void update_status_label(void)
{
    if (calc_get_mode() == calc_mode_integer)
    {
        if (gui_radix == gui_radix_dec)
            gtk_label_set_text(GTK_LABEL(lbl_status), "INTEGER : DEC");
        else
            gtk_label_set_text(GTK_LABEL(lbl_status), "INTEGER : HEX");
    }
    else
    {
        if (calc_get_angle() == calc_angle_deg)
            gtk_label_set_text(GTK_LABEL(lbl_status), "FLOATING : DEG");
        else if (calc_get_angle() == calc_angle_rad)
            gtk_label_set_text(GTK_LABEL(lbl_status), "FLOATING : RAD");
        else
            gtk_label_set_text(GTK_LABEL(lbl_status), "FLOATING : GRAD");
    }
}

/* update indication of non zero value stored in memories */
static void update_mem_label(void)
{
    bool m1 = calc_get_mem_non_zero(0);
    bool m2 = calc_get_mem_non_zero(1);
    if (m1 && m2)
        gtk_label_set_text(GTK_LABEL(lbl_mem), "M1 M2");
    else if (m1)
        gtk_label_set_text(GTK_LABEL(lbl_mem), "M1");
    else if (m2)
        gtk_label_set_text(GTK_LABEL(lbl_mem), "M2");
    else
        gtk_label_set_text(GTK_LABEL(lbl_mem), "");
}



/* Display the bin operator at the top of stack (if any) */
static void show_pending_bin_op(void)
{
    calc_op_enum cop = calc_get_top_of_bop_stack();

    const char *new_name;

    switch (cop)
    {
    case cop_add:
        new_name = "+";
        break;
    case cop_sub:
        new_name = "-";
        break;
    case cop_mul:
        new_name = "*";
        break;
    case cop_div:
        new_name = "/";
        break;
    case cop_mod:
        new_name = "mod";
        break;
    case cop_and:
        new_name = "and";
        break;
    case cop_or:
        new_name = "or";
        break;
    case cop_xor:
        new_name = "xor";
        break;
    case cop_pow:
        new_name = "pow";
        break;
    case cop_root:
        new_name = "root";
        break;
    case cop_gcd:
        new_name = "gcd";
        break;
    case cop_lsftn:
        new_name = "<<";
        break;
    case cop_rsftn:
        new_name = ">>";
        break;
    default:
        new_name = "";
        break;
    }

    gtk_label_set_text(GTK_LABEL(lbl_pending_bin_op), new_name);
}


/* Called by calculator after all operations */
static void gui_result_callback(uint64_t ival, stackf_t fval)
{
    last_float_format = display_get_float_format();
    /* this automatically clears disp exp_entry */
    display_set_val(ival, fval);
    display_set_float_format(disp_float_gmode);
    gtk_widget_set_sensitive(but_backspace, FALSE);
    arg_pending = false;
    show_pending_bin_op();
}

/* Called by calculator each time a new value is pushed to stack. */
static void gui_history_callback(uint64_t ival, stackf_t fval)
{
    gui_history_add(ival, fval);
}


/* Called by calculator to indicate the num of parentheses, the number
 * will be displayed on the ( but label */
static void gui_set_num_used_parentheses(int n)
{
    char buf[20];
    if (n > 0)
        sprintf(buf, "(%d", n);
    else
        sprintf(buf, "(");
    gtk_button_set_label(GTK_BUTTON(but_grid[bid_parl]), buf);
}


/* Callback for dialog response */
static void dialog_response(GtkDialog *dialog, gint response_id, gpointer data)
{
    (void)response_id;
    (void)data;
    gtk_widget_destroy(GTK_WIDGET(dialog));
}

/* Dialogs for warnings and, in case of bugs, errors */
static void gui_warn(const char *msg)
{
    GtkWidget *dialog;
    dialog = gtk_message_dialog_new(
                GTK_WINDOW(window_main),
                GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_WARNING,
                GTK_BUTTONS_OK,
                "%s", msg);
    g_signal_connect(dialog, "response",
                     G_CALLBACK(dialog_response), NULL);
    gtk_window_set_title(GTK_WINDOW(dialog), "Warning");
    gtk_widget_show(dialog);
}

static void gui_error(const char *msg)
{
    GtkWidget *dialog;
    dialog = gtk_message_dialog_new(
                GTK_WINDOW(window_main),
                GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
                GTK_MESSAGE_ERROR,
                GTK_BUTTONS_OK,
                "%s", msg);
    g_signal_connect(dialog, "response",
                     G_CALLBACK(dialog_response), NULL);
    gtk_window_set_title(GTK_WINDOW(dialog), "Error");
    gtk_widget_show(dialog);
}


/* pass value that user has entered into calculator */
void gui_give_arg_if_pending(void)
{
    give_arg_if_pending();
}


/* Strip leading whitespace and terminate at first non valid character, this will
 * give more chance that the pasted text will be acceptable to dfp_from_string */
static char *massage_float_txt(char *buf)
{
    /* strip leading whitespace */
    char *p = buf;
    while (*p != '\0' && *p <= ' ')
    {
        p++;
    }
    char *start = p;

    /* terminate at first non valid character */
    const char *valid = "0123456789.+-eE";
    while (*p != '\0')
    {
        if (!strchr(valid, *p))
        {
            *p = '\0';
            break;
        }
        p++;
    }

    return start;
}

static bool pasted_value_is_negative(const char *text)
{
    /* if first non white space char is a '-' */
    while (*text && *text <= ' ')
    {
        text++;
    }
    return *text == '-';
}

static void clipboard_copy(void)
{
    /* write to both default and primary seems to cover most possibilities */
    GtkClipboard *clip_default = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    GtkClipboard *clip_primary = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
    gtk_clipboard_set_text(clip_default, display_get_text(), -1);
    gtk_clipboard_set_text(clip_primary, display_get_text(), -1);
}


static const char *width_changed_warn = "Integer width changed to make value fit";
static const char *neg_range_warn = "Negative value was out of range (< INT64_MIN)";
static const char *pos_range_warn = "Positive value was out of range (> UINT64_MAX)";

static void clipboard_paste_callback(GtkClipboard *clipboard, const gchar *text, gpointer data)
{
    (void)clipboard;
    (void)data;

    if (text == NULL)
    {
        return;
    }

    if (calc_get_mode() == calc_mode_integer)
    {
        stackf_t dzero;
        dfp_zero(&dzero);

        bool ok;
        const char *msg = NULL;
        uint64_t uval;
        int base = gui_radix == gui_radix_dec ? 10 : 16;
        calc_width_enum current_width = calc_get_integer_width();
        calc_width_enum selected_width = current_width;
        bool negative = pasted_value_is_negative(text);

        if (negative)
        {
            ok = calc_util_signed_str_to_ival(text, calc_width_64, &uval, base);
        }
        else
        {
            ok = calc_util_unsigned_str_to_ival(text, calc_width_64, &uval, base);
        }

        if (ok)
        {
            /* The number was in the range of s64 (if it was negative) or
             * u64 (if it was positive), so there must be a calc width that fits.
             * See if we need to change width to make it fit. */
            selected_width = calc_util_get_changed_width(uval, negative, current_width);
            if (selected_width != current_width)
            {
                msg = width_changed_warn;
            }
        }
        else
        {
            /* The number was outside the range of s64 (if negative) or
             * u64 (if positive). Just return 0 and leave width unchanged. */
            uval = 0;
            msg = negative ? neg_range_warn : pos_range_warn;
        }

        if (selected_width == calc_width_8)
            gtk_button_clicked(GTK_BUTTON(rbut_int_width[0]));
        else if (selected_width == calc_width_16)
            gtk_button_clicked(GTK_BUTTON(rbut_int_width[1]));
        else if (selected_width == calc_width_32)
            gtk_button_clicked(GTK_BUTTON(rbut_int_width[2]));
        else
            gtk_button_clicked(GTK_BUTTON(rbut_int_width[3]));

        calc_give_arg(uval, dzero);
        calc_give_op(cop_peek);
        if (msg != NULL)
        {
            gui_warn(msg);
        }
    }
    else
    {
        /* dfp_from_string isn't very forgiving so pre process the text */
        char temp[DFP_STRING_MAX];
        temp[0] = '\0';
        strncat(temp, text, DFP_STRING_MAX-1);
        stackf_t fval;
        dfp_from_string(&fval, massage_float_txt(temp), &dfp_context);
        calc_give_arg(0, fval);
        calc_give_op(cop_peek);
    }
}

static void clipboard_paste_default(void)
{
    GtkClipboard *clip = gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    gtk_clipboard_request_text(clip, clipboard_paste_callback, NULL);
}

static void clipboard_paste_primary(void)
{
    GtkClipboard *clip = gtk_clipboard_get(GDK_SELECTION_PRIMARY);
    gtk_clipboard_request_text(clip, clipboard_paste_callback, NULL);
}
