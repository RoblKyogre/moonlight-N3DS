/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2015-2017 Iwan Timmer
 *
 * Moonlight is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Moonlight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Moonlight; if not, see <http://www.gnu.org/licenses/>.
 */

#include <stdbool.h>

#include <Limelight.h>

#ifdef HAVE_ALSA
extern AUDIO_RENDERER_CALLBACKS audio_callbacks_alsa;
#endif
#ifdef HAVE_SDL
extern AUDIO_RENDERER_CALLBACKS audio_callbacks_sdl;
#endif
#ifdef HAVE_PULSE
extern AUDIO_RENDERER_CALLBACKS audio_callbacks_pulse;
bool audio_pulse_init(char* audio_device);
#endif
#ifdef __FreeBSD__
extern AUDIO_RENDERER_CALLBACKS audio_callbacks_oss;
#endif
#ifdef __3DS__
extern bool n3ds_audio_disabled;
extern AUDIO_RENDERER_CALLBACKS audio_callbacks_n3ds;
#endif
