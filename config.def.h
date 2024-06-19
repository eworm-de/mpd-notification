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

#ifndef _CONFIG_H
#define _CONFIG_H

/* how long to display notifications */
#define NOTIFICATION_TIMEOUT	10

/* name of the icon used for notifications */
#define ICON_AUDIO_X_GENERIC	"audio-x-generic"

/* strings used to display notification messages
 * TEXT_PLAY & TEXT_PAUSE can include several specifiers:
 * %t for title, %a for artist and %A for album */
#define TEXT_TOPIC	"MPD Notification"
#define TEXT_PLAY	"Playing <b>%t</b>\nby <i>%a</i>\nfrom <i>%A</i>"
#define TEXT_PAUSE	"Paused <b>%t</b>\nby <i>%a</i>\nfrom <i>%A</i>"
#define TEXT_STOP	"Stopped playback"
#define TEXT_NONE	"No action received yet"
#define TEXT_UNKNOWN	"(unknown)"

/* this is a regular expression that has to match image filename used
 * for artwork */
#define REGEX_ARTWORK	"\\(folder\\|cover\\)\\.\\(jpg\\|png\\)"

/* how to connect to mpd host ?
 * MPD_HOST is the server's host name, IP address or Unix socket path. If the
 * resolver returns more than one IP address for a host name, this functions
 * tries all of them until one accepts the connection. NULL is allowed here,
 * which will connect to the default host.
 * MPD_PORT is the TCP port to connect to, 0 for default port. If "host" is
 * a Unix socket path, this parameter is ignored.
 * MPD_TIMEOUT is the timeout in milliseconds, 0 for the default timeout. */
#define MPD_HOST	NULL
#define MPD_PORT	0
#define MPD_TIMEOUT	0

#endif /* _CONFIG_H */

// vim: set syntax=c:
