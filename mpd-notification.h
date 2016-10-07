/*
 * (C) 2011-2016 by Christian Hesse <mail@eworm.de>
 *
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 */

#ifndef MPD_NOTIFICATION_H
#define MPD_NOTIFICATION_H

#define _GNU_SOURCE

#include <getopt.h>
#include <regex.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <iniparser.h>
#ifdef HAVE_LIBAV
#include <libavformat/avformat.h>
#endif
#include <libnotify/notify.h>
#include <mpd/client.h>

#include "config.h"
#include "version.h"

#define PROGNAME	"mpd-notification"

#define OPT_FILE_WORKAROUND UCHAR_MAX + 1

/*** received_signal ***/
void received_signal(int signal);

/*** retrieve_artwork ***/
GdkPixbuf * retrieve_artwork(const char * music_dir, const char * uri);

/*** append_string ***/
char * append_string(char * string, const char * format, const char delim, const char * s);

/*** main ***/
int main(int argc, char ** argv);

#endif /* MPD_NOTIFICATION_H */
