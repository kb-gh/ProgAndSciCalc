/*****************************************************************************
 * File area.c part of ProgAndSciCalc
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
static const name_val_t area_name_val[] =
{
    { "sq mm",    "nnn" }, etc.
    { "sq cm",
    { "sq m",
    { "hectare",
    { "sq km",
    { "sq inch",
    { "sq foot",
    { "sq yard",
    { "acre",
    { "sq mile",  "zzz" },
};
*/

int main(void)
{
    decContext dfp_context;
    char buf[DFP_STRING_MAX];

    decContextDefault(&dfp_context, DEC_INIT_DECQUAD);

    decQuad a, b, sq_m_to_inch, res;

    printf("static const name_val_t area_name_val[] =\n");
    printf("{\n");

    /* sq m */
    printf("    { \"%s\", \t\t\"%s\" },\n", "sq mm", "1e6");
    /* sq cm */
    printf("    { \"%s\", \t\t\"%s\" },\n", "sq cm", "10000");
    /* sq m */
    printf("    { \"%s\", \t\t\"%s\" },\n", "sq m", "1");
    /* hectare */
    printf("    { \"%s\", \t\t\"%s\" },\n", "hectare", "1e-4");
    /* sq km */
    printf("    { \"%s\", \t\t\"%s\" },\n", "sq km", "1e-6");
    /* sq inch  10000 / 2.54 / 2.54 */
    decQuadFromString(&a, "10000", &dfp_context);
    decQuadFromString(&b, "2.54", &dfp_context);
    decQuadDivide(&sq_m_to_inch, &a, &b, &dfp_context);
    decQuadDivide(&sq_m_to_inch, &sq_m_to_inch, &b, &dfp_context);
    decQuadToString(&sq_m_to_inch, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "sq inch", buf);
    /* sq foot  sqm_to_in / 144 */
    decQuadFromString(&a, "144", &dfp_context);
    decQuadDivide(&res, &sq_m_to_inch, &a, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "sq foot", buf);
    /* sq yard  sqm_to_in / 1296 */
    decQuadFromString(&a, "1296", &dfp_context);
    decQuadDivide(&res, &sq_m_to_inch, &a, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "sq yard", buf);
    /* acre  640.0 * sq_m_to_in / 63360 / 63360  */
    decQuadFromString(&b, "640", &dfp_context);
    decQuadMultiply(&a, &sq_m_to_inch, &b, &dfp_context);
    decQuadFromString(&b, "63360", &dfp_context);
    decQuadDivide(&res, &a, &b, &dfp_context);
    decQuadDivide(&res, &res, &b, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "acre", buf);
    /* sq mile  sq_m_to_in / 63360.0 / 63360.0  */
    decQuadFromString(&b, "63360", &dfp_context);
    decQuadDivide(&res, &sq_m_to_inch, &b, &dfp_context);
    decQuadDivide(&res, &res, &b, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "sq mile", buf);

    printf("};\n");
    return 0;
}
