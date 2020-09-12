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


bool calc_util_signed_str_to_ival(const char *str,
                                  calc_width_enum width,
                                  uint64_t *val,
                                  int base)
{
    bool ok = true;
    int64_t sres;
    uint64_t result;

    errno = 0;
    long long resll = strtoll(str, NULL, base);
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

    sres = resll;

    if (ok)
    {
        /* now check for smaller widths */
        ok = calc_util_is_in_signed_range(sres, width);
    }

    result = sres;
    calc_util_mask_width(&result, width);
    *val = result;
    return ok;
}


bool calc_util_unsigned_str_to_ival(const char *str,
                                    calc_width_enum width,
                                    uint64_t *val,
                                    int base)
{
    bool ok = true;
    uint64_t result;

    errno = 0;
    unsigned long long resll = strtoull(str, NULL, base);
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

    result = resll;
    if (ok)
    {
        /* now check for smaller widths */
        ok = calc_util_is_in_unsigned_range(result, width);
    }

    calc_util_mask_width(&result, width);
    *val = result;
    return ok;
}


static calc_width_enum calc_util_find_min_signed_width(int64_t x)
{
    if (calc_util_is_in_signed_range(x, calc_width_8))
        return calc_width_8;
    else if (calc_util_is_in_signed_range(x, calc_width_16))
        return calc_width_16;
    else if (calc_util_is_in_signed_range(x, calc_width_32))
        return calc_width_32;
    else
        return calc_width_64;
}

static calc_width_enum calc_util_find_min_unsigned_width(uint64_t x)
{
    if (calc_util_is_in_unsigned_range(x, calc_width_8))
        return calc_width_8;
    else if (calc_util_is_in_unsigned_range(x, calc_width_16))
        return calc_width_16;
    else if (calc_util_is_in_unsigned_range(x, calc_width_32))
        return calc_width_32;
    else
        return calc_width_64;
}

calc_width_enum calc_util_get_changed_width(uint64_t uval, bool negative,
                                            calc_width_enum current_width)
{
    calc_width_enum selected_width = current_width;
    calc_width_enum min_width;

    if (negative)
    {
        min_width = calc_util_find_min_signed_width((int64_t)uval);
    }
    else
    {
        min_width = calc_util_find_min_unsigned_width(uval);
    }

    if (min_width > selected_width)
    {
        selected_width = min_width;
    }
    return selected_width;
}

