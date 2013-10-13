/*
 * Schreoder's reverb
 *
 * sdelay.h
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


#ifndef SDELAY_H
#define SDELAY_H

#ifdef __cplusplus
extern "C" {
#endif

#include "audiobuf.h"

/*---------------------------------------------------
 * all pass filter
 *---------------------------------------------------*/
typedef struct {
  audiobuf_t *tap;
  float gain;
  float current_output_l;
  float current_output_r;
} sdelay_allpass_t;

#define SDELAY_NUM_ALLPASS 3
#define SDELAY_NUM_COMB 2

/*---------------------------------------------------
 * main structure
 *---------------------------------------------------*/
typedef struct {
  sdelay_allpass_t *apfilter[SDELAY_NUM_ALLPASS];
  /*  sdelay_comb_t cmfilter[SDELAY_NUM_COMB]; */ /* will implement someday. */
  float gain;
  float dry;
  float wet;
  float current_output_l;
  float current_output_r;
} sdelay_t;

/*---------------------------------------------------
 * cons,dest
 *---------------------------------------------------*/
sdelay_t *sdelay_create();
void sdelay_destroy(sdelay_t *s);

/*---------------------------------------------------
 * renders waves
 *---------------------------------------------------*/
void sdelay_render(sdelay_t *s, audiobuf_t *a, int start, int size);

/*---------------------------------------------------
 * set each ammount of dry and wet from 0 to 1.
 *---------------------------------------------------*/
void sdelay_set_drywet(sdelay_t *s, float dry, float wet );


#ifdef __cplusplus
}
#endif

#endif
