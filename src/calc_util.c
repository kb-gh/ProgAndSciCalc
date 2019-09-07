/*****************************************************************************
 * File calc_util.c part of ProgAndSciCalc
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


#include <errno.h>
#include "calc_internal.h"

void calc_util_mask_width(uint64_t *x, calc_width_enum width)
{
    switch (width)
    {
    case calc_width_8:
        *x &= 0xff;
        break;
    case calc_width_16:
        *x &= 0xffff;
        break;
    case calc_width_32:
        *x &= 0xffffffff;
        break;
    default:
        break;
    }
}

int64_t calc_util_get_signed(uint64_t x, calc_width_enum width)
{
    switch (width)
    {
    case calc_width_8:
        return (int64_t)(int8_t)x;
    case calc_width_16:
        return (int64_t)(int16_t)x;
    case calc_width_32:
        return (int64_t)(int32_t)x;
    default:
        return (int64_t)x;
    }
}

bool calc_util_is_in_signed_range(int64_t x, calc_width_enum width)
{
    switch (width)
    {
    case calc_width_8:
        return x >= INT8_MIN && x <= INT8_MAX;
    case calc_width_16:
        return x >= INT16_MIN && x <= INT16_MAX;
    case calc_width_32:
        return x >= INT32_MIN && x <= INT32_MAX;
    default:
        return true;
    }
}

bool calc_util_is_in_unsigned_range(uint64_t x, calc_width_enum width)
{
    switch (width)
    {
    case calc_width_8:
        return x <= UINT8_MAX;
    case calc_width_16:
        return x <= UINT16_MAX;
    case calc_width_32:
        return x <= UINT32_MAX;
    default:
        return true;
    }
}

bool calc_util_dec_signed_str_to_ival(const char *str,
                                      calc_width_enum width,
                                      uint64_t *val)
{
    bool ok = true;
    int64_t sres;
    uint64_t result;

    errno = 0;
    long long resll = strtoll(str, NULL, 10);
    if (errno == ERANGE)
    {
        //printf("signed_str_to_ival ERANGE\n");
        ok = false;
    }

    /* for the unlikely scenario that long long is more than 64 bit */
    if (ok)
    {
        if (resll < INT64_MIN || resll > INT64_MAX)
        {
            ok = false;
        }
    }

    if (ok)
    {
        /* must be in range of int64 to get here */
        sres = resll;
        /* now check for smaller widths */
        ok = calc_util_is_in_signed_range(sres, width);
    }

    if (!ok)
    {
        bool sign = resll < 0;
        /* set return value based on width */
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
    }

    result = sres;
    calc_util_mask_width(&result, width);
    *val = result;
    return ok;
}

bool calc_util_dec_unsigned_str_to_ival(const char *str,
                                        calc_width_enum width,
                                        uint64_t *val)
{
    bool ok = true;
    uint64_t result;

    errno = 0;
    unsigned long long resll = strtoull(str, NULL, 10);
    if (errno == ERANGE)
    {
        //printf("unsigned_str_to_ival ERANGE\n");
        ok = false;
    }

    /* for the unlikely scenario that long long is more than 64 bit */
    if (ok)
    {
        if (resll > UINT64_MAX)
        {
            ok = false;
        }
    }

    if (ok)
    {
        /* must be in range of uint64 to get here */
        result = resll;
        /* now check for smaller widths */
        ok = calc_util_is_in_unsigned_range(result, width);
    }

    if (!ok)
    {
        /* set return value based on width */
        switch (width)
        {
        case calc_width_8:
            result = UINT8_MAX;
            break;
        case calc_width_16:
            result = UINT16_MAX;
            break;
        case calc_width_32:
            result = UINT32_MAX;
            break;
        default:
            result = UINT64_MAX;
            break;
        }
    }

    /* unnecessary, but harmless */
    calc_util_mask_width(&result, width);
    *val = result;
    return ok;
}

bool calc_util_hex_strtoull(const char *str,
                            calc_width_enum width,
                            uint64_t *val)
{
    /* long long guaranteed to be at least 64bits so use that.
     * User input is already limited to at most 16 digits. */
    uint64_t result;
    result = strtoull(str, NULL, 16);

    /* From display, the str is already limited to the max number of digits
     * for the given width so this should always be true */
    bool ok = calc_util_is_in_unsigned_range(result, width);

    if (!ok)
    {
        /* set return value based on width */
        switch (width)
        {
        case calc_width_8:
            result = UINT8_MAX;
            break;
        case calc_width_16:
            result = UINT16_MAX;
            break;
        case calc_width_32:
            result = UINT32_MAX;
            break;
        default:
            result = UINT64_MAX;
            break;
        }
    }

    /* unnecessary, but harmless */
    calc_util_mask_width(&result, width);

    *val = result;
    return ok;
}

