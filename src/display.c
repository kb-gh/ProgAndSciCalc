/*****************************************************************************
 * File display.c part of ProgAndSciCalc
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


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <inttypes.h>

#include "display.h"
#include "display_widget.h"
#include "display_print.h"
#include "calc.h"


/* normally initialised form gui */
static disp_mode_enum disp_mode;
static disp_int_format_enum int_format;
static disp_float_format_enum float_format;
static int disp_float_num_digits = 10;

static void (*warn_callback)(const char *msg);
static void (*error_callback)(const char *msg);


#define DISP_MAX_DIGIT_HEX 16
/* allow for groups of 4, so 3 spaces in between 4 groups of 4 */
#define DISP_MAX_DIGIT_HEX_SPACED (DISP_MAX_DIGIT_HEX + 3)
/* Make big enough for any decQuad, although in practice the number of digits
 * displayed is restricted. This is also plenty big enough for 64 bit integer
 * (19 digits signed, 20 digits unsigned). */
#define DISP_STACK_SIZE (DFP_STRING_MAX)
#define DISP_MAX_LEN (DISP_STACK_SIZE - 1)

/* display txt is maintained in disp_stack array as a null terminated str */
static char disp_stack[DISP_STACK_SIZE];
static int disp_stack_index;

/* the number of digits to display in exponent entry mode */
#define EXP_DIGITS 3
/* are we in exponent entry mode */
static bool disp_exp_entry;


/* used to construct display when hex grouping is enabled,
 * +1 for null termination */
static char disp_hex_spaced[DISP_MAX_DIGIT_HEX_SPACED + 1];

/* Store ival since it is calculated on the fly for the bin display */
static uint64_t disp_ival;

/* 0, 4, or 8 */
static int hex_grouping;

//#define DISP_INFO
/* debugging info */
#ifdef DISP_INFO
static void disp_info(const char *msg)
{
    printf("disp info:  %s\n", msg);
}

static void print_disp_stack(void)
{
    printf("  ");
    for (int i = 0; i < DISP_STACK_SIZE; i++)
    {
        printf("%c", disp_stack[i] ? disp_stack[i] : '*');
    }
    printf("\n");
    printf("  disp stack index %d\n", disp_stack_index);
}
#else
#define disp_info(msg) ((void)0)
#define print_disp_stack() ((void)0)
#endif

static void disp_error(const char *msg)
{
    if (error_callback)
        error_callback(msg);
    else
    {
        fprintf(stderr, "*************** DISP ERROR : %s\n", msg);
        exit(1);
    }
}

#if 0
/* currently unused */
static void disp_warn(const char *msg)
{
    if (warn_callback)
        warn_callback(msg);
    else
        printf("**************** DISP WARN *** : %s\n", msg);
}
#endif

#define disp_stack_num_chars() disp_stack_index

static void disp_stack_push(char c, int limit)
{
    if (limit < 0 || limit > DISP_MAX_LEN)
        limit = DISP_MAX_LEN;

    if (disp_stack_index < limit)
    {
        disp_info("disp stack push");
        disp_stack[disp_stack_index] = c;
        disp_stack_index++;
        /* The whole stack was reset to 0 so no need to zero out here. */
        print_disp_stack();
    }
    else
    {
        /* This can be expected on user entry, not an error, do nothing. */
    }
}

static char disp_stack_pop(void)
{
    if (disp_stack_index > 0)
    {
        char c;
        disp_info("disp stack pop");
        disp_stack_index--;
        c = disp_stack[disp_stack_index];
        /* maintain null termination */
        disp_stack[disp_stack_index] = 0;
        print_disp_stack();
        return c;
    }
    else
    {
        disp_error("disp stack pop empty");
        return 0;
    }
}


/* Adjusts the display based on hex grouping if in integer/hex mode.
 * grouping assumed to be either 4 or 8 only. */
static void update_display_hex(int grouping)
{
    /* don't change disp_stack, construct separate disp_hex_spaced array,
     * working backwards */

    int si = DISP_MAX_DIGIT_HEX_SPACED; /* spaced index */

    /* null terminator */
    disp_hex_spaced[si--] = 0;

    int count = 0;
    int oi = disp_stack_index - 1;  /* original index */
    while(oi >= 0)
    {
        if (count == grouping)
        {
            disp_hex_spaced[si--] = ' ';
            count = 0;
        }
        disp_hex_spaced[si--] = disp_stack[oi--];
        count++;
    }
    si++;
    display_widget_main_set_text(&disp_hex_spaced[si]);
}


static bool is_equivalent_of_int_min(calc_width_enum width)
{
    /* check disp_stack to see if it holds the specific value */

    switch (width)
    {
    case calc_width_8:
        return strcmp(disp_stack, "128") == 0;
    case calc_width_16:
        return strcmp(disp_stack, "32768") == 0;
    case calc_width_32:
        return strcmp(disp_stack, "2147483648") == 0;
    default:
        return strcmp(disp_stack, "9223372036854775808") == 0;
    }
}


#define MAX_DEC_POP_COUNT 1

/* See description for display_add in the header file for explanation of
 * the return value. */
static bool update_display()
{
    bool ret = false;

    if (disp_mode == disp_mode_int)
    {
        calc_width_enum width = calc_get_integer_width();

        /* Since, for the purpose of the bin display, the ival needs to be
         * calculated each time it changes, may as well convert and store
         * the result. We can also use this to limit the user input range
         * in dec mode. */
        if (int_format == disp_int_dec)
        {
            if (calc_get_use_unsigned())
            {
                int count = 0;
                while (!calc_util_dec_unsigned_str_to_ival(disp_stack, width, &disp_ival))
                {
                    /* Fails if user entered out of range number.
                     * Remove a digit and try again, should only ever go round
                     * this loop once. */
                    disp_stack_pop();
                    if (++count > MAX_DEC_POP_COUNT)
                    {
                        /* should never happen */
                        disp_error("Bad conversion dec to integer");
                    }
                }
            }
            else
            {
                int count = 0;
                while (!calc_util_dec_signed_str_to_ival(disp_stack, width, &disp_ival))
                {
                    /* Fails if user has entered out of range number. Check for
                     * the special case of the +ve equivalent of int_min. */
                    if (is_equivalent_of_int_min(width))
                    {
                        ret = true;
                    }
                    /* Remove a digit and try again, should only ever go round
                     * this loop once (if above int_min test is true, the value
                     * gets sorted out after this func returns). */
                    disp_stack_pop();
                    if (++count > MAX_DEC_POP_COUNT)
                    {
                        /* should never happen */
                        disp_error("Bad conversion dec to integer");
                    }
                }
            }
        }
        else
        {
            /* For hex mode, the number of digits entered has already been
             * limited based on the the width. */
            if (!calc_util_hex_strtoull(disp_stack, width, &disp_ival))
            {
                /* should never happen */
                disp_error("Bad conversion hex to integer");
            }
        }

        display_widget_bin_set_val(disp_ival, width);
    }

    /* allow for hex grouping */
    if (disp_mode == disp_mode_int && int_format == disp_int_hex)
    {
        if (hex_grouping == 4 || hex_grouping == 8)
        {
            update_display_hex(hex_grouping);
        }
        else
        {
            display_widget_main_set_text(disp_stack);
        }
    }
    else
    {
        display_widget_main_set_text(disp_stack);
    }

    return ret;
}


void display_set_text(const char *msg)
{
    /* Reset all stack to 0, rest of the code assumes that all unused entries
     * are 0 so eg. no need to add a 0 after pushing char. */
    memset(disp_stack, 0, sizeof(disp_stack));
    disp_stack_index = 0;
    /* force to false */
    disp_exp_entry = false;

    while (*msg)
    {
        disp_stack_push(*msg, -1);
        msg++;
    }
    (void)update_display();
}


const char *display_get_text(void)
{
    return disp_stack;
}


static void remove_exponent(void)
{
    /* Shuffle exp digits right, fill in 0 from left */
    int right = disp_stack_index - 1;
    for (int i = 0; i < EXP_DIGITS - 1; i++)
    {
        disp_stack[right] = disp_stack[right - 1];
        right--;
    }
    disp_stack[right] = '0';
    (void)update_display();
}

void display_remove(void)
{
    if (disp_exp_entry)
    {
        remove_exponent();
        return;
    }

    if (disp_stack_index < 1)
    {
        /* shouldn't happen */
        return;
    }
    else if (disp_stack_index == 1)
    {
        /* just covert what's left to a zero */
        disp_stack[0] = '0';
    }
    else
    {
        disp_stack_pop();
        /* convert '-' sign if it's all that is left */
        if (disp_stack_index == 1 && disp_stack[0] == '-')
            disp_stack[0] = '0';
    }
    (void)update_display();
}

static void add_exponent(char c)
{
    if (c ==  '.')
        return;
    /* Shuffle exp digits left, add in the new one on the right. */
    int left = disp_stack_index - EXP_DIGITS;
    for (int i = 0; i < EXP_DIGITS - 1; i++)
    {
        disp_stack[left] = disp_stack[left + 1];
        left++;
    }
    disp_stack[left] = c;
    (void)update_display();
}

bool display_add(char c)
{
    if (disp_exp_entry)
    {
        add_exponent(c);
        return false;
    }

    int limit;

    /* suppress leading zeros */
    if ((disp_stack_index == 1 && disp_stack[0] == '0') ||
        (disp_stack_index == 2 && disp_stack[0] == '-' && disp_stack[1] == '0'))
    {
        if (c == '0')
            return false;
        else if (c != '.')
        {
            /* so any non zero digit replaces the zero */
            disp_stack_pop();
        }
    }

    if (disp_mode == disp_mode_int)
    {
        if (int_format == disp_int_dec)
        {
            /* limited by range check by str_to_ival in update_display */
            limit = -1;
        }
        else
        {
            /* For hex it is easier to simply limit the num digits */
            calc_width_enum width = calc_get_integer_width();
            if (width == calc_width_8)
                limit = 2;
            else if (width == calc_width_16)
                limit = 4;
            else if (width == calc_width_32)
                limit = 8;
            else
                limit = 16;
        }
    }
    else
    {
        bool existing_point = strchr(disp_stack, '.') != NULL;
        if (existing_point && c == '.')
            return false;
        /* A bit of faff to set how many characters are allowed to be entered.
         * If disp_float_num_digits == 6 as an example, then allow
         *  123456, -123456, 1.23456, -1.23456, 0.123456, -0.123456 */
        limit = disp_float_num_digits;
        int first = 0;
        if (disp_stack[0] == '-')
        {
            first = 1;
            limit++;
        }
        if (existing_point)
        {
            limit++;
            if (disp_stack[first] == '0')
            {
                limit++;
            }
        }
    }

    disp_stack_push(c, limit);
    return update_display();
}


int display_get_text_length(void)
{
    return disp_stack_num_chars();
}


void display_set_mode(disp_mode_enum mode)
{
    disp_mode = mode;
}


static void disp_toggle_exp_sign(void)
{
    int i = disp_stack_index - EXP_DIGITS - 1;
    if (disp_stack[i] == '+')
        disp_stack[i] = '-';
    else
        disp_stack[i] = '+';
    (void)update_display();
}

void disp_toggle_sign(void)
{
    if (disp_exp_entry)
    {
        disp_toggle_exp_sign();
        return;
    }

    if (disp_stack[0] != '-')
    {
        /* move to the right and stick a '-' on the front */

        /* right starts out pointing to first empty space */
        for (int right = disp_stack_index; right > 0; right--)
        {
            disp_stack[right] = disp_stack[right - 1];
        }
        disp_stack[0] = '-';
        /* since we have added the '-' */
        disp_stack_index++;
    }
    else
    {
        /* move left and lose the '-' on the front */

        /* go as far as i < disp_stack_index to copy in a 0 to maintain
         * null termination */
        for (int i = 0; i < disp_stack_index; i++)
        {
            disp_stack[i] = disp_stack[i+1];
        }
        /* since we have removed the '-' */
        disp_stack_index--;
    }
    (void)update_display();
}


void display_get_val(uint64_t *ival, stackf_t *fval)
{
    if (disp_mode == disp_mode_int)
    {
        /* ival is already calculated and stored */
        *ival = disp_ival;
        dfp_zero(fval);
    }
    else
    {
        *ival = 0;
        //*fval = strtod(disp_stack, NULL);
        dfp_from_string(fval, disp_stack, &dfp_context);
    }
}


static uint64_t umax(calc_width_enum width)
{
    uint64_t res;

    switch (width)
    {
    case calc_width_8:
        res = UINT8_MAX;
        break;
    case calc_width_16:
        res = UINT16_MAX;
        break;
    case calc_width_32:
        res = UINT32_MAX;
        break;
    default:
        res = UINT64_MAX;
        break;
    }
    /* unnecessary, but harmless */
    calc_util_mask_width(&res, width);
    return res;
}

static uint64_t imax(calc_width_enum width, bool sign)
{
    int64_t sres;
    uint64_t res;

    switch (width)
    {
    case calc_width_8:
        sres = sign ? INT8_MIN : INT8_MAX;
        break;
    case calc_width_16:
        sres = sign ? INT16_MIN : INT16_MAX;
        break;
    case calc_width_32:
        sres = sign ? INT32_MIN : INT32_MAX;
        break;
    default:
        sres = sign ? INT64_MIN : INT64_MAX;
        break;
    }
    res = sres;
    calc_util_mask_width(&res, width);
    return res;
}

bool display_get_best_integer(uint64_t *val, calc_width_enum width, bool use_unsigned)
{
    /* This is the one case where it probably makes sense to convert the value
     * actually on the display rather than convert from the (more precise,
     * unrounded) underlying floating point value. */

    uint64_t result;
    char *buf;
    char buffer1[DFP_STRING_MAX];
    char buffer2[DFP_STRING_MAX];
    int exp_index = -1;
    bool sign = false;
    bool ret = true;

    /* grab what is in the display */
    buf = buffer1;
    strcpy(buf, disp_stack);
    if (*buf == '-')
    {
        sign = true;
        buf++;
    }

    if (sign && use_unsigned)
    {
        *val = 0;
        return false;
    }

    /* anything starting with 0 converts to 0 */
    if (*buf == '0')
    {
        *val = 0;
        return true;
    }

    for (int i = 0; buf[i] && exp_index < 0; i++)
    {
        char c = buf[i];
        if (c == 'e' || c == 'E')
        {
            exp_index = i;
            /* should already be guaranteed small e, but make sure */
            buf[i] = 'e';
        }
        else if (c == 'a' || c == 'A')
        {
            /* must be nan */
            *val = 0;
            return true;
        }
        else if (c == 'f' || c == 'F')
        {
            /* must be inf(inity) */
            *val = use_unsigned ? umax(width) : imax(width, sign);
            return false;
        }
    }

    if (exp_index >= 0)
    {
        /* it has an exponent */
        int exp_val = (int)strtol(buf + exp_index + 1, NULL, 10);
        if (exp_val < 0)
        {
            /* any negative exp converts to 0 */
            *val = 0;
            return true;
        }
        if (use_unsigned)
        {
            if (exp_val > 19)
            {
                /* definitely out of range */
                *val = umax(width);
                return false;
            }
        }
        else
        {
            if (exp_val > 18)
            {
                /* definitely out of range */
                *val = imax(width, sign);
                return false;
            }
        }

        /* exp in range 0 to 18(19) */
        int new_point_index = exp_val + 1;
        int count = 0;
        char *dst = buffer2;
        char *src = buf;
        if (sign)
        {
            *dst++ = '-';
        }
        /* expand to non exponent form, copy all digits up to exponent,
         * inserting point at the appropriate location if it is within the
         * digits still */
        while (*src != 'e')
        {
            if (*src != '.')
            {
                *dst++ = *src;
                count++;
                if (count == new_point_index)
                {
                    *dst++ = '.';
                }
            }
            src++;
        }
        /* run out of digits, may need to add some zeros */
        count--;
        while (count < exp_val)
        {
            count++;
            *dst++ = '0';
        }
        /* null terminate */
        *dst = 0;
        /* terminate at point instead if there is one */
        char *pt = strchr(buffer2, '.');
        if (pt != NULL)
        {
            *pt = 0;
        }

        //printf("convert best A %s\n", buffer2);

        /* convert the resulting string, includes the sign if there is one */
        if (use_unsigned)
        {
            if (!calc_util_dec_unsigned_str_to_ival(buffer2, width, &result))
            {
                ret = false;
            }
        }
        else
        {
            if (!calc_util_dec_signed_str_to_ival(buffer2, width, &result))
            {
                ret = false;
            }
        }
    }
    else
    {
        /* no exponent, terminate at point if there is one */
        char *pt = strchr(buffer1, '.');
        if (pt != NULL)
        {
            *pt = 0;
        }

        //printf("convert best B %s\n", buffer1);

        /* convert the resulting string, includes the sign if there is one */
        if (use_unsigned)
        {
            if (!calc_util_dec_unsigned_str_to_ival(buffer1, width, &result))
            {
                ret = false;
            }
        }
        else
        {
            if (!calc_util_dec_signed_str_to_ival(buffer1, width, &result))
            {
                ret = false;
            }
        }
    }
    *val = result;
    return ret;
}


void display_set_val(uint64_t ival, stackf_t fval)
{
    char msg[DFP_STRING_MAX];
    calc_width_enum width = calc_get_integer_width();

    if (disp_mode == disp_mode_int)
    {
        if (int_format == disp_int_dec)
        {
            if (calc_get_use_unsigned())
            {
                sprintf(msg, "%" PRIu64, ival);
            }
            else
            {
                int64_t si = calc_util_get_signed(ival, width);
                sprintf(msg, "%" PRId64, si);
            }
        }
        else
        {
            /* Doing this unsigned, and for smaller widths, values are already
             * masked to the width,
             * eg. width=8, if the value represented is -1 then ival is
             * actually 0x00000000000000ff */
            sprintf(msg, "%" PRIX64, ival);
        }
    }
    else
    {
        if (float_format == disp_float_gmode)
        {
            //sprintf(msg, "%.*g", DISP_MAX_DIGIT_FLOAT, fval);
            display_print_gmode(msg, fval, disp_float_num_digits);
        }
        else
        {
            //sprintf(msg, "%.*e", DISP_MAX_DIGIT_FLOAT - 1, fval);
            display_print_emode(msg, fval, disp_float_num_digits);
        }
    }

    display_set_text(msg);
}

void display_set_exp_entry(bool enable)
{
    if (disp_mode != disp_mode_float)
        return;

    if (enable == disp_exp_entry)
        return;

    if (enable)
    {
        disp_stack_push('e', -1);
        disp_stack_push('+', -1);
        for (int i = 0; i < EXP_DIGITS; i++)
            disp_stack_push('0', -1);
        (void)update_display();
    }
    else
    {
        /* remove exp digits */
        for (int i = 0; i < EXP_DIGITS; i++)
            disp_stack_pop();
        /* remove sign */
        disp_stack_pop();
        /* remove 'e' */
        disp_stack_pop();
        (void)update_display();
    }
    disp_exp_entry = enable;
}

bool display_get_exp_entry(void)
{
    return disp_exp_entry;
}

void display_set_int_format(disp_int_format_enum format)
{
    int_format = format;
}

void display_set_float_format(disp_float_format_enum format)
{
    float_format = format;
}

disp_float_format_enum display_get_float_format(void)
{
    return float_format;
}

void display_set_hex_grouping(int hg)
{
    hex_grouping = hg;
}

void disp_set_warn_callback(void (*fn)(const char *msg))
{
    warn_callback = fn;
}

void disp_set_error_callback(void (*fn)(const char *msg))
{
    error_callback = fn;
}

void display_set_num_float_digits(int n)
{
    disp_float_num_digits = n;
}

void display_init(int hg)
{
    /* nothing else to do here at present */
    display_set_hex_grouping(hg);
}
