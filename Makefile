prefix = /usr/local

vtclock: vtclock.c font0.h font1.h font2.h font3.h msg.h msg.c vtclock.h figlet.c figlet.h
	cc -o vtclock vtclock.c msg.c figlet.c -lncurses
install: vtclock
	mkdir -p $(prefix)/bin
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
		--exclude='*.d' \
	)
.PHONY: install tar

.PHONY: ChangeLog
ChangeLog: 
	cvs2cl --stdout >$@

.PHONY: clean
clean:
	/bin/rm -f vtclock *.o #*# .#* *~ .*~ *.bak

