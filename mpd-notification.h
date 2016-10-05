/*
 * (C) 2011-2016 by Christian Hesse <mail@eworm.de>
 *
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 */

#ifndef MPD_NOTIFICATION_H
#define MPD_NOTIFICATION_H

#define _GNU_SOURCE

#include <mpd/client.h>

#include <libnotify/notify.h>

#ifdef HAVE_LIBAV
#include <libavformat/avformat.h>
#endif

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>
#include <regex.h>

#include "config.h"
#include "version.h"

#define PROGNAME	"mpd-notification"

/*** received_signal ***/
void received_signal(int signal);

/*** retrieve_artwork_media ***/
#ifdef HAVE_LIBAV
GdkPixbuf * retrieve_artwork_media(const char * music_dir, const char * uri);
#endif

/*** retrieve_artwork_image ***/
GdkPixbuf * retrieve_artwork_image(const char * music_dir, const char * uri);

/*** append_string ***/
char * append_string(char * string, const char * format, const char delim, const char * s);

/*** main ***/
int main(int argc, char ** argv);

#endif /* MPD_NOTIFICATION_H */
