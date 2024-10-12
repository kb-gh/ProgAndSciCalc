/*****************************************************************************
 * File power.c part of ProgAndSciCalc
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
static const name_val_t power_name_val[] =
{
    { "W",          "nnn" }, etc.
    { "kW",
    { "HP (imp)",
    { "HP (metric)",
    { "BTU / hour",     "zzz" },
};
*/

int main(void)
{
    decContext dfp_context;
    char buf[DFP_STRING_MAX];

    decContextDefault(&dfp_context, DEC_INIT_DECQUAD);

    decQuad a, b, g, nm_to_lbft, m_to_inch, kg_to_oz, res;

    /* g */
    decQuadFromString(&g, "9.80665", &dfp_context);

    /* m_to_inch  100 / 2.54 */
    decQuadFromString(&a, "100", &dfp_context);
    decQuadFromString(&b, "2.54", &dfp_context);
    decQuadDivide(&m_to_inch, &a, &b, &dfp_context);

    /* kg_to_oz  1e3 / 28.349523125 */
    decQuadFromString(&a, "1e3", &dfp_context);
    decQuadFromString(&b, "28.349523125", &dfp_context);
    decQuadDivide(&kg_to_oz, &a, &b, &dfp_context);

    /* nm_to_lbft  m_to_inch * kg_to_oz / 16.0 / _g / 12.0 */
    decQuadFromString(&a, "192", &dfp_context);
    decQuadMultiply(&b, &a, &g, &dfp_context);
    decQuadMultiply(&a, &m_to_inch, &kg_to_oz, &dfp_context);
    decQuadDivide(&nm_to_lbft, &a, &b, &dfp_context);

    printf("static const name_val_t power_name_val[] =\n");
    printf("{\n");

    /* W */
    printf("    { \"%s\", \t\t\"%s\" },\n", "W", "1");
    /* kW */
    printf("    { \"%s\", \t\t\"%s\" },\n", "kW", "1e-3");
    /* HP (imp)  nm_to_lbft / 550.0*/
    decQuadFromString(&b, "550", &dfp_context);
    decQuadDivide(&res, &nm_to_lbft, &b, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "HP (imp)", buf);
    /* HP (metric)  1 / 735.49875 */
    decQuadFromString(&a, "1", &dfp_context);
    decQuadFromString(&b, "735.49875", &dfp_context);
    decQuadDivide(&res, &a, &b, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "HP (metric)", buf);

    /* 1W is 1 / 1055.05585262 BTU per sec
     *  = 3600 / 1055.05585262 BTU per hour */
    decQuadFromString(&a, "3600", &dfp_context);
    decQuadFromString(&b, "1055.05585262", &dfp_context);
    decQuadDivide(&res, &a, &b, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "BTU / hour", buf);

    printf("};\n");
    return 0;
}
