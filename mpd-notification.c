/*
 * (C) 2011-2015 by Christian Hesse <mail@eworm.de>
 *
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 */

#include "mpd-notification.h"

const static char optstring[] = "hH:m:p:t:v";
const static struct option options_long[] = {
	/* name		has_arg			flag	val */
	{ "help",	no_argument,		NULL,	'h' },
	{ "host",	required_argument,	NULL,	'H' },
	{ "music-dir",	required_argument,	NULL,	'm' },
	{ "port",	required_argument,	NULL,	'p' },
	{ "notification-timeout",	required_argument,	NULL,	't' },
	{ "verbose",	no_argument,		NULL,	'v' },
	{ 0, 0, 0, 0 }
};

/* global variables */
char *program = NULL;
NotifyNotification * notification = NULL;
struct mpd_connection * conn = NULL;
uint8_t doexit = 0;
uint8_t verbose = 0;

/*** received_signal ***/
void received_signal(int signal) {
	GError * error = NULL;

	switch (signal) {
		case SIGINT:
		case SIGTERM:
			if (verbose > 0)
				printf("Received signal %s, preparing exit.\n", strsignal(signal));

			doexit++;
			mpd_send_noidle(conn);
			break;

		case SIGHUP:
		case SIGUSR1:
			if (verbose > 0)
				printf("Received signal %s, showing last notification again.\n", strsignal(signal));

			if (notify_notification_show(notification, &error) == FALSE) {
				g_printerr("%s: Error \"%s\" while trying to show notification again.\n", program, error->message);
				g_error_free(error);
			}
			break;
		default:
			fprintf(stderr, "Reveived signal %s (%d), no idea what to do...\n", strsignal(signal), signal);
	}
}

/*** retrieve_album_art ***/
char * retrieve_album_art(const char *path) {
	int i, ret = 0;
	FILE * album_art;
	char * album_art_file = NULL;
	AVPacket pkt;
	AVFormatContext * pFormatCtx = NULL;

	album_art_file = malloc(strlen(PROGNAME) + 15);
	sprintf(album_art_file, "/tmp/.%s-%d", PROGNAME, getpid());

	pFormatCtx = avformat_alloc_context();

	if (avformat_open_input(&pFormatCtx, path, NULL, NULL) != 0) {
		printf("avformat_open_input() failed");
		goto fail;
	}

	/* only mp3 file contain artwork, so ignore others */
	if (strcmp(pFormatCtx->iformat->name, "mp3") != 0)
		goto fail;

	if (pFormatCtx->iformat->read_header(pFormatCtx) < 0) {
		printf("could not read the format header\n");
		goto fail;
	}

	/* find the first attached picture, if available */
	for (i = 0; i < pFormatCtx->nb_streams; i++) {
		if (pFormatCtx->streams[i]->disposition & AV_DISPOSITION_ATTACHED_PIC) {
			pkt = pFormatCtx->streams[i]->attached_pic;
			album_art = fopen(album_art_file, "wb");
			ret = fwrite(pkt.data, pkt.size, 1, album_art);
			fclose(album_art);
			break;
		}
	}

fail:
	if (pFormatCtx)
		avformat_free_context(pFormatCtx);

	if (ret) {
		return album_art_file;
	} else {
		free(album_art_file);
		return NULL;
	}
}

/*** get_icon ***/
char * get_icon(const char * music_dir, const char * uri) {
	char * icon = NULL, * uri_path = NULL, * uri_dirname = NULL;
	DIR * dir;
	struct dirent * entry;
	regex_t regex;

	/* try album artwork first */
	uri_path = malloc(strlen(music_dir) + strlen(uri) + 2);
	sprintf(uri_path, "%s/%s", music_dir, uri);
	icon = retrieve_album_art(uri_path);

	if (icon != NULL)
		goto found;

	uri_dirname = strdup(uri);

	/* cut the dirname or just use "." (string, not char!) for current directory */
	if (strrchr(uri_dirname, '/') != NULL)
		*strrchr(uri_dirname, '/') = 0;
	else
		strcpy(uri_dirname, ".");

	if ((dir = opendir(uri_dirname)) == NULL) {
		fprintf(stderr, "%s: Can not read directory '%s': ", program, uri_dirname);
		return NULL;
	}

	if (regcomp(&regex, REGEX_ARTWORK, REG_NOSUB + REG_ICASE) != 0) {
		fprintf(stderr, "%s: Could not compile regex\n", program);
		return NULL;
	}

	while ((entry = readdir(dir))) {
		if (*entry->d_name == '.')
			continue;

		if (regexec(&regex, entry->d_name, 0, NULL, 0) == 0) {
			icon = malloc(strlen(music_dir) + strlen(uri_dirname) + strlen(entry->d_name) + 3);
			sprintf(icon, "%s/%s/%s", music_dir, uri_dirname, entry->d_name);
			break;
		}
	}

	regfree(&regex);
	closedir(dir);

found:
	if (uri_path)
		free(uri_path);
	if (uri_dirname)
		free(uri_dirname);

	return icon;
}

/*** main ***/
int main(int argc, char ** argv) {
	char * album = NULL, * artist = NULL, * icon = NULL, * notifystr = NULL, * title = NULL;
	GError * error = NULL;
	unsigned short int errcount = 0, state = MPD_STATE_UNKNOWN;
	const char * mpd_host = MPD_HOST, * music_dir = NULL, * uri = NULL;
	unsigned mpd_port = MPD_PORT, mpd_timeout = MPD_TIMEOUT, notification_timeout = NOTIFICATION_TIMEOUT;
	struct mpd_song * song = NULL;
	unsigned int i;

	program = argv[0];

	music_dir = getenv("XDG_MUSIC_DIR");

	/* get the verbose status */
	while ((i = getopt_long(argc, argv, optstring, options_long, NULL)) != -1) {
		switch (i) {
			case 'v':
				verbose++;
				break;
		}
	}

	/* reinitialize getopt() by resetting optind to 0 */
	optind = 0;

	/* say hello */
	if (verbose > 0)
		printf("%s: %s v%s (compiled: " __DATE__ ", " __TIME__ ")\n", program, PROGNAME, VERSION);

	/* get command line options */
	while ((i = getopt_long(argc, argv, optstring, options_long, NULL)) != -1) {
		switch (i) {
			case 'h':
				fprintf(stderr, "usage: %s [-h] [-H HOST] [-p PORT] [-m MUSIC-DIR] [-t NOTIFICATION-TIMEOUT] [-v]\n", program);
				return EXIT_SUCCESS;
			case 'p':
				mpd_port = atoi(optarg);
				if (verbose > 0)
					printf("%s: using port %d\n", program, mpd_port);
				break;
			case 'm':
				music_dir = optarg;
				if (verbose > 0)
					printf("%s: using music-dir %s\n", program, music_dir);
				break;
			case 'H':
				mpd_host = optarg;
				if (verbose > 0)
					printf("%s: using host %s\n", program, mpd_host);
				break;
			case 't':
				notification_timeout = atof(optarg) * 1000;
				if (verbose > 0)
					printf("%s: using notification-timeout %d\n", program, notification_timeout);
				break;
		}
	}

	/* disable artwork stuff if we are connected to a foreign host */
	if (mpd_host != NULL)
		music_dir = NULL;

	/* change directory to music base directory */
	if (music_dir != NULL) {
		if (chdir(music_dir) == -1) {
			fprintf(stderr, "%s: Can not change directory to '%s'.\n", program, music_dir);
			music_dir = NULL;
		}
	}

	/* libav */
	av_register_all();

	conn = mpd_connection_new(mpd_host, mpd_port, mpd_timeout);

	if (mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS) {
		fprintf(stderr,"%s: %s\n", program, mpd_connection_get_error_message(conn));
		mpd_connection_free(conn);
		exit(EXIT_FAILURE);
	}

	if(notify_init(PROGNAME) == FALSE) {
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

	signal(SIGHUP, received_signal);
	signal(SIGINT, received_signal);
	signal(SIGTERM, received_signal);
	signal(SIGUSR1, received_signal);

	while(doexit == 0 && mpd_run_idle_mask(conn, MPD_IDLE_PLAYER)) {
		mpd_command_list_begin(conn, true);
		mpd_send_status(conn);
		mpd_send_current_song(conn);
		mpd_command_list_end(conn);

		state = mpd_status_get_state(mpd_recv_status(conn));
		if (state == MPD_STATE_PLAY) {
			mpd_response_next(conn);

			song = mpd_recv_song(conn);

			uri = mpd_song_get_uri(song);

			if (music_dir != NULL && uri != NULL)
				icon = get_icon(music_dir, uri);

			if (verbose > 0 && icon != NULL)
				printf("%s: found icon: %s\n", program, icon);

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

		if (verbose > 0)
			printf("%s: %s\n", program, notifystr);

		notify_notification_update(notification, TEXT_TOPIC, notifystr, icon ? icon : ICON_SOUND);

		notify_notification_set_timeout(notification, notification_timeout);

		while(notify_notification_show(notification, &error) == FALSE) {
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

				if(notify_init(PROGNAME) == FALSE) {
					fprintf(stderr, "%s: Can't create notify.\n", program);
					exit(EXIT_FAILURE);
				}
			}
		}
		errcount = 0;

		if (state == MPD_STATE_PLAY)
			free(notifystr);
		if (icon != NULL) {
			free(icon);
			icon = NULL;
		}
		mpd_response_finish(conn);
	}

	if (verbose > 0)
		printf("Exiting...\n");

	mpd_connection_free(conn);

	g_object_unref(G_OBJECT(notification));
	notify_uninit();

	return EXIT_SUCCESS;
}
