#!/bin/sh
#
gcc -Wall -Wextra -O2 -std=c99 -o test_display_print test_display_print.c display_print.c decNumber/decContext.c decNumber/decQuad.c 
