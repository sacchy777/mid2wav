/*
 * Sound Module
 *
 * soundmodule.h
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

#ifndef SOUNDMODULE_H
#define SOUNDMODULE_H

#include "audiobuf.h"
#include "midifile.h"

#ifdef __cplusplus
extern "C" {
#endif

enum{
  SOUNDMODULE_CHAMBER_TYPE_WTS = 0,
  SOUNDMODULE_CHAMBER_TYPE_NR = 1,
};

/*---------------------------------------------------
 * chamber
 *---------------------------------------------------*/
typedef struct {
  int type;
  void *synth;
  int is_muted;
} soundmodule_chamber_t;

#define SOUNDMODULE_CHAMBER_MAX 16


/*---------------------------------------------------
 * main structure
 *---------------------------------------------------*/
typedef struct {
  soundmodule_chamber_t chambers[SOUNDMODULE_CHAMBER_MAX];
} soundmodule_t;


/*---------------------------------------------------
 *---------------------------------------------------*/
soundmodule_t *soundmodule_create();
void soundmodule_destroy(soundmodule_t *s);

/*---------------------------------------------------
 *---------------------------------------------------*/
void soundmodule_render(soundmodule_t *s, audiobuf_t *a, midifile_t *mf);

/*---------------------------------------------------
 *---------------------------------------------------*/
void soundmodule_mute(soundmodule_t *s, int channel);
int soundmodule_is_muted(soundmodule_t *s, int channel);
void soundmodule_unmute(soundmodule_t *s, int channel);
void soundmodule_solo(soundmodule_t *s, int channel);
void soundmodule_clear_mute(soundmodule_t *s);



void soundmodule_midi_key(soundmodule_t *s, int midi_key);

int soundmodule_get_current_key(soundmodule_t *s, int channel, int voice_index);
int soundmodule_is_key_offing(soundmodule_t *s, int channel, int voice_index);
int soundmodule_get_program(soundmodule_t *s, int channel);



#ifdef __cplusplus
}
#endif


#endif
