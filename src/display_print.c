/*****************************************************************************
 * File display_print.c part of ProgAndSciCalc
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

#include "display_print.h"
#include "calc_types.h"

/* functions to manipulate the output from decimal floating point to_string */

//#define DEBUG_DISP_PRINT

static bool round_to_max_digits(char *msg, int sig_count, int max_digits)
{
    if (sig_count <= max_digits)
    {
        return false;
    }

    /* msg is in form n.nnnnn (or could be just n but then we have
     * already returned since sig_count would be 1 and max_digits
     * will always be > 1) */

    bool carry = false;
    int i = max_digits;
    if (msg[i + 1] >= '5')
    {
        msg[i]++;
        if (msg[i] > '9')
        {
            msg[i] = '0';
            carry = true;
            i--;
        }
    }
    while (carry && msg[i] != '.')
    {
        msg[i]++;
        if (msg[i] > '9')
        {
            msg[i] = '0';
            i--;
        }
        else
        {
            carry = false;
        }
    }
    if (carry)
    {
        msg[0]++;
        if (msg[0] > '9')
        {
            /* 10 will become 1 with exponent incremented */
            msg[0] = '1';
        }
        else
        {
            carry = false;
        }
    }
    /* place null termination to shorten msg */
    msg[max_digits + 1] = 0;
    return carry;
}


static void strip_trailing_zeros(char *msg)
{
    bool has_point = false;
    char *p = msg;
    while (*p)
    {
        if (*p == '.')
        {
            has_point = true;
        }
        p++;
    }
    if (!has_point || p == msg)
    {
        return;
    }

    /* p currently pointing at null terminator */
    p--;
    while (*p == '0')
    {
        p--;
    }
    /* if we hit the point, remove that as well */
    if (*p == '.' && p != msg)
    {
        p--;
    }
    /* place null termination */
    p++;
    *p = 0;
}


static int print_emode(char *msg, stackf_t fval, int max_digits)
{
    char work_buffer[DFP_STRING_MAX];
    char ebuf[10];

    int exp_val = 0;
    int exp_index = -1;
    int point_index = -1;
    int sig_index = -1; /* first significant digit */
    int sig_count = 0;
    bool start_sig_count = false;
    bool sign = false;
    char *buf;

    /* put into msg so the result is there if return early due to 0, inf, nan */
    buf = msg;
    dfp_to_string(&fval, buf);

    if (strchr(buf, 'n') || strchr(buf, 'N'))
    {
        /* must be inf(inity) or nan */
        return 0;
    }

    if (*buf == '-')
    {
        sign = true;
        buf++;
    }

    for (int i = 0; buf[i] && exp_index < 0; i++)
    {
        char c = buf[i];
        if (c == '.')
        {
            point_index = i;
        }
        else if (c == 'E' || c == 'e')
        {
            exp_index = i;
            /* prefer small e */
            buf[i] = 'e';
        }
        else if (c >= '0' && c <= '9')
        {
            if (!start_sig_count && c >= '1')
            {
                start_sig_count = true;
                sig_index = i;
            }
            if (start_sig_count)
            {
                sig_count++;
            }
        }
    }

    if (sig_count == 0)
    {
        /* must be value 0 */
        return 0;
    }

#ifdef DEBUG_DISP_PRINT
    printf("emode: original (%c) %s\n", sign?'-':'+', buf);
#endif

    if (exp_index >= 0)
    {
        /* already in emode, grab exp_val, then copy to working buffer
         * without the exp for now, put it back later. */
        exp_val = (int)strtol(buf + exp_index + 1, NULL, 10);
        buf[exp_index] = 0;
        strcpy(work_buffer, buf);
    }
    else
    {
        /* not in emode, convert */
        if (point_index < 0)
        {
            /* no point eg. 12345 */
            exp_val = sig_count - 1;
        }
        else
        {
            if (point_index > sig_index)
            {
                /* 123.45  ==> exp 2 */
                exp_val = point_index - sig_index - 1;
            }
            else
            {
                /* 0.0012345  ==> exp -3 */
                exp_val = point_index - sig_index;
            }
        }

        /* copy to working buffer, converting to emode */
        char *dst = work_buffer;
        int count = 0;
        char *src = &buf[sig_index];
        while (*src)
        {
            if (*src != '.')
            {
                *dst++ = *src;
                count++;
                if (count == 1)
                {
                    /* this could leave a point at the end, if so it is
                     * removed later */
                    *dst++ = '.';
                }
            }
            src++;
        }

        /* if left with a point at the end, remove */
        if (*(dst - 1) == '.')
        {
            dst--;
        }
        /* null terminate */
        *dst = 0;
    }

#ifdef DEBUG_DISP_PRINT
    printf("emode: sig %d  point %d  exp %d exp_val %d   sig_count %d\n",
           sig_index, point_index, exp_index, exp_val, sig_count);
    sprintf(ebuf, "e%+d", exp_val);
    printf("emode: original   (%c) %s (%s)\n", sign?'-':'+', buf, ebuf);
    printf("emode: ===> exp   (%c) %s (%s)\n", sign?'-':'+', work_buffer, ebuf);
#endif

    if (round_to_max_digits(work_buffer, sig_count, max_digits))
    {
        exp_val++;
    }

#ifdef DEBUG_DISP_PRINT
    sprintf(ebuf, "e%+d", exp_val);
    printf("emode: ===> round (%c) %s (%s)\n", sign?'-':'+', work_buffer, ebuf);
#endif

    strip_trailing_zeros(work_buffer);

#ifdef DEBUG_DISP_PRINT
    sprintf(ebuf, "e%+d", exp_val);
    printf("emode: ===> strip (%c) %s (%s)\n", sign?'-':'+', work_buffer, ebuf);
#endif

    /* build final result to be returned in msg */
    buf = msg;
    if (sign)
    {
        *buf = '-';
        buf++;
    }
    strcpy(buf, work_buffer);
    /* put exponent back */
    if (exp_val != 0)
    {
        sprintf(ebuf, "e%+d", exp_val);
        strcat(buf, ebuf);
    }
    return exp_val;
}


void display_print_emode(char *msg, stackf_t fval, int max_digits)
{
    (void)print_emode(msg, fval, max_digits);
}


void display_print_gmode(char *msg, stackf_t fval, int max_digits)
{
    char work_buffer[DFP_STRING_MAX];
    bool sign = false;
    char *buf;

    /* get it into emode first */
    int exp_val = print_emode(msg, fval, max_digits);
#ifdef DEBUG_DISP_PRINT
    printf("gmode: exp %d\n", exp_val);
#endif

    if (exp_val == 0)
    {
        /* nothing to change */
        return;
    }

    buf = msg;
    if (*buf == '-')
    {
        sign = true;
        buf++;
    }

    if (exp_val > 0 && exp_val <= max_digits - 1)
    {
        /*  1e2      ==>  100    */
        /*  1.234e+1 ==>  12.34  */
        /*  1.234e+2 ==>  123.4  */
        /*  1.234e+3 ==>  1234   */
        /*  1.234e+4 ==>  12340  */
        /*  1.234e+5 ==>  123400 */
        int point_index = exp_val + 1;
        int count = 0;
        char *dst = work_buffer;
        char *src = buf;
        /* copy all digits up to exponent, inserting point at the appropriate
         * location if it is within the digits still */
        while (*src != 'e')
        {
            if (*src != '.')
            {
                *dst++ = *src;
                count++;
                if (count == point_index)
                {
                    /* this could leave a point at the end, if so it is
                     * removed later */
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
        /* if left with a point at the end, remove */
        if (*(dst - 1) == '.')
        {
            dst--;
        }
        /* null terminate */
        *dst = 0;
    }
    else if (exp_val < 0 && exp_val >= -4)
    {
        /*  1e-1     ==>  0.1       */
        /*  1.234e-1 ==>  0.1234    */
        /*  1.234e-2 ==>  0.01234   */
        /*  1.234e-3 ==>  0.001234  */
        /*  1.234e-4 ==>  0.0001234 */
        char *dst = work_buffer;
        char *src = buf;
        *dst++ = '0';
        *dst++ = '.';
        exp_val = -exp_val;
        for (int i = 1; i < exp_val; i++)
        {
            *dst++ = '0';
        }
        while (*src != 'e')
        {
            if (*src != '.')
            {
                *dst++ = *src;
            }
            src++;
        }
        /* null terminate */
        *dst = 0;
    }
    else
    {
        /* nothing to change */
        return;
    }

    /* we changed it, put result back from work_buffer to msg */
    buf = msg;
    if (sign)
    {
        *buf = '-';
        buf++;
    }
    strcpy(buf, work_buffer);
}
