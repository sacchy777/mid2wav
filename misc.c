/*
 * misc tools
 *
 * misc.c
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
/* ----------------------------------------------------------------------
 * ----------------------------------------------------------------------*/
int fwrite4Bleu(FILE *fp, unsigned int value){
  int error = 0;
  unsigned char v[4];
  int i;
  v[0] = value & 0xff;
  v[1] = (value>>8) & 0xff;
  v[2] = (value>>16) & 0xff;
  v[3] = (value>>24) & 0xff;
  for(i = 0; i < 4; i ++){
    error = fwrite(&v[i], 1, 1, fp);
    if(error < 1){
      return error;
    }
  }
  return 1;
}

/* ----------------------------------------------------------------------
 * ----------------------------------------------------------------------*/
int fread4Bbeu(FILE *fp, unsigned int *ret){
  int i;
  int error;
  unsigned char v[4];
  for(i = 0; i < 4; i ++){
    error = fread(&v[i], 1, 1, fp);
    if(error < 1){
      return error;
    }
  }
  
  *ret = v[0]*65536*256 + v[1]*65536 + v[2]*256 + v[3];
  return 1;
}

/* ----------------------------------------------------------------------
 * ----------------------------------------------------------------------*/
int fread2Bbeu(FILE *fp, unsigned short *ret){
  int i;
  int error;
  unsigned char v[2];
  for(i = 0; i < 2; i ++){
    error = fread(&v[i], 1, 1, fp);
    if(error < 1) return error;
  }

  *ret = v[0] * 256 + v[1];
  return 1;
}

/* ----------------------------------------------------------------------
 * ----------------------------------------------------------------------*/
int fwrite2Bleu(FILE *fp, unsigned short value){
  int error = 0;
  unsigned char v[2];
  int i;
  v[0] = value & 0xff;
  v[1] = (value>>8) & 0xff;
  for(i = 0; i < 2; i ++){
    error = fwrite(&v[i], 1, 1, fp);
    if(error < 1){
      return error;
    }
  }
  return 1;
}


/* ----------------------------------------------------------------------
 * ----------------------------------------------------------------------*/
int fwrite2Bles(FILE *fp, short value){
  int error = 0;
  unsigned char v[2];
  int i;
  v[0] = value & 0xff;
  v[1] = (value>>8) & 0xff;
  for(i = 0; i < 2; i ++){
    error = fwrite(&v[i], 1, 1, fp);
    if(error < 1){
      return error;
    }
  }
  return 1;
}
