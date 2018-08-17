/*
 * (C) 2011-2018 by Christian Hesse <mail@eworm.de>
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

/* systemd headers */
#ifdef HAVE_SYSTEMD
#include <systemd/sd-daemon.h>
#endif

#include <iniparser.h>
#include <libnotify/notify.h>
#include <mpd/client.h>

#ifdef HAVE_LIBAV
#include <libavformat/avformat.h>
#include <magic.h>
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

/**
 * Update the notification object stored. Adds elapsed time it needed, otherwise
 * adds total time.
 *
 * \param music_dir The directory used by mpd
 * \param image_scale The maximum side size of the picture used in the
 *  notification. The aspect ratio between the two side is kept.
 * \param notificaiton_image_workaround If positive, the notification image
 *  will be passed to the notification system as a path to a png file,
 *  otherwise it will be passed as a GdkPixbuf.
 * \return 1 if the notification as been succesfully updated, 0 otherwise.
 */
int update_notification(int show_elapsed_time);

/*** main ***/
int main(int argc, char ** argv);

#endif /* MPD_NOTIFICATION_H */
