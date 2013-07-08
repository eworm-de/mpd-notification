# mpd-notification - Notify about tracks played by mpd

CC	:= gcc
INSTALL	:= install
RM	:= rm
CFLAGS	+= -O2 -Wall -Werror
CFLAGS	+= $(shell pkg-config --cflags --libs libmpdclient) \
	   $(shell pkg-config --cflags --libs libnotify)
VERSION	= $(shell git describe --tags --long)

all: mpd-notification.c
	$(CC) $(CFLAGS) -o mpd-notification mpd-notification.c \
		-DVERSION="\"$(VERSION)\""

install:
	$(INSTALL) -D -m0755 mpd-notification $(DESTDIR)/usr/bin/mpd-notification
	$(INSTALL) -D -m0644 mpd-notification.desktop $(DESTDIR)/etc/xdg/autostart/mpd-notification.desktop
clean:
	$(RM) -f *.o *~ mpd-notification
