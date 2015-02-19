/*
 * noise rhythm Synthesizer 
 *
 * nrsynth.c
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

/*
 * noise rhythm synth
*/
#include "nrsynth.h"
#include "xor128.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*---------------------------------------------------
 * noise rhythm parameters
 * at : attack time in ticks
 * st : sustain time in ticks
 * rt : release time in ticks
 * mode : 0 high, 1 middle, 2 low
 *---------------------------------------------------*/
static const nrsynth_inst_t nrsynth_presets[NRSYNTH_NUM_PRESETS] = {
  /* dummy dummy dummy at st rt mode dummy */
  {0, 0, 0, 0,  100, 2000, 2, 0, }, /* 0 kick */
  {0, 0, 0, 0,    0, 2000, 1, 0, }, /* 1 snare */
  {0, 0, 0, 0,    0, 1000, 0, 0, }, /* 2 closed hh */
  {0, 0, 0, 0, 1000, 3000, 0, 0, }, /* 3 open hh */
};

/*---------------------------------------------------
 * midi note number to tone conversion map
 * -1 means tone is not specified.
 *---------------------------------------------------*/
static const int nrsynth_keymap_default[128] = {
  -1, -1, -1, -1, -1, -1, -1, -1, /* 0..7 */
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1,  0,  0, -1,  1, -1, /* 32..39 */
   1, -1,  2, -1, -1, -1,  3, -1, /* 40..47 */
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, /* 64..71 */
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1, /* 96..103 */
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
  -1, -1, -1, -1, -1, -1, -1, -1,
};

/*---------------------------------------------------
 * internal function.
 * This updates volume values for pre calculation.
 * need to call this if below param changes
 *---------------------------------------------------*/
static void nrsynth_update_volume(nrsynth_t *n){
  n->volume_total_l = n->expression * n->volume * n->panpot * NRSYNTH_VOLUME_SCALING;
  n->volume_total_r = n->expression * n->volume * (1.0 - n->panpot) * NRSYNTH_VOLUME_SCALING;
}


/*---------------------------------------------------
 *---------------------------------------------------*/
nrsynth_t *nrsynth_create(){
  nrsynth_t *n;
  n = (nrsynth_t *)malloc(sizeof(nrsynth_t));
  memset(n, 0, sizeof(nrsynth_t));
  nrsynth_init(n);
  return n;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
void nrsynth_set_instparam(nrsynth_t *n, int inst, int at, int st, int rt, int pm){
  nrsynth_inst_t *ni;
  ni = &n->inst[inst];
  ni -> attack_time = at;
  ni -> sustain_time = st;
  ni -> release_time = rt;
  ni -> pitchmode = pm;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
void nrsynth_init(nrsynth_t *n){
  memcpy(n->inst, nrsynth_presets, sizeof(nrsynth_inst_t) * NRSYNTH_NUM_PRESETS);
  memcpy(n->keymap, nrsynth_keymap_default, sizeof(int)*128);
  n->expression = NRSYNTH_EXPRESSION_DEFAULT;
  n->panpot = NRSYNTH_PANPOT_DEFAULT;
  n->volume = NRSYNTH_VOLUME_DEFAULT;
  nrsynth_update_volume(n);
}




/*---------------------------------------------------
 *---------------------------------------------------*/
void nrsynth_destroy(nrsynth_t *n){
  free(n);
}


/*---------------------------------------------------
 *---------------------------------------------------*/
void nrsynth_play(nrsynth_t *n, int key, float velocity){
  int inst_index = n->keymap[key];
    if(inst_index != -1){ /* do nothing if instrument is not assigned */
    nrsynth_inst_t *ni = &n->inst[inst_index];
    ni->key = key;
    ni->status = NRSYNTH_INST_PLAYING;
    ni->elapsed = 0;
    ni->velocity = velocity;
  }
}

/*---------------------------------------------------
 * internal funcion.
 *
 * This returns adsr value. nrsynth's ADSR is a 3
 * stage ADSR. 
 *---------------------------------------------------*/
static float nrsynth_inst_adsr(nrsynth_inst_t *ni){
  /* attack */
  if(ni->elapsed < ni->attack_time){
    return (float)ni->elapsed / (float)ni->attack_time;
  /* sustain */
  }else if(ni->elapsed < ni->attack_time + ni->sustain_time){
    return 1.0;
  /* release */
  }else if(ni->elapsed < ni->attack_time + ni->sustain_time + ni->release_time){
    return (float)(ni->release_time - (ni->elapsed - (ni->attack_time + ni->sustain_time))) / (float)ni->release_time;
  /* key off */
  }else{
    ni->status = NRSYNTH_INST_UNUSED;
    return 0.0;
  }
}


/*---------------------------------------------------
 * internal funcion.
 *
 * This returns wave value. 
 *---------------------------------------------------*/
static float nrsynth_inst_getwave(nrsynth_inst_t *ni){
  return 
    ni->rnd *
    nrsynth_inst_adsr(ni) *
    ni->velocity;
}


/*---------------------------------------------------
 * internal funcion.
 *
 * This proceeds instrument's time pointer
 * random value is generated here by xor128 based on pitchmode .
 * The pitchmode smaller, noise frequency higher
 *---------------------------------------------------*/
static void nrsynth_inst_next(nrsynth_inst_t *ni){
  ni->elapsed ++;
  switch(ni->pitchmode){
  case 0:
    ni->rnd = (float)xor128()/(float)0x80000000 - 1.0;
    break;
  case 1:
    if((ni->elapsed & 0x1) == 0)
      ni->rnd = (float)xor128()/(float)0x80000000 - 1.0;
    break;
  case 2:
    if((ni->elapsed & 0x7) == 0)
      ni->rnd = (float)xor128()/(float)0x80000000 - 1.0;
    break;
  }
}

/*---------------------------------------------------
 *---------------------------------------------------*/
void nrsynth_render(nrsynth_t *n, audiobuf_t *a, int start, int size){
  int i,j;
  float f;
  nrsynth_inst_t *ni;
  float l,r;
  float vl,vr;
  vl =n->volume_total_l;
  vr =n->volume_total_r;
  for(i = 0; i < NRSYNTH_INST_MAX; i ++){
    ni = &n->inst[i];
    if(ni->status == NRSYNTH_INST_UNUSED) continue;
    audiobuf_seek(a, start);
    for(j = 0; j < size; j ++){
      f = nrsynth_inst_getwave(ni);
      l = f * vl;
      r = f * vr;
      audiobuf_add_L(a, l);
      audiobuf_add_R(a, r);
      nrsynth_inst_next(ni);
    }
  }
}

/*---------------------------------------------------
 *---------------------------------------------------*/
void nrsynth_midi(nrsynth_t *n, midievent_t *e){
  switch(e->type){
  case MIDIEVENT_TYPE_NOTEOFF:
    break;
  case MIDIEVENT_TYPE_NOTEON:
    if(e->shortparam[1] != 0){
      nrsynth_play(n, e->shortparam[0], (float)e->shortparam[1]*(float)e->shortparam[1]/(127.0*127.0));
    }
    break;
  case MIDIEVENT_TYPE_KEYPRESSURE:
    break;
  case MIDIEVENT_TYPE_CONTROLCHANGE:
    switch(e->shortparam[0]){
    case MIDIEVENT_CC_VOLUME:
      n->volume = (float)e->shortparam[1]/127.0;
      nrsynth_update_volume(n);
      break;
    case MIDIEVENT_CC_PANPOT:
      n->panpot = (float)e->shortparam[1]/127.0;
      nrsynth_update_volume(n);
      break;
    case MIDIEVENT_CC_EXPRESSION:
      n->expression = (float)e->shortparam[1]/127.0;
      nrsynth_update_volume(n);
      break;
    default:
      ;
    }
    break;
  case MIDIEVENT_TYPE_PROGRAMCHANGE:
    break;
  case MIDIEVENT_TYPE_CHANNELPRESSURE:
    break;
  case MIDIEVENT_TYPE_PITCHBENDCHANGE:
    break;
  default:
    ;
  }

}
/*---------------------------------------------------
 *---------------------------------------------------*/
int nrsynth_get_current_key(nrsynth_t *n, int index){
  if(index < 0 || index >= NRSYNTH_INST_MAX){
    return -1;
  }
  nrsynth_inst_t *ni = &n->inst[index];
  if(ni->status == NRSYNTH_INST_UNUSED) return -1;
  return ni->key;
}

