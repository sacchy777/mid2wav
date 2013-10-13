#include "audiobuf.h"
#include "midifile.h"
#include "soundmodule.h"
#include "sdelay.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEMPFILE "test.wav"

typedef struct {
  midifile_t *mf;
  audiobuf_t *a;
  soundmodule_t *s;
  sdelay_t *d;
} mid2wav_t;




void print_help(){
  printf(
	 "Usage: mid2wav [options] filename\n"
	 "Renders a midi file to a wave file with stereo, 44100Hz sampling raw PCM.\n"
	 "\n"
	 "  -h            Shows this help.\n"
	 "  -n            Renders without reverb.\n"
	 "  -v,-vv        Shows log verbosely when midi file parsing.\n"
	 "\n"
	 "mid2wav version 0.0 (very alpha-1)\n"
	 "\n"
	 );
}


int main(int argc, char *argv[]){
  int size = 0;
  mid2wav_t m;
  int i;
  char *filename;
  char infilename[256];
  char outfilename[256];
  int no_reverb = 0;
  int verbose_level = 0;

  memset(outfilename,0,sizeof(char)*256);
  memset(infilename,0,sizeof(char)*256);
  if(argc < 2){
    print_help();
    return EXIT_FAILURE;
  }

  i = 1;
  while(i < argc){
    if(argv[i][0] == '-'){
      switch(argv[i][1]){
      case '\0':
	printf("Illegal option: %s\n\n", argv[i]);
	print_help();
	return EXIT_FAILURE;
      case 'n':
	no_reverb = 1;
	break;
      case 'h':
	print_help();
	break;
      case 'v':
	switch(argv[i][2]){
	case 'v':
	  verbose_level = 2;
	  break;
	case '\0':
	  verbose_level = 1;
	  break;
	default:
	  printf("Illegal option: %s\n\n", argv[i]);
	  print_help();
	  return EXIT_FAILURE;
	}
	break;
      default:
	printf("Unsupported option.\n\n");
	print_help();
	return EXIT_FAILURE;
      }
    }else{
      filename = argv[i];
      if(strlen(filename)>256-5){
	printf("filenmae too long.\n");
	return EXIT_FAILURE;
      }
      strcpy(infilename, filename);
      strtok(filename, ".");
      strcpy(outfilename, filename);
      strcat(outfilename, ".wav");
    }
    i ++;
  }
  if(infilename[0] == '\0'){
    printf("File name not specified.\n");
    return EXIT_FAILURE;
  }


  m.mf = midifile_create(100000);
  midifile_set_verbose(m.mf, verbose_level);
  m.a = audiobuf_create_default(44100*10); /* 10 sec sample buffer */
  m.s = soundmodule_create();
  m.d = sdelay_create();
  sdelay_set_drywet(m.d, 0.8, 0.3);

  if(midifile_load(m.mf, infilename, 44100) < 0){
    printf("File could not loaded.");
    return EXIT_FAILURE;
  }
  
  //  midifile_dump(mf);
  audiobuf_save_header(m.a, outfilename);
  while(midifile_has_events(m.mf)){
    audiobuf_clear(m.a);
    soundmodule_render(m.s, m.a, m.mf);
    
    if(!no_reverb){
      audiobuf_rewind(m.a);
      sdelay_render(m.d, m.a, 0, 44100*10);
    }

    audiobuf_save_append(m.a, outfilename);
    size += 44100*10;
    if(size > 44100*1000){
      printf("Too many waves.\n");
      return EXIT_FAILURE;
    }
    //    putchar('.');
    printf("%d%%\r", midifile_progress(m.mf));
    fflush(stdout);
  }
  putchar('\n');
  printf("Successfully converted to %s.\n", outfilename);
  return EXIT_SUCCESS;
}
