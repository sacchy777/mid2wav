/*
 * ctplay
 *
 * ctplay.c
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

/*------------------------------------------------------------------
 *  ctplay
 *  a yet another midi player
 *  
 * to compile:
 *  make -f Makefile.linux
 * or
 *  make -f Makefile.windows
 *------------------------------------------------------------------*/

#include "SDL_config.h"
#if HAVE_SIGNAL_H
#include <signal.h>
#endif
#include "SDL.h"
#include "SDL_audio.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "midifile.h"
#include "audiobuf.h"
#include "soundmodule.h"
#include "sdelay.h"
#include "dirfind.h"
#include <sys/stat.h>
#include <math.h>

#define APPNAME "ctplay ver. 0.8"



SDL_mutex *lock;
/*------------------------------------------------------------------
 * strip path from full path filename
 *------------------------------------------------------------------*/
char * get_filename(char *fullpath){
  int len = 0;
  char *p = NULL;
  if(fullpath == NULL) return fullpath;
  len = strlen(fullpath);
  if(len == 0) return fullpath;
  p = &fullpath[len-1];
  while(p != fullpath){
    if(*(p-1) == '\\' || *(p-1) == '/'){
      return p;
    }
    p --;
  }
  return p;
}

/*------------------------------------------------------------------
 * mid2wav structure
 *------------------------------------------------------------------*/
typedef struct {
  int is_active;
  midifile_t *mf;
  audiobuf_t *a;
  soundmodule_t *s;
  sdelay_t *d;
} mid2wav_t;

SDL_AudioSpec spec;


/*------------------------------------------------------------------
 * GUI stuffs
 *------------------------------------------------------------------*/
#ifdef USE_GUI
#include "SDL_ttf.h"
int muted[16];
char *instnames[17] = {
  "Piano", "Chromatic", "Organ", "Guitar", "Bass", "Strings", "Ensemble",
  "Brass", "Reed", "Pipe", "Synth Lead", "Synth Pad", "Synth Effect", "Ethnic",
  "Percussive", "Sound Effects", "Drums",
};

SDL_Texture *sprite;
#define KEYBOARD_X 100
#define KEYBOARD_Y 30
#define INST_X 4
#define CAP_X 4

typedef struct {
  SDL_Texture *tex;
  SDL_Rect pos;
} sdltext_t;

#define NUM_SDLTEXT 17
sdltext_t texts[NUM_SDLTEXT];
sdltext_t currentfile;
sdltext_t voices;
sdltext_t key_height;
sdltext_t repeat;

int n_voices_skipped = 0;
int frameskipcounter = 0;

SDL_Color textcolor = {192,0,0,0xff};
SDL_Color textcolor_dark = {48,0,0,0xff};

/*------------------------------------------------------------------
 * trivial function for rendering text
 *------------------------------------------------------------------*/
void sdltext_render(sdltext_t *s, SDL_Renderer *r, TTF_Font *font, char *string, SDL_Color c){
  SDL_Surface *tmp;
  tmp = TTF_RenderText_Solid(font, string, c);
  if(!tmp)return;
  if(s->tex) SDL_DestroyTexture(s->tex);
  s->pos.w = tmp->w;
  s->pos.h = tmp->h;
  s->tex = SDL_CreateTextureFromSurface(r, tmp);
  SDL_FreeSurface(tmp);
}


TTF_Font *font1;
TTF_Font *font2;

#endif
/*
main body
*------------------------------------------------------------------*/
mid2wav_t m;
static int loaded = 0;
static int no_reverb = 0;
static int solo = -1;
static int midi_key = 0;
static int loop_enable = 1;
static float reverb = 0;
static float reverb_div = 1;//sqrt(0.3 * 0.3 + (1.0 - 0.3) * (1.0 - 0.3));


dirfind_t *df = NULL;
char current_filename[1024];

/*------------------------------------------------------------------
 * is inside?
 *------------------------------------------------------------------*/
static int is_inside(SDL_Rect *r, int x, int y){
  return x >= r->x && x < r->x + r->w && y >= r->y && y < r->y + r->h;
}

static void quit(int rc){
  SDL_Quit();
  exit(rc);
}


static int done = 0;

void poked(int sig){
  done = 1;
}



void delete();
void init(char *filename);

void fileopen(char *files){
  /*
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "", files, window );
  */
  SDL_LockMutex(lock);
  loaded = 0;
  memset(muted, 0, sizeof(int[16]));
  if(m.is_active) delete();
  init(files);
  strcpy(current_filename, get_filename(files));
  SDL_UnlockMutex(lock);
}
void diropen(char *dir){
  char *file;
  char *exts[] = {".mid", ".MID"};
  if(df != NULL) dirfind_destroy(df);
  df = dirfind_create(dir, exts, 2);
  if(df == NULL) return;
  file = dirfind_get(df);
  fileopen(file);
}


/*------------------------------------------------------------------
 * audio fill callback
 *------------------------------------------------------------------*/
void SDLCALL fill(void *unused, Uint8 *stream, int len){
  int i;
  int j = 0;
  int d;

  if(loaded == 0 || SDL_LockMutex(lock) != 0){
    memset(stream, 0, len);
    return;
  }
  if(!m.is_active) return;
  audiobuf_clear(m.a);
  soundmodule_render(m.s, m.a, m.mf);
  if(!no_reverb)
    sdelay_render(m.d, m.a, 0, m.a->size);
  for(i = 0; i < m.a->size; i ++){
    /* left */
    d = 0x8000 * (m.a->data[0][i]+1.0) - 0x8000;
    stream[j++] = d & 0xff;
    stream[j++] = (d & 0xff00) >> 8;
    /* right */
    d = 0x8000 * (m.a->data[1][i]+1.0) - 0x8000;
    stream[j++] = d & 0xff;
    stream[j++] = (d & 0xff00) >> 8;
  }
  if(midifile_has_events(m.mf) == 0){
    if(df != NULL){
      char *file = dirfind_get(df);
      fileopen(file);
    }else
    if(loop_enable){
      midifile_rewind(m.mf);
    }else{
      //      done = 1;
      ; // do nothing
    }
  }
  SDL_UnlockMutex(lock);
  //  fprintf(stderr, "%d%%", midifile_progress(m.mf));
  //  fflush(stderr);
}



/*------------------------------------------------------------------
 * loading keyboard bitmap
 *------------------------------------------------------------------*/
#ifdef USE_GUI
void load_bitmap(SDL_Renderer *r){
  SDL_Surface *temp;
  temp = SDL_LoadBMP("mid2wav_keyboard.bmp");
  if(temp == NULL){
    fprintf(stderr, "failed loading bitmaps.\n");
    exit(EXIT_FAILURE);
  }
  sprite = SDL_CreateTextureFromSurface(r, temp);
  if(!sprite){
    fprintf(stderr, "failed creating texture from bitmaps.\n");
    SDL_FreeSurface(temp);
    exit(EXIT_FAILURE);
  }
  SDL_FreeSurface(temp);
}
SDL_Rect poses[12] = {
  {0, 10, 4, 6},
  {3,  1, 3, 8},
  {5, 10, 4, 6},
  {8,  1, 3, 8},
  {10, 10, 4, 6},
  {15, 10, 4, 6},
  {18,  1, 3, 8},
  {20, 10, 4, 6},
  {23,  1, 3, 8},
  {25, 10, 4, 6},
  {28,  1, 3, 8},
  {30, 10, 4, 6},
};


/*------------------------------------------------------------------
 * periodical draw function
 *------------------------------------------------------------------*/

void draw(SDL_Renderer *r){
  int i;
  int j;
  SDL_Rect pos;
  SDL_Rect box;
  int key;
  int is_offing;
  int key_d;
  char buf[256];

  frameskipcounter ++;
  pos.w = 280;
  pos.h = 16;
  pos.x = KEYBOARD_X;


  sdltext_render(&currentfile, r, font2, current_filename, textcolor);
  SDL_SetRenderDrawColor(r, 64, 0, 0, 0xff);
  SDL_RenderClear(r);
  for(i = 0; i < 16; i ++){
    pos.y = KEYBOARD_Y + i * 17;
    SDL_RenderCopy(r, sprite, NULL, &pos);

    if(!m.is_active) continue;
    //    if(i == 9) continue;
    for(j = 0; j < 8; j ++){ /* wtssynth's inst is 8, nrsynth's inst is 4, so can cover */
      key = soundmodule_get_current_key(m.s, i, j);
      if(key == -1) continue;
      if(key < 12 || key >= 12 * 8 + 12) continue;
      key -= 12;
      n_voices_skipped ++;
      is_offing = soundmodule_is_key_offing(m.s, i, j);
      key_d = key % 12;
      box.x = poses[key_d].x + (key/12) * 35 + KEYBOARD_X;
      box.y = poses[key_d].y + i * 17 + KEYBOARD_Y;
      box.w = poses[key_d].w;
      box.h = poses[key_d].h;
      SDL_SetRenderDrawColor(r, is_offing?192:240, 0, 0, 0xff);
      SDL_RenderFillRect(r, &box);
    }
  }


  if(m.is_active){
    for(i = 0; i < 16; i ++){
      if(i != 9){
	SDL_LockMutex(lock);
	j = soundmodule_get_program(m.s, i);
	SDL_UnlockMutex(lock);
	sdltext_render(&texts[i+1], r, font2, instnames[j/8], muted[i] == 0 ? textcolor : textcolor_dark);
      }else{
	sdltext_render(&texts[i+1], r, font2, instnames[16], muted[i] == 0 ? textcolor : textcolor_dark);
      }
    }
    for(i = 1; i < 17; i ++)
      SDL_RenderCopy(r, texts[i].tex, NULL, &texts[i].pos);
  }
  SDL_RenderCopy(r, texts[0].tex, NULL, &texts[0].pos);
  SDL_RenderCopy(r, currentfile.tex, NULL, &currentfile.pos);
  if(frameskipcounter%1000 == 0){n_voices_skipped /= 1000;
  sprintf(buf, "Voices %d", n_voices_skipped);
  n_voices_skipped = 0;
  sdltext_render(&voices, r, font2, buf, textcolor);
  voices.pos.x = 180;
  voices.pos.y = 8;
  }
  SDL_RenderCopy(r, voices.tex, NULL, &voices.pos);


  sprintf(buf, "< > key %d", midi_key);
  sdltext_render(&key_height, r, font2, buf, textcolor);
  SDL_RenderCopy(r, key_height.tex, NULL, &key_height.pos);

  sdltext_render(&repeat, r, font2, "repeat", loop_enable?textcolor:textcolor_dark);
  SDL_RenderCopy(r, repeat.tex, NULL, &repeat.pos);



  SDL_RenderPresent(r);
}
#endif

/*------------------------------------------------------------------
 * delete memoreis
 *------------------------------------------------------------------*/
void delete(){
  sdelay_destroy(m.d);
  soundmodule_destroy(m.s);
  audiobuf_destroy(m.a);
  midifile_destroy(m.mf, 1);
  memset(&m, 0, sizeof(mid2wav_t));
}

/*------------------------------------------------------------------
 * alloc memoreis
 *------------------------------------------------------------------*/
void init(char *filename){
  m.mf = midifile_create(100000);
  midifile_set_verbose(m.mf, 0);

#ifdef WINDOWS
  m.a = audiobuf_create_default(spec.samples);
#elif defined LINUX
  m.a = audiobuf_create_default(spec.samples / 2);
#else
  m.a = audiobuf_create_default(spec.samples);
#endif

  m.s = soundmodule_create();
  if(solo > 0 && solo <= 16){
    soundmodule_solo(m.s, solo - 1);
  }
  m.d = sdelay_create();
  //  sdelay_set_drywet(m.d, 0.8, 0.3);
  sdelay_set_drywet(m.d, (1.0 - reverb)/reverb_div, reverb/reverb_div);
  if(midifile_load(m.mf, filename, 44100) < 0){
    fprintf(stderr, "failed to load %s\n", filename);
    delete();
    return;
    //exit(EXIT_FAILURE);
  }
  //  fprintf(stderr, "loaded successfully\n");
  //  fflush(stderr);
  soundmodule_midi_key(m.s, midi_key);
  m.is_active = 1;
  loaded = 1;
}


/*------------------------------------------------------------------
 *
 *------------------------------------------------------------------*/
void print_help(){
    printf(
	   "Usage: ctplay [options] [filename]\n"
	   "  A yet another chip tune midi player\n"
	   "\n"
	   "  -r [reverb]            add reverb 0..100\n"
	   "  -k [key]               transpose by key in halftone\n"
	   "  -s [channel]           solo play with the specified channel(1..16)\n"
	   "  --no-repeat            start without repeat\n"
	   "\n"
	   APPNAME
	   );
}

/*------------------------------------------------------------------
 * main func
 *------------------------------------------------------------------*/
int main(int argc, char *argv[]){
  char *filename = NULL;
  char *files;
  int i;

#ifdef USE_GUI
  SDL_Window *window;
  SDL_Renderer *renderer;
  SDL_Event event;
  SDL_Surface *icon;
#endif
  

  SDL_AudioDeviceID dev;
  current_filename[0] = 0;

  lock = SDL_CreateMutex();

  
#if HAVE_SIGNAL_H
#ifdef SIGHUP
  signal(SIGHUP, poked);
#endif
  signal(SIGINT, poked);
#ifdef SIGQUIT
  signal(SIGQUIT, poked);
#endif
  signal(SIGTERM, poked);
#endif

  /*
   *
   * parse args
   *
   */

  for(i = 1; i < argc; i ++){
    /*
    if(strcmp("-n", argv[i]) == 0){
      no_reverb = 1;
      continue;
    }
    */
    if(strcmp("--no-repeat", argv[i]) == 0){
      loop_enable = 0;
      continue;
    }

    if(strcmp("-h", argv[i]) == 0){
      print_help();
      continue;
    }

    if(strcmp("-s", argv[i]) == 0){
      if(i + 1 == argc){
	printf("no parameter after option -s\n");
	return EXIT_FAILURE;
      }
      i ++;
      solo = atoi(argv[i]);
      if(solo == 0 || solo > 16){
	printf("unsuported parameter %s as option -s\n", argv[i]);
	return EXIT_FAILURE;
      }
      continue;
    }

    if(strcmp("-r", argv[i]) == 0){
      int reverb_int;
      if(i + 1 == argc){
	printf("no parameter after option -r\n");
	return EXIT_FAILURE;
      }
      i ++;
      reverb_int = atoi(argv[i]);
      if(reverb_int < 0 || reverb_int > 100){
	printf("unsuported parameter %s as option -r\n", argv[i]);
	return EXIT_FAILURE;
      }
      reverb = (float)reverb_int / 100.0;
      reverb_div = sqrt(reverb * reverb + (1.0-reverb)*(1.0-reverb));
      continue;
    }

    if(strcmp("-k", argv[i]) == 0){

      if(i + 1 == argc){
	printf("no parameter after option -k\n");
	return EXIT_FAILURE;
      }
      i ++;
      midi_key = atoi(argv[i]);
      if(midi_key == 0 && strcmp("0", argv[i]) != 0){
	printf("ignored parameter %s as option -s\n", argv[i]);
	return EXIT_FAILURE;
      }
      continue;
    }

    if(argv[i][0] == '-'){
      printf("unsupported option: %s\n", argv[i]);
      return EXIT_FAILURE;
    }
    filename = argv[i];
  }



  /*
   *
   * SDL initialize
   *
   */

#ifdef USE_GUI
  memset(muted, 0, sizeof(muted));
  if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0){
    fprintf(stderr, "Failed SDL_Init().\n");
    return EXIT_FAILURE;
  }
  if(SDL_CreateWindowAndRenderer(KEYBOARD_X + 280 + 4, KEYBOARD_Y + 17*16 + 4, 0, &window, &renderer) < 0){
    fprintf(stderr, "Failed SDL_CreateWindowAndRenderer().\n");
    return EXIT_FAILURE;
  }
  icon = SDL_LoadBMP("icon.bmp");
  if(!icon){
    fprintf(stderr, "Failed SDL_LoadBMP() for icon.\n");
    return EXIT_FAILURE;
  }
  SDL_SetWindowTitle(window, APPNAME);
  SDL_SetWindowIcon(window, icon);
  SDL_FreeSurface(icon);

  load_bitmap(renderer);

  TTF_Init();
  font1 = TTF_OpenFont("linesquare_rounded_extended.ttf", 40);
  font2 = TTF_OpenFont("linesquare_rounded_extended.ttf", 21);
  if(font1 == NULL || font2 == NULL){
    fprintf(stderr, "Failed TTF_OpenFont(). %s\n", SDL_GetError());
    return EXIT_FAILURE;
  }

  memset(texts, 0, sizeof(sdltext_t) * NUM_SDLTEXT);

  sdltext_render(&texts[0], renderer, font1, APPNAME, textcolor);
  texts[0].pos.x = CAP_X;

  memset(&repeat, 0, sizeof(sdltext_t));
  repeat.pos.x = 250;
  repeat.pos.y = 8;
  
  memset(&key_height, 0, sizeof(sdltext_t));
  key_height.pos.x = 310;
  key_height.pos.y = 8;
  

  for(i = 0; i < 16; i ++){
    sdltext_render(&texts[i+1], renderer, font2, instnames[0], textcolor);
    texts[i+1].pos.y = KEYBOARD_Y + i * 17 - 1;
    texts[i+1].pos.x = INST_X;
  }

  sdltext_render(&currentfile, renderer, font2, "no file loaded", textcolor);
  currentfile.pos.x = 180;
  currentfile.pos.y = 0;
#else
  if(SDL_Init(SDL_INIT_AUDIO) < 0){
    fprintf(stderr, "Failed SDL_Init().\n");
    return EXIT_FAILURE;
  }
#endif

  SDL_zero(spec);
  spec.freq = 44100;
  spec.format = AUDIO_S16LSB;
  spec.channels = 2;
  spec.samples = 2048;
  spec.callback = fill;

  dev = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, SDL_AUDIO_ALLOW_FORMAT_CHANGE);
  if(dev == 0){
    fprintf(stderr, "could not SDL_OpenAudioDevice().\n");
    return EXIT_FAILURE;
  }

  //  fprintf(stderr, "%d %d\n", spec.freq, spec.samples);
  //  fflush(stderr);


  if(filename){
    char *tmp;
    init(filename);
    tmp = get_filename(filename);
    sdltext_render(&currentfile, renderer, font2, tmp, textcolor);
  }


  SDL_PauseAudioDevice(dev, 0);

  /*
   *
   * Main loop
   *
   */

#ifdef USE_GUI
  while(!done){
    while(SDL_PollEvent(&event)){
      if(event.type == SDL_QUIT){
	done = 1;
      }
      if(event.type == SDL_DROPFILE){
	struct stat statbuf;
	files = event.drop.file;
	stat(files, &statbuf);
	if(S_ISDIR(statbuf.st_mode)){
	  diropen(files);
	}else{
	  if(df != NULL){
	    dirfind_destroy(df);
	    df = NULL;
	  }
	  fileopen(files);
	}
	SDL_free(files);
      }
      if(event.type == SDL_MOUSEBUTTONUP){
	fflush(stdout);
	int mx = event.button.x;
	int my = event.button.y;
	int chan = (my - KEYBOARD_Y) / 17;
	if(is_inside(&repeat.pos, mx, my)){
	  loop_enable = !loop_enable;
	}
	if(mx >= key_height.pos.x && mx < key_height.pos.x + 7 &&
	   my >= key_height.pos.y && my < key_height.pos.y + 10){
	  midi_key --;
	  if(m.is_active){
	    soundmodule_midi_key(m.s, midi_key);
	  }
	}
	if(mx >= key_height.pos.x + 8 && mx < key_height.pos.x + 7 + 8 &&
	   my >= key_height.pos.y && my < key_height.pos.y + 10){
	  midi_key ++;
	  if(m.is_active){
	    soundmodule_midi_key(m.s, midi_key);
	  }
	}
							       
	//	printf("event %d %d %d \n", mx, my, chan);
	if(m.is_active){
	  /* channel toggle */
	  if(mx > 0 && mx < KEYBOARD_X && chan >= 0 && chan < 16 ){
	    if(event.button.button == SDL_BUTTON_LEFT){

	      if(soundmodule_is_muted(m.s, chan)){
		soundmodule_unmute(m.s, chan);
		muted[chan] = 0;
	      }else{
		soundmodule_mute(m.s, chan);
		muted[chan] = 1;
	      }
	    }
	    else if(event.button.button == SDL_BUTTON_RIGHT){
	      int k;
	      int solo_key = -1;
	      if(chan < 0 || chan >= 16) continue;
	      for(k = 0 ; k < 16; k ++){
		if(soundmodule_is_muted(m.s, k)) continue;
		if(solo_key == -1){
		  solo_key = k;
		}else{
		  solo_key = -1;
		  break;
		}
	      }

	      if(solo_key != chan){
		for(k = 0 ; k < 16; k ++){
		  if(k == chan){
		    soundmodule_unmute(m.s, k);
		    muted[k] = 0;
		  }else{
		    soundmodule_mute(m.s, k);
		    muted[k] = 1;
		  }
		}
	      }else{
		for(k = 0 ; k < 16; k ++){
		  soundmodule_unmute(m.s, k);
		  muted[k] = 0;
		}
	      }

	    }
	  }
	}
	/*end of event handling */
      }
    }
    draw(renderer);
SDL_Delay(1);
  }
#else
  while(!done){
    SDL_Delay(1000);
  }
#endif

  TTF_CloseFont(font1);
  TTF_CloseFont(font2);
  SDL_CloseAudio();
  SDL_Quit();
  if(m.is_active)
    delete();

  SDL_DestroyMutex(lock);
  if(df != NULL) dirfind_destroy(df);
  return EXIT_SUCCESS;
}
