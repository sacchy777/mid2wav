#include "midifile.h"
#include <stdlib.h>
#define TEMPFILE "test.mid"
#define SAMPLING_RATE 44100
int main(int argc, char *argv[]){
  midifile_t *mf;
  mf = midifile_create_default();
  midifile_load(mf, TEMPFILE, SAMPLING_RATE);
  midifile_dump(mf);
  midifile_destroy(mf);
  return EXIT_SUCCESS;
}
