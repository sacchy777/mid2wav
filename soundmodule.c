/*
 * Sound Module
 *
 * soundmodule.c
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


#include "soundmodule.h"
#include "wtssynth.h"
#include "nrsynth.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*---------------------------------------------------
 *---------------------------------------------------*/
void soundmodule_chamber_init(soundmodule_chamber_t *c, int type){
  c->type = type;
  switch(type){
  case SOUNDMODULE_CHAMBER_TYPE_WTS:
    c->synth = wtssynth_create();
    break;
  case SOUNDMODULE_CHAMBER_TYPE_NR:
    c->synth = nrsynth_create();
    break;
  default:
    ;
  }
}

/*---------------------------------------------------
 *---------------------------------------------------*/
soundmodule_t *soundmodule_create(){
  soundmodule_t *s;
  int i;
  s = (soundmodule_t *)malloc(sizeof(soundmodule_t));
  memset(s, 0, sizeof(soundmodule_t));
  for(i = 0; i < SOUNDMODULE_CHAMBER_MAX; i ++){
    soundmodule_chamber_init(&s->chambers[i], 
			     i != 9 ? SOUNDMODULE_CHAMBER_TYPE_WTS : SOUNDMODULE_CHAMBER_TYPE_NR);
  }
  return s;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
void soundmodule_destroy(soundmodule_t *s){
  int i;
  if(s == NULL) return;
  for(i = 0; i < SOUNDMODULE_CHAMBER_MAX; i ++){
    if(s->chambers[i].synth == NULL) continue;
    if(i != 9){
      wtssynth_destroy(s->chambers[i].synth);
    }else{
      nrsynth_destroy(s->chambers[i].synth);
    }
  }
  free(s);
}

/*---------------------------------------------------
 *---------------------------------------------------*/
void soundmodule_dispatch_midi(soundmodule_t *s, midifile_t *mf){
  void *synth;
  midievent_t *e = midifile_current_event(mf);
  if(e->is_meta){
    ;
  }else{
    synth = s->chambers[e->channel].synth;
    switch(s->chambers[e->channel].type){
    case SOUNDMODULE_CHAMBER_TYPE_WTS:
      wtssynth_midi(synth, e);
      break;
    case SOUNDMODULE_CHAMBER_TYPE_NR:
      nrsynth_midi(synth, e);
      break;
    default:
      ;
    }
  }
}

/*---------------------------------------------------
 *---------------------------------------------------*/
void soundmodule_dispatch_wave(soundmodule_t *s, audiobuf_t *a, int current_bufpos, int length){
  int i;
  for(i = 0; i < SOUNDMODULE_CHAMBER_MAX; i ++){

    if(s->chambers[i].is_muted) continue;

    /* dispatch */
    switch(s->chambers[i].type){

    case SOUNDMODULE_CHAMBER_TYPE_WTS:
      wtssynth_render(s->chambers[i].synth, a, current_bufpos, length);
      break;

    case SOUNDMODULE_CHAMBER_TYPE_NR:
      nrsynth_render(s->chambers[i].synth, a, current_bufpos, length);
      break;

    default:
      ; /* cannot reach here */

    }

  }
}

/*---------------------------------------------------
 *---------------------------------------------------*/
void soundmodule_render(soundmodule_t *s, audiobuf_t *a, midifile_t *mf){
  int samples_to_render;
  int current_bufpos = 0;


  /* render all samples in audiobuf */
  while(current_bufpos < a->size){

    /* processes all midi events at this time if there are events */
    while(midifile_has_waiting_event(mf)){
      soundmodule_dispatch_midi(s, mf);
      midifile_next_event(mf);
    }

    /* get number of samples to be rendered */
    samples_to_render = midifile_get_nextevent_time(mf);

    if(samples_to_render <= 0 || /* no midi event in midifile */
       a->size - current_bufpos < samples_to_render){ /* midi event too far */
      samples_to_render = a->size - current_bufpos; /* render all of left samples */
    }
    
    /* render waves */
    soundmodule_dispatch_wave(s, a, current_bufpos, samples_to_render);
    
    /* increment ticks in midifile */
    midifile_ticks_proceed(mf, samples_to_render);

    /* increment current position of audiobuf */
    current_bufpos += samples_to_render;
  }
}

/*---------------------------------------------------
 *---------------------------------------------------*/
int soundmodule_is_muted(soundmodule_t *s, int channel){
  return s->chambers[channel].is_muted == 1;
}

void soundmodule_mute(soundmodule_t *s, int channel){
  s->chambers[channel].is_muted = 1;
}

void soundmodule_unmute(soundmodule_t *s, int channel){
  void *synth;
  s->chambers[channel].is_muted = 0;
  synth = s->chambers[channel].synth;
  switch(s->chambers[channel].type){
  case SOUNDMODULE_CHAMBER_TYPE_WTS:
    return wtssynth_all_note_off(synth);
  case SOUNDMODULE_CHAMBER_TYPE_NR:
    return ;
  default:
    return ;
  } /* end of switch */
}

void soundmodule_solo(soundmodule_t *s, int channel){
  int i;
  for(i = 0; i < SOUNDMODULE_CHAMBER_MAX; i ++){
    s->chambers[i].is_muted = i == channel ? 0 : 1;
  }
}

void soundmodule_clear_mute(soundmodule_t *s){
  int i;
  for(i = 0; i < SOUNDMODULE_CHAMBER_MAX; i ++){
    s->chambers[i].is_muted = 0;
  }
}


/*---------------------------------------------------
 *---------------------------------------------------*/
void soundmodule_midi_key(soundmodule_t *s, int midi_key){
  int i;
  void *synth;
  for(i = 0; i < SOUNDMODULE_CHAMBER_MAX; i ++){
    synth = s->chambers[i].synth;
    switch(s->chambers[i].type){
    case SOUNDMODULE_CHAMBER_TYPE_WTS:
      wtssynth_midi_key(synth, midi_key);
      break;
    case SOUNDMODULE_CHAMBER_TYPE_NR:
      break;
    default:
      ;
    } /* end of switch */

  }
}



int soundmodule_get_current_key(soundmodule_t *s, int channel, int voice_index){
  void *synth;
  synth = s->chambers[channel].synth;
  switch(s->chambers[channel].type){
  case SOUNDMODULE_CHAMBER_TYPE_WTS:
    return wtssynth_get_current_key(synth, voice_index);
  case SOUNDMODULE_CHAMBER_TYPE_NR:
    return nrsynth_get_current_key(synth, voice_index);
  default:
    return -1;
  } /* end of switch */
}

int soundmodule_is_key_offing(soundmodule_t *s, int channel, int voice_index){
  void *synth;
  synth = s->chambers[channel].synth;
  switch(s->chambers[channel].type){
  case SOUNDMODULE_CHAMBER_TYPE_WTS:
    return wtssynth_is_key_offing(synth, voice_index);
  case SOUNDMODULE_CHAMBER_TYPE_NR:
    return 0;
  default:
    return 0;
    ;
  } /* end of switch */
}
/*---------------------------------------------------
 *---------------------------------------------------*/
int soundmodule_get_program(soundmodule_t *s, int channel){
  void *synth;
  synth = s->chambers[channel].synth;
  switch(s->chambers[channel].type){
  case SOUNDMODULE_CHAMBER_TYPE_WTS:
    return wtssynth_get_program(synth);
  case SOUNDMODULE_CHAMBER_TYPE_NR:
    return 0;
  default:
    return 0;
  } /* end of switch */
}

