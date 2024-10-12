/*****************************************************************************
 * File calc_conversion.c part of ProgAndSciCalc
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


#include <string.h>
#include "calc_internal.h"
#include "calc_conversion.h"
#include "display_print.h"

/* The gui will only allow one conversion window to be open */
static int convert_from_row;
static stackf_t value_to_convert;

typedef struct
{
    char *name;
    char *val;
} name_val_t;

/* NOTE there is a limit MAX_CONVERSION_ROWS 20 in gui_menu_conversion.c,
 * this is plenty at the current time (max is 12), but if you add extra
 * rows to a category and go beyond 20 you will need to increase it. */

static const name_val_t length_name_val[] =
{
    { "mm",            "1000" },
    { "cm",            "100" },
    { "m",             "1" },
    { "km",            "0.001" },
    { "thou",          "39370.07874015748031496062992125984" },
    { "inch",          "39.37007874015748031496062992125984" },
    { "foot",          "3.28083989501312335958005249343832" },
    { "yard",          "1.09361329833770778652668416447944" },
    { "mile",          "0.0006213711922373339696174341843633182" },
    { "nautical mile", "0.0005399568034557235421166306695464363" },
};

static const name_val_t area_name_val[] =
{
    { "sq mm",        "1e6" },
    { "sq cm",        "10000" },
    { "sq m",         "1" },
    { "hectare",      "1e-4" },
    { "sq km",        "1e-6" },
    { "sq inch",      "1550.0031000062000124000248000496" },
    { "sq foot",      "10.7639104167097223083335055559" },
    { "sq yard",      "1.195990046301080256481500617322222" },
    { "acre",         "0.0002471053814671653422482439291988062" },
    { "sq mile",      "3.861021585424458472628811393731348E-7" },
};

static const name_val_t volume_name_val[] =
{
    { "ml (cc)",           "1e6" },
    { "litre",             "1e3" },
    { "cu m",              "1" },
    { "cu inch",           "61023.74409473228395275688189171654" },
    { "cu foot",           "35.31466672148859025043801035400263" },
    { "cu yard",           "1.307950619314392231497704087185282" },
    { "fl oz (imp)",       "35195.07972785404600436858927121988" },
    { "pint (imp)",        "1759.753986392702300218429463560994" },
    { "gallon (imp)",      "219.9692482990877875273036829451242" },
    { "fl oz (US)",        "33814.02270184299716862718996597280" },
    { "pint (US)",         "2113.376418865187323039199372873300" },
    { "gallon (US)",       "264.1720523581484153798999216091625" },
};

static const name_val_t mass_name_val[] =
{
    { "g",                "1e3" },
    { "kg",               "1" },
    { "tonne",            "1e-3" },
    { "ounce",            "35.27396194958041291567580821520433" },
    { "pound",            "2.204622621848775807229738013450271" },
    { "stone",            "0.1574730444177697005164098581035908" },
    { "cwt (imp)",        "0.01968413055222121256455123226294884" },
    { "ton (imp)",        "0.0009842065276110606282275616131474422" },
    { "cwt (US)",         "0.02204622621848775807229738013450271" },
    { "ton (US)",         "0.001102311310924387903614869006725135" },
};

static const name_val_t speed_name_val[] =
{
    { "m/s",          "1" },
    { "km/h",         "3.6" },
    { "ft/s",         "3.28083989501312335958005249343832" },
    { "mile/h",       "2.236936292054402290622763063707945" },
    { "knot",         "1.943844492440604751619870410367171" },
};

static const name_val_t pressure_name_val[] =
{
    { "Pa",          "1" },
    { "mbar",        "1e-2" },
    { "bar",         "1e-5" },
    { "atm",         "0.000009869232667160128300024673081667900" },
    { "psi",         "0.0001450377377302092151542410279511940" },
};

static const name_val_t force_name_val[] =
{
    { "N",           "1" },
    { "kgf",         "0.1019716212977928242570092743189570" },
    { "lbf",         "0.2248089430997104829100394134031775" },
};

static const name_val_t energy_name_val[] =
{
    { "eV",          "6241509125883257926.515862938249161" },
    { "keV",         "6241509125883257.926515862938249161" },
    { "MeV",         "6241509125883.257926515862938249161" },
    { "J",           "1" },
    { "kJ",          "1e-3" },
    { "kWh",         "2.777777777777777777777777777777778E-7" },
    { "cal",         "0.2388458966274959396197573325690265" },
    { "kcal",        "0.0002388458966274959396197573325690265" },
    { "BTU",         "0.0009478171203133172000127850444756106" },
};

static const name_val_t power_name_val[] =
{
    { "W",               "1" },
    { "kW",              "1e-3" },
    { "HP (imp)",        "0.001341022089595027934323785572674645" },
    { "HP (metric)",     "0.001359621617303904323426790324252760" },
    { "BTU / hour",      "3.412141633127941920046026160112198" },
};

static const name_val_t torque_name_val[] =
{
    { "N.m",           "1" },
    { "kg.cm",         "10.19716212977928242570092743189570" },
    { "lb.ft",         "0.7375621492772653638780820649710549" },
};

/* only use the name, values handled as special case */
static const name_val_t temperature_name_val[] =
{
    { "C",     NULL },
    { "F",     NULL },
    { "K",     NULL },
};

/* only use the name, values handled as special case */
static const name_val_t fuel_economy_name_val[] =
{
    { "litre/100km",     NULL },
    { "mpg (imp)",       NULL },
    { "mpg (US)",        NULL },
};


int calc_conversion_get_num_rows(ucv_enum category)
{
    int rows = 0;

    switch (category)
    {
    case ucv_length :
        rows = sizeof(length_name_val) / sizeof(length_name_val[0]);
        break;
    case ucv_area :
        rows = sizeof(area_name_val) / sizeof(area_name_val[0]);
        break;
    case ucv_volume :
        rows = sizeof(volume_name_val) / sizeof(volume_name_val[0]);
        break;
    case ucv_mass :
        rows = sizeof(mass_name_val) / sizeof(mass_name_val[0]);
        break;
    case ucv_speed :
        rows = sizeof(speed_name_val) / sizeof(speed_name_val[0]);
        break;
    case ucv_pressure :
        rows = sizeof(pressure_name_val) / sizeof(pressure_name_val[0]);
        break;
    case ucv_force :
        rows = sizeof(force_name_val) / sizeof(force_name_val[0]);
        break;
    case ucv_energy :
        rows = sizeof(energy_name_val) / sizeof(energy_name_val[0]);
        break;
    case ucv_power :
        rows = sizeof(power_name_val) / sizeof(power_name_val[0]);
        break;
    case ucv_temperature :
        rows = sizeof(temperature_name_val) / sizeof(temperature_name_val[0]);
        break;
    case ucv_torque :
        rows = sizeof(torque_name_val) / sizeof(torque_name_val[0]);
        break;
    case ucv_fuel_economy :
        rows = sizeof(fuel_economy_name_val) / sizeof(fuel_economy_name_val[0]);
        break;
    default :
        break;
    }
    return rows;
}

static const name_val_t *get_name_val(ucv_enum category)
{
    switch (category)
    {
    case ucv_length :
        return length_name_val;
    case ucv_area :
        return area_name_val;
    case ucv_volume :
        return volume_name_val;
    case ucv_mass :
        return mass_name_val;
    case ucv_speed :
        return speed_name_val;
    case ucv_pressure :
        return pressure_name_val;
    case ucv_force :
        return force_name_val;
    case ucv_energy :
        return energy_name_val;
    case ucv_power :
        return power_name_val;
    case ucv_temperature :
        return temperature_name_val;
    case ucv_torque :
        return torque_name_val;
    case ucv_fuel_economy :
        return fuel_economy_name_val;
    default :
        return NULL;
    }
}

/* helper functions for fuel economy conversion */
#define FUEL_ECO_K_IMP "282.4809363318221585938121371192238"
#define FUEL_ECO_K_US "235.2145833333333333333333333333333"
static stackf_t l100_to_mpg(stackf_t x)
{
    /* FUEL_ECO_K_IMP / x */
    stackf_t res, a;
    dfp_from_string(&a, FUEL_ECO_K_IMP, &dfp_context);
    dfp_divide(&res, &a, &x, &dfp_context);
    return res;
}
static stackf_t l100_to_mpg_us(stackf_t x)
{
    /* FUEL_ECO_K_US / x */
    stackf_t res, a;
    dfp_from_string(&a, FUEL_ECO_K_US, &dfp_context);
    dfp_divide(&res, &a, &x, &dfp_context);
    return res;
}
static stackf_t mpg_to_l100(stackf_t x)
{
    /* FUEL_ECO_K_IMP / x */
    /* symmetric */
    return l100_to_mpg(x);
}
static stackf_t mpg_to_mpg_us(stackf_t x)
{
    /* x * FUEL_ECO_K_US / FUEL_ECO_K_IMP */
    stackf_t res, a, b;
    dfp_from_string(&a, FUEL_ECO_K_US, &dfp_context);
    dfp_from_string(&b, FUEL_ECO_K_IMP, &dfp_context);
    dfp_multiply(&res, &a, &x, &dfp_context);
    dfp_divide(&res, &res, &b, &dfp_context);
    return res;
}
static stackf_t mpg_us_to_l100(stackf_t x)
{
    /* FUEL_ECO_K_US / x */
    /* symmetric */
    return l100_to_mpg_us(x);
}
static stackf_t mpg_us_to_mpg(stackf_t x)
{
    /* x * FUEL_ECO_K_IMP / FUEL_ECO_K_US */
    stackf_t res, a, b;
    dfp_from_string(&a, FUEL_ECO_K_IMP, &dfp_context);
    dfp_from_string(&b, FUEL_ECO_K_US, &dfp_context);
    dfp_multiply(&res, &a, &x, &dfp_context);
    dfp_divide(&res, &res, &b, &dfp_context);
    return res;
}

/* Get fuel economy value, handled as a special case */
static stackf_t get_fuel_economy_val(int row)
{
    stackf_t res = value_to_convert;

    /*
     * assumes row 0 = litre/100km
     *         row 1 = mpg (imp)
     *         row 2 = mpg (US)
     */
    if (convert_from_row == 0)
    {
        /* convert from l/100km */
        if (row == 1)       // to mpg(imp)
            res = l100_to_mpg(value_to_convert);
        else if (row == 2)  // to mpg(us)
            res = l100_to_mpg_us(value_to_convert);
    }
    else if (convert_from_row == 1)
    {
        /* convert from mpg(imp) */
        if (row == 0)       // to l/100km
            res = mpg_to_l100(value_to_convert);
        else if (row == 2)  // to mpg(us)
            res = mpg_to_mpg_us(value_to_convert);
    }
    else
    {
        /* convert from mpg(us) */
        if (row == 0)       // to l/100km
            res = mpg_us_to_l100(value_to_convert);
        else if (row == 1)  // to mpg(imp)
            res = mpg_us_to_mpg(value_to_convert);
    }

    return res;
}

/* helper functions for temperature conversion */
static stackf_t c_to_f(stackf_t tc)
{
    /* (tc * 1.8) + 32 */
    stackf_t a, b, res;
    dfp_from_string(&a, "1.8", &dfp_context);
    dfp_from_string(&b, "32", &dfp_context);
    dfp_multiply(&res, &tc, &a, &dfp_context);
    dfp_add(&res, &res, &b, &dfp_context);
    return res;
}
static stackf_t c_to_k(stackf_t tc)
{
    /* tc + 273.15 */
    stackf_t a, res;
    dfp_from_string(&a, "273.15", &dfp_context);
    dfp_add(&res, &tc, &a, &dfp_context);
    return res;
}
static stackf_t f_to_c(stackf_t tf)
{
    /* (tf - 32) / 1.8 */
    stackf_t a, b, res;
    dfp_from_string(&a, "1.8", &dfp_context);
    dfp_from_string(&b, "32", &dfp_context);
    dfp_subtract(&res, &tf, &b, &dfp_context);
    dfp_divide(&res, &res, &a, &dfp_context);
    return res;
}
static stackf_t f_to_k(stackf_t tf)
{
    stackf_t res;
    res = f_to_c(tf);
    res = c_to_k(res);
    return res;
}
static stackf_t k_to_c(stackf_t tk)
{
    /* tk - 273.15 */
    stackf_t a, res;
    dfp_from_string(&a, "273.15", &dfp_context);
    dfp_subtract(&res, &tk, &a, &dfp_context);
    return res;
}
static stackf_t k_to_f(stackf_t tk)
{
    stackf_t res;
    res = k_to_c(tk);
    res = c_to_f(res);
    return res;
}

/* calculate temperature value, handled as a special case */
static stackf_t get_temperature_val(int row)
{
    stackf_t res = value_to_convert;

    /*
     * assumes row 0 = C
     *         row 1 = F
     *         row 2 = K
     */

    if (convert_from_row == 0)
    {
        /* convert from C */
        if (row == 1)      // to F
            res = c_to_f(value_to_convert);
        else if (row == 2) // to K
            res = c_to_k(value_to_convert);
    }
    else if (convert_from_row == 1)
    {
        /* convert from F */
        if (row == 0)       // to C
            res = f_to_c(value_to_convert);
        else if (row == 2)  // to K
            res = f_to_k(value_to_convert);
    }
    else
    {
        /* convert from K */
        if (row == 0)       // to C
            res = k_to_c(value_to_convert);
        else if (row == 1)  // to F
            res = k_to_f(value_to_convert);
    }

    return res;
}


/* Calculate the value for a given row, returned in val, depends on
 * the 'convert from' units (ie. convert_from_row) and the fixed
 * value_to_convert. */
static bool calculate_value(ucv_enum category, int row, stackf_t *val)
{
    const name_val_t *nv = get_name_val(category);
    int max_rows = calc_conversion_get_num_rows(category);

    if (nv == NULL || row >= max_rows)
    {
        /* shouldn't happen */
        return false;
    }

    /* Handle temperature and fuel economy as special cases */
    if (category == ucv_temperature)
    {
        *val = get_temperature_val(row);
        return true;
    }
    if (category == ucv_fuel_economy)
    {
        *val = get_fuel_economy_val(row);
        return true;
    }

    /* The normal case for all other conversions */
    stackf_t cf, res;

    /* get conversion factor */
    if (row == convert_from_row)
    {
        dfp_from_string(&cf, "1", &dfp_context);
    }
    else
    {
        stackf_t a, b;
        dfp_from_string(&a, nv[row].val, &dfp_context);
        dfp_from_string(&b, nv[convert_from_row].val, &dfp_context);
        dfp_divide(&cf, &a, &b, &dfp_context);
    }
    /* multiply the value_to_convert by conversion factor */
    dfp_multiply(&res, &cf, &value_to_convert, &dfp_context);
    *val = res;
    return true;
}

/* Pass result into calculator */
void calc_conversion_apply_result(ucv_enum category, int row)
{
    stackf_t fval;

    if (!calculate_value(category, row, &fval))
    {
        /* shouldn't happen */
        dfp_zero(&fval);
    }

    calc_give_arg(0, fval);
    calc_give_op(cop_peek);
}

/* Fix the value to be converted. If it's 0 then, with exception of
 * temperature conversion, replace with 1. */
void calc_conversion_fix_value(ucv_enum category)
{
    stackf_t fval = calc_get_fval_top_of_stack();
    if (category != ucv_temperature && dfp_is_zero(&fval))
    {
        dfp_from_string(&fval, "1", &dfp_context);
    }
    value_to_convert = fval;
}

/* Get units name, returned in buf */
void calc_conversion_get_name(ucv_enum category, int row, char *buf, int buf_len)
{
    const name_val_t *nv = get_name_val(category);
    int max_rows = calc_conversion_get_num_rows(category);

    buf[0] = 0;

    if (nv == NULL || row >= max_rows)
    {
        /* shouldn't happen */
        return;
    }

    strncat(buf, nv[row].name, buf_len - 1);
}

/* Get (rounded) value as a string, returned in buf */
void calc_conversion_get_value(ucv_enum category, int row, char *buf, int buf_len)
{
    stackf_t res;
    char buf_temp[DFP_STRING_MAX];

    buf[0] = 0;

    if (!calculate_value(category, row, &res))
    {
        /* shouldn't happen */
        return;
    }
    /* round to 10 places, fine for displaying in conversion window */
    display_print_gmode(buf_temp, res, 10);
    strncat(buf, buf_temp, buf_len - 1);
}

/* Sets the 'convert from' units */
void calc_conversion_set_convert_from_row(ucv_enum category, int row)
{
    int max_rows = calc_conversion_get_num_rows(category);
    if (row >= max_rows)
    {
        /* shouldn't happen */
        convert_from_row = 0;
    }
    else
    {
        convert_from_row = row;
    }
}

/* One function used from constants gui to pass value into calculator */
void calc_constants_apply_result(const char *value)
{
    stackf_t fval;
    dfp_from_string(&fval, value, &dfp_context);
    calc_give_arg(0, fval);
    calc_give_op(cop_peek);
}

