vtclock: vtclock.c digits.h
	cc -o vtclock vtclock.c -lncurses
install: vtclock
	sudo cp vtclock /usr/local/bin/vt220clock
.PHONY: install
