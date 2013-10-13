/*
 * Audio Buffer library for audio processing
 *
 * audiobuf.h
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
 * This library needs:
 * misc.c
 *
 * This library assmes: 
 *  (unsigned) int as 32 bit integer,
 *  (unsigned) short as 16 bit integer
 *
 * ----------------------------------------------------------------------*/

#ifndef AUDIOBUF_H
#define AUDIOBUF_H

#ifdef __cplusplus
extern "C" {
#endif

#define AUDIOBUF_VERSION "0.1"

/* ----------------------------------------------------------------------
 * audio buffer data structure
 *
 * n_channels:
 *   number of audio channels, default 2(stereo)
 * sampling_rate:  
 *   sampling rate. used for making wav file header, default 44100 
 * qbit:  
 *   quantum bit. used for making wav file header, default 16
 * data:
 *   wave data in float(-1.0 .. 1.0)
 * index:
 *   current index of the data
 * size: 
 *   number of float data in each channel
 * ----------------------------------------------------------------------*/
typedef struct {
  int n_channels;
  int sampling_rate;
  int qbit;
  float **data;
  int *index;
  int size;
} audiobuf_t;

/* ----------------------------------------------------------------------
 * constructor
 * 
 * size: buffer size in number of floats in each channel
 * n_channels: number of channels, 2 for stereo
 * sampling_rate: number of samples in a second. 44100 for default
 *
 * returns:
 * pointer or NULL
 * ----------------------------------------------------------------------*/
audiobuf_t *audiobuf_create(int size, int n_channels, int sampling_rate, int qbit);
#define AUDIOBUF_SAMPLING_RATE_DEFAULT 44100
#define AUDIOBUF_QBIT_DEFAULT 16
#define AUDIOBUF_N_CHANNELS_DEFAULT 2
#define audiobuf_create_default(s) \
  audiobuf_create(s, \
		  AUDIOBUF_N_CHANNELS_DEFAULT, \
		  AUDIOBUF_SAMPLING_RATE_DEFAULT, \
		  AUDIOBUF_QBIT_DEFAULT);

/* ----------------------------------------------------------------------
 * another constructor
 *
 * creates audiobuf from an existing audiobuf
 * ----------------------------------------------------------------------*/
audiobuf_t *audiobuf_duplicate(audiobuf_t *a);

/* ----------------------------------------------------------------------
 * destructor
 * ----------------------------------------------------------------------*/
void audiobuf_destroy(audiobuf_t *a);

/* ----------------------------------------------------------------------
 * clears data
 * ----------------------------------------------------------------------*/
void audiobuf_clear(audiobuf_t *a);

/* ----------------------------------------------------------------------
 * rewinds index
 * ----------------------------------------------------------------------*/
void audiobuf_rewind(audiobuf_t *a);

/* ----------------------------------------------------------------------
 * sets index to pos
 * ----------------------------------------------------------------------*/
void audiobuf_seek(audiobuf_t *a, int pos);

/* ----------------------------------------------------------------------
 * adds data to specified channel, and proceed index
 * ----------------------------------------------------------------------*/
#define AUDIOBUF_CHANNEL_L 0
#define AUDIOBUF_CHANNEL_R 1
void audiobuf_add(audiobuf_t *a, int channel, float data);
#define audiobuf_add_L(a, d) audiobuf_add(a, AUDIOBUF_CHANNEL_L, d)
#define audiobuf_add_R(a, d) audiobuf_add(a, AUDIOBUF_CHANNEL_R, d)

/* ----------------------------------------------------------------------
 * overwrites data to specified channel, and proceed index
 * ----------------------------------------------------------------------*/
void audiobuf_write(audiobuf_t *a, int channel, float data);
#define audiobuf_write_L(a, d) audiobuf_write(a, AUDIOBUF_CHANNEL_L, d)
#define audiobuf_write_R(a, d) audiobuf_write(a, AUDIOBUF_CHANNEL_R, d)

/* ----------------------------------------------------------------------
 * save data to .wav file.
 * call audiobuf_save_header first.
 *
 * returns:
 * 0 .. success
 * -1 .. failed
 * ----------------------------------------------------------------------*/
int audiobuf_save_header(audiobuf_t *a, const char *filename);
int audiobuf_save_append(audiobuf_t *a, const char *filename);
#define audiobuf_save(a, f) audiobuf_save_header(a, f);audiobuf_save_append(a, f)

/* ----------------------------------------------------------------------
 * copies data from src to dst
 * ----------------------------------------------------------------------*/
void audoibuf_copy(audiobuf_t *src, audiobuf_t *dst);

/* ----------------------------------------------------------------------
 * adds data from src to dst
 * ----------------------------------------------------------------------*/
void audoibuf_merge(audiobuf_t *src, audiobuf_t *dst);


/* ----------------------------------------------------------------------
 * gets a wave sample at current position, index does not proceed by 
 * calling this.
 * ----------------------------------------------------------------------*/
float audiobuf_get(audiobuf_t *a, int channel);
#define audiobuf_get_L(a) audiobuf_get(a, AUDIOBUF_CHANNEL_L)
#define audiobuf_get_R(a) audiobuf_get(a, AUDIOBUF_CHANNEL_R)

/* ----------------------------------------------------------------------
 * rewinds index only if index is out of range
 * ----------------------------------------------------------------------*/
void audiobuf_wrap(audiobuf_t *a);


#ifdef __cplusplus
}
#endif

#endif
