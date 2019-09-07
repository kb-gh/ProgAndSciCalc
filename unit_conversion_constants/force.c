/*****************************************************************************
 * File force.c part of ProgAndSciCalc
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
static const name_val_t force_name_val[] =
{
    { "N",        "nnn" },
    { "kgf",      "xxx" },
    { "lbf",      "zzz" },
};
*/

int main(void)
{
    decContext dfp_context;
    char buf[DFP_STRING_MAX];

    decContextDefault(&dfp_context, DEC_INIT_DECQUAD);

    decQuad a, b, g, kg_to_oz, res;

    /* g */
    decQuadFromString(&g, "9.80665", &dfp_context);

    /* kg_to_oz  1e3 / 28.349523125 */
    decQuadFromString(&a, "1e3", &dfp_context);
    decQuadFromString(&b, "28.349523125", &dfp_context);
    decQuadDivide(&kg_to_oz, &a, &b, &dfp_context);

    printf("static const name_val_t force_name_val[] =\n");
    printf("{\n");

    /* N */
    printf("    { \"%s\", \t\t\"%s\" },\n", "N", "1");
    /* kgf  1 / g */
    decQuadFromString(&a, "1", &dfp_context);
    decQuadDivide(&res, &a, &g, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "kgf", buf);
    /* lbf  kg_to_oz / 16.0 / _g */
    decQuadFromString(&a, "16", &dfp_context);
    decQuadMultiply(&b, &a, &g, &dfp_context);
    decQuadDivide(&res, &kg_to_oz, &b, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "lbf", buf);

    printf("};\n");
    return 0;
}
