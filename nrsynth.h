/*
 * noise rhythm Synthesizer
 *
 * nrsynth.h
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

#ifndef NRSYNTH_H
#define NRSYNTH_H

#ifdef __cplusplus
extern "C" {
#endif

#include "audiobuf.h"
#include "midievent.h"
#include "midievent.h"

/*---------------------------------------------------
 * defines and constants
 *---------------------------------------------------*/

#define NRSYNTH_INST_MAX 4
#define NRSYNTH_NUM_PRESETS 4

#define NRSYNTH_KICKDRUM 0
#define NRSYNTH_SNAREDRUM 1

#define NRSYNTH_VOLUME_SCALING 0.25

#define NRSYNTH_PANPOT_DEFAULT 0.5
#define NRSYNTH_VOLUME_DEFAULT 1.0
#define NRSYNTH_EXPRESSION_DEFAULT 1.0

enum{
  NRSYNTH_INST_UNUSED = 0,
  NRSYNTH_INST_PLAYING,
};

/*---------------------------------------------------
 * instrument structure
 * also acts as a voice
 *---------------------------------------------------*/

typedef struct {
  int status;
  unsigned int elapsed;
  float velocity;
  unsigned int attack_time;
  unsigned int sustain_time;
  unsigned int release_time;
  int pitchmode; /* 0 highest, 1 middle, 2 lowest pitch */
  float rnd;
  int key; /* is not used for playing but for display*/
} nrsynth_inst_t;

/*---------------------------------------------------
 * main structure
 *---------------------------------------------------*/

typedef struct {
  nrsynth_inst_t inst[NRSYNTH_NUM_PRESETS];
  int keymap[128];
  float expression;
  float panpot;
  float volume;
  float volume_total_l;
  float volume_total_r;
} nrsynth_t;


/*---------------------------------------------------
 * constructor
 *
 *---------------------------------------------------*/
nrsynth_t *nrsynth_create();

/*---------------------------------------------------
 * destructor
 *
 *---------------------------------------------------*/
void nrsynth_destroy(nrsynth_t *n);

/*---------------------------------------------------
 * init
 *
 *---------------------------------------------------*/
void nrsynth_init(nrsynth_t *n);


/*---------------------------------------------------
 * This plays a rhythm
 * n: 
 * key: 0 to 127, each assigns instrument number.
 * velocity: 0 to 1.0
 *---------------------------------------------------*/
void nrsynth_play(nrsynth_t *n, int key, float velocity);

/*---------------------------------------------------
 * renders data to audiobuf
 *---------------------------------------------------*/
void nrsynth_render(nrsynth_t *n, audiobuf_t *a, int start, int size);

/*---------------------------------------------------
 * control this synth via midi event
 *---------------------------------------------------*/
void nrsynth_midi(nrsynth_t *n, midievent_t *e);


int nrsynth_get_current_key(nrsynth_t *n, int index);



#ifdef __cplusplus
}
#endif


#endif
