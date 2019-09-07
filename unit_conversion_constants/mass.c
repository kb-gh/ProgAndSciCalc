/*****************************************************************************
 * File mass.c part of ProgAndSciCalc
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
static const name_val_t mass_name_val[] =
{
    { "g",       "nnn" }, etc.
    { "kg",
    { "tonne",
    { "ounce",
    { "pound",
    { "stone",
    { "cwt (imp)",
    { "ton (imp)",
    { "cwt (US)",
    { "ton (US)",  "zzz" },
};
*/

int main(void)
{
    decContext dfp_context;
    char buf[DFP_STRING_MAX];

    decContextDefault(&dfp_context, DEC_INIT_DECQUAD);

    decQuad a, b, kg_to_oz, res;

    /* kg_to_oz  1e3 / 28.349523125 */
    decQuadFromString(&a, "1e3", &dfp_context);
    decQuadFromString(&b, "28.349523125", &dfp_context);
    decQuadDivide(&kg_to_oz, &a, &b, &dfp_context);

    printf("static const name_val_t mass_name_val[] =\n");
    printf("{\n");

    /* g */
    printf("    { \"%s\", \t\t\"%s\" },\n", "g", "1e3");
    /* kg */
    printf("    { \"%s\", \t\t\"%s\" },\n", "kg", "1");
    /* tonne */
    printf("    { \"%s\", \t\t\"%s\" },\n", "tonne", "1e-3");
    /* ounce */
    decQuadToString(&kg_to_oz, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "ounce", buf);
    /* pound  kg_to_oz / 16 */
    decQuadFromString(&b, "16", &dfp_context);
    decQuadDivide(&res, &kg_to_oz, &b, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "pound", buf);
    /* stone  kg_to_oz / 16 / 14 */
    decQuadFromString(&b, "224", &dfp_context);
    decQuadDivide(&res, &kg_to_oz, &b, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "stone", buf);
    /* cwt (imp)  kg_to_oz / 16 / 112 */
    decQuadFromString(&b, "1792", &dfp_context);
    decQuadDivide(&res, &kg_to_oz, &b, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "cwt (imp)", buf);
    /* ton (imp)  kg_to_oz / 16 / 2240 */
    decQuadFromString(&b, "35840", &dfp_context);
    decQuadDivide(&res, &kg_to_oz, &b, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "ton (imp)", buf);
    /* cwt (US)  kg_to_oz / 16 / 100 */
    decQuadFromString(&b, "1600", &dfp_context);
    decQuadDivide(&res, &kg_to_oz, &b, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "cwt (US)", buf);
    /* ton (US)  kg_to_oz / 16 / 2000 */
    decQuadFromString(&b, "32000", &dfp_context);
    decQuadDivide(&res, &kg_to_oz, &b, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "ton (US)", buf);

    printf("};\n");
    return 0;
}
