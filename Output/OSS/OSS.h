/*  XMMS - Cross-platform multimedia player
 *  Copyright (C) 1998-2003  Peter Alm, Mikael Alm, Olle Hallnas,
 *                           Thomas Nilsson and 4Front Technologies
 *  Copyright (C) 1999-2003  Haavard Kvaalen
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */
#ifndef OSS_H
#define OSS_H

#include "config.h"

#include "libxmms/gtk.h"

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/time.h>

#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef __FreeBSD__
#include <sys/soundcard.h>
#else
#include "soundcard.h"
#endif

#ifndef SNDCTL_DSP_GETPLAYVOL
#define SNDCTL_DSP_GETPLAYVOL	SOUND_MIXER_READ_PCM
#endif

#ifndef SNDCTL_DSP_SETPLAYVOL
#define SNDCTL_DSP_SETPLAYVOL	SOUND_MIXER_WRITE_PCM
#endif

#include "xmms/plugin.h"
#include "libxmms/configfile.h"

#ifdef WORDS_BIGENDIAN
# define IS_BIG_ENDIAN TRUE
#else
# define IS_BIG_ENDIAN FALSE
#endif

extern OutputPlugin op;

typedef struct
{
	gint audio_device;
	gint mixer_device;
	gint buffer_size;
	gint prebuffer;
	gboolean use_master;
	gboolean use_alt_audio_device, use_alt_mixer_device;
	gchar *alt_audio_device, *alt_mixer_device;
}
OSSConfig;

extern OSSConfig oss_cfg;

void oss_init(void);
void oss_about(void);
void oss_configure(void);

void oss_get_volume(int *l, int *r);
void oss_set_volume(int l, int r);

int oss_playing(void);
int oss_free(void);
void oss_write(void *ptr, int length);
void oss_close(void);
void oss_flush(int time);
void oss_pause(short p);
int oss_open(AFormat fmt, int rate, int nch);
int oss_get_output_time(void);
int oss_get_written_time(void);
void oss_set_audio_params(void);

int oss_get_fd(void);

void oss_free_convert_buffer(void);
int (*oss_get_convert_func(int output, int input))(void **, int);
int (*oss_get_stereo_convert_func(int output, int input))(void **, int, int);

#endif
