/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2015 Iwan Timmer
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

#include "video.h"
#include "ffmpeg.h"

#include "../sdl_main.h"
#include "../util.h"

#ifdef __3DS__
#include <SDL3/SDL.h>
#include <SDL3/SDL_thread.h>
#else
#include <SDL.h>
#include <SDL_thread.h>
#endif

#include <unistd.h>
#include <stdbool.h>

#define SLICES_PER_FRAME 4

static void* ffmpeg_buffer;
static size_t ffmpeg_buffer_size;

static int sdl_setup(int videoFormat, int width, int height, int redrawRate, void* context, int drFlags) {
  if (ffmpeg_init(videoFormat, width, height, SLICE_THREADING, SDL_BUFFER_FRAMES, SLICES_PER_FRAME) < 0) {
    fprintf(stderr, "Couldn't initialize video decoding\n");
    return -1;
  }

  ensure_buf_size(&ffmpeg_buffer, &ffmpeg_buffer_size, INITIAL_DECODER_BUFFER_SIZE + AV_INPUT_BUFFER_PADDING_SIZE);

  return 0;
}

static void sdl_cleanup() {
  ffmpeg_destroy();
}

static uint64_t sdl_submit_decode_unit_avgTime;
static uint64_t avgLoopCount = 0;

uint64_t get_sdl_submit_decode_unit_avgTime() {
    return sdl_submit_decode_unit_avgTime;
}

static int sdl_submit_decode_unit(PDECODE_UNIT decodeUnit) {
  uint64_t loopTimeStart = PltGetMillis();

  PLENTRY entry = decodeUnit->bufferList;
  int length = 0;

  ensure_buf_size(&ffmpeg_buffer, &ffmpeg_buffer_size, decodeUnit->fullLength + AV_INPUT_BUFFER_PADDING_SIZE);

  while (entry != NULL) {
    memcpy(ffmpeg_buffer+length, entry->data, entry->length);
    length += entry->length;
    entry = entry->next;
  }
  ffmpeg_decode(ffmpeg_buffer, length);

  SDL_LockMutex(mutex);
  AVFrame* frame = ffmpeg_get_frame(false);
  if (frame != NULL) {
    sdlNextFrame++;

    SDL_Event event;
    event.type = SDL_EVENT_USER;
    event.user.code = SDL_CODE_FRAME;
    event.user.data1 = &frame->data;
    event.user.data2 = &frame->linesize;
    SDL_PushEvent(&event);
  }
  SDL_UnlockMutex(mutex);

  uint64_t loopTimeElapsed = PltGetMillis() - loopTimeStart;
  if (avgLoopCount < 1) {
      sdl_submit_decode_unit_avgTime = loopTimeElapsed;
      avgLoopCount++;
  }
  else {
      sdl_submit_decode_unit_avgTime = ((sdl_submit_decode_unit_avgTime * avgLoopCount) + loopTimeElapsed) / (avgLoopCount + 1);
      if (avgLoopCount < 1000) {
          avgLoopCount++;
      }
  }
  return DR_OK;
}

DECODER_RENDERER_CALLBACKS decoder_callbacks_sdl = {
  .setup = sdl_setup,
  .cleanup = sdl_cleanup,
  .submitDecodeUnit = sdl_submit_decode_unit,
  .capabilities = CAPABILITY_SLICES_PER_FRAME(SLICES_PER_FRAME) | CAPABILITY_REFERENCE_FRAME_INVALIDATION_HEVC | CAPABILITY_DIRECT_SUBMIT,
};
