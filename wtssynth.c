/*
 * Wable Table Synthesizer 
 *
 * wtssynth.c
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
#include "audiobuf.h"
#include "freq_table.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

extern const wtstone_t wtstone_wave_presets[WTSTONE_NUM_PRESETS];

/*---------------------------------------------------
 *---------------------------------------------------*/
static void wtsconfig_precalc(wtsconfig_t *c){
  int i;
  for(i = 0 ; i < WTSTONE_NUM_PRESETS; i ++){
    wtstone_t *t = &c->presets[i];
    t->attack_time_int = t->attack_time * c->sampling_rate;
    t->decay_time_int = t->decay_time * c->sampling_rate;
    t->release_time_int = t->release_time * c->sampling_rate;
    t->attack_decay_time_int = t->attack_time_int + t->decay_time_int;
  }
}

/*---------------------------------------------------
 *---------------------------------------------------*/
wtsconfig_t *wtsconfig_create(){
  int i;
  wtsconfig_t *c;
  c = (wtsconfig_t *)malloc(sizeof(wtsconfig_t));
  wtsconfig_init(c);
  //  c->presets = (wtstone_t *)malloc(sizeof(wtstone_t)* NUM_PRESETS);
  memcpy(c->presets, wtstone_wave_presets, sizeof(wtstone_t) * WTSTONE_NUM_PRESETS);
  wtsconfig_precalc(c);
  /*
  for(i = 0 ; i < WTSTONE_NUM_PRESETS; i ++){
    wtstone_t *t = &c->presets[i];
    t->attack_decay_time = t->attack_time + t->decay_time;
  }
  */
  for(i = 0; i < 128; i ++){
    c->tonemap[i] = i / 8;
  }
  return c;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
void wtsconfig_destroy(wtsconfig_t *c){
  free(c->presets);
  free(c);
}

/*---------------------------------------------------
 *---------------------------------------------------*/
static wtstone_t *wtsconfig_gettone(wtsconfig_t *c){
  return &c->presets[c->tonemap[c->current_preset]];
}
void wtsconfig_dump(wtsconfig_t *c){
  printf("The dump of wtsconfig_t\n");
  printf("Sampling Rate : %d\n", c->sampling_rate);
  wtstone_dump(wtsconfig_gettone(c));
}


void wtsconfig_programchange(wtsconfig_t *c, int preset){
  c->current_preset = preset;
}

/*---------------------------------------------------
 * internal function.
 * This updates volume values for pre calculation.
 * need to call this if below param changes
 *---------------------------------------------------*/
static void wtsconfig_update_volume(wtsconfig_t *c){
  c->volume_total_l = c->volume * c->expression * c->panpot * WTSCONFIG_VOLUME_SCALING;
  c->volume_total_r = c->volume * c->expression * (1.0 - c->panpot) * WTSCONFIG_VOLUME_SCALING;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
void wtsconfig_init(wtsconfig_t *c){
  memset(c, 0, sizeof(wtsconfig_t));
  c->sampling_rate = WTSCONFIG_SAMPLING_RATE_DEFAULT;
  c->panpot = WTSCONFIG_PANPOT_DEFAULT;
  c->volume = WTSCONFIG_VOLUME_DEFAULT;
  c->expression = WTSCONFIG_EXPRESSION_DEFAULT;
  c->modulation = WTSCONFIG_MODULATION_DEFAULT;
  c->pitchbend = WTSCONFIG_PITCHBEND_DEFAULT;
  c->pitchbend_delta = 1.0;
  wtsconfig_update_volume(c);
  wtsconfig_programchange(c, 0);
}

/*---------------------------------------------------
 *---------------------------------------------------*/
void wtstone_init(wtstone_t *t){
  int i;
  memset(t, 0, sizeof(wtstone_t));
  for(i = 0; i < WTSTONE_WAVE_SIZE; i ++){
    t->wave[i] = i < WTSTONE_WAVE_SIZE/2 ? 1.0 : -1.0;
  }

  t->attack_time = 0.0;
  t->decay_time = 0.0;
  t->release_time = 0.0;

  t->attack_time_int = WTSADSR_ATTACK_TIME_DEFAULT;
  t->total_level = WTSADSR_TOTAL_LEVEL_DEFAULT;
  t->decay_time_int = WTSADSR_DECAY_TIME_DEFAULT;
  t->sustain_level = WTSADSR_SUSTAIN_LEVEL_DEFAULT;
  t->release_time_int = WTSADSR_RELEASE_TIME_DEFAULT;
  t->attack_decay_time_int = t->attack_time_int + t->decay_time_int;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
void wtstone_set_adsr(wtstone_t *t, float tl, float at, float dt, float sl, float rt, int sampling_rate){
  t->total_level = tl;
  t->attack_time = sampling_rate * at;
  t->decay_time = sampling_rate * dt;
  t->sustain_level = sl;
  t->release_time = sampling_rate * rt;
  t->attack_decay_time_int = t->attack_time + t->decay_time;
}
void wtstone_set_tone(wtstone_t *t, float *wave, float tl, float at, float dt, float sl, float rt, int sampling_rate){
  memcpy(t->wave, wave, sizeof(float)*WTSTONE_WAVE_SIZE);
  t->total_level = tl;
  t->attack_time = sampling_rate * at;
  t->decay_time = sampling_rate * dt;
  t->sustain_level = sl;
  t->release_time = sampling_rate * rt;
  t->attack_decay_time_int = t->attack_time_int + t->decay_time_int;
}


/* need to fix following 3 functions */

/*---------------------------------------------------
 *---------------------------------------------------*/
int wtstone_save(wtstone_t *t, char *filename){
  int i;
  FILE *fp = fopen(filename, "w");
  if(fp == NULL) return -1;
  for(i = 0; i < WTSTONE_WAVE_SIZE; i ++){
    fprintf(fp, "%f ", t->wave[i]);
  }
  fprintf(fp, "%d %f %d %f %d\n",
	  t->attack_time_int,
	  t->total_level,
	  t->decay_time_int,
	  t->sustain_level,
	  t->release_time_int);
  fclose(fp);
  return 0;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
int wtstone_load(wtstone_t *t, char *filename){
  int i;
  FILE *fp = fopen(filename, "r");
  if(fp == NULL) return -1;
  for(i = 0; i < WTSTONE_WAVE_SIZE; i ++){
    fscanf(fp, "%f ", &t->wave[i]);
  }
  fscanf(fp, "%d %f %d %f %d\n",
	  &t->attack_time_int,
	  &t->total_level,
	  &t->decay_time_int,
	  &t->sustain_level,
	 &t->release_time_int);
  fclose(fp);
  return 0;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
void wtstone_dump(wtstone_t *t){
  int i;
  printf("Wave\n");
  for(i = 0; i < WTSTONE_WAVE_SIZE; i ++){
    printf("%1.2f ", t->wave[i]);
    if(i % 8 == 7) printf("\n");
  }
  printf("AT %3d AL %1.2f DT %3d SL %1.2f RT %3d\n",
	  t->attack_time_int,
	  t->total_level,
	  t->decay_time_int,
	  t->sustain_level,
	 t->release_time_int);
}


/*---------------------------------------------------
 * searches empty voice slot.
 * if there are not any empty slot, returns a slot which has the most elpased time.
 * this function is called when a key is on to allocate the key
 *---------------------------------------------------*/
static wtsvoice_t *find_empty_voice(wtssynth_t *w){
  int i;
  wtsvoice_t *v;
  for(i = 0 ; i < WTSSYNTH_VOICE_MAX; i ++){
    if(w->voice[i].status == WTSVOICE_UNUSED) return &w->voice[i];
  }
  /* could not find empty voice */
  v = &w->voice[0];
  for(i = 1; i < WTSSYNTH_VOICE_MAX; i ++){
    if(w->voice[i].elapsed > v->elapsed) v = &w->voice[i];
  }
  w->voiceover = 1;
  return v;
}


/*---------------------------------------------------
 * internal function
 * search currently using slots for key off
 * if there are 2 or more slots having same key, 
 * returns a slot which have the most elpased time.
 * this function is called when key off
 *---------------------------------------------------*/
static wtsvoice_t *find_key_voice(wtssynth_t *w, int key){
  int i;
  wtsvoice_t *v = NULL;
  for(i = 0 ; i < WTSSYNTH_VOICE_MAX; i ++){
    if(w->voice[i].status == WTSVOICE_KEYON && w->voice[i].key == key){
      if(v == NULL){
	v = &w->voice[i];
	continue;
      }
      if(v->elapsed < w->voice[i].elapsed){
	v = &w->voice[i];
      }
    }
  }
  return v;
}


/*---------------------------------------------------
 * internal function
 * initializes voice structure
 *---------------------------------------------------*/
static void wtsvoice_init(wtsvoice_t *v){
  memset(v, 0, sizeof(wtsvoice_t));
}

/*---------------------------------------------------
 * internal function
 * returns adsr value of a voice
 * this value is to be used at wtsvoice_genwave()
 *---------------------------------------------------*/
static float wtsvoice_adsr(wtsvoice_t *v, wtstone_t *t){
  switch(v->status){

  case WTSVOICE_UNUSED:
    return 0.0;

  case WTSVOICE_KEYON:
    if(v->elapsed < t->attack_time_int){
      return t->total_level * (float)v->elapsed / (float)t->attack_time_int;
    }else if(v->elapsed < t->attack_decay_time_int){
      return t->total_level * (t->sustain_level * (float)(v->elapsed - t->attack_time_int) + 1.0 * (float)(t->attack_decay_time_int - v->elapsed)) / (float)t->decay_time_int;
    }else{
      return t->total_level * t->sustain_level;
    }

  case WTSVOICE_KEYOFF:
    if(v->elapsed - v->keyoff_time >= t->release_time_int){
      v->status = WTSVOICE_UNUSED; /* state change */
      return 0.0;
    }else{
      return t->total_level * v->keyoff_level * 
(float)(t->release_time_int - (v->elapsed - v->keyoff_time)) / (float)t->release_time_int;
    }

  } /* end of switch */

  return 0.0; /* never reached */
}

/*---------------------------------------------------
 * internal function
 * returns a wave sample at the current time pointer.
 * this does not proceed a time pointer, so needs to call
 * wtsvoice_next() if next sample is needed
 *---------------------------------------------------*/
static inline float wtsvoice_getwave(wtsvoice_t *v, wtstone_t *t){
  return 
    t->wave[0x1f&(v->phase>>16)] *
    wtsvoice_adsr(v, t) *
    v->velocity;
}

/*---------------------------------------------------
 * internal function
 * proceeds voice's time pointer.
 *---------------------------------------------------*/
static inline void wtsvoice_next(wtsvoice_t *v, float pitchbend_delta){
  //  v->phase += v->delta;
  v->phase += (v->delta * pitchbend_delta);
  v->phase &= 0x1FFFFF;
  v->elapsed ++;
}


/*---------------------------------------------------
 *---------------------------------------------------*/
wtssynth_t *wtssynth_create(){
  wtssynth_t *s;
  s = (wtssynth_t *)malloc(sizeof(wtssynth_t));
  s->config = wtsconfig_create();
  wtssynth_init(s);
  return s;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
void wtssynth_destroy(wtssynth_t *s){
  free(s);
}

/*---------------------------------------------------
 *---------------------------------------------------*/
void wtssynth_init(wtssynth_t *s){
  int i;
  for(i = 0;  i < WTSSYNTH_VOICE_MAX; i ++){
    wtsvoice_init(&s->voice[i]);
  }
}

/*---------------------------------------------------
 *---------------------------------------------------*/
void wtssynth_keyon(wtssynth_t *w, int key, float velocity){
  wtsvoice_t *v;
  v = find_empty_voice(w);
  v->status = WTSVOICE_KEYON;
  v->elapsed = 0;
  v->phase = 0;
  v->delta = 0x10000 * WTSTONE_WAVE_SIZE * freq_table[key] / w->config->sampling_rate;
  v->key = key;
  v->velocity = velocity;
  v->amp = (256 - key)/512.0;

}

/*---------------------------------------------------
 *---------------------------------------------------*/
void wtssynth_keyoff(wtssynth_t *w, int key){
  wtsvoice_t *v;
  wtstone_t *t;
  //  t = &w->config.tone;
  t = wtsconfig_gettone(w->config);
  v = find_key_voice(w,key);
  if(v == NULL) return; /* no voices to be key off */
  v->keyoff_level = wtsvoice_adsr(v, t);
  v->status = WTSVOICE_KEYOFF;
  v->keyoff_time = v->elapsed;

  //  printf("keyoff %d %d %f\n", key, v->keyoff_time, v->keyoff_level);
}


/*---------------------------------------------------
 *---------------------------------------------------*/
int wtssynth_get_current_key(wtssynth_t *w, int index){
  if(index < 0 || index >= WTSSYNTH_VOICE_MAX){
    return -1;
  }
  if(w->voice[index].status == WTSVOICE_UNUSED) return -1;
  return w->voice[index].key;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
int wtssynth_is_key_offing(wtssynth_t *w, int index){
  if(index < 0 || index >= WTSSYNTH_VOICE_MAX){
    return 0;
  }
  return w->voice[index].status == WTSVOICE_KEYOFF;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
int wtssynth_num_key_playing(wtssynth_t *w){
  int i, c;
  c = 0;
  for(i = 0; i < WTSSYNTH_VOICE_MAX; i ++){
    if(w->voice[i].status != WTSVOICE_UNUSED) c++;
  }
  return c;
}



/*---------------------------------------------------
 *---------------------------------------------------*/
void wtssynth_render(wtssynth_t *w, audiobuf_t *a, int start, int size){
  int i,j;
  float f;
  float l,r;
  float vl,vr;
  float amp;
  wtsvoice_t *v;
  wtsconfig_t *c = w->config;
  wtstone_t *t = wtsconfig_gettone(c);
  float pitchbend_delta = c->pitchbend_delta;

  vl = c->volume_total_l;
  vr = c->volume_total_r;
  for(i = 0; i < WTSSYNTH_VOICE_MAX; i ++){
    v = &w->voice[i];
    amp = v->amp;
    if(v->status == WTSVOICE_UNUSED) continue;
    audiobuf_seek(a, start);
    for(j = 0; j < size; j ++){
      f = wtsvoice_getwave(v, t);
      l = amp * f * vl;
      r = amp * f * vr;
      audiobuf_add_L(a, l);
      audiobuf_add_R(a, r);

      wtsvoice_next(v, pitchbend_delta);
      //      wtsvoice_next(v, 1.0);

    }    
  }
}

/*---------------------------------------------------
 *---------------------------------------------------*/
void wtssynth_midi(wtssynth_t *w, midievent_t *e){
  int key_temp = 0;

  key_temp = e->shortparam[0];
  key_temp += w->midi_key;
  if(key_temp < 0) key_temp = 0;
  if(key_temp > 127) key_temp = 127;

  switch(e->type){
  case MIDIEVENT_TYPE_NOTEOFF:
    wtssynth_keyoff(w, key_temp);
    break;
  case MIDIEVENT_TYPE_NOTEON:
    if(e->shortparam[1] == 0){
      wtssynth_keyoff(w, key_temp);
    }else{
      wtssynth_keyon(w, key_temp, (float)e->shortparam[1]*(float)e->shortparam[1]/(127.0*127.0));
    }
    break;
  case MIDIEVENT_TYPE_KEYPRESSURE:
    break;
  case MIDIEVENT_TYPE_CONTROLCHANGE:
    switch(e->shortparam[0]){
    case MIDIEVENT_CC_VOLUME:
      w->config->volume = (float)e->shortparam[1]/127.0;
      wtsconfig_update_volume(w->config);
      break;
    case MIDIEVENT_CC_PANPOT:
      w->config->panpot = (float)e->shortparam[1]/127.0;
      wtsconfig_update_volume(w->config);
      break;
    case MIDIEVENT_CC_EXPRESSION:
      w->config->expression = (float)e->shortparam[1]/127.0;
      wtsconfig_update_volume(w->config);
      break;
    default:
      ;
    }
    break;
  case MIDIEVENT_TYPE_PROGRAMCHANGE:
    wtsconfig_programchange(w->config, e->shortparam[0]);
    break;
  case MIDIEVENT_TYPE_CHANNELPRESSURE:
    break;
  case MIDIEVENT_TYPE_PITCHBENDCHANGE:
    //    w->config->pitchbend = BR * (((float)e->shortparam[1]/127.0 - 0.5) * 2.0) / 12.0;
    w->config->pitchbend = 2 * (((float)e->shortparam[1]/127.0 - 0.5) * 2.0) / 12.0;
    w->config->pitchbend_delta = pow(2.0, w->config->pitchbend);
    break;
  default:
    ;
  }
}

void wtssynth_midi_key(wtssynth_t *w, int midi_key){
  wtssynth_all_note_off(w);
  w->midi_key = midi_key;
}

int wtssynth_get_program(wtssynth_t *w){
  return w->config->current_preset;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
void wtssynth_all_note_off(wtssynth_t *w){
  int i;
  for(i = 0 ; i < WTSSYNTH_VOICE_MAX; i ++){
    w->voice[i].status = WTSVOICE_UNUSED;
  }  
}

