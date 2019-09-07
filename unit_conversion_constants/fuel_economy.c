/*****************************************************************************
 * File fuel_economy.c part of ProgAndSciCalc
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
#include <stdint.h>
#include <stdbool.h>
#include "decQuad.h"

#define DFP_STRING_MAX  DECQUAD_String  // 43

/*
    a bit different, just generate 2 constants

#define FUEL_ECO_K_IMP "xxxxxxxxxxxxxx"
#define FUEL_ECO_K_US  "xxxxxxxxxxxxxx"

*/

int main(void)
{
    decContext dfp_context;
    char buf[DFP_STRING_MAX];

    decContextDefault(&dfp_context, DEC_INIT_DECQUAD);

    decQuad a, b, res, m_to_inch, km_to_mile;
    decQuad l_to_pint, l_to_pint_us;
    decQuad l_to_gallon, l_to_gallon_us;

    /* m_to_inch  100 / 2.54 */
    decQuadFromString(&a, "100", &dfp_context);
    decQuadFromString(&b, "2.54", &dfp_context);
    decQuadDivide(&m_to_inch, &a, &b, &dfp_context);

    /* l_to_pint  1000 / 568.26125 */
    decQuadFromString(&a, "1000", &dfp_context);
    decQuadFromString(&b, "568.26125", &dfp_context);
    decQuadDivide(&l_to_pint, &a, &b, &dfp_context);

    /* l_to_pint_us  1000 / 473.176473 */
    decQuadFromString(&a, "1000", &dfp_context);
    decQuadFromString(&b, "473.176473", &dfp_context);
    decQuadDivide(&l_to_pint_us, &a, &b, &dfp_context);

    /* km_to_mile  1000 * m_to_inch / 63360 */
    decQuadFromString(&a, "1000", &dfp_context);
    decQuadFromString(&b, "63360", &dfp_context);
    decQuadMultiply(&res, &a, &m_to_inch, &dfp_context);
    decQuadDivide(&km_to_mile, &res, &b, &dfp_context);

    /* l_to_gallon  l_to_pint / 8 */
    decQuadFromString(&b, "8", &dfp_context);
    decQuadDivide(&l_to_gallon, &l_to_pint, &b, &dfp_context);

    /* l_to_gallon_us  l_to_pint_us / 8 */
    decQuadFromString(&b, "8", &dfp_context);
    decQuadDivide(&l_to_gallon_us, &l_to_pint_us, &b, &dfp_context);


    /* FUEL_ECO_K_IMP  100.0 * km_to_mile / l_to_gallon */
    decQuadFromString(&a, "100", &dfp_context);
    decQuadMultiply(&res, &a, &km_to_mile, &dfp_context);
    decQuadDivide(&res, &res, &l_to_gallon, &dfp_context);
    decQuadToString(&res, buf);
    printf("#define FUEL_ECO_K_IMP \"%s\"\n", buf);

    /* FUEL_ECO_K_US  100.0 * km_to_mile / l_to_gallon_us */
    decQuadFromString(&a, "100", &dfp_context);
    decQuadMultiply(&res, &a, &km_to_mile, &dfp_context);
    decQuadDivide(&res, &res, &l_to_gallon_us, &dfp_context);
    decQuadToString(&res, buf);
    printf("#define FUEL_ECO_K_US \"%s\"\n", buf);


    return 0;
}
