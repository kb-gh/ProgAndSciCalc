/*****************************************************************************
 * File calc_integer.c part of ProgAndSciCalc
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


#include "calc_internal.h"

/*
 * Operations for integer mode.
 * For integer mode, all values are stored and passed around as a uint64_t.
 * There is an assumption of a twos complement target where the bit
 * pattern is the same for unsigned/signed, and in particular a conversion
 * from unsigned to signed will never do anything unexpected (ie. for the
 * case where the unsigned value is outside the range of the signed
 * type).
 * The value in the uint64 is always masked off to the width eg. for signed
 * 8 bit, a value of -1 will be held in the uint64 as 0x00000000000000ff.
 * The masking is applied at the end of every operation (not always
 * necessary, but harmless to just do it).
 *
 * Compiled with -fwrapv (signed integer overflow defined to wrap) to avoid
 * having to consider any possible consequences of signed overflow.
 */

static const char *signed_overflow_msg = "Signed Integer Overflow";
static const char *unsigned_overflow_msg = "Unsigned Integer Overflow";
static const char *shift_range_msg = "Shift Out of Range";
static const char *div0_msg = "Divide by 0";

static void calc_signed_overflow_warn(void)
{
    if (calc_get_warn_on_signed_overflow())
        calc_warn(signed_overflow_msg);
}

static void calc_unsigned_overflow_warn(void)
{
    if (calc_get_warn_on_unsigned_overflow())
        calc_warn(unsigned_overflow_msg);
}

/* for debug print */
//#define DBGP(x) printf x
#define DBGP(x) ((void)0)


static bool is_int_min(int64_t x, calc_width_enum width)
{
    switch (width)
    {
    case calc_width_8:
        return x == INT8_MIN;
    case calc_width_16:
        return x == INT16_MIN;
    case calc_width_32:
        return x == INT32_MIN;
    default:
        return x == INT64_MIN;
    }
}

static bool is_shift_valid(uint64_t x, calc_width_enum width)
{
    switch (width)
    {
    case calc_width_8:
        return x < 8;
    case calc_width_16:
        return x < 16;
    case calc_width_32:
        return x < 32;
    default:
        return x < 64;
    }
}

/* unary ops */

uint64_t iop_plusminus(uint64_t arg)
{
    uint64_t res;
    calc_width_enum width = calc_get_integer_width();

    if (calc_get_use_unsigned())
    {
        res = -arg;
    }
    else
    {
        int64_t sa = calc_util_get_signed(arg, width);
        if (is_int_min(sa, width))
        {
            calc_signed_overflow_warn();
            return arg;
        }
        sa = -sa;
        res = sa;
    }

    DBGP(("op res before %016" PRIx64 "\n", res));
    calc_util_mask_width(&res, width);
    DBGP(("op res after %016" PRIx64 "\n", res));
    return res;
}

uint64_t iop_complement(uint64_t arg)
{
    uint64_t res;
    calc_width_enum width = calc_get_integer_width();

    res = ~arg;

    DBGP(("op res before %016" PRIx64 "\n", res));
    calc_util_mask_width(&res, width);
    DBGP(("op res after %016" PRIx64 "\n", res));
    return res;
}

uint64_t iop_square(uint64_t arg)
{
    return bin_iop_mul(arg, arg);
}

#if 0
static const char *sqrt_msg = "sqrt of negative number";

uint64_t iop_square_root(uint64_t arg)
{
    uint64_t res;
    calc_width_enum width = calc_get_integer_width();

    if (calc_get_use_unsigned())
    {
        res = sqrt((double)arg);
    }
    else
    {
        int64_t sa = calc_util_get_signed(arg, width);
        if (sa < 0)
        {
            calc_warn(sqrt_msg);
            return arg;
        }
        sa = sqrt((double)sa);
        res = sa;
    }

    DBGP(("op res before %016" PRIx64 "\n", res));
    calc_util_mask_width(&res, width);
    DBGP(("op res after %016" PRIx64 "\n", res));
    return res;
}
#endif

#if 0
uint64_t iop_2powx(uint64_t arg)
{
    uint64_t res;
    calc_width_enum width = calc_get_integer_width();

    if (!is_shift_valid(arg, width))
    {
        calc_warn(input_range_msg);
        return arg;
    }

    res = 1ULL << arg;

    DBGP(("op res before %016" PRIx64 "\n", res));
    calc_util_mask_width(&res, width);
    DBGP(("op res after %016" PRIx64 "\n", res));
    return res;
}
#endif

uint64_t iop_left_shift(uint64_t arg)
{
    uint64_t res;
    calc_width_enum width = calc_get_integer_width();

    res = arg << 1;

    DBGP(("op res before %016" PRIx64 "\n", res));
    calc_util_mask_width(&res, width);
    DBGP(("op res after %016" PRIx64 "\n", res));
    return res;
}

uint64_t iop_right_shift(uint64_t arg)
{
    uint64_t res;
    calc_width_enum width = calc_get_integer_width();

    if (calc_get_use_unsigned())
    {
        res = arg >> 1;
    }
    else
    {
        int64_t sa = calc_util_get_signed(arg, width);
        sa = sa >> 1;
        res = sa;
    }

    DBGP(("op res before %016" PRIx64 "\n", res));
    calc_util_mask_width(&res, width);
    DBGP(("op res after %016" PRIx64 "\n", res));
    return res;
}

/* rotate (circular shift) left 1 place */
uint64_t iop_rol(uint64_t arg)
{
    uint64_t res;
    calc_width_enum width = calc_get_integer_width();

    int rs;
    if (width == calc_width_8)
        rs = 7;
    else if (width == calc_width_16)
        rs = 15;
    else if (width == calc_width_32)
        rs = 31;
    else
        rs = 63;

    res = (arg << 1) | (arg >> rs);

    DBGP(("op res before %016" PRIx64 "\n", res));
    calc_util_mask_width(&res, width);
    DBGP(("op res after %016" PRIx64 "\n", res));
    return res;
}

/* rotate (circular shift) right 1 place */
uint64_t iop_ror(uint64_t arg)
{
    uint64_t res;
    calc_width_enum width = calc_get_integer_width();

    int ls;
    if (width == calc_width_8)
        ls = 7;
    else if (width == calc_width_16)
        ls = 15;
    else if (width == calc_width_32)
        ls = 31;
    else
        ls = 63;

    res = (arg >> 1) | (arg << ls);

    DBGP(("op res before %016" PRIx64 "\n", res));
    calc_util_mask_width(&res, width);
    DBGP(("op res after %016" PRIx64 "\n", res));
    return res;
}


/* binary ops */

static bool range_test_add64(int64_t a, int64_t b)
{
    bool ok = true;

    if (b > 0)
    {
        if (a > INT64_MAX - b)
            ok = false;
    }
    else
    {
        if (a < INT64_MIN - b)
            ok = false;
    }

    return ok;
}

uint64_t bin_iop_add(uint64_t a, uint64_t b)
{
    uint64_t res;
    calc_width_enum width = calc_get_integer_width();

    if (calc_get_use_unsigned())
    {
        bool ok;

        res = a + b;

        if (width < calc_width_64)
        {
            ok = calc_util_is_in_unsigned_range(res, width);
        }
        else
        {
            ok = res >= a;
        }
        if (!ok)
        {
            calc_unsigned_overflow_warn();
        }
    }
    else
    {
        int64_t sa = calc_util_get_signed(a, width);
        int64_t sb = calc_util_get_signed(b, width);
        int64_t sres;
        bool ok;

        if (width < calc_width_64)
        {
            /* easy, do operation in the wider int64 type then check result */
            sres = sa + sb;
            ok = calc_util_is_in_signed_range(sres, width);
        }
        else
        {
            /* do a range test first, but then go ahead anyway */
            ok = range_test_add64(sa, sb);
            sres = sa + sb;
        }
        if (!ok)
        {
            calc_signed_overflow_warn();
        }
        res = sres;
    }

    DBGP(("op res before %016" PRIx64 "\n", res));
    calc_util_mask_width(&res, width);
    DBGP(("op res after %016" PRIx64 "\n", res));
    return res;
}

static bool range_test_sub64(int64_t a, int64_t b)
{
    bool ok = true;

    if (b > 0)
    {
        if (a < INT64_MIN + b)
            ok = false;
    }
    else
    {
        if (a > INT64_MAX + b)
            ok = false;
    }

    return ok;
}

uint64_t bin_iop_sub(uint64_t a, uint64_t b)
{
    uint64_t res;
    calc_width_enum width = calc_get_integer_width();

    if (calc_get_use_unsigned())
    {
        bool ok;

        res = a - b;

        if (width < calc_width_64)
        {
            ok = calc_util_is_in_unsigned_range(res, width);
        }
        else
        {
            ok = res <= a;
        }
        if (!ok)
        {
            calc_unsigned_overflow_warn();
        }
    }
    else
    {
        int64_t sa = calc_util_get_signed(a, width);
        int64_t sb = calc_util_get_signed(b, width);
        int64_t sres;
        bool ok;

        if (width < calc_width_64)
        {
            /* easy, do operation in the wider int64 type then check result */
            sres = sa - sb;
            ok = calc_util_is_in_signed_range(sres, width);
        }
        else
        {
            /* do a range test first, but then go ahead anyway */
            ok = range_test_sub64(sa, sb);
            sres = sa - sb;
        }
        if (!ok)
        {
            calc_signed_overflow_warn();
        }
        res = sres;
    }

    DBGP(("op res before %016" PRIx64 "\n", res));
    calc_util_mask_width(&res, width);
    DBGP(("op res after %016" PRIx64 "\n", res));
    return res;
}

static bool range_test_mul64(int64_t a, int64_t b)
{
    if (a == 0 || b == 0)
        return true;
    if (a == 1 || b == 1)
        return true;

    bool ok = true;

    if (a > 0 && b > 0)
    {
        if (a > INT64_MAX / b)
            ok = false;
    }
    else if (a < 0 && b < 0)
    {
        if (a < INT64_MAX / b)
            ok = false;
    }
    else if (a > 0 && b < 0)
    {
        if (b < INT64_MIN / a)
            ok = false;
    }
    else if (a < 0 && b > 0)
    {
        if (a < INT64_MIN / b)
            ok = false;
    }

    return ok;
}

uint64_t bin_iop_mul(uint64_t a, uint64_t b)
{
    uint64_t res;
    calc_width_enum width = calc_get_integer_width();

    if (calc_get_use_unsigned())
    {
        bool ok;

        res = a * b;

        if (width < calc_width_64)
        {
            ok = calc_util_is_in_unsigned_range(res, width);
        }
        else
        {
            ok = (b == 0) || (a == res / b);
        }
        if (!ok)
        {
            calc_unsigned_overflow_warn();
        }
    }
    else
    {
        int64_t sa = calc_util_get_signed(a, width);
        int64_t sb = calc_util_get_signed(b, width);
        int64_t sres;
        bool ok;

        if (width < calc_width_64)
        {
            /* easy, do operation in the wider int64 type then check result */
            sres = sa * sb;
            ok = calc_util_is_in_signed_range(sres, width);
        }
        else
        {
            /* do a range test first, but then go ahead anyway */
            ok = range_test_mul64(sa, sb);
            sres = sa * sb;
        }
        if (!ok)
        {
            calc_signed_overflow_warn();
        }
        res = sres;
    }

    DBGP(("op res before %016" PRIx64 "\n", res));
    calc_util_mask_width(&res, width);
    DBGP(("op res after %016" PRIx64 "\n", res));
    return res;
}

uint64_t bin_iop_div(uint64_t a, uint64_t b)
{
    if (b == 0)
    {
        calc_warn(div0_msg);
        return 0;
    }

    uint64_t res;
    calc_width_enum width = calc_get_integer_width();

    if (calc_get_use_unsigned())
    {
        res = a / b;
    }
    else
    {
        int64_t sa = calc_util_get_signed(a, width);
        int64_t sb = calc_util_get_signed(b, width);
        int64_t sres;

        if (is_int_min(sa, width) && sb == -1)
        {
            calc_signed_overflow_warn();
            return a;
        }
        sres = sa / sb;
        res = sres;
    }

    DBGP(("op res before %016" PRIx64 "\n", res));
    calc_util_mask_width(&res, width);
    DBGP(("op res after %016" PRIx64 "\n", res));
    return res;
}

uint64_t bin_iop_mod(uint64_t a, uint64_t b)
{
    if (b == 0)
    {
        calc_warn(div0_msg);
        return 0;
    }

    uint64_t res;
    calc_width_enum width = calc_get_integer_width();

    if (calc_get_use_unsigned())
    {
        res = a % b;
    }
    else
    {
        int64_t sa = calc_util_get_signed(a, width);
        int64_t sb = calc_util_get_signed(b, width);
        int64_t sres;

        if (is_int_min(sa, width) && sb == -1)
        {
            /* does this count as overflow? */
            return 0;
        }
        sres = sa % sb;
        res = sres;
    }

    DBGP(("op res before %016" PRIx64 "\n", res));
    calc_util_mask_width(&res, width);
    DBGP(("op res after %016" PRIx64 "\n", res));
    return res;
}

uint64_t bin_iop_gcd(uint64_t a, uint64_t b)
{
    uint64_t res;
    calc_width_enum width = calc_get_integer_width();

    if (calc_get_use_unsigned())
    {
        /* if b > a to start with, the first iteration through the loop ends
         * up swapping a and b, then it follows from there, so don't need to
         * worry about which of a and b is greater. */
        while (b != 0)
        {
            uint64_t temp = b;
            b = a % b;
            a = temp;
        }
        res = a;
    }
    else
    {
        int64_t sa = calc_util_get_signed(a, width);
        int64_t sb = calc_util_get_signed(b, width);

        if (sa == INT64_MIN && sb == -1)
        {
            return -1;
        }
        if (sa == -1 && sb == INT64_MIN)
        {
            return -1;
        }

        while (sb != 0)
        {
            int64_t temp = sb;
            sb = sa % sb;
            sa = temp;
        }
        res = sa;
    }

    DBGP(("op res before %016" PRIx64 "\n", res));
    calc_util_mask_width(&res, width);
    DBGP(("op res after %016" PRIx64 "\n", res));
    return res;
}

uint64_t bin_iop_and(uint64_t a, uint64_t b)
{
    uint64_t res;
    calc_width_enum width = calc_get_integer_width();

    res = a & b;

    DBGP(("op res before %016" PRIx64 "\n", res));
    calc_util_mask_width(&res, width);
    DBGP(("op res after %016" PRIx64 "\n", res));
    return res;
}

uint64_t bin_iop_or(uint64_t a, uint64_t b)
{
    uint64_t res;
    calc_width_enum width = calc_get_integer_width();

    res = a | b;

    DBGP(("op res before %016" PRIx64 "\n", res));
    calc_util_mask_width(&res, width);
    DBGP(("op res after %016" PRIx64 "\n", res));
    return res;
}

uint64_t bin_iop_xor(uint64_t a, uint64_t b)
{
    uint64_t res;
    calc_width_enum width = calc_get_integer_width();

    res = a ^ b;

    DBGP(("op res before %016" PRIx64 "\n", res));
    calc_util_mask_width(&res, width);
    DBGP(("op res after %016" PRIx64 "\n", res));
    return res;
}

uint64_t bin_iop_left_shift(uint64_t a, uint64_t b)
{
    uint64_t res;
    calc_width_enum width = calc_get_integer_width();

    if (!is_shift_valid(b, width))
    {
        calc_warn(shift_range_msg);
        return a;
    }
    res = a << b;

    DBGP(("op res before %016" PRIx64 "\n", res));
    calc_util_mask_width(&res, width);
    DBGP(("op res after %016" PRIx64 "\n", res));
    return res;
}

uint64_t bin_iop_right_shift(uint64_t a, uint64_t b)
{
    uint64_t res;
    calc_width_enum width = calc_get_integer_width();

    if (!is_shift_valid(b, width))
    {
        calc_warn(shift_range_msg);
        return a;
    }
    if (calc_get_use_unsigned())
    {
        res = a >> b;
    }
    else
    {
        int64_t sa = calc_util_get_signed(a, width);
        sa = sa >> b;
        res = sa;
    }

    DBGP(("op res before %016" PRIx64 "\n", res));
    calc_util_mask_width(&res, width);
    DBGP(("op res after %016" PRIx64 "\n", res));
    return res;
}

