/*****************************************************************************
 * File pressure.c part of ProgAndSciCalc
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
static const name_val_t pressure_name_val[] =
{
    { "Pa",       "nnn" }, etc.
    { "mbar",
    { "bar",
    { "atm",
    { "psi",      "zzz" },
};
*/

int main(void)
{
    decContext dfp_context;
    char buf[DFP_STRING_MAX];

    decContextDefault(&dfp_context, DEC_INIT_DECQUAD);

    decQuad a, b, g, kg_to_oz, sq_m_to_inch, res;

    /* g */
    decQuadFromString(&g, "9.80665", &dfp_context);

    /* kg_to_oz  1e3 / 28.349523125 */
    decQuadFromString(&a, "1e3", &dfp_context);
    decQuadFromString(&b, "28.349523125", &dfp_context);
    decQuadDivide(&kg_to_oz, &a, &b, &dfp_context);

    /* sq_m_to_inch  10000 / 2.54 / 2.54 */
    decQuadFromString(&a, "10000", &dfp_context);
    decQuadFromString(&b, "2.54", &dfp_context);
    decQuadDivide(&sq_m_to_inch, &a, &b, &dfp_context);
    decQuadDivide(&sq_m_to_inch, &sq_m_to_inch, &b, &dfp_context);

    printf("static const name_val_t pressure_name_val[] =\n");
    printf("{\n");

    /* Pa */
    printf("    { \"%s\", \t\t\"%s\" },\n", "Pa", "1");
    /* mbar  1e-2 */
    printf("    { \"%s\", \t\t\"%s\" },\n", "mbar", "1e-2");
    /* bar  1e-5 */
    printf("    { \"%s\", \t\t\"%s\" },\n", "bar", "1e-5");
    /* atm  1.0 / 101325.0 */
    decQuadFromString(&a, "1", &dfp_context);
    decQuadFromString(&b, "101325", &dfp_context);
    decQuadDivide(&res, &a, &b, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "atm", buf);
    /* psi  kg_to_oz / 16.0 / _g / sq_m_to_inch */
    decQuadFromString(&a, "16", &dfp_context);
    decQuadMultiply(&b, &a, &g, &dfp_context);
    decQuadMultiply(&b, &b, &sq_m_to_inch, &dfp_context);
    decQuadDivide(&res, &kg_to_oz, &b, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "psi", buf);

    printf("};\n");
    return 0;
}
