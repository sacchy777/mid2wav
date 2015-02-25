/*
 * Audio Buffer library for audio processing
 *
 * audiobuf.c
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "audiobuf.h"
#include "misc.h"

/* ----------------------------------------------------------------------
 * ----------------------------------------------------------------------*/
audiobuf_t *audiobuf_create(int size, int n_channels, int sampling_rate, int qbit){
  int i;
  audiobuf_t *a;
  a = (audiobuf_t *)malloc(sizeof(audiobuf_t));
  if(a == NULL) goto error;
  memset(a, 0, sizeof(audiobuf_t));

  a->data = (float**)malloc(sizeof(float*) * n_channels);
  if(a->data == NULL) goto error;
  memset(a->data, 0, sizeof(float *) * n_channels);

  a->index = (int*)malloc(sizeof(int) * n_channels);
  if(a->index == NULL) goto error;
  memset(a->index, 0, sizeof(int) * n_channels);

  for(i = 0; i < n_channels; i ++){
    a->data[i] = (float *)malloc(sizeof(float) * size);
    if(a->data[i] == NULL) goto error;
  }
  a->n_channels = n_channels;
  a->size = size;
  a->qbit = qbit;
  a->sampling_rate = sampling_rate;
  audiobuf_clear(a);
  return a;
 error:
  audiobuf_destroy(a);
  return NULL;
}



/* ----------------------------------------------------------------------
 * ----------------------------------------------------------------------*/
void audiobuf_destroy(audiobuf_t *a){
  int i;
  if(a == NULL) return;
  if(a->data == NULL) goto free1;
  if(a->index == NULL) goto free2;
  for(i = 0; i < a->n_channels; i ++){
    if(a->data[i] == NULL){
      goto free3;
    }else{
      free(a->data[i]);
    }
  }
 free3:
  free(a->index);
 free2:
  free(a->data);
 free1:
  free(a);
}

/* ----------------------------------------------------------------------
 * ----------------------------------------------------------------------*/
void audiobuf_copy(audiobuf_t *src, audiobuf_t *dst){
  int i,j;
  int maxdata = src->size < dst->size ? src->size : dst->size;
  int maxchannel = src->n_channels < dst->n_channels ? src->n_channels : dst->n_channels;
  for(i = 0; i < maxchannel; i ++){
    for(j = 0; j < maxdata; j ++){
      dst->data[i][j] = src->data[i][j];
    }
  }
}

/* ----------------------------------------------------------------------
 * ----------------------------------------------------------------------*/
void audoibuf_merge(audiobuf_t *src, audiobuf_t *dst){
  int i,j;
  int maxdata = src->size < dst->size ? src->size : dst->size;
  int maxchannel = src->n_channels < dst->n_channels ? src->n_channels : dst->n_channels;
  for(i = 0; i < maxchannel; i ++){
    for(j = 0; j < maxdata; j ++){
      dst->data[i][j] += src->data[i][j];
    }
  }
}


/* ----------------------------------------------------------------------
 * ----------------------------------------------------------------------*/
audiobuf_t *audiobuf_duplicate(audiobuf_t *a){
  audiobuf_t *b;
  b = audiobuf_create(a->size, a->n_channels, a->sampling_rate, a->qbit);
  audiobuf_copy(a, b);
  return b;
}


/* ----------------------------------------------------------------------
 * ----------------------------------------------------------------------*/
void audiobuf_clear(audiobuf_t *a){
  int i;
  for(i = 0; i < a->n_channels; i++){
    memset(a->data[i], 0, sizeof(float) * a->size);
    a->index[i] = 0;
  }
}

/* ----------------------------------------------------------------------
 * ----------------------------------------------------------------------*/
void audiobuf_rewind(audiobuf_t *a){
  int i;
  for(i = 0; i < a->n_channels; i++){
    a->index[i] = 0;
  }
}

/* ----------------------------------------------------------------------
 * ----------------------------------------------------------------------*/
void audiobuf_seek(audiobuf_t *a, int pos){
  int i;
  for(i = 0; i < a->n_channels; i++){
    a->index[i] = pos;
  }
}

/* ----------------------------------------------------------------------
 * ----------------------------------------------------------------------*/
void audiobuf_add(audiobuf_t *a, int channel, float data){
  a->data[channel][a->index[channel]] += data;
  a->index[channel] ++;
}

/* ----------------------------------------------------------------------
 * ----------------------------------------------------------------------*/
void audiobuf_write(audiobuf_t *a, int channel, float data){
  a->data[channel][a->index[channel]] = data;
  a->index[channel] ++;
}

float audiobuf_get(audiobuf_t *a, int channel){
  return a->data[channel][a->index[channel]];
}

/* ----------------------------------------------------------------------
 * ----------------------------------------------------------------------*/
void audiobuf_wrap(audiobuf_t *a){
  int i;
  for(i = 0 ; i < a->n_channels; i ++){
    if(a->index[i] >= a->size) a->index[i] = 0;
  }
}


/* ----------------------------------------------------------------------
 * ----------------------------------------------------------------------*/
int audiobuf_save_append(audiobuf_t *a, const char *filename){
  int i, j;
  int d;
  size_t filesize;
  FILE *fp;
  struct stat statbuf;

  fp = fopen(filename, "ab");
  if(fp == NULL) return -1;

  /* write raw pcm data */
  for(i = 0 ; i < a->size; i ++){
    for(j = 0; j < a->n_channels; j ++){
      d = 0x8000 * (a->data[j][i]+1.0) - 0x8000;
      fwrite2Bles(fp, d);
    }
  }
  fclose(fp); /* To get next fseek work correctly, I have to close the file here */
  
  fp = fopen(filename, "r+b");
  if(fp == NULL) return -1;
  fstat(fileno(fp), &statbuf);
  filesize = statbuf.st_size;
  fseek(fp, 4L, SEEK_SET);
  fwrite4Bleu(fp, filesize - 8);
  fseek(fp, 40L, SEEK_SET);
  fwrite4Bleu(fp, filesize - 44);
  fclose(fp);

  return 0;
}

/* ----------------------------------------------------------------------
 * ----------------------------------------------------------------------*/
int audiobuf_save_header(audiobuf_t *a, const char *filename){
  FILE *fp;

  fp = fopen(filename, "wb");
  if(fp == NULL) return -1;

  fwrite("RIFF", 4, 1, fp); /* RIFF Header*/
  fwrite4Bleu(fp, 0); /* payload size */
  fwrite("WAVE", 4, 1, fp); /* WAVE Header*/
  fwrite("fmt ", 4, 1, fp); /* fmt chunk Header*/
  fwrite4Bleu(fp, 16); /* fmt chunk size in bytes */
  fwrite2Bleu(fp, 1); /* format ID of raw PCM */
  fwrite2Bleu(fp, a->n_channels); /* number of channels */
  fwrite4Bleu(fp, a->sampling_rate); /* sampling rate in Hz */
  fwrite4Bleu(fp, a->sampling_rate*a->n_channels*a->qbit/8); /* data rate */
  fwrite2Bleu(fp, a->n_channels*a->qbit/8); /* block size */
  fwrite2Bleu(fp, a->qbit); /* bit per sample */
  fwrite("data", 4, 1, fp); /* data chunk */
  fwrite4Bleu(fp, 0); /* data size in bytes */

  fclose(fp);

  return 0;
}




