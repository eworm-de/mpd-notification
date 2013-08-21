# mpd-notification - Notify about tracks played by mpd

CC	:= gcc
MD	:= markdown
INSTALL	:= install
CP	:= cp
RM	:= rm
CFLAGS	+= -O2 -Wall -Werror
CFLAGS	+= $(shell pkg-config --cflags --libs libmpdclient) \
	   $(shell pkg-config --cflags --libs libnotify)
VERSION	= $(shell git describe --tags --long)

all: mpd-notification README.html

mpd-notification: mpd-notification.c config.h
	$(CC) $(CFLAGS) -o mpd-notification mpd-notification.c \
		-DVERSION="\"$(VERSION)\""

config.h:
	$(CP) config.def.h config.h

README.html: README.md
	$(MD) README.md > README.html

install: install-bin install-doc

install-bin: mpd-notification
	$(INSTALL) -D -m0755 mpd-notification $(DESTDIR)/usr/bin/mpd-notification
	$(INSTALL) -D -m0644 mpd-notification.desktop $(DESTDIR)/etc/xdg/autostart/mpd-notification.desktop

install-doc: README.html
	$(INSTALL) -D -m0644 README.md $(DESTDIR)/usr/share/doc/mpd-notification/README.md
	$(INSTALL) -D -m0644 README.html $(DESTDIR)/usr/share/doc/mpd-notification/README.html
	$(INSTALL) -D -m0644 screenshot.png $(DESTDIR)/usr/share/doc/mpd-notification/screenshot.png

clean:
	$(RM) -f *.o *~ README.html mpd-notification
