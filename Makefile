prefix = /usr/local

vtclock: vtclock.c font0.h font1.h font2.h font3.h
	cc -o vtclock vtclock.c -lncurses
install: vtclock
	install -m0755 vtclock $(prefix)/bin/vtclock
tar:
	( cd .. ; tar cvvzf vtclock-`date +%Y-%m-%d`.tar.gz vtclock \
		--exclude=CVS \
                --exclude='#*#' \
                --exclude='.#*' \
                --exclude='.cvsignore' \
                --exclude='*~' \
                --exclude='.*~' \
		--exclude='vtclock/vtclock' \
		--exclude='*.o' \
	)
.PHONY: install tar
