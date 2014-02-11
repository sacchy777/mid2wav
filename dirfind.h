#define DIRFIND_MAX 512
typedef struct {
  char **files;
  char *dir;
  int n_files;
  int index;
  char fullpath[DIRFIND_MAX];
} dirfind_t;

void dirfind_destroy(dirfind_t *d);
char *dirfind_get(dirfind_t *d);
void dirfind_dump(dirfind_t *d);
dirfind_t *dirfind_create(char *dir, char **ext, int n_ext);

