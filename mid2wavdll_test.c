#include "mid2wavdll.h"
#include <dlfcn.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void print_help(){
  printf(
	 "Usage: mid2wav [options] filename\n"
	 "Renders a midi file to a wave file with stereo, 44100Hz sampling raw PCM.\n"
	 "\n"
	 "  -h            Shows this help.\n"
	 "  -n            Renders without reverb.\n"
	 "  -v,-vv        Shows log verbosely when midi file parsing.\n"
	 "\n"
	 );
}

int main(int argc, char *argv[]){
  mid2wav_t *m;
  int i;
  char *filename;
  char infilename[256];
  char outfilename[256];
  char version[256];
  int no_reverb = 0;
  int verbose_level = 0;

  void *handle;
  void *(*myopen)(void *, void *) = NULL;
  int (*myprocess)(void *) = NULL;
  void (*myversion)(char *) = NULL;
  int prog;

  handle = dlopen("mid2wav.dll", RTLD_LAZY);

  if(!handle){
    printf("Not found mid2wav.dll\n");
  }

  myopen = dlsym(handle, "mid2wav_open");
  if(!myopen){
    printf("Not found mid2wav_open\n");
  }

  myprocess = dlsym(handle, "mid2wav_process");
  if(!myprocess){
    printf("Not found mid2wav_process\n");
  }

  myversion = dlsym(handle, "mid2wav_version");
  if(!myversion){
    printf("Not found mid2wav_version\n");
  }


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

  m = myopen(infilename, outfilename);
  myversion(version);
  printf("%s\n", version);

  while(1){
    prog = myprocess(m);
    if(prog < 0 || prog >= 100) break;
    printf("%d%%\r", prog);
    fflush(stdout);
  }
  printf("Successfully converted to %s.\n", outfilename);

  dlclose(handle);

  return EXIT_SUCCESS;
}
