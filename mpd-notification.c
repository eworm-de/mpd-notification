/*
 * (C) 2011-2014 by Christian Hesse <mail@eworm.de>
 *
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 */

#include <mpd/client.h>

#include <libnotify/notify.h>

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <getopt.h>

#include "config.h"
#include "version.h"

#define PROGNAME	"mpd-notification"

#define NOTIFICATION_TIMEOUT	10000
#ifndef DEBUG
#define DEBUG	0
#endif

const static char optstring[] = "hH:p:";
const static struct option options_long[] = {
	/* name		has_arg			flag	val */
	{ "help",	no_argument,		NULL,	'h' },
	{ "host",	required_argument,	NULL,	'H' },
	{ "port",	required_argument,	NULL,	'p' },
	{ 0, 0, 0, 0 }
};

/* global variables */
char *program = NULL;
NotifyNotification * notification = NULL;

/*** show_again ***/
void show_again(int signal) {
	GError * error = NULL;

	if (!notify_notification_show(notification, &error)) {
		g_printerr("%s: Error \"%s\" while trying to show notification again.\n", program, error->message);
		g_error_free(error);
	}
}

/*** main ***/
int main(int argc, char ** argv) {
	char * album = NULL, * artist = NULL, * notifystr = NULL, * title = NULL;
	GError * error = NULL;
	unsigned short int errcount = 0, state = MPD_STATE_UNKNOWN;
	const char * mpd_host = MPD_HOST;
	unsigned mpd_port = MPD_PORT, mpd_timeout = MPD_TIMEOUT;
	struct mpd_connection * conn = NULL;
	struct mpd_song * song = NULL;
	unsigned int i;

	program = argv[0];

	printf("%s: %s v%s (compiled: " __DATE__ ", " __TIME__
#			if DEBUG
			", with debug output"
#			endif
			")\n", program, PROGNAME, VERSION);

#	if DEBUG
	printf("%s: Started with PID %d\n", program, getpid());
#	endif

	/* get command line options */
	while ((i = getopt_long(argc, argv, optstring, options_long, NULL)) != -1)
		switch (i) {
			case 'h':
				fprintf(stderr, "usage: %s [-h] [-H HOST] [-p PORT]\n", program);
				return EXIT_SUCCESS;
			case 'p':
				mpd_port = atoi(optarg);
				printf("%s: using port %d\n", program, mpd_port);
				break;
			case 'H':
				mpd_host = optarg;
				printf("%s: using host %s\n", program, mpd_host);
				break;
		}

	conn = mpd_connection_new(mpd_host, mpd_port, mpd_timeout);

	if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS) {
		fprintf(stderr,"%s: %s\n", program, mpd_connection_get_error_message(conn));
		mpd_connection_free(conn);
		exit(EXIT_FAILURE);
	}

	if(!notify_init(PROGNAME)) {
		fprintf(stderr, "%s: Can't create notify.\n", program);
		exit(EXIT_FAILURE);
	}

	notification =
#		if NOTIFY_CHECK_VERSION(0, 7, 0)
		notify_notification_new(TEXT_TOPIC, TEXT_NONE, ICON_SOUND);
#		else
		notify_notification_new(TEXT_TOPIC, TEXT_NONE, ICON_SOUND, NULL);
#		endif
	notify_notification_set_category(notification, PROGNAME);
	notify_notification_set_urgency (notification, NOTIFY_URGENCY_NORMAL);

	signal(SIGUSR1, show_again);

	while(mpd_run_idle_mask(conn, MPD_IDLE_PLAYER)) {
		mpd_command_list_begin(conn, true);
		mpd_send_status(conn);
		mpd_send_current_song(conn);
		mpd_command_list_end(conn);

		state = mpd_status_get_state(mpd_recv_status(conn));
		if (state == MPD_STATE_PLAY) {
			mpd_response_next(conn);

			song = mpd_recv_song(conn);

			if ((title = g_markup_escape_text(mpd_song_get_tag(song, MPD_TAG_TITLE, 0), -1)) == NULL)
				title = strdup(TEXT_UNKNOWN);
			if ((artist = g_markup_escape_text(mpd_song_get_tag(song, MPD_TAG_ARTIST, 0), -1)) == NULL)
				artist = strdup(TEXT_UNKNOWN);
			if ((album = g_markup_escape_text(mpd_song_get_tag(song, MPD_TAG_ALBUM, 0), -1)) == NULL)
				album = strdup(TEXT_UNKNOWN);

			notifystr = malloc(sizeof(TEXT_PLAY) + strlen(title) + strlen(artist) + strlen(album));
			sprintf(notifystr, TEXT_PLAY, title, artist, album);

			free(title);
			free(artist);
			free(album);

			mpd_song_free(song);
		} else if (state == MPD_STATE_PAUSE)
			notifystr = TEXT_PAUSE;
		else if (state == MPD_STATE_STOP)
			notifystr = TEXT_STOP;
		else
			notifystr = TEXT_UNKNOWN;

#		if DEBUG
		printf("%s: %s\n", program, notifystr);
#		endif

		notify_notification_update(notification, TEXT_TOPIC, notifystr, ICON_SOUND);

		notify_notification_set_timeout(notification, NOTIFICATION_TIMEOUT);

		while(!notify_notification_show(notification, &error)) {
			if (errcount > 1) {
				fprintf(stderr, "%s: Looks like we can not reconnect to notification daemon... Exiting.\n", program);
				exit(EXIT_FAILURE);
			} else {
				g_printerr("%s: Error \"%s\" while trying to show notification. Trying to reconnect.\n", program, error->message);
				errcount++;

				g_error_free(error);
				error = NULL;

				notify_uninit();

				usleep(500 * 1000);

				if(!notify_init(PROGNAME)) {
					fprintf(stderr, "%s: Can't create notify.\n", program);
					exit(EXIT_FAILURE);
				}
			}
		}
		errcount = 0;

		if (state == MPD_STATE_PLAY)
			free(notifystr);
		mpd_response_finish(conn);
	}
	mpd_connection_free(conn);

	g_object_unref(G_OBJECT(notification));
	notify_uninit();

	return EXIT_SUCCESS;
}
