vtclock: vtclock.c font0.h font1.h font2.h
	cc -o vtclock vtclock.c -lncurses
install: vtclock
	sudo cp vtclock /usr/local/bin/vt220clock
.PHONY: install
