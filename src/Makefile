#
# File Makefile part of ProgAndSciCalc
#
# Copyright (C) 2018 Ken Bromham
#
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#

# set to either 2 (build for GTK2) or 3 (build for GTK3)
GTK_VERSION = 2

PROG = progandscicalc

SRCS = main.c gui.c gui_menu.c display.c display_widget.c \
       calc.c calc_integer.c calc_float.c calc_util.c config.c \
       display_print.c gui_menu_options.c gui_menu_help.c \
       gui_menu_conversion.c calc_conversion.c \
       gui_menu_constants.c gui_history.c gui_util.c
		

HDRS = gui.h gui_internal.h display.h display_widget.h calc.h \
       calc_internal.h calc_types.h config.h display_print.h \
       calc_conversion.h gui_util.h

# place all build output under this directory
BUILD_DIR = build

OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(SRCS))


# decNumber files
# TODO decNumber header dependencies.
# For now, if you change anything in DN_DIR, do a make clean
DN_DIR = decNumber
DN_SRCS_BARE = decContext.c decQuad.c decNumber.c \
               decimal128.c decimal64.c decNumberMath.c
DN_SRCS = $(patsubst %, $(DN_DIR)/%, $(DN_SRCS_BARE))

DN_OBJS = $(patsubst %.c, $(BUILD_DIR)/%.o, $(DN_SRCS))


# eg. built program will be found under build/progandscicalc
PROG_TARGET = $(BUILD_DIR)/$(PROG)

CC       = gcc
CPPFLAGS = -DTARGET_GTK_VERSION=$(GTK_VERSION)
CFLAGS   = -std=c99 -O2 -Wall -Wextra -Wmissing-prototypes -fwrapv -Wno-deprecated-declarations
LDFLAGS  =

##############################################################################

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(BUILD_DIR)/$(DN_DIR)
	$(CC) -c $(CPPFLAGS) $(CFLAGS) `pkg-config --cflags gtk+-$(GTK_VERSION).0` $< -o $@

##############################################################################

all:    $(PROG_TARGET)


$(PROG_TARGET): $(OBJS) $(DN_OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(DN_OBJS) `pkg-config --libs gtk+-$(GTK_VERSION).0` -lm -o $@


# will rebuild everything if any header is changed, that'll do
$(OBJS):  $(HDRS)

clean:
	rm -f $(BUILD_DIR)/*.o
	rm -f $(BUILD_DIR)/$(DN_DIR)/*.o
	rm -f $(BUILD_DIR)/$(PROG)


.PHONY: all clean

