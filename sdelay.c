/*
 * Schroeder's reverb
 * sdelay.c
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

#include "sdelay.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*---------------------------------------------------
 *---------------------------------------------------*/
sdelay_allpass_t *sdelay_allpass_create(int n_taps, float gain){
  sdelay_allpass_t *a;
  a = (sdelay_allpass_t *)malloc(sizeof(sdelay_allpass_t));
  memset(a, 0, sizeof(sdelay_allpass_t));
  a->tap = audiobuf_create_default(n_taps);
  a->gain = gain;
  return a;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
void sdelay_allpass_destroy(sdelay_allpass_t *a){
  audiobuf_destroy(a->tap);
  free(a);
}

/*---------------------------------------------------
 *---------------------------------------------------*/
static void sdelay_allpass_process(sdelay_allpass_t *a, float l, float r){
  float g = a->gain;
  float dl,dr;
  dl = audiobuf_get_L(a->tap);
  dr = audiobuf_get_R(a->tap);
  audiobuf_write_L(a->tap, l+dl*g);
  audiobuf_write_R(a->tap, r+dr*g);
  a->current_output_l = dl - (l+dl*g)*g;
  a->current_output_r = dr - (r+dr*g)*g;
  audiobuf_wrap(a->tap);
}

static int tapdefs[] = {1301, 687, 335};
static float gaindefs[] = {0.7, 0.7, 0.7};

/*---------------------------------------------------
 *---------------------------------------------------*/
sdelay_t *sdelay_create(){
  sdelay_t *s;
  int i;
  s = (sdelay_t *)malloc(sizeof(sdelay_t));
  memset(s, 0, sizeof(sdelay_t));
  for(i = 0 ; i < SDELAY_NUM_ALLPASS; i ++){
    s->apfilter[i] = sdelay_allpass_create(tapdefs[i], gaindefs[i]);
  }
  s->dry = 0.0;
  s->wet = 1.0;
  return s;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
void sdelay_destroy(sdelay_t *s){
  int i;
  for(i = 0 ; i < SDELAY_NUM_ALLPASS; i ++){
    sdelay_allpass_destroy(s->apfilter[i]);
  }
  free(s);
}

/*---------------------------------------------------
 * all pass filter
 *---------------------------------------------------*/
static void sdelay_process(sdelay_t *s, float l, float r){
  int i;
  sdelay_allpass_process(s->apfilter[0], l, r);
  for(i = 1; i < SDELAY_NUM_ALLPASS; i++){
    sdelay_allpass_process(s->apfilter[i],
			   s->apfilter[i-1]->current_output_l,
			   s->apfilter[i-1]->current_output_r);
  }
}

/*---------------------------------------------------
 *---------------------------------------------------*/
static float sdelay_current_output_l(sdelay_t *s){
  return s->apfilter[SDELAY_NUM_ALLPASS-1]->current_output_l;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
static float sdelay_current_output_r(sdelay_t *s){
  return s->apfilter[SDELAY_NUM_ALLPASS-1]->current_output_r;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
void sdelay_set_drywet(sdelay_t *s, float dry, float wet){
  s->dry = dry;
  s->wet = wet;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
void sdelay_render(sdelay_t *s, audiobuf_t *a, int start, int size){
  int i;
  float il, ir;
  float ol, or;
  audiobuf_seek(a, start);
  for(i = 0; i < size; i ++){
    il = audiobuf_get_L(a);
    ir = audiobuf_get_R(a);
    sdelay_process(s, il, ir);
    ol = sdelay_current_output_l(s);
    or = sdelay_current_output_r(s);
    audiobuf_write_L(a, ol*s->wet + il*s->dry);
    audiobuf_write_R(a, or*s->wet + ir*s->dry);
  }
}
