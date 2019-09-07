/*****************************************************************************
 * File main.c part of ProgAndSciCalc
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
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <gtk/gtk.h>

#include "calc.h"
#include "gui.h"
#include "config.h"
#include "display.h"


int main(int argc, char *argv[])
{
    int debug_level = 0;

    srand((unsigned)time(NULL));

    gtk_init(&argc, &argv);

    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--debug") == 0)
            debug_level++;
    }

    config_init();

    display_init(config_get_hex_grouping());

    calc_mode_enum mode = config_get_calc_mode();
    int rand_range = config_get_random_01() ? 0 : config_get_random_n();
    bool sct_round = config_get_use_sct_rounding();
    calc_width_enum width = config_get_integer_width();
    bool int_unsigned = config_get_use_unsigned();
    bool warn_signed = config_get_warn_on_signed_overflow();
    bool warn_unsigned = config_get_warn_on_unsigned_overflow();

    calc_init(debug_level,
              mode,
              rand_range,
              sct_round,
              width,
              int_unsigned,
              warn_signed,
              warn_unsigned);

    gui_init(debug_level, config_get_float_digits());
    gui_create();

    gtk_main();

    config_save();

    return 0;
}
