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

/* ----------------------------------------------------------------------
 * This library needs:
 * audiobuf.c freq_table.c
 *
 * freq_table.c/.h can be generated automatically by freq_table.rb
 *
 * This library assmes: 
 *  (unsigned) int as 32 bit integer,
 *  (unsigned) short as 16 bit integer
 *
 * ----------------------------------------------------------------------*/

#ifndef WTSSYNTH_H
#define WTSSYNTH_H

#ifdef __cplusplus
extern "C" {
#endif

#define WTSSYNTH_VERSION "0.1"

#include "audiobuf.h"
#include "midievent.h"
#include "wtssynth.h"


/*---------------------------------------------------
 * synth's tone structure
 *---------------------------------------------------*/
#define WTSTONE_NUM_PRESETS 16

#define WTSTONE_WAVE_SHIFT 5
#define WTSTONE_WAVE_SIZE 32
#define WTSTONE_WAVE_MASK 0x3F

#define WTSCONFIG_SAMPLING_RATE_DEFAULT 44100

#define WTSADSR_ATTACK_TIME_DEFAULT (0.9 * WTSCONFIG_SAMPLING_RATE_DEFAULT)
#define WTSADSR_TOTAL_LEVEL_DEFAULT (0.3)
#define WTSADSR_DECAY_TIME_DEFAULT (0.7 * WTSCONFIG_SAMPLING_RATE_DEFAULT)
#define WTSADSR_SUSTAIN_LEVEL_DEFAULT (0.03)
#define WTSADSR_RELEASE_TIME_DEFAULT (0.1 * WTSCONFIG_SAMPLING_RATE_DEFAULT)

#define WTSCONFIG_PANPOT_DEFAULT 0.5
#define WTSCONFIG_VOLUME_DEFAULT 1.0
#define WTSCONFIG_EXPRESSION_DEFAULT 1.0
#define WTSCONFIG_MODULATION_DEFAULT 0.0
#define WTSCONFIG_PITCHBEND_DEFAULT 0.0

#define WTSCONFIG_VOLUME_SCALING 0.25 /* This number is used to reduce whole output intensity */

typedef struct {
  float wave[WTSTONE_WAVE_SIZE]; /* wave table, -1 to 1*/
  float attack_time; /* ADSR envelopes */
  float total_level;
  float decay_time;
  float sustain_level;
  float release_time;
  unsigned int attack_time_int;
  unsigned int decay_time_int;
  unsigned int release_time_int;
  unsigned int attack_decay_time_int; /* This is pre calculated value for optimization by given values aboves. */
} wtstone_t;


/*---------------------------------------------------
 * synth's config structure
 *---------------------------------------------------*/
typedef struct {
  int sampling_rate; /* 44100 for normal use */
  float panpot; /* panpot, 0(left) 0.5(center) 1(right) */
  float volume; /* total volume, default 1.0 */
  float expression; /* works as volume, default 1.0 */
  float modulation; /* not used for now. vibrato */
  float pitchbend; /* not used for now pitchbend */

  float volume_total_l; /* for reducing calculation */
  float volume_total_r; /* for reducing calculation */
  wtstone_t presets[WTSTONE_NUM_PRESETS];
  int tonemap[128];
  int current_preset;
} wtsconfig_t;

void wtsconfig_init(wtsconfig_t *c);
void wtsconfig_dump(wtsconfig_t *c);
void wtstone_init(wtstone_t *t);
wtsconfig_t *wtsconfig_create();
void wtsconfig_destroy(wtsconfig_t *c);
void wtsconfig_dump(wtsconfig_t *c);
void wtstone_dump(wtstone_t *t);
void wtsconfig_programchange(wtsconfig_t *c, int preset);

/*---------------------------------------------------
 * voice structure
 *---------------------------------------------------*/
enum{
  WTSVOICE_UNUSED = 0,
  WTSVOICE_KEYON,
  WTSVOICE_KEYOFF,
};

typedef struct {
  int status; /* one of UNUSED/KEYON/KEYOFF */
  unsigned int elapsed; /* use as a time pointer, number of sample time after keyon */
  unsigned int phase; /* phase of oscillator, 16bit fixed fraction number */
  unsigned int delta; /* delta phase in 1 sample time in 16bit fixed fraction number  */
  unsigned int keyoff_time; /* a time when key off */
  float keyoff_level;/* wave intensity when key off(adsr envelope) */
  float pitchbend; /* not used for now */
  float velocity; /* intensity of the voice */
  int key; /* key number (A5=69) */
} wtsvoice_t;

/*---------------------------------------------------
 * main structure & functions
 *---------------------------------------------------*/
#define WTSSYNTH_VOICE_MAX 16
typedef struct {
  wtsconfig_t *config; /* struct having configs */
  wtsvoice_t voice[WTSSYNTH_VOICE_MAX]; /* how many voices can be played simultaneously */
  int voiceover; /* 1 if it tries to play more than max voices */
  int midi_key; /* for key change */
} wtssynth_t;

wtssynth_t *wtssynth_create();
void wtssynth_destroy(wtssynth_t *s);
void wtssynth_init(wtssynth_t *s);

/*---------------------------------------------------
 * key-on a note by specifing key(0-127) and velocity(0-1)
 * key number is based on MIDI spec(69=A5)
 * This funciton searches an empty voice slot and assign the key.
 * If there are no empty slots, a slot having longest 
 * elapsed time is chosen to play.
 *---------------------------------------------------*/
void wtssynth_keyon(wtssynth_t *w, int key, float velocity);

/*---------------------------------------------------
 * key-off a note by specifing key(0-127)
 * key number is based on MIDI spec(69=A5)
 * This funciton searches the voice slot having same key.
 * If 2 ore more are found, the slot with longest elapsed
 * is chosend to key-off.
 * If no voice having same key found, it does nothing.
 *---------------------------------------------------*/
void wtssynth_keyoff(wtssynth_t *w, int key);

/*---------------------------------------------------
 * returns how many voices are playing
 *---------------------------------------------------*/
int wtssynth_num_key_playing(wtssynth_t *w);


/*---------------------------------------------------
 * renders wave into the audiobuf(additive)
 * overflow of the autdiobuf is not checked
 *---------------------------------------------------*/
void wtssynth_render(wtssynth_t *w, audiobuf_t *a, int start, int size);

/*---------------------------------------------------
 * control this synth via midi event
 *---------------------------------------------------*/
void wtssynth_midi(wtssynth_t *w, midievent_t *e);



void wtssynth_midi_key(wtssynth_t *w, int midi_key);

int wtssynth_get_current_key(wtssynth_t *w, int index);
int wtssynth_is_key_offing(wtssynth_t *w, int index);


int wtssynth_get_program(wtssynth_t *w);
void wtssynth_all_note_off(wtssynth_t *w);


#ifdef __cplusplus
}
#endif

#endif
