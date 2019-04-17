# mpd-notification - Notify about tracks played by mpd

# commands
CC	:= gcc
MD	:= markdown
INSTALL	:= install
CP	:= cp
RM	:= rm

# flags
CFLAGS	+= -std=c11 -O2 -fPIC -Wall -Werror
ifneq ($(wildcard /usr/include/iniparser),)
override CFLAGS	+= -I/usr/include/iniparser
endif
override CFLAGS	+= -liniparser
CFLAGS_SYSTEMD := $(shell pkg-config --cflags --libs libsystemd 2>/dev/null)
ifneq ($(CFLAGS_SYSTEMD),)
override CFLAGS	+= -DHAVE_SYSTEMD $(CFLAGS_SYSTEMD)
endif
override CFLAGS	+= $(shell pkg-config --cflags --libs libmpdclient)
override CFLAGS	+= $(shell pkg-config --cflags --libs libnotify)
override CFLAGS_LIBAV := $(shell pkg-config --cflags --libs libavformat libavutil 2>/dev/null)
ifneq ($(CFLAGS_LIBAV),)
override CFLAGS	+= -DHAVE_LIBAV $(CFLAGS_LIBAV)
override CFLAGS	+= -lmagic
endif
override LDFLAGS	+= -Wl,-z,now -Wl,-z,relro -pie

# this is just a fallback in case you do not use git but downloaded
# a release tarball...
VERSION := 0.8.5

all: mpd-notification README.html

mpd-notification: mpd-notification.c mpd-notification.h config.h version.h
	$(CC) $(CFLAGS) $(LDFLAGS) -o mpd-notification mpd-notification.c

config.h:
	$(CP) config.def.h config.h

version.h: $(wildcard .git/HEAD .git/index .git/refs/tags/*) Makefile
	printf "#ifndef VERSION\n#define VERSION \"%s\"\n#endif\n" $(shell git describe --long 2>/dev/null || echo ${VERSION}) > $@

README.html: README.md
	$(MD) README.md > README.html

install: install-bin install-doc

install-bin: mpd-notification
	$(INSTALL) -D -m0755 mpd-notification $(DESTDIR)/usr/bin/mpd-notification
	$(INSTALL) -D -m0644 systemd/mpd-notification.service $(DESTDIR)/usr/lib/systemd/user/mpd-notification.service

install-doc: README.html
	$(INSTALL) -D -m0644 README.md $(DESTDIR)/usr/share/doc/mpd-notification/README.md
	$(INSTALL) -D -m0644 README.html $(DESTDIR)/usr/share/doc/mpd-notification/README.html
	$(INSTALL) -D -m0644 screenshots/sound.png $(DESTDIR)/usr/share/doc/mpd-notification/screenshots/sound.png
	$(INSTALL) -D -m0644 screenshots/artwork.png $(DESTDIR)/usr/share/doc/mpd-notification/screenshots/artwork.png

clean:
	$(RM) -f *.o *~ README.html mpd-notification version.h

distclean:
	$(RM) -f *.o *~ README.html mpd-notification version.h config.h

release:
	git archive --format=tar.xz --prefix=mpd-notification-$(VERSION)/ $(VERSION) > mpd-notification-$(VERSION).tar.xz
	gpg --armor --detach-sign --comment mpd-notification-$(VERSION).tar.xz mpd-notification-$(VERSION).tar.xz
	git notes --ref=refs/notes/signatures/tar add -C $$(git archive --format=tar --prefix=mpd-notification-$(VERSION)/ $(VERSION) | gpg --armor --detach-sign --comment mpd-notification-$(VERSION).tar | git hash-object -w --stdin) $(VERSION)
