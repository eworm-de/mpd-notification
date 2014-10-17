# mpd-notification - Notify about tracks played by mpd

CC	:= gcc -std=c11
MD	:= markdown
INSTALL	:= install
CP	:= cp
RM	:= rm
CFLAGS	+= -O2 -Wall -Werror
CFLAGS	+= $(shell pkg-config --cflags --libs libmpdclient)
CFLAGS	+= $(shell pkg-config --cflags --libs libnotify)
# this is just a fallback in case you do not use git but downloaded
# a release tarball...
VERSION := 0.5.2

all: mpd-notification README.html

mpd-notification: mpd-notification.c config.h version.h
	$(CC) $(CFLAGS) -o mpd-notification mpd-notification.c

config.h:
	$(CP) config.def.h config.h

version.h: $(wildcard .git/HEAD .git/index .git/refs/tags/*) Makefile
	echo "#ifndef VERSION" > $@
	echo "#define VERSION \"$(shell git describe --tags --long 2>/dev/null || echo ${VERSION})\"" >> $@
	echo "#endif" >> $@

README.html: README.md
	$(MD) README.md > README.html

install: install-bin install-doc

install-bin: mpd-notification
	$(INSTALL) -D -m0755 mpd-notification $(DESTDIR)/usr/bin/mpd-notification
	$(INSTALL) -D -m0644 mpd-notification.desktop $(DESTDIR)/etc/xdg/autostart/mpd-notification.desktop

install-doc: README.html
	$(INSTALL) -D -m0644 README.md $(DESTDIR)/usr/share/doc/mpd-notification/README.md
	$(INSTALL) -D -m0644 README.html $(DESTDIR)/usr/share/doc/mpd-notification/README.html
	$(INSTALL) -D -m0644 screenshot-sound.png $(DESTDIR)/usr/share/doc/mpd-notification/screenshot-sound.png
	$(INSTALL) -D -m0644 screenshot-artwork.png $(DESTDIR)/usr/share/doc/mpd-notification/screenshot-artwork.png

clean:
	$(RM) -f *.o *~ README.html mpd-notification version.h

distclean:
	$(RM) -f *.o *~ README.html mpd-notification version.h config.h

release:
	git archive --format=tar.xz --prefix=mpd-notification-$(VERSION)/ $(VERSION) > mpd-notification-$(VERSION).tar.xz
	gpg -ab mpd-notification-$(VERSION).tar.xz
