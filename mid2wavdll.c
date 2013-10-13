/*
 * mid2wavdll.c
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

#include "mid2wavdll.h"


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*---------------------------------------------------
 *---------------------------------------------------*/
__declspec(dllexport) void __cdecl mid2wav_version(char *version){
  strcpy(version, "mid2wav.dll ver. 0.0(very alpha-1)");
}

/*---------------------------------------------------
 *---------------------------------------------------*/
__declspec(dllexport) mid2wav_t __cdecl *mid2wav_open(char *infilename, char *outfilename){
  mid2wav_t *m;
  m = malloc(sizeof(mid2wav_t));
  m->size = 0;
  memset(m, 0, sizeof(mid2wav_t));
  m->mf = midifile_create(100000);
  midifile_set_verbose(m->mf, 0);
  m->a = audiobuf_create_default(44100*10); /* 10 sec sample buffer */
  m->s = soundmodule_create();
  m->d = sdelay_create();
  sdelay_set_drywet(m->d, 0.8, 0.3);
  if(midifile_load(m->mf, infilename, 44100) < 0){
    return NULL;
  }
  audiobuf_save_header(m->a, outfilename);
  strcpy(m->outfilename, outfilename);
  return m;
}

/*---------------------------------------------------
 *---------------------------------------------------*/
__declspec(dllexport) int __cdecl mid2wav_process(mid2wav_t *m){
  int prog;
  audiobuf_clear(m->a);
  soundmodule_render(m->s, m->a, m->mf);
    
  audiobuf_rewind(m->a);
  sdelay_render(m->d, m->a, 0, 44100*10);

  audiobuf_save_append(m->a, m->outfilename);
  m->size += 44100*10;
  if(m->size > 44100*1000){
    printf("Too many waves.\n");
    return EXIT_FAILURE;
  }
  prog = midifile_progress(m->mf);
  return prog;
}

/*---------------------------------------------------
 * needed?
 *---------------------------------------------------*/
__declspec(dllexport) void __cdecl mid2wav_close(mid2wav_t *m){
  if(m != NULL){
    audiobuf_destroy(m->a);
    soundmodule_destroy(m->s);
    sdelay_destroy(m->d);
  }
  free(m);
}

