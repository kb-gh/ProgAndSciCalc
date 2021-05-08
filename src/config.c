/*****************************************************************************
 * File config.c part of ProgAndSciCalc
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
#include <stdbool.h>
#include "config.h"

/* startup in either floating or integer mode */
static calc_mode_enum calc_mode;

/* 0, 4, or 8 */
static int hex_grouping;

/* true means 0<=r<1, false means 1<=r<=random_n */
static bool random_01;
static int random_n;

/* extra rounding for sin cos tan */
static bool use_sct_rounding;

/* fontsizes for the main and binary display */
static int main_disp_fontsize;
static int bin_disp_fontsize;
/* fontsize for pending binary operator label */
static int binop_lbl_fontsize;

/* button height */
static int but_height;

/* number of digits displayed at startup in floating point mode */
static float_digits_enum float_digits;

/* integer width to use at startup */
static calc_width_enum integer_width;

/* use signed/unsigned type at startup */
static bool use_unsigned;

static bool warn_on_signed_overflow;
static bool warn_on_unsigned_overflow;

/* if you don't like the zero character in monospace font used for main
 * and binary displays, try using capital O instead */
static bool replace_zero_with_o;


static const char *CALCDIR = ".ProgAndSciCalc";
static const char *CONFIGFILE = "config";
static const char *SETTINGS = "Settings";
static const char *MAIN_FONTSIZE = "MainFontsize";
static const char *BIN_FONTSIZE = "BinFontsize";
static const char *BINOP_LBL_FONTSIZE = "BinOpLblFontsize";
static const char *BUT_HEIGHT = "ButHeight";
static const char *CALC_MODE = "CalcMode";
static const char *HEX_GROUP = "HexGroup";
static const char *RANDOM_01 = "Random01";
static const char *RANDOM_N = "RandomN";
static const char *SCT_ROUND = "SCTRounding";
static const char *FLOAT_DIGITS = "FloatDigits";
static const char *INTEGER_WIDTH = "IntegerWidth";
static const char *USE_UNSIGNED = "UseUnsigned";
static const char *WARN_SIGNED_OVERFLOW = "WarnSignedOverflow";
static const char *WARN_UNSIGNED_OVERFLOW = "WarnUnsignedOverflow";
static const char *REPLACE_ZERO_WITH_O = "ReplaceZeroWithO";

#define CALC_MODE_DEFAULT calc_mode_float
#define FLOAT_DIGITS_DEFAULT FLOAT_DIGITS_10_ID
#define INTEGER_WIDTH_DEFAULT calc_width_64

static int get_integer(GKeyFile *keyfile, const char *grp, const char *keyname, int default_val)
{
    GError *gerror = NULL;
    int res = g_key_file_get_integer(keyfile, grp, keyname, &gerror);
    if (gerror != NULL)
    {
        //printf("get_integer GError != NULL\n");
        res = default_val;
        g_error_free(gerror);
    }
    return res;
}

static bool get_boolean(GKeyFile *keyfile, const char *grp, const char *keyname, bool default_val)
{
    GError *gerror = NULL;
    bool res = g_key_file_get_boolean(keyfile, grp, keyname, &gerror);
    if (gerror != NULL)
    {
        //printf("get_boolean GError != NULL\n");
        res = default_val;
        g_error_free(gerror);
    }
    return res;
}

static gchar *get_full_config_filename(void)
{
    return g_build_filename(g_get_home_dir(), CALCDIR, CONFIGFILE, NULL);
}

void config_init(void)
{
    /* get from file, or fall back to defaults if no file found */
    gchar *filename = get_full_config_filename();
    GKeyFile *keyfile = g_key_file_new();
    bool file_ok = g_key_file_load_from_file(keyfile, filename, G_KEY_FILE_NONE, NULL);

    if (file_ok)
    {
        main_disp_fontsize = get_integer(keyfile, SETTINGS, MAIN_FONTSIZE, MAIN_FONTSIZE_DEFAULT);
        if (main_disp_fontsize < MAIN_FONTSIZE_MIN ||
            main_disp_fontsize > MAIN_FONTSIZE_MAX)
        {
            //printf("main fontsize invalid use default\n");
            main_disp_fontsize = MAIN_FONTSIZE_DEFAULT;
        }

        bin_disp_fontsize = get_integer(keyfile, SETTINGS, BIN_FONTSIZE, BIN_FONTSIZE_DEFAULT);
        if (bin_disp_fontsize < BIN_FONTSIZE_MIN ||
            bin_disp_fontsize > BIN_FONTSIZE_MAX)
        {
            //printf("bin fontsize invalid use default\n");
            bin_disp_fontsize = BIN_FONTSIZE_DEFAULT;
        }

        binop_lbl_fontsize = get_integer(keyfile, SETTINGS, BINOP_LBL_FONTSIZE, BINOP_LBL_FONTSIZE_DEFAULT);
        if (binop_lbl_fontsize < BINOP_LBL_FONTSIZE_MIN ||
            binop_lbl_fontsize > BINOP_LBL_FONTSIZE_MAX)
        {
            binop_lbl_fontsize = BINOP_LBL_FONTSIZE_DEFAULT;
        }

        but_height = get_integer(keyfile, SETTINGS, BUT_HEIGHT, BUT_HEIGHT_DEFAULT);
        if (but_height < BUT_HEIGHT_MIN ||
            but_height > BUT_HEIGHT_MAX)
        {
            but_height = BUT_HEIGHT_DEFAULT;
        }

        int cm = get_integer(keyfile, SETTINGS, CALC_MODE, CALC_MODE_DEFAULT);
        if (cm < (int)calc_mode_integer || cm > (int)calc_mode_float)
        {
            cm = CALC_MODE_DEFAULT;
        }
        calc_mode = (calc_mode_enum)cm;

        hex_grouping = get_integer(keyfile, SETTINGS, HEX_GROUP, 4);
        if (hex_grouping != 0 && hex_grouping != 4 && hex_grouping != 8)
        {
            //printf("hex grouping invalid use default\n");
            hex_grouping = 4;
        }

        random_01 = get_boolean(keyfile, SETTINGS, RANDOM_01, true);

        /* returns 0 on error and 0 is not valid, so not bothering with GError */
        random_n = get_integer(keyfile, SETTINGS, RANDOM_N, RANDOM_N_DEFAULT);
        if (random_n < RANDOM_N_MIN || random_n > RANDOM_N_MAX)
        {
            //printf("random_n invalid use default\n");
            random_n = RANDOM_N_DEFAULT;
        }

        use_sct_rounding = get_boolean(keyfile, SETTINGS, SCT_ROUND, true);

        int fd = get_integer(keyfile, SETTINGS, FLOAT_DIGITS, FLOAT_DIGITS_DEFAULT);
        if (fd < (int)FLOAT_DIGITS_8_ID || fd >= (int)NUM_FLOAT_DIGITS_ID)
        {
            fd = FLOAT_DIGITS_DEFAULT;
        }
        float_digits = (float_digits_enum)fd;

        int iw = get_integer(keyfile, SETTINGS, INTEGER_WIDTH, INTEGER_WIDTH_DEFAULT);
        if (iw < (int)calc_width_8 || iw >= (int)num_calc_widths)
        {
            iw = INTEGER_WIDTH_DEFAULT;
        }
        integer_width = (calc_width_enum)iw;

        use_unsigned = get_boolean(keyfile, SETTINGS, USE_UNSIGNED, false);
        warn_on_signed_overflow = get_boolean(keyfile, SETTINGS, WARN_SIGNED_OVERFLOW, true);
        warn_on_unsigned_overflow = get_boolean(keyfile, SETTINGS, WARN_UNSIGNED_OVERFLOW, true);
        replace_zero_with_o = get_boolean(keyfile, SETTINGS, REPLACE_ZERO_WITH_O, false);
    }
    else
    {
        //printf("keyfile error (probably not found)\n");
        calc_mode = CALC_MODE_DEFAULT;
        hex_grouping = 4;
        random_01 = true;
        random_n = RANDOM_N_DEFAULT;
        use_sct_rounding = true;
        main_disp_fontsize = MAIN_FONTSIZE_DEFAULT;
        bin_disp_fontsize = BIN_FONTSIZE_DEFAULT;
        binop_lbl_fontsize = BINOP_LBL_FONTSIZE_DEFAULT;
        but_height = BUT_HEIGHT_DEFAULT;
        float_digits = FLOAT_DIGITS_DEFAULT;
        integer_width = INTEGER_WIDTH_DEFAULT;
        use_unsigned = false;
        warn_on_signed_overflow = true;
        warn_on_unsigned_overflow = true;
        replace_zero_with_o = false;
    }
    g_free(filename);
    g_key_file_free(keyfile);
}

void config_save(void)
{
    /* write to file */

    /* g_key_file_save_to_file() doesn't seem to exist on some platforms,
     * so write the file manually */
#if 1
    FILE *fp;
    gchar *filename = get_full_config_filename();
    fp = fopen(filename, "w");
    if (fp == NULL)
    {
        //printf("couldn't open file %s for saving\n", filename);
        g_free(filename);
        return;
    }
    g_free(filename);
    fprintf(fp, "[%s]\n", SETTINGS);
    fprintf(fp, "%s=%d\n", MAIN_FONTSIZE, main_disp_fontsize);
    fprintf(fp, "%s=%d\n", BIN_FONTSIZE, bin_disp_fontsize);
    fprintf(fp, "%s=%d\n", BINOP_LBL_FONTSIZE, binop_lbl_fontsize);
    fprintf(fp, "%s=%d\n", BUT_HEIGHT, but_height);
    fprintf(fp, "%s=%d\n", CALC_MODE, (int)calc_mode);
    fprintf(fp, "%s=%d\n", HEX_GROUP, hex_grouping);
    fprintf(fp, "%s=%s\n", RANDOM_01, random_01 ? "true": "false");
    fprintf(fp, "%s=%d\n", RANDOM_N, random_n);
    fprintf(fp, "%s=%s\n", SCT_ROUND, use_sct_rounding ? "true": "false");
    fprintf(fp, "%s=%d\n", FLOAT_DIGITS, (int)float_digits);
    fprintf(fp, "%s=%d\n", INTEGER_WIDTH, (int)integer_width);
    fprintf(fp, "%s=%s\n", USE_UNSIGNED, use_unsigned ? "true": "false");
    fprintf(fp, "%s=%s\n", WARN_SIGNED_OVERFLOW, warn_on_signed_overflow ? "true": "false");
    fprintf(fp, "%s=%s\n", WARN_UNSIGNED_OVERFLOW, warn_on_unsigned_overflow ? "true": "false");
    fprintf(fp, "%s=%s\n", REPLACE_ZERO_WITH_O, replace_zero_with_o ? "true": "false");
    fclose(fp);

#else
    GKeyFile *keyfile = g_key_file_new();
    g_key_file_set_integer(keyfile, SETTINGS, MAIN_FONTSIZE, main_disp_fontsize);
    g_key_file_set_integer(keyfile, SETTINGS, BIN_FONTSIZE, bin_disp_fontsize);
    g_key_file_set_integer(keyfile, SETTINGS, BINOP_LBL_FONTSIZE, binop_lbl_fontsize);
    g_key_file_set_integer(keyfile, SETTINGS, BUT_HEIGHT, but_height);
    g_key_file_set_integer(keyfile, SETTINGS, CALC_MODE, (int)calc_mode);
    g_key_file_set_integer(keyfile, SETTINGS, HEX_GROUP, hex_grouping);
    g_key_file_set_boolean(keyfile, SETTINGS, RANDOM_01, random_01);
    g_key_file_set_integer(keyfile, SETTINGS, RANDOM_N, random_n);
    g_key_file_set_boolean(keyfile, SETTINGS, SCT_ROUND, use_sct_rounding);
    g_key_file_set_integer(keyfile, SETTINGS, FLOAT_DIGITS, (int)float_digits);
    g_key_file_set_integer(keyfile, SETTINGS, INTEGER_WIDTH, (int)integer_width);
    g_key_file_set_boolean(keyfile, SETTINGS, USE_UNSIGNED, use_unsigned);
    g_key_file_set_boolean(keyfile, SETTINGS, WARN_SIGNED_OVERFLOW, warn_on_signed_overflow);
    g_key_file_set_boolean(keyfile, SETTINGS, WARN_UNSIGNED_OVERFLOW, warn_on_unsigned_overflow);
    g_key_file_set_boolean(keyfile, SETTINGS, REPLACE_ZERO_WITH_O, replace_zero_with_o);

    gchar *filename = get_full_config_filename();
    g_key_file_save_to_file(keyfile, filename, NULL);
    g_free(filename);
    g_key_file_free(keyfile);
#endif
}

void config_set_calc_mode(calc_mode_enum mode)
{
    calc_mode = mode;
}

calc_mode_enum config_get_calc_mode(void)
{
    return calc_mode;
}

void config_set_hex_grouping(int hg)
{
    hex_grouping = hg;
}

int config_get_hex_grouping(void)
{
    return hex_grouping;
}

void config_set_random_01(bool en)
{
    random_01 = en;
}

bool config_get_random_01(void)
{
    return random_01;
}

void config_set_random_n(int n)
{
    random_n = n;
}

int config_get_random_n(void)
{
    return random_n;
}

void config_set_use_sct_rounding(bool en)
{
    use_sct_rounding = en;
}

bool config_get_use_sct_rounding(void)
{
    return use_sct_rounding;
}

void config_set_main_disp_fontsize(int fs)
{
    main_disp_fontsize = fs;
}

int config_get_main_disp_fontsize(void)
{
    return main_disp_fontsize;
}

void config_set_bin_disp_fontsize(int fs)
{
    bin_disp_fontsize = fs;
}

int config_get_bin_disp_fontsize(void)
{
    return bin_disp_fontsize;
}

void config_set_binop_lbl_fontsize(int fs)
{
    binop_lbl_fontsize = fs;
}

int config_get_binop_lbl_fontsize(void)
{
    return binop_lbl_fontsize;
}

void config_set_but_height(int h)
{
    but_height = h;
}

int config_get_but_height(void)
{
    return but_height;
}

void config_set_float_digits(float_digits_enum fd)
{
    float_digits = fd;
}

float_digits_enum config_get_float_digits(void)
{
    return float_digits;
}

const char *config_get_dirname(void)
{
    return CALCDIR;
}

void config_set_integer_width(calc_width_enum width)
{
    integer_width = width;
}

calc_width_enum config_get_integer_width(void)
{
    return integer_width;
}

void config_set_use_unsigned(bool en)
{
    use_unsigned = en;
}

bool config_get_use_unsigned(void)
{
    return use_unsigned;
}

void config_set_warn_on_signed_overflow(bool en)
{
    warn_on_signed_overflow = en;
}

bool config_get_warn_on_signed_overflow(void)
{
    return warn_on_signed_overflow;
}

void config_set_warn_on_unsigned_overflow(bool en)
{
    warn_on_unsigned_overflow = en;
}

bool config_get_warn_on_unsigned_overflow(void)
{
    return warn_on_unsigned_overflow;
}

void config_set_replace_zero_with_o(bool en)
{
    replace_zero_with_o = en;
}

bool config_get_replace_zero_with_o(void)
{
    return replace_zero_with_o;
}
