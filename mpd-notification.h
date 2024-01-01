/*
 * (C) 2011-2024 by Christian Hesse <mail@eworm.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
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

/* systemd headers */
#ifdef HAVE_SYSTEMD
#include <systemd/sd-daemon.h>
#endif

#include <iniparser.h>
#include <libnotify/notify.h>
#include <mpd/client.h>

#ifdef HAVE_LIBAV
#include <libavformat/avformat.h>
#endif

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
