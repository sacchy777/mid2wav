/*
 * xorsfhit random generator
 *
 * xor128.h
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

#include "xor128.h"

#include <stdio.h>

static unsigned long v [] = {123456789,362436069,521288629,88675123};
/*---------------------------------------------------
 *---------------------------------------------------*/
void xor128_init(unsigned long s){v[0] = s;}

/*---------------------------------------------------
 *---------------------------------------------------*/
unsigned long xor128(){
  unsigned long t=(v[0]^(v[0]<<11));
  v[0]=v[1];v[1]=v[2];v[2]=v[3];
  return (v[3]=(v[3]^(v[3]>>19))^(t^(t>>8)));
}

/*---------------------------------------------------
 *---------------------------------------------------*/
int xor128_save(char *filename){
  int result = 0;
  FILE *fp = fopen(filename, "w");
  if(fp == NULL) return -1;
  result = fprintf(fp, "%lu %lu %lu %lu", v[0], v[1], v[2], v[3]); 
  fclose(fp);
  return result;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
int xor128_load(char *filename){
  int result = 0;
  FILE *fp = fopen(filename, "r");
  if(fp == NULL) return -1;
  result = fscanf(fp, "%lu %lu %lu %lu", &v[0], &v[1], &v[2], &v[3]); 
  fclose(fp);
  return result;
}
