/*
 * Wavetable Synthesizer
 *
 * wtssynth.h
 *
 *
 * Copyright (c) 2013 sada.gussy (sada dot gussy at gmail dot com)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#ifndef MIDIFILE_H
#define MIDIFILE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "midievent.h"

#define MIDIFILE_EVENT_MAX 100000
#define MIDIPARAM_STRING_CHAR_MAX 129

/*---------------------------------------------------
 *---------------------------------------------------*/
typedef struct {
  unsigned int header_length;
  unsigned short format;
  unsigned short num_tracks;
  unsigned short deltatime;
  unsigned int track_length;
  unsigned int current_time;
  unsigned char command;
  unsigned char last_command; /* running status */
  unsigned char channel;
  unsigned char last_channel; /* running status */
  char author[129];
  char title[129];
} midiparam_t;


/*---------------------------------------------------
 * main structure
 *---------------------------------------------------*/
typedef struct {
  int size;
  midievent_t **events;
  int current_index;
  int record_index;
  midiparam_t midiparam;
  int tempo;
  int current_ticks;
  int verbose;
} midifile_t;

/*---------------------------------------------------
 *---------------------------------------------------*/
midifile_t *midifile_create(int size);
#define midifile_create_default() midifile_create(MIDIFILE_EVENT_MAX)
void midifile_set_verbose(midifile_t *mf, int level);
void midifile_destroy(midifile_t *mf, int deep);

void midifile_rewind(midifile_t *mf);

/*---------------------------------------------------
 * load a .mid file 
 *---------------------------------------------------*/
int midifile_load(midifile_t *mf, const char *filename, int sampling_rate);

/*---------------------------------------------------
 * return 1 if midifile has midi events to be processed.
 *---------------------------------------------------*/
int midifile_has_events(midifile_t *mf);

/*---------------------------------------------------
 * return ticks of the next midi event from current time
 * if midifile has a midi event to be processed, return 0.
 *---------------------------------------------------*/
int midifile_get_nextevent_time(midifile_t *mf);

/*---------------------------------------------------
 * proceed time
 *---------------------------------------------------*/
void midifile_ticks_proceed(midifile_t *mf, int ticks);

/*---------------------------------------------------
 * check current midi event,
 *---------------------------------------------------*/
midievent_t *midifile_current_event(midifile_t *mf);
void midifile_next_event(midifile_t *mf);

int midifile_has_waiting_event(midifile_t *mf);
void midifile_dump(midifile_t *mf);

#ifdef __cplusplus
}
#endif
int midifile_progress(midifile_t *mf);

#endif
