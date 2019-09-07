/* Decimal Number Library - Math Functions

   Copyright (C) 2006 IBM Corporation.
   Copyright (C) 2007-2015 Free Software Foundation, Inc.

   This file is part of the Decimal Floating Point C Library.

   Contributed by IBM Corporation.

   The Decimal Floating Point C Library is free software; you can
   redistribute it and/or modify it under the terms of the GNU Lesser
   General Public License version 2.1.

   The Decimal Floating Point C Library is distributed in the hope that
   it will be useful, but WITHOUT ANY WARRANTY; without even the implied
   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
   the GNU Lesser General Public License version 2.1 for more details.

   You should have received a copy of the GNU Lesser General Public
   License version 2.1 along with the Decimal Floating Point C Library;
   if not, write to the Free Software Foundation, Inc., 59 Temple Place,
   Suite 330, Boston, MA 02111-1307 USA.

   Please see libdfp/COPYING.txt for more information.  */


/* Calculator uses decQuad precision (34) throughout */
#define  DECNUMDIGITS 34
//#define  DECNUMDIGITS 60
#include "decNumber.h"             // base number library
#include "decNumberMath.h"

#define E  "2.7182818284590452353602874713526624977572470936999595749669676277240766303535"
#define PI "3.1415926535897932384626433832795028841971693993751058209749445923078164062862"


// ----------------------------------------------------------------------
// Basic Functions
// ----------------------------------------------------------------------

// Return the largest integer less than or equal to x
static decNumber* decNumberFloor (decNumber *result, decNumber *x,
			   decContext *set)
{
  int round = set->round;

  set->round = DEC_ROUND_DOWN;
  decNumberToIntegralValue (result, x, set);
  set->round = round;
  return result;
} /* decNumberFloor  */

#if 0
static int decNumberIsEqual (decNumber *x, decNumber *y, decContext *set)
{
  decNumber diff;
  decNumberSubtract (&diff, x, y, set);
  return decNumberIsZero (&diff);
} /* decNumberIsEqual  */

static int decNumberIsInteger (decNumber *x, decContext *set)
{
  decNumber y;

  decNumberToIntegralValue (&y, x, set);
  return decNumberIsEqual (x, &y, set);
} /* decNumberIsInteger  */
#endif

// Modulo function
static decNumber* decNumberMod (decNumber *result, decNumber *x, decNumber *y,
			 decContext *set)
{
  // x mod y = x - k*y where k = floor(x/y)
  decNumber k;

  decNumberDivide (&k, x, y, set);
  decNumberFloor (&k, &k, set);
  decNumberMultiply (&k, &k, y, set);
  decNumberSubtract (result, x, &k, set);
  return result;
} /* decNumberMod  */


// ----------------------------------------------------------------------
// Hyperbolic Functions
// ----------------------------------------------------------------------

decNumber* decNumberSinh (decNumber *result, decNumber *x, decContext *set)
{
  // sinh x = (e^x - e^-x)/2
  decNumber ex, emx, mx, two;

  decNumberExp (&ex, x, set);
  decNumberMinus (&mx, x, set);
  decNumberExp (&emx, &mx, set);
  decNumberSubtract (result, &ex, &emx, set);
  decNumberFromString (&two, "2", set);
  decNumberDivide (result, result, &two, set);
  return result;
}

decNumber* decNumberCosh (decNumber *result, decNumber *x, decContext *set)
{
  // cosh x = (e^x + e^-x)/2
  decNumber ex, emx, mx, two;

  decNumberExp (&ex, x, set);
  decNumberMinus (&mx, x, set);
  decNumberExp (&emx, &mx, set);
  decNumberAdd (result, &ex, &emx, set);
  decNumberFromString (&two, "2", set);
  decNumberDivide (result, result, &two, set);
  return result;
} /* decNumberCosh  */

decNumber* decNumberTanh (decNumber *result, decNumber *x, decContext *set)
{
  // tanh x = sinh x / cosh x = (e^x - e^-x) / (e^x + e^-x)
  decNumber ex, emx, mx, denominator;
  decNumberExp (&ex, x, set);
  decNumberMinus (&mx, x, set);
  decNumberExp (&emx, &mx, set);
  decNumberSubtract (result, &ex, &emx, set);
  decNumberAdd (&denominator, &ex, &emx, set);
  decNumberDivide (result, result, &denominator, set);
  return result;
} /* decNumberTanh  */

// ----------------------------------------------------------------------
// Trigonometric Functions
// ----------------------------------------------------------------------

decNumber* decNumberSin (decNumber *result, decNumber *y, decContext *set)
{
  decNumber pi, pi2, zero, one, two, x, cnt, term, cmp;
  int i;
  int negate = 0;

  decNumberFromString (&zero,"0", set);
  decNumberFromString (&one, "1", set);
  decNumberFromString (&two, "2", set);
  decNumberFromString (&pi,  PI , set);

  // Copy the argument y, so we can modify it.
  decNumberCopy (&x, y);
  // sin -x = - sin x
  /* if (decCompare (&x, &zero) < 0) { */
  if (decNumberIsNegative (&x)) { // x < 0
    decNumberMinus (&x, &x, set);
    negate = 1;
  }
  // We now have x >= 0
  decNumberMultiply (&pi2, &pi, &two, set); // pi2 = 2*pi
  decNumberMod (&x, &x, &pi2, set);
  // We now have 0 <= x < 2*pi
  /*if (decCompare (&x, &pi) >= 0) {*/
  decNumberCompare (&cmp, &x, &pi, set);
  if (!decNumberIsNegative (&cmp)) {
    // x >= pi
    decNumberSubtract (&x, &x, &pi, set);
    negate = 1-negate;
  }
  // We now have 0 <= x < pi
  decNumberDivide (&pi2, &pi, &two, set); // pi2 = pi/2
  /*if (decCompare (&x, &pi2) >= 0) {*/
  decNumberCompare (&cmp, &x, &pi2, set);
  if (!decNumberIsNegative (&cmp)) {
    // x >= pi/2, so let x = pi-x
    decNumberSubtract (&x, &pi, &x, set);
  }
  // We now have 0 <= x <= pi/2.

  //             x^3   x^5    x^7
  // sin x = x - --- + --- - ---- + ...
  //              6    120   5040
  //
  // term(0) = x
  // term(i) = - term(i-1) * x^2 / ((2*i)*(2*i+1))

  decNumberCopy (&cnt, &two);
  decNumberCopy (&term, &x);
  decNumberCopy (result, &x);
  // DECNUMDIGITS+3 terms are enough to achieve the required precision.
  for (i=0; i<DECNUMDIGITS+3; i++) {
    // term = -term * x^2 / (cnt*(cnt+1))
    // cnt = cnt+2
    decNumberMinus (&term, &term, set);
    decNumberMultiply (&term, &term, &x, set);
    decNumberMultiply (&term, &term, &x, set);
    decNumberDivide   (&term, &term, &cnt, set);
    decNumberAdd (&cnt, &cnt, &one, set);
    decNumberDivide   (&term, &term, &cnt, set);
    decNumberAdd (&cnt, &cnt, &one, set);
    // sum = sum + term
    decNumberAdd (result, result, &term, set);
  }
  if (negate) {
    decNumberMinus (result, result, set);
  }
  return result;
} /* decNumberSin  */

decNumber* decNumberCos (decNumber *result, decNumber *y, decContext *set)
{
  decNumber pi, pi2, zero, one, two, x, cnt, term, cmp;
  int i;
  int negate = 0;

  decNumberFromString (&zero,"0", set);
  decNumberFromString (&one, "1", set);
  decNumberFromString (&two, "2", set);
  decNumberFromString (&pi,  PI , set);

  // Copy the argument y, so we can modify it.
  decNumberCopy (&x, y);
  // cos -y = cos y
  /*if (decCompare (&x, &zero) < 0) {*/
  if (decNumberIsNegative (&x)) {
    decNumberMinus (&x, &x, set);
  }
  // We now have x >= 0
  decNumberMultiply (&pi2, &pi, &two, set); // pi2 = 2*pi
  decNumberMod (&x, &x, &pi2, set);
  // We now have 0 <= x < 2*pi
  /*if (decCompare (&x, &pi) >= 0) {*/
  decNumberCompare (&cmp, &x, &pi, set);
  if (!decNumberIsNegative (&cmp)) {
    // x >= pi
    decNumberSubtract (&x, &pi2, &x, set);
  }
  // We now have 0 <= x < pi
  decNumberDivide (&pi2, &pi, &two, set); // pi2 = pi/2
  /*if (decCompare (&x, &pi2) >= 0) {*/
  decNumberCompare (&cmp, &x, &pi2, set);
  if (!decNumberIsNegative (&cmp)) {
    // x >= pi/2, so let x = pi-x
    decNumberSubtract (&x, &pi, &x, set);
    negate = 1;
  }
  // We now have 0 <= x <= pi/2.

  //             x^2   x^4   x^6
  // cos x = 1 - --- + --- - --- + ...
  //              2     24   720
  //
  // term(0) = 1
  // term(i) = - term(i-1) * x^2 / ((2*i-1)*(2*i))
  decNumberCopy (&cnt, &one);
  decNumberCopy (&term, &one);
  decNumberCopy (result, &one);
  // DECNUMDIGITS+3 terms are enough to achieve the required precision.
  for (i=0; i<DECNUMDIGITS+3; i++) {
    // term = -term * x^2 / (cnt*(cnt+1))
    // cnt = cnt+2
    decNumberMinus (&term, &term, set);
    decNumberMultiply (&term, &term, &x, set);
    decNumberMultiply (&term, &term, &x, set);
    decNumberDivide   (&term, &term, &cnt, set);
    decNumberAdd (&cnt, &cnt, &one, set);
    decNumberDivide   (&term, &term, &cnt, set);
    decNumberAdd (&cnt, &cnt, &one, set);
    // sum = sum + term
    decNumberAdd (result, result, &term, set);
  }
  if (negate) {
    decNumberMinus (result, result, set);
  }
  return result;
} /* decNumberCos  */

decNumber* decNumberTan (decNumber *result, decNumber *y, decContext *set)
{
  // tan x = sin x / cos x
  decNumber denominator;

  decNumberSin (result, y, set);
  decNumberCos (&denominator, y, set);
  if (decNumberIsZero (&denominator))
    decNumberFromString (result, "NaN", set);
  else
    decNumberDivide (result, result, &denominator, set);
  return result;
} /* decNumberTan  */

decNumber* decNumberAtan (decNumber *result, decNumber *x, decContext *set)
{
  //                 x^3   x^5   x^7
  // arctan(x) = x - --- + --- - --- + ...
  //                  3     5     7
  //
  // This power series works well, if x is close to zero (|x|<0.5).
  // If x is larger, the series converges too slowly,
  // so in order to get a smaller x, we apply the identity
  //
  //                      sqrt(1+x^2) - 1
  // arctan(x) = 2*arctan ---------------
  //                             x
  //
  // twice. The first application gives us a new x with x < 1.
  // The second application gives us a new x with x < 0.4142136.
  // For that x, we use the power series and multiply the result by four.

  decNumber f, g, mx2, one, two, term;
  int i;

  decNumberFromString (&one, "1", set);
  decNumberFromString (&two, "2", set);

  if (decNumberIsZero (x)) {
    decNumberCopy (result, x);
    return result;
  }

  for (i=0; i<2; i++) {
    decNumber y;
    decNumberMultiply (&y, x, x, set);     // y = x^2
    decNumberAdd (&y, &y, &one, set);      // y = y+1
    decNumberSquareRoot (&y, &y, set);     // y = sqrt(y)
    decNumberSubtract (&y, &y, &one, set); // y = y-1
    decNumberDivide (x, &y, x, set);       // x = y/x
  }
  // f(0) = x
  // f(i) = f(i-1) * (-x^2)
  //
  // g(0) = 1
  // g(i) = g(i-1) + 2
  //
  // term(i) = f(i) / g(i)
  decNumberCopy (&f, x);     // f(0) = x
  decNumberCopy (&g, &one);  // g(0) = 1
  decNumberCopy (&term, x);  // term = x
  decNumberCopy (result, x); // sum  = x
  decNumberMultiply (&mx2, x, x, set); // mx2 = x^2
  decNumberMinus (&mx2, &mx2, set);    // mx2 = -x^2
  // Since x is less than sqrt(2)-1 = 0.4142...,
  // each term is smaller than the previous term by a factor of about 6,
  // so two iterations are more than enough to increase the precision
  // by one digit.
  // 2*DECNUMDIGITS terms are enough to achieve the required precision.
  for (i=0; i<2*DECNUMDIGITS; i++) {
    // f = f * (-x^2)
    decNumberMultiply (&f, &f, &mx2, set);
    // g = g+2
    decNumberAdd (&g, &g, &two, set);
    // term = f/g
    decNumberDivide (&term, &f, &g, set);
    // sum = sum + term
    decNumberAdd (result, result, &term, set);
  }
  // Multiply result by four.
  decNumberAdd (result, result, result, set);
  decNumberAdd (result, result, result, set);
  return result;
} /* decNumberAtan  */
