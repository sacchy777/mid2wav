/*
 * xorsfhit random generator
 *
 * xor128.h
 *
 * This is an implementation of the
 * www.jstatsoft.org/v08/i14/paper
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

#ifndef __XOR128_H__
#define __XOR128_H__

#ifdef __cplusplus
extern "C" {
#endif


/*---------------------------------------------------
 * Set a seed in 32bit integer.
 * default seed: 123456789
 *---------------------------------------------------*/
void xor128_init(unsigned long s);

/*---------------------------------------------------
 * Return a random value.
 * To get a random value of 0..9, write as this:
 * xor128() % 10 
 *---------------------------------------------------*/
unsigned long xor128();

/*---------------------------------------------------
 * Save a seed to a file.
 * Return value:
 * 0 or higher : done successfully
 * -1 : errors exist
 *---------------------------------------------------*/
int xor128_save(char *filename);

/*---------------------------------------------------
 * Load a seed from a file.
 * Return value:
 * 0 or higher : done successfully
 * -1 : errors exist
 *---------------------------------------------------*/
int xor128_load(char *filename);

#ifdef __cplusplus
}
#endif


#endif
