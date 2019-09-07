/*****************************************************************************
 * File calc_float.c part of ProgAndSciCalc
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
#include "decNumber/decNumberMath.h"

/* Operations for floating point mode, using decimal floating point type. */


#define PI_quad "3.1415926535897932384626433832795029"

static stackf_t inv_tan_helper(stackf_t arg, bool add_pi);

static const char *msg_fact_pos = "Factorial requires a positive value";
static const char *msg_fact_range = "Input out of range";

/* Angle conversions and utilities */
static stackf_t deg_to_rad(stackf_t arg)
{
    //return arg * M_PI / 180.0;
    stackf_t pi, n_180, res;
    dfp_from_string(&n_180, "180.0", &dfp_context);
    dfp_from_string(&pi, PI_quad, &dfp_context);
    dfp_multiply(&res, &arg, &pi, &dfp_context);
    dfp_divide(&res, &res, &n_180, &dfp_context);
    return res;
}

static stackf_t rad_to_deg(stackf_t arg)
{
    //return arg * 180.0 / M_PI;
    stackf_t pi, n_180, res;
    dfp_from_string(&n_180, "180.0", &dfp_context);
    dfp_from_string(&pi, PI_quad, &dfp_context);
    dfp_multiply(&res, &arg, &n_180, &dfp_context);
    dfp_divide(&res, &res, &pi, &dfp_context);
    return res;
}

static stackf_t grad_to_rad(stackf_t arg)
{
    //return arg * M_PI / 200.0;
    stackf_t pi, n_200, res;
    dfp_from_string(&n_200, "200.0", &dfp_context);
    dfp_from_string(&pi, PI_quad, &dfp_context);
    dfp_multiply(&res, &arg, &pi, &dfp_context);
    dfp_divide(&res, &res, &n_200, &dfp_context);
    return res;
}

static stackf_t rad_to_grad(stackf_t arg)
{
    //return arg * 200.0 / M_PI;
    stackf_t pi, n_200, res;
    dfp_from_string(&n_200, "200.0", &dfp_context);
    dfp_from_string(&pi, PI_quad, &dfp_context);
    dfp_multiply(&res, &arg, &n_200, &dfp_context);
    dfp_divide(&res, &res, &pi, &dfp_context);
    return res;
}

static stackf_t mod_360(stackf_t arg)
{
    stackf_t n_360;
    dfp_from_string(&n_360, "360.0", &dfp_context);
    return bin_fop_mod(arg, n_360);
}

static stackf_t mod_400(stackf_t arg)
{
    stackf_t n_400;
    dfp_from_string(&n_400, "400.0", &dfp_context);
    return bin_fop_mod(arg, n_400);
}

/* zero arg if abs(arg) < threshold */
static void abs_round_to_zero(stackf_t *arg, const stackf_t *threshold)
{
    stackf_t arg_abs, cmp;

    dfp_abs(&arg_abs, arg, &dfp_context);
    dfp_compare(&cmp, &arg_abs, threshold, &dfp_context);
    if (dfp_is_negative(&cmp))
    {
        dfp_zero(arg);
    }
}

/* is a > b */
static bool gt_(const stackf_t *a, const stackf_t *b)
{
    stackf_t cmp;
    dfp_compare(&cmp, a, b, &dfp_context);
    if (dfp_is_negative(&cmp) || dfp_is_zero(&cmp))
        return false;
    else
        return true;
}

/* is a < b */
static bool lt_(const stackf_t *a, const stackf_t *b)
{
    stackf_t cmp;
    dfp_compare(&cmp, a, b, &dfp_context);
    if (dfp_is_negative(&cmp))
        return true;
    else
        return false;
}

/* clamp to +/- 1 */
static void clamp_to_one(stackf_t *arg)
{
    stackf_t one, mone;
    dfp_from_string(&one, "1.0", &dfp_context);
    dfp_minus(&mone, &one, &dfp_context);
    if (gt_(arg, &one))
        *arg = one;
    else if (lt_(arg, &mone))
        *arg = mone;
}

/* used for sin(x) and atan(X), true if x can be considered 'small' */
static bool small_arg(stackf_t a)
{
    stackf_t small;
    dfp_from_string(&small, "1E-10", &dfp_context);
    if (dfp_is_negative(&a))
    {
        dfp_minus(&a, &a, &dfp_context);
    }
    return lt_(&a, &small);
}

/***************************************************************************
 * unary ops
 */

stackf_t fop_plusminus(stackf_t arg)
{
    dfp_context_clear_status(&dfp_context);

    stackf_t res;
    dfp_minus(&res, &arg, &dfp_context);
    return res;
}

stackf_t fop_square(stackf_t arg)
{
    dfp_context_clear_status(&dfp_context);

    /* don't think copy is actually needed, but it can't hurt */
    stackf_t copy = arg;
    stackf_t res;
    dfp_multiply(&res, &arg, &copy, &dfp_context);
    return res;
}

stackf_t fop_square_root(stackf_t arg)
{
    dfp_context_clear_status(&dfp_context);

    decNumber dn_arg, dn_res;
    stackf_t res;

    dfp_to_number(&arg, &dn_arg); // convert to decNumber
    decNumberSquareRoot(&dn_res, &dn_arg, &dfp_context);
    dfp_from_number(&res, &dn_res, &dfp_context); // convert back from decNumber
    return res;
}

stackf_t fop_one_over_x(stackf_t arg)
{
    dfp_context_clear_status(&dfp_context);

    stackf_t res;
    stackf_t one;
    dfp_from_string(&one, "1.0", &dfp_context);
    dfp_divide(&res, &one, &arg, &dfp_context);
    return res;
}

stackf_t fop_log(stackf_t arg)
{
    dfp_context_clear_status(&dfp_context);

    decNumber dn_arg, dn_res;
    stackf_t res;

    dfp_to_number(&arg, &dn_arg); // convert to decNumber
    decNumberLog10(&dn_res, &dn_arg, &dfp_context);
    dfp_from_number(&res, &dn_res, &dfp_context); // convert back from decNumber
    return res;
}

stackf_t fop_inv_log(stackf_t arg)
{
    dfp_context_clear_status(&dfp_context);

    //return pow(10.0, arg);
    stackf_t ten;
    dfp_from_string(&ten, "10.0", &dfp_context);
    return bin_fop_pow(ten, arg);
}

stackf_t fop_ln(stackf_t arg)
{
    dfp_context_clear_status(&dfp_context);

    decNumber dn_arg, dn_res;
    stackf_t res;

    dfp_to_number(&arg, &dn_arg); // convert to decNumber
    decNumberLn(&dn_res, &dn_arg, &dfp_context);
    dfp_from_number(&res, &dn_res, &dfp_context); // convert back from decNumber
    return res;
}

stackf_t fop_inv_ln(stackf_t arg)
{
    dfp_context_clear_status(&dfp_context);

    //return exp(arg);
    decNumber dn_arg, dn_res;
    stackf_t res;

    dfp_to_number(&arg, &dn_arg); // convert to decNumber
    decNumberExp(&dn_res, &dn_arg, &dfp_context);
    dfp_from_number(&res, &dn_res, &dfp_context); // convert back from decNumber
    return res;
}

/* If the abs(result) is smaller than this threshold then just call it 0.
 * Threshold chosen simply on the grounds of it feels like it's probably
 * small enough not to care. */
#define SIN_ZERO_THRESHOLD "1E-30"
#define COS_ZERO_THRESHOLD "1E-30"

stackf_t fop_sin(stackf_t arg)
{
    dfp_context_clear_status(&dfp_context);

    stackf_t res;
    decNumber dn_arg, dn_res;

    if (dfp_is_infinite(&arg) || dfp_is_nan(&arg))
    {
        stackf_t nan;
        dfp_from_string(&nan, "Nan", &dfp_context);
        return nan;
    }

    if (calc_get_angle() == calc_angle_deg)
    {
        arg = mod_360(arg);
        arg = deg_to_rad(arg);
    }
    else if (calc_get_angle() == calc_angle_grad)
    {
        arg = mod_400(arg);
        arg = grad_to_rad(arg);
    }

    /* Can be nan for extreme values where mod operation above fails */
    if (dfp_is_nan(&arg))
    {
        return arg;
    }

    dfp_to_number(&arg, &dn_arg);
    decNumberSin(&dn_res, &dn_arg, &dfp_context);
    dfp_from_number(&res, &dn_res, &dfp_context);

    /* in case result is fractionally >1 or <-1, clamp to +/- 1 */
    clamp_to_one(&res);

    if (!small_arg(arg) && calc_get_use_sct_rounding())
    {
        stackf_t thresh;
        dfp_from_string(&thresh, SIN_ZERO_THRESHOLD, &dfp_context);
        abs_round_to_zero(&res, &thresh);
    }
    return res;
}

stackf_t fop_inv_sin(stackf_t arg)
{
    dfp_context_clear_status(&dfp_context);

    /*
     * should be good enough to use
     *   asin(x) = atan(x / sqrt(1 - x*x))
     */

    stackf_t res;
    stackf_t one;
    dfp_from_string(&one, "1.0", &dfp_context);

    res = fop_square(arg);
    res = bin_fop_sub(one, res);
    res = fop_square_root(res);
    res = bin_fop_div(arg, res);
    res = inv_tan_helper(res, false);
    return res;
}

stackf_t fop_cos(stackf_t arg)
{
    dfp_context_clear_status(&dfp_context);

    stackf_t res;
    decNumber dn_arg, dn_res;

    if (dfp_is_infinite(&arg) || dfp_is_nan(&arg))
    {
        stackf_t nan;
        dfp_from_string(&nan, "Nan", &dfp_context);
        return nan;
    }

    if (calc_get_angle() == calc_angle_deg)
    {
        arg = mod_360(arg);
        arg = deg_to_rad(arg);
    }
    else if (calc_get_angle() == calc_angle_grad)
    {
        arg = mod_400(arg);
        arg = grad_to_rad(arg);
    }

    /* Can be nan for extreme values where mod operation above fails */
    if (dfp_is_nan(&arg))
    {
        return arg;
    }

    dfp_to_number(&arg, &dn_arg);
    decNumberCos(&dn_res, &dn_arg, &dfp_context);
    dfp_from_number(&res, &dn_res, &dfp_context);

    /* in case result is fractionally >1 or <-1, clamp to +/- 1 */
    clamp_to_one(&res);

    if (calc_get_use_sct_rounding())
    {
        stackf_t thresh;
        dfp_from_string(&thresh, COS_ZERO_THRESHOLD, &dfp_context);
        abs_round_to_zero(&res, &thresh);
    }
    return res;
}

stackf_t fop_inv_cos(stackf_t arg)
{
    dfp_context_clear_status(&dfp_context);

    /*
     * should be good enough to use
     *   acos(x) = atan(sqrt(1 - x*x) / x)
     */

    bool sign;
    stackf_t res;
    stackf_t one;

    sign = dfp_is_negative(&arg);
    dfp_from_string(&one, "1.0", &dfp_context);

    res = fop_square(arg);
    res = bin_fop_sub(one, res);
    res = fop_square_root(res);
    res = bin_fop_div(res, arg);

    res = inv_tan_helper(res, sign);
    return res;
}

stackf_t fop_tan(stackf_t arg)
{
    dfp_context_clear_status(&dfp_context);

    stackf_t top, bot;

    if (dfp_is_infinite(&arg) || dfp_is_nan(&arg))
    {
        stackf_t nan;
        dfp_from_string(&nan, "Nan", &dfp_context);
        return nan;
    }

    top = fop_sin(arg);
    bot = fop_cos(arg);
    return bin_fop_div(top, bot);
}

static stackf_t inv_tan_helper(stackf_t arg, bool add_pi)
{
    stackf_t res;
    stackf_t pi;
    dfp_from_string(&pi, PI_quad, &dfp_context);

    if (dfp_is_infinite(&arg))
    {
        /* special case for infinity */
        stackf_t n_2;
        dfp_from_string(&n_2, "2.0", &dfp_context);
        dfp_divide(&res, &pi, &n_2, &dfp_context);
        if (dfp_is_negative(&arg))
        {
            dfp_minus(&res, &res, &dfp_context);
        }
    }
    else
    {
        /* normal case */
        decNumber dn_arg, dn_res;

        /* for very small arg, arctan(arg) = arg */
        if (small_arg(arg))
        {
            res = arg;
        }
        else
        {
            dfp_to_number(&arg, &dn_arg);
            decNumberAtan(&dn_res, &dn_arg, &dfp_context);
            dfp_from_number(&res, &dn_res, &dfp_context);
        }
    }

    if (add_pi)
    {
        /* used with acos only */
        res = bin_fop_add(res, pi);
    }

    if (calc_get_angle() == calc_angle_deg)
        res = rad_to_deg(res);
    else if (calc_get_angle() == calc_angle_grad)
        res = rad_to_grad(res);

    return res;
}

stackf_t fop_inv_tan(stackf_t arg)
{
    dfp_context_clear_status(&dfp_context);
    stackf_t res = inv_tan_helper(arg, false);
    return res;
}


stackf_t fop_sinh(stackf_t arg)
{
    dfp_context_clear_status(&dfp_context);

    stackf_t res;
    decNumber dn_arg, dn_res;

    dfp_to_number(&arg, &dn_arg);
    decNumberSinh(&dn_res, &dn_arg, &dfp_context);
    dfp_from_number(&res, &dn_res, &dfp_context);
    return res;
}

stackf_t fop_inv_sinh(stackf_t arg)
{
    dfp_context_clear_status(&dfp_context);

    stackf_t res;
    stackf_t one;
    dfp_from_string(&one, "1.0", &dfp_context);
    /* ln(arg + sqrt(arg*arg + 1)) */
    res = fop_square(arg);
    res = bin_fop_add(res, one);
    res = fop_square_root(res);
    res = bin_fop_add(res, arg);
    res = fop_ln(res);
    return res;
}

stackf_t fop_cosh(stackf_t arg)
{
    dfp_context_clear_status(&dfp_context);

    stackf_t res;
    decNumber dn_arg, dn_res;

    dfp_to_number(&arg, &dn_arg);
    decNumberCosh(&dn_res, &dn_arg, &dfp_context);
    dfp_from_number(&res, &dn_res, &dfp_context);
    return res;
}

stackf_t fop_inv_cosh(stackf_t arg)
{
    dfp_context_clear_status(&dfp_context);

    stackf_t res;
    stackf_t one;
    dfp_from_string(&one, "1.0", &dfp_context);
    /* ln(arg + sqrt(arg*arg - 1)) */
    res = fop_square(arg);
    res = bin_fop_sub(res, one);
    res = fop_square_root(res);
    res = bin_fop_add(res, arg);
    res = fop_ln(res);
    return res;
}

stackf_t fop_tanh(stackf_t arg)
{
    dfp_context_clear_status(&dfp_context);

    stackf_t res;
    decNumber dn_arg, dn_res;

    dfp_to_number(&arg, &dn_arg);
    decNumberTanh(&dn_res, &dn_arg, &dfp_context);
    dfp_from_number(&res, &dn_res, &dfp_context);
    return res;
}

stackf_t fop_inv_tanh(stackf_t arg)
{
    dfp_context_clear_status(&dfp_context);

    stackf_t res, temp;
    stackf_t one, p5;
    dfp_from_string(&one, "1.0", &dfp_context);
    dfp_from_string(&p5, "0.5", &dfp_context);
    /* 0.5 * ln( (1 + arg) / (1 - arg) ) */
    res = bin_fop_add(one, arg);
    temp = bin_fop_sub(one, arg);
    res = bin_fop_div(res, temp);
    res = fop_ln(res);
    res = bin_fop_mul(p5, res);

    return res;
}

#define MAX_FACTORIAL "2123"

stackf_t fop_fact(stackf_t arg)
{
    dfp_context_clear_status(&dfp_context);

    stackf_t res;
    stackf_t one;
    stackf_t max;
    int32_t n;

    dfp_from_string(&one, "1.0", &dfp_context);
    dfp_from_string(&max, MAX_FACTORIAL, &dfp_context);

    if (dfp_is_nan(&arg) || dfp_is_negative(&arg))
    {
        calc_warn(msg_fact_pos);
        return arg;
    }

    if (gt_(&arg, &max))
    {
        calc_warn(msg_fact_range);
        return arg;
    }

    n = dfp_to_int32(&arg, &dfp_context, DEC_ROUND_HALF_UP);
    //printf("n = %d\n", n);

    dfp_from_int32(&arg, n);
    res = one;

    while (n > 1)
    {
        res = bin_fop_mul(res, arg);
        arg = bin_fop_sub(arg, one);
        n--;
    }
    return res;
}


/***************************************************************************
 * binary ops
 */

stackf_t bin_fop_add(stackf_t a, stackf_t b)
{
    dfp_context_clear_status(&dfp_context);

    stackf_t res;
    dfp_add(&res, &a, &b, &dfp_context);
    return res;
}

stackf_t bin_fop_sub(stackf_t a, stackf_t b)
{
    dfp_context_clear_status(&dfp_context);

    stackf_t res;
    dfp_subtract(&res, &a, &b, &dfp_context);
    return res;
}

stackf_t bin_fop_mul(stackf_t a, stackf_t b)
{
    dfp_context_clear_status(&dfp_context);

    stackf_t res;
    dfp_multiply(&res, &a, &b, &dfp_context);
    return res;
}

stackf_t bin_fop_div(stackf_t a, stackf_t b)
{
    dfp_context_clear_status(&dfp_context);

    stackf_t res;
    dfp_divide(&res, &a, &b, &dfp_context);
    return res;
}

stackf_t bin_fop_mod(stackf_t a, stackf_t b)
{
    dfp_context_clear_status(&dfp_context);

    stackf_t res;
    dfp_remainder(&res, &a, &b, &dfp_context);
    return res;
}

stackf_t bin_fop_pow(stackf_t a, stackf_t b)
{
    dfp_context_clear_status(&dfp_context);

    decNumber dn_a, dn_b, dn_res;
    stackf_t res;

    dfp_to_number(&a, &dn_a);
    dfp_to_number(&b, &dn_b);
    decNumberPower(&dn_res, &dn_a, &dn_b, &dfp_context);
    dfp_from_number(&res, &dn_res, &dfp_context);
    return res;
}

stackf_t bin_fop_root(stackf_t a, stackf_t b)
{
    dfp_context_clear_status(&dfp_context);

    stackf_t res;
    stackf_t one;
    stackf_t one_over_b;

    decNumber dn_a, dn_one_over_b, dn_res;

    dfp_from_string(&one, "1.0", &dfp_context);
    dfp_divide(&one_over_b, &one, &b, &dfp_context);

    /* convert to decNumber for power calculation */
    dfp_to_number(&a, &dn_a);
    dfp_to_number(&one_over_b, &dn_one_over_b);
    decNumberPower(&dn_res, &dn_a, &dn_one_over_b, &dfp_context);

    /* If a is negative, the above result will be nan (for b>1 I think), but it
     * could reasonably be made to work for odd integer values of b by fiddling
     * with the signs ie. return -pow(-a, 1.0 / b) */
    if (dfp_is_negative(&a))
    {
        //printf("a < 0\n");
        if (dfp_is_integer(&b))
        {
            //printf("b is integer\n");
            stackf_t rem, two;
            dfp_from_string(&two, "2.0", &dfp_context);
            dfp_remainder(&rem, &b, &two, &dfp_context);
            if (!dfp_is_zero(&rem))
            {
                //printf("b is odd\n");
                dfp_minus(&a, &a, &dfp_context);
                dfp_to_number(&a, &dn_a);
                decNumberPower(&dn_res, &dn_a, &dn_one_over_b, &dfp_context);
                decNumberMinus(&dn_res, &dn_res, &dfp_context);
            }
        }
    }

    /* convert back from decNumber */
    dfp_from_number(&res, &dn_res, &dfp_context);
    return res;
}

