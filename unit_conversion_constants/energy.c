/*****************************************************************************
 * File energy.c part of ProgAndSciCalc
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
static const name_val_t energy_name_val[] =
{
    { "eV",      "nnn" }, etc.
    { "keV",
    { "Mev",
    { "J",
    { "kJ",
    { "kWh",
    { "cal",
    { "kcal",
    { "BTU",     "zzz" },
};
*/

int main(void)
{
    decContext dfp_context;
    char buf[DFP_STRING_MAX];

    decContextDefault(&dfp_context, DEC_INIT_DECQUAD);

    decQuad a, b, res, ev;

    printf("static const name_val_t energy_name_val[] =\n");
    printf("{\n");

    /* eV  1 / 1.6021766208e-19 */
    decQuadFromString(&a, "1", &dfp_context);
    decQuadFromString(&b, "1.6021766208e-19", &dfp_context);
    decQuadDivide(&ev, &a, &b, &dfp_context);
    decQuadToString(&ev, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "eV", buf);
    /* keV  1 / 1.6021766208e-19 / 1e3 */
    decQuadFromString(&a, "1e3", &dfp_context);
    decQuadDivide(&res, &ev, &a, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "keV", buf);
    /* MeV  1 / 1.6021766208e-19 / 1e6 */
    decQuadFromString(&a, "1e6", &dfp_context);
    decQuadDivide(&res, &ev, &a, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "MeV", buf);

    /* J */
    printf("    { \"%s\", \t\t\"%s\" },\n", "J", "1");
    /* kJ */
    printf("    { \"%s\", \t\t\"%s\" },\n", "kJ", "1e-3");
    /* kWh  1 / 1000 / 3600 */
    decQuadFromString(&a, "1", &dfp_context);
    decQuadFromString(&b, "3600e3", &dfp_context);
    decQuadDivide(&res, &a, &b, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "kWh", buf);
    /* cal  1 / 4.1868 */
    decQuadFromString(&a, "1", &dfp_context);
    decQuadFromString(&b, "4.1868", &dfp_context);
    decQuadDivide(&res, &a, &b, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "cal", buf);
    /* kcal  1 / 4.1868 / 1000 */
    decQuadFromString(&a, "1", &dfp_context);
    decQuadFromString(&b, "4.1868e3", &dfp_context);
    decQuadDivide(&res, &a, &b, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "kcal", buf);
    /* BTU  1.0 / 1055.06 */
    decQuadFromString(&a, "1", &dfp_context);
    decQuadFromString(&b, "1055.06", &dfp_context);
    decQuadDivide(&res, &a, &b, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "BTU", buf);

    printf("};\n");
    return 0;
}
