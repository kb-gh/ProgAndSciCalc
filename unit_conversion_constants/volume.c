/*****************************************************************************
 * File volume.c part of ProgAndSciCalc
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
static const name_val_t volume_name_val[] =
{
    { "ml",    "nnn" }, etc.
    { "litre",
    { "cu m",
    { "cu inch",
    { "cu foot",
    { "cu yard",
    { "fl oz (imp)",
    { "pint (imp)",
    { "gallon (imp)",
    { "fl oz (US)",
    { "pint (US)",
    { "gallon (US)",   "zzz" },
};
*/

int main(void)
{
    decContext dfp_context;
    char buf[DFP_STRING_MAX];

    decContextDefault(&dfp_context, DEC_INIT_DECQUAD);

    decQuad a, b, cu_m_to_inch, res, cu_m_to_pint, cu_m_to_pint_us;

    /* cu m_to_inch  1e6 / 2.54 / 2.54 / 2.54 */
    decQuadFromString(&a, "1e6", &dfp_context);
    decQuadFromString(&b, "2.54", &dfp_context);
    decQuadDivide(&cu_m_to_inch, &a, &b, &dfp_context);
    decQuadDivide(&cu_m_to_inch, &cu_m_to_inch, &b, &dfp_context);
    decQuadDivide(&cu_m_to_inch, &cu_m_to_inch, &b, &dfp_context);

    /* cu m_to_pint  1e6 / 568.26125 */
    decQuadFromString(&a, "1e6", &dfp_context);
    decQuadFromString(&b, "568.26125", &dfp_context);
    decQuadDivide(&cu_m_to_pint, &a, &b, &dfp_context);

    /* cu m_to_pint_us  1e6 / 473.176473 */
    decQuadFromString(&a, "1e6", &dfp_context);
    decQuadFromString(&b, "473.176473", &dfp_context);
    decQuadDivide(&cu_m_to_pint_us, &a, &b, &dfp_context);


    printf("static const name_val_t volume_name_val[] =\n");
    printf("{\n");

    /* ml */
    printf("    { \"%s\", \t\t\"%s\" },\n", "ml (cc)", "1e6");
    /* litre */
    printf("    { \"%s\", \t\t\"%s\" },\n", "litre", "1e3");
    /* cu m */
    printf("    { \"%s\", \t\t\"%s\" },\n", "cu m", "1");
    /* cu inch */
    decQuadToString(&cu_m_to_inch, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "cu inch", buf);
    /* cu foot  cu_m_to_inch / 1728 */
    decQuadFromString(&b, "1728", &dfp_context);
    decQuadDivide(&res, &cu_m_to_inch, &b, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "cu foot", buf);
    /* cu yard  cu_m_to_inch / 46656 */
    decQuadFromString(&b, "46656", &dfp_context);
    decQuadDivide(&res, &cu_m_to_inch, &b, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "cu yard", buf);
    /* fl oz (imp)  20 * cu_m_to_pint */
    decQuadFromString(&b, "20", &dfp_context);
    decQuadMultiply(&res, &b, &cu_m_to_pint, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "fl oz (imp)", buf);
    /* pint (imp) */
    decQuadToString(&cu_m_to_pint, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "pint (imp)", buf);
    /* gallon (imp)  cu_m_to_pint / 8 */
    decQuadFromString(&b, "8", &dfp_context);
    decQuadDivide(&res, &cu_m_to_pint, &b, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "gallon (imp)", buf);
    /* fl oz (us)  16 * cu_m_to_pint_us */
    decQuadFromString(&b, "16", &dfp_context);
    decQuadMultiply(&res, &b, &cu_m_to_pint_us, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "fl oz (US)", buf);
    /* pint (us) */
    decQuadToString(&cu_m_to_pint_us, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "pint (US)", buf);
    /* gallon (us)  cu_m_to_pint_us / 8 */
    decQuadFromString(&b, "8", &dfp_context);
    decQuadDivide(&res, &cu_m_to_pint_us, &b, &dfp_context);
    decQuadToString(&res, buf);
    printf("    { \"%s\", \t\t\"%s\" },\n", "gallon (US)", buf);

    printf("};\n");
    return 0;
}
