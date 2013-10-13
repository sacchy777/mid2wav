#include "midifile.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "misc.h"


/*---------------------------------------------------
 *---------------------------------------------------*/
midifile_t *midifile_create(int size){
  midifile_t *mf;
  mf = (midifile_t *)malloc(sizeof(midifile_t));
  memset(mf, 0, sizeof(midifile_t));
  mf->events = (midievent_t **)malloc(sizeof(midievent_t*) * size);
  memset(mf->events, 0, sizeof(midievent_t *) * size);
  mf->current_index = 0;
  mf->record_index = 0;
  mf->size = size;
  mf->tempo = 500000; /* tempo 120 */
  mf->verbose = 2;
  return mf;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
void midifile_set_verbose(midifile_t *mf, int level){
  mf->verbose = level;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
void midifile_destroy(midifile_t *mf, int deep){
  int i;
  if(deep){
    for(i = 0; i < mf->record_index; i ++){
      midievent_destroy(mf->events[i]);
    }
  }
  free(mf->events);
  free(mf);
}

/*---------------------------------------------------
 *---------------------------------------------------*/
static void midifile_add(midifile_t *mf, midievent_t *e){
  if(mf->record_index < mf->size){
    //    printf("%d\n",mf->record_index);
    //    midievent_dump(e);
    //    if(mf->record_index > 100) exit(-1);
    mf->events[mf->record_index ++] = e;
  }else{
    printf("too many midi events\n");
    /* error! */
  }
}

/*---------------------------------------------------
 *---------------------------------------------------*/
midievent_t *midifile_current_event(midifile_t *mf){
  return mf->events[mf->current_index];
}
/*
midievent_t *midifile_get_waiting_event(midifile_t *mf){
  midievent_t *e;
  if(mf->current_index >= mf->record_index) return NULL;
  e = mf->events[mf->current_index];
  if(e->ticks > mf->current_ticks) return NULL;
  mf->current_index ++;
  return e;
}
*/

/*---------------------------------------------------
 *---------------------------------------------------*/
int midifile_has_waiting_event(midifile_t *mf){
  midievent_t *e;
  if(mf->current_index >= mf->record_index) return 0;
  e = mf->events[mf->current_index];
  if(e->ticks > mf->current_ticks) return 0;
  return 1;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
void midifile_next_event(midifile_t *mf){
  mf->current_index ++;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
int midifile_get_nextevent_time(midifile_t *mf){
  midievent_t *e;
  if(mf->current_index >= mf->record_index) return -1;
  e = midifile_current_event(mf);
  if(e->ticks > mf->current_ticks) return e->ticks - mf->current_ticks;
  return 0;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
void midifile_ticks_proceed(midifile_t *mf, int ticks){
  mf->current_ticks += ticks;
}


/*---------------------------------------------------
 *---------------------------------------------------*/
void midifile_rewind(midifile_t *mf){
  mf->current_ticks = 0;
  mf->current_index = 0;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
static int parse_header(midifile_t *mf, FILE *fp){
  char buf[4];
  midiparam_t *mp;
  mp = &mf->midiparam;
  fread(buf, 4, 1, fp);
  if(strncmp("MThd", buf, 4) != 0){
    return -1;
  }
  fread4Bbeu(fp, &mp->header_length);
  if(mp->header_length != 6){
    return -1;
  }
  fread2Bbeu(fp, &mp->format);
  if(mp->format != 0 && mp->format != 1){
    return -1;
  }
  fread2Bbeu(fp, &mp->num_tracks);
  if(mp->num_tracks == 0){
    return -1;
  }
  fread2Bbeu(fp, &mp->deltatime);
  return 1;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
static int getdeltatime(midifile_t *mf, FILE *fp, unsigned int *ret){
  int i;
  int error;
  unsigned int delta = 0;
  char d;
  midiparam_t *mp;
  mp = &mf->midiparam;
  for(i = 0; i < 4; i ++){
    error = fread(&d, 1, 1, fp);
    if(error < 1){
      return error;
    }    
    mp->track_length --;
    delta = delta * 128 + (d&0x7f);
    if(d >= 0) break;
  }
  *ret = delta;
  return 1;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
static int parse_noteoff(midifile_t *mf, FILE *fp){
  midiparam_t *mp;
  unsigned char key, velocity;
  midievent_t *e;
  mp = &mf->midiparam;
  fread(&key, 1, 1, fp);
  fread(&velocity, 1, 1, fp);
  mp->track_length -= 2;
  e = midievent_create_short(mp->current_time, mp->channel, MIDIEVENT_TYPE_NOTEOFF, key, velocity, 0, 0);
  midifile_add(mf, e);
  if(mf->verbose > 1) midievent_dump(e);
  return 1;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
static int parse_noteon(midifile_t *mf, FILE *fp){
  midiparam_t *mp;
  unsigned char key, velocity;
  midievent_t *e;
  mp = &mf->midiparam;
  fread(&key, 1, 1, fp);
  fread(&velocity, 1, 1, fp);
  mp->track_length -= 2;
  e = midievent_create_short(mp->current_time, mp->channel, MIDIEVENT_TYPE_NOTEON, key, velocity, 0, 0);
  midifile_add(mf, e);
  if(mf->verbose > 1) midievent_dump(e);
  return 1;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
static int parse_keypressure(midifile_t *mf, FILE *fp){
  midiparam_t *mp;
  unsigned char dummy;
  mp = &mf->midiparam;
  fread(&dummy, 1, 1, fp);
  fread(&dummy, 1, 1, fp);
  mp->track_length -= 2;
  if(mf->verbose > 1) printf("    Ignored key puressure.");
  return 1;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
static int parse_controlchange(midifile_t *mf, FILE *fp){
  midiparam_t *mp;
  unsigned char control, value;
  midievent_t *e;
  mp = &mf->midiparam;
  fread(&control, 1, 1, fp);
  fread(&value, 1, 1, fp);
  mp->track_length -= 2;
  e = midievent_create_short(mp->current_time, mp->channel, MIDIEVENT_TYPE_CONTROLCHANGE, control, value, 0, 0);
  midifile_add(mf, e);
  if(mf->verbose > 1) midievent_dump(e);
  return 1;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
static int parse_programchange(midifile_t *mf, FILE *fp){
  midiparam_t *mp;
  unsigned char program;
  midievent_t *e;
  mp = &mf->midiparam;
  fread(&program, 1, 1, fp);
  mp->track_length -= 1;
  e = midievent_create_short(mp->current_time, mp->channel, MIDIEVENT_TYPE_PROGRAMCHANGE, program, 0, 0, 0);
  midifile_add(mf, e);
  if(mf->verbose > 1) midievent_dump(e);
  //  puts("programchangge");
  return 1;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
static int parse_channelpressure(midifile_t *mf, FILE *fp){
  midiparam_t *mp;
  unsigned char dummy;
  mp = &mf->midiparam;
  fread(&dummy, 1, 1, fp);
  mp->track_length -= 1;
  if(mf->verbose > 1) printf("    Ignored channel puressure.");
  return 1;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
static int parse_pitchbendchange(midifile_t *mf, FILE *fp){
  midiparam_t *mp;
  unsigned char high, low;
  midievent_t *e;
  mp = &mf->midiparam;
  fread(&high, 1, 1, fp);
  fread(&low, 1, 1, fp);
  mp->track_length -= 2;
  e = midievent_create_short(mp->current_time, mp->channel, MIDIEVENT_TYPE_PITCHBENDCHANGE, high, low, 0, 0);
  midifile_add(mf, e);
  if(mf->verbose > 1) midievent_dump(e);
  return 1;
}


/*---------------------------------------------------
 *---------------------------------------------------*/
static int parse_sysex_body(midifile_t *mf, FILE *fp){

  midiparam_t *mp;
  mp = &mf->midiparam;
  char buf[128];
  char length;

  fread(&length, 1, 1, fp);
  mp->track_length --;

  fread(&buf, length, 1, fp);
  mp->track_length -= length;

  return 1;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
static int parse_meta(midifile_t *mf, FILE *fp){
  char buf[128];
  midiparam_t *mp;
  int tempo;
  mp = &mf->midiparam;
  char event;
  char length;
  int i;
  fread(&event, 1, 1, fp);
  fread(&length, 1, 1, fp);
  mp->track_length -= 2;
  switch(event){
  case 0x02: /* title */
    memset(mp->title, 0, sizeof(char) * MIDIPARAM_STRING_CHAR_MAX);
    fread(buf, length, 1, fp);
    mp->track_length -= length;
    break;
  case 0x03:
    memset(mp->author, 0, sizeof(char) * MIDIPARAM_STRING_CHAR_MAX);
    fread(buf, length, 1, fp);
    mp->track_length -= length;
    break;
  case 0x2f:
    //    fread(buf, length, 1, fp);
    //    mp->track_length -= length;
    /* do nothing */
    if(mf->verbose > 0) printf("   Detected End of track\n");
    break;
  case 0x51:
    fread(buf, length, 1, fp);
    mp->track_length -= length;
    tempo = (unsigned char)buf[0] * 65536 + (unsigned char)buf[1] * 256 + (unsigned char)buf[2];
    midifile_add(mf, midievent_create_meta_int(mp->current_time, tempo, MIDIEVENT_META_TEMPOCHANGE));    if(mf->verbose > 0) printf("   Detected Tempo Change %3.1f\n", (float)tempo/500000.0*120.0);
    break;

  default:
    if(length > 0)fread(buf, length, 1, fp);
    mp->track_length -= length;
    if(mf->verbose > 0){
      printf("   Detected unsupported Meta event:\n   f0 %02x ", length);
      for(i = 0 ; i < length; i++) printf("%02x ", buf[i]);
      printf("\n");;
    }
    break;
  }
  return 1;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
static int parse_sysex(midifile_t *mf, FILE *fp){
  midiparam_t *mp;
  mp = &mf->midiparam;
  switch(mp->channel){
  case 0x0:
  case 0x7:
    parse_sysex_body(mf, fp);
    break;
  case 0xf:
    parse_meta(mf, fp);
    break;
  default:
    ;
  }
  return 1;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
static int parse_tracks(midifile_t *mf, FILE *fp){
  int error;
  int i;
  midiparam_t *mp;
  char buf[256];
  mp = &mf->midiparam;
  if(mf->verbose > 0) printf(" Number of Tracks: %d\n", mp->num_tracks);
  for(i = 0; i < mp->num_tracks; i ++){
    mp->last_channel = 0;
    mp->last_command = 0x80;
    fread(buf, 4, 1, fp);
    if(strncmp("MTrk", buf, 4) != 0){
      return -1;
    }
    fread4Bbeu(fp, &mp->track_length);
    if(mf->verbose > 0) printf("  Track %d size: %d\n", i, mp->track_length);
    mp->current_time = 0;
    while(mp->track_length > 0){
      char c;
      unsigned int delta;
      error = getdeltatime(mf, fp, &delta);
      if(error <= 0) goto error;
      mp->current_time += delta;

      error = fread(&c, 1, 1, fp);
      mp->track_length --;

      if(c >= 0){
	mp->command = mp->last_command;
	mp->channel = mp->last_channel;
	fseek(fp, -1, SEEK_CUR);
	mp->track_length ++;
      }else{
	mp->command = c & 0xf0;
	mp->channel = c & 0x0f;
	mp->last_command = mp->command;
	mp->last_channel = mp->channel;
      }

      switch(mp->command){
      case MIDIEVENT_TYPE_NOTEOFF: error = parse_noteoff(mf, fp); break; /* 0x80 */
      case MIDIEVENT_TYPE_NOTEON: error = parse_noteon(mf, fp); break; /* 0x90 */
      case MIDIEVENT_TYPE_KEYPRESSURE: error = parse_keypressure(mf, fp); break; /* 0xa0 */
      case MIDIEVENT_TYPE_CONTROLCHANGE: error = parse_controlchange(mf, fp); break; /* 0xb0 */
      case MIDIEVENT_TYPE_PROGRAMCHANGE: error = parse_programchange(mf, fp); break; /* 0xc0 */
      case MIDIEVENT_TYPE_CHANNELPRESSURE: error = parse_channelpressure(mf, fp); break; /* 0xd0 */
      case MIDIEVENT_TYPE_PITCHBENDCHANGE: error = parse_pitchbendchange(mf, fp); break; /* 0xe0 */
      case MIDIEVENT_TYPE_SYSEX: error = parse_sysex(mf, fp); break; /* 0xf0 */
      default:
	break;
      } /* end of switch */
    }/* end of while */
    if(mf->verbose > 0) printf("\n");
  } /* for */
  return 0;
 error:
  return -1;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
static void calc_ticks(midifile_t *mf, int sampling_rate){
  int i;
  unsigned int base_absolute_time = 0;
  unsigned int base_absolute_ticks = 0;
  midiparam_t *mp;
  mp = &mf->midiparam;
  for(i = 0; i < mf->record_index; i ++){
    midievent_t *e = mf->events[i];
    e->ticks = base_absolute_ticks +
      (
       (float)(e->absolute_time - base_absolute_time) * 
       (float)sampling_rate / (float)mp->deltatime * (float)mf->tempo / 1000000.0
       );
    if(e->is_meta == 1 && e->type == MIDIEVENT_META_TEMPOCHANGE){
      mf->tempo = e->longparam_int;
      base_absolute_time = e->absolute_time;
      base_absolute_ticks = e->ticks;
    }
  }
}

/*---------------------------------------------------
 *---------------------------------------------------*/
int midifile_load(midifile_t *mf, const char *filename, int sampling_rate){
  FILE *fp;
  fp = fopen(filename, "rb");
  if(fp == NULL) return -1;
  if(mf->verbose > 0) printf("Started header parsing of %s.\n", filename);
  parse_header(mf, fp);
  if(mf->verbose > 0) printf("Started track parsing.\n");
  parse_tracks(mf, fp);
  fclose(fp);
  if(mf->verbose > 0) printf("Started track sorting.\n");
  qsort(mf->events, mf->record_index, sizeof(midievent_t*), midievent_compare);
  if(mf->verbose > 0) printf("Started time calculation.\n");
  calc_ticks(mf, sampling_rate);
  midifile_rewind(mf);
  if(mf->verbose > 0) printf("MIDI file parse done.\n");
  return 1;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
void midifile_dump(midifile_t *mf){
  int i;
  for(i = 0 ; i < mf->record_index; i ++){
    midievent_dump(mf->events[i]);
  }
}

/*---------------------------------------------------
 *---------------------------------------------------*/
int midifile_has_events(midifile_t *mf){
  return mf->current_index < mf->record_index;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
int midifile_progress(midifile_t *mf){
  return 100 * mf->current_index / mf->record_index;
}
