# vtclock - a text-mode giant digital clock
# Copyright (C) 2016 Darren Embry.  
# 
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307$

SRCS = vtclock.c msg.c figlet.c
# HDRS = font0.h font1.h font2.h font3.h msg.h vtclock.h figlet.h
program = vtclock
OBJS = $(SRCS:.c=.o)
PKGCONFIG_PKGS = ncurses
manpage = 

VERSION = $(shell sed -n '/^\#define[ 	][ 	]*VTCLOCK_VERSION[ 	][ 	]*"/{s/^[^"]*"//;s/".*$$//;p;}' vtclock.h)
DEBPKGS = pkg-config libncurses5-dev gcc
TAR = $(shell if which gtar >/dev/null 2>/dev/null ; then echo gtar ; else echo tar ; fi)

###############################################################################

SHELL = /bin/sh

.SUFFIXES:
.SUFFIXES: .c .o .d
ifdef manpage
.SUFFIXES: .1 .1.txt .1.ps .1.html .ps .pdf
endif

EXTRA_CFLAGS =
EXTRA_LIBS   = 

PKGCONFIG_CFLAGS = `pkg-config --cflags $(PKGCONFIG_PKGS)`
PKGCONFIG_LIBS   = `pkg-config --libs   $(PKGCONFIG_PKGS)`

ifeq (0,$(shell pkg-config --cflags ncurses >/dev/null 2>/dev/null))
else
  PKGCONFIG_CFLAGS =
  PKGCONFIG_LIBS   =
  EXTRA_CFLAGS =
  EXTRA_LIBS = -lncurses
endif

# Common prefix for installation directories.
# NOTE: This directory must exist when you start the install.
prefix = /usr/local
datarootdir = $(prefix)/share
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
mandir = $(datarootdir)/man

DESTDIR =

INSTALL = /usr/bin/install
INSTALL_PROGRAM = $(INSTALL)
INSTALL_DATA    = $(INSTALL) -m 0644
INSTALL_MKDIR   = $(INSTALL) -d

.PHONY: all
all: $(program)

$(program): $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(PKGCONFIG_LIBS) $(EXTRA_LIBS) $(CFLAGS)

%.o: %.c
	$(CC) -c $(CPPFLAGS) $(PKGCONFIG_CFLAGS) $(EXTRA_CFLAGS) $(CFLAGS) $<

%.d: %.c
	@ >&2 echo Fixing dependencies for $< . . .
	$(CC) -MM $(CPPFLAGS) $(PKGCONFIG_CFLAGS) $(CFLAGS) $< | \
		sed 's/^$*\.o/& $@/g' > $@ || true

-include $(SRCS:.c=.d)

.PHONY: install
install: $(program)
	$(INSTALL_MKDIR) $(DESTDIR)$(bindir)
	$(INSTALL_PROGRAM) $(program) $(DESTDIR)$(bindir)
    ifdef manpage
	$(INSTALL_MKDIR) $(DESTDIR)$(mandir)/man1
	$(INSTALL_DATA) $(manpage) $(DESTDIR)$(mandir)/man1
    endif

.PHONY: uninstall
uninstall:
	rm $(DESTDIR)$(bindir)/$(program)
    ifdef manpage
	rm $(DESTDIR)$(mandir)/man1/$(manpage)
    endif

DIST = $(program)-$(VERSION).tar.gz

.PHONY: dist
dist:
	$(TAR) cvvzf $(DIST) \
		--transform='s:^:$(program)-$(VERSION)/:' \
		--show-transformed-names \
		--exclude=perl-version \
		--exclude=MyMakefile \
		--exclude=CVS \
		--exclude=.svn \
		--exclude=.git \
		--exclude=test \
		--exclude='*.rej' \
                --exclude='#*#' \
                --exclude='.#*' \
                --exclude='.cvsignore' \
                --exclude='.svnignore' \
                --exclude='.gitignore' \
                --exclude='*~' \
                --exclude='.*~' \
		--exclude='.deps' \
		--exclude='*.1.*' \
		--exclude=$(program) \
		--exclude=TAGS \
		--exclude='*.o' \
		--exclude='*.d' \
		--exclude='*.tar.gz' \
		--exclude='*.tmp.*' \
		--exclude='*.tmp' \
		--exclude='*.log' \
		*

.PHONY: clean
clean:
	2>/dev/null rm $(program) *.d *.o '#'*'#' .'#'* *~ .*~ *.bak core || true

.PHONY: version
version:
	@echo $(VERSION)

###############################################################################
# Dependencies

%.d: %.c
	@ >&2 echo Fixing dependencies for $< . . .
	@ $(CC) -MM $(CPPFLAGS) $< | sed 's/^$*\.o/& $@/g' > $@ || true

-include $(SRCS:.c=.d)

###############################################################################
# Documentation

%.1.txt: %.1 Makefile
	nroff -man $< | col -bx > $@.tmp
	mv $@.tmp $@
%.1.ps: %.1 Makefile
	groff -Tps -man $< >$@.tmp
	mv $@.tmp $@
%.1.html: %.1 Makefile
	groff -Thtml -man $< >$@.tmp
	mv $@.tmp $@
%.pdf: %.ps Makefile
	ps2pdf $< $@.tmp.pdf
	mv $@.tmp.pdf $@

###############################################################################
# Debian

.PHONY: install-debian-prerequisites
install-debian-prerequisites:
	sudo apt-get install $(DEBPKGS)


###############################################################################
# My makefile customizations about which you need not be concerned :-)

-include MyMakefile

