/*
 * Test of 
 * Wable Table Synthesizer / Noise Rhythm synthesizer
 *
 * wtssynth_test.c
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

#include "wtssynth.h"
#include "nrsynth.h"
#include <assert.h>
#include "freq_table.h"
#include "audiobuf.h"
#include <stdio.h>
#include <stdlib.h>
#include "sdelay.h"
#include "midievent.h"
/*
 *
 *
 * gcc wtssynth_test.c wtssynth.c freq_table.c audiobuf.c xor128.c nrsynth.c
 *
 *
 */

#define BUFSIZE 44100 * 8 /* 8sec buffer */
#define TEMPFILE "test.wav"

#define VELOCITY 1.0

int g_current_time = 0;

/*---------------------------------------------------
 * plays seventh chord.
 *---------------------------------------------------*/
void seventh(wtssynth_t *w, audiobuf_t *a, int key1, int key2, int key3, int key4, int len){
  /*  play a chord */
  wtssynth_keyon(w, key1, VELOCITY);
  wtssynth_keyon(w, key2, VELOCITY);
  wtssynth_keyon(w, key3, VELOCITY);
  wtssynth_keyon(w, key4, VELOCITY);

  /* render audio data in a specified length */
  wtssynth_render(w, a, g_current_time, len);
  g_current_time += len;

  /* release a chord */
  wtssynth_keyoff(w, key1);
  wtssynth_keyoff(w, key2);
  wtssynth_keyoff(w, key3);
  wtssynth_keyoff(w, key4);
}

void seventh_midi(wtssynth_t *w, audiobuf_t *a, int key1, int key2, int key3, int key4, int len){
  midievent_t *e1, *e2, *e3, *e4, *e5, *e6, *e7, *e8;

  e1 = midievent_create_short(0, 0, MIDIEVENT_TYPE_NOTEON, key1, 100, 0, 0);
  e2 = midievent_create_short(0, 0, MIDIEVENT_TYPE_NOTEON, key2, 100, 0, 0);
  e3 = midievent_create_short(0, 0, MIDIEVENT_TYPE_NOTEON, key3, 100, 0, 0);
  e4 = midievent_create_short(0, 0, MIDIEVENT_TYPE_NOTEON, key4, 100, 0, 0);
  e5 = midievent_create_short(0, 0, MIDIEVENT_TYPE_NOTEOFF, key1, 0, 0, 0);
  e6 = midievent_create_short(0, 0, MIDIEVENT_TYPE_NOTEOFF, key2, 0, 0, 0);
  e7 = midievent_create_short(0, 0, MIDIEVENT_TYPE_NOTEOFF, key3, 0, 0, 0);
  e8 = midievent_create_short(0, 0, MIDIEVENT_TYPE_NOTEOFF, key4, 0, 0, 0);

  /*  play a chord */
  wtssynth_midi(w, e1);
  wtssynth_midi(w, e2);
  wtssynth_midi(w, e3);
  wtssynth_midi(w, e4);

  /* render audio data in a specified length */
  wtssynth_render(w, a, g_current_time, len);
  g_current_time += len;

  /* release a chord */
  wtssynth_midi(w, e5);
  wtssynth_midi(w, e6);
  wtssynth_midi(w, e7);
  wtssynth_midi(w, e8);

  midievent_destroy(e1);
  midievent_destroy(e2);
  midievent_destroy(e3);
  midievent_destroy(e4);
  midievent_destroy(e5);
  midievent_destroy(e6);
  midievent_destroy(e7);
  midievent_destroy(e8);
}

/*---------------------------------------------------
 * program main
 *---------------------------------------------------*/
int main(int argc, char *argv[]){
  int i;
  wtssynth_t *w;
  audiobuf_t *a;
  nrsynth_t *n;
  sdelay_t *s;

  /* 1sec length, equal to 2 quater notes length in BPM 120 at sampling rate 44100 */
  int delta = 44100; 

  w = wtssynth_create();
  assert(w != NULL);

  a = audiobuf_create_default(BUFSIZE);
  assert(a != NULL);

  n = nrsynth_create();
  assert(n != NULL);

  s = sdelay_create();
  assert(s != NULL);


  wtsconfig_programchange(w->config, 8);
  /*** chord rendering ***/
  g_current_time = 0; /* global variable */
  seventh_midi(w, a, key_C4, key_E5, key_G5, key_B5, delta); /* CMaj7 */
  seventh_midi(w, a, key_D4, key_F5, key_A5, key_C6, delta); /* Dm7 */
  seventh_midi(w, a, key_E4, key_G5, key_B5, key_D6, delta); /* Em7 */
  seventh_midi(w, a, key_Dp4, key_G5, key_Ap5, key_D6, delta); /* EbMaj7 */
  seventh_midi(w, a, key_D4, key_F5, key_A5, key_C6, delta*2); /* Dm7 */
  seventh_midi(w, a, key_Cp4, key_F5, key_Gp5, key_C6, delta*2); /* DbMaj7 */


  /*** rythm rendering ***/
  /* renders kick and snare */
  g_current_time = 0; 
  for(i = 0; i < 16; i++){
    nrsynth_play(n, i%2 == 0 ? 35 : 38, 1.0); /* 35=kick, 38=snare. see nrsynth_init() */
    nrsynth_render(n, a, g_current_time, BUFSIZE/16);
    g_current_time += BUFSIZE/16;
  }
  /* renders hihats */
  g_current_time = 0;
  for(i = 0; i < 32; i++){
    nrsynth_play(n, i%4 == 2 ? 46 : 42, 1.0); /* 42=closed HH, 46=open HH. see nrsynth_init() */
    nrsynth_render(n, a, g_current_time, BUFSIZE/32);
    g_current_time += BUFSIZE/32;
  }

  /*render reverb for all buffer */
  sdelay_set_drywet(s, 0.8, 0.5);
  sdelay_render(s, a, 0, BUFSIZE);

  /* write rendered audio data to a file in PCM format */
  audiobuf_save(a, TEMPFILE);

  sdelay_destroy(s);
  nrsynth_destroy(n);
  audiobuf_destroy(a);
  wtssynth_destroy(w);

  return EXIT_SUCCESS;
}
