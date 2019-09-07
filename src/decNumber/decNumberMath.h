/* Advanced decNumber functions

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

#ifndef _DECNUMBERMATH_H
#define _DECNUMBERMATH_H


/* Hyperbolic Functions */
extern decNumber* decNumberSinh (decNumber *, decNumber *, decContext *);
extern decNumber* decNumberCosh (decNumber *, decNumber *, decContext *);
extern decNumber* decNumberTanh (decNumber *, decNumber *, decContext *);

/* Trigonometric Functions */
extern decNumber* decNumberSin (decNumber *, decNumber *, decContext *);
extern decNumber* decNumberCos (decNumber *, decNumber *, decContext *);
extern decNumber* decNumberTan (decNumber *, decNumber *, decContext *);
extern decNumber* decNumberAtan (decNumber *, decNumber *, decContext *);

#endif /* _DECNUMBERMATH_H  */
