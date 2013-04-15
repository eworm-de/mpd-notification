# mpd-notification - Notify about tracks played by mpd

CC	:= gcc
CFLAGS	+= $(shell pkg-config --cflags --libs libmpdclient) \
	   $(shell pkg-config --cflags --libs libnotify)
VERSION	= $(shell git describe --tags --long)

all: mpd-notification.c
	$(CC) $(CFLAGS) -o mpd-notification mpd-notification.c \
		-DVERSION="\"$(VERSION)\""

clean:
	/bin/rm -f *.o *~ mpd-notification
