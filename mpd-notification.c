/*
 * (C) 2011-2013 by Christian Hesse <mail@eworm.de>
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

#include "config.h"

#define PROGNAME	"mpd-notification"

#define NOTIFICATION_TIMEOUT	10000
#ifndef DEBUG
#define DEBUG	0
#endif

int main(int argc, char ** argv) {
	char * album = NULL, * artist = NULL, * notifystr = NULL, * title = NULL;
	GError * error = NULL;
	unsigned short int errcount = 0, state = MPD_STATE_UNKNOWN;
	NotifyNotification * notification = NULL;
	struct mpd_connection * conn = NULL;
	struct mpd_song * song = NULL;

	printf("%s: %s v%s (compiled: " __DATE__ ", " __TIME__
#if DEBUG
			", with debug output"
#endif
			")\n", argv[0], PROGNAME, VERSION);

	conn = mpd_connection_new(NULL, 0, 30000);

	if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS) {
		fprintf(stderr,"%s: %s\n", argv[0], mpd_connection_get_error_message(conn));
		mpd_connection_free(conn);
		exit(EXIT_FAILURE);
	}

	if(!notify_init(PROGNAME)) {
		fprintf(stderr, "%s: Can't create notify.\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	notification =
#if NOTIFY_CHECK_VERSION(0, 7, 0)
		notify_notification_new(TEXT_TOPIC, NULL, ICON_SOUND);
#else
		notify_notification_new(TEXT_TOPIC, NULL, ICON_SOUND, NULL);
#endif
	notify_notification_set_category(notification, PROGNAME);
	notify_notification_set_urgency (notification, NOTIFY_URGENCY_NORMAL);

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
				title = TEXT_UNKNOWN;
			if ((artist = g_markup_escape_text(mpd_song_get_tag(song, MPD_TAG_ARTIST, 0), -1)) == NULL)
				artist = TEXT_UNKNOWN;
			if ((album = g_markup_escape_text(mpd_song_get_tag(song, MPD_TAG_ALBUM, 0), -1)) == NULL)
				album = TEXT_UNKNOWN;

			notifystr = malloc(sizeof(TEXT_PLAY) + strlen(title) + strlen(artist) + strlen(album));
			sprintf(notifystr, TEXT_PLAY, title, artist, album);

			mpd_song_free(song);
		} else if (state == MPD_STATE_PAUSE)
			notifystr = TEXT_PAUSE;
		else if (state == MPD_STATE_STOP)
			notifystr = TEXT_STOP;
		else
			notifystr = TEXT_UNKNOWN;

#if DEBUG
		printf("%s: %s\n", argv[0], notifystr);
#endif

		notify_notification_update(notification, TEXT_TOPIC, notifystr, ICON_SOUND);

		notify_notification_set_timeout(notification, NOTIFICATION_TIMEOUT);

		while(!notify_notification_show(notification, &error)) {
			if (errcount > 1) {
				fprintf(stderr, "%s: Looks like we can not reconnect to notification daemon... Exiting.\n", argv[0]);
				exit(EXIT_FAILURE);
			} else {
				g_printerr("%s: Error \"%s\" while trying to show notification. Trying to reconnect.\n", argv[0], error->message);
				errcount++;

				g_error_free(error);
				error = NULL;

				notify_uninit();

				usleep(500 * 1000);

				if(!notify_init(PROGNAME)) {
					fprintf(stderr, "%s: Can't create notify.\n", argv[0]);
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

	return EXIT_SUCCESS;
}
