/*
 * (C) 2011-2016 by Christian Hesse <mail@eworm.de>
 *
 * This software may be used and distributed according to the terms
 * of the GNU General Public License, incorporated herein by reference.
 */

#include "mpd-notification.h"

const static char optstring[] = "hH:m:p:t:vV";
const static struct option options_long[] = {
	/* name		has_arg			flag	val */
	{ "help",	no_argument,		NULL,	'h' },
	{ "host",	required_argument,	NULL,	'H' },
	{ "music-dir",	required_argument,	NULL,	'm' },
	{ "port",	required_argument,	NULL,	'p' },
	{ "timeout",	required_argument,	NULL,	't' },
	{ "verbose",	no_argument,		NULL,	'v' },
	{ "version",	no_argument,		NULL,	'V' },
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

#ifdef HAVE_LIBAV
/*** retrieve_album_art ***/
GdkPixbuf * retrieve_album_art(const char * music_dir, const char * uri) {
	int i;
	AVPacket pkt;
	AVFormatContext * pFormatCtx;
	GdkPixbufLoader * loader;
	GdkPixbuf * pixbuf = NULL;
	char * uri_path = NULL;

	/* try album artwork first */
	uri_path = malloc(strlen(music_dir) + strlen(uri) + 2);
	sprintf(uri_path, "%s/%s", music_dir, uri);

	pFormatCtx = avformat_alloc_context();

	if (avformat_open_input(&pFormatCtx, uri_path, NULL, NULL) != 0) {
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

			loader = gdk_pixbuf_loader_new();
			gdk_pixbuf_loader_write(loader, pkt.data, pkt.size, NULL);
			pixbuf = gdk_pixbuf_loader_get_pixbuf(loader);

			gdk_pixbuf_loader_close(loader, NULL);
			break;
		}
	}

fail:
	avformat_close_input(&pFormatCtx);
	avformat_free_context(pFormatCtx);

	if (uri_path)
		free(uri_path);

	return pixbuf;
}
#endif

/*** get_icon ***/
char * get_icon(const char * music_dir, const char * uri) {
	char * icon = NULL, * uri_dirname = NULL;
	DIR * dir;
	struct dirent * entry;
	regex_t regex;

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

	if (uri_dirname)
		free(uri_dirname);

	return icon;
}

/*** append_string ***/
char * append_string(char * string, const char * format, const char * s) {
	char * tmp;

	tmp = g_markup_escape_text(s, -1);

	string = realloc(string, strlen(string) + strlen(format) + strlen(tmp));

	sprintf(string + strlen(string), format, tmp);

	free(tmp);

	return string;
}

/*** main ***/
int main(int argc, char ** argv) {
	const char * title = NULL, * artist = NULL, * album = NULL;
	char * icon = NULL, * notifystr = NULL;
	GdkPixbuf * pixbuf = NULL;
	GError * error = NULL;
	unsigned short int errcount = 0, state = MPD_STATE_UNKNOWN;
	const char * mpd_host, * mpd_port_str, * music_dir, * uri = NULL;
	unsigned mpd_port = MPD_PORT, mpd_timeout = MPD_TIMEOUT, notification_timeout = NOTIFICATION_TIMEOUT;
	struct mpd_song * song = NULL;
	unsigned int i, version = 0, help = 0;

	program = argv[0];

	if ((mpd_host = getenv("MPD_HOST")) == NULL)
		mpd_host = MPD_HOST;

	if ((mpd_port_str = getenv("MPD_PORT")) == NULL)
		mpd_port = MPD_PORT;
	else
		mpd_port = atoi(mpd_port_str);

	music_dir = getenv("XDG_MUSIC_DIR");

	/* get the verbose status */
	while ((i = getopt_long(argc, argv, optstring, options_long, NULL)) != -1) {
		switch (i) {
			case 'h':
				help++;
				break;
			case 'v':
				verbose++;
				break;
			case 'V':
				verbose++;
				version++;
				break;
		}
	}

	/* reinitialize getopt() by resetting optind to 0 */
	optind = 0;

	/* say hello */
	if (verbose > 0)
		printf("%s: %s v%s (compiled: " __DATE__ ", " __TIME__ ")\n", program, PROGNAME, VERSION);

	if (help > 0)
		fprintf(stderr, "usage: %s [-h] [-H HOST] [-p PORT] [-m MUSIC-DIR] [-t TIMEOUT] [-v] [-V]\n", program);

	if (version > 0 || help > 0)
		return EXIT_SUCCESS;

	/* get command line options */
	while ((i = getopt_long(argc, argv, optstring, options_long, NULL)) != -1) {
		switch (i) {
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
	if (mpd_host != NULL && mpd_host[0] != '/')
		music_dir = NULL;

	/* change directory to music base directory */
	if (music_dir != NULL) {
		if (chdir(music_dir) == -1) {
			fprintf(stderr, "%s: Can not change directory to '%s'.\n", program, music_dir);
			music_dir = NULL;
		}
	}

#ifdef HAVE_LIBAV
	/* libav */
	av_register_all();

	/* only fatal messages from libav */
	if (verbose == 0)
		av_log_set_level(AV_LOG_FATAL);
#endif

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
		notify_notification_new(TEXT_TOPIC, TEXT_NONE, ICON_AUDIO_X_GENERIC);
#		else
		notify_notification_new(TEXT_TOPIC, TEXT_NONE, ICON_AUDIO_X_GENERIC, NULL);
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

			title = mpd_song_get_tag(song, MPD_TAG_TITLE, 0);

			/* ignore if we have no title */
			if (title == NULL)
				goto nonotification;

			/* initial allocation and string termination */
			notifystr = strdup("");

			notifystr = append_string(notifystr, TEXT_PLAY_TITLE, title);

			if ((artist = mpd_song_get_tag(song, MPD_TAG_ARTIST, 0)) != NULL)
				notifystr = append_string(notifystr, TEXT_PLAY_ARTIST, artist);

			if ((album = mpd_song_get_tag(song, MPD_TAG_ALBUM, 0)) != NULL)
				notifystr = append_string(notifystr, TEXT_PLAY_ALBUM, album);

			uri = mpd_song_get_uri(song);

			if (music_dir != NULL && uri != NULL) {
#ifdef HAVE_LIBAV
				pixbuf = retrieve_album_art(music_dir, uri);

				if (verbose > 0 && pixbuf != NULL)
					printf("%s: found artwork in media file: %s/%s\n", program, music_dir, uri);

				if (pixbuf == NULL)
#endif
					icon = get_icon(music_dir, uri);

				if (verbose > 0 && icon != NULL)
					printf("%s: found icon: %s\n", program, icon);
			}

			mpd_song_free(song);
		} else if (state == MPD_STATE_PAUSE)
			notifystr = strdup(TEXT_PAUSE);
		else if (state == MPD_STATE_STOP)
			notifystr = strdup(TEXT_STOP);
		else
			notifystr = strdup(TEXT_UNKNOWN);

		if (verbose > 0)
			printf("%s: %s\n", program, notifystr);

		/* What combinations do we have?
		 *   icon  pixbuf (impossible, icon is set only when !pixbuf)
		 *  !icon  pixbuf -> icon -> NULL (use pixbuf)
		 *  !icon !pixbuf -> ICON_AUDIO_X_GENERIC
		 *   icon !pixbuf -> icon */
		notify_notification_update(notification, TEXT_TOPIC, notifystr,
				icon == NULL && pixbuf == NULL ? ICON_AUDIO_X_GENERIC : icon);

		if (pixbuf != NULL)
			notify_notification_set_image_from_pixbuf(notification, pixbuf);

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

nonotification:
		if (notifystr != NULL) {
			free(notifystr);
			notifystr = NULL;
		}
		if (icon != NULL) {
			free(icon);
			icon = NULL;
		}
		if (pixbuf != NULL) {
			g_object_unref(pixbuf);
			pixbuf = NULL;
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
