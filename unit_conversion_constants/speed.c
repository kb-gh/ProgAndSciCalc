/*****************************************************************************
 * File speed.c part of ProgAndSciCalc
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
static const name_val_t speed_name_val[] =
{
    { "m/s",       "nnn" }, etc.
    { "km/h",
    { "ft/s",
    { "mile/h",
    { "knot",      "zzz" },
};
*/

int main(void)
{
    decContext dfp_context;
    char buf[DFP_STRING_MAX];

    decContextDefault(&dfp_context, DEC_INIT_DECQUAD);

    decQuad a, b, m_to_inch, res;

    /* m_to_inch  100 / 2.54 */
    decQuadFromString(&a, "100", &dfp_context);
    decQuadFromString(&b, "2.54", &dfp_context);
    decQuadDivide(&m_to_inch, &a, &b, &dfp_context);

    printf("static const name_val_t speed_name_val[] =\n");
    printf("{\n");

    /* m/s */
    printf("    { \"%s\", \t\t\"%s\" },\n", "m/s", "1");
    /* km/h  3600 / 1000 */
    decQuadFromString(&a, "3600", &dfp_context);
    decQuadFromString(&b, "1000", &dfp_context);
    decQuadDivide(&res, &a, &b, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "km/h", buf);
    /* ft/s  m_to_inch / 12 */
    decQuadFromString(&b, "12", &dfp_context);
    decQuadDivide(&res, &m_to_inch, &b, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "ft/s", buf);
    /* mile/h  3600 * m_to_inch / 63360 */
    decQuadFromString(&a, "3600", &dfp_context);
    decQuadFromString(&b, "63360", &dfp_context);
    decQuadMultiply(&res, &m_to_inch, &a, &dfp_context);
    decQuadDivide(&res, &res, &b, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "mile/h", buf);
    /* knot  3600 / 1852 */
    decQuadFromString(&a, "3600", &dfp_context);
    decQuadFromString(&b, "1852", &dfp_context);
    decQuadDivide(&res, &a, &b, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "knot", buf);

    printf("};\n");
    return 0;
}
