/*****************************************************************************
 * File length.c part of ProgAndSciCalc
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
//#include "decNumber/decimal128.h"

#define DFP_STRING_MAX  DECQUAD_String  // 43

/*
static const name_val_t length_name_val[] =
{
    { "mm", "nnn" }, etc.
    { "cm",
    { "m",
    { "km",
    { "thou",
    { "inch",
    { "foot",
    { "yard",
    { "mile",
    { "nautical mile", "zzz" },
};
*/

int main(void)
{
    decContext dfp_context;
    char buf[DFP_STRING_MAX];

    decContextDefault(&dfp_context, DEC_INIT_DECQUAD);

    decQuad a, b, m_to_inch, res;

    printf("static const name_val_t length_name_val[] =\n");
    printf("{\n");

    /* mm */
    printf("    { \"%s\", \t\t\"%s\" },\n", "mm", "1000");
    /* cm */
    printf("    { \"%s\", \t\t\"%s\" },\n", "cm", "100");
    /* m */
    printf("    { \"%s\", \t\t\"%s\" },\n", "m", "1");
    /* km */
    printf("    { \"%s\", \t\t\"%s\" },\n", "km", "0.001");
    /* thou  100 * 1000 / 2.54 */
    decQuadFromString(&a, "100000", &dfp_context);
    decQuadFromString(&b, "2.54", &dfp_context);
    decQuadDivide(&res, &a, &b, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "thou", buf);
    /* inch  100 / 2.54 */
    decQuadFromString(&a, "100", &dfp_context);
    decQuadFromString(&b, "2.54", &dfp_context);
    decQuadDivide(&m_to_inch, &a, &b, &dfp_context);
    decQuadToString(&m_to_inch, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "inch", buf);
    /* foot  m_to_in / 12 */
    decQuadFromString(&a, "12", &dfp_context);
    decQuadDivide(&res, &m_to_inch, &a, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "foot", buf);
    /* yard  m_to_in / 36 */
    decQuadFromString(&a, "36", &dfp_context);
    decQuadDivide(&res, &m_to_inch, &a, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "yard", buf);
    /* mile  m_to_in / 63360 */
    decQuadFromString(&a, "63360", &dfp_context);
    decQuadDivide(&res, &m_to_inch, &a, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "mile", buf);
    /* nautical mile  1 / 1852 */
    decQuadFromString(&a, "1", &dfp_context);
    decQuadFromString(&b, "1852", &dfp_context);
    decQuadDivide(&res, &a, &b, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \"%s\" },\n", "nautical mile", buf);

    printf("};\n");
    return 0;
}
