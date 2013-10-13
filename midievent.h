/*
 * Midievent
 *
 * midievent.h
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

/* ----------------------------------------------------------------------
 *
 * This library assmes: 
 *  (unsigned) int as 32 bit integer,
 *  (unsigned) short as 16 bit integer
 *
 * ----------------------------------------------------------------------*/

#ifndef MIDIEVENT_H
#define MIDIEVENT_H

#ifdef __cplusplus
extern "C" {
#endif

#define MIDIEVENT_TYPE_NONE 0
#define MIDIEVENT_TYPE_NOTEOFF 0x80
#define MIDIEVENT_TYPE_NOTEON 0x90
#define MIDIEVENT_TYPE_KEYPRESSURE 0xa0
#define MIDIEVENT_TYPE_CONTROLCHANGE 0xb0
#define MIDIEVENT_TYPE_PROGRAMCHANGE 0xc0
#define MIDIEVENT_TYPE_CHANNELPRESSURE 0xd0
#define MIDIEVENT_TYPE_PITCHBENDCHANGE 0xe0
#define MIDIEVENT_TYPE_SYSEX 0xf0
#define MIDIEVENT_MAX_ABSOLUTETIME 999999999

#define MIDIEVENT_META_TEMPOCHANGE 0x51
#define MIDIEVENT_META_TITLECHANGE 0x02
#define MIDIEVENT_META_AUTHORCHANGE 0x03

#define MIDIEVENT_STRING_CHAR_MAX 129

#define MIDIEVENT_CC_MODULATION 1
#define MIDIEVENT_CC_VOLUME 7
#define MIDIEVENT_CC_PANPOT 10
#define MIDIEVENT_CC_EXPRESSION 11

/*---------------------------------------------------
 * main structure
 *---------------------------------------------------*/
typedef struct {
  unsigned int absolute_time; /* MIDI delta time */
  unsigned int ticks; /* in samples */
  int is_meta;
  int channel;
  int type;
  int shortparam[4];
  int longparam_size;
  char *longparam_char;
  int longparam_int;
} midievent_t;

/*---------------------------------------------------
 * create a midievent with midi short message
 *---------------------------------------------------*/
midievent_t *midievent_create_short(unsigned int absolute_time, int channel, int type, int param0, int param1, int param2, int param3);

/*---------------------------------------------------
 * compare time of the event
 *---------------------------------------------------*/
int midievent_compare(const void *p, const void *q);

/*---------------------------------------------------
 * destructor
 *---------------------------------------------------*/
void midievent_destroy(midievent_t *e);

/*---------------------------------------------------
 * for debug
 *---------------------------------------------------*/
void midievent_dump(midievent_t *e);

/*---------------------------------------------------
 * create a midievent with midi long message
 *---------------------------------------------------*/
midievent_t *midievent_create_meta_string(unsigned int absolute_time, char *name, int length, int event);
#define midievent_create_meta_author(a, n) midievent_create_meta_string(a, n, MIDIEVENT_META_AUTHORCHANGE)
#define midievent_create_meta_title(a, n) midievent_create_meta_string(a, n, MIDIEVENT_META_TITLECHANGE)

/*---------------------------------------------------
 * create a midievent having int with midi long message, especially for TEMPO
 *---------------------------------------------------*/
midievent_t *midievent_create_meta_int(unsigned int absolute_time, int param, int event);


#ifdef __cplusplus
}
#endif

#endif
