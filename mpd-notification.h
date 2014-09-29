/*
 * (C) 2014 by Christian Hesse <mail@eworm.de>
 *
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 */

#ifndef MPD_NOTIFICATION_H
#define MPD_NOTIFICATION_H

#include <mpd/client.h>

#include <libnotify/notify.h>

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

/*** get_icon ***/
char * get_icon(const char * music_dir, const char * uri);

/*** main ***/
int main(int argc, char ** argv);

#endif /* MPD_NOTIFICATION_H */
