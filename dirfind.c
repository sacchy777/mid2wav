#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "dirfind.h"

static int match_ext(char *p, char *q){
  int lp, lq;
  int i;
  lp = strlen(p);
  lq = strlen(q);
  for(i = 1; i <= lq; i ++){
    if(lp - i < 0) return 0;
    if(p[lp - i] != q[lq - i]) return 0;
  }
  return 1;
}

dirfind_t *dirfind_create(char *dir, char **ext, int n_ext){
  DIR *dp;
  int n_files = 0;
  int i,j;
  struct dirent *entry;
  struct stat statbuf;
  dirfind_t *d;

#ifdef WINDOWS
  char separator = '\\';
#else
  char separator = '/';
#endif

  dp = opendir(dir);
  if(dp == NULL) return NULL;
  while((entry = readdir(dp)) != NULL){
    for(i = 0; i < n_ext; i ++){
      if(match_ext(entry->d_name, ext[i])){
	n_files ++;
      }
    }
  }

  if(n_files == 0){
    closedir(dp);
    return NULL;
  }

  d = malloc(sizeof(dirfind_t));
  if(d == NULL){
    closedir(dp);
    return NULL;
  }
  memset(d, 0, sizeof(dirfind_t));

  d->dir = malloc(sizeof(char)*(strlen(dir)+2));
  strcpy(d->dir, dir);
  if(dir[strlen(dir) - 1] != separator){
    d->dir[strlen(dir)] = separator;
    d->dir[strlen(dir) + 1] = '\0';
  }

  d->n_files = n_files;
  d->files = malloc(sizeof(char *) * n_files);
  if(d->files == NULL){
    free(d);
    closedir(dp);
    return NULL;
  }

  memset(d->files, 0, sizeof(char *) * n_files);
  
  seekdir(dp, 0);
  j = 0;
  while((entry = readdir(dp)) != NULL){
    for(i = 0; i < n_ext; i ++){
      if(match_ext(entry->d_name, ext[i])){
	d->files[j] = malloc(sizeof(char)*(strlen(entry->d_name)+1));
	if(d->files[j] == NULL){
	  dirfind_destroy(d);
	  closedir(dp);
	  return NULL;
	}
	strcpy(d->files[j], entry->d_name);
	j ++;
      }
    }
  }

  closedir(dp);
  return d;
}

void dirfind_destroy(dirfind_t *d){
  int i;
  for(i = 0;i < d->n_files; i++){
    if(d->files[i] != NULL) free(d->files[i]);
  }
  free(d->files);
  free(d->dir);
  free(d);
}

char *dirfind_get(dirfind_t *d){
  char *selected;
  selected = d->files[d->index ++];
  d->index %= d->n_files;
  strcpy(d->fullpath, d->dir);
  strcpy(&d->fullpath[strlen(d->dir)], selected);
  return d->fullpath;
}

void dirfind_dump(dirfind_t *d){
  int i;
  for(i = 0; i < d->n_files; i ++){
    printf("%s\n", d->files[i]);
  }
}
/*
void main(){
  dirfind_t * d;
  char *exts [] = {".mid"};
  int i;
  d = dirfind_create("./", exts, 1);
  dirfind_dump(d);
  for(i = 0; i < 25; i ++){
    printf("* %s\n", dirfind_get(d));
  }
  dirfind_destroy(d);
}
*/
