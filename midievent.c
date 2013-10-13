#include "midievent.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*---------------------------------------------------
 *---------------------------------------------------*/
midievent_t *midievent_create_short(unsigned int absolute_time, int channel, int type, int param0, int param1, int param2, int param3){
  midievent_t *e;
  e = (midievent_t *)malloc(sizeof(midievent_t));
  memset(e, 0, sizeof(midievent_t));
  e->is_meta = 0;
  e->absolute_time = absolute_time;
  e->channel = channel;
  e->type = type;
  e->shortparam[0] = param0;
  e->shortparam[1] = param1;
  e->shortparam[2] = param2;
  e->shortparam[3] = param3;
  return e;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
midievent_t *midievent_create_meta_string(unsigned int absolute_time, char *name, int length, int event){
  midievent_t *e;
  e = (midievent_t *)malloc(sizeof(midievent_t));
  memset(e, 0, sizeof(midievent_t));
  e->absolute_time = absolute_time;
  e->is_meta = 1;
  e->channel = 0;
  e->type = event;
  e->longparam_char = (char *)malloc(sizeof(char) * MIDIEVENT_STRING_CHAR_MAX);
  memset(e->longparam_char, 0, sizeof(char) * MIDIEVENT_STRING_CHAR_MAX);
  strncpy(name, e->longparam_char, length);
  return e;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
midievent_t *midievent_create_meta_int(unsigned int absolute_time, int param, int event){
  midievent_t *e;
  e = (midievent_t *)malloc(sizeof(midievent_t));
  memset(e, 0, sizeof(midievent_t));
  e->absolute_time = absolute_time;
  e->is_meta = 1;
  e->channel = 0;
  e->type = event;
  e->longparam_int = param;
  return e;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
void midievent_destroy(midievent_t *e){
  if(e->longparam_char != NULL){
    free(e->longparam_char);
  }
  free(e);
}

/*---------------------------------------------------
 *---------------------------------------------------*/
void midievent_dump(midievent_t *e){
  if(e->is_meta == 0){
    printf("    Ticks[%010d] Ch[%02d]: Type[%02x] Param[%d %d %d %d]\n", 
	   e->absolute_time, e->channel, e->type,
	   e->shortparam[0], e->shortparam[1], e->shortparam[2], e->shortparam[3]);
  }else{
    printf(" %010d %010d [%02d]: Long  %02x %d\n",
	   e->absolute_time, e->ticks, e->channel, e->type, e->longparam_int);
  }
}

/*---------------------------------------------------
 *---------------------------------------------------*/
int midievent_compare(const void *p, const void *q){
  midievent_t **_p = (midievent_t **)p;
  midievent_t **_q = (midievent_t **)q;
  return (*_p)->absolute_time - (*_q)->absolute_time;
}

