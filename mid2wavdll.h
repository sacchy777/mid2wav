/*
 * mid2wavdll.h
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

#ifndef MID2WAVDLL_H
#define MID2WAVDLL_H

#ifdef __cplusplus
extern "C" {
#endif

#include "audiobuf.h"
#include "midifile.h"
#include "soundmodule.h"
#include "sdelay.h"

/*---------------------------------------------------
 * main structure
 *---------------------------------------------------*/
typedef struct {
  midifile_t *mf;
  audiobuf_t *a;
  soundmodule_t *s;
  sdelay_t *d;
  int size;
  char outfilename[255];
} mid2wav_t;

/*---------------------------------------------------
 * open a midi file.
 *---------------------------------------------------*/
__declspec(dllexport) mid2wav_t __cdecl *mid2wav_open(char *infilename, char *outfilename);

/*---------------------------------------------------
 * render waves to the file.
 * return progress from 0 to 100
 *---------------------------------------------------*/
__declspec(dllexport) int __cdecl mid2wav_process(mid2wav_t *m);

/*---------------------------------------------------
 * return version string.
 * this destroys original buffer
 *---------------------------------------------------*/
__declspec(dllexport) void __cdecl mid2wav_version(char *version);


/*---------------------------------------------------
 *---------------------------------------------------*/
__declspec(dllexport) void __cdecl mid2wav_close(mid2wav_t *m);


#ifdef __cplusplus
}
#endif

#endif
