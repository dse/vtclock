vtclock: vtclock.c font0.h font1.h font2.h font3.h
	cc -o vtclock vtclock.c -lncurses
install: vtclock
	sudo cp vtclock /usr/local/bin/vt220clock
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
